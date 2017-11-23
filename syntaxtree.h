#ifndef SYNTAXTREE_H
#define SYNTAXTREE_H 1

//typ uzlu
typedef enum{
	n_const, n_var, n_expr, n_func, n_endfunc, n_call, n_param, n_return, n_if, n_else, n_endif, n_while, n_endwhile, n_print, n_read
} e_nstype;

//datovy typ
typedef enum{
	d_undef, d_void, d_bool, d_int, d_double, d_string
}	e_dstype;

//hodnota (bud podle e_dstype, nebo napevno receno)
typedef union{
	int v_int;
	double v_double;
	char *v_string;
} u_value;

//struktura uzlu syntaktickeho stromu
typedef struct _stree{
	e_nstype ntype;
	e_dstype dtype;
	u_value value;
	struct _stree *lptr;
	struct _stree *rptr;
} *s_stree;

//struktura seznamu stromu (implementace pomoci dynamickeho pole)
typedef struct _list{
	int size;
	int cap;
	s_stree *list;
} *s_list;

/*
promenna obsahujici kod chyby
0 - OK
1 - chyba alokace
2 - chyba parametru
*/
extern int STerror;

/*
vsechny funkce v pripade chyby nastavuji promennou STerror
pokud je jiz nastavena ihned ukoncuji svoji cinnost (krome funkce Lclear)
pro usnadneni prace pri zanorovani funkci STcreate... jsou v pripade chyby
parametry typu s_stree automaticky a radne uvolneny
pozor ostatni parametry typu char * nebo s_list uvolneny nejsou

priklad pouziti:
	s_list list;
	s_stree params;

	Linit(&list);

	//vyraz: (1.75*var1)+var2
	//index: 0
	Ladd(&list,STcreateExpr(STcreateExpr(STcreateDoubleConst(1.75),"*",STcreateVar("var1",d_double)),"+",STcreateVar("var2",d_double)));

	//funkce: int func1(int par1,double par2)
	//index: 1
	params=STcreateFirstParam(STcreateVar("par1",d_int));
	STaddNextParam(params,STcreateVar("par2",d_double));
	Ladd(&list,STcreateFunc("func1",d_int,params));

	//funkce: char *func2()
	//index: 2
	Ladd(&list,STcreateFunc("func2",d_string,NULL));

	//vypise: 1.75
	printf("%f\n",Lget(&list,0)->lptr->lptr->value.v_double);
	//vypise: par2
	printf("%s\n",Lget(&list,1)->rptr->rptr->lptr->value.v_string);

	Lclear(&list);
*/


// ********** DEFINICE STROMU **********

/*
vytvoreni uzlu typu konstanta
datovy typ urcen podle volane funkce
parametry:
value - hodnota v pozadovanem typu (v pripade char * je provedena hluboka kopie)
vyznam prvku struktury:
ntype = n_const
dtype = datovy typ podle volane funkce (d_int,d_double,d_string)
value = zadana hodnota (datovy typ podle dtype (v_int,v_double,v_string))
lptr = NULL
rptr = NULL
*/
s_stree STcreateIntConst(int value);
s_stree STcreateDoubleConst(double value);
s_stree STcreateStringConst(char *value);

/*
vytvoreni uzlu typu promenna
parametry:
varName - jmeno promenne (hluboka kopie)
type - datovy typ promenne
vyznam prvku struktury:
ntype = n_var
dtype = datovy typ promenne
value.v_string = jmeno promenne
lptr = NULL
rptr = NULL
*/
s_stree STcreateVar(char *varName, e_dstype type);

/*
vytvoreni uzlu typu expr
parametry:
leftValue - levy operand (uzel typu const/var/expr/call nebo NULL pro unarni operaci; melka kopie)
rightValue - pravy operand (uzel typu const/var/expr/call; melka kopie)
operation - binarni operator (hluboka kopie)
vyznam prvku struktury:
ntype = n_expr
dtype = UNDEF
value.v_string = operator
lptr = levy operand (uzel typu const/var/expr/call nebo NULL)
rptr = pravy operand (uzel typu const/var/expr/call)
*/
s_stree STcreateExpr(s_stree leftValue, char *operation, s_stree rightValue);

/*
vytovreni uzlu typu func
parametry:
labelName - jmeno funkce (hluboka kopie)
returnType - navratovy datovy typ
params - parametry (uzel typu param nebo NULL pro funkci bez parametru; melka kopie)
vyznam prvku struktury:
ntype = n_func
dtype = navratovy datovy typ
value.v_string = jmeno funkce
lptr = NULL
rptr = parametry funkce (uzel typu param pro prvni parametr nebo NULL)
*/
s_stree STcreateFunc(char *labelName, e_dstype returnType, s_stree params);

/*
vytvoreni uzlu typu endfunc
vyznam prvku struktury:
ntype = n_endfunc
dtype = UNDEF
value = UNDEF
lptr = NULL
rptr = NULL
*/
s_stree STcreateEndFunc();

