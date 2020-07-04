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


static UWORD current; // where we assemble the next instruction word or literal


void ass(UWORD instr)
{
    current = ALIGN(current);
    store_word(current, instr << 2 | OP_INSTRUCTION);
    current += WORD_BYTES;
}

void ass_byte(uint8_t b)
{
    store_byte(current++, b);
}

void push(WORD literal)
{
    current = ALIGN(current);
    WORD temp = literal << 2;
    ARSHIFT(temp, 2);
    assert(temp == literal);
    store_word(current, literal << 2 | OP_PUSH);
    current += WORD_BYTES;
}

static void addr_op(int op, WORD addr)
{
    current = ALIGN(current);
    assert(IS_ALIGNED(addr));
    store_word(current, (addr - current) | op);
    current += WORD_BYTES;
}

void call(WORD addr)
{
    addr_op(OP_CALL, addr);
}

void pushrel(UWORD addr)
{
    addr_op(OP_PUSHREL, addr);
}

void ass_goto(UWORD addr)
{
    current = addr;
}

_GL_ATTRIBUTE_PURE UWORD label(void)
{
    return current;
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
    "GET_SP", "SET_SP", "GET_RP", "SET_RP", "GET_MEMORY", "WORD_BYTES", NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
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

_GL_ATTRIBUTE_CONST const char *disass(WORD opcode, UWORD addr)
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
            text = xasprintf("PUSH %"PRIi32"=$%"PRIX32, opcode, (UWORD)opcode);
        }
        break;
    case OP_PUSHREL:
        text = xasprintf("PUSHREL $%"PRIX32, (addr + (opcode & ~OP_MASK)));
        break;
    case OP_INSTRUCTION:
        opcode >>= 2;
        if ((UWORD)opcode <= sizeof(mnemonic) / sizeof(mnemonic[0]) &&
            mnemonic[(UWORD)opcode] != NULL)
            text = xasprintf("%s", mnemonic[(UWORD)opcode]);
        else
            text = strdup("");
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
    { -5, "BEE_ERROR_INVALID_LOAD" },
    { -6, "BEE_ERROR_INVALID_STORE" },
    { -7, "BEE_ERROR_UNALIGNED_ADDRESS" },
    { -8, "BEE_ERROR_DIVISION_BY_ZERO" },
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
// which may not be valid, or some invalid value if the new value of PC
// cannot be computed.
static UWORD compute_next_PC(WORD inst)
{
    const UWORD next_PC_error = 1;
    switch (inst & OP_MASK) {
    case OP_CALL:
        return PC + inst;
    case OP_PUSH:
    case OP_PUSHREL:
        return PC + WORD_BYTES;
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
        case O_GET_SP:
        case O_SET_SP:
        case O_GET_RP:
        case O_SET_RP:
        case O_GET_MEMORY:
        case O_WORD_BYTES:
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
            return PC + WORD_BYTES;
        case O_JUMP:
            {
                if (SP < 1)
                    return next_PC_error;
                return S0[SP - 1];
            }
            break;
        case O_JUMPZ:
            {
                if (SP < 2)
                    return next_PC_error;
                return S0[SP - 2] == 0 ? (UWORD)S0[SP - 1] : PC + WORD_BYTES;
            }
            break;
        case O_CALL:
            {
                if (SP < 1)
                    return next_PC_error;
                return S0[SP - 1];
            }
            break;
        case O_RET:
            {
                if (RP < 1)
                    return next_PC_error;
                return R0[RP - 1] & ~1;
            }
            break;
        case O_CATCH:
            {
                if (SP < 1)
                    return next_PC_error;
                return S0[SP - 1];
            }
            break;
        case O_THROW:
            {
                if (HANDLER_RP == (UWORD)-1)
                    return next_PC_error;
                return R0[HANDLER_RP - 1] & ~1;
            }
            break;

        case O_BREAK:
        default:
            return next_PC_error;
        }
        break;
    default:
        return next_PC_error;
    }
}

WORD single_step(void)
{
    WORD temp = 0;
    int error = 0;
    WORD inst = LOAD_WORD(PC), next_inst;
    if (error != ERROR_OK)
        return error;
    UWORD next_PC = compute_next_PC(inst);
    int next_PC_valid = IS_ALIGNED(next_PC) && IS_VALID(next_PC);
    if (next_PC_valid) {
        next_inst = LOAD_WORD(next_PC);
        assert(error == ERROR_OK);
        STORE_WORD(next_PC, (O_BREAK << 2) | OP_INSTRUCTION);
        assert(error == ERROR_OK);
    }
    WORD ret = run();
    if (next_PC_valid) {
        STORE_WORD(next_PC, next_inst);
        assert(error == ERROR_OK);
        if (ret == ERROR_BREAK)
            PC = next_PC;
    }
    return ret;
}
