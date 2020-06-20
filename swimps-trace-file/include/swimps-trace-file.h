#pragma once

#include "swimps-time.h"

#include <stddef.h>
#include <unistd.h>

//!
//! \brief  Creates a file with the necessary headers to store a swimps_trace.
//!
//! \param[in]  path  Where to create the file.
//!
//! \returns  The file descriptor for the trace file, or -1 if something went wrong.
//!
//! \note  This function is async signal safe.
//!
int swimps_trace_file_create(const char* const path);

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
size_t swimps_trace_file_generate_name(const char* const programName,
                                       const swimps_timespec_t* const time,
                                       const pid_t pid,
                                       char* const targetBuffer,
                                       const size_t targetBufferSize);

