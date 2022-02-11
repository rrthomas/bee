// The interpreter storage allocation and main loop.
//
// (c) Reuben Thomas 1994-2022
//
// The package is distributed under the GNU General Public License version 3,
// or, at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#include "config.h"

#include <stdlib.h>
#include <string.h>

#include "bee/bee.h"
#include "bee/opcodes.h"

#include "private.h"


// VM registers

#define R(reg, type) \
    type bee_##reg;
#include "bee/registers.h"
#undef R


// Stacks

inline int bee_pop_stack(bee_word_t *s0, bee_uword_t ssize, bee_uword_t *sp, bee_word_t *val_ptr)
{
    if (unlikely(*sp == 0))
        return BEE_ERROR_STACK_UNDERFLOW;
    else if (unlikely(*sp > ssize))
        return BEE_ERROR_STACK_OVERFLOW;
    (*sp)--;
    *val_ptr = s0[*sp];
    return BEE_ERROR_OK;
}

inline int bee_push_stack(bee_word_t *s0, bee_uword_t ssize, bee_uword_t *sp, bee_word_t val)
{
    if (unlikely(*sp >= ssize))
        return BEE_ERROR_STACK_OVERFLOW;
    s0[*sp] = val;
    (*sp)++;
    return BEE_ERROR_OK;
}


// Initialise VM state.
int bee_init(bee_word_t *buf, bee_uword_t memory_size, bee_uword_t stack_size, bee_uword_t return_stack_size)
{
    if (buf == NULL)
        return -1;
    bee_m0 = buf;
    bee_msize = memory_size * BEE_WORD_BYTES;
    memset(bee_m0, 0, bee_msize);

    bee_pc = bee_m0;
    bee_dp = 0;

    bee_dsize = stack_size;
    bee_d0 = (bee_word_t *)calloc(bee_dsize, BEE_WORD_BYTES);
    if (bee_d0 == NULL)
        return -1;
    bee_handler_sp = bee_sp = 0;

    bee_ssize = return_stack_size;
    bee_s0 = (bee_word_t *)calloc(bee_ssize, BEE_WORD_BYTES);
    if (bee_s0 == NULL) {
        free(bee_d0);
        return -1;
    }

    return 0;
}

