// Test load_object().
//
// (c) Reuben Thomas 1995-2018
//
// The package is distributed under the GNU Public License version 3, or,
// at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#include "tests.h"


static int correct[] = { -2, -2, -1, -3, 0, 0, 0 };


static int try(char *file, UCELL address)
{
    FILE *fp = fopen(file, "r");
    int ret = load_object(fp, address);

    printf("load_object(\"%s\", 0) returns %d", file, ret);
    fclose(fp);

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

int main(int argc, char *argv[])
{
    const char *files[] = {
        "badobj1", "badobj2", "badobj3", "badobj4", "testobj1", "testobj2", "testobj3" };
    char *prefix = argv[1];
    int res;

    if (argc != 2) {
        printf("Usage: load_object DIRECTORY\n");
        exit(1);
    }

    CELL *memory = (CELL *)calloc(1024, 1);
    init(memory, 256);

    size_t i;
    for (i = 0; i < 4; i++) {
        char *s = obj_name(prefix, files[i]);
        res = try(s, 0);
        free(s);
        printf(" should be %d\n", correct[i]);
        if (res != correct[i]) {
            printf("Error in load_object() tests: file %s\n", files[i]);
            exit(1);
        }
    }

    for (; i < sizeof(files) / sizeof(files[0]); i++) {
        char *s = obj_name(prefix, files[i]);
        CELL c;
        res = try(s, 0);
        free(s);
        printf(" should be %d\n", correct[i]);
        printf("Word 0 of memory is %"PRIX32"; should be 1020304\n", (UCELL)(load_cell(0, &c), c));
        if ((load_cell(0, &c), c) != 0x1020304) {
            printf("Error in load_object() tests: file %s\n", files[i]);
            exit(1);
        }
        if (res != correct[i]) {
            printf("Error in load_object() tests: file %s\n", files[i]);
            exit(1);
        }
        memset(memory, 0, MEMORY); // Zero memory for next test
    }

    printf("load_object() tests ran OK\n");
    return 0;
}
