#include "swimps-io/swimps-io.h"

#include <cstdlib>

#include "swimps-assert/swimps-assert.h"

using swimps::container::Span;

std::size_t swimps::io::write_to_buffer(
    Span<const char> source,
    Span<char> target) {

    const std::size_t bytesToWrite = std::min(source.current_size(), target.current_size());
    memcpy(&target[0], &source[0], bytesToWrite);
    return bytesToWrite;
}

