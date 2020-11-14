#pragma once

#include <cstdarg>
#include <cstddef>

#include "swimps-container.h"

namespace swimps::io {

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
    //! \param[in]  sourceBuffer      The data to be written to the file. Does not need to be null terminated.
    //! \param[in]  sourceBufferSize  The number of bytes to read from the source buffer.
    //! \param[in]  fileDescriptor    The file to write the data to.
    //!
    //! \returns  The number of bytes written to the file.
    //!
    //! \note  This function is async signal safe.
    //!
    size_t write_to_file_descriptor(const char* sourceBuffer,
                                    size_t sourceBufferSize,
                                    int fileDescriptor);

    //!
    //! \brief  Formats the string, as per printf rules (see supported list), into the target buffer.
    //!
    //! \param[in]   formatBuffer      The buffer containing the format string. Does not need to be null terminated.
    //! \param[in]   formatBufferSize  The size of the format buffer, in bytes.
    //! \param[out]  targetBuffer      Where to write the formatted string.
    //! \param[in]   ...               The values to use when formatting the string.
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
    size_t format_string(const char* __restrict__ formatbuffer,
                         size_t formatbuffersize,
                         swimps::container::Span<char> targetbuffer,
                         ...);

    //!
    //! \brief  A variant of swimps::io::format_string that takes a va_list.
    //!
    //! \see swimps::io::format_string.
    //!
    size_t format_string_valist(const char* __restrict__ formatbuffer,
                                size_t formatbuffersize,
                                swimps::container::Span<char> targetbuffer,
                                va_list varargs);
}
