// Test the comparison operators. We only test simple cases here, assuming
// that the C compiler's comparison routines will work for other cases.
//
// (c) Reuben Thomas 1994-2020
//
// The package is distributed under the GNU General Public License version 3,
// or, at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#include "tests.h"


bee_word_t correct[] = { 0, 1, 0, 1, 1, 0, 0, 1, 0, 0 };


static void stack1(bee_state *S)
{
    S->dp = 0;	// empty the stack

    S->d0[S->dp++] = -4; S->d0[S->dp++] = 3;
    S->d0[S->dp++] = 2; S->d0[S->dp++] = 2;
    S->d0[S->dp++] = 1; S->d0[S->dp++] = 3;
    S->d0[S->dp++] = 3; S->d0[S->dp++] = 1;
}

static void stack2(bee_state *S)
{
    S->dp = 0;	// empty the stack

    S->d0[S->dp++] = 1; S->d0[S->dp++] = -1;
    S->d0[S->dp++] = 237; S->d0[S->dp++] = 237;
}

static void step(bee_state *S, unsigned start, unsigned end)
{
    if (end > start)
        for (unsigned i = start; i < end; i++) {
            printf("Instruction = %s\n", disass(*S->pc, S->pc));
            assert(single_step(S) == BEE_ERROR_BREAK);
            show_data_stack(S);
            printf("Result: %zd; correct result: %zd\n\n", S->d0[S->dp - 1], correct[i]);
            if (correct[i] != S->d0[S->dp - 1]) {
                printf("Error in comparison tests: pc = %p\n", S->pc);
                exit(1);
            }
            S->dp--;	// drop result of comparison
        }
}

int main(void)
{
    size_t size = 256;
    bee_word_t *m0 = (bee_word_t *)calloc(size, BEE_WORD_BYTES);
    bee_state *S = bee_init_defaults(m0);

    ass_goto(m0);
    ass(BEE_INSN_LT); ass(BEE_INSN_LT); ass(BEE_INSN_LT); ass(BEE_INSN_LT);
    ass(BEE_INSN_EQ); ass(BEE_INSN_EQ);
    ass(BEE_INSN_ULT); ass(BEE_INSN_ULT); ass(BEE_INSN_ULT); ass(BEE_INSN_ULT);

    stack1(S);      // set up the stack with four standard pairs to compare
    step(S, 0, 4);  // do the LT tests
    stack2(S);      // set up the stack with two standard pairs to compare
    step(S, 4, 6);  // do the EQ tests
    stack1(S);      // set up the stack with four standard pairs to compare
    step(S, 6, 10); // do the ULT tests

    printf("Comparison tests ran OK\n");
    return 0;
}
