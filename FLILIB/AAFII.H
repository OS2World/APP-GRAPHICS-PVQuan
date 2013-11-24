
#ifndef AAFII_H
#define AAFII_H

/* Low level decompression functions */
void far cdecl fii_unbrun(Cbuf *cbuf, Pixel *screen, int linect); /* 1st frame */
void far cdecl fii_unlccomp(Cbuf *cbuf, Pixel *screen);           /* delta frames */
void far cdecl fii_mem_fcuncomp(Cbuf *cbuf, Cmap *cmap);          /* colors to memory */
void far cdecl fii_reg_fcuncomp(Cbuf *cbuf);                                      /* colors to registers */

/* Longest run for byte-oriented compression */
#define FLI_MAX_RUN 127

/** Mid level routines to compress 1st frame, delta frames, and color maps **/
/* Run length compress a single frame using Animator 8086
	1st frame technique */
unsigned fii_brun(Pixel *s1, Cbuf *cbuf, int width, int height);
/* Delta compress a single frame using Animator 8086 byte-oriented scheme */
unsigned fii_lccomp(Pixel *s1,Pixel *s2, Cbuf *cbuf,int width,int height);
/* fii_fccomp - compress an rgb triples color map doing 'skip' compression */
unsigned fii_fccomp(Cmap *s1, Cmap *s2, Cbuf *cbuf, int count);

#endif /* AAFII_H */
