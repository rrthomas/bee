// Test that single_step works.
//
// (c) Reuben Thomas 1994-2020
//
// The package is distributed under the GNU Public License version 3, or,
// at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#include "tests.h"


int main(void)
{
    bee_init_defaults((bee_WORD *)calloc(1024, 1), 256);

    ass_goto(bee_m0);

    const bee_UWORD steps = 10;
    for (bee_UWORD i = 0; i < steps; i++) {
        // Assemble the test as we go!
        ass(BEE_INSN_WORD_BYTES);
        printf("bee_pc = %p\n", bee_pc);
        assert(single_step() == BEE_ERROR_BREAK);
    }

    bee_WORD *final_PC = bee_m0 + steps;
    printf("bee_pc should now be %p\n", final_PC);
    if (bee_pc != final_PC) {
        printf("Error in single_step() tests: bee_pc = %p\n", bee_pc);
        exit(1);
    }

    printf("single_step() tests ran OK\n");
    return 0;
}
