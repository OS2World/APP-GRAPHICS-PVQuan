/************************************************************************
 *                                                                      *
 *                  Copyright (c) 1991, Frank van der Hulst             *
 *                          All Rights Reserved                         *
 *                                                                      *
 * Authors:                                                             *
 *		  FvdH - Frank van der Hulst (Wellington, NZ)                     *
 *                                                                      *
 * Versions:                                                            *
 *      V1.1 910626 FvdH - QUANT released for DBW_RENDER                *
 *      V1.2 911021 FvdH - QUANT released for PoV Ray                   *
 *                                                                      *
 ************************************************************************/
/*

VGA 320 * 400 * 256 mode display module.

This file contains routines to display images in 320*400 mode by modifying
the VGA registers, as outlined in Programmer's Journal V7.1 (Jan/Feb '89)
article, pages 18-30, by Michael Abrash.

The advantage of 320 * 400, is that it gives two separate video pages, which
can be displayed on the screen independently. These contain two views of a
scene, taken from slightly different viewpoints. These are displayed
alternately on the screen, in sync with a pair of "chopper glasses", to give
a 3D effect.
*/

#include <dos.h>
#include <mem.h>

#include "vga.h"

static unsigned int act_page = 0;		/* Current page being written to */

#define VGA_SEGMENT        0xa000
#define SC_INDEX           0x3c4
#define GC_INDEX           0x3ce
#define CRTC_INDEX         0x3d4

#define MAP_MASK           2
#define MEMORY_MODE        4
#define GRAPHICS_MODE      5
#define MISCELLANEOUS      6
#define MAX_SCAN_LINE      9
#define START_ADDRESS_HIGH 0x0c
#define UNDERLINE          0x14
#define MODE_CONTROL       0x17

void putpixel_400(unsigned int x, unsigned int y, unsigned char colour)
{
	char far *addr;

	addr = MK_FP(VGA_SEGMENT, (x >> 2) + 320/4 * y + act_page);
	outport(SC_INDEX, (0x100 << (x & 3)) | MAP_MASK);
	*addr = colour;
}

void set320x400mode(void)
{
	struct REGPACK regs;
	unsigned char x;

	regs.r_ax = 0x13;				/* Set 320*200*256 graphics mode via BIOS */
	intr(0x10, &regs);

/* Change CPU addressing of video memory to linear (not odd/even, chain, or
	chain 4), to allow access to all 256K of display memory. Each byte will now
	control one pixel, with 4 adjacent pixels at any given address, one pixel
	per plane. */

	outportb(SC_INDEX, MEMORY_MODE);
	x = inportb(SC_INDEX+1);
	x &= 0xf7;									/* Turn off chain 4  */
	x |= 4;										/* Turn off odd/even */
	outportb(SC_INDEX+1, x);
	outportb(GC_INDEX, GRAPHICS_MODE);
	x = inportb(GC_INDEX+1);
	x &= 0xef;									/* Turn off odd/even */
	outportb(GC_INDEX+1, x);
	outportb(GC_INDEX, MISCELLANEOUS);
	x = inportb(GC_INDEX+1);
	x &= 0xfd;									/* Turn off chain */
	outportb(GC_INDEX+1, x);

/* Now clear the whole screen, since the mode 13h set only clears 64K. Do this
	before switching CRTC out of mode 13h, so that we don't see grabage on the
	screen. */

	outport(SC_INDEX, 0x0f00 | MAP_MASK);			/* Write to 4 planes at once */
	setmem(MK_FP(VGA_SEGMENT, 0), 0xffff, 0);

/* Change mode to 320*400 by not scanning each line twice. */
	outportb(CRTC_INDEX, MAX_SCAN_LINE);
	x = inportb(CRTC_INDEX+1);
	x &= 0xe0;								/* Set maximum scan line to 0 */
	outportb(CRTC_INDEX+1, x);

/* Change CRTC scanning from doubleword to byte mode, allowing the CRTC to
	scan more than 64K */
	outportb(CRTC_INDEX, UNDERLINE);
	x = inportb(CRTC_INDEX+1);
	x &= 0xbf;					/* Turn off doubleword */
	outportb(CRTC_INDEX+1, x);
	outportb(CRTC_INDEX, MODE_CONTROL);
	x = inportb(CRTC_INDEX+1);
	x |= 0x40;					/* Turn on the byte mode bit, so memory is linear */
	outportb(CRTC_INDEX+1, x);
	outport(CRTC_INDEX, 0 | START_ADDRESS_HIGH); /* Start on page 0 */
}

void end320x400mode(void)
{
	struct REGPACK regs;

	regs.r_ax = 3;				/* Return to text mode */
	intr(0x10, &regs);
}

void setvis_400(int page)
{
	outport(CRTC_INDEX, (page << 15) | START_ADDRESS_HIGH);
}

char far *setact_400(int page)
{
	act_page = page ? 0x8000 : 0;
	return MK_FP(VGA_SEGMENT, act_page);
}
