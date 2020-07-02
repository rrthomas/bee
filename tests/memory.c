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
    "", "0", "", "0", "", "0", "", "0",
};


int main(void)
{
    CELL temp = 0;
    int exception = 0;

    size_t size = 4096;
    init((CELL *)calloc(size, CELL_W), size);

    start_ass(PC);
    ass(O_GET_MEMORY);
    ass(O_WORD_BYTES);
    ass(O_NEGATE);
    ass(O_ADD);
    lit(513);
    lit(1);
    ass(O_DUP);
    ass(O_STORE);
    lit(0);
    ass(O_DUP);
    ass(O_LOAD);
    ass(O_POP);
    lit(0);
    ass(O_DUP);
    ass(O_LOAD1);
    ass(O_ADD);
    ass(O_LOAD1);
    lit(16383);
    ass(O_STORE1);
    lit(16380);
    ass(O_LOAD);
    ass(O_POP);
    ass(O_GET_SP);
    ass(O_SET_SP);
    ass(O_GET_RP);
    ass(O_POP);
    lit(0);
    ass(O_SET_RP);
    ass(O_GET_RP);

    for (size_t i = 0; i < sizeof(correct) / sizeof(correct[0]); i++) {
        printf("Instruction = %s\n", disass(LOAD_CELL(PC), PC));
        assert(single_step() == ERROR_STEP);
        show_data_stack();
        printf("Correct stack: %s\n\n", correct[i]);
        if (strcmp(correct[i], val_data_stack())) {
            printf("Error in memory tests: PC = %"PRIu32"\n", PC);
            exit(1);
        }
    }

    printf("Memory tests ran OK\n");
    return 0;
}
