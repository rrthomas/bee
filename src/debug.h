// VM debugging functions
// These are undocumented and subject to change.
//
// (c) Reuben Thomas 1994-2020
//
// The package is distributed under the GNU Public License version 3, or,
// at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

int byte_size(bee_WORD v); // return number of significant bytes in a bee_WORD quantity

void align(void);		// align assembly pointer to next word
void word(bee_WORD value);	// assemble the given word
void ass(bee_UWORD instr);	// assemble an instruction
void ass_trap(bee_UWORD code);	// assemble a trap
void ass_byte(uint8_t b);	// assemble a byte
void calli(bee_WORD *addr);	// assemble an immediate call
void pushi(bee_WORD literal);	// assemble a literal
void pushreli(bee_WORD *addr);	// assemble an offset to the given word-aligned address
void jumpi(bee_WORD *addr);	// assemble an immediate jump
void jumpzi(bee_WORD *addr);	// assemble an immediate jumpz
void ass_goto(bee_WORD *addr);	// start assembly at the given word-aligned address
bee_WORD *label(void);	// return address of bee_WORD currently being assembled to
const char *disass(bee_WORD opcode, bee_WORD *addr);  // disassemble an instruction
uint8_t toass(const char *token);    // convert a instruction to its opcode

char *val_data_stack(void); // return the current data stack as a string
void show_data_stack(void); // show the current contents of the data stack
void show_return_stack(void);	// show the current contents of the return stack
const char *error_to_msg(int code);	// translate error code to message
bee_WORD single_step(void); // single step
