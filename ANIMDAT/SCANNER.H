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
/* scanner.h	Interface for scanning and tokeninzing	*/
/*		an expression.				*/
/*------------------------------------------------------*/

#ifndef scanner_h
#define scanner_h

/* token types */
typedef enum {
		NO_TOKEN,	IDENTIFIER,	NUMBER,		STRING,
		CARET,		STAR,		LPAREN,		RPAREN,
		MINUS,		PLUS,		EQUAL,		LT,
		GT,		LE,		GE,		NE,
		SLASH,		COMMA,		OR,		AND,
		SIN,		COS,		TAN,		EXP,
		LOG,		RND,		ATAN,		ASIN,
		ACOS,		POUND,		QUOTE,		ERROR,
		NUMSCENE,	PERCENT,	END_OF_FILE
		} TOKEN_CODE;


/* globals for accessing tokens */
extern TOKEN_CODE	token;
extern char		word_string[];
extern double		literal_value;
extern char		*token_names[];

/* routines for processing tokens */
void init_scanner(char *buffer_ptr);	/* Set the scanner to a point in the
					   source buffer and start scanning
					   from that point. */
void get_token(void);	/* updates the token global variable */

#endif
