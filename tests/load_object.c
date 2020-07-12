// Test load_object().
//
// (c) Reuben Thomas 1995-2020
//
// The package is distributed under the GNU Public License version 3, or,
// at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#include "tests.h"


static int correct[] = { 0, 0, 0 };


static int try(char *file, WORD *ptr)
{
    FILE *fp = fopen(file, "r");
    int ret = load_object(fp, ptr);

    printf("load_object(\"%s\", 0) returns %d", file, ret);

    return ret;
}

static char *obj_name(const char *prefix, const char *file)
{
    char *s = malloc(strlen(prefix) + strlen(file) + 2);
    assert(s);
    strcpy(s, prefix);
    strcat(s, "/");
    strcat(s, file);
    return s;
}

#ifdef WORDS_BIGENDIAN
#define suffix "be"
#else
#define suffix "le"
#endif

int main(int argc, char *argv[])
{
    const char *files[] = { "testobj1-"suffix, "testobj2-"suffix };
    char *prefix = argv[1];
    int res;

    if (argc != 2) {
        printf("Usage: load_object DIRECTORY\n");
        exit(1);
    }

    WORD *_memory = (WORD *)calloc(1024, 1);
    init_defaults(_memory, 256);

    for (size_t i = 0; i < sizeof(files) / sizeof(files[0]); i++) {
        char *s = obj_name(prefix, files[i]);
        WORD c;
        res = try(s, M0);
        free(s);
        printf(" should be %d\n", correct[i]);
        printf("Word 0 of memory is %"PRIX32"; should be 1020304\n", (UWORD)(load_word(M0, &c), c));
        if ((load_word(0, &c), c) != 0x1020304) {
            printf("Error in load_object() tests: file %s\n", files[i]);
            exit(1);
        }
        if (res != correct[i]) {
            printf("Error in load_object() tests: file %s\n", files[i]);
            exit(1);
        }
        memset(_memory, 0, MEMORY); // Zero memory for next test
    }

    printf("load_object() tests ran OK\n");
    return 0;
}
