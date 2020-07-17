/****************************************************************************

                THIS SOFTWARE IS NOT COPYRIGHTED

   HP offers the following for use in the public domain.  HP makes no
   warranty with regard to the software or it's performance and the
   user accepts the software "AS IS" with all faults.

   HP DISCLAIMS ANY WARRANTIES, EXPRESS OR IMPLIED, WITH REGARD
   TO THIS SOFTWARE INCLUDING BUT NOT LIMITED TO THE WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.

****************************************************************************/

/****************************************************************************
 *  Based on sparc-stub.c from gdb sources.
 *
 *************
 *
 *  The following gdb commands are supported; see GDB manual section "Remote
 *  Protocol" for more details.
 *
 * command          function                               Return value
 *
 *    d             toggle debug messages                  None
 *    g             return the value of the CPU registers  hex data or ENN
 *    G             set the value of the CPU registers     OK or ENN
 *
 *    mAA..AA,LLLL  Read LLLL bytes at address AA..AA      hex data or ENN
 *    MAA..AA,LLLL: Write LLLL bytes at address AA.AA      OK or ENN
 *
 *    c             Resume at current address              SNN   (signal NN)
 *    cAA..AA       Continue at address AA..AA             SNN
 *
 *    k             kill
 *
 *    ?             What was the last sigval ?             SNN   (signal NN)
 *
 ****************************************************************************/

#include "config.h"

#include "external_syms.h"

#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include "bee/bee.h"
#include "bee/aux.h"

#include "debug.h"
#include "gdb-stub.h"


/************************************************************************
 *
 * external low-level support routines
 */

FILE *gdb_in, *gdb_out;
#define putDebugChar(c) putc(c, gdb_out)	/* write a single character      */
#define getDebugChar() getc(gdb_in)	/* read and return a single char */

/************************************************************************/
/* If non-zero, print debugging information about remote communication. */
static int remote_debug = 0;

static const char hexchars[]="0123456789abcdef";

/* Register name enumeration to use as array indices.  */
enum regnames {
#define R(reg, type) reg##_idx,
#include "registers.h"
#undef R
};

/* BUFMAX defines the maximum number of characters in inbound/outbound buffers. */
/* At least (number of registers) * bee_WORD_BYTES * 2 are needed for register packets. */
#define BUFMAX 2048

static _GL_ATTRIBUTE_FORMAT_PRINTF(1, 0) void debug(const char *format, ...)
{
  if (remote_debug)
    {
      va_list args;
      va_start(args, format);
      vfprintf(stderr, format, args);
      va_end(args);
    }
}

/* Convert ch from a hex digit to an int */

static int
hex (unsigned char ch)
{
  if (ch >= 'a' && ch <= 'f')
    return ch-'a'+10;
  if (ch >= '0' && ch <= '9')
    return ch-'0';
  if (ch >= 'A' && ch <= 'F')
    return ch-'A'+10;
  return -1;
}

static char remcomInBuffer[BUFMAX];
static char remcomOutBuffer[BUFMAX];

/* scan for the sequence $<data>#<checksum>     */

static char *
getpacket (void)
{
  char *buffer = &remcomInBuffer[0];
  unsigned char checksum;
  unsigned char xmitcsum;
  int count;
  char ch;

  while (1)
    {
      /* wait around for the start character, ignore all other characters */
      while ((ch = getDebugChar ()) != '$')
        ;

retry:
      checksum = 0;
      xmitcsum = -1;
      count = 0;

      /* now, read until a # or end of buffer is found */
      while (count < BUFMAX - 1)
        {
          ch = getDebugChar ();
          if (ch == '$')
            goto retry;
          if (ch == '#')
            break;
          checksum = checksum + ch;
          buffer[count] = (char)ch;
          count = count + 1;
        }
      buffer[count] = 0;

      if (ch == '#')
        {
          ch = getDebugChar ();
          xmitcsum = hex (ch) << 4;
          ch = getDebugChar ();
          xmitcsum += hex (ch);

          if (checksum != xmitcsum)
            {
              putDebugChar ('-');	/* failed checksum */
            }
          else
            {
              putDebugChar ('+');	/* successful transfer */
              debug("getpacket: %s\n", &buffer[0]);
              return &buffer[0];
            }
        }
    }
}

