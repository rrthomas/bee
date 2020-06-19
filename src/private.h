// Private implementation-specific APIs that are shared between modules.
//
// (c) Reuben Thomas 1994-2018
//
// The package is distributed under the GNU Public License version 3, or,
// at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#ifndef BEE_PRIVATE
#define BEE_PRIVATE


#include "config.h"

// Memory access

// Address checking
#define CHECK_ADDRESS(a, cond, code, label)     \
    if (!(cond)) {                              \
        exception = code;                       \
        goto label;                             \
    }

#define CHECK_ALIGNED(a)                                \
    CHECK_ADDRESS(a, IS_ALIGNED(a), -23, exception)


#endif
