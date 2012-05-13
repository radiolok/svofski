#include <stdio.h>
#include "interpreter.h"
#include "varpool.h"

// basic syntax

// line_number operator
// operator ::= {let|print|goto|...}
// expression ::= {assignment|evaluatable}
// assignment ::= identifier = expression

#define isdigit(c) ((c) >= '0' && (c) <= '9')
#define isequalsign(c) ((c)=='=')

#define trimlead(s) for (;(*(s) == ' ' | *(s) == '\t') && *(s) != 0; (s)++)

#define STATE0 0
#define STATE1 1

int streq(char *s1, char *s2) {
    for (;*s1 == *s2 && *s1 != 0 && *s2 != 0; s1++, s2++);
    return *s1 == *s2;
}

int xstrncpy(char *from, char *to, int len) {
    int i;
    for (i = 0; *from != 0 && i < len; i++, *to++ = *from++);
}

void xmemset(char *ptr, int len, char val) {
    int i;
    for (i = 0; i < len; i++) *(ptr+i) = 0;
}

void split_input(char *input, int *linenumber, char **statement);
void parse_statement(char *statement, char *stmt, char **var, char **rest);
char identify_statement(char *s);
char *cache_var(char *name, char type);

void evaluate(char *input) {
	char *statement;
	char *rest;
	int linenumber;
	char stmt_code;
	char *var;

    printf("%d %d %d\n", streq("a","b"), streq("a","a"), streq("abc", "ab"));

	split_input(input, &linenumber, &statement);
	printf("after split input: %d:%s\n", linenumber, statement);
	if (linenumber == 0) {
	    parse_statement(statement, &stmt_code, &var, &rest);
	}
}

void split_input(char *input, int *linenumber, char **statement) {
	int i;
	int state = STATE0;
	int has_linenumber = 0;
	static char numbuf[6];
	char *number;

	*linenumber = 0;
	*statement = NULL;

	has_linenumber = isdigit(*input);
	state = has_linenumber ? STATE0 : STATE1;

	for(i = 0; i < 6; i++) numbuf[i] = 0;
	number = numbuf;

	for (i = 0; *input != 0 && *statement == 0; i++, input++) {
		switch (state) {
		case STATE0:
			*number++ = *input;
			if (!isdigit(*(input + 1)) || i >= 4) {
				state = STATE1;
			}
			break;
		case STATE1:
			*statement = input;
			break;
		}
	}

	//for (;(**statement == ' ' | **statement == '\t') && **statement != 0; (*statement)++);
	trimlead(*statement);

	*linenumber = atoi(numbuf);
}

static char* statement_strs[] = {"LET", "PRINT", "GOTO", "IF", "THEN", "ELSE"};
enum statement_ids {
    STMT_LET = 0,
    STMT_PRINT = 1,
    STMT_GOTO = 2,
    STMT_IF = 3,
    STMT_THEN = 4,
    STMT_ELSE = 5};
#define N_STATEMENTS (STMT_ELSE + 1)

enum type_ids {
    TYPE_FLOAT = 0,
    TYPE_INT = 1,
    TYPE_STRING = 2
};

// could be assignment or else
void parse_statement(char *statement, char *stmt, char **var, char **rest) {
    int i;

    //char *L, *R;

    char L[8];
    char *identifier;

    for (i = 0; *statement != 0; statement++, i++) {
        L[i] = *statement;
        if (isequalsign(*statement)) {
            L[i] = 0;
            *stmt = STMT_LET;
            identifier = L;
            *var = cache_var(identifier, TYPE_INT);
            *rest = statement+1;
            break;
        } else if (isalnum(*statement)) {
            // keep on
        } else {
            L[i] = 0;
            *stmt = identify_statement(L);
            *rest = statement;
            break;
        }
    }

    trimlead(*rest);
}

char identify_statement(char *s) {
    char stmt = -1;
    char i;
    for (i = 0; i < N_STATEMENTS && stmt == -1; i++) {
        if (streq(statement_strs[i], s)) {
            stmt = i;
        }
    }
    return stmt;
}


