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

#include "private.h"
#include "traps.h"


// Assumption for file functions
verify(sizeof(int) <= sizeof(bee_word_t));


// I/O support

#if SIZEOF_INTPTR_T == 4
typedef uint64_t bee_duword_t;
#define PUSH_DOUBLE(ud)                                             \
    PUSH((bee_uword_t)(ud & (bee_uword_t)-1));                      \
    PUSH((bee_uword_t)((ud >> BEE_WORD_BIT) & (bee_uword_t)-1));
#define POP_DOUBLE(ud)                                                  \
    bee_word_t pop1, pop2;                                              \
    POP(&pop1);                                                         \
    POP(&pop2);                                                         \
    *ud = (((bee_duword_t)(bee_uword_t)pop1) << BEE_WORD_BIT | (bee_uword_t)pop2)
#else
typedef size_t bee_duword_t;
#define PUSH_DOUBLE(res) PUSH(res)
#define POP_DOUBLE(res)  POP((bee_word_t *)res)
#endif

// Register command-line args
static int main_argc = 0;
static const char **main_argv;
void bee_register_args(int argc, const char *argv[])
{
     main_argc = argc;
     main_argv = argv;
}


bee_word_t trap_libc(bee_uword_t function)
{
    bee_word_t temp = 0;

    int error = BEE_ERROR_OK;
    switch (function) {
    case TRAP_LIBC_STRLEN: // ( a-addr -- u )
        {
            const char *s;
            POP((bee_word_t *)&s);
            PUSH(strlen(s));
        }
        break;
    case TRAP_LIBC_STRNCPY: // ( a-addr1 a-addr2 u -- a-addr3 )
        {
            const char *src;
            char *dest;
            size_t n;
            POP((bee_word_t *)&n);
            POP((bee_word_t *)&src);
            POP((bee_word_t *)&dest);
            PUSH((bee_word_t)(size_t)(void *)strncpy(dest, src, n));
        }
        break;
    case TRAP_LIBC_STDIN:
        PUSH((bee_word_t)(STDIN_FILENO));
        break;
    case TRAP_LIBC_STDOUT:
        PUSH((bee_word_t)(STDOUT_FILENO));
        break;
    case TRAP_LIBC_STDERR:
        PUSH((bee_word_t)(STDERR_FILENO));
        break;
    case TRAP_LIBC_O_RDONLY:
        PUSH((bee_word_t)(O_RDONLY));
        break;
    case TRAP_LIBC_O_WRONLY:
        PUSH((bee_word_t)(O_WRONLY));
        break;
    case TRAP_LIBC_O_RDWR:
        PUSH((bee_word_t)(O_RDWR));
        break;
    case TRAP_LIBC_O_CREAT:
        PUSH((bee_word_t)(O_CREAT));
        break;
    case TRAP_LIBC_O_TRUNC:
        PUSH((bee_word_t)(O_TRUNC));
        break;
    case TRAP_LIBC_OPEN: // ( c-addr flags -- fd )
        {
            bee_uword_t flags;
            POP((bee_word_t *)&flags);
            char *file;
            POP((bee_word_t *)&file);
            int fd = open(file, flags, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
            PUSH((bee_word_t)fd);
            if (fd >= 0)
                set_binary_mode(fd, O_BINARY); // Best effort
        }
        break;
    case TRAP_LIBC_CLOSE:
        {
            POP(&temp);
            int fd = temp;
            PUSH((bee_word_t)close(fd));
        }
        break;
    case TRAP_LIBC_READ:
        {
            POP(&temp);
            int fd = temp;
            bee_uword_t nbytes;
            POP((bee_word_t *)&nbytes);
            uint8_t *buf;
            POP((bee_word_t *)&buf);
            PUSH(read(fd, buf, nbytes));
        }
        break;
    case TRAP_LIBC_WRITE:
        {
            POP(&temp);
            int fd = temp;
            bee_uword_t nbytes;
            POP((bee_word_t *)&nbytes);
            uint8_t *buf;
            POP((bee_word_t *)&buf);
            PUSH(write(fd, buf, nbytes));
        }
        break;
    case TRAP_LIBC_SEEK_SET:
        PUSH((bee_word_t)(SEEK_SET));
        break;
    case TRAP_LIBC_SEEK_CUR:
        PUSH((bee_word_t)(SEEK_CUR));
        break;
    case TRAP_LIBC_SEEK_END:
        PUSH((bee_word_t)(SEEK_END));
        break;
    case TRAP_LIBC_LSEEK:
        {
            POP(&temp);
            int whence = temp;
            bee_duword_t ud;
            POP_DOUBLE(&ud);
            POP(&temp);
            int fd = temp;
            off_t res = lseek(fd, (off_t)ud, whence);
            PUSH_DOUBLE((bee_duword_t)res);
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
            POP((bee_word_t *)&to);
            POP((bee_word_t *)&from);
            PUSH(rename(from, to));
        }
        break;
    case TRAP_LIBC_REMOVE:
        {
            char *file;
            POP((bee_word_t *)&file);
            PUSH(remove(file));
        }
        break;
    case TRAP_LIBC_FILE_SIZE:
        {
            struct stat st;
            POP(&temp);
            int fd = temp;
            int res = fstat(fd, &st);
            PUSH_DOUBLE((bee_duword_t)st.st_size);
            PUSH(res);
        }
        break;
    case TRAP_LIBC_RESIZE_FILE:
        {
            POP(&temp);
            int fd = temp;
            bee_duword_t ud;
            POP_DOUBLE(&ud);
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
        PUSH((bee_word_t)main_argv);
        break;
    default:
        error = BEE_ERROR_INVALID_FUNCTION;
        break;
    }

 error:
    return error;
}

bee_word_t trap(bee_word_t code)
{
    int error = BEE_ERROR_OK;
    switch (code) {
    case TRAP_LIBC:
        {
            bee_uword_t function = 0;
            POP((bee_word_t *)&function);
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
