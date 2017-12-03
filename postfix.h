/**
 * Projekt - Tým 097, varianta I
 * Autor: Pavel Kaleta (xkalet05)
 * Převedení infixového výrazu pomocí analýzi na postfixovou notaci
 */

#ifndef HEADER_POSTFIX
#define HEADER_POSTFIX

#define STACK_TYPE string
#define POSTFIX_OK 80
#define POSTFIX_END 81
#define POSTFIX_ERROR 79
#include "string.h"
#include "stack.h"
#include "parser.h"
#include "symtable.h"

typedef struct _expression{
	string *elements;
	int *types;
	int number;
}*Pexpression;

extern int token;

int isOperator(int token);

int detailToToken(string detail);

Pexpression initExpression();

void addToExpr(Pexpression expression, string element, int type);

int priority(string operator);

int isPossibleToken(int flag, int token);

int infixToPostfix(Pexpression *expression, int);

int removeElement(Pexpression expression, int index);

int isLoginOperation(const char *oper);

#endif