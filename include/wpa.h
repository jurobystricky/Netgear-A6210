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
	int i = 0;				\
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
void WPA_APSetGroupRekeyAction(PRTMP_ADAPTER pAd);

#endif /* CONFIG_AP_SUPPORT */

/*========================================
	The prototype is defined in cmm_wpa.c
  ========================================*/
void inc_iv_byte(UCHAR *iv, UINT len, UINT cnt);
void inc_byte_array(UCHAR *counter, int len);

BOOLEAN WpaMsgTypeSubst(UCHAR EAPType, int *MsgType);

int RtmpPasswordHash(char *password, unsigned char *ssid, int ssidlength,
	unsigned char *output);

void KDF(
	PUINT8 key,
	int key_len,
	PUINT8 label,
	int label_len,
	PUINT8 data,
	int data_len,
	PUINT8 output,
	USHORT len);

PUINT8 WPA_ExtractSuiteFromRSNIE(
	PUINT8 rsnie,
	UINT rsnie_len,
	UINT8 type,
	UINT8 *count);

/*
 =====================================
 	function prototype in cmm_wpa.c
 =====================================
*/
void WpaStateMachineInit(PRTMP_ADAPTER pAd, STATE_MACHINE *Sm,
	STATE_MACHINE_FUNC Trans[]);

void RTMPToWirelessSta(
	PRTMP_ADAPTER pAd,
	PMAC_TABLE_ENTRY pEntry,
	PUCHAR pHeader802_3,
	UINT HdrLen,
	PUCHAR pData,
	UINT DataLen,
	BOOLEAN bClearFrame);

void WpaDeriveGTK(
	UCHAR *PMK,
	UCHAR *GNonce,
	UCHAR *AA,
	UCHAR *output,
	UINT len);

void GenRandom(
	PRTMP_ADAPTER pAd,
	UCHAR *macAddr,
	UCHAR *random);

BOOLEAN RTMPCheckWPAframe(
	PRTMP_ADAPTER pAd,
	PMAC_TABLE_ENTRY pEntry,
	PUCHAR pData,
	ULONG DataByteCount,
	UCHAR FromWhichBSSID);

#ifdef HDR_TRANS_SUPPORT
BOOLEAN RTMPCheckWPAframe_Hdr_Trns(
	PRTMP_ADAPTER pAd,
	PMAC_TABLE_ENTRY pEntry,
	PUCHAR pData,
	ULONG DataByteCount,
	UCHAR FromWhichBSSID);
#endif

void WPA_ConstructKdeHdr(
	UINT8 data_type,
	UINT8 data_len,
	PUCHAR pBuf);

PCIPHER_KEY RTMPSwCipherKeySelection(
	PRTMP_ADAPTER pAd,
	PUCHAR pIV,
	RX_BLK *pRxBlk,
	PMAC_TABLE_ENTRY pEntry);

NDIS_STATUS RTMPSoftDecryptionAction(
	PRTMP_ADAPTER pAd,
	PUCHAR pHdr,
	UCHAR UserPriority,
	PCIPHER_KEY pKey,
	PUCHAR pData,
	UINT16 *DataByteCnt);

void RTMPSoftConstructIVHdr(
	UCHAR CipherAlg,
	UCHAR key_id,
	PUCHAR pTxIv,
	PUCHAR pHdrIv,
	UINT8 *hdr_iv_len);

void RTMPSoftEncryptionAction(
	PRTMP_ADAPTER pAd,
	UCHAR CipherAlg,
	PUCHAR pHdr,
	PUCHAR pSrcBufData,
	UINT32 SrcBufLen,
	UCHAR KeyIdx,
	PCIPHER_KEY pKey,
	UINT8 *ext_len);

void RTMPMakeRSNIE(
	PRTMP_ADAPTER pAd,
	UINT AuthMode,
	UINT WepStatus,
	UCHAR apidx);

void WPAInstallPairwiseKey(
	PRTMP_ADAPTER pAd,
	UINT8 BssIdx,
	PMAC_TABLE_ENTRY pEntry,
	BOOLEAN bAE);

void WPAInstallSharedKey(
	PRTMP_ADAPTER pAd,
	UINT8 GroupCipher,
	UINT8 BssIdx,
	UINT8 KeyIdx,
	UINT8 Wcid,
	BOOLEAN bAE,
	PUINT8 pGtk,
	UINT8 GtkLen);

void RTMPSetWcidSecurityInfo(
	PRTMP_ADAPTER pAd,
	UINT8 BssIdx,
	UINT8 KeyIdx,
	UINT8 CipherAlg,
	UINT8 Wcid,
	UINT8 KeyTabFlag);

void CalculateMIC(
	UCHAR KeyDescVer,
	UCHAR *PTK,
	PEAPOL_PACKET pMsg);

BOOLEAN rtmp_chk_tkip_mic(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, 
	RX_BLK *pRxBlk);

#ifdef WPA_SUPPLICANT_SUPPORT
int WpaCheckEapCode(
	 RTMP_ADAPTER *pAd,
	 UCHAR *pFrame,
	 USHORT FrameLen,
	 USHORT OffSet);
#endif /* WPA_SUPPLICANT_SUPPORT */

/*
 =====================================
 	function prototype in cmm_wep.c
 =====================================
*/
UINT RTMP_CALC_FCS32(
	UINT Fcs,
	PUCHAR Cp,
	int Len);

void RTMPConstructWEPIVHdr(
	UINT8 key_idx,
	UCHAR *pn,
	UCHAR *iv_hdr);

BOOLEAN RTMPSoftEncryptWEP(
	PRTMP_ADAPTER pAd,
	PUCHAR pIvHdr,
	PCIPHER_KEY pKey,
	PUCHAR pData,
	ULONG DataByteCnt);

BOOLEAN RTMPSoftDecryptWEP(
	PRTMP_ADAPTER pAd,
	PCIPHER_KEY pKey,
	PUCHAR pData,
	UINT16 *DataByteCnt);

/*
 =====================================
 	function prototype in cmm_tkip.c
 =====================================
*/
NDIS_STATUS RTMPSoftDecryptTKIP(
	PRTMP_ADAPTER pAd,
	PUCHAR pHdr,
	UCHAR UserPriority,
	PCIPHER_KEY pKey,
	PUCHAR pData,
	UINT16 *DataByteCnt);

void TKIP_GTK_KEY_WRAP(
	UCHAR *key,
	UCHAR *iv,
	UCHAR *input_text,
	UINT32 input_len,
	UCHAR *output_text);

void TKIP_GTK_KEY_UNWRAP(
	UCHAR *key,
	UCHAR *iv,
	UCHAR *input_text,
	UINT32 input_len,
	UCHAR *output_text);

/*
 =====================================
 	function prototype in cmm_aes.c
 =====================================
*/

void RTMPConstructCCMPHdr(
	UINT8 key_idx,
	UCHAR *pn,
	UCHAR *ccmp_hdr);

BOOLEAN RTMPSoftEncryptCCMP(
	PRTMP_ADAPTER pAd,
	PUCHAR pHdr,
	PUCHAR pIV,
	PUCHAR pKey,
	PUCHAR pData,
	UINT32 DataLen);

BOOLEAN RTMPSoftDecryptCCMP(
	PRTMP_ADAPTER pAd,
	PUCHAR pHdr,
	PCIPHER_KEY pKey,
	PUCHAR pData,
	UINT16 *DataLen);

#endif
