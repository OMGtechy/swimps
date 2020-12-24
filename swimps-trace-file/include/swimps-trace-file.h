#pragma once

#include <cstddef>
#include <optional>

#include <unistd.h>

#include "swimps-container.h"
#include "swimps-io-file.h"
#include "swimps-time.h"
#include "swimps-trace.h"


namespace swimps::trace::file {
    constexpr char swimps_v1_trace_file_marker[] = "swimps_v1_trace_file";
    constexpr size_t swimps_v1_trace_entry_marker_size  = sizeof "\n__!\n";
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
        //! \brief  Creates a trace file.
        //!
        //! \param[in]  path  Where to create the file.
        //!
        //! \note  This function is async signal safe.
        //!
        //! \note  People might want to open existing one sometimes,
        //!        should the ctor by hidden away and static member functions used
        //!        to make it clear which is happening?
        //!
        TraceFile(swimps::container::Span<const char> path) noexcept;

        //!
        //! \brief  Creates a trace file using an already open file descriptor.
        //!
        //! \param[in]  fileDescriptor  The open file descriptor to read from and write to.
        //! \param[in]  path            The path that matches the file descriptor.
        //!
        //! \note  This function is async signal safe.
        //!
        TraceFile(int fileDescriptor, swimps::container::Span<const char> path) noexcept;

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
        //! \brief  TODO
        //!
        virtual ~TraceFile() = default;

        TraceFile& operator=(TraceFile&&) = default;
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
    //! \brief  Adds a backtrace to the given file.
    //!
    //! \param[in]  targetFile  The file to add to.
    //! \param[in]  backtrace   The backtrace to add.
    //!
    //! \returns  The number of bytes written to the file.
    //!
    //! \note  This function is async signal safe.
    //!
    size_t add_backtrace(
        swimps::io::File& targetFile,
        const Backtrace& backtrace);

    //!
    //! \brief  Adds a stack frame to the given file.
    //!
    //! \param[in]  targetFile  The file to add to.
    //! \param[in]  stackFrame  The stack frame to add.
    //!
    //! \returns  The number of bytes written to the file.
    //!
    //! \note  This function is async signal safe.
    //!
    size_t add_stack_frame(
        swimps::io::File& targetFile,
        const StackFrame& stackFrame);

    //!
    //! \brief  Reads a trace from a given file.
    //!
    //! \param[in]  sourceFile  The file to read from.
    //!
    //! \returns  The trace contained in the file.
    //!
    //! \note  This function is *not* async signal safe.
    //!
    std::optional<Trace> read(swimps::io::File& sourceFile);

    //!
    //! \brief  Reads a backtrace from a given file.
    //!
    //! \param[in]  fileDescriptor  The file to read from.
    //!
    //! \returns  The backtrace if all went well, an empty optional otherwise.
    //!
    //! \note  No marker is expected, since the normal use case is to read to marker
    //!        to determine that it's a backtrace first.
    //!
    //! \note  The file descriptor shall be pointing at the data after the backtrace
    //!        if the read was successful. If it was not, its state is undefined.
    //!
    std::optional<swimps::trace::Backtrace> read_backtrace(swimps::io::File& sourceFile);

    //!
    //! \brief  Reads a stack frame from a given file.
    //!
    //! \param[in]  sourceFile  The file to read from.
    //!
    //! \returns  The stack frame if all went well, an empty optional otherwise.
    //!
    //! \note  No marker is expected.
    //!
    //! \note  The file descriptor shall be pointing at the data after the stack frame
    //!        if the read was successful. If it was not, its state is undefined.
    //!
    std::optional<swimps::trace::StackFrame> read_stack_frame(swimps::io::File& sourceFile);

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
    int finalise(swimps::io::File& traceFile);
}
