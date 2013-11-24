/* aaflisav.h  Copyright 1990 Dancing Flame, San Francisco */

#ifndef AAFLISAV_H
#define AAFLISAV_H

/* Compress a frame into a RAM buffer.  Return size of frame. */
unsigned fli_comp_frame(Cbuf *comp_buf, 		/* Buffer FLI_CBUF_SIZE or bigger */
	Pixel *last_screen, Cmap *last_cmap, 	/* Data from previous frame */
	Pixel *this_screen, Cmap *this_cmap,	/* Data for this frame */
	int type);										/* FLI_BRUN?  FLI_LC? */

/* Open FLI file and write out first frame.  Return file handle if things
   go well, error code otherwise */
FILE *fli_create(char *fliname,         /* file name */
	Fli_head *fh, 					/* fli_create will initialize this */
	int speed);						/* Speed in 1/70th of a second */

/* Write succeeding frames of a FLI. */
Errval fli_write_next(FILE *ff,                 /* FLI file returned by fli_create */
	Fli_head *fh, 						/* Same header used by fli_create */
	Vscreen *this, 						/* Current frame */
	Vscreen *last);						/* Previous frame */

/* Finish up writing a FLI file...
   Write the 'ring frame', that is the difference between the first and
   last frame of a fli.  Pass in the final frame of the FLI in last_frame.
   firstf_buf will be loaded with the first frame of the FLI as a side
   effect.  */ 
Errval fli_end(FILE *ff, Fli_head *fh, Vscreen *end_frame, Vscreen *firstf_buf);

#endif /* AAFLISAV_H */
