// Test the memory operators. Also uses previously tested instructions.
// See errors.c for address error handling tests.
//
// (c) Reuben Thomas 1994-2020
//
// The package is distributed under the GNU Public License version 3, or,
// at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#include "tests.h"


int main(void)
{
    const char *correct[64];
    unsigned steps = 0;

    size_t size = 4096;
    init_defaults((WORD *)calloc(size, WORD_BYTES), size);

    ass_goto(M0);
    pushrel(M0 + MSIZE / WORD_BYTES);
    correct[steps++] = xasprintf("%"PRIi32, m0 + memory);
    ass(O_WORD_BYTES);
    correct[steps++] = xasprintf("%"PRIi32" %d", m0 + memory, WORD_BYTES);
    ass(O_NEGATE);
    correct[steps++] = xasprintf("%"PRIi32" %d", m0 + memory, -WORD_BYTES);
    ass(O_ADD);
    correct[steps++] = xasprintf("%"PRIi32, m0 + memory - WORD_BYTES);
    push(513);
    correct[steps++] = xasprintf("%"PRIi32" %d", m0 + memory - WORD_BYTES, 513);
    push(1);
    correct[steps++] = xasprintf("%"PRIi32" %d %d", m0 + memory - WORD_BYTES, 513, 1);
    ass(O_DUP);
    correct[steps++] = xasprintf("%"PRIi32" %d %"PRIi32, m0 + memory - WORD_BYTES, 513, m0 + memory - WORD_BYTES);
    ass(O_STORE);
    correct[steps++] = xasprintf("%"PRIi32, m0 + memory - WORD_BYTES);
    push(0);
    correct[steps++] = xasprintf("%"PRIi32" %d", m0 + memory - WORD_BYTES, 0);
    ass(O_DUP);
    correct[steps++] = xasprintf("%"PRIi32" %"PRIi32, m0 + memory - WORD_BYTES, m0 + memory - WORD_BYTES);
    ass(O_LOAD);
    correct[steps++] = xasprintf("%"PRIi32" %d", m0 + memory - WORD_BYTES, 513);
    ass(O_POP);
    correct[steps++] = xasprintf("%"PRIi32, m0 + memory - WORD_BYTES);
    push(0);
    correct[steps++] = xasprintf("%"PRIi32" %d", m0 + memory - WORD_BYTES, 0);
    ass(O_DUP);
    correct[steps++] = xasprintf("%"PRIi32" %"PRIi32, m0 + memory - WORD_BYTES, m0 + memory - WORD_BYTES);
    ass(O_LOAD1);
    correct[steps++] = xasprintf("%"PRIi32" %d", m0 + memory - WORD_BYTES, 1);
    ass(O_ADD);
    correct[steps++] = xasprintf("%"PRIi32, m0 + memory - WORD_BYTES + 1);
    ass(O_LOAD1);
    correct[steps++] = xasprintf("%d", 2);
    pushrel(M0 + memory / WORD_BYTES);
    correct[steps++] = xasprintf("%d %"PRIi32, 2, m0 + memory);
    push(-1);
    correct[steps++] = xasprintf("%d %"PRIi32" %d", 2, m0 + memory, -1);
    ass(O_ADD);
    correct[steps++] = xasprintf("%d %"PRIi32, 2, m0 + memory - 1);
    ass(O_STORE1);
    correct[steps++] = xasprintf("%s", "");
    pushrel(M0 + memory / WORD_BYTES);
    correct[steps++] = xasprintf("%"PRIi32, m0 + memory);
    push(-4);
    correct[steps++] = xasprintf("%"PRIi32" %d", m0 + memory, -4);
    ass(O_ADD);
    correct[steps++] = xasprintf("%"PRIi32, m0 + memory - 4);
    ass(O_LOAD4);
    correct[steps++] = xasprintf("%"PRIi32, 33554945);
    ass(O_POP);
    correct[steps++] = xasprintf("%s", "");
    pushrel(M0 + memory / WORD_BYTES);
    correct[steps++] = xasprintf("%"PRIi32, m0 + memory);
    push(-4);
    correct[steps++] = xasprintf("%"PRIi32" %d", m0 + memory, -4);
    ass(O_ADD);
    correct[steps++] = xasprintf("%"PRIi32, m0 + memory - 4);
    ass(O_LOAD2);
    correct[steps++] = xasprintf("%d", 513);
    pushrel(M0 + memory / WORD_BYTES);
    correct[steps++] = xasprintf("%d %"PRIi32, 513, m0 + memory);
    push(-2);
    correct[steps++] = xasprintf("%d %"PRIi32" %d", 513, m0 + memory, -2);
    ass(O_ADD);
    correct[steps++] = xasprintf("%d %"PRIi32, 513, m0 + memory - 2);
    ass(O_STORE2);
    correct[steps++] = xasprintf("%s", "");
    pushrel(M0 + memory / WORD_BYTES);
    correct[steps++] = xasprintf("%"PRIi32, m0 + memory);
    push(-4);
    correct[steps++] = xasprintf("%"PRIi32" %d", m0 + memory, -4);
    ass(O_ADD);
    correct[steps++] = xasprintf("%"PRIi32, m0 + memory - 4);
    ass(O_LOAD);
    correct[steps++] = xasprintf("%"PRIi32, 33620481);
    ass(O_POP);
    correct[steps++] = xasprintf("%s", "");
    ass(O_GET_SP);
    correct[steps++] = xasprintf("%d", 0);
    ass(O_SET_SP);
    correct[steps++] = xasprintf("%s", "");
    ass(O_GET_RP);
    correct[steps++] = xasprintf("%d", 0);
    ass(O_POP);
    correct[steps++] = xasprintf("%s", "");
    push(0);
    correct[steps++] = xasprintf("%d", 0);
    ass(O_SET_RP);
    correct[steps++] = xasprintf("%s", "");
    ass(O_GET_RP);
    correct[steps++] = xasprintf("%d", 0);

    for (size_t i = 0; i < steps; i++) {
        WORD temp = 0;
        assert(load_word(PC, &temp) == ERROR_OK);
        printf("Instruction = %s\n", disass(temp, PC));
        assert(single_step() == ERROR_BREAK);
        show_data_stack();
        printf("Correct stack: %s\n\n", correct[i]);
        if (strcmp(correct[i], val_data_stack())) {
            printf("Error in memory tests: PC = %p\n", PC);
            exit(1);
        }
    }

    printf("Memory tests ran OK\n");
    return 0;
}
