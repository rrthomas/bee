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

// Make a NUL-terminated string from a counted string
static int getstr(char *adr, UWORD len, char **res)
{
    int error = 0;

    *res = calloc(1, len + 1);
    if (*res == NULL)
        error = -511;
    else
        strncpy(*res, adr, len);

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


WORD trap_libc(UWORD function)
{
    WORD temp = 0;

    int error = ERROR_OK;
    switch (function) {
    case TRAP_LIBC_ARGC: // ( -- u )
        PUSH(main_argc);
        break;
    case TRAP_LIBC_ARGLEN: // ( u1 -- u2 )
        {
            UWORD narg;
            POP((WORD *)&narg);
            if (narg >= (UWORD)main_argc)
                PUSH(0);
            else
                PUSH(main_argv_len[narg]);
        }
        break;
    case TRAP_LIBC_ARGCOPY: // ( u1 addr -- )
        {
            char *addr;
            POP((WORD *)&addr);
            UWORD narg;
            POP((WORD *)&narg);
            if (narg < (UWORD)main_argc) {
                UWORD len = (UWORD)main_argv_len[narg];
                strncpy(addr, main_argv[narg], len);
            }
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
    case TRAP_LIBC_OPEN_FILE:
        {
            bool binary = false;
            POP(&temp);
            int perm = getflags(temp, &binary);
            UWORD len;
            POP((WORD *)&len);
            char *str;
            POP((WORD *)&str);
            char *file;
            error = getstr(str, len, &file);
            int fd = error == 0 ? open(file, perm, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH) : -1;
            free(file);
            PUSH((WORD)fd);
            PUSH(fd < 0 || (binary && set_binary_mode(fd, O_BINARY) < 0) ? -1 : 0);
        }
        break;
    case TRAP_LIBC_CLOSE_FILE:
        {
            POP(&temp);
            int fd = temp;
            PUSH((WORD)close(fd));
        }
        break;
    case TRAP_LIBC_READ_FILE:
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
    case TRAP_LIBC_WRITE_FILE:
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
    case TRAP_LIBC_FILE_POSITION:
        {
            POP(&temp);
            int fd = temp;
            off_t res = lseek(fd, 0, SEEK_CUR);
            PUSH_DOUBLE((DUWORD)res);
            PUSH(res >= 0 ? 0 : -1);
        }
        break;
    case TRAP_LIBC_REPOSITION_FILE:
        {
            POP(&temp);
            int fd = temp;
            WORD pop1, pop2;
            POP(&pop1);
            POP(&pop2);
            DUWORD ud = DOUBLE_WORD(pop1, pop2);
            off_t res = lseek(fd, (off_t)ud, SEEK_SET);
            PUSH(res >= 0 ? 0 : -1);
        }
        break;
    case TRAP_LIBC_FLUSH_FILE:
        {
            POP(&temp);
            int fd = temp;
            int res = fdatasync(fd);
            PUSH(res);
        }
        break;
    case TRAP_LIBC_RENAME_FILE:
        {
            UWORD len1, len2;
            char *str1, *str2;
            POP((WORD *)&len1);
            POP((WORD *)&str1);
            POP((WORD *)&len2);
            POP((WORD *)&str2);
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
    case TRAP_LIBC_DELETE_FILE:
        {
            UWORD len;
            POP((WORD *)&len);
            char *str;
            POP((WORD *)&str);
            char *file;
            error = getstr(str, len, &file) || remove(file);
            free(file);
            PUSH(error);
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
    default:
        error = ERROR_INVALID_FUNCTION;
        break;
    }

 error:
    return error;
}

WORD trap(WORD code)
{
    int error = ERROR_OK;
    switch (code) {
    case TRAP_LIBC:
        {
            UWORD function = 0;
            POP((WORD *)&function);
            return trap_libc(function);
        }
        break;
    default:
        return ERROR_INVALID_LIBRARY;
        break;
    }

 error:
    return error;
}
