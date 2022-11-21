// Test the stack instructions. Also uses the PUSH instruction.
//
// (c) Reuben Thomas 1994-2022
//
// The package is distributed under the GNU General Public License version 3,
// or, at your option, any later version.
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
    size_t size = 256;
    bee_word_t *m0 = (bee_word_t *)calloc(size, BEE_WORD_BYTES);
    bee_state *S = init_defaults(m0);

    S->d0[S->dp++] = 3; S->d0[S->dp++] =2; S->d0[S->dp++] = 1;	// initialise the stack

    ass_goto(m0);
    ass(BEE_INSN_DUP);
    pushi(1);
    ass(BEE_INSN_DUP);
    ass(BEE_INSN_POP);
    pushi(0);
    ass(BEE_INSN_SWAP);
    pushi(1);
    ass(BEE_INSN_SWAP);
    ass(BEE_INSN_PUSHS);
    ass(BEE_INSN_DUPS);
    ass(BEE_INSN_POPS);

    for (size_t i = 0; i < sizeof(correct) / sizeof(correct[0]) - 1; i++) {
        show_data_stack(S);
        printf("Correct stack: %s\n\n", correct[i]);
        if (strcmp(correct[i], val_data_stack(S))) {
            printf("Error in stack tests: pc = %p\n", S->pc);
            exit(1);
        }
        printf("Instruction = %s\n", disass(*S->pc, S->pc));
        assert(single_step(S) == BEE_ERROR_BREAK);
    }

    printf("Stack tests ran OK\n");
    return 0;
}
