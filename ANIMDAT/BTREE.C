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
/* btree.c	Routines for creating and evaluating a binary	*/
/*		tree representation of a mathematical		*/
/*		expression. Requires the scanner and error	*/
/*		modules.					*/
/*--------------------------------------------------------------*/


#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "common.h"


/* Local procedures */
static btree_node_ptr	alloc_node(void);
static btree_node_ptr	simple_expression(void);
static btree_node_ptr	term(void);
static btree_node_ptr	factor(void);
static btree_node_ptr	unop(void);


/*--------------------------------------------------------------*/
/* btree_node_ptr	Allocate a node for a binary tree.	*/
/*--------------------------------------------------------------*/
static btree_node_ptr alloc_node()
{
 btree_node_ptr	np;

 np=(btree_node_ptr)malloc(sizeof(btree_node));
 if (np==NULL)
	error(FAILED_MALLOC,NULL);

 np->left = NULL;
 np->right = NULL;
 return (np);
}



/*----------------------------------------------------------------------*/
/* expression		Process an expression consisting of a simple	*/
/*			expression optionally followed by a relational	*/
/*			operator and a simple expression.		*/
/*----------------------------------------------------------------------*/
btree_node_ptr expression(void)
{
 btree_node_ptr	ltree,btree;

 /* Parse first simple expression */
 btree=simple_expression();

 /* Now if there is a relational operator, remember it and process the
    second simple expression */
 if ( (token == EQUAL) || (token == LT) || (token == GT) ||
      (token == NE) || (token == LE) || (token == GE) ) {

	ltree = btree;
	btree = alloc_node();
	btree->node_type = BINARYOP;
	btree->node_data.operator = token;

	get_token();		/* required by scanner module after using
				   operator token			 */

	btree->left = ltree;
	btree->right = simple_expression();
	}

 return (btree);
}



/*----------------------------------------------------------------------*/
/* simple_expression	Process a simple expression consisting of terms	*/
/*			separated by +,-, or OR operators. There may be	*/
/*			a unary + or - in front of the first term.	*/
/*----------------------------------------------------------------------*/
static btree_node_ptr simple_expression(void)
{
 btree_node_ptr	btree,ltree;

 btree = term();

 /* Loop to process all terms separated by operators */
 while ( (token == PLUS) || (token == MINUS) || (token == OR) ) {
	ltree = btree;
	btree = alloc_node();
	btree->node_type = BINARYOP;
	btree->node_data.operator = token;
	get_token();
	btree->left = ltree;
	btree->right = term();
	}

 return (btree);
}


/*----------------------------------------------------------------------*/
/* term			Process a term consisting of factors separated	*/
/*			by *, /, %, or AND operators.			*/
/*----------------------------------------------------------------------*/
btree_node_ptr term()
{
 btree_node_ptr btree,ltree;

 btree = factor();

 /* Loop to process all factors */
 while ( (token == STAR) || (token == SLASH) ||
	 (token == AND) || (token == PERCENT)) {
	ltree = btree;
	btree = alloc_node();
	btree->node_type = BINARYOP;
	btree->node_data.operator = token;
	get_token();
	btree->left = ltree;
	btree->right = factor();
	}

 return (btree);
}


/*----------------------------------------------------------------------*/
/* factor		Process a factor which consists of unops	*/
/*			separated by ^ (exponentiation) symbols.	*/
/*----------------------------------------------------------------------*/
btree_node_ptr factor()
{
 btree_node_ptr btree,ltree;

 btree = unop();

 while ( (token == CARET) ) {
	ltree = btree;
	btree = alloc_node();
	btree->node_type = BINARYOP;
	btree->node_data.operator = token;
	get_token();
	btree->left = ltree;
	btree->right = unop();
	}

 return (btree);
}


/*----------------------------------------------------------------------*/
/* unop			Process all unary operator symbols consisting	*/
/*			of +, -, SIN, COS, TAN, EXP, LOG, RND		*/
/*			followed by an					*/
/*			identifier, a number, or a parenthesized	*/
/*			subexpression.					*/
/*----------------------------------------------------------------------*/
btree_node_ptr unop()
{
 btree_node_ptr btree;

 switch (token) {

	case IDENTIFIER :
	case NUMSCENE :
		btree = alloc_node();
		btree->node_type = NAMEDVAR;
		btree->node_data.name = (char *)malloc(strlen(word_string)+1);
		strcpy(btree->node_data.name,word_string);
		btree->left = NULL;
		btree->right = NULL;
		get_token();
		break;

	case NUMBER :
		btree = alloc_node();
		btree->node_type = NUMBERVAL;
		btree->node_data.value = literal_value;
		btree->left = NULL;
		btree->right = NULL;
		get_token();
		break;

	case PLUS :
	case MINUS :
	case SIN :
	case COS :
	case TAN :
	case EXP :
	case LOG :
	case RND :
	case ASIN:
	case ACOS:
	case ATAN:
		btree = alloc_node();
		btree->node_type = UNARYOP;
		btree->node_data.operator = token;
		get_token();
		btree->right = NULL;
		btree->left = unop();
		break;

	case LPAREN :
		get_token();
		btree = expression();
		if ( token == RPAREN)
			get_token();
		else
			error(MISSING_RPAREN,cur_line);
		break;

	default :
		error(INVALID_EXPRESSION,cur_line);
		break;
	}

 return (btree);
}


