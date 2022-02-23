        ## Define trap codes
        .set LIBC, 0x0
        .set STDOUT_FILENO, 0x3
        .set WRITE_FILE, 0xd

        .text
main:
        pushreli string           # address of string
        pushi string_end - string # length of string
        pushi STDOUT_FILENO
        trap LIBC
        pushi WRITE_FILE
        trap LIBC
        pushi 0                   # normal termination
        throw

        .data
string:
        .ascii "Hello, world!"
string_end:
