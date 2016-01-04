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
	wpa.h

*/

#ifndef __WPA_H__
#define __WPA_H__

#ifndef ROUND_UP
#define ROUND_UP(__x, __y) \
	(((ULONG)((__x)+((__y)-1))) & ((ULONG)~((__y)-1)))
#endif

#define SET_UINT16_TO_ARRARY(_V, _LEN)		\
{						\
	_V[0] = ((UINT16)_LEN) >> 8;		\
	_V[1] = ((UINT16)_LEN & 0xFF);		\
}

#define INC_UINT16_TO_ARRARY(_V, _LEN)		\
{						\
	UINT16	var_len;			\
						\
	var_len = (_V[0]<<8) | (_V[1]);		\
	var_len += _LEN;			\
						\
	_V[0] = (var_len & 0xFF00) >> 8;	\
	_V[1] = (var_len & 0xFF);		\
}

#define CONV_ARRARY_TO_UINT16(_V)	((_V[0]<<8) | (_V[1]))

#define ADD_ONE_To_64BIT_VAR(_V)		\
{						\
	UCHAR cnt = LEN_KEY_DESC_REPLAY;	\
	do					\
	{					\
		cnt--;				\
		_V[cnt]++;			\
		if (cnt == 0)			\
			break;			\
	}while (_V[cnt] == 0);			\
}

#define INC_TX_TSC(_tsc, _cnt)			\
{						\
	INT i = 0;				\
	while (++_tsc[i] == 0x0) {		\
		i++;				\
		if (i == (_cnt))		\
			break;			\
	}					\
}

#define IS_WPA_CAPABILITY(a)	(((a) >= Ndis802_11AuthModeWPA) && ((a) <= Ndis802_11AuthModeWPA1PSKWPA2PSK))

/*
	WFA recommend to restrict the encryption type in 11n-HT mode.
 	So, the WEP and TKIP shall not be allowed to use HT rate.
 */
#define IS_INVALID_HT_SECURITY(_mode)			\
	(((_mode) == Ndis802_11Encryption1Enabled) || 	\
	 ((_mode) == Ndis802_11Encryption2Enabled))

#define MIX_CIPHER_WPA_TKIP_ON(x)       (((x) & 0x08) != 0)
#define MIX_CIPHER_WPA_AES_ON(x)        (((x) & 0x04) != 0)
#define MIX_CIPHER_WPA2_TKIP_ON(x)      (((x) & 0x02) != 0)
#define MIX_CIPHER_WPA2_AES_ON(x)       (((x) & 0x01) != 0)

#ifdef CONFIG_AP_SUPPORT
/*========================================
	The prototype is defined in ap_wpa.c
  ========================================*/
VOID WPA_APSetGroupRekeyAction(
	IN PRTMP_ADAPTER pAd);

#endif /* CONFIG_AP_SUPPORT */

/*========================================
	The prototype is defined in cmm_wpa.c
  ========================================*/
void inc_iv_byte(
	UCHAR *iv,
	UINT len,
	UINT cnt);

BOOLEAN WpaMsgTypeSubst(
	IN UCHAR EAPType,
	OUT INT *MsgType);

int RtmpPasswordHash(
	char *password,
	unsigned char *ssid,
	int ssidlength,
	unsigned char *output);

VOID KDF(
	IN PUINT8 key,
	IN INT key_len,
	IN PUINT8 label,
	IN INT label_len,
	IN PUINT8 data,
	IN INT data_len,
	OUT PUINT8 output,
	IN USHORT len);

PUINT8 WPA_ExtractSuiteFromRSNIE(
	IN PUINT8 rsnie,
	IN UINT rsnie_len,
	IN UINT8 type,
	OUT UINT8 *count);

/*
 =====================================
 	function prototype in cmm_wpa.c
 =====================================
*/
VOID WpaStateMachineInit(
    IN  PRTMP_ADAPTER   pAd,
    IN  STATE_MACHINE *Sm,
    OUT STATE_MACHINE_FUNC Trans[]);

VOID RTMPToWirelessSta(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry,
	IN PUCHAR pHeader802_3,
	IN UINT HdrLen,
	IN PUCHAR pData,
	IN UINT DataLen,
	IN BOOLEAN bClearFrame);

VOID WpaDeriveGTK(
	IN UCHAR *PMK,
	IN UCHAR *GNonce,
	IN UCHAR *AA,
	OUT UCHAR *output,
	IN UINT len);

VOID GenRandom(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR *macAddr,
	OUT UCHAR *random);

BOOLEAN RTMPCheckWPAframe(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry,
	IN PUCHAR pData,
	IN ULONG DataByteCount,
	IN UCHAR FromWhichBSSID);

#ifdef HDR_TRANS_SUPPORT
BOOLEAN RTMPCheckWPAframe_Hdr_Trns(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry,
	IN PUCHAR pData,
	IN ULONG DataByteCount,
	IN UCHAR FromWhichBSSID);
#endif /* HDR_TRANS_SUPPORT */

VOID WPA_ConstructKdeHdr(
	IN UINT8 data_type,
	IN UINT8 data_len,
	OUT PUCHAR pBuf);

