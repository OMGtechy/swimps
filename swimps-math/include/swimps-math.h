#pragma once

//!
//! \brief  Gets the minimum of the two arguments, compared using operator <.
//!
//! \param[in]  lhs  The left hand operand for operator <.
//! \param[in]  rhs  The right hand operand for operator <.
//!
//! \returns  The smaller of lhs and rhs, as defined by operator <.
//!
#define swimps_min(lhs, rhs) (((lhs) < (rhs)) ? (lhs) : (rhs))

//!
//! \brief  Gets the maximum of the two arguments, compared using operator >.
//!
//! \param[in]  lhs  The left hand operand for operator >.
//! \param[in]  rhs  The right hand operand for operator >.
//!
//! \returns  The larger of lhs and rhs, as defined by operator >.
//!
#define swimps_max(lhs, rhs) (((lhs) > (rhs)) ? (lhs) : (rhs))