/*--------------------------------------------------------------*/
/* display_btree	Display a binary tree in RPN form.	*/
/*--------------------------------------------------------------*/
void display_btree(btree_node_ptr btree)
{
 if (btree != NULL) {
	display_btree(btree->left);
	display_btree(btree->right);
	switch (btree->node_type) {
		case BINARYOP :
		case UNARYOP :
			printf(" %s",token_names[btree->node_data.operator]);
			break;
		case NUMBERVAL :
			printf(" %lf",btree->node_data.value);
			break;
		case NAMEDVAR :
			printf(" %s",btree->node_data.name);
			break;
		default :
			error(FUNCTION_NOT_SUPPORTED,NULL);
		}
	}
}




/*--------------------------------------------------------------*/
/* eval_binary		Evaluate the left and right subtrees	*/
/*			of a BINARYOP, apply the operator to 	*/
/*			the results.				*/
/*--------------------------------------------------------------*/
double eval_binary(btree_node_ptr btree)
{
 double left,right;

 left = eval_btree(btree->left);
 right = eval_btree(btree->right);

 switch (btree->node_data.operator) {
	case PLUS :	return (left + right);
	case MINUS :	return (left - right);
	case STAR :	return (left * right);
	case SLASH :	return (left / right);
	case PERCENT :	return fmod(left,right);
	case CARET :	return ( exp(right * log(left)) );
	case LT :	return ( (double)(left < right) );
	case GT :	return ( (double)(left > right) );
	case LE :	return ( (double)(left <= right) );
	case GE :	return ( (double)(left >= right) );
	case EQUAL :	return ( (double)(left == right) );
	case NE :	return ( (double)(left != right) );
	default :	error(FUNCTION_NOT_SUPPORTED,token_names[btree->node_data.operator]);
	}
 return (0.0);
}


/*--------------------------------------------------------------*/
/* eval_unary		Evaluate the left subtree of a UNARYOP	*/
/*			node and apply the operator to the	*/
/*			result.					*/
/*--------------------------------------------------------------*/
double eval_unary(btree_node_ptr btree)
{
 double left;

 left = eval_btree(btree->left);

 switch (btree->node_data.operator) {
	case PLUS :	return (left);
	case MINUS :	return ( (-1.0)*left);
	case SIN :	return (sin(left));
	case COS :	return (cos(left));
	case TAN :	return (tan(left));
	case EXP :	return (exp(left));
	case LOG :	return (log(left));
	case RND :	srand((int)left); return ( (double)(rand())/(double)(RAND_MAX) );
	case ASIN :	return (asin(left));
	case ACOS :	return (acos(left));
	case ATAN :	return (atan(left));
	default :	error(FUNCTION_NOT_SUPPORTED,token_names[btree->node_data.operator]);
	}
 return (0.0);
}



/*--------------------------------------------------------------*/
/* eval_btree		Evaluate a binary tree.			*/
/*--------------------------------------------------------------*/
double eval_btree(btree_node_ptr btree)
{
 switch (btree->node_type) {
	case NUMBERVAL: return (btree->node_data.value);

	case NAMEDVAR: return (eval_symbol(btree->node_data.name));

	case BINARYOP: return (eval_binary(btree));

	case UNARYOP: return (eval_unary(btree));

	default: error(SYNTAX_ERROR,NULL);
	}

 return (0.0);
}




/*--------------------------------------------------------------*/
/* traverse_btree	Recursively traverse a btree and call	*/
/*			the function once for each node.	*/
/*--------------------------------------------------------------*/
void traverse_btree(btree_node_ptr btree, void (*f)(btree_node_ptr btree))
{
 if (btree != NULL) {
	traverse_btree(btree->left,f);
	traverse_btree(btree->right,f);
	(*f)(btree);
	}
}
