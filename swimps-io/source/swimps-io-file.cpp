#include "swimps-io-file.h"

#include <cassert>
#include <cerrno>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "swimps-assert.h"

using swimps::container::Span;
using swimps::io::File;

File::File(Span<const char> path,
           const int openFlags,
           const int modeFlags) noexcept {
    const auto bytesWritten = swimps::io::write_to_buffer(path, m_path);
    swimps_assert(bytesWritten == path.current_size());

    m_fileDescriptor = open(&path[0], openFlags, modeFlags);

    swimps_assert(m_fileDescriptor != -1);
}

File::File(const int fileDescriptor) noexcept
: m_fileDescriptor(fileDescriptor) {
    swimps_assert(m_fileDescriptor != -1);
}

File::File(const int fileDescriptor, Span<const char> path) noexcept
: File(fileDescriptor) {
    const auto bytesWritten = swimps::io::write_to_buffer(path, m_path);
    swimps_assert(bytesWritten == path.current_size());
}

std::size_t File::read(Span<char> target) noexcept {
    swimps_assert(m_fileDescriptor != -1);

    std::size_t bytesRead = 0;

    while(target.current_size() > 0) {
        const auto newBytesReadOrError = ::read(
            m_fileDescriptor,
            &target[0],
            static_cast<ssize_t>(target.current_size())
        );

        if (newBytesReadOrError < 0) {
            // Just in case any subsequent calls modify it.
            const auto errorCode = errno;

            // This is the only "acceptable" error;
            // it can happen when a signal fires mid-read.
            swimps_assert(errorCode == EINTR);

            continue;
        }

        if (newBytesReadOrError == 0) {
            // End of file.
            return bytesRead;
        }

        const auto newBytesRead = static_cast<std::size_t>(newBytesReadOrError);
        target += newBytesRead;
        bytesRead += newBytesRead;
    }

    return bytesRead;
}

std::size_t File::write(Span<const char> dataSource) noexcept {
    std::size_t bytesWritten = 0;

    while(dataSource.current_size() > 0) {
        const auto newBytesWrittenOrError = ::write(
            m_fileDescriptor,
            &dataSource[0],
            dataSource.current_size()
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
        dataSource += newBytesWritten;
        bytesWritten += static_cast<size_t>(newBytesWritten);
    }

    return bytesWritten;
}

bool File::seekToStart() noexcept {
    swimps_assert(m_fileDescriptor != -1);
    return ::lseek(m_fileDescriptor, 0, SEEK_SET) == 0;
}

bool File::close() noexcept {
    swimps_assert(m_fileDescriptor != -1);
    while(true) {
        const auto closeResult = ::close(m_fileDescriptor);
        if (closeResult == 0) {
            m_fileDescriptor = -1;
            return true;
        }

        // Interrupted by signal.
        if (errno == EINTR) {
            continue;
        }

        return false;
    }
}

bool File::remove() noexcept {
    swimps_assert(m_fileDescriptor != -1);
    swimps_assert(m_path[0] != '\0');

    // Ignoring return code: it might have already been closed.
    close();

    return ::unlink(m_path) == 0;
}
