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
    init((WORD *)calloc(1024, 1), 256);

    const UWORD steps = 10;
    for (UWORD i = 0; i < steps; i++) {
        // Assemble the test as we go!
        ass(O_WORD_BYTES);
        printf("PC = %"PRIu32"\n", PC);
        assert(single_step() == ERROR_STEP);
    }

    UWORD final_PC = steps * WORD_BYTES;
    printf("PC should now be %"PRIu32"\n", final_PC);
    if (PC != final_PC) {
        printf("Error in single_step() tests: PC = %"PRIu32"\n", PC);
        exit(1);
    }

    printf("single_step() tests ran OK\n");
    return 0;
}
