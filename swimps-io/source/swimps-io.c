#include "swimps-io.h"
#include "swimps-math.h"

#include <string.h>
#include <unistd.h>

size_t swimps_write_to_buffer(const char* __restrict__ sourceBuffer,
                              size_t sourceBufferSize,
                              char* __restrict__ targetBuffer,
                              size_t targetBufferSize) {
    const size_t bytesToWrite = swimps_min(sourceBufferSize, targetBufferSize);
    memcpy(targetBuffer, sourceBuffer, bytesToWrite);
    return bytesToWrite;
}

size_t swimps_write_to_file_descriptor(const char* sourceBuffer,
                                       size_t sourceBufferSize,
                                       int fileDescriptor) {
    size_t bytesWrittenTotal = 0;

    while(sourceBufferSize > 0) {
        const ssize_t bytesWritten = write(fileDescriptor, sourceBuffer, sourceBufferSize);

        // i.e. if an error occured
        if (bytesWritten < 0) {
            break;
        }

        bytesWrittenTotal += (size_t) bytesWritten;
        sourceBuffer += bytesWritten;
        sourceBufferSize -= bytesWritten;
    }

    return bytesWrittenTotal;
}
