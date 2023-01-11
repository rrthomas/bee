// VM debugging functions
// These are undocumented and subject to change.
//
// (c) Reuben Thomas 1994-2023
//
// The package is distributed under the GNU General Public License version 3,
// or, at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#include <stdbool.h>

int byte_size(bee_word_t v); // return number of significant bytes in a bee_word_t quantity

void align(void);		// align assembly pointer to next word
void word(bee_word_t value);	// assemble the given word
void ass(bee_uword_t instr);	// assemble an instruction
void ass_trap(bee_uword_t code);	// assemble a trap
void ass_byte(uint8_t b);	// assemble a byte
void calli(bee_word_t *addr);	// assemble an immediate call
void pushi(bee_word_t literal);	// assemble an immediate literal
void push(bee_word_t literal);	// assemble a full-word literal
void pushreli(bee_word_t *addr);	// assemble an offset to the given word-aligned address
void jumpi(bee_word_t *addr);	// assemble an immediate jump
void jumpzi(bee_word_t *addr);	// assemble an immediate jumpz
void ass_goto(bee_word_t *addr);	// start assembly at the given word-aligned address
bee_word_t *label(void);	// return address of bee_word_t currently being assembled to
const char *disass(bee_word_t opcode, bee_word_t *addr);  // disassemble an instruction
uint8_t toass(const char *token);    // convert a instruction to its opcode

char *val_data_stack(bee_state * restrict S); // return the current data stack as a string
void show_data_stack(bee_state * restrict S); // show the current contents of the data stack
void show_return_stack(bee_state * restrict S);	// show the current contents of the return stack
const char *error_to_msg(int code);	// translate error code to message
bee_word_t single_step(bee_state * restrict S); // single step
bee_state *init_defaults(bee_word_t *pc); // initialize with stacks of size BEE_DEFAULT_STACK_SIZE
bool run_test(const char *name, bee_state *S, char *correct[], size_t steps, bool errors_allowed); // run a test, checking the results after each instruction
