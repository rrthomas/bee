// Test the literal instructions.
//
// (c) Reuben Thomas 1994-2020
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
    WORD temp = 0;
    int exception = 0;

    init((WORD *)calloc(1024, 1), 256);

    ass_goto(PC);
    push(ERROR_STEP); push(12345678);

    for (size_t i = 0; i < sizeof(correct) / sizeof(correct[0]); i++) {
        printf("Instruction = %s\n", disass(LOAD_WORD(PC), PC));
        assert(single_step() == ERROR_STEP);
        show_data_stack();
        printf("Correct stack: %s\n\n", correct[i]);
        if (strcmp(correct[i], val_data_stack())) {
            printf("Error in literals tests: PC = %"PRIu32"\n", PC);
            exit(1);
        }
    }

    printf("Literals tests ran OK\n");
    return 0;
}