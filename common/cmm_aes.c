/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	cmm_aes.c

	Abstract:

	Revision History:
	Who		When			What
	--------	----------		------------------------------
	Paul Wu		02-25-02		Initial
*/

#include "rt_config.h"


/*
	========================================================================

	Routine Description:
		Construct AAD of CCMP.

	Arguments:

	Return Value:

	Note:
		It's described in IEEE Std 802.11-2007.
		The AAD is constructed from the MPDU header.

	========================================================================
*/
static void RTMPConstructCCMPAAD(PUCHAR pHdr, BOOLEAN isDataFrame,
	UINT8 a4_exists, UINT8 qc_exists, UCHAR *aad_hdr, UINT *aad_len)
{
	UINT len = 0;

	/* Frame control -
		Subtype bits (bits 4 5 6) in a Data MPDU masked to 0
		Retry bit (bit 11) masked to 0
		PwrMgt bit (bit 12) masked to 0
		MoreData bit (bit 13) masked to 0
		Protected Frame bit (bit 14) always set to 1 */
	if (isDataFrame)
		aad_hdr[0] = (*pHdr) & 0x8f;
	else
		aad_hdr[0] = (*pHdr);
	aad_hdr[1] = (*(pHdr + 1)) & 0xc7;
	aad_hdr[1] = aad_hdr[1] | 0x40;
	len = 2;

	/* Append Addr 1, 2 & 3 */
	NdisMoveMemory(&aad_hdr[len], pHdr + 4, 3 * MAC_ADDR_LEN);
	len += (3 * MAC_ADDR_LEN);

	/*  SC -
		MPDU Sequence Control field, with the Sequence Number
		subfield (bits 4-15 of the Sequence Control field)
		masked to 0. The Fragment Number subfield is not modified. */
	aad_hdr[len] = (*(pHdr + 22)) & 0x0f;
	aad_hdr[len + 1] = 0x00;
	len += 2;

	/* Append the Addr4 field if present. */
	if (a4_exists) {
		NdisMoveMemory(&aad_hdr[len], pHdr + 24, MAC_ADDR_LEN);
		len += MAC_ADDR_LEN;
	}

	/*  QC -
		QoS Control field, if present, a 2-octet field that includes
		the MSDU priority. The QC TID field is used in the
		construction of the AAD and the remaining QC fields are
		set to 0 for the AAD calculation (bits 4 to 15 are set to 0). */
	if (qc_exists & a4_exists) {
		aad_hdr[len] = (*(pHdr + 30)) & 0x0f;   /* Qos_TC */
		aad_hdr[len + 1] = 0x00;
		len += 2;
	} else if (qc_exists & !a4_exists) {
		aad_hdr[len] = (*(pHdr + 24)) & 0x0f;   /* Qos_TC */
		aad_hdr[len + 1] = 0x00;
		len += 2;
	}

	*aad_len = len;
}

/*
	========================================================================

	Routine Description:
		Construct NONCE header of CCMP.

	Arguments:

	Return Value:

	Note:

	========================================================================
*/
static void RTMPConstructCCMPNonce(PUCHAR pHdr, UINT8 a4_exists, UINT8 qc_exists,
	BOOLEAN isMgmtFrame, UCHAR *pn, UCHAR *nonce_hdr, UINT *nonce_hdr_len)
{
	UINT n_offset = 0;
	int i;

	/* Decide the Priority Octet
		The Priority sub-field of the Nonce Flags field shall
		be set to the fixed value 0 when there is no QC field
		present in the MPDU header. When the QC field is present,
		bits 0 to 3 of the Priority field shall be set to the
		value of the QC TID (bits 0 to 3 of the QC field).*/
	if (qc_exists && a4_exists)
		nonce_hdr[0] = (*(pHdr + 30)) & 0x0f;
	if (qc_exists && !a4_exists)
		nonce_hdr[0] = (*(pHdr + 24)) & 0x0f;

#ifdef DOT11W_PMF_SUPPORT
	/* When Management Frame Protection is negotiated, the Management
		field of the Nonce Flags field shall be set to 1 if the Type
		field of the Frame Control field is 00 (Management frame); otherwise it
		is set to 0.  */
	if (isMgmtFrame)
		nonce_hdr[0] = nonce_hdr[0] | 0x10;
#endif /* DOT11W_PMF_SUPPORT */
	n_offset += 1;

	/* Fill in MPDU Address A2 field */
	NdisMoveMemory(&nonce_hdr[n_offset], pHdr + 10, MAC_ADDR_LEN);
	n_offset += MAC_ADDR_LEN;

	/* Fill in the PN. The PN field occupies octets 7¡V12.
		The octets of PN shall be ordered so that PN0 is at octet index 12
		and PN5 is at octet index 7. */
 	for (i = 0; i < 6; i++)
		nonce_hdr[n_offset + i] = pn[5 - i];
	n_offset += LEN_PN;

	*nonce_hdr_len = n_offset;

}

