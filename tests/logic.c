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
    init_defaults((WORD *)malloc(1024), 256);

    S0[SP++] = 0xff000000; S0[SP++] = 8; S0[SP++] = 0xff; S0[SP++] = 8;

    ass_goto(M0);
    ass(O_LSHIFT);
    pushi(1); ass(O_SWAP);
    pushi(0); ass(O_SWAP);
    ass(O_RSHIFT);
    ass(O_OR);
    pushi(1);
    ass(O_LSHIFT);
    pushi(1);
    ass(O_RSHIFT);
    ass(O_NOT);
    pushi(1);
    pushi(-1);
    ass(O_XOR);
    ass(O_AND);

    for (size_t i = 0; i < sizeof(correct) / sizeof(correct[0]); i++) {
        WORD temp = 0;
        assert(load_word(PC, &temp) == ERROR_OK);
        printf("Instruction = %s\n", disass(temp, PC));
        assert(single_step() == ERROR_BREAK);
        show_data_stack();
        printf("Correct stack: %s\n\n", correct[i]);
        if (strcmp(correct[i], val_data_stack())) {
            printf("Error in logic tests: PC = %p\n", PC);
            exit(1);
        }
    }

    printf("Logic tests ran OK\n");
    return 0;
}
