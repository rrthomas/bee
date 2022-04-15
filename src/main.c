// Standalone interpreter.
//
// (c) Reuben Thomas 1995-2022
//
// The package is distributed under the GNU General Public License version 3,
// or, at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#include "progname.h"
#include "xvasprintf.h"

#include "bee/bee.h"

#include "gdb-stub.h"


#define DEFAULT_MEMORY 1048576 // Default size of VM memory in words (4MB)
#define MAX_MEMORY 1073741824 // Maximum size of memory in words (4GB)
static bee_uword_t memory_size = DEFAULT_MEMORY; // Size of VM memory in words
bee_word_t *memory;

static bool gdb_target = false;
static int gdb_fdin = STDIN_FILENO, gdb_fdout = STDOUT_FILENO;

static _GL_ATTRIBUTE_FORMAT_PRINTF_STANDARD(1, 0) void verror(const char *format, va_list args)
{
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
}

static _GL_ATTRIBUTE_FORMAT_PRINTF_STANDARD(1, 2) void die(const char *format, ...)
{
    va_list args;

    va_start(args, format);
    verror(format, args);
    va_end(args);
    exit(1);
}


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

// Load an object file
static bool load_object(FILE *fp, bee_word_t *ptr)
{
    off_t len;
    return fp != NULL &&
        skip_hashbang(fp) != -1 &&
        (len = fleno(fp)) >= 0 &&
        (off_t)fread(ptr, 1, len, fp) == len &&
        fclose(fp) != EOF;
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

#define COPYRIGHT_STRING "(c) Reuben Thomas 1994-2022"

static void usage(void)
{
    char *shortopt, *buf;
    printf ("Usage: %s [OPTION...] OBJECT-FILE [ARGUMENT...]\n"
            "\n"
            "Run " PACKAGE_NAME ".\n"
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

static bee_uword_t parse_number(bee_uword_t min, bee_uword_t max, char **end, const char *type)
{
    char *endptr;
    unsigned long size = strtoul(optarg, &endptr, 10);
    if (*optarg == '\0' || (end == NULL && *endptr != '\0') ||
        (bee_uword_t)size < min || (bee_uword_t)size > max)
        die("%s must be a positive number between %zu and %zu", type, min, max);
    if (end != NULL)
        *end = endptr;
    return (bee_uword_t)size;
}

int main(int argc, char *argv[])
{
    set_program_name(argv[0]);

    bee_word_t stack_size = BEE_DEFAULT_STACK_SIZE;
    bee_word_t return_stack_size = BEE_DEFAULT_STACK_SIZE;

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
                memory_size = parse_number(1, (bee_uword_t)MAX_MEMORY, NULL, "memory size");
                break;
            case 1:
                stack_size = parse_number(1, (bee_uword_t)MAX_MEMORY, NULL, "stack size");
                break;
            case 2:
                return_stack_size = parse_number(1, (bee_uword_t)MAX_MEMORY, NULL, "stack size");
                break;
            case 3:
                gdb_target = true;
                if (optarg != NULL) {
                    char *end;
                    gdb_fdin = (int)parse_number(0, (bee_uword_t)INT_MAX, &end, "file descriptor");
                    if (*end == ',') {
                        optarg = end + 1;
                        gdb_fdout = (int)parse_number(0, (bee_uword_t)INT_MAX, NULL, "file descriptor");
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
                printf(PACKAGE_NAME " " VERSION " (%d-bit, %s)\n"
                       COPYRIGHT_STRING "\n"
                       PACKAGE_NAME " comes with ABSOLUTELY NO WARRANTY.\n"
                       "You may redistribute copies of " PACKAGE_NAME "\n"
                       "under the terms of the GNU General Public License.\n"
                       "For more information about these matters, see the file named COPYING.\n",
                       BEE_WORD_BIT,
#ifdef HAVE_MIJIT
                       "Mijit JIT"
#else
                       "C Interpreter"
#endif
                       );
                exit(EXIT_SUCCESS);
            default:
                break;
            }
    }

    if ((memory = (bee_word_t *)calloc(memory_size, BEE_WORD_BYTES)) == NULL)
        die("could not allocate %zu words of memory", memory_size);
    bee_state * restrict S = bee_init(memory, stack_size, return_stack_size);
    if (S == NULL)
        die("could not allocate Bee state");

    argc -= optind;
    if (argc < 1) {
        usage();
        exit(EXIT_FAILURE);
    }

    bee_register_args(argc, (const char **)(argv + optind));
    FILE *handle = fopen(argv[optind], "rb");
    if (handle == NULL)
        die("cannot not open file %s", argv[optind]);
    if (!load_object(handle, memory))
        die("could not read file %s, or file is invalid", argv[optind]);

    if (gdb_target == true) {
        gdb_run(S);
        exit(EXIT_SUCCESS);
    } else
        return bee_run(S);
}
