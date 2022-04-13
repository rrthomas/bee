// The traps.
//
// (c) Reuben Thomas 1994-2022
//
// The package is distributed under the GNU General Public License version 3,
// or, at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#include "config.h"

#include "bee/bee.h"

#include "private.h"
#include "traps.h"


bee_word_t trap(bee_state * restrict S, bee_word_t code)
{
    int error = BEE_ERROR_OK;

    switch (code) {
    case TRAP_LIBC:
        return trap_libc(S);
    default:
        return BEE_ERROR_INVALID_LIBRARY;
    }

    return error;
}
