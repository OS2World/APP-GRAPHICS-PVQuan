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
/* scanner.c	Scans and tokenizes a source buffer.	*/
/*------------------------------------------------------*/

#define scanner_c

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include "common.h"

#define MAX_LINE_LENGTH	256
#define EOB_CHAR	0
#define SPECIAL		1
#define LETTER		2
#define	DIGIT		3

/* globals */
TOKEN_CODE	token;
char		word_string[MAX_LINE_LENGTH];
double		literal_value;
char *token_names[]={"No token","Identifier","Number","String","^","*",
			"(",")","-","+","=","<",">","<=",">=","<>","/",",",
			"OR","AND","SIN","COS","TAN","EXP","LOG","RND","ATAN",
			"ASIN","ACOS","#","quote",
			"ERROR","NUMSCENE","%","END OF BUFFER"};

/* variable used inside scanner module */
static int	ch;		/* current input character from source buffer */
static char	*buffer_offset;	/* offset into source buffer */
static char	*bufferp;	/* start of source buffer */
static char	token_string[MAX_LINE_LENGTH];
static char	*tokenp = token_string;
static int	digit_count;
static char	char_table[256];

static char	rw_2[]={'o','r',OR,0};
static char	rw_3[]={'a','n','d',AND,
			's','i','n',SIN,
			'c','o','s',COS,
			't','a','n',TAN,
			'e','x','p',EXP,
			'l','o','g',LOG,
			'r','n','d',RND,0};
static char	rw_4[]={'a','t','a','n',ATAN,
			'a','s','i','n',ASIN,
			'a','c','o','s',ACOS,0};
static char	rw_10[]={'n','u','m','_','s','c','e','n','e','s',NUMSCENE,0};

static char	*rsvd_word_table[] = { NULL, NULL, rw_2, rw_3, rw_4,NULL,NULL,NULL,NULL,NULL, rw_10 };
#define MAX_RESERVED_WORD_LENGTH 10
#define MIN_RESERVED_WORD_LENGTH 2


/* Local procedures */
void get_char(void);
void skip_comment(void);
void skip_blanks(void);
void get_word(void);
void get_number(void);
void get_special(void);
void get_quote(void);
void downshift_word(void);
int  is_reserved_word(void);


#define char_code(ch) char_table[ch]


	/********************************/
	/*				*/
	/* Initialization routines	*/
	/*				*/
	/********************************/


/*----------------------------------------------------------------------*/
/* init_scanner		Initialize the scanner globals and start the	*/
/*			scanner at the specified point.			*/
/*----------------------------------------------------------------------*/

void init_scanner(char *source_buffer)
{
 ch = 0;
 token = NO_TOKEN;
 word_string[0] = 0;
 literal_value = 0.0;
 token_string[0] = 0;
 tokenp = token_string;
 digit_count = 0;
 bufferp = source_buffer;
 buffer_offset = bufferp;

 for (ch = 0; ch<256; ch++) char_table[ch] = SPECIAL;
 for (ch = '0'; ch <='9'; ch++) char_table[ch] = DIGIT;
 for (ch = 'a'; ch <='z'; ch++) char_table[ch] = LETTER;
 for (ch = 'A'; ch <='Z'; ch++) char_table[ch] = LETTER;
 char_table[0] = EOB_CHAR;

 get_char();	/* Get first character of source buffer */
 get_token();	/* initialize to first token in buffer */
}



	/********************************/
	/*				*/
	/*	Character routines	*/
	/*				*/
	/********************************/

/*----------------------------------------------------------------------*/
/* get_char		Set ch to the next character from the source	*/
/*			buffer.						*/
/*----------------------------------------------------------------------*/

void get_char(void)
{
 ch = *buffer_offset++;
 switch (ch) {
	case '\0' :
		buffer_offset--;
		break;
	case '\t' :			/* Make tab and new-line characters */
	case '\n' :			/* appear as spaces. */
		ch = ' ';
		break;

	case '{' :			/* Ignore comment and make it appear */
		skip_comment();		/* as a space. */
		ch = ' ';
		break;
	}
}


/*----------------------------------------------------------------------*/
/* skip_comment		Skip over a comment. Set ch to '}'		*/
/*----------------------------------------------------------------------*/

void skip_comment()
{
 do {
	get_char();
	} while ( (ch != '}') && (ch != EOB_CHAR) );
}


/*----------------------------------------------------------------------*/
/* skip_blanks		Skip over white space				*/
/*----------------------------------------------------------------------*/

void skip_blanks()
{
 while (ch == ' ') get_char();
}



	/********************************/
	/*				*/
	/*	Token routines		*/
	/*				*/
	/********************************/
/* Note: after a token has been extracted, ch is the first character after
   the token. */

/*----------------------------------------------------------------------*/
/* get_token		Extract the next token from the source buffer.	*/
/*----------------------------------------------------------------------*/

void get_token()
{
 skip_blanks();
 tokenp = token_string;

 switch (char_code(ch)) {
	case LETTER:	get_word();		break;
	case DIGIT:	get_number();		break;
	case EOB_CHAR:	token = END_OF_FILE;	break;
	default:	get_special();		break;
	}
}


