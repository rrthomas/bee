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


// Instruction types
enum {
    // Bits 0 and 1
    OP_CALLI    = 0x0,
    OP_PUSHI    = 0x1,
    OP_PUSHRELI = 0x2,
    OP_LEVEL2   = 0x3,
    OP_MASK     = 0x3,

    // Bits 2 and 3 when bits 0 and 1 are 11
    OP2_JUMPI   = 0x0,
    OP2_JUMPZI  = 0x1,
    OP2_INSN    = 0x2,
    OP2_TRAP    = 0x3,
    OP2_MASK    = 0x3,
};

// OP_INSN opcodes
enum {
    O_NOP = 0,
    O_NOT,
    O_AND,
    O_OR,
    O_XOR,
    O_LSHIFT,
    O_RSHIFT,
    O_ARSHIFT,
    O_POP = 0x8,
    O_DUP,
    O_SET,
    O_SWAP,
    O_JUMP,
    O_JUMPZ,
    O_CALL,
    O_RET,
    O_LOAD = 0x10,
    O_STORE,
    O_LOAD1,
    O_STORE1,
    O_LOAD2,
    O_STORE2,
    O_LOAD4,
    O_STORE4,
    O_NEGATE = 0x18,
    O_ADD,
    O_MUL,
    O_DIVMOD,
    O_UDIVMOD,
    O_EQ,
    O_LT,
    O_ULT,
    O_PUSHR = 0x20,
    O_POPR,
    O_DUPR,
    O_CATCH,
    O_THROW,
    O_BREAK,
    O_WORD_BYTES,
    O_GET_M0,
    O_GET_MSIZE = 0x28,
    O_GET_RSIZE,
    O_GET_RP,
    O_SET_RP,
    O_GET_SSIZE,
    O_GET_SP,
    O_SET_SP,
    O_GET_HANDLER_RP,

    O_UNDEFINED = 0x3f
};
