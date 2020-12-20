#include "swimps-io.h"

#include <algorithm>
#include <cassert>
#include <cstdlib>

#include <unistd.h>

size_t swimps::io::write_to_buffer(
    swimps::container::Span<const char> source,
    swimps::container::Span<char> target) {

    const size_t bytesToWrite = std::min(source.current_size(), target.current_size());
    memcpy(&target[0], &source[0], bytesToWrite);
    return bytesToWrite;
}
