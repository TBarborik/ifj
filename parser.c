/*
TODO: Nutno vybrat správné hlavičkové soubory
*/
#include "parser.h"

#define SYMTABLE_ERROR 41

/*
TODO: Nutno doimplementovat precedenční analýzu
*/
int prec_analyse();
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

				result = func_assign();
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
int func_assign()
{
	int result;

	if(token == M_EOL)
	{
		eol_flag = 1;
		return SYNTAX_OK;
	}

	if(token == M_EQUAL)
	{
		token = scanner();
		if (token == LEX_ERROR) return LEX_ERROR;

		s_stree prec_result;
		result = prec_analyse(&prec_result);
		if(result != SYNTAX_OK) return SYNTAX_ERROR;

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

	if(token == M_ID)	//TODO!!!ID->kontrolovat tabulku symbolů
	{
		token = scanner();
		if (token == LEX_ERROR) return LEX_ERROR;

		if(token == M_EQUAL)
		{	
			token = scanner();
			if (token == LEX_ERROR) return LEX_ERROR;

			result = func_stat_exfu();
			if(result != SYNTAX_OK) return SYNTAX_ERROR;

			return SYNTAX_OK;
		}
	}
	else if(token == M_INPUT)
	{
		token = scanner();
		if (token == LEX_ERROR) return LEX_ERROR;

		if(token == M_ID)	//TODO!!!ID->kontrolovat tabulku symbolů
		{	
			char *id = str_to_array(detail);
			s_btree node;
			int resultTree = BTget(&t_symtable, id, &node); // TODO

			if (resultTree == -1)
				return SEMANTIC_ERROR;
			else if (resultTree == 1)
				return SYMTABLE_ERROR;

			Ladd(&list, STcreateRead(STcreateVar(id, node->dtype)));

			return SYNTAX_OK;
		}
	}
	else if(token == M_PRINT)
	{
		//token = scanner();
		//if (token == LEX_ERROR) return LEX_ERROR;

		s_stree prec_result = NULL;

		result = prec_analyse(&prec_result);
		if(result != SYNTAX_OK) return SYNTAX_ERROR;

		token = scanner();
		if (token == LEX_ERROR) return LEX_ERROR;

		if(token == M_SEMICOLON)
		{	
			token = scanner();
			if (token == LEX_ERROR) return LEX_ERROR;

			result = func_expr_n();
			if(result != SYNTAX_OK) return SYNTAX_ERROR;

			return SYNTAX_OK;
		}
	}
	else if(token == M_IF)
	{
		//token = scanner();
		//if (token == LEX_ERROR) return LEX_ERROR;

		s_stree prec_result = NULL;
		result = prec_analyse(&prec_result);
		if(result != SYNTAX_OK) return SYNTAX_ERROR;

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

			s_stree prec_result = NULL;
			result = prec_analyse(&prec_result);

			if(token == M_EOL)
			{
				token = scanner();
				if (token == LEX_ERROR) return LEX_ERROR;

				result = func_stat_list_while();
				if(result != SYNTAX_OK) return SYNTAX_ERROR;

				if(token == M_LOOP)
				{
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
int func_stat_exfu()
{
	int result;

	char *id;

	if(token == M_ID)	//TODO!!!ID->kontrolovat tabulku symbolů
	{
		id = str_to_array(detail);
		
		//TODO - ověřit, jestli se jedná o správné použití knihovny STRING
		detail_act = str_new();  //Uložím si detaily aktuálního tokenu, pro případné použití v precedenční analýze
		str_put(detail_act,str_to_array(detail));

		token_n = scanner();	//Podívám se na další token, podle kterého rozhodnu jestli se jedná o volání funkce nebo přiřazení
		if (token_n == LEX_ERROR) return LEX_ERROR;

		if(token_n == M_LEFT_PARANTHESE)	//Pokud je dalším tokenem  levá závorka, tak se jedná o volání funkce
		{	
			token_n = 0;
			str_clear(detail_act);

			token = scanner();
			if (token == LEX_ERROR) return LEX_ERROR;
			
			result = func_in_param_list();
			if(result != SYNTAX_OK) return SYNTAX_ERROR;

			if(token == M_RIGHT_PARANTHESE)
			{
				return SYNTAX_OK;
			}
		}
		else	//V opačném případě předávám řízení precedenční analýze
		{	
			token_next_flag = 1;  //Pamatuji si, že mám již načteny 2 tokeny a musím nejdřív obsloužit je
								  //První token je v token a druhý v token_n
								  //Detaily o nich jsou v detail_act a detail
			
			s_stree prec_result = NULL;
			result = prec_analyse(&prec_result);
			if (result != SYNTAX_OK) return SYNTAX_ERROR;

			token_n = 0;
			token_next_flag = 0;
			str_clear(detail_act);

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

	if(token == M_EOL)
	{
		eol_flag = 1;
		return SYNTAX_OK;
	}
	
	s_stree prec_result = NULL;
	result = prec_analyse(&prec_result);
	if(result != SYNTAX_OK) return SYNTAX_ERROR;

	if(token == M_SEMICOLON)
	{
		token = scanner();
		if (token == LEX_ERROR) return LEX_ERROR;

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
		if(result != SYNTAX_OK) return SYNTAX_ERROR;

		token = scanner();
		if (token == LEX_ERROR) return LEX_ERROR;
		
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
		if(result != SYNTAX_OK) return SYNTAX_ERROR;

		token = scanner();
		if (token == LEX_ERROR) return LEX_ERROR;

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
	
	result = func_stat();
	if(result != SYNTAX_OK) return SYNTAX_ERROR;

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
		token = scanner();
		if (token == LEX_ERROR) return LEX_ERROR;

		s_stree prec_result = NULL;
		result = prec_analyse(&prec_result);
		if(result != SYNTAX_OK) return SYNTAX_ERROR;

		return SYNTAX_OK;
	}
	return SYNTAX_ERROR;
}

/**
*	IN_PARAM_LIST → term <IN_PARAM_N>
*	@return Kód propagující syntaktickou správnost či chybu
*/
int func_in_param_list()
{
	int result;

	if(token == (M_INTEGER_VAL || M_DOUBLE_VAL || M_STRING_VAL ))
	{
		token = scanner();
		if (token == LEX_ERROR) return LEX_ERROR;

		result = func_in_param_n();
		if(result != SYNTAX_OK) return SYNTAX_ERROR;

		return SYNTAX_OK;
	}
	else if(token == M_ID)
	{
		token = scanner();		//!!!Kontrola ID v tabulce symbolů
		if (token == LEX_ERROR) return LEX_ERROR;

		result = func_in_param_n();
		if(result != SYNTAX_OK) return SYNTAX_ERROR;

		return SYNTAX_OK;
	}
	return SYNTAX_ERROR;
}

/**
*	IN_PARAM_N → , term <IN_PARAM_N>
*	IN_PARAM_N → &
*	@return Kód propagující syntaktickou správnost či chybu
*/
int func_in_param_n()
{	
	if(token == M_RIGHT_PARANTHESE)
	{
		return SYNTAX_OK;
	}
	
	if(token == M_COMMA)
	{
		token = scanner();
		if (token == LEX_ERROR) return LEX_ERROR;

		if(token == (M_INTEGER_VAL || M_DOUBLE_VAL || M_STRING_VAL ))
		{
			token = scanner();
			if (token == LEX_ERROR) return LEX_ERROR;

			return func_in_param_n();
		}
		else if(token == M_ID)
		{
			//!!!Kontrola ID v tabulce symbolů
			token = scanner();
			if (token == LEX_ERROR) return LEX_ERROR;
			
			return func_in_param_n();
		}
	}
	return SYNTAX_ERROR;
}

//TODO- Precedenční analýza
/**
*	Funkce zpracující výrazy a určující pořadí jejich vyhodnocení
*	@return Kód propagující syntaktickou správnost či chybu
*/
int prec_analyse(s_stree *node)
{
	Pexpression expr;
	int result = infixToPostfix(&expr);

	printf("%d\n", result);
	if (result != POSTFIX_OK) return result;

	printf("%d\n", expr->number);
	for (int i = 0; i < expr->number; i++) {
		printf("%s", str_to_array(expr->elements[i]));
	}
	exit(1);

	return SYNTAX_OK;
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
//Inicializace tabulky symbolů a derivačního stromu/může být i v mainu
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