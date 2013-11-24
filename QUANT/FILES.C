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
 *    V1.3 911030 FvdH - Added 320x200x256x4 pages support              *
 *                     - Fixed bug in output_anim_files                 *
 *    V1.4 920303 FvdH - Ported to GNU C                                *
 *    V1.5 920331 FvdH - Allow Targa input                              *
 *         920403 FvdH - Allow any number of files                      *
 *    V1.6 921023 FvdH - Produce multi-image GIFs                       *
 *                     - Port to OS/2 IBM C Set/2                       *
 *                                                                      *
 ************************************************************************/

#include <string.h>
#ifdef __TURBOC__
#include <dos.h>

#define SC_INDEX           0x3c4
#define MAP_MASK           2
#endif

#ifdef __GNUC__
#define SEEK_SET 0
#endif

#include "gif_lib.h"
#include "quant.h"

int (*get_pixel)(UCHAR *pixel);

static FILE *out_file;
static FILE *in_file[3];

static int get_pixel_raw(UCHAR *pixel)
{
	int t;

#if INPUT_BITS == 8
	if ((t = getc(in_file[2])) == EOF) return FALSE;
	pixel[BLUE] = t;
	if ((t = getc(in_file[1])) == EOF) return FALSE;
	pixel[GREEN] = t;
	if ((t = getc(in_file[0])) == EOF) return FALSE;
	pixel[RED] = t;
#else
#define MAX_IN  ((0xff << (8 - INPUT_BITS)) & 0xff)
#define ROUND   (1 << (7 - INPUT_BITS))
#define R_SHIFT (8 - INPUT_BITS)
	if ((t = getc(in_file[2])) == EOF) return FALSE;
	pixel[BLUE]  = t < MAX_IN ? (t + ROUND) >> R_SHIFT : t >> R_SHIFT;
	if ((t = getc(in_file[1])) == EOF) return FALSE;
	pixel[GREEN] = t < MAX_IN ? (t + ROUND) >> R_SHIFT : t >> R_SHIFT;
	if ((t = getc(in_file[0])) == EOF) return FALSE;
	pixel[RED]   = t < MAX_IN ? (t + ROUND) >> R_SHIFT : t >> R_SHIFT;
#undef MAX_IN
#undef ROUND
#undef R_SHIFT
#endif
	return TRUE;
}


static int get_pixel_targa(UCHAR *pixel)
{
	int t;

#if INPUT_BITS == 8
	if ((t = getc(in_file[0])) == EOF) return FALSE;
	pixel[BLUE] = t;
	if ((t = getc(in_file[0])) == EOF) return FALSE;
	pixel[GREEN] = t;
	if ((t = getc(in_file[0])) == EOF) return FALSE;
	pixel[RED] = t;
#else
#define MAX_IN  ((0xff << (8 - INPUT_BITS)) & 0xff)
#define ROUND   (1 << (7 - INPUT_BITS))
#define R_SHIFT (8 - INPUT_BITS)
	if ((t = getc(in_file[0])) == EOF) return FALSE;
	pixel[BLUE]  = t < MAX_IN ? (t + ROUND) >> R_SHIFT : t >> R_SHIFT;
	if ((t = getc(in_file[0])) == EOF) return FALSE;
	pixel[GREEN] = t < MAX_IN ? (t + ROUND) >> R_SHIFT : t >> R_SHIFT;
	if ((t = getc(in_file[0])) == EOF) return FALSE;
	pixel[RED]   = t < MAX_IN ? (t + ROUND) >> R_SHIFT : t >> R_SHIFT;
#undef MAX_IN
#undef ROUND
#undef R_SHIFT
#endif
	return TRUE;
}


/* The "+ft" option of PVRAY writes out Targa format.  Specifically, the
	fields are:

Header:
   00 00 02 00 00      - Fixed header information for uncompressed type 2 image
   00 00 00
   0000                - Horizontal offset always is at 0000
	llll                - Vertical offset (1st line number, 16 bits, LSB first)
	wwww hhhh           - width, height of image (16 bits each, LSB first)
   18 20               - 24 bits per pixel, Top-down raster

For each line:
	bb gg rr bb gg rr ... - blue, green, and red data, 8 bits for each pixel.
*/