/* send the packet in buffer.  */

static void
putpacket (char *buffer)
{
  unsigned char checksum;
  int count;
  unsigned char ch, ack_char;

  debug("putpacket: %s\n", buffer);
  /*  $<packet info>#<checksum>. */
  do
    {
      putDebugChar('$');
      checksum = 0;
      count = 0;

      while ((ch = buffer[count]))
        {
          putDebugChar(ch);
          checksum += ch;
          count += 1;
        }

      putDebugChar('#');
      putDebugChar(hexchars[checksum >> 4]);
      putDebugChar(hexchars[checksum & 0xf]);

      ack_char = getDebugChar();
    }
  while (ack_char != '+');
}

/* Convert the memory pointed to by mem into hex, placing result in buf.
 * Return a pointer to the last char put in buf (NUL).
 */

static char *
mem2hex (uint8_t *mem, char *buf, int count)
{
  while (count-- > 0)
    {
      unsigned ch = *mem++;
      *buf++ = hexchars[ch >> 4];
      *buf++ = hexchars[ch & 0xf];
    }

  *buf = 0;

  return buf;
}

/* Convert the hex array pointed to by buf into binary to be placed in mem.
 * Return a pointer to the character AFTER the last byte written. */

static uint8_t *
hex2mem (char *buf, uint8_t *mem, int count)
{
  for (int i = 0; i < count; i++)
    {
      unsigned char ch = hex(*buf++) << 4;
      ch |= hex(*buf++);
      *mem++ = ch;
    }

  return mem;
}

/* This table contains the mapping between Bee error codes and signals,
   which are primarily what GDB understands. */

static struct error_info
{
  int error;		/* Bee error code */
  unsigned char signo;		/* Signal that we map this error into */
} error_info[] = {
  {bee_ERROR_INVALID_OPCODE, SIGILL},
  {bee_ERROR_STACK_UNDERFLOW, SIGSEGV},
  {bee_ERROR_STACK_OVERFLOW, SIGSEGV},
  {bee_ERROR_INVALID_LOAD, SIGSEGV},
  {bee_ERROR_INVALID_STORE, SIGSEGV},
  {bee_ERROR_UNALIGNED_ADDRESS, SIGBUS},
  {bee_ERROR_BREAK, SIGTRAP},
  {0, 0}			/* Must be last */
};

/* Convert the Bee error code to a UNIX signal number. */

static int
_GL_ATTRIBUTE_CONST computeSignal (int error)
{
  for (struct error_info *ei = error_info; ei->error && ei->signo; ei++)
    if (ei->error == error)
      return ei->signo;

  return SIGHUP;		/* default for things we don't know about */
}

/*
 * Parse all hex digits starting at *ptr, and update *ptr.
 * Keep reading characters even if it overflows.
 * Return flag indicating whether a number was successfully parsed.
 */

static unsigned
hexToInt(char **ptr, bee_UWORD *intValue)
{
  errno = 0;
  char *start = *ptr;
  *intValue = (bee_UWORD)strtoul(*ptr, ptr, 16);
  return errno == 0 && start != *ptr;
}

/*
 * Determine whether the given address range is a valid address in Bee's
 * memory or a stack.
 */

static int
valid_memory_or_stack_range(uint8_t *addr, bee_UWORD length)
{
  uint8_t *REND = (uint8_t *)bee_R0 + (RSIZE * bee_WORD_BYTES);
  uint8_t *SEND = (uint8_t *)bee_S0 + (SSIZE * bee_WORD_BYTES);
  return address_range_valid(addr, length) ||
    (addr >= (uint8_t *)bee_R0 && addr <= REND && length <= (bee_UWORD)(REND - addr)) ||
    (addr >= (uint8_t *)bee_S0 && addr <= SEND && length <= (bee_UWORD)(SEND - addr));
}

/*
 * This function does all command processing for interfacing to gdb.
 * Returns 1 for 'continue' and 0 for 'exit'.
 */

