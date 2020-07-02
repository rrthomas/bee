// Test the VM-generated exceptions and HALT codes.
//
// (c) Reuben Thomas 1995-2020
//
// The package is distributed under the GNU Public License version 3, or,
// at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#include "tests.h"


CELL result[] = {
    ERROR_STACK_OVERFLOW, ERROR_STACK_OVERFLOW, 0,
    ERROR_UNALIGNED_ADDRESS,
    ERROR_INVALID_LOAD, ERROR_INVALID_LOAD, ERROR_UNALIGNED_ADDRESS,
    ERROR_INVALID_OPCODE,
};
UCELL test[sizeof(result) / sizeof(result[0])];


int main(void)
{
    size_t size = 4096;
    init((CELL *)calloc(size, CELL_W), size);

    start_ass(0);
    // test 1: DUP with SP > SSIZE
    test[0] = ass_current();
    lit(SSIZE + 1);
    ass(O_SET_SP); ass(O_DUP);
    // test 2: set SP to SSIZE + 1, then try to pop (PUSHR) the stack
    test[1] = ass_current();
    lit(SSIZE + 1);
    ass(O_SET_SP); ass(O_PUSHR);
    // test 3: test SP can be SSIZE
    test[2] = ass_current();
    lit(SSIZE);
    ass(O_SET_SP); ass(O_PUSHR);
    lit(0); ass(O_HALT);
    // test 4: test CALL of unaligned address
    test[3] = ass_current();
    lit(1); ass(O_CALL);
    // test 5: allow execution to run off the end of memory
    test[4] = ass_current();
    lit(MEMORY - CELL_W); ass(O_JUMP);
    // test 6: load from an invalid address
    test[5] = ass_current();
    lit(0xffffffec);
    ass(O_LOAD);
    // test 7: load from an unaligned address
    test[6] = ass_current();
    lit(1); ass(O_LOAD);
    // test 8: test invalid opcode
    test[7] = ass_current();
    ass(O_UNDEFINED);

    UCELL error = 0;
    for (size_t i = 0; i < sizeof(test) / sizeof(test[0]); i++) {
        SP = 0;    // reset stack pointer

        printf("Test %zu\n", i + 1);

        if (i + 1 == 6) {
            // test 6: code to run at end of memory
            // Assemble now because it was overwritten by an earlier test
            start_ass(MEMORY - CELL_W);
            ass(O_WORD_BYTES);
        }

        PC = test[i];
        CELL res = run();

        if (result[i] != res) {
             printf("Error in exceptions tests: test %zu failed; PC = %"PRIu32"\n", i + 1, PC);
             printf("Return code is %d (%s); should be %d (%s)\n",
                    res, error_to_msg(res), result[i], error_to_msg(result[i]));
             error++;
        }
        putchar('\n');
    }

    if (error == 0)
        printf("Exceptions tests ran OK\n");
    return error;
}