/*
	========================================================================

	Routine Description:
		Construct CCMP header.

	Arguments:

	Return Value:

	Note:
		It's a 8-octets header.

	========================================================================
*/
void RTMPConstructCCMPHdr(UINT8 key_idx, UCHAR *pn, UCHAR *ccmp_hdr)
{
	NdisZeroMemory(ccmp_hdr, LEN_CCMP_HDR);
	ccmp_hdr[0] = pn[0];
	ccmp_hdr[1] = pn[1];
	ccmp_hdr[3] = (key_idx <<6) | 0x20;
	ccmp_hdr[4] = pn[2];
	ccmp_hdr[5] = pn[3];
	ccmp_hdr[6] = pn[4];
	ccmp_hdr[7] = pn[5];
}

BOOLEAN RTMPSoftEncryptCCMP(PRTMP_ADAPTER pAd, PUCHAR pHdr, PUCHAR pIV,
	PUCHAR pKey, PUCHAR pData, UINT32 DataLen)
{
	UINT8 frame_type, frame_subtype;
	UINT8 from_ds, to_ds;
	UINT8 a4_exists, qc_exists;
	UINT8 aad_hdr[30];
	UINT aad_len = 0;
	UINT8 nonce_hdr[13];
	UINT32 nonce_hdr_len = 0;
	UINT32 out_len = DataLen + 8;

#ifdef RT_BIG_ENDIAN
	RTMPFrameEndianChange(pAd, (PUCHAR)pHdr, DIR_READ, FALSE);
#endif
	/* Initial variable */
	NdisZeroMemory(aad_hdr, 30);
	NdisZeroMemory(nonce_hdr, 13);

	/* Indicate type and subtype of Frame Control field */
	frame_type = (((*pHdr) >> 2) & 0x03);
	frame_subtype = (((*pHdr) >> 4) & 0x0f);

	/* Indicate the fromDS and ToDS */
	from_ds = ((*(pHdr + 1)) & 0x2) >> 1;
	to_ds = ((*(pHdr + 1)) & 0x1);

	/* decide if the Address 4 exist or QoS exist */
	a4_exists = (from_ds & to_ds);
	qc_exists = 0;
	if (frame_type == FC_TYPE_DATA) {
	qc_exists = ((frame_subtype == SUBTYPE_QDATA) ||
			(frame_subtype == SUBTYPE_QDATA_CFACK) ||
			(frame_subtype == SUBTYPE_QDATA_CFPOLL) ||
			(frame_subtype == SUBTYPE_QDATA_CFACK_CFPOLL));
	}

	/* Construct AAD header */
	RTMPConstructCCMPAAD(pHdr, (frame_type == FC_TYPE_DATA), a4_exists,
			qc_exists, aad_hdr, &aad_len);

	/* Construct NONCE header */
	RTMPConstructCCMPNonce(pHdr, a4_exists, qc_exists,
			(frame_type == FC_TYPE_MGMT), pIV, nonce_hdr,
			&nonce_hdr_len);

	/* CCM originator processing -
	   Use the temporal key, AAD, nonce, and MPDU data to
	   form the cipher text and MIC. */
	if (AES_CCM_Encrypt(pData, DataLen, pKey, 16, nonce_hdr, nonce_hdr_len,
			aad_hdr, aad_len, LEN_CCMP_MIC, pData, &out_len))
		return FALSE;

#ifdef RT_BIG_ENDIAN
	RTMPFrameEndianChange(pAd, (PUCHAR)pHdr, DIR_READ, FALSE);
#endif
	return TRUE;
}

