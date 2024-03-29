// Public data structures and interface calls specified in the VM definition.
// This is the header file to include in programs using an embedded VM.
//
// (c) Reuben Thomas 1994-2022
//
// The package is distributed under the GNU General Public License version 3,
// or, at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#ifndef BEE_BEE
#define BEE_BEE


#include <stdint.h>
#include <limits.h>


// Basic types
typedef intptr_t bee_word_t;
typedef uintptr_t bee_uword_t;
// Cast to int for printf convenience
#define BEE_WORD_BYTES ((int)sizeof(bee_word_t))
#define BEE_WORD_BIT (BEE_WORD_BYTES * CHAR_BIT)
#define BEE_WORD_MIN INTPTR_MIN
#define BEE_WORD_MAX INTPTR_MAX
#define BEE_UWORD_MAX UINTPTR_MAX

// VM state
typedef struct bee_state {
#define R(reg, type)                            \
    type reg;
#include <bee/registers.h>
#undef R
} bee_state;

// Error codes
enum {
    BEE_ERROR_OK = 0,
    BEE_ERROR_INVALID_OPCODE = -1,
    BEE_ERROR_STACK_UNDERFLOW = -2,
    BEE_ERROR_STACK_OVERFLOW = -3,
    BEE_ERROR_UNALIGNED_ADDRESS = -4,
    BEE_ERROR_INVALID_LIBRARY = -16,
    BEE_ERROR_INVALID_FUNCTION = -17,
    BEE_ERROR_BREAK = -256,
};

// Stack access
int bee_check_stack(bee_uword_t ssize, bee_uword_t sp, bee_uword_t pushes, bee_uword_t pops);
int bee_pop_stack(bee_word_t *s0, bee_uword_t ssize, bee_uword_t *sp, bee_word_t *val_ptr);
int bee_push_stack(bee_word_t *s0, bee_uword_t ssize, bee_uword_t *sp, bee_word_t val);

// Default stacks size in words
#define BEE_DEFAULT_STACK_SIZE   4096

// VM state methods
bee_state *bee_init(bee_word_t *pc, bee_uword_t stack_size, bee_uword_t return_stack_size);
void bee_destroy(bee_state * restrict S);
bee_word_t bee_run(bee_state * restrict S);

void bee_register_args(int argc, const char *argv[]);


#endif
