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


const char *correct[] = { "-256", "-256 12345678" };


int main(void)
{
    WORD temp = 0;

    init_defaults((WORD *)calloc(1024, 1), 256);

    ass_goto(PC);
    push(ERROR_BREAK); push(12345678);

    for (size_t i = 0; i < sizeof(correct) / sizeof(correct[0]); i++) {
        assert(load_word(PC, &temp) == ERROR_OK);
        printf("Instruction = %s\n", disass(temp, PC));
        assert(single_step() == ERROR_BREAK);
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
