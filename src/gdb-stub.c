// GDB remote protocol stub.
// The following commands are supported: d, g, G, m, M, c, k and ?.
// See the GDB manual section "Remote Protocol" for more details.
//
// (c) Reuben Thomas 2020
//
// The package is distributed under the GNU Public License version 3, or,
// at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#include "config.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "bee/bee.h"

#include "private.h"


// Streams for communication with GDB
FILE *gdb_in, *gdb_out;

// The size of the inbound/outbound buffers
#define BUFFER_SIZE 2048

// If non-zero, print debugging information about remote communication
static int remote_debug = 0;


static _GL_ATTRIBUTE_FORMAT_PRINTF(1, 0) void debug(const char *format, ...)
{
    if (remote_debug) {
        va_list args;
        va_start(args, format);
        vfprintf(stderr, format, args);
        va_end(args);
    }
}


// Get a packet from `gdb_in`, of the form $<data>#<checksum>
static char *recv_packet(void)
{
    for (;;) {
        static char buffer[BUFFER_SIZE];

        // Slurp up to the next '$'
        char ch;
        while ((ch = getc(gdb_in)) != '$')
            ;

        // Read until a # or end of buffer is found; reset if we get a '$'
        unsigned char checksum = 0;
        size_t count;
        for (count = 0; count < BUFFER_SIZE - 1; ch = getc(gdb_in)) {
            if (ch == '$') {
                checksum = 0;
                count = 0;
            } else if (ch == '#')
                break;
            else {
                checksum += ch;
                buffer[count++] = ch;
            }
        }
        buffer[count] = '\0';

        if (ch == '#') {
            unsigned short xmitcsum = -1;
            char buf[2];
            // fscanf can read() more than 2 chars, which causes confusion
            if (fread(buf, 2, 1, gdb_in) == 1)
                sscanf(buf, "%2hx", &xmitcsum);
            if (checksum != xmitcsum)
                putc('-', gdb_out); // checksum failed
            else {
                putc('+', gdb_out); // checksum passed
                debug("recv_packet: %s\n", &buffer[0]);
                return buffer;
            }
        }
    }
}

// Send the given message, adding '$' prefix, '#' suffix and checksum
static void send_packet(char *buffer)
{
    debug("send_packet: %s\n", buffer);
    unsigned char checksum = 0, ch;
    for (int count = 0; (ch = buffer[count]) != '\0'; count++)
        checksum += ch;

    do
        fprintf(gdb_out, "$%s#%.2x", buffer, checksum);
    while (getc(gdb_in) != '+');
}


// Convert the memory pointed to by mem into hex, placing result in buf
// Return a pointer to the last char put in buf (NUL)
static char *mem_to_hex(uint8_t *mem, char *buf, int count)
{
    for (; count-- > 0; buf += 2)
        sprintf(buf, "%.2x", *mem++);
    return buf;
}

// Convert the hex array pointed to by `buf` into binary to be placed in
// `mem`
static const char *hex_to_mem(const char *buf, uint8_t *mem, unsigned count)
{
    for (unsigned i = 0;
         i < count && sscanf(buf, "%2hhx", mem) == 1;
         i++, mem++, buf += 2)
        ;
    return buf;
}

// Parse all hex digits starting at *ptr, and update *ptr
// Keep reading characters even if it overflows
// Return flag indicating whether a number was successfully parsed
static int hex_to_int(const char **ptr, bee_UWORD *intValue)
{
    errno = 0;
    char *end;
    *intValue = (bee_UWORD)strtoul(*ptr, &end, 16);
    int ok = errno == 0 && *ptr != end;
    *ptr = end;
    return ok;
}


