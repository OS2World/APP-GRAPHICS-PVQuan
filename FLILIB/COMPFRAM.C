#ifdef __TURBOC__
#include <mem.h>
#endif

#include "aatypes.h"
#include "aaerr.h"
#include "aascreen.h"
#include "aafli.h"
#include "aaflisav.h"
#include "aafii.h"
#include "str_low.h"

#define FLI_EMPTY_DCOMP 8  /* Size returned by fii functions
							to indicate no change */

static unsigned full_cmap(Cbuf *cbuf, Cmap *cmap)
{
	*cbuf++ = 1;
	*cbuf++ = 0;
	*cbuf++ = 0;
	*cbuf++ = 0;
	memcpy(cbuf, cmap, AA_COLORS*3);
	return AA_COLORS*3 + 4;
}

static void chunk_buf(UBYTE *p, struct fli_chunk *c)
{
#ifdef __TURBOC__
	memcpy(p, c, CHUNK_SIZE);
#else
	p = lbuf(p, c->size);
	p = wbuf(p, c->type);
#endif
}

static UBYTE *frame_buf(UBYTE *p, struct fli_frame *f)
{
#ifdef __TURBOC__
	memcpy(p, f, FRAME_SIZE);
	return p + FRAME_SIZE;
#else
	p = lbuf(p, f->size);
	p = wbuf(p, f->type);
	p = wbuf(p, f->chunks);
	memset(p, 0, 8);
	return p + 8;
#endif
}

unsigned fli_comp_frame(
	UBYTE *comp_buf, /* Buffer - should be FLI_CBUF_SIZE or bigger */
	Pixel *last_screen, Cmap *last_cmap, 	/* Data from previous frame */
	Pixel *this_screen, Cmap *this_cmap,	/* Data for this frame */
	int type)					/* FLI_BRUN?  FLI_LCCOMP? */
{
	Cbuf colour_map[AA_COLORS*3+4];
	Cbuf *screen_map, *c;
	struct fli_frame frame;
	struct fli_chunk colour_chunk, screen_chunk;
	unsigned buf_len;

	if ((screen_map = (Cbuf *)malloc(64000U)) == NULL) return AA_ERR_NOMEM;
	memset(&frame, 0, FRAME_SIZE);
	frame.type = FLIF_MAGIC;
	frame.size = FRAME_SIZE;

/* 1st make the color map chunk */
	if (type == FLI_BRUN)	buf_len = full_cmap(colour_map, this_cmap);
	else   			buf_len = fii_fccomp(last_cmap, this_cmap, colour_map, AA_COLORS);
	colour_chunk.type = FLI_COLOR;
	colour_chunk.size = buf_len + CHUNK_SIZE;
	if (colour_chunk.size != FLI_EMPTY_DCOMP) {
		frame.chunks = 1;
		frame.size += colour_chunk.size;
	}

	switch (type) {
	case FLI_LC:
		buf_len = fii_lccomp(last_screen, this_screen, screen_map, 320, 200);
		break;
	case FLI_BRUN:
		buf_len = fii_brun(this_screen, screen_map, 320, 200);
		break;
	}
	if (buf_len == 0)	{
		screen_chunk.size = 64000L+CHUNK_SIZE;
		screen_chunk.type = FLI_COPY;
		memcpy(screen_map, this_screen, 64000U);
	} else {
		screen_chunk.type = type;
		screen_chunk.size = buf_len + CHUNK_SIZE;
	}
	if (screen_chunk.size != FLI_EMPTY_DCOMP)	{
		frame.chunks++;
		frame.size += screen_chunk.size;
	}

	c = frame_buf(comp_buf, &frame);
	if (colour_chunk.size != FLI_EMPTY_DCOMP) {
		chunk_buf(c, &colour_chunk);
		memcpy(c + CHUNK_SIZE, colour_map, (unsigned)colour_chunk.size - CHUNK_SIZE);
		c += (unsigned)colour_chunk.size;
	}
	if (screen_chunk.size != FLI_EMPTY_DCOMP) {
		chunk_buf(c, &screen_chunk);
		memcpy(c + CHUNK_SIZE, screen_map, (unsigned)screen_chunk.size - CHUNK_SIZE);
		c += (unsigned)screen_chunk.size;
	}
	free(screen_map);
	return (unsigned)frame.size;
}
