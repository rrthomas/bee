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
#include <stdbool.h>
#include <string.h>
#include <limits.h>

#include "verify.h"

#include "bee/bee.h"
#include "bee/opcodes.h"

#ifdef HAVE_MIJIT
#include "../mijit-bee/mijit-bee.h"
#endif

#include "private.h"


// Optimization
// Hint that `x` is usually true/false.
// https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html
#if HAVE___BUILTIN_EXPECT == 1
#define unlikely(x) __builtin_expect(!!(x), 0)
#else
#define unlikely(x) (x)
#endif

// Basic assumption: a byte is 8 bits
verify(CHAR_BIT == 8);

// JIT compiler

#ifdef HAVE_MIJIT
mijit_bee_jit *bee_jit;
#endif


// Stacks

inline int bee_check_stack(bee_uword_t ssize, bee_uword_t sp, bee_uword_t pops, bee_uword_t pushes)
{
    if (unlikely(pops > sp))
        return BEE_ERROR_STACK_UNDERFLOW;
    if (unlikely(sp > ssize || ssize - (sp - pops) < pushes))
        return BEE_ERROR_STACK_OVERFLOW;
    return BEE_ERROR_OK;
}

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
bee_state *bee_init(bee_word_t *pc, bee_uword_t ssize, bee_uword_t dsize)
{
    bee_state * restrict S = (bee_state *)calloc(1, sizeof(bee_state));
    if (S == NULL)
        return NULL;

    S->pc = pc;
    S->dsize = dsize;
    S->d0 = (bee_word_t *)calloc(S->dsize, BEE_WORD_BYTES);
    if (S->d0 != NULL) {
        S->ssize = ssize;
        S->s0 = (bee_word_t *)calloc(S->ssize, BEE_WORD_BYTES);
        if (S->s0 != NULL) {
#ifdef HAVE_MIJIT
            bee_jit = mijit_bee_new();
            if (bee_jit != NULL)
                return S;
#else
            return S;
#endif
        }
        free(S->s0);
    }

    free(S->d0);
    return NULL;
}

void bee_destroy(bee_state * restrict S)
{
#ifdef HAVE_MIJIT
    mijit_bee_drop(bee_jit);
#endif
    free(S->s0);
    free(S->d0);
    free(S);
}


// Address checking
#define CHECK_ALIGNED(a)                                        \
    if (!IS_ALIGNED(a))                                         \
        THROW(BEE_ERROR_UNALIGNED_ADDRESS);

// Division macros
#define DIV_CATCH_ZERO(a, b) ((b) == 0 ? 0 : (a) / (b))
#define MOD_CATCH_ZERO(a, b) ((b) == 0 ? (a) : (a) % (b))


