// Test the comparison operators. We only test simple cases here, assuming
// that the C compiler's comparison routines will work for other cases.
//
// (c) Reuben Thomas 1994-2020
//
// The package is distributed under the GNU Public License version 3, or,
// at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#include "tests.h"


WORD correct[] = { 0, 1, 0, 1, 1, 0, 0, 1, 0, 0 };


static void stack1(void)
{
    DP = 0;	// empty the stack

    D0[DP++] = -4; D0[DP++] = 3;
    D0[DP++] = 2; D0[DP++] = 2;
    D0[DP++] = 1; D0[DP++] = 3;
    D0[DP++] = 3; D0[DP++] = 1;
}

static void stack2(void)
{
    DP = 0;	// empty the stack

    D0[DP++] = 1; D0[DP++] = -1;
    D0[DP++] = 237; D0[DP++] = 237;
}

static void step(unsigned start, unsigned end)
{
    if (end > start)
        for (unsigned i = start; i < end; i++) {
            printf("Instruction = %s\n", disass(*PC, PC));
            assert(single_step() == BEE_ERROR_BREAK);
            show_data_stack();
            printf("Result: %d; correct result: %d\n\n", D0[DP - 1], correct[i]);
            if (correct[i] != D0[DP - 1]) {
                printf("Error in comparison tests: PC = %p\n", PC);
                exit(1);
            }
            DP--;	// drop result of comparison
        }
}

int main(void)
{
    bee_init_defaults((WORD *)malloc(1024), 256);

    ass_goto(M0);
    ass(BEE_INSN_LT); ass(BEE_INSN_LT); ass(BEE_INSN_LT); ass(BEE_INSN_LT);
    ass(BEE_INSN_EQ); ass(BEE_INSN_EQ);
    ass(BEE_INSN_ULT); ass(BEE_INSN_ULT); ass(BEE_INSN_ULT); ass(BEE_INSN_ULT);

    stack1();       // set up the stack with four standard pairs to compare
    step(0, 4);     // do the < tests
    stack2();       // set up the stack with two standard pairs to compare
    step(4, 6);     // do the = tests
    stack1();       // set up the stack with four standard pairs to compare
    step(6, 10);    // do the U< tests

    printf("Comparison tests ran OK\n");
    return 0;
}
