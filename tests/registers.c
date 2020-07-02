// Test the register instructions, except for those operating on RP and SP
// (see memory.c).
//
// (c) Reuben Thomas 1994-2018
//
// The package is distributed under the GNU Public License version 3, or,
// at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#include "tests.h"


#define SIZE 1024

const char *correct[] = {
    str(SIZE), "",
};


int main(void)
{
    CELL temp = 0;
    int exception = 0;

    init((CELL *)malloc(SIZE), SIZE / CELL_W);

    start_ass(PC);
    ass(O_GET_MEMORY); ass(O_POP);

    for (size_t i = 0; i < sizeof(correct) / sizeof(correct[0]); i++) {
        assert(single_step() == ERROR_STEP);
        printf("Instruction = %s\n", disass(LOAD_CELL(PC), PC));
        show_data_stack();
        printf("Correct stack: %s\n\n", correct[i]);
        if (strcmp(correct[i], val_data_stack())) {
            printf("Error in registers tests: PC = %"PRIu32"\n", PC);
            exit(1);
        }
    }

    printf("Registers tests ran OK\n");
    return 0;
}
