#ifndef _EVALOPERATOR_H_
#define _EVALOPERATOR_H_

#include "eval.h"

typedef RPNEntry* (*oevaluator)(RPNEntry*, RPNEntry*);

#define N_OPERATORS 7
enum _operator_id {
    OPER_POW=1,
    OPER_MUL,
    OPER_DIV,
    OPER_REMAINDER,
    OPER_PLUS,
    OPER_MINUS,
    OPER_UMINUS,
};

//#define OPERATORS "^*/%+-"

#define LASSOC 1
#define RASSOC 2

typedef struct _operator {
    const int opid;
    const char literal; // token id
    const int argc; // ?
    struct {
        const unsigned char precedence : 4;
        const unsigned char associative : 4;
    };
    Evaluator evaluator;
} Operator;

#define is_lassoc(o) ((o).associative == LASSOC)
#define is_rassoc(o) ((o).associative == RASSOC)

Operator* get_operator(char *s);
Operator* get_operator_by_id(int id);

#endif
