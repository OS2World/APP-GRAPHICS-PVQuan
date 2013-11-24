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


/*--------------------------------------------------------------*/
/* animdat.c		Main file for animdat program.		*/
/*--------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "common.h"

/*  This definition controls what filenames are used for the generated	*/
/* files. If LONG_FILENAMES is not defined, then the generated files	*/
/* are named SCENEnnn.DAT where nnn is the number of the scene. If	*/
/* LONG_FILENAMES is defined, the generated filenames use the root name	*/
/* given on the command line and add _nnn.DAT .				*/
/*  This option is given because DOS machines have the limit of 8	*/
/* characters in the filename, so LONG_FILENAMES should not be used.	*/
#define	LONG_FILENAMES

#ifdef LONG_FILENAMES
#define MAX_FNAME_SIZE 100
#else
#define MAX_FNAME_SIZE 13
#endif

#define NAMELEN 20
#define NUMLEN 20
#define MAXSCENES 100
#define VARCHAR '@'
#define GATECHAR '&'
#define MAX_LINE_SIZE 256


/* Globals used in other modules */
int		numscenes = -1;
sym_ptr		symbol_table = NULL;
char		cur_line[MAX_LINE_SIZE];
char		*cur_file = NULL;
unsigned int	cur_line_num = 0;


/* Used within animdat.c */
static sym_ptr	x_symbol = NULL;
static sym_ptr	cur_sc_symbol = NULL;
static sym_ptr	num_sc_symbol = NULL;
static char	*cur_symbol = NULL;
static int	gate_closed = 0;


/*--------------------------------------------------------------*/
/* parse_equation	Attaches a binary tree representation	*/
/*			of a mathematical expression to a symbol*/
/*			given that the scanner module is	*/
/*			initialized to the first token in the	*/
/*			expression.				*/
/*--------------------------------------------------------------*/
void parse_equation(sym_ptr new_symbol)
{
 new_symbol->sym_info = (void *)expression();
 new_symbol->sym_type = SYM_DOUBLE;
 if (token == COMMA) {
	double sign_flag = 1.0;
	get_token();
	if (token == MINUS) {
		sign_flag = (-1.0);
		get_token();
		}
	else if (token == PLUS)
		get_token();

	if (token == NUMBER)
		new_symbol->cur_val = sign_flag * literal_value;
	else
		error(SYNTAX_ERROR,cur_line);
	}

}



/*--------------------------------------------------------------*/
/* parse_filename	Attaches a file pointer to a symbol.	*/
/*--------------------------------------------------------------*/
void parse_filename(sym_ptr new_symbol)
{
 get_token();
 new_symbol->sym_type = SYM_FILE;
 new_symbol->sym_info = (void *)fopen(word_string,"r");
 if (new_symbol->sym_info == NULL)
	error(FILE_ERROR,word_string);
 get_token();
}


/*--------------------------------------------------------------*/
/* parse_string		Attaches a string pointer to a symbol.	*/
/*--------------------------------------------------------------*/
void parse_string(sym_ptr new_symbol)
{
 char *quote;

 quote=(char *)malloc(strlen(word_string)+1);
 if (quote == NULL)
	error(FAILED_MALLOC,NULL);

 strcpy(quote,word_string);
 new_symbol->sym_type = SYM_STRING;
 new_symbol->sym_info = (void *)quote;
 get_token();
}




/*--------------------------------------------------------------*/
/* parsevarfile		Attempts to parse a file as a sequence	*/
/*			of lines each containing a symbol name	*/
/*			and a definition.			*/
/*--------------------------------------------------------------*/
void parsevarfile(FILE *varfile)
{
 int i;
 sym_ptr new_symbol;

 cur_line_num = 0;
 while (!feof(varfile)) {
	/* Read the current line of the file into a buffer */
	if (fgets(cur_line,MAX_LINE_SIZE,varfile) != NULL) {
		cur_line_num++;
		for (i=0; i<MAX_LINE_SIZE && cur_line[i]!='\0'; i++);
		if (cur_line[i]!='\0' || i>=MAX_LINE_SIZE)
			error(LINE_TOO_LONG,cur_line);

		init_scanner(cur_line);

		if (token == IDENTIFIER) {
			new_symbol = add_symbol(&symbol_table, word_string);
			get_token();
			if (token != EQUAL)
				error(SYNTAX_ERROR,cur_line);
			get_token();
			switch (token) {
				case POUND : parse_filename(new_symbol);break;
				case QUOTE : parse_string(new_symbol);	break;
				default    : parse_equation(new_symbol);
				}
			}
		else if (token == NUMSCENE) {
			get_token();
			if (token != EQUAL)
				error(SYNTAX_ERROR,cur_line);
			get_token();
			if (token != NUMBER)
				error(SYNTAX_ERROR,cur_line);
			numscenes = (int)literal_value;
			get_token();
			}
		else if (token != END_OF_FILE)
			error(SYNTAX_ERROR,cur_line);
		}
	}

 cur_line_num = 0;
}


/*--------------------------------------------------------------*/
/* add_system_variables		Creates the predefined symbols	*/
/*				but leaves them undefined.	*/
/*--------------------------------------------------------------*/
void add_system_variables(void)
{
 num_sc_symbol = add_symbol(&symbol_table,"num_scenes");
 x_symbol = add_symbol(&symbol_table,"x");
 cur_sc_symbol = add_symbol(&symbol_table,"cur_scene");
}




