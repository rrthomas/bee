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
    WORD_BYTES, 48, 48 + WORD_BYTES,
    10000, 10000 + WORD_BYTES, 10000 + WORD_BYTES * 2, 10000 + WORD_BYTES * 3,
    10000 + WORD_BYTES * 4, 10000 + WORD_BYTES * 5,
    11000, 11000 + WORD_BYTES,
    64, 300, 64 + WORD_BYTES,
};


int main(void)
{
    WORD temp = 0;
    int exception = 0;

    size_t size = 4096;
    init_defaults((WORD *)calloc(size, WORD_BYTES), size);

    ass_goto(PC);
    pushrel(48); ass(O_JUMP);

    ass_goto(48);
    push(10000); ass(O_JUMP);

    ass_goto(10000);
    push(1); push(0); ass(O_JUMPZ);
    push(0); push(11000); ass(O_JUMPZ);

    ass_goto(11000);
    push(64);
    ass(O_CALL);

    ass_goto(64);
    call(300);

    ass_goto(300);
    ass(O_RET);

    for (size_t i = 0; i < sizeof(correct) / sizeof(correct[0]); i++) {
        printf("Instruction = %s\n", disass(LOAD_WORD(PC), PC));
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
