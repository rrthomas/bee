// Test PUSHI.
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
    size_t size = 256;
    bee_init_defaults((bee_word_t *)calloc(size, BEE_WORD_BYTES), size);

    ass_goto(bee_m0);
    pushi(BEE_ERROR_BREAK); pushi(12345678);

    for (size_t i = 0; i < sizeof(correct) / sizeof(correct[0]); i++) {
        printf("Instruction = %s\n", disass(*bee_pc, bee_pc));
        assert(single_step() == BEE_ERROR_BREAK);
        show_data_stack();
        printf("Correct stack: %s\n\n", correct[i]);
        if (strcmp(correct[i], val_data_stack())) {
            printf("Error in constants tests: bee_pc = %p\n", bee_pc);
            exit(1);
        }
    }

    printf("Constants tests ran OK\n");
    return 0;
}
