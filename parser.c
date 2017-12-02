#include "parser.h"

#define SYMTABLE_ERROR 41
#define scan token = scanner(); if (token == LEX_ERROR) return LEX_ERROR;

int expr_analyse(s_stree*, int);
int parse();

int token;  //Uložení aktualního tokenu ze vstupu
int token_n;	//Uložení následujícího tokenu (nutno použít pro rozeznání přiřazení od volání funkce ve funkci func_stat_exfu)

string detail_act;	//Uložení detailu aktuálního tokenu (je-li to nutné)
string detail_next;	//Uložení detailu následujícího tokenu (je-li to nutné)

int eol_flag = 0;	//Flag značící, že mám již načtený token a tudíž musím přeskočit volání scanneru
int token_next_flag = 0;	//Flag značící zda-li je nutno v precedenční analýze kontrolovat 2 vstupní tokeny (Použití, při přiřazení viz.: int token_n)

s_btree g_symtable = NULL;
s_btree t_symtable = NULL;

/*
TODO: Nutno dokončit vkládání a ověřování ID v tabulce symbolů
TODO: Nutno dokončit vkládání do derivační tabulky pro generování mezikódu
	-> Hotovo: end if , end while , end func , else
TODO: Ověřit ve funkci func_stat_exfu správné použití knihovny STRING
*/



int strToInt(const char *str) {
	return atoi(str);
}

double strToDouble(const char *str) {
	return strtod(str, NULL);
}

/**
*	START → <F_DEC_DEF_LIST> BODY EOF
*	@return Kód propagující syntaktickou správnost či chybu
*/
int func_start()	// pravidlo <func_start> -> <func_f_dec_def_list> <func_body> <END_OF_FILE>
{
	int result;

	if(token == M_EOL)	// kontrola konce řádku na začátku vstupu
	{
		token = scanner();
		if (token == LEX_ERROR) return LEX_ERROR;
	}

	result = func_f_dec_def_list();
	if (result != SYNTAX_OK) return result;		//Kontroluji, jestli výstup funkce je v pořádku
	
	result = func_body();
	if (result != SYNTAX_OK) return result;		//Kontroluji, jestli výstup funkce je v pořádku
	
	token = scanner();
	if (token == LEX_ERROR) return LEX_ERROR;

	if(token == M_EOL)	// konrola konce řádku na konci vstupu
	{
		token = scanner();
		if (token == LEX_ERROR) return LEX_ERROR;
	}

	if (token != M_END_OF_FILE)
	{
		return SYNTAX_ERROR;
	}
	else
	{
		return SYNTAX_OK;	//Propaguji syntaktickou správnost
	}
}

/**
*	BODY → scope EOL <PROGRAM> end scope
*	@return Kód propagující syntaktickou správnost či chybu
*/
int func_body()
{
	int result;

	if (token == M_SCOPE)
	{
		token = scanner();
		if (token == LEX_ERROR) return LEX_ERROR;

		if(token == M_EOL)
		{
			token = scanner();
			if (token == LEX_ERROR) return LEX_ERROR;

			result = func_program();
			if (result != SYNTAX_OK) return result;

			if(token == M_END)
			{
				token = scanner();
				if (token == LEX_ERROR) return LEX_ERROR;

				if(token == M_SCOPE)
				{
					return SYNTAX_OK;
				}
			}
		}
	}

	return SYNTAX_ERROR;
}

/**
*	F_DEC_DEF_LIST → F_DEC_DEF EOL <F_DEC_DEF_LIST>
*	F_DEC_DEF_LIST → &
*	@return Kód propagující syntaktickou správnost či chybu
*/
int func_f_dec_def_list()  // pravidlo <func_f_dec_def_list> -> <func_f_dec_def> EOL <func_f_dec_def_list>
{
	int result;

	if(token == M_SCOPE)
	{
		return SYNTAX_OK;
	}

	result = func_f_dec_def();
	if (result != SYNTAX_OK) return result;		//Kontroluji, jestli výstup funkce je v pořádku

	token = scanner();
	if (token == LEX_ERROR) return LEX_ERROR;

	if(token == M_EOL)
	{
		token = scanner();
		if (token == LEX_ERROR) return LEX_ERROR;

		return func_f_dec_def_list();
	}

	return SYNTAX_ERROR;
}

