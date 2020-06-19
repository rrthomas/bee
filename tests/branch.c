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
    8, 48, 56, 10000, 10008, 10016, 10020, 10028, 10036, 11000, 11008,
    64, 300, 68,
};


int main(void)
{
    size_t size = 4096;
    init((CELL *)calloc(size, CELL_W), size);

    start_ass(EP);
    ass(O_LITERAL); lit(48); ass(O_BRANCH);

    start_ass(48);
    ass(O_LITERAL); lit(10000); ass(O_BRANCH);

    start_ass(10000);
    ass(O_LITERAL); lit(1); ass(O_LITERAL); lit(0); ass(O_QBRANCH);
    ass(O_LITERAL); lit(0); ass(O_LITERAL); lit(11000); ass(O_QBRANCH);

    start_ass(11000);
    ass(O_LITERAL); lit(64);
    ass(O_EXECUTE);

    start_ass(64);
    offset(300);

    start_ass(300);
    ass(O_EXIT);

    for (size_t i = 0; i < sizeof(correct) / sizeof(correct[0]); i++) {
        assert(single_step() == -257);
        printf("A = %s\n", disass(A));
        printf("Instruction %zu: EP = %u; should be %u\n\n", i, EP, correct[i]);
        if (correct[i] != EP) {
            printf("Error in branch tests: EP = %"PRIu32"\n", EP);
            exit(1);
        }
    }

    printf("Branch tests ran OK\n");
    return 0;
}
