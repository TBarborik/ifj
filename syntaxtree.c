#include "syntaxtree.h"
#include <stdlib.h>
#include <string.h>

int STerror=0;

//pouze v .c
void STinit(s_stree *root)
{
	if(STerror!=0)
		return;
	if(root==NULL)
	{
		STerror=2;
		return;
	}

	if((*root=malloc(sizeof(struct _stree)))==NULL)
	{
		STerror=1;
		return;
	}
}
void STclear(s_stree *root);

/*
N: const
D: (type)
V: (value)
*/
s_stree STcreateIntConst(int value)
{
	s_stree node;

	if(STerror!=0)
		return NULL;

	STinit(&node);
	if(STerror!=0)
		return NULL;

	node->ntype=n_const;
	node->dtype=d_int;
	node->value.v_int=value;
	node->rptr=NULL;
	node->lptr=NULL;

	return node;
}
s_stree STcreateDoubleConst(double value)
{
	s_stree node;

	STinit(&node);
	if(STerror!=0)
		return NULL;

	node->ntype=n_const;
	node->dtype=d_double;
	node->value.v_double=value;
	node->rptr=NULL;
	node->lptr=NULL;

	return node;
}
s_stree STcreateStringConst(char *value)
{
	s_stree node;

	if(STerror!=0)
		return NULL;
	if(value==NULL)
	{
		STerror=2;
		return NULL;
	}

	STinit(&node);
	if(STerror!=0)
		return NULL;

	node->ntype=n_const;
	node->dtype=d_string;
	if((node->value.v_string=malloc(strlen(value)+1))==NULL)
	{
		STerror=1;
		return NULL;
	}
	strcpy(node->value.v_string,value);
	node->rptr=NULL;
	node->lptr=NULL;

	return node;
}

/*
N: var
D: (type)
V(s): (var name)
*/
s_stree STcreateVar(char *varName, e_dstype type)
{
	s_stree node;

	if(STerror!=0)
		return NULL;
	if(varName==NULL || type==d_undef)
	{
		STerror=2;
		return NULL;
	}

	STinit(&node);
	if(STerror!=0)
		return NULL;

	node->ntype=n_var;
	node->dtype=type;
	if((node->value.v_string=malloc(strlen(varName)+1))==NULL)
	{
		STerror=1;
		return NULL;
	}
	strcpy(node->value.v_string,varName);
	node->rptr=NULL;
	node->lptr=NULL;

	return node;
}

/*
N: expr
D: UNDEF
V(s): (operation)
rptr: expr/var/const/call
lptr: expr/var/const/call/NULL
*/
s_stree STcreateExpr(s_stree leftValue, char *operation, s_stree rightValue)
{
	s_stree node;

	if(STerror!=0)
	{
		STclear(&leftValue);
		STclear(&rightValue);
		return NULL;
	}
	if(
		rightValue==NULL || operation==NULL
		|| (leftValue!=NULL && leftValue->ntype!=n_expr && leftValue->ntype!=n_call && leftValue->ntype!=n_var && leftValue->ntype!=n_const)
		|| (rightValue->ntype!=n_expr && rightValue->ntype!=n_call && rightValue->ntype!=n_var && rightValue->ntype!=n_const)
	)
	{
		STclear(&leftValue);
		STclear(&rightValue);
		STerror=2;
		return NULL;
	}


	STinit(&node);
	if(STerror!=0)
	{
		STclear(&leftValue);
		STclear(&rightValue);
		return NULL;
	}

	node->ntype=n_expr;
	node->dtype=d_string;
	if((node->value.v_string=malloc(strlen(operation)+1))==NULL)
	{
		STclear(&leftValue);
		STclear(&rightValue);
		STerror=1;
		return NULL;
	}
	strcpy(node->value.v_string,operation);
	node->rptr=rightValue;
	node->lptr=leftValue;

	return node;
}

