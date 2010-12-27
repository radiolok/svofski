#include <stdio.h>
#include "evalfunc.h"
#include "eval.h"
#include "math.h"

RPNEntry* fn_sin(RPNEntry *q, RPNEntry *a, RPNEntry *b);
RPNEntry* fn_cos(RPNEntry *q, RPNEntry *a, RPNEntry *b);
RPNEntry* fn_sqrt(RPNEntry *q, RPNEntry *a, RPNEntry *b);
RPNEntry* fn_log(RPNEntry *q, RPNEntry *a, RPNEntry *b);
RPNEntry* fn_exp(RPNEntry *q, RPNEntry *a, RPNEntry *b);
RPNEntry* fn_hypot(RPNEntry *q, RPNEntry *a, RPNEntry *b);


static Function function_list[N_FUNCTIONS] =
    {
        {FNID_SIN, "sin",1, &fn_sin},
        {FNID_COS, "cos",1, fn_cos},
        {FNID_SQRT, "sqrt",1, fn_sqrt},
        {FNID_LOG, "log",1, fn_log},
        {FNID_EXP, "exp",1, fn_exp},
        {FNID_HYPOT, "hypot",2, fn_hypot},
    };

// compare string with defined length against a zero terminated string
int lstreq(char *ls, int l, const char *sz) {
    int i = 0;
    for (i = 0; i < l && *sz != 0; i++, sz++) {
        if (*sz != *(ls+i)) {
            return 0; // fail
        }
    }
    return i == l && *sz == 0;
}

Function* get_function(char *s, int len) {
    int i;
    for (i = 0; i < N_FUNCTIONS; i++) {
        //if (strncmp(s, function_list[i].literal) == 0) {
        if (lstreq(s, len, function_list[i].literal)) {
            return &function_list[i];
        }
    }
    return NULL;
}

Function* get_function_by_id(int id) {
    int i;
    for (i = 0; i < N_FUNCTIONS; i++) {
        if (function_list[i].fnid == id) {
            return &function_list[i];
        }
    }
    return NULL;
}

RPNEntry* fn_sin(RPNEntry *q, RPNEntry *a, RPNEntry *b) {
    rpn_setreal(q, sin(rpn_getreal(a)));
    return q;
}

RPNEntry* fn_cos(RPNEntry *q, RPNEntry *a, RPNEntry *b) {
    rpn_setreal(q, cos(rpn_getreal(a)));
    return q;
}

RPNEntry* fn_sqrt(RPNEntry *q, RPNEntry *a, RPNEntry *b) {
    rpn_setreal(q, sqrt(rpn_getreal(a)));
    return q;
}

RPNEntry* fn_log(RPNEntry *q, RPNEntry *a, RPNEntry *b) {
    rpn_setreal(q, log(rpn_getreal(a)));
    return q;
}

RPNEntry* fn_exp(RPNEntry *q, RPNEntry *a, RPNEntry *b) {
    rpn_setreal(q, exp(rpn_getreal(a)));
    return q;
}

RPNEntry* fn_hypot(RPNEntry *q, RPNEntry *a, RPNEntry *b) {
    rpn_setreal(q, hypot(rpn_getreal(a), rpn_getreal(b)));
    return q;
}
