// The traps.
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

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include "binary-io.h"
#include "verify.h"

#include "bee/bee.h"
#include "bee/aux.h"

#include "private.h"
#include "traps.h"


// Assumption for file functions
verify(sizeof(int) <= sizeof(WORD));


// I/O support

typedef uint64_t DUWORD;
#define DOUBLE_WORD(pop1, pop2)                                 \
    (((DUWORD)(bee_UWORD)pop1) << bee_WORD_BIT | (UWORD)pop2)
#define PUSH_DOUBLE(ud)                                                 \
    bee_PUSH((bee_UWORD)(ud & bee_WORD_MASK));                          \
    bee_PUSH((bee_UWORD)((ud >> bee_WORD_BIT) & bee_WORD_MASK));

// Register command-line args
static int main_argc = 0;
static const char **main_argv;
void bee_register_args(int argc, const char *argv[])
{
     main_argc = argc;
     main_argv = argv;
}


WORD trap_libc(UWORD function)
{
    WORD temp = 0;

    int error = BEE_ERROR_OK;
    switch (function) {
    case TRAP_LIBC_STRLEN: // ( a-addr -- u )
        {
            const char *s;
            POP((WORD *)&s);
            PUSH(strlen(s));
        }
        break;
    case TRAP_LIBC_STRNCPY: // ( a-addr1 a-addr2 u -- a-addr3 )
        {
            const char *src;
            char *dest;
            size_t n;
            POP((WORD *)&n);
            POP((WORD *)&src);
            POP((WORD *)&dest);
            PUSH((WORD)(size_t)(void *)strncpy(dest, src, n));
        }
        break;
    case TRAP_LIBC_STDIN:
        PUSH((WORD)(STDIN_FILENO));
        break;
    case TRAP_LIBC_STDOUT:
        PUSH((WORD)(STDOUT_FILENO));
        break;
    case TRAP_LIBC_STDERR:
        PUSH((WORD)(STDERR_FILENO));
        break;
    case TRAP_LIBC_O_RDONLY:
        PUSH((WORD)(O_RDONLY));
        break;
    case TRAP_LIBC_O_WRONLY:
        PUSH((WORD)(O_WRONLY));
        break;
    case TRAP_LIBC_O_RDWR:
        PUSH((WORD)(O_RDWR));
        break;
    case TRAP_LIBC_O_CREAT:
        PUSH((WORD)(O_CREAT));
        break;
    case TRAP_LIBC_O_TRUNC:
        PUSH((WORD)(O_TRUNC));
        break;
    case TRAP_LIBC_OPEN: // ( c-addr flags -- fd )
        {
            UWORD flags;
            POP((WORD *)&flags);
            char *file;
            POP((WORD *)&file);
            int fd = open(file, flags, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
            PUSH((WORD)fd);
            if (fd >= 0)
                set_binary_mode(fd, O_BINARY); // Best effort
        }
        break;
    case TRAP_LIBC_CLOSE:
        {
            POP(&temp);
            int fd = temp;
            PUSH((WORD)close(fd));
        }
        break;
    case TRAP_LIBC_READ:
        {
            POP(&temp);
            int fd = temp;
            UWORD nbytes;
            POP((WORD *)&nbytes);
            uint8_t *buf;
            POP((WORD *)&buf);
            PUSH(read(fd, buf, nbytes));
        }
        break;
    case TRAP_LIBC_WRITE:
        {
            POP(&temp);
            int fd = temp;
            UWORD nbytes;
            POP((WORD *)&nbytes);
            uint8_t *buf;
            POP((WORD *)&buf);
            PUSH(write(fd, buf, nbytes));
        }
        break;
    case TRAP_LIBC_SEEK_SET:
        PUSH((WORD)(SEEK_SET));
        break;
    case TRAP_LIBC_SEEK_CUR:
        PUSH((WORD)(SEEK_CUR));
        break;
    case TRAP_LIBC_SEEK_END:
        PUSH((WORD)(SEEK_END));
        break;
    case TRAP_LIBC_LSEEK:
        {
            POP(&temp);
            int whence = temp;
            WORD pop1, pop2;
            POP(&pop1);
            POP(&pop2);
            DUWORD ud = DOUBLE_WORD(pop1, pop2);
            POP(&temp);
            int fd = temp;
            off_t res = lseek(fd, (off_t)ud, whence);
            PUSH_DOUBLE((DUWORD)res);
        }
        break;
    case TRAP_LIBC_FDATASYNC:
        {
            POP(&temp);
            int fd = temp;
            PUSH(fdatasync(fd));
        }
        break;
    case TRAP_LIBC_RENAME:
        {
            char *to, *from;
            POP((WORD *)&to);
            POP((WORD *)&from);
            PUSH(rename(from, to));
        }
        break;
    case TRAP_LIBC_REMOVE:
        {
            char *file;
            POP((WORD *)&file);
            PUSH(remove(file));
        }
        break;
    case TRAP_LIBC_FILE_SIZE:
        {
            struct stat st;
            POP(&temp);
            int fd = temp;
            int res = fstat(fd, &st);
            PUSH_DOUBLE((DUWORD)st.st_size);
            PUSH(res);
        }
        break;
    case TRAP_LIBC_RESIZE_FILE:
        {
            POP(&temp);
            int fd = temp;
            WORD pop1, pop2;
            POP(&pop1);
            POP(&pop2);
            DUWORD ud = DOUBLE_WORD(pop1, pop2);
            int res = ftruncate(fd, (off_t)ud);
            PUSH(res);
        }
        break;
    case TRAP_LIBC_FILE_STATUS:
        {
            struct stat st;
            POP(&temp);
            int fd = temp;
            int res = fstat(fd, &st);
            PUSH(st.st_mode);
            PUSH(res);
        }
        break;
    case TRAP_LIBC_ARGC: // ( -- u )
        PUSH(main_argc);
        break;
    case TRAP_LIBC_ARGV: // ( -- a-addr )
        PUSH((WORD)main_argv);
        break;
    default:
        error = BEE_ERROR_INVALID_FUNCTION;
        break;
    }

 error:
    return error;
}

WORD trap(WORD code)
{
    int error = BEE_ERROR_OK;
    switch (code) {
    case TRAP_LIBC:
        {
            UWORD function = 0;
            POP((WORD *)&function);
            return trap_libc(function);
        }
        break;
    default:
        return BEE_ERROR_INVALID_LIBRARY;
        break;
    }

 error:
    return error;
}
