/**
 * Projekt - Tým 097, varianta I
 * Autor: Tom Barbořík (xbarbo06), Pavel Kaleta (xkalet05), David Myška (xmyska05), Tichavský Miroslav (xticha04)
 * Hlavní soubor překladače
 */


#include <stdlib.h>
#include <stdio.h>
#include "string.h"
#include "scanner.h"
#include "generator.h"
#include "parser.h"

string detail;
int state = START;
int symbol = 0;
s_list list;

int main(){
	detail = str_new();	// Alokuje misto pro praci se stringem.
	int ret = 0;
	Linit(&list);

	ret = parse();
	
		// Uvolni pamet.
	
	if (ret == SYNTAX_OK){ // Lexikalni, syntakticka i semanticka analyza v poradku, pokracuje ke generovani kodu.
		generate(list, stdout);
		ret = 0;
	}
	else if(ret == LEX_ERROR){
		ret = 1; // Error pro lexikalni;
	}
	else if(ret == SYNTAX_ERROR){
		ret = 2; // Error pro syntaktickou;
	}
	else if(ret == SEMANTIC_ERROR_1){
		ret = 3; // Error pro semantickou;
	} else if (ret == SEMANTIC_ERROR_2) {
		ret = 4;
	} else if (ret == SEMANTIC_ERROR_3) {
		ret = 6;
	} else {
		ret = 99;
	}
	
	// Uvolneni pameti
	Lclear(&list);
	str_del(detail);
	
	return ret;
}