/**
*	F_DEC_DEF → declare function id (<PARAM_LIST>) as DATATYPE
*	F_DEC_DEF → function id (<PARAM_LIST>) as DATATYPE EOL <F_PROGRAM> end function
*	@return Kód propagující syntaktickou správnost či chybu
*/
int func_f_dec_def()
{
	int result;

	if (token == M_DECLARE)	//Větev deklarace funkce
	{
		token = scanner();
		if (token == LEX_ERROR) return LEX_ERROR;
		
		if (token == M_FUNCTION)
		{
			token = scanner();
			if (token == LEX_ERROR) return LEX_ERROR;
		
			if(token == M_ID)	//TODO!!!ID->kontrolovat tabulku symbolů
			{
/*				// pokud tento ID jiz byl deklarovan, jedna se o semantickou chybu
				if (tableInsert(table, &attr, TYPE_INT) == 1) return SEM_ERROR;
*/					

				char *id = str_to_array(detail); // Zbytečný kus kódu ... deklarace se nebude vkládat do stromu
				s_stree params = NULL;
				e_dtype returnType = d_undef;
				token = scanner();
				if (token == LEX_ERROR) return LEX_ERROR;

				if(token == M_LEFT_PARANTHESE)
				{
					token = scanner();
					if (token == LEX_ERROR) return LEX_ERROR;

					result = func_param_list(&params);
					if (result != SYNTAX_OK) return result;

					if(token == M_RIGHT_PARANTHESE)
					{
						token = scanner();
						if (token == LEX_ERROR) return LEX_ERROR;
						
						if(token == M_AS)
						{
							token = scanner();
							if (token == LEX_ERROR) return LEX_ERROR;

							result = func_datatype(&returnType);
							if (result != SYNTAX_OK) return result;

							s_btree node;
							int resultTree = 1;

							if ((resultTree = BTget(&g_symtable, id, &node)) == -1) {
								BTput(&g_symtable, id, sn_func_dec, returnType, params);
								return SYNTAX_OK;
							} else if (resultTree == 0) {
								return SEMANTIC_ERROR;
							} else {
								return SYMTABLE_ERROR;
							}

						}	
					}
				}
			}
		}
	}
	else if(token == M_FUNCTION)	//Větev definice funkce
	{
		token = scanner();
		if (token == LEX_ERROR) return LEX_ERROR;
		
		if(token == M_ID)	//TODO!!!ID->kontrolovat tabulku symbolů
		{
/*			// pokud již byla funkce jednou definována jedná se o chybu
			if (tableSearch(table, detail) != 1) return SEM_ERROR;
*/						

			char *id = str_to_array(detail);
			e_dtype type = d_void;
			s_stree params = NULL;

			token = scanner();
			if (token == LEX_ERROR) return LEX_ERROR;

			if(token == M_LEFT_PARANTHESE)
			{
				token = scanner();
				if (token == LEX_ERROR) return LEX_ERROR;

				result = func_param_list(&params);
				if (result != SYNTAX_OK) return result;

				if(token == M_RIGHT_PARANTHESE)
				{
					token = scanner();
					if (token == LEX_ERROR) return LEX_ERROR;

					if(token == M_AS)
					{
						token = scanner();
						if (token == LEX_ERROR) return LEX_ERROR;

						result = func_datatype(&type);
						if (result != SYNTAX_OK) return result;

						token = scanner();
						if (token == LEX_ERROR) return LEX_ERROR;

						if(token == M_EOL)
						{
							token = scanner();
							if (token == LEX_ERROR) return LEX_ERROR;

							Ladd(&list, STcreateFunc(id, type, params));


							BTinit(&t_symtable);

							s_stree t_params = params;
							while(t_params != NULL) {
								if (BTput(&t_symtable, t_params->lptr->value.v_string, sn_var, t_params->lptr->dtype, NULL) != 0)
									return SYMTABLE_ERROR;

								t_params = t_params->rptr;
							}

							result = func_f_program();
							BTdispose(&t_symtable);
							t_symtable = NULL;
							if (result != SYNTAX_OK) return result;

							if(token == M_END)
							{
								token = scanner();
								if(token == M_FUNCTION)
								{
									Ladd(&list,STcreateEndFunc());
									eol_flag = 0;

									s_btree node;
									int resultTree = 1;
		
									if (((resultTree = BTget(&g_symtable, id, &node)) == -1) || (resultTree == 0 && node->ntype == sn_func_dec)) { // nebyl nalezen nebo byl nalezen ale jenom deklarovan
										if (resultTree == 0) {
											s_stree dec_params = node->params;


											int param_result = node->dtype == type;
											while(param_result) {
												param_result = dec_params != NULL && params != NULL;
											
												if (!param_result) {
													break;
												}

												param_result = dec_params->lptr->dtype == params->lptr->dtype;
												if (param_result) {
													dec_params = dec_params->rptr;
													params = params->rptr;
												}
											}

											if (!param_result && dec_params != params) {
												return SEMANTIC_ERROR;
											}

											BTput(&g_symtable, id, sn_func_dec_def, type, node->params);

										} else {
											BTput(&g_symtable, id, sn_func_dec_def, type, params);
										}
										return SYNTAX_OK;
									} else if (resultTree == 0 && node->ntype == sn_func_dec_def) { // nalezen ale už i definován
										return SEMANTIC_ERROR;
									} else {
										return SYMTABLE_ERROR;
									}

									return SYNTAX_OK;
								}
							}
						}
					}
				}
			}
		}
	}

	return SYNTAX_ERROR;	//Pokud neodpovídá derivačnímu stromu
}

