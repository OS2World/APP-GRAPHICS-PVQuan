/************************************************************************
 *                                                                      *
 *                  Copyright (c) 1991, Frank van der Hulst             *
 *                          All Rights Reserved                         *
 *                                                                      *
 * Authors:                                                             *
 *    FvdH - Frank van der Hulst (Wellington, NZ)                       *
 *                                                                      *
 * Versions:                                                            *
 *    V1.1 910626 FvdH - QUANT released for DBW_RENDER                  *
 *    V1.2 911021 FvdH - QUANT released for PoV Ray                     *
 *    V1.6 921023 FvdH - Produce multi-image GIFs                       *
 *                     - Port to OS/2 IBM C Set/2                       *
 *                                                                      *
 ************************************************************************/
/* octree.h -- definitions for octree.c */

#ifdef __GNUC__
#define cdecl
#endif

struct colorsum {
	unsigned long r, g, b;
};

typedef struct node * OCTREE;
struct node {
	unsigned char leaf;
	unsigned char level;
	unsigned char colorindex;
	unsigned char children;
	unsigned long colorcount;
	struct colorsum rgbsum;
	OCTREE nextreduceable;
	OCTREE next[8];
};

void generateoctree(void);
UINT calc_palette(UINT i, double Cfactor);
