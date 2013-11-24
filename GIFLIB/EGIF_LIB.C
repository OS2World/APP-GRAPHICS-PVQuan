/************************************************************************
 *                                                                      *
 *                  Copyright (c) 1991, Frank van der Hulst             *
 *                          All Rights Reserved                         *
 *                                                                      *
 * Authors:                                                             *
 *        FvdH - Frank van der Hulst (Wellington, NZ)                   *
 *                                                                      *
 * Versions:                                                            *
 *      V1.1 910626 FvdH - QUANT released for DBW_RENDER                *
 *      V1.2 911021 FvdH - QUANT released for PoV Ray                   *
 *      V1.4 920303 FvdH - Ported to GNU                                *
 *                                                                      *
 ************************************************************************/
/******************************************************************
*   "Gif-Lib" - Yet another gif library.					      		*
*																		      		*
* Written by:  Gershon Elber				Ver 1.1, Aug. 1990   		*
*******************************************************************
* The kernel of the GIF Encoding process can be found here.			*
*******************************************************************
* History:								      									*
* 14 Jun 89 - Version 1.0 by Gershon Elber.						      *
*  3 Sep 90 - Version 1.1 by Gershon Elber (Gif89, Unique names). *
*******************************************************************/

#include <string.h>

#ifdef __TURBOC__
#include <alloc.h>
#endif

#include "gif_lib.h"
#include "gif_hash.h"

#define	TRUE	1
#define	FALSE	0

#define FLUSH_OUTPUT		4096	/* Impossible code, to signal flush. */
#define FIRST_CODE		4097    /* Impossible code, to signal first. */

#define RED		0
#define GREEN	1
#define BLUE	2

static int	Private_BitsPerPixel;	    	/* Bits per pixel (Codes uses at list this + 1). */
static unsigned long Private_PixelCount;
static unsigned char Private_Buf[256];	 		   	  	/* Compressed output is buffered here. */
static int Private_ClearCode;					   /* The CLEAR LZ code. */
static int Private_EOFCode;							/* The EOF LZ code. */
static int Private_RunningCode;				   /* The next code algorithm can generate. */
static int Private_RunningBits;					/* The number of bits required to represent RunningCode. */
static int Private_MaxCode1;						/* 1 bigger than max. possible code, in RunningBits bits. */
static int Private_CrntCode;						/* Current algorithm code. */
static int Private_CrntShiftState;				/* Number of bits in CrntShiftDWord. */
static unsigned long Private_CrntShiftDWord;  /* For bytes decomposition into codes. */

static int Gif_SBitsPerPixel;					 	/* How many colors can we generate? */

/* Masks given codes to BitsPerPixel, to make sure all codes are in range: */
static char CodeMask[] = {
    0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff
};

static unsigned long *hash_table;
FILE *GIF_file;

static char *GifVersionPrefix = "GIF87a";

static void	EGifPutWord(int Word);
static void	EGifSetupCompress(void);
static int EGifCompressLine(unsigned char *Line, int LineLen);
static int EGifCompressOutput(int Code);
static int EGifBufferedOutput(unsigned char *Buf, int c);

/******************************************************************************
*   Open a new gif file for write, given by its name. If TestExistance then   *
* if the file exists this routines fails (returns NULL).		      *
*   Returns GifFileType pointer dynamically allocated which serves as the gif *
* info record. _GifError is cleared if succesfull.			      *
******************************************************************************/
int EGifOpenFileName(char *FileName)
{
	GIF_file = fopen(FileName, "wb");
	if (GIF_file == NULL) {
		printf("EGIF Error: OPEN FAILED");
		return -1;
	}

	if (fwrite(GifVersionPrefix, 1, strlen(GifVersionPrefix), GIF_file) !=
						strlen(GifVersionPrefix)) {
		printf("EGIF Error: WRITE FAILED");
		return -1;
	}

	if ((hash_table = (unsigned long *) malloc(sizeof(long) * HT_SIZE)) == NULL) {
		printf("GIF Error: Not enough memory");
		return -1;
	}
	HashTable_Clear(hash_table);

	return 0;
}

/******************************************************************************
*   This routine should be called before any other EGif calls, immediately    *
* follows the GIF file openning.					      *
******************************************************************************/
void EGifPutScreenDesc(int Width, int Height, int ColorRes, int BackGround,
	int BitsPerPixel, unsigned char ColorMap[][3])
{
int i;

	Gif_SBitsPerPixel = BitsPerPixel;

    /* Put the screen descriptor into the file: */
	EGifPutWord(Width);
	EGifPutWord(Height);
	putc(0x80 | ((ColorRes - 1) << 4) | (BitsPerPixel - 1), GIF_file);
	putc(BackGround, GIF_file);
	putc(0, GIF_file);

    /* If we have Global color map - dump it also: */
	for (i = 0; i < (1 << BitsPerPixel); i++) {
	    /* Put the ColorMap out also: */
		putc(ColorMap[i][RED]   << (8 - ColorRes), GIF_file);
		putc(ColorMap[i][GREEN] << (8 - ColorRes), GIF_file);
		putc(ColorMap[i][BLUE]  << (8 - ColorRes), GIF_file);
	}
}

