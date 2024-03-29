// Test traps. Also uses previously-tested instructions.
// FIXME: test file routines.
//
// (c) Reuben Thomas 1994-2023
//
// The package is distributed under the GNU General Public License version 3,
// or, at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#include "traps.h"

#include "tests.h"


bool test(bee_state *S)
{
    // Data for ARGC/ARGV tests
    int argc = 3;
    const char *argv[] = {"foo", "bard", "basilisk"};

    bee_register_args(argc, argv);

    // ARGC test
    pushi(TRAP_LIBC_ARGC); ass_trap(TRAP_LIBC);

    // ARGV test
    pushi(TRAP_LIBC_ARGV); ass_trap(TRAP_LIBC);
    ass(BEE_INSN_WORD_BYTES); ass(BEE_INSN_ADD); ass(BEE_INSN_LOAD);
    pushi(TRAP_LIBC_STRLEN); ass_trap(TRAP_LIBC);
    bee_word_t *end = label();

    assert(single_step(S) == BEE_ERROR_BREAK);
    assert(single_step(S) == BEE_ERROR_BREAK);
    printf("argc is %zd, and should be %d\n", S->d0[S->dp - 1], argc);
    assert(S->dp > 0);
    if (S->d0[--S->dp] != argc) {
        printf("Error in traps tests: pc = %p\n", S->pc);
        exit(1);
    }

    while (S->pc < end)
        assert(single_step(S) == BEE_ERROR_BREAK);
    printf("arg 1's length is %zd, and should be %zu\n", S->d0[S->dp - 1], strlen(argv[1]));
    assert(S->dp > 0);
    if ((bee_uword_t)S->d0[--S->dp] != strlen(argv[1])) {
        printf("Error in traps tests: pc = %p\n", S->pc);
        exit(1);
    }

    printf("traps tests ran OK\n");
    return true;
}
