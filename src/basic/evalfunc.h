#ifndef _EVALFUNC_H_
#define _EVALFUNC_H_

#define N_FUNCTIONS 6

#include "eval.h"


enum _func_id {
    FNID_SIN,
    FNID_COS,
    FNID_SQRT,
    FNID_LOG,
    FNID_EXP,
    FNID_HYPOT
};

typedef struct _function {
    const int fnid;
    const char *literal;
    const int argc;
    const Evaluator evaluator;
} Function;

Function* get_function(char *s, int ls);
Function* get_function_by_id(int id);


#endif
