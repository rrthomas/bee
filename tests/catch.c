// Test CATCH.
//
// (c) Reuben Thomas 2020
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
    setbuf(stdout, NULL);

    size_t size = 1024;
    bee_init_defaults((bee_word_t *)calloc(size, BUMBLE_WORD_BYTES), size);

    ass_goto(bee_m0);
    pushi(8);
    correct[steps++] = xasprintf("%d", 8);
    pushi(5);
    correct[steps++] = xasprintf("%d %d", 8, 5);
    pushreli(bee_m0 + 0x200 / BUMBLE_WORD_BYTES);
    correct[steps++] = xasprintf("%d %d %zd", 8, 5, (bee_word_t)(bee_m0 + 0x200 / BUMBLE_WORD_BYTES));
    ass(BUMBLE_INSN_CATCH);
    bee_word_t *ret_addr = label();
    ass_goto(bee_m0 + 0x200 / BUMBLE_WORD_BYTES);

    correct[steps++] = xasprintf("%d %d", 8, 5);
    ass(BUMBLE_INSN_RET);
    correct[steps++] = xasprintf("%d %d %d", 8, 5, 0);
    ass_goto(ret_addr);

    pushreli(bee_m0 + 0x400 / BUMBLE_WORD_BYTES);
    correct[steps++] = xasprintf("%d %d %d %zd", 8, 5, 0, (bee_word_t)(bee_m0 + 0x400 / BUMBLE_WORD_BYTES));
    ass(BUMBLE_INSN_CATCH);
    correct[steps++] = xasprintf("%d %d %d", 8, 5, 0);
    ret_addr = label();
    ass_goto(bee_m0 + 0x400 / BUMBLE_WORD_BYTES);

    ass(BUMBLE_INSN_UNDEFINED);
    ass_goto(ret_addr);
    correct[steps++] = xasprintf("%d %d %d %d", 8, 5, 0, -1);

    ass(BUMBLE_INSN_POP);
    correct[steps++] = xasprintf("%d %d %d", 8, 5, 0);
    ass(BUMBLE_INSN_POP);
    correct[steps++] = xasprintf("%d %d", 8, 5);
    ass(BUMBLE_INSN_POP);
    correct[steps++] = xasprintf("%d", 8);
    ass(BUMBLE_INSN_POP);
    correct[steps++] = xasprintf("%s", "");
    pushreli(bee_m0 + 0x600 / BUMBLE_WORD_BYTES);
    correct[steps++] = xasprintf("%zd", (bee_word_t)(bee_m0 + 0x600 / BUMBLE_WORD_BYTES));
    ass(BUMBLE_INSN_CATCH);
    correct[steps++] = xasprintf("%s", "");
    ret_addr = label();
    ass_goto(bee_m0 + 0x600 / BUMBLE_WORD_BYTES);

    pushi(BUMBLE_ERROR_INVALID_OPCODE);
    correct[steps++] = xasprintf("%d", -1);
    ass(BUMBLE_INSN_THROW);
    correct[steps++] = xasprintf("%d", -1);
    ass_goto(ret_addr);

    pushi(BUMBLE_ERROR_OK);
    correct[steps++] = xasprintf("%d %d", -1, 0);
    ass(BUMBLE_INSN_THROW);
    correct[steps++] = xasprintf("%d", -1);

    for (unsigned i = 0; i < steps; i++) {
        printf("Instruction = %s\n", disass(*bee_pc, bee_pc));
        bee_word_t ret = single_step();
        printf("single_step() returns %zd (%s)\n", ret, error_to_msg(ret)); // Some instructions will error.
        show_data_stack();
        printf("Correct stack: %s\n\n", correct[i]);
        if (strcmp(correct[i], val_data_stack())) {
            printf("Error in catch tests: pc = %p\n", bee_pc);
            exit(1);
        }
        free(correct[i]);
    }

    printf("Catch tests ran OK\n");
    return 0;
}
