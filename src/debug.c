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

void lit(CELL literal)
{
    current = ALIGN(current);
    CELL temp = literal << 2;
    ARSHIFT(temp, 2);
    assert(temp == literal);
    store_cell(current, literal << 2 | OP_LITERAL);
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

void offset(UCELL addr)
{
    addr_op(OP_OFFSET, addr);
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
    "DROP", "PICK", "ROLL", ">R", "R>", "R@", "S0@", "S0!",
    "SP@", "SP!", "R0@", "R0!", "RP@", "RP!", "MEMORY@", "CELL",
    "@", "!", "C@", "C!", "+", "NEGATE", "*", "U/MOD",
    "S/REM", "=", "<", "U<", "INVERT", "AND", "OR", "XOR",
    "LSHIFT", "RSHIFT", "EXIT", "EXECUTE", "HALT", "BRANCH", "?BRANCH", NULL,
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
    case OP_LITERAL:
        {
            ARSHIFT(opcode, 2);
            text = xasprintf("LITERAL %"PRIi32"=$%"PRIX32, opcode, (UCELL)opcode);
        }
        break;
    case OP_OFFSET:
        text = xasprintf("OFFSET $%"PRIX32, (addr + (opcode & ~OP_MASK)));
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
    if (!STACK_UNDERFLOW(SP, S0))
        for (UCELL i = S0; i != SP;) {
            CELL c;
            char *ptr;
            i += CELL_W * STACK_DIRECTION;
            int exception = load_cell(i, &c);
            if (exception != 0) {
                ptr = xasprintf("%sinvalid address!", picture);
                free(picture);
                picture = ptr;
                break;
            }
            ptr = xasprintf("%s%"PRId32, picture, c);
            free(picture);
            picture = ptr;
            if (with_hex) {
                ptr = xasprintf("%s ($%"PRIX32") ", picture, (UCELL)c);
                free(picture);
                picture = ptr;
            }
            if (i != SP) {
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
    if (SP == S0)
        printf("Data stack empty\n");
    else if (STACK_UNDERFLOW(SP, S0))
        printf("Data stack underflow\n");
    else
        printf("Data stack: %s\n", _val_data_stack(true));
}

void show_return_stack(void)
{
    if (RP == R0)
        printf("Return stack empty\n");
    else if (STACK_UNDERFLOW(RP, R0))
        printf("Return stack underflow\n");
    else {
        printf("Return stack: ");
        for (UCELL i = R0; i != RP;) {
            CELL c;
            i += CELL_W * STACK_DIRECTION;
            int exception = load_cell(i, &c);
            if (exception != 0) {
                printf("invalid address!\n");
                break;
            }
            printf("$%"PRIX32" ", (UCELL)c);
        }
        putchar('\n');
    }
}