/*
N: func
D: (return type)
V(s): (label (func name))
rptr: param/NULL
*/
s_stree STcreateFunc(char *labelName, e_dstype returnType, s_stree params)
{
	s_stree node;
	s_stree ptr;

	if(STerror!=0)
	{
		STclear(&params);
		return NULL;
	}
	if(labelName==NULL || returnType==d_undef)
	{
		STclear(&params);
		STerror=2;
		return NULL;
	}
	ptr=params;
	while(ptr!=NULL)
	{
		if(ptr->lptr->ntype!=n_var)
		{
			STclear(&params);
			STerror=2;
			return NULL;
		}
		ptr=ptr->rptr;
	}

	STinit(&node);
	if(STerror!=0)
	{
		STclear(&params);
		return NULL;
	}

	node->ntype=n_func;
	node->dtype=returnType;
	if((node->value.v_string=malloc(strlen(labelName)+1))==NULL)
	{
		STclear(&params);
		STerror=1;
		return NULL;
	}
	strcpy(node->value.v_string,labelName);
	node->rptr=params;
	node->lptr=NULL;

	return node;
}
/*
N: endfunc
D: UNDEF
V: UNDEF
*/
s_stree STcreateEndFunc()
{
	s_stree node;

	if(STerror!=0)
		return NULL;

	STinit(&node);
	if(STerror!=0)
		return NULL;

	node->ntype=n_endfunc;
	node->dtype=d_undef;
	node->value.v_string=NULL;
	node->rptr=NULL;
	node->lptr=NULL;

	return node;
}

/*
N: call
D: UNDEF
V(s): (label name)
rptr: param/NULL
*/
s_stree STcreateCall(char *labelName, s_stree params)
{
	s_stree node;
	s_stree ptr;

	if(STerror!=0)
	{
		STclear(&params);
		return NULL;
	}
	if(labelName==NULL)
	{
		STclear(&params);
		STerror=2;
		return NULL;
	}
	ptr=params;

	while(ptr!=NULL)
	{
		if(ptr->lptr->ntype!=n_var && ptr->lptr->ntype!=n_const && ptr->lptr->ntype!=n_call && ptr->lptr->ntype!=n_expr)
		{
			STclear(&params);
			STerror=2;
			return NULL;
		}
		ptr=ptr->rptr;
	}

	STinit(&node);
	if(STerror!=0)
	{
		STclear(&params);
		return NULL;
	}

	node->ntype=n_call;
	node->dtype=d_undef;
	if((node->value.v_string=malloc(strlen(labelName)+1))==NULL)
	{
		STclear(&params);
		STerror=1;
		return NULL;
	}
	strcpy(node->value.v_string,labelName);
	node->rptr=params;
	node->lptr=NULL;

	return node;
}

/*
N: param
D: UNDEF
V: UNDEF
rptr: next param/NULL
lptr: var nebo pro call var/const/call/expr
*/
s_stree STcreateFirstParam(s_stree param)
{
	s_stree node;

	if(STerror!=0)
	{
		STclear(&param);
		return NULL;
	}
	if(param==NULL || (param->ntype!=n_var && param->ntype!=n_const && param->ntype!=n_call && param->ntype!=n_expr))
	{
		STclear(&param);
		STerror=2;
		return NULL;
	}

	STinit(&node);
	if(STerror!=0)
	{
		STclear(&param);
		return NULL;
	}

	node->ntype=n_param;
	node->dtype=d_undef;
	node->value.v_string=NULL;
	node->rptr=NULL;
	node->lptr=param;

	return node;
}
void STaddNextParam(s_stree firstParam, s_stree param)
{
	s_stree node;

	if(STerror!=0)
	{
		STclear(&firstParam);
		STclear(&param);
		return;
	}
	if(firstParam==NULL || param==NULL || (param->ntype!=n_var && param->ntype!=n_const && param->ntype!=n_call && param->ntype!=n_expr))
	{
		STclear(&firstParam);
		STclear(&param);
		STerror=2;
		return;
	}

	STinit(&node);
	if(STerror!=0)
	{
		STclear(&firstParam);
		STclear(&param);
		return;
	}

	node->ntype=n_param;
	node->dtype=d_undef;
	node->value.v_string=NULL;
	node->rptr=NULL;
	node->lptr=param;

	while(firstParam->rptr!=NULL)
		firstParam=firstParam->rptr;

	firstParam->rptr=node;
}

