// Functions useful for VM debugging.
//
// (c) Reuben Thomas 1994-2020
//
// The package is distributed under the GNU Public License version 3, or,
// at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#include "config.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "xvasprintf.h"

#include "bee/bee.h"
#include "bee/opcodes.h"

#include "private.h"
#include "debug.h"


static uint8_t *current; // where we assemble the next instruction word or literal


void word(bee_WORD value)
{
    current = (uint8_t *)ALIGN(current);
    *(bee_WORD *)current = value;
    current += bee_WORD_BYTES;
}

void ass(bee_UWORD inst)
{
    word((((inst << 2) | BEE_OP2_INSN) << 2) | BEE_OP_LEVEL2);
}

void ass_trap(bee_UWORD code)
{
    word((((code << 2) | BEE_OP2_TRAP) << 2) | BEE_OP_LEVEL2);
}

void ass_byte(uint8_t b)
{
    *current++ = b;
}

void pushi(bee_WORD literal)
{
    bee_WORD temp = LSHIFT(literal, 2);
    assert(ARSHIFT(temp, 2) == (bee_UWORD)literal);
    word(temp | BEE_OP_PUSHI);
}

static void addr_op(int op, bee_WORD *addr)
{
    word(LSHIFT(addr - (bee_WORD *)current, 2) | op);
}

void calli(bee_WORD *addr)
{
    addr_op(BEE_OP_CALLI, addr);
}

void pushreli(bee_WORD *addr)
{
    addr_op(BEE_OP_PUSHRELI, addr);
}

static void addr_op2(int op, bee_WORD *addr)
{
    bee_WORD offset = LSHIFT(addr - (bee_WORD *)current, 2);
    bee_WORD temp = LSHIFT(offset, 2);
    assert(temp >> 2 == offset);
    word(LSHIFT(offset | op, 2) | BEE_OP_LEVEL2);
}

void jumpi(bee_WORD *addr)
{
    addr_op2(BEE_OP2_JUMPI, addr);
}

void jumpzi(bee_WORD *addr)
{
    addr_op2(BEE_OP2_JUMPZI, addr);
}

void ass_goto(bee_WORD *addr)
{
    current = (uint8_t *)addr;
}

_GL_ATTRIBUTE_PURE bee_WORD *label(void)
{
    return (bee_WORD *)current;
}

static const char *mnemonic[BEE_INSN_UNDEFINED + 1] = {
// 0x00
    "NOP", "NOT", "AND", "OR", "XOR", "LSHIFT", "RSHIFT", "ARSHIFT",
    "POP", "DUP", "SET", "SWAP", "JUMP", "JUMPZ", "CALL", "RET",
// 0x10
    "LOAD", "STORE", "LOAD1", "STORE1", "LOAD2", "STORE2", "LOAD4", "STORE4",
    "NEG", "ADD", "MUL", "DIVMOD", "UDIVMOD", "EQ", "LT", "ULT",
// 0x20
    "PUSHR", "POPR", "DUPR", "CATCH", "THROW", "BREAK", "bee_WORD_BYTES", "GET_M0",
    "GET_MSIZE", "GET_SSIZE", "GET_SP", "SET_SP", "GET_DSIZE", "GET_SP", "SET_SP", "GET_HANDLER_SP",
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
};