/**
*	DATATYPE → integer
*	DATATYPE → double
*	DATATYPE → string
*	@return Kód propagující syntaktickou správnost či chybu
*/
int func_datatype(e_dtype *type)
{
	switch(token)
	{
		case M_INTEGER:
			*type = d_int;
			return SYNTAX_OK;
		case M_DOUBLE:
			*type = d_double;
			return SYNTAX_OK;
		case M_STRING:
			*type = d_string;
			return SYNTAX_OK;
	}

	return SYNTAX_ERROR;
}

/**
*	F_PROGRAM → F_P_DEF_STAT EOL <F_PROGRAM>
*	F_PROGRAM → &
*	@return Kód propagující syntaktickou správnost či chybu
*/
int func_f_program()
{
	int result;


	if(token == M_END)
	{
		return SYNTAX_OK;
	}
	result = func_f_p_def_stat();

	if (result != SYNTAX_OK) return result;

	if(eol_flag != 1)
	{
		token = scanner();
		if (token == LEX_ERROR) return LEX_ERROR;
	}
	else if(eol_flag == 1)	//Již jsem přečetl symbol EOL
	{
		eol_flag = 0;
	}

	if (token == M_EOL)	//Po definici funkcí či příkazech je odřádkování
	{
		token = scanner();
		if (token == LEX_ERROR) return LEX_ERROR;

		return func_f_program();
	}

	return SYNTAX_ERROR;	//Pokud neodpovídá derivačnímu stromu
}

/**
*	PROGRAM → P_DEF_STAT EOL <PROGRAM>
*	PROGRAM → &
*	@return Kód propagující syntaktickou správnost či chybu
*/
int func_program()
{
	int result;

	if(token == M_END)
	{
		return SYNTAX_OK;
	}

	result = func_p_def_stat();
	if (result != SYNTAX_OK) return result;

	if(eol_flag != 1)
	{
		token = scanner();
		if (token == LEX_ERROR) return LEX_ERROR;
	}
	else if(eol_flag == 1)	//Již jsem přečetl symbol EOL
	{
		eol_flag = 0;
	}

	if (token == M_EOL)	//Po definici funkcí či příkazech je odřádkování
	{
		token = scanner();
		if (token == LEX_ERROR) return LEX_ERROR;

		return func_program();
	}
	
	return SYNTAX_ERROR;	//Pokud neodpovídá derivačnímu stromu
	
}

/**
*	F_P_DEF_STAT → P_DEFINITION
*	F_P_DEF_STAT → STAT
*	F_P_DEF_STAT → STAT_RET
*	@return Kód propagující syntaktickou správnost či chybu
*/
int func_f_p_def_stat()
{
	int result;

	if (token == M_DIM)
	{
		result = func_p_definition();
		if (result != SYNTAX_OK) return result;

		return SYNTAX_OK;
	}
	else if (token == M_ID || token == M_INPUT || token == M_PRINT || token == M_IF || token == M_DO)
	{
		result = func_stat();
		if (result != SYNTAX_OK) return result;

		return SYNTAX_OK;
	}
	else if(token == M_RETURN)
	{
		result = func_stat_ret();
		if (result != SYNTAX_OK) return result;

		return SYNTAX_OK;
	}

	return SYNTAX_ERROR;	//Pokud neodpovídá derivačnímu stromu
}

/**
*	P_DEFINITION → dim id as DATATYPE_DEF <ASSIGN>
*	@return Kód propagující syntaktickou správnost či chybu
*/
int func_p_definition()
{
	int result;

	if(token == M_DIM)
	{
		token = scanner();
		if (token == LEX_ERROR) return LEX_ERROR;

		if(token == M_ID)
		{
			char *id = str_to_array(detail);
			token = scanner();
			if (token == LEX_ERROR) return LEX_ERROR;

			if(token == M_AS)
			{
				token = scanner();
				if (token == LEX_ERROR) return LEX_ERROR;

				e_dtype type = d_void;
				result = func_datatype(&type);
				if (result != SYNTAX_OK) return result;

				s_btree node;
				int resultTree = BTget(&t_symtable, id, &node);
			
				if (resultTree == 0) { // už existuje
					return SEMANTIC_ERROR;
				} else if (resultTree == 1) {
					return SYMTABLE_ERROR;
				}

				BTput(&t_symtable, id, sn_var, type, NULL);

				Ladd(&list, STcreateVar(id, type));
				token = scanner();
				if (token == LEX_ERROR) return LEX_ERROR;

				result = func_assign(id, type);
				if (result != SYNTAX_OK) return result;

				return SYNTAX_OK;
			}
		}
	}

	return SYNTAX_ERROR;
}

