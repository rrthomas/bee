// The trap opcodes.
//
// (c) Reuben Thomas 1994-2020
//
// The package is distributed under the GNU General Public License version 3,
// or, at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

enum {
    TRAP_LIBC,
};

enum {
    TRAP_LIBC_STRLEN,
    TRAP_LIBC_STRNCPY,
    TRAP_LIBC_STDIN,
    TRAP_LIBC_STDOUT,
    TRAP_LIBC_STDERR,
    TRAP_LIBC_O_RDONLY,
    TRAP_LIBC_O_WRONLY,
    TRAP_LIBC_O_RDWR,
    TRAP_LIBC_O_CREAT,
    TRAP_LIBC_O_TRUNC,
    TRAP_LIBC_OPEN,
    TRAP_LIBC_CLOSE,
    TRAP_LIBC_READ,
    TRAP_LIBC_WRITE,
    TRAP_LIBC_SEEK_SET,
    TRAP_LIBC_SEEK_CUR,
    TRAP_LIBC_SEEK_END,
    TRAP_LIBC_LSEEK,
    TRAP_LIBC_FDATASYNC,
    TRAP_LIBC_RENAME,
    TRAP_LIBC_REMOVE,
    TRAP_LIBC_FILE_SIZE,
    TRAP_LIBC_RESIZE_FILE,
    TRAP_LIBC_FILE_STATUS,

    TRAP_LIBC_ARGC = 0x100,
    TRAP_LIBC_ARGV,
};
