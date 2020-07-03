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
verify(sizeof(int) <= sizeof(WORD));


// Check whether a VM address points to a native word-aligned word
#define IS_VALID(a)                                     \
    (native_address_of_range((a), WORD_BYTES) != NULL)

#define CHECK_VALID_WORD(a)                                           \
    CHECK_ADDRESS(a, IS_ALIGNED(a), ERROR_UNALIGNED_ADDRESS, error)     \
    CHECK_ADDRESS(a, IS_VALID(a), ERROR_INVALID_LOAD, error)


// Division macros
#define DIV_CATCH_ZERO(a, b) ((b) == 0 ? 0 : (a) / (b))
#define MOD_CATCH_ZERO(a, b) ((b) == 0 ? (a) : (a) % (b))
#define DIV_WOULD_OVERFLOW(a, b) (((a) == WORD_MIN) && ((b) == -1))
#define DIV_WITH_OVERFLOW(a, b) (DIV_WOULD_OVERFLOW((a), (b)) ? WORD_MIN : DIV_CATCH_ZERO((a), (b)))
#define MOD_WITH_OVERFLOW(a, b) (DIV_WOULD_OVERFLOW((a), (b)) ? 0 : MOD_CATCH_ZERO((a), (b)))


// I/O support

// Copy a string from VM to native memory
static int getstr(UWORD adr, UWORD len, char **res)
{
    int error = 0;

    *res = calloc(1, len + 1);
    if (*res == NULL)
        error = -511;
    else
        for (size_t i = 0; error == 0 && i < len; i++, adr++) {
            error = load_byte(adr, (uint8_t *)((*res) + i));
        }

    return error;
}

// Convert portable open(2) flags bits to system flags
static int getflags(UWORD perm, bool *binary)
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
static UWORD *main_argv_len;
int register_args(int argc, const char *argv[])
{
    main_argc = argc;
    main_argv = argv;
    if ((main_argv_len = calloc(argc, sizeof(UWORD))) == NULL)
        return -1;

    for (int i = 0; i < argc; i++) {
        size_t len = strlen(argv[i]);
        if (len > WORD_MAX)
            return -2;
        main_argv_len[i] = len;
    }
    return 0;
}

