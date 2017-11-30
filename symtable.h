#ifndef SYMTABLE_H
#define SYMTABLE_H 1

#include "syntaxtree.h"

//Typ uzlu stromu
typedef enum{
  sn_func_dec, sn_func_dec_def, sn_var
} e_ntype;

//Datovy typ
typedef enum {
	sd_int, sd_double, sd_string, sd_void
} e_dtype;

typedef struct {
	e_dtype type;
	char *name;
} e_param;

//Struktura uzlu stromu
typedef struct _btree{
	e_dtype dtype;
	e_ntype ntype;
	char *key;
	struct _btree *lptr;
	struct _btree *rptr;
	struct _btree *l_symtable;
	s_stree params;
} *s_btree;

/*
Inicializace stromu
!! Musi byt zavolana pouze jednou nad danym stromem, jinak muze dojit ke ztrate dat !!
return:
	0 - OK
	1 - chyba vstupniho parametru
*/
int BTinit(s_btree *root);

/*
Vraci pres parametr 'ret' ukazatel na uzel ve strome 'root' odpovidajiciho klici 'key'.
!! Neni provedena hluboka kopie uzlu => NEUPRAVOVAT !!
return:
	0 - OK
	-1 - OK, ale uzel s klicem 'key' nebyl nalezen
	1 - chyba vstupniho parametru
*/
int BTget(s_btree *root, const char *key, s_btree *ret);

/*
Vytvori novy uzel odpovidajici klici 'key' (popr. aktualizuje hodnoty: ntype,
dtype).
return:
	0 - OK
	1 - chyba vstupniho parametru
	2 - chyba v alokaci
*/
int BTput(s_btree *root, const char *key, e_ntype ntype, e_dtype dtype, s_stree params);

/*
Odstrani uzel odpovidajici klici 'key', vcetne uvolneni pameti.
return:
	0 - OK
	-1 - OK, ale uzel s klicem 'key' nebyl nalezen
	1 - chyba vstupniho parametru
*/
int BTdelete(s_btree *root, const char *key);

/*
Uvolni cely strom a uvolneni veskere pameti.
return:
	0 - OK
	1 - chyba vstupniho parametru
*/
int BTdispose(s_btree *root);

/*
Vyhleda, zapise a spocita vsechny uzly stromu 'root' odpovidajici typu uzlu
zadaneho v parametru 'ntype'. Pokud je 'list_ena' nastaven na 1, pak je
alokovano pole retezcu a naplneno klici vsech odpovidajicich uzlu (pouze melka
kopie) !NEUPRAVOVAT!, na konci pouzivani je nutne rucni uvoneni vytvoreneho
pole, nikoliv vsak jednotlivych retezcu! Pocet odpovidajicich uzlu se vraci
pomoci parametru 'num'.
return:
	!=NULL - OK, jen v pripade, ze parametr 'list_ena' je roven 1
	NULL - chyba, nebo parametr 'list_ena' je roven 0
parametr num:
	>=0 - OK, hodnota odpovida poctu odpovidajicich uzlu
	-1 - chyba vstupniho parametru
	-2 - chyba v alokaci
*/
char **BTlistof(s_btree *root, e_ntype ntype, int *num, int list_ena);
#endif //SYMTABLE_H