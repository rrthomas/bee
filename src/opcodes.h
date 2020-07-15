/* Bee instruction set description.

   (c) Reuben Thomas 1994-2020

   The package is distributed under the GNU Public License version 3, or,
   at your option, any later version.

   THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
   RISK.  */


/* Instructions occupy a word, and take the following forms:

   1. Opcode in bits 0-1, rest of word is immediate operand.
      The opcodes are BEE_OP_* below.
   2. Bits 0-1 are 11, opcode in bits 2-3, rest of word is immediate
      operand.  The opcodes are BEE_OP2_* below.
   3. Bits 0-3 are 1011, rest of word is instruction opcode.
      The opcodes are BEE_INSN_* below.
*/


/* Bee opcode info.  */
typedef struct bee_opc_info_t
{
  int opcode;		/* opcode bits for particular instruction.  */
  const char * name;	/* instruction name, where applicable.  */
} bee_opc_info_t;

/* Table to identify an instruction from its 4 LSbits.  */
extern const bee_opc_info_t bee_opc_info[16];
/* Table of information about short instructions (BEE_OP2_INSN).  */
extern const bee_opc_info_t bee_inst_info[0x40];

/* Largest trap code.  */
#define BEE_MAX_TRAP ((1 << 28) - 1)

/* Instruction types.  */
enum {
  /* Bits 0 and 1.  */
  BEE_OP_CALLI    = 0x0,
  BEE_OP_PUSHI    = 0x1,
  BEE_OP_PUSHRELI = 0x2,
  BEE_OP_LEVEL2   = 0x3,
  BEE_OP_MASK     = 0x3,

  /* Bits 2 and 3 when bits 0 and 1 are 11.  */
  BEE_OP2_JUMPI   = 0x0,
  BEE_OP2_JUMPZI  = 0x1,
  BEE_OP2_INSN    = 0x2,
  BEE_OP2_TRAP    = 0x3,
  BEE_OP2_MASK    = 0x3,

  BEE_OP2_BAD     = 0x4,
};

/* OP_INSN opcodes.  */
enum {
  BEE_INSN_NOP = 0,
  BEE_INSN_NOT,
  BEE_INSN_AND,
  BEE_INSN_OR,
  BEE_INSN_XOR,
  BEE_INSN_LSHIFT,
  BEE_INSN_RSHIFT,
  BEE_INSN_ARSHIFT,
  BEE_INSN_POP = 0x8,
  BEE_INSN_DUP,
  BEE_INSN_SET,
  BEE_INSN_SWAP,
  BEE_INSN_JUMP,
  BEE_INSN_JUMPZ,
  BEE_INSN_CALL,
  BEE_INSN_RET,
  BEE_INSN_LOAD = 0x10,
  BEE_INSN_STORE,
  BEE_INSN_LOAD1,
  BEE_INSN_STORE1,
  BEE_INSN_LOAD2,
  BEE_INSN_STORE2,
  BEE_INSN_LOAD4,
  BEE_INSN_STORE4,
  BEE_INSN_NEGATE = 0x18,
  BEE_INSN_ADD,
  BEE_INSN_MUL,
  BEE_INSN_DIVMOD,
  BEE_INSN_UDIVMOD,
  BEE_INSN_EQ,
  BEE_INSN_LT,
  BEE_INSN_ULT,
  BEE_INSN_PUSHR = 0x20,
  BEE_INSN_POPR,
  BEE_INSN_DUPR,
  BEE_INSN_CATCH,
  BEE_INSN_THROW,
  BEE_INSN_BREAK,
  BEE_INSN_WORD_BYTES,
  BEE_INSN_GET_M0,
  BEE_INSN_GET_MSIZE = 0x28,
  BEE_INSN_GET_RSIZE,
  BEE_INSN_GET_RP,
  BEE_INSN_SET_RP,
  BEE_INSN_GET_SSIZE,
  BEE_INSN_GET_SP,
  BEE_INSN_SET_SP,
  BEE_INSN_GET_HANDLER_RP,

  BEE_INSN_UNDEFINED = 0x3f
};