_GL_ATTRIBUTE_CONST const char *disass(bee_WORD opcode, bee_WORD *pc)
{
    static char *text = NULL;

    free(text);
    switch (opcode & BEE_OP_MASK) {
    case BEE_OP_CALLI:
        {
            bee_WORD *addr = pc + ARSHIFT(opcode, 2);
            text = xasprintf("CALLI $%"PRIX32, (bee_UWORD)addr);
        }
        break;
    case BEE_OP_PUSHI:
        {
            opcode = ARSHIFT(opcode, 2);
            text = xasprintf("PUSHI %"PRIi32"=$%"PRIX32, opcode, (bee_UWORD)opcode);
        }
        break;
    case BEE_OP_PUSHRELI:
        text = xasprintf("PUSHRELI $%"PRIX32, (bee_UWORD)(pc + (opcode & ~BEE_OP_MASK)));
        break;
    default:
        opcode = ARSHIFT(opcode, 2);
        switch (opcode & BEE_OP2_MASK) {
        case BEE_OP2_JUMPI:
            {
                bee_WORD *addr = pc + ARSHIFT(opcode, 2);
                text = xasprintf("JUMPI $%"PRIX32, (bee_UWORD)addr);
            }
            break;
        case BEE_OP2_JUMPZI:
            {
                bee_WORD *addr = pc + ARSHIFT(opcode, 2);
                text = xasprintf("JUMPZI $%"PRIX32, (bee_UWORD)addr);
            }
            break;
        case BEE_OP2_TRAP:
            text = xasprintf("TRAP $%"PRIX32, (bee_UWORD)opcode >> 2);
            break;
        case BEE_OP2_INSN:
            opcode >>= 2;
            if ((bee_UWORD)opcode <= sizeof(mnemonic) / sizeof(mnemonic[0]) &&
                mnemonic[(bee_UWORD)opcode] != NULL)
                text = xasprintf("%s", mnemonic[(bee_UWORD)opcode]);
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

static char *_val_data_stack(bool with_hex)
{
    static char *picture = NULL;

    free(picture);
    picture = xasprintf("%s", "");
    if (bee_dp > bee_dsize) {
        picture = xasprintf("%s", "stack overflow");
    } else
        for (bee_UWORD i = 0; i < bee_dp; i++) {
            char *ptr = xasprintf("%s%"PRId32, picture, bee_d0[i]);
            free(picture);
            picture = ptr;
            if (with_hex) {
                ptr = xasprintf("%s ($%"PRIX32") ", picture, (bee_UWORD)bee_d0[i]);
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
        for (bee_UWORD i = 0; i < bee_sp; i++)
            printf("$%"PRIX32" ", (bee_UWORD)bee_s0[i]);
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
    { -4, "BEE_ERROR_INVALID_LOAD" },
    { -5, "BEE_ERROR_INVALID_STORE" },
    { -6, "BEE_ERROR_UNALIGNED_ADDRESS" },
    { -7, "BEE_ERROR_DIVISION_BY_ZERO" },
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
static bee_WORD *compute_next_PC(bee_WORD inst)
{
    switch (inst & BEE_OP_MASK) {
    case BEE_OP_CALLI:
        return bee_pc + ARSHIFT(inst, 2);
    case BEE_OP_PUSHI:
    case BEE_OP_PUSHRELI:
        return bee_pc + 1;
    case BEE_OP_LEVEL2:
        inst = ARSHIFT(inst, 2);
        switch (inst & BEE_OP2_MASK) {
        case BEE_OP2_JUMPI:
        case BEE_OP2_JUMPZI:
            return bee_pc + ARSHIFT(inst, 2);
            break;
        case BEE_OP2_TRAP:
            return bee_pc + 1;
            break;
        case BEE_OP2_INSN:
            switch (inst >> 2) {
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
            case BEE_INSN_NEG:
            case BEE_INSN_ADD:
            case BEE_INSN_MUL:
            case BEE_INSN_DIVMOD:
            case BEE_INSN_UDIVMOD:
            case BEE_INSN_EQ:
            case BEE_INSN_LT:
            case BEE_INSN_ULT:
            case BEE_INSN_PUSHR:
            case BEE_INSN_POPR:
            case BEE_INSN_DUPR:
            case BEE_INSN_WORD_BYTES:
            case BEE_INSN_GET_M0:
            case BEE_INSN_GET_MSIZE:
            case BEE_INSN_GET_SSIZE:
            case BEE_INSN_GET_SP:
            case BEE_INSN_SET_SP:
            case BEE_INSN_GET_DSIZE:
            case BEE_INSN_GET_DP:
            case BEE_INSN_SET_DP:
            case BEE_INSN_GET_HANDLER_SP:
                return bee_pc + 1;
            case BEE_INSN_JUMP:
            case BEE_INSN_CALL:
            case BEE_INSN_CATCH:
                if (bee_dp < 1)
                    return NULL;
                return (bee_WORD *)(bee_d0[bee_dp - 1]);
            case BEE_INSN_JUMPZ:
                if (bee_dp < 2)
                    return NULL;
                return bee_d0[bee_dp - 2] == 0 ? (bee_WORD *)bee_d0[bee_dp - 1] : bee_pc + 1;
            case BEE_INSN_RET:
                if (bee_sp < 1)
                    return NULL;
                return (bee_WORD *)(bee_s0[bee_sp - 1] & ~1);
            case BEE_INSN_THROW:
                if (bee_handler_sp == (bee_UWORD)-1 || bee_handler_sp < 2)
                    return NULL;
                return (bee_WORD *)(bee_s0[bee_handler_sp - 1] & ~1);
            case BEE_INSN_BREAK:
            default:
                return NULL;
            }
            break;
        }
    default:
        return NULL;
    }
}

bee_WORD single_step(void)
{
    bee_WORD error = 0;
    bee_WORD inst = *bee_pc, next_inst;
    bee_WORD *next_PC = compute_next_PC(inst);
    int next_PC_valid = next_PC != NULL && IS_ALIGNED(next_PC);
    if (next_PC_valid) {
        next_inst = *next_PC;
        *next_PC = (((BEE_INSN_BREAK << 2) | BEE_OP2_INSN) << 2) | BEE_OP_LEVEL2;
    }
    bee_UWORD save_handler_sp = bee_handler_sp;
    bee_handler_sp = -1;
    error = bee_run();
    if (next_PC_valid) {
        *next_PC = next_inst;
        if (error == BEE_ERROR_BREAK)
            bee_pc = next_PC;
    }
    // Restore bee_handler_sp if it wasn't set by CATCH
    if (bee_handler_sp == (bee_UWORD)-1)
        bee_handler_sp = save_handler_sp;
    if ((error != BEE_ERROR_BREAK || inst == ((((BEE_INSN_THROW << 2) | BEE_OP2_INSN) << 2) | BEE_OP_LEVEL2)) &&
        bee_handler_sp != (bee_UWORD)-1) {
        // If an error occurred or THROW was executed, and there's a saved
        // error handler, execute it.
        if (bee_dp < bee_dsize)
            bee_d0[bee_dp++] = error;
        bee_sp = bee_handler_sp;
        bee_UWORD addr;
        POP_RETURN((bee_WORD *)&addr);
        POP_RETURN((bee_WORD *)&bee_handler_sp);
        bee_pc = (bee_WORD *)(addr & ~1);
    }
 error:
    return error;
}
