// XFAIL: Linux
// RUN: %hc %s -o %t.out && %t.out

#include <hc.hpp>

#include <iostream>
#include <vector>
#include <random>

#define WAVEFRONT_SIZE (hc::get_default_device_wavefront_size())

#define TEST_DEBUG (0)

// A test case to verify HSAIL builtin function
// - __shfl_xor

bool test_reduce(int grid_size) {
  bool ret = true;

  using namespace hc;
  extent<1> ex(grid_size);
  array<int, 1> table(grid_size);
  size_t wavefront_size = WAVEFRONT_SIZE;

  parallel_for_each(ex, [&, wavefront_size](index<1>& idx) [[hc]] {
    int laneId = __lane_id();
    int value = (wavefront_size - 1) - laneId;

    // use xor mode to perform butterfly reduction
    for (int i = (wavefront_size / 2); i >= 1; i /= 2)
      value += __shfl_xor(value, i);

    table(idx) = value;
  }).wait();

  std::vector<int> output = table;
  for (int i = 0; i < grid_size; ++i) {
    int activeWavefrontSize = (((i / wavefront_size + 1) * wavefront_size) <= grid_size) ? wavefront_size : (grid_size % wavefront_size);
    int expected = ((wavefront_size - 1) + (wavefront_size - 1 - activeWavefrontSize + 1)) * activeWavefrontSize / 2;
    ret &= (output[i] == expected);
#if TEST_DEBUG
    std::cout << expected << " ";
    std::cout << output[i] << " ";
#endif
  }
#if TEST_DEBUG
    std::cout << "\n";
#endif

  return ret;
}

int main() {
  bool ret = true;

  // test reduce algorithm using __shfl_xor
  // NOTICE we don't test __shfl_xor with width parameter
  ret &= test_reduce(2);
  ret &= test_reduce(4);
  ret &= test_reduce(8);
  ret &= test_reduce(16);
  ret &= test_reduce(32);
  ret &= test_reduce(64);

  return !(ret == true);
}

