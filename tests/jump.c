// Test the branch instructions.
// See errors.c for address error handling tests.
// The test program contains an infinite loop, but this is only executed
// once.
//
// (c) Reuben Thomas 1994-2020
//
// The package is distributed under the GNU Public License version 3, or,
// at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#include "tests.h"


int main(void)
{
    WORD *correct[64];
    unsigned steps = 0;

    size_t size = 4096;
    bee_init_defaults((WORD *)calloc(size, WORD_BYTES), size);

    ass_goto(M0);
    correct[steps++] = label();
    jumpi(M0 + 48 / WORD_BYTES);

    ass_goto(M0 + 48 / WORD_BYTES);
    correct[steps++] = label();
    pushreli(M0 + 10000 / WORD_BYTES);
    correct[steps++] = label();
    ass(BEE_INSN_JUMP);

    ass_goto(M0 + 10000 / WORD_BYTES);
    correct[steps++] = label();
    pushi(1);
    correct[steps++] = label();
    pushi(0);
    correct[steps++] = label();
    ass(BEE_INSN_JUMPZ);
    correct[steps++] = label();
    pushi(0);
    correct[steps++] = label();
    jumpzi(M0 + 11000 / WORD_BYTES);

    ass_goto(M0 + 11000 / WORD_BYTES);
    correct[steps++] = label();
    pushreli(M0 + 64 / WORD_BYTES);
    correct[steps++] = label();
    ass(BEE_INSN_CALL);

    ass_goto(M0 + 64 / WORD_BYTES);
    correct[steps++] = label();
    calli(M0 + 400 / WORD_BYTES);

    ass_goto(M0 + 400 / WORD_BYTES);
    correct[steps++] = label();
    ass(BEE_INSN_RET);

    for (unsigned i = 0; i < steps; i++) {
        printf("Instruction = %s\n", disass(*PC, PC));
        printf("Instruction %zu: PC = %p; should be %p\n\n", i, PC, correct[i]);
        if (correct[i] != PC) {
            printf("Error in branch tests: PC = %p\n", PC);
            exit(1);
        }
        assert(single_step() == ERROR_BREAK);
    }

    printf("Branch tests ran OK\n");
    return 0;
}
