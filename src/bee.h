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

#ifndef bee_BEE_BEE
#define bee_BEE_BEE


#include "config.h"

#include <stdio.h>      // for the FILE type
#include <stdint.h>
#include <limits.h>


// Basic types
typedef int32_t bee_WORD;
typedef uint32_t bee_UWORD;
typedef uint64_t bee_DUWORD;
#define bee_CHAR_MASK ((1 << CHAR_BIT) - 1)
#define bee_WORD_BIT (sizeof(bee_WORD_BYTES) * CHAR_BIT)
#define bee_WORD_MIN ((bee_WORD)(1UL << (bee_WORD_BIT - 1)))
#define bee_WORD_MAX (UINT32_MAX)
#define bee_WORD_MASK bee_WORD_MAX

// VM registers

extern bee_UWORD bee_PC;
extern bee_WORD *bee_M0, *bee_R0, *bee_S0;
extern bee_UWORD bee_RSIZE, bee_SSIZE;
extern bee_UWORD bee_MEMORY;
extern bee_UWORD bee_SP, bee_RP, bee_HANDLER_RP;

// Error codes
enum {
    bee_ERROR_OK = 0,
    bee_ERROR_INVALID_OPCODE = -1,
    bee_ERROR_STACK_UNDERFLOW = -2,
    bee_ERROR_STACK_OVERFLOW = -3,
    bee_ERROR_INVALID_LOAD = -4,
    bee_ERROR_INVALID_STORE = -5,
    bee_ERROR_UNALIGNED_ADDRESS = -6,
    bee_ERROR_DIVISION_BY_ZERO = -7,
    bee_ERROR_BREAK = -256,
};

// Stack access
_GL_ATTRIBUTE_PURE WORD *bee_stack_position(WORD *s0, UWORD sp, UWORD pos);
int bee_pop_stack(WORD *s0, UWORD ssize, UWORD *sp, WORD *val_ptr);
int bee_push_stack(WORD *s0, UWORD ssize, UWORD *sp, WORD val);


// Memory access

// Return value is 0 if OK, or error code for invalid or unaligned address
int bee_load_word(bee_UWORD addr, bee_WORD *value);
int bee_store_word(bee_UWORD addr, bee_WORD value);
int bee_load_byte(bee_UWORD addr, uint8_t *value);
int bee_store_byte(bee_UWORD addr, uint8_t value);

// Interface calls
uint8_t *native_address_of_range(bee_UWORD addr, bee_UWORD length);
bee_WORD bee_run(void);
int bee_load_object(FILE *file, bee_UWORD address);

// Default stacks size in words
#define bee_DEFAULT_STACK_SIZE   4096

// Additional implementation-specific routines, macros, types and quantities
int bee_init(bee_WORD *c_array, WORD memory_size, WORD stack_size, WORD return_stack_size);
int bee_init_defaults(bee_WORD *c_array, WORD memory_size);
int bee_register_args(int argc, const char *argv[]);

#define bee_WORD_BYTES 4    // the width of a word in bytes

#endif
