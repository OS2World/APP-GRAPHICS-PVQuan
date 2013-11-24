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
*   "Gif-Lib" - Yet another gif library.				     						*
*									     														*
* Written by:  Gershon Elber			IBM PC Ver 0.1,	Jun. 1989    		*
*************************************************************************
* Module to support the following operations:				     				*
*									     														*
* 1. InitHashTable - initialize hash table.				     					*
* 2. ClearHashTable - clear the hash table to an empty state.		     	*
* 2. InsertHashTable - insert one item into data structure.		     		*
* 3. ExistsHashTable - test if item exists in data structure.		     	*
*									     														*
* This module is used to hash the GIF codes during encoding.		     	*
*************************************************************************
* History:								     												*
* 14 Jun 89 - Version 1.0 by Gershon Elber.				     					*
*************************************************************************/

#ifdef __TURBOC__
#include <mem.h>
#endif

#include "gif_hash.h"

#define HT_KEY_MASK		0x1FFF			      /* 13bits keys */

/* The 32 bits of the long are divided into two parts for the key & code:   */
/* 1. The code is 12 bits as our compression algorithm is limited to 12bits */
/* 2. The key is 12 bits Prefix code + 8 bit new char or 20 bits.	    */
#define HT_GET_KEY(l)	(l >> 12)
#define HT_GET_CODE(l)	(l & 0x0FFF)
#define HT_PUT_KEY(l)	(l << 12)
#define HT_PUT_CODE(l)	(l & 0x0FFF)

/******************************************************************************
* Routine to generate an HKey for the hashtable out of the given unique key.  *
* The given Key is assumed to be 20 bits as follows: lower 8 bits are the     *
* new postfix character, while the upper 12 bits are the prefix code.	      *
* Because the average hit ratio is only 2 (2 hash references per entry),      *
* evaluating more complex keys (such as twin prime keys) does not worth it!   *
******************************************************************************/
static int KeyItem(unsigned long Item)
{
	 return ((int)((Item >> 12) ^ Item)) & HT_KEY_MASK;
}


/******************************************************************************
* Routine to clear the HashTable to an empty state.			      *
* This part is a little machine depended. Use the commented part otherwise.   *
******************************************************************************/
void HashTable_Clear(unsigned long *HashTable)
{
	 memset(HashTable, 0xFF, HT_SIZE * sizeof(long));
}

/******************************************************************************
* Routine to insert a new Item into the HashTable. The data is assumed to be  *
* new one.								      *
******************************************************************************/
void HashTable_Insert(unsigned long *HashTable, unsigned long Key, int Code)
{
int HKey = KeyItem(Key);
unsigned long *HTable = HashTable;

	while (HT_GET_KEY(HTable[HKey]) != 0xFFFFFL) {
		HKey = (HKey + 1) & HT_KEY_MASK;
	}
	HTable[HKey] = HT_PUT_KEY(Key) | HT_PUT_CODE(Code);
}

/******************************************************************************
* Routine to test if given Key exists in HashTable and if so returns its code *
* Returns the Code if key was found, -1 if not.				      *
******************************************************************************/
int HashTable_Exists(unsigned long *HashTable, unsigned long Key)
{
int HKey = KeyItem(Key);
unsigned long *HTable = HashTable, HTKey;

	while ((HTKey = HT_GET_KEY(HTable[HKey])) != 0xFFFFFL) {
		if (Key == HTKey) return (int)HT_GET_CODE(HTable[HKey]);
		HKey = (HKey + 1) & HT_KEY_MASK;
	}

    return -1;
}