/**
*	ASSIGN → = expr
*	ASSIGN → &
*	@return Kód propagující syntaktickou správnost či chybu
*/
int func_assign(char *id, e_dtype type)
{
	int result;

	if(token == M_EOL) // podle typu se inicializuje na default. hodnotu
	{
		switch (type) {
			case sd_int: {
				Ladd(&list, STcreateExpr(STcreateVar(id, d_int), "asg", STcreateIntConst(0)));
				break;
			}
			case sd_double: {
				Ladd(&list, STcreateExpr(STcreateVar(id, d_double), "asg", STcreateDoubleConst(0.0)));
				break;
			}
			case sd_string: {
				Ladd(&list, STcreateExpr(STcreateVar(id, d_string), "asg", STcreateStringConst("")));
				break;
			}
			default: {
				break;
			}
		}
		
		eol_flag = 1;
		return SYNTAX_OK;
	}

	if(token == M_EQUAL)
	{
		s_stree expr_result;
		result = expr_analyse(&expr_result, 2);
		if(result != SYNTAX_OK) return result;

		if (expr_result->dtype == d_string && type != sd_string)
			return SEMANTIC_ERROR;

		Ladd(&list, STcreateExpr(STcreateVar(id, type), "asg", expr_result));

		switch (expr_result->dtype) {
			case d_int: {
				if (type == sd_double)
					Ladd(&list, STcreateInt2Float(STcreateVar(id, d_double)));
				break;
			}
			
			case d_double: {
				if (type == sd_int)
					Ladd(&list, STcreateFloat2Int(STcreateVar(id, d_int)));
				break;
			}

			case d_string: {
				break;
			}

			default: {
				return SEMANTIC_ERROR;
			}
		}

		eol_flag = 1;

		return SYNTAX_OK;
	}

	return SYNTAX_ERROR;
}

/**
*	P_DEF_STAT → P_DEFINITION
*	P_DEF_STAT → STAT
*	@return Kód propagující syntaktickou správnost či chybu
*/
int func_p_def_stat()
{
	int result;

	if (token == M_DIM)
	{
		result = func_p_definition();
		if (result != SYNTAX_OK) return result;

		return SYNTAX_OK;
	}
	else if (token == M_ID || token == M_INPUT || token == M_PRINT || token == M_IF || token == M_DO)
	{
		result = func_stat();
		if (result != SYNTAX_OK) return result;

		return SYNTAX_OK;
	}
	return SYNTAX_ERROR;	//Pokud neodpovídá derivačnímu stromu
}

/**
*	PARAM_LIST → id as DATATYPE <PARAM_N>
*	PARAM_LIST → &
*	@return Kód propagující syntaktickou správnost či chybu
*/
int func_param_list(s_stree *params)
{
	int result;

	if(token == M_RIGHT_PARANTHESE)
	{
		return SYNTAX_OK;
	}


	if(token == M_ID)	//TODO!!!ID->kontrolovat tabulku symbolů
	{
		char *id = str_to_array(detail);
		e_dtype type = 0;

		token = scanner();
		if (token == LEX_ERROR) return LEX_ERROR;

		if(token == M_AS)
		{

			token = scanner();
			if (token == LEX_ERROR) return LEX_ERROR;

			result = func_datatype(&type);
			if(result != SYNTAX_OK) return SYNTAX_ERROR;

			token = scanner();
			if (token == LEX_ERROR) return LEX_ERROR;

			*params = STcreateFirstParam(STcreateVar(id, type));

			result = func_param_n(params);
			if(result != SYNTAX_OK) return SYNTAX_ERROR;

			return SYNTAX_OK;
		}
	}
	return SYNTAX_ERROR;
}

/**
*	PARAM_N → , id as DATATYPE <PARAM_N>
*	PARAM_N → &
*	@return Kód propagující syntaktickou správnost či chybu
*/
int func_param_n(s_stree *params)
{
	int result;

	if(token == M_RIGHT_PARANTHESE)
	{
		return SYNTAX_OK;
	}

	if(token == M_COMMA)
	{
		token = scanner();
		if (token == LEX_ERROR) return LEX_ERROR;

		if(token == M_ID)	//TODO!!ID->kontrolovat tabulku symbolů	
		{
			char *id = str_to_array(detail);
			e_dtype type = 0;

			token = scanner();
			if (token == LEX_ERROR) return LEX_ERROR;

			if(token == M_AS)
			{
				token = scanner();
				if (token == LEX_ERROR) return LEX_ERROR;

				result = func_datatype(&type);
				if(result != SYNTAX_OK) return SYNTAX_ERROR;

				STaddNextParam(*params, STcreateVar(id, type));

				token = scanner();
				if (token == LEX_ERROR) return LEX_ERROR;
				
				return func_param_n(params);
			}
		}
	}
	return SYNTAX_ERROR;
}

