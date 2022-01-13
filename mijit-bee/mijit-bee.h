// C declarations of Rust APIs.
//
// (c) Reuben Thomas 2022
//
// The package is distributed under the GNU Public License version 3, or,
// at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#ifndef MIJIT_BEE
#define MIJIT_BEE

#include <assert.h>
#include <stdint.h>

typedef struct mijit_bee_jit mijit_bee_jit;

static_assert(sizeof(intptr_t) == 8);
static_assert(sizeof(intptr_t *) == 8);

typedef struct mijit_bee_registers {
    intptr_t *pc;
    intptr_t *m0;
    uintptr_t msize;
    intptr_t *s0;
    uintptr_t ssize;
    uintptr_t sp;
    intptr_t *d0;
    uintptr_t dsize;
    uintptr_t dp;
    uintptr_t handler_sp;
} mijit_bee_registers;

mijit_bee_jit *mijit_bee_new(void);

void mijit_bee_drop(mijit_bee_jit *jit);

void mijit_bee_run(mijit_bee_jit *jit, mijit_bee_registers *registers);

#endif
