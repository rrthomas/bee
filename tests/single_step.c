// Test that single_step works.
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
    const bee_uword_t cycles = 10;
    for (bee_uword_t i = 0; i < cycles; i++) {
        // Assemble the test as we go!
        ass(BEE_INSN_WORD_BYTES);
        printf("pc = %p\n", S->pc);
        assert(single_step(S) == BEE_ERROR_BREAK);
    }

    // We execute two instructions per word: WORD_BYTES followed by NOP.
    bee_word_t *final_pc = m0 + cycles / 2;
    printf("S->pc should now be %p\n", final_pc);
    if (S->pc != final_pc) {
        printf("Error in single_step() tests: pc = %p\n", S->pc);
        return false;
    }

    printf("single_step() tests ran OK\n");
    return true;
}