void open_file(char *fname, int num)
{
char filename[256];

	if (input_type == 0) {
		sprintf(filename, "%s_%d.red", fname, num);
		if ((in_file[0] = fopen(filename, "rb")) == NULL) {
			printf("Cannot open %s.\n", filename);
			err_exit();
		}
		sprintf(filename, "%s_%d.grn", fname, num);
		if ((in_file[1] = fopen(filename, "rb")) == NULL) {
			printf("Cannot open %s.\n", filename);
			err_exit();
		}
		sprintf(filename, "%s_%d.blu", fname, num);
		if ((in_file[2] = fopen(filename, "rb")) == NULL) {
			printf("Cannot open %s.\n", filename);
			err_exit();
		}
		get_pixel = get_pixel_raw;
		return;
	}
	if (input_type == 1) {
		sprintf(filename, "%s_%d.tga", fname, num);
		if ((in_file[0] = fopen(filename, "rb")) == NULL) {
			printf("Cannot open %s.\n", filename);
			err_exit();
		}
		fseek(in_file[0], 18, SEEK_SET);		/* Skip header */
		get_pixel = get_pixel_targa;
		return;
	}
}

void close_file(void)
{
	fclose(in_file[0]);
	if (input_type == 0) {
		fclose(in_file[1]);
		fclose(in_file[2]);
	}
}

void write_4_planes(char *infname, int num, UINT Xres, UINT Yres)
{
	UINT plane, x, y;
	UCHAR pixel[3];
	char more_data;
	char dummy[3];
#ifdef __TURBOC__
	char far *VGA_addr = MK_FP(0xa000,0);
#endif

	open_file(infname, num);
	for (plane = 0; plane < 4; plane++) {
		more_data = 1;
		fseek(in_file[0], (long)plane, SEEK_SET);
		fseek(in_file[1], (long)plane, SEEK_SET);
		fseek(in_file[2], (long)plane, SEEK_SET);
#ifdef __TURBOC__
		if (disp_image) 	outport(SC_INDEX, (0x100 << plane) | MAP_MASK);
#endif
		for (y = 0; y < Yres; y++) {
			for (x = plane; x < Xres; x += 4) {
				if (more_data) {
					if (!get_pixel(pixel))
						pixel[RED] = pixel[GREEN] = pixel[BLUE] = more_data = 0;
					else {
						fread(dummy, 3, 1, in_file[0]);   /* Skip to next pixel for this plane */
						fread(dummy, 3, 1, in_file[1]);
						fread(dummy, 3, 1, in_file[2]);
					}
					putc(pal_index(pixel), out_file);
#ifdef __TURBOC__
					if (disp_image)		*VGA_addr++ = pal_index(pixel);
#endif
				} else	putc(pal_index(pixel), out_file);
			}
		}
	}
	close_file();
}

static void write_linear(char *infname, int num, UINT Xres, UINT Yres)
{
	UINT x, y;
	UCHAR pixel[3];
	char more_data;
#ifdef __TURBOC__
	char far *VGA_addr = MK_FP(0xa000,0);
#endif

	open_file(infname, num);
	more_data = 1;
	for (y = 0; y < Yres; y++) {
		for (x = 0; x < Xres; x++) {
			if (more_data) {
				if (!get_pixel(pixel))
					pixel[RED] = pixel[GREEN] = pixel[BLUE] = more_data = 0;
			}
			putc(pal_index(pixel), out_file);
#ifdef __TURBOC__
			if (disp_image) {
				outport(SC_INDEX, (0x100 << (x & 3)) | MAP_MASK);
				VGA_addr[y*(320/4)+x/4] = pal_index(pixel);
			}
#endif
		}
	}
	close_file();
}

/****************************************************************************
	Convert Raw image (One byte per pixel) into Gif file. Raw data is read
	from in_file, and Gif is dumped to out_fname. ImageWidth times ImageHeight
	bytes are read. Color map is dumped from ColorMap.
*/

