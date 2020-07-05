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

const char *correct[] = { str(SIZE) };


int main(void)
{
    WORD temp = 0;

    init_defaults((WORD *)malloc(SIZE), SIZE / WORD_BYTES);

    ass_goto(PC);
    ass(O_GET_MEMORY);

    for (size_t i = 0; i < sizeof(correct) / sizeof(correct[0]); i++) {
        assert(single_step() == ERROR_BREAK);
        assert(load_word(PC, &temp) == ERROR_OK);
        printf("Instruction = %s\n", disass(temp, PC));
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
