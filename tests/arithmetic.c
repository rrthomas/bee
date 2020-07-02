// Test the arithmetic operators. Also uses the ROLL, POP, and PUSH
// instructions. Since unsigned arithmetic overflow behaviour is guaranteed
// by the ISO C standard, we only test the stack handling and basic
// correctness of the operators here, assuming that if the arithmetic works
// in one case, it will work in all. Note that the correct stack values are
// not quite independent of the cell size (in CELL_W and str(CELL_W)); some
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
    "1", "-1", "-1 " str(CELL_W), "-1 " str(CELL_W) " -" str(CELL_W),
    "-1 " str(CELL_W) " -" str(CELL_W) " 2", str(CELL_W) " -" str(CELL_W) " -1",
    str(CELL_W) " -5", "-1", "-1 " str(CELL_W), "-" str(CELL_W),
    "-" str(CELL_W) " 3", "-1 -1", "-1", "-1 -2", "1 1"
};


int main(void)
{
    CELL temp = 0;
    int exception = 0;

    init((CELL *)calloc(1024, 1), 256);

    start_ass(PC);
    push(1); ass(O_NEGATE);
    ass(O_WORD_BYTES);
    push(-CELL_W);
    push(2); ass(O_ROLL);
    ass(O_ADD); ass(O_ADD);
    ass(O_WORD_BYTES);
    ass(O_MUL);
    push(3);
    ass(O_DIVMOD); ass(O_POP);
    push(-2);
    ass(O_UDIVMOD);

    for (size_t i = 0; i < sizeof(correct) / sizeof(correct[0]); i++) {
        printf("Instruction = %s\n", disass(LOAD_CELL(PC), PC));
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
