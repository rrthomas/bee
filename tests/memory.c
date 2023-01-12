// Test the memory instructions. Also uses previously tested instructions.
// See errors.c for address error handling tests.
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
    // Naturally bee_uword_t, but must be printed as bee_word_t for comparison with
    // output of val_data_stack(S).
    bee_word_t *MEND = m0 + size;
    bee_word_t *LAST_WORD = MEND - 1;
    bee_word_t MAGIC_NUMBER = 0xf201;
    int endism =
#ifdef WORDS_BIGENDIAN
        1
#else
        0
#endif
        ;

    pushi(MAGIC_NUMBER);
    correct[steps++] = xasprintf("%zd", MAGIC_NUMBER);
    pushreli(LAST_WORD);
    correct[steps++] = xasprintf("%zd %zd", MAGIC_NUMBER, (bee_word_t)LAST_WORD);
    ass(BEE_INSN_STORE);
    correct[steps++] = xasprintf("%s", "");
    pushreli(LAST_WORD);
    correct[steps++] = xasprintf("%zd", (bee_word_t)LAST_WORD);
    ass(BEE_INSN_LOAD);
    correct[steps++] = xasprintf("%zd", MAGIC_NUMBER);
    ass(BEE_INSN_POP);
    correct[steps++] = xasprintf("%s", "");
    push((bee_word_t)((uint8_t *)LAST_WORD + (BEE_WORD_BYTES - 2) * endism));
    correct[steps++] = xasprintf("%s", "");
    correct[steps++] = xasprintf("%zd", (bee_word_t)(label() - 3));
    correct[steps++] = xasprintf("%zd", (bee_word_t)((uint8_t *)LAST_WORD + (BEE_WORD_BYTES - 2) * endism));
    ass(BEE_INSN_LOAD2);
    correct[steps++] = xasprintf("%zd", MAGIC_NUMBER);
    ass(BEE_INSN_POP);
    correct[steps++] = xasprintf("%s", "");
    push((bee_word_t)((uint8_t *)LAST_WORD + (BEE_WORD_BYTES - 2) * endism + 1 * (1 - endism)));
    correct[steps++] = xasprintf("%s", "");
    correct[steps++] = xasprintf("%zd", (bee_word_t)(label() - 3));
    correct[steps++] = xasprintf("%zd", (bee_word_t)((uint8_t *)LAST_WORD + (BEE_WORD_BYTES - 2) * endism + 1 * (1 - endism)));
    ass(BEE_INSN_LOAD1);
    correct[steps++] = xasprintf("%zd", MAGIC_NUMBER >> CHAR_BIT);
    push((bee_word_t)((uint8_t *)MEND - ((BEE_WORD_BYTES - 1) * endism + 1)));
    correct[steps++] = xasprintf("%zd", MAGIC_NUMBER >> CHAR_BIT);
    correct[steps++] = xasprintf("%zd %zd", MAGIC_NUMBER >> CHAR_BIT, (bee_word_t)(label() - 3));
    correct[steps++] = xasprintf("%zd %zd", MAGIC_NUMBER >> CHAR_BIT, (bee_word_t)((uint8_t *)LAST_WORD + (BEE_WORD_BYTES - 1) * (1 - endism)));
    ass(BEE_INSN_STORE1);
    correct[steps++] = xasprintf("%s", "");
    pushreli(LAST_WORD);
    correct[steps++] = xasprintf("%zd", (bee_word_t)LAST_WORD);
    ass(BEE_INSN_LOAD);
    correct[steps++] = xasprintf("%zd", ((MAGIC_NUMBER >> CHAR_BIT) << (BEE_WORD_BIT - CHAR_BIT)) | MAGIC_NUMBER);
    ass(BEE_INSN_POP);
    correct[steps++] = xasprintf("%s", "");
    ass(BEE_INSN_GET_DP);
    correct[steps++] = xasprintf("%d", 0);
    ass(BEE_INSN_SET_DP);
    correct[steps++] = xasprintf("%s", "");
    ass(BEE_INSN_GET_SP);
    correct[steps++] = xasprintf("%d", 0);
    ass(BEE_INSN_POP);
    correct[steps++] = xasprintf("%s", "");
    pushi(0);
    correct[steps++] = xasprintf("%d", 0);
    ass(BEE_INSN_SET_SP);
    correct[steps++] = xasprintf("%s", "");
    ass(BEE_INSN_GET_SP);
    correct[steps++] = xasprintf("%d", 0);
    // This is redundant but clears the stack for the following tests.
    ass(BEE_INSN_SET_SP);
    correct[steps++] = xasprintf("%s", "");

    // Pre/post increment/decrement load/store instructions.
    pushi(MAGIC_NUMBER - 1);
    correct[steps++] = xasprintf("%zd", MAGIC_NUMBER - 1);
    pushreli(LAST_WORD);
    correct[steps++] = xasprintf("%zd %zd", MAGIC_NUMBER - 1, (bee_word_t)LAST_WORD);

    ass(BEE_INSN_STORE_DA);
    correct[steps++] = xasprintf("%zd", (bee_word_t)(LAST_WORD - 1));
    ass(BEE_INSN_LOAD_IB);
    correct[steps++] = xasprintf("%zd %zd", MAGIC_NUMBER - 1, (bee_word_t)LAST_WORD);

    ass(BEE_INSN_STORE_DB);
    correct[steps++] = xasprintf("%zd", (bee_word_t)(LAST_WORD - 1));
    ass(BEE_INSN_LOAD_IA);
    correct[steps++] = xasprintf("%zd %zd", MAGIC_NUMBER - 1, (bee_word_t)LAST_WORD);

    // Clear the stack and push new test data.
    ass(BEE_INSN_POP);
    correct[steps++] = xasprintf("%zd", MAGIC_NUMBER - 1);
    ass(BEE_INSN_POP);
    correct[steps++] = xasprintf("%s", "");
    pushi(MAGIC_NUMBER - 2);
    correct[steps++] = xasprintf("%zd", MAGIC_NUMBER - 2);
    pushreli(LAST_WORD - 4);
    correct[steps++] = xasprintf("%zd %zd", MAGIC_NUMBER - 2, (bee_word_t)(LAST_WORD - 4));

    ass(BEE_INSN_STORE_IB);
    correct[steps++] = xasprintf("%zd", (bee_word_t)(LAST_WORD - 3));
    ass(BEE_INSN_LOAD_DA);
    correct[steps++] = xasprintf("%zd %zd", MAGIC_NUMBER - 2, (bee_word_t)(LAST_WORD - 4));

    ass(BEE_INSN_STORE_IA);
    correct[steps++] = xasprintf("%zd", (bee_word_t)(LAST_WORD - 3));
    ass(BEE_INSN_LOAD_DB);
    correct[steps++] = xasprintf("%zd %zd", MAGIC_NUMBER - 2, (bee_word_t)(LAST_WORD - 4));

    return run_test("memory", S, false);
}