/*
	========================================================================

	Routine Description:
		Decrypt data with CCMP.

	Arguments:

	Return Value:

	Note:

	========================================================================
*/
BOOLEAN RTMPSoftDecryptCCMP(PRTMP_ADAPTER pAd, PUCHAR pHdr, PCIPHER_KEY pKey,
	PUCHAR pData, UINT16 *DataLen)
{
	UINT8 frame_type, frame_subtype;
	UINT8 from_ds, to_ds;
	UINT8 a4_exists, qc_exists;
	UINT8 aad_hdr[30];
	UINT aad_len = 0;
	UINT8 pn[LEN_PN];
	PUCHAR cipherData_ptr;
	UINT32 cipherData_len;
	UINT8 nonce_hdr[13];
	UINT32 nonce_hdr_len = 0;
	UINT32 out_len = *DataLen;

#ifdef RT_BIG_ENDIAN
	RTMPFrameEndianChange(pAd, (PUCHAR)pHdr, DIR_READ, FALSE);
#endif
	/* Check the key is valid */
	if (pKey->KeyLen == 0) {
		DBGPRINT(RT_DEBUG_ERROR, ("%s : The key is not available !\n",
				__FUNCTION__));
		return FALSE;
	}

	/* Initial variable */
	NdisZeroMemory(aad_hdr, 30);
	NdisZeroMemory(nonce_hdr, 13);

	/* Indicate type and subtype of Frame Control field */
	frame_type = (((*pHdr) >> 2) & 0x03);
	frame_subtype = (((*pHdr) >> 4) & 0x0f);

	/* Indicate the fromDS and ToDS */
	from_ds = ((*(pHdr + 1)) & 0x2) >> 1;
	to_ds = ((*(pHdr + 1)) & 0x1);

	/* decide if the Address 4 exist or QoS exist */
	a4_exists = (from_ds & to_ds);
	qc_exists = 0;
	if (frame_type == FC_TYPE_DATA) {
	qc_exists = ((frame_subtype == SUBTYPE_QDATA) ||
			(frame_subtype == SUBTYPE_QDATA_CFACK) ||
			(frame_subtype == SUBTYPE_QDATA_CFPOLL) ||
			(frame_subtype == SUBTYPE_QDATA_CFACK_CFPOLL));
	}

	/* Extract PN and from CCMP header */
	pn[0] =	pData[0];
	pn[1] = pData[1];
	pn[2] = pData[4];
	pn[3] = pData[5];
	pn[4] = pData[6];
	pn[5] = pData[7];

	/* skip ccmp header */
	cipherData_ptr = pData + LEN_CCMP_HDR;
	cipherData_len = *DataLen - LEN_CCMP_HDR;

	/*skip payload length is zero*/
	if ((*DataLen ) <= LEN_CCMP_HDR)
		return FALSE;

	/* Construct AAD header */
	RTMPConstructCCMPAAD(pHdr, (frame_type == FC_TYPE_DATA), a4_exists,
			qc_exists, aad_hdr, &aad_len);

	/* Construct NONCE header */
	RTMPConstructCCMPNonce(pHdr, a4_exists, qc_exists,
			(frame_type == FC_TYPE_MGMT), pn, nonce_hdr,
			&nonce_hdr_len);

	/* CCM recipient processing -
	   uses the temporal key, AAD, nonce, MIC,
	   and MPDU cipher text data */
	if (AES_CCM_Decrypt(cipherData_ptr, cipherData_len, pKey->Key, 16,
			nonce_hdr, nonce_hdr_len, aad_hdr, aad_len,
			LEN_CCMP_MIC, pData, &out_len))
		return FALSE;

	*DataLen = out_len;

#ifdef RT_BIG_ENDIAN
	RTMPFrameEndianChange(pAd, (PUCHAR)pHdr, DIR_READ, FALSE);
#endif
	return TRUE;
}


