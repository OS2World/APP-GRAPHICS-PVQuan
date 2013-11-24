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
/* symtab.h	Routines for interfacing to a symbol	*/
/*		table organized as a binary tree.	*/
/*------------------------------------------------------*/

#ifndef symtab_h
#define symtab_h

typedef enum {SYM_DOUBLE, SYM_FILE, SYM_STRING} sym_types;

typedef struct sym_s {
			char		*name;
			sym_types	sym_type;
			char		update_flag;
			void		*sym_info;
			double		cur_val;
			struct sym_s	*left;
			struct sym_s	*right;
			} sym, *sym_ptr;


sym_ptr	search_symtab(sym_ptr symtab, char *name);
sym_ptr	add_symbol(sym_ptr *symtab, char *name);
void	traverse_symtab(sym_ptr symtab,void (*f)(sym_ptr symbol) );
double	eval_symbol(char *name);
void	print_symbol(FILE *ofile, char *name);

#endif
