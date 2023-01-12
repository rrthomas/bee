// Test the logic instructions. Also uses the SWAP instruction. We only test
// the stack handling and basic correctness of the instructions here,
// assuming that if the logic works in one case, it will work in all (if the
// C compiler doesn't implement it correctly, we're in trouble anyway!).
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
    bee_word_t BOTTOM_BYTE_SET = 0xffUL;
    bee_word_t SECOND_BYTE_SET = 0xffUL << CHAR_BIT;
    bee_word_t PENULTIMATE_BYTE_SET = 0xffUL << (BEE_WORD_BIT - 2 * CHAR_BIT);
    bee_word_t TOP_BYTE_SET = 0xffUL << (BEE_WORD_BIT - CHAR_BIT);

    pushi(CHAR_BIT);
    correct[steps++] = xasprintf("%d", CHAR_BIT);
    pushi(TOP_BYTE_SET);
    correct[steps++] = xasprintf("%d %zd", CHAR_BIT, TOP_BYTE_SET);
    pushi(BOTTOM_BYTE_SET);
    correct[steps++] = xasprintf("%d %zd %zd", CHAR_BIT, TOP_BYTE_SET, BOTTOM_BYTE_SET);
    pushi(CHAR_BIT);
    correct[steps++] = xasprintf("%d %zd %zd %d", CHAR_BIT, TOP_BYTE_SET, BOTTOM_BYTE_SET, CHAR_BIT);
    ass(BEE_INSN_LSHIFT);
    correct[steps++] = xasprintf("%d %zd %zd", CHAR_BIT, TOP_BYTE_SET, SECOND_BYTE_SET);
    pushi(1);
    correct[steps++] = xasprintf("%d %zd %zd %d", CHAR_BIT, TOP_BYTE_SET, SECOND_BYTE_SET, 1);
    ass(BEE_INSN_SWAP);
    correct[steps++] = xasprintf("%zd %zd %d", SECOND_BYTE_SET, TOP_BYTE_SET, CHAR_BIT);
    ass(BEE_INSN_RSHIFT);
    correct[steps++] = xasprintf("%zd %zd", SECOND_BYTE_SET, PENULTIMATE_BYTE_SET);
    ass(BEE_INSN_OR);
    correct[steps++] = xasprintf("%zd", SECOND_BYTE_SET | PENULTIMATE_BYTE_SET);
    ass(BEE_INSN_NOT);
    correct[steps++] = xasprintf("%zd", ~(SECOND_BYTE_SET | PENULTIMATE_BYTE_SET));
    pushi(1024);
    correct[steps++] = xasprintf("%zd %d", ~(SECOND_BYTE_SET | PENULTIMATE_BYTE_SET), 1024);
    pushi(-1);
    correct[steps++] = xasprintf("%zd %d %d", ~(SECOND_BYTE_SET | PENULTIMATE_BYTE_SET), 1024, -1);
    ass(BEE_INSN_XOR);
    correct[steps++] = xasprintf("%zd %d", ~(SECOND_BYTE_SET | PENULTIMATE_BYTE_SET), -1025);
    ass(BEE_INSN_AND);
    correct[steps++] = xasprintf("%zd", ~(SECOND_BYTE_SET | PENULTIMATE_BYTE_SET) & -1025);

    return run_test("logic", S, false);
}
