/* aatypes.h  Copyright 1990 Dancing Flame, San Francisco */

#ifndef AATYPES_H
#define AATYPES_H

#define UBYTE unsigned char
#define BYTE signed char
#define SHORT short
#define USHORT unsigned short
#define LONG long
#define ULONG unsigned long

typedef void (*AAvec)();	/* pointer to a void-valued function */
typedef int (*AAivec)();	/* pointer to an integer valued function */

typedef int Boolean;	/* functions explicitly returning 0 or 1 */
#define TRUE 1
#define FALSE 0
#define Success 0		/* for Errval returns */

/* useful macro to count elements in an array */
#define Array_els(arr) (sizeof(arr)/sizeof(arr[0]))

#endif /* AATYPES_H */
