/*--------------------------------------------------------------*/
/*			ANIMDAT 1.1				*/
/*		copyright 1992 - TODD SANKEY			*/
/*								*/
/*  The author hereby grants permission for the use and sharing	*/
/* of both source code end executable versions of this software	*/
/* at no charge. This software is not for sale and no other	*/
/* shall charge for it without the expressed consent of the	*/
/* author.							*/
/*								*/
/*  The source code can be freely modified, but it must retain	*/
/* the original copyright notice, and the author must be	*/
/* notified of these changes if the altered code is to be	*/
/* distributed.							*/
/*--------------------------------------------------------------*/
/*------------------------------------------------------*/
/* symtab.c	Routines for interfacing to a symbol	*/
/*		table organized as a binary tree.	*/
/*------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "common.h"

/*------------------------------------------------------*/
/* alloc_symbol		Allocate a symbol node.		*/
/*------------------------------------------------------*/
static sym_ptr alloc_symbol(void)
{
 sym_ptr sp;

 sp = (sym_ptr)malloc(sizeof(sym));
 if (sp == NULL)
	error(FAILED_MALLOC,NULL);

 sp->left = NULL;
 sp->right = NULL;
 sp->name = NULL;
 sp->sym_info = NULL;
 return (sp);
}


/*------------------------------------------------------*/
/* search_symtab	Search a symbol table for a	*/
/*			named symbol.			*/
/*------------------------------------------------------*/
sym_ptr search_symtab(sym_ptr symtab, char *name)
{
 int comp;

 if (symtab!=NULL)
	do {
		comp=strcmp(name,symtab->name);
		if (comp < 0)
			symtab = symtab->left;
		else if (comp > 0)
			symtab = symtab->right;
		} while (comp && symtab!=NULL);

 return (symtab);
}

/*------------------------------------------------------*/
/* add_symbol		Add a symbol to a symbol table	*/
/*			provided an equivalently named	*/
/*			symbol does not already exist.	*/
/*------------------------------------------------------*/
sym_ptr add_symbol(sym_ptr *symtab, char *name)
{
 sym_ptr sp,osp;
 int comp;

 if ((*symtab) == NULL) {
	*symtab = alloc_symbol();
	sp = (*symtab);
	}

 else {
	sp = (*symtab);
	do {
		comp=strcmp(name,sp->name);
		osp = sp;
		if (comp < 0)
			sp = sp->left;
		else if (comp > 0)
			sp = sp->right;
		} while (comp && (sp != NULL) );

	if (comp < 0) {
		osp->left = alloc_symbol();
		sp = osp->left;
		}
	else if (comp > 0) {
		osp->right = alloc_symbol();
		sp = osp->right;
		}
	else
		error(SYMBOL_REDEFINED,name);
	}

 sp->name = (char *)malloc(strlen(name)+1);
 strcpy(sp->name,name);

 return (sp);
}




/*------------------------------------------------------*/
/* traverse_symtab	Call a function once for each	*/
/*			node in a symbol table.		*/
/*------------------------------------------------------*/
void traverse_symtab(sym_ptr symtab,void (*f)(sym_ptr symbol))
{
 if (symtab != NULL) {
	traverse_symtab(symtab->left,f);
	(*f)(symtab);
	traverse_symtab(symtab->right,f);
	}
}



/*------------------------------------------------------*/
/* display_symbol	Display a symbol's name.	*/
/*------------------------------------------------------*/
static void display_symbol(sym_ptr symbol)
{
 printf(" %s",symbol->name);
}


/*------------------------------------------------------*/
/* display_symtab	Display every name in a symbol	*/
/*			table.				*/
/*------------------------------------------------------*/
void display_symtab(sym_ptr symtab)
{
 traverse_symtab(symtab,display_symbol);
}



/*------------------------------------------------------*/
/* eval_symbol		Evaluate a mathematical symbol. */
/*------------------------------------------------------*/
double eval_symbol(char *name)
{
 sym_ptr symbol;

 symbol = search_symtab(symbol_table,name);
 if (symbol == NULL)
	error(UNDEFINED_SYMBOL,name);

 if (!symbol->update_flag) {
	symbol->update_flag = 1;
	switch (symbol->sym_type) {
		case SYM_DOUBLE :
			symbol->cur_val = eval_btree((btree_node_ptr)(symbol->sym_info));
			break;
		case SYM_FILE :
			fscanf((FILE *)symbol->sym_info,"%lf",&(symbol->cur_val));
			break;
		default :
			sprintf(cur_line,"Symbol -%s- could not be evaluated numerically",name);
			error(SYNTAX_ERROR,cur_line);
		}
	}

 return (symbol->cur_val);
}


/*------------------------------------------------------*/
/* print_symbol		Print the value for a symbol	*/
/*			of any type.			*/
/*------------------------------------------------------*/
void print_symbol(FILE *ofile,char *name)
{
 sym_ptr symbol;

 symbol = search_symtab(symbol_table, name);
 if (symbol == NULL)
	error(UNDEFINED_SYMBOL, name);

 switch (symbol->sym_type) {
	case SYM_DOUBLE:
	case SYM_FILE:
			fprintf(ofile,"%lf",eval_symbol(name));
			break;
	case SYM_STRING:
			fprintf(ofile,"%s",(char *)symbol->sym_info);
			break;
	default:
			error(SYNTAX_ERROR,"Bad symbol type");
	}
}
