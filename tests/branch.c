// Test the branch instructions. Also uses other instructions with lower
// opcodes than the instructions tested (i.e. those already tested).
// See exceptions.c for address exception handling tests.
// The test program contains an infinite loop, but this is only executed
// once.
//
// (c) Reuben Thomas 1994-2020
//
// The package is distributed under the GNU Public License version 3, or,
// at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#include "tests.h"


unsigned correct[] = {
    CELL_W, 48, 48 + CELL_W,
    10000, 10000 + CELL_W, 10000 + CELL_W * 2, 10000 + CELL_W * 3,
    10000 + CELL_W * 4, 10000 + CELL_W * 5,
    11000, 11000 + CELL_W,
    64, 300, 64 + CELL_W,
};


int main(void)
{
    size_t size = 4096;
    init((CELL *)calloc(size, CELL_W), size);

    start_ass(EP);
    lit(48); ass(O_BRANCH);

    start_ass(48);
    lit(10000); ass(O_BRANCH);

    start_ass(10000);
    lit(1); lit(0); ass(O_QBRANCH);
    lit(0); lit(11000); ass(O_QBRANCH);

    start_ass(11000);
    lit(64);
    ass(O_EXECUTE);

    start_ass(64);
    call(300);

    start_ass(300);
    ass(O_EXIT);

    for (size_t i = 0; i < sizeof(correct) / sizeof(correct[0]); i++) {
        assert(single_step() == -257);
        printf("A = %s\n", disass(A, EP));
        printf("Instruction %zu: EP = %u; should be %u\n\n", i, EP, correct[i]);
        if (correct[i] != EP) {
            printf("Error in branch tests: EP = %"PRIu32"\n", EP);
            exit(1);
        }
    }

    printf("Branch tests ran OK\n");
    return 0;
}
