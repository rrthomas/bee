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
WORD bee_run(void)
{
    for (;;) {
        WORD error = BEE_ERROR_OK, IR = *PC++;

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
            case BEE_OP2_TRAP:
                THROW_IF_ERROR(trap(IR >> 2));
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
                        if (DP == 0)
                            THROW(BEE_ERROR_STACK_UNDERFLOW);
                        DP--;
                        break;
                    case BEE_INSN_DUP:
                        {
                            UWORD depth;
                            POP((WORD *)&depth);
                            if (depth >= DP)
                                THROW(BEE_ERROR_STACK_UNDERFLOW);
                            else
                                PUSH(D0[DP - (depth + 1)]);
                        }
                        break;
                    case BEE_INSN_SET:
                        {
                            UWORD depth;
                            POP((WORD *)&depth);
                            WORD value;
                            POP(&value);
                            if (depth >= DP)
                                THROW(BEE_ERROR_STACK_UNDERFLOW);
                            else
                                D0[DP - (depth + 1)] = value;
                        }
                        break;
                    case BEE_INSN_SWAP:
                        {
                            UWORD depth;
                            POP((WORD *)&depth);
                            if (DP == 0 || depth >= DP - 1)
                                THROW(BEE_ERROR_STACK_UNDERFLOW);
                            else {
                                WORD temp = D0[DP - (depth + 2)];
                                D0[DP - (depth + 2)] = D0[DP - 1];
                                D0[DP - 1] = temp;
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
                                POP_RETURN((WORD *)&HANDLER_SP);
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
                                THROW(BEE_ERROR_UNALIGNED_ADDRESS);
                            PUSH(*addr);
                        }
                        break;
                    case BEE_INSN_STORE:
                        {
                            WORD *addr;
                            POP((WORD *)&addr);
                            if (!IS_ALIGNED(addr))
                                THROW(BEE_ERROR_UNALIGNED_ADDRESS);
                            WORD value;
                            POP(&value);
                            *addr = value;
                        }
                        break;
                    case BEE_INSN_LOAD1:
                        {
                            uint8_t *addr;
                            POP((WORD *)&addr);
                            uint8_t value = *(uint8_t *)addr;
                            PUSH((WORD)value);
                        }
                        break;
                    case BEE_INSN_STORE1:
                        {
                            uint8_t *addr;
                            POP((WORD *)&addr);
                            WORD value;
                            POP(&value);
                            *(uint8_t *)addr = (uint8_t)value;
                        }
                        break;
                    case BEE_INSN_LOAD2:
                        {
                            uint16_t *addr;
                            POP((WORD *)&addr);
                            if ((UWORD)addr % 2 != 0)
                                THROW(BEE_ERROR_UNALIGNED_ADDRESS);
                            PUSH(*addr);
                        }
                        break;
                    case BEE_INSN_STORE2:
                        {
                            uint16_t *addr;
                            POP((WORD *)&addr);
                            if ((UWORD)addr % 2 != 0)
                                THROW(BEE_ERROR_UNALIGNED_ADDRESS);
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
                                THROW(BEE_ERROR_UNALIGNED_ADDRESS);
                            PUSH(*addr);
                        }
                        break;
                    case BEE_INSN_STORE4:
                        {
                            uint32_t *addr;
                            POP((WORD *)&addr);
                            if ((UWORD)addr % 4 != 0)
                                THROW(BEE_ERROR_UNALIGNED_ADDRESS);
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
                            if (error == BEE_ERROR_OK)
                                PUSH(value);
                        }
                        break;
                    case BEE_INSN_DUPR:
                        if (SP == 0)
                            THROW(BEE_ERROR_STACK_UNDERFLOW);
                        else {
                            WORD value = *bee_stack_position(S0, SP, 0);
                            PUSH(value);
                        }
                        break;
                    case BEE_INSN_CATCH:
                        {
                            WORD *addr;
                            POP((WORD *)&addr);
                            CHECK_ALIGNED(addr);
                            PUSH_RETURN(HANDLER_SP);
                            PUSH_RETURN((UWORD)PC | 1);
                            HANDLER_SP = SP;
                            PC = addr;
                        }
                        break;
                    case BEE_INSN_THROW:
                        {
                            if (DP < 1)
                                error = BEE_ERROR_STACK_UNDERFLOW;
                            else
                                POP(&error);
                        error:
                            if (HANDLER_SP == (UWORD)-1)
                                return error;
                            // Don't push error code if the stack is full.
                            if (DP < DSIZE)
                                D0[DP++] = error;
                            SP = HANDLER_SP;
                            UWORD addr;
                            POP_RETURN((WORD *)&addr);
                            POP_RETURN((WORD *)&HANDLER_SP);
                            PC = (WORD *)(addr & ~1);
                        }
                        break;
                    case BEE_INSN_BREAK:
                        PC--;
                        return BEE_ERROR_BREAK;
                    case BEE_INSN_WORD_BYTES:
                        PUSH(WORD_BYTES);
                        break;
                    case BEE_INSN_GET_M0:
                        PUSH((UWORD)M0);
                        break;
                    case BEE_INSN_GET_MSIZE:
                        PUSH(MSIZE);
                        break;
                    case BEE_INSN_GET_SSIZE:
                        PUSH(SSIZE);
                        break;
                    case BEE_INSN_GET_SP:
                        PUSH(SP);
                        break;
                    case BEE_INSN_SET_SP:
                        POP((WORD *)&SP);
                        break;
                    case BEE_INSN_GET_DSIZE:
                        PUSH(DSIZE);
                        break;
                    case BEE_INSN_GET_DP:
                        {
                            WORD value = DP;
                            PUSH(value);
                        }
                        break;
                    case BEE_INSN_SET_DP:
                        {
                            WORD value;
                            POP(&value);
                            DP = value;
                        }
                        break;
                    case BEE_INSN_GET_HANDLER_SP:
                        PUSH(HANDLER_SP);
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
