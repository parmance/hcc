// XFAIL: Linux
// RUN: %hc %s -o %t.out && %t.out

#include <hc.hpp>

#include <iostream>
#include <random>
#include <vector>

#define WAVEFRONT_SIZE (hc::get_default_device_wavefront_size())

#define GRID_SIZE (1024)

#define TEST_DEBUG (0)

// A test case to verify HSAIL builtin function
// - __activelaneid_u32

// test __activelaneid_u32
bool test() {
  using namespace hc;
  bool ret = true;

  array<uint32_t, 1> output_GPU(GRID_SIZE);
  extent<1> ex(GRID_SIZE);
  parallel_for_each(ex, [&](index<1>& idx) [[hc]] {
    output_GPU(idx) = __activelaneid_u32();
  }).wait();

  // verify result
  std::vector<uint32_t> output = output_GPU;
  for (int i = 0; i < GRID_SIZE / WAVEFRONT_SIZE; ++i) {
    for (int j = 0; j < WAVEFRONT_SIZE; ++j) {
      // each work-item in each wavefront must have unique active lane id
      ret &= (output[i * WAVEFRONT_SIZE + j] == j);
#if TEST_DEBUG
      std::cout << output[i * WAVEFRONT_SIZE + j] << " ";
#endif
    }
#if TEST_DEBUG
    std::cout << "\n";
#endif
  }

  return ret;
}


int main() {
  bool ret = true;

#if __hcc_backend__ == HCC_BACKEND_AMDGPU
  // XXX activelaneid is not yet implemented on LC backend. let this case fail directly.
  ret = false;
#else
  ret &= test();
#endif

  return !(ret == true);
}

