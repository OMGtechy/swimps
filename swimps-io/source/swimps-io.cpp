#include "swimps-io.h"

#include <algorithm>
#include <cassert>
#include <cstdlib>

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
