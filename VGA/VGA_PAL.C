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

VGA 256 colour palette setting module.

*/

#include <dos.h>

#include "vga.h"

/* Setvgapalette sets the given number of VGA colours */
/* palette contains RGB values for 'colours' colours  */
/* R,G,B values range from 0 to 63	                  */

void setvgapalette(PALETTE *palette, int colours)
{
  struct REGPACK reg;

  reg.r_ax = 0x1012;
  reg.r_bx = 0;
  reg.r_cx = colours;
  reg.r_es = FP_SEG(palette);
  reg.r_dx = FP_OFF(palette);
  intr(0x10, &reg);
}
