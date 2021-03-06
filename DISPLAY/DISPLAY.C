/************************************************************************
 *                                                                      *
 *                  Copyright (c) 1991, Frank van der Hulst             *
 *                          All Rights Reserved                         *
 *                                                                      *
 ************************************************************************
 *                                                                      *
 * Authors:                                                             *
 *		  FvdH - Frank van der Hulst (Wellington, NZ)                     *
 *                                                                      *
 * Versions:                                                            *
 *    V1.1 910626 FvdH - DISPLAY released as part of DBW_RENDER       	*
 *    V1.2 910913 FvdH - DISPLAY released as part of DKB_TRACE        	*
 *    V1.3 911031 FvdH - Added 320x200x256x4 pages support					*
 *                                                                      *
 ************************************************************************/

/* VGA 320*400*256 or 320*200*256 ray tracer display program.

Written by: F van der Hulst, 26/6/91

This program reads files produced by QUANT.EXE and displays them on a VGA
screen in 320*400 or 320*200 mode.

Command line format:
	display FILE1 [FILE2]... [/W] [/C] [/R]

	where FILE1, FILE2... are the names (with extensions) of files to be
			displayed.
	/W 	=> wait after each following image has been displayed
	/C    => clear screen after each image
	/R		=> reverse eye-image sync for following 3D images
*/

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <conio.h>
#include <dos.h>

#include "vga.h"

#define SC_INDEX           0x3c4
#define MAP_MASK           2

FILE *fp;
PALETTE palette;
int	colours;
char	wait_flag = 1, clr_scr = 1, rev = 0;
int	num_rows = 400;

void			cdecl (*display_pixel)(unsigned int x, unsigned int y, unsigned char colour);
void			cdecl (*setvispage)(int page);
char far *	cdecl (*setactpage)(int page);
void			cdecl (*end_graphics)(void);

void display_320(char format)
{
int plane, mask;
char far *base;

	if (rev) {
		setvispage(1);			/* Reverse 3D image */
		base = setactpage(1);
	} else base = setactpage(0);

	if (clr_scr)	mask = 0xf00;
	else				mask = 0x100;
	for (plane = 0; plane < 4; plane++, mask <<= 1) {
		outport(SC_INDEX, (mask & 0xf00) | MAP_MASK);
		fread(base, num_rows * (320/4), 1, fp);
	}
	if (format == '3') {					/* 3D image */
		if (rev) {
			setvispage(0);			/* Reverse */
			base = setactpage(0);
		} else {
			setvispage(1);				/* Normal */
			base = setactpage(1);
		}
		for (plane = 0; plane < 4; plane++) {
			outport(SC_INDEX, (0x100 << plane) | MAP_MASK);
			fread(base, 400 * (320/4), 1, fp);
		}
	}
	fclose(fp);
	if (format == '3') {					/* 3D image */
		while (!kbhit()) {
			WaitForVerticalRetrace();
			setvispage(0);
			WaitForVerticalRetrace();
			setvispage(1);
		}
	}
	if (wait_flag) getch();
}

void goto_400(void)
{
	set320x400mode();
	display_pixel 	= putpixel_400;
	setvispage		= setvis_400;
	setactpage		= setact_400;
	end_graphics	= end320x400mode;
	num_rows = 400;
}

void goto_200(void)
{
	set320x200x4mode();
	display_pixel 	= putpixel_200;
	setvispage		= setvis_200;
	setactpage		= setact_200;
	end_graphics	= end320x200mode;
	num_rows = 200;
}

void draw_view(char *fname)
{
int maxcol, maxrow;
char format;

	fp = fopen(fname,"rb");
	if (fp == NULL) {
		end_graphics();
		printf( "Error opening %s.\n", fname);
		exit(1);
	}
	format = getc(fp);
	if ((format < '2') || (format > '3') || getc(fp) != 'D') {		/* Check signature for 320x400 format */
		end_graphics();
		printf( "Incorrect signature on file.\n");
		exit(1);
	}

	maxcol = getw(fp); 		/* store columns */
	maxrow = getw(fp);		/* store rows */
	if (maxcol != 320 || (maxrow != 400 && maxrow != 200)) {
		end_graphics();
		printf( "Invalid image size -- must be 320*200 or 320*400, but is %d*%d.\n",
					maxcol, maxrow);
		exit(1);
	}
	if (maxrow != num_rows) {
		end_graphics();
		if (maxrow == 400) 	goto_400();
		else						goto_200();
	}

	colours = getc(fp);
	if (colours == 0) colours = 256;
	fread(palette, 3, colours, fp);
	setvgapalette(&palette, colours);
	display_320(format);
}

void cdecl main(int argc, char *argv[])
{
int par;

	if (argc < 2) {
		printf("Filename(s) must be specified\n");
		exit(1);
	}

	goto_400();
	for (par = 1; par < argc; par++)	{
		if (argv[par][0] == '/') {
			switch(toupper(argv[par][1])) {
			case 'R': rev 			= !rev;			break;
			case 'W': wait_flag 	= !wait_flag;	break;
			case 'C': clr_scr		= !clr_scr;		break;
			case 'H':
				if (num_rows == 400)		goto_200();
				else 							goto_400();
				break;
			}
		} else draw_view(argv[par]);
	}
	end_graphics();
}

