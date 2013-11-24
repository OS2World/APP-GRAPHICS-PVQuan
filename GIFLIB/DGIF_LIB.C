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
 *      V1.6 921022 FvdH - Ported to OS/2                               *
 *                                                                      *
 ************************************************************************/
/******************************************************************************
*   "Gif-Lib" - Yet another gif library.				      *
*									      *
* Written by:  Gershon Elber			IBM PC Ver 1.1,	Aug. 1990     *
*******************************************************************************
* The kernel of the GIF Decoding process can be found here.		      *
*******************************************************************************
* History:								      *
* 16 Jun 89 - Version 1.0 by Gershon Elber.				      *
*  3 Sep 90 - Version 1.1 by Gershon Elber (Support for Gif89, Unique names). *
******************************************************************************/


#include <stdlib.h>
#ifdef __TURBOC__
#include <io.h>
#include <alloc.h>
#include <sys\stat.h>
#endif

#include <fcntl.h>
#include <string.h>

#include "gif_lib.h"
#include "gif_hash.h"

#define LZ_MAX_CODE	4095		/* Biggest code possible in 12 bits. */
#define LZ_BITS		12

#define GIF_STAMP_LEN	sizeof(GIF_STAMP) - 1
#define GIF_STAMP	"GIFVER"	 /* First chars in file - GIF stamp. */
#define GIF_VERSION_POS	3		/* Version first character in stamp. */

#define NO_SUCH_CODE		4098    /* Impossible code, to signal empty. */

#define RED		0
#define GREEN	1
#define BLUE	2

static int
	priv_BitsPerPixel,	    /* Bits per pixel (Codes uses at list this + 1). */
	priv_ClearCode,				       /* The CLEAR LZ code. */
	priv_EOFCode,				         /* The EOF LZ code. */
	priv_RunningCode,		    /* The next code algorithm can generate. */
	priv_RunningBits,/* The number of bits required to represent RunningCode. */
	priv_MaxCode1,  /* 1 bigger than max. possible code, in RunningBits bits. */
	priv_LastCode,		        /* The code before the current code. */
	priv_StackPtr,		         /* For character stack (see below). */
	priv_CrntShiftState;		        /* Number of bits in CrntShiftDWord. */
static	 unsigned long priv_CrntShiftDWord,     /* For bytes decomposition into codes. */
		  priv_PixelCount;		       /* Number of pixels in image. */
static	 FILE *priv_File;						  /* File as stream. */
static	 GifByteType priv_Buf[256];	       /* Compressed input is buffered here. */
static	 GifByteType priv_Stack[LZ_MAX_CODE];	 /* Decoded pixels are stacked here. */
static	 GifByteType priv_Suffix[LZ_MAX_CODE+1];	       /* So we can trace the codes. */
static	 unsigned int priv_Prefix[LZ_MAX_CODE+1];

static int DGifDecompressLine(GifPixelType *Line, int LineLen);
static int DGifGetPrefixChar(unsigned int *Prefix, int Code, int ClearCode);
static int DGifDecompressInput(int *Code);
static int DGifBufferedInput(FILE *File, GifByteType *Buf, unsigned char *NextByte);
static int DGifGetCodeNext(GifByteType **CodeBlock);

#ifndef __TURBOC__
int getw(FILE *fp)
{
    int temp;
    temp = getc(fp);
    temp = getc(fp) * 0x100 + temp;
    return temp;
}
#endif

/******************************************************************************
*   Open a new gif file for read, given by its name.			      *
*   Returns GifFileType pointer dynamically allocated which serves as the gif *
* info record. _GifError is cleared if succesfull.			      *
******************************************************************************/
FILE *DGifOpenFile(char *name)
{
	char Buf[GIF_STAMP_LEN+1];

	if ((priv_File = fopen(name, "rb")) == NULL) return NULL;

	/* Lets see if this is GIF file: */
	if (fread(Buf, 1, GIF_STAMP_LEN, priv_File) != GIF_STAMP_LEN) {
		return NULL;
	}

	/* The GIF Version number is ignored at this time. Maybe we should do    */
	/* something more useful with it.					     */
	Buf[GIF_STAMP_LEN] = 0;
	if (strncmp(GIF_STAMP, Buf, GIF_VERSION_POS) != 0) return NULL;
	return priv_File;
}

