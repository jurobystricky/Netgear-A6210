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
    ap.h

    Abstract:
    Miniport generic portion header file

*/
#ifndef __AP_H__
#define __AP_H__

/* ============================================================= */
/*      Common definition */
/* ============================================================= */
#define MBSS_VLAN_INFO_GET(								\
	__pAd, __VLAN_VID, __VLAN_Priority, __FromWhichBSSID) 				\
{											\
	if ((__FromWhichBSSID < __pAd->ApCfg.BssidNum) &&				\
		(__FromWhichBSSID < HW_BEACON_MAX_NUM) &&				\
		(__pAd->ApCfg.MBSSID[__FromWhichBSSID].wdev.VLAN_VID != 0))		\
	{										\
		__VLAN_VID = __pAd->ApCfg.MBSSID[__FromWhichBSSID].wdev.VLAN_VID;	\
		__VLAN_Priority = __pAd->ApCfg.MBSSID[__FromWhichBSSID].wdev.VLAN_Priority; \
	}										\
}

/* ============================================================= */
/*      Function Prototypes */
/* ============================================================= */

BOOLEAN APBridgeToWirelessSta(PRTMP_ADAPTER pAd, PUCHAR pHeader, UINT HdrLen,
	PUCHAR pData, UINT DataLen, ULONG fromwdsidx);

int ApAllowToSendPacket(RTMP_ADAPTER *pAd, struct wifi_dev *wdev,
	PNDIS_PACKET pPacket, UCHAR *pWcid);

int APSendPacket(RTMP_ADAPTER *pAd, PNDIS_PACKET pPacket);

NDIS_STATUS APInsertPsQueue(PRTMP_ADAPTER pAd, PNDIS_PACKET pPacket,
	MAC_TABLE_ENTRY *pMacEntry, UCHAR QueIdx);

NDIS_STATUS APHardTransmit(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk, UCHAR QueIdx);

void APRxEAPOLFrameIndicate(PRTMP_ADAPTER pAd, MAC_TABLE_ENTRY *pEntry,
	RX_BLK *pRxBlk, UCHAR FromWhichBSSID);

void APHandleRxDataFrame(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk);
void APRxErrorHandle(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk);
int APCheckRxError(RTMP_ADAPTER *pAd, RXINFO_STRUC *pRxInfo, RX_BLK *pRxBlk);
BOOLEAN APChkCls2Cls3Err(RTMP_ADAPTER *pAd, UCHAR Wcid, HEADER_802_11 *pHeader);
void RTMPDescriptorEndianChange(UCHAR *pData, ULONG DescriptorType);
    
void RTMPFrameEndianChange(RTMP_ADAPTER *pAd, UCHAR *pData, ULONG Dir,
	BOOLEAN FromRxDoneInt);

/* ap_assoc.c */

void APAssocStateMachineInit(PRTMP_ADAPTER pAd, STATE_MACHINE *S, 
	STATE_MACHINE_FUNC Trans[]);

void MbssKickOutStas(RTMP_ADAPTER *pAd, int apidx, USHORT Reason);
void APMlmeKickOutSta(RTMP_ADAPTER *pAd, UCHAR *staAddr, UCHAR Wcid, USHORT Reason);

#ifdef DOT11W_PMF_SUPPORT
void APMlmeKickOutAllSta(RTMP_ADAPTER *pAd, UCHAR apidx, USHORT Reason);
#endif

void APCls3errAction(RTMP_ADAPTER *pAd, ULONG wcid, HEADER_802_11 *hdr);

/* ap_auth.c */

void APAuthStateMachineInit(PRTMP_ADAPTER pAd, STATE_MACHINE *Sm, 
	STATE_MACHINE_FUNC Trans[]);

void APCls2errAction(RTMP_ADAPTER *pAd, ULONG wcid, HEADER_802_11 *hdr);

/* ap_connect.c */

#ifdef CONFIG_AP_SUPPORT
BOOLEAN BeaconTransmitRequired(RTMP_ADAPTER *pAd, int apidx, MULTISSID_STRUCT *mbss);
#endif

void APMakeBssBeacon(RTMP_ADAPTER *pAd, int apidx);
void APUpdateBeaconFrame(RTMP_ADAPTER *pAd, int apidx);
void APMakeAllBssBeacon(RTMP_ADAPTER *pAd);
void APUpdateAllBeaconFrame(RTMP_ADAPTER *pAd);

/* ap_sync.c */
void APSyncStateMachineInit(PRTMP_ADAPTER pAd, STATE_MACHINE *Sm,
	STATE_MACHINE_FUNC Trans[]);

