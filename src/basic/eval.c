#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arrays.h"

#include "eval.h"
#include "evalfunc.h"
#include "evaloperator.h"
#include "tokenizer.h"
#include "rpn.h"

int is_operand(char *token) {
    return ((unsigned char)*token) < 128 && (isalnum(*token) || *token == '.');
}

int is_lparen(char *token) {
    return *token == '(';
}

int is_rparen(char *token) {
    return *token == ')';
}

int is_function(char *token, int len) {
    int i;
    return get_function(token, len) != NULL;
}

int is_operator(char *s) {
    return get_operator(s) != NULL;
}

int is_arg_separator(char *token) {
    return *token == ',';
}

int is_digit(char c) {
    return c >= '0' && c <= '9';
}

int compare_ops(char *o1, char *o2) {
    int o1p, o2p;
    Operator* o1o;
    Operator* o2o;

    if (is_lparen(o2)) return 0;

    o1o = get_operator(o1);
    o2o = get_operator(o2);

    o1p = o1o->precedence;
    o2p = o2o->precedence;

    if ((is_lassoc(*o1o) && o1p <= o2p) ||
        (is_rassoc(*o1o) != -1 && o1p < o2p)) {
        return 1;
    }

    return 0;
}


#define MIN(a,b) (((a)<(b))?(a):(b))

char *gettoken(char *expr, int *tokenlen, char *loxxokahead) {
    int inoperand;

    char lookahead;
    static char token[64];
    char *poperand;

    char *result = nexttoken(expr, tokenlen, &lookahead);
    if (result == NULL) {
        return NULL;
    }
    // check if we get full real number before going on
    inoperand = *result == '.' || is_digit(*result);
    poperand = &token[0];
    strncpy(poperand, result, *tokenlen);
    *(poperand + *tokenlen) = 0;

    for(;inoperand;) {
        switch (inoperand) {
            case 1:
                if (*result == '.' && is_digit(lookahead)) {
                    inoperand = 3;
                } else if (is_digit(*result)) {
                    if (lookahead == '.') {
                        inoperand = 2;
                    } else if (*(result+*tokenlen-1) == 'E' || *(result+*tokenlen-1) == 'e') {
                        inoperand = 11;
                    } else {
                        inoperand = 0;
                    }
                } else {
                    inoperand = 0;
                }
                break;
            case 2:
                // skip dot
                inoperand = 3;
                break;
            case 3:
                if (is_digit(*result)) {
                    if (*(result+*tokenlen-1) == 'E' || *(result+*tokenlen-1) == 'e') {
                        inoperand = 11;
                    } else {
                        inoperand = 0;
                    }
                }
                break;
            case 10:
                inoperand = 11;
                break;
            case 11:
                if (*result == '+' || *result == '-') {
                    inoperand = 12;
                } else if (is_digit(*result)) {
                    inoperand = 0; // done
                } else {
                    inoperand = 0;
                }
                break;
            case 12:
                inoperand = 0;
                break;
        }

        if (inoperand) {
            poperand += *tokenlen;
            result = nexttoken(NULL, tokenlen, &lookahead);
            strncpy(poperand, result, *tokenlen);
            *(poperand + *tokenlen) = 0;
        }

    }

    *tokenlen = strlen(token);
    return &token[0];
}

