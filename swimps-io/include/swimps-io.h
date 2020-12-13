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
    //! \brief  Formats the string with the args provided, using % to denote where each should be inserted.
    //!
    //! \param[in]   format  The format string. Does not need to be null terminated.
    //! \param[out]  target  Where to write the formatted string.
    //! \param[in]   args    The values to use when formatting the string.
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
    //! \note  Types supported for formatting:
    //!        - int
    //!        - a null terminated string as a char* or const char*
    //!
    template <typename T, typename... ArgTypes>
    size_t format_string(
        swimps::container::Span<const char> format,
        swimps::container::Span<char> target,
        const ArgTypes ... args);
}

#include "swimps-io.impl"
