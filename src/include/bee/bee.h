// Public data structures and interface calls specified in the VM definition.
// This is the header file to include in programs using an embedded VM.
//
// (c) Reuben Thomas 1994-2020
//
// The package is distributed under the GNU Public License version 3, or,
// at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#ifndef BEE_BEE
#define BEE_BEE


#include <stdio.h>      // for the FILE type
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>


// Basic types
typedef int32_t bee_WORD;
typedef uint32_t bee_UWORD;
typedef uint64_t bee_DUWORD;
#define bee_WORD_BIT (sizeof(bee_WORD_BYTES) * CHAR_BIT)
#define bee_WORD_MIN ((bee_WORD)(1UL << (bee_WORD_BIT - 1)))
#define bee_WORD_MAX (UINT32_MAX)
#define bee_WORD_MASK bee_WORD_MAX

// VM registers

extern bee_WORD *bee_PC, *bee_M0, *bee_S0, *bee_D0;
extern bee_UWORD bee_MSIZE, bee_SSIZE, bee_DSIZE;
extern bee_UWORD bee_DP, bee_SP, bee_HANDLER_SP;

// Error codes
enum {
    bee_ERROR_OK = 0,
    bee_ERROR_INVALID_OPCODE = -1,
    bee_ERROR_STACK_UNDERFLOW = -2,
    bee_ERROR_STACK_OVERFLOW = -3,
    bee_ERROR_INVALID_LOAD = -4,
    bee_ERROR_INVALID_STORE = -5,
    bee_ERROR_UNALIGNED_ADDRESS = -6,
    bee_ERROR_INVALID_LIBRARY = -16,
    bee_ERROR_INVALID_FUNCTION = -17,
    bee_ERROR_BREAK = -256,
};

// Stack access
_GL_ATTRIBUTE_PURE bee_WORD *bee_stack_position(bee_WORD *s0, bee_UWORD sp, bee_UWORD pos);
int bee_pop_stack(bee_WORD *s0, bee_UWORD ssize, bee_UWORD *sp, bee_WORD *val_ptr);
int bee_push_stack(bee_WORD *s0, bee_UWORD ssize, bee_UWORD *sp, bee_WORD val);


// Memory access

// Return value is 0 if OK, or error code for invalid address
int bee_load_word(bee_WORD *ptr, bee_WORD *value);
int bee_store_word(bee_WORD *ptr, bee_WORD value);
int bee_load_byte(uint8_t *ptr, uint8_t *value);
int bee_store_byte(uint8_t *ptr, uint8_t value);

// Interface calls
bee_WORD bee_run(void);
int bee_load_object(FILE *file, bee_WORD *ptr);

// Default stacks size in words
#define bee_DEFAULT_STACK_SIZE   4096

// Additional implementation-specific routines, macros, types and quantities
int bee_init(bee_WORD *c_array, bee_WORD memory_size, bee_WORD stack_size, bee_WORD return_stack_size);
int bee_init_defaults(bee_WORD *c_array, bee_WORD memory_size);
void bee_register_args(int argc, const char *argv[]);

#define bee_WORD_BYTES 4    // the width of a word in bytes

#endif