void infix2rpn(char *expr, RPNEntry ***rpne, int *nrpne) {
    Array *rpn = array_new(32, 16);
    Stack *stack_ops = stack_new(16);
    //static char token[64];
    char *token;
    int tokenlen;
    int i;

    int uminus = 1;
    int inoperand = 0;
    //printf("%s:-> ", expr);

    char lookahead;

    for(token = gettoken(expr, &tokenlen, &lookahead);token != NULL; token = gettoken(NULL, &tokenlen, &lookahead)) {
        if (uminus) {
            if (*token == T_MINUS) {
                *token = T_UMINUS;
            }
            uminus = 0;
        }

        if (is_function(token, tokenlen)) {
            stack_push(stack_ops, token);
        } else if (is_operand(token)) {
            array_add(rpn, token);
        } else if (is_arg_separator(token)) {
            while(!stack_empty(stack_ops) && !is_lparen(stack_peek(stack_ops))) {
                array_add(rpn, stack_pop(stack_ops));
            }
        } else if (is_operator(token)) {
            uminus = 1;
            if (stack_empty(stack_ops)) {
                stack_push(stack_ops, token);
            } else {
                if (compare_ops(token, stack_peek(stack_ops))) {
                    array_add(rpn, stack_pop(stack_ops));
                }
                stack_push(stack_ops, token);
            }
        } else if (is_lparen(token)) {
            uminus = 1;
            stack_push(stack_ops, token);
        } else if (is_rparen(token)) {
            while(!stack_empty(stack_ops) && !is_lparen(stack_peek(stack_ops))) {
                array_add(rpn, stack_pop(stack_ops));
            }
            stack_pop(stack_ops);
        } else {
            // errorski
        }
    }
    while (!stack_empty(stack_ops)) {
        array_add(rpn, stack_pop(stack_ops));
    }

    stack_free(&stack_ops);
    *rpne = (RPNEntry **)malloc(rpn->count * sizeof(RPNEntry*));
    for (i = 0; i < rpn->count; i++) {
        token = array_get(rpn, i);
        (*rpne)[i] = new_rpnentry(token, strlen(token));
    }
    *nrpne = rpn->count;
    array_free(&rpn);
}

int eval_operator_on_stack(RPNEntry **rpnlist, int osp, int operator_id, int fn_id) {
    int error = 0;
    double result = 0;
    int sp = osp;
    int i, k;
    int shiftc = 0;
    static char sbuf[16];

    RPNEntry *newentry = NULL;
    RPNEntry *rarg1 = NULL;
    RPNEntry *rarg2 = NULL;

    if (operator_id != -1) {
        //arg2 = rpnlist[--sp]->data.int_value;
        //arg1 = rpnlist[--sp]->data.int_value;
        Operator *op = get_operator_by_id(operator_id);
        if (op != NULL) {
            shiftc = op->argc;
            if (op->argc == 2) {
                if (sp < 2) {
                    return ERROR_EVAL_UNBALANCED;
                }
                rarg2 = rpnlist[--sp];
                rarg1 = rpnlist[--sp];
            } else {
                if (sp < 1) {
                    return ERROR_EVAL_UNBALANCED;
                }
                rarg1 = rpnlist[--sp];
            }
            newentry = new_rpnentry_real(0);
            op->evaluator(newentry, rarg1, rarg2);
        }
    } else if (fn_id != -1) {
        Function *fn = get_function_by_id(fn_id);
        if (fn != NULL) {
            shiftc = fn->argc;
            switch (fn->argc) {
                case 1:
                    if (sp < 1) {
                        return ERROR_EVAL_UNBALANCED;
                    }
                    rarg1 = rpnlist[--sp];
                    break;
                case 2:
                    if (sp < 2) {
                        return ERROR_EVAL_UNBALANCED;
                    }
                    rarg2 = rpnlist[--sp];
                    rarg1 = rpnlist[--sp];
                    break;
            }
            newentry = new_rpnentry_real(0);
            fn->evaluator(newentry, rarg1, rarg2);
        }
    }

    //printf("{%d}", result);

    free_rpnentry(&rpnlist[osp]);       // free operator/function
    for (k = 1; k <= shiftc; k++) {
        free_rpnentry(&rpnlist[osp-k]); // free arguments
    }

    // collapse used entries
    for (k = shiftc; k >= 1; k--) {
        for (i = osp - k; i >= 1; i--) {
            rpnlist[i] = rpnlist[i - 1];
        }
    }
    for (k = 0; k < shiftc; k++) {
        rpnlist[k] = NULL;
    }
    rpnlist[osp] = newentry;

    return 0;
}

