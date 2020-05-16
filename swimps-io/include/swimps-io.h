#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

//!
//! \brief  Writes the provided data to the buffer specified, up to the target size.
//!         This is much the same as memcpy_s on Windows: it's a memcpy that checks the size of the target buffer.
//!
//! \param[in]   sourceBuffer      The data to be written to the buffer. Does not need to be null terminated.
//! \param[in]   sourceBufferSize  The number of bytes to read from the source buffer.
//! \param[out]  targetBuffer      The buffer to write the data to. Will not be null terminated.
//! \param[in]   targetBufferSize  The number maximum of bytes to write to the target buffer.
//!
//! \returns  The number of bytes written to the target buffer.
//!
//! \note  The source will be written to the target completely if there is space.
//!        If there is not enough space, as many bytes as possible will be written.
//!
//! \note  The source and target buffers must *not* overlap in memory.
//!
//! \note  The source and target must not be NULL.
//!
size_t swimps_write_to_buffer(const char* __restrict__ sourceBuffer,
                              size_t sourceBufferSize,
                              char* __restrict__ targetBuffer,
                              size_t targetBufferSize);

#ifdef __cplusplus
}
#endif
