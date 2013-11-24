#ifdef __TURBOC__
#include <mem.h>
#endif

#ifdef __GNUC__
#define SEEK_SET 0
#endif

#include "aatypes.h"
#include "aaerr.h"
#include "aascreen.h"
#include "aafli.h"
#include "aaflisav.h"
#include "str_low.h"

static Errval write_a_fframe(FILE *ff,  /* Fli file returned by fli_create */
	Fli_head *fh, 					/* Header inited by fli_create */
	Vscreen *this, Vscreen *last, 		/* Current and previous frame */
	int compress_type, 					/* FLI_BRUN, FLI_LC, etc. */
	int frame_counts)					/* 0 on ring frame, otherwise 1 */
{
	Cbuf *cbuf;
	unsigned int fsize;
	Pixel *lpixels;
	Pixel *lcmap;
	Errval err = AA_SUCCESS;

	if ((cbuf = (Cbuf *)malloc(FLI_CBUF_SIZE)) == NULL)	return AA_ERR_NOMEM;
	if (last == NULL)	{
		lpixels = NULL;
		lcmap = NULL;
	} else {
		lpixels = last->pmap;
		lcmap = last->cmap;
	}
	fsize = fli_comp_frame(cbuf, lpixels, lcmap, this->pmap, this->cmap,	compress_type);
	fsize = fwrite(cbuf, 1, fsize, ff);
	if (fsize == 0)		err = AA_ERR_SHORTWRITE;
	free(cbuf);
	fh->size += fsize;
	fh->frame_count += frame_counts;
	return err;
}

static Errval write_head(FILE *ff, Fli_head *fh)
{
#ifdef __TURBOC__
	if (fwrite(fh, HEAD_SIZE, 1, ff) != 1)													return AA_ERR_SHORTWRITE;
	return AA_SUCCESS;
#else
	Fli_head buf;
	Cbuf *c;

	c = lbuf((Cbuf *) &buf, fh->size);
	c = wbuf(c, fh->type);
	c = wbuf(c, fh->frame_count);
	c = wbuf(c, fh->width);
	c = wbuf(c, fh->height);
	c = wbuf(c, fh->bits_a_pixel);
	c = wbuf(c, fh->flags);
	c = wbuf(c, fh->speed);
	c = lbuf(c, fh->next_head);
	c = lbuf(c, fh->frames_in_table);
	c = wbuf(c, fh->file);
	c = lbuf(c, fh->frame1_off);
	c = lbuf(c, fh->strokes);
	c = lbuf(c, fh->session);
	memset(c, 0, 88);
	if (fwrite(&buf, HEAD_SIZE, 1, ff) != 1)													return AA_ERR_SHORTWRITE;
	return AA_SUCCESS;
#endif
}

Errval fli_write_next(FILE *ff,         /* Fli file returned by fli_create */
	Fli_head *fh, 					/* Same header used by fli_create */
	Vscreen *this, 						/* Current frame */
	Vscreen *last)						/* Previous frame */
{
	return write_a_fframe(ff, fh, this, last,
		(fh->frame_count == 0 ? FLI_BRUN : FLI_LC), 1);
}

/* Write the 'ring frame', that is the difference between the first and
	last frame of a fli.  Pass in the final frame of the FLI in last_frame,
	and the first frame in firstf_buf. */

Errval fli_end(FILE *ff, Fli_head *fh, Vscreen *end_frame, Vscreen *firstf_buf)
{
	Errval err;

	if ((err = write_a_fframe(ff, fh, firstf_buf, end_frame, FLI_LC, 0)) < AA_SUCCESS)	return err;
	if (fseek (ff, 0L, SEEK_SET) < AA_SUCCESS)														return AA_ERR_SEEK;
	fh->flags = (FLI_FINISHED | FLI_LOOPED);
	return write_head(ff, fh);
}

FILE *fli_create(char *fliname, Fli_head *fh, int speed)
{
	FILE *ff;

	if ((ff = fopen(fliname, "wb")) == NULL)    return NULL;
	memset(fh, 0, HEAD_SIZE);	/* zero out counts and so forth */
	fh->type = FLIH_MAGIC;
	fh->size = HEAD_SIZE;
	fh->width = 320;
	fh->height = 200;
	fh->bits_a_pixel = 8;
	fh->speed = speed;
	if (fwrite(fh, HEAD_SIZE, 1, ff) != 1) {
		fclose(ff);
		return NULL;
	}
	return ff;
}
