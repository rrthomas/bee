// Test the memory operators. Also uses previously tested instructions.
// See errors.c for address error handling tests.
//
// (c) Reuben Thomas 1994-2020
//
// The package is distributed under the GNU Public License version 3, or,
// at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#include "tests.h"


int main(void)
{
    char *correct[64];
    unsigned steps = 0;

    size_t size = 4096;
    bee_init_defaults((bee_word_t *)calloc(size, BEE_WORD_BYTES), size);

    // Naturally bee_uword_t, but must be printed as bee_word_t for comparison with
    // output of val_data_stack().
    bee_word_t *MEND = (bee_word_t *)((uint8_t *)bee_m0 + bee_msize);
    bee_word_t *LAST_WORD = MEND - 1;
    bee_word_t MAGIC_NUMBER = 0xf201;
    int endism =
#ifdef WORDS_BIGENDIAN
        1
#else
        0
#endif
        ;

    ass_goto(bee_m0);

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
    correct[steps++] = xasprintf("%zd", MAGIC_NUMBER >> 8);
    push((bee_word_t)((uint8_t *)MEND - ((BEE_WORD_BYTES - 1) * endism + 1)));
    correct[steps++] = xasprintf("%zd", MAGIC_NUMBER >> 8);
    correct[steps++] = xasprintf("%zd %zd", MAGIC_NUMBER >> 8, (bee_word_t)(label() - 3));
    correct[steps++] = xasprintf("%zd %zd", MAGIC_NUMBER >> 8, (bee_word_t)((uint8_t *)LAST_WORD + (BEE_WORD_BYTES - 1) * (1 - endism)));
    ass(BEE_INSN_STORE1);
    correct[steps++] = xasprintf("%s", "");
    pushreli(LAST_WORD);
    correct[steps++] = xasprintf("%zd", (bee_word_t)LAST_WORD);
    ass(BEE_INSN_LOAD);
    correct[steps++] = xasprintf("%zd", ((MAGIC_NUMBER >> 8) << (BEE_WORD_BIT - 8)) | MAGIC_NUMBER);
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

    for (size_t i = 0; i < steps; i++) {
        printf("Instruction = %s\n", disass(*bee_pc, bee_pc));
        assert(single_step() == BEE_ERROR_BREAK);
        show_data_stack();
        printf("Correct stack: %s\n\n", correct[i]);
        if (strcmp(correct[i], val_data_stack())) {
            printf("Error in memory tests: pc = %p\n", bee_pc);
            exit(1);
        }
        free(correct[i]);
    }

    printf("Memory tests ran OK\n");
    return 0;
}
