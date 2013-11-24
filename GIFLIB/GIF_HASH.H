/************************************************************************
 *                                                                      *
 *                  Copyright (c) 1991, Frank van der Hulst             *
 *                          All Rights Reserved                         *
 *                                                                      *
 * Authors:                                                             *
 *		  FvdH - Frank van der Hulst (Wellington, NZ)                     *
 *                                                                      *
 * Versions:                                                            *
 *      V1.1 910626 FvdH - QUANT released for DBW_RENDER                *
 *      V1.2 911021 FvdH - QUANT released for PoV Ray                   *
 *                                                                      *
 ************************************************************************/
/************************************************************************
* Declarations, global to other of the GIF-HASH.C module.		      	*
*									      													*
*					Written by Gershon Elber,  Jun 1989   							*
*************************************************************************
* History:								      											*
* 14 Jun 89 - Version 1.0 by Gershon Elber.				      				*
*************************************************************************/

#ifdef __GNUC__
#define cdecl
#endif

#define ZL_MAX_CODE 4095	    /* Biggest code possible in 12 bits */
#define HT_SIZE 8192                /* 12 bits = 2 * 4096 */

void HashTable_Clear(unsigned long *HashTable);
void HashTable_Insert(unsigned long *HashTable, unsigned long Key, int Code);
int  HashTable_Exists(unsigned long *HashTable, unsigned long Key);

#define HT_KEY_MASK		0x1FFF			      /* 13bits keys */
#define HT_KEY_NUM_BITS		13			      /* 13bits keys */
#define HT_MAX_KEY		8191	/* 13bits - 1, maximal code possible */
#define HT_MAX_CODE		4095	/* Biggest code possible in 12 bits. */

/* The 32 bits of the long are divided into two parts for the key & code:   */
/* 1. The code is 12 bits as our compression algorithm is limited to 12bits */
/* 2. The key is 12 bits Prefix code + 8 bit new char or 20 bits.	    */
#define HT_GET_KEY(l)	(l >> 12)
#define HT_GET_CODE(l)	(l & 0x0FFF)
#define HT_PUT_KEY(l)	(l << 12)
#define HT_PUT_CODE(l)	(l & 0x0FFF)
