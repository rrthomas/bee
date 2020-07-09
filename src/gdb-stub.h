// GDB stub API declarations.
//
// Copied from GDB documentation by Reuben Thomas 2020.
// This file is in the public domain.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

int gdb_init(int gdb_fdin, int gdb_fdout);
int handle_exception (int error);
