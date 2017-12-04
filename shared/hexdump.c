// Lab 4: hexdump.c - utility functions to display memory content ranges - C Part 2 Lect

// See: http://stackoverflow.com/questions/7775991/how-to-get-hexdump-of-a-structure-data

#include <stdio.h>
#include "hexdump.h"

// void * means a generic pointer i.e. a pointer of any type
//  only to be used after checking nothing else is applicable
void hexdump(const char *desc, const void *addr, size_t len)
{
    int i;
    unsigned char buff[17];
    unsigned char *pc = (unsigned char *) addr;

    // Output description if given.
    if (desc != NULL)
	printf("variable: %s, start address: %p, length: %zu\n", desc,
	       addr, len);

    if (len == 0) {
	printf("  ZERO LENGTH\n");
	return;
    }
    // Process every byte in the data.
    for (i = 0; i < len; i++) {	// Implicit cast size_t to int
	// Multiple of 16 means new line (with line offset).

	if ((i % 16) == 0) {
	    // Just don't print ASCII for the zeroth line.
	    if (i != 0)
		printf("  %s\n", buff);

	    // Output the offset.
	    printf("  %04x ", i);
	}
	// Now the hex code for the specific character.
	printf(" %02x", pc[i]);

	// And store a printable ASCII character for later.
	if ((pc[i] < 0x20) || (pc[i] > 0x7e))
	    buff[i % 16] = '.';
	else
	    buff[i % 16] = pc[i];
	buff[(i % 16) + 1] = '\0';
    }

    // Pad out last line if not exactly 16 characters.
    while ((i % 16) != 0) {
	printf("   ");
	i++;
    }

    // And print the final ASCII bit.
    printf("  %s\n\n", buff);
}				//end of hexdump

void chardump(const char *desc, const void *addr, size_t len)
{
    int i;
    unsigned char buff[17];
    unsigned char *pc = (unsigned char *) addr;

    // Output description if given.
    if (desc != NULL)
	printf("variable: %s, start address: %p, length: %zu\n", desc,
	       addr, len);

    if (len == 0) {
	printf("  ZERO LENGTH\n");
	return;
    }
    // Process every byte in the data.
    for (i = 0; i < len; i++) {
	// Multiple of 16 means new line (with line offset).

	if ((i % 16) == 0) {
	    // Just don't print ASCII for the zeroth line.
	    if (i != 0)
		printf("  %s\n", buff);

	    // Output the offset.
	    printf("  %04x ", i);
	}
	// Now the specific character.
	// or pretty print '\0' as appropriate
	if (pc[i] == '\0')
	    printf(" \\0");
	else
	    printf("  %c", pc[i]);

	// And store a printable ASCII character for later.
	if ((pc[i] < 0x20) || (pc[i] > 0x7e))
	    buff[i % 16] = '.';
	else
	    buff[i % 16] = pc[i];
	buff[(i % 16) + 1] = '\0';
    }

    // Pad out last line if not exactly 16 characters.
    while ((i % 16) != 0) {
	printf("   ");
	i++;
    }

    // And print the final ASCII bit.
    printf("  %s\n\n", buff);
}				//end of chardump
