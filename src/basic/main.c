#include <stdio.h>

#include "interpreter.h"
#include "varpool.h"
#include "arrays.h"
#include "eval.h"

main(int argc, char **argv) {
	char *input;
	term_open(stdin, stdout);

    printf("%e\n", 0.1+0.1+0.1-0.3);

	testarray();
	testqueue();
	teststack();
	testtokenizer();
	testeval();

	for (;;) {
		evaluate(term_readln());
		dump_vars();
	}
}
