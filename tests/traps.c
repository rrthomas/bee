// Test traps. Also uses previously-tested instructions.
// FIXME: test file routines.
//
// (c) Reuben Thomas 1994-2020
//
// The package is distributed under the GNU General Public License version 3,
// or, at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#include "traps.h"

#include "tests.h"


int main(void)
{
    // Data for ARGC/ARGV tests
    int argc = 3;
    const char *argv[] = {"foo", "bard", "basilisk"};

    size_t size = 1024;
    bee_init_defaults((bee_word_t *)calloc(size, BEE_WORD_BYTES), size);
    bee_register_args(argc, argv);

    ass_goto(bee_R(m0));

    // ARGC test
    pushi(TRAP_LIBC_ARGC); ass_trap(TRAP_LIBC);

    // ARGV test
    pushi(TRAP_LIBC_ARGV); ass_trap(TRAP_LIBC);
    ass(BEE_INSN_WORD_BYTES); ass(BEE_INSN_ADD); ass(BEE_INSN_LOAD);
    pushi(TRAP_LIBC_STRLEN); ass_trap(TRAP_LIBC);
    bee_word_t *end = label();

    assert(single_step() == BEE_ERROR_BREAK);
    assert(single_step() == BEE_ERROR_BREAK);
    printf("argc is %zd, and should be %d\n", bee_R(d0)[bee_R(dp) - 1], argc);
    assert(bee_R(dp) > 0);
    if (bee_R(d0)[--bee_R(dp)] != argc) {
        printf("Error in traps tests: pc = %p\n", bee_R(pc));
        exit(1);
    }

    while (bee_R(pc) < end)
        assert(single_step() == BEE_ERROR_BREAK);
    printf("arg 1's length is %zd, and should be %zu\n", bee_R(d0)[bee_R(dp) - 1], strlen(argv[1]));
    assert(bee_R(dp) > 0);
    if ((bee_uword_t)bee_R(d0)[--bee_R(dp)] != strlen(argv[1])) {
        printf("Error in traps tests: pc = %p\n", bee_R(pc));
        exit(1);
    }

    printf("Traps tests ran OK\n");
    return 0;
}