/******************************************************************************
*   This routine should be called before any other DGif calls. Note that      *
* this routine is called automatically from DGif file open routines.	      *
******************************************************************************/
void DGifGetScreenDesc(int *Width, int *Height, int *ColorRes, int *BackGround,
	int *BitsPerPixel, unsigned char *palette)
{
	int i, Size;
	char Buf;

	/* Put the screen descriptor into the file: */
	*Width = getw(priv_File);
	*Height = getw(priv_File);

	Buf = fgetc(priv_File);
	*ColorRes = (((Buf & 0x70) + 1) >> 4) + 1;
	*BitsPerPixel = (Buf & 0x07) + 1;
	*BackGround = fgetc(priv_File);
	fgetc(priv_File);
	if (Buf & 0x80) {		     /* Do we have global color map? */
		Size = (1 << *BitsPerPixel) * 3;
		/* Get the global color map: */
		for (i = 0; i < Size; i += 3) {
			Buf = getc(priv_File);
			palette[i+RED]   = Buf /*getc(priv_File)*/ >> (8 - *ColorRes);
			palette[i+GREEN] = getc(priv_File) >> (8 - *ColorRes);
			palette[i+BLUE]  = getc(priv_File) >> (8 - *ColorRes);
		}
	}
}

/******************************************************************************
*   This routine should be called before any attemp to read an image.         *
******************************************************************************/
int DGifGetRecordType(GifRecordType *Type)
{
    GifByteType Buf;

    if (fread(&Buf, 1, 1, priv_File) != 1) {
	return GIF_ERROR;
    }

    switch (Buf) {
	case ',':
	    *Type = IMAGE_DESC_RECORD_TYPE;
	    break;
	case '!':
	    *Type = EXTENSION_RECORD_TYPE;
	    break;
	case ';':
	    *Type = TERMINATE_RECORD_TYPE;
	    break;
	default:
	    *Type = UNDEFINED_RECORD_TYPE;
	    return GIF_ERROR;
    }

    return GIF_OK;
}

/******************************************************************************
*   This routine should be called before any attemp to read an image.         *
*   Note it is assumed the Image desc. header (',') has been read.	      *
******************************************************************************/
void 	DGifGetImageDesc(unsigned int *Left, unsigned int *Top, unsigned int *Width, unsigned int *Height, unsigned char *palette)
{
	int Size, i;
	int ColorRes, BitsPerPixel;
	char Buf;

	*Left   = getw(priv_File);
	*Top    = getw(priv_File);
	*Width  = getw(priv_File);
	*Height = getw(priv_File);
	Buf = fgetc(priv_File);
	ColorRes = (((Buf & 0x70) + 1) >> 4) + 1;
	BitsPerPixel = (Buf & 0x07) + 1;
	if (Buf & 0x80) {	    /* Does this image have local color map? */
		Size = (1 << BitsPerPixel) * 3;
		/* Get the image local color map: */
		for (i = 0; i < Size; i += 3) {
			palette[i+RED]   = getc(priv_File) >> (8 - ColorRes);
			palette[i+GREEN] = getc(priv_File) >> (8 - ColorRes);
			palette[i+BLUE]  = getc(priv_File) >> (8 - ColorRes);
		}
	}

	DGifSetupDecompress((long) *Width * (long) *Height);  /* Reset decompress algorithm parameters. */
}

/******************************************************************************
*  Get one full scanned line (Line) of length LineLen from GIF file.	      *
******************************************************************************/
int DGifGetLine(GifPixelType *Line, int LineLen)
{
	GifByteType *Dummy;

	if ((priv_PixelCount -= LineLen) < 0) 	return GIF_ERROR;

	if (DGifDecompressLine(Line, LineLen) != GIF_OK) return GIF_ERROR;
	if (priv_PixelCount == 0) {
		/* We probably would not be called any more, so lets clean 	     */
		/* everything before we return: need to flush out all rest of    */
		/* image until empty block (size 0) detected. We use GetCodeNext.*/
		do if (DGifGetCodeNext(&Dummy) == GIF_ERROR) return GIF_ERROR;
		while (Dummy != NULL);
	}
	return GIF_OK;
}

/******************************************************************************
*   This routine should be called last, to close GIF file.		      *
******************************************************************************/
void DGifCloseFile(void)
{
	 fclose(priv_File);
}

/******************************************************************************
*   Continue to get the image code in compressed form. This routine should be *
* called until NULL block is returned.					      *
*   The block should NOT be freed by the user (not dynamically allocated).    *
******************************************************************************/
static int DGifGetCodeNext(GifByteType **CodeBlock)
{
    GifByteType Buf;

	 if (fread(&Buf, 1, 1, priv_File) != 1) 	return GIF_ERROR;

    if (Buf > 0) {
	*CodeBlock = priv_Buf;	       /* Use private unused buffer. */
	(*CodeBlock)[0] = Buf;  /* Pascal strings notation (pos. 0 is len.). */
	if (fread(&((*CodeBlock)[1]), 1, Buf, priv_File) != Buf)	 return GIF_ERROR;
    }
    else {
	*CodeBlock = NULL;
	priv_Buf[0] = 0;		   /* Make sure the buffer is empty! */
	priv_PixelCount = 0;   /* And local info. indicate image read. */
    }

    return GIF_OK;
}

