// Test CATCH.
//
// (c) Reuben Thomas 2020
//
// The package is distributed under the GNU Public License version 3, or,
// at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#include "tests.h"



int main(void)
{
    const char *correct[64];
    unsigned steps = 0;

    init_defaults((WORD *)malloc(4096), 1024);

    ass_goto(M0);
    pushi(8);
    correct[steps++] = xasprintf("%d", 8);
    pushi(5);
    correct[steps++] = xasprintf("%d %d", 8, 5);
    pushreli(M0 + (0x200 / WORD_BYTES));
    correct[steps++] = xasprintf("%d %d %"PRIi32, 8, 5, m0 + 0x200);
    ass(O_CATCH);
    WORD *ret_addr = label();
    ass_goto(M0 + (0x200 / WORD_BYTES));

    correct[steps++] = xasprintf("%d %d", 8, 5);
    ass(O_DIVMOD);
    correct[steps++] = xasprintf("%d %d", 1, 3);
    ass(O_RET);
    correct[steps++] = xasprintf("%d %d %d", 1, 3, 0);
    ass_goto(ret_addr);

    pushreli(M0 + (0x400 / WORD_BYTES));
    correct[steps++] = xasprintf("%d %d %d %"PRIi32, 1, 3, 0, m0 + 0x400);
    ass(O_CATCH);
    correct[steps++] = xasprintf("%d %d %d", 1, 3, 0);
    ret_addr = label();
    ass_goto(M0 + (0x400 / WORD_BYTES));

    ass(O_UNDEFINED);
    ass_goto(ret_addr);
    correct[steps++] = xasprintf("%d %d %d %d", 1, 3, 0, -1);

    ass(O_POP);
    correct[steps++] = xasprintf("%d %d %d", 1, 3, 0);
    ass(O_POP);
    correct[steps++] = xasprintf("%d %d", 1, 3);
    ass(O_POP);
    correct[steps++] = xasprintf("%d", 1);
    ass(O_POP);
    correct[steps++] = xasprintf("%s", "");
    pushreli(M0 + (0x600 / WORD_BYTES));
    correct[steps++] = xasprintf("%"PRIi32, m0 + 0x600);
    ass(O_CATCH);
    correct[steps++] = xasprintf("%s", "");
    ret_addr = label();
    ass_goto(M0 + (0x600 / WORD_BYTES));

    pushi(ERROR_INVALID_OPCODE);
    correct[steps++] = xasprintf("%d", -1);
    ass(O_THROW);
    correct[steps++] = xasprintf("%d", -1);
    ass_goto(ret_addr);

    pushi(ERROR_OK);
    correct[steps++] = xasprintf("%d %d", -1, 0);
    ass(O_THROW);
    correct[steps++] = xasprintf("%d", -1);

    for (unsigned i = 0; i < steps; i++) {
        WORD temp = 0;
        assert(load_word(PC, &temp) == ERROR_OK);
        printf("Instruction = %s\n", disass(temp, PC));
        WORD ret = single_step();
        printf("single_step() returns %d (%s)\n", ret, error_to_msg(ret)); // Some instructions will error.
        show_data_stack();
        printf("Correct stack: %s\n\n", correct[i]);
        if (strcmp(correct[i], val_data_stack())) {
            printf("Error in catch tests: PC = %p\n", PC);
            exit(1);
        }
    }

    printf("Catch tests ran OK\n");
    return 0;
}
