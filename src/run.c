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

#include "bee.h"
#include "bee_aux.h"
#include "private.h"
#include "bee_opcodes.h"


#define CHECK_VALID_WORD(a)                                           \
    CHECK_ADDRESS(a, IS_ALIGNED(a), ERROR_UNALIGNED_ADDRESS, error)   \
    CHECK_ADDRESS(a, IS_VALID(a), ERROR_INVALID_LOAD, error)


// Division macros
#define DIV_CATCH_ZERO(a, b) ((b) == 0 ? 0 : (a) / (b))
#define MOD_CATCH_ZERO(a, b) ((b) == 0 ? (a) : (a) % (b))
#define DIV_WOULD_OVERFLOW(a, b) (((a) == WORD_MIN) && ((b) == -1))
#define DIV_WITH_OVERFLOW(a, b) (DIV_WOULD_OVERFLOW((a), (b)) ? WORD_MIN : DIV_CATCH_ZERO((a), (b)))
#define MOD_WITH_OVERFLOW(a, b) (DIV_WOULD_OVERFLOW((a), (b)) ? 0 : MOD_CATCH_ZERO((a), (b)))


// Execution function
WORD run(void)
{
    int error = ERROR_OK;
    HANDLER_RP = (UWORD)-1;
    do {
        WORD temp = 0, opcode;
        uint8_t byte = 0;
        WORD A = LOAD_WORD(PC);

        PC += WORD_BYTES;

        switch (A & OP_MASK) {
        case OP_CALL:
            PUSH_RETURN(PC);
            CHECK_VALID_WORD(PC + A);
            PC += A - WORD_BYTES;
            break;
        case OP_PUSH:
            temp = A;
            ARSHIFT(temp, 2);
            PUSH(temp);
            break;
        case OP_PUSHREL:
            PUSH(PC - WORD_BYTES + (A & ~OP_MASK));
            break;
        case OP_INSTRUCTION:
            opcode = A >> 2;
            switch (opcode) {
            case O_NOP:
                break;
            case O_NOT:
                {
                    WORD a = POP;
                    PUSH(~a);
                }
                break;
            case O_AND:
                {
                    WORD a = POP;
                    WORD b = POP;
                    PUSH(a & b);
                }
                break;
            case O_OR:
                {
                    WORD a = POP;
                    WORD b = POP;
                    PUSH(a | b);
                }
                break;
            case O_XOR:
                {
                    WORD a = POP;
                    WORD b = POP;
                    PUSH(a ^ b);
                }
                break;
            case O_LSHIFT:
                {
                    WORD shift = POP;
                    WORD value = POP;
                    PUSH(shift < (WORD)WORD_BIT ? value << shift : 0);
                }
                break;
            case O_RSHIFT:
                {
                    WORD shift = POP;
                    WORD value = POP;
                    PUSH(shift < (WORD)WORD_BIT ? (WORD)((UWORD)value >> shift) : 0);
                }
                break;
            case O_ARSHIFT:
                {
                    WORD shift = POP;
                    WORD value = POP;
                    WORD result = ARSHIFT(value, shift);
                    PUSH(result);
                }
                break;
            case O_POP:
                (void)POP;
                break;
            case O_DUP:
                {
                    UWORD depth = POP;
                    if (depth >= SP)
                        error = ERROR_STACK_UNDERFLOW;
                    else
                        PUSH(S0[SP - (depth + 1)]);
                }
                break;
            case O_SET:
                {
                    UWORD depth = POP;
                    UWORD value = POP;
                    if (depth >= SP)
                        error = ERROR_STACK_UNDERFLOW;
                    else
                        S0[SP - (depth + 1)] = value;
                }
                break;
            case O_SWAP:
                {
                    UWORD depth = POP;
                    if (SP == 0 || depth >= SP - 1)
                        error = ERROR_STACK_UNDERFLOW;
                    else {
                        temp = S0[SP - (depth + 2)];
                        S0[SP - (depth + 2)] = S0[SP - 1];
                        S0[SP - 1] = temp;
                    }
                }
                break;
            case O_JUMP:
                {
                    WORD addr = POP;
                    CHECK_VALID_WORD(addr);
                    PC = addr;
                }
                break;
            case O_JUMPZ:
                {
                    WORD addr = POP;
                    if (POP == 0) {
                        CHECK_VALID_WORD(addr);
                        PC = addr;
                    }
                }
                break;
            case O_CALL:
                {
                    WORD addr = POP;
                    CHECK_VALID_WORD(addr);
                    PUSH_RETURN(PC);
                    PC = addr;
                }
                break;
            case O_RET:
                {
                    WORD addr = POP_RETURN;
                    if ((addr & 1) == 1) {
                        HANDLER_RP = POP_RETURN;
                        addr = addr & ~1;
                    }
                    CHECK_VALID_WORD(addr);
                    PC = addr;
                }
                break;
            case O_LOAD:
                {
                    WORD addr = POP;
                    WORD value = LOAD_WORD(addr);
                    PUSH(value);
                }
                break;
            case O_STORE:
                {
                    WORD addr = POP;
                    WORD value = POP;
                    STORE_WORD(addr, value);
                }
                break;
            case O_LOAD1:
                {
                    WORD addr = POP;
                    uint8_t value = LOAD_BYTE(addr);
                    PUSH((WORD)value);
                }
                break;
            case O_STORE1:
                {
                    WORD addr = POP;
                    uint8_t value = (uint8_t)POP;
                    STORE_BYTE(addr, value);
                }
                break;
            case O_LOAD2:
                {
                    WORD addr = POP;
                    if (addr % 2 != 0)
                        error = ERROR_UNALIGNED_ADDRESS;
                    else {
                        uint8_t byte1 = LOAD_BYTE(addr);
                        uint8_t byte2 = LOAD_BYTE(addr + 1);
                        PUSH((WORD)(uint16_t)(byte1 | (byte2 << 8)));
                    }
                }
                break;
            case O_STORE2:
                {
                    WORD addr = POP;
                    if (addr % 2 != 0)
                        error = ERROR_UNALIGNED_ADDRESS;
                    else {
                        uint16_t value = (uint16_t)POP;
                        STORE_BYTE(addr, (uint8_t)value);
                        STORE_BYTE(addr + 1, (uint8_t)(value >> 8));
                    }
                }
                break;
            case O_LOAD4:
                {
                    WORD addr = POP;
                    WORD value = LOAD_WORD(addr);
                    PUSH(value);
                }
                break;
            case O_STORE4:
                {
                    WORD addr = POP;
                    WORD value = POP;
                    STORE_WORD(addr, value);
                }
                break;
            case O_NEGATE:
                {
                    WORD a = POP;
                    PUSH(-a);
                }
                break;
            case O_ADD:
                {
                    WORD a = POP;
                    WORD b = POP;
                    PUSH(b + a);
                }
                break;
            case O_MUL:
                {
                    WORD multiplier = POP;
                    WORD multiplicand = POP;
                    PUSH(multiplier * multiplicand);
                }
                break;
            case O_DIVMOD:
                {
                    WORD divisor = POP;
                    WORD dividend = POP;
                    PUSH(MOD_WITH_OVERFLOW(dividend, divisor));
                    PUSH(DIV_WITH_OVERFLOW(dividend, divisor));
                }
                break;
            case O_UDIVMOD:
                {
                    UWORD divisor = POP;
                    UWORD dividend = POP;
                    PUSH(MOD_CATCH_ZERO(dividend, divisor));
                    PUSH(DIV_CATCH_ZERO(dividend, divisor));
                }
                break;
            case O_EQ:
                {
                    WORD a = POP;
                    WORD b = POP;
                    PUSH(a == b);
                }
                break;
            case O_LT:
                {
                    WORD a = POP;
                    WORD b = POP;
                    PUSH(b < a);
                }
                break;
            case O_ULT:
                {
                    UWORD a = POP;
                    UWORD b = POP;
                    PUSH(b < a);
                }
                break;
            case O_PUSHR:
                {
                    WORD value = POP;
                    PUSH_RETURN(value);
                }
                break;
            case O_POPR:
                {
                    WORD value = POP_RETURN;
                    if (error == ERROR_OK)
                        PUSH(value);
                }
                break;
            case O_DUPR:
                if (RP == 0)
                    error = ERROR_STACK_UNDERFLOW;
                else {
                    WORD value = *stack_position(R0, RP, 0);
                    PUSH(value);
                }
                break;
            case O_CATCH:
                {
                    UWORD addr = POP;
                    CHECK_VALID_WORD(addr);
                    PUSH_RETURN(HANDLER_RP);
                    PUSH_RETURN(PC | 1);
                    HANDLER_RP = RP;
                    PC = addr;
                }
                break;
            case O_THROW:
                {
                    if (HANDLER_RP == (UWORD)-1)
                        return POP;
                    RP = HANDLER_RP;
                    UWORD addr = POP_RETURN;
                    HANDLER_RP = POP_RETURN;
                    PC = addr & ~1;
                }
                break;
            case O_BREAK:
                return ERROR_BREAK;
                break;

            case O_GET_SP:
                {
                    WORD value = SP;
                    PUSH(value);
                }
                break;
            case O_SET_SP:
                {
                    WORD value = POP;
                    SP = value;
                }
                break;
            case O_GET_RP:
                PUSH(RP);
                break;
            case O_SET_RP:
                RP = POP;
                break;
            case O_GET_MEMORY:
                PUSH(MEMORY);
                break;
            case O_WORD_BYTES:
                PUSH(WORD_BYTES);
                break;

            default:
                if (opcode >= OX_ARGC && opcode <= OX_FILE_STATUS)
                    error = extra_instruction(opcode);
                else
                    error = ERROR_INVALID_OPCODE;
                break;
            }
            break;
        }
    } while (error == 0);

 error:
    return error;
}
