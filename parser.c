/**
 * Projekt - Tým 097, varianta I
 * Autor: Tichavský Miroslav (xticha04), Tom Barbořík (xbarbo06), Pavel Kaleta (xkalet05)
 * Parser zpracovávající výstup lexikální analýzy a ověřující jeho syntaktickou a sémantickou správnost   
 */

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

char *fce_id;
e_dtype fce_type;

int in_scope = 0;

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

	// vložení vestavěných fcí
	BTput(&g_symtable, "length", sn_func_dec_def, sd_int, STcreateFirstParam(STcreateVar("s", d_string)));
	BTput(&g_symtable, "chr", sn_func_dec_def, sd_string, STcreateFirstParam(STcreateVar("i", d_int)));

	s_stree s_params = STcreateFirstParam(STcreateVar("s", d_string));
	STaddNextParam(s_params, STcreateVar("i", d_int));
	STaddNextParam(s_params, STcreateVar("n", d_int));
	BTput(&g_symtable, "substr", sn_func_dec_def, sd_string, s_params);

	s_stree a_params = STcreateFirstParam(STcreateVar("s", d_string));
	STaddNextParam(a_params, STcreateVar("i", d_int));
	BTput(&g_symtable, "asc", sn_func_dec_def, sd_int, a_params);

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

			in_scope = 1;
			result = func_program();
			if (result != SYNTAX_OK) return result;
			in_scope = 0;

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
		
			if(token == M_ID)	
			{
				char *id = str_to_array(detail);
				s_btree node;
				int resultTree = BTget(&g_symtable, id, &node);

				if (resultTree == 0) { // redeklarace
					return SEMANTIC_ERROR_1;
				} else if (resultTree == 1) {
					return SYMTABLE_ERROR;
				}


				s_stree params = NULL;
				e_dtype returnType = d_undef;
				BTput(&g_symtable, id, sn_func_dec, returnType, params);
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

							BTput(&g_symtable, id, sn_func_dec, returnType, params);

							return SYNTAX_OK;
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
		
		if(token == M_ID)
		{			

			char *id = str_to_array(detail);			
			e_dtype type = d_void;
			s_stree params = NULL;			

			s_btree node;
			int resultTree = BTget(&g_symtable, id, &node);
			if (resultTree == 0 && node->ntype == sn_func_dec_def) { // redefinice funkce
				return SEMANTIC_ERROR_1;
			} else if (resultTree == -1) { // deklarace + definice
				BTput(&g_symtable, id, sn_func_dec_def, type, params);
			}

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

							if (resultTree == -1) {
								BTput(&g_symtable, id, sn_func_dec_def, type, params);
							}

							BTinit(&t_symtable);
							s_stree t_params = params;
							while(t_params != NULL) {
								if (BTput(&t_symtable, t_params->lptr->value.v_string, sn_var, t_params->lptr->dtype, NULL) != 0)
									return SYMTABLE_ERROR;

								t_params = t_params->rptr;
							}

							fce_id = id;
							fce_type = type;
							result = func_f_program(id, type);
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
		
									if (resultTree == -1 || (resultTree == 0 && node->ntype == sn_func_dec)) { // nebyl nalezen nebo byl nalezen ale jenom deklarovan
										if (resultTree == 0) {
											s_stree dec_params = node->params;
											s_stree t_params = params;


											while(dec_params != NULL || t_params != NULL) {
												
												if ((dec_params == NULL && t_params != NULL) || (t_params == NULL && dec_params != NULL)) { // nesouhlasný počet parametrů
													return SEMANTIC_ERROR_1;
												}

												if (dec_params->lptr->dtype != t_params->lptr->dtype) { // nesouhlasný dat. typ parametrů
													return SEMANTIC_ERROR_1;
												}

												dec_params = dec_params->rptr;
												t_params = t_params->rptr;
											}


											if (node->dtype != type) // nesouhlasné datové type pro návrat (dec / def)
												return SEMANTIC_ERROR_1;

											BTput(&g_symtable, id, sn_func_dec_def, type, node->params);

										} else {
											BTput(&g_symtable, id, sn_func_dec_def, type, params);
										}
										return SYNTAX_OK;
									 
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
int func_f_program(char *id, e_dtype type)
{
	int result;


	if(token == M_END)
	{
		return SYNTAX_OK;
	}
	result = func_f_p_def_stat(id, type);

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

		return func_f_program(id, type);
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
int func_f_p_def_stat(char *id, e_dtype type)
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
		result = func_stat_ret(id, type);
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

			s_btree node, node_g;
			int resultTree = BTget(&t_symtable, id, &node);
			int resultGlobal = BTget(&g_symtable, id, &node_g);
		
			if (resultTree == 0 || resultGlobal == 0) { // redefinice proměnné, existuje fce s daným názvem
				return SEMANTIC_ERROR_1;
			} else if (resultTree == 1 || resultGlobal == 1) {
				return SYMTABLE_ERROR;
			}

			token = scanner();
			if (token == LEX_ERROR) return LEX_ERROR;

			if(token == M_AS)
			{
				token = scanner();
				if (token == LEX_ERROR) return LEX_ERROR;

				e_dtype type = d_void;
				result = func_datatype(&type);
				if (result != SYNTAX_OK) return result;

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
		if (expr_result == NULL) return SYNTAX_ERROR;

		if ((expr_result->dtype == d_string && type != sd_string) || (type == sd_string && expr_result->dtype != d_string)) // nesouhlasné typy
			return SEMANTIC_ERROR_2;

		Ladd(&list, STcreateExpr(STcreateVar(id, type), "asg", expr_result));

		switch (expr_result->dtype) {
			case d_int: {
				if (type == sd_double) {
					Ladd(&list, STcreateInt2Float(STcreateVar(id, d_double)));
					BTput(&t_symtable, id, sn_var, sd_double, NULL);
				}
				break;
			}
			
			case d_double: {
				if (type == sd_int) {
					Ladd(&list, STcreateFloat2Int(STcreateVar(id, d_int)));
					BTput(&t_symtable, id, sn_var, sd_int, NULL);
				}
				break;
			}

			case d_string: {
				break;
			}

			default: {
				return SEMANTIC_ERROR_2; // typy
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

	if(token == M_ID)
	{
		char *id = str_to_array(detail);

		s_btree gnode;
		int ret = BTget(&g_symtable, id, &gnode);

		if (ret == 0) { // název parametru == název funkce
			return SEMANTIC_ERROR_1;
		} else if (ret == 1) {
			return SYMTABLE_ERROR;
		}

		e_dtype type = 0;

		token = scanner();
		if (token == LEX_ERROR) return LEX_ERROR;

		if(token == M_AS)
		{

			token = scanner();
			if (token == LEX_ERROR) return LEX_ERROR;

			result = func_datatype(&type);
			if(result != SYNTAX_OK) return result;

			*params = STcreateFirstParam(STcreateVar(id, type));
			
			token = scanner();
			if (token == LEX_ERROR) return LEX_ERROR;

			result = func_param_n(params);
			if(result != SYNTAX_OK) return result;

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

			s_btree gnode;
			int ret = BTget(&g_symtable, id, &gnode);

			if (ret == 0) { // název parametru == název funkce
				return SEMANTIC_ERROR_1;
			} else if (ret == 1) {
				return SYMTABLE_ERROR;
			}

			s_stree pnode = *params;
			while (pnode != NULL) {
				if (strcmp(pnode->lptr->value.v_string, id) == 0) {
					return SEMANTIC_ERROR_1;
				}

				pnode = pnode->rptr;
			}

			token = scanner();
			if (token == LEX_ERROR) return LEX_ERROR;

			if(token == M_AS)
			{
				token = scanner();
				if (token == LEX_ERROR) return LEX_ERROR;

				result = func_datatype(&type);
				if(result != SYNTAX_OK) return result;

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
	

	if(token == M_ID)
	{
		char *id = str_to_array(detail);
		
		token = scanner();
		if (token == LEX_ERROR) return LEX_ERROR;


		s_btree node;
		if (BTget(&t_symtable, id, &node) != 0) // neexistující id
			return SEMANTIC_ERROR_1;

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

			if (resultTree == -1) // neexistující id
				return SEMANTIC_ERROR_1;
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
		
		if (expr_result == NULL) return SYNTAX_ERROR;
		Ladd(&list, STcreatePrint(expr_result));

		//token = scanner();
		//if (token == LEX_ERROR) return LEX_ERROR;

		if(token == M_SEMICOLON)
		{	
			result = func_expr_n();
			if(result != SYNTAX_OK) return result;

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
		if (expr_result == NULL) return SYNTAX_ERROR;
		if (expr_result->dtype != d_bool) return SEMANTIC_ERROR_2;
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
				if(result != SYNTAX_OK) return result;

				result = func_stat_else();
				if(result != SYNTAX_OK) return result;

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
			if (expr_result == NULL) return SYNTAX_ERROR;
			if (expr_result->dtype != d_bool) return SEMANTIC_ERROR_2;
			Ladd(&list, STcreateWhile(expr_result));

			if(token == M_EOL)
			{
				token = scanner();
				if (token == LEX_ERROR) return LEX_ERROR;
				result = func_stat_list_while();
				//printf(">> %s\n", str_to_array(detail));
				if(result != SYNTAX_OK) return result;

				if(token == M_LOOP)
				{
					eol_flag = 0;
					Ladd(&list,STcreateEndWhile());
					return SYNTAX_OK;
				}
			}
		}
	} else if (token == M_RETURN && in_scope == 0) {
		result = func_stat_ret(fce_id, fce_type);
		if (result != SYNTAX_OK) return result;

		return SYNTAX_OK;
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

	if (BTget(&t_symtable, dest, &node) != 0) // neexistující proměnná (cíl)
		return SEMANTIC_ERROR_1;

	char *id;
	if(token == M_ID || token == M_INTEGER_VAL || token == M_DOUBLE_VAL || token == M_STRING_VAL || token == M_CHR || token == M_ASC || token == M_LENGTH || token == M_SUBSTR || token == M_LEFT_PARANTHESE)
	{
		int fce = 0;
		s_btree node2;
		if (token == M_ID || token == M_LENGTH || token == M_CHR || token == M_ASC || token == M_SUBSTR) {
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

			switch (node2->dtype) {
				case sd_int: {
					if (type == d_string)
						return SEMANTIC_ERROR_2; // nesouhlasný typ
					if (type == d_double) {
						Ladd(&list, STcreateInt2Float(STcreateVar(dest, type)));
						BTput(&t_symtable, dest, sn_var, sd_double, NULL);
					}

					break;
				}

				case sd_double: {
					if (type == d_string)
						return SEMANTIC_ERROR_2; // nesouhlasný typ
					if (type == d_int) {
						Ladd(&list, STcreateFloat2Int(STcreateVar(dest, type)));
						BTput(&t_symtable, dest, sn_var, sd_int, NULL);
					}
					break;
				}

				case sd_string: {
					if (type != d_string)
						return SEMANTIC_ERROR_2; // nesouhlasný typ
					break;
				}

				default: {
					return SEMANTIC_ERROR_2; // nesouhlasný typ
				}
			}

			eol_flag = 0;

			return SYNTAX_OK;
		} else { // výraz

			s_stree expr_result;
			result = expr_analyse(&expr_result, 5);
			if(result != SYNTAX_OK) return result;
			if (expr_result == NULL) return SYNTAX_ERROR;

			if (expr_result->ntype == n_const && expr_result->dtype == d_int && node->dtype == sd_double)
				expr_result = STcreateDoubleConst(expr_result->value.v_int);

			Ladd(&list, STcreateExpr(STcreateVar(dest, node->dtype), "asg", expr_result));
	
			switch (expr_result->dtype) {
				case d_int: {
					if (node->dtype == sd_string)
						return SEMANTIC_ERROR_2; // nesouhlasný typ
					if (node->dtype == sd_double) {
						Ladd(&list, STcreateInt2Float(STcreateVar(dest, node->dtype)));
						BTput(&t_symtable, dest, sn_var, sd_double, NULL);
					}

					break;
				}

				case d_double: {
					if (node->dtype == sd_string)
						return SEMANTIC_ERROR_2; // nesouhlasný typ
					if (node->dtype == sd_int) {
						Ladd(&list, STcreateFloat2Int(STcreateVar(dest, node->dtype)));
						BTput(&t_symtable, dest, sn_var, sd_int, NULL);
					}
					break;
				}

				case d_string: {
					if (node->dtype != sd_string)
						return SEMANTIC_ERROR_2; // nesouhlasný typ
					break;
				}

				default: {
					return SEMANTIC_ERROR_2; // nesouhlasný typ
				}
			}

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
		
	
	if (token == M_EOL) {
		eol_flag = 1;
		return SYNTAX_OK;
	}
	
	result = expr_analyse(&expr_result, 4);
	if(result != SYNTAX_OK) return result;
	if (expr_result == NULL)
		return SYNTAX_ERROR;

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
			if(result != SYNTAX_OK) return result;

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
int func_stat_ret(char *id, e_dtype type)
{
	int result;
	(void) id;

	if(token == M_RETURN)
	{

		s_stree expr_result = NULL;
		result = expr_analyse(&expr_result, 2);
		if(result != SYNTAX_OK) return result;
		if (expr_result == NULL) return SYNTAX_ERROR;

		if ((expr_result->dtype == d_string && type != sd_string) || (expr_result->dtype != d_string && type == sd_string)) // nesouhlasný typ
			return SEMANTIC_ERROR_2;
		
		switch(expr_result->dtype) {
			case d_int: {
				if (expr_result->ntype == n_const && type == sd_double) expr_result = STcreateDoubleConst(expr_result->value.v_int);
				if (expr_result->ntype == n_var && type == sd_double) {
					Ladd(&list, STcreateInt2Float(STcreateVar(expr_result->value.v_string, expr_result->dtype)));
				}
				break;
			}

			case d_double: {
				if (expr_result->ntype == n_var && type == sd_int)  {
					Ladd(&list, STcreateFloat2Int(STcreateVar(expr_result->value.v_string, expr_result->dtype)));
				}
				break;
			}

			case d_string: {
				break;
			}
			default: {
				return SEMANTIC_ERROR_2; // nesouhlasný typ
			}
		}

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
			return SEMANTIC_ERROR_2; // špatný počet parametrů 

		return SYNTAX_OK;
	}


	if(token == M_INTEGER_VAL || token == M_DOUBLE_VAL || token == M_STRING_VAL)
	{
		switch (token) {
			case M_INTEGER_VAL: {
				if (f_params->lptr->dtype == d_string) return SEMANTIC_ERROR_2; // špatný typ
				if (f_params->lptr->dtype == d_double)
					*params = STcreateFirstParam(STcreateDoubleConst(strToDouble(str_to_array(detail))));
				else
					*params = STcreateFirstParam(STcreateIntConst(strToInt(str_to_array(detail)))); 
				break;
			}
			case M_DOUBLE_VAL: {
				if (f_params->lptr->dtype == d_string) return SEMANTIC_ERROR_2; // špatný typ
				*params = STcreateFirstParam(STcreateDoubleConst(strToDouble(str_to_array(detail)))); 
				break;
			}
			case M_STRING_VAL: {
				if (f_params->lptr->dtype != d_string) return SEMANTIC_ERROR_2; // špatný typ
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

		if (r != 0) // neexistující proměnná
			return SEMANTIC_ERROR_1;
		else if ((node->dtype == sd_string && f_params->lptr->dtype != d_string) || (node->dtype != sd_string && f_params->lptr->dtype == d_string)) // špatný typ
			return SEMANTIC_ERROR_2;

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
			return SEMANTIC_ERROR_2; // špatný počet parametrů
		
		return SYNTAX_OK;
	}
	
	if(token == M_COMMA)
	{
		if (f_params == NULL)
			return SEMANTIC_ERROR_2;

		token = scanner();
		if (token == LEX_ERROR) return LEX_ERROR;

		if(token == M_INTEGER_VAL || token == M_DOUBLE_VAL || token == M_STRING_VAL )
		{

			switch (token) {
				case M_INTEGER_VAL: {
					if (f_params->lptr->dtype == d_string) return SEMANTIC_ERROR_2; // špatný typ
					if (f_params->lptr->dtype == d_double)
						STaddNextParam(*params, STcreateDoubleConst(strToDouble(str_to_array(detail)))); 
					else
						STaddNextParam(*params, STcreateIntConst(strToInt(str_to_array(detail)))); 
					break;
				}

				case M_DOUBLE_VAL: {
					if (f_params->lptr->dtype == d_string) return SEMANTIC_ERROR_2; // špatný typ
					STaddNextParam(*params, STcreateDoubleConst(strToDouble(str_to_array(detail)))); 
					break;
				}

				case M_STRING_VAL: {
					if (f_params->lptr->dtype != d_string) return SEMANTIC_ERROR_2; // špatný typ
					STaddNextParam(*params, STcreateStringConst(str_to_array(detail))); 
					break;
				}
			}

			return func_in_param_n(params, f_params->rptr);
		}
		else if(token == M_ID)
		{
			
			char *id = str_to_array(detail);
	
			s_btree node;
			int r = BTget(&t_symtable, id, &node);
	
			if (r != 0) // neexistující proměnná
				return SEMANTIC_ERROR_1;
			else if ((node->dtype == sd_string && f_params->lptr->dtype != d_string) || (node->dtype != sd_string && f_params->lptr->dtype == d_string)) // špatný typ
				return SEMANTIC_ERROR_2;

			STaddNextParam(*params, STcreateVar(id, node->dtype));
	
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
			return SEMANTIC_ERROR_2;
		}

		if (left->dtype == d_string && strcmp(oper, "+") != 0) // operace pro string je pouze '+' / logicka operace
			if (!isLoginOperation(oper))
				return SEMANTIC_ERROR_2;

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

void print_stromecek(s_stree node, int l) {

	if (node == NULL) return;
	
	for (int i = 0; i < l; i++) {
		printf("\t");
	}

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
	} else if (node->ntype == n_expr) {
		printf("> %s (e -> %d)\n", node->value.v_string, node->dtype);
	}

	print_stromecek(node->rptr, l+1);
	print_stromecek(node->lptr, l+1);

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
	if (expr->number == 0) return SYNTAX_ERROR;

	int r = expr_recurse(node, expr, expr->number - 1);
	free(expr->elements);
	free(expr->types);
	//print_stromecek(*node, 0); exit(0);
	return r;

}

int allDefined(s_btree node) {
	if (node == NULL)
		return 1;

	return node->ntype == sn_func_dec_def && allDefined(node->lptr) && allDefined(node->rptr);
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

	if(!allDefined(g_symtable)) {
		return SEMANTIC_ERROR_1;
	}


	BTdispose(&g_symtable);

	return result;
}