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
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>


// Memory access

// Check whether a VM address points to a native word-aligned word
#define IS_VALID(a)                                     \
    (native_address_of_range((a), WORD_BYTES) != NULL)

#define bee__LOAD_WORD(a, temp)                                             \
    ((error = error ? error : bee_load_word((a), &temp)), temp)
#define bee_LOAD_WORD(a) bee__LOAD_WORD(a, temp)
#define bee_STORE_WORD(a, v)                                                \
    (error = error ? error : bee_store_word((a), (v)))
#define bee_LOAD_BYTE(a)                                                    \
    ((error = error ? error : bee_load_byte((a), &byte)), byte)
#define bee_STORE_BYTE(a, v)                                                \
    (error = error ? error : bee_store_byte((a), (v)))
#define bee_PUSH(v)                             \
    (error = error ? error : bee_push_stack(S0, SSIZE, &SP, v))
#define bee_POP                                 \
    (error = error ? error : bee_pop_stack(S0, SSIZE, &SP, &temp), temp)
#define bee_PUSH_DOUBLE(ud)                                             \
    bee_PUSH((bee_UWORD)(ud & bee_WORD_MASK));                          \
    bee_PUSH((bee_UWORD)((ud >> bee_WORD_BIT) & bee_WORD_MASK));
#define bee_POP_DOUBLE                                                  \
    (tempd = ((bee_DUWORD)(bee_UWORD)bee_POP) << bee_WORD_BIT,          \
     tempd |= (bee_UWORD)bee_POP,                                       \
     tempd)

#define bee_PUSH_RETURN(v)                                              \
    (error = error ? error : bee_push_stack(R0, RSIZE, &RP, v))
#define bee_POP_RETURN                                                  \
    (error = error ? error : bee_pop_stack(R0, RSIZE, &RP, &temp), temp)

uint8_t *native_address_of_range(bee_UWORD start, bee_UWORD length);

// Align a VM address
#define bee_ALIGN(a) ((a + bee_WORD_BYTES - 1) & (-bee_WORD_BYTES))

// Check whether a VM address is aligned
#define bee_IS_ALIGNED(a)     (((a) & (bee_WORD_BYTES - 1)) == 0)

// Portable arithmetic right shift (the behaviour of >> on signed
// quantities is implementation-defined in C99)
#define bee_ARSHIFT(n, p) ((n) = ((n) >> (p)) | (-((n) < 0) << (bee_WORD_BIT - p)))


#endif
