        ## Define extra instruction codes
        .set STDOUT_FILENO, 0x84
        .set WRITE_FILE, 0x89

        .text
main:
        pushrel string           # address of string
        push string_end - string # length of string
        extra STDOUT_FILENO
        extra WRITE_FILE
        push 0                   # normal termination
        throw

        .data
string:
        .ascii "Hello, world!"
string_end:
