// Test that single_step works, and that address alignment and bounds
// checking is properly performed on EP.
//
// (c) Reuben Thomas 1994-2018
//
// The package is distributed under the GNU Public License version 3, or,
// at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#include "tests.h"


int main(void)
{
    int exception = 0;

    init((CELL *)calloc(1024, 1), 256);

    assert(single_step() == -259);

    for (int i = 0; i < 10; i++) {
        printf("EP = %u\n", EP);
        single_step();
    }

    printf("EP should now be 44\n");
    if (EP != 44) {
        printf("Error in single_step() tests: EP = %"PRIu32"\n", EP);
        exit(1);
    }

    assert(exception == 0);
    printf("single_step() tests ran OK\n");
    return 0;
}
