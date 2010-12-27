#include <stdio.h>
#include "varpool.h"

static var_list* variables;

var_list* new_var(var_list* cur) {
    var_list* vl = malloc(sizeof(var_list));
    vl->next = cur;
    return vl;
}

var_list* find_var(char *name) {
    var_list *u = variables;

    for (;u != NULL && !streq(u->id, name); u = u->next);

    return u;
}

char *cache_var(char *name, char type) {
    var_list* var = find_var(name);
    if (var == NULL) {
        variables = new_var(variables);
        var = variables;
        xmemset(var->id, 7, 0);
        xstrncpy(name, var->id, 7);
        var->type = type;
        var->value.intvalue = 0;
    }
    return var->id;
}

void dump_vars() {
    var_list* v = variables;
    for (;v != NULL; v = v->next) {
        printf("Variable %s = %d\n", v->id, v->value.intvalue);
    }
}
