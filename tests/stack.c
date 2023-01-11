// Test the stack instructions. Also uses the PUSH instruction.
//
// (c) Reuben Thomas 1994-2023
//
// The package is distributed under the GNU General Public License version 3,
// or, at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#include "tests.h"


int main(void)
{
    char *correct[64];
    unsigned steps = 0;

    size_t size = 256;
    bee_word_t *m0 = (bee_word_t *)calloc(size, BEE_WORD_BYTES);
    bee_state *S = init_defaults(m0);

    S->d0[S->dp++] = 3; S->d0[S->dp++] = 2; S->d0[S->dp++] = 1;	// initialise the stack

    ass_goto(m0);
    ass(BEE_INSN_DUP);
    correct[steps++] = xasprintf("3 2 3");
    pushi(1);
    correct[steps++] = xasprintf("3 2 3 1");
    ass(BEE_INSN_DUP);
    correct[steps++] = xasprintf("3 2 3 2");
    ass(BEE_INSN_POP);
    correct[steps++] = xasprintf("3 2 3");
    pushi(0);
    correct[steps++] = xasprintf("3 2 3 0");
    ass(BEE_INSN_SWAP);
    correct[steps++] = xasprintf("3 3 2");
    pushi(1);
    correct[steps++] = xasprintf("3 3 2 1");
    ass(BEE_INSN_SWAP);
    correct[steps++] = xasprintf("2 3 3");
    ass(BEE_INSN_PUSHS);
    correct[steps++] = xasprintf("2 3");
    ass(BEE_INSN_DUPS);
    correct[steps++] = xasprintf("2 3 3");
    ass(BEE_INSN_POPS);
    correct[steps++] = xasprintf("2 3 3 3");

    assert(run_test("stack", S, correct, steps, false));
    bee_destroy(S);
    free(m0);
    return 0;
}
