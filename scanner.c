#include <stdlib.h>
#include <stdio.h>	//PRO TESTOVACI UCELY
#include "string.h"
#include "scanner.h"

char* normalize(char* str){
	int i = 0;
	while(str[i] != '\0'){
		if (str[i] >= 65 && str[i] <= 90){
			str[i] += 32;
		}
		i++;
	}
	
	return str;
}

int scanner(){
	int c;
	int id = 0;
	
	str_clear(detail);
	if (symbol != 0){
		str_append_char(detail, symbol);
		symbol = 0;
		if (strcmp(str_to_array(detail), "(") == 0)
			id = 32;
		else if (strcmp(str_to_array(detail), ")") == 0)
			id = 33;
		else if (strcmp(str_to_array(detail), "+") == 0)
			id = 34;
		else if (strcmp(str_to_array(detail), "-") == 0)
			id = 35;
		else if (strcmp(str_to_array(detail), "*") == 0)
			id = 36;
		else if (strcmp(str_to_array(detail), "/") == 0)
			id = 37;
		else if (strcmp(str_to_array(detail), "\\") == 0)
			id = 38;
		else if (strcmp(str_to_array(detail), "=") == 0)
			id = 39;
		else if (strcmp(str_to_array(detail), ";") == 0)
			id = 40;
		else if (strcmp(str_to_array(detail), "<") == 0)
			id = 41;
		else if (strcmp(str_to_array(detail), ">") == 0)
			id = 42;
		else if (strcmp(str_to_array(detail), "<=") == 0)
			id = 43;
		else if (strcmp(str_to_array(detail), "<>") == 0)
			id = 44;
		else if (strcmp(str_to_array(detail), ">=") == 0)
			id = 45;
		else if (strcmp(str_to_array(detail), "\n") == 0)
			id = 46;
		else if (strcmp(str_to_array(detail), ",") == 0)
			id = 47;
		//printf("ID: %d Detail: %s\n", id, str_to_array(detail));		//PRO TESTOVACI UCELY
		return id; // Symbol, v detailu presny znak.
	}
	while (id == 0){
		c = fgetc(stdin);
		switch(state){
			case START:
				if (c == '_' || (c >= 65 && c <= 90) || (c >= 97 && c <= 122)){
					state = ID_IN;
					str_append_char(detail, c);
					break;
					
				}
				
				else if (c >= 48 && c <= 57){
					state = INT_IN;
					str_append_char(detail, c);
					break;
				}
				
				else if (c == '!'){
					state = STR_BEFORE;
					break;
				}
				
				else if (c == '\''){
					state = COMMENT;
					break;
				}
				
				else if (c == 9 || c == 11 || c == 12 || c == 13 || c == 32){
					break;
				}
				
				else if (c == '/'){
					state = SLASH;
					break;
				}
				
				else if (c == '+' || c == '-' || c == '*' || c == '\\' || c == '=' || c == ';' || c == '\n' || c == ',' || c == '(' || c == ')'){
					str_put_char(detail, c);
					id = 5;
					if (c == '\n'){
						state = NEWLINE;
					}
					break;
				}
				
				else if (c == '<'){
					state = LESS;
					break;
				}
				
				else if (c == '>'){
					state = MORE;
					break;
				}
				
				else if (c == EOF){
					state = FINISH;
					break;
				}				
				else{
					state = E;
					break;
				}
				break;
				
			case ID_IN:
				if (c == '_' || (c >= 65 && c <= 90) || (c >= 97 && c <= 122) || (c >= 48 && c <= 57)){
					str_append_char(detail, c);
					break;
				}
				
				else if (c == 9 || c == 11 || c == 12 || c == 13 || c == 32){
					state = START;
					id = 1;
				}
				else if (c == '<'){
					state = LESS;
					id = 1;
					break;
				}
				
				else if (c == '>'){
					state = MORE;
					id = 1;
					break;
				}
				else if (c == EOF){
					state = FINISH;
					id = 1;
					break;
				}
				
				else if (c == '+' || c == '-' || c == '*' || c == '\\' || c == '='  || c == ';' || c == '\n' || c == ',' || c == '(' || c == ')'){
					state = START;
					id = 1;
					symbol = c;
					if (c == '\n'){
						state = NEWLINE;
					}
					break;
				}
				
				else{
					state = E;
					break;
				}
				break;
				
			case INT_IN:
				if (c >= 48 && c <= 57){
					str_append_char(detail, c);
					break;
				}
				
				else if (c == 'e' || c == 'E'){
					state = EXP_BEFORE;
					str_append_char(detail, c);
					break;
				}
				
				else if (c == '.'){
					state = DBL_BEFORE;
					str_append_char(detail, c);
					break;
				}
				
				else if (c == 9 || c == 11 || c == 12 || c == 13 || c == 32){
					state = START;
					id = 2;
					break;
				}
				
				else if (c == '/'){
					state = SLASH;
					id = 2;
					break;
				}
				
				else if (c == '+' || c == '-' || c == '*' || c == '\n' || c == '\\' || c == '=' || c == ';' || c == ',' || c == '(' || c == ')'){
					state = START;
					id = 2;
					symbol = c;
					if (c == '\n'){
						state = NEWLINE;
					}
					break;
				}
				
				else if (c == EOF){
					state = FINISH;
					id = 2;
					break;
				}
				
				else{
					state = E;
					break;
				}
				break;
				
			case EXP_BEFORE:
				if (c == '+' || c == '-'){
					state = EXP_INBEF;
					str_append_char(detail, c);
					break;
				}
				
				else if (c >= 48 && c <= 57){
					state = EXP_IN;
					str_append_char(detail, c);
					break;
				}
				
				else{
					state = E;
					break;
				}
				break;
			
			case EXP_INBEF:
				if (c >= 48 && c <= 57){
					state = EXP_IN;
					str_append_char(detail, c);
					break;
				}
				
				else{
					state = E;
					break;
				}
				
			case EXP_IN:
				if (c >= 48 && c <= 57){
					str_append_char(detail, c);
					break;
				}
				
				else if (c == 9 || c == 11 || c == 12 || c == 13 || c == 32){
					id = 3;
					break;
				}
				
				else if (c == '/'){
					state = SLASH;
					id = 3;
					break;
				}
				
				else if (c == '+' || c == '-' || c == '*' || c == '\\' || c == '='  || c == ';' || c == '\n' ){
					state = START;
					id = 3;
					symbol = c;
					if (c == '\n'){
						state = NEWLINE;
					}
					break;
				}
				
				else if (c == EOF){
					state = FINISH;
					id = 3;
					break;
				}
				
				else{
					state = E;
					break;
				}
				
			case DBL_BEFORE:
				if (c >= 48 && c <= 57){
					state = DBL_IN;
					str_append_char(detail, c);
					break;
				}
				
				else{
					state = E;
					break;
				}
				
				break;
				
			case DBL_IN:
				if (c == 'e' || c == 'E'){
					state = EXP_BEFORE;
					str_append_char(detail, c);
					break;
				}
				
				else if (c >= 48 && c <= 57){
					str_append_char(detail, c);
					break;
				}
				
				else if (c == 9 || c == 11 || c == 12 || c == 13 || c == 32){
					state = START;
					id = 3;
					break;
				}
				
				else if (c == '+' || c == '-' || c == '*' || c == '\\' || c == ','  || c == ';' || c == '=' || c == '\n'){
					state = START;
					id = 3;
					symbol = c;
					if (c == '\n'){
						state = NEWLINE;
					}
					break;
				}
				
				else if (c == EOF){
					state = FINISH;
					id = 3;
					break;
				}
				
				else{
					state = E;
					break;
				}
				break;
				
			case STR_BEFORE:
				if (c == '"'){
					state = STR_START;
					break;
				}
				
				else{
					state = E;
					break;
				}
				break;
				
			case STR_START:
				if (c == '\n'){
					state = E;
					break;
				}
				
				else if (c == '"'){
					state = START;
					id = 4;
					break;
				}
				
				else if (c == EOF){
					state = E;
					break;
				}
				
				else {
					state = STR_IN;
					str_append_char(detail, c);
					break;
				}
				
				break;
				
			case STR_IN:
				if (c == '"'){
					state = START;
					id = 4;
					break;
				}
				
				else if (c == '\n'){
					state = E;
					break;
				}
				
				else if (c == EOF){
					state = E;
					break;
				}
				
				else{
					str_append_char(detail, c);
					break;
				}
				break;
				
			case SLASH:
				if (c == '\''){
					state = COMMENT_BLOCK;
					break;
				}
				
				else if (c == EOF){
					state = FINISH;
					id = 5;
					str_append_char(detail, '/');
					break;
				}
				
				else{
					state = START;
					id = 5;
					str_append_char(detail, '/');
					break;
				}
				
				break;
				
			case COMMENT:
				if (c == '\n'){
					state = NEWLINE;
					break;
				}
				
				else if (c == EOF){
					state = FINISH;
					break;
				}
				
				else{
					break;
				}
			
			case COMMENT_BLOCK:
				if (c == '\''){
					state = COMMENT_ENDING;
					break;
				}
				
				else if (c == EOF){
					state = FINISH;
					break;
				}
				
				else{
					break;
				}
				
			case COMMENT_ENDING:
				if (c == '/'){
					state = START;
					break;
				}
				
				else if (c == EOF){
					state = FINISH;
					break;
				}
				
				else{
					state = COMMENT_BLOCK;
					break;
				}
				
			case LESS:
				if (c == '='){
					state = START;
					str_append_char(detail, '<');
					str_append_char(detail, '=');
					id = 5;
					break;
				}
				
				else if (c == '>'){
					state = START;
					str_append_char(detail, '<');
					str_append_char(detail, '>');
					id = 5;
					break;
				}
				
				else if (c == 9 || c == 10 || c == 11 || c == 12 || c == 13	|| c == 32){
					state = START;
					str_append_char(detail, '<');
					id = 5;
					break;
				}
				
				else if (c == EOF){
					state = FINISH;
					str_append_char(detail, '<');
					id = 5;
					break;
				}
				
				else{
					//TODO: MUZE NASLEDOVAT COKOLIV BEZ MEZERY, POTREBA VYRESIT JAK ODESLAT (STAVY JAKO PRO START?) 
					break;
				}
				break;
				
			case MORE:
				if (c == '='){
					state = START;
					str_append_char(detail, '>');
					str_append_char(detail, '=');
					id = 5;
					break;
				}
				
				else if (c == 9 || c == 10 || c == 11 || c == 12 || c == 13	|| c == 32){
					state = START;
					str_append_char(detail, '>');
					id = 5;
					break;
				}
				
				else if (c == EOF){
					state = FINISH;
					str_append_char(detail, '>');
					id = 5;
					break;
				}
				
				else{
					//TODO: MUZE NASLEDOVAT COKOLIV BEZ MEZERY, POTREBA VYRESIT JAK ODESLAT (STAVY JAKO PRO START?) 
					break;
				}
				break;
				
			case NEWLINE:
				if (c == '\n'){
						break;
					}
					
				else if (c == '_' || (c >= 65 && c <= 90) || (c >= 97 && c <= 122)){
					state = ID_IN;
					str_append_char(detail, c);
					break;
					
				}
				
				else if (c >= 48 && c <= 57){
					state = INT_IN;
					str_append_char(detail, c);
					break;
				}
				
				else if (c == '!'){
					state = STR_BEFORE;
					break;
				}
				
				else if (c == '\''){
					state = COMMENT;
					break;
				}
				
				else if (c == 9 || c == 11 || c == 12 || c == 13 || c == 32){
					break;
				}
				
				else if (c == '/'){
					state = SLASH;
					break;
				}
				
				else if (c == '+' || c == '-' || c == '*' || c == '\\' || c == '=' || c == ';' || c == '\n' || c == ',' || c == '(' || c == ')'){
					str_put_char(detail, c);
					id = 5;
					if (c == '\n'){
						state = NEWLINE;
					}
					break;
				}
				
				else if (c == '<'){
					state = LESS;
					break;
				}
				
				else if (c == '>'){
					state = MORE;
					break;
				}
				
				else if (c == EOF){
					state = FINISH;
					break;
				}				
				else{
					state = E;
					break;
				}
				break;	
			
			case FINISH:
				id = 6;
				break;
				
			case E:
				id = 99;
				break;

		}
	}
	
	str_put(detail, normalize(str_to_array(detail)));
	if (id == 1){
		if (strcmp(str_to_array(detail), "as") == 0)
			id = 10;
		else if (strcmp(str_to_array(detail), "asc") == 0)
			id = 11;
		else if (strcmp(str_to_array(detail), "declare") == 0)
			id = 12;
		else if (strcmp(str_to_array(detail), "dim") == 0)
			id = 13;
		else if (strcmp(str_to_array(detail), "do") == 0)
			id = 14;
		else if (strcmp(str_to_array(detail), "double") == 0)
			id = 15;
		else if (strcmp(str_to_array(detail), "else") == 0)
			id = 16;
		else if (strcmp(str_to_array(detail), "end") == 0)
			id = 17;
		else if (strcmp(str_to_array(detail), "chr") == 0)
			id = 18;
		else if (strcmp(str_to_array(detail), "function") == 0)
			id = 19;
		else if (strcmp(str_to_array(detail), "if") == 0)
			id = 20;
		else if (strcmp(str_to_array(detail), "input") == 0)
			id = 21;
		else if (strcmp(str_to_array(detail), "integer") == 0)
			id = 22;
		else if (strcmp(str_to_array(detail), "length") == 0)
			id = 23;
		else if (strcmp(str_to_array(detail), "loop") == 0)
			id = 24;
		else if (strcmp(str_to_array(detail), "print") == 0)
			id = 25;
		else if (strcmp(str_to_array(detail), "return") == 0)
			id = 26;
		else if (strcmp(str_to_array(detail), "scope") == 0)
			id = 27;
		else if (strcmp(str_to_array(detail), "string") == 0)
			id = 28;
		else if (strcmp(str_to_array(detail), "substr") == 0)
			id = 29;
		else if (strcmp(str_to_array(detail), "then") == 0)
			id = 30;
		else if (strcmp(str_to_array(detail), "while") == 0)
			id = 31;
		else if (strcmp(str_to_array(detail), "and") == 0)
			id = 7;
		else if (strcmp(str_to_array(detail), "boolean") == 0)
			id = 7;
		else if (strcmp(str_to_array(detail), "continue") == 0)
			id = 7;
		else if (strcmp(str_to_array(detail), "elseif") == 0)
			id = 7;
		else if (strcmp(str_to_array(detail), "exit") == 0)
			id = 7;
		else if (strcmp(str_to_array(detail), "false") == 0)
			id = 7;
		else if (strcmp(str_to_array(detail), "for") == 0)
			id = 7;
		else if (strcmp(str_to_array(detail), "next") == 0)
			id = 7;
		else if (strcmp(str_to_array(detail), "not") == 0)
			id = 7;
		else if (strcmp(str_to_array(detail), "or") == 0)
			id = 7;
		else if (strcmp(str_to_array(detail), "shared") == 0)
			id = 7;
		else if (strcmp(str_to_array(detail), "static") == 0)
			id = 7;
		else if (strcmp(str_to_array(detail), "true") == 0)
			id = 7;
	}
	else if (id == 5){
		if (strcmp(str_to_array(detail), "(") == 0)
			id = 32;
		else if (strcmp(str_to_array(detail), ")") == 0)
			id = 33;
		else if (strcmp(str_to_array(detail), "+") == 0)
			id = 34;
		else if (strcmp(str_to_array(detail), "-") == 0)
			id = 35;
		else if (strcmp(str_to_array(detail), "*") == 0)
			id = 36;
		else if (strcmp(str_to_array(detail), "/") == 0)
			id = 37;
		else if (strcmp(str_to_array(detail), "\\") == 0)
			id = 38;
		else if (strcmp(str_to_array(detail), "=") == 0)
			id = 39;
		else if (strcmp(str_to_array(detail), ";") == 0)
			id = 40;
		else if (strcmp(str_to_array(detail), "<") == 0)
			id = 41;
		else if (strcmp(str_to_array(detail), ">") == 0)
			id = 42;
		else if (strcmp(str_to_array(detail), "<=") == 0)
			id = 43;
		else if (strcmp(str_to_array(detail), "<>") == 0)
			id = 44;
		else if (strcmp(str_to_array(detail), ">=") == 0)
			id = 45;
		else if (strcmp(str_to_array(detail), "\n") == 0)
			id = 46;
		else if (strcmp(str_to_array(detail), ",") == 0)
			id = 47;
	}
	
	//printf("ID: %d Detail: %s\n", id, str_to_array(detail));	//PRO TESTOVACI UCELY
	return id;	
}



/*
// PRO TESTOVACI UCELY
int main(){
	int ret = 0;
	detail = str_new();
	while (ret != 99 && ret != 6){
		ret = scanner();
	}
	return 0;
}
*/

/// odeslat pouze posledni EOL


