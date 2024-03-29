// Test the VM-generated errors and error codes.
//
// (c) Reuben Thomas 1995-2023
//
// The package is distributed under the GNU General Public License version 3,
// or, at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#include "tests.h"


bee_word_t result[] = {
    BEE_ERROR_STACK_OVERFLOW, BEE_ERROR_STACK_OVERFLOW, 0,
    BEE_ERROR_UNALIGNED_ADDRESS,
    BEE_ERROR_UNALIGNED_ADDRESS,
    BEE_ERROR_INVALID_OPCODE,
};
bee_word_t *test_addr[sizeof(result) / sizeof(result[0])];


bool test(bee_state *S)
{
    size_t tests = 0;

    // test 1: DUP with dp > dsize
    test_addr[tests++] = label();
    pushi(S->dsize + 1);
    ass(BEE_INSN_SET_DP); ass(BEE_INSN_DUP);
    // test 2: set dp to dsize + 1, then try to pop (PUSHR) the stack
    test_addr[tests++] = label();
    pushi(S->dsize + 1);
    ass(BEE_INSN_SET_DP); ass(BEE_INSN_PUSHS);
    // test 3: test dp can be dsize
    test_addr[tests++] = label();
    pushi(S->dsize);
    ass(BEE_INSN_SET_DP); ass(BEE_INSN_PUSHS);
    pushi(0); ass(BEE_INSN_THROW);
    // test 4: test CALL of unaligned address
    test_addr[tests++] = label();
    pushi(1); ass(BEE_INSN_CALL);
    // test 5: load from an unaligned address
    test_addr[tests++] = label();
    pushi(1); ass(BEE_INSN_LOAD);
    // test 6: test invalid opcode
    test_addr[tests++] = label();
    ass(BEE_INSN_UNDEFINED);

    bee_uword_t error = 0;
    for (size_t i = 0; i < sizeof(test_addr) / sizeof(test_addr[0]); i++) {
        S->dp = 0;    // reset stack pointer

        printf("Test %zu\n", i + 1);

        S->pc = test_addr[i];
        bee_word_t res = bee_run(S);

        if (result[i] != res) {
            printf("Error in errors tests: test %zu failed; pc = %p\n", i + 1, S->pc);
            printf("Return code is %zd (%s); should be %zd (%s)\n",
                   res, error_to_msg(res), result[i], error_to_msg(result[i]));
            error++;
        }
        putchar('\n');
    }

    if (error == 0)
        printf("errors tests ran OK\n");
    return error == 0;
}
