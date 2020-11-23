// Test the register instructions, except for those operating on sp and dp
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
    char *correct[64];
    unsigned steps = 0;

    size_t size = 1024;
    bee_init_defaults((bee_word_t *)calloc(size, 1), size / BEE_WORD_BYTES);

    ass_goto(bee_m0);
    ass(BEE_INSN_GET_M0);
    correct[steps++] = xasprintf("%zd", (bee_word_t)bee_m0);
    ass(BEE_INSN_POP);
    correct[steps++] = xasprintf("%s", "");
    ass(BEE_INSN_GET_MSIZE);
    correct[steps++] = xasprintf("%zd", (bee_word_t)bee_msize);
    ass(BEE_INSN_POP);
    correct[steps++] = xasprintf("%s", "");
    ass(BEE_INSN_GET_SSIZE);
    correct[steps++] = xasprintf("%zd", (bee_word_t)bee_ssize);
    ass(BEE_INSN_POP);
    correct[steps++] = xasprintf("%s", "");
    ass(BEE_INSN_GET_DSIZE);
    correct[steps++] = xasprintf("%zd", (bee_word_t)bee_dsize);
    ass(BEE_INSN_POP);
    correct[steps++] = xasprintf("%s", "");
    ass(BEE_INSN_GET_HANDLER_SP);
    correct[steps++] = xasprintf("%zd", (bee_word_t)bee_handler_sp);
    ass(BEE_INSN_POP);
    correct[steps++] = xasprintf("%s", "");

    for (unsigned i = 0; i < steps; i++) {
        printf("Instruction = %s\n", disass(*bee_pc, bee_pc));
        assert(single_step() == BEE_ERROR_BREAK);
        show_data_stack();
        printf("Correct stack: %s\n\n", correct[i]);
        if (strcmp(correct[i], val_data_stack())) {
            printf("Error in registers tests: pc = %p\n", bee_pc);
            exit(1);
        }
        free(correct[i]);
    }

    printf("Registers tests ran OK\n");
    return 0;
}
