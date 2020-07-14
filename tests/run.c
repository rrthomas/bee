// Test that run works, and that the return value of the HALT instruction is
// correctly returned.
//
// (c) Reuben Thomas 1995-2020
//
// The package is distributed under the GNU Public License version 3, or,
// at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#include "tests.h"


int main(void)
{
    int i = init_defaults((WORD *)calloc(1024, 1), 256);
    if (i != 0) {
        printf("Error in run() tests: init with valid parameters failed\n");
        exit(1);
    }

    ass_goto(M0);

    int ret_code = 37;
    pushi(ret_code);
    ass(O_THROW);
    WORD *final_PC = label();

    WORD ret = run();

    printf("Return value should be %d and is %"PRId32"\n", ret_code, ret);
    if (ret != ret_code) {
        printf("Error in run() tests: incorrect return value from run\n");
        exit(1);
    }

    printf("PC should now be %p\n", final_PC);
    if (PC != (WORD *)final_PC) {
        printf("Error in run() tests: PC = %p\n", PC);
        exit(1);
    }

    printf("run() tests ran OK\n");
    return 0;
}
