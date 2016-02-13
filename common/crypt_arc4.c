/****************************************************************************
 * Ralink Tech Inc.
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************/


/****************************************************************************
	Module Name:
	crypt_arc4

	Revision History:
	Who         When            What
	--------    ----------      ------------------------------------------
	Eddy        2009/05/13      ARC4
***************************************************************************/

#include "crypt_arc4.h"


/*
========================================================================
Routine Description:
	ARC4 initialize the key block

Arguments:
	pARC4_CTX	Pointer to ARC4 CONTEXT
	Key		Cipher key, it may be 16, 24,
			or 32 bytes (128, 192, or 256 bits)
	KeyLength	The length of cipher key in bytes

========================================================================
*/
void ARC4_INIT(ARC4_CTX_STRUC *pARC4_CTX, PUCHAR pKey, UINT KeyLength)
{
	UINT BlockIndex, SWAPIndex, KeyIndex;
	UINT8 TempValue;

	/*Initialize the block value*/
	pARC4_CTX->BlockIndex1 = 0;
	pARC4_CTX->BlockIndex2 = 0;
	for (BlockIndex = 0; BlockIndex < ARC4_KEY_BLOCK_SIZE; BlockIndex++)
		pARC4_CTX->KeyBlock[BlockIndex] = (UINT8) BlockIndex;

	/*Key schedule*/
	SWAPIndex = 0;
	for (BlockIndex = 0; BlockIndex < ARC4_KEY_BLOCK_SIZE; BlockIndex++) {
		TempValue = pARC4_CTX->KeyBlock[BlockIndex];
		KeyIndex = BlockIndex % KeyLength;
		SWAPIndex = (SWAPIndex + TempValue + pKey[KeyIndex]) & 0xff;
		pARC4_CTX->KeyBlock[BlockIndex] = pARC4_CTX->KeyBlock[SWAPIndex];
		pARC4_CTX->KeyBlock[SWAPIndex] = TempValue;
	}
}


/*
========================================================================
Routine Description:
	ARC4 encryption/decryption

Arguments:
	pARC4_CTX	Pointer to ARC4 CONTEXT
	InputText	Input text
	InputTextLength	The length of input text in bytes

Return Value:
	OutputBlock	Return output text
 ========================================================================
*/
void ARC4_Compute(ARC4_CTX_STRUC *pARC4_CTX, UINT8 InputBlock[],
	UINT InputBlockSize, UINT8 OutputBlock[])
{
	UINT InputIndex;
	UINT8 TempValue;

	for (InputIndex = 0; InputIndex < InputBlockSize; InputIndex++) {
		pARC4_CTX->BlockIndex1 = (pARC4_CTX->BlockIndex1 + 1) & 0xff;
		TempValue = pARC4_CTX->KeyBlock[pARC4_CTX->BlockIndex1];
		pARC4_CTX->BlockIndex2 = (pARC4_CTX->BlockIndex2 + TempValue) & 0xff;

		pARC4_CTX->KeyBlock[pARC4_CTX->BlockIndex1] = 
				pARC4_CTX->KeyBlock[pARC4_CTX->BlockIndex2];
		pARC4_CTX->KeyBlock[pARC4_CTX->BlockIndex2] = TempValue;

		TempValue = (TempValue + 
				pARC4_CTX->KeyBlock[pARC4_CTX->BlockIndex1]) & 0xff;
		OutputBlock[InputIndex] = 
				InputBlock[InputIndex]^pARC4_CTX->KeyBlock[TempValue];
	}
}


/*
========================================================================
Routine Description:
	Discard the key length

Arguments:
	pARC4_CTX	Pointer to ARC4 CONTEXT
	Length		Discard the key length

========================================================================
*/
void ARC4_Discard_KeyLength (ARC4_CTX_STRUC *pARC4_CTX, UINT Length)
{
	UINT Index;
	UINT8 TempValue;

	for (Index = 0; Index < Length; Index++) {
		pARC4_CTX->BlockIndex1 = (pARC4_CTX->BlockIndex1 + 1) & 0xff;
		TempValue = pARC4_CTX->KeyBlock[pARC4_CTX->BlockIndex1];
		pARC4_CTX->BlockIndex2 = (pARC4_CTX->BlockIndex2 + TempValue) & 0xff;

		pARC4_CTX->KeyBlock[pARC4_CTX->BlockIndex1] = 
				pARC4_CTX->KeyBlock[pARC4_CTX->BlockIndex2];
		pARC4_CTX->KeyBlock[pARC4_CTX->BlockIndex2] = TempValue;
	}
}


