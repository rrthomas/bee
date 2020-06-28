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
    int i = init((CELL *)calloc(1024, 1), 256);
    if (i != 0) {
        printf("Error in run() tests: init with valid parameters failed\n");
        exit(1);
    }

    int ret_code = 37;
    lit(ret_code);
    ass(O_HALT);

    CELL ret = run();

    printf("Return value should be %d and is %"PRId32"\n", ret_code, ret);
    if (ret != ret_code) {
        printf("Error in run() tests: incorrect return value from run\n");
        exit(1);
    }

    UCELL final_EP = 2 * CELL_W;
    printf("EP should now be %"PRIu32"\n", final_EP);
    if (EP != final_EP) {
        printf("Error in run() tests: EP = %"PRIu32"\n", EP);
        exit(1);
    }

    printf("run() tests ran OK\n");
    return 0;
}