/*
N: return
D: UNDEF
V: UNDEF
rptr: var/const/expr/call/NULL
*/
s_stree STcreateReturn(s_stree returnValue)
{
	s_stree node;

	if(STerror!=0)
	{
		STclear(&returnValue);
		return NULL;
	}
	if(returnValue!=NULL && returnValue->ntype!=n_var && returnValue->ntype!=n_const && returnValue->ntype!=n_expr && returnValue->ntype!=n_call)
	{
		STclear(&returnValue);
		STerror=2;
		return NULL;
	}

	STinit(&node);
	if(STerror!=0)
	{
		STclear(&returnValue);
		return NULL;
	}

	node->ntype=n_return;
	node->dtype=d_undef;
	node->value.v_string=NULL;
	node->rptr=returnValue;
	node->lptr=NULL;

	return node;
}

/*
N: if
D: UNDEF
V: UNDEF
rptr: (expr)
*/
s_stree STcreateIf(s_stree expr)
{
	s_stree node;

	if(STerror!=0)
	{
		STclear(&expr);
		return NULL;
	}
	if(expr==NULL || expr->ntype!=n_expr)
	{
		STclear(&expr);
		STerror=2;
		return NULL;
	}

	STinit(&node);
	if(STerror!=0)
	{
		STclear(&expr);
		return NULL;
	}

	node->ntype=n_if;
	node->dtype=d_undef;
	node->value.v_string=NULL;
	node->rptr=expr;
	node->lptr=NULL;

	return node;
}
/*
N: else
D: UNDEF
V: UNDEF
*/
s_stree STcreateElse()
{
	s_stree node;

	if(STerror!=0)
		return NULL;

	STinit(&node);
	if(STerror!=0)
		return NULL;

	node->ntype=n_else;
	node->dtype=d_undef;
	node->value.v_string=NULL;
	node->rptr=NULL;
	node->lptr=NULL;

	return node;
}
/*
N: endif
D: UNDEF
V: UNDEF
*/
s_stree STcreateEndIf()
{
	s_stree node;

	if(STerror!=0)
		return NULL;

	STinit(&node);
	if(STerror!=0)
		return NULL;

	node->ntype=n_endif;
	node->dtype=d_undef;
	node->value.v_string=NULL;
	node->rptr=NULL;
	node->lptr=NULL;

	return node;
}

/*
N: while
D: UNDEF
V: UNDEF
rptr: (expr)
*/
s_stree STcreateWhile(s_stree expr)
{
	s_stree node;

	if(STerror!=0)
	{
		STclear(&expr);
		return NULL;
	}
	if(expr==NULL || expr->ntype!=n_expr)
	{
		STclear(&expr);
		STerror=2;
		return NULL;
	}

	STinit(&node);
	if(STerror!=0)
	{
		STclear(&expr);
		return NULL;
	}

	node->ntype=n_while;
	node->dtype=d_undef;
	node->value.v_string=NULL;
	node->rptr=expr;
	node->lptr=NULL;

	return node;
}
/*
N: endwhile
D: UNDEF
V: UNDEF
*/
s_stree STcreateEndWhile()
{
	s_stree node;

	if(STerror!=0)
		return NULL;

	STinit(&node);
	if(STerror!=0)
		return NULL;

	node->ntype=n_endwhile;
	node->dtype=d_undef;
	node->value.v_string=NULL;
	node->rptr=NULL;
	node->lptr=NULL;

	return node;
}

/*
N: print
D: UNDEF
V: UNDEF
rptr: var/const/expr/call
*/
s_stree STcreatePrint(s_stree printValue)
{
	s_stree node;

	if(STerror!=0)
	{
		STclear(&printValue);
		return NULL;
	}
	if(printValue==NULL || (printValue->ntype!=n_var && printValue->ntype!=n_const && printValue->ntype!=n_expr && printValue->ntype!=n_call))
	{
		STclear(&printValue);
		STerror=2;
		return NULL;
	}

	STinit(&node);
	if(STerror!=0)
	{
		STclear(&printValue);
		return NULL;
	}

	node->ntype=n_print;
	node->dtype=d_undef;
	node->value.v_string=NULL;
	node->rptr=printValue;
	node->lptr=NULL;

	return node;
}

