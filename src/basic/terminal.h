#ifndef _TERMINAL_H_
#define _TERMINAL_H

void term_open(FILE*, FILE*);
char *term_readln();
void term_println(char *line);

#endif
