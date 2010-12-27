#include <stdlib.h>
#include "rpn.h"
#include "evalfunc.h"
#include "evaloperator.h"

static int rpnentry_count = 0;

double rpn_getreal(RPNEntry *x) {
    double d = 0.0;
    switch (x->kind) {
        case RPNE_REAL_OPERAND:
            d = x->data.real_value;
            break;
        case RPNE_INT_OPERAND:
            d = x->data.int_value;
            break;
    }
    return d;
}

int rpn_getint(RPNEntry *x) {
    switch (x->kind) {
        case RPNE_REAL_OPERAND:
            return (int)round(x->data.real_value);
        case RPNE_INT_OPERAND:
            return x->data.int_value;
    }
    return 0;
}

void rpn_copy(RPNEntry* from, RPNEntry* to) {
    int i;
    to->kind = from->kind;
    for (i = 0; i < RPN_DATA_LENGTH; i++) { to->data.raw[i] = from->data.raw[i]; }
}

RPNEntry *new_rpnentry(char *token, int ltoken) {
    RPNEntry *e = (RPNEntry *)malloc(sizeof(RPNEntry));

    Function *fn = get_function(token, ltoken);
    if (fn != NULL) {
        e->kind = RPNE_FUNCTION_ID;
        e->data.function_id = fn->fnid;
    } else {
        Operator* op = get_operator(token);
        if (op != NULL) {
            e->kind = RPNE_OPERATOR_ID;
            e->data.operator_id = op->opid;
        } else {
            e->kind = RPNE_REAL_OPERAND;
            e->data.real_value = atof(token);
        }
    }
    rpnentry_count++;

    return e;
}

RPNEntry *new_rpnentry_real(double value) {
    RPNEntry *e = (RPNEntry *)malloc(sizeof(RPNEntry));
    e->kind = RPNE_REAL_OPERAND;
    e->data.real_value = value;
    rpnentry_count++;
    return e;
}

void free_rpnentry(RPNEntry **e) {
    if (*e == NULL) {
        return;
    }
    switch ((*e)->kind) {
        case RPNE_STRING_OPERAND:
            if ((*e)->data.string_value != NULL) {
                free((*e)->data.string_value);
            }
            break;
    }
    free(*e);
    *e = NULL;
    rpnentry_count--;
    //printf("-RPNEntry count=%d\n", rpnentry_count);
}
