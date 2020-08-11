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
bee_word_t bee_run(void)
{
    for (;;) {
        bee_word_t error = BEE_ERROR_OK, ir = *bee_pc++;

        switch (ir & BEE_OP_MASK1) {
        case BEE_OP_CALLI:
            {
                PUSH_RETURN((bee_uword_t)bee_pc);
                bee_word_t *addr = (bee_pc - 1) + ARSHIFT(ir, 2);
                CHECK_ALIGNED(addr);
                bee_pc = addr;
            }
            break;
        case BEE_OP_PUSHI:
            PUSH(ARSHIFT(ir, 2));
            break;
        case BEE_OP_PUSHRELI:
            PUSH((bee_uword_t)((bee_pc - 1) + ARSHIFT(ir, 2)));
            break;
        default:
            switch (ir & BEE_OP_MASK2) {
            case BEE_OP_JUMPI:
                {
                    bee_word_t *addr = (bee_pc - 1) + ARSHIFT(ir, 4);
                    CHECK_ALIGNED(addr);
                    bee_pc = addr;
                }
                break;
            case BEE_OP_JUMPZI:
                {
                    bee_word_t *addr = (bee_pc - 1) + ARSHIFT(ir, 4);
                    bee_word_t flag;
                    POP(&flag);
                    if (flag == 0) {
                        CHECK_ALIGNED(addr);
                        bee_pc = addr;
                    }
                }
                break;
            case BEE_OP_TRAP:
                THROW_IF_ERROR(trap((bee_uword_t)ir >> 4));
                break;
            case BEE_OP_INSN:
                {
                    bee_uword_t opcode = (bee_uword_t)ir >> 4;
                    switch (opcode) {
                    case BEE_INSN_NOP:
                        break;
                    case BEE_INSN_NOT:
                        {
                            bee_word_t a;
                            POP(&a);
                            PUSH(~a);
                        }
                        break;
                    case BEE_INSN_AND:
                        {
                            bee_word_t a, b;
                            POP(&a);
                            POP(&b);
                            PUSH(a & b);
                        }
                        break;
                    case BEE_INSN_OR:
                        {
                            bee_word_t a, b;
                            POP(&a);
                            POP(&b);
                            PUSH(a | b);
                        }
                        break;
                    case BEE_INSN_XOR:
                        {
                            bee_word_t a, b;
                            POP(&a);
                            POP(&b);
                            PUSH(a ^ b);
                        }
                        break;
                    case BEE_INSN_LSHIFT:
                        {
                            bee_word_t shift, value;
                            POP(&shift);
                            POP(&value);
                            PUSH(shift < (bee_word_t)BEE_WORD_BIT ? LSHIFT(value, shift) : 0);
                        }
                        break;
                    case BEE_INSN_RSHIFT:
                        {
                            bee_word_t shift, value;
                            POP(&shift);
                            POP(&value);
                            PUSH(shift < (bee_word_t)BEE_WORD_BIT ? (bee_word_t)((bee_uword_t)value >> shift) : 0);
                        }
                        break;
                    case BEE_INSN_ARSHIFT:
                        {
                            bee_word_t shift, value;
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
                            bee_uword_t depth;
                            POP((bee_word_t *)&depth);
                            if (depth >= bee_dp)
                                THROW(BEE_ERROR_STACK_UNDERFLOW);
                            else
                                PUSH(bee_d0[bee_dp - (depth + 1)]);
                        }
                        break;
                    case BEE_INSN_SET:
                        {
                            bee_uword_t depth;
                            POP((bee_word_t *)&depth);
                            bee_word_t value;
                            POP(&value);
                            if (depth >= bee_dp)
                                THROW(BEE_ERROR_STACK_UNDERFLOW);
                            else
                                bee_d0[bee_dp - (depth + 1)] = value;
                        }
                        break;
                    case BEE_INSN_SWAP:
                        {
                            bee_uword_t depth;
                            POP((bee_word_t *)&depth);
                            if (bee_dp == 0 || depth >= bee_dp - 1)
                                THROW(BEE_ERROR_STACK_UNDERFLOW);
                            else {
                                bee_word_t temp = bee_d0[bee_dp - (depth + 2)];
                                bee_d0[bee_dp - (depth + 2)] = bee_d0[bee_dp - 1];
                                bee_d0[bee_dp - 1] = temp;
                            }
                        }
                        break;
                    case BEE_INSN_JUMP:
                        {
                            bee_word_t *addr;
                            POP((bee_word_t *)&addr);
                            CHECK_ALIGNED(addr);
                            bee_pc = addr;
                        }
                        break;
                    case BEE_INSN_JUMPZ:
                        {
                            bee_word_t *addr;
                            POP((bee_word_t *)&addr);
                            bee_word_t flag;
                            POP(&flag);
                            if (flag == 0) {
                                CHECK_ALIGNED(addr);
                                bee_pc = addr;
                            }
                        }
                        break;
                    case BEE_INSN_CALL:
                        {
                            bee_word_t *addr;
                            POP((bee_word_t *)&addr);
                            CHECK_ALIGNED(addr);
                            PUSH_RETURN((bee_uword_t)bee_pc);
                            bee_pc = addr;
                        }
                        break;
                    case BEE_INSN_RET:
                        {
                            bee_word_t *addr;
                            POP_RETURN((bee_word_t *)&addr);
                            CHECK_ALIGNED(addr);
                            if (bee_sp < bee_handler_sp) {
                                POP_RETURN((bee_word_t *)&bee_handler_sp);
                                PUSH(0);
                            }
                            bee_pc = addr;
                        }
                        break;
                    case BEE_INSN_LOAD:
                        {
                            bee_word_t *addr;
                            POP((bee_word_t *)&addr);
                            CHECK_ALIGNED(addr);
                            PUSH(*addr);
                        }
                        break;
                    case BEE_INSN_STORE:
                        {
                            bee_word_t *addr;
                            POP((bee_word_t *)&addr);
                            CHECK_ALIGNED(addr);
                            bee_word_t value;
                            POP(&value);
                            *addr = value;
                        }
                        break;
                    case BEE_INSN_LOAD1:
                        {
                            uint8_t *addr;
                            POP((bee_word_t *)&addr);
                            uint8_t value = *(uint8_t *)addr;
                            PUSH((bee_word_t)value);
                        }
                        break;
                    case BEE_INSN_STORE1:
                        {
                            uint8_t *addr;
                            POP((bee_word_t *)&addr);
                            bee_word_t value;
                            POP(&value);
                            *(uint8_t *)addr = (uint8_t)value;
                        }
                        break;
                    case BEE_INSN_LOAD2:
                        {
                            uint16_t *addr;
                            POP((bee_word_t *)&addr);
                            if ((bee_uword_t)addr % 2 != 0)
                                THROW(BEE_ERROR_UNALIGNED_ADDRESS);
                            PUSH(*addr);
                        }
                        break;
                    case BEE_INSN_STORE2:
                        {
                            uint16_t *addr;
                            POP((bee_word_t *)&addr);
                            if ((bee_uword_t)addr % 2 != 0)
                                THROW(BEE_ERROR_UNALIGNED_ADDRESS);
                            bee_word_t value;
                            POP(&value);
                            *addr = (uint16_t)value;
                        }
                        break;
                    case BEE_INSN_LOAD4:
                        {
                            uint32_t *addr;
                            POP((bee_word_t *)&addr);
                            if ((bee_uword_t)addr % 4 != 0)
                                THROW(BEE_ERROR_UNALIGNED_ADDRESS);
                            PUSH(*addr);
                        }
                        break;
                    case BEE_INSN_STORE4:
                        {
                            uint32_t *addr;
                            POP((bee_word_t *)&addr);
                            if ((bee_uword_t)addr % 4 != 0)
                                THROW(BEE_ERROR_UNALIGNED_ADDRESS);
                            bee_word_t value;
                            POP(&value);
                            *addr = (uint32_t)value;
                        }
                        break;
                    case BEE_INSN_NEG:
                        {
                            bee_uword_t a;
                            POP((bee_word_t *)&a);
                            PUSH((bee_word_t)-a);
                        }
                        break;
                    case BEE_INSN_ADD:
                        {
                            bee_uword_t a, b;
                            POP((bee_word_t *)&a);
                            POP((bee_word_t *)&b);
                            PUSH((bee_word_t)(b + a));
                        }
                        break;
                    case BEE_INSN_MUL:
                        {
                            bee_uword_t a, b;
                            POP((bee_word_t *)&a);
                            POP((bee_word_t *)&b);
                            PUSH((bee_word_t)(a * b));
                        }
                        break;
                    case BEE_INSN_DIVMOD:
                        {
                            bee_word_t divisor, dividend;
                            POP(&divisor);
                            POP(&dividend);
                            if (dividend == BEE_WORD_MIN && divisor == -1) {
                                PUSH(BEE_WORD_MIN);
                                PUSH(0);
                            } else {
                                PUSH(DIV_CATCH_ZERO(dividend, divisor));
                                PUSH(MOD_CATCH_ZERO(dividend, divisor));
                            }
                        }
                        break;
                    case BEE_INSN_UDIVMOD:
                        {
                            bee_uword_t divisor, dividend;
                            POP((bee_word_t *)&divisor);
                            POP((bee_word_t *)&dividend);
                            PUSH(DIV_CATCH_ZERO(dividend, divisor));
                            PUSH(MOD_CATCH_ZERO(dividend, divisor));
                        }
                        break;
                    case BEE_INSN_EQ:
                        {
                            bee_word_t a, b;
                            POP(&a);
                            POP(&b);
                            PUSH(a == b);
                        }
                        break;
                    case BEE_INSN_LT:
                        {
                            bee_word_t a, b;
                            POP(&a);
                            POP(&b);
                            PUSH(b < a);
                        }
                        break;
                    case BEE_INSN_ULT:
                        {
                            bee_uword_t a, b;
                            POP((bee_word_t *)&a);
                            POP((bee_word_t *)&b);
                            PUSH(b < a);
                        }
                        break;
                    case BEE_INSN_PUSHR:
                        {
                            bee_word_t value;
                            POP(&value);
                            PUSH_RETURN(value);
                        }
                        break;
                    case BEE_INSN_POPR:
                        {
                            bee_word_t value;
                            POP_RETURN(&value);
                            if (error == BEE_ERROR_OK)
                                PUSH(value);
                        }
                        break;
                    case BEE_INSN_DUPR:
                        if (bee_sp == 0)
                            THROW(BEE_ERROR_STACK_UNDERFLOW);
                        else {
                            bee_word_t value = bee_s0[bee_sp - 1];
                            PUSH(value);
                        }
                        break;
                    case BEE_INSN_CATCH:
                        {
                            bee_word_t *addr;
                            POP((bee_word_t *)&addr);
                            CHECK_ALIGNED(addr);
                            PUSH_RETURN(bee_handler_sp);
                            PUSH_RETURN((bee_uword_t)bee_pc);
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
                            if (bee_handler_sp == 0)
                                return error;
                            // Don't push error code if the stack is full.
                            if (bee_dp < bee_dsize)
                                bee_d0[bee_dp++] = error;
                            bee_sp = bee_handler_sp;
                            bee_word_t *addr;
                            POP_RETURN((bee_word_t *)&addr);
                            POP_RETURN((bee_word_t *)&bee_handler_sp);
                            bee_pc = addr;
                        }
                        break;
                    case BEE_INSN_BREAK:
                        bee_pc--;
                        return BEE_ERROR_BREAK;
                    case BEE_INSN_WORD_BYTES:
                        PUSH(BEE_WORD_BYTES);
                        break;
                    case BEE_INSN_GET_M0:
                        PUSH((bee_uword_t)bee_m0);
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
                        POP((bee_word_t *)&bee_sp);
                        break;
                    case BEE_INSN_GET_DSIZE:
                        PUSH(bee_dsize);
                        break;
                    case BEE_INSN_GET_DP:
                        {
                            bee_word_t value = bee_dp;
                            PUSH(value);
                        }
                        break;
                    case BEE_INSN_SET_DP:
                        {
                            bee_word_t value;
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
