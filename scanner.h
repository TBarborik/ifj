/**
 * Projekt - Tým 097, varianta I
 * Autor: Pavel Kaleta (xkalet05)
 * Skenuje jednotlivá slova a provádí lexikální analýzu
 */

#define START 1
#define ID_IN 2
#define INT_IN 3
#define EXP_BEFORE 4
#define EXP_IN 5
#define DBL_BEFORE 6
#define DBL_IN 7
#define STR_BEFORE 8
#define STR_START 9
#define STR_BCKSLSH 10
#define STR_IN 11
#define COMMENT 12
#define SLASH 13
#define COMMENT_BLOCK 14
#define COMMENT_ENDING 15
#define LESS 16
#define MORE 17
#define EXP_INBEF 18
#define E 19
#define FINISH 20
#define NEWLINE 21
#define COMMENT_ON_NEWLINE 22
#define BACKSLASH_IN_STR 23
#define SLASH_ON_NEWLINE 24
#define COMMENT_BLOCK_ON_NEWLINE 25
#define COMMENT_ENDING_ON_NEWLINE 26

extern string detail;
extern int state;
extern int symbol;

int scanner();