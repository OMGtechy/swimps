#include "swimps-io.h"
#include "swimps-math.h"

#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>

size_t swimps_write_to_buffer(const char* __restrict__ sourceBuffer,
                              size_t sourceBufferSize,
                              char* __restrict__ targetBuffer,
                              size_t targetBufferSize) {

    assert(sourceBuffer != NULL);
    assert(targetBuffer != NULL);

    const size_t bytesToWrite = swimps_min(sourceBufferSize, targetBufferSize);
    memcpy(targetBuffer, sourceBuffer, bytesToWrite);
    return bytesToWrite;
}

size_t swimps_write_to_file_descriptor(const char* sourceBuffer,
                                       size_t sourceBufferSize,
                                       int fileDescriptor) {

    assert(sourceBuffer != NULL);

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

    va_list varargs;
    va_start(varargs, targetBufferSize);

    const size_t bytesWritten = swimps_format_string_valist(
        formatBuffer,
        formatBufferSize,
        targetBuffer,
        targetBufferSize,
        varargs
    );

    va_end(varargs);

    return bytesWritten;
}

size_t swimps_format_string_valist(const char* __restrict__ formatBuffer,
                                   size_t formatBufferSize,
                                   char* __restrict__ targetBuffer,
                                   size_t targetBufferSize,
                                   va_list varargs) {

    assert(formatBuffer != NULL);
    assert(targetBuffer != NULL);

    size_t bytesWritten = 0;

    do {
        const size_t bytesToProcess = swimps_min(formatBufferSize, targetBufferSize);

        // The return value is either:
        // 1) NULL, meaning the we're done.
        // 2) A pointer to the character after the %, meaning some formatting needs doing.
        const char* const formatCharacterTarget = memccpy(targetBuffer,
                                                          formatBuffer,
                                                          '%',
                                                          bytesToProcess);

        {
            const size_t newBytesWritten =
                formatCharacterTarget == NULL ? bytesToProcess
                                              : ((size_t)(formatCharacterTarget - targetBuffer));

            formatBuffer += newBytesWritten;
            formatBufferSize -= newBytesWritten;

            targetBuffer += newBytesWritten;
            targetBufferSize -= newBytesWritten;

            bytesWritten += newBytesWritten;
        }

        if (formatCharacterTarget == NULL || formatBufferSize == 0 || targetBufferSize == 0) {
            // end of string!
            break;
        }

        assert(targetBuffer == formatCharacterTarget);

        // Since we've hit a format string, targetBuffer now contains % as the last character.
        // We want to overwrite this with the formatted data, hence the - 1.
        bytesWritten -= 1;
        targetBuffer -= 1;
        targetBufferSize += 1;

        switch(*formatBuffer) {
        case 'd':
            {
                formatBuffer += 1;
                formatBufferSize -=1;
                int value = va_arg(varargs, int);
                const unsigned int numberOfDigitsInValue = floor(log10(abs(value))) + 1;
                unsigned int numberOfDigitsInValueLeft = numberOfDigitsInValue;

                assert(targetBufferSize != 0);

                // Ensure that we do not write to memory we do not own.
                while (targetBufferSize <= numberOfDigitsInValueLeft - 1)
                {
                    numberOfDigitsInValueLeft -= 1;
                    value /= 10;
                }

                targetBuffer += numberOfDigitsInValueLeft - 1;

                while (targetBufferSize > 0 && value > 0 && numberOfDigitsInValueLeft != 0)
                {
                    *targetBuffer = '0' + (value % 10);
                    targetBuffer -= 1;
                    targetBufferSize -= 1;

                    bytesWritten += 1;
                    numberOfDigitsInValueLeft -= 1;
                    value /= 10;
                }

                targetBuffer += numberOfDigitsInValue + 1;
                break;
            }
        case 's':
            {
                formatBuffer += 1;
                formatBufferSize -=1;
                const char* const value = va_arg(varargs, const char*);
                for (size_t index = 0; value[index] != '\0' || targetBufferSize == 0; ++index)
                {
                    *targetBuffer = value[index];
                    targetBuffer += 1;
                    targetBufferSize -= 1;
                    bytesWritten += 1;
                }
                break;
            }
        default:
            {
                *targetBuffer = '?';
                targetBuffer += 1;
                targetBufferSize -= 1;
                formatBuffer += 1;
                formatBufferSize -=1;
                bytesWritten += 1;
                break;
            }
        }

    } while(formatBufferSize > 0 && targetBufferSize > 0);

    return bytesWritten;
}
