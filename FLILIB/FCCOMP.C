/* fii_fccomp.c -  code used to delta compress colors. */

#include "aatypes.h"
#include "aaerr.h"
#include "aascreen.h"
#include "str_low.h"
#include "aafli.h"
#include "aaflisav.h"
#include "aafii.h"

/* fii_fccomp - compress an rgb triples color map just doing 'skip' compression */
unsigned fii_fccomp(Cmap *s1, Cmap *s2, Cbuf *cbuf, int count)
{
	USHORT wcount;
	Cbuf *c;
	USHORT op_count;
	USHORT dif_count;
	USHORT bcount;
	USHORT c3;

	c = cbuf + 2;
	op_count = 0;
	count *= 3;
	wcount = wcompare((USHORT *)s1, (USHORT *)s2, count>>1);
	wcount <<= 1;
	if (wcount == count)		return 2;	/* stupid way to say got nothing... */
	while (TRUE)	{
		/* first find out how many words to skip... */
		c3 = bcompare((char *) s1, (char *) s2, count)/3;
		wcount = c3*3;
		if ((count -= wcount) == 0)	break;	/* same until the end... */
		*c++ = c3;
		s1 += wcount;
		s2 += wcount;
		op_count++;

		/* figure out how long until the next worthwhile "skip" */
		dif_count = 0;
		bcount = count;
		while (TRUE) {
			wcount = bcontrast((char *) s1, (char *) s2, bcount)/3;
			dif_count += wcount;
			wcount *= 3;
			s1 += wcount;
			s2 += wcount;
			bcount -= wcount;
			if (bcount >= 3) {
				if ((wcount = bcompare((char *) s1, (char *) s2, 3)) == 3)	break;
				else {
					dif_count += 1;
					s1 += 3;
					s2 += 3;
					bcount -= 3;
				}
			} else break;
		}
		*c++ = dif_count;
		dif_count *= 3;
		s2 -= dif_count;
		count -= dif_count;
		while (dif_count != 0) {
			dif_count--;
			*c++ = *s2++;
		}
		if (count <= 0)	break;
	}
#ifdef __TURBOC__
	*(USHORT *)cbuf = op_count;
#else
	wbuf(cbuf, op_count);
#endif
	return (unsigned) (c - cbuf);
}

