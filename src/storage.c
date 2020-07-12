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

#include "bee.h"
#include "bee_aux.h"
#include "private.h"


// VM registers

#define R(name, type) \
    type name;
#include "tbl_registers.h"
#undef R


// Stacks

_GL_ATTRIBUTE_CONST WORD *stack_position(WORD *s0, UWORD sp, UWORD pos)
{
    if (pos >= sp)
        return NULL;
    return &s0[sp - pos - 1];
}

int pop_stack(WORD *s0, UWORD ssize, UWORD *sp, WORD *val_ptr)
{
    if (*sp == 0)
        return ERROR_STACK_UNDERFLOW;
    else if (*sp > ssize)
        return ERROR_STACK_OVERFLOW;
    (*sp)--;
    *val_ptr = s0[*sp];
    return ERROR_OK;
}

int push_stack(WORD *s0, UWORD ssize, UWORD *sp, WORD val)
{
    if (unlikely(*sp >= ssize))
        return ERROR_STACK_OVERFLOW;
    s0[*sp] = val;
    (*sp)++;
    return ERROR_OK;
}


// General memory access

// Return native address of a range of VM memory, or NULL if invalid
_GL_ATTRIBUTE_PURE bool address_range_valid(uint8_t *start, UWORD length)
{
    return (start >= (uint8_t *)M0 &&
            start <= (uint8_t *)M0 + MEMORY &&
            length <= (UWORD)((uint8_t *)M0 + MEMORY - start));
}

int load_word(WORD *ptr, WORD *value)
{
    if (!address_range_valid((uint8_t *)ptr, WORD_BYTES))
        return ERROR_INVALID_LOAD;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
    *value = *(WORD *)ptr;
#pragma GCC diagnostic pop
    return ERROR_OK;
}

int load_byte(uint8_t *ptr, uint8_t *value)
{
    if (!address_range_valid(ptr, 1))
        return ERROR_INVALID_LOAD;
    *value = *ptr;
    return ERROR_OK;
}

int store_word(WORD *ptr, WORD value)
{
    if (!address_range_valid((uint8_t *)ptr, WORD_BYTES))
        return ERROR_INVALID_STORE;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
    *(WORD *)ptr = value;
#pragma GCC diagnostic pop
    return ERROR_OK;
}

int store_byte(uint8_t *ptr, uint8_t value)
{
    if (!address_range_valid(ptr, 1))
        return ERROR_INVALID_STORE;
    *ptr = value;
    return ERROR_OK;
}


// Initialise registers that are not fixed

int init(WORD *buf, WORD memory_size, WORD stack_size, WORD return_stack_size)
{
    if (buf == NULL)
        return -1;
    M0 = buf;
    MEMORY = memory_size * WORD_BYTES;
    memset(M0, 0, MEMORY);

    PC = M0;
    SP = 0;

    SSIZE = stack_size;
    S0 = (WORD *)calloc(SSIZE, WORD_BYTES);
    if (S0 == NULL)
        return -1;
    RP = 0;
    HANDLER_RP = (UWORD)-1;

    RSIZE = return_stack_size;
    R0 = (WORD *)calloc(RSIZE, WORD_BYTES);
    if (R0 == NULL) {
        free(R0);
        return -1;
    }

    return 0;
}

int init_defaults(WORD *buf, WORD memory_size)
{
    return init(buf, memory_size, DEFAULT_STACK_SIZE, DEFAULT_STACK_SIZE);
}
