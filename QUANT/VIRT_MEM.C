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
 *    V1.4 920303 FvdH - Ported to GNU C                                *
 *    V1.6 921023 FvdH - Produce multi-image GIFs                       *
 *                     - Port to OS/2 IBM C Set/2                       *
 *                                                                      *
 ************************************************************************/
/* virt_mem.c
	"Virtual memory" for QUANT. On a PC, these routines open a disk file
	for use as a virtual memory buffer for BOX structure items. The most
	commonly used boxes are cached in memory. As much memory as is available
	is utilised for this cache. In SCO Unix sufficient memory is simply
	malloc'ed. The buffer can be up to 800K long, depending on how	many
	colours are to be produced. */

#ifdef __TURBOC__
#include <mem.h>
#endif

#include "quant.h"
#include "heckbert.h"

#ifdef __TURBOC__
typedef struct {
	int	num;
	int	age;
	int	in_use;
	Box	box;
} BOX_BUFF;

static FILE *box_file;

static BOX_BUFF **box_buff;
#else
static Box **box_buff;
#endif
static int num_buffs;

/* Get a box (from disk if necessary), and lock it in memory so that it
	can't be swapped out again. Returns a pointer to where the box is
	located in memory. */

Box *get_box(int n)
{
#ifdef __TURBOC__
int i, oldest;
	oldest = 0;
	for (i = 0; i < num_buffs; i++) {
		if (box_buff[i]->num == n) {
			box_buff[i]->age--;
			box_buff[i]->in_use = TRUE;
			return &box_buff[i]->box;
		} else {
			if (box_buff[i]->in_use) continue;
			if (box_buff[i]->age > box_buff[oldest]->age)		oldest = i;
		}
	}
	if (box_buff[oldest]->in_use) {
		printf("\nInsufficient virtual memory buffers.\n");
		err_exit();
	}
	if (box_buff[oldest]->num != -1) {
		if (fseek(box_file, (long) box_buff[oldest]->num * sizeof(Box), SEEK_SET) != 0) {
			printf("\nError seeking BOX_FILE.TMP for write\n");
			err_exit();
		}
		if (fwrite(&box_buff[oldest]->box, sizeof(Box), 1, box_file) != 1) {
			printf("\nError writing BOX_FILE.TMP\n");
			err_exit();
		}
	}
	if (fseek(box_file, (long) n * sizeof(Box), SEEK_SET) != 0) {
		printf("\nError seeking BOX_FILE.TMP for read\n");
		err_exit();
	}
	if (fread(&box_buff[oldest]->box, sizeof(Box), 1, box_file) != 1) {
		printf("\nError reading BOX_FILE.TMP\n");
		err_exit();
	}
	box_buff[oldest]->num = n;
	box_buff[oldest]->age = 0;
	box_buff[oldest]->in_use = TRUE;
	return &box_buff[oldest]->box;
#else
	return box_buff[n];
#endif
}

/* Get a box (from disk if necessary), and allow it to be swapped out again.
	Returns a pointer to where the box is located in memory. */

Box *get_box_tmp(int n)
{
#ifdef __TURBOC__
int i, oldest;

	oldest = 0;
	for (i = 0; i < num_buffs; i++) {
		if (box_buff[i]->num == n) {
			box_buff[i]->age--;
			return &box_buff[i]->box;
		} else {
			if (box_buff[i]->in_use) continue;
			if (box_buff[i]->age > box_buff[oldest]->age)		oldest = i;
		}
	}
	if (box_buff[oldest]->in_use) {
		printf("\nInsufficient virtual memory buffers.\n");
		err_exit();
	}
	if (box_buff[oldest]->num != -1) {
		if (fseek(box_file, (long) box_buff[oldest]->num * sizeof(Box), SEEK_SET) != 0) {
			printf("\nError seeking BOX_FILE.TMP for write\n");
			err_exit();
		}
		if (fwrite(&box_buff[oldest]->box, sizeof(Box), 1, box_file) != 1) {
			printf("\nError writing BOX_FILE.TMP\n");
			err_exit();
		}
	}
	if (fseek(box_file, (long) n * sizeof(Box), SEEK_SET) != 0) {
		printf("\nError seeking BOX_FILE.TMP for read\n");
		err_exit();
	}
	if (fread(&box_buff[oldest]->box, sizeof(Box), 1, box_file) != 1) {
		printf("\nError reading BOX_FILE.TMP\n");
		err_exit();
	}
	box_buff[oldest]->num = n;
	box_buff[oldest]->age = 0;
	return &box_buff[oldest]->box;
#else
	return box_buff[n];
#endif
}

