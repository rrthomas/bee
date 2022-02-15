// Functions useful for VM debugging.
//
// (c) Reuben Thomas 1994-2020
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
#include "debug.h"


static uint8_t *current; // where we assemble the next instruction word or literal


void word(bee_word_t value)
{
    current = (uint8_t *)ALIGN(current);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
    *(bee_word_t *)current = value;
#pragma GCC diagnostic pop
    current += BUMBLE_WORD_BYTES;
}

void ass(bee_uword_t inst)
{
    word((inst << BUMBLE_OP2_SHIFT) | BUMBLE_OP_INSN);
}

void ass_byte(uint8_t b)
{
    *current++ = b;
}

void pushi(bee_word_t literal)
{
    bee_word_t temp = LSHIFT(literal, BUMBLE_OP1_SHIFT);
    assert(ARSHIFT(temp, BUMBLE_OP1_SHIFT) == literal);
    word(temp | BUMBLE_OP_PUSHI);
}

static void addr_op(int op, bee_word_t *addr)
{
    assert(IS_ALIGNED(current));
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
    word(LSHIFT(addr - (bee_word_t *)current, BUMBLE_OP1_SHIFT) | op);
#pragma GCC diagnostic pop
}

void calli(bee_word_t *addr)
{
    addr_op(BUMBLE_OP_CALLI, addr);
}

void pushreli(bee_word_t *addr)
{
    addr_op(BUMBLE_OP_PUSHRELI, addr);
}

static void addr_op2(int op, bee_word_t *addr)
{
    assert(IS_ALIGNED(current));
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
    bee_word_t offset = addr - (bee_word_t *)current;
#pragma GCC diagnostic pop
    bee_word_t temp = LSHIFT(offset, BUMBLE_OP2_SHIFT);
    assert(ARSHIFT(temp, BUMBLE_OP2_SHIFT) == offset);
    word(temp | op);
}

void jumpi(bee_word_t *addr)
{
    addr_op2(BUMBLE_OP_JUMPI, addr);
}

