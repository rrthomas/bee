// Test the VM-generated errors and HALT codes.
//
// (c) Reuben Thomas 1995-2020
//
// The package is distributed under the GNU General Public License version 3,
// or, at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#include "tests.h"


bee_word_t result[] = {
    BUMBLE_ERROR_STACK_OVERFLOW, BUMBLE_ERROR_STACK_OVERFLOW, 0,
    BUMBLE_ERROR_UNALIGNED_ADDRESS,
    BUMBLE_ERROR_INVALID_OPCODE,
};
bee_word_t *test[sizeof(result) / sizeof(result[0])];


int main(void)
{
    size_t size = 4096, tests = 0;
    bee_init_defaults((bee_word_t *)calloc(size, BUMBLE_WORD_BYTES), size);
    setbuf(stdout, NULL);

    ass_goto(bee_m0);

    // test 1: DUP with dp > dsize
    test[tests++] = label();
    pushi(bee_dsize + 1);
    ass(BUMBLE_INSN_SET_DP); ass(BUMBLE_INSN_DUP);
    // test 2: set dp to dsize + 1, then try to pop (PUSHR) the stack
    test[tests++] = label();
    pushi(bee_dsize + 1);
    ass(BUMBLE_INSN_SET_DP); ass(BUMBLE_INSN_PUSHS);
    // test 3: test dp can be dsize
    test[tests++] = label();
    pushi(bee_dsize);
    ass(BUMBLE_INSN_SET_DP); ass(BUMBLE_INSN_PUSHS);
    pushi(0); ass(BUMBLE_INSN_THROW);
    // test 4: test CALL of unaligned address
    test[tests++] = label();
    pushi(1); ass(BUMBLE_INSN_CALL);
    // test 5: test invalid opcode
    test[tests++] = label();
    ass(BUMBLE_INSN_UNDEFINED);

    bee_uword_t error = 0;
    for (size_t i = 0; i < sizeof(test) / sizeof(test[0]); i++) {
        bee_dp = 0;    // reset stack pointer

        printf("Test %zu\n", i + 1);

        if (i + 1 == 5) {
            // test 6: code to run at end of memory
            // Assemble now because it was overwritten by an earlier test
            ass_goto(bee_m0 + (bee_msize / BUMBLE_WORD_BYTES) - 1);
            ass(BUMBLE_INSN_NOP);
        }

        bee_pc = test[i];
        bee_word_t res = bee_run();

        if (result[i] != res) {
             printf("Error in errors tests: test %zu failed; pc = %p\n", i + 1, bee_pc);
             printf("Return code is %zd (%s); should be %zd (%s)\n",
                    res, error_to_msg(res), result[i], error_to_msg(result[i]));
             error++;
        }
        putchar('\n');
    }

    if (error == 0)
        printf("Errors tests ran OK\n");
    return error;
}
