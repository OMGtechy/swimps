#include "swimps-io-file.h"

#include <cerrno>

using swimps::container::Span;
using swimps::io::File;
using swimps::io::format_string;
using swimps::io::write_to_buffer;

File File::create(const Span<const char> path,
                  const Permissions permissions) noexcept {
    File file;
    file.create_internal(path, permissions);
    return file;
}

File File::create_temporary(const Span<const char> pathPrefix,
                            const Permissions permissions) noexcept {
    File file;
    file.create_temporary_internal(pathPrefix, permissions);
    return file;
}

File File::open(const Span<const char> path,
                const Permissions permissions) noexcept {
    File file;
    file.open_internal(path, permissions);
    return file;
}

File::File(const int fileDescriptor) noexcept
: File(fileDescriptor, Span<const char>{m_path, 0}) { }

File::File(const int fileDescriptor, Span<const char> path) noexcept
: m_fileDescriptor(fileDescriptor) {
    swimps_assert(m_fileDescriptor != -1);
    const auto bytesWritten = write_to_buffer(path, m_path);
    m_pathLength = bytesWritten;
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

Span<const char> File::getPath() const noexcept {
    return { m_path, m_pathLength };
}

void File::create_internal(const Span<const char> path, const Permissions permissions) noexcept {
    path_based_internal(path, static_cast<decltype(O_RDWR)>(permissions) | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
}

void File::create_temporary_internal(const Span<const char> pathPrefix, const Permissions permissions) noexcept {
    m_pathLength = format_string("/tmp/%_XXXXXX", m_path, &pathPrefix[0]);
    m_fileDescriptor = mkostemp(&m_path[0], static_cast<decltype(O_RDWR)>(permissions));
    swimps_assert(m_fileDescriptor != -1);
}

void File::open_internal(const Span<const char> path, const Permissions permissions) noexcept {
    path_based_internal(path, static_cast<decltype(O_RDWR)>(permissions), 0);
}

void File::path_based_internal(const Span<const char> path, const int flags, const int modeFlags) noexcept {
    const auto bytesWritten = write_to_buffer(path, m_path);
    m_pathLength = bytesWritten;
    swimps_assert(bytesWritten == path.current_size());

    m_fileDescriptor = ::open(&path[0], flags, modeFlags);

    swimps_assert(m_fileDescriptor != -1);
}
