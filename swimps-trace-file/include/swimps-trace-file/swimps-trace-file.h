#pragma once

#include <cstddef>
#include <optional>
#include <variant>

#include <unistd.h>

#include <signalsafe/file.hpp>

#include "swimps-error/swimps-error.h"
#include "swimps-trace/swimps-trace.h"

namespace swimps::trace {
    //!
    //! \brief  Represents a swimps trace file.
    //!
    class TraceFile : public signalsafe::File {
    public:
        //!
        //! \brief  Creates an empty trace file instance.
        //!
        TraceFile() = default;

        //!
        //! \brief  Creates a trace file at the given path.
        //!
        //! \param[in]  path         Where to create the file.
        //! \param[in]  permissions  The permissions to create the file with.
        //!
        //! \note  This function is async signal safe.
        //!
        static TraceFile create_and_open(std::string_view path, Permissions permissions) noexcept;

        //!
        //! \brief  Creates a temporary trace file.
        //!
        //! \returns  The temporary trace file.
        //!
        //! \note  This function is async signal safe.
        //!
        static TraceFile create_temporary() noexcept;

        //!
        //!  \brief  Opens a trace file.
        //!
        //!  \param[in]  path         Where the trace file to be opened is.
        //!  \param[in]  permissions  The permissions to open the trace file with.
        //!
        //!  \returns  The requested trace file.
        //!
        //!  \note  This function is async signal safe.
        //!
        static TraceFile open_existing(std::string_view path, Permissions permissions) noexcept;

        static TraceFile from_raw(std::string_view pathView) noexcept;

        //!
        //! \brief  Adds a sample to the trace file.
        //!
        //! \param[in]  sample  The sample to add.
        //!
        //! \returns  The number of bytes written to the file.
        //!
        //! \note  This function is async signal safe.
        //!
        std::size_t add_sample(const swimps::trace::Sample& sample);

        //!
        //! \brief  Adds a backtrace to the trace file.
        //!
        //! \param[in]  backtrace  The backtrace to add.
        //!
        //! \returns  The number of bytes written to the file.
        //!
        //! \note  This function is *not* async signal safe.
        //!
        std::size_t add_backtrace(const Backtrace& backtrace);

        //!
        //! \brief  Adds a stack frame to the trace file.
        //!
        //! \param[in]  stackFrame  The stack frame to add.
        //!
        //! \returns  The number of bytes written to the file.
        //!
        //! \note  This function is async signal safe.
        //!
        std::size_t add_stack_frame(const StackFrame& stackFrame);

        using Entry = std::variant<Backtrace, Sample, StackFrame, swimps::error::ErrorCode>;

        //!
        //! \brief  Reads the next entry in the trace file.
        //!
        //! \returns  The next entry, or an error code if something went wrong (such as EndOfFile).
        //!
        //! \note  This function is *not* async signal safe.
        //!
        Entry read_next_entry() noexcept;

        //!
        //! \brief  Reads the entire trace from the trace file.
        //!
        //! \returns  The trace contained in the file, if successful.
        //!
        //! \note  This function is *not* async signal safe.
        //!
        std::optional<Trace> read_trace() noexcept;

        //!
        //! \brief  Finalises the trace file.
        //!
        //! \returns  Whether finalising was successful or not.
        //!
        //! \note  You should not modify the file after finalising it.
        //!
        //! \note  This function is *not* async signal safe.
        //!
        bool finalise() noexcept;

        TraceFile(TraceFile&&) = default;
        TraceFile& operator=(TraceFile&&) = default;

        TraceFile(const TraceFile&) = delete;
        TraceFile& operator=(const TraceFile&) = delete;
    };
}
