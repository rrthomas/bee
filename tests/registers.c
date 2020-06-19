// Test the register instructions, except for those operating on RP and SP
// (see memory.c). Also uses NEXT.
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
    "768", "", str(SIZE), "", str(SIZE),
};


int main(void)
{
    init((CELL *)malloc(SIZE), SIZE / CELL_W);

    start_ass(EP);
    ass(O_S0FETCH); ass(O_DROP);
    ass(O_R0FETCH); ass(O_DROP);
    ass(O_MEMORYFETCH); ass(O_DROP);

    for (size_t i = 0; i < sizeof(correct) / sizeof(correct[0]); i++) {
        assert(single_step() == -257);
        printf("A = %s\n", disass(A));
        show_data_stack();
        printf("Correct stack: %s\n\n", correct[i]);
        if (strcmp(correct[i], val_data_stack())) {
            printf("Error in registers tests: EP = %"PRIu32"\n", EP);
            exit(1);
        }
    }

    printf("Registers tests ran OK\n");
    return 0;
}
