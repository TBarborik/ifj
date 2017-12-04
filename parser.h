/**
 * Projekt - Tým 097, varianta I
 * Autor: Tichavský Miroslav (xticha04), Tom Barbořík (xbarbo06), Pavel Kaleta (xkalet05)
 * Parser zpracovávající výstup lexikální analýzy a ověřující jeho syntaktickou a sémantickou správnost   
 */

#ifndef PARSER_H
#define PARSER_H

#define M_ID 1
#define M_INTEGER_VAL 2
#define M_DOUBLE_VAL 3
#define M_STRING_VAL 4
#define M_END_OF_FILE 6
#define M_AS 10
#define M_ASC 11
#define M_DECLARE 12
#define M_DIM 13
#define M_DO 14
#define M_DOUBLE 15
#define M_ELSE 16
#define M_END 17
#define M_CHR 18
#define M_FUNCTION	19
#define M_IF 20
#define M_INPUT 21
#define M_INTEGER 22
#define M_LENGTH 23
#define M_LOOP 24
#define M_PRINT 25
#define M_RETURN 26
#define M_SCOPE 27
#define M_STRING 28
#define M_SUBSTR 29
#define M_THEN 30
#define M_WHILE 31
#define M_EOL 46
#define M_LEFT_PARANTHESE 32
#define M_RIGHT_PARANTHESE 33
#define M_EQUAL 39
#define M_NOTEQUAL 44
#define M_COMMA 47
#define M_SEMICOLON 40
#define M_PLUS 34
#define M_MINUS 35
#define M_MODULO 
#define M_TIMES 36
#define M_DIVIDE 37
#define M_INTEGER_DIVIDE 38
#define M_MORE 42
#define M_LESS 41
#define M_MORE_EQUAL 45
#define M_LESS_EQUAL 43

#define POSTFIX_OK 80

//TODO-Doplnit správné include
#include <stdio.h>
#include "string.h"
#include "precedence.h"
#include "symtable.h"
#include "syntaxtree.h"
#include "scanner.h"

typedef enum {
    SYNTAX_OK = 90,
    SYNTAX_ERROR = 98,
    LEX_ERROR = 99,
    SEMANTIC_ERROR_1 = 3, // typová nekompatibilita, parametry fce
    SEMANTIC_ERROR_2 = 4, // nedefinovaná funkce/proměnná, pokus o redefinování funkce / proměnné
    SEMANTIC_ERROR_3 = 6, // ostatní sémantika
    INFIX_TRANSFORM_ERROR = 100
} p_status;

/**
*	START → <F_DEC_DEF_LIST> BODY EOF
*	@return Kód propagující syntaktickou správnost či chybu
*/
int func_start();

/**
*	F_DEC_DEF_LIST → F_DEC_DEF EOL <F_DEC_DEF_LIST>
*	F_DEC_DEF_LIST → &
*	@return Kód propagující syntaktickou správnost či chybu
*/
int func_f_dec_def_list();

/**
*	F_DEC_DEF → declare function id (<PARAM_LIST>) as DATATYPE
*	F_DEC_DEF → function id (<PARAM_LIST>) as DATATYPE EOL <F_PROGRAM> end function
*	@return Kód propagující syntaktickou správnost či chybu
*/
int func_f_dec_def();

/**
*	F_PROGRAM → F_P_DEF_STAT EOL <F_PROGRAM>
*	F_PROGRAM → &
*	@return Kód propagující syntaktickou správnost či chybu
*/
int func_f_program();

/**
*	F_P_DEF_STAT → P_DEFINITION
*	F_P_DEF_STAT → STAT
*	F_P_DEF_STAT → STAT_RET
*	@return Kód propagující syntaktickou správnost či chybu
*/
int func_f_p_def_stat();

/**
*	BODY → scope EOL <PROGRAM> end scope
*	@return Kód propagující syntaktickou správnost či chybu
*/
int func_body();

/**
*	PROGRAM → P_DEF_STAT EOL <PROGRAM>
*	PROGRAM → &
*	@return Kód propagující syntaktickou správnost či chybu
*/
int func_program();