/**
*	STAT → id = func_stat_exfu
*	STAT → input id
*	STAT → print expr ; <EXPR_N>
*	STAT → if expr then EOL <STAT_LIST_IF> <ELSE> end if 
*	STAT → do while expr EOL <STAT_LIST_WHILE> loop
*	@return Kód propagující syntaktickou správnost či chybu
*/
int func_stat()
{
	int result;
	
	//printf("%s = %d<<\n", str_to_array(detail), token);
	

	if(token == M_ID)	//TODO!!!ID->kontrolovat tabulku symbolů
	{
		char *id = str_to_array(detail);
		
		token = scanner();
		if (token == LEX_ERROR) return LEX_ERROR;


		s_btree node;
		if (BTget(&t_symtable, id, &node) != 0)
			return SEMANTIC_ERROR;

		if(token == M_EQUAL)
		{	
			token = scanner();
			if (token == LEX_ERROR) return LEX_ERROR;

			result = func_stat_exfu(id, node->dtype);
			if(result != SYNTAX_OK) return result;

			return SYNTAX_OK;
		}
	}
	else if(token == M_INPUT)
	{
		token = scanner();
		if (token == LEX_ERROR) return LEX_ERROR;

		if(token == M_ID)
		{	
			char *id = str_to_array(detail);
			s_btree node;
			int resultTree = BTget(&t_symtable, id, &node); 

			if (resultTree == -1)
				return SEMANTIC_ERROR;
			else if (resultTree == 1)
				return SYMTABLE_ERROR;

			Ladd(&list, STcreateRead(STcreateVar(id, node->dtype)));

			//token = scanner();
			//if (token == LEX_ERROR) return LEX_ERROR;

			return SYNTAX_OK;
		}
	}
	else if(token == M_PRINT)
	{
		//token = scanner();
		//if (token == LEX_ERROR) return LEX_ERROR;

		s_stree expr_result = NULL;
		token = scanner();
		if (token == LEX_ERROR) return LEX_ERROR;

		result = expr_analyse(&expr_result, 4);
		if(result != SYNTAX_OK) return result;
		Ladd(&list, STcreatePrint(expr_result));

		//token = scanner();
		//if (token == LEX_ERROR) return LEX_ERROR;

		if(token == M_SEMICOLON)
		{	
			result = func_expr_n();
			if(result != SYNTAX_OK) return SYNTAX_ERROR;

			return SYNTAX_OK;
		}
	}
	else if(token == M_IF)
	{
		//token = scanner();
		//if (token == LEX_ERROR) return LEX_ERROR;

		s_stree expr_result = NULL;
		result = expr_analyse(&expr_result, 1);
		if(result != SYNTAX_OK) return result;
		Ladd(&list, STcreateIf(expr_result));

		if(token == M_THEN)
		{
			token = scanner();
			if (token == LEX_ERROR) return LEX_ERROR;

			if(token == M_EOL)
			{
				token = scanner();
				if (token == LEX_ERROR) return LEX_ERROR;

				result = func_stat_list_if();
				if(result != SYNTAX_OK) return SYNTAX_ERROR;

				result = func_stat_else();
				if(result != SYNTAX_OK) return SYNTAX_ERROR;

				if(token == M_END)
				{
					token = scanner();
					if (token == LEX_ERROR) return LEX_ERROR;

					if(token == M_IF)
					{
						eol_flag = 0;
						Ladd(&list,STcreateEndIf());
						return SYNTAX_OK;
					}
				}
			}

		}
	}
	else if(token == M_DO)
	{
	
		token = scanner();
		if (token == LEX_ERROR) return LEX_ERROR;

		if(token == M_WHILE)
		{
			//token = scanner();
			//if (token == LEX_ERROR) return LEX_ERROR;

			s_stree expr_result = NULL;
			result = expr_analyse(&expr_result, 3);
			if (result != SYNTAX_OK) return result;
			Ladd(&list, STcreateWhile(expr_result));

			if(token == M_EOL)
			{
				token = scanner();
				if (token == LEX_ERROR) return LEX_ERROR;
				result = func_stat_list_while();
				//printf(">> %s\n", str_to_array(detail));
				if(result != SYNTAX_OK) return SYNTAX_ERROR;

				if(token == M_LOOP)
				{
					eol_flag = 0;
					Ladd(&list,STcreateEndWhile());
					return SYNTAX_OK;
				}
			}
		}
	}
	return SYNTAX_ERROR;
}

