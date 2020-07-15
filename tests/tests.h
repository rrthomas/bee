// Header for VM tests.
//
// (c) Reuben Thomas 1995-2020
//
// The package is distributed under the GNU Public License version 3, or,
// at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#ifndef BEE_TESTS
#define BEE_TESTS


#include "config.h"

#include "external_syms.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "xvasprintf.h"

#include "bee/bee.h"
#include "bee/aux.h"
#include "bee/debug.h"
#include "bee/opcodes.h"

#include "stringify.h"


// Helper macros for test data, used to generate absolute addresses without
// too many casts.
#define m0 ((WORD)M0)
#define memory ((WORD)MSIZE)


#endif
