// Test the register instructions, except for those operating on sp and dp
// (see memory.c).
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

    size_t size = 1024;
    bee_word_t *m0 = (bee_word_t *)calloc(size, 1);
    bee_state *S = init_defaults(m0);

    ass_goto(m0);
    ass(BEE_INSN_GET_SSIZE);
    correct[steps++] = xasprintf("%zd", (bee_word_t)S->ssize);
    ass(BEE_INSN_POP);
    correct[steps++] = xasprintf("%s", "");
    ass(BEE_INSN_GET_DSIZE);
    correct[steps++] = xasprintf("%zd", (bee_word_t)S->dsize);
    ass(BEE_INSN_POP);
    correct[steps++] = xasprintf("%s", "");
    ass(BEE_INSN_GET_HANDLER_SP);
    correct[steps++] = xasprintf("%zd", (bee_word_t)S->handler_sp);
    ass(BEE_INSN_POP);
    correct[steps++] = xasprintf("%s", "");

    assert(run_test("registers", S, correct, steps, false));
    bee_destroy(S);
    free(m0);
    return 0;
}