void jumpzi(bee_word_t *addr)
{
    addr_op2(BUMBLE_OP_JUMPZI, addr);
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

static const char *mnemonic[BUMBLE_INSN_UNDEFINED + 1] = {
// 0x00
    "NOP", NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    "POPD", "DUP", "SET", "SWAP", "JUMP", "JUMPZ", "CALL", "RET",
// 0x10
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
// 0x20
    "PUSHS", "POPS", "DUPS", "CATCH", "THROW", "BREAK", NULL, "GET_M0",
    "GET_MSIZE", "GET_SSIZE", "GET_SP", "SET_SP", "GET_DSIZE", "GET_SP", "SET_SP", "GET_HANDLER_SP",
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
};

_GL_ATTRIBUTE_CONST const char *disass(bee_word_t opcode, bee_word_t *pc)
{
    static char *text = NULL;

    free(text);
    switch (opcode & BUMBLE_OP1_MASK) {
    case BUMBLE_OP_CALLI:
        {
            bee_word_t *addr = pc + ARSHIFT(opcode, BUMBLE_OP1_SHIFT);
            text = xasprintf("CALLI $%zx", (bee_uword_t)addr);
        }
        break;
    case BUMBLE_OP_PUSHI:
        {
            opcode = ARSHIFT(opcode, BUMBLE_OP1_SHIFT);
            text = xasprintf("PUSHI %zd=$%zx", opcode, (bee_uword_t)opcode);
        }
        break;
    case BUMBLE_OP_PUSHRELI:
        text = xasprintf("PUSHRELI $%zx", (bee_uword_t)(pc + (opcode & ~BUMBLE_OP1_MASK)));
        break;
    default:
        switch (opcode & BUMBLE_OP2_MASK) {
        case BUMBLE_OP_JUMPI:
            {
                bee_word_t *addr = pc + ARSHIFT(opcode, BUMBLE_OP2_SHIFT);
                text = xasprintf("JUMPI $%zx", (bee_uword_t)addr);
            }
            break;
        case BUMBLE_OP_JUMPZI:
            {
                bee_word_t *addr = pc + ARSHIFT(opcode, BUMBLE_OP2_SHIFT);
                text = xasprintf("JUMPZI $%zx", (bee_uword_t)addr);
            }
            break;
        case BUMBLE_OP_INSN:
            opcode = (bee_uword_t)opcode >> BUMBLE_OP2_SHIFT;
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

    return BUMBLE_INSN_UNDEFINED;
}

static char *_val_data_stack(bool with_hex)
{
    static char *picture = NULL;

    free(picture);
    picture = xasprintf("%s", "");
    if (bee_dp > bee_dsize) {
        picture = xasprintf("%s", "stack overflow");
    } else
        for (bee_uword_t i = 0; i < bee_dp; i++) {
            char *ptr = xasprintf("%s%zd", picture, bee_d0[i]);
            free(picture);
            picture = ptr;
            if (with_hex) {
                ptr = xasprintf("%s ($%zx) ", picture, (bee_uword_t)bee_d0[i]);
                free(picture);
                picture = ptr;
            }
            if (i != bee_dp - 1) {
                ptr = xasprintf("%s ", picture);
                free(picture);
                picture = ptr;
            }
        }

    return picture;
}

char *val_data_stack(void)
{
    return _val_data_stack(false);
}

void show_data_stack(void)
{
    if (bee_dp == 0)
        printf("Data stack empty\n");
    else if (bee_dp > bee_dsize)
        printf("Data stack overflow\n");
    else
        printf("Data stack: %s\n", _val_data_stack(true));
}

void show_return_stack(void)
{
    if (bee_sp == 0)
        printf("Return stack empty\n");
    else if (bee_sp > bee_ssize)
        printf("Return stack overflow\n");
    else {
        printf("Return stack: ");
        for (bee_uword_t i = 0; i < bee_sp; i++)
            printf("$%zx ", (bee_uword_t)bee_s0[i]);
        putchar('\n');
    }
}


struct {
    int code;
    const char *msg;
} error_msg[] = {
    { 0, "BUMBLE_ERROR_OK" },
    { -1, "BUMBLE_ERROR_INVALID_OPCODE" },
    { -2, "BUMBLE_ERROR_STACK_UNDERFLOW" },
    { -3, "BUMBLE_ERROR_STACK_OVERFLOW" },
    { -4, "BUMBLE_ERROR_INVALID_LOAD" },
    { -5, "BUMBLE_ERROR_INVALID_STORE" },
    { -6, "BUMBLE_ERROR_UNALIGNED_ADDRESS" },
    { -7, "BUMBLE_ERROR_DIVISION_BY_ZERO" },
    { -256, "BUMBLE_ERROR_BREAK" },
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
static bee_word_t *compute_next_PC(bee_word_t inst)
{
    switch (inst & BUMBLE_OP1_MASK) {
    case BUMBLE_OP_CALLI:
        return bee_pc + ARSHIFT(inst, BUMBLE_OP1_SHIFT);
    case BUMBLE_OP_PUSHI:
    case BUMBLE_OP_PUSHRELI:
        return bee_pc + 1;
    default:
        switch (inst & BUMBLE_OP2_MASK) {
        case BUMBLE_OP_JUMPI:
        case BUMBLE_OP_JUMPZI:
            return bee_pc + ARSHIFT(inst, BUMBLE_OP2_SHIFT);
            break;
        case BUMBLE_OP_INSN:
            switch (inst >> BUMBLE_OP2_SHIFT) {
            case BUMBLE_INSN_NOP:
            case BUMBLE_INSN_POP:
            case BUMBLE_INSN_DUP:
            case BUMBLE_INSN_SET:
            case BUMBLE_INSN_SWAP:
            case BUMBLE_INSN_PUSHS:
            case BUMBLE_INSN_POPS:
            case BUMBLE_INSN_DUPS:
            case BUMBLE_INSN_GET_M0:
            case BUMBLE_INSN_GET_MSIZE:
            case BUMBLE_INSN_GET_SSIZE:
            case BUMBLE_INSN_GET_SP:
            case BUMBLE_INSN_SET_SP:
            case BUMBLE_INSN_GET_DSIZE:
            case BUMBLE_INSN_GET_DP:
            case BUMBLE_INSN_SET_DP:
            case BUMBLE_INSN_GET_HANDLER_SP:
                return bee_pc + 1;
            case BUMBLE_INSN_JUMP:
            case BUMBLE_INSN_CALL:
            case BUMBLE_INSN_CATCH:
                if (bee_dp < 1)
                    return NULL;
                return (bee_word_t *)(bee_d0[bee_dp - 1]);
            case BUMBLE_INSN_JUMPZ:
                if (bee_dp < 2)
                    return NULL;
                return bee_d0[bee_dp - 2] == 0 ? (bee_word_t *)bee_d0[bee_dp - 1] : bee_pc + 1;
            case BUMBLE_INSN_RET:
                if (bee_sp < 1)
                    return NULL;
                return (bee_word_t *)(bee_s0[bee_sp - 1] & ~1);
            case BUMBLE_INSN_THROW:
                if (bee_handler_sp < 2)
                    return NULL;
                return (bee_word_t *)(bee_s0[bee_handler_sp - 1]);
            case BUMBLE_INSN_BREAK:
            default:
                return NULL;
            }
            break;
        default:
            return NULL;
        }
    }
}

bee_word_t single_step(void)
{
    bee_word_t error = 0;
    bee_word_t inst = *bee_pc, next_inst;
    bee_word_t *next_PC = compute_next_PC(inst);
    int next_PC_valid = next_PC != NULL && IS_ALIGNED(next_PC);
    if (next_PC_valid) {
        next_inst = *next_PC;
        *next_PC = (BUMBLE_INSN_BREAK << BUMBLE_OP2_SHIFT) | BUMBLE_OP_INSN;
    }
    bee_uword_t save_handler_sp = bee_handler_sp;
    bee_handler_sp = 0;
    error = bee_run();
    if (next_PC_valid) {
        *next_PC = next_inst;
        if (error == BUMBLE_ERROR_BREAK)
            bee_pc = next_PC;
    }
    // Restore bee_handler_sp if it wasn't set by CATCH
    if (bee_handler_sp == 0)
        bee_handler_sp = save_handler_sp;
    // If there's a saved error handler, execute any actions that would have
    // been executed by `run()` had there been an error handler.
    if (bee_handler_sp != 0) {
        if (error != BUMBLE_ERROR_BREAK || inst == ((BUMBLE_INSN_THROW << BUMBLE_OP2_SHIFT) | BUMBLE_OP_INSN)) {
        // If an error occurred or THROW was performed, go to the error
        // handler.
        if (bee_dp < bee_dsize)
            bee_d0[bee_dp++] = error;
        bee_sp = bee_handler_sp;
        bee_uword_t addr;
        POPS((bee_word_t *)&addr);
        POPS((bee_word_t *)&bee_handler_sp);
        bee_pc = (bee_word_t *)(addr & ~1);
        } else if (inst == ((BUMBLE_INSN_RET << BUMBLE_OP2_SHIFT) | BUMBLE_OP_INSN) && bee_sp < bee_handler_sp) {
            // Otherwise, if the last instruction was RET, pop an error handler if necessary.
            POPS((bee_word_t *)&bee_handler_sp);
            PUSHD(0);
        }
    }
 error:
    return error;
}
