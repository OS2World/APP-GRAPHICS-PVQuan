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
 *    V1.4 920303 FvdH - Ported to GNU                                  *
 *    V1.6 921023 FvdH - Produce multi-image GIFs                       *
 *                     - Port to OS/2 IBM C Set/2                       *
 *                                                                      *
 ************************************************************************/
/*
 * This software is copyrighted as noted below.  It may be freely copied,
 * modified, and redistributed, provided that the copyright notice is
 * preserved on all copies.
 *
 * There is no warranty or other guarantee of fitness for this software,
 * it is provided solely "as is".  Bug reports or fixes may be sent
 * to the author, who may or may not act on them as he desires.
 *
 * You may not include this software in a program or other software product
 * without supplying the source, or without informing the end-user that the
 * source is available for no extra charge.
 *
 * If you modify this software, you should include a notice giving the
 * name of the person performing the modification, the date of modification,
 * and the reason for such modification.
*/
/*
 * colorquant.c
 *
 * Perform variance-based color quantization on a "full color" image.
 * Author:	Craig Kolb
 *		Department of Mathematics
 *		Yale University
 *		kolb@yale.edu
 * Date:	Tue Aug 22 1989
 * Copyright (C) 1989 Craig E. Kolb
 * $Id: colorquant.c,v 1.3 89/12/03 18:27:16 craig Exp $
 *
 * $Log:	colorquant.c,v $
 *
 * Revision 1.4  91/06/26  16:00:00  Frank van der Hulst
 * Ported to Turbo C;
 *       Call farmalloc rather than malloc
 *       Virtual memory added to swap box data to/from disk
 *       Rewritten in ANSI C
 *       Removed call to QuantHistogram() from colorquant, to allow two
 *       image files to create one palette
 *       Changed QuantHistogram() to read from file, rather than from an
 *       array
 *	      Changed format of palette to conform with the VGA palette
 *
 * Revision 1.3  89/12/03  18:27:16  craig
 * Removed bogus integer casts in distance calculation in makenearest().
 *
 * Revision 1.2  89/12/03  18:13:12  craig
 * FindCutpoint now returns FALSE if the given box cannot be cut.  This
 * to avoid overflow problems in CutBox.
 * "whichbox" in GreatestVariance() is now initialized to 0.
 *
 */

#ifdef __TURBOC__
#include <mem.h>
#define HUGE 1.79e308
#endif
#include <math.h>

#include "quant.h"
#include "heckbert.h"

#define MAX(x,y)	((x) > (y) ? (x) : (y))

static unsigned long		NPixels = 0L;			/* total # of pixels */

static int neighbours[MAXCOLORS];

/*
 * Compute the histogram of the image as well as the projected frequency
 * arrays for the first world-encompassing box.
 * We compute both the histogram and the proj. frequencies of
 * the first box at the same time to save a pass through the
 * entire image.
 * The projected frequency arrays of the largest box are zeroed out as
 * as part of open_box_file(), called from main().
 */

void QuantHistogram(Box *box)
{
unsigned long *rf, *gf, *bf;
UCHAR pixel[3];

	rf = box->freq[RED];
	gf = box->freq[GREEN];
	bf = box->freq[BLUE];

	while (get_pixel(pixel)) {
		rf[pixel[RED]]++;
		gf[pixel[GREEN]]++;
		bf[pixel[BLUE]]++;
		Histogram[(((pixel[RED]<<INPUT_BITS)|pixel[GREEN])<<INPUT_BITS)|pixel[BLUE]]++;
		NPixels++;
	}
}

/*
 * Compute mean and weighted variance of the given box.
 */
void BoxStats(Box HUGE_PTR box)
{
int i, color;
unsigned long *freq;
double mean, var;

	if(box->weight == 0) {
		box->weightedvar = 0;
		return;
	}

	box->weightedvar = 0.0;
	for (color = 0; color < 3; color++) {
		var = mean = 0;
		i = box->low[color];
		freq = &box->freq[color][i];
		for (; i < box->high[color]; i++, freq++) {
			mean += i * *freq;
			var += i*i* *freq;
		}
		box->mean[color] = mean / box->weight;
		box->weightedvar += var - box->mean[color]*box->mean[color]*box->weight;
	}
	box->weightedvar /= NPixels;
}

/*
 * Return the number of the box in 'boxes' with the greatest variance.
 * Restrict the search to those boxes with indices between 0 and n-1.
 */
int GreatestVariance(int n)
{
	int i, whichbox = 0;
	double max;
	Box *box;

	max = -1;
	for (i = 0; i < n; i++) {
		box = get_box_tmp(i);
		if (box->weightedvar > max) {
			max = box->weightedvar;
			whichbox = i;
		}
	}
	return whichbox;
}

