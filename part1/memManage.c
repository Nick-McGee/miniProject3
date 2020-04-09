#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    // open memAdr.txt
    FILE *memAdr  = fopen("memAdr.txt", "r");

    /*
    n - the n lowest significant bits that represent the offset
    m - the next m bits that represent the page number; assume that n+m is always 16 
    v1 - first virtual address that needs to be mapped to a page number and offset
    v2 - second virtual address
    v3 etc... -  a sequence of virtual addresses, one per line until the end of the file
    */

    // check for memAdr.txt
    if(memAdr == NULL) {
        printf("Error! File named 'memAdr.txt' is missing.");
        exit(1);
    }

    // assumption is made that m+n adds up to 16 bits
    unsigned int n;  // the lower n bits
    fscanf(memAdr,"%d", &n);
    unsigned int m;  // the upper m bits
    fscanf(memAdr,"%d", &m);

    // read from memAdr.txt one int per line until end of file
    unsigned int virAdr;
    while(fscanf(memAdr,"%d", &virAdr) != EOF) {
        unsigned int pageNum = virAdr >> n;  // Logical right-shift n number of bits to get upper m bits.
        unsigned int offset = virAdr & ((1 << n) - 1); // Bitmask w/ bitwise AND to get lowest n significant bits.
        printf("Virtual Address %d is in page number %d and offset %d\n", virAdr, pageNum, offset);
    }

    // close memAdr.txt
    fclose(memAdr);
}
