// The interface call run() : integer.
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

#include <inttypes.h>

#include "bee/bee.h"
#include "bee/aux.h"

#include "opcodes.h"
#include "private.h"


// Division macros
#define DIV_CATCH_ZERO(a, b) ((b) == 0 ? 0 : (a) / (b))
#define MOD_CATCH_ZERO(a, b) ((b) == 0 ? (a) : (a) % (b))
#define DIV_WOULD_OVERFLOW(a, b) (((a) == WORD_MIN) && ((b) == -1))
#define DIV_WITH_OVERFLOW(a, b) (DIV_WOULD_OVERFLOW((a), (b)) ? WORD_MIN : DIV_CATCH_ZERO((a), (b)))
#define MOD_WITH_OVERFLOW(a, b) (DIV_WOULD_OVERFLOW((a), (b)) ? 0 : MOD_CATCH_ZERO((a), (b)))


// Execution function
WORD run(void)
{
    for (;;) {
        WORD error = ERROR_OK, IR;
        THROW_IF_ERROR(load_word(PC, &IR));
        PC++;

        switch (IR & BEE_OP_MASK) {
        case BEE_OP_CALLI:
            {
                PUSH_RETURN((UWORD)PC);
                WORD *addr = (PC - 1) + (IR >> 2);
                CHECK_ALIGNED(addr);
                PC = addr;
            }
            break;
        case BEE_OP_PUSHI:
            PUSH(ARSHIFT(IR, 2));
            break;
        case BEE_OP_PUSHRELI:
            PUSH((UWORD)((PC - 1) + (IR >> 2)));
            break;
        case BEE_OP_LEVEL2:
            IR = ARSHIFT(IR, 2);
            switch (IR & BEE_OP2_MASK) {
            case BEE_OP2_JUMPI:
                {
                    WORD *addr = (PC - 1) + (IR >> 2);
                    CHECK_ALIGNED(addr);
                    PC = addr;
                }
                break;
            case BEE_OP2_JUMPZI:
                {
                    WORD *addr = (PC - 1) + (IR >> 2);
                    WORD flag;
                    POP(&flag);
                    if (flag == 0) {
                        CHECK_ALIGNED(addr);
                        PC = addr;
                    }
                }
                break;
            case BEE_OP2_INSN:
                {
                    WORD opcode = IR >> 2;
                    switch (opcode) {
                    case BEE_INSN_NOP:
                        break;
                    case BEE_INSN_NOT:
                        {
                            WORD a;
                            POP(&a);
                            PUSH(~a);
                        }
                        break;
                    case BEE_INSN_AND:
                        {
                            WORD a, b;
                            POP(&a);
                            POP(&b);
                            PUSH(a & b);
                        }
                        break;
                    case BEE_INSN_OR:
                        {
                            WORD a, b;
                            POP(&a);
                            POP(&b);
                            PUSH(a | b);
                        }
                        break;
                    case BEE_INSN_XOR:
                        {
                            WORD a, b;
                            POP(&a);
                            POP(&b);
                            PUSH(a ^ b);
                        }
                        break;
                    case BEE_INSN_LSHIFT:
                        {
                            WORD shift, value;
                            POP(&shift);
                            POP(&value);
                            PUSH(shift < (WORD)WORD_BIT ? LSHIFT(value, shift) : 0);
                        }
                        break;
                    case BEE_INSN_RSHIFT:
                        {
                            WORD shift, value;
                            POP(&shift);
                            POP(&value);
                            PUSH(shift < (WORD)WORD_BIT ? (WORD)((UWORD)value >> shift) : 0);
                        }
                        break;
                    case BEE_INSN_ARSHIFT:
                        {
                            WORD shift, value;
                            POP(&shift);
                            POP(&value);
                            PUSH(ARSHIFT(value, shift));
                        }
                        break;
                    case BEE_INSN_POP:
                        if (SP == 0)
                            THROW(ERROR_STACK_UNDERFLOW);
                        SP--;
                        break;
                    case BEE_INSN_DUP:
                        {
                            UWORD depth;
                            POP((WORD *)&depth);
                            if (depth >= SP)
                                THROW(ERROR_STACK_UNDERFLOW);
                            else
                                PUSH(S0[SP - (depth + 1)]);
                        }
                        break;
                    case BEE_INSN_SET:
                        {
                            UWORD depth;
                            POP((WORD *)&depth);
                            WORD value;
                            POP(&value);
                            if (depth >= SP)
                                THROW(ERROR_STACK_UNDERFLOW);
                            else
                                S0[SP - (depth + 1)] = value;
                        }
                        break;
                    case BEE_INSN_SWAP:
                        {
                            UWORD depth;
                            POP((WORD *)&depth);
                            if (SP == 0 || depth >= SP - 1)
                                THROW(ERROR_STACK_UNDERFLOW);
                            else {
                                WORD temp = S0[SP - (depth + 2)];
                                S0[SP - (depth + 2)] = S0[SP - 1];
                                S0[SP - 1] = temp;
                            }
                        }
                        break;
                    case BEE_INSN_JUMP:
                        {
                            WORD *addr;
                            POP((WORD *)&addr);
                            CHECK_ALIGNED(addr);
                            PC = addr;
                        }
                        break;
                    case BEE_INSN_JUMPZ:
                        {
                            WORD *addr;
                            POP((WORD *)&addr);
                            WORD flag;
                            POP(&flag);
                            if (flag == 0) {
                                CHECK_ALIGNED(addr);
                                PC = addr;
                            }
                        }
                        break;
                    case BEE_INSN_CALL:
                        {
                            WORD *addr;
                            POP((WORD *)&addr);
                            CHECK_ALIGNED(addr);
                            PUSH_RETURN((UWORD)PC);
                            PC = addr;
                        }
                        break;
                    case BEE_INSN_RET:
                        {
                            UWORD addr;
                            POP_RETURN((WORD *)&addr);
                            WORD *real_addr = (WORD *)(addr & ~1);
                            CHECK_ALIGNED(real_addr);
                            if ((addr & 1) == 1) {
                                POP_RETURN((WORD *)&HANDLER_RP);
                                PUSH(0);
                            }
                            PC = real_addr;
                        }
                        break;
                    case BEE_INSN_LOAD:
                        {
                            WORD *addr;
                            POP((WORD *)&addr);
                            if (!IS_ALIGNED(addr))
                                THROW(ERROR_UNALIGNED_ADDRESS);
                            WORD value;
                            THROW_IF_ERROR(load_word(addr, &value));
                            PUSH(value);
                        }
                        break;
                    case BEE_INSN_STORE:
                        {
                            WORD *addr;
                            POP((WORD *)&addr);
                            if (!IS_ALIGNED(addr))
                                THROW(ERROR_UNALIGNED_ADDRESS);
                            WORD value;
                            POP(&value);
                            THROW_IF_ERROR(store_word(addr, value));
                        }
                        break;
                    case BEE_INSN_LOAD1:
                        {
                            uint8_t *addr;
                            POP((WORD *)&addr);
                            uint8_t value;
                            THROW_IF_ERROR(load_byte(addr, &value));
                            PUSH((WORD)value);
                        }
                        break;
                    case BEE_INSN_STORE1:
                        {
                            uint8_t *addr;
                            POP((WORD *)&addr);
                            WORD value;
                            POP(&value);
                            THROW_IF_ERROR(store_byte(addr, (uint8_t)value));
                        }
                        break;
                    case BEE_INSN_LOAD2:
                        {
                            uint16_t *addr;
                            POP((WORD *)&addr);
                            if ((UWORD)addr % 2 != 0)
                                THROW(ERROR_UNALIGNED_ADDRESS);
                            PUSH(*addr);
                        }
                        break;
                    case BEE_INSN_STORE2:
                        {
                            uint16_t *addr;
                            POP((WORD *)&addr);
                            if ((UWORD)addr % 2 != 0)
                                THROW(ERROR_UNALIGNED_ADDRESS);
                            WORD value;
                            POP(&value);
                            *addr = (uint16_t)value;
                        }
                        break;
                    case BEE_INSN_LOAD4:
                        {
                            uint32_t *addr;
                            POP((WORD *)&addr);
                            if ((UWORD)addr % 4 != 0)
                                THROW(ERROR_UNALIGNED_ADDRESS);
                            PUSH(*addr);
                        }
                        break;
                    case BEE_INSN_STORE4:
                        {
                            uint32_t *addr;
                            POP((WORD *)&addr);
                            if ((UWORD)addr % 4 != 0)
                                THROW(ERROR_UNALIGNED_ADDRESS);
                            WORD value;
                            POP(&value);
                            *addr = (uint32_t)value;
                        }
                        break;
                    case BEE_INSN_NEGATE:
                        {
                            WORD a;
                            POP(&a);
                            PUSH(-a);
                        }
                        break;
                    case BEE_INSN_ADD:
                        {
                            WORD a, b;
                            POP(&a);
                            POP(&b);
                            PUSH(b + a);
                        }
                        break;
                    case BEE_INSN_MUL:
                        {
                            WORD a, b;
                            POP(&a);
                            POP(&b);
                            PUSH(a * b);
                        }
                        break;
                    case BEE_INSN_DIVMOD:
                        {
                            WORD divisor, dividend;
                            POP(&divisor);
                            POP(&dividend);
                            PUSH(DIV_WITH_OVERFLOW(dividend, divisor));
                            PUSH(MOD_WITH_OVERFLOW(dividend, divisor));
                        }
                        break;
                    case BEE_INSN_UDIVMOD:
                        {
                            UWORD divisor, dividend;
                            POP((WORD *)&divisor);
                            POP((WORD *)&dividend);
                            PUSH(DIV_CATCH_ZERO(dividend, divisor));
                            PUSH(MOD_CATCH_ZERO(dividend, divisor));
                        }
                        break;
                    case BEE_INSN_EQ:
                        {
                            WORD a, b;
                            POP(&a);
                            POP(&b);
                            PUSH(a == b);
                        }
                        break;
                    case BEE_INSN_LT:
                        {
                            WORD a, b;
                            POP(&a);
                            POP(&b);
                            PUSH(b < a);
                        }
                        break;
                    case BEE_INSN_ULT:
                        {
                            UWORD a, b;
                            POP((WORD *)&a);
                            POP((WORD *)&b);
                            PUSH(b < a);
                        }
                        break;
                    case BEE_INSN_PUSHR:
                        {
                            WORD value;
                            POP(&value);
                            PUSH_RETURN(value);
                        }
                        break;
                    case BEE_INSN_POPR:
                        {
                            WORD value;
                            POP_RETURN(&value);
                            if (error == ERROR_OK)
                                PUSH(value);
                        }
                        break;
                    case BEE_INSN_DUPR:
                        if (RP == 0)
                            THROW(ERROR_STACK_UNDERFLOW);
                        else {
                            WORD value = *stack_position(R0, RP, 0);
                            PUSH(value);
                        }
                        break;
                    case BEE_INSN_CATCH:
                        {
                            WORD *addr;
                            POP((WORD *)&addr);
                            CHECK_ALIGNED(addr);
                            PUSH_RETURN(HANDLER_RP);
                            PUSH_RETURN((UWORD)PC | 1);
                            HANDLER_RP = RP;
                            PC = addr;
                        }
                        break;
                    case BEE_INSN_THROW:
                        {
                            if (SP < 1)
                                error = ERROR_STACK_UNDERFLOW;
                            else
                                POP(&error);
                        error:
                            if (HANDLER_RP == (UWORD)-1)
                                return error;
                            // Don't push error code if the stack is full.
                            if (SP < SSIZE)
                                S0[SP++] = error;
                            RP = HANDLER_RP;
                            UWORD addr;
                            POP_RETURN((WORD *)&addr);
                            POP_RETURN((WORD *)&HANDLER_RP);
                            PC = (WORD *)(addr & ~1);
                        }
                        break;
                    case BEE_INSN_BREAK:
                        PC--;
                        return ERROR_BREAK;
                    case BEE_INSN_WORD_BYTES:
                        PUSH(WORD_BYTES);
                        break;
                    case BEE_INSN_GET_M0:
                        PUSH((UWORD)M0);
                        break;
                    case BEE_INSN_GET_MSIZE:
                        PUSH(MSIZE);
                        break;
                    case BEE_INSN_GET_RSIZE:
                        PUSH(RSIZE);
                        break;
                    case BEE_INSN_GET_RP:
                        PUSH(RP);
                        break;
                    case BEE_INSN_SET_RP:
                        POP((WORD *)&RP);
                        break;
                    case BEE_INSN_GET_SSIZE:
                        PUSH(SSIZE);
                        break;
                    case BEE_INSN_GET_SP:
                        {
                            WORD value = SP;
                            PUSH(value);
                        }
                        break;
                    case BEE_INSN_SET_SP:
                        {
                            WORD value;
                            POP(&value);
                            SP = value;
                        }
                        break;
                    case BEE_INSN_GET_HANDLER_RP:
                        PUSH(HANDLER_RP);
                        break;
                    default:
                        THROW(ERROR_INVALID_OPCODE);
                        break;
                    }
                }
                break;
            case BEE_OP2_TRAP:
                THROW_IF_ERROR(trap(IR >> 2));
                break;
            }
        }
    }
}