/*
 * Update projected frequency arrays for two boxes which used to be
 * a single box.
 */
void UpdateFrequencies(Box HUGE_PTR box1, Box HUGE_PTR box2)
{
unsigned long myfreq, HUGE_PTR h;
int b, g, r;
int roff;

	memset(box1->freq[0], 0, IN_COLOURS * sizeof(unsigned long));
	memset(box1->freq[1], 0, IN_COLOURS * sizeof(unsigned long));
	memset(box1->freq[2], 0, IN_COLOURS * sizeof(unsigned long));

	for (r = box1->low[0]; r < box1->high[0]; r++) {
		roff = r << INPUT_BITS;
		for (g = box1->low[1];g < box1->high[1]; g++) {
			b = box1->low[2];
			h = Histogram + (((roff | g) << INPUT_BITS) | b);
			for (; b < box1->high[2]; b++) {
				if ((myfreq = *h++) == 0)
					continue;
				box1->freq[0][r] += myfreq;
				box1->freq[1][g] += myfreq;
				box1->freq[2][b] += myfreq;
				box2->freq[0][r] -= myfreq;
				box2->freq[1][g] -= myfreq;
				box2->freq[2][b] -= myfreq;
			}
		}
	}
}

/*
 * Compute the 'optimal' cutpoint for the given box along the axis
 * indicated by 'color'.  Store the boxes which result from the cut
 * in newbox1 and newbox2.
 */
int FindCutpoint(Box HUGE_PTR box, int color, Box HUGE_PTR newbox1, Box HUGE_PTR newbox2)
{
	double u, v, max;
	int i, maxindex, minindex, cutpoint;
	unsigned long optweight, curweight;

	if (box->low[color] + 1 == box->high[color])
		return FALSE;	/* Cannot be cut. */
	minindex = (int)((box->low[color] + box->mean[color]) * 0.5);
	maxindex = (int)((box->mean[color] + box->high[color]) * 0.5);

	cutpoint = minindex;
	optweight = box->weight;

	curweight = 0.;
	for (i = box->low[color] ; i < minindex ; i++)
		curweight += box->freq[color][i];
	u = 0.;
	max = -1;
	for (i = minindex; i <= maxindex ; i++) {
		curweight += box->freq[color][i];
		if (curweight == box->weight)
			break;
		u += (double)(i * box->freq[color][i]) / box->weight;
		v = ((double)curweight / (box->weight-curweight)) *
				(box->mean[color]-u)*(box->mean[color]-u);
		if (v > max) {
			max = v;
			cutpoint = i;
			optweight = curweight;
		}
	}
	cutpoint++;
	*newbox1 = *newbox2 = *box;
	newbox1->weight = optweight;
	newbox2->weight -= optweight;
	newbox1->high[color] = cutpoint;
	newbox2->low[color] = cutpoint;
	UpdateFrequencies(newbox1, newbox2);
	BoxStats(newbox1);
	BoxStats(newbox2);

	return TRUE;	/* Found cutpoint. */
}

/*
 * Cut the given box.  Returns TRUE if the box could be cut, FALSE otherwise.
 */
int CutBox(Box HUGE_PTR box, Box HUGE_PTR newbox)
{
	int i;
	double totalvar[3];
	static Box newboxes[3][2];  /* Only used by CutBox, but don't want it on stack */

	if (box->weightedvar == 0. || box->weight == 0)
		/*
		 * Can't cut this box.
		 */
		return FALSE;

	/*
	 * Find 'optimal' cutpoint along each of the red, green and blue
	 * axes.  Sum the variances of the two boxes which would result
	 * by making each cut and store the resultant boxes for
	 * (possible) later use.
	 */
	for (i = 0; i < 3; i++) {
		if (FindCutpoint(box, i, &newboxes[i][0], &newboxes[i][1]))
			totalvar[i] = newboxes[i][0].weightedvar +
				newboxes[i][1].weightedvar;
		else
			totalvar[i] = HUGE;
	}

	/*
	 * Find which of the three cuts minimized the total variance
	 * and make that the 'real' cut.
	 */
	if (totalvar[RED] <= totalvar[GREEN] &&
	    totalvar[RED] <= totalvar[BLUE]) {
		*box = newboxes[RED][0];
		*newbox = newboxes[RED][1];
	} else if (totalvar[GREEN] <= totalvar[RED] &&
		 totalvar[GREEN] <= totalvar[BLUE]) {
		*box = newboxes[GREEN][0];
		*newbox = newboxes[GREEN][1];
	} else {
		*box = newboxes[BLUE][0];
		*newbox = newboxes[BLUE][1];
	}

	return TRUE;
}

