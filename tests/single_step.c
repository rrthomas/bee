// Test that single_step works.
//
// (c) Reuben Thomas 1994-2022
//
// The package is distributed under the GNU General Public License version 3,
// or, at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#include "tests.h"


int main(void)
{
    size_t size = 256;
    bee_word_t *m0 = (bee_word_t *)calloc(size, BEE_WORD_BYTES);
    bee_state *S = init_defaults(m0);

    ass_goto(m0);

    const bee_uword_t steps = 10;
    for (bee_uword_t i = 0; i < steps; i++) {
        // Assemble the test as we go!
        ass(BEE_INSN_WORD_BYTES);
        printf("pc = %p\n", S->pc);
        assert(single_step(S) == BEE_ERROR_BREAK);
    }

    // We execute two instructions per word: WORD_BYTES followed by NOP.
    bee_word_t *final_pc = m0 + steps / 2;
    printf("S->pc should now be %p\n", final_pc);
    if (S->pc != final_pc) {
        printf("Error in single_step() tests: pc = %p\n", S->pc);
        exit(1);
    }

    printf("single_step() tests ran OK\n");
    bee_destroy(S);
    free(m0);
    return 0;
}
