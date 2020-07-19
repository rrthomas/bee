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
    bee_WORD *correct[64];
    unsigned steps = 0;

    size_t size = 4096;
    bee_init_defaults((bee_WORD *)calloc(size, bee_WORD_BYTES), size);

    ass_goto(bee_m0);
    correct[steps++] = label();
    jumpi(bee_m0 + 48 / bee_WORD_BYTES);

    ass_goto(bee_m0 + 48 / bee_WORD_BYTES);
    correct[steps++] = label();
    pushreli(bee_m0 + 10000 / bee_WORD_BYTES);
    correct[steps++] = label();
    ass(BEE_INSN_JUMP);

    ass_goto(bee_m0 + 10000 / bee_WORD_BYTES);
    correct[steps++] = label();
    pushi(1);
    correct[steps++] = label();
    pushi(0);
    correct[steps++] = label();
    ass(BEE_INSN_JUMPZ);
    correct[steps++] = label();
    pushi(0);
    correct[steps++] = label();
    jumpzi(bee_m0 + 11000 / bee_WORD_BYTES);

    ass_goto(bee_m0 + 11000 / bee_WORD_BYTES);
    correct[steps++] = label();
    pushreli(bee_m0 + 64 / bee_WORD_BYTES);
    correct[steps++] = label();
    ass(BEE_INSN_CALL);

    ass_goto(bee_m0 + 64 / bee_WORD_BYTES);
    correct[steps++] = label();
    calli(bee_m0 + 400 / bee_WORD_BYTES);

    ass_goto(bee_m0 + 400 / bee_WORD_BYTES);
    correct[steps++] = label();
    ass(BEE_INSN_RET);

    for (unsigned i = 0; i < steps; i++) {
        printf("Instruction = %s\n", disass(*bee_pc, bee_pc));
        printf("Instruction %zu: bee_pc = %p; should be %p\n\n", i, bee_pc, correct[i]);
        if (correct[i] != bee_pc) {
            printf("Error in branch tests: bee_pc = %p\n", bee_pc);
            exit(1);
        }
        assert(single_step() == BEE_ERROR_BREAK);
    }

    printf("Branch tests ran OK\n");
    return 0;
}