/**
*	STAT_EXFU → expr
*	STAT_EXFU → id (<IN_PARAM_LIST>)
*	@return Kód propagující syntaktickou správnost či chybu
*/
int func_stat_exfu(char *dest, e_dstype type)
{
	int result;

	s_btree node;

	if (BTget(&t_symtable, dest, &node) != 0)
		return SEMANTIC_ERROR;

	char *id;
	if(token == M_ID || token == M_INTEGER_VAL || token == M_DOUBLE_VAL || token == M_STRING_VAL)
	{
		int fce = 0;
		s_btree node2;
		if (token == M_ID) {
			id = str_to_array(detail);
			fce = BTget(&g_symtable, id, &node2) == 0;
		}

		if (fce) { // volání fce
			token = scanner();
			if (token == LEX_ERROR) return LEX_ERROR;

			if (token != M_LEFT_PARANTHESE)
				return SYNTAX_ERROR;

			s_stree params = NULL;

			result = func_in_param_list(&params, node2->params);
			if(result != SYNTAX_OK) return result;

			Ladd(&list, STcreateExpr(
				STcreateVar(dest, type),
				"asg",
				STcreateCall(id, params)
			));
			eol_flag = 0;

			return SYNTAX_OK;
		} else { // výraz

			s_stree expr_result;
			result = expr_analyse(&expr_result, 5);
			if(result != SYNTAX_OK) return result;

			if (expr_result->ntype == n_const && expr_result->dtype == d_int && node->dtype == sd_double)
				expr_result = STcreateDoubleConst(expr_result->value.v_int);

			Ladd(&list, STcreateExpr(STcreateVar(dest, node->dtype), "asg", expr_result));
	
			eol_flag = 1;

			return SYNTAX_OK;
		}
	}
	return SYNTAX_ERROR;	
}

/**
*	EXPR_N → expr ; <EXPR_N>
*	EXPR_N → &
*	@return Kód propagující syntaktickou správnost či chybu
*/
int func_expr_n()
{
	int result;
	
	s_stree expr_result = NULL;
	int tmp_token = token;

	
	//printf("1. asas");
	token = scanner();
	if (token == LEX_ERROR) return LEX_ERROR;

	if (tmp_token != M_SEMICOLON && token == M_EOL)
		return SYNTAX_ERROR;
		
	//		printf("2. asas");
	
	if (token == M_EOL) {
		eol_flag = 1;
		return SYNTAX_OK;
	}
	
	//printf(">> %s\n", str_to_array(detail));
	result = expr_analyse(&expr_result, 4);
	
	//	printf("3. asas");

	if(result != SYNTAX_OK) return result;
	
	//	printf("4. asas");

	
	//printf(">>3\n");
	Ladd(&list, STcreatePrint(expr_result));

	if(token == M_SEMICOLON)
	{
		return func_expr_n();
	}
	return SYNTAX_ERROR;
}

/**
*	STAT_LIST_ELSE → STAT EOL <STAT_LIST_ELSE>
*	STAT_LIST_ELSE → &
*	@return Kód propagující syntaktickou správnost či chybu
*/
int func_stat_list_else()
{
	int result;
	
	if(token == M_END)
	{
		return SYNTAX_OK;
	}
	else
	{
		result = func_stat();
		if(result != SYNTAX_OK) return result;

		if(eol_flag != 1) {
			token = scanner();
			if (token == LEX_ERROR) return LEX_ERROR;
		}
		
		if(token == M_EOL)
		{
			token = scanner();
			if (token == LEX_ERROR) return LEX_ERROR;

			return func_stat_list_else();
		}
	}
	return SYNTAX_ERROR;
}

/**
*	STAT_LIST_IF → STAT EOL  <STAT_LIST IF>
*	STAT_LIST_IF → &
*	@return Kód propagující syntaktickou správnost či chybu
*/
int func_stat_list_if()
{
	int result;

	if(token == M_END)
	{
		return SYNTAX_OK;
	}
	else if(token == M_ELSE)
	{
		return SYNTAX_OK;
	}
	else
	{
		result = func_stat();
		if(result != SYNTAX_OK) return result;

		if (eol_flag != 1) {
			token = scanner();
			if (token == LEX_ERROR) return LEX_ERROR;
		}

		if(token == M_EOL)
		{
			token = scanner();
			if (token == LEX_ERROR) return LEX_ERROR;

			return func_stat_list_if();
		}
	}
	return SYNTAX_ERROR;
}

/**
*	STAT_LIST_WHILE → STAT EOL <STAT_LIST_WHILE>
*	STAT_LIST_WHILE → &
*	@return Kód propagující syntaktickou správnost či chybu
*/
int func_stat_list_while()
{
	int result;

	if(token == M_LOOP)
	{
		return SYNTAX_OK;
	}
	
	//printf(">list_white< %s\n", str_to_array(detail));
	result = func_stat();
	if(result != SYNTAX_OK) return result;

	if (token != M_EOL) {
		token = scanner(); // < něco
		if (token == LEX_ERROR) return LEX_ERROR;
	}

	if(token == M_EOL)
	{
		token = scanner();
		if (token == LEX_ERROR) return LEX_ERROR;

		return func_stat_list_while();
	}
	return SYNTAX_ERROR;
}

/**
*	ELSE → else EOL <STAT_LIST_IF>
*	ELSE → &
*	@return Kód propagující syntaktickou správnost či chybu
*/
int func_stat_else()
{
	int result;
	
	if(token == M_END)
	{
		return SYNTAX_OK;
	}

	if(token == M_ELSE)
	{
		Ladd(&list,STcreateElse());

		token = scanner();
		if (token == LEX_ERROR) return LEX_ERROR;

		if(token == M_EOL)
		{
			token = scanner();
			if (token == LEX_ERROR) return LEX_ERROR;

			result = func_stat_list_else();
			if(result != SYNTAX_OK) return SYNTAX_ERROR;

			return SYNTAX_OK;
		}
	}
	return SYNTAX_ERROR;
}

