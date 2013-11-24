/************************************************************************
 *                                                                      *
 *                  Copyright (c) 1991, Frank van der Hulst             *
 *                          All Rights Reserved                         *
 *                                                                      *
 * Authors:                                                             *
 *      FvdH - Frank van der Hulst (Wellington, NZ)                     *
 *                                                                      *
 * Versions:                                                            *
 *    V1.1 910626 FvdH - QUANT released for DBW_RENDER                  *
 *    V1.2 911021 FvdH - QUANT released for PoV Ray                     *
 *    V1.6 921023 FvdH - Produce multi-image GIFs                       *
 *                     - Port to OS/2 IBM C Set/2                       *
 *                                                                      *
 ************************************************************************/
/* This code implements the alogrithm described in:
 * Graphic Gems edited by Andrew Glassner
 * Article: A Simple Method for Color Quantisation:
 *          Octree Quantisation, pg. 287 ff
 * by:      Michael Gervautz, Werner Purgathofer
 *          Technical University Vienna
 *          Vienna, Austria
 */

/* Written by wRZL (Wolfgang Stuerzlinger) */

/* This code is hereby placed in public domain */

#include <string.h>
#include "quant.h"
#include "octree.h"

#define RGB(r,g,b) ((((unsigned long)(b) << input_bits) | ((g) << input_bits)) | (r))
#define TESTBIT(a,i) ( ((a) >> (i)) & 1)
#define MAXDEPTH 7

static UINT size;
static UINT reducelevel;
static UINT leaflevel;
static OCTREE tree;
static OCTREE reducelist[MAXDEPTH + 1];

static unsigned char quant_r,	/* Originally a parameter for quant2(), */
							quant_g,	/* moved here to reduce stack usage */
							quant_b;

static char quant2(OCTREE tree)
{
	if (tree->leaf)   return(tree->colorindex);
	else					return(quant2(tree->next[
								TESTBIT(quant_r, MAXDEPTH - tree->level) * 4 +
								TESTBIT(quant_g, MAXDEPTH - tree->level) * 2 +
								TESTBIT(quant_b, MAXDEPTH - tree->level)]));
}

int pal_index(UCHAR *p)
{
	quant_r = p[RED];
	quant_g = p[GREEN];
	quant_b = p[BLUE];
	return quant2(tree);
}

static double init_Cfactor;
static UINT init_col_num;

static void initpalette(OCTREE tree)
{
	UINT j;

	if (tree == NULL) return;
	if (tree->leaf || tree->level == leaflevel) {
		palette[init_col_num][RED]   = (char) ((init_Cfactor * tree->rgbsum.r) / tree->colorcount + 0.5);
		palette[init_col_num][GREEN] = (char) ((init_Cfactor * tree->rgbsum.g) / tree->colorcount + 0.5);
		palette[init_col_num][BLUE]  = (char) ((init_Cfactor * tree->rgbsum.b) / tree->colorcount + 0.5);
		tree->colorindex = init_col_num;
		tree->leaf = TRUE;
		init_col_num++;
	} else {
		for (j = 0; j < 8; j++)
			initpalette(tree->next[j]);
	}
}

UINT calc_palette(UINT i, double Cfactor)
{
	init_Cfactor = Cfactor;
	init_col_num = i;
	initpalette(tree);
	return init_col_num;
}


static void newandinit(OCTREE *tree, UINT depth)
    {
	*tree = (OCTREE)calloc(1,sizeof(struct node));
	if (*tree == NULL) {
        printf("out of memory");
        err_exit();
        }
    (*tree)->level = depth;
    (*tree)->leaf = (depth >= leaflevel);
    if ((*tree)->leaf)
        size++;
    }

static void getreduceable(OCTREE *node)
    {
    UINT newreducelevel;

    newreducelevel = reducelevel;
    while (reducelist[newreducelevel] == NULL)
        newreducelevel--;
    *node = reducelist[newreducelevel];
    reducelist[newreducelevel] =
                reducelist[newreducelevel]->nextreduceable;
    }

static void makereduceable(UINT level,OCTREE node)
{
	node->nextreduceable = reducelist[level];
	reducelist[level] = node;
}

static void reducetree(void)
{
	OCTREE node;
	UINT depth;

	getreduceable(&node);
	node->leaf = 1;
	size = size - node->children + 1;
	depth = node->level;
	if (depth < reducelevel) {
		reducelevel = depth;
		leaflevel = reducelevel + 1;
	}
}

static UCHAR insert_rgb[3];   /* Originally a parameter for inserttree(), moved
										here to reduce stack usage */

static void inserttree(OCTREE *tree, UINT depth)
{
	UINT branch;

	if (*tree == NULL)
		newandinit(tree,depth);
	(*tree)->colorcount++;
	(*tree)->rgbsum.r += insert_rgb[RED];
	(*tree)->rgbsum.g += insert_rgb[GREEN];
	(*tree)->rgbsum.b += insert_rgb[BLUE];
	if ((*tree)->leaf == FALSE && depth < leaflevel) {
		branch = TESTBIT(insert_rgb[RED],MAXDEPTH - depth) * 4 +
					TESTBIT(insert_rgb[GREEN],MAXDEPTH - depth) * 2 +
					TESTBIT(insert_rgb[BLUE],MAXDEPTH - depth);
		if ((*tree)->next[branch] == NULL) {
			(*tree)->children++;
			if ((*tree)->children == 2)
				makereduceable(depth,*tree);
		}
		inserttree(&((*tree)->next[branch]), depth + 1);
	}
}

void generateoctree(void)
{
	reducelevel = MAXDEPTH;
	leaflevel = reducelevel + 1;

	while (get_pixel(insert_rgb)) {
		inserttree(&tree, 0);
		if (size > MAXCOLORS - 1)		/* max number of colors ! */
			reducetree();
	}
}
