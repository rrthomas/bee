// Front-end and shell.
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

#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <inttypes.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#include "progname.h"
#include "xvasprintf.h"

#include "bee.h"
#include "bee_aux.h"
#include "bee_debug.h"
#include "bee_opcodes.h"


#define DEFAULT_MEMORY 1048576 // Default size of VM memory in words (4MB)
#define MAX_MEMORY 1073741824 // Maximum size of memory in words (4GB)
static UWORD memory_size = DEFAULT_MEMORY; // Size of VM memory in words
WORD *memory;


static _GL_ATTRIBUTE_FORMAT_PRINTF(1, 0) void verror(const char *format, va_list args)
{
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
}

static _GL_ATTRIBUTE_FORMAT_PRINTF(1, 2) void die(const char *format, ...)
{
    va_list args;

    va_start(args, format);
    verror(format, args);
    va_end(args);
    exit(1);
}


// Options table
struct option longopts[] = {
#define OPT(longname, shortname, arg, argstring, docstring) \
  {longname, arg, NULL, shortname},
#define ARG(argstring, docstring)
#define DOC(docstring)
#include "cmdline.h"
#undef OPT
#undef ARG
#undef DOC
  {0, 0, 0, 0}
};

#define COPYRIGHT_STRING "(c) Reuben Thomas 1994-2020"

static void usage(void)
{
    char *shortopt, *buf;
    printf ("Usage: %s [OPTION...] OBJECT-FILE [ARGUMENT...]\n"
            "\n"
            "Run Bee.\n"
            "\n",
            program_name);
#define OPT(longname, shortname, arg, argstring, docstring)             \
    shortopt = xasprintf(", -%c", shortname);                           \
    buf = xasprintf("--%s%s %s", longname, shortname ? shortopt : "", argstring); \
    printf("  %-26s%s\n", buf, docstring);                              \
    free(buf);                                                          \
    free(shortopt);
#define ARG(argstring, docstring)                 \
    printf("  %-26s%s\n", argstring, docstring);
#define DOC(text)                                 \
    printf(text "\n");
#include "cmdline.h"
#undef OPT
#undef ARG
#undef DOC
}

static WORD parse_size(UWORD max)
{
    char *endptr;
    errno = 0;
    long size = (WORD)strtol(optarg, &endptr, 10);
    if (*optarg == '\0' || *endptr != '\0' || size <= 0 || (UWORD)size > max)
        die("memory size must be a positive number up to %"PRIu32, max);
    return size;
}

int main(int argc, char *argv[])
{
    set_program_name(argv[0]);

    WORD stack_size = DEFAULT_STACK_SIZE;
    WORD return_stack_size = DEFAULT_STACK_SIZE;

    // Options string starts with '+' to stop option processing at first non-option, then
    // leading ':' so as to return ':' for a missing arg, not '?'
    for (;;) {
        int this_optind = optind ? optind : 1, longindex = -1;
        int c = getopt_long(argc, argv, "+:dm:r:s:", longopts, &longindex);

        if (c == -1)
            break;
        else if (c == ':')
            die("option '%s' requires an argument", argv[this_optind]);
        else if (c == '?')
            die("unrecognised option '%s'\nTry '%s --help' for more information.", argv[this_optind], program_name);
        else if (c == 'r')
            longindex = 2;
        else if (c == 's')
            longindex = 1;
        else if (c == 'm')
            longindex = 0;

        switch (longindex) {
            case 0:
                memory_size = parse_size((UWORD)MAX_MEMORY);
                break;
            case 1:
                stack_size = parse_size((UWORD)MAX_MEMORY);
                break;
            case 2:
                return_stack_size = parse_size((UWORD)MAX_MEMORY);
                break;
            case 3:
                usage();
                exit(EXIT_SUCCESS);
            case 4:
                printf("Bee " VERSION "\n"
                       COPYRIGHT_STRING "\n"
                       "Bee comes with ABSOLUTELY NO WARRANTY.\n"
                       "You may redistribute copies of Bee\n"
                       "under the terms of the GNU General Public License.\n"
                       "For more information about these matters, see the file named COPYING.\n");
                exit(EXIT_SUCCESS);
            default:
                break;
            }
    }

    if ((memory = (WORD *)calloc(memory_size, WORD_BYTES)) == NULL)
        die("could not allocate %"PRIu32" words of memory", memory_size);
    init(memory, memory_size, stack_size, return_stack_size);

    argc -= optind;
    if (argc < 1) {
        usage();
        exit(EXIT_FAILURE);
    }

    if (register_args(argc, (const char **)(argv + optind)) != 0)
        die("could not register command-line arguments");
    FILE *handle = fopen(argv[optind], "rb");
    if (handle == NULL)
        die("cannot not open file %s", argv[optind]);
    if (load_object(handle, M0) != 0)
        die("could not read file %s, or file is invalid", argv[optind]);

    return run();
}
