// XFAIL: Linux
// RUN: %hc %s -o %t.out && %t.out

/* This test case reveals multiple problems in the hcc runtime:

   Due to a race condition, if the kernel execution is not fast
   enough, there will be a too early release of temporary buffers
   in ~rw_info().  If this issue is circumvented by disabling the
   freeing, there's another race in copying back the results from
   the first kernel execution. If the kernel doesn't finish fast
   enough, the result reading doesn't get the produced results,
   but the input data.

   The below patch reproduces the problem in Carrizo by adding
   a delay to the dispatch packet signaling:

diff --git a/lib/hsa/mcwamp_hsa.cpp b/lib/hsa/mcwamp_hsa.cpp
index 15a040b..45e8c76 100644
--- a/lib/hsa/mcwamp_hsa.cpp
+++ b/lib/hsa/mcwamp_hsa.cpp
@@ -3162,8 +3162,19 @@ HSADispatch::dispatchKernel(hsa_queue_t* commandQueue, const void *hostKernarg,
     std::cerr << "ring door bell to dispatch kernel\n";
 #endif
 
+    std::thread([commandQueue, index]() {
+	// With a 500 ms delay here it works. Increasing it to about 650 ms
+	// and above starts to introduce errors due to the results being
+	// read out before finishing the computatiton.
+	std::this_thread::sleep_for(std::chrono::milliseconds(650));
+	hsa_signal_store_relaxed(commandQueue->doorbell_signal, index);
+#if KALMAR_DEBUG
+	std::cerr << "dispatched kernel\n";
+#endif
+      }).detach();
+
     // Ring door bell
-    hsa_signal_store_relaxed(commandQueue->doorbell_signal, index);
+    //hsa_signal_store_relaxed(commandQueue->doorbell_signal, index);
 
     isDispatched = true;
   

 */

#include <random>
#include <algorithm>
#include <vector>
#include <iostream>
#include <cmath>

// header file for the hc API
#include <hc.hpp>

int main() {

  constexpr int N = 1024 * 1024 * 64;
  constexpr float a = 100.0f;

  std::vector<float> host_x(N);
  std::vector<float> host_y(N);

  // initialize the input data
  std::default_random_engine random_gen;
  std::uniform_real_distribution<float> distribution(-N, N);
  std::generate(host_x.begin(), host_x.end(), [&]() { return distribution(random_gen); });
  std::generate(host_y.begin(), host_y.end(), [&]() { return distribution(random_gen); });

  // CPU implementation of saxpy
  
  std::vector<float> host_result_y(N);
  for (int i = 0; i < N; i++) {
    host_result_y[i] = a * host_x[i] + host_y[i];
  }
  
  std::vector<hc::accelerator> all_accelerators = hc::accelerator::get_all();
  std::vector<hc::accelerator> accelerators;
  for (auto a = all_accelerators.begin(); a != all_accelerators.end(); a++) {

    // only pick accelerators supported by the HSA runtime
    if (a->is_hsa_accelerator()) {
      accelerators.push_back(*a);
    }
  }

  constexpr int numViewPerAcc = 2;
  int numSaxpyPerView = N/(accelerators.size() * numViewPerAcc);

  std::vector<hc::accelerator_view> acc_views;

  std::vector<hc::completion_future> futures;

  std::vector<hc::array<float,1>> x_arrays;
  std::vector<hc::array<float,1>> y_arrays;



  int dataCursor = 0;
  for (auto acc = accelerators.begin(); acc != accelerators.end(); acc++) {
    for (int i = 0; i < numViewPerAcc; i++) {

      // create a new accelerator_view
      acc_views.push_back(acc->create_view());

      int newDataCursor = dataCursor+numSaxpyPerView;

      // create array_views that only covers the data portion needed by this accelerator_view
      x_arrays.push_back(hc::array<float,1>(numSaxpyPerView, acc_views.back()));
      auto& x_array = x_arrays.back();

      hc::completion_future cpfuture_x = hc::copy_async(host_x.begin() + dataCursor
                                                        , host_x.begin() + newDataCursor
                                                        , x_array);

      y_arrays.push_back(hc::array<float,1>(numSaxpyPerView, acc_views.back()));
      auto& y_array = y_arrays.back();

      hc::completion_future cpfuture_y = hc::copy_async(host_y.begin() + dataCursor
                                                        , host_y.begin() + newDataCursor
                                                        , y_array);
      dataCursor = newDataCursor;
      cpfuture_x.wait();
      cpfuture_y.wait();
      
      hc::completion_future f;
      f = hc::parallel_for_each(acc_views.back(), x_array.get_extent()
                            , [&,a](hc::index<1> i) [[hc]] {
        y_array[i] = a * x_array[i] + y_array[i];
      });
      futures.push_back(f);

      //printf("dataCursor: %d\n",dataCursor);
    }
  }

  // If N is not a multiple of the number of acc_views,
  // calculate the remaining saxpy on the host
  for (; dataCursor!=N; dataCursor++) {
    host_y[dataCursor] = a * host_x[dataCursor] + host_y[dataCursor];
  }

  // synchronize all the results back to the host
  auto kernel_future = futures.begin();
  dataCursor = 0;
  for(auto v = y_arrays.begin(); v != y_arrays.end(); v++) {
    kernel_future->wait();
    *kernel_future = hc::copy_async(*v, host_y.begin() + dataCursor);
    dataCursor+=numSaxpyPerView;
    kernel_future++;
  }

  // wait for the copies to complete
  for(auto f = futures.begin(); f != futures.end(); f++) {
    f->wait();
  }
 
  // verify the results
  int errors = 0;
  for (int i = 0; i < N; i++) {
    if (fabs(host_y[i] - host_result_y[i]) > fabs(host_result_y[i] * 0.0001f))
      errors++;
  }
  //std::cout << errors << " errors" << std::endl;

  return errors;
}
