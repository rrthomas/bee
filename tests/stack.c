// Test the stack operators. Also uses the PUSH instruction.
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
    "3 2 1", "3 2 3", "3 2 3 1", "3 2 3 2", "3 2 3", "3 2 3 0", "3 3 2",
    "3 3 2 1", "2 3 3", "2 3", "2 3 3", "2 3 3 3",
};


int main(void)
{
    WORD temp = 0;
    int error = 0;

    init_defaults((WORD *)malloc(1024), 256);

    PUSH(3); PUSH(2); PUSH(1);	// initialise the stack

    ass_goto(PC);
    ass(O_DUP);
    push(1);
    ass(O_DUP);
    ass(O_POP);
    push(0);
    ass(O_SWAP);
    push(1);
    ass(O_SWAP);
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
        printf("Instruction = %s\n", disass(LOAD_WORD(PC), PC));
        assert(single_step() == ERROR_STEP);
    }

    assert(error == 0);
    printf("Stack tests ran OK\n");
    return 0;
}
