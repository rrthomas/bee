// Test the branch instructions. Also uses other instructions with lower
// opcodes than the instructions tested (i.e. those already tested).
// See exceptions.c for address exception handling tests.
// The test program contains an infinite loop, but this is only executed
// once.
//
// (c) Reuben Thomas 1994-2020
//
// The package is distributed under the GNU Public License version 3, or,
// at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#include "tests.h"


unsigned correct[] = {
    CELL_W, 48, 48 + CELL_W,
    10000, 10000 + CELL_W, 10000 + CELL_W * 2, 10000 + CELL_W * 3,
    10000 + CELL_W * 4, 10000 + CELL_W * 5,
    11000, 11000 + CELL_W,
    64, 300, 64 + CELL_W,
};


int main(void)
{
    CELL temp = 0;
    int exception = 0;

    size_t size = 4096;
    init((CELL *)calloc(size, CELL_W), size);

    start_ass(PC);
    pushrel(48); ass(O_JUMP);

    start_ass(48);
    push(10000); ass(O_JUMP);

    start_ass(10000);
    push(1); push(0); ass(O_JUMPZ);
    push(0); push(11000); ass(O_JUMPZ);

    start_ass(11000);
    push(64);
    ass(O_CALL);

    start_ass(64);
    call(300);

    start_ass(300);
    ass(O_RET);

    for (size_t i = 0; i < sizeof(correct) / sizeof(correct[0]); i++) {
        printf("Instruction = %s\n", disass(LOAD_CELL(PC), PC));
        assert(single_step() == ERROR_STEP);
        printf("Instruction %zu: PC = %u; should be %u\n\n", i, PC, correct[i]);
        if (correct[i] != PC) {
            printf("Error in branch tests: PC = %"PRIu32"\n", PC);
            exit(1);
        }
    }

    printf("Branch tests ran OK\n");
    return 0;
}
