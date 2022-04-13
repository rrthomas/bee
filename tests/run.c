// Test that bee_run() works, that the NOP instruction works, and that the
// return value of the THROW instruction is correctly returned.
//
// (c) Reuben Thomas 1995-2022
//
// The package is distributed under the GNU General Public License version 3,
// or, at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#include "tests.h"


int main(void)
{
    size_t size = 256;
    bee_word_t *m0 = (bee_word_t *)calloc(size, BEE_WORD_BYTES);
    int i = bee_init_defaults(m0);
    if (i != 0) {
        printf("Error in run() tests: init with valid parameters failed\n");
        exit(1);
    }

    ass_goto(m0);

    int ret_code = 37;
    pushi(ret_code);
    ass(BEE_INSN_NOP);
    ass(BEE_INSN_THROW);
    bee_word_t *final_PC = label();

    bee_word_t ret = bee_run();

    printf("Return value should be %d and is %zd\n", ret_code, ret);
    if (ret != ret_code) {
        printf("Error in run() tests: incorrect return value from run\n");
        exit(1);
    }

    printf("bee_R(pc) should now be %p\n", final_PC);
    if (bee_R(pc) != (bee_word_t *)final_PC) {
        printf("Error in bee_run() tests: pc = %p\n", bee_R(pc));
        exit(1);
    }

    printf("run() tests ran OK\n");
    return 0;
}
