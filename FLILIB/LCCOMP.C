/* fii_lccomp.c - Some C code that mixes with the assembler code in
   comp.asm and skip.asm to make up compressed pixel packets suitable
   for incorporation into a FLI file.  See also writefli.c */

#ifdef __TURBOC__
#include <mem.h>
#endif

#include "aatypes.h"
#include "aaerr.h"
#include "aascreen.h"
#include "str_low.h"
#include "aafli.h"
#include "aaflisav.h"
#include "aafii.h"

#define INERTIA 4

static Pixel *sbrc_line(Pixel *s1, Pixel *s2, Pixel *cbuf, int count)
{
	register int wcount;
	register Pixel *c;
	int op_count;
	int next_match;
	int bcount;

	op_count = 0;
	c = cbuf+1;
	while(count > 0) {
		/* first find out how many bytes to skip... */
		wcount = bcompare((char *) s1, (char *) s2, count);
		if ((count -= wcount) <= 0) break;	/* same until the end... */
		/* if skip is longer than 255 have to break it up into smaller ops */
		while (wcount > 255) {
			s1 += 255+1;
			s2 += 255;
			wcount -= 255+1;
			/* make dummy copy 1 op */
			*c++ = 255;
			*c++ = 1;
			*c++ = *s2++;
			op_count++;
		}
		/* save initial skip and move screen pointer to 1st different byte */
		*c++ = wcount;
		s1 += wcount;
		s2 += wcount;
		op_count++;

		/* if have skipped to near the end do a literal copy... */
		if (count <= INERTIA) {
			*c++ = count;
			memcpy(c, s2, count);
			c += count;
			break;
		}

		/* now look for a run of same... */
		bcount = count;
		if (bcount > FLI_MAX_RUN)		bcount = FLI_MAX_RUN;

		wcount = bsame((char *) s2, bcount);
		if (wcount >= INERTIA) {	/* it's worth doing a same thing thing */
			next_match = fii_tnskip(s1, s2, wcount,INERTIA);

			if (next_match < wcount) /* if it's in our space and a decent size */
											 /* we'll cut short same run for the skip */
				wcount = next_match;
			*c++ = -wcount;
			*c++ = *s2;
			s1 += wcount;
			s2 += wcount;
			count -= wcount;
		} else {	/* doing a literal copy.  What can we do to make it short? */
			/* figure out how long until the next worthwhile "skip" */
			/* Have wcount of stuff we can't skip through. */
			wcount = fii_tnsame(s2, fii_tnskip(s1,s2,bcount,INERTIA-1),INERTIA);
			/* Say copy positive count as lit copy op, and put bytes to copy
				into the compression buffer */
			*c++ = wcount;
			memcpy(c, s2, wcount);
			s1 += wcount;
			s2 += wcount;
			c += wcount;
			count -= wcount;
		}
	}
	*cbuf = op_count;
	return c;
}

unsigned fii_lccomp(Pixel *s1, Pixel *s2, Cbuf *cbuf, int width, int height)
{
	USHORT skip_count, j;
	Pixel *c;
	Pixel *oc;
	USHORT acc;
	long total;
	USHORT last_real;

/* find out how many lines of s1 and s2 are the same */
	acc = width >> 1;	/* SHORTS in line */
	j = height;
	skip_count = 0;
	total = 0;
	while (--j >= 0)	{
		if (wcompare((USHORT *)s1, (USHORT *)s2, acc) != acc)		break;
		s1 += width;
		s2 += width;
		skip_count++;
	}

/* If all same do special case for empty frame*/
	if (skip_count == height)		return 2;

/* store offset of 1st real line and set up for main line-at-a-time loop */
#ifdef __TURBOC__
	*((USHORT *)cbuf)++ = skip_count;
#else
	cbuf = wbuf(cbuf, skip_count);
#endif
	height -= skip_count;
	c = cbuf + 2;
	last_real = 0;	/* keep track of last moving line */
	for (j=1; j<=height; j++) {
		oc = c;
		if (wcompare((USHORT *)s1, (USHORT *)s2, acc) == acc)	/* whole line is the same */
			*c++ = 0;								/* set op count to 0 */
		else	{					/* compress line */
			c = sbrc_line(s1, s2, c, width);
			last_real = j;
		}
		total += (long)(c - oc);
		if (total >= 60000L)		return 0;
		s1 += width;
		s2 += width;
	}
/* set # of lines in compression to last real, removing empty bottom lines
   from buffer */
#ifdef __TURBOC__
	*(USHORT *)cbuf = last_real;
#else
	wbuf(cbuf, last_real);
#endif
	c -= height-last_real;
	return (unsigned)(c - cbuf) + 2;
}
