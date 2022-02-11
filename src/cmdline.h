// Command-line help.
//
// Copyright (c) 2009-2020 Reuben Thomas
//
// The package is distributed under the GNU General Public License version 3,
// or, at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

// D: documentation line
// O: Option
// A: Action
//
// D(text)
// O(long name, short name ('\0' for none), argument, argument docstring, docstring)
// A(argument, docstring)

#define xstr(s) #s
#define str(s) xstr(s)

#define MEMORY_MESSAGE(type, max, def) \
  "set " type " size to the given NUMBER of words\n"                    \
  "                            0 < NUMBER <= " str(max) " [default " str(def) "]"
OPT("memory", 'm', required_argument, "NUMBER", MEMORY_MESSAGE("memory", MAX_MEMORY, DEFAULT_MEMORY))
OPT("stack", 's', required_argument, "NUMBER", MEMORY_MESSAGE("data stack", MAX_MEMORY, BEE_DEFAULT_STACK_SIZE))
OPT("return-stack", 'r', required_argument, "NUMBER", MEMORY_MESSAGE("return stack", MAX_MEMORY, BEE_DEFAULT_STACK_SIZE))
OPT("gdb", '\0', optional_argument, "IN,OUT", "start as remote target for GDB; use file descriptors\n"
  "                            IN and OUT [default stdin and stdout]")
OPT("help", '\0', no_argument, "", "display this help message and exit")
OPT("version", '\0', no_argument, "", "display version information and exit")
ARG("OBJECT-FILE", "load and run object OBJECT-FILE")
DOC("")
DOC("The ARGUMENTs are available to Bee.")
DOC("")
DOC("Report bugs to " PACKAGE_BUGREPORT ".")
