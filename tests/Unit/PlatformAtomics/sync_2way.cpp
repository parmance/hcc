// XFAIL: Linux
// RUN: %hc %s -o %t.out && %t.out

#include <iostream>
#include <iomanip>
#include <random>
#include <atomic>
#include <thread>
#include <chrono>
#include <amp.h>

// added for checking HSA profile
#include <hc.hpp>

// test C++AMP with platform atomics functions
// requires HSA Full Profile to operate successfully

bool test() {
  // define inputs and output
  const int vecSize = 16;
  const int cpuSleepMsec = 25;

  std::atomic_int table_a[vecSize];
  auto ptr_a = &table_a[0];

  std::atomic_int thread_flag;
  auto ptr_flag = &thread_flag;

  std::atomic_bool done(false);
  auto ptr_done = &done;

  // initialize test data
  for (int i = 0; i < vecSize; ++i) {
    table_a[i].store(0);
  }
  thread_flag.store(0);

  // fire CPU thread
  std::thread cpu_thread([=]() {
    std::cout << "Enter CPU monitor thread..." << std::endl;
    std::chrono::milliseconds dura( cpuSleepMsec );
    int val;
    while (!*ptr_done) {
      for (int i = 0; i < vecSize; ++i) {
        val = (ptr_a + i)->load(std::memory_order_acquire);
        std::cout << std::setw(2) << val;
        if (i < vecSize - 1) {
          std::cout << ", ";
        } else {
          std::cout << std::endl;
        }

        if (val == 1) {
          ptr_flag->fetch_add(1, std::memory_order_acq_rel);
        }
      }

      std::this_thread::sleep_for( dura );
    }
    std::cout << "Leave CPU monitor thread." << std::endl;
  });

  // launch kernel
  Concurrency::extent<1> e(vecSize);
  parallel_for_each(
    e,
    [=](Concurrency::index<1> idx) restrict(amp) {

    int tid = idx[0];
    int flag;
    while (true) {
      flag = ptr_flag->load(std::memory_order_acquire);

      // HSA does not guarantee forward progress among work-items in
      // the same work group without barriers. It is also unclear where
      // the barrier is suposed to be, thus whether this test case is
      // a legal one or not with HSA as it assumes work-items proceed
      // despite the "for ever loop". Add barrier here to make it work
      // with phsa which wants to execute the work-items of this
      // kernel sequentially.
      amp_barrier(CLK_GLOBAL_MEM_FENCE);

      if (flag == vecSize) {
        break;
      }

      if (flag == tid) {
        (ptr_a + tid)->store(1, std::memory_order_release);
      } else {
        (ptr_a + tid)->store(0, std::memory_order_release);
      }
    }
  });

  // stop CPU thread
  done.store(true);
  cpu_thread.join();

  // Verify
  std::cout << "Final value:\n";
  int error = 0;
  int val = 0;
  for (int i = 0; i < vecSize; ++i) {
    val = (ptr_a + i)->load();

    std::cout << std::setw(2) << val;
    if (i < vecSize - 1) {
      std::cout << ", ";
      if (val != 0) {
        ++error;
      }
    } else {
      std::cout << std::endl;
      if (val != 1) {
        ++error;
      }
    }
  }

  if (error == 0) {
    std::cout << "Verify success!\n";
  } else {
    std::cout << "Verify failed!\n";
  }
  return (error == 0);
}

int main() {
  bool ret = true;

  // only conduct the test in case we are running on a HSA full profile stack
  hc::accelerator acc;
  if (acc.is_hsa_accelerator() &&
      acc.get_profile() == hc::hcAgentProfileFull) {
    ret &= test();
  }

  return !(ret == true);
}

