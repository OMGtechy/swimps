#include "swimps-io.h"
#include "swimps-math.h"

#include <string.h>

size_t swimps_write_to_buffer(const char* __restrict__ sourceBuffer,
                              size_t sourceBufferSize,
                              char* __restrict__ targetBuffer,
                              size_t targetBufferSize) {
    const size_t bytesToWrite = swimps_min(sourceBufferSize, targetBufferSize);
    memcpy(targetBuffer, sourceBuffer, bytesToWrite);
    return bytesToWrite;
}
