Created by Nicholas McGee.

Compilation Instructions
Run in the terminal:
gcc -o memManage memManage.c
./memManage


NOTE: This program does not take any command line arguements or user input, just a valid memAdr.txt file in the same directory.



Description
This program reads the file as such:
    1) n - the n lowest significant bits that represent the offset
    2) m - the next m bits that represent the page number; assume that n+m is always 16 
    3) v1 - first virtual address that needs to be mapped to a page number and offset
    4) v2 - second virtual address
    5 and on) v3 etc... -  a sequence of virtual addresses, one per line until the end of the file

First, the program reads both n and m seperately.
Then, a while loop reads the virtual address until it reaches the end of the file.

The page number is calculated with the code 'virAdr >> n;' by using a logical right-shift n number of bits to get upper m bits.

The offset is calulcated with the code 'virAdr & ((1 << n) - 1);' by creating a bitmask w/ bitwise AND to get lowest n significant bits.
