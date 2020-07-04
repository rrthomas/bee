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

UWORD PC;
WORD *M0, *R0, *S0;
UWORD RSIZE, SSIZE;
UWORD SP, RP, HANDLER_RP;
UWORD MEMORY;


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
_GL_ATTRIBUTE_PURE uint8_t *native_address_of_range(UWORD start, UWORD length)
{
    if (start > MEMORY || MEMORY - start < length)
        return NULL;

    return ((uint8_t *)M0) + start;
}

// Macro for byte addressing
#ifdef WORDS_BIGENDIAN
#define FLIP(addr) ((addr) ^ (WORD_BYTES - 1))
#else
#define FLIP(addr) (addr)
#endif

int load_word(UWORD addr, WORD *value)
{
    if (!IS_ALIGNED(addr))
        return ERROR_UNALIGNED_ADDRESS;

    uint8_t *ptr = native_address_of_range(addr, WORD_BYTES);
    if (ptr == NULL || !IS_ALIGNED((size_t)ptr))
        return ERROR_INVALID_LOAD;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
    *value = *(WORD *)ptr;
#pragma GCC diagnostic pop
    return ERROR_OK;
}

int load_byte(UWORD addr, uint8_t *value)
{
    uint8_t *ptr = native_address_of_range(FLIP(addr), 1);
    if (ptr == NULL)
        return ERROR_INVALID_LOAD;
    *value = *ptr;
    return ERROR_OK;
}

int store_word(UWORD addr, WORD value)
{
    if (!IS_ALIGNED(addr))
        return ERROR_UNALIGNED_ADDRESS;

    uint8_t *ptr = native_address_of_range(addr, WORD_BYTES);
    if (ptr == NULL || !IS_ALIGNED((size_t)ptr))
        return ERROR_INVALID_STORE;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
    *(WORD *)ptr = value;
#pragma GCC diagnostic pop
    return ERROR_OK;
}

int store_byte(UWORD addr, uint8_t value)
{
    uint8_t *ptr = native_address_of_range(FLIP(addr), 1);
    if (ptr == NULL)
        return ERROR_INVALID_STORE;
    *ptr = value;
    return ERROR_OK;
}


_GL_ATTRIBUTE_CONST WORD reverse_word(WORD value)
{
    WORD res = 0;
    for (unsigned i = 0; i < WORD_BYTES / 2; i++) {
        unsigned lopos = CHAR_BIT * i;
        unsigned hipos = CHAR_BIT * (WORD_BYTES - 1 - i);
        unsigned move = hipos - lopos;
        res |= ((((UWORD)value) & (CHAR_MASK << hipos)) >> move)
            | ((((UWORD)value) & (CHAR_MASK << lopos)) << move);
    }
    return res;
}

int reverse(UWORD start, UWORD length)
{
    int ret = 0;
    for (UWORD i = 0; ret == 0 && i < length; i ++) {
        WORD c;
        ret = load_word(start + i * WORD_BYTES, &c)
            || store_word(start + i, reverse_word(c));
    }
    return ret;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsuggest-attribute=pure"
int pre_dma(UWORD from, UWORD to)
{
    int error = 0;

    // Expand range to words
    from &= -WORD_BYTES;
    to = ALIGN(to);

    if (to < from || native_address_of_range(from, to - from) == NULL)
        error = -1;
    if (error == 0 && ENDISM)
        error = reverse(from, to - from);
    return error;
}
#pragma GCC diagnostic pop

int post_dma(UWORD from, UWORD to)
{
    return pre_dma(from, to);
}


// Initialise registers that are not fixed

int init(WORD *buf, WORD memory_size, WORD stack_size, WORD return_stack_size)
{
    if (buf == NULL)
        return -1;
    M0 = buf;
    MEMORY = memory_size * WORD_BYTES;
    memset(M0, 0, MEMORY);

    PC = 0;
    SP = 0;

    SSIZE = stack_size;
    S0 = (WORD *)calloc(SSIZE, WORD_BYTES);
    if (S0 == NULL)
        return -1;
    RP = 0;

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