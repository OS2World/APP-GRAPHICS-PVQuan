/* str_low.c -- String low-level operations */

#include "aatypes.h"
#include "aascreen.h"
#include "str_low.h"

int wcompare(USHORT *s1, USHORT *s2, int count)
{
	unsigned i;

	for (i = 0; *s1++ == *s2++ && i < count; i++) ;
	return i;
}

int bcompare(char *s1, char *s2, int count)
{
	unsigned i;

	for (i = 0; *s1++ == *s2++ && i < count; i++) ;
	return i;
}

/* return how many bytes of s1 and s2 are different */
int bcontrast(char *s1, char *s2, int count)
{
	unsigned i;

	for (i = 0; *s1++ != *s2++ && i < count; i++) ;
	return i;
}

/* find out how many bytes in a row are the same value */
int bsame(char *s1, int count)
{
	char ch;
	unsigned i;

	if (count == 0) return 0;
	ch = *s1++;
	for (i = 1; *s1++ == ch && i < count; i++) ;
	return i;
}

/* Find out how far until have the next match of mustmatch or more pixels */

unsigned int fii_tnskip(Pixel *s1, Pixel *s2, unsigned bcount, unsigned mustmatch)
{
	unsigned int difcount = 0;
	unsigned int ax;

	while (1) {
		ax = bcontrast((char *)s1, (char *)s2, bcount); /* calculate number of pixels different in s1 and s2 into ax */
		s1 += ax;
		s2 += ax;				/* move source pointers just past this different run */
		difcount += ax;		/* add different count to return value */
		bcount -= ax;
		if (bcount < mustmatch) {	/* see if near the end... */
			if (bcompare((char *) s1, (char *) s2, bcount) != bcount)	/* check last couple of pixels */
				difcount += bcount;							/* if all of them match between s1 and s2 go home */
			break;
		}
		if ((ax = bcompare((char *) s1, (char *) s2, mustmatch)) == mustmatch)	/* see if enough in a row match to break out of this */
			break;		/* if all of them match between s1 and s2 go home */

		difcount += ax;			/* Add ones that do match into difcount */
		bcount -= ax;				/* sub it from pixels left to examine */
		s1 += ax;
		s2 += ax;					/* update s1,s2 pointers */
	}
	return	difcount;
}

/* Find out how far until next run of identical pixels mustmatch long */

unsigned int fii_tnsame(Pixel *s, unsigned int wcount, unsigned int mustmatch)
{
	unsigned bx = wcount;			/* Number of pixels left */
	unsigned difcount = wcount;
	unsigned si = 0;					/* si is # of pixels examined */
	unsigned same_count;

	while (bx > mustmatch) {		/* break out of loop if less than 4 pixels left to examine */
		same_count = bsame((char *) s, wcount);
		s += same_count - 1;
		if (same_count >= mustmatch)	return si;			/* if mustmatch or more, go truncate dif_count */
		si += same_count;
		s += same_count;  /* This may be a bug in the assembler program */
		bx -= same_count;
	}
	return difcount;
}

UBYTE *wbuf(UBYTE *p, unsigned x)
{
	*p++ = x         & 0xff;
	*p++ = (x >>  8) & 0xff;
	return p;
}

UBYTE *lbuf(UBYTE *p, unsigned long x)
{
	p = wbuf(p, (unsigned)(x & 0xffffL));
	p = wbuf(p, (unsigned)((x >> 16) & 0xffffL));
	return p;
}