static void output_gif_file(char *infname, int num_files, char *out_fname, UINT Xres, UINT Yres, UINT num_colours)
{
   int i, x, y;
   UCHAR pixel[3];
   UCHAR *ScanLine;

   for (i = 0; i < 8 && (2 << i) < num_colours; i++);
   num_colours = 2 << i;
   if (num_colours > 256) {
      printf("Colour map must be less than 256 colours.\n");
      err_exit();
   }

   printf("\nImage size is %dx%d:     ", Xres, Yres);

   if (EGifOpenFileName(out_fname) == -1)    return;
   EGifPutScreenDesc(Xres, Yres, 6, 0, 8, palette);

   for (i = 0; i < num_files ; i++) {
      open_file(infname, i);
      EGifPutImageDesc(0, 0, Xres, Yres, 8);

      CHECK_ALLOC(ScanLine, UCHAR, Xres, "Scan Line");

      /* Here it is - get one raw line from input file, and dump to Gif: */
      for (y = 0; y < Yres; y++) {
      /* Note we assume here PixelSize == Byte, which is not necessarily   */
      /* so. If not - must read one byte at a time, and coerce to pixel.   */
         for (x = 0; x < Xres; x++) {
            if (!get_pixel(pixel)) {
               printf("RAW input file ended prematurely.\n");
               err_exit();
            }
            ScanLine[x] = pal_index(pixel);
         }
 
         if (EGifPutLine(ScanLine, Xres)) break;
         printf("\b\b\b\b%-4d", y);
      }
   } /* endfor */

   EGifCloseFile();
   free(ScanLine);
   close_file();
}

void output_xd_file(char *infname, int num, char *out_fname, int colors,
                    int num_files, int Xres, int Yres, int type)
{
int i;

	if ((out_file = fopen(out_fname, "wb")) == NULL) {
		printf("Couldn't open %s.\n", out_fname);
		err_exit();
	}
	putc(num_files + '1', out_file);
	putc('D', out_file);
	putw(Xres, out_file);
	putw(Yres, out_file);
	putc(colors, out_file);
	fwrite(palette, 3, colors, out_file);

#ifdef __TURBOC__
	if ((Xres != 320) || ((Yres != 200) && (Yres != 400))) disp_image = FALSE;
	if (disp_image) {
		if (Yres == 200) 			set320x200x4mode();
		else if (Yres == 400) 	set320x400mode();
		setvgapalette(&palette, colors);
	}
#endif
	for (i = 0; i < num_files; i++)
		if (type == 0) 	write_4_planes(infname, num + i, Xres, Yres);
		else              write_linear(infname, num + i, Xres, Yres);
	fclose(out_file);
#ifdef __TURBOC__
	if (disp_image)
		if (Yres == 200)		end320x200mode();
		else						end320x400mode();
#endif
}

void output_anim_files(char *infname, char *out_fname,
                       int colors, int num_files, UINT Xres, UINT Yres, int type)
{
	int i;
	char outname[256];

	for (i = 0; i < num_files; i++) {
		sprintf(outname, "%s.%d", out_fname, i);
		printf("Outputting to %s\n", outname);
		output_xd_file(infname, i, outname, colors, 1, Xres, Yres, type);
	}
}

void write_file(int num_files, char *input_file, int Xres, int Yres, 
                int colors, int output_type)
{
char outfilename[256];
   if (num_files > 2)
      strcpy(outfilename, input_file);
   else if (output_type == 2)
      sprintf(outfilename, "%s.gif", input_file);
   else
      sprintf(outfilename, "%s.%cd", input_file, num_files + '1');

   printf("Outputting to %s\n", outfilename);

   if (output_type == 2)
      output_gif_file(input_file, num_files, outfilename, Xres, Yres, colors);
   else if (num_files < 3) {
      output_xd_file(input_file, 0, outfilename, colors, num_files, Xres, Yres, output_type);
   } else output_anim_files(input_file, outfilename, colors, num_files, Xres, Yres, output_type);
}
