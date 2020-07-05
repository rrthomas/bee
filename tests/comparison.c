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


int error = 0;
WORD temp;

WORD correct[] = { 0, 1, 0, 1, 1, 0, 0, 1, 0, 0 };


static void stack1(void)
{
    SP = 0;	// empty the stack

    S0[SP++] = -4; S0[SP++] = 3;
    S0[SP++] = 2; S0[SP++] = 2;
    S0[SP++] = 1; S0[SP++] = 3;
    S0[SP++] = 3; S0[SP++] = 1;
}

static void stack2(void)
{
    SP = 0;	// empty the stack

    S0[SP++] = 1; S0[SP++] = -1;
    S0[SP++] = 237; S0[SP++] = 237;
}

static void step(unsigned start, unsigned end)
{
    if (end > start)
        for (unsigned i = start; i < end; i++) {
            assert(load_word(PC, &temp) == ERROR_OK);
            printf("Instruction = %s\n", disass(temp, PC));
            assert(single_step() == ERROR_BREAK);
            show_data_stack();
            printf("Result: %d; correct result: %d\n\n", *stack_position(S0, SP, 0),
                   correct[i]);
            if (correct[i] != *stack_position(S0, SP, 0)) {
                printf("Error in comparison tests: PC = %"PRIu32"\n", PC);
                exit(1);
            }
            SP--;	// drop result of comparison
        }
}

int main(void)
{
    init_defaults((WORD *)malloc(1024), 256);

    ass_goto(PC);
    ass(O_LT); ass(O_LT); ass(O_LT); ass(O_LT);
    ass(O_EQ); ass(O_EQ);
    ass(O_ULT); ass(O_ULT); ass(O_ULT); ass(O_ULT);

    stack1();       // set up the stack with four standard pairs to compare
    step(0, 4);     // do the < tests
    stack2();       // set up the stack with two standard pairs to compare
    step(4, 6);     // do the = tests
    stack1();       // set up the stack with four standard pairs to compare
    step(6, 10);    // do the U< tests

    assert(error == 0);
    printf("Comparison tests ran OK\n");
    return 0;
}
