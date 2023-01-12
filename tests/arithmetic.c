// Test the arithmetic instructions. Also uses the SWAP, POP, and PUSH
// instructions. Since unsigned arithmetic overflow behaviour is guaranteed
// by the ISO C standard, we only test the stack handling and basic
// correctness of the instructions here, assuming that if the arithmetic
// works in one case, it will work in all.
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
    pushi(1);
    correct[steps++] = xasprintf("1");
    ass(BEE_INSN_NEG);
    correct[steps++] = xasprintf("-1");
    ass(BEE_INSN_WORD_BYTES);
    correct[steps++] = xasprintf("-1 %d", BEE_WORD_BYTES);
    pushi(-BEE_WORD_BYTES);
    correct[steps++] = xasprintf("-1 %d -%d", BEE_WORD_BYTES, BEE_WORD_BYTES);
    pushi(0);
    correct[steps++] = xasprintf("-1 %d -%d 0", BEE_WORD_BYTES, BEE_WORD_BYTES);
    ass(BEE_INSN_SWAP);
    correct[steps++] = xasprintf("-1 -%d %d", BEE_WORD_BYTES, BEE_WORD_BYTES);
    pushi(1);
    correct[steps++] = xasprintf("-1 -%d %d 1", BEE_WORD_BYTES, BEE_WORD_BYTES);
    ass(BEE_INSN_SWAP);
    correct[steps++] = xasprintf("%d -%d -1", BEE_WORD_BYTES, BEE_WORD_BYTES);
    ass(BEE_INSN_ADD);
    correct[steps++] = xasprintf("%d %d", BEE_WORD_BYTES, -BEE_WORD_BYTES - 1);
    ass(BEE_INSN_ADD);
    correct[steps++] = xasprintf("-1");
    ass(BEE_INSN_WORD_BYTES);
    correct[steps++] = xasprintf("-1 %d", BEE_WORD_BYTES);
    ass(BEE_INSN_MUL);
    correct[steps++] = xasprintf("-%d", BEE_WORD_BYTES);
    pushi(BEE_WORD_BYTES - 1);
    correct[steps++] = xasprintf("-%d %d", BEE_WORD_BYTES, BEE_WORD_BYTES - 1);
    ass(BEE_INSN_DIVMOD);
    correct[steps++] = xasprintf("-1 -1");
    ass(BEE_INSN_POP);
    correct[steps++] = xasprintf("-1");
    pushi(-2);
    correct[steps++] = xasprintf("-1 -2");
    ass(BEE_INSN_UDIVMOD);
    correct[steps++] = xasprintf("1 1");

    return run_test("arithmetic", S, false);
}
