// Test the VM-generated errors and HALT codes.
//
// (c) Reuben Thomas 1995-2020
//
// The package is distributed under the GNU Public License version 3, or,
// at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#include "tests.h"


WORD result[] = {
    ERROR_STACK_OVERFLOW, ERROR_STACK_OVERFLOW, 0,
    ERROR_UNALIGNED_ADDRESS,
    ERROR_INVALID_LOAD, ERROR_INVALID_LOAD, ERROR_UNALIGNED_ADDRESS,
    ERROR_INVALID_OPCODE,
};
WORD *test[sizeof(result) / sizeof(result[0])];


int main(void)
{
    size_t size = 4096;
    init_defaults((WORD *)calloc(size, WORD_BYTES), size);

    ass_goto(M0);

    // test 1: DUP with SP > SSIZE
    test[0] = label();
    push(SSIZE + 1);
    ass(O_SET_SP); ass(O_DUP);
    // test 2: set SP to SSIZE + 1, then try to pop (PUSHR) the stack
    test[1] = label();
    push(SSIZE + 1);
    ass(O_SET_SP); ass(O_PUSHR);
    // test 3: test SP can be SSIZE
    test[2] = label();
    push(SSIZE);
    ass(O_SET_SP); ass(O_PUSHR);
    push(0); ass(O_THROW);
    // test 4: test CALL of unaligned address
    test[3] = label();
    push(1); ass(O_CALL);
    // test 5: allow execution to run off the end of memory
    test[4] = label();
    push(MEMORY - WORD_BYTES); ass(O_JUMP);
    // test 6: load from an invalid address
    test[5] = label();
    push(0);
    ass(O_LOAD);
    // test 7: load from an unaligned address
    test[6] = label();
    push(1); ass(O_LOAD);
    // test 8: test invalid opcode
    test[7] = label();
    ass(O_UNDEFINED);

    UWORD error = 0;
    for (size_t i = 0; i < sizeof(test) / sizeof(test[0]); i++) {
        SP = 0;    // reset stack pointer

        printf("Test %zu\n", i + 1);

        if (i + 1 == 6) {
            // test 6: code to run at end of memory
            // Assemble now because it was overwritten by an earlier test
            ass_goto(M0 + (MEMORY / WORD_BYTES) - 1);
            ass(O_WORD_BYTES);
        }

        PC = test[i];
        WORD res = run();

        if (result[i] != res) {
             printf("Error in errors tests: test %zu failed; PC = %p\n", i + 1, PC);
             printf("Return code is %d (%s); should be %d (%s)\n",
                    res, error_to_msg(res), result[i], error_to_msg(result[i]));
             error++;
        }
        putchar('\n');
    }

    if (error == 0)
        printf("Errors tests ran OK\n");
    return error;
}
