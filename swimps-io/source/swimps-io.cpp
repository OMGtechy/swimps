#include "swimps-io.h"

#include <algorithm>
#include <cassert>
#include <cstdlib>

#include <unistd.h>

#include "swimps-assert.h"

using swimps::container::Span;

std::size_t swimps::io::write_to_buffer(
    Span<const char> source,
    Span<char> target) {

    const std::size_t bytesToWrite = std::min(source.current_size(), target.current_size());
    memcpy(&target[0], &source[0], bytesToWrite);
    return bytesToWrite;
}

std::size_t swimps::io::write_to_file_descriptor(
    const int targetFileDescriptor,
    Span<const char> dataToWrite) {

    std::size_t bytesWritten = 0;

    while(dataToWrite.current_size() > 0) {
        const auto newBytesWrittenOrError = ::write(
            targetFileDescriptor,
            &dataToWrite[0],
            dataToWrite.current_size()
        );

        if (newBytesWrittenOrError < 0) {
            // Just in case any subsequent calls modify it.
            const auto errorCode = errno;

            // This is the only "acceptable" error;
            // it can happen when a signal fires mid-write.
            swimps_assert(errorCode == EINTR);

            continue;
        }

        const auto newBytesWritten = static_cast<std::size_t>(newBytesWrittenOrError);
        dataToWrite += newBytesWritten;
        bytesWritten += static_cast<size_t>(newBytesWritten);
    }

    return bytesWritten;
}
