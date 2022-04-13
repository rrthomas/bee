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


static void stack1(void)
{
    bee_R(dp) = 0;	// empty the stack

    bee_R(d0)[bee_R(dp)++] = -4; bee_R(d0)[bee_R(dp)++] = 3;
    bee_R(d0)[bee_R(dp)++] = 2; bee_R(d0)[bee_R(dp)++] = 2;
    bee_R(d0)[bee_R(dp)++] = 1; bee_R(d0)[bee_R(dp)++] = 3;
    bee_R(d0)[bee_R(dp)++] = 3; bee_R(d0)[bee_R(dp)++] = 1;
}

static void stack2(void)
{
    bee_R(dp) = 0;	// empty the stack

    bee_R(d0)[bee_R(dp)++] = 1; bee_R(d0)[bee_R(dp)++] = -1;
    bee_R(d0)[bee_R(dp)++] = 237; bee_R(d0)[bee_R(dp)++] = 237;
}

static void step(unsigned start, unsigned end)
{
    if (end > start)
        for (unsigned i = start; i < end; i++) {
            printf("Instruction = %s\n", disass(*bee_R(pc), bee_R(pc)));
            assert(single_step() == BEE_ERROR_BREAK);
            show_data_stack();
            printf("Result: %zd; correct result: %zd\n\n", bee_R(d0)[bee_R(dp) - 1], correct[i]);
            if (correct[i] != bee_R(d0)[bee_R(dp) - 1]) {
                printf("Error in comparison tests: pc = %p\n", bee_R(pc));
                exit(1);
            }
            bee_R(dp)--;	// drop result of comparison
        }
}

int main(void)
{
    size_t size = 256;
    bee_word_t *m0 = (bee_word_t *)calloc(size, BEE_WORD_BYTES);
    bee_init_defaults(m0);

    ass_goto(m0);
    ass(BEE_INSN_LT); ass(BEE_INSN_LT); ass(BEE_INSN_LT); ass(BEE_INSN_LT);
    ass(BEE_INSN_EQ); ass(BEE_INSN_EQ);
    ass(BEE_INSN_ULT); ass(BEE_INSN_ULT); ass(BEE_INSN_ULT); ass(BEE_INSN_ULT);

    stack1();       // set up the stack with four standard pairs to compare
    step(0, 4);     // do the LT tests
    stack2();       // set up the stack with two standard pairs to compare
    step(4, 6);     // do the EQ tests
    stack1();       // set up the stack with four standard pairs to compare
    step(6, 10);    // do the ULT tests

    printf("Comparison tests ran OK\n");
    return 0;
}
