// The interface call load_object(file, address) : integer.
//
// (c) Reuben Thomas 1995-2018
//
// The package is distributed under the GNU Public License version 3, or,
// at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#include "config.h"

#include "external_syms.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "bee.h"
#include "bee_aux.h"


int load_object(FILE *file, UWORD address)
{
    if (!IS_ALIGNED(address))
        return -1;

    size_t len = strlen("BEE");
    char magic[7];
    assert(len + 1 <= sizeof(magic));

    // Skip any #! header
    if (fread(&magic[0], 1, 2, file) != 2)
        return -3;
    size_t read = 2;
    if (magic[0] == '#' && magic[1] == '!') {
        while (getc(file) != '\n')
            ;
        read = 0;
    }

    if (fread(&magic[read], 1, sizeof(magic) - read, file) != sizeof(magic) - read)
        return -3;
    if (strncmp(magic, "BEE", sizeof(magic)))
        return -2;

    uint8_t endism;
    if (fread(&endism, 1, 1, file) != 1)
        return -3;
    if (endism != 0 && endism != 1)
        return -2;
    int reversed = endism ^ ENDISM;

    UWORD length = 0;
    if (fread(&length, 1, WORD_BYTES, file) != WORD_BYTES)
        return -3;
    if (reversed)
        length = (UWORD)reverse_word((WORD)length);

    uint8_t *ptr = native_address_of_range(address, length * WORD_BYTES);
    if (ptr == NULL)
        return -1;

    if (fread(ptr, WORD_BYTES, length, file) != length)
        return -3;
    if (reversed)
        reverse(address, length);

    return 0;
}
