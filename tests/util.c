// Functions useful for VM debugging.
//
// (c) Reuben Thomas 1994-2022
//
// The package is distributed under the GNU General Public License version 3,
// or, at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#include "config.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xvasprintf.h"

#include "bee/bee.h"
#include "bee/opcodes.h"

#include "private.h"
#include "util.h"


static uint8_t *current; // where we assemble the next instruction word or literal


void word(bee_word_t value)
{
    current = (uint8_t *)ALIGN(current);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
    *(bee_word_t *)current = value;
#pragma GCC diagnostic pop
    current += BEE_WORD_BYTES;
}

void ass(bee_uword_t inst)
{
    word((inst << BEE_OP2_SHIFT) | BEE_OP_INSN);
}

void ass_trap(bee_uword_t code)
{
    word((code << BEE_OP2_SHIFT) | BEE_OP_TRAP);
}

void ass_byte(uint8_t b)
{
    *current++ = b;
}

void pushi(bee_word_t literal)
{
    bee_word_t temp = LSHIFT(literal, BEE_OP1_SHIFT);
    assert(ARSHIFT(temp, BEE_OP1_SHIFT) == literal);
    word(temp | BEE_OP_PUSHI);
}

void push(bee_word_t literal)
{
    assert(IS_ALIGNED(current));
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
    calli((bee_word_t *)current + 2);
#pragma GCC diagnostic pop
    word(literal);
    ass(BEE_INSN_POPS);
    ass(BEE_INSN_LOAD);
}

static void addr_op(int op, bee_word_t *addr)
{
    assert(IS_ALIGNED(current));
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
    word(LSHIFT(addr - (bee_word_t *)current, BEE_OP1_SHIFT) | op);
#pragma GCC diagnostic pop
}

void calli(bee_word_t *addr)
{
    addr_op(BEE_OP_CALLI, addr);
}

void pushreli(bee_word_t *addr)
{
    addr_op(BEE_OP_PUSHRELI, addr);
}

static void addr_op2(int op, bee_word_t *addr)
{
    assert(IS_ALIGNED(current));
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
    bee_word_t offset = addr - (bee_word_t *)current;
#pragma GCC diagnostic pop
    bee_word_t temp = LSHIFT(offset, BEE_OP2_SHIFT);
    assert(ARSHIFT(temp, BEE_OP2_SHIFT) == offset);
    word(temp | op);
}

void jumpi(bee_word_t *addr)
{
    addr_op2(BEE_OP_JUMPI, addr);
}

void jumpzi(bee_word_t *addr)
{
    addr_op2(BEE_OP_JUMPZI, addr);
}

void ass_goto(bee_word_t *addr)
{
    current = (uint8_t *)addr;
}

_GL_ATTRIBUTE_PURE bee_word_t *label(void)
{
    assert(IS_ALIGNED(current));
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
    return (bee_word_t *)current;
#pragma GCC diagnostic pop
}

static const char *mnemonic[BEE_INSN_UNDEFINED + 1] = {
// 0x00
    "NOP", "NOT", "AND", "OR", "XOR", "LSHIFT", "RSHIFT", "ARSHIFT",
    "POPD", "DUP", "SET", "SWAP", "JUMP", "JUMPZ", "CALL", "RET",
// 0x10
    "LOAD", "STORE", "LOAD1", "STORE1", "LOAD2", "STORE2", "LOAD4", "STORE4",
    "LOAD_IA", "STORE_DB", "LOAD_IB", "STORE_DA", "LOAD_DA", "STORE_IB", "LOAD_DB", "STORE_IA",
// 0x20
    "NEG", "ADD", "MUL", "DIVMOD", "UDIVMOD", "EQ", "LT", "ULT",
    "PUSHS", "POPS", "DUPS", "CATCH", "THROW", "BREAK", "BEE_WORD_BYTES", NULL,
// 0x30
    NULL, "GET_SSIZE", "GET_SP", "SET_SP", "GET_DSIZE", "GET_SP", "SET_SP", "GET_HANDLER_SP",
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
};

