#define STACK_TYPE int

#include "syntaxtree.h"
#include "generator.h"
#include <stdio.h>

int main()
{
    s_list list;
	s_stree params;

	s_stree var1 = STcreateVar("test_var", d_int);
	s_stree var2 = STcreateVar("test_var2", d_int);
	s_stree var3 = STcreateVar("test_var3", d_double);

	Linit(&list);
	
	Ladd(&list, var1);
	Ladd(&list, var2);
	Ladd(&list, STcreateExpr(var1, "=", STcreateIntConst(15)));
	Ladd(&list, STcreateExpr(var2, "=", STcreateExpr(
		var1,
		"\\",
		STcreateDoubleConst(3.12)
	)));
	Ladd(&list, STcreatePrint(var2));

	// RETURN FUNCTION
	s_stree fc_var = STcreateVar("fc_var", d_int);
	Ladd(&list, STcreateFunc("test_function", d_double, NULL));
	Ladd(&list, fc_var);
	Ladd(&list, STcreateExpr(fc_var, "=", STcreateDoubleConst(10.15)));
	Ladd(&list, STcreateReturn(fc_var));
	Ladd(&list, STcreateEndFunc());

	Ladd(&list, var3);
	Ladd(&list, STcreateExpr(var3, "=", STcreateCall("test_function", NULL)));
	Ladd(&list, STcreatePrint(var3));

	Ladd(&list, STcreatePrint(STcreateCall("test_function", NULL)));

	// FACTORIAL
	s_stree param1 = STcreateVar("param1", d_int);
	Ladd(&list, STcreateFunc("factorial", d_int, STcreateFirstParam(param1)));
	Ladd(&list, STcreateIf(STcreateExpr(param1, "==", STcreateIntConst(1))));
	Ladd(&list, STcreateReturn(param1));
	Ladd(&list, STcreateEndIf());
	Ladd(&list, STcreateExpr(param1, "=", STcreateExpr(param1, "-", STcreateIntConst(1))));
	Ladd(&list, STcreateReturn(STcreateExpr(
		param1,
		"*",
		STcreateCall("factorial", STcreateFirstParam(param1))
	)));;
	Ladd(&list, STcreateEndFunc());

	Ladd(&list, STcreatePrint(STcreateCall("factorial", STcreateFirstParam(STcreateIntConst(5)))));


    generate(list, stdout);

	Lclear(&list);
    return 0;
}
