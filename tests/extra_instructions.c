// Test extra instructions. Also uses previously-tested instructions.
// FIXME: test file routines.
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
    int exception = 0;
    CELL temp = 0;

    // Data for ARGC/ARGLEN/ARGCOPY tests
    int argc = 3;
    UCELL buf = 32;
    const char *argv[] = {"foo", "bard", "basilisk"};

    init((CELL *)malloc(4096), 1024);
    assert(register_args(argc, argv) == 0);

    start_ass(PC);
    ass(OX_ARGC);
    push(1); ass(OX_ARGLEN);
    push(1); push(buf); ass(OX_ARGCOPY);

    assert(single_step() == ERROR_STEP);
    printf("argc is %"PRId32", and should be %d\n\n", *stack_position(S0, SP, 0), argc);
    if (POP != argc) {
       printf("Error in extra instructions tests: PC = %"PRIu32"\n", PC);
        exit(1);
    }

    assert(single_step() == ERROR_STEP);
    assert(single_step() == ERROR_STEP);
    printf("arg 1's length is %"PRId32", and should be %zu\n", *stack_position(S0, SP, 0), strlen(argv[1]));
    if ((UCELL)POP != strlen(argv[1])) {
        printf("Error in extra instructions tests: PC = %"PRIu32"\n", PC);
        exit(1);
    }

    assert(single_step() == ERROR_STEP);
    assert(single_step() == ERROR_STEP);
    assert(single_step() == ERROR_STEP);
    const char *arg = (char *)native_address_of_range(buf, 0), *correct_arg = argv[1];
    printf("arg is %s, and should be %s\n", arg, correct_arg);
    if (strcmp(arg, correct_arg) != 0) {
        printf("Error in extra instructions tests: PC = %"PRIu32"\n", PC);
        exit(1);
    }

    printf("Extra instructions tests ran OK\n");
    return 0;
}
