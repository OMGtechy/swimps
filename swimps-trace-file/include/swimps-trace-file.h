#pragma once

#include "swimps-time.h"
#include "swimps-trace.h"

#include <cstddef>
#include <optional>

#include <unistd.h>

namespace swimps::trace::file {
    constexpr char swimps_v1_trace_file_marker[] = "swimps_v1_trace_file";
    constexpr size_t swimps_v1_trace_entry_marker_size  = sizeof "\n__!\n";
    constexpr char swimps_v1_trace_symbolic_backtrace_marker[swimps_v1_trace_entry_marker_size] = "\nsb!\n";
    constexpr char swimps_v1_trace_sample_marker[swimps_v1_trace_entry_marker_size] = "\nsp!\n";

    //!
    //! \brief  Creates a file with the necessary headers to store a swimps_trace.
    //!
    //! \param[in]  path  Where to create the file.
    //!
    //! \returns  The file descriptor for the trace file, or -1 if something went wrong.
    //!
    //! \note  This function is async signal safe.
    //!
    int create(const char* const path);

    //!
    //! \brief  Generates a name for the trace file based upon a program name.
    //!
    //! \param[in]   programName       The name of the program the trace file is for.
    //! \param[in]   time              The timestamp to use when generating the name.
    //! \param[in]   pid               The process ID of the program the trace file is for.
    //! \param[out]  targetBuffer      Where to write the generated name.
    //! \param[in]   targetBufferSize  How large the target buffer is.
    //!
    //! \returns  The number of bytes written to the target buffer.
    //!
    //! \note  This function is *not* async signal safe.
    //!        TODO: should it be? Would need to implement format specifiers for int64 for that...
    //!
    size_t generate_name(
        const char* const programName,
         const swimps::time::TimeSpecification* const time,
         const pid_t pid,
         char* const targetBuffer,
         const size_t targetBufferSize);

    //!
    //! \brief  Adds a sample to the given file.
    //!
    //! \param[in]  targetFileDescriptor  The file to add to.
    //! \param[in]  sample                The sample to add.
    //!
    //! \returns  The number of bytes written to the file.
    //!
    //! \note  This function is async signal safe.
    //!
    size_t add_sample(
        const int targetFileDescriptor,
        const swimps::trace::Sample* const sample);

    //!
    //! \brief  Adds a backtrace to the given file.
    //!
    //! \param[in]  targetFileDescriptor  The file to add to.
    //! \param[in]  backtrace             The backtrace to add.
    //!
    //! \returns  The number of bytes written to the file.
    //!
    //! \note  This function is async signal safe.
    //!
    size_t add_backtrace(
        const int targetFileDescriptor,
        const Backtrace& backtrace);

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
    //         if the read was successful. If it was not, its state is undefined.
    //!
    std::optional<swimps::trace::Backtrace> read_backtrace(const int fileDescriptor);

    //!
    //! \brief  Finalises an opened trace file.
    //!
    //! \param[in]  fileDescriptor     The file to finalise.
    //! \param[in]  traceFilePath      The path to the trace file.
    //! \param[in]  traceFilePathSize  The size of the trace file path.
    //!
    //! \returns  0 if successful, -1 otherwise (errno may be set).
    //!
    //! \note  You should not modify the file are finalising it.
    //!
    //! \note  This function is *not* async signal safe.
    //!
    int finalise(const int fileDescriptor, const char* traceFilePath, const size_t traceFilePathSize);
}
