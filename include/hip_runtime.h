// XXX(Yan-Ming): borrow from AMS's hip repo
#pragma once

#include <hip.h>
#include <iostream>
#include <vector>

#define hipThreadIdx_x (lp.threadId.x)
#define hipThreadIdx_y (lp.threadId.y)
#define hipThreadIdx_z (lp.threadId.z)

#define hipBlockIdx_x  (lp.groupId.x)
#define hipBlockIdx_y  (lp.groupId.y)
#define hipBlockIdx_z  (lp.groupId.z)

#define hipBlockDim_x  (lp.groupDim.x)
#define hipBlockDim_y  (lp.groupDim.y)
#define hipBlockDim_z  (lp.groupDim.z)

#define hipGridDim_x  (lp.gridDim.x)
#define hipGridDim_y  (lp.gridDim.y)
#define hipGridDim_z  (lp.gridDim.z)

typedef enum hipError_t {
   hipSuccess = 0
} hipError_t;


static std::vector<hc::accelerator> *g_device = nullptr;
static int _the_device = 1;
void __attribute__ ((constructor)) hip_init() {
  g_device = new std::vector<hc::accelerator>(hc::accelerator().get_all());
  // use HSA device by default
  _the_device = 1;
}


inline hipError_t hipStreamCreate(hipStream_t *stream) {
  *stream = new ihipStream_t((*g_device)[_the_device].create_view());
  // XXX(Yan-Ming): Error handling
  return hipSuccess;
}


hipError_t hipStreamSynchronize(hipStream_t stream=nullptr) {
  if (stream == nullptr)
    hc::accelerator().get_default_view().wait();
  else
    stream->av.wait();
  return hipSuccess;
};


hipError_t hipStreamDestroy(hipStream_t stream) {
  delete stream;
  return hipSuccess;
}


hipError_t hipSetDevice(int device) {
  if (0 <= device && device < g_device->size())
    _the_device = device;
  return hipSuccess;
}


hipError_t hipGetDevice(int *device) {
  *device = _the_device;
  return hipSuccess;
}


hipError_t hipGetDeviceCount(int *count) {
  *count = g_device->size();
  return hipSuccess;
}


//-------------------------------
// hipMemcpy*Async
//-------------------------------

hipError_t hipMemcpy2DAsync(void *dst, size_t dpitch,
                            const void *src, size_t spitch,
                            size_t width, size_t height,
                            hipMemcpyKind kind,
                            hipStream_t stream=nullptr) {
  assert(0 && "hipMemcpy2DAsync unimplemented yet.");
}


hipError_t hipMemcpy2DFromArrayAsync(void *dst, size_t dpitch,
                                     const hipArray *src,
                                     size_t wOffset, size_t hOffset,
                                     size_t width, size_t height,
                                     hipMemcpyKind kind,
                                     hipStream_t stream=nullptr) {
  assert(0 && "hipMemcpy2DFromArrayAsync unimplemented yet.");
}


hipError_t hipMemcpy2DToArrayAsync(hipArray *dst,
                                   size_t wOffset, size_t hOffset,
                                   const void *src, size_t spitch,
                                   size_t width, size_t height,
                                   hipMemcpyKind kind,
                                   hipStream_t stream=nullptr) {
  assert(0 && "hipMemcpy2DToArrayAsync unimplemented yet.");
}


hipError_t hipMemcpyAsync(void *dst, const void *src,
                          size_t  count,
                          hipMemcpyKind kind,
                          hipStream_t stream=nullptr) {
  // XXX(Yan-Ming): Does kind matter?
  char *d = (char *)dst;
  char *s = (char *)src;

  // byte by byte copy
  hc::parallel_for_each(stream ? stream->av :
                                 hc::accelerator().get_default_view(),
                        hc::extent<1>(count),
                        [s, d](hc::index<1> idx) __attribute((hc)) {
    d[idx[0]] = s[idx[0]];
  });

  return hipSuccess;
}


hipError_t hipMemcpyFromArrayAsync(void *dst, const hipArray *src,
                                   size_t wOffset, size_t hOffset,
                                   size_t count,
                                   hipMemcpyKind kind,
                                   hipStream_t stream=nullptr) {
  assert(0 && "hipMemcpyFromArrayAsync unimplemented yet.");
}


hipError_t hipMemcpyFromSymbolAsync(void *dst, const char *symbol,
                                    size_t count, size_t offset,
                                    hipMemcpyKind kind,
                                    hipStream_t stream=nullptr) {
  assert(0 && "hipMemcpyFromSymbolAsync unimplemented yet.");
}


hipError_t hipMemcpyPeerAsync(void *dst, int dstDevice,
                              const void *src, int srcDevice,
                              size_t count,
                              hipStream_t stream =nullptr) {
  assert(0 && "hipMemcpyPeerAsync unimplemented yet.");
}


hipError_t hipMemcpyToArrayAsync(hipArray *dst,
                                 size_t wOffset, size_t hOffset,
                                 const void *src,
                                 size_t count,
                                 hipMemcpyKind kind,
                                 hipStream_t stream = 0) {

  assert(0 && "hipMemcpyToArrayAsync unimplemented yet.");
}


hipError_t hipMemcpyToSymbolAsync(const char *symbol,
                                  const void *src,
                                  size_t count, size_t offset,
                                  hipMemcpyKind kind,
                                  hipStream_t stream=nullptr) {

  assert(0 && "hipMemcpyToSymbolAsync unimplemented yet.");
}


//-------------------------------
// hipMemset*Async
//-------------------------------

hipError_t hipMemset2DAsync(void *devPtr, size_t pitch, int value,
                            size_t width, size_t height,
                            hipStream_t stream=nullptr) {

  assert(0 && "hipMemset2DAsync unimplemented yet.");
}


hipError_t hipMemsetAsync(void *dst, int value, size_t count,
                          hipStream_t stream=nullptr) {
  char *d = (char *)dst;

  // byte by byte assignment
  hc::parallel_for_each(stream ? stream->av :
                                 hc::accelerator().get_default_view(),
                        hc::extent<1>(count),
                        [d, value](hc::index<1> idx) __attribute((hc)) {
    d[idx[0]] = value;
  });

  return hipSuccess;
}
