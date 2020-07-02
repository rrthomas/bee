// Test the literal instructions.
//
// (c) Reuben Thomas 1994-2018
//
// The package is distributed under the GNU Public License version 3, or,
// at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#include "tests.h"


const char *correct[] = { "-257", "-257 12345678", "-257 12345678" };


int main(void)
{
    init((CELL *)calloc(1024, 1), 256);

    start_ass(EP);
    lit(ERROR_STEP); lit(12345678);

    for (size_t i = 0; i < sizeof(correct) / sizeof(correct[0]); i++) {
        assert(single_step() == ERROR_STEP);
        printf("A = %s\n", disass(A, EP));
        show_data_stack();
        printf("Correct stack: %s\n\n", correct[i]);
        if (strcmp(correct[i], val_data_stack())) {
            printf("Error in literals tests: EP = %"PRIu32"\n", EP);
            exit(1);
        }
    }

    printf("Literals tests ran OK\n");
    return 0;
}