_GL_ATTRIBUTE_CONST const char *disass(bee_word_t opcode, bee_word_t *pc)
{
    static char *text = NULL;

    free(text);
    switch (opcode & BEE_OP1_MASK) {
    case BEE_OP_CALLI:
        {
            bee_word_t *addr = pc + ARSHIFT(opcode, BEE_OP1_SHIFT);
            text = xasprintf("CALLI $%zx", (bee_uword_t)addr);
        }
        break;
    case BEE_OP_PUSHI:
        {
            opcode = ARSHIFT(opcode, BEE_OP1_SHIFT);
            text = xasprintf("PUSHI %zd=$%zx", opcode, (bee_uword_t)opcode);
        }
        break;
    case BEE_OP_PUSHRELI:
        text = xasprintf("PUSHRELI $%zx", (bee_uword_t)(pc + (opcode & ~BEE_OP1_MASK)));
        break;
    default:
        switch (opcode & BEE_OP2_MASK) {
        case BEE_OP_JUMPI:
            {
                bee_word_t *addr = pc + ARSHIFT(opcode, BEE_OP2_SHIFT);
                text = xasprintf("JUMPI $%zx", (bee_uword_t)addr);
            }
            break;
        case BEE_OP_JUMPZI:
            {
                bee_word_t *addr = pc + ARSHIFT(opcode, BEE_OP2_SHIFT);
                text = xasprintf("JUMPZI $%zx", (bee_uword_t)addr);
            }
            break;
        case BEE_OP_TRAP:
            text = xasprintf("TRAP $%zx", (bee_uword_t)opcode >> BEE_OP2_SHIFT);
            break;
        case BEE_OP_INSN:
            opcode = (bee_uword_t)opcode >> BEE_OP2_SHIFT;
            if ((bee_uword_t)opcode <= sizeof(mnemonic) / sizeof(mnemonic[0]) &&
                mnemonic[(bee_uword_t)opcode] != NULL)
                text = xasprintf("%s", mnemonic[(bee_uword_t)opcode]);
            else
                text = strdup("(invalid instruction!)");
            break;
        }
    }
    return text;
}

_GL_ATTRIBUTE_PURE uint8_t toass(const char *token)
{
    for (size_t i = 0; i < sizeof(mnemonic) / sizeof(mnemonic[0]); i++)
        if (mnemonic[i] && strcmp(token, mnemonic[i]) == 0) return i;

    return BEE_INSN_UNDEFINED;
}

static char *_val_data_stack(bee_state * restrict S, bool with_hex)
{
    static char *picture = NULL;

    free(picture);
    picture = xasprintf("%s", "");
    if (S->dp > S->dsize) {
        picture = xasprintf("%s", "stack overflow");
    } else
        for (bee_uword_t i = 0; i < S->dp; i++) {
            char *ptr = xasprintf("%s%zd", picture, S->d0[i]);
            free(picture);
            picture = ptr;
            if (with_hex) {
                ptr = xasprintf("%s ($%zx) ", picture, (bee_uword_t)S->d0[i]);
                free(picture);
                picture = ptr;
            }
            if (i != S->dp - 1) {
                ptr = xasprintf("%s ", picture);
                free(picture);
                picture = ptr;
            }
        }

    return picture;
}

char *val_data_stack(bee_state * restrict S)
{
    return _val_data_stack(S, false);
}

void show_data_stack(bee_state * restrict S)
{
    if (S->dp == 0)
        printf("Data stack empty\n");
    else if (S->dp > S->dsize)
        printf("Data stack overflow\n");
    else
        printf("Data stack: %s\n", _val_data_stack(S, true));
}

void show_return_stack(bee_state * restrict S)
{
    if (S->sp == 0)
        printf("Return stack empty\n");
    else if (S->sp > S->ssize)
        printf("Return stack overflow\n");
    else {
        printf("Return stack: ");
        for (bee_uword_t i = 0; i < S->sp; i++)
            printf("$%zx ", (bee_uword_t)S->s0[i]);
        putchar('\n');
    }
}


