// Test the comparison instructions. We only test simple cases here,
// assuming that the C compiler's comparison routines will work for other
// cases.
//
// (c) Reuben Thomas 1994-2023
//
// The package is distributed under the GNU General Public License version 3,
// or, at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#include "tests.h"


static void ass_comp_test(bee_word_t inst, bee_word_t left, bee_word_t right, bee_word_t res)
{
    pushi(left);
    correct[steps++] = xasprintf("%zd", left);
    pushi(right);
    correct[steps++] = xasprintf("%zd %zd", left, right);
    ass(inst);
    correct[steps++] = xasprintf("%zd", res);
    ass(BEE_INSN_POP);
    correct[steps++] = xasprintf("%s", "");
}

bool test(bee_state *S)
{
    ass_comp_test(BEE_INSN_LT, 3, 1, 0);
    ass_comp_test(BEE_INSN_LT, 1, 3, 1);
    ass_comp_test(BEE_INSN_LT, 2, 2, 0);
    ass_comp_test(BEE_INSN_LT, -4, 3, 1);
    ass_comp_test(BEE_INSN_EQ, 237, 237, 1);
    ass_comp_test(BEE_INSN_EQ, 1, -1, 0);
    ass_comp_test(BEE_INSN_ULT, 3, 1, 0);
    ass_comp_test(BEE_INSN_ULT, 1, 3, 1);
    ass_comp_test(BEE_INSN_ULT, 2, 2, 0);
    ass_comp_test(BEE_INSN_ULT, -4, 3, 0);

    return run_test("comparison", S, false);
}
