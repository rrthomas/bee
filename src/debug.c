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

#include "verify.h"
#include "xvasprintf.h"

#include "bee.h"
#include "bee_aux.h"
#include "bee_debug.h"
#include "bee_opcodes.h"


static uint8_t *current; // where we assemble the next instruction word or literal


void word(WORD value)
{
    current = (uint8_t *)ALIGN(current);
    store_word((WORD *)current, value);
    current += WORD_BYTES;
}

void ass(UWORD instr)
{
    word(instr << 2 | OP_INSTRUCTION);
}

void ass_byte(uint8_t b)
{
    store_byte(current++, b);
}

void push(WORD literal)
{
    WORD temp = LSHIFT(literal, 2);
    temp = ARSHIFT(temp, 2);
    assert(temp == literal);
    word(LSHIFT(literal, 2) | OP_PUSH);
}

static void addr_op(int op, WORD *addr)
{
    word(((uint8_t *)addr - current) | op);
}

void call(WORD *addr)
{
    addr_op(OP_CALL, addr);
}

void pushrel(WORD *addr)
{
    addr_op(OP_PUSHREL, addr);
}

void ass_goto(WORD *addr)
{
    current = (uint8_t *)addr;
}

_GL_ATTRIBUTE_PURE WORD *label(void)
{
    return (WORD *)current;
}

static const char *mnemonic[UINT8_MAX + 1] = {
// 0x00
    "NOP", "NOT", "AND", "OR", "XOR", "LSHIFT", "RSHIFT", "ARSHIFT",
    "POP", "DUP", "SET", "SWAP", "JUMP", "JUMPZ", "CALL", "RET",
// 0x10
    "LOAD", "STORE", "LOAD1", "STORE1", "LOAD2", "STORE2", "LOAD4", "STORE4",
    "NEGATE", "ADD", "MUL", "DIVMOD", "UDIVMOD", "EQ", "LT", "ULT",
// 0x20
    "PUSHR", "POPR", "DUPR", "CATCH", "THROW", NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
// 0x40
    "WORD_BYTES", "GET_M0", "GET_MSIZE", "GET_RSIZE", "GET_RP", "SET_RP", "GET_SSIZE", "GET_SP",
    "SET_SP", "GET_HANDLER_RP", NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
// 0x80
    "ARGC", "ARGLEN", "ARGCOPY", "STDIN_FILENO", "STDOUT_FILENO", "STDERR_FILENO", "OPEN-FILE", "CLOSE-FILE",
    "READ-FILE", "WRITE-FILE", "FILE-POSITION", "REPOSITION-FILE", "FLUSH-FILE", "RENAME-FILE", "DELETE-FILE", "FILE-SIZE",
    "RESIZE-FILE", "FILE-STATUS", NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, };

_GL_ATTRIBUTE_CONST const char *disass(WORD opcode, WORD *addr)
{
    static char *text = NULL;

    free(text);
    switch (opcode & OP_MASK) {
    case OP_CALL:
        {
            WORD *dest = addr + (opcode >> 2);
            text = xasprintf("CALL $%"PRIX32, (UWORD)dest);
        }
        break;
    case OP_PUSH:
        {
            opcode = ARSHIFT(opcode, 2);
            text = xasprintf("PUSH %"PRIi32"=$%"PRIX32, opcode, (UWORD)opcode);
        }
        break;
    case OP_PUSHREL:
        text = xasprintf("PUSHREL $%"PRIX32, (UWORD)(addr + (opcode & ~OP_MASK)));
        break;
    case OP_INSTRUCTION:
        opcode >>= 2;
        if ((UWORD)opcode <= sizeof(mnemonic) / sizeof(mnemonic[0]) &&
            mnemonic[(UWORD)opcode] != NULL)
            text = xasprintf("%s", mnemonic[(UWORD)opcode]);
        else
            text = strdup("(invalid instruction!)");
        break;
    }
    return text;
}

_GL_ATTRIBUTE_PURE uint8_t toass(const char *token)
{
    for (size_t i = 0; i < sizeof(mnemonic) / sizeof(mnemonic[0]); i++)
        if (mnemonic[i] && strcmp(token, mnemonic[i]) == 0) return i;

    return O_UNDEFINED;
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
    switch (inst & OP_MASK) {
    case OP_CALL:
        return (WORD *)((uint8_t *)PC + inst);
    case OP_PUSH:
    case OP_PUSHREL:
        return PC + 1;
    case OP_INSTRUCTION:
        switch (inst >> 2) {
        case O_NOP:
        case O_NOT:
        case O_AND:
        case O_OR:
        case O_XOR:
        case O_LSHIFT:
        case O_RSHIFT:
        case O_ARSHIFT:
        case O_POP:
        case O_DUP:
        case O_SET:
        case O_SWAP:
        case O_LOAD:
        case O_STORE:
        case O_LOAD1:
        case O_STORE1:
        case O_LOAD2:
        case O_STORE2:
        case O_LOAD4:
        case O_STORE4:
        case O_NEGATE:
        case O_ADD:
        case O_MUL:
        case O_DIVMOD:
        case O_UDIVMOD:
        case O_EQ:
        case O_LT:
        case O_ULT:
        case O_PUSHR:
        case O_POPR:
        case O_DUPR:

        case O_WORD_BYTES:
        case O_GET_M0:
        case O_GET_MSIZE:
        case O_GET_RSIZE:
        case O_GET_RP:
        case O_SET_RP:
        case O_GET_SSIZE:
        case O_GET_SP:
        case O_SET_SP:
        case O_GET_HANDLER_RP:

        case OX_ARGC:
        case OX_ARGLEN:
        case OX_ARGCOPY:
        case OX_STDIN:
        case OX_STDOUT:
        case OX_STDERR:
        case OX_OPEN_FILE:
        case OX_CLOSE_FILE:
        case OX_READ_FILE:
        case OX_WRITE_FILE:
        case OX_FILE_POSITION:
        case OX_REPOSITION_FILE:
        case OX_FLUSH_FILE:
        case OX_RENAME_FILE:
        case OX_DELETE_FILE:
        case OX_FILE_SIZE:
        case OX_RESIZE_FILE:
        case OX_FILE_STATUS:
            return PC + 1;
        case O_JUMP:
        case O_CALL:
        case O_CATCH:
            if (SP < 1)
                return NULL;
            return (WORD *)(S0[SP - 1]);
        case O_JUMPZ:
            if (SP < 2)
                return NULL;
            return S0[SP - 2] == 0 ? (WORD *)S0[SP - 1] : PC + 1;
        case O_RET:
            if (RP < 1)
                return NULL;
            return (WORD *)(R0[RP - 1] & ~1);
        case O_THROW:
            if (HANDLER_RP == (UWORD)-1 || HANDLER_RP < 2)
                return NULL;
            return (WORD *)(R0[HANDLER_RP - 1] & ~1);
        case O_BREAK:
        default:
            return NULL;
        }
        break;
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
    int next_PC_valid = IS_ALIGNED(next_PC) && IS_VALID(next_PC);
    if (next_PC_valid) {
        assert(load_word(next_PC, &next_inst) == ERROR_OK);
        assert(store_word(next_PC, (O_BREAK << 2) | OP_INSTRUCTION) == ERROR_OK);
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
    if ((error != ERROR_BREAK || inst == ((O_THROW << 2) | OP_INSTRUCTION)) &&
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
