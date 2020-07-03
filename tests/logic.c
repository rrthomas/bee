// Test the logic operators. Also uses the SWAP instruction. We only test
// the stack handling and basic correctness of the operators here, assuming
// that if the logic works in one case, it will work in all (if the C
// compiler doesn't implement it correctly, we're in trouble anyway!).
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
    "-16777216 8 65280",
    "-16777216 8 65280 1",
    "65280 8 -16777216",
    "65280 8 -16777216 0",
    "65280 -16777216 8",
    "65280 16711680",
    "16776960",
    "16776960 1",
    "33553920",
    "33553920 1",
    "16776960",
    "-16776961",
    "-16776961 1",
    "-16776961 1 -1",
    "-16776961 -2",
    "-16776962",
};


int main(void)
{
    WORD temp = 0;
    int error = 0;

    init_defaults((WORD *)malloc(1024), 256);

    PUSH(0xff000000); PUSH(8); PUSH(0xff); PUSH(8);

    ass_goto(PC);
    ass(O_LSHIFT);
    push(1); ass(O_SWAP);
    push(0); ass(O_SWAP);
    ass(O_RSHIFT);
    ass(O_OR);
    push(1);
    ass(O_LSHIFT);
    push(1);
    ass(O_RSHIFT);
    ass(O_NOT);
    push(1);
    push(-1);
    ass(O_XOR);
    ass(O_AND);

    for (size_t i = 0; i < sizeof(correct) / sizeof(correct[0]); i++) {
        printf("Instruction = %s\n", disass(LOAD_WORD(PC), PC));
        assert(single_step() == ERROR_STEP);
        show_data_stack();
        printf("Correct stack: %s\n\n", correct[i]);
        if (strcmp(correct[i], val_data_stack())) {
            printf("Error in logic tests: PC = %"PRIu32"\n", PC);
            exit(1);
        }
    }

    assert(error == 0);
    printf("Logic tests ran OK\n");
    return 0;
}
