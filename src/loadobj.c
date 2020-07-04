// The interface call load_object(file, address) : integer.
//
// (c) Reuben Thomas 1995-2020
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


// Return the length of a seekable stream, or `-1` if not seekable
static off_t fleno(FILE *fp)
{
    off_t pos = ftello(fp);
    if (pos != -1 && fseeko(fp, 0, SEEK_END) == 0) {
        off_t end = ftello(fp);
        if (end != -1 && fseeko(fp, pos, SEEK_SET) == 0)
            return end - pos;
    }
    return -1;
}

// Skip any #! header
static int skip_hashbang(FILE *fp)
{
    if (getc(fp) != '#' || getc(fp) != '!')
        return fseeko(fp, 0, SEEK_SET);
    for (int res; (res = getc(fp)) != '\n'; )
        if (res == EOF)
            return -1;
    return 0;
}

int load_object(FILE *fp, UWORD address)
{
    if (!IS_ALIGNED(address))
        return -1;
    if (fp == NULL || skip_hashbang(fp) == -1)
        return -2;
    off_t len = fleno(fp);
    if (len == -1)
        return -2;
    uint8_t *ptr = native_address_of_range(address, len);
    if (ptr == NULL)
        return -1;
    if ((off_t)fread(ptr, 1, len, fp) != len || fclose(fp) == EOF)
        return -2;

    return 0;
}
