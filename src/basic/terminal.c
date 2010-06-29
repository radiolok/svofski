#include <stdio.h>
#include "terminal.h"

static char inputbuffer[257];
static FILE* instream;
static FILE* outstream;

void term_open(FILE* sin, FILE* sout) {
	instream = sin == NULL ?  stdin : sin;
	outstream = sout == NULL ? stdout : sout;
}

char *term_readln() {
	fgets(inputbuffer, 256, instream);
}

void term_println(char *line) {
	fprintf(outstream, "%s\n", line);
}
