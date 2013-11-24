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

VGA 320 * 200 * 256 mode display module.

This code is based on an assembler source file by K. Heidenstrom.
It contains routines to display images in 320*200 mode by modifying
the VGA registers.

Advantages of 320 * 200:
-	Four separate video pages which can be displayed independently.
	These contain two views of a scene, taken from slightly different
	viewpoints. These are displayed alternately on the screen, in sync with
	a pair of "chopper glasses", to give a 3D effect. Four pages allows
	double-buffered animation.

-	Less than 64K bytes per image
*/

#include <dos.h>
#include <mem.h>

#include "vga.h"

#define SC_INDEX           0x3c4
#define GC_INDEX           0x3ce
#define CRTC_INDEX         0x3d4

#define VGA_SEGMENT			0xa000
#define MAP_MASK           2
#define MEMORY_MODE        4
#define GRAPHICS_MODE      5
#define MISCELLANEOUS      6
#define MAX_SCAN_LINE      9
#define START_ADDRESS_HIGH 0x0c
#define UNDERLINE          0x14
#define MODE_CONTROL       0x17

static unsigned int act_segment = VGA_SEGMENT;			/* Current page being written to */

void putpixel_200(unsigned int x, unsigned int y, unsigned char colour)
{
	char far *addr;

	addr = MK_FP(act_segment, y * (320/4) + x/4);
	outport(SC_INDEX, (0x0100 << (x & 3)) | MAP_MASK);	/* Write to only 1 plane */
	*addr = colour;
}

void set320x200x4mode(void)
{
	struct REGPACK regs;

	regs.r_ax = 0x13;				/* Set 320*200*256 graphics mode via BIOS */
	intr(0x10, &regs);

/* Setup Sequencer - disable Chain 4 bit for bitplane-architecture
	memory map */

	outport(SC_INDEX, 0x604);		/* Sequencer -- disable 'Chain 4' bit */

/* Set up CRTC for special video mode */

	outport(CRTC_INDEX, 0xe317);	/* CRTC -- Select "byte mode" */
	outport(CRTC_INDEX, 20);		/* Turn off "double word mode" */

/* Set up Graphics Controller for 256-colour mode, write mode 0, replace mode
	Following is default setup anyway - not strictly needed */

	outport(GC_INDEX, 0x4005);			/* Graphics controller address register */
											/* 256-colour mode, write mode zero */
	outport(GC_INDEX, 3);				/* Replace pixel values */

/* Now clear the whole screen, since the mode 13h set only clears 64K. Do this
	before switching CRTC out of mode 13h, so that we don't see garbage on the
	screen. */

	outport(SC_INDEX, 0x0f00 | MAP_MASK);			/* Write to 4 planes at once */
	setmem(MK_FP(VGA_SEGMENT, 0), 0xffff, 0);
}

void set320x200x1mode(void)
{
	struct REGPACK regs;

	regs.r_ax = 0x13;				/* Set 320*200*256 graphics mode via BIOS */
	intr(0x10, &regs);
}

void end320x200mode(void)
{
	struct REGPACK regs;

	regs.r_ax = 3;				/* Return to text mode */
	intr(0x10, &regs);
}

void setvis_200(int page)
{
	outport(CRTC_INDEX, (page << 14) | 0x0c);
}

char far *setact_200(int page)
{
	act_segment = VGA_SEGMENT + (page << 10);
	return MK_FP(act_segment, 0);
}
