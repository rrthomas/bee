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

#include <stdlib.h>
#include <string.h>

#include "bee/bee.h"

#include "private.h"


// VM registers

#define R(reg, type) \
    type bee_##reg;
#include "bee/registers.h"
#undef R


// Stacks

int bee_pop_stack(bee_word_t *s0, bee_uword_t ssize, bee_uword_t *sp, bee_word_t *val_ptr)
{
    if (unlikely(*sp == 0))
        return BEE_ERROR_STACK_UNDERFLOW;
    else if (unlikely(*sp > ssize))
        return BEE_ERROR_STACK_OVERFLOW;
    (*sp)--;
    *val_ptr = s0[*sp];
    return BEE_ERROR_OK;
}

int bee_push_stack(bee_word_t *s0, bee_uword_t ssize, bee_uword_t *sp, bee_word_t val)
{
    if (unlikely(*sp >= ssize))
        return BEE_ERROR_STACK_OVERFLOW;
    s0[*sp] = val;
    (*sp)++;
    return BEE_ERROR_OK;
}


// Initialise VM state.
int bee_init(bee_word_t *buf, bee_uword_t memory_size, bee_uword_t stack_size, bee_uword_t return_stack_size)
{
    if (buf == NULL)
        return -1;
    bee_m0 = buf;
    bee_msize = memory_size * BEE_WORD_BYTES;
    memset(bee_m0, 0, bee_msize);

    bee_pc = bee_m0;
    bee_dp = 0;

    bee_dsize = stack_size;
    bee_d0 = (bee_word_t *)calloc(bee_dsize, BEE_WORD_BYTES);
    if (bee_d0 == NULL)
        return -1;
    bee_handler_sp = bee_sp = 0;

    bee_ssize = return_stack_size;
    bee_s0 = (bee_word_t *)calloc(bee_ssize, BEE_WORD_BYTES);
    if (bee_s0 == NULL) {
        free(bee_s0);
        return -1;
    }

    return 0;
}

int bee_init_defaults(bee_word_t *buf, bee_uword_t memory_size)
{
    return bee_init(buf, memory_size, BEE_DEFAULT_STACK_SIZE, BEE_DEFAULT_STACK_SIZE);
}
