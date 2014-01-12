#include <stdio.h>
#include <stdlib.h>


#include "tokenizer.h"
// tokens
// separate & && | ||


enum _state  {
    S0, S1, S2_AMP, S2_PIPE, S2_EQ, S_END
};

static char *single_terminals = "()+-/*.,:;$%^!~";
static char *single_or_double = "&|=";

// duplets -=, +=, /=, *= ?

int index(const char *s, char c) {
    int i, len;
    len = strlen(s);
    for (i = 0; i < len; i++) {
        if (*(s+i) == c) {
            return i;
        }
    }
    return -1;
}

int is_whitespace(char c) {
    return (c == ' ' | c == '\t');
}

int is_nonterminal(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_';
}

int is_single_terminal(char c) {
    return index(single_terminals, c) != -1;
}

int is_single_or_double_terminal(char c) {
    return c == '&' || c == '|' || c == '=';
}

// return pointer to next token and its length in *toklen
char *nexttoken(char *stream, int *toklen, char *lookahead) {
    static int state = S0;

    static char* streamski;

    unsigned char c;

    int tokenlength = 0;

    char *tokenstart;

    if (state == S_END && stream == NULL) {
        return NULL;
    }

    if (stream != NULL) {
        streamski = stream;
        state = S0;
    }

    *toklen = 0;
    for(;*toklen == 0;) {
        unsigned char c = *streamski;
        if (c == 0) {
            *toklen = tokenlength;
            if (tokenlength == 0) {
                tokenstart = NULL;
            }
            *lookahead = 0;
            state = S_END;
            break;
        }
        switch (state) {
            case S0:
                if (is_whitespace(c)) {
                    streamski++;
                } else if (is_nonterminal(c)) {
                    state = S1;
                    tokenstart = streamski;
                    tokenlength = 1;
                    streamski++;
                } else if (is_single_terminal(c)) {
                    tokenstart = streamski;
                    *toklen = 1; // issue terminal token
                    streamski++;
                    break;
                } else if (is_single_or_double_terminal(c)) {
                    tokenstart = streamski;
                    tokenlength = 0;
                    streamski++;
                    switch (c) {
                        case '&': state = S2_AMP; break;
                        case '|': state = S2_PIPE; break;
                        case '=': state = S2_EQ; break;
                    }
                }
                break;
            case S1:
                if (is_nonterminal(c)) {
                    streamski++;
                    tokenlength++;
                } else {
                    // issue token
                    *toklen = tokenlength;
                    state = S0;
                }
                break;
            case S2_AMP:
                if (c == '&') {
                    // && happen
                    *toklen = 1;
                    *tokenstart = T_BOOLAND;
                    state = S0;
                    streamski++;
                } else {
                    *toklen = 1;
                    state = S0;
                }
                break;
            case S2_EQ:
                if (c == '=') {
                    *toklen = 1;
                    *tokenstart = T_EQUALS;
                    state = S0;
                    streamski++;
                } else {
                    *toklen = 1;
                    state = S0;
                }
                break;
            case S2_PIPE:
                if (c == '|') {
                    *toklen = 1;
                    *tokenstart = T_BOOLOR;
                    state = S0;
                    streamski++;
                } else {
                    *toklen = 1;
                    state = S0;
                }
                break;
        }
    }

    *lookahead = *streamski;
    return tokenstart;
}

void testtokenizer() {
    //char *t1 = "(22+18*x4^7)/(1+sqrt(8)) == 0 && foo == bar";
    char *t1 = "1*(2+3)";
    char *t;
    char la;
    int tlen;
    char tokbuf[32];

    for (t = nexttoken(t1, &tlen, &la); t != NULL; t = nexttoken(NULL, &tlen, &la)) {
        strncpy(tokbuf, t, tlen);
        tokbuf[tlen] = 0;
        printf("[%s(%c)]", tokbuf, la);
    }
}
