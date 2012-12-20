#ifndef _TOKENIZER_H_
#define _TOKENIZER_H_

enum tokens {
    T_LPAREN    = '(',
    T_RPAREN    = ')',
    T_PLUS      = '+',
    T_MINUS     = '-',
    T_UMINUS    = 0xd4,
    T_DIV       = '/',
    T_MUL       = '*',
    T_POINT     = '.',
    T_COMMA     = ',',
    T_COLON     = ':',
    T_SEMICOLON = ';',
    T_DOLLAR    = '$',
    T_PERCENT   = '%',
    T_HAT       = '^',
    T_EQUALS    = '=',
    T_BOOLAND   = 0xd0, // &&
    T_BOOLOR    = 0xd1, // ||
    T_BOOLNOT   = 0xd2,
    T_BOOLXOR   = 0xd3,
    T_BITAND    = '&',
    T_BITOR     = '|',
    T_BITXOR    = '~',
    T_BITNOT    = '!',
    TO_LET      = 0xe0,
    TO_PRINT,
    TO_REM,
    TO_NEW,
    TO_GOTO,
    TO_IF,
    TO_THEN,
    TO_ELSE,
    TO_FOR,
    TO_TO,
    TO_STEP,
    TO_NEXT
};

char *nexttoken(char *stream, int *toklen, char *lookahead);
void testtokenizer();

#endif
