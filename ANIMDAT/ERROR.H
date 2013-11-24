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
/* error.h	Module for standardizing method of	*/
/*		error handling across all modules.	*/
/*------------------------------------------------------*/

#ifndef error_h
#define error_h

/* error codes */
typedef enum {
	NO_ERROR,		SYNTAX_ERROR,		FAILED_FILE_OPEN,
	INVALID_NUMBER,		MISSING_RPAREN,		INVALID_EXPRESSION,
	MISSING_IDENTIFIER,	STACK_OVERFLOW,		FAILED_MALLOC,
	SYMBOL_REDEFINED,	LINE_TOO_LONG,		UNDEFINED_SYMBOL,
	FUNCTION_NOT_SUPPORTED,	NO_NUMSCENE,		FILE_ERROR
	} ERROR_CODE;


void error(ERROR_CODE code,char *error_string);

#endif
