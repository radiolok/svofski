#include <stdio.h>
#include <math.h>

#include "evaloperator.h"
#include "tokenizer.h"

#define DEF_O(x)    RPNEntry *x(RPNEntry*,RPNEntry*,RPNEntry*)

DEF_O(o_pow);
DEF_O(o_mul);
DEF_O(o_div);
DEF_O(o_remainder);
DEF_O(o_add);
DEF_O(o_sub);
DEF_O(o_neg);
/*
RPNEntry *o_pow(RPNEntry *, RPNEntry*, RPNEntry *);
RPNEntry *o_mul(RPNEntry *, RPNEntry*, RPNEntry *);
RPNEntry *o_div(RPNEntry *, RPNEntry*, RPNEntry *);
RPNEntry *o_remainder(RPNEntry *, RPNEntry*, RPNEntry *);
RPNEntry *o_add(RPNEntry *, RPNEntry*, RPNEntry *);
RPNEntry *o_sub(RPNEntry *, RPNEntry*, RPNEntry *);
RPNEntry *o_neg(RPNEntry *, RPNEntry*, RPNEntry *);
*/
static Operator operator_list[N_OPERATORS] =
    {
        {OPER_POW,          '^', 2, 8, RASSOC, o_pow},
        {OPER_MUL,          '*', 2, 7, LASSOC, o_mul},
        {OPER_DIV,          '/', 2, 7, LASSOC, o_div},
        {OPER_REMAINDER,    '%', 2, 7, LASSOC, o_remainder},
        {OPER_PLUS,         '+', 2, 6, LASSOC, o_add},
        {OPER_MINUS,        '-', 2, 6, LASSOC, o_sub},
        {OPER_UMINUS,       T_UMINUS, 1, 9, LASSOC, o_neg},
    };

Operator* get_operator(char *s) {
    int i;
    for (i = 0; i < N_OPERATORS; i++) {
        if (*s == operator_list[i].literal) {
            return &operator_list[i];
        }
    }
    return NULL;
}

Operator* get_operator_by_id(int id) {
    int i;
    for (i = 0; i < N_OPERATORS; i++) {
        if (operator_list[i].opid == id) {
            return &operator_list[i];
        }
    }
    return NULL;
}

RPNEntry *o_pow(RPNEntry* q, RPNEntry* x, RPNEntry *y) {
    rpn_setreal(q, pow(rpn_getreal(x),rpn_getreal(y)));

    return q;
}

RPNEntry *o_mul(RPNEntry* q, RPNEntry* x, RPNEntry *y) {
    rpn_setreal(q, rpn_getreal(x) * rpn_getreal(y));

    return q;
}

RPNEntry *o_div(RPNEntry* q, RPNEntry* x, RPNEntry *y) {
    // ka-pow!
    rpn_setreal(q, rpn_getreal(x) / rpn_getreal(y));

    return q;
}

RPNEntry *o_remainder(RPNEntry* q, RPNEntry* x, RPNEntry *y) {
    rpn_setreal(q, rpn_getint(x) % rpn_getint(y));

    return q;
}

RPNEntry *o_add(RPNEntry* q, RPNEntry* x, RPNEntry *y) {
    rpn_setreal(q, rpn_getreal(x) + rpn_getreal(y));

    return q;
}

RPNEntry *o_sub(RPNEntry* q, RPNEntry* x, RPNEntry *y) {
    rpn_setreal(q, rpn_getreal(x) - rpn_getreal(y));

    return q;
}

RPNEntry *o_neg(RPNEntry* q, RPNEntry*x, RPNEntry* y) {
    switch (x->kind) {
        case RPNE_INT_OPERAND:
            rpn_setint(q, -rpn_getint(x));
            break;
        case RPNE_REAL_OPERAND:
            rpn_setreal(q, -rpn_getreal(x));
            break;
    }

    return q;
}
