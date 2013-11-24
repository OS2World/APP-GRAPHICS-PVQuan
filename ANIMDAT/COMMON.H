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
/* common.h		Common variables.		*/
/*------------------------------------------------------*/

#ifndef common_h
#define common_h

#include "error.h"
#include "scanner.h"
#include "btree.h"
#include "symtab.h"

#ifndef RAND_MAX			/* For use in random number generator */
#define RAND_MAX 32767			/* Not automatically defined in gcc */
#endif

extern sym_ptr		symbol_table;	/* Table of defined symbols as a binary tree */
extern char		cur_line[];	/* Line of source file currently being parsed */
extern char		*cur_file;	/* Name of file currently being parsed */
extern unsigned	int	cur_line_num;	/* Line number in file being parsed */

#endif