/*
N: read
D: UNDEF
V: UNDEF
rptr: var
*/
s_stree STcreateRead(s_stree var)
{
	s_stree node;

	if(STerror!=0)
	{
		STclear(&var);
		return NULL;
	}
	if(var==NULL || var->ntype!=n_var)
	{
		STclear(&var);
		STerror=2;
		return NULL;
	}

	STinit(&node);
	if(STerror!=0)
	{
		STclear(&var);
		return NULL;
	}

	node->ntype=n_read;
	node->dtype=d_undef;
	node->value.v_string=NULL;
	node->rptr=var;
	node->lptr=NULL;

	return node;
}


/*
N: i2f
D: UNDEF
V: UNDEF
rptr: var
*/
s_stree STcreateInt2Float(s_stree var)
{
	s_stree node;

	if(STerror!=0)
	{
		STclear(&var);
		return NULL;
	}
	if(var==NULL || var->ntype!=n_var)
	{
		STclear(&var);
		STerror=2;
		return NULL;
	}

	STinit(&node);
	if(STerror!=0)
	{
		STclear(&var);
		return NULL;
	}

	node->ntype=n_i2f;
	node->dtype=d_undef;
	node->value.v_string=NULL;
	node->rptr=var;
	node->lptr=NULL;

	return node;
}

/*
N: f2i
D: UNDEF
V: UNDEF
rptr: var
*/
s_stree STcreateFloat2Int(s_stree var)
{
	s_stree node;

	if(STerror!=0)
	{
		STclear(&var);
		return NULL;
	}
	if(var==NULL || var->ntype!=n_var)
	{
		STclear(&var);
		STerror=2;
		return NULL;
	}

	STinit(&node);
	if(STerror!=0)
	{
		STclear(&var);
		return NULL;
	}

	node->ntype=n_f2i;
	node->dtype=d_undef;
	node->value.v_string=NULL;
	node->rptr=var;
	node->lptr=NULL;

	return node;
}

//pouze v .c
void STclear(s_stree *root)
{
	if(root==NULL)
	{
		STerror=2;
		return;
	}

	if(*root==NULL)
		return;
	STclear(&((*root)->rptr));
	STclear(&((*root)->lptr));

	if(
		((*root)->ntype==n_const && (*root)->dtype==d_string)
		|| (*root)->ntype==n_var || (*root)->ntype==n_expr
		|| (*root)->ntype==n_func || (*root)->ntype==n_call
	)
		free((*root)->value.v_string);

	free(*root);
	*root=NULL;
}

//list
void Linit(s_list *list)
{
	if(STerror!=0)
		return;
	if(list==NULL)
	{
		STerror=2;
		return;
	}

	if((*list=malloc(sizeof(struct _list)))==NULL)
	{
		STerror=1;
		return;
	}

	(*list)->size=0;
	(*list)->cap=64;
	if(((*list)->list=malloc(sizeof(s_stree)*(*list)->cap))==NULL)
		STerror=1;
}

void Ladd(s_list *list, s_stree tree)
{
	s_stree *tmp;

	if(STerror!=0)
	{
		STclear(&tree);
		return;
	}
	if(list==NULL || *list==NULL || tree==NULL)
	{
		STclear(&tree);
		STerror=2;
		return;
	}

	if((*list)->size==(*list)->cap)
	{
		(*list)->cap<<=1;
		if((tmp=realloc((*list)->list,sizeof(s_stree)*(*list)->cap))==NULL)
		{
			STclear(&tree);
			STerror=1;
			return;
		}
		(*list)->list=tmp;
	}

	(*list)->list[(*list)->size]=tree;
	(*list)->size++;
}

s_stree Lget(s_list *list, int index)
{
	if(STerror!=0)
		return NULL;
	if(list==NULL || *list==NULL || index<0 || index>=(*list)->size)
	{
		STerror=2;
		return NULL;
	}
	return (*list)->list[index];
}

void Lclear(s_list *list)
{
	if(list==NULL)
	{
		STerror=2;
		return;
	}

	if(*list==NULL)
		return;

	for(int i=0;i<(*list)->size;i++)
		STclear(&((*list)->list[i]));

	free((*list)->list);
	free(*list);
	*list=NULL;
}







