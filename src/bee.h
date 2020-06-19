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
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>


// Basic types
typedef uint8_t bee_BYTE;
typedef int32_t bee_CELL;
typedef uint32_t bee_UCELL;
typedef uint64_t bee_DUCELL;
#define bee_CHAR_MASK ((1 << CHAR_BIT) - 1)
#define bee_CELL_BIT (sizeof(bee_CELL_W) * CHAR_BIT)
#define bee_CELL_MIN ((bee_CELL)(1UL << (bee_CELL_BIT - 1)))
#define bee_CELL_MAX (UINT32_MAX)
#define bee_CELL_MASK bee_CELL_MAX

// VM registers

// bee_ENDISM is fixed at compile-time, which seems reasonable, as
// machines rarely change endianness while switched on!
#ifdef WORDS_BIGENDIAN
#define bee_ENDISM ((bee_BYTE)1)
#else
#define bee_ENDISM ((bee_BYTE)0)
#endif

extern bee_UCELL bee_EP;
extern bee_CELL bee_A;
extern bee_CELL *M0;
extern bee_UCELL bee_MEMORY;
extern bee_UCELL bee_SP, bee_RP;
extern bee_UCELL bee_S0, bee_R0;

// Memory access

// Return value is 0 if OK, or exception code for invalid or unaligned address
int bee_load_cell(bee_UCELL addr, bee_CELL *value);
int bee_store_cell(bee_UCELL addr, bee_CELL value);
int bee_load_byte(bee_UCELL addr, bee_BYTE *value);
int bee_store_byte(bee_UCELL addr, bee_BYTE value);

int bee_pre_dma(bee_UCELL from, bee_UCELL to);
int bee_post_dma(bee_UCELL from, bee_UCELL to);

// Interface calls
uint8_t *native_address_of_range(bee_UCELL addr, bee_UCELL length);
bee_CELL bee_run(void);
bee_CELL bee_single_step(void);
int bee_load_object(FILE *file, bee_UCELL address);

// Additional implementation-specific routines, macros, types and quantities
int bee_init(bee_CELL *c_array, size_t size);
int bee_register_args(int argc, const char *argv[]);

#define BEE_TRUE bee_CELL_MASK            // VM TRUE flag
#define BEE_FALSE ((bee_CELL)0)           // VM FALSE flag

#define bee_CELL_W 4    // the width of a cell in bytes
#define bee_POINTER_W (sizeof(void *) / bee_CELL_W)   // the width of a machine pointer in cells

// bee_A union to allow storage of machine pointers in VM bee_memory
union _CELL_pointer {
    bee_CELL cells[bee_POINTER_W];
    void (*pointer)(void);
};
typedef union _CELL_pointer bee_CELL_pointer;

#endif
