// The interface calls run() : integer and single_step() : integer.
//
// (c) Reuben Thomas 1994-2020
//
// The package is distributed under the GNU Public License version 3, or,
// at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#include "config.h"

#include "external_syms.h"

#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include "binary-io.h"
#include "minmax.h"
#include "verify.h"

#include "bee.h"
#include "bee_aux.h"
#include "private.h"
#include "bee_opcodes.h"


// Assumption for file functions
verify(sizeof(int) <= sizeof(CELL));


// Check whether a VM address points to a native cell-aligned cell
#define IS_VALID(a)                                     \
    (native_address_of_range((a), CELL_W) != NULL)

#define CHECK_VALID_CELL(a)                             \
    CHECK_ADDRESS(a, IS_ALIGNED(a), -23, exception)     \
    CHECK_ADDRESS(a, IS_VALID(a), -9, exception)


// Division macros
#define DIV_CATCH_ZERO(a, b) ((b) == 0 ? 0 : (a) / (b))
#define MOD_CATCH_ZERO(a, b) ((b) == 0 ? (a) : (a) % (b))
#define DIV_WOULD_OVERFLOW(a, b) (((a) == CELL_MIN) && ((b) == -1))
#define DIV_WITH_OVERFLOW(a, b) (DIV_WOULD_OVERFLOW((a), (b)) ? CELL_MIN : DIV_CATCH_ZERO((a), (b)))
#define MOD_WITH_OVERFLOW(a, b) (DIV_WOULD_OVERFLOW((a), (b)) ? 0 : MOD_CATCH_ZERO((a), (b)))


// I/O support

// Copy a string from VM to native memory
static int getstr(UCELL adr, UCELL len, char **res)
{
    int exception = 0;

    *res = calloc(1, len + 1);
    if (*res == NULL)
        exception = -511;
    else
        for (size_t i = 0; exception == 0 && i < len; i++, adr++) {
            exception = load_byte(adr, (BYTE *)((*res) + i));
        }

    return exception;
}

// Convert portable open(2) flags bits to system flags
static int getflags(UCELL perm, bool *binary)
{
    int flags = 0;

    switch (perm & 3) {
    case 0:
        flags = O_RDONLY;
        break;
    case 1:
        flags = O_WRONLY;
        break;
    case 2:
        flags = O_RDWR;
        break;
    default:
        break;
    }
    if (perm & 4)
        flags |= O_CREAT | O_TRUNC;

    if (perm & 8)
        *binary = true;

    return flags;
}

// Register command-line args
static int main_argc = 0;
static const char **main_argv;
static UCELL *main_argv_len;
int register_args(int argc, const char *argv[])
{
    main_argc = argc;
    main_argv = argv;
    if ((main_argv_len = calloc(argc, sizeof(UCELL))) == NULL)
        return -1;

    for (int i = 0; i < argc; i++) {
        size_t len = strlen(argv[i]);
        if (len > CELL_MAX)
            return -2;
        main_argv_len[i] = len;
    }
    return 0;
}

