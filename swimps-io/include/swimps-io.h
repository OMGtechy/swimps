#pragma once

#include <cstdarg>
#include <cstddef>

namespace swimps::io {

    //!
    //! \brief  Writes the provided data to the buffer specified, up to the target size.
    //!         This is much the same as memcpy_s on Windows: it's a memcpy that checks the size of the target buffer.
    //!
    //! \param[in]   sourceBuffer      The data to be written to the buffer. Does not need to be null terminated.
    //! \param[in]   sourceBufferSize  The number of bytes to read from the source buffer.
    //! \param[out]  targetBuffer      The buffer to write the data to. Will not be null terminated.
    //! \param[in]   targetBufferSize  The number maximum of bytes to write to the target buffer.
    //!
    //! \returns  The number of bytes written to the target buffer.
    //!
    //! \note  The source will be written to the target completely if there is space.
    //!        If there is not enough space, as many bytes as possible will be written.
    //!
    //! \note  The source and target buffers must *not* overlap in memory.
    //!
    //! \note  The source and target must not be NULL.
    //!
    //! \note  This function is async signal safe.
    //!
    size_t write_to_buffer(const char* __restrict__ sourceBuffer,
                           size_t sourceBufferSize,
                           char* __restrict__ targetBuffer,
                           size_t targetBufferSize);

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
    //! \param[in]   targetBufferSize  The size of the target buffer, in bytes.
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
                         char* __restrict__ targetbuffer,
                         size_t targetbuffersize,
                         ...);

    //!
    //! \brief  A variant of swimps::io::format_string that takes a va_list.
    //!
    //! \see swimps::io::format_string.
    //!
    size_t format_string_valist(const char* __restrict__ formatbuffer,
                                size_t formatbuffersize,
                                char* __restrict__ targetbuffer,
                                size_t targetbuffersize,
                                va_list varargs);
}
