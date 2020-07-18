// Test the register instructions, except for those operating on SP and DP
// (see memory.c).
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
    const char *correct[64];
    unsigned steps = 0;

    size_t size = 1024;
    bee_init_defaults((WORD *)calloc(size, 1), size / WORD_BYTES);

    ass_goto(M0);
    ass(BEE_INSN_GET_M0);
    correct[steps++] = xasprintf("%"PRIi32, (WORD)M0);
    ass(BEE_INSN_POP);
    correct[steps++] = xasprintf("%s", "");
    ass(BEE_INSN_GET_MSIZE);
    correct[steps++] = xasprintf("%"PRIi32, (WORD)MSIZE);
    ass(BEE_INSN_POP);
    correct[steps++] = xasprintf("%s", "");
    ass(BEE_INSN_GET_SSIZE);
    correct[steps++] = xasprintf("%"PRIi32, (WORD)SSIZE);
    ass(BEE_INSN_POP);
    correct[steps++] = xasprintf("%s", "");
    ass(BEE_INSN_GET_DSIZE);
    correct[steps++] = xasprintf("%"PRIi32, (WORD)DSIZE);
    ass(BEE_INSN_POP);
    correct[steps++] = xasprintf("%s", "");
    ass(BEE_INSN_GET_HANDLER_SP);
    correct[steps++] = xasprintf("%"PRIi32, (WORD)HANDLER_SP);
    ass(BEE_INSN_POP);
    correct[steps++] = xasprintf("%s", "");

    for (unsigned i = 0; i < steps; i++) {
        printf("Instruction = %s\n", disass(*PC, PC));
        assert(single_step() == BEE_ERROR_BREAK);
        show_data_stack();
        printf("Correct stack: %s\n\n", correct[i]);
        if (strcmp(correct[i], val_data_stack())) {
            printf("Error in registers tests: PC = %p\n", PC);
            exit(1);
        }
    }

    printf("Registers tests ran OK\n");
    return 0;
}
