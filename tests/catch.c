// Test CATCH.
//
// (c) Reuben Thomas 2022-2023
//
// The package is distributed under the GNU General Public License version 3,
// or, at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#include "tests.h"


bool test(bee_state *S _GL_UNUSED)
{
    pushi(8);
    correct[steps++] = xasprintf("%d", 8);
    pushi(5);
    correct[steps++] = xasprintf("%d %d", 8, 5);
    pushreli(m0 + 0x200 / BEE_WORD_BYTES);
    correct[steps++] = xasprintf("%d %d %zd", 8, 5, (bee_word_t)(m0 + 0x200 / BEE_WORD_BYTES));
    ass(BEE_INSN_CATCH);
    bee_word_t *ret_addr = label();
    ass_goto(m0 + 0x200 / BEE_WORD_BYTES);

    correct[steps++] = xasprintf("%d %d", 8, 5);
    ass(BEE_INSN_DIVMOD);
    correct[steps++] = xasprintf("%d %d", 1, 3);
    ass(BEE_INSN_RET);
    correct[steps++] = xasprintf("%d %d %d", 1, 3, 0);
    ass_goto(ret_addr);

    pushreli(m0 + 0x400 / BEE_WORD_BYTES);
    correct[steps++] = xasprintf("%d %d %d %zd", 1, 3, 0, (bee_word_t)(m0 + 0x400 / BEE_WORD_BYTES));
    ass(BEE_INSN_CATCH);
    correct[steps++] = xasprintf("%d %d %d", 1, 3, 0);
    ret_addr = label();
    ass_goto(m0 + 0x400 / BEE_WORD_BYTES);

    ass(BEE_INSN_UNDEFINED);
    ass_goto(ret_addr);
    correct[steps++] = xasprintf("%d %d %d %d", 1, 3, 0, -1);

    ass(BEE_INSN_POP);
    correct[steps++] = xasprintf("%d %d %d", 1, 3, 0);
    ass(BEE_INSN_POP);
    correct[steps++] = xasprintf("%d %d", 1, 3);
    ass(BEE_INSN_POP);
    correct[steps++] = xasprintf("%d", 1);
    ass(BEE_INSN_POP);
    correct[steps++] = xasprintf("%s", "");
    pushreli(m0 + 0x600 / BEE_WORD_BYTES);
    correct[steps++] = xasprintf("%zd", (bee_word_t)(m0 + 0x600 / BEE_WORD_BYTES));
    ass(BEE_INSN_CATCH);
    correct[steps++] = xasprintf("%s", "");
    ret_addr = label();
    ass_goto(m0 + 0x600 / BEE_WORD_BYTES);

    pushi(BEE_ERROR_INVALID_OPCODE);
    correct[steps++] = xasprintf("%d", -1);
    ass(BEE_INSN_THROW);
    correct[steps++] = xasprintf("%d", -1);
    ass_goto(ret_addr);

    pushi(BEE_ERROR_OK);
    correct[steps++] = xasprintf("%d %d", -1, 0);
    ass(BEE_INSN_THROW);
    correct[steps++] = xasprintf("%d", -1);

    return run_test("catch", S, true);
}