void rpnprint(RPNEntry** entries, int n) {
    int i;
    RPNEntry *rpne;

    for (i = 0; i < n; i++) {
        rpne = entries[i];
        if (rpne == NULL) {
            printf("(null)");
            continue;
        }
        switch (rpne->kind) {
            case RPNE_OPERATOR_ID:
                printf("O%d", rpne->data.operator_id);
                break;
            case RPNE_FUNCTION_ID:
                printf("F%d", rpne->data.function_id);
                break;
            case RPNE_INT_OPERAND:
                printf("I%d", rpne->data.int_value);
                break;
        }
        printf(" ");
    }
    printf("\n");
}

int rpneval(RPNEntry** rpnlist, int n, RPNEntry *out) {
    int i;
    int sp;
    double result;
    RPNEntry *rpne;
    int error = 0;

    //rpnprint(rpnlist, n);
    for (i = 0; i < n && error == ERROR_NONE; i++) {
        rpne = rpnlist[i];
        switch (rpne->kind) {
            case RPNE_OPERATOR_ID:
                //rpnprint(rpnlist, n);
                error = eval_operator_on_stack(rpnlist, i, rpne->data.operator_id, -1);
                break;
            case RPNE_FUNCTION_ID:
                //rpnprint(rpnlist, n);
                error = eval_operator_on_stack(rpnlist, i, -1, rpne->data.function_id);
                break;
            case RPNE_INT_OPERAND:
                break;
        }
    }
    //result = rpn_getreal(rpnlist[n-1]);
    if (error == ERROR_NONE) {
        rpn_copy(rpnlist[n-1], out);
    } else {
        rpn_setreal(out, nan("NaN"));
    }
    //free_rpnentry(&rpnlist[n-1]);
    for (i  = 0; i < n; i++) {
        free_rpnentry(&rpnlist[i]);
    }
    return error;
}

#define EVALPRINT(expr,rpn,nrpn,out,e) {infix2rpn(expr, &rpn, &nrpn);\
                        e=rpneval(rpn,nrpn,&out);\
                        printf("%s evaluated and returned %+0.4f\n", expr, rpn_getreal(&out));\
                        free(rpn);}

char *trimend(char *ic) {
    char* c = ic;
    for(; *c != 0; c++) if (*c == '\n') *c = 0;
    return ic;
}

void testeval() {
    char expr01[] = "3e-3";
    char expr11[] = "10+20*30";
    char expr12[] = "1*(2+3)";
    char expr13[] = "(1*2)+3";
    char expr14[] = "(1+2)*3";
    char expr15[] = "1+(2+3)";
    char expr3[] = "1*2^8+3";
    char expr4[] = "1*(2^8)+3";

    char expr51[] = "2*sin(1)";
    char expr52[] = "2*hypot(1,2+3)";

    char buffa[256];

    RPNEntry** rpn;
    RPNEntry out;
    int nrpn;
    int e;

    EVALPRINT(expr01,rpn,nrpn,out,e);

    EVALPRINT(expr11,rpn,nrpn,out,e);
    EVALPRINT(expr12,rpn,nrpn,out,e);
    EVALPRINT(expr13,rpn,nrpn,out,e);
    EVALPRINT(expr14,rpn,nrpn,out,e);
    EVALPRINT(expr15,rpn,nrpn,out,e);

    EVALPRINT(expr3,rpn,nrpn,out,e);
    EVALPRINT(expr4,rpn,nrpn,out,e);
    EVALPRINT(expr51,rpn,nrpn,out,e);
    EVALPRINT(expr52,rpn,nrpn,out,e);

    for(;;) {
        printf(":");
        trimend(fgets(buffa, 255, stdin));
        EVALPRINT(buffa, rpn, nrpn,out,e);
    }

}
