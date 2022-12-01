// Test PUSHI.
//
// (c) Reuben Thomas 1994-2022
//
// The package is distributed under the GNU General Public License version 3,
// or, at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#include "tests.h"


const char *correct[] = { "-256", "-256 12345678" };


int main(void)
{
    size_t size = 256;
    bee_word_t *m0 = (bee_word_t *)calloc(size, BEE_WORD_BYTES);
    bee_state *S = init_defaults(m0);

    ass_goto(m0);
    pushi(BEE_ERROR_BREAK); pushi(12345678);

    for (size_t i = 0; i < sizeof(correct) / sizeof(correct[0]); i++) {
        printf("Instruction = %s\n", disass(*S->pc, S->pc));
        assert(single_step(S) == BEE_ERROR_BREAK);
        show_data_stack(S);
        printf("Correct stack: %s\n\n", correct[i]);
        if (strcmp(correct[i], val_data_stack(S))) {
            printf("Error in constants tests: pc = %p\n", S->pc);
            exit(1);
        }
    }

    printf("Constants tests ran OK\n");
    bee_destroy(S);
    free(m0);
    return 0;
}
