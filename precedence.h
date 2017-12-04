#ifndef H_PRECEDENCE
#define H_PRECEDENCE

#include "string.h"
#include "stack.h"
#include "parser.h"
#include "symtable.h"
#include "postfix.h"
#include "parser.h"

#define P_PLUS 0
#define P_MINUS 1
#define P_TIMES 2
#define P_DOUBLEDIVIDE 3
#define P_INTDIVIDE 4
#define P_LESS 5
#define P_MORE 6
#define P_LESSEQUAL 7
#define P_MOREEQUAL 8
#define P_EQUAL 9
#define P_NOTEQUAL 10
#define P_LEFTPARENTHESIS 11
#define P_RIGHTPARENTHESIS 12
#define P_ID 13
#define P_$ 14

int prec_analyse(Pexpression *, int);

#endif