void APScanTimeout(PVOID SystemSpecific1, PVOID FunctionContext,
	PVOID SystemSpecific2, PVOID SystemSpecific3);

void ApSiteSurvey(PRTMP_ADAPTER pAd, PNDIS_802_11_SSID pSsid, UCHAR ScanType,
	BOOLEAN ChannelSel);

void SupportRate(PUCHAR SupRate, UCHAR SupRateLen, PUCHAR ExtRate, UCHAR ExtRateLen,
	PUCHAR *Rates, PUCHAR RatesLen, PUCHAR pMaxSupportRate);

BOOLEAN ApScanRunning(RTMP_ADAPTER *pAd);

#ifdef AP_PARTIAL_SCAN_SUPPORT
UCHAR FindPartialScanChannel(PRTMP_ADAPTER pAd);
#endif

#ifdef DOT11_N_SUPPORT
void APUpdateOperationMode(RTMP_ADAPTER *pAd);

#ifdef DOT11N_DRAFT3
void APOverlappingBSSScan(RTMP_ADAPTER *pAd);

int GetBssCoexEffectedChRange(RTMP_ADAPTER *pAd, BSS_COEX_CH_RANGE *pCoexChRange);
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */

/* ap_mlme.c */
void APMlmePeriodicExec(RTMP_ADAPTER *pAd);

BOOLEAN APMsgTypeSubst(PRTMP_ADAPTER pAd, PFRAME_802_11 pFrame, int *Machine, 
	int *MsgType);

void APQuickResponeForRateUpExec(PVOID SystemSpecific1, PVOID FunctionContext, 
	PVOID SystemSpecific2, PVOID SystemSpecific3);

void APAsicEvaluateRxAnt(RTMP_ADAPTER *pAd);
void APAsicRxAntEvalTimeout(RTMP_ADAPTER *pAd);

/* ap.c */
UCHAR get_apidx_by_addr(RTMP_ADAPTER *pAd, UCHAR *addr);

NDIS_STATUS APInitialize(RTMP_ADAPTER *pAd);
void APShutdown(RTMP_ADAPTER *pAd);
void APStartUp(RTMP_ADAPTER *pAd);
void APStop(RTMP_ADAPTER *pAd);
void APCleanupPsQueue(RTMP_ADAPTER *pAd, QUEUE_HEADER *pQueue);
void MacTableMaintenance(RTMP_ADAPTER *pAd);
UINT32 MacTableAssocStaNumGet(RTMP_ADAPTER *pAd);

MAC_TABLE_ENTRY *APSsPsInquiry(PRTMP_ADAPTER pAd, PUCHAR pAddr, SST *Sst, 
	USHORT *Aid, UCHAR *PsMode, UCHAR *Rate); 

#ifdef SYSTEM_LOG_SUPPORT
void ApLogEvent(PRTMP_ADAPTER pAd, PUCHAR pAddr, USHORT Event);
#else
#define ApLogEvent(_pAd, _pAddr, _Event)
#endif

void APUpdateCapabilityAndErpIe(RTMP_ADAPTER *pAd);
BOOLEAN ApCheckAccessControlList(RTMP_ADAPTER *pAd, UCHAR *addr, UCHAR apidx);
void ApUpdateAccessControlList(RTMP_ADAPTER *pAd, UCHAR apidx);

BOOLEAN PeerAssocReqCmmSanity(PRTMP_ADAPTER pAd, BOOLEAN isRessoc, void *Msg, 
	int MsgLen, IE_LISTS *ie_lists);

BOOLEAN PeerDisassocReqSanity(PRTMP_ADAPTER pAd, void *Msg, ULONG MsgLen, 
	PUCHAR pAddr2, UINT16 *SeqNum, USHORT *Reason);

BOOLEAN PeerDeauthReqSanity(PRTMP_ADAPTER pAd, void *Msg, ULONG MsgLen, 
	PUCHAR pAddr2, UINT16 *SeqNum, USHORT *Reason);

BOOLEAN APPeerAuthSanity(PRTMP_ADAPTER pAd, void *Msg, ULONG MsgLen, PUCHAR pAddr1, 
	PUCHAR pAddr2, USHORT *Alg, USHORT *Seq, USHORT *Status, CHAR *ChlgText);

#ifdef DOT1X_SUPPORT
int Set_OwnIPAddr_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
int Set_EAPIfName_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
int Set_PreAuthIfName_Proc(RTMP_ADAPTER *pAd, PSTRING arg);

/* Define in ap.c */
BOOLEAN DOT1X_InternalCmdAction(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry,
	UINT8 cmd);

BOOLEAN DOT1X_EapTriggerAction(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry);
#endif /* DOT1X_SUPPORT */

#endif  /* __AP_H__ */

