// Public data structures and interface calls specified in the VM definition.
// This is the header file to include in programs using an embedded VM.
//
// (c) Reuben Thomas 1994-2018
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
typedef uint8_t bee_BYTE;
typedef int32_t bee_WORD;
typedef uint32_t bee_UWORD;
typedef uint64_t bee_DUWORD;
#define bee_CHAR_MASK ((1 << CHAR_BIT) - 1)
#define bee_WORD_BIT (sizeof(bee_WORD_BYTES) * CHAR_BIT)
#define bee_WORD_MIN ((bee_WORD)(1UL << (bee_WORD_BIT - 1)))
#define bee_WORD_MAX (UINT32_MAX)
#define bee_WORD_MASK bee_WORD_MAX

// VM registers

// bee_ENDISM is fixed at compile-time, which seems reasonable, as
// machines rarely change endianness while switched on!
#ifdef WORDS_BIGENDIAN
#define bee_ENDISM ((bee_BYTE)1)
#else
#define bee_ENDISM ((bee_BYTE)0)
#endif

extern bee_UWORD bee_PC;
extern bee_WORD *bee_M0, *bee_R0, *bee_S0;
extern bee_UWORD bee_RSIZE, bee_SSIZE;
extern bee_UWORD bee_MEMORY;
extern bee_UWORD bee_SP, bee_RP;

// Error codes
enum {
    BEE_ERROR_OK = 0,
    BEE_ERROR_INVALID_OPCODE = -1,
    BEE_ERROR_STACK_UNDERFLOW = -2,
    BEE_ERROR_STACK_OVERFLOW = -3,
    BEE_ERROR_INVALID_MEMORY_READ = -5,
    BEE_ERROR_INVALID_MEMORY_WRITE = -6,
    BEE_ERROR_UNALIGNED_ADDRESS = -7,
    BEE_ERROR_DIVISION_BY_ZERO = -8,
    BEE_ERROR_STEP = -257,
};

// Stack access
_GL_ATTRIBUTE_PURE WORD *bee_stack_position(WORD *s0, UWORD sp, UWORD pos);
int bee_pop_stack(WORD *s0, UWORD ssize, UWORD *sp, WORD *val_ptr);
int bee_push_stack(WORD *s0, UWORD ssize, UWORD *sp, WORD val);


// Memory access

// Return value is 0 if OK, or exception code for invalid or unaligned address
int bee_load_word(bee_UWORD addr, bee_WORD *value);
int bee_store_word(bee_UWORD addr, bee_WORD value);
int bee_load_byte(bee_UWORD addr, bee_BYTE *value);
int bee_store_byte(bee_UWORD addr, bee_BYTE value);

int bee_pre_dma(bee_UWORD from, bee_UWORD to);
int bee_post_dma(bee_UWORD from, bee_UWORD to);

// Interface calls
uint8_t *native_address_of_range(bee_UWORD addr, bee_UWORD length);
bee_WORD bee_run(void);
bee_WORD bee_single_step(void);
int bee_load_object(FILE *file, bee_UWORD address);

// Default stacks size in words
#define bee_DEFAULT_STACK_SIZE   4096

// Additional implementation-specific routines, macros, types and quantities
int bee_init(bee_WORD *c_array, WORD memory_size, WORD stack_size, WORD return_stack_size);
int bee_init_defaults(bee_WORD *c_array, WORD memory_size);
int bee_register_args(int argc, const char *argv[]);

#define BEE_TRUE bee_WORD_MASK            // VM TRUE flag
#define BEE_FALSE ((bee_WORD)0)           // VM FALSE flag

#define bee_WORD_BYTES 4    // the width of a word in bytes
#define bee_POINTER_W (sizeof(void *) / bee_WORD_BYTES)   // the width of a machine pointer in words

// bee_A union to allow storage of machine pointers in VM bee_memory
union _WORD_pointer {
    bee_WORD words[bee_POINTER_W];
    void (*pointer)(void);
};
typedef union _WORD_pointer bee_WORD_pointer;

#endif
