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
/* error.c		Default error handler.		*/
/*------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include "common.h"

char *error_msg[] = {	"No error","Syntax error","Failed file open",
			"Invalid number","Missing right parenthesis",
			"Invalid expression","Missing identifier",
			"Stack overflow","Failed memory allocation",
			"Symbol redefined","Line too long in input file",
			"Symbol not defined","Unsupported function",
			"Nothing to do - num_scenes not defined",
			"File system error"
			};

void error(ERROR_CODE code,char *error_string)
{
 if (error_string != NULL)
	printf("\n%s\n",error_string);
 printf("%s: %s",cur_file,error_msg[code]);
 if (cur_line_num)
	printf(" on line %u",cur_line_num);
 printf("\n");
 exit(1);
}
