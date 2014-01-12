#ifndef _VPOOL_H_
#define _VPOOL_H_
typedef struct _var_list {
    char id[7];
    char type;
    union {
        int intvalue;
        int floatvalue;
        char *stringvalue;
    } value;

    struct _var_list *next;
} var_list;


char *cache_var(char *name, char type);

#endif
