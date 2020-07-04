// The extra instructions.
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


WORD extra_instruction(WORD opcode)
{
    WORD temp = 0;
    DUWORD tempd = 0;

    int error = ERROR_OK;
    switch (opcode) {
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
                if (ptr != NULL)
                    strncpy(ptr, main_argv[narg], len);
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
            if (error == 0)
                res = read(fd, native_address_of_range(buf, 0), nbytes);

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
            if (error == 0)
                res = write(fd, native_address_of_range(buf, 0), nbytes);

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
    }

    return error;
}