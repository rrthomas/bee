// Test the memory operators. Also uses previously tested instructions.
// See exceptions.c for address exception handling tests.
//
// (c) Reuben Thomas 1994-2020
//
// The package is distributed under the GNU Public License version 3, or,
// at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#include "tests.h"


const char *correct[] = {
    "16384", "16384 " str(CELL_W), "16384 -" str(CELL_W), "16380",
    "16380 513", "16380 513 1", "16380 513 16380", "16380", "16380 0",
    "16380 16380", "16380 513", "16380", "16380 0",
    "16380 16380", "16380 1", "16381", "2", "2 16383", "", "16380", "33554945",
    "", "16128", "", "16384", "", "0", "", "0",
};


int main(void)
{
    size_t size = 4096;
    init((CELL *)calloc(size, CELL_W), size);

    start_ass(EP);
    ass(O_MEMORYFETCH);
    ass(O_CELL);
    ass(O_NEGATE);
    ass(O_PLUS);
    ass(O_LITERAL); lit(513);
    ass(O_LITERAL); lit(1);
    ass(O_PICK);
    ass(O_STORE);
    ass(O_LITERAL); lit(0);
    ass(O_PICK);
    ass(O_FETCH);
    ass(O_DROP);
    ass(O_LITERAL); lit(0);
    ass(O_PICK);
    ass(O_CFETCH);
    ass(O_PLUS);
    ass(O_CFETCH);
    ass(O_LITERAL); lit(16383);
    ass(O_CSTORE);
    ass(O_LITERAL); lit(16380);
    ass(O_FETCH);
    ass(O_DROP);
    ass(O_SPFETCH);
    ass(O_SPSTORE);
    ass(O_RPFETCH);
    ass(O_DROP);
    ass(O_LITERAL); lit(0);
    ass(O_RPSTORE);
    ass(O_RPFETCH);

    for (size_t i = 0; i < sizeof(correct) / sizeof(correct[0]); i++) {
        assert(single_step() == -257);
        printf("A = %s\n", disass(A));
        show_data_stack();
        printf("Correct stack: %s\n\n", correct[i]);
        if (strcmp(correct[i], val_data_stack())) {
            printf("Error in memory tests: EP = %"PRIu32"\n", EP);
            exit(1);
        }
    }

    printf("Memory tests ran OK\n");
    return 0;
}