int bee_init_defaults(bee_word_t *buf, bee_uword_t memory_size)
{
    return bee_init(buf, memory_size, BEE_DEFAULT_STACK_SIZE, BEE_DEFAULT_STACK_SIZE);
}


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
    bee_word_t error = BEE_ERROR_OK;

    for (;;) {
        bee_word_t ir = *bee_pc++;

        switch (ir & BEE_OP1_MASK) {
        case BEE_OP_CALLI:
            {
                PUSHS((bee_uword_t)bee_pc);
                bee_word_t *addr = (bee_pc - 1) + ARSHIFT(ir, BEE_OP1_SHIFT);
                CHECK_ALIGNED(addr);
                bee_pc = addr;
            }
            break;
        case BEE_OP_PUSHI:
            PUSHD(ARSHIFT(ir, BEE_OP1_SHIFT));
            break;
        case BEE_OP_PUSHRELI:
            PUSHD((bee_uword_t)((bee_pc - 1) + ARSHIFT(ir, BEE_OP1_SHIFT)));
            break;
        default:
            switch (ir & BEE_OP2_MASK) {
            case BEE_OP_JUMPI:
                {
                    bee_word_t *addr = (bee_pc - 1) + ARSHIFT(ir, BEE_OP2_SHIFT);
                    CHECK_ALIGNED(addr);
                    bee_pc = addr;
                }
                break;
            case BEE_OP_JUMPZI:
                {
                    bee_word_t *addr = (bee_pc - 1) + ARSHIFT(ir, BEE_OP2_SHIFT);
                    bee_word_t flag;
                    POPD(&flag);
                    if (flag == 0) {
                        CHECK_ALIGNED(addr);
                        bee_pc = addr;
                    }
                }
                break;
            case BEE_OP_TRAP:
                THROW_IF_ERROR(trap((bee_uword_t)ir >> BEE_OP2_SHIFT));
                break;
            case BEE_OP_INSN:
                {
                    bee_uword_t opcode = (bee_uword_t)ir >> BEE_OP2_SHIFT;
                    switch (opcode) {
                    case BEE_INSN_NOP:
                        break;
                    case BEE_INSN_NOT:
                        {
                            bee_word_t a;
                            POPD(&a);
                            PUSHD(~a);
                        }
                        break;
                    case BEE_INSN_AND:
                        {
                            bee_word_t a, b;
                            POPD(&a);
                            POPD(&b);
                            PUSHD(a & b);
                        }
                        break;
                    case BEE_INSN_OR:
                        {
                            bee_word_t a, b;
                            POPD(&a);
                            POPD(&b);
                            PUSHD(a | b);
                        }
                        break;
                    case BEE_INSN_XOR:
                        {
                            bee_word_t a, b;
                            POPD(&a);
                            POPD(&b);
                            PUSHD(a ^ b);
                        }
                        break;
                    case BEE_INSN_LSHIFT:
                        {
                            bee_word_t shift, value;
                            POPD(&shift);
                            POPD(&value);
                            PUSHD(shift < (bee_word_t)BEE_WORD_BIT ? LSHIFT(value, shift) : 0);
                        }
                        break;
                    case BEE_INSN_RSHIFT:
                        {
                            bee_word_t shift, value;
                            POPD(&shift);
                            POPD(&value);
                            PUSHD(shift < (bee_word_t)BEE_WORD_BIT ? (bee_word_t)((bee_uword_t)value >> shift) : 0);
                        }
                        break;
                    case BEE_INSN_ARSHIFT:
                        {
                            bee_word_t shift, value;
                            POPD(&shift);
                            POPD(&value);
                            PUSHD(ARSHIFT(value, shift));
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
                            POPD((bee_word_t *)&depth);
                            if (depth >= bee_dp)
                                THROW(BEE_ERROR_STACK_UNDERFLOW);
                            else
                                PUSHD(bee_d0[bee_dp - (depth + 1)]);
                        }
                        break;
                    case BEE_INSN_SET:
                        {
                            bee_uword_t depth;
                            POPD((bee_word_t *)&depth);
                            bee_word_t value;
                            POPD(&value);
                            if (depth >= bee_dp)
                                THROW(BEE_ERROR_STACK_UNDERFLOW);
                            else
                                bee_d0[bee_dp - (depth + 1)] = value;
                        }
                        break;
                    case BEE_INSN_SWAP:
                        {
                            bee_uword_t depth;
                            POPD((bee_word_t *)&depth);
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
                            POPD((bee_word_t *)&addr);
                            CHECK_ALIGNED(addr);
                            bee_pc = addr;
                        }
                        break;
                    case BEE_INSN_JUMPZ:
                        {
                            bee_word_t *addr;
                            POPD((bee_word_t *)&addr);
                            bee_word_t flag;
                            POPD(&flag);
                            if (flag == 0) {
                                CHECK_ALIGNED(addr);
                                bee_pc = addr;
                            }
                        }
                        break;
                    case BEE_INSN_CALL:
                        {
                            bee_word_t *addr;
                            POPD((bee_word_t *)&addr);
                            CHECK_ALIGNED(addr);
                            PUSHS((bee_uword_t)bee_pc);
                            bee_pc = addr;
                        }
                        break;
                    case BEE_INSN_RET:
                        {
                            bee_word_t *addr;
                            POPS((bee_word_t *)&addr);
                            CHECK_ALIGNED(addr);
                            if (bee_sp < bee_handler_sp) {
                                POPS((bee_word_t *)&bee_handler_sp);
                                PUSHD(0);
                            }
                            bee_pc = addr;
                        }
                        break;
                    case BEE_INSN_LOAD:
                        {
                            bee_word_t *addr;
                            POPD((bee_word_t *)&addr);
                            CHECK_ALIGNED(addr);
                            PUSHD(*addr);
                        }
                        break;
                    case BEE_INSN_STORE:
                        {
                            bee_word_t *addr;
                            POPD((bee_word_t *)&addr);
                            CHECK_ALIGNED(addr);
                            bee_word_t value;
                            POPD(&value);
                            *addr = value;
                        }
                        break;
                    case BEE_INSN_LOAD1:
                        {
                            uint8_t *addr;
                            POPD((bee_word_t *)&addr);
                            uint8_t value = *addr;
                            PUSHD((bee_word_t)value);
                        }
                        break;
                    case BEE_INSN_STORE1:
                        {
                            uint8_t *addr;
                            POPD((bee_word_t *)&addr);
                            bee_word_t value;
                            POPD(&value);
                            *addr = (uint8_t)value;
                        }
                        break;
                    case BEE_INSN_LOAD2:
                        {
                            uint16_t *addr;
                            POPD((bee_word_t *)&addr);
                            if ((bee_uword_t)addr % 2 != 0)
                                THROW(BEE_ERROR_UNALIGNED_ADDRESS);
                            PUSHD(*addr);
                        }
                        break;
                    case BEE_INSN_STORE2:
                        {
                            uint16_t *addr;
                            POPD((bee_word_t *)&addr);
                            if ((bee_uword_t)addr % 2 != 0)
                                THROW(BEE_ERROR_UNALIGNED_ADDRESS);
                            bee_word_t value;
                            POPD(&value);
                            *addr = (uint16_t)value;
                        }
                        break;
                    case BEE_INSN_LOAD4:
                        {
                            uint32_t *addr;
                            POPD((bee_word_t *)&addr);
                            if ((bee_uword_t)addr % 4 != 0)
                                THROW(BEE_ERROR_UNALIGNED_ADDRESS);
                            PUSHD(*addr);
                        }
                        break;
                    case BEE_INSN_STORE4:
                        {
                            uint32_t *addr;
                            POPD((bee_word_t *)&addr);
                            if ((bee_uword_t)addr % 4 != 0)
                                THROW(BEE_ERROR_UNALIGNED_ADDRESS);
                            bee_word_t value;
                            POPD(&value);
                            *addr = (uint32_t)value;
                        }
                        break;
                    case BEE_INSN_NEG:
                        {
                            bee_uword_t a;
                            POPD((bee_word_t *)&a);
                            PUSHD((bee_word_t)-a);
                        }
                        break;
                    case BEE_INSN_ADD:
                        {
                            bee_uword_t a, b;
                            POPD((bee_word_t *)&a);
                            POPD((bee_word_t *)&b);
                            PUSHD((bee_word_t)(b + a));
                        }
                        break;
                    case BEE_INSN_MUL:
                        {
                            bee_uword_t a, b;
                            POPD((bee_word_t *)&a);
                            POPD((bee_word_t *)&b);
                            PUSHD((bee_word_t)(a * b));
                        }
                        break;
                    case BEE_INSN_DIVMOD:
                        {
                            bee_word_t divisor, dividend;
                            POPD(&divisor);
                            POPD(&dividend);
                            if (dividend == BEE_WORD_MIN && divisor == -1) {
                                PUSHD(BEE_WORD_MIN);
                                PUSHD(0);
                            } else {
                                PUSHD(DIV_CATCH_ZERO(dividend, divisor));
                                PUSHD(MOD_CATCH_ZERO(dividend, divisor));
                            }
                        }
                        break;
                    case BEE_INSN_UDIVMOD:
                        {
                            bee_uword_t divisor, dividend;
                            POPD((bee_word_t *)&divisor);
                            POPD((bee_word_t *)&dividend);
                            PUSHD(DIV_CATCH_ZERO(dividend, divisor));
                            PUSHD(MOD_CATCH_ZERO(dividend, divisor));
                        }
                        break;
                    case BEE_INSN_EQ:
                        {
                            bee_word_t a, b;
                            POPD(&a);
                            POPD(&b);
                            PUSHD(a == b);
                        }
                        break;
                    case BEE_INSN_LT:
                        {
                            bee_word_t a, b;
                            POPD(&a);
                            POPD(&b);
                            PUSHD(b < a);
                        }
                        break;
                    case BEE_INSN_ULT:
                        {
                            bee_uword_t a, b;
                            POPD((bee_word_t *)&a);
                            POPD((bee_word_t *)&b);
                            PUSHD(b < a);
                        }
                        break;
                    case BEE_INSN_PUSHS:
                        {
                            bee_word_t value;
                            POPD(&value);
                            PUSHS(value);
                        }
                        break;
                    case BEE_INSN_POPS:
                        {
                            bee_word_t value;
                            POPS(&value);
                            if (error == BEE_ERROR_OK)
                                PUSHD(value);
                        }
                        break;
                    case BEE_INSN_DUPS:
                        if (bee_sp == 0)
                            THROW(BEE_ERROR_STACK_UNDERFLOW);
                        else {
                            bee_word_t value = bee_s0[bee_sp - 1];
                            PUSHD(value);
                        }
                        break;
                    case BEE_INSN_CATCH:
                        {
                            bee_word_t *addr;
                            POPD((bee_word_t *)&addr);
                            CHECK_ALIGNED(addr);
                            PUSHS(bee_handler_sp);
                            PUSHS((bee_uword_t)bee_pc);
                            bee_handler_sp = bee_sp;
                            bee_pc = addr;
                        }
                        break;
                    case BEE_INSN_THROW:
                        {
                            if (bee_dp < 1)
                                error = BEE_ERROR_STACK_UNDERFLOW;
                            else
                                POPD(&error);
                        error:
                            if (bee_handler_sp == 0)
                                return error;
                            // Don't push error code if the stack is full.
                            if (bee_dp < bee_dsize)
                                bee_d0[bee_dp++] = error;
                            bee_sp = bee_handler_sp;
                            bee_word_t *addr;
                            POPS((bee_word_t *)&addr);
                            POPS((bee_word_t *)&bee_handler_sp);
                            bee_pc = addr;
                        }
                        break;
                    case BEE_INSN_BREAK:
                        bee_pc--;
                        return BEE_ERROR_BREAK;
                    case BEE_INSN_WORD_BYTES:
                        PUSHD(BEE_WORD_BYTES);
                        break;
                    case BEE_INSN_GET_M0:
                        PUSHD((bee_uword_t)bee_m0);
                        break;
                    case BEE_INSN_GET_MSIZE:
                        PUSHD(bee_msize);
                        break;
                    case BEE_INSN_GET_SSIZE:
                        PUSHD(bee_ssize);
                        break;
                    case BEE_INSN_GET_SP:
                        PUSHD(bee_sp);
                        break;
                    case BEE_INSN_SET_SP:
                        POPD((bee_word_t *)&bee_sp);
                        break;
                    case BEE_INSN_GET_DSIZE:
                        PUSHD(bee_dsize);
                        break;
                    case BEE_INSN_GET_DP:
                        {
                            bee_word_t value = bee_dp;
                            PUSHD(value);
                        }
                        break;
                    case BEE_INSN_SET_DP:
                        {
                            bee_word_t value;
                            POPD(&value);
                            bee_dp = value;
                        }
                        break;
                    case BEE_INSN_GET_HANDLER_SP:
                        PUSHD(bee_handler_sp);
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
