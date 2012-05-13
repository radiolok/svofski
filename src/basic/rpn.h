#ifndef _RPN_H_
#define _RPN_H_

enum RPNEntryKind {
    RPNE_VAR_OPERAND,
    RPNE_INT_OPERAND,
    RPNE_REAL_OPERAND,
    RPNE_STRING_OPERAND,
    RPNE_FUNCTION_ID,
    RPNE_OPERATOR_ID
};

#define RPN_DATA_LENGTH sizeof(double)
typedef struct _rpn_entry {
    char kind;
    union {
        int var_id;
        int int_value;
        double real_value;
        char* string_value;
        int function_id;
        int operator_id;
        unsigned char raw[RPN_DATA_LENGTH];
    } data;
} RPNEntry;

#define rpn_setreal(q,x)    {(q)->kind = RPNE_REAL_OPERAND;(q)->data.real_value=(x);}
#define rpn_setint(q,x)     {(q)->kind = RPNE_INT_OPERAND;(q)->data.int_value=(x);}
double rpn_getreal(RPNEntry *x);
int rpn_getint(RPNEntry *x);
void rpn_copy(RPNEntry* from, RPNEntry* to);

RPNEntry *new_rpnentry(char *token, int ltoken);
RPNEntry *new_rpnentry_real(double value);
void free_rpnentry(RPNEntry **e);
#endif
