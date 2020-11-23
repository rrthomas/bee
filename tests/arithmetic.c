// Test the arithmetic operators. Also uses the SWAP, POP, and PUSH
// instructions. Since unsigned arithmetic overflow behaviour is guaranteed
// by the ISO C standard, we only test the stack handling and basic
// correctness of the operators here, assuming that if the arithmetic works
// in one case, it will work in all. Note that the correct stack values are
// not quite independent of the word size (in BEE_WORD_BYTES); some stack
// pictures implicitly refer to it.
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

    size_t size = 256;
    bee_init_defaults((bee_word_t *)calloc(size, BEE_WORD_BYTES), size);

    ass_goto(bee_m0);
    pushi(1);
    correct[steps++] = xasprintf("1");
    ass(BEE_INSN_NEG);
    correct[steps++] = xasprintf("-1");
    ass(BEE_INSN_WORD_BYTES);
    correct[steps++] = xasprintf("-1 %d", BEE_WORD_BYTES);
    pushi(-BEE_WORD_BYTES);
    correct[steps++] = xasprintf("-1 %d -%d", BEE_WORD_BYTES, BEE_WORD_BYTES);
    pushi(0);
    correct[steps++] = xasprintf("-1 %d -%d 0", BEE_WORD_BYTES, BEE_WORD_BYTES);
    ass(BEE_INSN_SWAP);
    correct[steps++] = xasprintf("-1 -%d %d", BEE_WORD_BYTES, BEE_WORD_BYTES);
    pushi(1);
    correct[steps++] = xasprintf("-1 -%d %d 1", BEE_WORD_BYTES, BEE_WORD_BYTES);
    ass(BEE_INSN_SWAP);
    correct[steps++] = xasprintf("%d -%d -1", BEE_WORD_BYTES, BEE_WORD_BYTES);
    ass(BEE_INSN_ADD);
    correct[steps++] = xasprintf("%d %d", BEE_WORD_BYTES, -BEE_WORD_BYTES - 1);
    ass(BEE_INSN_ADD);
    correct[steps++] = xasprintf("-1");
    ass(BEE_INSN_WORD_BYTES);
    correct[steps++] = xasprintf("-1 %d", BEE_WORD_BYTES);
    ass(BEE_INSN_MUL);
    correct[steps++] = xasprintf("-%d", BEE_WORD_BYTES);
    pushi(BEE_WORD_BYTES - 1);
    correct[steps++] = xasprintf("-%d %d", BEE_WORD_BYTES, BEE_WORD_BYTES - 1);
    ass(BEE_INSN_DIVMOD);
    correct[steps++] = xasprintf("-1 -1");
    ass(BEE_INSN_POP);
    correct[steps++] = xasprintf("-1");
    pushi(-2);
    correct[steps++] = xasprintf("-1 -2");
    ass(BEE_INSN_UDIVMOD);
    correct[steps++] = xasprintf("1 1");

    for (size_t i = 0; i < steps; i++) {
        printf("Instruction = %s\n", disass(*bee_pc, bee_pc));
        assert(single_step() == BEE_ERROR_BREAK);
        show_data_stack();
        printf("Correct stack: %s\n\n", correct[i]);
        if (strcmp(correct[i], val_data_stack())) {
            printf("Error in arithmetic tests: pc = %p\n", bee_pc);
            exit(1);
        }
        free(correct[i]);
    }

    printf("Arithmetic tests ran OK\n");
    return 0;
}
