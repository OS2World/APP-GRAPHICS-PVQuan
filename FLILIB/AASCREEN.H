/* aascreen.h  Copyright 1990 Dancing Flame, San Francisco */

#ifndef AASCREEN_H
#define AASCREEN_H

#include <stdlib.h>

/* Graphics types */
typedef unsigned char Cmap;
typedef unsigned char Pixel;
typedef unsigned char Bitplane;

/* Constants pertaining to 320x200 256 color mode mostly */
#define AA_VGA_SCREEN ((Pixel *)0xa0000000L)
#define AA_XMAX 320
#define AA_YMAX 200
#define AA_BPR 320
#define AA_COLORS 256

/* This structure is something we can draw on.  A superset of Vcel
   (and keep it that way or things break!)  */
struct vscreen
	{
	int x, y;	/* upper left corner in screen coordinates */
	unsigned w, h;	/* width, height */
	unsigned bpr;	/* bytes per row of image p */
	Pixel *pmap;	/* Screen memory map */
	Cmap *cmap;
	unsigned psize;	/* size of pixels */
	};
typedef struct vscreen Vscreen;

#ifdef __TURBOC__
extern Vscreen aa_screen;
extern Cmap aa_colors[];	/* software echo of color map */

Boolean aa_open_vga_screen(void);	/* opens 256 color screen */
void aa_close_vga_screen(void);
void far cdecl aa_wait_vblank(void);
#endif

/* Open a screen can draw on but not see */
Vscreen *aa_alloc_mem_screen(void);
/* For screens not full size */
Vscreen *aa_alloc_mem_cel(int x, int y, int w, int h);
void aa_free_mem_screen(Vscreen *ms);	/* dispose of a memory screen */
void aa_copy_screen(Vscreen *source, Vscreen *dest);
void aa_clear_screen(Vscreen *vs);

#endif /* AASCREEN_H */
