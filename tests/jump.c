// Test the branch instructions.
// See errors.c for address error handling tests.
// The test program contains an infinite loop, but this is only executed
// once.
//
// (c) Reuben Thomas 1994-2022
//
// The package is distributed under the GNU General Public License version 3,
// or, at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#include "tests.h"


int main(void)
{
    bee_word_t *correct[64];
    unsigned steps = 0;

    size_t size = 4096;
    bee_word_t *m0 = (bee_word_t *)calloc(size, BEE_WORD_BYTES);
    bee_state *S = init_defaults(m0);

    ass_goto(m0);
    correct[steps++] = label();
    jumpi(m0 + 48 / BEE_WORD_BYTES);

    ass_goto(m0 + 48 / BEE_WORD_BYTES);
    correct[steps++] = label();
    pushreli(m0 + 10000 / BEE_WORD_BYTES);
    correct[steps++] = label();
    ass(BEE_INSN_JUMP);

    ass_goto(m0 + 10000 / BEE_WORD_BYTES);
    correct[steps++] = label();
    pushi(1);
    correct[steps++] = label();
    pushi(0);
    correct[steps++] = label();
    ass(BEE_INSN_JUMPZ);
    correct[steps++] = label();
    pushi(0);
    correct[steps++] = label();
    jumpzi(m0 + 11000 / BEE_WORD_BYTES);

    ass_goto(m0 + 11000 / BEE_WORD_BYTES);
    correct[steps++] = label();
    pushreli(m0 + 64 / BEE_WORD_BYTES);
    correct[steps++] = label();
    ass(BEE_INSN_CALL);

    ass_goto(m0 + 64 / BEE_WORD_BYTES);
    correct[steps++] = label();
    calli(m0 + 400 / BEE_WORD_BYTES);

    ass_goto(m0 + 400 / BEE_WORD_BYTES);
    correct[steps++] = label();
    ass(BEE_INSN_RET);

    for (unsigned i = 0; i < steps; i++) {
        printf("Instruction = %s\n", disass(*S->pc, S->pc));
        printf("Instruction %u: pc = %p; should be %p\n\n", i, S->pc, correct[i]);
        if (correct[i] != S->pc) {
            printf("Error in branch tests: pc = %p\n", S->pc);
            exit(1);
        }
        assert(single_step(S) == BEE_ERROR_BREAK);
    }

    printf("Branch tests ran OK\n");
    return 0;
}
