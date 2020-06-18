// Auxiliary public functions.
// These are undocumented and subject to change.
//
// (c) Reuben Thomas 1994-2018
//
// The package is distributed under the GNU Public License version 3, or,
// at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER‘S
// RISK.

#ifndef bee_BEE_AUX
#define bee_BEE_AUX


#include "config.h"

#include <stdio.h>      // for the FILE type
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>


// Stacks size
#define bee_DEFAULT_STACK_SIZE   16384

// Memory access

// Return value is 0 if OK, or exception code for invalid or unaligned address
bee_CELL bee_reverse_cell(bee_CELL value);
int bee_reverse(bee_UCELL start, bee_UCELL length);

#define bee_STACK_DIRECTION -1
#define bee__LOAD_CELL(a, temp)                                             \
    ((exception = exception ? exception : bee_load_cell((a), &temp)), temp)
#define bee_LOAD_CELL(a) bee__LOAD_CELL(a, temp)
#define bee_STORE_CELL(a, v)                                                \
    (exception = exception ? exception : bee_store_cell((a), (v)))
#define bee_LOAD_BYTE(a)                                                    \
    ((exception = exception ? exception : bee_load_byte((a), &byte)), byte)
#define bee_STORE_BYTE(a, v)                                                \
    (exception = exception ? exception : bee_store_byte((a), (v)))
#define bee_PUSH(v)                                 \
    (bee_SP += bee_CELL_W * bee_STACK_DIRECTION, bee_STORE_CELL(bee_SP, (v)))
#define bee_POP                                     \
    (bee_SP -= bee_CELL_W * bee_STACK_DIRECTION, bee_LOAD_CELL(bee_SP + bee_CELL_W * bee_STACK_DIRECTION))
#define bee_PUSH_DOUBLE(ud)                         \
    bee_PUSH((bee_UCELL)(ud & bee_CELL_MASK));              \
    bee_PUSH((bee_UCELL)((ud >> bee_CELL_BIT) & bee_CELL_MASK))
#define bee_POP_DOUBLE                              \
    (bee_SP -= 2 * bee_CELL_W * bee_STACK_DIRECTION, (bee_UCELL)bee_LOAD_CELL(bee_SP + bee_CELL_W * bee_STACK_DIRECTION), temp | \
     ((bee_DUCELL)(bee_UCELL)bee__LOAD_CELL(bee_SP + 2 * bee_CELL_W * bee_STACK_DIRECTION, temp2) << bee_CELL_BIT))
#define bee_PUSH_RETURN(v)                          \
    (bee_RP += bee_CELL_W * bee_STACK_DIRECTION, bee_STORE_CELL(bee_RP, (v)))
#define bee_POP_RETURN                              \
    (bee_RP -= bee_CELL_W * bee_STACK_DIRECTION, bee_LOAD_CELL(bee_RP + bee_CELL_W * bee_STACK_DIRECTION))
#define bee_STACK_UNDERFLOW(ptr, base)              \
    (ptr - base == 0 ? false : (bee_STACK_DIRECTION > 0 ? ptr < base : ptr > base))

uint8_t *native_address_of_range(bee_UCELL start, bee_UCELL length);

// Align a VM address
#define bee_ALIGN(a) ((a + bee_CELL_W - 1) & (-bee_CELL_W))

// Check whether a VM address is aligned
#define bee_IS_ALIGNED(a)     (((a) & (bee_CELL_W - 1)) == 0)

// Portable arithmetic right shift (the behaviour of >> on signed
// quantities is implementation-defined in C99)
#define bee_ARSHIFT(n, p) ((n) = ((n) >> (p)) | (-((n) < 0) << (bee_CELL_BIT - p)))


#endif
