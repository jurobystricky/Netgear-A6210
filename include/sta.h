/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	sta.h

	Abstract:
	Miniport generic portion header file

*/

#ifndef __STA_H__
#define __STA_H__

void AdjustChannelRelatedValue(PRTMP_ADAPTER pAd, UCHAR *pBwFallBack,
	USHORT ifIndex, BOOLEAN BandWidth, UCHAR PriCh, UCHAR ExtraCh);

int RTMPCheckRxError(RTMP_ADAPTER *pAd, HEADER_802_11 *pHeader, RX_BLK *pRxBlk,
	RXINFO_STRUC *pRxInfo);

void WpaDisassocApAndBlockAssoc(PVOID SystemSpecific1, PVOID FunctionContext,
	PVOID SystemSpecific2, PVOID SystemSpecific3);

void STARxEAPOLFrameIndicate(PRTMP_ADAPTER pAd, MAC_TABLE_ENTRY *pEntry,
	RX_BLK *pRxBlk, UCHAR FromWhichBSSID);

void WpaMicFailureReportFrame(PRTMP_ADAPTER pAd, MLME_QUEUE_ELEM *Elem);
void WpaSendEapolStart(PRTMP_ADAPTER pAdapter, PUCHAR pBssid);
void STAHandleRxDataFrame(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk);
BOOLEAN RTMPCheckChannel(PRTMP_ADAPTER pAd, UCHAR CentralChannel, UCHAR Channel);
void RTMPReportMicError(PRTMP_ADAPTER pAd, PCIPHER_KEY pWpaKey);
NDIS_STATUS STAHardTransmit(PRTMP_ADAPTER pAd, TX_BLK *pTxBlk, UCHAR QueIdx);
NDIS_STATUS STASendPacket(RTMP_ADAPTER *pAd, PNDIS_PACKET pPacket);
int STAInitialize(RTMP_ADAPTER *pAd);

#endif /* __STA_H__ */