/*
 * Iteratively cut the boxes.
 */
CutBoxes(int colors)
{
int curbox, varbox;
Box *box = get_box(0);

	box->low[RED] = box->low[GREEN] = box->low[BLUE] = 0;
	box->high[RED] = box->high[GREEN] = box->high[BLUE] = IN_COLOURS;
	box->weight = NPixels;

	BoxStats(box);
	free_box(0);

	printf("%d Boxes: cutting box: ", colors);
	for (curbox = 1; curbox < colors; curbox++) {
		printf("%3d\b\b\b", curbox);
		varbox = GreatestVariance(curbox);
		if (CutBox(get_box(varbox), get_box(curbox)) == FALSE)
				break;
		free_box(curbox);
		free_box(varbox);
	}
	printf("Done\n");

	return curbox;
}

/*
 * Make the centroid of "boxnum" serve as the representative for
 * each color in the box.
 */
void SetRGBmap(int boxnum, Box *box, int bits)
{
int r, g, b;

	for (r = box->low[RED]; r < box->high[RED]; r++) {
		for (g = box->low[GREEN]; g < box->high[GREEN]; g++) {
			for (b = box->low[BLUE]; b < box->high[BLUE]; b++) {
				RGBmap[(((r<<bits)|g)<<bits)|b]=(char)boxnum;
			}
		}
	}
}

/*
 * In order to minimize our search for 'best representative', we form the
 * 'neighbors' array.  This array contains the number of the boxes whose
 * centroids *might* be used as a representative for some color in the
 * current box.  We need only consider those boxes whose centroids are closer
 * to one or more of the current box's corners than is the centroid of the
 * current box. 'Closeness' is measured by Euclidean distance.
 */
int getneighbors(int num, int colors)
{
	int i, j;
	Box *bp;
	double dist, LowR, LowG, LowB, HighR, HighG, HighB, ldiff, hdiff;

	bp = get_box_tmp(num);

	ldiff = bp->low[RED] - bp->mean[RED];
	ldiff *= ldiff;
	hdiff = bp->high[RED] - bp->mean[RED];
	hdiff *= hdiff;
	dist = MAX(ldiff, hdiff);

	ldiff = bp->low[GREEN] - bp->mean[GREEN];
	ldiff *= ldiff;
	hdiff = bp->high[GREEN] - bp->mean[GREEN];
	hdiff *= hdiff;
	dist += MAX(ldiff, hdiff);

	ldiff = bp->low[BLUE] - bp->mean[BLUE];
	ldiff *= ldiff;
	hdiff = bp->high[BLUE] - bp->mean[BLUE];
	hdiff *= hdiff;
	dist += MAX(ldiff, hdiff);

	dist = sqrt(dist);

	/*
	 * Loop over all colors in the colormap, the ith entry of which
	 * corresponds to the ith box.
	 *
	 * If the centroid of a box is as close to any corner of the
	 * current box as is the centroid of the current box, add that
	 * box to the list of "neighbors" of the current box.
	 */
	HighR = (double)bp->high[RED] + dist;
	HighG = (double)bp->high[GREEN] + dist;
	HighB = (double)bp->high[BLUE] + dist;
	LowR = (double)bp->low[RED] - dist;
	LowG = (double)bp->low[GREEN] - dist;
	LowB = (double)bp->low[BLUE] - dist;
	for (i = j = 0; i < colors; i++) {
		bp = get_box_tmp(i);
		if (LowR <= bp->mean[RED] && HighR >= bp->mean[RED] &&
		    LowG <= bp->mean[GREEN] && HighG >= bp->mean[GREEN] &&
		    LowB <= bp->mean[BLUE] && HighB >= bp->mean[BLUE])
			neighbours[j++] = i;
	}

	return j;	/* Return the number of neighbors found. */
}

/*
 * Assign representative colors to every pixel in a given box through
 * the construction of the NearestColor array.  For each color in the
 * given box, we look at the list of neighbors passed to find the
 * one whose centroid is closest to the current color.
 */
