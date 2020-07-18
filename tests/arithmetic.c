// Test the arithmetic operators. Also uses the SWAP, POP, and PUSH
// instructions. Since unsigned arithmetic overflow behaviour is guaranteed
// by the ISO C standard, we only test the stack handling and basic
// correctness of the operators here, assuming that if the arithmetic works
// in one case, it will work in all. Note that the correct stack values are
// not quite independent of the word size (in WORD_BYTES and str(WORD_BYTES)); some
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
    "1", "-1", "-1 " str(WORD_BYTES), "-1 " str(WORD_BYTES) " -" str(WORD_BYTES),
    "-1 " str(WORD_BYTES) " -" str(WORD_BYTES) " 0", "-1 -" str(WORD_BYTES) " " str(WORD_BYTES),
    "-1 -" str(WORD_BYTES) " " str(WORD_BYTES) " 1", str(WORD_BYTES) " -" str(WORD_BYTES) " -1",
    str(WORD_BYTES) " -5", "-1", "-1 " str(WORD_BYTES), "-" str(WORD_BYTES),
    "-" str(WORD_BYTES) " 3", "-1 -1", "-1", "-1 -2", "1 1"
};


int main(void)
{
    init_defaults((WORD *)calloc(1024, 1), 256);

    ass_goto(M0);
    pushi(1); ass(BEE_INSN_NEGATE);
    ass(BEE_INSN_WORD_BYTES);
    pushi(-WORD_BYTES);
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
        printf("Instruction = %s\n", disass(*PC, PC));
        assert(single_step() == ERROR_BREAK);
        show_data_stack();
        printf("Correct stack: %s\n\n", correct[i]);
        if (strcmp(correct[i], val_data_stack())) {
            printf("Error in arithmetic tests: PC = %p\n", PC);
            exit(1);
        }
    }

    printf("Arithmetic tests ran OK\n");
    return 0;
}
