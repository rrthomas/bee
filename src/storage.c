// Allocate storage for the registers and memory.
//
// (c) Reuben Thomas 1994-2020
//
// The package is distributed under the GNU Public License version 3, or,
// at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#include "config.h"

#include "external_syms.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "bee/bee.h"

#include "aux.h"
#include "private.h"


// VM registers

#define R(name, type) \
    type name;
#include "registers.h"
#undef R


// Stacks

_GL_ATTRIBUTE_CONST WORD *bee_stack_position(WORD *s0, UWORD sp, UWORD pos)
{
    if (unlikely(pos >= sp))
        return NULL;
    return &s0[sp - pos - 1];
}

int bee_pop_stack(WORD *s0, UWORD ssize, UWORD *sp, WORD *val_ptr)
{
    if (unlikely(*sp == 0))
        return BEE_ERROR_STACK_UNDERFLOW;
    else if (unlikely(*sp > ssize))
        return BEE_ERROR_STACK_OVERFLOW;
    (*sp)--;
    *val_ptr = s0[*sp];
    return BEE_ERROR_OK;
}

int bee_push_stack(WORD *s0, UWORD ssize, UWORD *sp, WORD val)
{
    if (unlikely(*sp >= ssize))
        return BEE_ERROR_STACK_OVERFLOW;
    s0[*sp] = val;
    (*sp)++;
    return BEE_ERROR_OK;
}


// Initialise VM state.
int bee_init(WORD *buf, WORD memory_size, WORD stack_size, WORD return_stack_size)
{
    if (buf == NULL)
        return -1;
    M0 = buf;
    MSIZE = memory_size * WORD_BYTES;
    memset(M0, 0, MSIZE);

    PC = M0;
    DP = 0;

    DSIZE = stack_size;
    D0 = (WORD *)calloc(DSIZE, WORD_BYTES);
    if (D0 == NULL)
        return -1;
    SP = 0;
    HANDLER_SP = (UWORD)-1;

    SSIZE = return_stack_size;
    S0 = (WORD *)calloc(SSIZE, WORD_BYTES);
    if (S0 == NULL) {
        free(S0);
        return -1;
    }

    return 0;
}

int bee_init_defaults(WORD *buf, WORD memory_size)
{
    return bee_init(buf, memory_size, DEFAULT_STACK_SIZE, DEFAULT_STACK_SIZE);
}
