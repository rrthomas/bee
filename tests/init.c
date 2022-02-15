// Test that the VM headers compile properly, and that the
// storage allocation and register initialisation in storage.c works.
//
// (c) Reuben Thomas 1994-2018
//
// The package is distributed under the GNU General Public License version 3,
// or, at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#include "tests.h"


int main(void)
{
    int i = bee_init_defaults((bee_word_t *)NULL, 4);
    printf("bee_init_defaults((bee_word_t *)NULL, 4) should return -1; returns: %d\n", i);
    if (i != -1) {
        printf("Error in bee_init_defaults() tests: init with invalid parameters "
            "succeeded\n");
        exit(1);
    }

    size_t size = 1024;
    bee_word_t *ptr = (bee_word_t *)malloc(size);
    assert(ptr);
    i = bee_init_defaults(ptr, size / BUMBLE_WORD_BYTES);
    if (i != 0) {
        printf("Error in bee_init_defaults() tests: init with valid parameters failed\n");
        exit(1);
    }

    printf("bee_init_defaults() tests ran OK\n");
    return 0;
}