/*
vytvoreni uzlu typu call
parametry:
labelName - jmeno volane funkce
params - parametry (uzel typu param nebo NULL pro funkci bez parametru; melka kopie)
vyznam prvku struktury:
ntype = n_call
dtype = UNDEF
value.v_string = jmeno volane funkce
lptr = NULL
rptr = parametry funkce (uzel typu param pro prvni parametr nebo NULL)
*/
s_stree STcreateCall(char *labelName, s_stree params);

/*
vytvoreni uzlu typu param
pozor tyto dve funkce nelze zanorovat do dalsich STcreate...
parametry:
param - promenna reprezentujici dany parametr (pro call muze byt i const/call/expr; melka kopie)
firstParam - prvni parametr (uzel typu param, vytvoreny funkci STcreateFirstParam; jen upravena)
vyznam prvku struktury:
ntype = n_param
dtype = UNDEF
value = UNDEF
lptr = promenna reprezentujici dany parametr (uzel typu var pro call muze byt i typu const/call/expr)
rptr = dalsi parametr nebo NULL pro posledni parametr
*/
s_stree STcreateFirstParam(s_stree param);
void STaddNextParam(s_stree firstParam, s_stree param);

/*
vytvoreni uzlu typu return
parametry:
returnValue - navratova hodnota (uzel typu var/const/expr/call nebo NULL pro funkci bez navratove hodnoty; melka kopie)
vyznam prvku struktury:
ntype = n_return
dtype = UNDEF
value = UNDEF
lptr = NULL
rptr = navratova hodnota (uzel typu var/const/expr/call nebo NULL)
*/
s_stree STcreateReturn(s_stree returnValue);

/*
vytvoreni uzlu typu if
parametry:
expr - podminkovy vyraz (melka kopie)
vyznam prvku struktury:
ntype = n_if
dtype = UNDEF
value = UNDEF
lptr = NULL
rptr = podminkovy vyraz (uzel typu expr)
*/
s_stree STcreateIf(s_stree expr);
/*
vytvoreni uzlu typu else
vyznam prvku struktury:
ntype = n_else
dtype = UNDEF
value = UNDEF
lptr = NULL
rptr = NULL
*/
s_stree STcreateElse();
/*
vytovreni uzlu typu endif
vyznam prvku struktury:
ntype = n_endif
dtype = UNDEF
value = UNDEF
lptr = NULL
rptr = NULL
*/
s_stree STcreateEndIf();

/*
vytvoreni uzlu typu while
parametry:
expr - podminkovy vyraz (melka kopie)
vyznam prvku struktury:
ntype = n_while
dtype = UNDEF
value = UNDEF
lptr = NULL
rptr = podminkovy vyraz (uzel typu expr)
*/
s_stree STcreateWhile(s_stree expr);
/*
vytvoreni uzlu typu endwhile
vyznam prvku struktury:
ntype = n_endwhile
dtype = UNDEF
value = UNDEF
lptr = NULL
rptr = NULL
*/
s_stree STcreateEndWhile();

/*
vytvoreni uzlu typu print
parametry:
printValue - hodnota k vypisu (uzel typu var/const/expr/call; melka kopie)
vyznam prvku struktury:
ntype = n_print
dtype = UNDEF
value = UNDEF
lptr = NULL
rptr = promenna, konstanta nebo vyraz (uzel typu var/const/expr/call)
*/
s_stree STcreatePrint(s_stree printValue);

/*
vytvoreni uzlu typu read
parametry:
var - promenna kam bude nactena hodnota ulozena (melka kopie)
vyznam prvku struktury:
ntype = n_read
dtype = UNDEF
value = UNDEF
lptr = NULL
rptr = promenna (uzel typu var)
*/
s_stree STcreateRead(s_stree var);

// ********** DEFINICE SEZNAMU **********

/*
inicializace seznamu
pro dany seznam muze byt volana pouze jednou jinak muze dojit ke ztrate dat
parametry:
list - ukazatel na seznam (muze byt predan jako &list)
*/
void Linit(s_list *list);

/*
pridani syntaktickeho stromu do seznamu (na konec)
parametry:
list - ukazatel na seznam (muze byt predan jako &list)
tree - syntakticky strom vytvoreny funkcemi STcreate... (melka kopie)
*/
void Ladd(s_list *list, s_stree tree);

/*
ziskani syntaktickeho stromu na danem indexu
kontroluje rozsah indexu
pouze melka kopie NEUPRAVOVAT!
parametry:
list - ukazatel na seznam (muze byt predan jako &list)
index - index pozadovaneho stromu
vraci pozadovany strom nebo NULL pri chybe
*/
s_stree Lget(s_list *list, int index);

/*
smazani celeho seznamu vcetne potrebneho uvolneni pameti
parametry:
list - ukazatel na seznam (muze byt predan jako &list)
*/
void Lclear(s_list *list);

#endif