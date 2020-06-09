// Public data structures and interface calls specified in the VM definition.
// This is the header file to include in programs using an embedded VM.
//
// (c) Reuben Thomas 1994-2018
//
// The package is distributed under the GNU Public License version 3, or,
// at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER‘S
// RISK.

#ifndef beetle_BEETLE_BEETLE
#define beetle_BEETLE_BEETLE


#include "config.h"

#include <stdio.h>      // for the FILE type
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>


// Basic types
typedef uint8_t beetle_BYTE;
typedef int32_t beetle_CELL;
typedef uint32_t beetle_UCELL;
typedef uint64_t beetle_DUCELL;
#define beetle_CHAR_MASK ((1 << CHAR_BIT) - 1)
#define beetle_CELL_BIT (sizeof(beetle_CELL_W) * CHAR_BIT)
#define beetle_CELL_MIN ((beetle_CELL)(1UL << (beetle_CELL_BIT - 1)))
#define beetle_CELL_MAX (UINT32_MAX)
#define beetle_CELL_MASK beetle_CELL_MAX

// VM registers

// beetle_ENDISM is fixed at compile-time, which seems reasonable, as
// machines rarely change endianness while switched on!
#ifdef WORDS_BIGENDIAN
#define beetle_ENDISM ((beetle_BYTE)1)
#else
#define beetle_ENDISM ((beetle_BYTE)0)
#endif

extern beetle_UCELL beetle_EP;
extern beetle_BYTE beetle_I;
extern beetle_CELL beetle_A;
extern beetle_CELL *M0;
extern beetle_UCELL beetle_MEMORY;
extern beetle_UCELL beetle_SP, beetle_RP;
extern beetle_UCELL beetle_S0, beetle_R0;
extern beetle_UCELL beetle_THROW;
extern beetle_UCELL beetle_BAD;
extern beetle_UCELL beetle_NOT_ADDRESS;
#define beetle_CHECKED 1       // address checking is mandatory in this implementation

// Memory access

// Return value is 0 if OK, or exception code for invalid or unaligned address
int beetle_load_cell(beetle_UCELL addr, beetle_CELL *value);
int beetle_store_cell(beetle_UCELL addr, beetle_CELL value);
int beetle_load_byte(beetle_UCELL addr, beetle_BYTE *value);
int beetle_store_byte(beetle_UCELL addr, beetle_BYTE value);

int beetle_pre_dma(beetle_UCELL from, beetle_UCELL to);
int beetle_post_dma(beetle_UCELL from, beetle_UCELL to);

// Interface calls
uint8_t *native_address_of_range(beetle_UCELL addr, beetle_UCELL length);
beetle_CELL beetle_run(void);
beetle_CELL beetle_single_step(void);
int beetle_load_object(FILE *file, beetle_UCELL address);

// Additional implementation-specific routines, macros, types and quantities
int beetle_init(beetle_CELL *c_array, size_t size);
int beetle_register_args(int argc, const char *argv[]);

#define BEETLE_TRUE beetle_CELL_MASK            // VM TRUE flag
#define BEETLE_FALSE ((beetle_CELL)0)           // VM FALSE flag

#define beetle_CELL_W 4    // the width of a cell in bytes
#define beetle_POINTER_W (sizeof(void *) / beetle_CELL_W)   // the width of a machine pointer in cells

// beetle_A union to allow storage of machine pointers in VM beetle_memory
union _CELL_pointer {
    beetle_CELL cells[beetle_POINTER_W];
    void (*pointer)(void);
};
typedef union _CELL_pointer beetle_CELL_pointer;

#endif
