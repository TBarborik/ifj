/**
 * Projekt - Tým 097, varianta I
 * Autor: Tom Barbořík (xbarbo06)
 * Obecný zásobní pro ukládání a vyzvedávání hodnot (např. pro výrazy)
 */

/**
 * Abstract stack library. Works with preprocessor
 * constant STACK_TYPE to determine what
 * type of data will be stored.
 * If STACK_TYPE is not 
 * defined, void * is
 * used as type of
 * data.
 * 
 * @author Tom Barbořík
 */
 #ifndef STACK_H
 #define STACK_H
 
 #include <stdlib.h> // memory
 #include "string.h" // string

 #ifndef STACK_TYPE
 #define STACK_TYPE string
 #endif
 
 typedef struct sti {
     STACK_TYPE data; // item's data
     struct sti *next; // next item (null means this is the last)
 } *stackItem;

 typedef struct stc {
     stackItem top;
 } *stack;
 
 /**
  * Creates new stack
  * @return stack
  */
 stack stack_new();

 /**
  * Removes stack and all its items
  * @param s stack
  */
 void stack_del(stack s);

 /**
  * Checks if stack is empty
  * @return int 0 for empty stack, 1 for not empty stack
  */
 int stack_empty(stack s);

 /**
  * Adds new item to the top of the stack
  * @param s stack
  * @param data STACK_TYPE (void * if not specified)
  * @return stackItem
  */
 stackItem stack_push(stack s, STACK_TYPE data);

 /**
  * Returns data from item at the top of the stack and removes it.
  * @param s stack
  * @return STACK_TYPE (void * if not specified)
  */
 STACK_TYPE stack_pop(stack s);

 /**
  * Returns data from item at the top of the stack.
  * @param s stack
  * @return STACK_TYPE (void * if not specified)
  */
 STACK_TYPE stack_top(stack s); 

 /**
  * Returns length of stack
  * @param s stack
  * @return unsigned
  */
 unsigned stack_length(stack s);

 #endif