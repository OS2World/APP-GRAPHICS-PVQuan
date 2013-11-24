#ifndef AAI86_H
#define AAI86_H

#ifdef __TURBOC__
int far cdecl bsame(char *d, int count);  /* how many bytes in a row the same ? */
int far cdecl bcompare(char *s1,char *s2,int count); /* how many bytes match */
int far cdecl wcompare(USHORT *s1,USHORT *s2,int count); /* how many words match */
int far cdecl bcontrast(char *s1, char *s2, int count); /* bytes til 1st match? */

/** Low level assembler routines used by fii_lccomp to do a single line **/
/* Find out how far until have the next match of mustmatch or more pixels */
int far cdecl fii_tnskip(Pixel *s1,Pixel *s2,int bcount,int mustmatch);
/* Find out how far until next run of identical pixels mustmatch long */
int far cdecl fii_tnsame(Pixel *s,int bcount,int mustmatch);

#else

/* Overcome Intel/Motorola byte-order problems */

UBYTE *wbuf(UBYTE *p, unsigned w);
UBYTE *lbuf(UBYTE *p, unsigned long l);

int wcompare(USHORT *s1, USHORT *s2,int count); /* how many words match */
int bcompare(char *s1,char *s2,int count); /* how many bytes match */
int bcontrast(char *s1, char *s2, int count); /* bytes til 1st match? */
int bsame(char *d, int count);  /* how many bytes in a row the same ? */

/** Low level assembler routines used by fii_lccomp to do a single line **/
/* Find out how far until have the next match of mustmatch or more pixels */
unsigned int fii_tnskip(Pixel *s1, Pixel *s2, unsigned int bcount, unsigned int mustmatch);
/* Find out how far until next run of identical pixels mustmatch long */
unsigned int fii_tnsame(Pixel *s, unsigned int bcount, unsigned int mustmatch);
	      
#endif
#endif /* AAI86_H */