/**
*	P_DEF_STAT → P_DEFINITION
*	P_DEF_STAT → STAT
*	@return Kód propagující syntaktickou správnost či chybu
*/
int func_p_def_stat();

/**
*	P_DEFINITION → dim id as DATATYPE_DEF <ASSIGN>
*	@return Kód propagující syntaktickou správnost či chybu
*/
int func_p_definition();

/**
*	ASSIGN → = expr
*	ASSIGN → &
*	@return Kód propagující syntaktickou správnost či chybu
*/
int func_assign();

/**
*	PARAM_LIST → id as DATATYPE <PARAM_N>
*	PARAM_LIST → &
*	@return Kód propagující syntaktickou správnost či chybu
*/
int func_param_list();

/**
*	IN_PARAM_LIST → term <IN_PARAM_N>
*	@return Kód propagující syntaktickou správnost či chybu
*/
int func_in_param_list();

/**
*	IN_PARAM_N → , term <IN_PARAM_N>
*	IN_PARAM_N → &
*	@return Kód propagující syntaktickou správnost či chybu
*/
int func_in_param_n();

/**
*	PARAM_N → , id as DATATYPE <PARAM_N>
*	PARAM_N → &
*	@return Kód propagující syntaktickou správnost či chybu
*/
int func_param_n();

/**
*	DATATYPE → integer
*	DATATYPE → double
*	DATATYPE → string
*	@return Kód propagující syntaktickou správnost či chybu
*/
int func_datatype();

/**
*	STAT → id = func_stat_exfu
*	STAT → input id
*	STAT → print expr ; <EXPR_N>
*	STAT → if expr then EOL <STAT_LIST_IF> <ELSE> end if 
*	STAT → do while expr EOL <STAT_LIST_WHILE> loop
*	@return Kód propagující syntaktickou správnost či chybu
*/
int func_stat();

/**
*	STAT_EXFU → expr
*	STAT_EXFU → id (<IN_PARAM_LIST>)
*	@return Kód propagující syntaktickou správnost či chybu
*/
int func_stat_exfu();

/**
*	EXPR_N → expr ; <EXPR_N>
*	EXPR_N → &
*	@return Kód propagující syntaktickou správnost či chybu
*/
int func_expr_n();

/**
*	STAT_LIST_IF → STAT EOL  <STAT_LIST IF>
*	STAT_LIST_IF → &
*	@return Kód propagující syntaktickou správnost či chybu
*/
int func_stat_list_if();

/**
*	STAT_LIST_ELSE → STAT EOL <STAT_LIST_ELSE>
*	STAT_LIST_ELSE → &
*	@return Kód propagující syntaktickou správnost či chybu
*/
int func_stat_list_else();

/**
*	STAT_LIST_WHILE → STAT EOL <STAT_LIST_WHILE>
*	STAT_LIST_WHILE → &
*	@return Kód propagující syntaktickou správnost či chybu
*/
int func_stat_list_while();

/**
*	ELSE → else EOL <STAT_LIST_IF>
*	ELSE → &
*	@return Kód propagující syntaktickou správnost či chybu
*/
int func_stat_else();

/**
*	STAT_RET → return expr
*	STAT_RET → &
*	@return Kód propagující syntaktickou správnost či chybu
*/
int func_stat_ret();


/**
*	Hlavní volání syntaktického analyzátoru
*	@return Kód propagující syntaktickou správnost či chybu
*/
int parse();

extern int token;	//Proměnná pro uchování typu aktuálního tokenu
extern int token_n;	//Proměnná pro uchování typu následujícího tokenu (pokud je pro další postup nutno znát 2 tokeny)

extern string detail_act;	//Proměnná pro uchování detailu aktuálního tokenu
extern string detail_next;	//Proměnná pro uchování detailu následujícího tokenu

extern int eol_flag;	//=1 při již načteném tokenu EOL, tudíž jednou přeskakuji volání lexikálního analyzátoru
extern int token_next_flag;  //= 1 pokud do precedenční analýzy vstupují 2 znaky

extern s_list list;

#endif