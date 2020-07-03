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
    WORD temp = 0;
    int error = 0;

    init_defaults((WORD *)calloc(1024, 1), 256);

    ass_goto(PC);
    push(1); ass(O_NEGATE);
    ass(O_WORD_BYTES);
    push(-WORD_BYTES);
    push(0); ass(O_SWAP);
    push(1); ass(O_SWAP);
    ass(O_ADD); ass(O_ADD);
    ass(O_WORD_BYTES);
    ass(O_MUL);
    push(3);
    ass(O_DIVMOD); ass(O_POP);
    push(-2);
    ass(O_UDIVMOD);

    for (size_t i = 0; i < sizeof(correct) / sizeof(correct[0]); i++) {
        printf("Instruction = %s\n", disass(LOAD_WORD(PC), PC));
        assert(single_step() == ERROR_STEP);
        show_data_stack();
        printf("Correct stack: %s\n\n", correct[i]);
        if (strcmp(correct[i], val_data_stack())) {
            printf("Error in arithmetic tests: PC = %"PRIu32"\n", PC);
            exit(1);
        }
    }

    printf("Arithmetic tests ran OK\n");
    return 0;
}
