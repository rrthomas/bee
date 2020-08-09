// Test the memory operators. Also uses previously tested instructions.
// See errors.c for address error handling tests.
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

    size_t size = 4096;
    bee_init_defaults((bee_word_t *)calloc(size, BEE_WORD_BYTES), size);

    // Naturally bee_uword_t, but must be printed as bee_word_t for comparison with
    // output of val_data_stack().
    bee_word_t MEND = (bee_word_t)(uint8_t *)bee_m0 + bee_msize;
    ass_goto(bee_m0);
    pushreli((bee_word_t *)MEND);
    correct[steps++] = xasprintf("%"PRIi32, MEND);
    ass(BEE_INSN_WORD_BYTES);
    correct[steps++] = xasprintf("%"PRIi32" %d", MEND, BEE_WORD_BYTES);
    ass(BEE_INSN_NEG);
    correct[steps++] = xasprintf("%"PRIi32" %d", MEND, -BEE_WORD_BYTES);
    ass(BEE_INSN_ADD);
    correct[steps++] = xasprintf("%"PRIi32, MEND - BEE_WORD_BYTES);
    pushi(513);
    correct[steps++] = xasprintf("%"PRIi32" %d", MEND - BEE_WORD_BYTES, 513);
    pushi(1);
    correct[steps++] = xasprintf("%"PRIi32" %d %d", MEND - BEE_WORD_BYTES, 513, 1);
    ass(BEE_INSN_DUP);
    correct[steps++] = xasprintf("%"PRIi32" %d %"PRIi32, MEND - BEE_WORD_BYTES, 513, MEND - BEE_WORD_BYTES);
    ass(BEE_INSN_STORE);
    correct[steps++] = xasprintf("%"PRIi32, MEND - BEE_WORD_BYTES);
    pushi(0);
    correct[steps++] = xasprintf("%"PRIi32" %d", MEND - BEE_WORD_BYTES, 0);
    ass(BEE_INSN_DUP);
    correct[steps++] = xasprintf("%"PRIi32" %"PRIi32, MEND - BEE_WORD_BYTES, MEND - BEE_WORD_BYTES);
    ass(BEE_INSN_LOAD);
    correct[steps++] = xasprintf("%"PRIi32" %d", MEND - BEE_WORD_BYTES, 513);
    ass(BEE_INSN_POP);
    correct[steps++] = xasprintf("%"PRIi32, MEND - BEE_WORD_BYTES);
    pushi(0);
    correct[steps++] = xasprintf("%"PRIi32" %d", MEND - BEE_WORD_BYTES, 0);
    ass(BEE_INSN_DUP);
    correct[steps++] = xasprintf("%"PRIi32" %"PRIi32, MEND - BEE_WORD_BYTES, MEND - BEE_WORD_BYTES);
    ass(BEE_INSN_LOAD1);
    correct[steps++] = xasprintf("%"PRIi32" %d", MEND - BEE_WORD_BYTES, 1);
    ass(BEE_INSN_ADD);
    correct[steps++] = xasprintf("%"PRIi32, MEND - BEE_WORD_BYTES + 1);
    ass(BEE_INSN_LOAD1);
    correct[steps++] = xasprintf("%d", 2);
    pushreli((bee_word_t *)MEND);
    correct[steps++] = xasprintf("%d %"PRIi32, 2, MEND);
    pushi(-1);
    correct[steps++] = xasprintf("%d %"PRIi32" %d", 2, MEND, -1);
    ass(BEE_INSN_ADD);
    correct[steps++] = xasprintf("%d %"PRIi32, 2, MEND - 1);
    ass(BEE_INSN_STORE1);
    correct[steps++] = xasprintf("%s", "");
    pushreli((bee_word_t *)MEND);
    correct[steps++] = xasprintf("%"PRIi32, MEND);
    pushi(-4);
    correct[steps++] = xasprintf("%"PRIi32" %d", MEND, -4);
    ass(BEE_INSN_ADD);
    correct[steps++] = xasprintf("%"PRIi32, MEND - 4);
    ass(BEE_INSN_LOAD4);
    correct[steps++] = xasprintf("%"PRIi32, 33554945);
    ass(BEE_INSN_POP);
    correct[steps++] = xasprintf("%s", "");
    pushreli((bee_word_t *)MEND);
    correct[steps++] = xasprintf("%"PRIi32, MEND);
    pushi(-4);
    correct[steps++] = xasprintf("%"PRIi32" %d", MEND, -4);
    ass(BEE_INSN_ADD);
    correct[steps++] = xasprintf("%"PRIi32, MEND - 4);
    ass(BEE_INSN_LOAD2);
    correct[steps++] = xasprintf("%d", 513);
    pushreli((bee_word_t *)MEND);
    correct[steps++] = xasprintf("%d %"PRIi32, 513, MEND);
    pushi(-2);
    correct[steps++] = xasprintf("%d %"PRIi32" %d", 513, MEND, -2);
    ass(BEE_INSN_ADD);
    correct[steps++] = xasprintf("%d %"PRIi32, 513, MEND - 2);
    ass(BEE_INSN_STORE2);
    correct[steps++] = xasprintf("%s", "");
    pushreli((bee_word_t *)MEND);
    correct[steps++] = xasprintf("%"PRIi32, MEND);
    pushi(-4);
    correct[steps++] = xasprintf("%"PRIi32" %d", MEND, -4);
    ass(BEE_INSN_ADD);
    correct[steps++] = xasprintf("%"PRIi32, MEND - 4);
    ass(BEE_INSN_LOAD);
    correct[steps++] = xasprintf("%"PRIi32, 33620481);
    ass(BEE_INSN_POP);
    correct[steps++] = xasprintf("%s", "");
    ass(BEE_INSN_GET_SP);
    correct[steps++] = xasprintf("%d", 0);
    ass(BEE_INSN_SET_SP);
    correct[steps++] = xasprintf("%s", "");
    ass(BEE_INSN_GET_SP);
    correct[steps++] = xasprintf("%d", 0);
    ass(BEE_INSN_POP);
    correct[steps++] = xasprintf("%s", "");
    pushi(0);
    correct[steps++] = xasprintf("%d", 0);
    ass(BEE_INSN_SET_SP);
    correct[steps++] = xasprintf("%s", "");
    ass(BEE_INSN_GET_SP);
    correct[steps++] = xasprintf("%d", 0);

    for (size_t i = 0; i < steps; i++) {
        printf("Instruction = %s\n", disass(*bee_pc, bee_pc));
        assert(single_step() == BEE_ERROR_BREAK);
        show_data_stack();
        printf("Correct stack: %s\n\n", correct[i]);
        if (strcmp(correct[i], val_data_stack())) {
            printf("Error in memory tests: bee_pc = %p\n", bee_pc);
            exit(1);
        }
    }

    printf("Memory tests ran OK\n");
    return 0;
}