/******************************************************************************
*   Setup the LZ decompression for this image:				      *
******************************************************************************/
void DGifSetupDecompress(long size)
{
    int i, BitsPerPixel;
    GifByteType CodeSize;
    unsigned int *Prefix;

	priv_PixelCount = size;

    fread(&CodeSize, 1, 1, priv_File);    /* Read Code size from file. */
    BitsPerPixel = CodeSize;

    priv_Buf[0] = 0;			      /* Input Buffer empty. */
    priv_BitsPerPixel = BitsPerPixel;
    priv_ClearCode = (1 << BitsPerPixel);
    priv_EOFCode = priv_ClearCode + 1;
    priv_RunningCode = priv_EOFCode + 1;
    priv_RunningBits = BitsPerPixel + 1;	 /* Number of bits per code. */
    priv_MaxCode1 = 1 << priv_RunningBits;     /* Max. code + 1. */
    priv_StackPtr = 0;		    /* No pixels on the pixel stack. */
    priv_LastCode = NO_SUCH_CODE;
    priv_CrntShiftState = 0;	/* No information in CrntShiftDWord. */
    priv_CrntShiftDWord = 0;

    Prefix = priv_Prefix;
    for (i = 0; i < LZ_MAX_CODE; i++) Prefix[i] = NO_SUCH_CODE;
}

/******************************************************************************
*   The LZ decompression routine:					      *
*   This version decompress the given gif file into Line of length LineLen.   *
*   This routine can be called few times (one per scan line, for example), in *
* order the complete the whole image.					      *
******************************************************************************/
static int DGifDecompressLine(GifPixelType *Line, int LineLen)
{
    int i = 0, j, CrntCode, EOFCode, ClearCode, CrntPrefix, LastCode, StackPtr;
    GifByteType *Stack, *Suffix;
    unsigned int *Prefix;

    StackPtr = priv_StackPtr;
    Prefix = priv_Prefix;
    Suffix = priv_Suffix;
    Stack = priv_Stack;
    EOFCode = priv_EOFCode;
    ClearCode = priv_ClearCode;
    LastCode = priv_LastCode;

    if (StackPtr != 0) {
	/* Let pop the stack off before continueing to read the gif file: */
	while (StackPtr != 0 && i < LineLen) Line[i++] = Stack[--StackPtr];
    }

    while (i < LineLen) {			    /* Decode LineLen items. */
	if (DGifDecompressInput(&CrntCode) == GIF_ERROR)
    	    return GIF_ERROR;

	if (CrntCode == EOFCode) {
	    /* Note however that usually we will not be here as we will stop */
	    /* decoding as soon as we got all the pixel, or EOF code will    */
	    /* not be read at all, and DGifGetLine/Pixel clean everything.   */
	    if (i != LineLen - 1 || priv_PixelCount != 0) {
		return GIF_ERROR;
	    }
	    i++;
	}
	else if (CrntCode == ClearCode) {
	    /* We need to start over again: */
	    for (j = 0; j < LZ_MAX_CODE; j++) Prefix[j] = NO_SUCH_CODE;
	    priv_RunningCode = priv_EOFCode + 1;
	    priv_RunningBits = priv_BitsPerPixel + 1;
	    priv_MaxCode1 = 1 << priv_RunningBits;
	    LastCode = priv_LastCode = NO_SUCH_CODE;
	}
	else {
	    /* Its regular code - if in pixel range simply add it to output  */
	    /* stream, otherwise trace to codes linked list until the prefix */
	    /* is in pixel range:					     */
	    if (CrntCode < ClearCode) {
		/* This is simple - its pixel scalar, so add it to output:   */
		Line[i++] = CrntCode;
	    }
	    else {
		/* Its a code to needed to be traced: trace the linked list  */
		/* until the prefix is a pixel, while pushing the suffix     */
		/* pixels on our stack. If we done, pop the stack in reverse */
		/* (thats what stack is good for!) order to output.	     */
		if (Prefix[CrntCode] == NO_SUCH_CODE) {
		    /* Only allowed if CrntCode is exactly the running code: */
		    /* In that case CrntCode = XXXCode, CrntCode or the	     */
		    /* prefix code is last code and the suffix char is	     */
		    /* exactly the prefix of last code!			     */
		    if (CrntCode == priv_RunningCode - 2) {
			CrntPrefix = LastCode;
			Suffix[priv_RunningCode - 2] =
			Stack[StackPtr++] = DGifGetPrefixChar(Prefix,
							LastCode, ClearCode);
		    }
		    else {
			return GIF_ERROR;
		    }
		}
		else
		    CrntPrefix = CrntCode;

		/* Now (if image is O.K.) we should not get and NO_SUCH_CODE */
		/* During the trace. As we might loop forever, in case of    */
		/* defective image, we count the number of loops we trace    */
		/* and stop if we got LZ_MAX_CODE. obviously we can not      */
		/* loop more than that.					     */
		j = 0;
		while (j++ <= LZ_MAX_CODE &&
		       CrntPrefix > ClearCode &&
		       CrntPrefix <= LZ_MAX_CODE) {
		    Stack[StackPtr++] =	Suffix[CrntPrefix];
		    CrntPrefix = Prefix[CrntPrefix];
		}
		if (j >= LZ_MAX_CODE || CrntPrefix > LZ_MAX_CODE) {
		    return GIF_ERROR;
		}
		/* Push the last character on stack: */
		Stack[StackPtr++] = CrntPrefix;

		/* Now lets pop all the stack into output: */
		while (StackPtr != 0 && i < LineLen)
		    Line[i++] = Stack[--StackPtr];
	    }
	    if (LastCode != NO_SUCH_CODE) {
		Prefix[priv_RunningCode - 2] = LastCode;

		if (CrntCode == priv_RunningCode - 2) {
		    /* Only allowed if CrntCode is exactly the running code: */
		    /* In that case CrntCode = XXXCode, CrntCode or the	     */
		    /* prefix code is last code and the suffix char is	     */
		    /* exactly the prefix of last code!			     */
		    Suffix[priv_RunningCode - 2] =
			DGifGetPrefixChar(Prefix, LastCode, ClearCode);
		}
		else {
		    Suffix[priv_RunningCode - 2] =
			DGifGetPrefixChar(Prefix, CrntCode, ClearCode);
		}
	    }
	    LastCode = CrntCode;
	}
    }

    priv_LastCode = LastCode;
    priv_StackPtr = StackPtr;

    return GIF_OK;
}

