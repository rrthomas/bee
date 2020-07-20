// The interpreter main loop.
//
// (c) Reuben Thomas 1994-2020
//
// The package is distributed under the GNU Public License version 3, or,
// at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#include "config.h"

#include <inttypes.h>

#include "bee/bee.h"
#include "bee/opcodes.h"

#include "private.h"


// Address checking
#define CHECK_ALIGNED(a)                                        \
    if (!IS_ALIGNED(a))                                         \
        THROW(BEE_ERROR_UNALIGNED_ADDRESS);

// Division macros
#define DIV_CATCH_ZERO(a, b) ((b) == 0 ? 0 : (a) / (b))
#define MOD_CATCH_ZERO(a, b) ((b) == 0 ? (a) : (a) % (b))


// Execution function
bee_WORD bee_run(void)
{
    for (;;) {
        bee_WORD error = BEE_ERROR_OK, ir = *bee_pc++;

        switch (ir & BEE_OP_MASK) {
        case BEE_OP_CALLI:
            {
                PUSH_RETURN((bee_UWORD)bee_pc);
                bee_WORD *addr = (bee_pc - 1) + (ir >> 2);
                CHECK_ALIGNED(addr);
                bee_pc = addr;
            }
            break;
        case BEE_OP_PUSHI:
            PUSH(ARSHIFT(ir, 2));
            break;
        case BEE_OP_PUSHRELI:
            PUSH((bee_UWORD)((bee_pc - 1) + (ir >> 2)));
            break;
        case BEE_OP_LEVEL2:
            ir = ARSHIFT(ir, 2);
            switch (ir & BEE_OP2_MASK) {
            case BEE_OP2_JUMPI:
                {
                    bee_WORD *addr = (bee_pc - 1) + (ir >> 2);
                    CHECK_ALIGNED(addr);
                    bee_pc = addr;
                }
                break;
            case BEE_OP2_JUMPZI:
                {
                    bee_WORD *addr = (bee_pc - 1) + (ir >> 2);
                    bee_WORD flag;
                    POP(&flag);
                    if (flag == 0) {
                        CHECK_ALIGNED(addr);
                        bee_pc = addr;
                    }
                }
                break;
            case BEE_OP2_TRAP:
                THROW_IF_ERROR(trap(ir >> 2));
                break;
            case BEE_OP2_INSN:
                {
                    bee_WORD opcode = ir >> 2;
                    switch (opcode) {
                    case BEE_INSN_NOP:
                        break;
                    case BEE_INSN_NOT:
                        {
                            bee_WORD a;
                            POP(&a);
                            PUSH(~a);
                        }
                        break;
                    case BEE_INSN_AND:
                        {
                            bee_WORD a, b;
                            POP(&a);
                            POP(&b);
                            PUSH(a & b);
                        }
                        break;
                    case BEE_INSN_OR:
                        {
                            bee_WORD a, b;
                            POP(&a);
                            POP(&b);
                            PUSH(a | b);
                        }
                        break;
                    case BEE_INSN_XOR:
                        {
                            bee_WORD a, b;
                            POP(&a);
                            POP(&b);
                            PUSH(a ^ b);
                        }
                        break;
                    case BEE_INSN_LSHIFT:
                        {
                            bee_WORD shift, value;
                            POP(&shift);
                            POP(&value);
                            PUSH(shift < (bee_WORD)bee_WORD_BIT ? LSHIFT(value, shift) : 0);
                        }
                        break;
                    case BEE_INSN_RSHIFT:
                        {
                            bee_WORD shift, value;
                            POP(&shift);
                            POP(&value);
                            PUSH(shift < (bee_WORD)bee_WORD_BIT ? (bee_WORD)((bee_UWORD)value >> shift) : 0);
                        }
                        break;
                    case BEE_INSN_ARSHIFT:
                        {
                            bee_WORD shift, value;
                            POP(&shift);
                            POP(&value);
                            PUSH(ARSHIFT(value, shift));
                        }
                        break;
                    case BEE_INSN_POP:
                        if (bee_dp == 0)
                            THROW(BEE_ERROR_STACK_UNDERFLOW);
                        bee_dp--;
                        break;
                    case BEE_INSN_DUP:
                        {
                            bee_UWORD depth;
                            POP((bee_WORD *)&depth);
                            if (depth >= bee_dp)
                                THROW(BEE_ERROR_STACK_UNDERFLOW);
                            else
                                PUSH(bee_d0[bee_dp - (depth + 1)]);
                        }
                        break;
                    case BEE_INSN_SET:
                        {
                            bee_UWORD depth;
                            POP((bee_WORD *)&depth);
                            bee_WORD value;
                            POP(&value);
                            if (depth >= bee_dp)
                                THROW(BEE_ERROR_STACK_UNDERFLOW);
                            else
                                bee_d0[bee_dp - (depth + 1)] = value;
                        }
                        break;
                    case BEE_INSN_SWAP:
                        {
                            bee_UWORD depth;
                            POP((bee_WORD *)&depth);
                            if (bee_dp == 0 || depth >= bee_dp - 1)
                                THROW(BEE_ERROR_STACK_UNDERFLOW);
                            else {
                                bee_WORD temp = bee_d0[bee_dp - (depth + 2)];
                                bee_d0[bee_dp - (depth + 2)] = bee_d0[bee_dp - 1];
                                bee_d0[bee_dp - 1] = temp;
                            }
                        }
                        break;
                    case BEE_INSN_JUMP:
                        {
                            bee_WORD *addr;
                            POP((bee_WORD *)&addr);
                            CHECK_ALIGNED(addr);
                            bee_pc = addr;
                        }
                        break;
                    case BEE_INSN_JUMPZ:
                        {
                            bee_WORD *addr;
                            POP((bee_WORD *)&addr);
                            bee_WORD flag;
                            POP(&flag);
                            if (flag == 0) {
                                CHECK_ALIGNED(addr);
                                bee_pc = addr;
                            }
                        }
                        break;
                    case BEE_INSN_CALL:
                        {
                            bee_WORD *addr;
                            POP((bee_WORD *)&addr);
                            CHECK_ALIGNED(addr);
                            PUSH_RETURN((bee_UWORD)bee_pc);
                            bee_pc = addr;
                        }
                        break;
                    case BEE_INSN_RET:
                        {
                            bee_UWORD addr;
                            POP_RETURN((bee_WORD *)&addr);
                            bee_WORD *real_addr = (bee_WORD *)(addr & ~1);
                            CHECK_ALIGNED(real_addr);
                            if ((addr & 1) == 1) {
                                POP_RETURN((bee_WORD *)&bee_handler_sp);
                                PUSH(0);
                            }
                            bee_pc = real_addr;
                        }
                        break;
                    case BEE_INSN_LOAD:
                        {
                            bee_WORD *addr;
                            POP((bee_WORD *)&addr);
                            CHECK_ALIGNED(addr);
                            PUSH(*addr);
                        }
                        break;
                    case BEE_INSN_STORE:
                        {
                            bee_WORD *addr;
                            POP((bee_WORD *)&addr);
                            CHECK_ALIGNED(addr);
                            bee_WORD value;
                            POP(&value);
                            *addr = value;
                        }
                        break;
                    case BEE_INSN_LOAD1:
                        {
                            uint8_t *addr;
                            POP((bee_WORD *)&addr);
                            uint8_t value = *(uint8_t *)addr;
                            PUSH((bee_WORD)value);
                        }
                        break;
                    case BEE_INSN_STORE1:
                        {
                            uint8_t *addr;
                            POP((bee_WORD *)&addr);
                            bee_WORD value;
                            POP(&value);
                            *(uint8_t *)addr = (uint8_t)value;
                        }
                        break;
                    case BEE_INSN_LOAD2:
                        {
                            uint16_t *addr;
                            POP((bee_WORD *)&addr);
                            if ((bee_UWORD)addr % 2 != 0)
                                THROW(BEE_ERROR_UNALIGNED_ADDRESS);
                            PUSH(*addr);
                        }
                        break;
                    case BEE_INSN_STORE2:
                        {
                            uint16_t *addr;
                            POP((bee_WORD *)&addr);
                            if ((bee_UWORD)addr % 2 != 0)
                                THROW(BEE_ERROR_UNALIGNED_ADDRESS);
                            bee_WORD value;
                            POP(&value);
                            *addr = (uint16_t)value;
                        }
                        break;
                    case BEE_INSN_LOAD4:
                        {
                            uint32_t *addr;
                            POP((bee_WORD *)&addr);
                            if ((bee_UWORD)addr % 4 != 0)
                                THROW(BEE_ERROR_UNALIGNED_ADDRESS);
                            PUSH(*addr);
                        }
                        break;
                    case BEE_INSN_STORE4:
                        {
                            uint32_t *addr;
                            POP((bee_WORD *)&addr);
                            if ((bee_UWORD)addr % 4 != 0)
                                THROW(BEE_ERROR_UNALIGNED_ADDRESS);
                            bee_WORD value;
                            POP(&value);
                            *addr = (uint32_t)value;
                        }
                        break;
                    case BEE_INSN_NEG:
                        {
                            bee_UWORD a;
                            POP((bee_WORD *)&a);
                            PUSH((bee_WORD)-a);
                        }
                        break;
                    case BEE_INSN_ADD:
                        {
                            bee_UWORD a, b;
                            POP((bee_WORD *)&a);
                            POP((bee_WORD *)&b);
                            PUSH((bee_WORD)(b + a));
                        }
                        break;
                    case BEE_INSN_MUL:
                        {
                            bee_UWORD a, b;
                            POP((bee_WORD *)&a);
                            POP((bee_WORD *)&b);
                            PUSH((bee_WORD)(a * b));
                        }
                        break;
                    case BEE_INSN_DIVMOD:
                        {
                            bee_WORD divisor, dividend;
                            POP(&divisor);
                            POP(&dividend);
                            if (dividend == bee_WORD_MIN && divisor == -1) {
                                PUSH(bee_WORD_MIN);
                                PUSH(0);
                            } else {
                                PUSH(DIV_CATCH_ZERO(dividend, divisor));
                                PUSH(MOD_CATCH_ZERO(dividend, divisor));
                            }
                        }
                        break;
                    case BEE_INSN_UDIVMOD:
                        {
                            bee_UWORD divisor, dividend;
                            POP((bee_WORD *)&divisor);
                            POP((bee_WORD *)&dividend);
                            PUSH(DIV_CATCH_ZERO(dividend, divisor));
                            PUSH(MOD_CATCH_ZERO(dividend, divisor));
                        }
                        break;
                    case BEE_INSN_EQ:
                        {
                            bee_WORD a, b;
                            POP(&a);
                            POP(&b);
                            PUSH(a == b);
                        }
                        break;
                    case BEE_INSN_LT:
                        {
                            bee_WORD a, b;
                            POP(&a);
                            POP(&b);
                            PUSH(b < a);
                        }
                        break;
                    case BEE_INSN_ULT:
                        {
                            bee_UWORD a, b;
                            POP((bee_WORD *)&a);
                            POP((bee_WORD *)&b);
                            PUSH(b < a);
                        }
                        break;
                    case BEE_INSN_PUSHR:
                        {
                            bee_WORD value;
                            POP(&value);
                            PUSH_RETURN(value);
                        }
                        break;
                    case BEE_INSN_POPR:
                        {
                            bee_WORD value;
                            POP_RETURN(&value);
                            if (error == BEE_ERROR_OK)
                                PUSH(value);
                        }
                        break;
                    case BEE_INSN_DUPR:
                        if (bee_sp == 0)
                            THROW(BEE_ERROR_STACK_UNDERFLOW);
                        else {
                            bee_WORD value = bee_s0[bee_sp - 1];
                            PUSH(value);
                        }
                        break;
                    case BEE_INSN_CATCH:
                        {
                            bee_WORD *addr;
                            POP((bee_WORD *)&addr);
                            CHECK_ALIGNED(addr);
                            PUSH_RETURN(bee_handler_sp);
                            PUSH_RETURN((bee_UWORD)bee_pc | 1);
                            bee_handler_sp = bee_sp;
                            bee_pc = addr;
                        }
                        break;
                    case BEE_INSN_THROW:
                        {
                            if (bee_dp < 1)
                                error = BEE_ERROR_STACK_UNDERFLOW;
                            else
                                POP(&error);
                        error:
                            if (bee_handler_sp == (bee_UWORD)-1)
                                return error;
                            // Don't push error code if the stack is full.
                            if (bee_dp < bee_dsize)
                                bee_d0[bee_dp++] = error;
                            bee_sp = bee_handler_sp;
                            bee_UWORD addr;
                            POP_RETURN((bee_WORD *)&addr);
                            POP_RETURN((bee_WORD *)&bee_handler_sp);
                            bee_pc = (bee_WORD *)(addr & ~1);
                        }
                        break;
                    case BEE_INSN_BREAK:
                        bee_pc--;
                        return BEE_ERROR_BREAK;
                    case BEE_INSN_WORD_BYTES:
                        PUSH(bee_WORD_BYTES);
                        break;
                    case BEE_INSN_GET_M0:
                        PUSH((bee_UWORD)bee_m0);
                        break;
                    case BEE_INSN_GET_MSIZE:
                        PUSH(bee_msize);
                        break;
                    case BEE_INSN_GET_SSIZE:
                        PUSH(bee_ssize);
                        break;
                    case BEE_INSN_GET_SP:
                        PUSH(bee_sp);
                        break;
                    case BEE_INSN_SET_SP:
                        POP((bee_WORD *)&bee_sp);
                        break;
                    case BEE_INSN_GET_DSIZE:
                        PUSH(bee_dsize);
                        break;
                    case BEE_INSN_GET_DP:
                        {
                            bee_WORD value = bee_dp;
                            PUSH(value);
                        }
                        break;
                    case BEE_INSN_SET_DP:
                        {
                            bee_WORD value;
                            POP(&value);
                            bee_dp = value;
                        }
                        break;
                    case BEE_INSN_GET_HANDLER_SP:
                        PUSH(bee_handler_sp);
                        break;
                    default:
                        THROW(BEE_ERROR_INVALID_OPCODE);
                        break;
                    }
                }
                break;
            }
        }
    }
}
