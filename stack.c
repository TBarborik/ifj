#include "stack.h"

stack stack_new()
{
    stack s = (stack) malloc(sizeof(struct stc));

    if (s == NULL)
        return NULL;

    s->top = NULL;
    return s;
}
  
void stack_del(stack s)
{
    while (!stack_empty(s)) {
        stack_pop(s);
    }

    free(s);
}

int stack_empty(stack s)
{
    return s->top == NULL;
}

stackItem stack_push(stack s, STACK_TYPE data)
{
    stackItem item = (stackItem) malloc(sizeof(struct sti));

    if (item == NULL)
        return NULL;

    item->next = s->top;
    item->data = data;
    s->top = item;

    return item;
}
  
STACK_TYPE stack_pop(stack s)
{
    stackItem i = s->top;
    STACK_TYPE d = i->data;
    
    s->top = i->next;
    free(i);

    return d;
}
  
STACK_TYPE stack_top(stack s)
{
    return s->top->data;
}

unsigned stack_length(stack s)
{
    unsigned i = 0;
    
    for (stackItem item = s->top; item != NULL; i++, item = item->next);

    return i;
}