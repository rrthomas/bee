/* Bumble instruction set description.

   (c) Reuben Thomas 1994-2022

   The package is distributed under the GNU General Public License version 3,
   or, at your option, any later version.

   THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
   RISK.  */


/* Instructions occupy a word, and take the following forms:

   32-bit Bumble:

   1. Opcode in bits 0-1, rest of word is immediate operand.
      The opcodes are BUMBLE_OP_* below masked with BUMBLE_OP1_MASK.
   2. Bits 0-1 are 11, opcode in bits 2-3, rest of word is immediate operand.
      The opcodes are BUMBLE_OP_* below masked with BUMBLE_OP2_MASK.
   3. Bits 0-3 are 1111, rest of word is instruction opcode.
      The opcodes are BUMBLE_INSN_* below.

   64-bit Bumble:

   1. Opcode in bits 0-2, rest of word is immediate operand.
      The opcodes are BUMBLE_OP_* below masked with BUMBLE_OP1_MASK == BUMBLE_OP2_MASK.
   2. Bits 0-3 are 111, rest of word is instruction opcode.
      The opcodes are BUMBLE_INSN_* below.
*/


/* Bumble opcode info.  */
typedef struct bee_opc_info_t
{
  int opcode;		/* opcode bits for particular instruction.  */
  const char * name;	/* instruction name, where applicable.  */
} bee_opc_info_t;

/* Table to identify an instruction from its 4 LSbits.  */
extern const bee_opc_info_t bee_opc_info[16];
/* Table of information about short instructions (BUMBLE_INSN_*).  */
extern const bee_opc_info_t bee_inst_info[0x40];

#if BUMBLE_WORD_BYTES == 4
#define BUMBLE_OP1_SHIFT 2
#define BUMBLE_OP2_SHIFT 4
/* 32-bit Instruction types.  */
enum {
  /* Bits 0-1.  */
  BUMBLE_OP1_MASK    = 0x3,
  BUMBLE_OP_CALLI    = 0x0,
  BUMBLE_OP_PUSHI    = 0x1,
  BUMBLE_OP_PUSHRELI = 0x2,
  BUMBLE_OP_LEVEL2   = 0x3,

  /* Bits 0-3 when bits 0-1 are 11.  */
  BUMBLE_OP2_MASK    = 0xf,
  BUMBLE_OP_JUMPI    = 0x3,
  BUMBLE_OP_JUMPZI   = 0x7,
  BUMBLE_OP_INSN     = 0xf,
};
#else
/* 64-bit Instruction types.  */
#define BUMBLE_OP1_SHIFT 3
#define BUMBLE_OP2_SHIFT 3
enum {
  BUMBLE_OP1_MASK    = 0x7,
  BUMBLE_OP2_MASK    = 0x7,

  /* Bits 0-2.  */
  BUMBLE_OP_CALLI    = 0x0,
  BUMBLE_OP_PUSHI    = 0x1,
  BUMBLE_OP_PUSHRELI = 0x2,
  BUMBLE_OP_JUMPI    = 0x3,
  BUMBLE_OP_JUMPZI   = 0x4,
  BUMBLE_OP_INSN     = 0x7,
};
#endif

/* OP_INSN opcodes.  */
enum {
  BUMBLE_INSN_NOP = 0,
  BUMBLE_INSN_POP = 0x8,
  BUMBLE_INSN_DUP,
  BUMBLE_INSN_SET,
  BUMBLE_INSN_SWAP,
  BUMBLE_INSN_JUMP,
  BUMBLE_INSN_JUMPZ,
  BUMBLE_INSN_CALL,
  BUMBLE_INSN_RET,
  BUMBLE_INSN_PUSHS = 0x20,
  BUMBLE_INSN_POPS,
  BUMBLE_INSN_DUPS,
  BUMBLE_INSN_CATCH,
  BUMBLE_INSN_THROW,
  BUMBLE_INSN_BREAK,
  BUMBLE_INSN_GET_M0 = 0x27,
  BUMBLE_INSN_GET_MSIZE = 0x28,
  BUMBLE_INSN_GET_SSIZE,
  BUMBLE_INSN_GET_SP,
  BUMBLE_INSN_SET_SP,
  BUMBLE_INSN_GET_DSIZE,
  BUMBLE_INSN_GET_DP,
  BUMBLE_INSN_SET_DP,
  BUMBLE_INSN_GET_HANDLER_SP,

  BUMBLE_INSN_UNDEFINED = 0x3f
};
