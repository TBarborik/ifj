#include "symtable.h"
#include <string.h>
#include <stdlib.h>

//pomocny zasobnik
typedef struct _stack{
	s_btree *ptr;
	struct _stack *next;
} *s_stack;

void Sinit(s_stack *stack)
{
	*stack=NULL;
}

int Sempty(s_stack *stack)
{
	return (*stack==NULL);
}

int Spush(s_stack *stack, s_btree *ptr)
{
	s_stack pom;

	if((pom=malloc(sizeof(struct _stack)))==NULL)
		return -1;
	pom->next=*stack;
	pom->ptr=ptr;
	*stack=pom;

	return 0;
}

s_btree *Spop(s_stack *stack)
{
	s_btree *pom=(*stack)->ptr;
	s_stack pom2=*stack;
	*stack=(*stack)->next;
	free(pom2);

	return pom;
}

//bin tree
int BTinit(s_btree *root)
{
	if(root==NULL)
		return 1;
	*root=NULL;
	return 0;
}

int BTget(s_btree *root, const char *key, s_btree *ret)
{
	int res;

	if(root==NULL || key==NULL)
		return 1;

	while(*root!=NULL)
	{
		res=strcmp(key,(*root)->key);
		if(res==0)
		{
			if(ret!=NULL)
				*ret=*root;
			return 0;
		}

		if(res<0)
			root=&((*root)->lptr);
		else
			root=&((*root)->rptr);
	}

	return -1;
}

int BTput(s_btree *root, const char *key, e_ntype ntype, e_dtype dtype, s_stree params)
{
	int res;

	if(root==NULL || key==NULL)
		return 1;

	while(1)
	{
		if(*root==NULL)
		{
			*root=malloc(sizeof(struct _btree));
			if(*root==NULL)
				return 2;
			(*root)->key=malloc(strlen(key)+1);
			if((*root)->key==NULL)
			{
				free(*root);
				*root=NULL;
				return 2;
			}
			strcpy((*root)->key,key);
			(*root)->dtype=dtype;
			(*root)->ntype=ntype;
			(*root)->lptr=NULL;
			(*root)->rptr=NULL;
			(*root)->params = params;
			//(*root)->params = params;
			return 0;
		}

		res=strcmp(key,(*root)->key);
		if(res==0)
		{
			(*root)->dtype = dtype;
			(*root)->ntype = ntype;
			(*root)->params = params;

			return 0;
		}

		if(res<0)
			root=&((*root)->lptr);
		else
			root=&((*root)->rptr);
	}
}

int ReplaceByRightmost(s_btree rep, s_btree *root){

	s_btree pom;

	while(1)
		if((*root)->rptr==NULL)
		{
			pom=*root;
			free(rep->key);
			rep->key=pom->key;
			rep->dtype=pom->dtype;
			rep->ntype=pom->ntype;
			rep->params = pom->params;
			*root=pom->lptr;
			free(pom);
			return 0;
		}
		else
			root=&((*root)->rptr);
}

int BTdelete(s_btree *root, const char *key)
{
	int res;
	s_btree pom;

	if(root==NULL || key==NULL)
		return 1;

	while(*root!=NULL)
	{
		res=strcmp(key,(*root)->key);
		if(res==0)
		{
			pom=*root;

			if(pom->lptr!=NULL && pom->rptr!=NULL)
			{
				ReplaceByRightmost(*root,&(pom->lptr));
				return 0;
			}
			else if(pom->lptr!=NULL && pom->rptr==NULL)
				*root=pom->lptr;
			else
				*root=pom->rptr;
			
			if (pom->params != NULL)
				free(pom->params);
			free(pom->key);
			free(pom);
			return 0;
		}

		if(res<0)
			root=&((*root)->lptr);
		else
			root=&((*root)->rptr);
	}

	return -1;
}

int BTdispose(s_btree *root)
{
	s_btree pom;

	if(root==NULL)
		return 1;

	while(root != NULL && *root!=NULL)
	{
		pom=*root;

		if(pom->lptr!=NULL && pom->rptr!=NULL) {
			ReplaceByRightmost(*root,&(pom->lptr));
		}
		else
		{
			
			if(pom->lptr!=NULL && pom->rptr==NULL)
				*root=pom->lptr;
			else
				*root=pom->rptr;
			
			if (pom->params != NULL)
				free(pom->params);
			free(pom->key);
			free(pom);
		}
	}
	return 0;
}

int Leftmost(s_btree *root, s_stack *stack)
{
	s_stack pom;

	while(*root!=NULL)
	{
		if(Spush(stack,root)==-1)
		{
			while(*stack!=NULL)
			{
				pom=*stack;
				*stack=(*stack)->next;
				free(pom);
			}
			return -1;
		}
		root=&((*root)->lptr);
	}

	return 0;
}

char **BTlistof(s_btree *root, e_ntype ntype, int *num, int list_ena)
{
	char **arr=NULL;
	s_stack stack;

	*num=0;

	if(root==NULL || num==NULL || list_ena<0 || list_ena>1)
	{
		*num=-1;
		return NULL;
	}

	Sinit(&stack);
	if(Leftmost(root,&stack)==-1)
	{
		*num=-2;
		return NULL;
	}
	while(!Sempty(&stack))
	{
		root=Spop(&stack);

		if((*root)->ntype==ntype)
		{
			if(list_ena==1)
			{
				if(arr==NULL)
				{
					if((arr=malloc(50*sizeof(char *)))==NULL)
					{
						*num=-2;
						return NULL;
					}
				}
				else if(*num%50==0)
				{
					if((arr=realloc(arr,(*num+50)*sizeof(char *)))==NULL)
					{
						*num=-2;
						free(arr);
						return NULL;
					}
				}
				arr[*num]=(*root)->key;
			}
			(*num)++;
		}

		if(Leftmost(&((*root)->rptr),&stack)==-1)
		{
			*num=-2;
			free(arr);
			return NULL;
		}
	}

	return arr;
}