// Inner execution function
static CELL run_or_step(bool run)
{
    int exception = 0;
    do {
        CELL temp = 0, temp2 = 0;
        BYTE byte = 0;

        A = LOAD_CELL(EP);
        EP += CELL_W;

        if ((A & 1) == 0) {
            PUSH_RETURN(EP);
            CHECK_VALID_CELL(EP + A);
            EP += A - CELL_W;
        } else
            switch (A >> 1) {
            case O_DROP:
                (void)POP;
                break;
            case O_PICK:
                {
                    CELL depth = POP;
                    CELL pickee = LOAD_CELL(SP - depth * CELL_W * STACK_DIRECTION);
                    PUSH(pickee);
                }
                break;
            case O_ROLL:
                {
                    CELL depth = POP;
                    CELL rollee = LOAD_CELL(SP - depth * CELL_W * STACK_DIRECTION);
                    for (int i = depth; i > 0; i--)
                        STORE_CELL(SP - i * CELL_W * STACK_DIRECTION,
                                   LOAD_CELL(SP - (i - 1) * CELL_W * STACK_DIRECTION));
                    STORE_CELL(SP, rollee);
                }
                break;
            case O_TOR:
                {
                    CELL value = POP;
                    PUSH_RETURN(value);
                }
                break;
            case O_RFROM:
                {
                    CELL value = POP_RETURN;
                    PUSH(value);
                }
                break;
            case O_RFETCH:
                {
                    CELL value = LOAD_CELL(RP);
                    PUSH(value);
                }
                break;
            case O_LESS:
                {
                    CELL a = POP;
                    CELL b = POP;
                    PUSH(b < a ? BEE_TRUE : BEE_FALSE);
                }
                break;
            case O_EQUAL:
                {
                    CELL a = POP;
                    CELL b = POP;
                    PUSH(a == b ? BEE_TRUE : BEE_FALSE);
                }
                break;
            case O_ULESS:
                {
                    UCELL a = POP;
                    UCELL b = POP;
                    PUSH(b < a ? BEE_TRUE : BEE_FALSE);
                }
                break;
            case O_CELL:
                PUSH(CELL_W);
                break;
            case O_PLUS:
                {
                    CELL a = POP;
                    CELL b = POP;
                    PUSH(b + a);
                }
                break;
            case O_STAR:
                {
                    CELL multiplier = POP;
                    CELL multiplicand = POP;
                    PUSH(multiplier * multiplicand);
                }
                break;
            case O_USLASHMOD:
                {
                    UCELL divisor = POP;
                    UCELL dividend = POP;
                    PUSH(MOD_CATCH_ZERO(dividend, divisor));
                    PUSH(DIV_CATCH_ZERO(dividend, divisor));
                }
                break;
            case O_SSLASHREM:
                {
                    CELL divisor = POP;
                    CELL dividend = POP;
                    PUSH(MOD_WITH_OVERFLOW(dividend, divisor));
                    PUSH(DIV_WITH_OVERFLOW(dividend, divisor));
                }
                break;
            case O_NEGATE:
                {
                    CELL a = POP;
                    PUSH(-a);
                }
                break;
            case O_INVERT:
                {
                    CELL a = POP;
                    PUSH(~a);
                }
                break;
            case O_AND:
                {
                    CELL a = POP;
                    CELL b = POP;
                    PUSH(a & b);
                }
                break;
            case O_OR:
                {
                    CELL a = POP;
                    CELL b = POP;
                    PUSH(a | b);
                }
                break;
            case O_XOR:
                {
                    CELL a = POP;
                    CELL b = POP;
                    PUSH(a ^ b);
                }
                break;
            case O_LSHIFT:
                {
                    CELL shift = POP;
                    CELL value = POP;
                    PUSH(shift < (CELL)CELL_BIT ? value << shift : 0);
                }
                break;
            case O_RSHIFT:
                {
                    CELL shift = POP;
                    CELL value = POP;
                    PUSH(shift < (CELL)CELL_BIT ? (CELL)((UCELL)value >> shift) : 0);
                }
                break;
            case O_FETCH:
                {
                    CELL addr = POP;
                    CELL value = LOAD_CELL(addr);
                    PUSH(value);
                }
                break;
            case O_STORE:
                {
                    CELL addr = POP;
                    CELL value = POP;
                    STORE_CELL(addr, value);
                }
                break;
            case O_CFETCH:
                {
                    CELL addr = POP;
                    BYTE value = LOAD_BYTE(addr);
                    PUSH((CELL)value);
                }
                break;
            case O_CSTORE:
                {
                    CELL addr = POP;
                    BYTE value = (BYTE)POP;
                    STORE_BYTE(addr, value);
                }
                break;
            case O_SPFETCH:
                {
                    CELL value = SP;
                    PUSH(value);
                }
                break;
            case O_SPSTORE:
                {
                    CELL value = POP;
                    CHECK_ALIGNED(value);
                    SP = value;
                }
                break;
            case O_RPFETCH:
                PUSH(RP);
                break;
            case O_RPSTORE:
                {
                    CELL value = POP;
                    CHECK_ALIGNED(value);
                    RP = value;
                }
                break;
            case O_BRANCH:
                {
                    CELL addr = POP;
                    CHECK_VALID_CELL(addr);
                    EP = addr;
                }
                break;
            case O_QBRANCH:
                {
                    CELL addr = POP;
                    if (POP == BEE_FALSE) {
                        CHECK_VALID_CELL(addr);
                        EP = addr;
                    }
                }
                break;
            case O_EXECUTE:
                {
                    CELL addr = POP;
                    CHECK_VALID_CELL(addr);
                    PUSH_RETURN(EP);
                    EP = addr;
                }
                break;
            case O_EXIT:
                {
                    CELL addr = POP_RETURN;
                    CHECK_VALID_CELL(addr);
                    EP = addr;
                }
                break;
            case O_LITERAL:
                PUSH(LOAD_CELL(EP));
                EP += CELL_W;
                break;
            case O_OFFSET:
                PUSH(EP - CELL_W + LOAD_CELL(EP));
                EP += CELL_W;
                break;
            case O_HALT:
                return POP;
            case O_S0FETCH:
                PUSH(S0);
                break;
            case O_S0STORE:
                {
                    CELL value = POP;
                    CHECK_ALIGNED(value);
                    S0 = value;
                }
                break;
            case O_R0FETCH:
                PUSH(R0);
                break;
            case O_R0STORE:
                {
                    CELL value = POP;
                    CHECK_ALIGNED(value);
                    R0 = value;
                }
                break;
            case O_MEMORYFETCH:
                PUSH(MEMORY);
                break;

            case OX_ARGC: // ( -- u )
                PUSH(main_argc);
                break;
            case OX_ARGLEN: // ( u1 -- u2 )
                {
                    UCELL narg = POP;
                    if (narg >= (UCELL)main_argc)
                        PUSH(0);
                    else
                        PUSH(main_argv_len[narg]);
                }
                break;
            case OX_ARGCOPY: // ( u1 addr -- )
                {
                    UCELL addr = POP;
                    UCELL narg = POP;
                    if (narg < (UCELL)main_argc) {
                        UCELL len = (UCELL)main_argv_len[narg];
                        char *ptr = (char *)native_address_of_range(addr, len);
                        if (ptr != NULL) {
                            UCELL end = ALIGN(addr + len);
                            pre_dma(addr, end);
                            strncpy(ptr, main_argv[narg], len);
                            post_dma(addr, end);
                        }
                    }
                }
                break;
            case OX_STDIN:
                PUSH((CELL)(STDIN_FILENO));
                break;
            case OX_STDOUT:
                PUSH((CELL)(STDOUT_FILENO));
                break;
            case OX_STDERR:
                PUSH((CELL)(STDERR_FILENO));
                break;
            case OX_OPEN_FILE:
                {
                    bool binary = false;
                    int perm = getflags(POP, &binary);
                    UCELL len = POP;
                    UCELL str = POP;
                    char *file;
                    exception = getstr(str, len, &file);
                    int fd = exception == 0 ? open(file, perm, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH) : -1;
                    free(file);
                    PUSH((CELL)fd);
                    PUSH(fd < 0 || (binary && set_binary_mode(fd, O_BINARY) < 0) ? -1 : 0);
                }
                break;
            case OX_CLOSE_FILE:
                {
                    int fd = POP;
                    PUSH((CELL)close(fd));
                }
                break;
            case OX_READ_FILE:
                {
                    int fd = POP;
                    UCELL nbytes = POP;
                    UCELL buf = POP;

                    ssize_t res = 0;
                    if (exception == 0) {
                        exception = pre_dma(buf, buf + nbytes);
                        if (exception == 0) {
                            res = read(fd, native_address_of_range(buf, 0), nbytes);
                            exception = post_dma(buf, buf + nbytes);
                        }
                    }

                    PUSH(res);
                    PUSH((exception == 0 && res >= 0) ? 0 : -1);
                }
                break;
            case OX_WRITE_FILE:
                {
                    int fd = POP;
                    UCELL nbytes = POP;
                    UCELL buf = POP;

                    ssize_t res = 0;
                    if (exception == 0) {
                        exception = pre_dma(buf, buf + nbytes);
                        if (exception == 0) {
                            res = write(fd, native_address_of_range(buf, 0), nbytes);
                            exception = post_dma(buf, buf + nbytes);
                        }
                    }

                    PUSH((exception == 0 && res >= 0) ? 0 : -1);
                }
                break;
            case OX_FILE_POSITION:
                {
                    int fd = POP;
                    off_t res = lseek(fd, 0, SEEK_CUR);
                    PUSH_DOUBLE((DUCELL)res);
                    PUSH(res >= 0 ? 0 : -1);
                }
                break;
            case OX_REPOSITION_FILE:
                {
                    int fd = POP;
                    DUCELL ud = POP_DOUBLE;
                    off_t res = lseek(fd, (off_t)ud, SEEK_SET);
                    PUSH(res >= 0 ? 0 : -1);
                }
                break;
            case OX_FLUSH_FILE:
                {
                    int fd = POP;
                    int res = fdatasync(fd);
                    PUSH(res);
                }
                break;
            case OX_RENAME_FILE:
                {
                    UCELL len1 = POP;
                    UCELL str1 = POP;
                    UCELL len2 = POP;
                    UCELL str2 = POP;
                    char *from;
                    char *to = NULL;
                    exception = getstr(str2, len2, &from) ||
                        getstr(str1, len1, &to) ||
                        rename(from, to);
                    free(from);
                    free(to);
                    PUSH(exception);
                }
                break;
            case OX_DELETE_FILE:
                {
                    UCELL len = POP;
                    UCELL str = POP;
                    char *file;
                    exception = getstr(str, len, &file) ||
                        remove(file);
                    free(file);
                    PUSH(exception);
                }
                break;
            case OX_FILE_SIZE:
                {
                    struct stat st;
                    int fd = POP;
                    int res = fstat(fd, &st);
                    PUSH_DOUBLE(st.st_size);
                    PUSH(res);
                }
                break;
            case OX_RESIZE_FILE:
                {
                    int fd = POP;
                    DUCELL ud = POP_DOUBLE;
                    int res = ftruncate(fd, (off_t)ud);
                    PUSH(res);
                }
                break;
            case OX_FILE_STATUS:
                {
                    struct stat st;
                    int fd = POP;
                    int res = fstat(fd, &st);
                    PUSH(st.st_mode);
                    PUSH(res);
                }
                break;

            default:
                exception = -256;
                goto exception;
            }
    } while (run == true && exception == 0);

 exception:
    if (exception == 0 && run == false)
        exception = -257; // single_step terminated OK
    return exception;
}

// Perform one pass of the execution cycle
CELL single_step(void)
{
    return run_or_step(false);
}

CELL run(void)
{
    return run_or_step(true);
}
