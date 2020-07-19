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

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include "bee/bee.h"

#include "private.h"
#include "debug.h"
#include "gdb-stub.h"


/************************************************************************
 *
 * external low-level support routines
 */

FILE *gdb_in, *gdb_out;

/************************************************************************/
/* If non-zero, print debugging information about remote communication. */
static int remote_debug = 0;

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

static char remcomInBuffer[BUFMAX];
static char remcomOutBuffer[BUFMAX];

/* scan for the sequence $<data>#<checksum>     */

static char *
getpacket (void)
{
  char *buffer = &remcomInBuffer[0];
  unsigned char checksum, xmitcsum;
  int count;
  char ch;

  while (1)
    {
      /* wait around for the start character, ignore all other characters */
      while ((ch = getc (gdb_in)) != '$')
        ;

retry:
      checksum = 0;
      xmitcsum = -1;
      count = 0;

      /* now, read until a # or end of buffer is found */
      while (count < BUFMAX - 1)
        {
          ch = getc (gdb_in);
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
          char buf[2];
          fread(buf, 2, 1, gdb_in);
          /* fscanf can read() more than 2 chars, which causes confusion. */
          sscanf(buf, "%2hhx", &xmitcsum);
          if (checksum != xmitcsum)
            {
              putc ('-', gdb_out);	/* failed checksum */
            }
          else
            {
              putc ('+', gdb_out);	/* successful transfer */
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
  debug("putpacket: %s\n", buffer);
  /*  $<packet info>#<checksum>. */
  unsigned char checksum = 0, ch;
  for (int count = 0; (ch = buffer[count]) != '\0'; count++)
    checksum += ch;

  do
    fprintf(gdb_out, "$%s#%.2x", buffer, checksum);
  while (getc (gdb_in) != '+');
}

/* Convert the memory pointed to by mem into hex, placing result in buf.
 * Return a pointer to the last char put in buf (NUL).
 */

static char *
mem2hex (uint8_t *mem, char *buf, int count)
{
  while (count-- > 0)
    {
      sprintf(buf, "%.2x", *mem++);
      buf += 2;
    }

  return buf;
}

/* Convert the hex array pointed to by buf into binary to be placed in
   mem.  */

static const char *
hex2mem (const char *buf, uint8_t *mem, unsigned count)
{
  for (unsigned i = 0;
       i < count && sscanf(buf, "%2hhx", mem) == 1;
       i++, mem++, buf += 2)
    ;
  return buf;
}

/* This table contains the mapping between Bee error codes and signals,
   which are primarily what GDB understands. */

static struct error_info
{
  int error;		/* Bee error code */
  unsigned char signo;		/* Signal that we map this error into */
} error_info[] = {
  {BEE_ERROR_INVALID_OPCODE, SIGILL},
  {BEE_ERROR_STACK_UNDERFLOW, SIGSEGV},
  {BEE_ERROR_STACK_OVERFLOW, SIGSEGV},
  {BEE_ERROR_INVALID_LOAD, SIGSEGV},
  {BEE_ERROR_INVALID_STORE, SIGSEGV},
  {BEE_ERROR_UNALIGNED_ADDRESS, SIGBUS},
  {BEE_ERROR_BREAK, SIGTRAP},
  {0, 0}			/* Must be last */
};

/* Convert the Bee error code to a UNIX signal number. */

static unsigned
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

static int
hexToInt(const char **ptr, bee_UWORD *intValue)
{
  errno = 0;
  char *end;
  *intValue = (bee_UWORD)strtoul(*ptr, &end, 16);
  int ok = errno == 0 && *ptr != end;
  *ptr = end;
  return ok;
}

/*
 * This function does all command processing for interfacing to gdb.
 * Returns 1 for 'continue' and 0 for 'exit'.
 */

int
handle_exception (int error)
{
  bee_UWORD addr;
  bee_UWORD length = 0;

  /* reply to host that an exception has occurred */
  debug("handle_exception: %d\n", error);
  unsigned sigval = computeSignal(error);

  /* Send 'S' stop reply packet (signal).  */
  snprintf(remcomOutBuffer, BUFMAX, "S%.2x", sigval);
  putpacket(remcomOutBuffer);

  /* Command loop.  */
  while (1)
    {
      char *out_ptr = remcomOutBuffer;
      *out_ptr = '\0';

      const char *in_ptr = getpacket();
      switch (*in_ptr++)
        {
        /* The '?' packet need not be supported according to the GDB
           documentation, but GDB hangs if the stub does not implement
           it.  */
        case '?':
          snprintf(remcomOutBuffer, BUFMAX, "S%.2x", sigval);
          break;

        case 'd':		/* toggle debug flag */
          remote_debug = !remote_debug;
          break;

        case 'g':		/* return the value of the CPU registers */
#define R(reg, type)                                                    \
          out_ptr = mem2hex((uint8_t *)&reg, out_ptr, bee_WORD_BYTES);
#include "registers.h"
#undef R
          break;

        case 'G':	   /* set the value of the CPU registers - return OK */
#define R(reg, type)                                                    \
          in_ptr = hex2mem(in_ptr, (uint8_t *)&reg, bee_WORD_BYTES);
#include "registers.h"
#undef R
          strcpy(out_ptr, "OK");
          break;

        case 'm':	  /* mAA..AA,LLLL  Read LLLL bytes at address AA..AA */
          /* Try to read %x,%x.  */

          if (hexToInt(&in_ptr, &addr)
              && *in_ptr++ == ','
              && hexToInt(&in_ptr, &length))
            {
              uint8_t *mem = (uint8_t *)addr;
              mem2hex(mem, out_ptr, length);
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
              hex2mem(in_ptr, (uint8_t *)addr, length);
              strcpy(out_ptr, "OK");
            }
          else
            strcpy(out_ptr, "E02");
          break;

        case 'c':    /* cAA..AA    Continue at address AA..AA(optional) */
          /* try to read optional parameter, pc unchanged if no parm */
          if (hexToInt(&in_ptr, &addr))
            bee_pc = (bee_WORD *)addr;
          return 1;

        case 'k':    /* kill the program */
          return 0;

        default:     /* unknown command; reply with empty packet */
          break;
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
