// Test that single_step works.
//
// (c) Reuben Thomas 1994-2020
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
    bee_init_defaults((bee_word_t *)calloc(size, BEE_WORD_BYTES), size);

    ass_goto(bee_m0);

    const bee_uword_t steps = 10;
    for (bee_uword_t i = 0; i < steps; i++) {
        // Assemble the test as we go!
        ass(BEE_INSN_WORD_BYTES);
        printf("pc = %p\n", bee_pc);
        assert(single_step() == BEE_ERROR_BREAK);
    }

    bee_word_t *final_PC = bee_m0 + steps;
    printf("bee_pc should now be %p\n", final_PC);
    if (bee_pc != final_PC) {
        printf("Error in single_step() tests: pc = %p\n", bee_pc);
        exit(1);
    }

    printf("single_step() tests ran OK\n");
    return 0;
}
