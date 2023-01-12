// Test PUSHI.
//
// (c) Reuben Thomas 1994-2023
//
// The package is distributed under the GNU General Public License version 3,
// or, at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#include "tests.h"


bool test(bee_state *S)
{
    pushi(BEE_ERROR_BREAK);
    correct[steps++] = xasprintf("-256");
    pushi(12345678);
    correct[steps++] = xasprintf("-256 12345678");

    return run_test("constants", S, false);
}
