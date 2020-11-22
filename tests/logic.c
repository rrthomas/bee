// Test the logic operators. Also uses the SWAP instruction. We only test
// the stack handling and basic correctness of the operators here, assuming
// that if the logic works in one case, it will work in all (if the C
// compiler doesn't implement it correctly, we're in trouble anyway!).
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

    size_t size = 256;
    bee_init_defaults((bee_word_t *)calloc(size, BEE_WORD_BYTES), size);

    int BYTE_BIT = 8;
    bee_word_t BOTTOM_BYTE_SET = 0xffUL;
    bee_word_t SECOND_BYTE_SET = LSHIFT(0xffUL, BYTE_BIT);
    bee_word_t PENULTIMATE_BYTE_SET = LSHIFT(0xffUL, BEE_WORD_BIT - 2 * BYTE_BIT);
    bee_word_t TOP_BYTE_SET = LSHIFT(-1, BEE_WORD_BIT - BYTE_BIT);

    ass_goto(bee_m0);

    pushi(BYTE_BIT);
    correct[steps++] = xasprintf("%d", BYTE_BIT);
    pushi(TOP_BYTE_SET);
    correct[steps++] = xasprintf("%d %zd", BYTE_BIT, TOP_BYTE_SET);
    pushi(BOTTOM_BYTE_SET);
    correct[steps++] = xasprintf("%d %zd %zd", BYTE_BIT, TOP_BYTE_SET, BOTTOM_BYTE_SET);
    pushi(BYTE_BIT);
    correct[steps++] = xasprintf("%d %zd %zd %d", BYTE_BIT, TOP_BYTE_SET, BOTTOM_BYTE_SET, BYTE_BIT);
    ass(BEE_INSN_LSHIFT);
    correct[steps++] = xasprintf("%d %zd %zd", BYTE_BIT, TOP_BYTE_SET, SECOND_BYTE_SET);
    pushi(1);
    correct[steps++] = xasprintf("%d %zd %zd %d", BYTE_BIT, TOP_BYTE_SET, SECOND_BYTE_SET, 1);
    ass(BEE_INSN_SWAP);
    correct[steps++] = xasprintf("%zd %zd %d", SECOND_BYTE_SET, TOP_BYTE_SET, BYTE_BIT);
    ass(BEE_INSN_RSHIFT);
    correct[steps++] = xasprintf("%zd %zd", SECOND_BYTE_SET, PENULTIMATE_BYTE_SET);
    ass(BEE_INSN_OR);
    correct[steps++] = xasprintf("%zd", SECOND_BYTE_SET | PENULTIMATE_BYTE_SET);
    ass(BEE_INSN_NOT);
    correct[steps++] = xasprintf("%zd", ~(SECOND_BYTE_SET | PENULTIMATE_BYTE_SET));
    pushi(1024);
    correct[steps++] = xasprintf("%zd %d", ~(SECOND_BYTE_SET | PENULTIMATE_BYTE_SET), 1024);
    pushi(-1);
    correct[steps++] = xasprintf("%zd %d %d", ~(SECOND_BYTE_SET | PENULTIMATE_BYTE_SET), 1024, -1);
    ass(BEE_INSN_XOR);
    correct[steps++] = xasprintf("%zd %d", ~(SECOND_BYTE_SET | PENULTIMATE_BYTE_SET), -1025);
    ass(BEE_INSN_AND);
    correct[steps++] = xasprintf("%zd", ~(SECOND_BYTE_SET | PENULTIMATE_BYTE_SET) & -1025);

    for (size_t i = 0; i < steps; i++) {
        printf("Instruction = %s\n", disass(*bee_pc, bee_pc));
        assert(single_step() == BEE_ERROR_BREAK);
        show_data_stack();
        printf("Correct stack: %s\n\n", correct[i]);
        if (strcmp(correct[i], val_data_stack())) {
            printf("Error in logic tests: pc = %p\n", bee_pc);
            exit(1);
        }
    }

    printf("Logic tests ran OK\n");
    return 0;
}
