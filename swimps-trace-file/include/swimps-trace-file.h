#pragma once

#include <cstddef>
#include <optional>
#include <variant>

#include <unistd.h>

#include "swimps-container.h"
#include "swimps-error.h"
#include "swimps-io-file.h"
#include "swimps-time.h"
#include "swimps-trace.h"

namespace swimps::trace::file {
    constexpr size_t swimps_v1_trace_entry_marker_size = 6;
    constexpr char swimps_v1_trace_file_marker[swimps_v1_trace_entry_marker_size] = "s_v1\n";
    constexpr char swimps_v1_trace_symbolic_backtrace_marker[swimps_v1_trace_entry_marker_size] = "\nsb!\n";
    constexpr char swimps_v1_trace_sample_marker[swimps_v1_trace_entry_marker_size] = "\nsp!\n";
    constexpr char swimps_v1_trace_stack_frame_marker[swimps_v1_trace_entry_marker_size] = "\nsf!\n";

    //!
    //! \brief  Represents a swimps trace file.
    //!
    class TraceFile : public swimps::io::File {
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
        static TraceFile create(swimps::container::Span<const char> path, Permissions permissions) noexcept;

        //!
        //! \brief  Creates a temporary trace file.
        //!
        //! \param[in]  pathPrefix   The prefix to add to the temporary file name.
        //! \param[in]  permissions  The permissions to create the file with.
        //!
        //! \returns  The temporary trace file.
        //!
        //! \note  This function is async signal safe.
        //!
        static TraceFile create_temporary(swimps::container::Span<const char> pathPrefix, Permissions permissions) noexcept;

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
        static TraceFile open(swimps::container::Span<const char> path, Permissions permissions) noexcept;

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
        //! \note  This function is async signal safe.
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

        TraceFile(TraceFile&&) = default;
        TraceFile& operator=(TraceFile&&) = default;

        TraceFile(const TraceFile&) = delete;
        TraceFile& operator=(const TraceFile&) = delete;
    };

    //!
    //! \brief  Generates a name for the trace file based upon a program name.
    //!
    //! \param[in]   programName       The name of the program the trace file is for.
    //! \param[in]   time              The timestamp to use when generating the name.
    //! \param[out]  target            Where to write the generated name.
    //!
    //! \returns  The number of bytes written to the target buffer.
    //!
    //! \note  This function is async signal safe.
    //!
    size_t generate_name(
         const char* const programName,
         const swimps::time::TimeSpecification& time,
         swimps::container::Span<char> target);

    //!
    //! \brief  Finalises an opened trace file.
    //!
    //! \param[in]  traceFile          The trace file to finalise.
    //!
    //! \returns  0 if successful, -1 otherwise (errno may be set).
    //!
    //! \note  You should not modify the file are finalising it.
    //!
    //! \note  This function is *not* async signal safe.
    //!
    int finalise(TraceFile& traceFile);
}
