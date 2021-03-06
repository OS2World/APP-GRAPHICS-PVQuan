/************************************************************************
 *                                                                      *
 *                  Copyright (c) 1992, Frank van der Hulst             *
 *                          All Rights Reserved                         *
 *                                                                      *
 ************************************************************************
 *                                                                      *
 * Authors:                                                             *
 *        FvdH - Frank van der Hulst (Wellington, NZ)                   *
 *                                                                      *
 * Versions:                                                            *
 *    V1.5 920401 FvdH - Released as part of PVQUAN15.ZIP               *
 *    V1.6 921101 FvdH - Ported to OS/2                                 *
 *                                                                      *
 ************************************************************************/

/*
  This program uses FLI.LIB to create FLIC (Autodesk Animator) files from a
  series of 2D frames. */

#include <stdlib.h>
#include <stdio.h>
#ifdef __TURBOC__
#include <dos.h>
#include <mem.h>
#include <io.h>
#include <conio.h>

#endif
#ifdef __GNUC__
#define cdecl

int getw(FILE *fp)
{
   int x;

   x = getc(fp);
   return (getc(fp) << 8) | x;
}
#endif

#include "aatypes.h"
#include "aascreen.h"
#include "aaerr.h"
#include "aafli.h"
#include "aaflisav.h"

UBYTE palette[256][3];

void err_exit(char *msg)
{
   printf("%s\n", msg);
   exit(1);
}

/* Read one image from a 2D file, and load it into the buffers whose addresses
   have been passed to it. Returns TRUE if successful, FALSE if not */

int read_image(char *fname, unsigned char *buff, unsigned char *palette)
{
   FILE *fp;
   int maxcol, maxrow, colours;

   printf("Reading image %s\n", fname);
   if ((fp = fopen(fname, "rb")) == NULL) return FALSE;

   if (getc(fp) != '2') err_exit("\nIncorrect signature on file.\n");
   if (getc(fp) != 'D') err_exit("\nIncorrect signature on file.\n");

   maxcol = getw(fp);     /* store columns */
   maxrow = getw(fp);     /* store rows */
   if (maxcol != 320 || maxrow != 200) err_exit("\nInvalid image size -- must be 320*200.\n");
   colours = getc(fp);
   if (colours == 0) colours = 256;
   fread(palette, 3, colours, fp);
   fread(buff, 1, maxrow * maxcol, fp);
   fclose(fp);
   return TRUE;
}

/* first_scr contains a copy of the first screen in the sequence, so that
   the last frame->first frame wrap can be maintained. This is a little
   wasteful of memory, but avoids having to decompress the first frame again
   at the end. That saves having to port the decompression code too. */

void make_fli(char *in, char *out)
{
   Vscreen *curr_scr, *prev_scr, *first_scr;
   Fli_head outhead;
   FILE *outfile;
   int file_no;
   char in_fname[256];
   int file_ok;
   int result;

   if ((prev_scr  = aa_alloc_mem_screen()) == NULL) err_exit(fli_error_message(AA_ERR_NOMEM));
   if ((curr_scr  = aa_alloc_mem_screen()) == NULL) err_exit(fli_error_message(AA_ERR_NOMEM));
   if ((first_scr = aa_alloc_mem_screen()) == NULL) err_exit(fli_error_message(AA_ERR_NOMEM));
   if ((outfile = fli_create(out, &outhead, 5)) == NULL) err_exit("Couldn't create output file.");
   file_no = 0;
   sprintf(in_fname, "%s.0", in);
   file_ok = read_image(in_fname, curr_scr->pmap, curr_scr->cmap);
   aa_copy_screen(curr_scr, first_scr);
   if ((result = fli_write_next(outfile, &outhead, curr_scr, curr_scr)) < AA_SUCCESS) err_exit(fli_error_message(result));
   while (file_ok) {
      aa_copy_screen(curr_scr, prev_scr);
      file_no++;
      sprintf(in_fname, "%s.%d", in, file_no);
      if (!read_image(in_fname, curr_scr->pmap, curr_scr->cmap)) break;
      if ((result = fli_write_next(outfile, &outhead, curr_scr, prev_scr)) < AA_SUCCESS) err_exit(fli_error_message(result));
   }
   if ((result = fli_end(outfile, &outhead, prev_scr, first_scr)) < AA_SUCCESS) err_exit(fli_error_message(result));
   fclose(outfile);
   aa_free_mem_screen(prev_scr);
   aa_free_mem_screen(curr_scr);
   aa_free_mem_screen(first_scr);
}

void cdecl main(int argc, char *argv[])
{
   char out_fname[256];

   printf("ANIMFLI v1.6 -- Create a FLI file containing an animation sequence.\n");
   printf("By F van der Hulst. Copyright 1992\n\n");
   if (argc != 3) {
      printf("Usage: %s frame output\n", argv[0]);
      printf("       frame  is the name (without extension) of the input frame files\n");
      printf("       output is the name (without extension) of the FLI output file\n\n");
      exit(1);
   }

   sprintf(out_fname, "%s.fli", argv[2]);
   make_fli(argv[1], out_fname);
}
