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
#include <setjmp.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/wait.h>
#include <libgen.h>
#include <glob.h>

#include "progname.h"
#include "xalloc.h"
#include "xvasprintf.h"

#include "bee.h"
#include "bee_aux.h"
#include "bee_debug.h"
#include "bee_opcodes.h"


#define DEFAULT_MEMORY 1048576 // Default size of VM memory in words (4MB)
#define MAX_MEMORY 1073741824 // Maximum size of memory in words (4GB)
static UWORD memory_size = DEFAULT_MEMORY; // Size of VM memory in words
WORD *memory;

static bool interactive;
static unsigned long lineno;
static jmp_buf env;

static bool debug_on_error = false;

static _GL_ATTRIBUTE_FORMAT_PRINTF(1, 0) void verror(const char *format, va_list args)
{
    if (!interactive)
        fprintf(stderr, "Bee:%lu: ", lineno);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
}

static _GL_ATTRIBUTE_FORMAT_PRINTF(1, 2) void warn(const char *format, ...)
{
    va_list args;

    va_start(args, format);
    verror(format, args);
    va_end(args);
}

static _GL_ATTRIBUTE_FORMAT_PRINTF(1, 2) void fatal(const char *format, ...)
{
    va_list args;

    va_start(args, format);
    verror(format, args);
    va_end(args);
    longjmp(env, 1);
}

static _GL_ATTRIBUTE_FORMAT_PRINTF(1, 2) void die(const char *format, ...)
{
    va_list args;

    va_start(args, format);
    verror(format, args);
    va_end(args);
    exit(1);
}

static const char *command[] = {
#define C(cmd) #cmd,
#include "tbl_commands.h"
#undef C
};
enum commands {
#define C(cmd) c_##cmd,
#include "tbl_commands.h"
#undef C
};
static int commands = sizeof(command) / sizeof(*command);

static const char *regist[] = {
#define R(reg) #reg,
#include "tbl_registers.h"
#undef R
};
enum registers {
#define R(reg) r_##reg,
#include "tbl_registers.h"
#undef R
};
static int registers = sizeof(regist) / sizeof(*regist);


static const char *globfile(const char *file)
{
    static glob_t globbuf;
    static bool first_time = true;
    if (!first_time)
        globfree(&globbuf);
    first_time = false;

    if (glob(file, GLOB_TILDE_CHECK, NULL, &globbuf) != 0)
        fatal("cannot find file '%s'", file);
    else if (globbuf.gl_pathc != 1)
        fatal("'%s' matches more than one file", file);
    return globbuf.gl_pathv[0];
}

