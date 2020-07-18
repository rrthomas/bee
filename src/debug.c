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

#include "external_syms.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "xvasprintf.h"

#include "bee/bee.h"
#include "bee/aux.h"

#include "debug.h"
#include "opcodes.h"


static uint8_t *current; // where we assemble the next instruction word or literal


void word(WORD value)
{
    current = (uint8_t *)ALIGN(current);
    store_word((WORD *)current, value);
    current += WORD_BYTES;
}

void ass(UWORD inst)
{
    word((((inst << 2) | BEE_OP2_INSN) << 2) | BEE_OP_LEVEL2);
}

void ass_trap(UWORD code)
{
    word((((code << 2) | BEE_OP2_TRAP) << 2) | BEE_OP_LEVEL2);
}

void ass_byte(uint8_t b)
{
    store_byte(current++, b);
}

void pushi(WORD literal)
{
    WORD temp = LSHIFT(literal, 2);
    assert(ARSHIFT(temp, 2) == (UWORD)literal);
    word(temp | BEE_OP_PUSHI);
}

static void addr_op(int op, WORD *addr)
{
    word(LSHIFT(addr - (WORD *)current, 2) | op);
}

void calli(WORD *addr)
{
    addr_op(BEE_OP_CALLI, addr);
}

void pushreli(WORD *addr)
{
    addr_op(BEE_OP_PUSHRELI, addr);
}

static void addr_op2(int op, WORD *addr)
{
    WORD offset = LSHIFT(addr - (WORD *)current, 2);
    WORD temp = LSHIFT(offset, 2);
    assert(temp >> 2 == offset);
    word(LSHIFT(offset | op, 2) | BEE_OP_LEVEL2);
}

void jumpi(WORD *addr)
{
    addr_op2(BEE_OP2_JUMPI, addr);
}

void jumpzi(WORD *addr)
{
    addr_op2(BEE_OP2_JUMPZI, addr);
}

void ass_goto(WORD *addr)
{
    current = (uint8_t *)addr;
}

_GL_ATTRIBUTE_PURE WORD *label(void)
{
    return (WORD *)current;
}

static const char *mnemonic[BEE_INSN_UNDEFINED + 1] = {
// 0x00
    "NOP", "NOT", "AND", "OR", "XOR", "LSHIFT", "RSHIFT", "ARSHIFT",
    "POP", "DUP", "SET", "SWAP", "JUMP", "JUMPZ", "CALL", "RET",
// 0x10
    "LOAD", "STORE", "LOAD1", "STORE1", "LOAD2", "STORE2", "LOAD4", "STORE4",
    "NEGATE", "ADD", "MUL", "DIVMOD", "UDIVMOD", "EQ", "LT", "ULT",
// 0x20
    "PUSHR", "POPR", "DUPR", "CATCH", "THROW", "BREAK", "WORD_BYTES", "GET_M0",
    "GET_MSIZE", "GET_RSIZE", "GET_RP", "SET_RP", "GET_SSIZE", "GET_SP", "SET_SP", "GET_HANDLER_RP",
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
};