// Execution function
bee_word_t bee_run(bee_state * restrict S)
{
    bee_word_t error = BEE_ERROR_OK;
    CHECK_ALIGNED(S->pc);

    for (;;) {
#ifdef HAVE_MIJIT
        mijit_bee_run(bee_jit, (mijit_bee_registers *)S);
#endif

        S->ir = *S->pc++;

        switch (S->ir & BEE_OP1_MASK) {
        case BEE_OP_CALLI:
            {
                CHECKS(0, 1);
                PUSHS((bee_uword_t)S->pc);
                bee_word_t *addr = (S->pc - 1) + ARSHIFT(S->ir, BEE_OP1_SHIFT);
                CHECK_ALIGNED(addr);
                S->pc = addr;
            }
            break;
        case BEE_OP_PUSHI:
            CHECKD(0, 1);
            PUSHD(ARSHIFT(S->ir, BEE_OP1_SHIFT));
            break;
        case BEE_OP_PUSHRELI:
            CHECKD(0, 1);
            PUSHD((bee_uword_t)((S->pc - 1) + ARSHIFT(S->ir, BEE_OP1_SHIFT)));
            break;
        default:
            switch (S->ir & BEE_OP2_MASK) {
            case BEE_OP_JUMPI:
                {
                    bee_word_t *addr = (S->pc - 1) + ARSHIFT(S->ir, BEE_OP2_SHIFT);
                    CHECK_ALIGNED(addr);
                    S->pc = addr;
                }
                break;
            case BEE_OP_JUMPZI:
                {
                    CHECKD(1, 0);
                    bee_word_t *addr = (S->pc - 1) + ARSHIFT(S->ir, BEE_OP2_SHIFT);
                    bee_word_t flag;
                    POPD(&flag);
                    if (flag == 0) {
                        CHECK_ALIGNED(addr);
                        S->pc = addr;
                    }
                }
                break;
            case BEE_OP_TRAP:
                THROW_IF_ERROR(trap(S, (bee_uword_t)(S->ir) >> BEE_OP2_SHIFT));
                CHECK_ALIGNED(S->pc);
                if (S->sp > S->ssize || S->dp > S->dsize)
                    THROW(BEE_ERROR_STACK_OVERFLOW);
                break;
            case BEE_OP_INSN:
                {
                    S->ir = (bee_word_t)((bee_uword_t)(S->ir) >> BEE_OP2_SHIFT);
                    do {
                        bee_uword_t opcode = S->ir & BEE_INSN_MASK;
                        S->ir = (bee_word_t)((bee_uword_t)(S->ir) >> BEE_INSN_BITS);
                        switch (opcode) {
                        case BEE_INSN_NOP:
                            goto end;
                            break;
                        case BEE_INSN_NOT:
                            {
                                CHECKD(1, 1);
                                bee_word_t a;
                                POPD(&a);
                                PUSHD(~a);
                            }
                            break;
                        case BEE_INSN_AND:
                            {
                                CHECKD(2, 1);
                                bee_word_t a, b;
                                POPD(&a);
                                POPD(&b);
                                PUSHD(a & b);
                            }
                            break;
                        case BEE_INSN_OR:
                            {
                                CHECKD(2, 1);
                                bee_word_t a, b;
                                POPD(&a);
                                POPD(&b);
                                PUSHD(a | b);
                            }
                            break;
                        case BEE_INSN_XOR:
                            {
                                CHECKD(2, 1);
                                bee_word_t a, b;
                                POPD(&a);
                                POPD(&b);
                                PUSHD(a ^ b);
                            }
                            break;
                        case BEE_INSN_LSHIFT:
                            {
                                CHECKD(2, 1);
                                bee_word_t shift, value;
                                POPD(&shift);
                                POPD(&value);
                                PUSHD(shift < (bee_word_t)BEE_WORD_BIT ? LSHIFT(value, shift) : 0);
                            }
                            break;
                        case BEE_INSN_RSHIFT:
                            {
                                CHECKD(2, 1);
                                bee_word_t shift, value;
                                POPD(&shift);
                                POPD(&value);
                                PUSHD(shift < (bee_word_t)BEE_WORD_BIT ? (bee_word_t)((bee_uword_t)value >> shift) : 0);
                            }
                            break;
                        case BEE_INSN_ARSHIFT:
                            {
                                CHECKD(2, 1);
                                bee_word_t shift, value;
                                POPD(&shift);
                                POPD(&value);
                                PUSHD(ARSHIFT(value, shift));
                            }
                            break;
                        case BEE_INSN_POP:
                            CHECKD(1, 0);
                            S->dp--;
                            break;
                        case BEE_INSN_DUP:
                            {
                                CHECKD(1, 1);
                                bee_uword_t depth;
                                POPD((bee_word_t *)&depth);
                                if (depth >= S->dp) {
                                    PUSHD(depth);
                                    THROW(BEE_ERROR_STACK_UNDERFLOW);
                                } else
                                    PUSHD(S->d0[S->dp - (depth + 1)]);
                            }
                            break;
                        case BEE_INSN_SET:
                            {
                                CHECKD(2, 1);
                                bee_uword_t depth;
                                POPD((bee_word_t *)&depth);
                                if (depth >= S->dp) {
                                    PUSHD(depth);
                                    THROW(BEE_ERROR_STACK_UNDERFLOW);
                                } else {
                                    bee_word_t value;
                                    POPD(&value);
                                    S->d0[S->dp - (depth + 1)] = value;
                                }
                            }
                            break;
                        case BEE_INSN_SWAP:
                            {
                                CHECKD(1, 0);
                                bee_uword_t depth;
                                POPD((bee_word_t *)&depth);
                                if (S->dp == 0 || depth >= S->dp - 1) {
                                    PUSHD(depth);
                                    THROW(BEE_ERROR_STACK_UNDERFLOW);
                                } else {
                                    bee_word_t temp = S->d0[S->dp - (depth + 2)];
                                    S->d0[S->dp - (depth + 2)] = S->d0[S->dp - 1];
                                    S->d0[S->dp - 1] = temp;
                                }
                            }
                            break;
                        case BEE_INSN_JUMP:
                            {
                                CHECKD(1, 0);
                                bee_word_t *addr;
                                POPD((bee_word_t *)&addr);
                                CHECK_ALIGNED(addr);
                                S->pc = addr;
                            }
                            break;
                        case BEE_INSN_JUMPZ:
                            {
                                CHECKD(2, 0);
                                bee_word_t *addr;
                                POPD((bee_word_t *)&addr);
                                bee_word_t flag;
                                POPD(&flag);
                                if (flag == 0) {
                                    CHECK_ALIGNED(addr);
                                    S->pc = addr;
                                }
                            }
                            break;
                        case BEE_INSN_CALL:
                            {
                                CHECKD(1, 0);
                                CHECKS(0, 1);
                                bee_word_t *addr;
                                POPD((bee_word_t *)&addr);
                                CHECK_ALIGNED(addr);
                                PUSHS((bee_uword_t)S->pc);
                                S->pc = addr;
                            }
                            break;
                        case BEE_INSN_RET:
                            {
                                CHECKS(1, 0);
                                if (S->sp < S->handler_sp) {
                                    CHECKS(1, 0);
                                    CHECKD(0, 1);
                                }
                                bee_word_t *addr;
                                POPS((bee_word_t *)&addr);
                                CHECK_ALIGNED(addr);
                                if (S->sp < S->handler_sp) {
                                    POPS((bee_word_t *)&S->handler_sp);
                                    PUSHD(0);
                                }
                                S->pc = addr;
                            }
                            break;
                        case BEE_INSN_LOAD:
                            {
                                CHECKD(1, 1);
                                bee_word_t *addr;
                                POPD((bee_word_t *)&addr);
                                CHECK_ALIGNED(addr);
                                PUSHD(*addr);
                            }
                            break;
                        case BEE_INSN_STORE:
                            {
                                CHECKD(2, 0);
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
                                CHECKD(1, 1);
                                uint8_t *addr;
                                POPD((bee_word_t *)&addr);
                                uint8_t value = *addr;
                                PUSHD((bee_word_t)value);
                            }
                            break;
                        case BEE_INSN_STORE1:
                            {
                                CHECKD(2, 0);
                                uint8_t *addr;
                                POPD((bee_word_t *)&addr);
                                bee_word_t value;
                                POPD(&value);
                                *addr = (uint8_t)value;
                            }
                            break;
                        case BEE_INSN_LOAD2:
                            {
                                CHECKD(1, 1);
                                uint16_t *addr;
                                POPD((bee_word_t *)&addr);
                                if ((bee_uword_t)addr % 2 != 0)
                                    THROW(BEE_ERROR_UNALIGNED_ADDRESS);
                                PUSHD(*addr);
                            }
                            break;
                        case BEE_INSN_STORE2:
                            {
                                CHECKD(2, 0);
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
                                CHECKD(1, 1);
                                uint32_t *addr;
                                POPD((bee_word_t *)&addr);
                                if ((bee_uword_t)addr % 4 != 0)
                                    THROW(BEE_ERROR_UNALIGNED_ADDRESS);
                                PUSHD(*addr);
                            }
                            break;
                        case BEE_INSN_STORE4:
                            {
                                CHECKD(2, 0);
                                uint32_t *addr;
                                POPD((bee_word_t *)&addr);
                                if ((bee_uword_t)addr % 4 != 0)
                                    THROW(BEE_ERROR_UNALIGNED_ADDRESS);
                                bee_word_t value;
                                POPD(&value);
                                *addr = (uint32_t)value;
                            }
                            break;
                        case BEE_INSN_LOAD_IA:
                            {
                                CHECKD(1, 2);
                                bee_word_t *addr;
                                POPD((bee_word_t *)&addr);
                                CHECK_ALIGNED(addr);
                                PUSHD(*addr);
                                PUSHD((bee_word_t)(addr + 1));
                            }
                            break;
                        case BEE_INSN_STORE_DB:
                            {
                                CHECKD(2, 1);
                                bee_word_t *addr;
                                POPD((bee_word_t *)&addr);
                                CHECK_ALIGNED(addr);
                                bee_word_t value;
                                POPD(&value);
                                addr[-1] = value;
                                PUSHD((bee_word_t)(addr - 1));
                            }
                            break;
                        case BEE_INSN_LOAD_IB:
                            {
                                CHECKD(1, 2);
                                bee_word_t *addr;
                                POPD((bee_word_t *)&addr);
                                CHECK_ALIGNED(addr);
                                PUSHD(addr[1]);
                                PUSHD((bee_word_t)(addr + 1));
                            }
                            break;
                        case BEE_INSN_STORE_DA:
                            {
                                CHECKD(2, 1);
                                bee_word_t *addr;
                                POPD((bee_word_t *)&addr);
                                CHECK_ALIGNED(addr);
                                bee_word_t value;
                                POPD(&value);
                                *addr = value;
                                PUSHD((bee_word_t)(addr - 1));
                            }
                            break;
                        case BEE_INSN_LOAD_DA:
                            {
                                CHECKD(1, 2);
                                bee_word_t *addr;
                                POPD((bee_word_t *)&addr);
                                CHECK_ALIGNED(addr);
                                PUSHD(*addr);
                                PUSHD((bee_word_t)(addr - 1));
                            }
                            break;
                        case BEE_INSN_STORE_IB:
                            {
                                CHECKD(2, 1);
                                bee_word_t *addr;
                                POPD((bee_word_t *)&addr);
                                CHECK_ALIGNED(addr);
                                bee_word_t value;
                                POPD(&value);
                                addr[1] = value;
                                PUSHD((bee_word_t)(addr + 1));
                            }
                            break;
                        case BEE_INSN_LOAD_DB:
                            {
                                CHECKD(1, 2);
                                bee_word_t *addr;
                                POPD((bee_word_t *)&addr);
                                CHECK_ALIGNED(addr);
                                PUSHD(addr[-1]);
                                PUSHD((bee_word_t)(addr - 1));
                            }
                            break;
                        case BEE_INSN_STORE_IA:
                            {
                                CHECKD(2, 1);
                                bee_word_t *addr;
                                POPD((bee_word_t *)&addr);
                                CHECK_ALIGNED(addr);
                                bee_word_t value;
                                POPD(&value);
                                *addr = value;
                                PUSHD((bee_word_t)(addr + 1));
                            }
                            break;
                        case BEE_INSN_NEG:
                            {
                                CHECKD(1, 1);
                                bee_uword_t a;
                                POPD((bee_word_t *)&a);
                                PUSHD((bee_word_t)-a);
                            }
                            break;
                        case BEE_INSN_ADD:
                            {
                                CHECKD(2, 1);
                                bee_uword_t a, b;
                                POPD((bee_word_t *)&a);
                                POPD((bee_word_t *)&b);
                                PUSHD((bee_word_t)(b + a));
                            }
                            break;
                        case BEE_INSN_MUL:
                            {
                                CHECKD(2, 1);
                                bee_uword_t a, b;
                                POPD((bee_word_t *)&a);
                                POPD((bee_word_t *)&b);
                                PUSHD((bee_word_t)(a * b));
                            }
                            break;
                        case BEE_INSN_DIVMOD:
                            {
                                CHECKD(2, 2);
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
                                CHECKD(2, 2);
                                bee_uword_t divisor, dividend;
                                POPD((bee_word_t *)&divisor);
                                POPD((bee_word_t *)&dividend);
                                PUSHD(DIV_CATCH_ZERO(dividend, divisor));
                                PUSHD(MOD_CATCH_ZERO(dividend, divisor));
                            }
                            break;
                        case BEE_INSN_EQ:
                            {
                                CHECKD(2, 1);
                                bee_word_t a, b;
                                POPD(&a);
                                POPD(&b);
                                PUSHD(a == b);
                            }
                            break;
                        case BEE_INSN_LT:
                            {
                                CHECKD(2, 1);
                                bee_word_t a, b;
                                POPD(&a);
                                POPD(&b);
                                PUSHD(b < a);
                            }
                            break;
                        case BEE_INSN_ULT:
                            {
                                CHECKD(2, 1);
                                bee_uword_t a, b;
                                POPD((bee_word_t *)&a);
                                POPD((bee_word_t *)&b);
                                PUSHD(b < a);
                            }
                            break;
                        case BEE_INSN_PUSHS:
                            {
                                CHECKD(1, 0);
                                CHECKS(0, 1);
                                bee_word_t value;
                                POPD(&value);
                                PUSHS(value);
                            }
                            break;
                        case BEE_INSN_POPS:
                            {
                                CHECKS(1, 0);
                                CHECKD(0, 1);
                                bee_word_t value;
                                POPS(&value);
                                PUSHD(value);
                            }
                            break;
                        case BEE_INSN_DUPS:
                            CHECKS(1, 1);
                            CHECKD(0, 1);
                            PUSHD(S->s0[S->sp - 1]);
                            break;
                        case BEE_INSN_CATCH:
                            {
                                CHECKS(0, 2);
                                CHECKD(1, 0);
                                bee_word_t *addr;
                                POPD((bee_word_t *)&addr);
                                CHECK_ALIGNED(addr);
                                PUSHS(S->handler_sp);
                                PUSHS((bee_uword_t)S->pc);
                                S->handler_sp = S->sp;
                                S->pc = addr;
                            }
                            break;
                        case BEE_INSN_THROW:
                            {
                                CHECKD(1, 0);
                                POPD(&error);
                            error:
                                if (S->handler_sp < 2)
                                    return error;
                                // Don't push error code if the stack is full.
                                if (S->dp < S->dsize)
                                    S->d0[S->dp++] = error;
                                S->sp = S->handler_sp;
                                bee_word_t *addr;
                                POPS((bee_word_t *)&addr);
                                POPS((bee_word_t *)&S->handler_sp);
                                // If this check fails, we will pop the next handler.
                                CHECK_ALIGNED(addr);
                                S->pc = addr;
                            }
                            break;
                        case BEE_INSN_BREAK:
                            S->pc--;
                            return BEE_ERROR_BREAK;
                        case BEE_INSN_WORD_BYTES:
                            CHECKD(0, 1);
                            PUSHD(BEE_WORD_BYTES);
                            break;
                        case BEE_INSN_GET_SSIZE:
                            CHECKD(0, 1);
                            PUSHD(S->ssize);
                            break;
                        case BEE_INSN_GET_SP:
                            CHECKD(0, 1);
                            PUSHD(S->sp);
                            break;
                        case BEE_INSN_SET_SP:
                            CHECKD(1, 0);
                            POPD((bee_word_t *)&S->sp);
                            break;
                        case BEE_INSN_GET_DSIZE:
                            CHECKD(0, 1);
                            PUSHD(S->dsize);
                            break;
                        case BEE_INSN_GET_DP:
                            {
                                CHECKD(0, 1);
                                bee_word_t value = S->dp;
                                PUSHD(value);
                            }
                            break;
                        case BEE_INSN_SET_DP:
                            {
                                CHECKD(1, 0);
                                bee_word_t value;
                                POPD(&value);
                                S->dp = value;
                            }
                            break;
                        case BEE_INSN_GET_HANDLER_SP:
                            CHECKD(0, 1);
                            PUSHD(S->handler_sp);
                            break;
                        default:
                            THROW(BEE_ERROR_INVALID_OPCODE);
                            break;
                        }
                        continue;
                    end:
                        break;
                    } while (true);
                }
                break;
            }
        }
    }
}
