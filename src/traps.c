// The traps.
//
// (c) Reuben Thomas 1994-2022
//
// The package is distributed under the GNU General Public License version 3,
// or, at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#include "config.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
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


// Assumptions for file functions
verify(sizeof(int) <= sizeof(bee_word_t));
verify(sizeof(off_t) <= sizeof(bee_word_t));


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
            POPD((bee_word_t *)&s);
            PUSHD(strlen(s));
        }
        break;
    case TRAP_LIBC_STRNCPY: // ( a-addr1 a-addr2 u -- a-addr3 )
        {
            const char *src;
            char *dest;
            size_t n;
            POPD((bee_word_t *)&n);
            POPD((bee_word_t *)&src);
            POPD((bee_word_t *)&dest);
            PUSHD((bee_word_t)(size_t)(void *)strncpy(dest, src, n));
        }
        break;
    case TRAP_LIBC_STDIN:
        PUSHD((bee_word_t)(STDIN_FILENO));
        break;
    case TRAP_LIBC_STDOUT:
        PUSHD((bee_word_t)(STDOUT_FILENO));
        break;
    case TRAP_LIBC_STDERR:
        PUSHD((bee_word_t)(STDERR_FILENO));
        break;
    case TRAP_LIBC_O_RDONLY:
        PUSHD((bee_word_t)(O_RDONLY));
        break;
    case TRAP_LIBC_O_WRONLY:
        PUSHD((bee_word_t)(O_WRONLY));
        break;
    case TRAP_LIBC_O_RDWR:
        PUSHD((bee_word_t)(O_RDWR));
        break;
    case TRAP_LIBC_O_CREAT:
        PUSHD((bee_word_t)(O_CREAT));
        break;
    case TRAP_LIBC_O_TRUNC:
        PUSHD((bee_word_t)(O_TRUNC));
        break;
    case TRAP_LIBC_OPEN: // ( c-addr flags -- fd )
        {
            bee_uword_t flags;
            POPD((bee_word_t *)&flags);
            char *file;
            POPD((bee_word_t *)&file);
            int fd = open(file, flags, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
            PUSHD((bee_word_t)fd);
            if (fd >= 0)
                set_binary_mode(fd, O_BINARY); // Best effort
        }
        break;
    case TRAP_LIBC_CLOSE:
        {
            POPD(&temp);
            int fd = temp;
            PUSHD((bee_word_t)close(fd));
        }
        break;
    case TRAP_LIBC_READ:
        {
            POPD(&temp);
            int fd = temp;
            bee_uword_t nbytes;
            POPD((bee_word_t *)&nbytes);
            uint8_t *buf;
            POPD((bee_word_t *)&buf);
            PUSHD(read(fd, buf, nbytes));
        }
        break;
    case TRAP_LIBC_WRITE:
        {
            POPD(&temp);
            int fd = temp;
            bee_uword_t nbytes;
            POPD((bee_word_t *)&nbytes);
            uint8_t *buf;
            POPD((bee_word_t *)&buf);
            PUSHD(write(fd, buf, nbytes));
        }
        break;
    case TRAP_LIBC_SEEK_SET:
        PUSHD((bee_word_t)(SEEK_SET));
        break;
    case TRAP_LIBC_SEEK_CUR:
        PUSHD((bee_word_t)(SEEK_CUR));
        break;
    case TRAP_LIBC_SEEK_END:
        PUSHD((bee_word_t)(SEEK_END));
        break;
    case TRAP_LIBC_LSEEK:
        {
            POPD(&temp);
            int whence = temp;
            off_t off;
            POPD(&off);
            POPD(&temp);
            int fd = temp;
            off_t res = lseek(fd, off, whence);
            PUSHD(res);
        }
        break;
    case TRAP_LIBC_FDATASYNC:
        {
            POPD(&temp);
            int fd = temp;
            PUSHD(fdatasync(fd));
        }
        break;
    case TRAP_LIBC_RENAME:
        {
            char *to, *from;
            POPD((bee_word_t *)&to);
            POPD((bee_word_t *)&from);
            PUSHD(rename(from, to));
        }
        break;
    case TRAP_LIBC_REMOVE:
        {
            char *file;
            POPD((bee_word_t *)&file);
            PUSHD(remove(file));
        }
        break;
    case TRAP_LIBC_FILE_SIZE:
        {
            struct stat st;
            POPD(&temp);
            int fd = temp;
            int res = fstat(fd, &st);
            PUSHD(st.st_size);
            PUSHD(res);
        }
        break;
    case TRAP_LIBC_RESIZE_FILE:
        {
            POPD(&temp);
            int fd = temp;
            off_t off;
            POPD(&off);
            int res = ftruncate(fd, off);
            PUSHD(res);
        }
        break;
    case TRAP_LIBC_FILE_STATUS:
        {
            struct stat st;
            POPD(&temp);
            int fd = temp;
            int res = fstat(fd, &st);
            PUSHD(st.st_mode);
            PUSHD(res);
        }
        break;
    case TRAP_LIBC_ARGC: // ( -- u )
        PUSHD(main_argc);
        break;
    case TRAP_LIBC_ARGV: // ( -- a-addr )
        PUSHD((bee_word_t)main_argv);
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
            POPD((bee_word_t *)&function);
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