// Inner execution function
static WORD run_or_step(bool run)
{
    int error = 0;
    do {
        WORD temp = 0;
        DUWORD tempd = 0;
        uint8_t byte = 0;
        WORD A = LOAD_WORD(PC);

        PC += WORD_BYTES;

        switch (A & OP_MASK) {
        case OP_CALL:
            PUSH_RETURN(PC);
            CHECK_VALID_WORD(PC + A);
            PC += A - WORD_BYTES;
            break;
        case OP_PUSH:
            temp = A;
            ARSHIFT(temp, 2);
            PUSH(temp);
            break;
        case OP_PUSHREL:
            PUSH(PC - WORD_BYTES + (A & ~OP_MASK));
            break;
        case OP_INSTRUCTION:
            switch (A >> 2) {
            case O_NOP:
                break;
            case O_NOT:
                {
                    WORD a = POP;
                    PUSH(~a);
                }
                break;
            case O_AND:
                {
                    WORD a = POP;
                    WORD b = POP;
                    PUSH(a & b);
                }
                break;
            case O_OR:
                {
                    WORD a = POP;
                    WORD b = POP;
                    PUSH(a | b);
                }
                break;
            case O_XOR:
                {
                    WORD a = POP;
                    WORD b = POP;
                    PUSH(a ^ b);
                }
                break;
            case O_LSHIFT:
                {
                    WORD shift = POP;
                    WORD value = POP;
                    PUSH(shift < (WORD)WORD_BIT ? value << shift : 0);
                }
                break;
            case O_RSHIFT:
                {
                    WORD shift = POP;
                    WORD value = POP;
                    PUSH(shift < (WORD)WORD_BIT ? (WORD)((UWORD)value >> shift) : 0);
                }
                break;
            case O_ARSHIFT:
                {
                    WORD shift = POP;
                    WORD value = POP;
                    WORD result = ARSHIFT(value, shift);
                    PUSH(result);
                }
                break;
            case O_POP:
                (void)POP;
                break;
            case O_DUP:
                {
                    UWORD depth = POP;
                    if (depth >= SP)
                        error = ERROR_STACK_UNDERFLOW;
                    else
                        PUSH(S0[SP - (depth + 1)]);
                }
                break;
            case O_SET:
                {
                    UWORD depth = POP;
                    UWORD value = POP;
                    if (depth >= SP)
                        error = ERROR_STACK_UNDERFLOW;
                    else
                        S0[SP - (depth + 1)] = value;
                }
                break;
            case O_SWAP:
                {
                    UWORD depth = POP;
                    if (SP == 0 || depth >= SP - 1)
                        error = ERROR_STACK_UNDERFLOW;
                    else {
                        temp = S0[SP - (depth + 2)];
                        S0[SP - (depth + 2)] = S0[SP - 1];
                        S0[SP - 1] = temp;
                    }
                }
                break;
            case O_JUMP:
                {
                    WORD addr = POP;
                    CHECK_VALID_WORD(addr);
                    PC = addr;
                }
                break;
            case O_JUMPZ:
                {
                    WORD addr = POP;
                    if (POP == BEE_FALSE) {
                        CHECK_VALID_WORD(addr);
                        PC = addr;
                    }
                }
                break;
            case O_CALL:
                {
                    WORD addr = POP;
                    CHECK_VALID_WORD(addr);
                    PUSH_RETURN(PC);
                    PC = addr;
                }
                break;
            case O_RET:
                {
                    WORD addr = POP_RETURN;
                    CHECK_VALID_WORD(addr);
                    PC = addr;
                }
                break;
            case O_LOAD:
                {
                    WORD addr = POP;
                    WORD value = LOAD_WORD(addr);
                    PUSH(value);
                }
                break;
            case O_STORE:
                {
                    WORD addr = POP;
                    WORD value = POP;
                    STORE_WORD(addr, value);
                }
                break;
            case O_LOAD1:
                {
                    WORD addr = POP;
                    uint8_t value = LOAD_BYTE(addr);
                    PUSH((WORD)value);
                }
                break;
            case O_STORE1:
                {
                    WORD addr = POP;
                    uint8_t value = (uint8_t)POP;
                    STORE_BYTE(addr, value);
                }
                break;
            case O_LOAD2:
                {
                    WORD addr = POP;
                    if (addr % 2 != 0)
                        error = ERROR_UNALIGNED_ADDRESS;
                    else {
                        uint8_t byte1 = LOAD_BYTE(addr);
                        uint8_t byte2 = LOAD_BYTE(addr + 1);
                        PUSH((WORD)(uint16_t)(byte1 | (byte2 << 8)));
                    }
                }
                break;
            case O_STORE2:
                {
                    WORD addr = POP;
                    if (addr % 2 != 0)
                        error = ERROR_UNALIGNED_ADDRESS;
                    else {
                        uint16_t value = (uint16_t)POP;
                        STORE_BYTE(addr, (uint8_t)value);
                        STORE_BYTE(addr + 1, (uint8_t)(value >> 8));
                    }
                }
                break;
            case O_LOAD4:
                {
                    WORD addr = POP;
                    WORD value = LOAD_WORD(addr);
                    PUSH(value);
                }
                break;
            case O_STORE4:
                {
                    WORD addr = POP;
                    WORD value = POP;
                    STORE_WORD(addr, value);
                }
                break;
            case O_NEGATE:
                {
                    WORD a = POP;
                    PUSH(-a);
                }
                break;
            case O_ADD:
                {
                    WORD a = POP;
                    WORD b = POP;
                    PUSH(b + a);
                }
                break;
            case O_MUL:
                {
                    WORD multiplier = POP;
                    WORD multiplicand = POP;
                    PUSH(multiplier * multiplicand);
                }
                break;
            case O_DIVMOD:
                {
                    WORD divisor = POP;
                    WORD dividend = POP;
                    PUSH(MOD_WITH_OVERFLOW(dividend, divisor));
                    PUSH(DIV_WITH_OVERFLOW(dividend, divisor));
                }
                break;
            case O_UDIVMOD:
                {
                    UWORD divisor = POP;
                    UWORD dividend = POP;
                    PUSH(MOD_CATCH_ZERO(dividend, divisor));
                    PUSH(DIV_CATCH_ZERO(dividend, divisor));
                }
                break;
            case O_EQ:
                {
                    WORD a = POP;
                    WORD b = POP;
                    PUSH(a == b ? BEE_TRUE : BEE_FALSE);
                }
                break;
            case O_LT:
                {
                    WORD a = POP;
                    WORD b = POP;
                    PUSH(b < a ? BEE_TRUE : BEE_FALSE);
                }
                break;
            case O_ULT:
                {
                    UWORD a = POP;
                    UWORD b = POP;
                    PUSH(b < a ? BEE_TRUE : BEE_FALSE);
                }
                break;
            case O_PUSHR:
                {
                    WORD value = POP;
                    PUSH_RETURN(value);
                }
                break;
            case O_POPR:
                {
                    WORD value = POP_RETURN;
                    if (error == ERROR_OK)
                        PUSH(value);
                }
                break;
            case O_DUPR:
                {
                    if (RP == 0)
                        error = ERROR_STACK_UNDERFLOW;
                    else {
                        WORD value = *stack_position(R0, RP, 0);
                        PUSH(value);
                    }
                }
                break;
            case O_CATCH:
                error = ERROR_INVALID_OPCODE;
                break;
            case O_THROW:
                return POP;

            case O_GET_SP:
                {
                    WORD value = SP;
                    PUSH(value);
                }
                break;
            case O_SET_SP:
                {
                    WORD value = POP;
                    SP = value;
                }
                break;
            case O_GET_RP:
                PUSH(RP);
                break;
            case O_SET_RP:
                {
                    WORD value = POP;
                    RP = value;
                }
                break;
            case O_GET_MEMORY:
                PUSH(MEMORY);
                break;
            case O_WORD_BYTES:
                PUSH(WORD_BYTES);
                break;

            case OX_ARGC: // ( -- u )
                PUSH(main_argc);
                break;
            case OX_ARGLEN: // ( u1 -- u2 )
                {
                    UWORD narg = POP;
                    if (narg >= (UWORD)main_argc)
                        PUSH(0);
                    else
                        PUSH(main_argv_len[narg]);
                }
                break;
            case OX_ARGCOPY: // ( u1 addr -- )
                {
                    UWORD addr = POP;
                    UWORD narg = POP;
                    if (narg < (UWORD)main_argc) {
                        UWORD len = (UWORD)main_argv_len[narg];
                        char *ptr = (char *)native_address_of_range(addr, len);
                        if (ptr != NULL) {
                            UWORD end = ALIGN(addr + len);
                            pre_dma(addr, end);
                            strncpy(ptr, main_argv[narg], len);
                            post_dma(addr, end);
                        }
                    }
                }
                break;
            case OX_STDIN:
                PUSH((WORD)(STDIN_FILENO));
                break;
            case OX_STDOUT:
                PUSH((WORD)(STDOUT_FILENO));
                break;
            case OX_STDERR:
                PUSH((WORD)(STDERR_FILENO));
                break;
            case OX_OPEN_FILE:
                {
                    bool binary = false;
                    int perm = getflags(POP, &binary);
                    UWORD len = POP;
                    UWORD str = POP;
                    char *file;
                    error = getstr(str, len, &file);
                    int fd = error == 0 ? open(file, perm, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH) : -1;
                    free(file);
                    PUSH((WORD)fd);
                    PUSH(fd < 0 || (binary && set_binary_mode(fd, O_BINARY) < 0) ? -1 : 0);
                }
                break;
            case OX_CLOSE_FILE:
                {
                    int fd = POP;
                    PUSH((WORD)close(fd));
                }
                break;
            case OX_READ_FILE:
                {
                    int fd = POP;
                    UWORD nbytes = POP;
                    UWORD buf = POP;

                    ssize_t res = 0;
                    if (error == 0) {
                        error = pre_dma(buf, buf + nbytes);
                        if (error == 0) {
                            res = read(fd, native_address_of_range(buf, 0), nbytes);
                            error = post_dma(buf, buf + nbytes);
                        }
                    }

                    PUSH(res);
                    PUSH((error == 0 && res >= 0) ? 0 : -1);
                }
                break;
            case OX_WRITE_FILE:
                {
                    int fd = POP;
                    UWORD nbytes = POP;
                    UWORD buf = POP;

                    ssize_t res = 0;
                    if (error == 0) {
                        error = pre_dma(buf, buf + nbytes);
                        if (error == 0) {
                            res = write(fd, native_address_of_range(buf, 0), nbytes);
                            error = post_dma(buf, buf + nbytes);
                        }
                    }

                    PUSH((error == 0 && res >= 0) ? 0 : -1);
                }
                break;
            case OX_FILE_POSITION:
                {
                    int fd = POP;
                    off_t res = lseek(fd, 0, SEEK_CUR);
                    PUSH_DOUBLE((DUWORD)res);
                    PUSH(res >= 0 ? 0 : -1);
                }
                break;
            case OX_REPOSITION_FILE:
                {
                    int fd = POP;
                    DUWORD ud = POP_DOUBLE;
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
                    UWORD len1 = POP;
                    UWORD str1 = POP;
                    UWORD len2 = POP;
                    UWORD str2 = POP;
                    char *from;
                    char *to = NULL;
                    error = getstr(str2, len2, &from) ||
                        getstr(str1, len1, &to) ||
                        rename(from, to);
                    free(from);
                    free(to);
                    PUSH(error);
                }
                break;
            case OX_DELETE_FILE:
                {
                    UWORD len = POP;
                    UWORD str = POP;
                    char *file;
                    error = getstr(str, len, &file) ||
                        remove(file);
                    free(file);
                    PUSH(error);
                }
                break;
            case OX_FILE_SIZE:
                {
                    struct stat st;
                    int fd = POP;
                    int res = fstat(fd, &st);
                    PUSH_DOUBLE((DUWORD)st.st_size);
                    PUSH(res);
                }
                break;
            case OX_RESIZE_FILE:
                {
                    int fd = POP;
                    DUWORD ud = POP_DOUBLE;
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
                error = ERROR_INVALID_OPCODE;
                break;
            }
            break;
        }
    } while (run == true && error == 0);

 error:
    if (error == 0 && run == false)
        error = ERROR_STEP; // single_step terminated OK
    return error;
}

// Perform one pass of the execution cycle
WORD single_step(void)
{
    return run_or_step(false);
}

WORD run(void)
{
    return run_or_step(true);
}
