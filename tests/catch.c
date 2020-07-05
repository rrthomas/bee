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


const char *correct[] = {
    "8",
    "8 5",
    "8 5 512",
    "8 5",
    "1 3",
    "1 3 0",
    "1 3 0 1024",
    "1 3 0",
    "1 3 0 -1",
    "1 3 0",
    "1 3",
    "1",
    "",
    "1536",
    "",
    "-1",
    "-1",
    "-1 0",
};


int main(void)
{
    WORD temp = 0;
    int error = 0;

    init_defaults((WORD *)malloc(4096), 1024);

    push(8);
    push(5);
    pushrel(0x200);
    ass(O_CATCH);
    UWORD ret_addr = label();
    ass_goto(0x200);

    ass(O_DIVMOD);
    ass(O_RET);
    ass_goto(ret_addr);

    pushrel(0x400);
    ass(O_CATCH);
    ret_addr = label();
    ass_goto(0x400);

    ass(O_UNDEFINED);
    ass_goto(ret_addr);

    ass(O_POP);
    ass(O_POP);
    ass(O_POP);
    ass(O_POP);
    pushrel(0x600);
    ass(O_CATCH);
    ret_addr = label();
    ass_goto(0x600);

    push(ERROR_INVALID_OPCODE);
    ass(O_THROW);
    ass_goto(ret_addr);

    push(ERROR_OK);
    ass(O_THROW);

    for (size_t i = 0; i < sizeof(correct) / sizeof(correct[0]); i++) {
        assert(load_word(PC, &temp) == ERROR_OK);
        printf("Instruction = %s\n", disass(temp, PC));
        WORD ret = single_step();
        printf("single_step() returns %d (%s)\n", ret, error_to_msg(ret)); // Some instructions will error.
        show_data_stack();
        printf("Correct stack: %s\n\n", correct[i]);
        if (strcmp(correct[i], val_data_stack())) {
            printf("Error in logic tests: PC = %"PRIu32"\n", PC);
            exit(1);
        }
    }

    assert(error == 0);
    printf("Catch tests ran OK\n");
    return 0;
}
