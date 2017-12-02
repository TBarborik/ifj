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

int removeElement(Pexpression expression, int index){
	if (expression->number == 0){
		return -1;
	}
	
	if (index >= expression->number){
		return -1;
	}
	
	if(expression->elements[index] == NULL){
		return -1;
	}
	
	str_clear(expression->elements[index]);
	for (int i = index; i > 0; i--){
		expression->elements[i] = expression->elements[i-1];
		expression->types[i] = expression->types[i-1];
	}
	expression->elements[0] = NULL;
	expression->types[0] = -1;
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

void addToExpr(Pexpression expression, string element, int type){
	expression->elements = (string *) realloc(expression->elements, (expression->number + 1) * sizeof(struct str));
	expression->types = (int *) realloc(expression->types, (expression->number + 1) * sizeof(struct str));
	expression->elements[expression->number] = str_new();
	str_put(expression->elements[expression->number], str_to_array(element));
	expression->types[expression->number] = type;
	expression->number++;
}

int priority(string operator){
	if (strcmp(str_to_array(operator), "*") == 0 || strcmp(str_to_array(operator), "/") == 0){
		return 4;
	}
	else if (strcmp(str_to_array(operator), "\\") == 0){
		return 3;
	}
	else if (strcmp(str_to_array(operator), "+") == 0 || strcmp(str_to_array(operator), "-") == 0){
		return 3;
	}
	else if (strcmp(str_to_array(operator), "=") == 0 || strcmp(str_to_array(operator), "<>") == 0 || strcmp(str_to_array(operator), "<") == 0
			|| strcmp(str_to_array(operator), "<=") == 0 || strcmp(str_to_array(operator), ">") == 0 || strcmp(str_to_array(operator), ">=") == 0){
		return 1;
	}
	
	return 0;
}

int isLoginOperation(const char *oper) {
	if (strcmp(oper, "=") == 0 || strcmp(oper, "<>") == 0 || strcmp(oper, "<") == 0
	|| strcmp(oper, "<=") == 0 || strcmp(oper, ">") == 0 || strcmp(oper, ">=") == 0){
		return 1;
	}

	return 0;
}

int isPossibleToken(int flag, int token){
	extern s_btree t_symtable;
	s_btree tmp;
	int r = 0;

	if (isOperator(token) || token == M_LEFT_PARANTHESE || token == M_STRING_VAL
		|| token == M_RIGHT_PARANTHESE || token == M_INTEGER_VAL || token == M_DOUBLE_VAL || ((r = BTget(&t_symtable, str_to_array(detail), &tmp)) == 0)){
		return 1;	// V poradku, pokracuj
	}
	else if (flag == 1 && token == M_THEN){
		return POSTFIX_END;	// V poradku, konec vyrazu v IF
	}
	else if ((flag == 2 || flag == 3 || flag == 5) && token == M_EOL){
		return POSTFIX_END;	// V poradku, konec vyrazu v LOOP nebo Prirazeni
	}
	else if (flag == 4 && (token == M_SEMICOLON)){
		return POSTFIX_END;
	}

	return r != 0 ? SEMANTIC_ERROR : POSTFIX_ERROR;		
}

int infixToPostfix(Pexpression *expression, int flag){	// FLAG: 1: IF		2: Prirazeni 	3: LOOP		4: PRINT	5: PRIRAZENI FUNC/VYRAZ
	
	stack exprStack;
	exprStack = stack_new();
	*expression = initExpression();
	
	int allowScanFlag = 1;
	
	if (flag == 4 || flag == 5){
		allowScanFlag = 0; 
	}
	
	while(1){
		if (allowScanFlag){
			token = scanner();
		}
		else{
			allowScanFlag = 1;
		}

		if (token == LEX_ERROR) return LEX_ERROR;

		int r = -1;
		
		if (isPossibleToken(flag, token) == POSTFIX_END){
			flag = 6; // Pro ukonceni vyrazu, ale aby se jeste zpracovalo, co je na zasobniku
		}
		else if ((r = isPossibleToken(flag, token)) == POSTFIX_ERROR || r == SEMANTIC_ERROR){
			return r;
		}
		
		if (flag == 6){
			while (!stack_empty(exprStack)){
				addToExpr(*expression, stack_pop(exprStack), 0);
			}
			break;
		}
		else if (!isOperator(token) && token != M_LEFT_PARANTHESE && token != M_RIGHT_PARANTHESE){
			addToExpr(*expression, detail, token);
		}
		else if (token == M_LEFT_PARANTHESE){
			string tmp = str_new();
			str_put(tmp, "(");
			stack_push(exprStack, tmp);
		}
		else if(isOperator(token)){

			while (1){
	
				if (stack_empty(exprStack)) {
					string tmp = str_new();
					str_append(tmp, str_to_array(detail));
					stack_push(exprStack, tmp);
					break;
				}
				else if (strcmp(str_to_array(stack_top(exprStack)), "(") == 0) {
					string tmp = str_new();
					str_append(tmp, str_to_array(detail));
					stack_push(exprStack, tmp);
					break;
				}
				else if (isOperator(detailToToken(stack_top(exprStack))) && (priority(stack_top(exprStack)) < priority(detail))) {
					string tmp = str_new();
					str_append(tmp, str_to_array(detail));
					stack_push(exprStack, tmp);
					break;
				}
				else if (isOperator(detailToToken(stack_top(exprStack))) && (priority(stack_top(exprStack)) >= priority(detail))) {
					addToExpr(*expression, stack_pop(exprStack), 0);
				}
				else{
					break;
				}
			}
		}
		else if (token == M_RIGHT_PARANTHESE){
			string tmp;
			string tmp2 = str_new();
			str_put(tmp2, "(");
			while (!stack_empty(exprStack) && (str_cmp((tmp = stack_pop(exprStack)), tmp2) != 0)){
				addToExpr(*expression, tmp, 0);
			}
			str_clear(tmp);
			str_clear(tmp2);
		}
		
	}
	
	return POSTFIX_OK;
}


















