// Test that bee_run() works, that the NOP instruction works, and that the
// return value of the THROW instruction is correctly returned.
//
// (c) Reuben Thomas 1995-2023
//
// The package is distributed under the GNU General Public License version 3,
// or, at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#include "tests.h"

bool test(bee_state *S)
{
    int ret_code = 37;
    pushi(ret_code);
    ass(BEE_INSN_NOP);
    ass(BEE_INSN_THROW);
    bee_word_t *final_pc = label();

    bee_word_t ret = bee_run(S);

    printf("Return value should be %d and is %zd\n", ret_code, ret);
    if (ret != ret_code) {
        printf("Error in run() tests: incorrect return value from run\n");
        return false;
    }

    printf("S->pc should now be %p\n", final_pc);
    if (S->pc != (bee_word_t *)final_pc) {
        printf("Error in bee_run() tests: pc = %p\n", S->pc);
        return false;
    }

    printf("run() tests ran OK\n");
    return true;
}