_GL_ATTRIBUTE_CONST const char *disass(WORD opcode, WORD *pc)
{
    static char *text = NULL;

    free(text);
    switch (opcode & BEE_OP_MASK) {
    case BEE_OP_CALLI:
        {
            WORD *addr = pc + ARSHIFT(opcode, 2);
            text = xasprintf("CALLI $%"PRIX32, (UWORD)addr);
        }
        break;
    case BEE_OP_PUSHI:
        {
            opcode = ARSHIFT(opcode, 2);
            text = xasprintf("PUSHI %"PRIi32"=$%"PRIX32, opcode, (UWORD)opcode);
        }
        break;
    case BEE_OP_PUSHRELI:
        text = xasprintf("PUSHRELI $%"PRIX32, (UWORD)(pc + (opcode & ~BEE_OP_MASK)));
        break;
    default:
        opcode = ARSHIFT(opcode, 2);
        switch (opcode & BEE_OP2_MASK) {
        case BEE_OP2_JUMPI:
            {
                WORD *addr = pc + ARSHIFT(opcode, 2);
                text = xasprintf("JUMPI $%"PRIX32, (UWORD)addr);
            }
            break;
        case BEE_OP2_JUMPZI:
            {
                WORD *addr = pc + ARSHIFT(opcode, 2);
                text = xasprintf("JUMPZI $%"PRIX32, (UWORD)addr);
            }
            break;
        case BEE_OP2_INSN:
            opcode >>= 2;
            if ((UWORD)opcode <= sizeof(mnemonic) / sizeof(mnemonic[0]) &&
                mnemonic[(UWORD)opcode] != NULL)
                text = xasprintf("%s", mnemonic[(UWORD)opcode]);
            else
                text = strdup("(invalid instruction!)");
            break;
        case BEE_OP2_TRAP:
            text = xasprintf("TRAP $%"PRIX32, (UWORD)opcode >> 2);
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
    if (SP > SSIZE) {
        picture = xasprintf("%s", "stack overflow");
    } else
        for (UWORD i = 0; i < SP; i++) {
            char *ptr = xasprintf("%s%"PRId32, picture, S0[i]);
            free(picture);
            picture = ptr;
            if (with_hex) {
                ptr = xasprintf("%s ($%"PRIX32") ", picture, (UWORD)S0[i]);
                free(picture);
                picture = ptr;
            }
            if (i != SP - 1) {
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
    if (SP == 0)
        printf("Data stack empty\n");
    else if (SP > SSIZE)
        printf("Data stack overflow\n");
    else
        printf("Data stack: %s\n", _val_data_stack(true));
}

void show_return_stack(void)
{
    if (RP == 0)
        printf("Return stack empty\n");
    else if (RP > RSIZE)
        printf("Return stack overflow\n");
    else {
        printf("Return stack: ");
        for (UWORD i = 0; i < RP; i++)
            printf("$%"PRIX32" ", (UWORD)R0[i]);
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

// Return value of PC after successful execution of the next instruction,
// which may not be valid, or NULL if the new value of PC cannot be
// computed.
static WORD *compute_next_PC(WORD inst)
{
    switch (inst & BEE_OP_MASK) {
    case BEE_OP_CALLI:
        return PC + ARSHIFT(inst, 2);
    case BEE_OP_PUSHI:
    case BEE_OP_PUSHRELI:
        return PC + 1;
    case BEE_OP_LEVEL2:
        inst = ARSHIFT(inst, 2);
        switch (inst & BEE_OP2_MASK) {
        case BEE_OP2_JUMPI:
        case BEE_OP2_JUMPZI:
            return PC + ARSHIFT(inst, 2);
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
            case BEE_INSN_NEGATE:
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
            case BEE_INSN_GET_RSIZE:
            case BEE_INSN_GET_RP:
            case BEE_INSN_SET_RP:
            case BEE_INSN_GET_SSIZE:
            case BEE_INSN_GET_SP:
            case BEE_INSN_SET_SP:
            case BEE_INSN_GET_HANDLER_RP:
                return PC + 1;
            case BEE_INSN_JUMP:
            case BEE_INSN_CALL:
            case BEE_INSN_CATCH:
                if (SP < 1)
                    return NULL;
                return (WORD *)(S0[SP - 1]);
            case BEE_INSN_JUMPZ:
                if (SP < 2)
                    return NULL;
                return S0[SP - 2] == 0 ? (WORD *)S0[SP - 1] : PC + 1;
            case BEE_INSN_RET:
                if (RP < 1)
                    return NULL;
                return (WORD *)(R0[RP - 1] & ~1);
            case BEE_INSN_THROW:
                if (HANDLER_RP == (UWORD)-1 || HANDLER_RP < 2)
                    return NULL;
                return (WORD *)(R0[HANDLER_RP - 1] & ~1);
            case BEE_INSN_BREAK:
            default:
                return NULL;
            }
            break;
        case BEE_OP2_TRAP:
            return PC + 1;
            break;
        }
    default:
        return NULL;
    }
}

WORD single_step(void)
{
    WORD error = 0;
    WORD inst, next_inst;
    if ((error = load_word(PC, &inst)) != ERROR_OK)
        return error;
    WORD *next_PC = compute_next_PC(inst);
    int next_PC_valid = next_PC != NULL && IS_ALIGNED(next_PC);
    if (next_PC_valid) {
        assert(load_word(next_PC, &next_inst) == ERROR_OK);
        assert(store_word(next_PC, (((BEE_INSN_BREAK << 2) | BEE_OP2_INSN) << 2) | BEE_OP_LEVEL2) == ERROR_OK);
    }
    UWORD save_HANDLER_RP = HANDLER_RP;
    HANDLER_RP = -1;
    error = run();
    if (next_PC_valid) {
        assert(store_word(next_PC, next_inst) == ERROR_OK);
        if (error == ERROR_BREAK)
            PC = next_PC;
    }
    // Restore HANDLER_RP if it wasn't set by CATCH
    if (HANDLER_RP == (UWORD)-1)
        HANDLER_RP = save_HANDLER_RP;
    if ((error != ERROR_BREAK || inst == ((((BEE_INSN_THROW << 2) | BEE_OP2_INSN) << 2) | BEE_OP_LEVEL2)) &&
        HANDLER_RP != (UWORD)-1) {
        // If an error occurred or THROW was executed, and there's a saved
        // error handler, execute it.
        if (SP < SSIZE)
            S0[SP++] = error;
        RP = HANDLER_RP;
        UWORD addr;
        POP_RETURN((WORD *)&addr);
        POP_RETURN((WORD *)&HANDLER_RP);
        PC = (WORD *)(addr & ~1);
    }
 error:
    return error;
}
