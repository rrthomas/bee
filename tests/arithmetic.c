// Test the arithmetic operators. Also uses the ROLL, POP, and LITERAL
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
    init((CELL *)calloc(1024, 1), 256);

    start_ass(EP);
    lit(1); ass(O_NEGATE);
    ass(O_WORD_BYTES);
    lit(-CELL_W);
    lit(2); ass(O_ROLL);
    ass(O_ADD); ass(O_ADD);
    ass(O_WORD_BYTES);
    ass(O_MUL);
    lit(3);
    ass(O_DIVMOD); ass(O_POP);
    lit(-2);
    ass(O_UDIVMOD);

    for (size_t i = 0; i < sizeof(correct) / sizeof(correct[0]); i++) {
        assert(single_step() == ERROR_STEP);
        printf("A = %s\n", disass(A, EP));
        show_data_stack();
        printf("Correct stack: %s\n\n", correct[i]);
        if (strcmp(correct[i], val_data_stack())) {
            printf("Error in arithmetic tests: EP = %"PRIu32"\n", EP);
            exit(1);
        }
    }

    printf("Arithmetic tests ran OK\n");
    return 0;
}