/******************************************************************************
* Routine to trace the Prefixes linked list until we get a prefix which is    *
* not code, but a pixel value (less than ClearCode). Returns that pixel value.*
* If image is defective, we might loop here forever, so we limit the loops to *
* the maximum possible if image O.k. - LZ_MAX_CODE times.		      *
******************************************************************************/
static int DGifGetPrefixChar(unsigned int *Prefix, int Code, int ClearCode)
{
    int i = 0;

    while (Code > ClearCode && i++ <= LZ_MAX_CODE) Code = Prefix[Code];
    return Code;
}

/******************************************************************************
*   The LZ decompression input routine:					      *
*   This routine is responsable for the decompression of the bit stream from  *
* 8 bits (bytes) packets, into the real codes.				      *
*   Returns GIF_OK if read succesfully.					      *
******************************************************************************/
static int DGifDecompressInput(int *Code)
{
    GifByteType NextByte;
    static unsigned int CodeMasks[] = {
	0x0000, 0x0001, 0x0003, 0x0007,
	0x000f, 0x001f, 0x003f, 0x007f,
	0x00ff, 0x01ff, 0x03ff, 0x07ff,
	0x0fff
    };

    while (priv_CrntShiftState < priv_RunningBits) {
	/* Needs to get more bytes from input stream for next code: */
	if (DGifBufferedInput(priv_File, priv_Buf, &NextByte)
	    == GIF_ERROR) {
	    return GIF_ERROR;
	}
	priv_CrntShiftDWord |=
		((unsigned long) NextByte) << priv_CrntShiftState;
	priv_CrntShiftState += 8;
    }
	 *Code = (int) priv_CrntShiftDWord & CodeMasks[priv_RunningBits];

    priv_CrntShiftDWord >>= priv_RunningBits;
    priv_CrntShiftState -= priv_RunningBits;

    /* If code cannt fit into RunningBits bits, must raise its size. Note */
    /* however that codes above 4095 are used for special signaling.      */
    if (++priv_RunningCode > priv_MaxCode1 &&
	priv_RunningBits < LZ_BITS) {
	priv_MaxCode1 <<= 1;
	priv_RunningBits++;
    }
    return GIF_OK;
}

/******************************************************************************
*   This routines read one gif data block at a time and buffers it internally *
* so that the decompression routine could access it.			      *
*   The routine returns the next byte from its internal buffer (or read next  *
* block in if buffer empty) and returns GIF_OK if succesful.		      *
******************************************************************************/
static int DGifBufferedInput(FILE *File, GifByteType *Buf,
						      GifByteType *NextByte)
{
    if (Buf[0] == 0) {
	/* Needs to read the next buffer - this one is empty: */
	if (fread(Buf, 1, 1, File) != 1)
	{
	    return GIF_ERROR;
	}
	if (fread(&Buf[1], 1, Buf[0], File) != Buf[0])
	{
	    return GIF_ERROR;
	}
	*NextByte = Buf[1];
	Buf[1] = 2;	   /* We use now the second place as last char read! */
	Buf[0]--;
    }
    else {
	*NextByte = Buf[Buf[1]++];
	Buf[0]--;
    }

    return GIF_OK;
}