void makenearest(int boxnum, int nneighbors, int bits)
{
	int n, b, g, r;
	double rdist, gdist, bdist, dist, mindist;
	int which, *np;
	Box *box, *bp;

	box = get_box(boxnum);

	for (r = box->low[RED]; r < box->high[RED]; r++) {
		for (g = box->low[GREEN]; g < box->high[GREEN]; g++) {
			for (b = box->low[BLUE]; b < box->high[BLUE]; b++) {
/*
 * The following two lines should be commented out if the RGBmap is going
 * to be used for images other than the one given.
 */
				if (Histogram[(((r<<bits)|g)<<bits)|b] == 0)
					continue;
				mindist = HUGE;
				/*
				 * Find the colormap entry which is
				 * closest to the current color.
				 */
				np = neighbours;
				for (n = 0; n < nneighbors; n++, np++) {
					bp = get_box_tmp(*np);
					rdist = r - bp->mean[RED];
					gdist = g - bp->mean[GREEN];
					bdist = b - bp->mean[BLUE];
					dist = rdist*rdist + gdist*gdist + bdist*bdist;
					if (dist < mindist) {
						mindist = dist;
						which = *np;
					}
				}
				/*
				 * The colormap entry closest to this
				 * color is used as a representative.
				 */
				RGBmap[(((r<<bits)|g)<<bits)|b] = which;
			}
		}
	}
	free_box(boxnum);
}
/*
 * Form colormap and NearestColor arrays.
 */
void find_colors(int colors, int bits)
{
int i;
int num;

	/*
	 * Form map of representative (nearest) colors.
	 */
	printf("Mapping colours for box: ");
	for (i = 0; i < colors; i++) {
		printf("%3d\b\b\b", i);
		/*
		 * Create list of candidate neighbors and
		 * find closest representative for each
		 * color in the box.
		 */
		num = getneighbors(i, colors);
		makenearest(i, num, bits);
	}
	printf("Done\n");
}

/*
 * Compute RGB to colormap index map.
 */
void ComputeRGBMap(int colors, int bits, int fast)
{
int i;

	if (fast) {
		/*
		 * The centroid of each box serves as the representative
		 * for each color in the box.
		 */
		for (i = 0; i < colors; i++)
			SetRGBmap(i, get_box_tmp(i), bits);
	} else
		/*
		 * Find the 'nearest' representative for each pixel.
		 */
		find_colors(colors, bits);
}

/*
 * Perform variance-based color quantization on a 24-bit image.
 *
 * Input consists of:
 *	in_file	Pointer to file containing of red, green and blue pixel
 *				intensities stored as unsigned characters.
 *				The color of the ith pixel is given consecutive bytes, in the
 *				order red, then green, then blue. Only the LS 4 bits are used.
 *				0 indicates zero intensity, 15 full intensity.
 *	pixels	The length of the red, green and blue arrays
 *				in bytes, stored as an unsigned long int.
 *	colormap	Points to the colormap.  The colormap
 *				consists of red, green and blue arrays.
 *				The red/green/blue values of the ith
 *				colormap entry are given respectively by
 *				colormap[0][i], colormap[1][i] and
 *				colormap[2][i].  Each entry is an unsigned char.
 *	colors	The number of colormap entries, stored
 *				as an integer.
 *	bits		The number of significant bits in each entry
 *				of the red, green and blue arrays. An integer.
 *	rgbmap	An array of unsigned chars of size (2^bits)^3.
 *				This array is used to map from pixels to
 *				colormap entries.  The 'prequantized' red,
 *				green and blue components of a pixel
 *				are used as an index into rgbmap to retrieve
 *				the index which should be used into the colormap
 *				to represent the pixel.  In short:
 *				index = rgbmap[(((r << bits) | g) << bits) | b];
 * 	fast	If non-zero, the rgbmap will be constructed
 *				quickly.  If zero, the rgbmap will be built
 *				much slower, but more accurately.  In most
 *				cases, fast should be non-zero, as the error
 *				introduced by the approximation is usually
 *				small.  'Fast' is stored as an integer.
 *	Cfactor	Conversion factor.
 *
 * colorquant returns the number of colors to which the image was
 * quantized.
 */

int colorquant(int colors, int bits, int fast, double Cfactor)
{
int	i,						/* Counter */
		OutColors;			/* # of entries computed */
Box	*box;

	OutColors = CutBoxes(colors);
	/*
	 * We now know the set of representative colors.  We now
	 * must fill in the colormap and convert the representatives
	 * from their 'prequantized' range to 0-FULLINTENSITY.
	 */
	for (i = 0; i < OutColors; i++) {
		box = get_box_tmp(i);
		palette[i][RED]   = (char)(box->mean[RED] * Cfactor + 0.5);
		palette[i][GREEN] = (char)(box->mean[GREEN] * Cfactor + 0.5);
		palette[i][BLUE]  = (char)(box->mean[BLUE] * Cfactor + 0.5);
	}

	ComputeRGBMap(OutColors, bits, fast);

	return OutColors;		/* Return # of colormap entries */
}

int pal_index(UCHAR *pixel)
{
	return RGBmap[(((pixel[RED]<<INPUT_BITS)|pixel[GREEN])<<INPUT_BITS)|pixel[BLUE]];
}

