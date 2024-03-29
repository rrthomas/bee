// Private implementation-specific APIs that are shared between modules.
//
// (c) Reuben Thomas 1994-2022
//
// The package is distributed under the GNU General Public License version 3,
// or, at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

// Errors
#define THROW(code)                             \
    do {                                        \
        error = code;                           \
        goto error;                             \
    } while (0)

#define THROW_IF_ERROR(code)                    \
    do {                                        \
        bee_word_t _err = code;                 \
        if (_err != BEE_ERROR_OK)               \
            THROW(_err);                        \
    } while (0)


// Stack access
#define CHECKS(pops, pushes)                                            \
    THROW_IF_ERROR(bee_check_stack(S->ssize, S->sp, pops, pushes))
#define POPS(ptr)                                                       \
    THROW_IF_ERROR(bee_pop_stack(S->s0, S->ssize, &S->sp, ptr))
#define PUSHS(val)                                                      \
    THROW_IF_ERROR(bee_push_stack(S->s0, S->ssize, &S->sp, val))

#define CHECKD(pops, pushes)                                            \
    THROW_IF_ERROR(bee_check_stack(S->dsize, S->dp, pops, pushes))
#define POPD(ptr)                                                       \
    THROW_IF_ERROR(bee_pop_stack(S->d0, S->dsize, &S->dp, ptr))
#define PUSHD(val)                                                      \
    THROW_IF_ERROR(bee_push_stack(S->d0, S->dsize, &S->dp, val))


// Memory access

// Align a VM address
#define ALIGN(a)                                                \
    (((bee_uword_t)(a) + BEE_WORD_BYTES - 1) & (-BEE_WORD_BYTES))

// Check whether a VM address is aligned
#define IS_ALIGNED(a)                                           \
    (((uint8_t *)((bee_uword_t)(a) & (BEE_WORD_BYTES - 1)) == 0))


// Portable left shift (the behaviour of << with overflow (including on any
// negative number) is undefined)
#define LSHIFT(n, p)                            \
    (((n) & (BEE_UWORD_MAX >> (p))) << (p))


// Portable arithmetic right shift (the behaviour of >> on signed
// quantities is implementation-defined in C99)
#if HAVE_ARITHMETIC_RSHIFT
#define ARSHIFT(n, p)                           \
    ((bee_word_t)(n) >> (p))
#else
#define ARSHIFT(n, p)                                                   \
    (((n) >> (p)) | (bee_word_t)(LSHIFT(-((n) < 0), (BEE_WORD_BIT - p))))
#endif


// Traps
bee_word_t trap(bee_state * restrict S, bee_word_t code);
bee_word_t trap_libc(bee_state * restrict S);


// Jit compiler
#ifdef HAVE_MIJIT
#include "../mijit-bee/mijit-bee.h"
extern mijit_bee_jit *bee_jit;
#endif