/*----------------------------------------------------------------------*/
/* get_word		Extract a word token and downshift its		*/
/*			characters. Check if its a reserved word. Set	*/
/*			token to IDENTIFIER if it's not.		*/
/*----------------------------------------------------------------------*/

void get_word()
{
 while ( (char_code(ch) == LETTER) || (char_code(ch) == DIGIT)
		|| (ch == '_') || (ch == '.') ) {
	*tokenp++ = ch;
	get_char();
	}
 *tokenp = '\0';
 downshift_word();
 if (!is_reserved_word() )
	token = IDENTIFIER;
}


/*----------------------------------------------------------------------*/
/* get_number		Extract a number token and set literal_value to	*/
/*			its value. Set token to NUMBER.			*/
/*----------------------------------------------------------------------*/

void get_number()
{
 double	real_part = 0.0, temp_real, tenths;
 long	whole_part = 0;

 /* Accumulate whole number part */
 while ( char_code(ch) == DIGIT) {
	whole_part = (10*whole_part) + (ch - '0');
	*tokenp++ = ch;
	get_char();
	}

 if (ch == '.') {
	tenths = 10.0;
	*tokenp++ = ch;
	get_char();
	while ( char_code(ch) == DIGIT) {
		temp_real = (double)(ch - '0');
		temp_real /= tenths;
		real_part+= temp_real;
		tenths *= 10.0;
		*tokenp++ = ch;
		get_char();
		}
	}

 *tokenp = '\0';
 real_part += (double)whole_part;
 token = NUMBER;
 literal_value = real_part;
}



/*----------------------------------------------------------------------*/
/* get_quote		Extract the literal contents between two	*/
/*			quotation marks.				*/
/*----------------------------------------------------------------------*/

void get_quote(void)
{
 int count;

 ch = *buffer_offset++;
 for (count=0 ; (count < MAX_LINE_LENGTH) && (ch != '"'); count++) {
	word_string[count] = ch;
	ch = *buffer_offset++;
	}
 if (count >= MAX_LINE_LENGTH)
	error(LINE_TOO_LONG,cur_line);

 word_string[count] = '\0';
 get_char();
}


/*----------------------------------------------------------------------*/
/* get_special		Extract a special token. Some are single	*/
/*			character and some are double. Set token	*/
/*			appropriately.					*/
/*----------------------------------------------------------------------*/

void get_special()
{
 *tokenp++ = ch;
 switch (ch) {
	case '^':	token = CARET;		get_char();	break;
	case '*':	token = STAR;		get_char();	break;
	case '(':	token = LPAREN;		get_char();	break;
	case ')':	token = RPAREN;		get_char();	break;
	case '-':	token = MINUS;		get_char();	break;
	case '+':	token = PLUS;		get_char();	break;
	case '/':	token = SLASH;		get_char();	break;
	case '=':	token = EQUAL;		get_char();	break;
	case ',':	token = COMMA;		get_char();	break;
	case '#':	token = POUND;		get_char();	break;
	case '"':	token = QUOTE;		get_quote();	break;
	case '%':	token = PERCENT;	get_char();	break;
	case '<':	get_char();
			if (ch == '=') {	/* <= */
				*tokenp++ = ch;
				token = LE;
				get_char();
				}
			else if (ch == '>') {	/* <> */
				*tokenp++ = ch;
				token = NE;
				get_char();
				}
			else
				token = LT;
			break;

	case '>':	get_char();
			if (ch == '=') {	/* >= */
				*tokenp++ = ch;
				token = GE;
				get_char();
				}
			else
				token = GT;
			break;

	case '!':	get_char();
			if (ch == '=') {
				*tokenp++ = ch;
				token = NE;
				get_char();
				}
			else
				token = ERROR;
			break;

	default:	token = ERROR;
			get_char();
			break;
	}
 *tokenp = '\0';
}


/*----------------------------------------------------------------------*/
/* downshift_word	Copy a word token into word_string with all	*/
/*			characters converted to lower case.		*/
/*----------------------------------------------------------------------*/

void downshift_word()
{
 char	*wp = word_string;
 char	*tp = token_string;

 do {
	*wp++ = (*tp++);
	} while ( (*tp) != '\0' );
 *wp = '\0';
}


/*----------------------------------------------------------------------*/
/* is_reserved_word	Checks if a word token is a reserved word. If	*/
/*			so, set token appropriately and return 1.	*/
/*			Otherwise return 0.				*/
/*----------------------------------------------------------------------*/

int is_reserved_word()
{
 int word_length;
 char *rwp;

 word_length = strlen(word_string);
 if (word_length >= MIN_RESERVED_WORD_LENGTH && word_length <= MAX_RESERVED_WORD_LENGTH) {
	for (rwp = rsvd_word_table[word_length];
		(rwp != NULL) && (*rwp != '\0') ;
		rwp += (word_length+1) ) {
		if (!strncmp(word_string,rwp,word_length) ) {
			token=(TOKEN_CODE)( *(rwp+word_length));
			return (1);
			}
		}
	}
 return (0);
}