PCIPHER_KEY RTMPSwCipherKeySelection(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pIV,
	IN RX_BLK *pRxBlk,
	IN PMAC_TABLE_ENTRY pEntry);

NDIS_STATUS RTMPSoftDecryptionAction(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pHdr,
	IN UCHAR UserPriority,
	IN PCIPHER_KEY pKey,
	INOUT PUCHAR pData,
	INOUT UINT16 *DataByteCnt);

VOID RTMPSoftConstructIVHdr(
	IN UCHAR CipherAlg,
	IN UCHAR key_id,
	IN PUCHAR pTxIv,
	OUT PUCHAR pHdrIv,
	OUT UINT8 *hdr_iv_len);

VOID RTMPSoftEncryptionAction(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR CipherAlg,
	IN PUCHAR pHdr,
	IN PUCHAR pSrcBufData,
	IN UINT32 SrcBufLen,
	IN UCHAR KeyIdx,
	IN PCIPHER_KEY pKey,
	OUT UINT8 *ext_len);

VOID RTMPMakeRSNIE(
	IN PRTMP_ADAPTER pAd,
	IN UINT AuthMode,
	IN UINT WepStatus,
	IN UCHAR apidx);

VOID WPAInstallPairwiseKey(
	PRTMP_ADAPTER pAd,
	UINT8 BssIdx,
	PMAC_TABLE_ENTRY pEntry,
	BOOLEAN bAE);

VOID WPAInstallSharedKey(
	PRTMP_ADAPTER pAd,
	UINT8 GroupCipher,
	UINT8 BssIdx,
	UINT8 KeyIdx,
	UINT8 Wcid,
	BOOLEAN bAE,
	PUINT8 pGtk,
	UINT8 GtkLen);

VOID RTMPSetWcidSecurityInfo(
	PRTMP_ADAPTER pAd,
	UINT8 BssIdx,
	UINT8 KeyIdx,
	UINT8 CipherAlg,
	UINT8 Wcid,
	UINT8 KeyTabFlag);

VOID CalculateMIC(
	IN UCHAR KeyDescVer,
	IN UCHAR *PTK,
	OUT PEAPOL_PACKET pMsg);

BOOLEAN rtmp_chk_tkip_mic(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, RX_BLK *pRxBlk);

#ifdef WPA_SUPPLICANT_SUPPORT
INT WpaCheckEapCode(
	IN  RTMP_ADAPTER *pAd,
	IN  UCHAR *pFrame,
	IN  USHORT FrameLen,
	IN  USHORT OffSet);
#endif /* WPA_SUPPLICANT_SUPPORT */

/*
 =====================================
 	function prototype in cmm_wep.c
 =====================================
*/
UINT RTMP_CALC_FCS32(
	IN UINT Fcs,
	IN PUCHAR Cp,
	IN INT Len);

VOID RTMPConstructWEPIVHdr(
	IN UINT8 key_idx,
	IN UCHAR *pn,
	OUT UCHAR *iv_hdr);

BOOLEAN RTMPSoftEncryptWEP(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pIvHdr,
	IN PCIPHER_KEY pKey,
	INOUT PUCHAR pData,
	IN ULONG DataByteCnt);

BOOLEAN RTMPSoftDecryptWEP(
	IN PRTMP_ADAPTER pAd,
	IN PCIPHER_KEY pKey,
	INOUT PUCHAR pData,
	INOUT UINT16 *DataByteCnt);

/*
 =====================================
 	function prototype in cmm_tkip.c
 =====================================
*/
BOOLEAN RTMPSoftDecryptTKIP(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pHdr,
	IN UCHAR UserPriority,
	IN PCIPHER_KEY pKey,
	INOUT PUCHAR pData,
	IN UINT16 *DataByteCnt);

VOID TKIP_GTK_KEY_WRAP(
	IN UCHAR *key,
	IN UCHAR *iv,
	IN UCHAR *input_text,
	IN UINT32 input_len,
	OUT UCHAR *output_text);

VOID TKIP_GTK_KEY_UNWRAP(
	IN UCHAR *key,
	IN UCHAR *iv,
	IN UCHAR *input_text,
	IN UINT32 input_len,
	OUT UCHAR *output_text);

/*
 =====================================
 	function prototype in cmm_aes.c
 =====================================
*/
BOOLEAN RTMPSoftDecryptAES(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pData,
	IN ULONG DataByteCnt,
	IN PCIPHER_KEY pWpaKey);

VOID RTMPConstructCCMPHdr(
	IN UINT8 key_idx,
	IN UCHAR *pn,
	OUT UCHAR *ccmp_hdr);

BOOLEAN RTMPSoftEncryptCCMP(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pHdr,
	IN PUCHAR pIV,
	IN PUCHAR pKey,
	INOUT PUCHAR pData,
	IN UINT32 DataLen);

BOOLEAN RTMPSoftDecryptCCMP(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pHdr,
	IN PCIPHER_KEY pKey,
	INOUT PUCHAR pData,
	INOUT UINT16 *DataLen);

VOID CCMP_test_vector(
	IN PRTMP_ADAPTER pAd,
	IN INT input);

void inc_byte_array(UCHAR *counter, int len);

#endif
