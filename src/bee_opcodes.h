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
    OP_PUSH,
    OP_PUSHREL,
    OP_INSTRUCTION,
    OP_MASK = 3,
};

enum {
    O_POP,
    O_DUP,
    O_ROLL,
    O_PUSHR,
    O_POPR,
    O_DUPR,
    O_GET_SP,
    O_SET_SP,
    O_GET_RP,
    O_SET_RP,
    O_GET_MEMORY,
    O_WORD_BYTES,
    O_LOAD,
    O_STORE,
    O_LOAD1,
    O_STORE1,
    O_ADD,
    O_NEGATE,
    O_MUL,
    O_UDIVMOD,
    O_DIVMOD,
    O_EQ,
    O_LT,
    O_ULT,
    O_NOT,
    O_AND,
    O_OR,
    O_XOR,
    O_LSHIFT,
    O_RSHIFT,
    O_RET,
    O_CALL,
    O_HALT,
    O_JUMP,
    O_JUMPZ,
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
