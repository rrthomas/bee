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

#ifndef bee_BEE_DEBUG
#define bee_BEE_DEBUG


int bee_byte_size(bee_WORD v); // return number of significant bytes in a bee_WORD quantity

void bee_align(void);		// align assembly pointer to next word
void bee_ass(bee_UWORD instr);	// assemble an instruction
void bee_ass_byte(bee_BYTE b);	// assemble a byte
void bee_call(bee_WORD addr);	// assemble a call
void bee_push(bee_WORD literal);	// assemble a literal
void bee_pushrel(bee_UWORD addr);	// assemble an offset to the given address
void bee_ass_goto(bee_UWORD addr);	// start assembly, initialising variables
bee_UWORD bee_label(void);	// return address of bee_WORD currently being assembled to
const char *bee_disass(bee_WORD opcode, bee_UWORD addr);  // disassemble an instruction
bee_BYTE bee_toass(const char *token);    // convert a instruction to its opcode

char *bee_val_data_stack(void); // return the current data stack as a string
void bee_show_data_stack(void); // show the current contents of the data stack
void bee_show_return_stack(void);	// show the current contents of the return stack
const char *bee_error_to_msg(int code);	// translate error code to message


#endif
