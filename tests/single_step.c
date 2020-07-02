// Test that single_step works.
//
// (c) Reuben Thomas 1994-2020
//
// The package is distributed under the GNU Public License version 3, or,
// at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#include "tests.h"


int main(void)
{
    init((CELL *)calloc(1024, 1), 256);

    const UCELL steps = 10;
    for (UCELL i = 0; i < steps; i++) {
        // Assemble the test as we go!
        ass(O_WORD_BYTES);
        printf("EP = %"PRIu32"\n", EP);
        assert(single_step() == ERROR_STEP);
    }

    UCELL final_EP = steps * CELL_W;
    printf("EP should now be %"PRIu32"\n", final_EP);
    if (EP != final_EP) {
        printf("Error in single_step() tests: EP = %"PRIu32"\n", EP);
        exit(1);
    }

    printf("single_step() tests ran OK\n");
    return 0;
}
