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


static UCELL current; // where we assemble the next instruction word or literal


void ass(UCELL instr)
{
    current = ALIGN(current);
    store_cell(current, instr << 2 | OP_INSTRUCTION);
    current += CELL_W;
}

void ass_byte(BYTE b)
{
    store_byte(current++, b);
}

void push(CELL literal)
{
    current = ALIGN(current);
    CELL temp = literal << 2;
    ARSHIFT(temp, 2);
    assert(temp == literal);
    store_cell(current, literal << 2 | OP_PUSH);
    current += CELL_W;
}

static void addr_op(int op, CELL addr)
{
    current = ALIGN(current);
    assert(IS_ALIGNED(addr));
    store_cell(current, (addr - current) | op);
    current += CELL_W;
}

void call(CELL addr)
{
    addr_op(OP_CALL, addr);
}

void pushrel(UCELL addr)
{
    addr_op(OP_PUSHREL, addr);
}

void start_ass(UCELL addr)
{
    current = addr;
}

_GL_ATTRIBUTE_PURE UCELL ass_current(void)
{
    return current;
}

static const char *mnemonic[UINT8_MAX + 1] = {
    "POP", "DUP", "ROLL", "PUSHR", "POPR", "DUPR", "GET_SP", "SET_SP",
    "GET_RP", "SET_RP", "GET_MEMORY", "WORD_BYTES", "LOAD", "STORE", "LOAD1", "STORE1",
    "ADD", "NEGATE", "MUL", "UDIVMOD", "DIVMOD", "EQ", "LT", "ULT",
    "NOT", "AND", "OR", "XOR", "LSHIFT", "RSHIFT", "RET", "CALL",
    "HALT", "JUMP", "JUMPZ", NULL, NULL, NULL, NULL, NULL,
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
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
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

_GL_ATTRIBUTE_CONST const char *disass(CELL opcode, UCELL addr)
{
    static char *text = NULL;

    free(text);
    switch (opcode & OP_MASK) {
    case OP_CALL:
        text = xasprintf("CALL $%"PRIX32, (addr + (opcode & ~OP_MASK)));
        break;
    case OP_PUSH:
        {
            ARSHIFT(opcode, 2);
            text = xasprintf("PUSH %"PRIi32"=$%"PRIX32, opcode, (UCELL)opcode);
        }
        break;
    case OP_PUSHREL:
        text = xasprintf("PUSHREL $%"PRIX32, (addr + (opcode & ~OP_MASK)));
        break;
    case OP_INSTRUCTION:
        opcode >>= 2;
        if ((UCELL)opcode <= sizeof(mnemonic) / sizeof(mnemonic[0]) &&
            mnemonic[(UCELL)opcode] != NULL)
            text = xasprintf("%s", mnemonic[(UCELL)opcode]);
        else
            text = strdup("");
        break;
    }
    return text;
}

_GL_ATTRIBUTE_PURE BYTE toass(const char *token)
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
        for (UCELL i = 0; i < SP; i++) {
            char *ptr = xasprintf("%s%"PRId32, picture, S0[i]);
            free(picture);
            picture = ptr;
            if (with_hex) {
                ptr = xasprintf("%s ($%"PRIX32") ", picture, (UCELL)S0[i]);
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
        for (UCELL i = 0; i < RP; i++)
            printf("$%"PRIX32" ", (UCELL)R0[i]);
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
    { -5, "BEE_ERROR_INVALID_MEMORY_READ" },
    { -6, "BEE_ERROR_INVALID_MEMORY_WRITE" },
    { -7, "BEE_ERROR_UNALIGNED_ADDRESS" },
    { -8, "BEE_ERROR_DIVISION_BY_ZERO" },
    { -257, "BEE_ERROR_STEP" },
};

_GL_ATTRIBUTE_PURE const char *error_to_msg(int code)
{
    for (size_t i = 0; i < sizeof(error_msg) / sizeof(error_msg[0]); i++) {
        if (error_msg[i].code == code)
            return error_msg[i].msg;
    }
    return "unknown error";
}
