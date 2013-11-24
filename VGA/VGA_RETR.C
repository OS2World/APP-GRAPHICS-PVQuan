/************************************************************************
 *                                                                      *
 *                  Copyright (c) 1991, Frank van der Hulst             *
 *                          All Rights Reserved                         *
 *                                                                      *
 * Authors:                                                             *
 *		  FvdH - Frank van der Hulst (Wellington, NZ)                     *
 *                                                                      *
 * Versions:                                                            *
 *    V1.1 910626 FvdH - QUANT released for DBW_RENDER                	*
 *    V1.2 911021 FvdH - QUANT released for PoV Ray                  	*
 *    V1.3 911030 FvdH - Added 320x200x256x4 pages support					*
 *                                                                      *
 ************************************************************************/
/*
VGA Vertical retrace detection module.
*/

#include <dos.h>

#include "vga.h"

#define DISPIO             0x3DA
#define VRT_bit            8

void WaitForVerticalRetrace(void)
{
static char chopper = 1;

	while (inportb(DISPIO) & VRT_bit) 			/* wait */ ;
	while ((inportb(DISPIO) & VRT_bit) == 0)	/* wait */ ;
	if ((chopper++ & 1)== 0)		outportb(0x3fc, 1);
	else									outportb(0x3fc, 3);
}

