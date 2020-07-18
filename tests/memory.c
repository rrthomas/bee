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
    bee_init_defaults((WORD *)calloc(size, WORD_BYTES), size);

    ass_goto(M0);
    pushreli(M0 + MSIZE / WORD_BYTES);
    correct[steps++] = xasprintf("%"PRIi32, m0 + memory);
    ass(BEE_INSN_WORD_BYTES);
    correct[steps++] = xasprintf("%"PRIi32" %d", m0 + memory, WORD_BYTES);
    ass(BEE_INSN_NEGATE);
    correct[steps++] = xasprintf("%"PRIi32" %d", m0 + memory, -WORD_BYTES);
    ass(BEE_INSN_ADD);
    correct[steps++] = xasprintf("%"PRIi32, m0 + memory - WORD_BYTES);
    pushi(513);
    correct[steps++] = xasprintf("%"PRIi32" %d", m0 + memory - WORD_BYTES, 513);
    pushi(1);
    correct[steps++] = xasprintf("%"PRIi32" %d %d", m0 + memory - WORD_BYTES, 513, 1);
    ass(BEE_INSN_DUP);
    correct[steps++] = xasprintf("%"PRIi32" %d %"PRIi32, m0 + memory - WORD_BYTES, 513, m0 + memory - WORD_BYTES);
    ass(BEE_INSN_STORE);
    correct[steps++] = xasprintf("%"PRIi32, m0 + memory - WORD_BYTES);
    pushi(0);
    correct[steps++] = xasprintf("%"PRIi32" %d", m0 + memory - WORD_BYTES, 0);
    ass(BEE_INSN_DUP);
    correct[steps++] = xasprintf("%"PRIi32" %"PRIi32, m0 + memory - WORD_BYTES, m0 + memory - WORD_BYTES);
    ass(BEE_INSN_LOAD);
    correct[steps++] = xasprintf("%"PRIi32" %d", m0 + memory - WORD_BYTES, 513);
    ass(BEE_INSN_POP);
    correct[steps++] = xasprintf("%"PRIi32, m0 + memory - WORD_BYTES);
    pushi(0);
    correct[steps++] = xasprintf("%"PRIi32" %d", m0 + memory - WORD_BYTES, 0);
    ass(BEE_INSN_DUP);
    correct[steps++] = xasprintf("%"PRIi32" %"PRIi32, m0 + memory - WORD_BYTES, m0 + memory - WORD_BYTES);
    ass(BEE_INSN_LOAD1);
    correct[steps++] = xasprintf("%"PRIi32" %d", m0 + memory - WORD_BYTES, 1);
    ass(BEE_INSN_ADD);
    correct[steps++] = xasprintf("%"PRIi32, m0 + memory - WORD_BYTES + 1);
    ass(BEE_INSN_LOAD1);
    correct[steps++] = xasprintf("%d", 2);
    pushreli(M0 + memory / WORD_BYTES);
    correct[steps++] = xasprintf("%d %"PRIi32, 2, m0 + memory);
    pushi(-1);
    correct[steps++] = xasprintf("%d %"PRIi32" %d", 2, m0 + memory, -1);
    ass(BEE_INSN_ADD);
    correct[steps++] = xasprintf("%d %"PRIi32, 2, m0 + memory - 1);
    ass(BEE_INSN_STORE1);
    correct[steps++] = xasprintf("%s", "");
    pushreli(M0 + memory / WORD_BYTES);
    correct[steps++] = xasprintf("%"PRIi32, m0 + memory);
    pushi(-4);
    correct[steps++] = xasprintf("%"PRIi32" %d", m0 + memory, -4);
    ass(BEE_INSN_ADD);
    correct[steps++] = xasprintf("%"PRIi32, m0 + memory - 4);
    ass(BEE_INSN_LOAD4);
    correct[steps++] = xasprintf("%"PRIi32, 33554945);
    ass(BEE_INSN_POP);
    correct[steps++] = xasprintf("%s", "");
    pushreli(M0 + memory / WORD_BYTES);
    correct[steps++] = xasprintf("%"PRIi32, m0 + memory);
    pushi(-4);
    correct[steps++] = xasprintf("%"PRIi32" %d", m0 + memory, -4);
    ass(BEE_INSN_ADD);
    correct[steps++] = xasprintf("%"PRIi32, m0 + memory - 4);
    ass(BEE_INSN_LOAD2);
    correct[steps++] = xasprintf("%d", 513);
    pushreli(M0 + memory / WORD_BYTES);
    correct[steps++] = xasprintf("%d %"PRIi32, 513, m0 + memory);
    pushi(-2);
    correct[steps++] = xasprintf("%d %"PRIi32" %d", 513, m0 + memory, -2);
    ass(BEE_INSN_ADD);
    correct[steps++] = xasprintf("%d %"PRIi32, 513, m0 + memory - 2);
    ass(BEE_INSN_STORE2);
    correct[steps++] = xasprintf("%s", "");
    pushreli(M0 + memory / WORD_BYTES);
    correct[steps++] = xasprintf("%"PRIi32, m0 + memory);
    pushi(-4);
    correct[steps++] = xasprintf("%"PRIi32" %d", m0 + memory, -4);
    ass(BEE_INSN_ADD);
    correct[steps++] = xasprintf("%"PRIi32, m0 + memory - 4);
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
        printf("Instruction = %s\n", disass(*PC, PC));
        assert(single_step() == BEE_ERROR_BREAK);
        show_data_stack();
        printf("Correct stack: %s\n\n", correct[i]);
        if (strcmp(correct[i], val_data_stack())) {
            printf("Error in memory tests: PC = %p\n", PC);
            exit(1);
        }
    }

    printf("Memory tests ran OK\n");
    return 0;
}