// Mapping from Bee error codes to signals
static struct error_info
{
    int error;
    unsigned char signo;
} error_info[] =
    {
     {BEE_ERROR_INVALID_OPCODE, SIGILL},
     {BEE_ERROR_STACK_UNDERFLOW, SIGSEGV},
     {BEE_ERROR_STACK_OVERFLOW, SIGSEGV},
     {BEE_ERROR_INVALID_LOAD, SIGSEGV},
     {BEE_ERROR_INVALID_STORE, SIGSEGV},
     {BEE_ERROR_UNALIGNED_ADDRESS, SIGBUS},
     {BEE_ERROR_BREAK, SIGTRAP},
     {0, 0}, // GDB_SIGNAL_0, sent on initial connection
    };

// Convert the Bee error code to a UNIX signal number
static unsigned _GL_ATTRIBUTE_CONST error_to_signal(int error)
{
    for (size_t i = 0; i < sizeof(error_info) / sizeof(error_info[0]); i++)
        if (error_info[i].error == error)
            return error_info[i].signo;

    return SIGHUP; // default for things we don't know about
}


// Process GDB commands until told to continue or exit
// Returns 1 for 'continue' and 0 for 'exit'
static int new_connection = 0;
int handle_exception(int error)
{
    debug("handle_exception: %d\n", error);

    char buf[BUFFER_SIZE];

    // If this is not the first time we are called, tell host an exception
    // has occurred: send 'S' stop reply packet
    unsigned sigval = error_to_signal(error);
    if (new_connection)
        new_connection = 0;
    else {
        snprintf(buf, BUFFER_SIZE, "S%.2x", sigval);
        send_packet(buf);
    }

    // Main loop
    for (;;) {
        bee_UWORD addr, length = 0;
        char *out_ptr = buf;
        *out_ptr = '\0';

        const char *in_ptr = recv_packet();
        switch (*in_ptr++) {
            // The '?' packet need not be supported according to the GDB
            // documentation, but GDB hangs if the stub does not implement it
        case '?':
            sprintf(out_ptr, "S%.2x", sigval);
            break;

        case 'd': // toggle debug flag
            remote_debug = !remote_debug;
            break;

        case 'g': // return the value of the CPU registers
#define R(reg, type)                                                    \
            out_ptr = mem_to_hex((uint8_t *)&bee_##reg, out_ptr, bee_WORD_BYTES);
#include "bee/registers.h"
#undef R
            break;

        case 'G': // set the value of the CPU registers, return OK
#define R(reg, type)                                                    \
            in_ptr = hex_to_mem(in_ptr, (uint8_t *)&bee_##reg, bee_WORD_BYTES);
#include "bee/registers.h"
#undef R
            strcpy(out_ptr, "OK");
            break;

        case 'm': // mAA..AA,LLLL  Read LLLL bytes at address AA..AA
            // Try to read '%x,%x'
            if (hex_to_int(&in_ptr, &addr)
                && *in_ptr++ == ','
                && hex_to_int(&in_ptr, &length)
                && length <= BUFFER_SIZE / 2) {
                mem_to_hex((uint8_t *)addr, out_ptr, length);
            } else
                strcpy(out_ptr, "E01");
            break;

        case 'M': // MAA..AA,LLLL: Write LLLL bytes at address AA.AA, return OK
            // Try to read '%x,%x:'
            if (hex_to_int(&in_ptr, &addr)
                && *in_ptr++ == ','
                && hex_to_int(&in_ptr, &length)
                && *in_ptr++ == ':'
                && length <= BUFFER_SIZE / 2) {
                hex_to_mem(in_ptr, (uint8_t *)addr, length);
                strcpy(out_ptr, "OK");
            } else
                strcpy(out_ptr, "E02");
            break;

        case 'c': // cAA..AA    Continue at address AA..AA (optional)
            // If no parameter, pc is unchanged
            if (hex_to_int(&in_ptr, &addr))
                bee_pc = (bee_WORD *)addr;
            return 1;

        case 'k': // Kill the program
            return 0;

        default: // Unknown command; reply with empty packet
            break;
        }

        // Send the (possibly empty) reply
        send_packet(buf);
    }
}

// Initialize
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
    new_connection = 1;
    return 0;
}
