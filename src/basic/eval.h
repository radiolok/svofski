#ifndef _EVAL_H_
#define _EVAL_H_

#include "rpn.h"

enum ERRORS {
    ERROR_NONE = 0,
    ERROR_EVAL_UNBALANCED,
};


typedef RPNEntry* (*Evaluator)(RPNEntry*, RPNEntry*, RPNEntry*);

void testeval();

#endif
