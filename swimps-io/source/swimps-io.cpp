#include "swimps-io.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <cstring>

#include <unistd.h>

bool swimps::io::read_from_file_descriptor(
    const int sourceFileDescriptor,
    swimps::container::Span<char> target) {

    const auto bytesToRead = static_cast<ssize_t>(target.current_size());
    return bytesToRead == read(
        sourceFileDescriptor,
        &target[0],
        bytesToRead
    );
}

size_t swimps::io::write_to_buffer(
    swimps::container::Span<const char> source,
    swimps::container::Span<char> target) {

    const size_t bytesToWrite = std::min(source.current_size(), target.current_size());
    memcpy(&target[0], &source[0], bytesToWrite);
    return bytesToWrite;
}

size_t swimps::io::write_to_file_descriptor(
    swimps::container::Span<const char> source,
    int fileDescriptor) {

    size_t bytesWrittenTotal = 0;

    while(source.current_size() > 0) {
        const ssize_t bytesWritten = write(fileDescriptor, &source[0], source.current_size());

        // i.e. if an error occured
        if (bytesWritten < 0) {
            break;
        }

        bytesWrittenTotal += (size_t) bytesWritten;
        source += bytesWritten;
    }

    return bytesWrittenTotal;
}

size_t swimps::io::format_string(
    swimps::container::Span<const char> format,
    swimps::container::Span<char> target,
    ...) {

    va_list varargs;
    va_start(varargs, target);

    const size_t bytesWritten = swimps::io::format_string_valist(
        format,
        target,
        varargs
    );

    va_end(varargs);

    return bytesWritten;
}

size_t swimps::io::format_string_valist(
    swimps::container::Span<const char> format,
    swimps::container::Span<char> target,
    va_list varargs) {

    size_t bytesWritten = 0;

    do {
        const size_t bytesToProcess = std::min(format.current_size(), target.current_size());

        // The return value is either:
        // 1) NULL, meaning the we're done.
        // 2) A pointer to the character after the % in the target buffer,
        //    meaning some formatting needs doing.
        const char* const formatCharacterTarget = static_cast<char*>(
            memccpy(
                &target[0],
                &format[0],
                '%',
                bytesToProcess
            )
        );

        {
            assert(formatCharacterTarget == NULL || *(formatCharacterTarget - 1) == '%');
            const bool foundAPercentSign =
                formatCharacterTarget != NULL;

            const size_t newBytesWritten =
                !foundAPercentSign ? bytesToProcess
                                   : static_cast<size_t>(formatCharacterTarget - &target[0]);

            format += newBytesWritten;

            // Overwrite % sign if present.
            target += newBytesWritten - 1;

            bytesWritten += newBytesWritten;
        }

        if (formatCharacterTarget == NULL || format.current_size() == 0 || target.current_size() == 0) {
            // end of string!
            break;
        }

        // Overwriting % sign, so one less byte written.
        bytesWritten -= 1;

        switch(format[0]) {
        case 'd':
            {
                format += 1;

                int value = va_arg(varargs, int);

                if (value == 0) {
                    target[0] = '0';
                    target += 1;
                    bytesWritten += 1;
                    break;
                }

                // If the value is a negative, write '-' into the target buffer.
                if (value < 0) {
                    assert(target.current_size() != 0);
                    target[0] = '-';
                    bytesWritten += 1;

                    target += 1;

                    value = value * -1;

                    if (target.current_size() == 0) {
                        // No room for anything other then the '-' sign.
                        break;
                    }
                }

                const unsigned int numberOfDigitsInValue = floor(log10(abs(value))) + 1;
                unsigned int numberOfDigitsInValueLeft = numberOfDigitsInValue;

                assert(target.current_size() != 0);

                // Ensure that we do not write to memory we do not own.
                while (target.current_size() <= numberOfDigitsInValueLeft - 1)
                {
                    numberOfDigitsInValueLeft -= 1;
                    value /= 10;
                }

                unsigned int numberOfDigitsWritten = 0;
                while (target.current_size() > 0 && value > 0 && numberOfDigitsInValueLeft != 0)
                {
                    target[numberOfDigitsInValueLeft - 1] = '0' + (value % 10);
                    bytesWritten += 1;
                    numberOfDigitsInValueLeft -= 1;
                    numberOfDigitsWritten += 1;
                    value /= 10;
                }

                target += numberOfDigitsWritten;
                break;
            }
        case 's':
            {
                format += 1;

                const char* const value = va_arg(varargs, const char*);
                for (size_t index = 0; value[index] != '\0' || target.current_size() == 0; ++index)
                {
                    target[0] = value[index];
                    target += 1;
                    bytesWritten += 1;
                }
                break;
            }
        default:
            {
                target[0] = '?';
                target += 1;
                format += 1;
                bytesWritten += 1;
                break;
            }
        }

    } while(format.current_size() > 0 && target.current_size() > 0);

    return bytesWritten;
}
