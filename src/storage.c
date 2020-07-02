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

#include <stdlib.h>
#include <string.h>

#include "bee.h"
#include "bee_aux.h"
#include "private.h"


// VM registers

UCELL PC;
BYTE I;
CELL A;
CELL *M0, *R0, *S0;
UCELL RSIZE, SSIZE;
UCELL SP, RP;
UCELL MEMORY;


// Stacks

_GL_ATTRIBUTE_CONST CELL *stack_position(CELL *s0, UCELL sp, UCELL pos)
{
    if (pos >= sp)
        return NULL;
    return &s0[sp - pos - 1];
}

int pop_stack(CELL *s0, UCELL ssize, UCELL *sp, CELL *val_ptr)
{
    if (*sp == 0)
        return ERROR_STACK_UNDERFLOW;
    else if (*sp > ssize)
        return ERROR_STACK_OVERFLOW;
    (*sp)--;
    *val_ptr = s0[*sp];
    return ERROR_OK;
}

int push_stack(CELL *s0, UCELL ssize, UCELL *sp, CELL val)
{
    if (unlikely(*sp >= ssize))
        return ERROR_STACK_OVERFLOW;
    s0[*sp] = val;
    (*sp)++;
    return ERROR_OK;
}


// General memory access

// Return native address of a range of VM memory, or NULL if invalid
_GL_ATTRIBUTE_PURE uint8_t *native_address_of_range(UCELL start, UCELL length)
{
    if (start > MEMORY || MEMORY - start < length)
        return NULL;

    return ((uint8_t *)M0) + start;
}

// Macro for byte addressing
#ifdef WORDS_BIGENDIAN
#define FLIP(addr) ((addr) ^ (CELL_W - 1))
#else
#define FLIP(addr) (addr)
#endif

int load_cell(UCELL addr, CELL *value)
{
    if (!IS_ALIGNED(addr))
        return ERROR_UNALIGNED_ADDRESS;

    uint8_t *ptr = native_address_of_range(addr, CELL_W);
    if (ptr == NULL || !IS_ALIGNED((size_t)ptr))
        return ERROR_INVALID_LOAD;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
    *value = *(CELL *)ptr;
#pragma GCC diagnostic pop
    return ERROR_OK;
}

int load_byte(UCELL addr, BYTE *value)
{
    uint8_t *ptr = native_address_of_range(FLIP(addr), 1);
    if (ptr == NULL)
        return ERROR_INVALID_LOAD;
    *value = *ptr;
    return ERROR_OK;
}

int store_cell(UCELL addr, CELL value)
{
    if (!IS_ALIGNED(addr))
        return ERROR_UNALIGNED_ADDRESS;

    uint8_t *ptr = native_address_of_range(addr, CELL_W);
    if (ptr == NULL || !IS_ALIGNED((size_t)ptr))
        return ERROR_INVALID_STORE;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
    *(CELL *)ptr = value;
#pragma GCC diagnostic pop
    return ERROR_OK;
}

int store_byte(UCELL addr, BYTE value)
{
    uint8_t *ptr = native_address_of_range(FLIP(addr), 1);
    if (ptr == NULL)
        return ERROR_INVALID_STORE;
    *ptr = value;
    return ERROR_OK;
}


_GL_ATTRIBUTE_CONST CELL reverse_cell(CELL value)
{
    CELL res = 0;
    for (unsigned i = 0; i < CELL_W / 2; i++) {
        unsigned lopos = CHAR_BIT * i;
        unsigned hipos = CHAR_BIT * (CELL_W - 1 - i);
        unsigned move = hipos - lopos;
        res |= ((((UCELL)value) & (CHAR_MASK << hipos)) >> move)
            | ((((UCELL)value) & (CHAR_MASK << lopos)) << move);
    }
    return res;
}

int reverse(UCELL start, UCELL length)
{
    int ret = 0;
    for (UCELL i = 0; ret == 0 && i < length; i ++) {
        CELL c;
        ret = load_cell(start + i * CELL_W, &c)
            || store_cell(start + i, reverse_cell(c));
    }
    return ret;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsuggest-attribute=pure"
int pre_dma(UCELL from, UCELL to)
{
    int exception = 0;

    // Expand range to words
    from &= -CELL_W;
    to = ALIGN(to);

    if (to < from || native_address_of_range(from, to - from) == NULL)
        exception = -1;
    if (exception == 0 && ENDISM)
        exception = reverse(from, to - from);
    return exception;
}
#pragma GCC diagnostic pop

int post_dma(UCELL from, UCELL to)
{
    return pre_dma(from, to);
}


// Initialise registers that are not fixed

int init(CELL *buf, size_t size)
{
    if (buf == NULL)
        return -1;
    M0 = buf;
    MEMORY = size * CELL_W;
    memset(M0, 0, MEMORY);

    PC = 0;
    A = 0;
    SP = 0;
    SSIZE = /* FIXME: Variable */ 4096;
    S0 = (CELL *)calloc(SSIZE, CELL_W);
    if (S0 == NULL) {
        free(buf);
        return -1;
    }
    RP = 0;
    RSIZE = /* FIXME: Variable */ 4096;
    R0 = (CELL *)calloc(RSIZE, CELL_W);
    if (R0 == NULL) {
        free(buf);
        free(R0);
        return -1;
    }

    return 0;
}
