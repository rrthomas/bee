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
    init_defaults((WORD *)malloc(1024), 256);

    D0[DP++] = 3; D0[DP++] =2; D0[DP++] = 1;	// initialise the stack

    ass_goto(M0);
    ass(BEE_INSN_DUP);
    pushi(1);
    ass(BEE_INSN_DUP);
    ass(BEE_INSN_POP);
    pushi(0);
    ass(BEE_INSN_SWAP);
    pushi(1);
    ass(BEE_INSN_SWAP);
    ass(BEE_INSN_PUSHR);
    ass(BEE_INSN_DUPR);
    ass(BEE_INSN_POPR);

    for (size_t i = 0; i < sizeof(correct) / sizeof(correct[0]) - 1; i++) {
        show_data_stack();
        printf("Correct stack: %s\n\n", correct[i]);
        if (strcmp(correct[i], val_data_stack())) {
            printf("Error in stack tests: PC = %p\n", PC);
            exit(1);
        }
        printf("Instruction = %s\n", disass(*PC, PC));
        assert(single_step() == ERROR_BREAK);
    }

    printf("Stack tests ran OK\n");
    return 0;
}
