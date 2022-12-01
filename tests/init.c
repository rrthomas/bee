// Test that the VM headers compile properly, and that the storage
// allocation and register initialisation works.
//
// (c) Reuben Thomas 1994-2022
//
// The package is distributed under the GNU General Public License version 3,
// or, at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#include "tests.h"


int main(void)
{
    size_t size = 1024;
    bee_word_t *m0 = (bee_word_t *)malloc(size);
    assert(m0);
    bee_state *S = init_defaults(m0);
    if (S == NULL) {
        printf("Error in init_defaults() tests: init with valid parameters failed\n");
        exit(1);
    }

    printf("init_defaults() tests ran OK\n");
    bee_destroy(S);
    free(m0);
    return 0;
}
