#pragma once

#include <cstddef>
#include <optional>

#include <linux/limits.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "swimps-assert/swimps-assert.h"
#include "swimps-container/swimps-container.h"
#include "swimps-io/swimps-io.h"

namespace swimps::io {
    //!
    //!  \brief  Provides an async signal safe type for file operations.
    //!
    class File {
    public:
        template <typename T>
        using Span = swimps::container::Span<T>;

        //!
        //!  \brief  Creates a File instance that refers to nothing.
        //!
        //!  \note  This function is async signal safe.
        //!
        File() = default;

        //!
        //!  \brief  Closes the file.
        //!
        virtual ~File();

        enum class Permissions : decltype(O_RDWR) {
            ReadOnly = O_RDONLY,
            WriteOnly = O_WRONLY,
            ReadWrite = O_RDWR
        };

        //!
        //!  \brief  Creates a File.
        //!
        //!  \param[in]  path         Where the file should be created.
        //!  \param[in]  permissions  The permissions to create the file with.
        //!
        //!  \returns  The requested file.
        //!
        //!  \note  This function is async signal safe.
        //!
        static File create(Span<const char> path, Permissions permissions) noexcept;

        //!
        //! \brief  Creates a temporary file.
        //!
        //! \param[in]  pathPrefix   The prefix to add to the temporary file name.
        //! \param[in]  permissions  The permissions to create the file with.
        //!
        //! \returns  The temporary file.
        //!
        //! \note  This function is async signal safe.
        //!
        static File create_temporary(swimps::container::Span<const char> pathPrefix, Permissions permissions) noexcept;

        //!
        //!  \brief  Opens a file.
        //!
        //!  \param[in]  path         Where the file to be opened is.
        //!  \param[in]  permissions  The permissions to open the file with.
        //!
        //!  \returns  The requested file.
        //!
        //!  \note  This function is async signal safe.
        //!
        static File open(Span<const char> path, Permissions permissions) noexcept;

        //!
        //!  \brief  Move constructs a File.
        //!
        //!  \param[in,out]  other  The File to move from.
        //!
        //!  \note  The moved-from File is left in an unspecified state.
        //!
        //!  \note  This function is async signal safe.
        //!
        File(File&& other);

        //!
        //!  \brief  Move assigns a File.
        //!
        //!  \param[in,out]  other  The File to move from.
        //!
        //!  \returns  A reference to the File assigned to.
        //!
        //!  \note  The moved-from File is left in an unspecified state.
        //!
        //!  \note  This function is async signal safe.
        //!
        File& operator=(File&& other);

        //!
        //!  \brief  Tries to fill the target with data from the file.
        //!
        //!  \param[out]  target  Where to write the data to.
        //!
        //!  \return  How many bytes were read.
        //!
        //!  \note  This function is async signal safe.
        //!
        std::size_t read(Span<char> target) noexcept;

        //!
        //!  \brief  Writes the provided data to the file.
        //!
        //!  \param[in]  dataSource  The data to be written to the file.
        //!
        //!  \returns  The number of bytes written to the file.
        //!
        //!  \note  This function is async signal safe.
        //!
        std::size_t write(Span<const char> dataSource) noexcept;

        //!
        //!  \brief  Seeks to the start of the file.
        //!
        //!  \returns  true if successful, false otherwise.
        //!
        //!  \note  This function is async signal safe.
        //!
        bool seekToStart() noexcept;

        //!
        //!  \brief  Closes a file.
        //!
        //!  \returns  Whether the file was closed successfully.
        //!
        //!  \note  This function is async signal safe.
        //!
        bool close() noexcept;

        //!
        //!  \brief  Removes (a.k.a. deletes) a file.
        //!
        //!  \returns  Whether the file was removed successfully.
        //!
        //!  \note  This function is async signal safe.
        //!
        bool remove() noexcept;

        //!
        //!  \brief  Gets the path to the file, if there is one.
        //!
        //!  \returns  A span covering the path data.
        //!            If there is no path, the span will be empty.
        //!
        //!  \note  This function is async signal safe.
        //!
        Span<const char> getPath() const noexcept;

        //!
        //!  \brief  Gets the file descriptor of the file.
        //!
        //!  \returns  The file descriptor for the file.
        //!
        int getFileDescriptor() const noexcept;

        //!
        //!  \brief  Non-copyable.
        //!          One person might expect the same file to be referred to.
        //!          Another might expect a copy of file under another name.
        //!
        File(const File&) = delete;
        File& operator=(const File&) = delete;

        //!
        //!  \brief  It's not clear what equality means for files.
        //!          Same file name, same descriptor, same contents?
        //!
        bool operator==(const File&) = delete;

    protected:
        void create_internal(Span<const char> path, Permissions permissions) noexcept;
        void create_temporary_internal(Span<const char> pathPrefix, Permissions permissions) noexcept;
        void open_internal(Span<const char> path, Permissions permissions) noexcept;
        void path_based_internal(Span<const char> path, int flags, int modeFlags) noexcept;

        int m_fileDescriptor = -1;
        std::array<char, PATH_MAX + 1 /* null terminator */> m_path = {};
        std::size_t m_pathLength = 0;
    };
}