/**
*	STAT_RET → return expr
*	STAT_RET → &
*	@return Kód propagující syntaktickou správnost či chybu
*/
int func_stat_ret()
{
	int result;

	if(token == M_RETURN)
	{

		s_stree expr_result = NULL;
		result = expr_analyse(&expr_result, 2);
		if(result != SYNTAX_OK) return result;

		Ladd(&list, STcreateReturn(expr_result));

		eol_flag = 1;

		return SYNTAX_OK;
	}
	return SYNTAX_ERROR;
}

/**
*	IN_PARAM_LIST → term <IN_PARAM_N>
*	@return Kód propagující syntaktickou správnost či chybu
*/
int func_in_param_list(s_stree *params, s_stree f_params)
{
	int result;

	token = scanner();
	if (token == LEX_ERROR) return LEX_ERROR;

	if (token == M_RIGHT_PARANTHESE) {
		if (f_params != NULL)
			return SEMANTIC_ERROR;

		return SYNTAX_OK;
	}

	if(token == M_INTEGER_VAL || token == M_DOUBLE_VAL || token == M_STRING_VAL)
	{
		switch (token) {
			case M_INTEGER_VAL: {
				if (f_params->lptr->dtype == d_string) return SEMANTIC_ERROR;
				if (f_params->lptr->dtype == d_double)
					*params = STcreateFirstParam(STcreateDoubleConst(strToDouble(str_to_array(detail))));
				else
					*params = STcreateFirstParam(STcreateIntConst(strToInt(str_to_array(detail)))); 
				break;
			}
			case M_DOUBLE_VAL: {
				if (f_params->lptr->dtype == d_string) return SEMANTIC_ERROR;
				*params = STcreateFirstParam(STcreateDoubleConst(strToDouble(str_to_array(detail)))); 
				break;
			}
			case M_STRING_VAL: {
				if (f_params->lptr->dtype != d_string) return SEMANTIC_ERROR;
				*params = STcreateFirstParam(STcreateStringConst(str_to_array(detail))); 
				break;
			}
		}

		result = func_in_param_n(params, f_params->rptr);
		if(result != SYNTAX_OK) return result;

		return SYNTAX_OK;
	}
	else if(token == M_ID)
	{
		char *id = str_to_array(detail);

		s_btree node;
		int r = BTget(&t_symtable, id, &node);

		if (r != 0 || (node->dtype == sd_string && f_params->lptr->dtype != d_string) || (node->dtype != sd_string && f_params->lptr->dtype == d_string)) // string nelze převáděť -> chyba
			return SEMANTIC_ERROR;

		*params = STcreateFirstParam(STcreateVar(id, node->dtype));

		result = func_in_param_n(params, f_params->rptr);
		if(result != SYNTAX_OK) return result;

		return SYNTAX_OK;
	}
	return SYNTAX_ERROR;
}

/**
*	IN_PARAM_N → , term <IN_PARAM_N>
*	IN_PARAM_N → &
*	@return Kód propagující syntaktickou správnost či chybu
*/
int func_in_param_n(s_stree *params, s_stree f_params)
{	

	token = scanner();
	if (token == LEX_ERROR) return LEX_ERROR;

	if(token == M_RIGHT_PARANTHESE)
	{
		if (f_params != NULL)
			return SEMANTIC_ERROR;
		
		return SYNTAX_OK;
	}
	
	if(token == M_COMMA)
	{
		token = scanner();
		if (token == LEX_ERROR) return LEX_ERROR;

		if(token == M_INTEGER_VAL || token == M_DOUBLE_VAL || token == M_STRING_VAL )
		{
			switch (token) {
				case M_INTEGER_VAL: {
					if (f_params->lptr->dtype == d_string) return SEMANTIC_ERROR;
					if (f_params->lptr->dtype == d_double)
						STaddNextParam(*params, STcreateDoubleConst(strToDouble(str_to_array(detail)))); 
					else
						STaddNextParam(*params, STcreateIntConst(strToInt(str_to_array(detail)))); 
					break;
				}

				case M_DOUBLE_VAL: {
					if (f_params->lptr->dtype == d_string) return SEMANTIC_ERROR;
					STaddNextParam(*params, STcreateDoubleConst(strToDouble(str_to_array(detail)))); 
					break;
				}

				case M_STRING_VAL: {
					if (f_params->lptr->dtype != d_string) return SEMANTIC_ERROR;
					STaddNextParam(*params, STcreateStringConst(str_to_array(detail))); 
					break;
				}
			}

			token = scanner();
			if (token == LEX_ERROR) return LEX_ERROR;

			return func_in_param_n(params, f_params->rptr);
		}
		else if(token == M_ID)
		{
			char *id = str_to_array(detail);
			token = scanner();		
			if (token == LEX_ERROR) return LEX_ERROR;
	
			s_btree node;
			int r = BTget(&t_symtable, id, &node);
	
			if (r != 0 || (node->dtype == sd_string && f_params->lptr->dtype != d_string) || (node->dtype != sd_string && f_params->lptr->dtype == d_string)) // string nelze převáděť -> chyba
				return SEMANTIC_ERROR;

			STaddNextParam(*params, STcreateVar(id, node->dtype));
			//!!!Kontrola ID v tabulce symbolů
			token = scanner();
			if (token == LEX_ERROR) return LEX_ERROR;
			
			return func_in_param_n(params, f_params->rptr);
		}
	}
	return SYNTAX_ERROR;
}

