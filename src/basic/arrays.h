#ifndef _ARR_H_
#define _ARR_H_


typedef struct _Array {
    char **elements;
    int count;
    int allocated;
    int chunk;
} Array;

typedef struct _Queue {
    Array *a;
    int length;
    int ihead, itail;
} Queue;

typedef struct _Stack {
    Array *a;
    int size;
    int head;
} Stack;

void array_expand(Array *a);
Array* array_new(int initialsize, int chunk);
char *array_add(Array *a, char *entry);
char *array_get(Array *a, int i);
char *array_set(Array *a, int i, char *val);
void array_init(Array *a, int size, char *val);
void array_swap(Array *a, int i1, int i2);
void array_free(Array **a);

Stack* stack_new(int stacksize);
char *stack_push(Stack *s, char *val);
char *stack_pop(Stack *s);
void stack_swap(Stack *s);
int stack_empty(Stack *s);
char *stack_peek(Stack *s);
void stack_free(Stack **s);

Queue* queue_new(int qlength);
char* queue_put(Queue *q, char *s);
char* queue_take(Queue *q);
char* queue_free(Queue **q);



void testarray(void);
void testqueue(void);
void teststack(void);

#endif


