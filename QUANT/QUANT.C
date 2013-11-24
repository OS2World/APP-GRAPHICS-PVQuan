/************************************************************************
 *                                                                      *
 *                  Copyright (c) 1991, Frank van der Hulst             *
 *                          All Rights Reserved                         *
 *                                                                      *
 * Authors:                                                             *
 *        FvdH - Frank van der Hulst (Wellington, NZ)                   *
 *                                                                      *
 * Versions:                                                            *
 *    V1.1 910626 FvdH - QUANT released for DBW_RENDER                  *
 *    V1.2 911021 FvdH - QUANT released for PoV Ray                     *
 *    V1.3 911031 FvdH - Added 320x200x256x4 pages support              *
 *    V1.4 920303 FvdH - Ported to GNU                                  *
 *                     - Changed usage() to fix bug                     *
 *    V1.5 920331 FvdH - Read Targa file format                         *
 *         920403 FvdH - Allow any number of files                      *
 *    V1.6 921023 FvdH - Produce multi-image GIFs                       *
 *                     - Port to OS/2 IBM C Set/2                       *
 *                                                                      *
 ************************************************************************/
/*
 * quant.c
 *
 * Program to do colour quantization. This program reads PoV Ray raw
 * format files and quantizes the image(s) to produce a file in one of
 * 3 formats -- 2D, 3D or GIF. It can use either Heckbert's median-splitting
 * algorithm, or else the Octree Quantisation algorithm by Michael Gervautz
 * and Werner Purgathofer.
 *
 * This program compiles using Turbo-C v2.01 and runs on an MS-DOS machine,
 * using cc on a SCO Unix system, gcc on a Sun system, IBM's icc on OS/2.
 */

#include <string.h>
#ifdef __TURBOC__
#include <ctype.h>
#endif

#include "quant.h"

#ifdef HECKBERT
#include "heckbert.h"
#else
#include "octree.h"
#endif

unsigned long HUGE_PTR Histogram;		/* image histogram */
unsigned char HUGE_PTR RGBmap;			/* RGB -> index map */

unsigned int	ColormaxI;				/* # of colors, 2^input_bits */
UCHAR palette[MAXCOLORS][3];

#ifdef __TURBOC__
char				disp_image	= 1;		/* Display image while quantising */
#else
char				disp_image	= 0;		/* Display image while quantising */
#endif
char 				input_type  = 1;		/* Input format switch */

static int output_bits		= 6;		/* No. of sig. bits on output (must be <= 8) */
#ifdef HECKBERT
static int fast_quant		= 0;		/* Quantisation speed switch */
/*	If zero, the rgbmap will be built very slowly, but more accurately. The
	error introduced by the approximation is usually small.

	If one, the rgbmap will be constructed quickly. */
#endif

static int image_width		= 320;	/* Width of image in pixels */
static int image_height		= 200;	/* Height of image in pixels */
static int output_colours 	= 256;	/* No. of separate colours to produce ( <= 256) */
static int output_type     = 1;     /* Output format switch */

char *input_file;
int num_files = 1;

void err_exit(void)
{
#ifdef __OS2__
        getchar();
#endif
        exit(1);
}

void usage(char *prog_name)
{
	printf("Command syntax: %s [-O=outputbits][-C=colours][-S=speed][-T=outtype]\n", prog_name);
	printf("                   [-D=display][-W=width][-H=height][-I=intype][-N=numfiles] filename\n\n");
	printf("outputbits = bits per colour being output          [6]\n");
#ifdef HECKBERT
	printf("speed      = 0 or 1                                [%d]\n", fast_quant);
#endif
	printf("outtype    = 0 for 4-plane, 1 for planar,2 for GIF [%d]\n", output_type);
	printf("intype     = 0 for raw, 1 for Targa                [%d]\n", input_type);
#ifdef __TURBOC__
	printf("display    = 1 or 0 for displaying during output   [%d]\n", disp_image);
#endif
	printf("colours    = number of separate colours to produce [%d]\n", output_colours);
	printf("width      = width of image in pixels              [%d]\n", image_width);
	printf("height     = height of image in pixels             [%d]\n", image_height);
	printf("numfiles   = number of files to quantise           [%d]\n", num_files);
	printf("filename   = name part of file(s) to read, without extension\n");
	err_exit();
}