static const char *globdirname(const char *file)
{
    static char *globbed_file = NULL;
    free(globbed_file);

    if (strchr(file, '/') == NULL)
        return file;

    char *filecopy = xstrdup(file);
    const char *dir = globfile(dirname(filecopy));
    free(filecopy);
    filecopy = xstrdup(file);
    char *base = basename(filecopy);
    globbed_file = xasprintf("%s/%s", dir, base);
    free(filecopy);

    return globbed_file;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsuggest-attribute=pure"
static void check_aligned(uint8_t *adr)
{
    if (!IS_ALIGNED(adr))
        fatal("Address must be word-aligned");
}
#pragma GCC diagnostic pop

static void check_range(uint8_t *start, uint8_t *end, const char *quantity)
{
    if (start > end)
        fatal("start address cannot be greater than end address");
    if (!address_range_valid((uint8_t *)start, end - start))
        fatal("%s is invalid", quantity);
}

static void upper(char *s)
{
    size_t len = strlen(s);

    for (size_t i = 0; i < len; i++, s++)
        *s = toupper(*s);
}

static size_t search(const char *token, const char *list[], size_t entries)
{
    size_t len = strlen(token);

    for (size_t i = 0; i < entries; i++) {
        size_t entry_len = strlen(list[i]);
        if (entry_len > 1 && len == 1)
            continue;
        if (strncmp(token, list[i], len) == 0)
            return i;
    }

    return SIZE_MAX;
}

static uint8_t parse_instruction(const char *token)
{
    uint8_t opcode = O_UNDEFINED;
    if (token[0] == 'O') {
        opcode = toass(token + 1);
        if (opcode == O_UNDEFINED)
            fatal("invalid opcode");
    }
    return opcode;
}

static unsigned long parse_number(const char *s, char **endp)
{
    return s[0] == '$' ? strtoul(s + 1, endp, 16) : strtoul(s, endp, 10);
}

static int is_byte(long value)
{
    return (long)(uint8_t)value == value;
}

static long single_arg(const char *s)
{
    if (s == NULL)
        fatal("too few arguments");

    char *endp;
    long n = parse_number(s, &endp);

    if (endp != &s[strlen(s)])
        fatal("invalid number");

    return n;
}

static void double_arg(char *s, long *start, long *len)
{
    char *token;
    static char *copy = NULL;
    free(copy);
    copy = NULL;
    if (s != NULL && (token = strtok((copy = xstrdup(s)), " +")) != NULL) {
        size_t i;
        for (i = strlen(token); s[i] == ' ' && i < strlen(s); i++)
            ;

        *start = single_arg(token);

        if ((token = strtok(NULL, " +")) != NULL)
            *len = single_arg(token);
    }
}


static void disassemble(WORD *start, WORD *end)
{
    for (WORD *p = start; p < end && p <= (WORD *)-1 - 1; p ++) {
        printf("$%08"PRIX32": ", (UWORD)p);
        WORD a;
        load_word(p, &a);
        printf("%s\n", disass(a, p));
    }
}


static int save_object(FILE *file, WORD *ptr, UWORD length)
{
    if (!IS_ALIGNED(ptr) || !address_range_valid((uint8_t *)ptr, length))
        return -1;

    if (fwrite(ptr, WORD_BYTES, length, file) != length)
        return -2;

    return 0;
}


static void do_assign(char *token)
{
    char *number = strtok(NULL, " ");
    long value;

    upper(number);
    value = parse_instruction(number);
    value = single_arg(number);

    int no = search(token, regist, registers);
    switch (no) {
        case r_MEMORY:
            fatal("cannot assign to %s", regist[no]);
            break;
        case r_PC:
            PC = (WORD *)value;
            ass_goto(PC);
            break;
        case r_RP:
            RP = value;
            break;
        case r_HANDLER_RP:
            HANDLER_RP = value;
            break;
        case r_SP:
            SP = value;
            break;
        default:
            {
                uint8_t *adr = (uint8_t *)(unsigned long)single_arg(token);

                if (!IS_ALIGNED(adr) && !is_byte(value))
                    fatal("only a byte can be assigned to an unaligned address");
                if (is_byte(value)) {
                    check_range(adr, adr + 1, "Address");
                    store_byte(adr, value);
                } else {
                    check_range(adr, adr + WORD_BYTES, "Address");
                    if ((unsigned long)value > (UWORD)-1)
                        fatal("the value assigned to a word must fit in a word");
                    store_word((WORD *)adr, value);
                }
            }
    }
}

static void do_display(size_t no, const char *format)
{
    char *display;

    switch (no) {
        case r_PC:
            display = xasprintf("PC = $%p", PC);
            break;
        case r_MEMORY:
            display = xasprintf("MEMORY = $%"PRIX32" (%"PRIu32")", MEMORY, MEMORY);
            break;
        case r_RP:
            display = xasprintf("RP = $%"PRIX32" (%"PRIu32")", RP, RP);
            break;
        case r_HANDLER_RP:
            display = xasprintf("HANDLER_RP = $%"PRIX32" (%"PRIu32")", HANDLER_RP, HANDLER_RP);
            break;
        case r_SP:
            display = xasprintf("SP = $%"PRIX32" (%"PRIu32")", SP, SP);
            break;
        default:
            display = xasprintf("unknown register");
            break;
    }
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
    printf(format, display);
#pragma GCC diagnostic pop
    free(display);
}

static void do_info(void)
{
    do_display(r_PC, "%-25s");
    WORD c;
    load_word(PC, &c);
    printf("%-16s\n", disass((UWORD)c, PC));
    show_data_stack();
    show_return_stack();
}

static void do_command(int no)
{
    int error = 0;

    switch (no) {
    case c_TOD:
        {
            long value = single_arg(strtok(NULL, " "));
            PUSH(value);
        }
        break;
    case c_TOR:
        {
            long value = single_arg(strtok(NULL, " "));
            PUSH_RETURN(value);
        }
        break;
    case c_DISASSEMBLE:
        {
            long start = (long)(((PC - M0) * WORD_BYTES <= 16 ? (uint8_t *)M0 : (uint8_t *)PC - 16));
            long len = 64;
            double_arg(strtok(NULL, ""), &start, &len);
            check_aligned((uint8_t *)start);
            check_aligned((uint8_t *)len);
            check_range((uint8_t *)start, (uint8_t *)start + len, "Address");
            disassemble((WORD *)start, (WORD *)((uint8_t *)start + len));
        }
        break;
    case c_DFROM:
        {
            if (SP == 0)
                fatal("data stack is empty");
            WORD value = S0[--SP];
            printf("%"PRId32" ($%"PRIX32")\n", value, (UWORD)value);
        }
        break;
    case c_DUMP:
        {
            long start = (long)(((PC - M0) * WORD_BYTES <= 64 ? (uint8_t*)M0 : (uint8_t *)PC - 64));
            long len = 256;
            double_arg(strtok(NULL, ""), &start, &len);
            long end = start + len;
            check_range((uint8_t *)start, (uint8_t *)end, "Address");
            while (start < end) {
                printf("$%08lX ", (unsigned long)start);
                // Use #define to avoid a variable-length array
                #define chunk 16
                char ascii[chunk];
                for (int i = 0; i < chunk && start < end; i++) {
                    uint8_t byte;
                    load_byte((uint8_t *)start + i, &byte);
                    if (i % 8 == 0)
                        putchar(' ');
                    printf("%02X ", byte);
                    ascii[i] = isprint(byte) ? byte : '.';
                }
                start += chunk;
                printf(" |%.*s|\n", chunk, ascii);
            }
        }
        break;
    case c_INFO:
        do_info();
        break;
    case c_LOAD:
        {
            const char *file = strtok(NULL, " ");
            if (file == NULL)
                fatal("LOAD requires a file name");
            UWORD adr = (UWORD)M0;
            char *arg = strtok(NULL, " ");
            if (arg != NULL)
                adr = single_arg(arg);

            FILE *handle = fopen(globfile(file), "rb");
            if (handle == NULL)
                fatal("cannot open file '%s'", file);
            int ret = load_object(handle, (WORD *)adr);

            switch (ret) {
            case -1:
                fatal("address out of range or unaligned, or module too large");
                break;
            case -2:
                fatal("error while loading module");
                break;
            default:
                break;
            }
        }
        break;
    case c_QUIT:
        exit(0);
    case c_RFROM:
        {
            WORD value;
            POP_RETURN(&value);
            printf("$%"PRIX32" (%"PRId32")\n", (UWORD)value, value);
        }
        break;
    case c_RUN:
        {
            WORD ret = run();
            printf("HALT code %"PRId32" (%s) was returned\n", ret, error_to_msg(ret));
        }
        break;
    case c_STEP:
    case c_TRACE:
        {
            char *arg = strtok(NULL, " ");
            WORD ret = ERROR_BREAK;

            if (arg == NULL) {
                if (no == c_TRACE) do_info();
                if ((ret = single_step()))
                    printf("HALT code %"PRId32" (%s) was returned\n", ret, error_to_msg(ret));
            } else {
                upper(arg);
                if (strcmp(arg, "TO") == 0) {
                    unsigned long limit = (unsigned long)single_arg(strtok(NULL, " "));
                    check_range((uint8_t *)limit, (uint8_t *)limit, "Address");
                    check_aligned((uint8_t *)limit);
                    while ((unsigned long)PC != limit && ret == ERROR_BREAK) {
                        if (no == c_TRACE) do_info();
                        ret = single_step();
                    }
                    if (ret != 0)
                        printf("HALT code %"PRId32" (%s) was returned at PC = $%p\n",
                               ret, error_to_msg(ret), PC);
                } else {
                    unsigned long limit = (unsigned long)single_arg(arg), i;
                    for (i = 0; i < limit && ret == ERROR_BREAK; i++) {
                        if (no == c_TRACE) do_info();
                        ret = single_step();
                    }
                    if (ret != 0)
                        printf("HALT code %"PRId32" (%s) was returned after %lu "
                               "steps\n", ret, error_to_msg(ret), i);
                }
            }
        }
        break;
    case c_SAVE:
        {
            const char *file = strtok(NULL, " ");
            long start = 0, len = 0;
            double_arg(strtok(NULL, ""), &start, &len);
            if (len == 0) {
                len = start;
                start = (long)M0;
            }

            FILE *handle;
            if ((handle = fopen(globdirname(file), "wb")) == NULL)
                fatal("cannot open file %s", file);
            int ret = save_object(handle, (WORD *)start, (UWORD)(len / WORD_BYTES));
            fclose(handle);

            switch (ret) {
            case -1:
                fatal("save area contains an invalid address");
                break;
            case -2:
                fatal("error while saving module");
                break;
            default:
                break;
            }
        }
        break;
    case c_BLITERAL:
        {
            long value = single_arg(strtok(NULL, " "));
            if (!is_byte(value))
                fatal("the argument to BLITERAL must fit in a byte");
            ass_byte((uint8_t)value);
            break;
        }
    case c_CALL:
        {
            unsigned long adr = (unsigned long)single_arg(strtok(NULL, " "));
            check_aligned((uint8_t *)adr);
            check_range((uint8_t *)adr, (uint8_t *)adr + 1, "Address");
            call((WORD *)adr);
            break;
        }
    case c_PUSH:
        {
            long value = single_arg(strtok(NULL, " "));
            WORD stored_val = ARSHIFT((WORD)value << 2, 2);
            if ((long)stored_val != value)
                fatal("invalid argument to PUSH");
            push(value);
            break;
        }
    case c_PUSHREL:
        {
            unsigned long adr = (unsigned long)single_arg(strtok(NULL, " "));
            check_aligned((uint8_t *)M0 + adr);
            check_range((uint8_t *)M0 + adr, (uint8_t *)M0 + adr + WORD_BYTES, "Address");
            pushrel((WORD *)((uint8_t *)M0 + adr));
            break;
        }
    default: // This cannot happen
        break;
    }

 error:
    switch (error) {
    case ERROR_INVALID_LOAD:
    case ERROR_INVALID_STORE:
        fatal("invalid address");
        break;
    case ERROR_UNALIGNED_ADDRESS:
        fatal("address alignment error");
        break;
    default:
    case ERROR_OK:
        break;
    }
}


static void parse(char *input)
{
    // Handle shell command
    if (input[0] == '!') {
        int result = system(input + 1);
        if (result == -1)
            fatal("could not run command");
        else if (result != 0 && WIFEXITED(result))
            fatal("command exited with value %d", WEXITSTATUS(result));
        return;
    }

    // Hide any comment from the parser
    char *comment = strstr(input, "//");
    if (comment != NULL)
        *comment = '\0';

    static char *copy = NULL;
    free(copy);
    copy = xstrdup(input);
    char *token = strtok(copy, strchr(copy, '=') == NULL ? " " : "=");
    if (token == NULL || strlen(token) == 0) return;
    upper(token);

    size_t i;
    for (i = strlen(token); input[i] == ' ' && i < strlen(input); i++)
        ;

    bool assign = false;
    if (i < strlen(input))
        assign = input[i] == '=';

    size_t no = search(token, command, commands);
    if (no != SIZE_MAX)
        do_command(no);
    else {
        if (assign)
            do_assign(token);
        else {
            uint8_t opcode = parse_instruction(token);
            if (opcode != O_UNDEFINED) {
                ass(opcode);
                return;
            }

            no = search(token, regist, registers);
            if (no == SIZE_MAX) {
                char *endp, *display;
                UWORD adr = (UWORD)parse_number(token, &endp);

                if (endp != &token[strlen(token)])
                    fatal("unknown command or register '%s'", token);

                if (!IS_ALIGNED(adr)) {
                    check_range((uint8_t *)adr, (uint8_t *)adr + 1, "Address");
                    uint8_t b;
                    load_byte((uint8_t *)adr, &b);
                    display = xasprintf("$%"PRIX32": $%X (%d) (byte)", (UWORD)adr,
                                        b, b);
                } else {
                    check_range((uint8_t *)adr, (uint8_t *)adr + WORD_BYTES, "Address");
                    WORD c;
                    load_word((WORD *)adr, &c);
                    display = xasprintf("$%"PRIX32": $%"PRIX32" (%"PRId32") (word)", (UWORD)adr,
                                        (UWORD)c, c);
                }
                printf("%s\n", display);
                free(display);
            } else
                do_display(no, "%s\n");
        }
    }
}


static _GL_ATTRIBUTE_FORMAT_PRINTF(1, 2) void interactive_printf(const char *format, ...)
{
    if (interactive == false)
        return;

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
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

#define VERSION_STRING "Bee shell (C Bee release "PACKAGE_VERSION")"
#define COPYRIGHT_STRING "(c) Reuben Thomas 1994-2020"

static void usage(void)
{
    char *shortopt, *buf;
    printf ("Usage: %s [OPTION...] [OBJECT-FILE ARGUMENT...]\n"
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
    interactive = isatty(fileno(stdin));

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
        else if (c == 'd')
            longindex = 3;
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
                debug_on_error = true;
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
    init(memory, memory_size, stack_size, return_stack_size);
    ass_goto(PC);

    argc -= optind;
    if (argc >= 1) {
        if (register_args(argc, (const char **)(argv + optind)) != 0)
            die("could not register command-line arguments");
        FILE *handle = fopen(argv[optind], "rb");
        if (handle == NULL)
            die("cannot not open file %s", argv[optind]);
        if (load_object(handle, M0) != 0)
            die("could not read file %s, or file is invalid", argv[optind]);

        int res = run();
        if (!debug_on_error || res >= 0)
            return res;
        warn("error %d (%s) raised", res, error_to_msg(res));
    } else
        interactive_printf("%s\n%s\n\n", VERSION_STRING, COPYRIGHT_STRING);

    while (1) {
        int jmp_val = setjmp(env);
        if (jmp_val == 0) {
            static char *input = NULL;
            static size_t len = 0;
            interactive_printf(">");
            if (getline(&input, &len, stdin) == -1) {
                if (feof(stdin)) {
                    interactive_printf("\n"); // Empty line after prompt
                    exit(EXIT_SUCCESS);
                }
                die("input error");
            }
            lineno++;
            char *nl;
            if ((nl = strrchr(input, '\n')))
                *nl = '\0';
            parse(input);
        } else if (interactive == false)
            exit(jmp_val);
    }
}
