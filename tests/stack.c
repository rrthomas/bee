// Test the stack operators. Also uses the LITERAL instruction.
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
    "3 2 1", "3 2 3", "3 2 3 1", "3 2 3 2", "3 2 3", "3 2 3 1", "3 3 2",
    "3 3 2 2", "3 2 3", "3 2", "3 2 3", "3 2 3 3",
};


int main(void)
{
    CELL temp = 0;
    int exception = 0;

    init((CELL *)malloc(1024), 256);

    PUSH(3); PUSH(2); PUSH(1);	// initialise the stack

    start_ass(PC);
    ass(O_DUP);
    lit(1);
    ass(O_DUP);
    ass(O_POP);
    lit(1);
    ass(O_ROLL);
    lit(2);
    ass(O_ROLL);
    ass(O_PUSHR);
    ass(O_DUPR);
    ass(O_POPR);

    for (size_t i = 0; i < sizeof(correct) / sizeof(correct[0]); i++) {
        show_data_stack();
        printf("Correct stack: %s\n\n", correct[i]);
        if (strcmp(correct[i], val_data_stack())) {
            printf("Error in stack tests: PC = %"PRIu32"\n", PC);
            exit(1);
        }
        printf("Instruction = %s\n", disass(LOAD_CELL(PC), PC));
        assert(single_step() == ERROR_STEP);
    }

    assert(exception == 0);
    printf("Stack tests ran OK\n");
    return 0;
}
