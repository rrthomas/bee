/* LIB.C

    Vrsn  Date   Comment
    ----|-------|---------------------------------------------------------------
    0.00 24mar95 Code removed from step.c v0.21. See that file for earlier
    	    	 history. Code made into a #included fragment rather than a
    	    	 function.
    0.01 02apr95 Added GETCH.
    0.02 03apr95 Added PUTCH and NEWL.
    0.03 18apr95 Added file-handling code.
    0.04 19apr95 Made OPEN-FILE call (4) work on big-endian Beetles.
    0.05 18jun96 Made into a function for ease of interworking with hand-coded
                 run and step implementations. Address checks disabled.
    0.06 01aug96 Moved code to read zero-terminated string from Beetle's memory
    		 to getstr. Added more file-handling code.
    0.07 30mar97 Changed lib's parameter to a UCELL. Added #include of lib.h.

    Reuben Thomas


    Beetle's standard library.

*/


#include <stdio.h>
#include "beetle.h" 	/* main header */
#include "opcodes.h"	/* opcode enumeration */
#include "lib.h"        /* the header we're implementing */


#define CHECKA(x)
#define PTRS 16

void getstr(unsigned char *s, UCELL adr)
{
    int i;

    for (i = 0; *(M0 + FLIP(adr)) != 0; adr++)
    	s[i++] = *(M0 + FLIP(adr));
    s[i] = '\0';
}

void lib(UCELL routine)
{
    static FILE *ptr[PTRS];
    static lastptr = 0;

    switch (routine) {

        /* BL */
        case 0: CHECKA(SP - 1); *--SP = 32; break;

        /* CR */
        case 1: NEWL; break;

        /* EMIT */
        case 2: CHECKA(SP); PUTCH((BYTE)*SP); SP++; break;

        /* KEY */
        case 3: CHECKA(SP - 1); *--SP = (CELL)(GETCH); break;

        /* OPEN-FILE */
        case 4:
        	{
        	    int p = (lastptr == PTRS ? -1 : lastptr++);
        	    unsigned char file[256], perm[4];

        	    if (p == -1) *SP = -1;
        	    else {
        	        getstr(file, *((UCELL *)SP + 1));
        	        getstr(perm, *(UCELL *)SP);
        	        ptr[p] = fopen((char *)file, (char *)perm);
        	        *SP = 0;
        	        *(SP + 1) = p;
        	    }
        	}
        	break;

        /* CLOSE-FILE */
        case 5:
        	{
        	    int p = *SP, i;

        	    *SP = fclose(ptr[p]);
        	    for (i = p; i < lastptr; i++) ptr[i] = ptr[i + 1];
        	    lastptr--;
        	}
        	break;

        /* READ-FILE */
        case 6:
        	{
        	    unsigned long i;
        	    int c = 0;

        	    for (i = 0; i < *((UCELL *)SP + 1) && c != EOF; i++) {
        	    	c = fgetc(ptr[*SP]);
        	    	if (c != EOF)
        	    	    *(M0 + FLIP(*((UCELL *)SP + 2) + i)) = (BYTE)c;
        	    }
        	    SP++;
        	    if (c != EOF) *SP = 0;
        	    else *SP = -1;
        	    *((UCELL *)SP + 1) = (UCELL)i;
        	}
        	break;

	/* WRITE-FILE */
        case 7:
        	{
        	    unsigned long i;
        	    int c = 0;

    	    	    for (i = 0; i < *((UCELL *)SP + 1) && c != EOF; i++)
    	    	        c = fputc(*(M0 + FLIP(*((UCELL *)SP + 2) + i)),
    	    	            ptr[*SP]);
        	    SP += 2;
        	    if (c != EOF) *SP = 0;
        	    else *SP = -1;
        	}
        	break;

        /* FILE-POSITION */
        case 8:
        	{
        	    long res = ftell(ptr[*SP]);

        	    *((UCELL *)SP--) = (UCELL)res;
        	    if (res != -1) *SP = 0;
        	    else *SP = -1;
                }
                break;

        /* REPOSITION-FILE */
        case 9:
        	{
        	    int res = fseek(ptr[*SP], *((UCELL *)SP + 1), SEEK_SET);

        	    *++SP = (UCELL)res;
        	}
        	break;

        /* FLUSH-FILE */
        case 10:
        	{
        	    int res = fflush(ptr[*SP]);

        	    if (res != EOF) *SP = 0;
        	    else *SP = -1;
        	}
        	break;

        /* RENAME-FILE */
        case 11:
        	{
        	    int res;
        	    unsigned char from[256], to[256];

        	    getstr(from, *((UCELL *)SP + 1));
        	    getstr(to, *(UCELL *)SP++);
        	    res = rename((char *)from, (char *)to);

        	    if (res != 0) *SP = -1;
        	    else *SP = 0;
        	}
        	break;

        /* DELETE-FILE */
        case 12:
        	{
        	    int res;
        	    unsigned char file[256];

        	    getstr(file, *(UCELL *)SP);
        	    res = remove((char *)file);

        	    if (res != 0) *SP = -1;
        	    else *SP = 0;
        	}
        	break;

    }
}
