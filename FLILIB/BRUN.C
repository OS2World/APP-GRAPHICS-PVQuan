
#include "aatypes.h"
#include "aascreen.h"
#include "str_low.h"
#include "aaerr.h"
#include "aafli.h"
#include "aafii.h"
#include "aaflisav.h"

static UBYTE *fii_brun_comp_line(Pixel *s1, Pixel *cbuf, int count)
{
int wcount;
register UBYTE *c;
register int bcount;
int op_count;
Pixel *start_dif;
int dif_count;

	c = cbuf+1;
	op_count = 0;
	start_dif = s1;
	dif_count = 0;
	for (;;) {
		if (count < 3) {
			dif_count += count;
			while (dif_count > 0) {
				bcount = (dif_count < FLI_MAX_RUN ? dif_count : FLI_MAX_RUN );
				*c++ = -bcount;
				dif_count -= bcount;
				while (--bcount >= 0)		*c++ = *start_dif++;
				op_count++;
			}
			*cbuf = op_count;
			return c;
		}
		bcount = (count < FLI_MAX_RUN ? count : FLI_MAX_RUN );
		if ((wcount = bsame((char *)s1, bcount)) >= 3) {
			while (dif_count > 0) {
				bcount = (dif_count < FLI_MAX_RUN ? dif_count : FLI_MAX_RUN );
				*c++ = -bcount;
				dif_count -= bcount;
				while (--bcount >= 0)	*c++ = *start_dif++;
				op_count++;
			}
			*c++ = wcount;
			*c++ = *s1;
			op_count++;
			s1 += wcount;
			count -= wcount;
			start_dif = s1;
		} else {
			dif_count++;
			s1++;
			count -= 1;
		}
	}
}


unsigned fii_brun(Pixel *s1, UBYTE *cbuf, int width, int height)
{
	UBYTE *oc, *c = cbuf;
	unsigned total = 0;

/* store offset of 1st real line and set up for main line-at-a-time loop */
	while (--height >= 0) {
		oc = c;
		c = fii_brun_comp_line(s1, c, width);
		total += (unsigned)(c - oc);
		if (total >= 60000U) 	return 0;
		s1 += width;
	}
	return (unsigned) (c - cbuf);
}
