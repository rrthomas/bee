// Standalone interpreter.
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
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#include "progname.h"
#include "xvasprintf.h"

#include "bee/bee.h"
#include "bee/aux.h"

#include "debug.h"
#include "gdb-stub.h"


#define DEFAULT_MEMORY 1048576 // Default size of VM memory in words (4MB)
#define MAX_MEMORY 1073741824 // Maximum size of memory in words (4GB)
static UWORD memory_size = DEFAULT_MEMORY; // Size of VM memory in words
WORD *memory;

static bool gdb_target = false;
static int gdb_fdin = STDIN_FILENO, gdb_fdout = STDOUT_FILENO;

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
    shortopt = xasprintf(", -%c ", shortname);                           \
    buf = xasprintf("--%s%s%s", longname, shortname ? shortopt : (arg == optional_argument ? "=" : ""), argstring); \
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

static UWORD parse_number(UWORD min, UWORD max, char **end, const char *type)
{
    char *endptr;
    unsigned long size = strtoul(optarg, &endptr, 10);
    if (*optarg == '\0' || (end == NULL && *endptr != '\0') ||
        (UWORD)size < min || (UWORD)size > max)
        die("%s must be a positive number between %"PRIu32 " and %"PRIu32, type, min, max);
    if (end != NULL)
        *end = endptr;
    return (UWORD)size;
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
                memory_size = parse_number(1, (UWORD)MAX_MEMORY, NULL, "memory size");
                break;
            case 1:
                stack_size = parse_number(1, (UWORD)MAX_MEMORY, NULL, "stack size");
                break;
            case 2:
                return_stack_size = parse_number(1, (UWORD)MAX_MEMORY, NULL, "stack size");
                break;
            case 3:
                gdb_target = true;
                if (optarg != NULL) {
                    char *end;
                    gdb_fdin = (int)parse_number(0, (UWORD)INT_MAX, &end, "file descriptor");
                    if (*end == ',') {
                        optarg = end + 1;
                        gdb_fdout = (int)parse_number(0, (UWORD)INT_MAX, NULL, "file descriptor");
                    } else
                        die("option '--gdb' takes two comma-separated file descriptors");
                    if (gdb_fdin == gdb_fdout)
                        die("file descriptors given to option '--gdb' must be different");
                }
                if (gdb_init(gdb_fdin, gdb_fdout))
                    die("option '--gdb': could not open file descriptors");
                break;
            case 4:
                usage();
                exit(EXIT_SUCCESS);
            case 5:
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
    bee_init(memory, memory_size, stack_size, return_stack_size);

    argc -= optind;
    if (argc < 1) {
        usage();
        exit(EXIT_FAILURE);
    }

    bee_register_args(argc, (const char **)(argv + optind));
    FILE *handle = fopen(argv[optind], "rb");
    if (handle == NULL)
        die("cannot not open file %s", argv[optind]);
    if (bee_load_object(handle, M0) != 0)
        die("could not read file %s, or file is invalid", argv[optind]);

    if (gdb_target == true) {
        for (int res = BEE_ERROR_BREAK; handle_exception(res); res = bee_run())
            ;
        exit(EXIT_SUCCESS);
    } else
        return bee_run();
}
