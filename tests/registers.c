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


bool test(bee_state *S)
{
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

    return run_test("registers", S, false);
}
