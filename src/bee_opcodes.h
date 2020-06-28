// enum type for the opcodes to make the interpreter more readable. Opcode
// names which are not valid C identifiers have been altered.
//
// (c) Reuben Thomas 1994-2020
//
// The package is distributed under the GNU Public License version 3, or,
// at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#ifndef BEE_OPCODES
#define BEE_OPCODES


enum {
    OP_CALL,
    OP_LITERAL,
    OP_OFFSET,
    OP_INSTRUCTION,
    OP_MASK = 3,
};

enum {
    O_DROP,
    O_PICK,
    O_ROLL,
    O_TOR,
    O_RFROM,
    O_RFETCH,
    O_S0FETCH,
    O_S0STORE,
    O_SPFETCH,
    O_SPSTORE,
    O_R0FETCH,
    O_R0STORE,
    O_RPFETCH,
    O_RPSTORE,
    O_MEMORYFETCH,
    O_CELL,
    O_FETCH,
    O_STORE,
    O_CFETCH,
    O_CSTORE,
    O_PLUS,
    O_NEGATE,
    O_STAR,
    O_USLASHMOD,
    O_SSLASHREM,
    O_EQUAL,
    O_LESS,
    O_ULESS,
    O_INVERT,
    O_AND,
    O_OR,
    O_XOR,
    O_LSHIFT,
    O_RSHIFT,
    O_EXIT,
    O_EXECUTE,
    O_HALT,
    O_BRANCH,
    O_QBRANCH,
    O_LITERAL,
    O_UNDEFINED = 0x7f,
    OX_ARGC = 0x80,
    OX_ARGLEN,
    OX_ARGCOPY,
    OX_STDIN,
    OX_STDOUT,
    OX_STDERR,
    OX_OPEN_FILE,
    OX_CLOSE_FILE,
    OX_READ_FILE,
    OX_WRITE_FILE,
    OX_FILE_POSITION,
    OX_REPOSITION_FILE,
    OX_FLUSH_FILE,
    OX_RENAME_FILE,
    OX_DELETE_FILE,
    OX_FILE_SIZE,
    OX_RESIZE_FILE,
    OX_FILE_STATUS,
};


#endif
