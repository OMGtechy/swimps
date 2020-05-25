#include "swimps-io.h"
#include "swimps-math.h"

#include <string.h>
#include <unistd.h>
#include <assert.h>

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

size_t swimps_format_string(const char* __restrict__ formatBuffer,
                            size_t formatBufferSize,
                            char* __restrict__ targetBuffer,
                            size_t targetBufferSize,
                            ...) {
    size_t bytesWritten = 0;

    do {
        const size_t bytesToProcess = swimps_min(formatBufferSize, targetBufferSize);

        // the return value is either:
        // 1) NULL, meaning the we're done.
        // 2) A pointer to the character after the %, meaning some formatting needs doing.
        const char* formatCharacter = memccpy(targetBuffer,
                                              formatBuffer,
                                              '%',
                                              bytesToProcess);

        {
            const size_t newBytesWritten =
                formatCharacter == NULL ? bytesToProcess
                                        : ((size_t)(formatCharacter - targetBuffer));

            formatBuffer += newBytesWritten;
            formatBufferSize -= newBytesWritten;

            targetBuffer += newBytesWritten;
            targetBufferSize -= newBytesWritten;

            bytesWritten += newBytesWritten;
        }

        if (formatCharacter == NULL) {
            // end of string!
            break;
        }

        assert(targetBuffer == formatCharacter);

        // TODO: format
    } while(formatBufferSize > 0 && targetBufferSize > 0);

    return bytesWritten;
}