struct {
    int code;
    const char *msg;
} error_msg[] = {
    { 0, "BEE_ERROR_OK" },
    { -1, "BEE_ERROR_INVALID_OPCODE" },
    { -2, "BEE_ERROR_STACK_UNDERFLOW" },
    { -3, "BEE_ERROR_STACK_OVERFLOW" },
    { -4, "BEE_ERROR_UNALIGNED_ADDRESS" },
    { -16, "BEE_ERROR_INVALID_LIBRARY" },
    { -17, "BEE_ERROR_INVALID_FUNCTION" },
    { -256, "BEE_ERROR_BREAK" },
};

_GL_ATTRIBUTE_PURE const char *error_to_msg(int code)
{
    for (size_t i = 0; i < sizeof(error_msg) / sizeof(error_msg[0]); i++) {
        if (error_msg[i].code == code)
            return error_msg[i].msg;
    }
    return "unknown error";
}

// Return value of pc after successful execution of the next instruction,
// which may not be valid, or NULL if the new value of pc cannot be
// computed.
static bee_word_t *compute_next_PC(bee_state * restrict S, bee_word_t inst)
{
    switch (inst & BEE_OP1_MASK) {
    case BEE_OP_CALLI:
        return S->pc + ARSHIFT(inst, BEE_OP1_SHIFT);
    case BEE_OP_PUSHI:
    case BEE_OP_PUSHRELI:
        return S->pc + 1;
    default:
        switch (inst & BEE_OP2_MASK) {
        case BEE_OP_JUMPI:
        case BEE_OP_JUMPZI:
            return S->pc + ARSHIFT(inst, BEE_OP2_SHIFT);
            break;
        case BEE_OP_TRAP:
            return S->pc + 1;
            break;
        case BEE_OP_INSN:
            switch (inst >> BEE_OP2_SHIFT) {
            case BEE_INSN_NOP:
            case BEE_INSN_NOT:
            case BEE_INSN_AND:
            case BEE_INSN_OR:
            case BEE_INSN_XOR:
            case BEE_INSN_LSHIFT:
            case BEE_INSN_RSHIFT:
            case BEE_INSN_ARSHIFT:
            case BEE_INSN_POP:
            case BEE_INSN_DUP:
            case BEE_INSN_SET:
            case BEE_INSN_SWAP:
            case BEE_INSN_LOAD:
            case BEE_INSN_STORE:
            case BEE_INSN_LOAD1:
            case BEE_INSN_STORE1:
            case BEE_INSN_LOAD2:
            case BEE_INSN_STORE2:
            case BEE_INSN_LOAD4:
            case BEE_INSN_STORE4:
            case BEE_INSN_LOAD_IA:
            case BEE_INSN_STORE_DB:
            case BEE_INSN_LOAD_IB:
            case BEE_INSN_STORE_DA:
            case BEE_INSN_LOAD_DA:
            case BEE_INSN_STORE_IB:
            case BEE_INSN_LOAD_DB:
            case BEE_INSN_STORE_IA:
            case BEE_INSN_NEG:
            case BEE_INSN_ADD:
            case BEE_INSN_MUL:
            case BEE_INSN_DIVMOD:
            case BEE_INSN_UDIVMOD:
            case BEE_INSN_EQ:
            case BEE_INSN_LT:
            case BEE_INSN_ULT:
            case BEE_INSN_PUSHS:
            case BEE_INSN_POPS:
            case BEE_INSN_DUPS:
            case BEE_INSN_WORD_BYTES:
            case BEE_INSN_GET_SSIZE:
            case BEE_INSN_GET_SP:
            case BEE_INSN_SET_SP:
            case BEE_INSN_GET_DSIZE:
            case BEE_INSN_GET_DP:
            case BEE_INSN_SET_DP:
            case BEE_INSN_GET_HANDLER_SP:
                return S->pc + 1;
            case BEE_INSN_JUMP:
            case BEE_INSN_CALL:
            case BEE_INSN_CATCH:
                if (S->dp < 1)
                    return NULL;
                return (bee_word_t *)(S->d0[S->dp - 1]);
            case BEE_INSN_JUMPZ:
                if (S->dp < 2)
                    return NULL;
                return S->d0[S->dp - 2] == 0 ? (bee_word_t *)S->d0[S->dp - 1] : S->pc + 1;
            case BEE_INSN_RET:
                if (S->sp < 1)
                    return NULL;
                return (bee_word_t *)(S->s0[S->sp - 1] & ~1);
            case BEE_INSN_THROW:
                if (S->handler_sp < 2)
                    return NULL;
                return (bee_word_t *)(S->s0[S->handler_sp - 1]);
            case BEE_INSN_BREAK:
            default:
                return NULL;
            }
            break;
        default:
            return NULL;
        }
    }
}