/********************************************************************
 Process command line arguments. */

void get_args(int argc, char *argv[])
{
	int i;

	if (argc == 1) usage(argv[0]);

	for (i = 1; i < argc; i++) {
		if (argv[i][0] != '/' && argv[i][0] != '-') {
			input_file = argv[i];
			continue;
		}

		if (argv[i][2] != '=') {
			printf("Invalid command line switch: %s\n", argv[i]);
			usage(argv[0]);
		}

		switch(toupper(argv[i][1])) {
		case 'O': output_bits    	= atoi(&argv[i][3]); break;
#ifdef HECKBERT
		case 'S': fast_quant	    	= atoi(&argv[i][3]); break;
#endif
		case 'T': output_type		= atoi(&argv[i][3]); break;
		case 'I': input_type			= atoi(&argv[i][3]); break;
#ifdef __TURBOC__
		case 'D': disp_image	   	= atoi(&argv[i][3]); break;
#endif
		case 'C': output_colours 	= atoi(&argv[i][3]); break;
		case 'W': image_width    	= atoi(&argv[i][3]); break;
		case 'H': image_height     = atoi(&argv[i][3]); break;
		case 'N': num_files        = atoi(&argv[i][3]);	break;
		default:
			printf("Invalid command line switch: %s\n", argv[i]);
			usage(argv[0]);
		}
	}
}

void main(int argc, char *argv[])
{
	int colors, i;
#ifdef HECKBERT
	long int li;
#endif

	printf("QUANT v1.60 -- Colour quantisation for PVRay\n");
#ifdef HECKBERT
	printf("By F van der Hulst, based on COLORQUANT by Craig E. Kolb.\n\n");
#else
	printf("By F van der Hulst, based on Octree Quantisation by Wolfgang Stuerzlinger.\n\n");
#endif

	get_args(argc, argv);
        if ((output_type == 0) &&
		 ((image_width != 320) ||
		  ((image_height != 400) && (image_height != 200)))) {
		printf("Only 320x400 and 320x200 images can be written in 4-plane format.\n");
		err_exit();
	}

#ifdef HECKBERT
	CHECK_ALLOC(RGBmap, unsigned char, BYTE_COUNT, "RGB map");

	CHECK_ALLOC(Histogram, unsigned long, BYTE_COUNT, "Histogram");
	for (li = 0; li < BYTE_COUNT; li++)		Histogram[li] = 0L;
	open_box_file(output_colours);

/* The projected frequency arrays of the largest box are zeroed out as
	as part of open_box_file(). */

#endif

	for (i = 0; i < num_files; i++) {
		open_file(input_file, i);

#ifdef HECKBERT
		printf("Building Histogram from %s_%d: ...", input_file, i);
		QuantHistogram(get_box(0));
		printf("\b\b\bDone\n");
#else
		printf("Building Octree from %s_%d: ...", input_file, i);
		generateoctree();			/* read the file through */

		printf("\b\b\bDone\n");
#endif
		close_file();
	}

	for (i = 0; i < MAXCOLORS; i++)          /* init palette */

		palette[i][0] = palette[i][1] = palette[i][2] = 0;	/* 0 usually is black ! */

#ifdef HECKBERT
	colors = colorquant(output_colours, INPUT_BITS, fast_quant,
						(double) ((1 << output_bits) - 1) / ((1 << INPUT_BITS) - 1));

	close_box_file();
#ifdef __TURBOC__
	free((void far *)Histogram);
#else
	free(Histogram);
#endif
#else
	colors = calc_palette(1, (double) ((1 << output_bits) - 1) / 0xff);
						/* entry 0 is left black here ! */

#endif
	printf("%d %s quantized to %d colors.\n", num_files, num_files == 1 ? "file" : "files", colors);
	write_file(num_files, input_file, image_width, image_height, colors, output_type);
#ifdef HECKBERT
#ifdef __TURBOC__
	free((void far *)RGBmap);
#else
	free(RGBmap);
#endif
#endif
}
