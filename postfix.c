#include "postfix.h"

extern int token;
extern string detail;

int isOperator(int token){
	if (token == M_PLUS || token == M_MINUS || token == M_TIMES || token == M_DIVIDE || token == M_INTEGER_DIVIDE || token == M_LESS 
	|| token == M_MORE || token == M_LESS_EQUAL || token == M_MORE_EQUAL || token == M_EQUAL || token == M_NOTEQUAL){
		return 1;
	}
	
	return 0;
}

int detailToToken(string detail){
	if (strcmp(str_to_array(detail), "+") == 0){
		return M_PLUS;
	}
	else if (strcmp(str_to_array(detail), "-") == 0){
		return M_MINUS;
	}
	else if (strcmp(str_to_array(detail), "*") == 0){
		return M_TIMES;
	}
	else if (strcmp(str_to_array(detail), "/") == 0){
		return M_DIVIDE;
	}
	else if (strcmp(str_to_array(detail), "\\") == 0){
		return M_INTEGER_DIVIDE;
	}
	else if (strcmp(str_to_array(detail), "<") == 0){
		return M_LESS;
	}
	else if (strcmp(str_to_array(detail), ">") == 0){
		return M_MORE;
	}
	else if (strcmp(str_to_array(detail), "<=") == 0){
		return M_LESS_EQUAL;
	}
	else if (strcmp(str_to_array(detail), ">=") == 0){
		return M_MORE_EQUAL;
	}
	else if (strcmp(str_to_array(detail), "=") == 0){
		return M_EQUAL;
	}
	else if (strcmp(str_to_array(detail), "<>") == 0){
		return M_NOTEQUAL;
	}
	return 0;
}


Pexpression initExpression(){
	Pexpression expression;
	if ((expression = (Pexpression) malloc(sizeof(struct _expression))) == NULL){
		return NULL;
	}
	
	expression->number = 0;
	return expression;
}

void addToExpr(Pexpression expression, string element){
	expression->elements = (string *) realloc(expression->elements, (expression->number + 1) * sizeof(struct str) );
	expression->elements[expression->number] = str_new();
	str_put(expression->elements[expression->number], str_to_array(element));
	expression->number++;
}

int priority(string operator){
	if (strcmp(str_to_array(operator), "*") == 0 || strcmp(str_to_array(operator), "/") == 0){
		return 1;
	}
	else if (strcmp(str_to_array(operator), "\\") == 0){
		return 2;
	}
	else if (strcmp(str_to_array(operator), "+") == 0 || strcmp(str_to_array(operator), "-") == 0){
		return 3;
	}
	else if (strcmp(str_to_array(operator), "=") == 0 || strcmp(str_to_array(operator), "<>") == 0 || strcmp(str_to_array(operator), "<") == 0
			|| strcmp(str_to_array(operator), "<=") == 0 || strcmp(str_to_array(operator), ">") == 0 || strcmp(str_to_array(operator), ">=") == 0){
		return 4;
	}
	
	return 0;
}

int isPossibleToken(int flag, int token){
	extern s_btree t_symtable;
	s_btree tmp;

		if (isOperator(token) || BTget(&t_symtable, str_to_array(detail), &tmp) == 0 || token == M_LEFT_PARANTHESE || token == M_RIGHT_PARANTHESE || token == M_INTEGER_VAL || token == M_DOUBLE_VAL){
			return 1;	// V poradku, pokracuj
		}
		else if (flag == 1 && token == M_THEN){
			return POSTFIX_END;	// V poradku, konec vyrazu v IF
		}
		else if ((flag == 2 || flag == 3) && token == M_EOL){
			return POSTFIX_END;	// V poradku, konec vyrazu v LOOP nebo Prirazeni
		}
		

		return POSTFIX_ERROR;		
}

int infixToPostfix(Pexpression *expression){
	
	stack exprStack;
	exprStack = stack_new();
	*expression = initExpression();
	
	int flag = 0; // 1: IF		2: Prirazeni 	3: LOOP	
	
	if (token == M_IF){
		flag = 1;
	}
	else if (token == M_EQUAL){
		flag = 2;
	}
	else if (token == M_WHILE){
		flag = 3;
	}
	
	while(1){
		token = scanner();
		if (token == LEX_ERROR) return LEX_ERROR;
		
		if (isPossibleToken(flag, token) == POSTFIX_END && stack_empty(exprStack)) {
			break;
		}
		else if (isPossibleToken(flag, token) == POSTFIX_ERROR){
			return POSTFIX_ERROR;
		}

		if ((!isOperator(token) && isPossibleToken(flag, token) != POSTFIX_END) && token != M_LEFT_PARANTHESE && token != M_RIGHT_PARANTHESE){
			
			addToExpr(*expression, detail);
		}
		else if (token == M_LEFT_PARANTHESE){
			string tmp = str_new();
			str_put(tmp, "(");
			stack_push(exprStack, tmp);
			str_clear(tmp);
		}
		else if(isOperator(token) || ((isPossibleToken(flag, token) == POSTFIX_END) && !stack_empty(exprStack))) {
			printf("> %s\n", str_to_array(detail));
			printf(">>> emptiness: %d\n", stack_empty(exprStack));
			while (1){
				if (stack_empty(exprStack)){
					stack_push(exprStack, detail);
					break;
				}
				else if (strcmp(str_to_array(stack_top(exprStack)), "(") == 0){
					stack_push(exprStack, detail);
					break;
				}
				else if (isOperator(detailToToken(stack_top(exprStack))) && (priority(stack_top(exprStack)) < priority(detail))){
					stack_push(exprStack, detail);
					break;
				}
				else if (isOperator(detailToToken(stack_top(exprStack))) && (priority(stack_top(exprStack)) >= priority(detail))){
					printf(">> %s\n", str_to_array(detail));
					addToExpr(*expression, stack_pop(exprStack));
					break;
				}
			}
		}
		else if (token == M_RIGHT_PARANTHESE){
			string tmp;
			string tmp2 = str_new();
			str_put(tmp2, ")");
			while (str_cmp((tmp = stack_pop(exprStack)), tmp2) == 0){
				addToExpr(*expression, tmp);
			}
			str_clear(tmp);
			str_clear(tmp2);
		}
		else if (token == M_EQUAL){
			while (!stack_empty(exprStack)){
				addToExpr(*expression, stack_pop(exprStack));
			}
			addToExpr(*expression, detail);
			break;
		}

		if (isPossibleToken(flag, token) == POSTFIX_END) {

			break;
		}
	}
	
	return POSTFIX_OK;
}


















