// XFAIL: Linux
// RUN: %hc -lhc_am %s -o %t.out && %t.out


// Tests use of [[ hc_grid_launch ]] CXX11-style attribute.


#include "hc.hpp"
#include "grid_launch.hpp"
#include "hc_am.hpp"
#include <iostream>

#define GRID_SIZE 16
#define TILE_SIZE 16

// C++11 style attribute
[[hc]] int foo(grid_launch_parm lp) {
  int idx = hc_get_workitem_id(0) + hc_get_group_id(0)*lp.group_dim.x;
  return idx;
}

// C-style attribute
__attribute__((hc)) int foo2(grid_launch_parm lp) {
  int idx = hc_get_workitem_id(0) + hc_get_group_id(0)*lp.group_dim.x;
  return idx + 2;
}

// No attribute
int foo3(grid_launch_parm lp, int idx) {
  return idx + 3;
}

[[ hc_grid_launch ]] void kernel(grid_launch_parm lp, int* x) {
  int idx = foo(lp);
  x[idx] = idx + foo2(lp) + foo3(lp, idx);
}


int main() {

  const int sz = GRID_SIZE*TILE_SIZE;

  int* data1 = (int* )malloc(sz*sizeof(int));

  auto acc = hc::accelerator();
  int* data1_d = (int*)hc::am_alloc(sz*sizeof(int), acc, 0);

  grid_launch_parm lp;
  grid_launch_init(&lp);

  lp.grid_dim = gl_dim3(GRID_SIZE, 1);
  lp.group_dim = gl_dim3(TILE_SIZE, 1);

  hc::completion_future cf;
  lp.cf = &cf;
  kernel(lp, data1_d);
  lp.cf->wait();

  static hc::accelerator_view av = acc.get_default_view();
  av.copy(data1_d, data1, sz*sizeof(int));

  bool ret = true;

  for(int i = 0; i < sz; ++i) {
    ret &= (data1[i] == i + i + 2 + i + 3);
  }

  hc::am_free(data1_d);
  free(data1);

  return !ret;

}
