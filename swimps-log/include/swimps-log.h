#pragma once

#include <stddef.h>

typedef enum swimps_log_level {
    SWIMPS_LOG_LEVEL_FATAL,   //! something bad *is* happening and it cannot be recovered from
    SWIMPS_LOG_LEVEL_ERROR,   //! something bad *is* happening
    SWIMPS_LOG_LEVEL_WARNING, //! something bad *might* be happening
    SWIMPS_LOG_LEVEL_INFO,    //! something the user might want to know about is happening
    SWIMPS_LOG_LEVEL_DEBUG    //! something the developer might want to know about is happening
} swimps_log_level_t;

//!
//! \brief  Formats a message so that it's ready to be written to a log
//!
//! \param[in]   logLevel          The kind of log message (error, info, etc).
//! \param[in]   message           The message to be written.
//! \param[in]   messageSize       The size of the message, in bytes.
//! \param[out]  targetBuffer      Where to write the formatted message.
//! \param[in]   targetBufferSize  The size of the target buffer, in bytes.
//!
//! \returns  The number of bytes written to the target buffer.
//!
//! \note  If the target buffer isn't big enough to fit the whole message,
//!        it will be truncated.
//!
//! \note  The formatted message does not need to be null terminated.
//!
//! \note  The formatted message is not null terminated.
//!
//! \note  Message and target buffers must not overlap.
//!
//! \note  This function is async signal safe.
//!
size_t swimps_format_log_message(
    const swimps_log_level_t logLevel,
    const char* const __restrict__ message,
    const size_t messageSize,
    char* __restrict__ targetBuffer,
    const size_t targetBufferSize);

//!
//! \brief  Writes a message to all log targets, with the correct formatting.
//!
//! \param[in]  logLevel     The kind of log message (error, info, etc).
//! \param[in]  message      The message to write.
//! \param[in]  messageSize  The size of the message, in bytes.
//!
//! \returns  The number of bytes written to the log.
//!
//! \note  Messages must be less than 2048 bytes long *after* formatting.
//!
//! \note  This function is async signal safe.
//!
size_t swimps_write_to_log(
    const swimps_log_level_t loglLevel,
    const char* const message,
    const size_t messageSize);
