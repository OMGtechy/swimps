#pragma once

#include <cmath>
#include <cstring>
#include <cstdarg>
#include <cstddef>
#include <concepts>

#include "swimps-container/swimps-container.h"

namespace swimps::io {
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
    std::size_t format_string(
        swimps::container::Span<const char> format,
        swimps::container::Span<char> target,
        const ArgTypes ... args);
}

#include "swimps-io/swimps-io.impl"
