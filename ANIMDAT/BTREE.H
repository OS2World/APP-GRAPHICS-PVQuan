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
/* btree.h	Routines for creating a binary tree	*/
/*		based on an expression.			*/
/*------------------------------------------------------*/

#ifndef btree_h
#define btree_h

/* Possible node types */
typedef enum { NAMEDVAR, BINARYOP, UNARYOP, NUMBERVAL } node_types;

/* Node data structure */
typedef struct btree_node_s {
				node_types	node_type;
				union	{
					char		*name;
					TOKEN_CODE	operator;
					double		value;
					} node_data;
				struct btree_node_s	*left;
				struct btree_node_s	*right;
				} btree_node , *btree_node_ptr;


/* Interface routines for creating and evaluating a tree */
btree_node_ptr	expression(void);
void		display_btree(btree_node_ptr btree);
double		eval_btree(btree_node_ptr btree);
void		traverse_btree(btree_node_ptr btree, void (*f)(btree_node_ptr));
#endif
