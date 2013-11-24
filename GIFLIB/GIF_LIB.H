/******************************************************************************
* In order to make life a little bit easier when using the GIF file format,   *
* this library was written, and which does all the dirty work...	      *
*									      *
*					Written by Gershon Elber,  Jun. 1989  *
*******************************************************************************
* History:								      *
* 14 Jun 89 - Version 1.0 by Gershon Elber.				      *
*  3 Sep 90 - Version 1.1 by Gershon Elber (Support for Gif89, Unique names). *
******************************************************************************/

#ifndef GIF_LIB_H
#define GIF_LIB_H


#include <stdio.h>
#define GIF_LIB_VERSION	" Version 1.1, "

#define	GIF_ERROR	0
#define GIF_OK		1

#ifndef TRUE
#define TRUE		1
#define FALSE		0
#endif

typedef	int		GifBooleanType;
typedef	unsigned char	GifPixelType;
typedef unsigned char *	GifRowType;
typedef unsigned char	GifByteType;

typedef enum {
    UNDEFINED_RECORD_TYPE,
    SCREEN_DESC_RECORD_TYPE,
	 IMAGE_DESC_RECORD_TYPE,				/* Begin with ',' */
    EXTENSION_RECORD_TYPE,				   /* Begin with '!' */
    TERMINATE_RECORD_TYPE				   /* Begin with ';' */
} GifRecordType;

/******************************************************************************
* O.k. here are the routines one can access in order to decode GIF file:      *
* (GIF_LIB file DGIF_LIB.C).						      *
******************************************************************************/

FILE *    DGifOpenFile(char *GifFileName);
void      DGifGetScreenDesc(int *Width, int *Height, int *ColorRes, int *BackGround,
				int *BitsPerPixel, unsigned char *palette);
int       DGifGetRecordType(GifRecordType *GifType);
void      DGifGetImageDesc(unsigned int *Left, unsigned int *Top, unsigned int *Width, unsigned int *Height, unsigned char *palette);
void      DGifSetupDecompress(long size);
int       DGifGetLine(GifPixelType *GifLine, int GifLineLen);
void      DGifCloseFile(void);

int       EGifOpenFileName(char *FileName);
void      EGifPutScreenDesc(int Width, int Height, int ColorRes, int BackGround,
				int BitsPerPixel, unsigned char ColorMap[][3]);
void      EGifPutImageDesc(int Left, int Top, int Width, int Height, int BitsPerPixel);
int       EGifPutLine(unsigned char *Line, int LineLen);
void      EGifCloseFile(void);

#endif /* GIF_LIB_H */
