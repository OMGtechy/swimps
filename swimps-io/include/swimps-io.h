#pragma once

#include <cmath>
#include <cstring>
#include <cstdarg>
#include <cstddef>
#include <concepts>

#include "swimps-container.h"

namespace swimps::io {

    //!
    //! \brief  Reads data from a file to fill the target.
    //!
    //! \param[in]   sourceFileDescriptor  Where to read the data from.
    //! \param[out]  target                Where to write the data to.
    //!
    //! \return  Whether the read was successful or not.
    //!
    bool read_from_file_descriptor(
        const int sourceFileDescriptor,
        swimps::container::Span<char> target);

    //!
    //! \brief  Writes the provided data to the spans specified.
    //!         This is much the same as memcpy_s on Windows: it's a memcpy that checks the size of the target.
    //!
    //! \param[in]   source  Where to read the data from. Does not need to be null terminated.
    //! \param[out]  target  Where to write the data. Will not be null terminated.
    //!
    //! \returns  The number of bytes written to the target.
    //!
    //! \note  The source will be written to the target completely if there is space.
    //!        If there is not enough space, as many bytes as possible will be written.
    //!
    //! \note  The source and target buffers must *not* overlap in memory.
    //!
    //! \note  This function is async signal safe.
    //!
    size_t write_to_buffer(
        swimps::container::Span<const char> source,
        swimps::container::Span<char> target);

    //!
    //! \brief  Writes the provided data to the file specified.
    //!
    //! \param[in]  source          The data to be written to the file. Does not need to be null terminated.
    //! \param[in]  fileDescriptor  The file to write the data to.
    //!
    //! \returns  The number of bytes written to the file.
    //!
    //! \note  This function is async signal safe.
    //!
    size_t write_to_file_descriptor(
        swimps::container::Span<const char> source,
        int fileDescriptor);

    //!
    //! \brief  Formats the string, as per printf rules (see supported list), into the target.
    //!
    //! \param[in]   format  The format string. Does not need to be null terminated.
    //! \param[out]  target  Where to write the formatted string.
    //! \param[in]   ...     The values to use when formatting the string.
    //!
    //! \returns  The number of bytes written to the target buffer.
    //!
    //! \note  If there isn't enough room in the target buffer to write the formatted string,
    //!        it will be truncated.
    //!
    //! \note  The resulting string is not NULL terminated.
    //!
    //! \note  This function is async signal safe.
    //!
    //! \note  Only single-byte ASCII characters are supported.
    //!
    //! \note  Supported format specifiers are:
    //!        - %d: int
    //!        - %s: a null terminated string as a const char*
    //!
    template <typename... ArgTypes>
    size_t format_string(
        swimps::container::Span<const char> format,
        swimps::container::Span<char> target) {
        const size_t bytesToWrite = std::min(target.current_size(), format.current_size());
        strncpy(&target[0], &format[0], format.current_size());
        return bytesToWrite;
    }

    template <typename T, typename... ArgTypes>
    size_t format_string(
        swimps::container::Span<const char> format,
        swimps::container::Span<char> target,
        const T firstArg,
        const ArgTypes ... otherArgs) {

        size_t bytesWritten = 0;
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
            return bytesWritten;
        }

        // Overwriting % sign, so one less byte written.
        bytesWritten -= 1;

        const auto formatCharacter = format[0];
        format += 1;

        do {
            if constexpr (std::is_same_v<int, T>) {
                assert(formatCharacter == 'd');
                int value = firstArg;

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
            } else if constexpr (std::is_same_v<T, const char*> || std::is_same_v<T, char*>) {
                const char* const value = firstArg;
                for (size_t index = 0; value[index] != '\0' || target.current_size() == 0; ++index)
                {
                    target[0] = value[index];
                    target += 1;
                    bytesWritten += 1;
                }
            } else {
                target[0] = '?';
                target += 1;
                bytesWritten += 1;
            }
        } while (false);

        return bytesWritten + format_string(format, target, otherArgs...);
    }
}
