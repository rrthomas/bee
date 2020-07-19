// Test the arithmetic operators. Also uses the SWAP, POP, and PUSH
// instructions. Since unsigned arithmetic overflow behaviour is guaranteed
// by the ISO C standard, we only test the stack handling and basic
// correctness of the operators here, assuming that if the arithmetic works
// in one case, it will work in all. Note that the correct stack values are
// not quite independent of the word size (in bee_WORD_BYTES and str(bee_WORD_BYTES)); some
// stack pictures implicitly refer to it.
//
// (c) Reuben Thomas 1994-2020
//
// The package is distributed under the GNU Public License version 3, or,
// at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#include "tests.h"


const char *correct[] = {
    "1", "-1", "-1 " str(bee_WORD_BYTES), "-1 " str(bee_WORD_BYTES) " -" str(bee_WORD_BYTES),
    "-1 " str(bee_WORD_BYTES) " -" str(bee_WORD_BYTES) " 0", "-1 -" str(bee_WORD_BYTES) " " str(bee_WORD_BYTES),
    "-1 -" str(bee_WORD_BYTES) " " str(bee_WORD_BYTES) " 1", str(bee_WORD_BYTES) " -" str(bee_WORD_BYTES) " -1",
    str(bee_WORD_BYTES) " -5", "-1", "-1 " str(bee_WORD_BYTES), "-" str(bee_WORD_BYTES),
    "-" str(bee_WORD_BYTES) " 3", "-1 -1", "-1", "-1 -2", "1 1"
};


int main(void)
{
    bee_init_defaults((bee_WORD *)calloc(1024, 1), 256);

    ass_goto(bee_m0);
    pushi(1); ass(BEE_INSN_NEGATE);
    ass(BEE_INSN_WORD_BYTES);
    pushi(-bee_WORD_BYTES);
    pushi(0); ass(BEE_INSN_SWAP);
    pushi(1); ass(BEE_INSN_SWAP);
    ass(BEE_INSN_ADD); ass(BEE_INSN_ADD);
    ass(BEE_INSN_WORD_BYTES);
    ass(BEE_INSN_MUL);
    pushi(3);
    ass(BEE_INSN_DIVMOD); ass(BEE_INSN_POP);
    pushi(-2);
    ass(BEE_INSN_UDIVMOD);

    for (size_t i = 0; i < sizeof(correct) / sizeof(correct[0]); i++) {
        printf("Instruction = %s\n", disass(*bee_pc, bee_pc));
        assert(single_step() == BEE_ERROR_BREAK);
        show_data_stack();
        printf("Correct stack: %s\n\n", correct[i]);
        if (strcmp(correct[i], val_data_stack())) {
            printf("Error in arithmetic tests: bee_pc = %p\n", bee_pc);
            exit(1);
        }
    }

    printf("Arithmetic tests ran OK\n");
    return 0;
}
