#include <stdio.h>
#include <string.h>

#include "arrays.h"

Stack* stack_new(int stacksize) {
    Stack *s = (Stack *)malloc(sizeof(Stack));
    s->size = stacksize;
    s->head = stacksize;
    s->a = array_new(stacksize, 0);
    array_init(s->a, stacksize, NULL);

    return s;
}

char *stack_push(Stack *s, char *val) {
    char* retval = NULL;
    if (s->head > 0) {
        s->head--;
        retval = array_set(s->a, s->head, val);
    }

    return retval;
}

char *stack_peek(Stack *s) {
    char *retval = NULL;
    if (!stack_empty(s)) {
        retval = array_get(s->a, s->head);
    }

    return retval;
}

char *stack_pop(Stack *s) {
    char* retval = NULL;
    if (s->head < s->size) {
        retval = array_get(s->a, s->head);
        s->head++;
    }
    return retval;
}

void stack_swap(Stack *s) {
    if(s->head + 1 < s->size) {
        array_swap(s->a, s->head, s->head+1);
    }
}

int stack_empty(Stack *s) {
    return s->head == s->size;
}


void stack_free(Stack **s) {
    array_free(&((*s)->a));
    free(*s);
    *s = NULL;
}

Queue* queue_new(int qlength) {
    Queue* q = (Queue *)malloc(sizeof(Queue));
    q->ihead = 0;
    q->itail = 0;
    q->length = qlength;
    q->a = array_new(qlength, 0); // fixed size
    array_init(q->a, qlength, NULL);
    return q;
}

char* queue_put(Queue *q, char *s) {
    char *retval = NULL;
    int newhead = (q->ihead + 1) % q->length;
    if (newhead != q->itail) {
        retval = array_set(q->a, q->ihead, s);
        q->ihead = newhead;
    }
    return retval;
}

char* queue_take(Queue *q) {
    char* retval = NULL;
    if (q->ihead != q->itail) {
        retval = array_get(q->a, q->itail);
        q->itail = (q->itail + 1) % q->length;
    }
    return retval;
}

char* queue_free(Queue **q) {
    array_free(&((*q)->a));
    free(*q);
    *q = NULL;
}

Array* array_new(int initialsize, int chunk) {
    Array *arr = (Array*)malloc(sizeof(Array));
    arr->elements = (char **)malloc(initialsize * sizeof(char *));
    arr->count = 0;
    arr->chunk = chunk;
    arr->allocated = initialsize;

    return arr;
}

void array_expand(Array *a) {
    int newallocated = a->allocated + a->chunk;
    char **newelements = (char **)realloc(a->elements, newallocated*sizeof(char *));
    if (newelements != NULL) {
        a->elements = newelements;
        a->allocated = newallocated;
    }
}

// returns a copy
char *array_add(Array *a, char *entry) {
    char *retval = NULL;

    if (a->count == a->allocated) {
        array_expand(a);
    }

    if (entry != NULL) {
        a->elements[a->count] = (char *)malloc(strlen(entry) + 1);
        retval = a->elements[a->count];
        strcpy(retval, entry);
    } else {
        a->elements[a->count] = NULL;
    }

    a->count++;

    return retval;
}

char *array_get(Array *a, int i) {
    if (i < a->count) {
        return a->elements[i];
    }
    return NULL;
}

void array_swap(Array *a, int i1, int i2) {
    char *temp = a->elements[i1];
    a->elements[i1] = a->elements[i2];
    a->elements[i2] = temp;
}

char *array_set(Array *a, int i, char *val) {
    char *retval = NULL;
    if (i < a->count) {
        free(a->elements[i]);
        retval = (char *)malloc(strlen(val) + 1);
        a->elements[i] = retval;
        strcpy(retval, val);
    }
    return retval;
}

void array_init(Array *a, int size, char *val) {
    int i;
    if (a->count == 0) {
        for (i = 0; i < size; i++) {
            array_add(a, val);
        }
    }
}

void array_free(Array **a) {
    int i;
    for (i = (*a)->count; --i >= 0;) {
        if ((*a)->elements[i] != NULL) {
            free((*a)->elements[i]);
        }
    }
    free((*a)->elements);
    free(*a);
    *a = NULL;
}



void testarray() {
    int i;
    Array *a1 = array_new(1, 2);
    array_add(a1, "a");
    array_add(a1, "b");
    array_add(a1, "c");
    array_add(a1, "d");
    array_add(a1, "e");
    for (i = 0; i < a1->count; i++) {
        printf("%s ", array_get(a1, i));
    }
    array_free(&a1);
    printf("\n");
}

void testqueue() {
    int i;
    Queue *q1 = queue_new(5);
    queue_put(q1, "first");
    queue_put(q1, "second");
    queue_put(q1, "third");

    printf("%s ", queue_take(q1));
    printf("%s ", queue_take(q1));

    queue_put(q1, "fourth");
    queue_put(q1, "fifth");
    queue_put(q1, "sixth");
    printf("%s ", queue_take(q1));
    printf("%s ", queue_take(q1));
    printf("%s ", queue_take(q1));
    printf("%s ", queue_take(q1));
    printf("%s ", queue_take(q1));

    queue_free(&q1);
    printf("\n");
}

void teststack() {
    Stack *s1 = stack_new(4);
    stack_push(s1, "aa");
    stack_push(s1, "bb");
    stack_push(s1, "cc");
    stack_push(s1, "dd");
    stack_push(s1, "xx");

    printf("%s ", stack_pop(s1));

    stack_swap(s1);
    printf("%s ", stack_pop(s1));
    printf("%s ", stack_pop(s1));
    printf("%s ", stack_pop(s1));

    stack_free(&s1);
    printf("\n");
}