bee_word_t single_step(bee_state * restrict S)
{
    bee_word_t error = 0;
    bee_word_t inst = *S->pc, next_inst;
    bee_word_t *next_PC = compute_next_PC(S, inst);
    int next_PC_valid = next_PC != NULL && IS_ALIGNED(next_PC);
    if (next_PC_valid) {
        next_inst = *next_PC;
        *next_PC = (BEE_INSN_BREAK << BEE_OP2_SHIFT) | BEE_OP_INSN;
    }
    bee_uword_t save_handler_sp = S->handler_sp;
    S->handler_sp = 0;
    error = bee_run(S);
    if (next_PC_valid) {
        *next_PC = next_inst;
        if (error == BEE_ERROR_BREAK)
            S->pc = next_PC;
    }
    // Restore S->handler_sp if it wasn't set by CATCH
    if (S->handler_sp == 0)
        S->handler_sp = save_handler_sp;
    // If there's a saved error handler, execute any actions that would have
    // been executed by `run()` had there been an error handler.
    if (S->handler_sp != 0) {
        if (error != BEE_ERROR_BREAK || inst == ((BEE_INSN_THROW << BEE_OP2_SHIFT) | BEE_OP_INSN)) {
        // If an error occurred or THROW was performed, go to the error
        // handler.
        if (S->dp < S->dsize)
            S->d0[S->dp++] = error;
        S->sp = S->handler_sp;
        bee_uword_t addr;
        POPS((bee_word_t *)&addr);
        POPS((bee_word_t *)&S->handler_sp);
        S->pc = (bee_word_t *)(addr & ~1);
        } else if (inst == ((BEE_INSN_RET << BEE_OP2_SHIFT) | BEE_OP_INSN) && S->sp < S->handler_sp) {
            // Otherwise, if the last instruction was RET, pop an error handler if necessary.
            POPS((bee_word_t *)&S->handler_sp);
            PUSHD(0);
        }
    }
 error:
    return error;
}

bee_state *init_defaults(bee_word_t *pc)
{
    return bee_init(pc, BEE_DEFAULT_STACK_SIZE, BEE_DEFAULT_STACK_SIZE);
}

bool run_test(const char *name, bee_state *S, char *correct[], size_t steps, bool errors_allowed)
{
    bool ok = true;

    for (size_t i = 0; i < steps; i++) {
        printf("Instruction = %s\n", disass(*S->pc, S->pc));
        bee_word_t ret = single_step(S);
        if (errors_allowed)
            printf("single_step() returns %zd (%s)\n", ret, error_to_msg(ret));
        else
            assert(ret == BEE_ERROR_BREAK);
        show_data_stack(S);
        printf("Correct stack: %s\n\n", correct[i]);
        if (strcmp(correct[i], val_data_stack(S))) {
            printf("Error in %s tests: pc = %p\n", name, S->pc);
            ok = false;
            break;
        }
    }

    for (size_t i = 0; i < steps; i++)
        free(correct[i]);

    if (ok)
        printf("%s tests ran OK\n", name);
    fflush(stdout);
    return ok;
}