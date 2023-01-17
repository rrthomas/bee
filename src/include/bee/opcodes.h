/* Bee instruction set description.

   (c) Reuben Thomas 1994-2022

   The package is distributed under the GNU General Public License version 3,
   or, at your option, any later version.

   THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
   RISK.  */


/* Instructions occupy a word, and take the following forms:

   32-bit Bee:

   1. Opcode in bits 0-1, rest of word is immediate operand.
      The opcodes are BEE_OP_* below masked with BEE_OP1_MASK.
   2. Bits 0-1 are 11, opcode in bits 2-3, rest of word is immediate operand.
      The opcodes are BEE_OP_* below masked with BEE_OP2_MASK.
   3. Bits 0-3 are 1111, rest of word is instruction opcode.
      The opcodes are BEE_INSN_* below.

   64-bit Bee:

   1. Opcode in bits 0-2, rest of word is immediate operand.
      The opcodes are BEE_OP_* below masked with BEE_OP1_MASK == BEE_OP2_MASK.
   2. Bits 0-3 are 111, rest of word is instruction opcode.
      The opcodes are BEE_INSN_* below.
*/

#include <limits.h>


/* Bee opcode info.  */
typedef struct bee_opc_info_t
{
  int opcode;		/* opcode bits for particular instruction.  */
  const char * name;	/* instruction name, where applicable.  */
} bee_opc_info_t;

#if BEE_UWORD_MAX == 4294967295U // BEE_WORD_BYTES == 4
#define BEE_OP1_SHIFT 2
#define BEE_OP2_SHIFT 4
/* 32-bit Instruction types.  */
enum {
  /* Bits 0-1.  */
  BEE_OP1_MASK    = 0x3,
  BEE_OP_CALLI    = 0x1,
  BEE_OP_PUSHI    = 0x2,
  BEE_OP_PUSHRELI = 0x3,

  /* Bits 0-3 when bits 0-1 are 00.  */
  BEE_OP2_MASK    = 0xf,
  BEE_OP_INSN     = 0x0,
  BEE_OP_JUMPI    = 0x4,
  BEE_OP_JUMPZI   = 0x8,
  BEE_OP_TRAP     = 0xc,
};
#else
/* 64-bit Instruction types.  */
#define BEE_OP1_SHIFT 3
#define BEE_OP2_SHIFT 3
enum {
  BEE_OP1_MASK    = 0x7,
  BEE_OP2_MASK    = 0x7,

  /* Bits 0-2.  */
  BEE_OP_INSN     = 0x0,
  BEE_OP_CALLI    = 0x1,
  BEE_OP_PUSHI    = 0x2,
  BEE_OP_PUSHRELI = 0x3,
  BEE_OP_JUMPI    = 0x4,
  BEE_OP_JUMPZI   = 0x5,
  BEE_OP_TRAP     = 0x7,
};
#endif

/* Largest trap code.  */
#define BEE_MAX_TRAP ((1L << (BEE_WORD_BIT - BEE_OP2_SHIFT)) - 1)

/* OP_INSN opcodes.  */
#define BEE_INSN_BITS 6
#define BEE_INSN_MASK ((1 << BEE_INSN_BITS) - 1)
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
  BEE_INSN_LOAD_IA = 0x18,
  BEE_INSN_POP_FD = 0x18,
  BEE_INSN_STORE_DB = 0x19,
  BEE_INSN_PUSH_FD = 0x19,
  BEE_INSN_LOAD_IB = 0x1a,
  BEE_INSN_POP_ED = 0x1a,
  BEE_INSN_STORE_DA = 0x1b,
  BEE_INSN_PUSH_ED = 0x1b,
  BEE_INSN_LOAD_DA = 0x1c,
  BEE_INSN_POP_FA = 0x1c,
  BEE_INSN_STORE_IB = 0x1d,
  BEE_INSN_PUSH_FA = 0x1d,
  BEE_INSN_LOAD_DB = 0x1e,
  BEE_INSN_POP_EA = 0x1e,
  BEE_INSN_STORE_IA = 0x1f,
  BEE_INSN_PUSH_EA = 0x1f,
  BEE_INSN_NEG = 0x20,
  BEE_INSN_ADD,
  BEE_INSN_MUL,
  BEE_INSN_DIVMOD,
  BEE_INSN_UDIVMOD,
  BEE_INSN_EQ,
  BEE_INSN_LT,
  BEE_INSN_ULT,
  BEE_INSN_PUSHS = 0x28,
  BEE_INSN_POPS,
  BEE_INSN_DUPS,
  BEE_INSN_CATCH,
  BEE_INSN_THROW,
  BEE_INSN_BREAK,
  BEE_INSN_WORD_BYTES,
  BEE_INSN_GET_SSIZE = 0x31,
  BEE_INSN_GET_SP,
  BEE_INSN_SET_SP,
  BEE_INSN_GET_DSIZE,
  BEE_INSN_GET_DP,
  BEE_INSN_SET_DP,
  BEE_INSN_GET_HANDLER_SP,

  BEE_INSN_UNDEFINED = 0x3f
};