int expr_recurse(s_stree *node, Pexpression expr, int i) {
	
	if (i < 0 || expr->elements[expr->number - 1] == NULL)
		return SYNTAX_OK;


	if (expr->types[i] != 0) {
		switch (expr->types[i]) {
			case M_INTEGER_VAL: {
				*node = STcreateIntConst(strToInt(str_to_array(expr->elements[i])));
				break;
			}

			case M_DOUBLE_VAL: {
				*node = STcreateDoubleConst(strToDouble(str_to_array(expr->elements[i])));
				break;
			}

			case M_STRING_VAL: {
				*node = STcreateStringConst(str_to_array(expr->elements[i]));
				break;
			}

			case M_ID: {
				s_btree vnode;
				BTget(&t_symtable, str_to_array(expr->elements[i]), &vnode);
				*node = STcreateVar(str_to_array(expr->elements[i]), vnode->dtype);
				break;
			}
		}

		removeElement(expr, i);		
	} else {
		char *oper = str_to_array(expr->elements[i]);
		*node = STcreateExpr(STcreateIntConst(1), oper, STcreateIntConst(1));
		s_stree left, right;
		expr_recurse(&right, expr, i - 1);
		(*node)->rptr = right;

		expr_recurse(&left, expr, i - 1);
		(*node)->lptr = left;	

		if ((left->dtype == d_string && right->dtype != d_string) || (left->dtype != d_string && right->dtype == d_string)) { // u stringu neexistuje konverze
			return SEMANTIC_ERROR;
		}

		if (left->dtype == d_string && strcmp(oper, "+") != 0) // operace pro string je pouze '+' / konkatenace
			return SEMANTIC_ERROR;

		if (isLoginOperation(oper))
			(*node)->dtype = d_bool;
		else if (strcmp(oper, "\\") == 0)
			(*node)->dtype = d_int;
		else if (strcmp(oper, "/") == 0)
			(*node)->dtype = d_double;
		else if (left->dtype == d_string)
			(*node)->dtype = d_string;
		else {
			if (left->dtype == d_double || right->dtype == d_double)
				(*node)->dtype = d_double;
			else
				(*node)->dtype = d_int;
		}

		removeElement(expr, i);
	}

	return SYNTAX_OK;
}

void print_stromecek(s_stree node) {

	if (node == NULL) return;
	
	if (node->ntype == n_var) {
		printf("> %s (var) \n", node->value.v_string);
	} else if (node->ntype == n_const) {
		switch (node->dtype) {
			case d_double: printf("> %f (f)\n", node->value.v_double); break;
			case d_string: printf("> %s (s)\n", node->value.v_string); break;
			case d_int: printf("> %d (int)\n", node->value.v_int); break;
			case d_undef: printf("> %s (undef)\n", node->value.v_string); break;
			default: break;
		}
	}

	printf("\n");
	print_stromecek(node->rptr);
	print_stromecek(node->lptr);

}

/**
*	Funkce zpracující výrazy a určující pořadí jejich vyhodnocení
*	@return Kód propagující syntaktickou správnost či chybu
*/
int expr_analyse(s_stree *node, int f)
{
	Pexpression expr;
	int result = infixToPostfix(&expr, f);


	if (result != POSTFIX_OK) return result;

/*
	for (int i = 0; i < expr->number; i++)
		printf("%s", str_to_array(expr->elements[i]));

	printf("\n\n");*/

	return expr_recurse(node, expr, expr->number - 1);

	//print_stromecek(*node); exit(0);
}

/*
TODO: Nutno upravit hlavní volání parseru
*/
/**
*	Hlavní volání syntaktického analyzátoru
*	@return Kód propagující syntaktickou správnost či chybu
*/
int parse()
{
	int result;
	s_list list;
	Linit(&list);
	BTinit(&g_symtable);

	if ((token = scanner()) == LEX_ERROR)
	{
		 result = LEX_ERROR;	// nastala chyba jiz pri nacteni prvniho lexemu
	}
	else
	{
		 result = func_start();		// volám sfunkci pro start analýzy
	}

	BTdispose(&g_symtable);

	return result;
}