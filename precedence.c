#include "precedence.h"

int tokenToPrecIndex(int token){
	int index = -1;
	switch(token){
		case M_PLUS:
			index = 0;
			break;
		case M_MINUS:
			index = 1;
			break;
		case M_TIMES:
			index = 2;
			break;
		case M_DIVIDE:
			index = 3;
			break;
		case M_INTEGER_DIVIDE:
			index = 4;
			break;
		case M_LESS:
			index = 5;
			break;
		case M_MORE:
			index = 6;
			break;
		case M_LESS_EQUAL:
			index = 7;
			break;
		case M_MORE_EQUAL:
			index = 8;
			break;
		case M_EQUAL:
			index = 9;
			break;
		case M_ID:
			index = 13;
			break;
		case '@':
			index = 14;
		default:
			return SYNTAX_ERROR;
	}
	
	return index;
}

int stringToPrecIndex(string str){
	if (strcmp("+", str_to_array(str)) == 0)
		return 0;
	else if (strcmp("-", str_to_array(str)) == 0)
		return 1;
	else if (strcmp("*", str_to_array(str)) == 0)
		return 2;
	else if (strcmp("/", str_to_array(str)) == 0)
		return 3;
	else if (strcmp("\\", str_to_array(str)) == 0)
		return 4;
	else if (strcmp("<", str_to_array(str)) == 0)
		return 5;
	else if (strcmp(">", str_to_array(str)) == 0)
		return 6;
	else if (strcmp("<=", str_to_array(str)) == 0)
		return 7;
	else if (strcmp(">=", str_to_array(str)) == 0)
		return 8;
	else if (strcmp("=", str_to_array(str)) == 0)
		return 9;
	else if (strcmp("<>", str_to_array(str)) == 0)
		return 10;
	else if (strcmp("(", str_to_array(str)) == 0)
		return 11;
	else if (strcmp(")", str_to_array(str)) == 0)
		return 12;
	else if (strcmp("@", str_to_array(str)) == 0)
		return 14;
	else return 13;
	
	
}

string rule_exist(string prava_strana){ //TODO
	(void) prava_strana;
	return NULL;
}

int prec_analyse(Pexpression *expression, int flag){
	char precedence_table[15][15]={
		// sloupce = prichozi tokeny, radky = zasobnik
		//+   -   *   /   \   <   >  <=  >=   =  <>	  (	  )   i   $
		{'>','>','<','<','<','>','>','>','>','>','>','<','>','<','>'},	// +
		{'>','>','<','<','<','>','>','>','>','>','>','<','>','<','>'},	// -
		{'>','>','>','>','>','>','>','>','>','>','>','<','>','<','>'},	// *
		{'>','>','>','>','>','>','>','>','>','>','>','<','>','<','>'},	// /
		{'>','>','>','>','>','>','>','>','>','>','>','<','>','<','>'},	// \   '
		{'<','<','<','<','<','>','>','>','>','>','>','<','>','<','>'},	// <
		{'<','<','<','<','<','>','>','>','>','>','>','<','>','<','>'},	// >
		{'<','<','<','<','<','>','>','>','>','>','>','<','>','<','>'},	// <=
		{'<','<','<','<','<','>','>','>','>','>','>','<','>','<','>'},	// >=
		{'<','<','<','<','<','>','>','>','>','>','>','<','>','<','>'},	// =
		{'<','<','<','<','<','>','>','>','>','>','>','<','>','<','>'},	// <>
		{'<','<','<','<','<','<','<','<','<','<','<','<','=','<','0'},	// (
		{'>','>','>','>','>','>','>','>','>','>','>','0','>','0','>'},	// )
		{'>','>','>','>','>','>','>','>','>','>','>','0','>','0','>'},	// i
		{'<','<','<','<','<','<','<','<','<','<','<','<','0','<','@'},	// @
	};
	
	extern int token;
	int b;
	stack pStack = stack_new(); // Zasobnik pro precedencni analyzu
	string tmp_at = str_new();
	str_append_char(tmp_at, '@');
	stack_push(pStack, tmp_at); // Dno zasobniku
	int allowScanFlag = 1; // Pomocny flag pro zjisteni, zda uz byl nacten token v predchozim kroku
	
	if (flag == 4 || flag == 5){ // Pomocny flag pro spravne ukonceni vyrazu
		allowScanFlag = 0; 
	} return infixToPostfix(expression, flag);
	while (1){
		if (allowScanFlag){
			token = scanner();
		}
		else{
			allowScanFlag = 1;
		}
		if (isPossibleToken(flag, token) != 81){ // Vstupni retezec dorazil do konce
			allowScanFlag = 0; // Zastavi nacitani noveho tokenu
			token = '@'; // Nastavi konec retezce
		}
		b = tokenToPrecIndex(token);
		int a = stringToPrecIndex(stack_top(pStack));
		
		switch(precedence_table[a][b]){
			case '=': {
				string tmp = str_new();
				str_append(tmp, str_to_array(detail));
				stack_push(pStack, tmp);
				break;
			}
			case '<': {
				string prec_l = str_new();
				str_append(prec_l, "PREC<");
				stack_push(pStack, prec_l);
				break;
			}
			case '>': {
				string tmp = stack_pop(pStack); // Prava strana pravidla
				if (strcmp("PREC<", str_to_array(stack_pop(pStack))) == 0){ // Pred pravou stranou je PREC<
					string leva_strana = rule_exist(tmp); // Existuje pravidlo s pravou stranou tmp ? Pokud ano, vrati redukovanou (levou) stranu
					if (!leva_strana){
						return SYNTAX_ERROR;
					}
					stack_push(pStack, leva_strana); // Vlozi levou (redukovanou) stranu
					break;
				}
				else {	
					return SYNTAX_ERROR;
				}
			}
			case '@':	// Syntakticka analyza v poradku
				return SYNTAX_OK;
			case '0':	// Syntakticka chyba ve vstupnim retezci
				return SYNTAX_ERROR;
				
				
		}

	}
	return SYNTAX_OK;

}