int
handle_exception (int error)
{
  int sigval;
  bee_UWORD addr;
  bee_UWORD length = 0;

  /* reply to host that an exception has occurred */
  debug("handle_exception: %d\n", error);
  sigval = computeSignal(error);

  /* Send 'T' stop reply packet (summary of state).  */
  char *ptr = remcomOutBuffer;
  *ptr++ = 'T';
  *ptr++ = hexchars[sigval >> 4];
  *ptr++ = hexchars[sigval & 0xf];
#define R(reg, type)                                            \
  *ptr++ = hexchars[reg##_idx >> 4];                            \
  *ptr++ = hexchars[reg##_idx & 0xf];                           \
  *ptr++ = ':';                                                 \
  ptr = mem2hex((uint8_t *)&bee_##reg, ptr, bee_WORD_BYTES);    \
  *ptr++ = ';';
#include "registers.h"
#undef R
  *ptr++ = 0;
  putpacket(remcomOutBuffer);

  /* Command loop.  */
  while (1)
    {
      char *out_ptr = remcomOutBuffer;
      *out_ptr = 0;

      char *in_ptr = getpacket();
      switch (*in_ptr++)
        {
        case '?':
          *out_ptr++ = 'S';
          *out_ptr++ = hexchars[sigval >> 4];
          *out_ptr++ = hexchars[sigval & 0xf];
          *out_ptr++ = 0;
          break;

        case 'd':		/* toggle debug flag */
          remote_debug = !remote_debug;
          break;

        case 'g':		/* return the value of the CPU registers */
          {
#define R(reg, type)                                                    \
            out_ptr = mem2hex((uint8_t *)&bee_##reg, out_ptr, bee_WORD_BYTES);
#include "registers.h"
#undef R
          }
          break;

        case 'G':	   /* set the value of the CPU registers - return OK */
          {
#define R(reg, type)                                                    \
            hex2mem(in_ptr, (uint8_t *)&bee_##reg, bee_WORD_BYTES);     \
            in_ptr += bee_WORD_BYTES * 2; /* Advance over WORD_BYTES*2 hex characters. */
#include "registers.h"
#undef R
            strcpy(out_ptr, "OK");
          }
          break;

        case 'm':	  /* mAA..AA,LLLL  Read LLLL bytes at address AA..AA */
          /* Try to read %x,%x.  */

          if (hexToInt(&in_ptr, &addr)
              && *in_ptr++ == ','
              && hexToInt(&in_ptr, &length))
            {
              uint8_t *mem = (uint8_t *)addr;
              if (valid_memory_or_stack_range(mem, length))
                mem2hex(mem, out_ptr, length);
              else
                strcpy(out_ptr, "E03");
            }
          else
            strcpy(out_ptr, "E01");
          break;

        case 'M': /* MAA..AA,LLLL: Write LLLL bytes at address AA.AA return OK */
          /* Try to read '%x,%x:'.  */

          if (hexToInt(&in_ptr, &addr)
              && *in_ptr++ == ','
              && hexToInt(&in_ptr, &length)
              && *in_ptr++ == ':')
            {
              uint8_t *mem = (uint8_t *)addr;
              if (valid_memory_or_stack_range(mem, length))
                {
                  hex2mem(in_ptr, mem, length);
                  strcpy(out_ptr, "OK");
                }
              else
                strcpy(out_ptr, "E03");
            }
          else
            strcpy(out_ptr, "E02");
          break;

        case 'c':    /* cAA..AA    Continue at address AA..AA(optional) */
          /* try to read optional parameter, pc unchanged if no parm */
          if (hexToInt(&in_ptr, &addr))
            bee_PC = (bee_WORD *)addr;
          return 1;

        case 'k':    /* kill the program */
          return 0;
        }

      /* reply to the request */
      putpacket(remcomOutBuffer);
    }
}

/*
 * Initialize: set up the streams to communicate with gdb.
 */
int gdb_init(int in, int out)
{
  gdb_in = fdopen(in, "rb");
  if (gdb_in == NULL)
    return -1;
  gdb_out = fdopen(out, "wb");
  if (gdb_out == NULL) {
    fclose(gdb_in);
    return -1;
  }

  setbuf(gdb_in, NULL);
  setbuf(gdb_out, NULL);
  return 0;
}
