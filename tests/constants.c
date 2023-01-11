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


int main(void)
{
    char *correct[64];
    unsigned steps = 0;

    size_t size = 256;
    bee_word_t *m0 = (bee_word_t *)calloc(size, BEE_WORD_BYTES);
    bee_state *S = init_defaults(m0);

    ass_goto(m0);
    pushi(BEE_ERROR_BREAK);
    correct[steps++] = xasprintf("-256");
    pushi(12345678);
    correct[steps++] = xasprintf("-256 12345678");

    assert(run_test("constants", S, correct, steps, false));
    bee_destroy(S);
    free(m0);
    return 0;
}