/******************************************************************************
*   This routine should be called before any attemp to dump an image - any    *
* call to any of the pixel dump routines.				      *
******************************************************************************/
void EGifPutImageDesc(int Left, int Top, int Width, int Height, int BitsPerPixel)
{
    /* Put the image descriptor into the file: */
	 putc(',', GIF_file);
	 EGifPutWord(Left);
	 EGifPutWord(Top);
	 EGifPutWord(Width);
	 EGifPutWord(Height);
	 putc(BitsPerPixel - 1, GIF_file);

    Private_PixelCount = (long) Width * (long) Height;

	 EGifSetupCompress();      /* Reset compress algorithm parameters. */
}

/******************************************************************************
*  Put one full scanned line (Line) of length LineLen into GIF file.	      *
******************************************************************************/
int EGifPutLine(unsigned char *Line, int LineLen)
{
    int i;
	char Mask;

    if ((Private_PixelCount -= LineLen) < 0) {
	printf("E_GIF_ERR_DATA_TOO_BIG");
	return TRUE;
    }

    /* Make sure the codes are not out of bit range, as we might generate    */
    /* wrong code (because of overflow when we combine them) in this case:   */
    Mask = CodeMask[Private_BitsPerPixel];
    for (i = 0; i < LineLen; i++) Line[i] &= Mask;

	 return EGifCompressLine(Line, LineLen);
}

/******************************************************************************
*   This routine should be called last, to close GIF file.		      *
******************************************************************************/
void EGifCloseFile(void)
{
	putc(';', GIF_file);

	if (hash_table) free(hash_table);

	if (fclose(GIF_file) != 0) printf("E_GIF_ERR_CLOSE_FAILED");
}

/******************************************************************************
*   Put 2 bytes (word) into the given file:				      *
******************************************************************************/
static void EGifPutWord(int Word)
{
char c[2];

	c[0] = Word & 0xff;
	c[1] = (Word >> 8) & 0xff;
	fwrite(c, 1, 2, GIF_file);
}

/******************************************************************************
*   Setup the LZ compression for this image:				      *
******************************************************************************/
static void EGifSetupCompress(void)
{
    int BitsPerPixel;
	char Buf;

    /* Test and see what color map to use, and from it # bits per pixel: */
	BitsPerPixel = Gif_SBitsPerPixel;

    Buf = BitsPerPixel = (BitsPerPixel < 2 ? 2 : BitsPerPixel);
    fwrite(&Buf, 1, 1, GIF_file);     /* Write the Code size to file. */

    Private_Buf[0] = 0;			  /* Nothing was output yet. */
    Private_BitsPerPixel = BitsPerPixel;
    Private_ClearCode = (1 << BitsPerPixel);
    Private_EOFCode = Private_ClearCode + 1;
    Private_RunningCode = Private_EOFCode + 1;
    Private_RunningBits = BitsPerPixel + 1;	 /* Number of bits per code. */
    Private_MaxCode1 = 1 << Private_RunningBits;	   /* Max. code + 1. */
    Private_CrntCode = FIRST_CODE;	   /* Signal that this is first one! */
    Private_CrntShiftState = 0;      /* No information in CrntShiftDWord. */
    Private_CrntShiftDWord = 0;

    /* Clear hash table and send Clear to make sure the decoder do the same. */
	 HashTable_Clear(hash_table);
	if (EGifCompressOutput(Private_ClearCode)) printf("E_GIF_ERR_DISK_IS_FULL");
}

