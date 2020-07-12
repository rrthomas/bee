// Auxiliary public functions.
// These are undocumented and subject to change.
//
// (c) Reuben Thomas 1994-2018
//
// The package is distributed under the GNU Public License version 3, or,
// at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#ifndef bee_BEE_AUX
#define bee_BEE_AUX


#include "config.h"

#include <stdio.h>      // for the FILE type
#include <stdint.h>
#include <limits.h>


// Errors
#define THROW(code)                             \
    do {                                        \
        error = code;                           \
        goto error;                             \
    } while (0)

#define THROW_IF_ERROR(code)                    \
    if (code != ERROR_OK)                       \
        THROW(code)


// Stack access
#define bee_POP(ptr)                                    \
    THROW_IF_ERROR(bee_pop_stack(S0, SSIZE, &SP, ptr))
#define bee_PUSH(v)                                             \
    do {                                                        \
        THROW_IF_ERROR(bee_push_stack(S0, SSIZE, &SP, v));      \
    } while (0)

#define bee_DOUBLE_WORD(pop1, pop2)                                     \
    (((bee_DUWORD)(bee_UWORD)pop1) << bee_WORD_BIT | (bee_UWORD)pop2)
#define bee_PUSH_DOUBLE(ud)                                             \
    bee_PUSH((bee_UWORD)(ud & bee_WORD_MASK));                          \
    bee_PUSH((bee_UWORD)((ud >> bee_WORD_BIT) & bee_WORD_MASK));

#define bee_POP_RETURN(ptr)                             \
    THROW_IF_ERROR(bee_pop_stack(R0, RSIZE, &RP, ptr))
#define bee_PUSH_RETURN(v)                                      \
    do {                                                        \
        THROW_IF_ERROR(bee_push_stack(R0, RSIZE, &RP, v));      \
    } while (0)


// Memory access

// Check whether a VM address points to a native word-aligned word
#define IS_VALID(a)                                     \
    (native_address_of_range((a), WORD_BYTES) != NULL)

uint8_t *native_address_of_range(bee_UWORD start, bee_UWORD length);

// Align a VM address
#define bee_ALIGN(a) ((a + bee_WORD_BYTES - 1) & (-bee_WORD_BYTES))

// Check whether a VM address is aligned
#define bee_IS_ALIGNED(a)     (((a) & (bee_WORD_BYTES - 1)) == 0)

// Portable left shift (the behaviour of << with overflow (including on any
// negative number) is undefined)
#define bee_LSHIFT(n, p) (((n) & ((UWORD)-1 >> p)) << p)

// Portable arithmetic right shift (the behaviour of >> on signed
// quantities is implementation-defined in C99)
#define bee_ARSHIFT(n, p) (((n) >> (p)) | (LSHIFT(-((n) < 0), (bee_WORD_BIT - p))))


#endif
