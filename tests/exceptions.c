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


UCELL test[] = { 0, 16, 32, 72, 92, 104, 112, 124, 136, };
CELL result[] = { -9, -9, 0, -23, -23, -9, -9, -23, -256, };


int main(void)
{
    size_t size = 4096;
    init((CELL *)calloc(size, CELL_W), size);

    start_ass(0);
    // test 1: PICK into non-existent memory
    ass(O_LITERAL); lit(0xfffffff0);
    ass(O_SPSTORE); ass(O_PICK);
    // test 2: set SP to MEMORY, then try to pop (>R) the stack
    ass(O_LITERAL); lit(MEMORY);
    ass(O_SPSTORE); ass(O_TOR);
    // test 3: test SP can point to just after a memory area
    ass(O_LITERAL); lit(MEMORY);
    ass(O_LITERAL); lit(-CELL_W);
    ass(O_PLUS);
    ass(O_SPSTORE); ass(O_TOR);
    ass(O_LITERAL); lit(0); ass(O_HALT);
    // test 4: test setting SP to unaligned address
    ass(O_CELL); ass(O_LITERAL); lit(1); ass(O_PLUS); ass(O_SPSTORE);
    // test 5: test EXECUTE of unaligned address
    ass(O_LITERAL); lit(1); ass(O_EXECUTE);
    // test 6: allow execution to run off the end of a memory area
    ass(O_BRANCH); lit(MEMORY - CELL_W);
    // test 7: fetch from an invalid address
    ass(O_LITERAL); lit(0xffffffec);
    ass(O_FETCH);
    // test 8: fetch from an unaligned address
    ass(O_LITERAL); lit(1); ass(O_FETCH);
    // test 9: test invalid opcode
    ass(O_UNDEFINED);

    UCELL error = 0;
    for (size_t i = 0; i < sizeof(test) / sizeof(test[0]); i++) {
        SP = S0;    // reset stack pointer

        printf("Test %zu\n", i + 1);

        if (i + 1 == 6) {
            // test 6: code to run at end of memory
            // Assemble now because it was overwritten by an earlier test
            start_ass(MEMORY - CELL_W);
            ass(O_CELL);
        }

        EP = test[i];
        CELL res = run();

        if (result[i] != res) {
             printf("Error in exceptions tests: test %zu failed; EP = %"PRIu32"\n", i + 1, EP);
             printf("Return code is %d; should be %d\n", res, result[i]);
             error++;
        }
        putchar('\n');
    }

    if (error == 0)
        printf("Exceptions tests ran OK\n");
    return error;
}