/******************************************************************************
*   The LZ compression routine:						      *
*   This version compress the given buffer Line of length LineLen.	      *
*   This routine can be called few times (one per scan line, for example), in *
* order the complete the whole image.					      *
******************************************************************************/
static int EGifCompressLine(unsigned char *Line, int LineLen)
{
int i = 0, CrntCode, NewCode;
unsigned long NewKey;
unsigned char Pixel;

	if (Private_CrntCode == FIRST_CODE)		  /* Its first time! */
		CrntCode = Line[i++];
	else CrntCode = Private_CrntCode;     /* Get last code in compression. */

	while (i < LineLen) {			    /* Decode LineLen items. */
		Pixel = Line[i++];		      /* Get next pixel from stream. */
	/* Form a new unique key to search hash table for the code combines  */
	/* CrntCode as Prefix string with Pixel as postfix char.	     */
		NewKey = (((unsigned long) CrntCode) << 8) + Pixel;
		if ((NewCode = HashTable_Exists(hash_table, NewKey)) >= 0) {
	    /* This Key is already there, or the string is old one, so	     */
	    /* simple take new code as our CrntCode:			     */
			CrntCode = NewCode;
		} else {
	    /* Put it in hash table, output the prefix code, and make our    */
	    /* CrntCode equal to Pixel.					     */
			if (EGifCompressOutput(CrntCode)) {
				printf("E_GIF_ERR_DISK_IS_FULL");
				return TRUE;
			}
			CrntCode = Pixel;

	    /* If however the HashTable if full, we send a clear first and   */
	    /* Clear the hash table.					     */
			if (Private_RunningCode >= ZL_MAX_CODE) {
		/* Time to do some clearance: */
				if (EGifCompressOutput(Private_ClearCode)) {
					printf("E_GIF_ERR_DISK_IS_FULL");
					return TRUE;
				}
				Private_RunningCode = Private_EOFCode + 1;
				Private_RunningBits = Private_BitsPerPixel + 1;
				Private_MaxCode1 = 1 << Private_RunningBits;
				HashTable_Clear(hash_table);
			} else {
		/* Put this unique key with its relative Code in hash table: */
				HashTable_Insert(hash_table, NewKey, Private_RunningCode++);
			}
		}
	}

    /* Preserve the current state of the compression algorithm: */
	Private_CrntCode = CrntCode;

	if (Private_PixelCount == 0) {
	/* We are done - output last Code and flush output buffers: */
		if (EGifCompressOutput(CrntCode)) {
			printf("E_GIF_ERR_DISK_IS_FULL");
			return TRUE;
		}
		if (EGifCompressOutput(Private_EOFCode)) {
			printf("E_GIF_ERR_DISK_IS_FULL");
			return TRUE;
		}
		if (EGifCompressOutput(FLUSH_OUTPUT)) {
			printf("E_GIF_ERR_DISK_IS_FULL");
			return TRUE;
		}
	}

	return FALSE;
}

/******************************************************************************
*   The LZ compression output routine:					      *
*   This routine is responsable for the compression of the bit stream into    *
* 8 bits (bytes) packets.						      *
*   Returns GIF_OK if written succesfully.				      *
******************************************************************************/
static int EGifCompressOutput(int Code)
{
int retval = FALSE;

	if (Code == FLUSH_OUTPUT) {
		while (Private_CrntShiftState > 0) {
	    /* Get Rid of what is left in DWord, and flush it. */
			if (EGifBufferedOutput(Private_Buf,	(int)(Private_CrntShiftDWord) & 0xff))
				retval = TRUE;
			Private_CrntShiftDWord >>= 8;
			Private_CrntShiftState -= 8;
		}
		Private_CrntShiftState = 0;			   /* For next time. */
		if (EGifBufferedOutput(Private_Buf, FLUSH_OUTPUT))
			retval = TRUE;
	 } else {
		Private_CrntShiftDWord |= ((long) Code) << Private_CrntShiftState;
		Private_CrntShiftState += Private_RunningBits;
		while (Private_CrntShiftState >= 8) {
	    /* Dump out full bytes: */
			if (EGifBufferedOutput(Private_Buf,	(int)(Private_CrntShiftDWord) & 0xff))
				retval = TRUE;
			Private_CrntShiftDWord >>= 8;
			Private_CrntShiftState -= 8;
		}
	}

    /* If code cannt fit into RunningBits bits, must raise its size. Note */
    /* however that codes above 4095 are used for special signaling.      */
	if (Private_RunningCode >= Private_MaxCode1 && Code <= 4095) {
		Private_MaxCode1 = 1 << ++Private_RunningBits;
	}

	return retval;
}

/******************************************************************************
*   This routines buffers the given characters until 255 characters are ready *
* to be output. If Code is equal to -1 the buffer is flushed (EOF).	      *
*   The buffer is Dumped with first byte as its size, as GIF format requires. *
*   Returns GIF_OK if written succesfully.				      *
******************************************************************************/
static int EGifBufferedOutput(unsigned char *Buf, int c)
{
	if (c == FLUSH_OUTPUT) {
	/* Flush everything out. */
		if (Buf[0] != 0 && fwrite(Buf, 1, Buf[0]+1, GIF_file) != Buf[0] + 1)	{
			printf("E_GIF_ERR_WRITE_FAILED");
			return TRUE;
		}
	/* Mark end of compressed data, by an empty block (see GIF doc): */
		putc(0, GIF_file);
	} else {
		if (Buf[0] == 255) {
	    /* Dump out this buffer - it is full: */
			if (fwrite(Buf, 1, Buf[0] + 1, GIF_file) != Buf[0] + 1) {
				printf("E_GIF_ERR_WRITE_FAILED");
				return TRUE;
			}
			Buf[0] = 0;
		}
		Buf[++Buf[0]] = c;
	}

	return FALSE;
}