/*--------------------------------------------------------------*/
/* update_system_variables	Defines the system variables.	*/
/*--------------------------------------------------------------*/
void update_system_variables(void)
{
 char eq[MAX_LINE_SIZE];
 double xinc;

 xinc = 1.0 / ((double)numscenes);
 sprintf(eq,"x + %lf \0",xinc);
 init_scanner(eq);
 x_symbol->sym_info = (void *)expression();
 x_symbol->cur_val = 0.0;
 x_symbol->sym_type = SYM_DOUBLE;

 sprintf(eq,"cur_scene + 1 \0");
 init_scanner(eq);
 cur_sc_symbol->sym_info = (void *)expression();
 cur_sc_symbol->cur_val = 0.0;
 cur_sc_symbol->sym_type = SYM_DOUBLE;

 sprintf(eq,"%d \0",numscenes);
 init_scanner(eq);
 num_sc_symbol->sym_info = (void *)expression();
 num_sc_symbol->cur_val = (double)numscenes;
 num_sc_symbol->sym_type = SYM_DOUBLE;
}





/*--------------------------------------------------------------*/
/* check_btree		Scans a binary tree representation	*/
/*			of a mathematical expression checking	*/
/*			for undefined symbols.			*/
/*--------------------------------------------------------------*/
void check_btree(btree_node_ptr bnode)
{
 if (bnode->node_type == NAMEDVAR) {
	if (search_symtab(symbol_table,bnode->node_data.name) == NULL) {
		sprintf(cur_line,"%s in definition of %s",bnode->node_data.name,cur_symbol);
		error(UNDEFINED_SYMBOL,cur_line);
		}
	}
}




/*--------------------------------------------------------------*/
/* check_symbol		Verifies the definition of a symbol.	*/
/*--------------------------------------------------------------*/
void check_symbol(sym_ptr symbol)
{
 cur_symbol = symbol->name;
 if (symbol->sym_type == SYM_DOUBLE)
	traverse_btree((btree_node_ptr)(symbol->sym_info),check_btree);
}




/*--------------------------------------------------------------*/
/* reset_flags		Clears the flag in a symbol which	*/
/*			indicates that the symbol value has	*/
/*			already been evaluated for this scene.	*/
/*--------------------------------------------------------------*/
void reset_flags(sym_ptr symbol)
{
 symbol->update_flag = 0;
}




void main(int argc,char **argv)
{
 int	curscene,inchar,i;
 char	varname[MAX_LINE_SIZE];
 FILE	*datfile,*curscenefile,*varfile;
 char	dat_name[MAX_FNAME_SIZE];
 char	var_name[MAX_FNAME_SIZE];
 char	scene_name[MAX_FNAME_SIZE];

 if (argc != 2) {
	printf("ANIMDAT <root file>\n");
	printf("  where rootfile.dat is the template file\n");
	printf("  and   rootfile.var is the variable definition file\n");
	exit(1);
	}

 strcpy(dat_name, argv[1]);
 strcat(dat_name,".pov");
 datfile=fopen(dat_name,"r");
 if (datfile == NULL) {
	printf(" Could not open datfile: %s\n", dat_name);
	exit(1);
	}

 strcpy(var_name, argv[1]);
 strcat(var_name, ".var");
 varfile=fopen(var_name,"r");
 if (varfile == NULL) {
	printf(" Could not open varfile: %s\n", var_name);
	exit(1);
	}

 cur_file = var_name;
 add_system_variables();
 parsevarfile(varfile);
 fclose(varfile);
 update_system_variables();
 traverse_symtab(symbol_table,check_symbol);

 if (numscenes<=0)
	error(NO_NUMSCENE,NULL);

 cur_file = dat_name;
 cur_line_num = 1;

#ifndef LONG_FILENAMES
 strcpy(scene_name,"scene000.dat");
#endif

 for (curscene=1;curscene<=numscenes;curscene++) {

#ifdef LONG_FILENAMES
	sprintf(scene_name,"%s_%d.pov",argv[1],curscene - 1);
#else
	sprintf(scene_name,"scene%03d.pov",curscene - 1);
#endif

	curscenefile=fopen(scene_name,"w");

	if (curscenefile==NULL) {
		printf("Could not open file %s\n",scene_name);
		exit(1);
		}

	rewind(datfile);
	traverse_symtab(symbol_table,reset_flags);

	gate_closed = 0;
	while (!feof(datfile)) {
		inchar=fgetc(datfile);

		switch (inchar) {
		case VARCHAR :
			if (!gate_closed) {
				i = 0;
				do {
					inchar=fgetc(datfile);
					if (inchar != VARCHAR) {
						varname[i] = inchar;
						i++;
						}
					} while ( i < (MAX_LINE_SIZE - 1) && inchar !=VARCHAR);

				varname[i] = '\0';
				if (inchar!=VARCHAR)
					fprintf(curscenefile,"%c%s",VARCHAR,varname);
				else
					print_symbol(curscenefile,varname);
				}
			break;

		case GATECHAR :
			inchar = fgetc(datfile);
			if (isalpha(inchar)) {	/* Assume start of gate */
				i = 0;
				do {
					varname[i] = inchar;
					i++;
					inchar = fgetc(datfile);
					} while (i < (MAX_LINE_SIZE - 1) && (isalnum(inchar) || inchar == '_'));
				varname[i] = '\0';
				if ( (eval_symbol(varname) <= 0.0) || gate_closed)
					gate_closed++;
				}

			else if (gate_closed)
				gate_closed--; /* End of gate */
			break;

		case EOF :
			break;

		default:
			if (!gate_closed) {
				fputc(inchar,curscenefile);
				if (inchar == '\n')
					cur_line_num++;
				}



			}
		}

	fclose(curscenefile);
	}

}
