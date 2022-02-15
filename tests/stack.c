// Test the stack operators. Also uses the PUSH instruction.
//
// (c) Reuben Thomas 1994-2020
//
// The package is distributed under the GNU General Public License version 3,
// or, at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#include "tests.h"


const char *correct[] = {
    "3 2 1", "3 2 3", "3 2 3 1", "3 2 3 2", "3 2 3", "3 2 3 0", "3 3 2",
    "3 3 2 1", "2 3 3", "2 3", "2 3 3", "2 3 3 3",
};


int main(void)
{
    size_t size = 256;
    bee_init_defaults((bee_word_t *)calloc(size, BUMBLE_WORD_BYTES), size);

    bee_d0[bee_dp++] = 3; bee_d0[bee_dp++] =2; bee_d0[bee_dp++] = 1;	// initialise the stack

    ass_goto(bee_m0);
    ass(BUMBLE_INSN_DUP);
    pushi(1);
    ass(BUMBLE_INSN_DUP);
    ass(BUMBLE_INSN_POP);
    pushi(0);
    ass(BUMBLE_INSN_SWAP);
    pushi(1);
    ass(BUMBLE_INSN_SWAP);
    ass(BUMBLE_INSN_PUSHS);
    ass(BUMBLE_INSN_DUPS);
    ass(BUMBLE_INSN_POPS);

    for (size_t i = 0; i < sizeof(correct) / sizeof(correct[0]) - 1; i++) {
        show_data_stack();
        printf("Correct stack: %s\n\n", correct[i]);
        if (strcmp(correct[i], val_data_stack())) {
            printf("Error in stack tests: pc = %p\n", bee_pc);
            exit(1);
        }
        printf("Instruction = %s\n", disass(*bee_pc, bee_pc));
        assert(single_step() == BUMBLE_ERROR_BREAK);
    }

    printf("Stack tests ran OK\n");
    return 0;
}