/* Free a previously locked box so that it can be swapped out to disk. */

void free_box(int n)
{
#ifdef __TURBOC__
int i;

	for (i = 0; i < num_buffs; i++) {
		if (box_buff[i]->num == n) {
			box_buff[i]->in_use = FALSE;
			return;
		}
	}
#endif
}

/* Grab almost all of the remaining memory (up to the number of colours
	being produced), leaving only a little for printf(), etc.

	At least 3 buffers must be available, since the program locks 2 and
	then wants to load a third one to compare data.

	This routine clears box[0], which is assumed by function main() in
	QUANT.C. It also writes 0's to the entire disk buffer. Initially, all
	cache buffers are set to -1, unused.
*/

void open_box_file(int buffs_needed)
{
int i;
#ifdef __TURBOC__
unsigned long coreleft;

	coreleft = farcoreleft();
	if (coreleft < 0x4000L + sizeof(BOX_BUFF) * 3) {
		printf("Insufficient memory: %ld needed, %ld available\n",
					0x4000L + sizeof(BOX_BUFF) * 3, coreleft);
		err_exit();
	}
	num_buffs = (int)((coreleft - 0x4000L) / sizeof(BOX_BUFF));
	if (num_buffs > buffs_needed) num_buffs = buffs_needed;
	CHECK_ALLOC(box_buff, BOX_BUFF *, num_buffs, "Box Buffer");
	for (i = 0; i < num_buffs; i++)
		CHECK_ALLOC(box_buff[i], BOX_BUFF, 1, "Box Buffer");
	printf("%d virtual memory buffers allocated\n", num_buffs);
	box_file = fopen("VIRT_MEM.TMP", "w+b");
	if (box_file == NULL) {
		printf("Couldn't open VIRT_MEM.TMP\n");
		err_exit();
	}
	for (i = 0; i < num_buffs; i++) {
		box_buff[i]->num    = -1;
		box_buff[i]->age    = 0x7fff;
		box_buff[i]->in_use = FALSE;
	}

	printf("Clearing virtual memory disk buffer.\n");
	memset(&box_buff[0]->box, 0, sizeof(Box));
	for (i = 0; i < buffs_needed; i++) {
		if (fwrite(&box_buff[0]->box, sizeof(Box), 1, box_file) != 1) {
			printf("\nError writing VIRT_MEM.TMP\n");
			err_exit();
		}
	}
#else

    CHECK_ALLOC(box_buff, Box *, buffs_needed, "Box buffer");
	num_buffs = buffs_needed;
	for (i = 0; i < buffs_needed; i++)
		CHECK_ALLOC(box_buff[i], Box, 1, "Box buffer");
	printf("%d box buffers allocated\n", buffs_needed);

	for (i = 0; i < buffs_needed; i++)
		memset(box_buff[i], 0, sizeof(Box));
#endif
}

/* Release cache buffers, and delete the disk buffer file */

void close_box_file(void)
{
int i;
	for (i = 0; i < num_buffs; i++) 	free(box_buff[i]);
	free(box_buff);
#ifdef __TURBOC__
	fclose(box_file);
	unlink("VIRT_MEM.TMP");
#endif
}
