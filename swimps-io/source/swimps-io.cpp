#include "swimps-io.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>

#include <string.h>
#include <unistd.h>

size_t swimps::io::write_to_buffer(const char* __restrict__ sourceBuffer,
                              size_t sourceBufferSize,
                              char* __restrict__ targetBuffer,
                              size_t targetBufferSize) {

    assert(sourceBuffer != NULL);
    assert(targetBuffer != NULL);

    const size_t bytesToWrite = std::min(sourceBufferSize, targetBufferSize);
    memcpy(targetBuffer, sourceBuffer, bytesToWrite);
    return bytesToWrite;
}

size_t swimps::io::write_to_file_descriptor(const char* sourceBuffer,
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

size_t swimps::io::format_string(const char* __restrict__ formatBuffer,
                            size_t formatBufferSize,
                            swimps::container::Span<char> targetBuffer,
                            ...) {
    va_list varargs;
    va_start(varargs, targetBuffer);

    const size_t bytesWritten = swimps::io::format_string_valist(
        formatBuffer,
        formatBufferSize,
        targetBuffer,
        varargs
    );

    va_end(varargs);

    return bytesWritten;
}

size_t swimps::io::format_string_valist(const char* __restrict__ formatBuffer,
                                   size_t formatBufferSize,
                                   swimps::container::Span<char> targetBuffer,
                                   va_list varargs) {

    assert(formatBuffer != NULL);

    size_t bytesWritten = 0;

    do {
        const size_t bytesToProcess = std::min(formatBufferSize, targetBuffer.current_size());

        // The return value is either:
        // 1) NULL, meaning the we're done.
        // 2) A pointer to the character after the % in the target buffer,
        //    meaning some formatting needs doing.
        const char* const formatCharacterTarget = static_cast<char*>(memccpy(&targetBuffer[0],
                                                                     formatBuffer,
                                                                     '%',
                                                                     bytesToProcess));

        {
            assert(formatCharacterTarget == NULL || *(formatCharacterTarget - 1) == '%');
            const bool foundAPercentSign =
                formatCharacterTarget != NULL;

            const size_t newBytesWritten =
                !foundAPercentSign ? bytesToProcess
                                   : static_cast<size_t>(formatCharacterTarget - &targetBuffer[0]);

            formatBuffer += newBytesWritten;
            formatBufferSize -= newBytesWritten;

            // Overwrite % sign if present.
            targetBuffer += newBytesWritten - 1;

            bytesWritten += newBytesWritten;
        }

        if (formatCharacterTarget == NULL || formatBufferSize == 0 || targetBuffer.current_size() == 0) {
            // end of string!
            break;
        }

        // Overwriting % sign, so one less byte written.
        bytesWritten -= 1;

        switch(*formatBuffer) {
        case 'd':
            {
                formatBuffer += 1;
                formatBufferSize -=1;
                int value = va_arg(varargs, int);

                if (value == 0) {
                    targetBuffer[0] = '0';
                    targetBuffer += 1;
                    bytesWritten += 1;
                    break;
                }

                // If the value is a negative, write '-' into the target buffer.
                if (value < 0) {
                    assert(targetBuffer.current_size() != 0);
                    targetBuffer[0] = '-';
                    bytesWritten += 1;

                    targetBuffer += 1;

                    value = value * -1;

                    if (targetBuffer.current_size() == 0) {
                        // No room for anything other then the '-' sign.
                        break;
                    }
                }

                const unsigned int numberOfDigitsInValue = floor(log10(abs(value))) + 1;
                unsigned int numberOfDigitsInValueLeft = numberOfDigitsInValue;

                assert(targetBuffer.current_size() != 0);

                // Ensure that we do not write to memory we do not own.
                while (targetBuffer.current_size() <= numberOfDigitsInValueLeft - 1)
                {
                    numberOfDigitsInValueLeft -= 1;
                    value /= 10;
                }

                unsigned int numberOfDigitsWritten = 0;
                while (targetBuffer.current_size() > 0 && value > 0 && numberOfDigitsInValueLeft != 0)
                {
                    targetBuffer[numberOfDigitsInValueLeft - 1] = '0' + (value % 10);
                    bytesWritten += 1;
                    numberOfDigitsInValueLeft -= 1;
                    numberOfDigitsWritten += 1;
                    value /= 10;
                }

                targetBuffer += numberOfDigitsWritten;
                break;
            }
        case 's':
            {
                formatBuffer += 1;
                formatBufferSize -=1;
                const char* const value = va_arg(varargs, const char*);
                for (size_t index = 0; value[index] != '\0' || targetBuffer.current_size() == 0; ++index)
                {
                    targetBuffer[0] = value[index];
                    targetBuffer += 1;
                    bytesWritten += 1;
                }
                break;
            }
        default:
            {
                targetBuffer[0] = '?';
                targetBuffer += 1;
                formatBuffer += 1;
                formatBufferSize -=1;
                bytesWritten += 1;
                break;
            }
        }

    } while(formatBufferSize > 0 && targetBuffer.current_size() > 0);

    return bytesWritten;
}
