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

extern string detail;
extern int state;
extern int symbol;

int scanner();