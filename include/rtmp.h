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
	rtmp.h

	Abstract:
	Miniport generic portion header file

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
	Paul Lin    2002-08-01    created
	James Tan   2002-09-06    modified (Revise NTCRegTable)
	John Chang  2004-09-06    modified for RT2600
*/
#ifndef __RTMP_H__
#define __RTMP_H__

#include "link_list.h"
#include "spectrum_def.h"
#include "rtmp_dot11.h"
#include "wpa_cmm.h"

#ifdef CONFIG_AP_SUPPORT
#include "ap_autoChSel_cmm.h"
#endif

#include "rtmp_chip.h"

#ifdef DOT11W_PMF_SUPPORT
#include "pmf_cmm.h"
#endif

#ifdef RT_CFG80211_SUPPORT
#include "cfg80211_cmm.h"
#endif

#include "drs_extr.h"

struct _RTMP_RA_LEGACY_TB;

typedef struct _RTMP_ADAPTER RTMP_ADAPTER;
typedef struct _RTMP_ADAPTER *PRTMP_ADAPTER;
typedef struct _RTMP_CHIP_OP_ RTMP_CHIP_OP;
typedef struct _RTMP_CHIP_CAP_ RTMP_CHIP_CAP;

typedef struct _UAPSD_INFO {
	BOOLEAN bAPSDCapable;
} UAPSD_INFO;

#include "mcu/mcu.h"

#ifdef CONFIG_ANDES_SUPPORT
#include "mcu/mcu_and.h"
#endif

#include "radar.h"

#ifdef DFS_SUPPORT
#include "dfs.h"
#endif

#ifdef RTMP_FREQ_CALIBRATION_SUPPORT
#include "frq_cal.h"
#endif

#ifdef RALINK_ATE
#include "rt_ate.h"
#endif

// TODO: shiang-6590, remove it after ATE fully re-organized! copy from rtmp_bbp.h
#ifndef MAX_BBP_ID
	#define MAX_BBP_ID	136
#endif
// TODO: ---End

/*#define DBG_DIAGNOSE		1 */

/*+++Used for merge MiniportMMRequest() and MiniportDataMMRequest() into one function */
#define MAX_DATAMM_RETRY	3
#define MGMT_USE_QUEUE_FLAG	0x80
/*---Used for merge MiniportMMRequest() and MiniportDataMMRequest() into one function */
/* The number of channels for per-channel Tx power offset */


#define	MAXSEQ		(0xFFF)

#ifdef DOT11N_SS3_SUPPORT
#define MAX_MCS_SET 24		/* From MCS 0 ~ MCS 23 */
#else
#define MAX_MCS_SET 16		/* From MCS 0 ~ MCS 15 */
#endif /* DOT11N_SS3_SUPPORT */
#define MAX_VHT_MCS_SET 	20 /* for 1ss~ 2ss with MCS0~9 */

#define MAX_TXPOWER_ARRAY_SIZE	5
#define MAX_EEPROM_BUFFER_SIZE	1024

extern unsigned char CISCO_OUI[];
extern UCHAR BROADCAST_ADDR[MAC_ADDR_LEN];
extern UCHAR ZERO_MAC_ADDR[MAC_ADDR_LEN];

#ifdef DBG
extern char *CipherName[];
#endif

extern UCHAR SNAP_802_1H[6];
extern UCHAR SNAP_BRIDGE_TUNNEL[6];
extern UCHAR EAPOL[2];
extern UCHAR IPX[2];
extern UCHAR TPID[];
extern UCHAR APPLE_TALK[2];
extern UCHAR OfdmRateToRxwiMCS[];
extern UCHAR WMM_UP2AC_MAP[8];
extern unsigned char RateIdToMbps[];
extern USHORT RateIdTo500Kbps[];
extern UCHAR SsidIe;
extern UCHAR SupRateIe;
extern UCHAR ExtRateIe;

#ifdef DOT11_N_SUPPORT
extern UCHAR HtCapIe;
extern UCHAR AddHtInfoIe;
extern UCHAR NewExtChanIe;
extern UCHAR BssCoexistIe;
extern UCHAR ExtHtCapIe;
#endif /* DOT11_N_SUPPORT */

extern UCHAR ExtCapIe;
extern UCHAR ErpIe;
extern UCHAR DsIe;
extern UCHAR TimIe;
extern UCHAR WpaIe;
extern UCHAR Wpa2Ie;
extern UCHAR IbssIe;
extern UCHAR WapiIe;
extern UCHAR WPA_OUI[];
extern UCHAR RSN_OUI[];
extern UCHAR WAPI_OUI[];
extern UCHAR WME_INFO_ELEM[];
extern UCHAR WME_PARM_ELEM[];
extern UCHAR RALINK_OUI[];
extern UCHAR PowerConstraintIE[];

struct _RX_BLK;

typedef union _CAPTURE_MODE_PACKET_BUFFER {
	struct
	{
		UINT32 BYTE0:8;
		UINT32 BYTE1:8;
		UINT32 BYTE2:8;
		UINT32 BYTE3:8;
	} field;
	UINT32 Value;
}CAPTURE_MODE_PACKET_BUFFER, *PCAPTURE_MODE_PACKET_BUFFER;

typedef struct _RSSI_SAMPLE {
	CHAR LastRssi0;		/* last received RSSI */
	CHAR LastRssi1;		/* last received RSSI */
	CHAR LastRssi2;		/* last received RSSI */
	CHAR AvgRssi0;
	CHAR AvgRssi1;
	CHAR AvgRssi2;
	SHORT AvgRssi0X8;
	SHORT AvgRssi1X8;
	SHORT AvgRssi2X8;
	CHAR LastSnr0;
	CHAR LastSnr1;
	CHAR LastSnr2;
	CHAR AvgSnr0;
	CHAR AvgSnr1;
	CHAR AvgSnr2;
	SHORT AvgSnr0X8;
	SHORT AvgSnr1X8;
	SHORT AvgSnr2X8;
	CHAR LastNoiseLevel0;
	CHAR LastNoiseLevel1;
	CHAR LastNoiseLevel2;
} RSSI_SAMPLE;

struct raw_rssi_info;

struct rx_signal_info{
	CHAR raw_rssi[3];
	UCHAR raw_snr[3];
	CHAR freq_offset;
};


/*
	Queue structure and macros
*/
#define InitializeQueueHeader(QueueHeader)              \
{                                                       \
	(QueueHeader)->Head = (QueueHeader)->Tail = NULL;   \
	(QueueHeader)->Number = 0;                          \
}

#define RemoveHeadQueue(QueueHeader)                \
(QueueHeader)->Head;                                \
{                                                   \
	PQUEUE_ENTRY pNext;                             \
	if ((QueueHeader)->Head != NULL)				\
	{												\
		pNext = (QueueHeader)->Head->Next;          \
		(QueueHeader)->Head->Next = NULL;		\
		(QueueHeader)->Head = pNext;                \
		if (pNext == NULL)                          \
			(QueueHeader)->Tail = NULL;             \
		(QueueHeader)->Number--;                    \
	}												\
}

#define InsertHeadQueue(QueueHeader, QueueEntry)            \
{                                                           \
		((PQUEUE_ENTRY)QueueEntry)->Next = (QueueHeader)->Head; \
		(QueueHeader)->Head = (PQUEUE_ENTRY)(QueueEntry);       \
		if ((QueueHeader)->Tail == NULL)                        \
			(QueueHeader)->Tail = (PQUEUE_ENTRY)(QueueEntry);   \
		(QueueHeader)->Number++;                                \
}

#define InsertTailQueue(QueueHeader, QueueEntry)				\
{                                                               \
	((PQUEUE_ENTRY)QueueEntry)->Next = NULL;                    \
	if ((QueueHeader)->Tail)                                    \
		(QueueHeader)->Tail->Next = (PQUEUE_ENTRY)(QueueEntry); \
	else                                                        \
		(QueueHeader)->Head = (PQUEUE_ENTRY)(QueueEntry);       \
	(QueueHeader)->Tail = (PQUEUE_ENTRY)(QueueEntry);           \
	(QueueHeader)->Number++;                                    \
}

#define InsertTailQueueAc(pAd, pEntry, QueueHeader, QueueEntry)		\
{									\
	((PQUEUE_ENTRY)QueueEntry)->Next = NULL;			\
	if ((QueueHeader)->Tail)					\
		(QueueHeader)->Tail->Next = (PQUEUE_ENTRY)(QueueEntry);	\
	else								\
		(QueueHeader)->Head = (PQUEUE_ENTRY)(QueueEntry);	\
	(QueueHeader)->Tail = (PQUEUE_ENTRY)(QueueEntry);		\
	(QueueHeader)->Number++;					\
}


/* */
/*  Macros for flag and ref count operations */
/* */
#define RTMP_SET_FLAG(_M, _F)       ((_M)->Flags |= (_F))
#define RTMP_CLEAR_FLAG(_M, _F)     ((_M)->Flags &= ~(_F))
#define RTMP_CLEAR_FLAGS(_M)        ((_M)->Flags = 0)
#define RTMP_TEST_FLAG(_M, _F)      (((_M)->Flags & (_F)) != 0)
#define RTMP_TEST_FLAGS(_M, _F)     (((_M)->Flags & (_F)) == (_F))
/* Macro for power save flag. */
#define RTMP_SET_PSFLAG(_M, _F)       ((_M)->PSFlags |= (_F))
#define RTMP_CLEAR_PSFLAG(_M, _F)     ((_M)->PSFlags &= ~(_F))
#define RTMP_CLEAR_PSFLAGS(_M)        ((_M)->PSFlags = 0)
#define RTMP_TEST_PSFLAG(_M, _F)      (((_M)->PSFlags & (_F)) != 0)
#define RTMP_TEST_PSFLAGS(_M, _F)     (((_M)->PSFlags & (_F)) == (_F))

#define OPSTATUS_SET_FLAG(_pAd, _F)     ((_pAd)->CommonCfg.OpStatusFlags |= (_F))
#define OPSTATUS_CLEAR_FLAG(_pAd, _F)   ((_pAd)->CommonCfg.OpStatusFlags &= ~(_F))
#define OPSTATUS_TEST_FLAG(_pAd, _F)    (((_pAd)->CommonCfg.OpStatusFlags & (_F)) != 0)

#define WIFI_TEST_SET_FLAG(_pAd, _F)     ((_pAd)->CommonCfg.WiFiTestFlags |= (_F))
#define WIFI_TEST_CLEAR_FLAG(_pAd, _F)   ((_pAd)->CommonCfg.WiFiTestFlags &= ~(_F))
#define WIFI_TEST_CHECK_FLAG(_pAd, _F)    (((_pAd)->CommonCfg.WiFiTestFlags & (_F)) != 0)

#define CLIENT_STATUS_SET_FLAG(_pEntry,_F)      ((_pEntry)->ClientStatusFlags |= (_F))
#define CLIENT_STATUS_CLEAR_FLAG(_pEntry,_F)    ((_pEntry)->ClientStatusFlags &= ~(_F))
#define CLIENT_STATUS_TEST_FLAG(_pEntry,_F)     (((_pEntry)->ClientStatusFlags & (_F)) != 0)

#define RX_FILTER_SET_FLAG(_pAd, _F)    ((_pAd)->CommonCfg.PacketFilter |= (_F))
#define RX_FILTER_CLEAR_FLAG(_pAd, _F)  ((_pAd)->CommonCfg.PacketFilter &= ~(_F))
#define RX_FILTER_TEST_FLAG(_pAd, _F)   (((_pAd)->CommonCfg.PacketFilter & (_F)) != 0)

#define RTMP_SET_MORE_FLAG(_M, _F)	 ((_M)->MoreFlags |= (_F))
#define RTMP_TEST_MORE_FLAG(_M, _F)	(((_M)->MoreFlags & (_F)) != 0)
#define RTMP_CLEAR_MORE_FLAG(_M, _F)	((_M)->MoreFlags &= ~(_F))

#define SET_ASIC_CAP(_pAd, _caps)	((_pAd)->chipCap.asic_caps |= (_caps))
#define IS_ASIC_CAP(_pAd, _caps)	(((_pAd)->chipCap.asic_caps & (_caps)) != 0)
#define CLR_ASIC_CAP(_pAd, _caps)	((_pAd)->chipCap.asic_caps &= ~(_caps))

#ifdef CONFIG_STA_SUPPORT
#define STA_NO_SECURITY_ON(_p)		(_p->StaCfg.wdev.WepStatus == Ndis802_11EncryptionDisabled)
#define STA_WEP_ON(_p)			(_p->StaCfg.wdev.WepStatus == Ndis802_11WEPEnabled)
#define STA_TKIP_ON(_p)			(_p->StaCfg.wdev.WepStatus == Ndis802_11TKIPEnable)
#define STA_AES_ON(_p)			(_p->StaCfg.wdev.WepStatus == Ndis802_11AESEnable)

#define STA_TGN_WIFI_ON(_p)		(_p->StaCfg.bTGnWifiTest == TRUE)
#endif /* CONFIG_STA_SUPPORT */

#define CKIP_KP_ON(_p)			((((_p)->StaCfg.CkipFlag) & 0x10) && ((_p)->StaCfg.bCkipCmicOn == TRUE))
#define CKIP_CMIC_ON(_p)		((((_p)->StaCfg.CkipFlag) & 0x08) && ((_p)->StaCfg.bCkipCmicOn == TRUE))

#define INC_RING_INDEX(_idx, _RingSize)    \
{                                          \
	(_idx) = (_idx+1) % (_RingSize);       \
}

#ifdef USB_BULK_BUF_ALIGMENT
#define CUR_WRITE_IDX_INC(_idx, _RingSize)    \
{                                          \
	(_idx) = (_idx+1) % (_RingSize);       \
}
#endif /* USB_BULK_BUF_ALIGMENT */

#ifdef DOT11_N_SUPPORT
/* StaActive.SupportedHtPhy.MCSSet is copied from AP beacon.  Don't need to update here. */
#define COPY_HTSETTINGS_FROM_MLME_AUX_TO_ACTIVE_CFG(_pAd)                                 \
{                                                                                       \
	_pAd->StaActive.SupportedHtPhy.ChannelWidth = _pAd->MlmeAux.HtCapability.HtCapInfo.ChannelWidth;      \
	_pAd->StaActive.SupportedHtPhy.MimoPs = _pAd->MlmeAux.HtCapability.HtCapInfo.MimoPs;      \
	_pAd->StaActive.SupportedHtPhy.GF = _pAd->MlmeAux.HtCapability.HtCapInfo.GF;      \
	_pAd->StaActive.SupportedHtPhy.ShortGIfor20 = _pAd->MlmeAux.HtCapability.HtCapInfo.ShortGIfor20;      \
	_pAd->StaActive.SupportedHtPhy.ShortGIfor40 = _pAd->MlmeAux.HtCapability.HtCapInfo.ShortGIfor40;      \
	_pAd->StaActive.SupportedHtPhy.TxSTBC = _pAd->MlmeAux.HtCapability.HtCapInfo.TxSTBC;      \
	_pAd->StaActive.SupportedHtPhy.RxSTBC = _pAd->MlmeAux.HtCapability.HtCapInfo.RxSTBC;      \
	_pAd->StaActive.SupportedHtPhy.ExtChanOffset = _pAd->MlmeAux.AddHtInfo.AddHtInfo.ExtChanOffset;      \
	_pAd->StaActive.SupportedHtPhy.RecomWidth = _pAd->MlmeAux.AddHtInfo.AddHtInfo.RecomWidth;      \
	_pAd->StaActive.SupportedHtPhy.OperaionMode = _pAd->MlmeAux.AddHtInfo.AddHtInfo2.OperaionMode;      \
	_pAd->StaActive.SupportedHtPhy.NonGfPresent = _pAd->MlmeAux.AddHtInfo.AddHtInfo2.NonGfPresent;      \
	NdisMoveMemory((_pAd)->MacTab.Content[BSSID_WCID].HTCapability.MCSSet, (_pAd)->StaActive.SupportedPhyInfo.MCSSet, sizeof(UCHAR) * 16);\
}

#define COPY_AP_HTSETTINGS_FROM_BEACON(_pAd, _pHtCapability)                                 \
{                                                                                       \
	_pAd->MacTab.Content[BSSID_WCID].AMsduSize = (UCHAR)(_pHtCapability->HtCapInfo.AMsduSize);	\
	_pAd->MacTab.Content[BSSID_WCID].MmpsMode= (UCHAR)(_pHtCapability->HtCapInfo.MimoPs);	\
	_pAd->MacTab.Content[BSSID_WCID].MaxRAmpduFactor = (UCHAR)(_pHtCapability->HtCapParm.MaxRAmpduFactor);	\
}
#endif /* DOT11_N_SUPPORT */

#ifdef DOT11_VHT_AC
#define COPY_VHT_FROM_MLME_AUX_TO_ACTIVE_CFG(_pAd)                                 \
{                                                                                       \
}
#endif /* DOT11_VHT_AC */


/*
	Common fragment list structure -  Identical to the scatter gather frag list structure
*/
#define NIC_MAX_PHYS_BUF_COUNT              8

typedef struct _RTMP_SCATTER_GATHER_ELEMENT {
	PVOID Address;
	ULONG Length;
	PULONG Reserved;
} RTMP_SCATTER_GATHER_ELEMENT, *PRTMP_SCATTER_GATHER_ELEMENT;

typedef struct _RTMP_SCATTER_GATHER_LIST {
	ULONG NumberOfElements;
	PULONG Reserved;
	RTMP_SCATTER_GATHER_ELEMENT Elements[NIC_MAX_PHYS_BUF_COUNT];
} RTMP_SCATTER_GATHER_LIST, *PRTMP_SCATTER_GATHER_LIST;


/*
	Some utility macros
*/
#ifndef min
#define min(_a, _b)     (((_a) < (_b)) ? (_a) : (_b))
#endif

#ifndef max
#define max(_a, _b)     (((_a) > (_b)) ? (_a) : (_b))
#endif

#define GET_LNA_GAIN(_pAd)	((_pAd->LatchRfRegs.Channel <= 14) ? (_pAd->BLNAGain) : ((_pAd->LatchRfRegs.Channel <= 64) ? (_pAd->ALNAGain0) : ((_pAd->LatchRfRegs.Channel <= 128) ? (_pAd->ALNAGain1) : (_pAd->ALNAGain2))))

#define INC_COUNTER64(Val)          (Val.QuadPart++)

#define INFRA_ON(_p)                (OPSTATUS_TEST_FLAG(_p, fOP_STATUS_INFRA_ON))
#define ADHOC_ON(_p)                (OPSTATUS_TEST_FLAG(_p, fOP_STATUS_ADHOC_ON))
#ifdef CONFIG_STA_SUPPORT
#define MONITOR_ON(_p)              (((_p)->StaCfg.BssType) == BSS_MONITOR)
#else
#define MONITOR_ON(_p)              (((_p)->ApCfg.BssType) == BSS_MONITOR)
#endif

#define IDLE_ON(_p)                 (!INFRA_ON(_p) && !ADHOC_ON(_p))

/* Check LEAP & CCKM flags */
#define LEAP_ON(_p)                 (((_p)->StaCfg.LeapAuthMode) == CISCO_AuthModeLEAP)
#define LEAP_CCKM_ON(_p)            ((((_p)->StaCfg.LeapAuthMode) == CISCO_AuthModeLEAP) && ((_p)->StaCfg.LeapAuthInfo.CCKM == TRUE))

/* if orginal Ethernet frame contains no LLC/SNAP, then an extra LLC/SNAP encap is required */
#define EXTRA_LLCSNAP_ENCAP_FROM_PKT_START(_pBufVA, _pExtraLlcSnapEncap)	\
{										\
	if (((*(_pBufVA + 12) << 8) + *(_pBufVA + 13)) > 1500)			\
	{									\
		_pExtraLlcSnapEncap = SNAP_802_1H;				\
		if (NdisEqualMemory(IPX, _pBufVA + 12, 2) || 			\
			NdisEqualMemory(APPLE_TALK, _pBufVA + 12, 2))		\
		{								\
			_pExtraLlcSnapEncap = SNAP_BRIDGE_TUNNEL;		\
		}								\
	}									\
	else									\
	{									\
		_pExtraLlcSnapEncap = NULL;					\
	}									\
}

/* New Define for new Tx Path. */
#define EXTRA_LLCSNAP_ENCAP_FROM_PKT_OFFSET(_pBufVA, _pExtraLlcSnapEncap)	\
{																\
	if (((*(_pBufVA) << 8) + *(_pBufVA + 1)) > 1500)			\
	{															\
		_pExtraLlcSnapEncap = SNAP_802_1H;						\
		if (NdisEqualMemory(IPX, _pBufVA, 2) || 				\
			NdisEqualMemory(APPLE_TALK, _pBufVA, 2))			\
		{														\
			_pExtraLlcSnapEncap = SNAP_BRIDGE_TUNNEL;			\
		}														\
	}															\
	else														\
	{															\
		_pExtraLlcSnapEncap = NULL;								\
	}															\
}

#define MAKE_802_3_HEADER(_buf, _pMac1, _pMac2, _pType)                   \
{                                                                       \
	NdisMoveMemory(_buf, _pMac1, MAC_ADDR_LEN);                           \
	NdisMoveMemory((_buf + MAC_ADDR_LEN), _pMac2, MAC_ADDR_LEN);          \
	NdisMoveMemory((_buf + MAC_ADDR_LEN * 2), _pType, LENGTH_802_3_TYPE); \
}

/*
	if pData has no LLC/SNAP (neither RFC1042 nor Bridge tunnel),
		keep it that way.
	else if the received frame is LLC/SNAP-encaped IPX or APPLETALK,
		preserve the LLC/SNAP field
	else remove the LLC/SNAP field from the result Ethernet frame

	Patch for WHQL only, which did not turn on Netbios but use IPX within its payload
	Note:
		_pData & _DataSize may be altered (remove 8-byte LLC/SNAP) by this MACRO
		_pRemovedLLCSNAP: pointer to removed LLC/SNAP; NULL is not removed
*/
#define CONVERT_TO_802_3(_p8023hdr, _pDA, _pSA, _pData, _DataSize, _pRemovedLLCSNAP)      \
{                                                                       \
	char LLC_Len[2];                                                    \
																		\
	_pRemovedLLCSNAP = NULL;                                            \
	if (NdisEqualMemory(SNAP_802_1H, _pData, 6)  ||                     \
		NdisEqualMemory(SNAP_BRIDGE_TUNNEL, _pData, 6))                 \
	{                                                                   \
		PUCHAR pProto = _pData + 6;                                     \
																		\
		if ((NdisEqualMemory(IPX, pProto, 2) || NdisEqualMemory(APPLE_TALK, pProto, 2)) &&  \
			NdisEqualMemory(SNAP_802_1H, _pData, 6))                    \
		{                                                               \
			LLC_Len[0] = (UCHAR)(_DataSize >> 8);                       \
			LLC_Len[1] = (UCHAR)(_DataSize & (256 - 1));                \
			MAKE_802_3_HEADER(_p8023hdr, _pDA, _pSA, LLC_Len);          \
		}                                                               \
		else                                                            \
		{                                                               \
			MAKE_802_3_HEADER(_p8023hdr, _pDA, _pSA, pProto);           \
			_pRemovedLLCSNAP = _pData;                                  \
			_DataSize -= LENGTH_802_1_H;                                \
			_pData += LENGTH_802_1_H;                                   \
		}                                                               \
	}                                                                   \
	else                                                                \
	{                                                                   \
		LLC_Len[0] = (UCHAR)(_DataSize >> 8);                           \
		LLC_Len[1] = (UCHAR)(_DataSize & (256 - 1));                    \
		MAKE_802_3_HEADER(_p8023hdr, _pDA, _pSA, LLC_Len);              \
	}                                                                   \
}

/*
	Enqueue this frame to MLME engine
	We need to enqueue the whole frame because MLME need to pass data type
	information from 802.11 header
*/
#ifdef RTMP_MAC_USB
#define REPORT_MGMT_FRAME_TO_MLME(_pAd, Wcid, _pFrame, _FrameSize, _Rssi0, _Rssi1, _Rssi2, _MinSNR, _OpMode)        \
{                                                                                       \
	UINT32 High32TSF=0, Low32TSF=0;                                                          \
	MlmeEnqueueForRecv(_pAd, Wcid, High32TSF, Low32TSF, (UCHAR)_Rssi0, (UCHAR)_Rssi1,(UCHAR)_Rssi2,_FrameSize, _pFrame, (UCHAR)_MinSNR, _OpMode);   \
}
#endif /* RTMP_MAC_USB */

#define IPV4_ADDR_EQUAL(pAddr1, pAddr2)         RTMPEqualMemory((PVOID)(pAddr1), (PVOID)(pAddr2), 4)
#define IPV6_ADDR_EQUAL(pAddr1, pAddr2)         RTMPEqualMemory((PVOID)(pAddr1), (PVOID)(pAddr2), 16)
#define MAC_ADDR_EQUAL(pAddr1,pAddr2)           RTMPEqualMemory((PVOID)(pAddr1), (PVOID)(pAddr2), MAC_ADDR_LEN)
#define SSID_EQUAL(ssid1, len1, ssid2, len2)    ((len1==len2) && (RTMPEqualMemory(ssid1, ssid2, len1)))


#ifdef CONFIG_STA_SUPPORT
#define STA_EXTRA_SETTING(_pAd)

#define STA_PORT_SECURED(_pAd) \
{ \
	BOOLEAN Cancelled; \
	(_pAd)->StaCfg.wdev.PortSecured = WPA_802_1X_PORT_SECURED; \
	RTMP_IndicateMediaState(_pAd, NdisMediaStateConnected); \
	NdisAcquireSpinLock(&((_pAd)->MacTabLock)); \
	(_pAd)->MacTab.Content[BSSID_WCID].PortSecured = (_pAd)->StaCfg.wdev.PortSecured; \
	(_pAd)->MacTab.Content[BSSID_WCID].PrivacyFilter = Ndis802_11PrivFilterAcceptAll;\
	NdisReleaseSpinLock(&(_pAd)->MacTabLock); \
	RTMPCancelTimer(&((_pAd)->Mlme.LinkDownTimer), &Cancelled);\
	STA_EXTRA_SETTING(_pAd); \
}
#endif /* CONFIG_STA_SUPPORT */

/*
	Data buffer for DMA operation, the buffer must be contiguous physical memory
	Both DMA to / from CPU use the same structure.
*/
typedef struct _RTMP_DMABUF {
	ULONG AllocSize;
	PVOID AllocVa;		/* TxBuf virtual address */
	NDIS_PHYSICAL_ADDRESS AllocPa;	/* TxBuf physical address */
} RTMP_DMABUF, *PRTMP_DMABUF;


/*
	Control block (Descriptor) for all ring descriptor DMA operation, buffer must be
	contiguous physical memory. NDIS_PACKET stored the binding Rx packet descriptor
	which won't be released, driver has to wait until upper layer return the packet
	before giveing up this rx ring descriptor to ASIC. NDIS_BUFFER is assocaited pair
	to describe the packet buffer. For Tx, NDIS_PACKET stored the tx packet descriptor
	which driver should ACK upper layer when the tx is physically done or failed.
*/
typedef struct _RTMP_DMACB {
	ULONG AllocSize;	/* Control block size */
	PVOID AllocVa;		/* Control block virtual address */
	NDIS_PHYSICAL_ADDRESS AllocPa;	/* Control block physical address */
	PNDIS_PACKET pNdisPacket;
	PNDIS_PACKET pNextNdisPacket;

	RTMP_DMABUF DmaBuf;	/* Associated DMA buffer structure */
#ifdef CACHE_LINE_32B
	RXD_STRUC LastBDInfo;
#endif /* CACHE_LINE_32B */
} RTMP_DMACB, *PRTMP_DMACB;

typedef struct _RTMP_TX_RING {
	RTMP_DMACB Cell[TX_RING_SIZE];
	UINT32 TxCpuIdx;
	UINT32 TxDmaIdx;
	UINT32 TxSwFreeIdx;	/* software next free tx index */
	UINT32 hw_desc_base;
	UINT32 hw_cidx_addr;
	UINT32 hw_didx_addr;
	UINT32 hw_cnt_addr;
} RTMP_TX_RING;

typedef struct _RTMP_RX_RING {
	RTMP_DMACB Cell[RX_RING_SIZE];
	UINT32 RxCpuIdx;
	UINT32 RxDmaIdx;
	INT32 RxSwReadIdx;	/* software next read index */
	UINT32 hw_desc_base;
	UINT32 hw_cidx_addr;
	UINT32 hw_didx_addr;
	UINT32 hw_cnt_addr;
} RTMP_RX_RING;

typedef struct _RTMP_MGMT_RING {
	RTMP_DMACB Cell[MGMT_RING_SIZE];
	UINT32 TxCpuIdx;
	UINT32 TxDmaIdx;
	UINT32 TxSwFreeIdx;	/* software next free tx index */
	UINT32 hw_desc_base;
	UINT32 hw_cidx_addr;
	UINT32 hw_didx_addr;
	UINT32 hw_cnt_addr;
} RTMP_MGMT_RING, *PRTMP_MGMT_RING;

typedef struct _RTMP_CTRL_RING {
	RTMP_DMACB Cell[MGMT_RING_SIZE];
	UINT32 TxCpuIdx;
	UINT32 TxDmaIdx;
	UINT32 TxSwFreeIdx;	/* software next free tx index */
	UINT32 hw_desc_base;
	UINT32 hw_cidx_addr;
	UINT32 hw_didx_addr;
	UINT32 hw_cnt_addr;
} RTMP_CTRL_RING, *PRTMP_CTRL_RING;

/*
	Statistic counter structure
*/
typedef struct _COUNTER_802_3 {
	/* General Stats */
	ULONG GoodTransmits;
	ULONG GoodReceives;
	ULONG TxErrors;
	ULONG RxErrors;
	ULONG RxNoBuffer;
} COUNTER_802_3, *PCOUNTER_802_3;

typedef struct _COUNTER_802_11 {
	ULONG Length;
/*	LARGE_INTEGER   LastTransmittedFragmentCount; */
	LARGE_INTEGER TransmittedFragmentCount;
	LARGE_INTEGER MulticastTransmittedFrameCount;
	LARGE_INTEGER FailedCount;
	LARGE_INTEGER RetryCount;
	LARGE_INTEGER MultipleRetryCount;
	LARGE_INTEGER RTSSuccessCount;
	LARGE_INTEGER RTSFailureCount;
	LARGE_INTEGER ACKFailureCount;
	LARGE_INTEGER FrameDuplicateCount;
	LARGE_INTEGER ReceivedFragmentCount;
	LARGE_INTEGER MulticastReceivedFrameCount;
	LARGE_INTEGER FCSErrorCount;
	LARGE_INTEGER TransmittedFrameCount;
	LARGE_INTEGER WEPUndecryptableCount;
	LARGE_INTEGER TransmitCountFrmOs;
} COUNTER_802_11, *PCOUNTER_802_11;


typedef struct _COUNTER_RALINK {
	UINT32 OneSecStart;	/* for one sec count clear use */
	UINT32 OneSecBeaconSentCnt;
	UINT32 OneSecFalseCCACnt;	/* CCA error count, for debug purpose, might move to global counter */
	UINT32 OneSecRxFcsErrCnt;	/* CRC error */
	UINT32 OneSecRxOkCnt;	/* RX without error */
	UINT32 OneSecTxFailCount;
	UINT32 OneSecTxNoRetryOkCount;
	UINT32 OneSecTxRetryOkCount;
	UINT32 OneSecRxOkDataCnt;	/* unicast-to-me DATA frame count */
	UINT32 OneSecTransmittedByteCount;	/* both successful and failure, used to calculate TX throughput */

	ULONG OneSecOsTxCount[NUM_OF_TX_RING];
	ULONG OneSecDmaDoneCount[NUM_OF_TX_RING];
	UINT32 OneSecTxDoneCount;
	ULONG OneSecRxCount;
	UINT32 OneSecReceivedByteCount;
	UINT32 OneSecTxAggregationCount;
	UINT32 OneSecRxAggregationCount;
	UINT32 OneSecEnd;	/* for one sec count clear use */

	ULONG TransmittedByteCount;	/* both successful and failure, used to calculate TX throughput */
	ULONG ReceivedByteCount;	/* both CRC okay and CRC error, used to calculate RX throughput */
	ULONG BadCQIAutoRecoveryCount;
	ULONG PoorCQIRoamingCount;
	ULONG MgmtRingFullCount;
	ULONG RxCountSinceLastNULL;
	ULONG RxCount;
	ULONG KickTxCount;
	LARGE_INTEGER RealFcsErrCount;
	ULONG PendingNdisPacketCount;
	ULONG FalseCCACnt;                    /* CCA error count */

	UINT32 LastOneSecTotalTxCount;	/* OneSecTxNoRetryOkCount + OneSecTxRetryOkCount + OneSecTxFailCount */
	UINT32 LastOneSecRxOkDataCnt;	/* OneSecRxOkDataCnt */
	ULONG DuplicateRcv;
	ULONG TxAggCount;
	ULONG TxNonAggCount;
	ULONG TxAgg1MPDUCount;
	ULONG TxAgg2MPDUCount;
	ULONG TxAgg3MPDUCount;
	ULONG TxAgg4MPDUCount;
	ULONG TxAgg5MPDUCount;
	ULONG TxAgg6MPDUCount;
	ULONG TxAgg7MPDUCount;
	ULONG TxAgg8MPDUCount;
	ULONG TxAgg9MPDUCount;
	ULONG TxAgg10MPDUCount;
	ULONG TxAgg11MPDUCount;
	ULONG TxAgg12MPDUCount;
	ULONG TxAgg13MPDUCount;
	ULONG TxAgg14MPDUCount;
	ULONG TxAgg15MPDUCount;
	ULONG TxAgg16MPDUCount;

	LARGE_INTEGER TransmittedOctetsInAMSDU;
	LARGE_INTEGER TransmittedAMSDUCount;
	LARGE_INTEGER ReceivedOctesInAMSDUCount;
	LARGE_INTEGER ReceivedAMSDUCount;
	LARGE_INTEGER TransmittedAMPDUCount;
	LARGE_INTEGER TransmittedMPDUsInAMPDUCount;
	LARGE_INTEGER TransmittedOctetsInAMPDUCount;
	LARGE_INTEGER MPDUInReceivedAMPDUCount;

	ULONG PhyErrCnt;
	ULONG PlcpErrCnt;
} COUNTER_RALINK, *PCOUNTER_RALINK;

typedef struct _COUNTER_DRS {
	/* to record the each TX rate's quality. 0 is best, the bigger the worse. */
	USHORT TxQuality[MAX_TX_RATE_INDEX+1];
	UCHAR PER[MAX_TX_RATE_INDEX+1];
	UCHAR TxRateUpPenalty;	/* extra # of second penalty due to last unstable condition */
	ULONG CurrTxRateStableTime;	/* # of second in current TX rate */
	/*BOOLEAN         fNoisyEnvironment; */
	BOOLEAN fLastSecAccordingRSSI;
	UCHAR LastSecTxRateChangeAction;	/* 0: no change, 1:rate UP, 2:rate down */
	UCHAR LastTimeTxRateChangeAction;	/*Keep last time value of LastSecTxRateChangeAction */
	ULONG LastTxOkCount;
} COUNTER_DRS, *PCOUNTER_DRS;


#ifdef DOT11_N_SUPPORT
#ifdef TXBF_SUPPORT
typedef struct _COUNTER_TXBF{
	ULONG TxSuccessCount;
	ULONG TxRetryCount;
	ULONG TxFailCount;
	ULONG ETxSuccessCount;
	ULONG ETxRetryCount;
	ULONG ETxFailCount;
	ULONG ITxSuccessCount;
	ULONG ITxRetryCount;
	ULONG ITxFailCount;
} COUNTER_TXBF;
#endif /* TXBF_SUPPORT */
#endif /* DOT11_N_SUPPORT */


#ifdef STREAM_MODE_SUPPORT
typedef struct _STREAM_MODE_ENTRY_{
#define STREAM_MODE_STATIC		1
	USHORT flag;
	UCHAR macAddr[MAC_ADDR_LEN];
}STREAM_MODE_ENTRY;
#endif /* STREAM_MODE_SUPPORT */

/* for Microwave oven */
#ifdef MICROWAVE_OVEN_SUPPORT
typedef struct _MO_CFG_STRUCT {
	BOOLEAN bEnable;
	UINT8 nPeriod_Cnt; 	/* measurement period 100ms, mitigate the interference period 900 ms */
	UINT16 nFalseCCACnt;
	UINT16 nFalseCCATh;	/* default is 100 */
} MO_CFG_STRUCT, *PMO_CFG_STRUCT;
#endif /* MICROWAVE_OVEN_SUPPORT */

/* TODO: need to integrate with MICROWAVE_OVEN_SUPPORT */
#ifdef DYNAMIC_VGA_SUPPORT
/* for dynamic vga */
typedef struct _LNA_VGA_CTL_STRUCT {
	BOOLEAN bEnable;
	BOOLEAN bDyncVgaEnable;
	UINT8 nPeriod_Cnt; 	/* measurement period 100ms, mitigate the interference period 900 ms */
	UINT16 nFalseCCACnt;
	UINT16 nFalseCCATh;	/* default is 100 */
	UINT16 nLowFalseCCATh;
	UINT32 agc1_r8_backup;
	UCHAR agc_vga_init_0;
	UCHAR agc_vga_ori_0; /* the original vga gain initialized by firmware at start up */
	UINT16 agc_0_vga_set1_2;
	UINT32 agc1_r9_backup;
	UCHAR agc_vga_init_1;
	UCHAR agc_vga_ori_1; /* the original vga gain initialized by firmware at start up */
	UINT16 agc_1_vga_set1_2;
} LNA_VGA_CTL_STRUCT, *PLNA_VGA_CTL_STRUCT;
#endif

/***************************************************************************
  *	security key related data structure
  **************************************************************************/

/* structure to define WPA Group Key Rekey Interval */
typedef struct GNU_PACKED _RT_802_11_WPA_REKEY {
	ULONG ReKeyMethod;	/* mechanism for rekeying: 0:disable, 1: time-based, 2: packet-based */
	ULONG ReKeyInterval;	/* time-based: seconds, packet-based: kilo-packets */
} RT_WPA_REKEY,*PRT_WPA_REKEY, RT_802_11_WPA_REKEY, *PRT_802_11_WPA_REKEY;


#ifdef RTMP_MAC_USB
/***************************************************************************
  *	RTUSB I/O related data structure
  **************************************************************************/

/* for USB interface, avoid in interrupt when write key */
typedef struct RT_ADD_PAIRWISE_KEY_ENTRY {
	UCHAR MacAddr[6];
	USHORT MacTabMatchWCID;	/* ASIC */
	CIPHER_KEY CipherKey;
} RT_ADD_PAIRWISE_KEY_ENTRY,*PRT_ADD_PAIRWISE_KEY_ENTRY;


/* Cipher suite type for mixed mode group cipher, P802.11i-2004 */
typedef enum _RT_802_11_CIPHER_SUITE_TYPE {
	Cipher_Type_NONE,
	Cipher_Type_WEP40,
	Cipher_Type_TKIP,
	Cipher_Type_RSVD,
	Cipher_Type_CCMP,
	Cipher_Type_WEP104
} RT_802_11_CIPHER_SUITE_TYPE, *PRT_802_11_CIPHER_SUITE_TYPE;
#endif /* RTMP_MAC_USB */

typedef struct {
	UCHAR Addr[MAC_ADDR_LEN];
	UCHAR ErrorCode[2];	/*00 01-Invalid authentication type */
	/*00 02-Authentication timeout */
	/*00 03-Challenge from AP failed */
	/*00 04-Challenge to AP failed */
	BOOLEAN Reported;
} ROGUEAP_ENTRY, *PROGUEAP_ENTRY;

typedef struct {
	UCHAR RogueApNr;
	ROGUEAP_ENTRY RogueApEntry[MAX_LEN_OF_BSS_TABLE];
} ROGUEAP_TABLE, *PROGUEAP_TABLE;

/*
  *	Fragment Frame structure
  */
typedef struct _FRAGMENT_FRAME {
	PNDIS_PACKET pFragPacket;
	ULONG RxSize;
	USHORT Sequence;
	USHORT LastFrag;
	ULONG Flags;		/* Some extra frame information. bit 0: LLC presented */
} FRAGMENT_FRAME, *PFRAGMENT_FRAME;


/*
	Tkip Key structure which RC4 key & MIC calculation
*/
typedef struct _TKIP_KEY_INFO {
	UINT nBytesInM;		/* # bytes in M for MICKEY */
	ULONG IV16;
	ULONG IV32;
	ULONG K0;		/* for MICKEY Low */
	ULONG K1;		/* for MICKEY Hig */
	ULONG L;		/* Current state for MICKEY */
	ULONG R;		/* Current state for MICKEY */
	ULONG M;		/* Message accumulator for MICKEY */
	UCHAR RC4KEY[16];
	UCHAR MIC[8];
} TKIP_KEY_INFO, *PTKIP_KEY_INFO;


/*
	Private / Misc data, counters for driver internal use
*/
typedef struct __PRIVATE_STRUC {
	UINT SystemResetCnt;	/* System reset counter */
	/* Tx ring full occurrance number */
	UINT TxRingFullCnt;
	UINT PhyRxErrCnt;	/* PHY Rx error count, for debug purpose, might move to global counter */
	/* Variables for WEP encryption / decryption in rtmp_wep.c */
	/* Tkip stuff */
	TKIP_KEY_INFO Tx;
	TKIP_KEY_INFO Rx;
} PRIVATE_STRUC, *PPRIVATE_STRUC;


/***************************************************************************
  *	Channel and BBP related data structures
  **************************************************************************/
/* structure to tune BBP R66 (BBP TUNING) */
typedef struct _BBP_R66_TUNING {
	BOOLEAN bEnable;
	USHORT FalseCcaLowerThreshold;	/* default 100 */
	USHORT FalseCcaUpperThreshold;	/* default 512 */
	UCHAR R66Delta;
	UCHAR R66CurrentValue;
	BOOLEAN R66LowerUpperSelect;	/*Before LinkUp, Used LowerBound or UpperBound as R66 value. */
} BBP_R66_TUNING, *PBBP_R66_TUNING;


#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
#define EFFECTED_CH_SECONDARY 	0x1
#define EFFECTED_CH_PRIMARY	0x2
#define EFFECTED_CH_LEGACY	0x4
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */

/* structure to store channel TX power */
typedef struct _CHANNEL_TX_POWER {
	USHORT RemainingTimeForUse;	/*unit: sec */
	UCHAR Channel;
#ifdef DOT11N_DRAFT3
	BOOLEAN bEffectedChannel;	/* For BW 40 operating in 2.4GHz , the "effected channel" is the channel that is covered in 40Mhz. */
#endif /* DOT11N_DRAFT3 */
	CHAR Power;
	CHAR Power2;
#ifdef DOT11N_SS3_SUPPORT
	CHAR Power3;
#endif /* DOT11N_SS3_SUPPORT */
	UCHAR MaxTxPwr;
	UCHAR DfsReq;
	UCHAR RegulatoryDomain;
/*
	Channel property:

	CHANNEL_DISABLED: The channel is disabled.
	CHANNEL_PASSIVE_SCAN: Only passive scanning is allowed.
	CHANNEL_NO_IBSS: IBSS is not allowed.
	CHANNEL_RADAR: Radar detection is required.
	CHANNEL_NO_FAT_ABOVE: Extension channel above this channel is not allowed.
	CHANNEL_NO_FAT_BELOW: Extension channel below this channel is not allowed.
	CHANNEL_40M_CAP: 40 BW channel group
	CHANNEL_80M_CAP: 800 BW channel group
 */
#define CHANNEL_DEFAULT_PROP	0x00
#define CHANNEL_DISABLED	0x01	/* no use */
#define CHANNEL_PASSIVE_SCAN	0x02
#define CHANNEL_NO_IBSS		0x04
#define CHANNEL_RADAR		0x08
#define CHANNEL_NO_FAT_ABOVE	0x10
#define CHANNEL_NO_FAT_BELOW	0x20
#define CHANNEL_40M_CAP		0x40
#define CHANNEL_80M_CAP		0x80

	UCHAR Flags;

} CHANNEL_TX_POWER, *PCHANNEL_TX_POWER;

/* Channel list subset */
typedef struct _CHANNEL_LIST_SUB {
	UCHAR	Channel;
	UCHAR	IdxMap; /* Index mapping to original channel list */
} CHANNEL_LIST_SUB, *PCHANNEL_LIST_SUB;


typedef struct _SOFT_RX_ANT_DIVERSITY_STRUCT {
	UCHAR EvaluatePeriod;	/* 0:not evalute status, 1: evaluate status, 2: switching status */
	UCHAR EvaluateStableCnt;
	UCHAR Pair1PrimaryRxAnt;	/* 0:Ant-E1, 1:Ant-E2 */
	UCHAR Pair1SecondaryRxAnt;	/* 0:Ant-E1, 1:Ant-E2 */
#ifdef CONFIG_STA_SUPPORT
	SHORT Pair1AvgRssi[2];	/* AvgRssi[0]:E1, AvgRssi[1]:E2 */
	SHORT Pair2AvgRssi[2];	/* AvgRssi[0]:E3, AvgRssi[1]:E4 */
#endif /* CONFIG_STA_SUPPORT */
	SHORT Pair1LastAvgRssi;	/* */
	SHORT Pair2LastAvgRssi;	/* */
	ULONG RcvPktNumWhenEvaluate;
	BOOLEAN FirstPktArrivedWhenEvaluate;
#ifdef CONFIG_AP_SUPPORT
	LONG Pair1AvgRssiGroup1[2];
	LONG Pair1AvgRssiGroup2[2];
	ULONG RcvPktNum[2];
#endif /* CONFIG_AP_SUPPORT */
} SOFT_RX_ANT_DIVERSITY, *PSOFT_RX_ANT_DIVERSITY;

typedef enum _ABGBAND_STATE_ {
	UNKNOWN_BAND,
	BG_BAND,
	A_BAND,
} ABGBAND_STATE;


/***************************************************************************
  *	structure for MLME state machine
  **************************************************************************/
typedef struct _MLME_STRUCT {
#ifdef CONFIG_STA_SUPPORT
	/* STA state machines */
	STATE_MACHINE CntlMachine;
	STATE_MACHINE AssocMachine;
	STATE_MACHINE AuthMachine;
	STATE_MACHINE AuthRspMachine;
	STATE_MACHINE SyncMachine;
	STATE_MACHINE WpaPskMachine;
	STATE_MACHINE LeapMachine;
	STATE_MACHINE_FUNC AssocFunc[ASSOC_FUNC_SIZE];
	STATE_MACHINE_FUNC AuthFunc[AUTH_FUNC_SIZE];
	STATE_MACHINE_FUNC AuthRspFunc[AUTH_RSP_FUNC_SIZE];
	STATE_MACHINE_FUNC SyncFunc[SYNC_FUNC_SIZE];

#endif /* CONFIG_STA_SUPPORT */
	STATE_MACHINE_FUNC ActFunc[ACT_FUNC_SIZE];
	/* Action */
	STATE_MACHINE ActMachine;

#ifdef CONFIG_AP_SUPPORT
	/* AP state machines */
	STATE_MACHINE ApAssocMachine;
	STATE_MACHINE ApAuthMachine;
	STATE_MACHINE ApSyncMachine;
	STATE_MACHINE_FUNC ApAssocFunc[AP_ASSOC_FUNC_SIZE];
/*	STATE_MACHINE_FUNC		ApDlsFunc[DLS_FUNC_SIZE]; */
	STATE_MACHINE_FUNC ApAuthFunc[AP_AUTH_FUNC_SIZE];
	STATE_MACHINE_FUNC ApSyncFunc[AP_SYNC_FUNC_SIZE];
#ifdef APCLI_SUPPORT
	STATE_MACHINE ApCliAuthMachine;
	STATE_MACHINE ApCliAssocMachine;
	STATE_MACHINE ApCliCtrlMachine;
	STATE_MACHINE ApCliSyncMachine;
	STATE_MACHINE ApCliWpaPskMachine;

	STATE_MACHINE_FUNC ApCliAuthFunc[APCLI_AUTH_FUNC_SIZE];
	STATE_MACHINE_FUNC ApCliAssocFunc[APCLI_ASSOC_FUNC_SIZE];
	STATE_MACHINE_FUNC ApCliCtrlFunc[APCLI_CTRL_FUNC_SIZE];
	STATE_MACHINE_FUNC ApCliSyncFunc[APCLI_SYNC_FUNC_SIZE];
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

	/* common WPA state machine */
	STATE_MACHINE WpaMachine;
	STATE_MACHINE_FUNC WpaFunc[WPA_FUNC_SIZE];

	ULONG ChannelQuality;	/* 0..100, Channel Quality Indication for Roaming */
	ULONG Now32;		/* latch the value of NdisGetSystemUpTime() */
	ULONG LastSendNULLpsmTime;
	BOOLEAN bRunning;
	spinlock_t TaskLock;
	MLME_QUEUE Queue;
	UINT ShiftReg;
	RALINK_TIMER_STRUCT PeriodicTimer;
	RALINK_TIMER_STRUCT APSDPeriodicTimer;
	RALINK_TIMER_STRUCT LinkDownTimer;
	RALINK_TIMER_STRUCT LinkUpTimer;
	ULONG PeriodicRound;
	ULONG GPIORound;
	ULONG OneSecPeriodicRound;
	UCHAR RealRxPath;
	BOOLEAN bLowThroughput;
	BOOLEAN bEnableAutoAntennaCheck;
	RALINK_TIMER_STRUCT RxAntEvalTimer;

#ifdef RTMP_MAC_USB
	RALINK_TIMER_STRUCT AutoWakeupTimer;
	BOOLEAN AutoWakeupTimerRunning;
#endif /* RTMP_MAC_USB */
} MLME_STRUCT, *PMLME_STRUCT;

#ifdef DOT11_N_SUPPORT
/***************************************************************************
  *	802.11 N related data structures
  **************************************************************************/
struct reordering_mpdu {
	struct reordering_mpdu *next;
	PNDIS_PACKET pPacket;	/* coverted to 802.3 frame */
	int Sequence;		/* sequence number of MPDU */
	BOOLEAN bAMSDU;
	UCHAR OpMode;
};

struct reordering_list {
	struct reordering_mpdu *next;
	int qlen;
};

struct reordering_mpdu_pool {
	PVOID mem;
	spinlock_t lock;
	struct reordering_list freelist;
};

typedef enum _REC_BLOCKACK_STATUS {
	Recipient_NONE = 0,
	Recipient_USED,
	Recipient_HandleRes,
	Recipient_Accept
} REC_BLOCKACK_STATUS, *PREC_BLOCKACK_STATUS;

typedef enum _ORI_BLOCKACK_STATUS {
	Originator_NONE = 0,
	Originator_USED,
	Originator_WaitRes,
	Originator_Done
} ORI_BLOCKACK_STATUS, *PORI_BLOCKACK_STATUS;

typedef struct _BA_ORI_ENTRY {
	UCHAR Wcid;
	UCHAR TID;
	UCHAR BAWinSize;
	UCHAR Token;
	UCHAR amsdu_cap;
/* Sequence is to fill every outgoing QoS DATA frame's sequence field in 802.11 header. */
	USHORT Sequence;
	USHORT TimeOutValue;
	ORI_BLOCKACK_STATUS ORI_BA_Status;
	RALINK_TIMER_STRUCT ORIBATimer;
	PVOID pAdapter;
} BA_ORI_ENTRY, *PBA_ORI_ENTRY;

typedef struct _BA_REC_ENTRY {
	UCHAR Wcid;
	UCHAR TID;
	UCHAR BAWinSize;	/* 7.3.1.14. each buffer is capable of holding a max AMSDU or MSDU. */
	/*UCHAR NumOfRxPkt; */
	/*UCHAR Curindidx; // the head in the RX reordering buffer */
	USHORT LastIndSeq;
/*	USHORT LastIndSeqAtTimer; */
	USHORT TimeOutValue;
	RALINK_TIMER_STRUCT RECBATimer;
	ULONG LastIndSeqAtTimer;
	ULONG nDropPacket;
	ULONG rcvSeq;
	REC_BLOCKACK_STATUS REC_BA_Status;
/*	UCHAR RxBufIdxUsed; */
	/* corresponding virtual address for RX reordering packet storage. */
	/*RTMP_REORDERDMABUF MAP_RXBuf[MAX_RX_REORDERBUF]; */
	spinlock_t RxReRingLock;	/* Rx Ring spinlock */
/*	struct _BA_REC_ENTRY *pNext; */
	PVOID pAdapter;
	struct reordering_list list;
} BA_REC_ENTRY, *PBA_REC_ENTRY;


typedef struct {
	ULONG numAsRecipient;	/* I am recipient of numAsRecipient clients. These client are in the BARecEntry[] */
	ULONG numAsOriginator;	/* I am originator of   numAsOriginator clients. These clients are in the BAOriEntry[] */
	ULONG numDoneOriginator;	/* count Done Originator sessions */
	BA_ORI_ENTRY BAOriEntry[MAX_LEN_OF_BA_ORI_TABLE];
	BA_REC_ENTRY BARecEntry[MAX_LEN_OF_BA_REC_TABLE];
} BA_TABLE, *PBA_TABLE;

/*For QureyBATableOID use; */
typedef struct GNU_PACKED _OID_BA_REC_ENTRY {
	UCHAR MACAddr[MAC_ADDR_LEN];
	UCHAR BaBitmap;		/* if (BaBitmap&(1<<TID)), this session with{MACAddr, TID}exists, so read BufSize[TID] for BufferSize */
	UCHAR rsv;
	UCHAR BufSize[8];
	REC_BLOCKACK_STATUS REC_BA_Status[8];
} OID_BA_REC_ENTRY, *POID_BA_REC_ENTRY;

/*For QureyBATableOID use; */
typedef struct GNU_PACKED _OID_BA_ORI_ENTRY {
	UCHAR MACAddr[MAC_ADDR_LEN];
	UCHAR BaBitmap;		/* if (BaBitmap&(1<<TID)), this session with{MACAddr, TID}exists, so read BufSize[TID] for BufferSize, read ORI_BA_Status[TID] for status */
	UCHAR rsv;
	UCHAR BufSize[8];
	ORI_BLOCKACK_STATUS ORI_BA_Status[8];
} OID_BA_ORI_ENTRY, *POID_BA_ORI_ENTRY;

typedef struct _QUERYBA_TABLE {
	OID_BA_ORI_ENTRY BAOriEntry[32];
	OID_BA_REC_ENTRY BARecEntry[32];
	UCHAR OriNum;		/* Number of below BAOriEntry */
	UCHAR RecNum;		/* Number of below BARecEntry */
} QUERYBA_TABLE, *PQUERYBA_TABLE;

typedef union _BACAP_STRUC {
#ifdef RT_BIG_ENDIAN
	struct {
		UINT32:4;
		UINT32 b2040CoexistScanSup:1;	/*As Sta, support do 2040 coexistence scan for AP. As Ap, support monitor trigger event to check if can use BW 40MHz. */
		UINT32 bHtAdhoc:1;	/* adhoc can use ht rate. */
		UINT32 MMPSmode:2;	/* MIMO power save more, 0:static, 1:dynamic, 2:rsv, 3:mimo enable */
		UINT32 AmsduSize:1;	/* 0:3839, 1:7935 bytes. UINT  MSDUSizeToBytes[]        = { 3839, 7935}; */
		UINT32 AmsduEnable:1;	/*Enable AMSDU transmisstion */
		UINT32 MpduDensity:3;
		UINT32 Policy:2;	/* 0: DELAY_BA 1:IMMED_BA  (//BA Policy subfiled value in ADDBA frame)   2:BA-not use */
		UINT32 AutoBA:1;	/* automatically BA */
		UINT32 TxBAWinLimit:8;
		UINT32 RxBAWinLimit:8;
	} field;
#else
	struct {
		UINT32 RxBAWinLimit:8;
		UINT32 TxBAWinLimit:8;
		UINT32 AutoBA:1;	/* automatically BA */
		UINT32 Policy:2;	/* 0: DELAY_BA 1:IMMED_BA  (//BA Policy subfiled value in ADDBA frame)   2:BA-not use */
		UINT32 MpduDensity:3;
		UINT32 AmsduEnable:1;	/*Enable AMSDU transmisstion */
		UINT32 AmsduSize:1;	/* 0:3839, 1:7935 bytes. UINT  MSDUSizeToBytes[]        = { 3839, 7935}; */
		UINT32 MMPSmode:2;	/* MIMO power save more, 0:static, 1:dynamic, 2:rsv, 3:mimo enable */
		UINT32 bHtAdhoc:1;	/* adhoc can use ht rate. */
		UINT32 b2040CoexistScanSup:1;	/*As Sta, support do 2040 coexistence scan for AP. As Ap, support monitor trigger event to check if can use BW 40MHz. */
		UINT32:4;
	} field;
#endif
	UINT32 word;
} BACAP_STRUC, *PBACAP_STRUC;

typedef struct {
	BOOLEAN IsRecipient;
	UCHAR MACAddr[MAC_ADDR_LEN];
	UCHAR TID;
	UCHAR nMSDU;
	USHORT TimeOut;
	BOOLEAN bAllTid;	/* If True, delete all TID for BA sessions with this MACaddr. */
} OID_ADD_BA_ENTRY, *POID_ADD_BA_ENTRY;

#ifdef DOT11N_DRAFT3
typedef enum _BSS2040COEXIST_FLAG {
	BSS_2040_COEXIST_DISABLE = 0,
	BSS_2040_COEXIST_TIMER_FIRED = 1,
	BSS_2040_COEXIST_INFO_SYNC = 2,
	BSS_2040_COEXIST_INFO_NOTIFY = 4,
} BSS2040COEXIST_FLAG;

typedef struct _BssCoexChRange_ {
	UCHAR primaryCh;
	UCHAR secondaryCh;
	UCHAR effectChStart;
	UCHAR effectChEnd;
} BSS_COEX_CH_RANGE;
#endif /* DOT11N_DRAFT3 */

#define IS_VHT_STA(_pMacEntry)	(_pMacEntry->MaxHTPhyMode.field.MODE == MODE_VHT)
#define IS_HT_STA(_pMacEntry)	\
	(_pMacEntry->MaxHTPhyMode.field.MODE >= MODE_HTMIX)

#define IS_HT_RATE(_pMacEntry)	\
	(_pMacEntry->HTPhyMode.field.MODE >= MODE_HTMIX)

#ifdef DOT11_VHT_AC
#define IS_VHT_RATE(_pMacEntry)	\
	(_pMacEntry->HTPhyMode.field.MODE == MODE_VHT)
#endif
#define PEER_IS_HT_RATE(_pMacEntry)	\
	(_pMacEntry->HTPhyMode.field.MODE >= MODE_HTMIX)

#endif /* DOT11_N_SUPPORT */

/*This structure is for all 802.11n card InterOptibilityTest action. Reset all Num every n second.  (Details see MLMEPeriodic) */
typedef struct _IOT_STRUC {
	BOOLEAN bRTSLongProtOn;
#ifdef CONFIG_STA_SUPPORT
	BOOLEAN bLastAtheros;
	//BOOLEAN bCurrentAtheros;
	//BOOLEAN bNowAtherosBurstOn;
	//BOOLEAN bNextDisableRxBA;
	BOOLEAN bToggle;
#endif /* CONFIG_STA_SUPPORT */
} IOT_STRUC;

/* This is the registry setting for 802.11n transmit setting.  Used in advanced page. */
typedef union _REG_TRANSMIT_SETTING {
#ifdef RT_BIG_ENDIAN
	struct {
		UINT32 rsv:13;
		UINT32 EXTCHA:2;
		UINT32 HTMODE:1;
		UINT32 TRANSNO:2;
		UINT32 STBC:1;	/*SPACE */
		UINT32 ShortGI:1;
		UINT32 BW:1;	/*channel bandwidth 20MHz or 40 MHz */
		UINT32 TxBF:1;	/* 3*3 */
		UINT32 ITxBfEn:1;
		UINT32 rsv0:9;
		/*UINT32  MCS:7;                 // MCS */
		/*UINT32  PhyMode:4; */
	} field;
#else
	struct {
		/*UINT32  PhyMode:4; */
		/*UINT32  MCS:7;                 // MCS */
		UINT32 rsv0:9;
		UINT32 ITxBfEn:1;
		UINT32 TxBF:1;
		UINT32 BW:1;	/*channel bandwidth 20MHz or 40 MHz */
		UINT32 ShortGI:1;
		UINT32 STBC:1;	/*SPACE */
		UINT32 TRANSNO:2;
		UINT32 HTMODE:1;
		UINT32 EXTCHA:2;
		UINT32 rsv:13;
	} field;
#endif
	UINT32 word;
} REG_TRANSMIT_SETTING;


typedef union _DESIRED_TRANSMIT_SETTING {
#ifdef RT_BIG_ENDIAN
	struct {
		USHORT rsv:2;
		USHORT FixedTxMode:3;	/* If MCS isn't AUTO, fix rate in CCK, OFDM, HT or VHT mode. */
		USHORT PhyMode:4;
		USHORT MCS:7;	/* MCS */
	} field;
#else
	struct {
		USHORT MCS:7;
		USHORT PhyMode:4;
		USHORT FixedTxMode:3;
		USHORT rsv:2;
	} field;
#endif
	USHORT word;
 } DESIRED_TRANSMIT_SETTING;


struct hw_setting{
	UCHAR prim_ch;
	UCHAR cent_ch;
	UINT8 bbp_bw;
	UCHAR rf_band;
	UCHAR cur_ch_pwr[3];
	CHAR lan_gain;
};


#define WDEV_TYPE_AP		0x01
#define WDEV_TYPE_STA		0x02
#define WDEV_TYPE_ADHOC		0x04
#define WDEV_TYPE_WDS		0x08
#define WDEV_TYPE_MESH		0x10
#define WDEV_NUM_MAX		16

struct wifi_dev{
	PNET_DEV if_dev;
	void *func_dev;
	void *sys_handle;

	CHAR wdev_idx;	/* index refer from pAd->wdev_list[] */
	CHAR func_idx; /* index refer to func_dev which pointer to */
	UCHAR tr_tb_idx; /* index refer to BSS which this device belong */
	UCHAR wdev_type;
	UCHAR PhyMode;
	UCHAR channel;
	UCHAR if_addr[MAC_ADDR_LEN];
	UCHAR bssid[MAC_ADDR_LEN];

	/* security segment */
	NDIS_802_11_AUTHENTICATION_MODE AuthMode;
	NDIS_802_11_WEP_STATUS WepStatus;
	NDIS_802_11_WEP_STATUS GroupKeyWepStatus;
	WPA_MIX_PAIR_CIPHER WpaMixPairCipher;
	UCHAR DefaultKeyId;
	UCHAR PortSecured;
#if defined(DOT1X_SUPPORT) || defined(WPA_SUPPLICANT_SUPPORT)
	BOOLEAN IEEE8021X; /* Only indicate if we are running in dynamic WEP mode (WEP+802.1x) */
#endif /* DOT1X_SUPPORT */
	BOOLEAN bWpaAutoMode;

	/* transmit segment */
	BOOLEAN allow_data_tx;
	BOOLEAN IgmpSnoopEnable; /* Only enabled for AP/WDS mode */
	RT_PHY_INFO DesiredHtPhyInfo;
	DESIRED_TRANSMIT_SETTING DesiredTransmitSetting;	/* Desired transmit setting. this is for reading registry setting only. not useful. */
	BOOLEAN bAutoTxRateSwitch;
	HTTRANSMIT_SETTING HTPhyMode, MaxHTPhyMode, MinHTPhyMode;	/* For transmit phy setting in TXWI. */

	/* 802.11 protocol related characters */
	BOOLEAN bWmmCapable;	/* 0:disable WMM, 1:enable WMM */
	/* UAPSD information: such as enable or disable, do not remove */
	UAPSD_INFO UapsdInfo;

	/* VLAN related */
	BOOLEAN bVLAN_Tag;
	USHORT VLAN_VID;
	USHORT VLAN_Priority;

	/* operations */
	BOOLEAN (*tx_pkt_allowed)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pPacket, UCHAR *pWcid);
	int (*tx_pkt_handle)(struct _RTMP_ADAPTER *pAd, PNDIS_PACKET pPacket);
	BOOLEAN (*rx_pkt_allowed)(struct _RTMP_ADAPTER *pAd, struct _RX_BLK *pRxBlk);
	int (*rx_pkt_hdr_chk)(struct _RTMP_ADAPTER *pAd, struct _RX_BLK *pRxBlk);
	int (*rx_ps_handle)(struct _RTMP_ADAPTER *pAd, struct _RX_BLK *pRxBlk);
	int (*rx_pkt_foward)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pPacket);

	/* last received packet's SNR for each antenna */
	UCHAR LastSNR0;
	UCHAR LastSNR1;
#ifdef DOT11N_SS3_SUPPORT
	UCHAR LastSNR2;
	INT32 BF_SNR[3];	/* Last RXWI BF SNR. Units=0.25 dB */
#endif /* DOT11N_SS3_SUPPORT */
	RSSI_SAMPLE RssiSample;
	ULONG NumOfAvgRssiSample;
#if defined(RT_CFG80211_SUPPORT) || defined(HOSTAPD_SUPPORT)
	NDIS_HOSTAPD_STATUS Hostapd;
#endif
};


#ifdef RTMP_MAC_USB
/***************************************************************************
  *	USB-based chip Beacon related data structures
  **************************************************************************/
#define BEACON_BITMAP_MASK		0xff
typedef struct _BEACON_SYNC_STRUCT_ {
	UCHAR BeaconBuf[HW_BEACON_MAX_NUM][HW_BEACON_OFFSET];
	UCHAR *BeaconTxWI[HW_BEACON_MAX_NUM];
	ULONG TimIELocationInBeacon[HW_BEACON_MAX_NUM];
	ULONG CapabilityInfoLocationInBeacon[HW_BEACON_MAX_NUM];
	BOOLEAN EnableBeacon;	/* trigger to enable beacon transmission. */
	UCHAR BeaconBitMap;	/* NOTE: If the MAX_MBSSID_NUM is larger than 8, this parameter need to change. */
	UCHAR DtimBitOn;	/* NOTE: If the MAX_MBSSID_NUM is larger than 8, this parameter need to change. */
} BEACON_SYNC_STRUCT;
#endif /* RTMP_MAC_USB */

/***************************************************************************
  *	Multiple SSID related data structures
  **************************************************************************/
#define WLAN_MAX_NUM_OF_TIM	((MAX_LEN_OF_MAC_TABLE >> 3) + 1)	/* /8 + 1 */
#define WLAN_CT_TIM_BCMC_OFFSET	0	/* unit: 32B */

/* clear bcmc TIM bit */
#define WLAN_MR_TIM_BCMC_CLEAR(apidx) \
	pAd->ApCfg.MBSSID[apidx].TimBitmaps[WLAN_CT_TIM_BCMC_OFFSET] &= ~NUM_BIT8[0];

/* set bcmc TIM bit */
#define WLAN_MR_TIM_BCMC_SET(apidx) \
	pAd->ApCfg.MBSSID[apidx].TimBitmaps[WLAN_CT_TIM_BCMC_OFFSET] |= NUM_BIT8[0];

/* clear a station PS TIM bit */
#define WLAN_MR_TIM_BIT_CLEAR(ad_p, apidx, _aid) \
	{	UCHAR tim_offset = _aid >> 3; \
		UCHAR bit_offset = _aid & 0x7; \
		ad_p->ApCfg.MBSSID[apidx].TimBitmaps[tim_offset] &= (~NUM_BIT8[bit_offset]); }

/* set a station PS TIM bit */
#define WLAN_MR_TIM_BIT_SET(ad_p, apidx, _aid) \
	{	UCHAR tim_offset = _aid >> 3; \
		UCHAR bit_offset = _aid & 0x7; \
		ad_p->ApCfg.MBSSID[apidx].TimBitmaps[tim_offset] |= NUM_BIT8[bit_offset]; }

#ifdef CONFIG_AP_SUPPORT
typedef struct _MULTISSID_STRUCT {
	struct wifi_dev wdev;
	int mbss_idx;

#ifdef HOSTAPD_SUPPORT
	NDIS_HOSTAPD_STATUS Hostapd;
	BOOLEAN HostapdWPS;
#endif

	CHAR Ssid[MAX_LEN_OF_SSID];
	UCHAR SsidLen;
	BOOLEAN bHideSsid;
	USHORT CapabilityInfo;
	UCHAR MaxStaNum;	/* Limit the STA connection number per BSS */
	UCHAR StaCount;
	UINT16 StationKeepAliveTime;	/* unit: second */
	PNET_DEV MSSIDDev;

	/* Security segment */
	UCHAR RSNIE_Len[2];
	UCHAR RSN_IE[2][MAX_LEN_OF_RSNIE];

	/* WPA */
	UCHAR WPAKeyString[65];
	UCHAR GMK[32];
	UCHAR PMK[32];
	UCHAR GTK[32];
	UCHAR GNonce[32];
	NDIS_802_11_PRIVACY_FILTER PrivacyFilter;

	/* for Group Rekey, AP ONLY */
	RT_WPA_REKEY WPAREKEY;
	ULONG REKEYCOUNTER;
	RALINK_TIMER_STRUCT REKEYTimer;
	UCHAR REKEYTimerRunning;
	UINT8 RekeyCountDown;

	/* For PMK Cache using, AP ONLY */
	ULONG PMKCachePeriod;	/* unit : jiffies */
	NDIS_AP_802_11_PMKID PMKIDCache;

#ifdef DOT1X_SUPPORT
	BOOLEAN PreAuth;

	/* For 802.1x daemon setting per BSS */
	UINT8 radius_srv_num;
	RADIUS_SRV_INFO radius_srv_info[MAX_RADIUS_SRV_NUM];
	UINT8 NasId[IFNAMSIZ];
	UINT8 NasIdLen;
#endif /* DOT1X_SUPPORT */

	/* Transmitting segment */
	UCHAR TxRate; /* RATE_1, RATE_2, RATE_5_5, RATE_11, ... */
	UCHAR DesiredRates[MAX_LEN_OF_SUPPORTED_RATES];	/* OID_802_11_DESIRED_RATES */
	UCHAR DesiredRatesIndex;
	UCHAR MaxTxRate; /* RATE_1, RATE_2, RATE_5_5, RATE_11 */

	/* Statistics segment */
	/*MBSS_STATISTICS MbssStat;*/
	ULONG TxCount;
	ULONG RxCount;
	ULONG ReceivedByteCount;
	ULONG TransmittedByteCount;
	ULONG RxErrorCount;
	ULONG RxDropCount;
	ULONG TxErrorCount;
	ULONG TxDropCount;
	ULONG ucPktsTx;
	ULONG ucPktsRx;
	ULONG mcPktsTx;
	ULONG mcPktsRx;
	ULONG bcPktsTx;
	ULONG bcPktsRx;
	ULONG StatTxRetryOkCount;
	ULONG StatTxFailCount;
	UCHAR BANClass3Data;
	ULONG IsolateInterStaTraffic;
	UCHAR IsolateInterStaMBCast;

	/* outgoing BEACON frame buffer and corresponding TXWI */
	BOOLEAN bBcnSntReq;	/* used in if beacon send or stop */
	UCHAR BcnBufIdx;
	CHAR BeaconBuf[MAX_BEACON_SIZE];	/* NOTE: BeaconBuf should be 4-byte aligned */
	UCHAR TimBitmaps[WLAN_MAX_NUM_OF_TIM];
	UCHAR TimIELocationInBeacon;
	UCHAR CapabilityInfoLocationInBeacon;
	RT_802_11_ACL AccessControlList;

	/* EDCA Qos */
	/*BOOLEAN bWmmCapable;*/	/* 0:disable WMM, 1:enable WMM */
	BOOLEAN bDLSCapable;	/* 0:disable DLS, 1:enable DLS */

	/*
	   Why need the parameter: 2009/09/22

	   1. iwpriv ra0 set WmmCapable=0
	   2. iwpriv ra0 set WirelessMode=9
	   3. iwpriv ra0 set WirelessMode=0
	   4. iwpriv ra0 set SSID=SampleAP

	   After the 4 commands, WMM is still enabled.
	   So we need the parameter to recover WMM Capable flag.

	   No the problem in station mode.
	 */
	BOOLEAN bWmmCapableOrg;	/* origin Wmm Capable in non-11n mode */

	/* UAPSD information: such as enable or disable, do not remove */
	UAPSD_INFO UapsdInfo;

	/* WPS segment */
	WSC_LV_INFO WscIEBeacon;
	WSC_LV_INFO WscIEProbeResp;

#ifdef DOT11W_PMF_SUPPORT
	PMF_CFG PmfCfg;
#endif
	/* YF@20120417: Avoid connecting to AP in Poor Env, value 0 fOr disable. */
	CHAR AssocReqFailRssiThreshold;
	CHAR AssocReqNoRspRssiThreshold;
	CHAR AuthFailRssiThreshold;
	CHAR AuthNoRspRssiThreshold;
	CHAR RssiLowForStaKickOut;
	CHAR ProbeRspRssiThreshold;
	CHAR FilterUnusedPacket;

#ifdef SPECIFIC_TX_POWER_SUPPORT
	CHAR TxPwrAdj;
#endif /* SPECIFIC_TX_POWER_SUPPORT */

	CHAR ProbeRspTimes;
} MULTISSID_STRUCT, *PMULTISSID_STRUCT;

#define FILTER_NONE 	0x00
#define FILTER_IPV6_MC	0x01
#define FILTER_IPV4_MC	0x02
#define FILTER_IPV6_ALL 0x04
#define FILTER_TOTAL	0x08
#endif /* CONFIG_AP_SUPPORT */
/* configuration common to OPMODE_AP as well as OPMODE_STA */
typedef struct _COMMON_CONFIG {
	BOOLEAN bCountryFlag;
	UCHAR CountryCode[3];
#ifdef EXT_BUILD_CHANNEL_LIST
	UCHAR Geography;
	UCHAR DfsType;
	PUCHAR pChDesp;
#endif /* EXT_BUILD_CHANNEL_LIST */
	UCHAR CountryRegion;	/* Enum of country region, 0:FCC, 1:IC, 2:ETSI, 3:SPAIN, 4:France, 5:MKK, 6:MKK1, 7:Israel */
	UCHAR CountryRegionForABand;	/* Enum of country region for A band */
	UCHAR PhyMode;
	UCHAR cfg_wmode;
	UCHAR SavedPhyMode;
	USHORT Dsifs;		/* in units of usec */
	ULONG PacketFilter;	/* Packet filter for receiving */
	UINT8 RegulatoryClass[MAX_NUM_OF_REGULATORY_CLASS];
	CHAR Ssid[MAX_LEN_OF_SSID];	/* NOT NULL-terminated */
	UCHAR SsidLen;		/* the actual ssid length in used */
	UCHAR LastSsidLen;	/* the actual ssid length in used */
	CHAR LastSsid[MAX_LEN_OF_SSID];	/* NOT NULL-terminated */
	UCHAR LastBssid[MAC_ADDR_LEN];
	UCHAR Bssid[MAC_ADDR_LEN];
	USHORT BeaconPeriod;
	UCHAR Channel;
	UCHAR CentralChannel;	/* Central Channel when using 40MHz is indicating. not real channel. */
	UCHAR SupRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR SupRateLen;
	UCHAR ExtRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR ExtRateLen;
	UCHAR DesireRate[MAX_LEN_OF_SUPPORTED_RATES];	/* OID_802_11_DESIRED_RATES */
	UCHAR MaxDesiredRate;
	UCHAR ExpectedACKRate[MAX_LEN_OF_SUPPORTED_RATES];
	ULONG BasicRateBitmap;	/* backup basic ratebitmap */
	ULONG BasicRateBitmapOld;	/* backup basic ratebitmap */
	BOOLEAN bInServicePeriod;
	BOOLEAN bAPSDAC_BE;
	BOOLEAN bAPSDAC_BK;
	BOOLEAN bAPSDAC_VI;
	BOOLEAN bAPSDAC_VO;

	/* because TSPEC can modify the APSD flag, we need to keep the APSD flag
	   requested in association stage from the station;
	   we need to recover the APSD flag after the TSPEC is deleted. */
	BOOLEAN bACMAPSDBackup[4];	/* for delivery-enabled & trigger-enabled both */
	BOOLEAN bACMAPSDTr[4];	/* no use */
	UCHAR MaxSPLength;
	BOOLEAN bNeedSendTriggerFrame;
	BOOLEAN bAPSDForcePowerSave;	/* Force power save mode, should only use in APSD-STAUT */
	ULONG TriggerTimerCount;
	UCHAR BBPCurrentBW;	/* BW_10, BW_20, BW_40, BW_80 */
	REG_TRANSMIT_SETTING RegTransmitSetting;	/*registry transmit setting. this is for reading registry setting only. not useful. */
	UCHAR TxRate;		/* Same value to fill in TXD. TxRate is 6-bit */
	UCHAR MaxTxRate;	/* RATE_1, RATE_2, RATE_5_5, RATE_11 */
	UCHAR TxRateIndex;	/* Tx rate index in Rate Switch Table */
	UCHAR MinTxRate;	/* RATE_1, RATE_2, RATE_5_5, RATE_11 */
	UCHAR RtsRate;		/* RATE_xxx */
	HTTRANSMIT_SETTING MlmeTransmit;	/* MGMT frame PHY rate setting when operatin at Ht rate. */
	UCHAR MlmeRate;		/* RATE_xxx, used to send MLME frames */
	UCHAR BasicMlmeRate;	/* Default Rate for sending MLME frames */
	USHORT RtsThreshold;	/* in unit of BYTE */
	USHORT FragmentThreshold;	/* in unit of BYTE */
	UCHAR TxPower;		/* in unit of mW */
	ULONG TxPowerPercentage;	/* 0~100 % */
	ULONG TxPowerDefault;	/* keep for TxPowerPercentage */
	UINT8 PwrConstraint;

#ifdef DOT11_N_SUPPORT
	BACAP_STRUC BACapability;	/*   NO USE = 0XFF  ;  IMMED_BA =1  ;  DELAY_BA=0 */
	BACAP_STRUC REGBACapability;	/*   NO USE = 0XFF  ;  IMMED_BA =1  ;  DELAY_BA=0 */
#endif /* DOT11_N_SUPPORT */

#ifdef DOT11_VHT_AC
	BOOLEAN force_vht;
	UCHAR vht_bw;
	UCHAR vht_sgi_80;
	UCHAR vht_stbc;
	UCHAR vht_bw_signal;
	UCHAR vht_cent_ch;
	UCHAR vht_cent_ch2;
	UCHAR vht_mcs_cap;
	UCHAR vht_nss_cap;
	USHORT vht_tx_hrate;
	USHORT vht_rx_hrate;
	BOOLEAN ht20_forbid;
	BOOLEAN vht_ldpc;
#endif /* DOT11_VHT_AC */

	IOT_STRUC IOTestParm;	/* 802.11n InterOpbility Test Parameter; */
	ULONG TxPreamble;	/* Rt802_11PreambleLong, Rt802_11PreambleShort, Rt802_11PreambleAuto */
	BOOLEAN bUseZeroToDisableFragment;	/* Microsoft use 0 as disable */
	ULONG UseBGProtection;	/* 0: auto, 1: always use, 2: always not use */
	BOOLEAN bUseShortSlotTime;	/* 0: disable, 1 - use short slot (9us) */
	BOOLEAN bEnableTxBurst;	/* 1: enble TX PACKET BURST (when BA is established or AP is not a legacy WMM AP), 0: disable TX PACKET BURST */
	BOOLEAN bAggregationCapable;	/* 1: enable TX aggregation when the peer supports it */
	BOOLEAN bPiggyBackCapable;	/* 1: enable TX piggy-back according MAC's version */
	BOOLEAN bIEEE80211H;	/* 1: enable IEEE802.11h spec. */
	UCHAR RDDurRegion; /* Region of radar detection */
	ULONG DisableOLBCDetect;	/* 0: enable OLBC detect; 1 disable OLBC detect */

#ifdef DOT11_N_SUPPORT
	BOOLEAN bRdg;
#endif

#ifdef DOT11_VHT_AC
	BOOLEAN b256QAM_2G;
#endif

	BOOLEAN bWmmCapable;	/* 0:disable WMM, 1:enable WMM */
	QOS_CAPABILITY_PARM APQosCapability;	/* QOS capability of the current associated AP */
	EDCA_PARM APEdcaParm;	/* EDCA parameters of the current associated AP */
#ifdef MULTI_CLIENT_SUPPORT
	BOOLEAN bWmm;		/* have BSS enable/disable WMM */
	UCHAR APCwmin;		/* record ap cwmin */
	UCHAR APCwmax;	/* record ap cwmax */
	UCHAR BSSCwmin;	/* record BSS cwmin */
	UINT32 txRetryCfg;
#endif /* MULTI_CLIENT_SUPPORT */
	QBSS_LOAD_PARM APQbssLoad;	/* QBSS load of the current associated AP */
	UCHAR AckPolicy[4];	/* ACK policy of the specified AC. see ACK_xxx */
#ifdef CONFIG_STA_SUPPORT
	BOOLEAN bDLSCapable;	/* 0:disable DLS, 1:enable DLS */
#endif /* CONFIG_STA_SUPPORT */
	/* a bitmap of BOOLEAN flags. each bit represent an operation status of a particular */
	/* BOOLEAN control, either ON or OFF. These flags should always be accessed via */
	/* OPSTATUS_TEST_FLAG(), OPSTATUS_SET_FLAG(), OP_STATUS_CLEAR_FLAG() macros. */
	/* see fOP_STATUS_xxx in RTMP_DEF.C for detail bit definition */
	ULONG OpStatusFlags;

	BOOLEAN NdisRadioStateOff;	/*For HCT 12.0, set this flag to TRUE instead of called MlmeRadioOff. */
	ABGBAND_STATE BandState;        /* For setting BBP used on B/G or A mode. */

#ifdef DFS_SUPPORT
	/* IEEE802.11H--DFS. */
	RADAR_DETECT_STRUCT RadarDetect;
#endif /* DFS_SUPPORT */

#ifdef DOT11_N_SUPPORT
	/* HT */
	RT_HT_CAPABILITY DesiredHtPhy;
	HT_CAPABILITY_IE HtCapability;
	ADD_HT_INFO_IE AddHTInfo;	/* Useful as AP. */
	/*This IE is used with channel switch announcement element when changing to a new 40MHz. */
	/*This IE is included in channel switch ammouncement frames 7.4.1.5, beacons, probe Rsp. */
	NEW_EXT_CHAN_IE NewExtChanOffset;	/*7.3.2.20A, 1 if extension channel is above the control channel, 3 if below, 0 if not present */

	EXT_CAP_INFO_ELEMENT ExtCapIE;	/* this is the extened capibility IE appreed in MGMT frames. Doesn't need to update once set in Init. */

#ifdef DOT11N_DRAFT3
	BOOLEAN bBssCoexEnable;
	/*
	   Following two paramters now only used for the initial scan operation. the AP only do
	   bandwidth fallback when BssCoexApCnt > BssCoexApCntThr
	   By default, the "BssCoexApCntThr" is set as 0 in "UserCfgInit()".
	 */
	UCHAR BssCoexApCntThr;
	UCHAR BssCoexApCnt;

	UCHAR Bss2040CoexistFlag;	/* bit 0: bBssCoexistTimerRunning, bit 1: NeedSyncAddHtInfo. */
	RALINK_TIMER_STRUCT Bss2040CoexistTimer;
	UCHAR Bss2040NeedFallBack; 	/* 1: Need Fall back to 20MHz */

	/*This IE is used for 20/40 BSS Coexistence. */
	BSS_2040_COEXIST_IE BSS2040CoexistInfo;

	USHORT Dot11OBssScanPassiveDwell;	/* Unit : TU. 5~1000 */
	USHORT Dot11OBssScanActiveDwell;	/* Unit : TU. 10~1000 */
	USHORT Dot11BssWidthTriggerScanInt;	/* Unit : Second */
	USHORT Dot11OBssScanPassiveTotalPerChannel;	/* Unit : TU. 200~10000 */
	USHORT Dot11OBssScanActiveTotalPerChannel;	/* Unit : TU. 20~10000 */
	USHORT Dot11BssWidthChanTranDelayFactor;
	USHORT Dot11OBssScanActivityThre;	/* Unit : percentage */
	ULONG Dot11BssWidthChanTranDelay;	/* multiple of (Dot11BssWidthTriggerScanInt * Dot11BssWidthChanTranDelayFactor) */
	ULONG CountDownCtr;	/* CountDown Counter from (Dot11BssWidthTriggerScanInt * Dot11BssWidthChanTranDelayFactor) */
	BSS_2040_COEXIST_IE LastBSSCoexist2040;
	BSS_2040_COEXIST_IE BSSCoexist2040;
	TRIGGER_EVENT_TAB TriggerEventTab;
	UCHAR ChannelListIdx;
	BOOLEAN bOverlapScanning;
	BOOLEAN bBssCoexNotify;
#endif /* DOT11N_DRAFT3 */

	BOOLEAN bHTProtect;
	BOOLEAN bMIMOPSEnable;
	BOOLEAN bBADecline;
	BOOLEAN bDisableReordering;
	BOOLEAN bForty_Mhz_Intolerant;
	BOOLEAN bExtChannelSwitchAnnouncement;
	BOOLEAN bRcvBSSWidthTriggerEvents;
	ULONG LastRcvBSSWidthTriggerEventsTime;
	UCHAR TxBASize;
	BOOLEAN bRalinkBurstMode;
	UINT32 RestoreBurstMode;
	BOOLEAN ht_ldpc;
#endif /* DOT11_N_SUPPORT */

#ifdef DOT11_VHT_AC
	UINT32 cfg_vht;
	VHT_CAP_INFO vht_info;
	VHT_CAP_IE vht_cap_ie;
	BOOLEAN bNonVhtDisallow; /* Disallow non-VHT connection */
#endif /* DOT11_VHT_AC */

#ifdef SYSTEM_LOG_SUPPORT
	/* Enable wireless event */
	BOOLEAN bWirelessEvent;
#endif /* SYSTEM_LOG_SUPPORT */

	BOOLEAN bWiFiTest;	/* Enable this parameter for WiFi test */

	/* Tx & Rx Stream number selection */
	UCHAR TxStream;
	UCHAR RxStream;

	/* transmit phy mode, trasmit rate for Multicast. */
#ifdef MCAST_RATE_SPECIFIC
	UCHAR McastTransmitMcs;
	UCHAR McastTransmitPhyMode;
#endif /* MCAST_RATE_SPECIFIC */

	BOOLEAN bHardwareRadio;	/* Hardware controlled Radio enabled */

#ifdef RTMP_MAC_USB
	BOOLEAN bMultipleIRP;	/* Multiple Bulk IN flag */
	UCHAR NumOfBulkInIRP;	/* if bMultipleIRP == TRUE, NumOfBulkInIRP will be 4 otherwise be 1 */
	RT_HT_CAPABILITY SupportedHtPhy;
	ULONG MaxPktOneTxBulk;
	UCHAR TxBulkFactor;
	UCHAR RxBulkFactor;

	BOOLEAN IsUpdateBeacon;
	BEACON_SYNC_STRUCT *pBeaconSync;
	RALINK_TIMER_STRUCT BeaconUpdateTimer;
	UINT32 BeaconAdjust;
	UINT32 BeaconFactor;
	UINT32 BeaconRemain;
#endif /* RTMP_MAC_USB */

	spinlock_t MeasureReqTabLock;
	PMEASURE_REQ_TAB pMeasureReqTab;
	spinlock_t TpcReqTabLock;
	PTPC_REQ_TAB pTpcReqTab;

	/* transmit phy mode, trasmit rate for Multicast. */
#ifdef MCAST_RATE_SPECIFIC
	HTTRANSMIT_SETTING MCastPhyMode;
#endif

#ifdef SINGLE_SKU
	UINT16 DefineMaxTxPwr;
	BOOLEAN bSKUMode;
	UINT16 AntGain;
	UINT16 BandedgeDelta;
	UINT16 ModuleTxpower;
#endif

	BOOLEAN HT_DisallowTKIP;	/* Restrict the encryption type in 11n HT mode */
	BOOLEAN HT_Disable;	/* 1: disable HT function; 0: enable HT function */

#ifdef PRE_ANT_SWITCH
	BOOLEAN PreAntSwitch;	/* Preamble Antenna Switch */
	SHORT PreAntSwitchRSSI;	/* Preamble Antenna Switch RSSI threshold */
	SHORT PreAntSwitchTimeout; /* Preamble Antenna Switch timeout in seconds */
#endif

#ifdef CFO_TRACK
	SHORT CFOTrack;	/* CFO Tracking. 0=>use default, 1=>track, 2-7=> track 8-n times, 8=>done tracking */
#endif

#ifdef NEW_RATE_ADAPT_SUPPORT
	USHORT lowTrafficThrd;		/* Threshold for reverting to default MCS when traffic is low */
	SHORT TrainUpRuleRSSI;	/* If TrainUpRule=2 then use Hybrid rule when RSSI < TrainUpRuleRSSI */
	USHORT TrainUpLowThrd;		/* QuickDRS Hybrid train up low threshold */
	USHORT TrainUpHighThrd;	/* QuickDRS Hybrid train up high threshold */
	BOOLEAN TrainUpRule;		/* QuickDRS train up criterion: 0=>Throughput, 1=>PER, 2=> Throughput & PER */
#endif

#ifdef STREAM_MODE_SUPPORT
#define STREAM_MODE_STA_NUM 4

	UCHAR StreamMode; /* 0=disabled, 1=enable for 1SS, 2=enable for 2SS, 3=enable for 1,2SS */
	UCHAR StreamModeMac[STREAM_MODE_STA_NUM][MAC_ADDR_LEN];
	UINT16 StreamModeMCS;	/* Bit map for enabling Stream Mode based on MCS */
#endif /* STREAM_MODE_SUPPORT */

#ifdef DOT11_N_SUPPORT
#ifdef TXBF_SUPPORT
	ULONG ITxBfTimeout;
	ULONG ETxBfTimeout;
	ULONG ETxBfEnCond;		/* Enable sending of sounding and beamforming */
	BOOLEAN ETxBfNoncompress;	/* Force non-compressed Sounding Response */
	BOOLEAN ETxBfIncapable;		/* Report Incapable of BF in TX BF Capabilities */
#endif /* TXBF_SUPPORT */
#endif /* DOT11_N_SUPPORT */

#ifdef DBG_CTRL_SUPPORT
	ULONG DebugFlags;	/* Temporary debug flags */
#endif /* DBG_CTRL_SUPPORT */

#ifdef MICROWAVE_OVEN_SUPPORT
	MO_CFG_STRUCT MO_Cfg;	/* data structure for mitigating microwave interference */
#endif

/* TODO: need to integrate with MICROWAVE_OVEN_SUPPORT */
#ifdef DYNAMIC_VGA_SUPPORT
	LNA_VGA_CTL_STRUCT lna_vga_ctl;
#endif

} COMMON_CONFIG, *PCOMMON_CONFIG;

#ifdef DBG_CTRL_SUPPORT
/* DebugFlag definitions */
#define DBF_NO_BF_AWARE_RA		0x0001	/* Revert to older Rate Adaptation that is not BF aware */
#define DBF_SHOW_BF_STATS		0x0002	/* Display BF statistics in AP "iwpriv stat" display */
#define DBF_NO_TXBF_3SS			0x0004	/* Disable TXBF for MCS > 20 */
#define DBF_UNUSED0008			0x0008	/* Unused */
#define DBF_DBQ_RA_LOG			0x0010	/* Log RA information in DBQ */
#define DBF_INIT_MCS_MARGIN		0x0020	/* Use 6 dB margin when selecting initial MCS */
#define DBF_INIT_MCS_DIS1		0x0040	/* Disable highest MCSs when selecting initial MCS */
#define DBF_FORCE_QUICK_DRS		0x0080	/* Force Quick DRS even if rate didn't change */
#define DBF_FORCE_SGI			0x0100	/* Force Short GI */
#define DBF_DBQ_NO_BCN			0x0200	/* Disable logging of RX Beacon frames */
#define DBF_LOG_VCO_CAL			0x0400	/* Log VCO cal */
#define DBF_DISABLE_CAL			0x0800	/* Disable Divider Calibration at channel change */
#ifdef INCLUDE_DEBUG_QUEUE
#define DBF_DBQ_TXFIFO			0x1000	/* Enable logging of TX information from FIFO */
#define DBF_DBQ_TXFRAME			0x2000	/* Enable logging of Frames queued for TX */
#define DBF_DBQ_RXWI_FULL		0x4000	/* Enable logging of full RXWI */
#define DBF_DBQ_RXWI			0x8000	/* Enable logging of partial RXWI */
#endif /* INCLUDE_DEBUG_QUEUE */

#define DBF_SHOW_RA_LOG			0x010000	/* Display concise Rate Adaptation information */
#define DBF_SHOW_ZERO_RA_LOG		0x020000	/* Include RA Log entries when TxCount is 0 */
#define DBF_FORCE_20MHZ			0x040000	/* Force 20 MHz TX */
#define DBF_FORCE_40MHZ 		0x080000	/* Force 40 MHz Tx */
#define DBF_DISABLE_CCK			0x100000	/* Disable CCK */
#define DBF_UNUSED200000		0x200000	/* Unused */
#define DBF_ENABLE_HT_DUP		0x400000	/* Allow HT Duplicate mode in TX rate table */
#define DBF_ENABLE_CCK_5G		0x800000	/* Enable CCK rates in 5G band */
#define DBF_UNUSED0100000		0x0100000	/* Unused */
#define DBF_ENABLE_20MHZ_MCS8		0x02000000	/* Substitute 20MHz MCS8 for 40MHz MCS8 */
#define DBF_DISABLE_20MHZ_MCS0		0x04000000	/* Disable substitution of 20MHz MCS0 for 40MHz MCS32 */
#define DBF_DISABLE_20MHZ_MCS1		0x08000000	/* Disable substitution of 20MHz MCS1 for 40MHz MCS0 */
#endif /* DBG_CTRL_SUPPORT */

#ifdef WPA_SUPPLICANT_SUPPORT

typedef struct _WPA_SUPPLICANT_INFO{
	/*
		802.1x WEP + MD5 will set key to driver before assoc, but we need to apply the key to
		ASIC after get EAPOL-Success frame, so we use this flag to indicate that
	*/
	BOOLEAN IEEE8021x_required_keys;
	CIPHER_KEY DesireSharedKey[4];  /* Record user desired WEP keys */
	UCHAR DesireSharedKeyId;

	/* 0x00: driver ignores wpa_supplicant */
	/* 0x01: wpa_supplicant initiates scanning and AP selection */
	/* 0x02: driver takes care of scanning, AP selection, and IEEE 802.11 association parameters */
	/* 0x80: wpa_supplicant trigger driver to do WPS */
	UCHAR WpaSupplicantUP;
	UCHAR WpaSupplicantScanCount;
	BOOLEAN bRSN_IE_FromWpaSupplicant;
	BOOLEAN bLostAp;
	UCHAR *pWpsProbeReqIe;
	UINT WpsProbeReqIeLen;
	UCHAR *pWpaAssocIe;
	UINT WpaAssocIeLen;
} WPA_SUPPLICANT_INFO;
#endif /* WPA_SUPPLICANT_SUPPORT */

#ifdef CONFIG_STA_SUPPORT

#ifdef CREDENTIAL_STORE
typedef struct _STA_CONNECT_INFO {
	BOOLEAN Changeable;
	BOOLEAN IEEE8021X;
	CHAR Ssid[MAX_LEN_OF_SSID]; // NOT NULL-terminated
	UCHAR SsidLen; // the actual ssid length in used
	NDIS_802_11_AUTHENTICATION_MODE AuthMode; // This should match to whatever microsoft defined
	NDIS_802_11_WEP_STATUS WepStatus;
	UCHAR DefaultKeyId;
	UCHAR PMK[LEN_PMK]; // WPA PSK mode PMK
	UCHAR WpaPassPhrase[64]; // WPA PSK pass phrase
	UINT WpaPassPhraseLen; // the length of WPA PSK pass phrase
	UINT8 WpaState;
	CIPHER_KEY SharedKey[1][4]; // STA always use SharedKey[BSS0][0..3]
	spinlock_t Lock;
} STA_CONNECT_INFO, *P_STA_CONNECT_INFO;
#endif /* CREDENTIAL_STORE */

/* Modified by Wu Xi-Kun 4/21/2006 */
/* STA configuration and status */
typedef struct _STA_ADMIN_CONFIG {
	struct wifi_dev wdev;

	/*
		GROUP 1 -
		User configuration loaded from Registry, E2PROM or OID_xxx. These settings describe
		the user intended configuration, but not necessary fully equal to the final
		settings in ACTIVE BSS after negotiation/compromize with the BSS holder (either
		AP or IBSS holder).
		Once initialized, user configuration can only be changed via OID_xxx
	*/
	UCHAR BssType;		/* BSS_INFRA or BSS_ADHOC */

#ifdef MONITOR_FLAG_11N_SNIFFER_SUPPORT
#define MONITOR_FLAG_11N_SNIFFER		0x01
	UCHAR BssMonitorFlag;	/* Specific flag for monitor */
#endif /* MONITOR_FLAG_11N_SNIFFER_SUPPORT */

	USHORT AtimWin;		/* used when starting a new IBSS */

	/*
		GROUP 2 -
		User configuration loaded from Registry, E2PROM or OID_xxx. These settings describe
		the user intended configuration, and should be always applied to the final
		settings in ACTIVE BSS without compromising with the BSS holder.
		Once initialized, user configuration can only be changed via OID_xxx
	*/
	USHORT DefaultListenCount;	/* default listen count; */
	USHORT ThisTbttNumToNextWakeUp;
	ULONG WindowsPowerMode;	/* Power mode for AC power */
	ULONG WindowsBatteryPowerMode;	/* Power mode for battery if exists */
	BOOLEAN bWindowsACCAMEnable;	/* Enable CAM power mode when AC on */
	BOOLEAN bAutoReconnect;	/* Set to TRUE when setting OID_802_11_SSID with no matching BSSID */
	UCHAR RssiTrigger;
	UCHAR RssiTriggerMode;	/* RSSI_TRIGGERED_UPON_BELOW_THRESHOLD or RSSI_TRIGGERED_UPON_EXCCEED_THRESHOLD */

	ULONG WindowsPowerProfile;	/* Windows power profile, for NDIS5.1 PnP */

	BOOLEAN  FlgPsmCanNotSleep; /* TRUE: can not switch ASIC to sleep */
	/* MIB:ieee802dot11.dot11smt(1).dot11StationConfigTable(1) */
	USHORT Psm;		/* power management mode   (PWR_ACTIVE|PWR_SAVE) */
	USHORT DisassocReason;
	UCHAR DisassocSta[MAC_ADDR_LEN];
	USHORT DeauthReason;
	UCHAR DeauthSta[MAC_ADDR_LEN];
	USHORT AuthFailReason;
	UCHAR AuthFailSta[MAC_ADDR_LEN];

	/*
		Security segment
	*/
	NDIS_802_11_PRIVACY_FILTER PrivacyFilter;	/* PrivacyFilter enum for 802.1X */

	/* Add to support different cipher suite for WPA2/WPA mode */
	NDIS_802_11_ENCRYPTION_STATUS GroupCipher;	/* Multicast cipher suite */
	NDIS_802_11_ENCRYPTION_STATUS PairCipher;	/* Unicast cipher suite */
	BOOLEAN bMixCipher;	/* Indicate current Pair & Group use different cipher suites */
	USHORT RsnCapability;

	UCHAR WpaPassPhrase[64];/* WPA PSK pass phrase */
	UINT WpaPassPhraseLen;	/* the length of WPA PSK pass phrase */
	UCHAR PMK[LEN_PMK];	/* WPA PSK mode PMK */
	UCHAR PTK[LEN_PTK];	/* WPA PSK mode PTK */
	UCHAR GMK[LEN_GMK];	/* WPA PSK mode GMK */
	UCHAR GTK[MAX_LEN_GTK];	/* GTK from authenticator */
	UCHAR GNonce[32];	/* GNonce for WPA2PSK from authenticator */
	CIPHER_KEY TxGTK;
	BSSID_INFO SavedPMK[PMKID_NO];
	UINT SavedPMKNum;	/* Saved PMKID number */

	/* For WPA countermeasures */
	ULONG LastMicErrorTime;	/* record last MIC error time */
	ULONG MicErrCnt;	/* Should be 0, 1, 2, then reset to zero (after disassoiciation). */
	BOOLEAN bBlockAssoc;	/* Block associate attempt for 60 seconds after counter measure occurred. */
	/* For WPA-PSK supplicant state */
	UINT8 WpaState;		/* Default is SS_NOTUSE and handled by microsoft 802.1x */
	UCHAR ReplayCounter[8];
	UCHAR ANonce[32];	/* ANonce for WPA-PSK from aurhenticator */
	UCHAR SNonce[32];	/* SNonce for WPA-PSK */
	UCHAR LastSNR0;		/* last received BEACON's SNR */
	UCHAR LastSNR1;		/* last received BEACON's SNR for 2nd  antenna */
#ifdef DOT11N_SS3_SUPPORT
	UCHAR LastSNR2;		/* last received BEACON's SNR for 3nd  antenna */
	INT32 BF_SNR[3];	/* Last RXWI BF SNR. Units=0.25 dB */
#endif /* DOT11N_SS3_SUPPORT */
	RSSI_SAMPLE RssiSample;
	ULONG NumOfAvgRssiSample;
	ULONG LastBeaconRxTime;	/* OS's timestamp of the last BEACON RX time */
	ULONG LastScanTime;	/* Record last scan time for issue BSSID_SCAN_LIST */
	BOOLEAN bNotFirstScan;	/* Sam add for ADHOC flag to do first scan when do initialization */
	BOOLEAN bSwRadio;	/* Software controlled Radio On/Off, TRUE: On */
	BOOLEAN bHwRadio;	/* Hardware controlled Radio On/Off, TRUE: On */
	BOOLEAN bRadio;		/* Radio state, And of Sw & Hw radio state */
	BOOLEAN bHardwareRadio;	/* Hardware controlled Radio enabled */
	BOOLEAN bShowHiddenSSID;/* Show all known SSID in SSID list get operation */

	/* New for WPA, windows want us to to keep association information and */
	/* Fixed IEs from last association response */
	NDIS_802_11_ASSOCIATION_INFORMATION AssocInfo;
	USHORT ReqVarIELen;	/* Length of next VIE include EID & Length */
	UCHAR ReqVarIEs[MAX_VIE_LEN];	/* The content saved here should be little-endian format. */
	USHORT ResVarIELen;	/* Length of next VIE include EID & Length */
	UCHAR ResVarIEs[MAX_VIE_LEN];
	UCHAR RSNIE_Len;
	UCHAR RSN_IE[MAX_LEN_OF_RSNIE];	/* The content saved here should be little-endian format. */

	//ULONG CLBusyBytes;	/* Save the total bytes received durning channel load scan time */
	USHORT RPIDensity[8];	/* Array for RPI density collection */

	UCHAR RMReqCnt;		/* Number of measurement request saved. */
	UCHAR CurrentRMReqIdx;	/* Number of measurement request saved. */
	BOOLEAN ParallelReq;	/* Parallel measurement, only one request performed, */
	/* It must be the same channel with maximum duration */
	USHORT ParallelDuration;	/* Maximum duration for parallel measurement */
	UCHAR ParallelChannel;	/* Only one channel with parallel measurement */
	USHORT IAPPToken;	/* IAPP dialog token */
	/* Hack for channel load and noise histogram parameters */
	UCHAR NHFactor;		/* Parameter for Noise histogram */
	UCHAR CLFactor;		/* Parameter for channel load */
	RALINK_TIMER_STRUCT StaQuickResponeForRateUpTimer;
	BOOLEAN StaQuickResponeForRateUpTimerRunning;
	UCHAR DtimCount;	/* 0.. DtimPeriod-1 */
	UCHAR DtimPeriod;	/* default = 3 */
	RALINK_TIMER_STRUCT WpaDisassocAndBlockAssocTimer;
	/* Fast Roaming */
	BOOLEAN bAutoRoaming;	/* 0:disable auto roaming by RSSI, 1:enable auto roaming by RSSI */
	CHAR dBmToRoam;		/* the condition to roam when receiving Rssi less than this value. It's negative value. */
#ifdef WPA_SUPPLICANT_SUPPORT
	WPA_SUPPLICANT_INFO wpa_supplicant_info;
#endif
	CHAR dev_name[16];
	USHORT OriDevType;
	BOOLEAN bTGnWifiTest;
	BOOLEAN bSkipAutoScanConn;

#ifdef EXT_BUILD_CHANNEL_LIST
	UCHAR IEEE80211dClientMode;
	UCHAR StaOriCountryCode[3];
	UCHAR StaOriGeography;
#endif /* EXT_BUILD_CHANNEL_LIST */

#ifdef DOT11W_PMF_SUPPORT
	PMF_CFG PmfCfg;
#endif

	BOOLEAN bAutoConnectByBssid;
	ULONG BeaconLostTime;	/* seconds */
	BOOLEAN bForceTxBurst;	/* 1: force enble TX PACKET BURST, 0: disable */
	BOOLEAN bAutoConnectIfNoSSID;
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
	UCHAR RegClass;		/*IE_SUPP_REG_CLASS: 2009 PF#3: For 20/40 Intolerant Channel Report */
#endif
	BOOLEAN bAdhocN;
#endif /* DOT11_N_SUPPORT */
	BOOLEAN bAdhocCreator;	/*TRUE indicates divice is Creator. */

	/*
	   Enhancement Scanning Mechanism
	   To decrease the possibility of ping loss
	 */
	BOOLEAN bImprovedScan;
	BOOLEAN BssNr;
	UCHAR ScanChannelCnt;	/* 0 at the beginning of scan, stop at 7 */
	UCHAR LastScanChannel;
	BOOLEAN bFastConnect;

/*connectinfo  for tmp store connect info from UI*/
	BOOLEAN Connectinfoflag;
	UCHAR ConnectinfoBssid[MAC_ADDR_LEN];
	UCHAR ConnectinfoChannel;
	UCHAR ConnectinfoSsidLen;
	CHAR  ConnectinfoSsid[MAX_LEN_OF_SSID];
	UCHAR ConnectinfoBssType;

#ifdef RTMP_FREQ_CALIBRATION_SUPPORT
	BOOLEAN AdaptiveFreq;  /* Todo: iwpriv and profile support. */
#endif

	/* UAPSD information: such as enable or disable, do not remove */
	UAPSD_INFO UapsdInfo;
} STA_ADMIN_CONFIG, *PSTA_ADMIN_CONFIG;


/*
	This data structure keep the current active BSS/IBSS's configuration that
	this STA had agreed upon joining the network. Which means these parameters
	are usually decided by the BSS/IBSS creator instead of user configuration.
	Data in this data structurre is valid only when either ADHOC_ON()/INFRA_ON()
	is TRUE. Normally, after SCAN or failed roaming attempts, we need to
	recover back to the current active settings
*/
typedef struct _STA_ACTIVE_CONFIG {
	USHORT Aid;
	USHORT AtimWin;		/* in kusec; IBSS parameter set element */
	USHORT CapabilityInfo;
	EXT_CAP_INFO_ELEMENT ExtCapInfo;
	USHORT CfpMaxDuration;
	USHORT CfpPeriod;

	/* Copy supported rate from desired AP's beacon. We are trying to match */
	/* AP's supported and extended rate settings. */
	UCHAR SupRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR ExtRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR SupRateLen;
	UCHAR ExtRateLen;
	/* Copy supported ht from desired AP's beacon. We are trying to match */
	RT_PHY_INFO SupportedPhyInfo;
	RT_HT_CAPABILITY SupportedHtPhy;
#ifdef DOT11_VHT_AC
	RT_VHT_CAP SupVhtCap;
#endif /* DOT11_VHT_AC */
} STA_ACTIVE_CONFIG, *PSTA_ACTIVE_CONFIG;

#endif /* CONFIG_STA_SUPPORT */


#ifdef CONFIG_AP_SUPPORT
/***************************************************************************
  *	AP related data structures
  **************************************************************************/
/* AUTH-RSP State Machine Aux data structure */
typedef struct _AP_MLME_AUX {
	UCHAR Addr[MAC_ADDR_LEN];
	USHORT Alg;
	CHAR Challenge[CIPHER_TEXT_LEN];
} AP_MLME_AUX, *PAP_MLME_AUX;
#endif /* CONFIG_AP_SUPPORT */

typedef struct _MAC_TABLE_ENTRY {
	/*
	   0:Invalid,
	   Bit 0: AsCli, Bit 1: AsWds, Bit 2: AsAPCLI,
	   Bit 3: AsMesh, Bit 4: AsDls, Bit 5: AsTDls
	 */
	UINT32 EntryType;
	UINT32 ent_status;
	struct wifi_dev *wdev;
	PVOID pAd;
	struct _MAC_TABLE_ENTRY *pNext;

	/*
		A bitmap of BOOLEAN flags. each bit represent an operation status of a particular
		BOOLEAN control, either ON or OFF. These flags should always be accessed via
		CLIENT_STATUS_TEST_FLAG(), CLIENT_STATUS_SET_FLAG(), CLIENT_STATUS_CLEAR_FLAG() macros.
		see fOP_STATUS_xxx in RTMP_DEF.C for detail bit definition. fCLIENT_STATUS_AMSDU_INUSED
	*/
	ULONG ClientStatusFlags;

	HTTRANSMIT_SETTING HTPhyMode, MaxHTPhyMode;	/* For transmit phy setting in TXWI. */
	HTTRANSMIT_SETTING MinHTPhyMode;

/*
	wcid:

	tr_tb_idx:

	func_tb_idx used to indicate following index:
		in ApCfg.ApCliTab
		in pAd->MeshTab
		in WdsTab.MacTab

	apidx: should remove this
*/
	UCHAR wcid;
	UCHAR tr_tb_idx;
	UCHAR func_tb_idx;
	UCHAR apidx;		/* MBSS number */
	BOOLEAN isCached;
	BOOLEAN isRalink;
	BOOLEAN bIAmBadAtheros;	/* Flag if this is Atheros chip that has IOT problem.  We need to turn on RTS/CTS protection. */

	UCHAR Addr[MAC_ADDR_LEN];
#ifdef CONFIG_AP_SUPPORT
	MULTISSID_STRUCT *pMbss;
#endif

	/*
		STATE MACHINE Status
	*/
	USHORT Aid;	/* in range 1~2007, with bit 14~15 = b'11, e.g., 0xc001~0xc7d7 */
	SST Sst;
	AUTH_STATE AuthState;	/* for SHARED KEY authentication state machine used only */

#ifdef RT_CFG80211_P2P_SUPPORT
	CFG_P2P_ENTRY_PARM CFGP2pInfo;
#endif /* RT_CFG80211_SUPPORT */

#ifdef VENDOR_FEATURE1_SUPPORT
	/* total 128B, use UINT32 to avoid alignment problem */
	UINT32 HeaderBuf[32];	/* (total 128B) TempBuffer for TX_INFO + TX_WI + 802.11 Header + padding + AMSDU SubHeader + LLC/SNAP */

	UCHAR HdrPadLen;	/* recording Header Padding Length; */
	UCHAR MpduHeaderLen;
	UINT16 Protocol;
#endif /* VENDOR_FEATURE1_SUPPORT */

	USHORT TxSeq[NUM_OF_TID];
	USHORT NonQosDataSeq;

	/* Rx status related parameters */
	UCHAR LastRssi;
	RSSI_SAMPLE RssiSample;
	UINT32 LastTxRate;
	UINT32 LastRxRate;
	SHORT freqOffset;	/* Last RXWI FOFFSET */
	SHORT freqOffsetValid;	/* Set when freqOffset field has been updated */

#ifdef CONFIG_AP_SUPPORT
#define MAX_LAST_DATA_RSSI_LEN 5
	CHAR LastDataRssi[MAX_LAST_DATA_RSSI_LEN];
	CHAR curLastDataRssiIndex;
#endif /* CONFIG_AP_SUPPORT */

#ifdef DOT11_N_SUPPORT
	USHORT NoBADataCountDown;
	UINT32 CachedBuf[16];	/* UINT (4 bytes) for alignment */
#endif /* DOT11_N_SUPPORT */

	/* WPA/WPA2 4-way database */
	UCHAR EnqueueEapolStartTimerRunning;		/* Enqueue EAPoL-Start for triggering EAP SM */
	RALINK_TIMER_STRUCT EnqueueStartForPSKTimer;	/* A timer which enqueue EAPoL-Start for triggering PSK SM */

#ifdef DROP_MASK_SUPPORT
	BOOLEAN tx_fail_drop_mask_enabled;
	spinlock_t drop_mask_lock;
	BOOLEAN ps_drop_mask_enabled;
	RALINK_TIMER_STRUCT dropmask_timer;
#endif /* DROP_MASK_SUPPORT */

	/* record which entry revoke MIC Failure , if it leaves the BSS itself, AP won't update aMICFailTime MIB */
	UCHAR CMTimerRunning;
	UCHAR RSNIE_Len;
	UCHAR RSN_IE[MAX_LEN_OF_RSNIE];
	UCHAR ANonce[LEN_KEY_DESC_NONCE];
	UCHAR SNonce[LEN_KEY_DESC_NONCE];
	UCHAR R_Counter[LEN_KEY_DESC_REPLAY];
	UCHAR PTK[64];
	UCHAR ReTryCounter;
	RALINK_TIMER_STRUCT RetryTimer;
	NDIS_802_11_AUTHENTICATION_MODE AuthMode;	/* This should match to whatever microsoft defined */
	NDIS_802_11_WEP_STATUS WepStatus;
	NDIS_802_11_WEP_STATUS GroupKeyWepStatus;
	UINT8 WpaState;
	UINT8 GTKState;
	USHORT PortSecured;
	NDIS_802_11_PRIVACY_FILTER PrivacyFilter;	/* PrivacyFilter enum for 802.1X */
	CIPHER_KEY PairwiseKey;
	int PMKID_CacheIdx;
	UCHAR PMKID[LEN_PMKID];
	UCHAR NegotiatedAKM[LEN_OUI_SUITE];	/* It indicate the negotiated AKM suite */
	UCHAR bssid[MAC_ADDR_LEN];
	BOOLEAN IsReassocSta;	/* Indicate whether this is a reassociation procedure */
	ULONG NoDataIdleCount;
	ULONG AssocDeadLine;
	UINT16 StationKeepAliveCount;	/* unit: second */
	USHORT CapabilityInfo;
	UCHAR PsMode;
	UCHAR FlgPsModeIsWakeForAWhile; /* wake up for a while until a condition */
	UCHAR VirtualTimeout; /* peer power save virtual timeout */
	ULONG PsQIdleCount;
	QUEUE_HEADER PsQueue;

/*
	wdev_idx used to indicate following index:
		in ApCfg.ApCliTab
		in pAd->MeshTab
		in WdsTab.MacTab
*/
	UCHAR wdev_idx;

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */

	UINT32 StaConnectTime;	/* the live time of this station since associated with AP */
	UINT32 StaIdleTimeout;	/* idle timeout per entry */

#ifdef UAPSD_SUPPORT
	/* these UAPSD states are used on the fly */
	/* 0:AC_BK, 1:AC_BE, 2:AC_VI, 3:AC_VO */
	BOOLEAN bAPSDCapablePerAC[4];	/* for trigger-enabled */
	BOOLEAN bAPSDDeliverEnabledPerAC[4];	/* for delivery-enabled */
	UCHAR MaxSPLength;
	BOOLEAN bAPSDAllAC;	/* 1: all AC are delivery-enabled U-APSD */
	QUEUE_HEADER UAPSDQueue[WMM_NUM_OF_AC];	/* queue for each U-APSD */
	USHORT UAPSDQIdleCount;	/* U-APSD queue timeout */
	PQUEUE_ENTRY pUAPSDEOSPFrame;	/* the last U-APSD frame */
	USHORT UAPSDTxNum;	/* total U-APSD frame number */
	BOOLEAN bAPSDFlagEOSPOK;	/* 1: EOSP frame is tx by ASIC */
	BOOLEAN bAPSDFlagSPStart;	/* 1: SP is started */

	/* need to use unsigned long, because time parameters in OS is defined as
	   unsigned long */
	unsigned long UAPSDTimeStampLast;	/* unit: 1000000/OS_HZ */
	BOOLEAN bAPSDFlagSpRoughUse;	/* 1: use rough SP (default: accurate) */

	/* we will set the flag when PS-poll frame is received and
	   clear it when statistics handle.
	   if the flag is set when PS-poll frame is received then calling
	   statistics handler to clear it. */
	BOOLEAN bAPSDFlagLegacySent;	/* 1: Legacy PS sent but yet statistics handle */

#ifdef RTMP_MAC_USB
	UINT32 UAPSDTagOffset[WMM_NUM_OF_AC];
#endif /* RTMP_MAC_USB */
#endif /* UAPSD_SUPPORT */

#ifdef STREAM_MODE_SUPPORT
	UINT32 StreamModeMACReg;	/* MAC reg used to control stream mode for this client. 0=>No stream mode */
#endif /* STREAM_MODE_SUPPORT */

	UINT FIFOCount;
	UINT DebugFIFOCount;
	UINT DebugTxCount;

/* ==================================================== */
	enum RATE_ADAPT_ALG rateAlg;
	UCHAR RateLen;
	UCHAR MaxSupportedRate;
	BOOLEAN bAutoTxRateSwitch;
	UCHAR CurrTxRate;
	UCHAR CurrTxRateIndex;
	UCHAR lastRateIdx;
	UCHAR *pTable;	/* Pointer to this entry's Tx Rate Table */

	UCHAR lowTrafficCount;
#ifdef NEW_RATE_ADAPT_SUPPORT
	UCHAR fewPktsCnt;
	BOOLEAN perThrdAdj;
	UCHAR mcsGroup;/* the mcs group to be tried */
#endif /* NEW_RATE_ADAPT_SUPPORT */

#ifdef AGS_SUPPORT
	AGS_CONTROL AGSCtrl;	/* AGS control */
#endif /* AGS_SUPPORT */

	/* to record the each TX rate's quality. 0 is best, the bigger the worse. */
	USHORT TxQuality[MAX_TX_RATE_INDEX + 1];
	BOOLEAN fLastSecAccordingRSSI;
	UCHAR LastSecTxRateChangeAction;	/* 0: no change, 1:rate UP, 2:rate down */
	CHAR LastTimeTxRateChangeAction;	/* Keep last time value of LastSecTxRateChangeAction */
	ULONG LastTxOkCount; 		/* TxSuccess count in last Rate Adaptation interval */
	UCHAR LastTxPER;		/* Tx PER in last Rate Adaptation interval */
	UCHAR PER[MAX_TX_RATE_INDEX + 1];
	UINT32 CurrTxRateStableTime;	/* # of second in current TX rate */
	UCHAR TxRateUpPenalty;		/* extra # of second penalty due to last unstable condition */

	UCHAR SupportRateMode; /* 1: CCK 2:OFDM 4: HT, 8:VHT */
	BOOLEAN SupportCCKMCS[MAX_LEN_OF_CCK_RATES];
	BOOLEAN SupportOFDMMCS[MAX_LEN_OF_OFDM_RATES];
#ifdef DOT11_N_SUPPORT
	BOOLEAN SupportHTMCS[MAX_LEN_OF_HT_RATES];
#ifdef DOT11_VHT_AC
	BOOLEAN SupportVHTMCS[MAX_LEN_OF_VHT_RATES];
#endif
#endif /* DOT11_N_SUPPORT */

#ifdef PEER_DELBA_TX_ADAPT
	UINT32 bPeerDelBaTxAdaptEn;
	RALINK_TIMER_STRUCT DelBA_tx_AdaptTimer;
#endif

#ifdef MFB_SUPPORT
	UCHAR lastLegalMfb;	/* last legal mfb which is used to set rate */
	BOOLEAN isMfbChanged;	/* purpose: true when mfb has changed but the new mfb is not adopted for Tx */
	struct _RTMP_RA_LEGACY_TB *LegalMfbRS;
	BOOLEAN fLastChangeAccordingMfb;
	spinlock_t fLastChangeAccordingMfbLock;
/* Tx MRQ */
	BOOLEAN toTxMrq;
	UCHAR msiToTx, mrqCnt;	/*mrqCnt is used to count down the inverted-BF mrq to be sent */
/* Rx mfb */
	UCHAR pendingMfsi;
/* Tx MFB */
	BOOLEAN toTxMfb;
	UCHAR mfbToTx;
	UCHAR mfb0, mfb1;
#endif	/* MFB_SUPPORT */
#ifdef TXBF_SUPPORT
	UCHAR TxSndgType;
	spinlock_t TxSndgLock;

/* ETxBF */
	UCHAR bfState;
	UCHAR sndgMcs;
	UCHAR sndgBW;
	UCHAR sndg0Mcs;
	int sndg0Snr0, sndg0Snr1, sndg0Snr2;

#ifdef ETXBF_EN_COND3_SUPPORT
	UCHAR bestMethod;
	UCHAR sndgRateIdx;
	UCHAR bf0Mcs, sndg0RateIdx, bf0RateIdx;
	UCHAR sndg1Mcs, bf1Mcs, sndg1RateIdx, bf1RateIdx;
	int sndg1Snr0, sndg1Snr1, sndg1Snr2;
#endif /* ETXBF_EN_COND3_SUPPORT */
	UCHAR noSndgCnt;
	UCHAR eTxBfEnCond;
	UCHAR noSndgCntThrd, ndpSndgStreams;
	UCHAR iTxBfEn;
	RALINK_TIMER_STRUCT eTxBfProbeTimer;

	BOOLEAN phyETxBf;		/* True=>Set ETxBF bit in PHY rate */
	BOOLEAN phyITxBf;		/* True=>Set ITxBF bit in PHY rate */
	UCHAR lastNonBfRate;		/* Last good non-BF rate */
	BOOLEAN lastRatePhyTxBf;	/* For Quick Check. True if last rate was BF */
	USHORT BfTxQuality[MAX_TX_RATE_INDEX + 1];	/* Beamformed TX Quality */

	COUNTER_TXBF TxBFCounters;	/* TxBF Statistics */
	UINT LastETxCount;		/* Used to compute %BF statistics */
	UINT LastITxCount;
	UINT LastTxCount;
#endif /* TXBF_SUPPORT */

#ifdef VHT_TXBF_SUPPORT
	UINT8 snd_dialog_token;
#ifdef SOFT_SOUNDING
	BOOLEAN snd_reqired;
	HTTRANSMIT_SETTING snd_rate;
#endif /* SOFT_SOUNDING */
#endif /* VHT_TXBF_SUPPORT */

	UINT32 OneSecTxNoRetryOkCount;
	UINT32 OneSecTxRetryOkCount;
	UINT32 OneSecTxFailCount;
	UINT32 OneSecRxLGICount;	/* unicast-to-me Long GI count */
	UINT32 OneSecRxSGICount;	/* unicast-to-me Short GI count */
	UINT32 StatTxRetryOkCount;
	UINT32 StatTxFailCount;
	INT32 DownTxMCSRate[NUM_OF_SWFB];
	UINT32 LowPacket;
	UCHAR LastSaveRateIdx;

#ifdef FIFO_EXT_SUPPORT
	UINT32 fifoTxSucCnt;
	UINT32 fifoTxRtyCnt;
#endif /* FIFO_EXT_SUPPORT */

	UINT32 ContinueTxFailCnt;
	ULONG TimeStamp_toTxRing;

/*==================================================== */
	EXT_CAP_INFO_ELEMENT ext_cap;

#ifdef DOT11_N_SUPPORT
	HT_CAPABILITY_IE HTCapability;

	USHORT RXBAbitmap;	/* fill to on-chip  RXWI_BA_BITMASK in 8.1.3RX attribute entry format */
	USHORT TXBAbitmap;	/* This bitmap as originator, only keep in software used to mark AMPDU bit in TXWI */
	USHORT TXAutoBAbitmap;
	USHORT BADeclineBitmap;
	USHORT BARecWcidArray[NUM_OF_TID];	/* The mapping wcid of recipient session. if RXBAbitmap bit is masked */
	USHORT BAOriWcidArray[NUM_OF_TID];	/* The mapping wcid of originator session. if TXBAbitmap bit is masked */
	USHORT BAOriSequence[NUM_OF_TID];	/* The mapping wcid of originator session. if TXBAbitmap bit is masked */
	UCHAR MpduDensity;
	UCHAR MaxRAmpduFactor;
	UCHAR AMsduSize;
	UCHAR MmpsMode;		/* MIMO power save mode. */

#ifdef DOT11N_DRAFT3
	UCHAR BSS2040CoexistenceMgmtSupport;
	BOOLEAN bForty_Mhz_Intolerant;
#endif /* DOT11N_DRAFT3 */

#ifdef DOT11_VHT_AC
	VHT_CAP_IE vht_cap_ie;

	/* only take effect if ext_cap.operating_mode_notification = 1 */
	BOOLEAN force_op_mode;
	OPERATING_MODE operating_mode;
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */

	BOOLEAN bWscCapable;
	UCHAR Receive_EapolStart_EapRspId;
	UINT32 TXMCSExpected[MAX_MCS_SET];
	UINT32 TXMCSSuccessful[MAX_MCS_SET];
	UINT32 TXMCSFailed[MAX_MCS_SET];
	UINT32 TXMCSAutoFallBack[MAX_MCS_SET][MAX_MCS_SET];

#ifdef CONFIG_STA_SUPPORT
	ULONG LastBeaconRxTime;
#endif /* CONFIG_STA_SUPPORT */

#ifdef DOT11W_PMF_SUPPORT
	UCHAR PmfTxTsc[LEN_WPA_TSC];
	UCHAR PmfRxTsc[LEN_WPA_TSC];
	RALINK_TIMER_STRUCT SAQueryTimer;
	RALINK_TIMER_STRUCT SAQueryConfirmTimer;
	UCHAR SAQueryStatus;
	USHORT TransactionID;
#endif /* DOT11W_PMF_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
	LARGE_INTEGER TxPackets;
	LARGE_INTEGER RxPackets;
	ULONG TxBytes;
	ULONG RxBytes;
#endif /* CONFIG_AP_SUPPORT */

	ULONG ChannelQuality;	/* 0..100, Channel Quality Indication for Roaming */
} MAC_TABLE_ENTRY, *PMAC_TABLE_ENTRY;


typedef enum _MAC_ENT_STATUS_{
	/* fAnyStationInPsm */
	MAC_TB_ANY_PSM = 0x1,
	/*
		fAnyStationBadAtheros
		Check if any Station is atheros 802.11n Chip.  We need to use RTS/CTS with Atheros 802,.11n chip.
	*/
	MAC_TB_ANY_ATH = 0x2,
	/*
		fAnyTxOPForceDisable
		Check if it is necessary to disable BE TxOP
	*/
	MAC_TB_FORCE_TxOP = 0x4,
	/*
		fAllStationAsRalink
		Check if all stations are ralink-chipset
	*/
	MAC_TB_ALL_RALINK = 0x8,
	/*
		fAnyStationIsLegacy
		Check if I use legacy rate to transmit to my BSS Station
	*/
	MAC_TB_ANY_LEGACY = 0x10,
	/*
		fAnyStationNonGF
		Check if any Station can't support GF
	*/
	MAC_TB_ANY_NON_GF = 0x20,
	/* fAnyStation20Only */
	MAC_TB_ANY_HT20 = 0x40,
	/*
		fAnyStationMIMOPSDynamic
		Check if any Station is MIMO Dynamic
	*/
	MAC_TB_ANY_MIMO_DYNAMIC = 0x80,
	/*
		fAnyBASession
		Check if there is BA session.  Force turn on RTS/CTS
	*/
	MAC_TB_ANY_BA = 0x100,
	/* fAnyStaFortyIntolerant */
	MAC_TB_ANY_40_INTOlERANT = 0x200,
	/*
		fAllStationGainGoodMCS
		Check if all stations more than MCS threshold
	*/
	MAC_TB_ALL_GOOD_MCS = 0x400,
	/*
		fAnyStationIsHT
		Check if still has any station set the Intolerant bit on!
	*/
	MAC_TB_ANY_HT = 0x800,
	/* fAnyWapiStation */
	MAC_TB_ANY_WAPI = 0x1000,
} MAC_ENT_STATUS;

typedef struct _MAC_TABLE {
	MAC_TABLE_ENTRY *Hash[HASH_TABLE_SIZE];
	MAC_TABLE_ENTRY Content[MAX_LEN_OF_MAC_TABLE];
	USHORT Size;
	QUEUE_HEADER McastPsQueue;
	ULONG PsQIdleCount;
	BOOLEAN fAnyStationInPsm;
	BOOLEAN fAnyStationBadAtheros;	/* Check if any Station is atheros 802.11n Chip.  We need to use RTS/CTS with Atheros 802,.11n chip. */
	BOOLEAN fAnyTxOPForceDisable;	/* Check if it is necessary to disable BE TxOP */
	BOOLEAN fAllStationAsRalink;	/* Check if all stations are ralink-chipset */
#ifdef DOT11_N_SUPPORT
	BOOLEAN fAnyStationIsLegacy;	/* Check if I use legacy rate to transmit to my BSS Station/ */
	BOOLEAN fAnyStationNonGF;	/* Check if any Station can't support GF. */
	BOOLEAN fAnyStation20Only;	/* Check if any Station can't support GF. */
	BOOLEAN fAnyStationMIMOPSDynamic;	/* Check if any Station is MIMO Dynamic */
	BOOLEAN fAnyBASession;	/* Check if there is BA session.  Force turn on RTS/CTS */
	BOOLEAN fAnyStaFortyIntolerant;	/* Check if still has any station set the Intolerant bit on! */
	BOOLEAN fAllStationGainGoodMCS; /* Check if all stations more than MCS threshold */

#ifdef CONFIG_AP_SUPPORT
	BOOLEAN fAnyStationIsHT;	/* Check if there is 11n STA.  Force turn off AP MIMO PS */
#endif /* CONFIG_AP_SUPPORT */
#endif /* DOT11_N_SUPPORT */

	USHORT MsduLifeTime; /* life time for PS packet */
} MAC_TABLE, *PMAC_TABLE;


#ifdef CONFIG_AP_SUPPORT
/***************************************************************************
  *	AP WDS related data structures
  **************************************************************************/


/***************************************************************************
  *	AP APCLI related data structures
  **************************************************************************/
typedef struct _APCLI_STRUCT {
	struct wifi_dev wdev;
	BOOLEAN Enable;		/* Set it as 1 if the apcli interface was configured to "1"  or by iwpriv cmd "ApCliEnable" */
	BOOLEAN Valid;		/* Set it as 1 if the apcli interface associated success to remote AP. */
	MLME_AUX MlmeAux;	/* temporary settings used during MLME state machine */
	UCHAR MacTabWCID;	/*WCID value, which point to the entry of ASIC Mac table. */
	UCHAR SsidLen;
	CHAR Ssid[MAX_LEN_OF_SSID];
#ifdef APCLI_CONNECTION_TRIAL
	UCHAR TrialCh; /* the channel that Apcli interface will try to connect the rootap locates */
	RALINK_TIMER_STRUCT TrialConnectTimer;
	RALINK_TIMER_STRUCT TrialConnectPhase2Timer;
	RALINK_TIMER_STRUCT TrialConnectRetryTimer;
	MAC_TABLE_ENTRY	oldRootAP;
	USHORT NewRootApRetryCnt;
	UCHAR ifIndex;
	PVOID pAd;
#endif /* APCLI_CONNECTION_TRIAL */

	UCHAR CfgSsidLen;
	CHAR CfgSsid[MAX_LEN_OF_SSID];
	UCHAR CfgApCliBssid[MAC_ADDR_LEN];
	ULONG ApCliRcvBeaconTime;
	ULONG ApCliLinkUpTime;
	USHORT ApCliBeaconPeriod;
	ULONG CtrlCurrState;
	ULONG SyncCurrState;
	ULONG AuthCurrState;
	ULONG AssocCurrState;
	ULONG WpaPskCurrState;

#ifdef APCLI_AUTO_CONNECT_SUPPORT
	USHORT	ProbeReqCnt;
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
	USHORT AuthReqCnt;
	USHORT AssocReqCnt;
	UCHAR MpduDensity;

	/*
		Security segment
	*/
	/* Add to support different cipher suite for WPA2/WPA mode */
	NDIS_802_11_ENCRYPTION_STATUS GroupCipher;	/* Multicast cipher suite */
	NDIS_802_11_ENCRYPTION_STATUS PairCipher;	/* Unicast cipher suite */
	BOOLEAN bMixCipher;	/* Indicate current Pair & Group use different cipher suites */
	USHORT RsnCapability;
	UCHAR PSK[100];		/* reserve PSK key material */
	UCHAR PSKLen;
	UCHAR PMK[32];		/* WPA PSK mode PMK */
	UCHAR PTK[64];		/* WPA PSK mode PTK */
	UCHAR GTK[32];		/* GTK from authenticator */

	/*CIPHER_KEY            PairwiseKey; */
	CIPHER_KEY SharedKey[SHARE_KEY_NUM];

	/* store RSN_IE built by driver */
	UCHAR RSN_IE[MAX_LEN_OF_RSNIE];	/* The content saved here should be convert to little-endian format. */
	UCHAR RSNIE_Len;

	/* For WPA countermeasures */
	ULONG LastMicErrorTime;	/* record last MIC error time */
	ULONG MicErrCnt; /* Should be 0, 1, 2, then reset to zero (after disassoiciation). */
	BOOLEAN bBlockAssoc;	/* Block associate attempt for 60 seconds after counter measure occurred. */

	/* For WPA-PSK supplicant state */
	UCHAR ReplayCounter[8];
	UCHAR SNonce[32];	/* SNonce for WPA-PSK */
	UCHAR GNonce[32];	/* GNonce for WPA-PSK from authenticator */

#ifdef WPA_SUPPLICANT_SUPPORT
	WPA_SUPPLICANT_INFO wpa_supplicant_info;

	BOOLEAN bScanReqIsFromWebUI;
	BSSID_INFO SavedPMK[PMKID_NO];
	UINT SavedPMKNum; /* Saved PMKID number */
	BOOLEAN bConfigChanged;
	NDIS_802_11_ASSOCIATION_INFORMATION AssocInfo;
	USHORT ReqVarIELen; /* Length of next VIE include EID & Length */
	UCHAR ReqVarIEs[MAX_VIE_LEN]; /* The content saved here should be little-endian format. */
	USHORT ResVarIELen; /* Length of next VIE include EID & Length */
	UCHAR ResVarIEs[MAX_VIE_LEN];
	UCHAR LastSsidLen;               /* the actual ssid length in used */
	CHAR LastSsid[MAX_LEN_OF_SSID]; /* NOT NULL-terminated */
	UCHAR LastBssid[MAC_ADDR_LEN];
#endif /* WPA_SUPPLICANT_SUPPORT */

	/*
		Transmitting segment
	*/
	UCHAR RxMcsSet[16];
	PSPOLL_FRAME PsPollFrame;
	HEADER_802_11 NullFrame;
	UAPSD_INFO UapsdInfo;

} APCLI_STRUCT, *PAPCLI_STRUCT;


typedef struct _AP_ADMIN_CONFIG {
	USHORT CapabilityInfo;
	/* Multiple SSID */
	UCHAR BssidNum;
	UCHAR MacMask;
	MULTISSID_STRUCT MBSSID[HW_BEACON_MAX_NUM];
	ULONG IsolateInterStaTrafficBTNBSSID;
#ifdef APCLI_SUPPORT
	UCHAR ApCliInfRunned;	/* Number of  ApClient interface which was running. value from 0 to MAX_APCLI_INTERFACE */
	UINT8 ApCliNum;
	BOOLEAN FlgApCliIsUapsdInfoUpdated;
	APCLI_STRUCT ApCliTab[MAX_APCLI_NUM];	/*AP-client */
#ifdef APCLI_AUTO_CONNECT_SUPPORT
	BOOLEAN ApCliAutoConnectRunning;
	BOOLEAN ApCliAutoConnectChannelSwitching;
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
#endif /* APCLI_SUPPORT */

#ifdef AP_PARTIAL_SCAN_SUPPORT
	BOOLEAN bPartialScanning;
#define DEFLAUT_PARTIAL_SCAN_CH_NUM 1		/* Must be move to other place */
	UINT8 PartialScanChannelNum; 		/* How many channels to scan each time */
	UINT8 LastPartialScanChannel;
#define DEFLAUT_PARTIAL_SCAN_BREAK_TIME	4	/* Period of partial scaning: unit: 100ms *//* Must be move to other place */
	UINT8 PartialScanBreakTime;		/* Period of partial scaning: unit: 100ms */
#endif /* AP_PARTIAL_SCAN_SUPPORT */

	/* for wpa */
	RALINK_TIMER_STRUCT CounterMeasureTimer;
	UCHAR CMTimerRunning;
	UCHAR BANClass3Data;
	LARGE_INTEGER aMICFailTime;
	LARGE_INTEGER PrevaMICFailTime;
	ULONG MICFailureCounter;
	RSSI_SAMPLE RssiSample;
	ULONG NumOfAvgRssiSample;
	BOOLEAN bAutoChannelAtBootup;	/* 0: disable, 1: enable */
	ChannelSel_Alg AutoChannelAlg;	/* Alg for selecting Channel */
#ifdef AP_SCAN_SUPPORT
	UINT32  ACSCheckTime;		/* Periodic timer to trigger Auto Channel Selection (unit: second) */
	UINT32  ACSCheckCount;		/* if  ACSCheckCount > ACSCheckTime, then do ACS check */
#endif /* AP_SCAN_SUPPORT */
	BOOLEAN bAvoidDfsChannel;	/* 0: disable, 1: enable */
	BOOLEAN bIsolateInterStaTraffic;
	BOOLEAN bHideSsid;

	/* temporary latch for Auto channel selection */
	ULONG ApCnt;			/* max RSSI during Auto Channel Selection period */
	UCHAR AutoChannel_Channel;	/* channel number during Auto Channel Selection */
	UCHAR current_channel_index;	/* current index of channel list */
	UCHAR AutoChannelSkipListNum;	/* number of rejected channel list */
	UCHAR AutoChannelSkipList[10];
	UCHAR DtimCount;		/* 0.. DtimPeriod-1 */
	UCHAR DtimPeriod;		/* default = 3 */
	UCHAR ErpIeContent;
	ULONG LastOLBCDetectTime;
	ULONG LastNoneHTOLBCDetectTime;
	ULONG LastScanTime;		/* Record last scan time for issue BSSID_SCAN_LIST */
	UCHAR LastSNR0;			/* last received BEACON's SNR */
	UCHAR LastSNR1;			/* last received BEACON's SNR for 2nd  antenna */
#ifdef DOT11N_SS3_SUPPORT
	UCHAR LastSNR2;			/* last received BEACON's SNR for 2nd  antenna */
#endif

#ifdef DOT1X_SUPPORT
	/* dot1x related parameter */
	UINT32 own_ip_addr;
	UINT32 retry_interval;
	UINT32 session_timeout_interval;
	UINT32 quiet_interval;
	UCHAR EAPifname[HW_BEACON_MAX_NUM][IFNAMSIZ];		/* indicate as the binding interface for EAP negotiation. */
	UCHAR EAPifname_len[HW_BEACON_MAX_NUM];
	UCHAR PreAuthifname[HW_BEACON_MAX_NUM][IFNAMSIZ];	/* indicate as the binding interface for WPA2 Pre-authentication. */
	UCHAR PreAuthifname_len[HW_BEACON_MAX_NUM];
#endif /* DOT1X_SUPPORT */

	/* EDCA parameters to be announced to its local BSS */
	EDCA_PARM BssEdcaParm;
	RALINK_TIMER_STRUCT ApQuickResponeForRateUpTimer;
	BOOLEAN ApQuickResponeForRateUpTimerRunning;

	/* Indicate the maximum idle timeout */
	UINT32 StaIdleTimeout;
	ULONG EntryLifeCheck;

#ifdef DOT11_N_SUPPORT
#ifdef GREENAP_SUPPORT
	UCHAR GreenAPLevel;
	BOOLEAN bGreenAPEnable;
	BOOLEAN bGreenAPActive;
#endif /* GREENAP_SUPPORT */
#endif /* DOT11_N_SUPPORT */

	UCHAR EntryClientCount;
	UCHAR ChangeTxOpClient;
} AP_ADMIN_CONFIG, *PAP_ADMIN_CONFIG;


#ifdef DOT11_N_SUPPORT
#ifdef GREENAP_SUPPORT
typedef enum _RT_GREEN_AP_LEVEL {
	GREENAP_11BGN_STAS = 0,
	GREENAP_ONLY_11BG_STAS,
	GREENAP_WITHOUT_ANY_STAS_CONNECT
} RT_GREEN_AP_LEVEL;
#endif /* DOT11_N_SUPPORT */
#endif /* GREENAP_SUPPORT */

/* ----------- end of AP ---------------------------- */
#endif /* CONFIG_AP_SUPPORT */

#ifdef BLOCK_NET_IF
typedef struct _BLOCK_QUEUE_ENTRY {
	BOOLEAN SwTxQueueBlockFlag;
	LIST_HEADER NetIfList;
} BLOCK_QUEUE_ENTRY, *PBLOCK_QUEUE_ENTRY;
#endif /* BLOCK_NET_IF */

struct wificonf {
	BOOLEAN bShortGI;
	BOOLEAN bGreenField;
};

typedef struct _RTMP_DEV_INFO_ {
	UCHAR chipName[16];
	RTMP_INF_TYPE infType;
} RTMP_DEV_INFO;

#ifdef DBG_DIAGNOSE
#define DIAGNOSE_TIME	10	/* 10 sec */

struct dbg_diag_info{
	USHORT TxDataCnt;	/* Tx total data count */
	USHORT TxFailCnt;
	USHORT RxDataCnt;	/* Rx Total Data count. */
	USHORT RxCrcErrCnt;

#ifdef DBG_TXQ_DEPTH
	/* TxSwQueue length in scale of 0, 1, 2, 3, 4, 5, 6, 7, >=8 */
	USHORT TxSWQueCnt[4][9];
#endif /* DBG_TXQ_DEPTH */

#ifdef DBG_TX_RING_DEPTH
	/* TxDesc queue length in scale of 0~14, >=15 */
	USHORT TxDescCnt[24];
#endif /* DBG_TX_RING_DEPTH */

#ifdef DBG_TX_AGG_CNT
	USHORT TxAggCnt;
	USHORT TxNonAggCnt;
	/* TxDMA APMDU Aggregation count in range from 0 to 15, in setp of 1. */
	USHORT TxAMPDUCnt[MAX_MCS_SET];
#endif /* DBG_TX_AGG_CNT */

#ifdef DBG_TX_MCS
	/* TxDate MCS Count in range from 0 to 15, step in 1 */
	USHORT TxMcsCnt_HT[MAX_MCS_SET];
#ifdef DOT11_VHT_AC
	UINT TxMcsCnt_VHT[MAX_VHT_MCS_SET];
#endif /* DOT11_VHT_AC */
#endif /* DBG_TX_MCS */

#ifdef DBG_RX_MCS
	USHORT RxCrcErrCnt_HT[MAX_MCS_SET];
	/* Rx HT MCS Count in range from 0 to 15, step in 1 */
	USHORT RxMcsCnt_HT[MAX_MCS_SET];
#ifdef DOT11_VHT_AC
	USHORT RxCrcErrCnt_VHT[MAX_VHT_MCS_SET];
	/* for VHT80MHz only, not calcuate 20/40 MHz packets */
	UINT RxMcsCnt_VHT[MAX_VHT_MCS_SET];
#endif /* DOT11_VHT_AC */
#endif /* DBG_RX_MCS */
};

typedef struct _RtmpDiagStrcut_ {	/* Diagnosis Related element */
	unsigned char inited;
	unsigned char qIdx;
	unsigned char ArrayStartIdx;
	unsigned char ArrayCurIdx;
	struct dbg_diag_info diag_info[DIAGNOSE_TIME];
} RtmpDiagStruct;
#endif /* DBG_DIAGNOSE */

#if defined(RTMP_INTERNAL_TX_ALC) || defined(RTMP_TEMPERATURE_COMPENSATION)
/*
	The number of channels for per-channel Tx power offset
*/
#define NUM_OF_CH_FOR_PER_CH_TX_PWR_OFFSET	14

/* The Tx power control using the internal ALC */
#ifdef RT8592
// TODO: shiang-6590, fix me for this ugly definition!
#define LOOKUP_TB_SIZE	45
#else
#define LOOKUP_TB_SIZE	33
#endif /* RT8592 */

typedef struct _TX_POWER_CONTROL {
	BOOLEAN bInternalTxALC; 	/* Internal Tx ALC */
	BOOLEAN bExtendedTssiMode; 	/* The extended TSSI mode (each channel has different Tx power if needed) */
	CHAR PerChTxPwrOffset[NUM_OF_CH_FOR_PER_CH_TX_PWR_OFFSET + 1]; /* Per-channel Tx power offset */
	CHAR idxTxPowerTable; 		/* The index of the Tx power table for ant0 */
	CHAR idxTxPowerTable2; 		/* The index of the Tx power table for ant1 */
	CHAR RF_TX_ALC; 		/* 3390: RF R12[4:0]: Tx0 ALC, 3352: RF R47[4:0]: Tx0 ALC, 5390: RF R49[5:0]: Tx0 ALC */
	CHAR MAC_PowerDelta; 		/* Tx power control over MAC 0x1314~0x1324 */
	CHAR MAC_PowerDelta2; 		/* Tx power control for Tx1 */
	CHAR TotalDeltaPower2; 		/* Tx power control for Tx1 */
#ifdef RTMP_TEMPERATURE_COMPENSATION
	int LookupTable[IEEE80211_BAND_NUMS][LOOKUP_TB_SIZE];
	int RefTemp[IEEE80211_BAND_NUMS];
	UCHAR TssiGain[IEEE80211_BAND_NUMS];
	/* Index offset, -7....25. */
	int LookupTableIndex;
#endif /* RTMP_TEMPERATURE_COMPENSATION */

} TX_POWER_CONTROL, *PTX_POWER_CONTROL;
#endif /* RTMP_INTERNAL_TX_ALC || RTMP_TEMPERATURE_COMPENSATION */

/* */
/* The entry of transmit power control over MAC */
/* */
typedef struct _TX_POWER_CONTROL_OVER_MAC_ENTRY {
	USHORT MACRegisterOffset;	/* MAC register offset */
	ULONG RegisterValue;	/* Register value */
} TX_POWER_CONTROL_OVER_MAC_ENTRY, *PTX_POWER_CONTROL_OVER_MAC_ENTRY;

/* */
/* The maximum registers of transmit power control */
/* */
#define MAX_TX_PWR_CONTROL_OVER_MAC_REGISTERS 5

/* */
/* The configuration of the transmit power control over MAC */
/* */
typedef struct _CONFIGURATION_OF_TX_POWER_CONTROL_OVER_MAC {
	UCHAR NumOfEntries;	/* Number of entries */
	TX_POWER_CONTROL_OVER_MAC_ENTRY TxPwrCtrlOverMAC[MAX_TX_PWR_CONTROL_OVER_MAC_REGISTERS];
} CONFIGURATION_OF_TX_POWER_CONTROL_OVER_MAC, *PCONFIGURATION_OF_TX_POWER_CONTROL_OVER_MAC;

/* */
/* The extension of the transmit power control over MAC */
/* */
typedef struct _TX_POWER_CONTROL_EXT_OVER_MAC {
	struct {
		ULONG TxPwrCfg0;	/* MAC 0x1314 */
		ULONG TxPwrCfg0Ext;	/* MAC 0x1390 */
		ULONG TxPwrCfg1;	/* MAC 0x1318 */
		ULONG TxPwrCfg1Ext;	/* MAC 0x1394 */
		ULONG TxPwrCfg2;	/* MAC 0x131C */
		ULONG TxPwrCfg2Ext;	/* MAC 0x1398 */
		ULONG TxPwrCfg3;	/* MAC 0x1320 */
		ULONG TxPwrCfg3Ext;	/* MAC 0x139C */
		ULONG TxPwrCfg4;	/* MAC 0x1324 */
		ULONG TxPwrCfg4Ext;	/* MAC 0x13A0 */
		ULONG TxPwrCfg5;	/* MAC 0x1384 */
		ULONG TxPwrCfg6;	/* MAC 0x1388 */
		ULONG TxPwrCfg7;	/* MAC 0x13D4 */
		ULONG TxPwrCfg8;	/* MAC 0x13D8 */
		ULONG TxPwrCfg9;	/* MAC 0x13DC */
	} BW20Over2Dot4G;
	struct {
		ULONG TxPwrCfg0;	/* MAC 0x1314 */
		ULONG TxPwrCfg0Ext;	/* MAC 0x1390 */
		ULONG TxPwrCfg1;	/* MAC 0x1318 */
		ULONG TxPwrCfg1Ext;	/* MAC 0x1394 */
		ULONG TxPwrCfg2;	/* MAC 0x131C */
		ULONG TxPwrCfg2Ext;	/* MAC 0x1398 */
		ULONG TxPwrCfg3;	/* MAC 0x1320 */
		ULONG TxPwrCfg3Ext;	/* MAC 0x139C */
		ULONG TxPwrCfg4;	/* MAC 0x1324 */
		ULONG TxPwrCfg4Ext;	/* MAC 0x13A0 */
		ULONG TxPwrCfg5;	/* MAC 0x1384 */
		ULONG TxPwrCfg6;	/* MAC 0x1388 */
		ULONG TxPwrCfg7;	/* MAC 0x13D4 */
		ULONG TxPwrCfg8;	/* MAC 0x13D8 */
		ULONG TxPwrCfg9;	/* MAC 0x13DC */
	} BW40Over2Dot4G;
	struct {
		ULONG TxPwrCfg0;	/* MAC 0x1314 */
		ULONG TxPwrCfg0Ext;	/* MAC 0x1390 */
		ULONG TxPwrCfg1;	/* MAC 0x1318 */
		ULONG TxPwrCfg1Ext;	/* MAC 0x1394 */
		ULONG TxPwrCfg2;	/* MAC 0x131C */
		ULONG TxPwrCfg2Ext;	/* MAC 0x1398 */
		ULONG TxPwrCfg3;	/* MAC 0x1320 */
		ULONG TxPwrCfg3Ext;	/* MAC 0x139C */
		ULONG TxPwrCfg4;	/* MAC 0x1324 */
		ULONG TxPwrCfg4Ext;	/* MAC 0x13A0 */
		ULONG TxPwrCfg5;	/* MAC 0x1384 */
		ULONG TxPwrCfg6;	/* MAC 0x1388 */
		ULONG TxPwrCfg7;	/* MAC 0x13D4 */
		ULONG TxPwrCfg8;	/* MAC 0x13D8 */
		ULONG TxPwrCfg9;	/* MAC 0x13DC */
	} BW20Over5G;
	struct {
		ULONG TxPwrCfg0;	/* MAC 0x1314 */
		ULONG TxPwrCfg0Ext;	/* MAC 0x1390 */
		ULONG TxPwrCfg1;	/* MAC 0x1318 */
		ULONG TxPwrCfg1Ext;	/* MAC 0x1394 */
		ULONG TxPwrCfg2;	/* MAC 0x131C */
		ULONG TxPwrCfg2Ext;	/* MAC 0x1398 */
		ULONG TxPwrCfg3;	/* MAC 0x1320 */
		ULONG TxPwrCfg3Ext;	/* MAC 0x139C */
		ULONG TxPwrCfg4;	/* MAC 0x1324 */
		ULONG TxPwrCfg4Ext;	/* MAC 0x13A0 */
		ULONG TxPwrCfg5;	/* MAC 0x1384 */
		ULONG TxPwrCfg6;	/* MAC 0x1388 */
		ULONG TxPwrCfg7;	/* MAC 0x13D4 */
		ULONG TxPwrCfg8;	/* MAC 0x13D8 */
		ULONG TxPwrCfg9;	/* MAC 0x13DC */
	} BW40Over5G;
} TX_POWER_CONTROL_EXT_OVER_MAC, *PTX_POWER_CONTROL_EXT_OVER_MAC;

/* For Wake on Wireless LAN */
#if (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT)
typedef struct _WOW_CFG_STRUCT {
	BOOLEAN bEnable;		/* Enable WOW function*/
	BOOLEAN bWOWFirmware;		/* Enable WOW function, trigger to reload WOW-support firmware */
	BOOLEAN bInBand;		/* use in-band signal to wakeup system */
	UINT8 nSelectedGPIO;		/* Side band signal to wake up system */
	UINT8 nDelay;			/* Delay number is multiple of 3 secs, and it used to postpone the WOW function */
	UINT8 nHoldTime;		/* GPIO puls hold time, unit: 10ms */
} WOW_CFG_STRUCT, *PWOW_CFG_STRUCT;
#endif /* (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) */

#ifdef NEW_WOW_SUPPORT
typedef enum {
	WOW_PKT_TO_HOST,
	WOW_PKT_TO_ANDES
} WOW_PKT_FLOW_T;

typedef enum {
	WOW_WAKEUP_BY_PCIE,
	WOW_WAKEUP_BY_USB,
	WOW_WAKEUP_BY_GPIO
} WOW_WAKEUP_METHOD_T;

typedef enum {
	WOW_ENABLE = 1,
	WOW_TRAFFIC = 3,
	WOW_WAKEUP = 4
} WOW_FEATURE_T;

typedef enum {
	WOW_MASK_CFG = 1,
	WOW_SEC_CFG,
	WOW_INFRA_CFG,
	WOW_P2P_CFG,
} WOW_CONFIG_T;

enum {
	WOW_MAGIC_PKT,
	WOW_BITMAP,
	WOW_IPV4_TCP_SYNC,
	WOW_IPV6_TCP_SYNC
};

typedef struct NEW_WOW_MASK_CFG_STRUCT {
	UINT32 Config_Type;
	UINT32 Function_Enable;
	UINT32 Detect_Mask;
	UINT32 Event_Mask;
} NEW_WOW_MASK_CFG_STRUCT, PNEW_WOW_MASK_CFG_STRUCT;

typedef struct NEW_WOW_SEC_CFG_STRUCT {
	UINT32 Config_Type;
	UINT32 WPA_Ver;
	UCHAR PTK[64];
	UCHAR R_COUNTER[8];
	UCHAR Key_Id;
	UCHAR Cipher_Alg;
	UCHAR WCID;
	UCHAR Group_Cipher;
} NEW_WOW_SEC_CFG_STRUCT, PNEW_WOW_SEC_CFG_STRUCT;

typedef struct NEW_WOW_INFRA_CFG_STRUCT {
	UINT32 Config_Type;
	UCHAR STA_MAC[6];
	UCHAR AP_MAC[6];
	UINT32 AP_Status;
} NEW_WOW_INFRA_CFG_STRUCT, PNEW_WOW_INFRA_CFG_STRUCT;

typedef struct _NEW_WOW_P2P_CFG_STRUCT {
	UINT32 Config_Type;
	UCHAR GO_MAC[6];
	UCHAR CLI_MAC[6];
	UINT32 P2P_Status;
} NEW_WOW_P2P_CFG_STRUCT, *PNEW_WOW_P2P_CFG_STRUCT;

typedef struct _NEW_WOW_PARAM_STRUCT {
	UINT32 Parameter;
	UINT32 Value;
} NEW_WOW_PARAM_STRUCT, *PNEW_WOW_PARAM_STRUCT;
#endif /* NEW_WOW_SUPPORT */

/*
	Packet drop reason code
*/
typedef enum{
	PKT_ATE_ON = 1 << 8,
	PKT_RADAR_ON = 2 << 8,
	PKT_RRM_QUIET = 3 << 8,
	PKT_TX_STOP = 4 <<8,
	PKT_TX_JAM = 5 << 8,

	PKT_NETDEV_DOWN = 6 < 8,
	PKT_NETDEV_NO_MATCH = 7 << 8,
	PKT_NOT_ALLOW_SEND = 8 << 8,

	PKT_INVALID_DST = 9<< 8,
	PKT_INVALID_SRC = 10 << 8,
	PKT_INVALID_PKT_DATA = 11 << 8,
	PKT_INVALID_PKT_LEN = 12 << 8,
	PKT_INVALID_ETH_TYPE = 13 << 8,
	PKT_INVALID_TXBLK_INFO = 14 << 8,
	PKT_INVALID_SW_ENCRYPT = 15 << 8,
	PKT_INVALID_PKT_TYPE = 16 << 8,
	PKT_INVALID_PKT_MIC = 17 << 8,

	PKT_PORT_NOT_SECURE = 18 << 8,
	PKT_TSPEC_NO_MATCH  = 19 << 8,
	PKT_NO_ASSOCED_STA = 20 << 8,
	PKT_INVALID_MAC_ENTRY = 21 << 8,

	PKT_TX_QUE_FULL = 22 << 8,
	PKT_TX_QUE_ADJUST = 23<<8,

	PKT_PS_QUE_TIMEOUT = 24 <<8,
	PKT_PS_QUE_CLEAN = 25 << 8,
	PKT_MCAST_PS_QUE_FULL = 26 << 8,
	PKT_UCAST_PS_QUE_FULL = 27 << 8,

	PKT_RX_EAPOL_SANITY_FAIL = 28 <<8,
	PKT_RX_NOT_TO_KERNEL = 29 << 8,
	PKT_RX_MESH_SIG_FAIL = 30 << 8,
	PKT_APCLI_FAIL = 31 << 8,
	PKT_ZERO_DATA = 32 <<8,
	PKT_SW_DECRYPT_FAIL = 33 << 8,
	PKT_TX_SW_ENC_FAIL = 34 << 8,

	PKT_ACM_FAIL = 35 << 8,
	PKT_IGMP_GRP_FAIL = 36 << 8,
	PKT_MGMT_FAIL = 37 << 8,
	PKT_AMPDU_OUT_ORDER = 38 << 8,
	PKT_UAPSD_EOSP = 39 << 8,
	PKT_UAPSD_Q_FULL = 40 << 8,

	PKT_DRO_REASON_MAX = 41,
} PKT_DROP_REASON;

/* Packet drop Direction code */
typedef enum{
	PKT_TX = 0,
	PKT_RX = 1 << 31,
} PKT_DROP_DIECTION;

typedef struct _BBP_RESET_CTL
{
#define BBP_RECORD_NUM	48

	REG_PAIR BBPRegDB[BBP_RECORD_NUM];
	BOOLEAN AsicCheckEn;
} BBP_RESET_CTL, *PBBP_RESET_CTL;


typedef struct _SCAN_CTRL_{
	UCHAR ScanType;
	UCHAR BssType;
	UCHAR Channel;
	UCHAR SsidLen;
	CHAR Ssid[MAX_LEN_OF_SSID];
	UCHAR Bssid[MAC_ADDR_LEN];

#ifdef CONFIG_AP_SUPPORT
	RALINK_TIMER_STRUCT APScanTimer;
#endif /* CONFIG_AP_SUPPORT */
} SCAN_CTRL;


#ifdef RT_CFG80211_SUPPORT
typedef struct _CFG80211_VIF_DEV
{
	struct _CFG80211_VIF_DEV *pNext;
	BOOLEAN isMainDev;
	UINT32 devType;
	PNET_DEV net_dev;
	UCHAR CUR_MAC[MAC_ADDR_LEN];

	/* ProbeReq Frame */
	BOOLEAN Cfg80211RegisterProbeReqFrame;
	CHAR Cfg80211ProbeReqCount;

	/* Action Frame */
	BOOLEAN Cfg80211RegisterActionFrame;
	CHAR Cfg80211ActionCount;
} CFG80211_VIF_DEV, *PCFG80211_VIF_DEV;

typedef struct _CFG80211_VIF_DEV_SET
{
#define MAX_CFG80211_VIF_DEV_NUM  2

	BOOLEAN inUsed;
	UINT32 vifDevNum;
	LIST_HEADER vifDevList;
	BOOLEAN isGoingOn; 		/* To check any vif in list */
} CFG80211_VIF_DEV_SET;

/* TxMmgt Related */
typedef struct _CFG80211_TX_PACKET
{
	struct _CFG80211_TX_PACKET *pNext;
	UINT32 TxStatusSeq;		/* TxMgmt Packet ID from sequence */
	UCHAR *pTxStatusBuf;		/* TxMgmt Packet buffer content */
	UINT32 TxStatusBufLen;		/* TxMgmt Packet buffer Length */

} CFG80211_TX_PACKET, *PCFG80211_TX_PACKET;

/* CFG80211 Total CTRL Point */
typedef struct _CFG80211_CONTROL
{
	BOOLEAN FlgCfg8021Disable2040Scan;
	BOOLEAN FlgCfg80211Scanning; 	/* Record it When scanReq from wpa_supplicant */
	BOOLEAN FlgCfg80211Connecting;	/* Record it When ConnectReq from wpa_supplicant*/
	BOOLEAN FlgCfg80211ApBeaconUpdate;
	/* Scan Related */
	UINT32 *pCfg80211ChanList; 	/* the channel list from from wpa_supplicant */
	UCHAR Cfg80211ChanListLen; 	/* channel list length */
	UCHAR Cfg80211CurChanIndex; 	/* current index in channel list when driver in scanning */

	UCHAR *pExtraIe;  		/* Carry on Scan action from supplicant */
	UINT ExtraIeLen;

	UCHAR Cfg_pending_Ssid[MAX_LEN_OF_SSID+1]; /* Record the ssid, When ScanTable Full */
   	UCHAR Cfg_pending_SsidLen;

	/* ROC Related */
	RALINK_TIMER_STRUCT Cfg80211RocTimer;
	CMD_RTPRIV_IOCTL_80211_CHAN Cfg80211ChanInfo;
	BOOLEAN Cfg80211RocTimerInit;
	BOOLEAN Cfg80211RocTimerRunning;

	/* Tx_Mmgt Related */
	UINT32 TxStatusSeq;			/* TxMgmt Packet ID from sequence */
	UCHAR *pTxStatusBuf;			/* TxMgmt Packet buffer content */
	UINT32 TxStatusBufLen;			/* TxMgmt Packet buffer Length */
	BOOLEAN TxStatusInUsed;
	LIST_HEADER cfg80211TxPacketList;

	/* P2P Releated*/
	UCHAR P2PCurrentAddress[MAC_ADDR_LEN];	/* User changed MAC address */
	BOOLEAN isCfgDeviceInP2p; 		/* For BaseRate 6 */

	/* MainDevice Info. */
	CFG80211_VIF_DEV cfg80211MainDev;

	/* For add_virtual_intf */
	CFG80211_VIF_DEV_SET Cfg80211VifDevSet;

#ifdef RT_CFG80211_P2P_SUPPORT
	BOOLEAN bP2pCliPmEnable;
	BOOLEAN bPreKeepSlient;
	BOOLEAN bKeepSlient;
	UCHAR MyGOwcid;
	UCHAR NoAIndex;
	UCHAR CTWindows; /* CTWindows and OppPS parameter field */
	P2PCLIENT_NOA_SCHEDULE GONoASchedule;
	RALINK_TIMER_STRUCT P2pCTWindowTimer;
	RALINK_TIMER_STRUCT P2pSwNoATimer;
	RALINK_TIMER_STRUCT P2pPreAbsenTimer;
	UCHAR P2pSupRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR P2pSupRateLen;
	UCHAR P2pExtRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR P2pExtRateLen;

#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
	/* Dummy P2P Device for ANDROID JB */
	PNET_DEV dummy_p2p_net_dev;
	BOOLEAN flg_cfg_dummy_p2p_init;
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */

#ifdef RT_CFG80211_P2P_SINGLE_DEVICE
	ULONG P2POpStatusFlags; /* P2P_CLI_UP / P2P_GO_UP*/
#endif /* RT_CFG80211_P2P_SINGLE_DEVICE */
#endif /* RT_CFG80211_P2P_SUPPORT */

	/* In AP Mode */
	UINT8 isCfgInApMode;    /* Is any one Device in AP Mode */
	UCHAR *beacon_tail_buf; /* Beacon buf from upper layer */
	UINT32 beacon_tail_len;
	UCHAR *pCfg80211ExtraIeAssocRsp;
	UINT32 Cfg80211ExtraIeAssocRspLen;

	/* TODO: need fix it */
	UCHAR Cfg80211_Alpha2[2];
} CFG80211_CTRL, *PCFG80211_CTRL;
#endif /* RT_CFG80211_SUPPORT */

typedef struct rtmp_phy_ctrl{
	UINT8 rf_band_cap;

#ifdef CONFIG_AP_SUPPORT
#ifdef AP_QLOAD_SUPPORT
	UINT8 FlgQloadEnable;	/* 1: any BSS WMM is enabled */
	ULONG QloadUpTimeLast;	/* last up time */
	UINT8 QloadChanUtil;	/* last QBSS Load, unit: us */
	UINT32 QloadChanUtilTotal;	/* current QBSS Load Total */
	UINT8 QloadChanUtilBeaconCnt;	/* 1~100, default: 50 */
	UINT8 QloadChanUtilBeaconInt;	/* 1~100, default: 50 */
	UINT32 QloadLatestChannelBusyTimePri;
	UINT32 QloadLatestChannelBusyTimeSec;

	/*
	   ex: For 100ms beacon interval,
	   if the busy time in last TBTT is smaller than 5ms, QloadBusyCount[0] ++;
	   if the busy time in last TBTT is between 5 and 10ms, QloadBusyCount[1] ++;
	   ......
	   if the busy time in last TBTT is larger than 95ms, QloadBusyCount[19] ++;

	   Command: "iwpriv ra0 qload show".
	 */

/* provide busy time statistics for every TBTT */
#define QLOAD_FUNC_BUSY_TIME_STATS

/* provide busy time alarm mechanism */
/* use the function to avoid to locate in some noise environments */
#define QLOAD_FUNC_BUSY_TIME_ALARM

#ifdef QLOAD_FUNC_BUSY_TIME_STATS
#define QLOAD_BUSY_INTERVALS	20	/* partition TBTT to QLOAD_BUSY_INTERVALS */
	/* for primary channel & secondary channel */
	UINT32 QloadBusyCountPri[QLOAD_BUSY_INTERVALS];
	UINT32 QloadBusyCountSec[QLOAD_BUSY_INTERVALS];
#endif /* QLOAD_FUNC_BUSY_TIME_STATS */

#ifdef QLOAD_FUNC_BUSY_TIME_ALARM
#define QLOAD_DOES_ALARM_OCCUR(pAd)	(pAd->phy_ctrl.FlgQloadAlarmIsSuspended == TRUE)
#define QLOAD_ALARM_EVER_OCCUR(pAd) (pAd->phy_ctrl.QloadAlarmNumber > 0)
	BOOLEAN FlgQloadAlarmIsSuspended;	/* 1: suspend */

	UINT8 QloadAlarmBusyTimeThreshold;	/* unit: 1/100 */
	UINT8 QloadAlarmBusyNumThreshold;	/* unit: 1 */
	UINT8 QloadAlarmBusyNum;
	UINT8 QloadAlarmDuration;		/* unit: TBTT */

	UINT32 QloadAlarmNumber;		/* total alarm times */
	BOOLEAN FlgQloadAlarm;			/* 1: alarm occurs */

	/* speed up use */
	UINT32 QloadTimePeriodLast;
	UINT32 QloadBusyTimeThreshold;
#else

#define QLOAD_DOES_ALARM_OCCUR(pAd)	0
#endif /* QLOAD_FUNC_BUSY_TIME_ALARM */

#endif /* AP_QLOAD_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
} RTMP_PHY_CTRL;


/*
	The miniport adapter structure
*/
struct _RTMP_ADAPTER {
	void *OS_Cookie;	/* save specific structure relative to OS */
	PNET_DEV net_dev;
	spinlock_t irq_lock;

	/*======Cmd Thread in PCI/RBUS/USB */
	CmdQ CmdQ;
	spinlock_t CmdQLock;	/* CmdQLock spinlock */
	RTMP_OS_TASK cmdQTask;

#ifdef RTMP_MAC_USB
/**********************************************************/
/*      USB related parameters                                                         */
/**********************************************************/
/*	struct usb_config_descriptor *config; */
	void *config;
	UINT NumberOfPipes;
	USHORT BulkOutMaxPacketSize;
	USHORT BulkInMaxPacketSize;
	UINT8 BulkOutEpAddr[6];
	UINT8 BulkInEpAddr[2];

	/*======Control Flags */
	ULONG BulkFlags;
	BOOLEAN bUsbTxBulkAggre;	/* Flags for bulk out data priority */

	/*======Semaphores (event) */
	RTMP_OS_SEM UsbVendorReq_semaphore;
	RTMP_OS_SEM reg_atomic;
	RTMP_OS_SEM hw_atomic;
	RTMP_OS_SEM mcu_atomic;
	RTMP_OS_SEM tssi_lock;
	PVOID UsbVendorReqBuf;
/*	wait_queue_head_t	 *wait; */
	void *wait;

#ifdef RALINK_ATE
	/* lock for ATE */
	spinlock_t GenericLock;	/* ATE Tx/Rx generic spinlock */
#endif

#endif /* RTMP_MAC_USB */

	/* resource for software backlog queues */
	spinlock_t page_lock;	/* for nat speedup by bruce */

/*********************************************************/
/*      Both PCI/USB related parameters                  */
/*********************************************************/
	/*RTMP_DEV_INFO chipInfo; */
	RTMP_INF_TYPE infType;

/*********************************************************/
/*      Driver Mgmt related parameters                   */
/*********************************************************/
	/* OP mode: either AP or STA */
	UCHAR OpMode;		/* OPMODE_STA, OPMODE_AP */
	struct wifi_dev *wdev_list[WDEV_NUM_MAX];
	RTMP_OS_TASK mlmeTask;
#ifdef RTMP_TIMER_TASK_SUPPORT
	/* If you want use timer task to handle the timer related jobs, enable this. */
	RTMP_TIMER_TASK_QUEUE TimerQ;
	spinlock_t TimerQLock;
	RTMP_OS_TASK timerTask;
#endif /* RTMP_TIMER_TASK_SUPPORT */

/*********************************************************/
/*      Tx related parameters                            */
/*********************************************************/
	BOOLEAN DeQueueRunning[NUM_OF_TX_RING];	/* for ensuring RTUSBDeQueuePacket get call once */
	spinlock_t DeQueueLock[NUM_OF_TX_RING];

#ifdef RTMP_MAC_USB
	/* Data related context and AC specified, 4 AC supported */
	spinlock_t BulkOutLock[6];	/* BulkOut spinlock for 4 ACs */
	spinlock_t MLMEBulkOutLock;	/* MLME BulkOut lock */

	HT_TX_CONTEXT TxContext[NUM_OF_TX_RING];
	spinlock_t TxContextQueueLock[NUM_OF_TX_RING];	/* TxContextQueue spinlock */

	/* 4 sets of Bulk Out index and pending flag */
	/*
	   array size of NextBulkOutIndex must be larger than or equal to 5;
	   Or BulkOutPending[0] will be overwrited in NICInitTransmit().
	 */
	UCHAR NextBulkOutIndex[NUM_OF_TX_RING];	/* only used for 4 EDCA bulkout pipe */

	BOOLEAN BulkOutPending[6];	/* used for total 6 bulkout pipe */
	UCHAR bulkResetPipeid;
	BOOLEAN MgmtBulkPending;
	ULONG bulkResetReq[6];
#ifdef INF_AMAZON_SE
	ULONG BulkOutDataSizeCount[NUM_OF_TX_RING];
	BOOLEAN BulkOutDataFlag[NUM_OF_TX_RING];
	ULONG BulkOutDataSizeLimit[NUM_OF_TX_RING];
	UCHAR RunningQueueNoCount;
	UCHAR LastRunningQueueNo;
#endif /* #ifdef INF_AMAZON_SE */

#ifdef CONFIG_STA_SUPPORT
	USHORT CountDowntoPsm;
#endif /* CONFIG_STA_SUPPORT */

#endif /* RTMP_MAC_USB */

	/* resource for software backlog queues */
	QUEUE_HEADER TxSwQueue[NUM_OF_TX_RING];	/* 4 AC + 1 HCCA */
	spinlock_t TxSwQueueLock[NUM_OF_TX_RING];	/* TxSwQueue spinlock */

	/* Maximum allowed tx software Queue length */
	UINT32 TxSwQMaxLen;
	RTMP_DMABUF MgmtDescRing;	/* Shared memory for MGMT descriptors */
	RTMP_MGMT_RING MgmtRing;
	spinlock_t MgmtRingLock;	/* Prio Ring spinlock */
	UCHAR LastMCUCmd;

/*********************************************************/
/*      Rx related parameters                            */
/*********************************************************/

#ifdef RTMP_MAC_USB
	RX_CONTEXT RxContext[RX_RING_SIZE];	/* 1 for redundant multiple IRP bulk in. */
	spinlock_t BulkInLock;	/* BulkIn spinlock for 4 ACs */
	spinlock_t CmdRspLock;
	UCHAR PendingRx;	/* The Maximum pending Rx value should be       RX_RING_SIZE. */
	UCHAR NextRxBulkInIndex;	/* Indicate the current RxContext Index which hold by Host controller. */
	UCHAR NextRxBulkInReadIndex;	/* Indicate the current RxContext Index which driver can read & process it. */
	ULONG NextRxBulkInPosition;	/* Want to contatenate 2 URB buffer while 1st is bulkin failed URB. This Position is 1st URB TransferLength. */
	ULONG TransferBufferLength;	/* current length of the packet buffer */
	ULONG ReadPosition;	/* current read position in a packet buffer */

	CMD_RSP_CONTEXT CmdRspEventContext;
#endif /* RTMP_MAC_USB */

	/* RX re-assembly buffer for fragmentation */
	FRAGMENT_FRAME FragFrame;	/* Frame storage for fragment frame */

/***********************************************************/
/*      ASIC related parameters                            */
/***********************************************************/
	RTMP_CHIP_OP chipOps;
	RTMP_CHIP_CAP chipCap;
	struct phy_ops *phy_op;
	struct hw_setting hw_cfg;

	UINT32 MACVersion;	/* MAC version. Record rt2860C(0x28600100) or rt2860D (0x28600101).. */
	UINT32 ChipID;
	UINT16 ChipId; 		/* Chip version. Read from EEPROM 0x00 to identify RT5390H */
	int dev_idx;

	/* --------------------------- */
	/* E2PROM                                     */
	/* --------------------------- */
	enum EEPROM_STORAGE_TYPE eeprom_type;
	ULONG EepromVersion;	/* byte 0: version, byte 1: revision, byte 2~3: unused */
	ULONG FirmwareVersion;	/* byte 0: Minor version, byte 1: Major version, otherwise unused. */
	USHORT EEPROMDefaultValue[NUM_EEPROM_BBP_PARMS];
	UCHAR EEPROMAddressNum;	/* 93c46=6  93c66=8 */
	BOOLEAN EepromAccess;
	UCHAR EFuseTag;

#ifdef RTMP_EFUSE_SUPPORT
	BOOLEAN bUseEfuse;
#endif /* RTMP_EFUSE_SUPPORT */

	UCHAR EEPROMImage[MAX_EEPROM_BUFFER_SIZE];
	UCHAR E2pAccessMode; /* Used to identify flash, efuse, eeprom or bin from start-up */

#ifdef RTMP_FLASH_SUPPORT
	UCHAR *eebuf;
	UINT32 flash_offset;
#endif /* RTMP_FLASH_SUPPORT */

	EEPROM_ANTENNA_STRUC Antenna;	/* Since ANtenna definition is different for a & g. We need to save it for future reference. */
	EEPROM_NIC_CONFIG2_STRUC NicConfig2;
#if defined(BT_COEXISTENCE_SUPPORT) || defined(RT3290) || defined(RT8592) || defined(MT76x2)
	EEPROM_NIC_CONFIG3_STRUC NicConfig3;
#endif /* defined(BT_COEXISTENCE_SUPPORT) || defined(RT3290) || defined(RT8592) */

	/* --------------------------- */
	/* BBP Control                               */
	/* --------------------------- */
#if defined(RTMP_BBP) || defined(RALINK_ATE)
	// TODO: shiang-6590, remove "defined(RALINK_ATE)" after ATE has fully re-organized!
	UCHAR BbpWriteLatch[MAX_BBP_ID + 1];	/* record last BBP register value written via BBP_IO_WRITE/BBP_IO_WRITE_VY_REG_ID */
	UCHAR Bbp94;
#endif /* defined(RTMP_BBP) || defined(RALINK_ATE) */

	CHAR BbpRssiToDbmDelta;	/* change from UCHAR to CHAR for high power */
	BBP_R66_TUNING BbpTuning;

	/* ---------------------------- */
	/* RFIC control                                 */
	/* ---------------------------- */
	struct rtmp_phy_ctrl phy_ctrl;

	UCHAR RfIcType;		/* RFIC_xxx */
	UCHAR RfFreqOffset;	/* Frequency offset for channel switching */
	RTMP_RF_REGS LatchRfRegs;	/* latch th latest RF programming value since RF IC doesn't support READ */

	/* This soft Rx Antenna Diversity mechanism is used only when user set */
	/* RX Antenna = DIVERSITY ON */
	SOFT_RX_ANT_DIVERSITY RxAnt;

	/* ---------------------------- */
	/* TxPower control                           */
	/* ---------------------------- */
	CHANNEL_TX_POWER TxPower[MAX_NUM_OF_CHANNELS];	/* Store Tx power value for all channels. */
	CHANNEL_TX_POWER ChannelList[MAX_NUM_OF_CHANNELS];	/* list all supported channels for site survey */

	UCHAR ChannelListNum;	/* number of channel in ChannelList[] */
	BOOLEAN BbpForCCK;
	ULONG Tx20MPwrCfgABand[MAX_TXPOWER_ARRAY_SIZE];
	ULONG Tx20MPwrCfgGBand[MAX_TXPOWER_ARRAY_SIZE];
	ULONG Tx40MPwrCfgABand[MAX_TXPOWER_ARRAY_SIZE];
	ULONG Tx40MPwrCfgGBand[MAX_TXPOWER_ARRAY_SIZE];
#ifdef DOT11_VHT_AC
	ULONG Tx80MPwrCfgABand[MAX_TXPOWER_ARRAY_SIZE]; // Per-rate Tx power control for VHT BW80 (5GHz only)
	BOOLEAN force_vht_op_mode;
#endif

	BOOLEAN bAutoTxAgcA;	/* Enable driver auto Tx Agc control */
	UCHAR TssiRefA;		/* Store Tssi reference value as 25 temperature. */
	UCHAR TssiPlusBoundaryA[5];	/* Tssi boundary for increase Tx power to compensate. */
	UCHAR TssiMinusBoundaryA[5];	/* Tssi boundary for decrease Tx power to compensate. */
	UCHAR TxAgcStepA;	/* Store Tx TSSI delta increment / decrement value */
	CHAR TxAgcCompensateA;	/* Store the compensation (TxAgcStep * (idx-1)) */

	BOOLEAN bAutoTxAgcG;	/* Enable driver auto Tx Agc control */
	UCHAR TssiRefG;		/* Store Tssi reference value as 25 temperature. */
	UCHAR TssiPlusBoundaryG[5];	/* Tssi boundary for increase Tx power to compensate. */
	UCHAR TssiMinusBoundaryG[5];	/* Tssi boundary for decrease Tx power to compensate. */
	UCHAR TxAgcStepG;	/* Store Tx TSSI delta increment / decrement value */
	CHAR TxAgcCompensateG;	/* Store the compensation (TxAgcStep * (idx-1)) */
#if defined(RTMP_INTERNAL_TX_ALC) || defined(RTMP_TEMPERATURE_COMPENSATION)
	TX_POWER_CONTROL TxPowerCtrl;	/* The Tx power control using the internal ALC */
	CHAR curr_temp;
	CHAR rx_temp_base[2];	/* initial VGA value for chain 0/1,  used for base of rx temp compensation power base */
#endif

#ifdef THERMAL_PROTECT_SUPPORT
	BOOLEAN force_one_tx_stream;
	INT32 last_thermal_pro_temp;
	INT32 thermal_pro_criteria;
#endif

#ifdef RTMP_FREQ_CALIBRATION_SUPPORT
	FREQUENCY_CALIBRATION_CONTROL FreqCalibrationCtrl;	/* The frequency calibration control */
#endif

	signed char BGRssiOffset[3]; /* Store B/G RSSI #0/1/2 Offset value on EEPROM 0x46h */
	signed char ARssiOffset[3]; /* Store A RSSI 0/1/2 Offset value on EEPROM 0x4Ah */

	CHAR BLNAGain;		/* Store B/G external LNA#0 value on EEPROM 0x44h */
	CHAR ALNAGain0;		/* Store A external LNA#0 value for ch36~64 */
	CHAR ALNAGain1;		/* Store A external LNA#1 value for ch100~128 */
	CHAR ALNAGain2;		/* Store A external LNA#2 value for ch132~165 */
	UCHAR TxMixerGain24G;	/* Tx mixer gain value from EEPROM to improve Tx EVM / Tx DAC, 2.4G */
	UCHAR TxMixerGain5G;

	/* ---------------------------- */
	/* MAC control                                 */
	/* ---------------------------- */

	UCHAR wmm_cw_min; /* CW_MIN_IN_BITS, actual CwMin = 2^CW_MIN_IN_BITS - 1 */
	UCHAR wmm_cw_max; /* CW_MAX_IN_BITS, actual CwMax = 2^CW_MAX_IN_BITS - 1 */

#ifdef RT8592
	CHAR bw_cal[3];	// bw cal value for RF_R32, 0:20M, 1:40M, 2:80M
// TODO: shiang-6590, temporary get from windows and need to revise it!!
	/* IQ Calibration */
	UCHAR IQGainTx[3][4];
	UCHAR IQPhaseTx[3][4];
	USHORT IQControl;
#endif /* RT8592 */

#if defined(RT3290) || defined(RLT_MAC)

#ifdef RTMP_MAC_USB
//	RTMP_OS_SEM WlanEnLock;
#endif

	WLAN_FUN_CTRL_STRUC WlanFunCtrl;
#endif /* defined(RT3290) || defined(RLT_MAC) */

/*****************************************************************************************/
/*      802.11 related parameters                                                        */
/*****************************************************************************************/
	/* outgoing BEACON frame buffer and corresponding TXD */
	TXWI_STRUC BeaconTxWI;
	PUCHAR BeaconBuf;
	USHORT BeaconOffset[HW_BEACON_MAX_NUM];

	/* pre-build PS-POLL and NULL frame upon link up. for efficiency purpose. */
#ifdef CONFIG_STA_SUPPORT
	PSPOLL_FRAME PsPollFrame;
#endif /* CONFIG_STA_SUPPORT */
	HEADER_802_11 NullFrame;

#ifdef RTMP_MAC_USB
	TX_CONTEXT NullContext;
	TX_CONTEXT PsPollContext;
#endif /* RTMP_MAC_USB */

#ifdef UAPSD_SUPPORT
	spinlock_t UAPSDEOSPLock;	/* EOSP frame access lock use */
	BOOLEAN bAPSDFlagSPSuspend;	/* 1: SP is suspended; 0: SP is not */
#endif /* UAPSD_SUPPORT */

/*=========AP=========== */
#ifdef CONFIG_AP_SUPPORT
	/* ----------------------------------------------- */
	/* AP specific configuration & operation status */
	/* used only when pAd->OpMode == OPMODE_AP */
	/* ----------------------------------------------- */
	AP_ADMIN_CONFIG ApCfg;	/* user configuration when in AP mode */
	AP_MLME_AUX ApMlmeAux;

#ifdef MBSS_SUPPORT
	BOOLEAN FlgMbssInit;
#endif

#ifdef APCLI_SUPPORT
	BOOLEAN flg_apcli_init;
#endif

/*#ifdef AUTO_CH_SELECT_ENHANCE */
	PBSSINFO pBssInfoTab;
	PCHANNELINFO pChannelInfo;
/*#endif // AUTO_CH_SELECT_ENHANCE */

#endif /* CONFIG_AP_SUPPORT */

/*=======STA=========== */
#ifdef CONFIG_STA_SUPPORT
	/* ----------------------------------------------- */
	/* STA specific configuration & operation status */
	/* used only when pAd->OpMode == OPMODE_STA */
	/* ----------------------------------------------- */
	STA_ADMIN_CONFIG StaCfg;	/* user desired settings */
	STA_ACTIVE_CONFIG StaActive;	/* valid only when ADHOC_ON(pAd) || INFRA_ON(pAd) */
	CHAR nickname[IW_ESSID_MAX_SIZE + 1];	/* nickname, only used in the iwconfig i/f */
	NDIS_MEDIA_STATE PreMediaState;
#endif /* CONFIG_STA_SUPPORT */

/*=======Common=========== */
	enum RATE_ADAPT_ALG rateAlg;		/* Rate adaptation algorithm */

	NDIS_MEDIA_STATE IndicateMediaState;	/* Base on Indication state, default is NdisMediaStateDisConnected */

#ifdef PROFILE_STORE
	RTMP_OS_TASK WriteDatTask;
	BOOLEAN bWriteDat;
#endif

#ifdef CREDENTIAL_STORE
	STA_CONNECT_INFO StaCtIf;
#endif

	/* MAT related parameters */
	/*
		Frequency setting for rate adaptation
			@ra_interval: 		for baseline time interval
			@ra_fast_interval:	for quick response time interval
	*/
	UINT32 ra_interval;
	UINT32 ra_fast_interval;

	/* configuration: read from Registry & E2PROM */
	BOOLEAN bLocalAdminMAC;	/* Use user changed MAC */
	UCHAR PermanentAddress[MAC_ADDR_LEN];	/* Factory default MAC address */
	UCHAR CurrentAddress[MAC_ADDR_LEN];	/* User changed MAC address */

	/* ------------------------------------------------------ */
	/* common configuration to both OPMODE_STA and OPMODE_AP */
	/* ------------------------------------------------------ */
	UINT VirtualIfCnt;

	COMMON_CONFIG CommonCfg;
	MLME_STRUCT Mlme;

	/* AP needs those vaiables for site survey feature. */
	MLME_AUX MlmeAux;	/* temporary settings used during MLME state machine */
#if defined(AP_SCAN_SUPPORT) || defined(CONFIG_STA_SUPPORT)
	SCAN_CTRL ScanCtrl;
	BSS_TABLE ScanTab;	/* store the latest SCAN result */
#endif /* defined(AP_SCAN_SUPPORT) || defined(CONFIG_STA_SUPPORT) */

	/*About MacTab, the sta driver will use #0 and #1 for multicast and AP. */
	MAC_TABLE MacTab;	/* ASIC on-chip WCID entry table.  At TX, ASIC always use key according to this on-chip table. */
	spinlock_t MacTabLock;

#ifdef DOT11_N_SUPPORT
	BA_TABLE BATable;
	spinlock_t BATabLock;
	RALINK_TIMER_STRUCT RECBATimer;

	BOOLEAN HTCEnable;
#endif

	EXT_CAP_INFO_ELEMENT ExtCapInfo;

	/* DOT11_H */
	DOT11_H Dot11_H;

	/* encryption/decryption KEY tables */
	CIPHER_KEY SharedKey[HW_BEACON_MAX_NUM + MAX_P2P_NUM][4];	/* STA always use SharedKey[BSS0][0..3] */

	/* various Counters */
	COUNTER_802_3 Counters8023;	/* 802.3 counters */
	COUNTER_802_11 WlanCounters;	/* 802.11 MIB counters */
	COUNTER_RALINK RalinkCounters;	/* Ralink propriety counters */
	/* COUNTER_DRS DrsCounters;	*/ /* counters for Dynamic TX Rate Switching */
	PRIVATE_STRUC PrivateInfo;	/* Private information & counters */

	/* flags, see fRTMP_ADAPTER_xxx flags */
	ULONG Flags;			/* Represent current device status */
	ULONG PSFlags;			/* Power Save operation flag. */
	ULONG MoreFlags;		/* Represent specific requirement */

	/* current TX sequence # */
	USHORT Sequence;

	/* Control disconnect / connect event generation */
	/*+++Didn't used anymore */
	ULONG LinkDownTime;
	/*--- */
	ULONG LastRxRate;
	ULONG LastTxRate;
	/*+++Used only for Station */
	BOOLEAN bConfigChanged;	/* Config Change flag for the same SSID setting */
	/*--- */

	ULONG ExtraInfo;	/* Extra information for displaying status of UI */
	ULONG SystemErrorBitmap;	/* b0: E2PROM version error */

#ifdef SYSTEM_LOG_SUPPORT
	/* --------------------------- */
	/* System event log            */
	/* --------------------------- */
	RT_802_11_EVENT_TABLE EventTab;
#endif

#ifdef HOSTAPD_SUPPORT
	UINT32 IoctlIF;
#endif

#ifdef INF_PPA_SUPPORT
	UINT32 g_if_id;
	BOOLEAN PPAEnable;
	PPA_DIRECTPATH_CB *pDirectpathCb;
#endif /* INF_PPA_SUPPORT */

	/**********************************************************/
	/*      Statistic related parameters                                                    */
	/**********************************************************/
#ifdef RTMP_MAC_USB
	ULONG BulkOutDataOneSecCount;
	ULONG BulkInDataOneSecCount;
	ULONG BulkLastOneSecCount;	/* BulkOutDataOneSecCount + BulkInDataOneSecCount */
	ULONG watchDogRxCnt;
	ULONG watchDogRxOverFlowCnt;
	ULONG watchDogTxPendingCnt[NUM_OF_TX_RING];
#endif

	BOOLEAN bUpdateBcnCntDone;
	ULONG macwd;

	/* ---------------------------- */
	/* DEBUG paramerts */
	/* ---------------------------- */

	/* ---------------------------- */
	/* rt2860c emulation-use Parameters */
	/* ---------------------------- */
	BOOLEAN bLinkAdapt;
	BOOLEAN bForcePrintTX;
	BOOLEAN bForcePrintRX;
	BOOLEAN bStaFifoTest;
	BOOLEAN bProtectionTest;
	BOOLEAN bHCCATest;
	BOOLEAN bGenOneHCCA;
	BOOLEAN bBroadComHT;
	/*+++Following add from RT2870 USB. */
	ULONG BulkOutReq;
	ULONG BulkOutComplete;
	ULONG BulkOutCompleteOther;
	ULONG BulkOutCompleteCancel;	/* seems not use now? */
	ULONG BulkInReq;
	ULONG BulkInComplete;
	ULONG BulkInCompleteFail;
	/*--- */

	INT32 rts_tx_retry_num;
	struct wificonf WIFItestbed;

	UCHAR TssiGain;
#ifdef RALINK_ATE
	ATE_INFO ate;
#ifdef RTMP_MAC_USB
	BOOLEAN ContinBulkOut;	/*ATE bulk out control */
	BOOLEAN ContinBulkIn;	/*ATE bulk in control */
	RTMP_OS_ATOMIC BulkOutRemained;
	RTMP_OS_ATOMIC BulkInRemained;
#endif /* RTMP_MAC_USB */
#endif /* RALINK_ATE */

#ifdef DOT11_N_SUPPORT
	struct reordering_mpdu_pool mpdu_blk_pool;
#endif /* DOT11_N_SUPPORT */

	/* statistics count */
	void *iw_stats;
	void *stats;

#ifdef BLOCK_NET_IF
	BLOCK_QUEUE_ENTRY blockQueueTab[NUM_OF_TX_RING];
#endif

#ifdef MULTIPLE_CARD_SUPPORT
	INT32 MC_RowID;
	STRING MC_FileName[256];
	UINT32 E2P_OFFSET_IN_FLASH[MAX_NUM_OF_MULTIPLE_CARD];
#endif /* MULTIPLE_CARD_SUPPORT */

	ULONG TbttTickCount;	/* beacon timestamp work-around */

#ifdef CONFIG_AP_SUPPORT
	RALINK_TIMER_STRUCT PeriodicTimer;
#endif /* CONFIG_AP_SUPPORT */
	/* for detect_wmm_traffic() BE TXOP use */
	ULONG OneSecondnonBEpackets;	/* record non BE packets per second */
	UCHAR is_on;

	/* for detect_wmm_traffic() BE/BK TXOP use */
#define TIME_BASE			(1000000/OS_HZ)
#define TIME_ONE_SECOND			(1000000/TIME_BASE)
	UCHAR flg_be_adjust;
	ULONG be_adjust_last_time;
	UINT8 FlgCtsEnabled;
	UINT8 PM_FlgSuspend;
	UCHAR FifoUpdateDone, FifoUpdateRx;

#ifdef LINUX
#ifdef RT_CFG80211_SUPPORT
	CFG80211_CTRL cfg80211_ctrl;
	void *pCfg80211_CB;
#endif /* RT_CFG80211_SUPPORT */
#endif /* LINUX */

	UINT32 ContinueMemAllocFailCount;

	struct {
		int IeLen;
		UCHAR *pIe;
	} ProbeRespIE[MAX_LEN_OF_BSS_TABLE];

	/* purpose: We free all kernel resources when module is removed */
	LIST_HEADER RscTimerMemList;	/* resource timers memory */
	LIST_HEADER RscTaskMemList;	/* resource tasks memory */
	LIST_HEADER RscLockMemList;	/* resource locks memory */
	LIST_HEADER RscTaskletMemList;	/* resource tasklets memory */
	LIST_HEADER RscSemMemList;	/* resource semaphore memory */
	LIST_HEADER RscAtomicMemList;	/* resource atomic memory */

	/* purpose: Cancel all timers when module is removed */
	LIST_HEADER RscTimerCreateList;	/* timers list */

#if (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT)
	WOW_CFG_STRUCT WOW_Cfg; /* data structure for wake on wireless */
#endif

	TXWI_STRUC NullTxWI;
	USHORT NullBufOffset[2];

#ifdef APCLI_CERT_SUPPORT
	BOOLEAN bApCliCertTest;
#endif /* APCLI_CERT_SUPPORT */
#ifdef MULTI_MAC_ADDR_EXT_SUPPORT
	BOOLEAN bUseMultiMacAddrExt;
#endif /* MULTI_MAC_ADDR_EXT_SUPPORT */

#ifdef MCS_LUT_SUPPORT
	BOOLEAN bUseHwTxLURate;
#endif /* MCS_LUT_SUPPORT */

#ifdef CONFIG_ANDES_SUPPORT
	struct MCU_CTRL MCUCtrl;
#endif /* CONFIG_ANDES_SUPPORT */

#ifdef WLAN_SKB_RECYCLE
	struct sk_buff_head rx0_recycle;
#endif

#ifdef SINGLE_SKU_V2
	DL_LIST SingleSkuPwrList;
	UCHAR DefaultTargetPwr;
	CHAR SingleSkuRatePwrDiff[18];
	BOOLEAN bOpenFileSuccess;
	BOOLEAN sku_init_done;
	UCHAR tc_init_val;
#endif

#ifdef ED_MONITOR
	BOOLEAN ed_chk;

//for AP Mode's threshold
#ifdef CONFIG_AP_SUPPORT
	UCHAR ed_sta_threshold;
	UCHAR ed_ap_threshold;
#endif

//for STA Mode's threshold
#ifdef CONFIG_STA_SUPPORT
	UCHAR ed_ap_scaned;
	UCHAR ed_current_ch_aps;
	CHAR ed_rssi_threshold;
#endif /* CONFIG_STA_SUPPORT */

	UCHAR ed_threshold;
	UINT false_cca_threshold;
	UINT ed_block_tx_threshold;
	int ed_chk_period;  // in unit of ms
	UCHAR ed_stat_sidx;
	UCHAR ed_stat_lidx;
	BOOLEAN ed_tx_stoped;
	UINT ed_trigger_cnt;
	UINT ed_silent_cnt;
	UINT ed_false_cca_cnt;

#define ED_STAT_CNT 20
	UINT32 ed_stat[ED_STAT_CNT];
	UINT32 ed_trigger_stat[ED_STAT_CNT];
	UINT32 ed_silent_stat[ED_STAT_CNT];
	UINT32 ed_2nd_stat[ED_STAT_CNT];
	UINT32 ch_idle_stat[ED_STAT_CNT];
	UINT32 ch_busy_stat[ED_STAT_CNT];
	UINT32 false_cca_stat[ED_STAT_CNT];
	ULONG chk_time[ED_STAT_CNT];
	RALINK_TIMER_STRUCT ed_timer;
	BOOLEAN ed_timer_inited;
#endif

#ifdef WFA_VHT_PF
	BOOLEAN force_amsdu;
	BOOLEAN force_noack;
	BOOLEAN vht_force_sgi;
	BOOLEAN vht_force_tx_stbc;
	BOOLEAN force_vht_op_mode;
	UCHAR vht_pf_op_ss;
	UCHAR vht_pf_op_bw;
#endif

#ifdef DBG_DIAGNOSE
	RtmpDiagStruct DiagStruct;
#endif

#ifdef RTMP_USB_SUPPORT
	struct usb_control usb_ctl;
#endif

#ifdef VHT_TXBF_SUPPORT
	BOOLEAN NDPA_Request;
	BOOLEAN BeaconSndDimensionFlag;    // Peer sounding dimension flag
#endif
};

#ifdef ED_MONITOR
int ed_status_read(RTMP_ADAPTER *pAd);
int ed_monitor_init(RTMP_ADAPTER *pAd);
int ed_monitor_exit(RTMP_ADAPTER *pAd);
#endif /* ED_MONITOR */

#if defined(RTMP_INTERNAL_TX_ALC) || defined(RTMP_TEMPERATURE_COMPENSATION)
/* The offset of the Tx power tuning entry (zero-based array) */
#define TX_POWER_TUNING_ENTRY_OFFSET			30

/* The lower-bound of the Tx power tuning entry */
#define LOWERBOUND_TX_POWER_TUNING_ENTRY		-30

/* The upper-bound of the Tx power tuning entry in G band */
#define UPPERBOUND_TX_POWER_TUNING_ENTRY(__pAd)		((__pAd)->chipCap.TxAlcTxPowerUpperBound_2G)

#ifdef A_BAND_SUPPORT
/* The upper-bound of the Tx power tuning entry in A band */
#define UPPERBOUND_TX_POWER_TUNING_ENTRY_5G(__pAd)	((__pAd)->chipCap.TxAlcTxPowerUpperBound_5G)
#endif /* A_BAND_SUPPORT */

/* Temperature compensation lookup table */

#define TEMPERATURE_COMPENSATION_LOOKUP_TABLE_OFFSET	7

/* The lower/upper power delta index for the TSSI rate table */

#define LOWER_POWER_DELTA_INDEX		0
#define UPPER_POWER_DELTA_INDEX		24

/* The offset of the TSSI rate table */

#define TSSI_RATIO_TABLE_OFFSET	12

/* Get the power delta bound */

#define GET_TSSI_RATE_TABLE_INDEX(x) (((x) > UPPER_POWER_DELTA_INDEX) ? (UPPER_POWER_DELTA_INDEX) : (((x) < LOWER_POWER_DELTA_INDEX) ? (LOWER_POWER_DELTA_INDEX) : ((x))))

/* 802.11b CCK TSSI information */

typedef union _CCK_TSSI_INFO
{
#ifdef RT_BIG_ENDIAN
	struct
	{
		UCHAR Reserved:1;
		UCHAR ShortPreamble:1;
		UCHAR Rate:2;
		UCHAR Tx40MSel:2;
		UCHAR TxType:2;
	} field;
#else
	struct
	{
		UCHAR TxType:2;
		UCHAR Tx40MSel:2;
		UCHAR Rate:2;
		UCHAR ShortPreamble:1;
		UCHAR Reserved:1;
	} field;
#endif /* RT_BIG_ENDIAN */

	UCHAR value;
} CCK_TSSI_INFO, *PCCK_TSSI_INFO;


/* 802.11a/g OFDM TSSI information */

typedef union _OFDM_TSSI_INFO
{
#ifdef RT_BIG_ENDIAN
	struct
	{
		UCHAR Rate:4;
		UCHAR Tx40MSel:2;
		UCHAR TxType:2;
	} field;
#else
	struct
	{
		UCHAR TxType:2;
		UCHAR Tx40MSel:2;
		UCHAR Rate:4;
	} field;
#endif /* RT_BIG_ENDIAN */

	UCHAR	value;
} OFDM_TSSI_INFO, *POFDM_TSSI_INFO;


/* 802.11n HT TSSI information */

typedef struct _HT_TSSI_INFO {
	union {
#ifdef RT_BIG_ENDIAN
		struct {
			UCHAR SGI:1;
			UCHAR STBC:2;
			UCHAR Aggregation:1;
			UCHAR Tx40MSel:2;
			UCHAR TxType:2;
		} field;
#else
		struct {
			UCHAR TxType:2;
			UCHAR Tx40MSel:2;
			UCHAR Aggregation:1;
			UCHAR STBC:2;
			UCHAR SGI:1;
		} field;
#endif /* RT_BIG_ENDIAN */

		UCHAR value;
	} PartA;

	union {
#ifdef RT_BIG_ENDIAN
		struct {
			UCHAR BW:1;
			UCHAR MCS:7;
		} field;
#else
		struct {
			UCHAR MCS:7;
			UCHAR BW:1;
		} field;
#endif /* RT_BIG_ENDIAN */
		UCHAR	value;
	} PartB;
} HT_TSSI_INFO, *PHT_TSSI_INFO;

typedef struct _TSSI_INFO_{
	UCHAR tssi_info_0;
	union {
		CCK_TSSI_INFO cck_tssi_info;
		OFDM_TSSI_INFO ofdm_tssi_info;
		HT_TSSI_INFO ht_tssi_info_1;
		UCHAR byte;
	} tssi_info_1;
	HT_TSSI_INFO ht_tssi_info_2;
} TSSI_INFO;

#endif /* RTMP_INTERNAL_TX_ALC || RTMP_TEMPERATURE_COMPENSATION */


typedef struct _PEER_PROBE_REQ_PARAM {
	UCHAR Addr2[MAC_ADDR_LEN];
	CHAR Ssid[MAX_LEN_OF_SSID];
	UCHAR SsidLen;
	BOOLEAN bRequestRssi;
} PEER_PROBE_REQ_PARAM, *PPEER_PROBE_REQ_PARAM;


/***************************************************************************
  *	Rx Path software control block related data structures
  **************************************************************************/
typedef enum RX_BLK_FLAGS{
	fRX_AMPDU = 0x0001,
	fRX_AMSDU = 0x0002,
	fRX_ARALINK = 0x0004,
	fRX_HTC = 0x0008,
	fRX_PAD = 0x0010,
	fRX_QOS = 0x0020,
	fRX_EAP = 0x0040,
	fRX_WPI = 0x0080,
	fRX_AP = 0x0100,	// Packet received from AP
	fRX_STA = 0x0200,	// Packet received from Client(Infra moed)
	fRX_ADHOC = 0x400,	// packet received from AdHoc peer
	fRX_WDS = 0x0800,	// Packet received from WDS
	fRX_MESH = 0x1000,	// Packet received from MESH
	fRX_DLS = 0x2000,	// Packet received from DLS peer
	fRX_TDLS = 0x4000,	// Packet received from TDLS peer
}RX_BLK_FLAGS;

typedef struct _RX_BLK
{
	UCHAR hw_rx_info[RXD_SIZE]; /* include "RXD_STRUC RxD" and "RXINFO_STRUC rx_info " */
	RXINFO_STRUC *pRxInfo; /* for RLT, in head of frame buffer, for RTMP, in hw_rx_info[RXINFO_OFFSET] */
#ifdef RLT_MAC
	RXFCE_INFO *pRxFceInfo; /* for RLT, in in hw_rx_info[RXINFO_OFFSET], for RTMP, no such field */
#endif /* RLT_MAC */
	RXWI_STRUC *pRxWI; /*in frame buffer and after "rx_info" fields */
	HEADER_802_11 *pHeader; /* poiter of 802.11 header, pointer to frame buffer and shall not shift this pointer */
	PNDIS_PACKET pRxPacket; /* os_packet pointer, shall not change */
	UCHAR *pData; /* init to pRxPacket->data, refer to frame buffer, may changed depends on processing */
	USHORT DataSize; /* init to  RXWI->MPDUtotalByteCnt, and may changes depends on processing */
	USHORT Flags;

	/* Mirror info of partial fields of RxWI and RxInfo */
	USHORT MPDUtotalByteCnt; 	/* Refer to RXWI->MPDUtotalByteCnt */
	UCHAR UserPriority;		/* for calculate TKIP MIC using */
	UCHAR OpMode;			/* 0:OPMODE_STA 1:OPMODE_AP */
	UCHAR wcid;			/* copy of pRxWI->wcid */
	UCHAR U2M;
	UCHAR key_idx;
	UCHAR bss_idx;
	UCHAR TID;
	CHAR rssi[3];
	CHAR snr[3];
	CHAR freq_offset;
	CHAR ldpc_ex_sym;
	HTTRANSMIT_SETTING rx_rate;
#ifdef HDR_TRANS_SUPPORT
	BOOLEAN bHdrRxTrans;	/* this packet's header is translated to 802.3 by HW  */
	BOOLEAN bHdrVlanTaged;	/* VLAN tag is added to this header */
	UCHAR *pTransData;
	USHORT TransDataSize;
#endif /* HDR_TRANS_SUPPORT */
} RX_BLK;


#define RX_BLK_SET_FLAG(_pRxBlk, _flag)		(_pRxBlk->Flags |= _flag)
#define RX_BLK_TEST_FLAG(_pRxBlk, _flag)	(_pRxBlk->Flags & _flag)
#define RX_BLK_CLEAR_FLAG(_pRxBlk, _flag)	(_pRxBlk->Flags &= ~(_flag))

#define fRX_WDS			0x0001
#define fRX_AMSDU		0x0002
#define fRX_ARALINK		0x0004
#define fRX_HTC			0x0008
#define fRX_PAD			0x0010
#define fRX_AMPDU		0x0020
#define fRX_QOS			0x0040
#define fRX_INFRA		0x0080
#define fRX_EAP			0x0100
#define fRX_MESH		0x0200
#define fRX_APCLI		0x0400
#define fRX_DLS			0x0800
#define fRX_WPI			0x1000
#define fRX_P2PGO		0x2000
#define fRX_P2PCLI		0x4000

#define AMSDU_SUBHEAD_LEN	14
#define ARALINK_SUBHEAD_LEN	14
#define ARALINK_HEADER_LEN	 2

/***************************************************************************
  *	Tx Path software control block related data structures
  **************************************************************************/
#define TX_UNKOWN_FRAME		0x00
#define TX_MCAST_FRAME		0x01
#define TX_LEGACY_FRAME		0x02
#define TX_AMPDU_FRAME		0x04
#define TX_AMSDU_FRAME		0x08
#define TX_RALINK_FRAME		0x10
#define TX_FRAG_FRAME		0x20
#define TX_NDPA_FRAME		0x40

/* Currently the sizeof(TX_BLK) is 148 bytes. */
typedef struct _TX_BLK {
	UCHAR QueIdx;
	UCHAR TxFrameType;			/* Indicate the Transmission type of the all frames in one batch */
	UCHAR TotalFrameNum;			/* Total frame number want to send-out in one batch */
	USHORT TotalFragNum;			/* Total frame fragments required in one batch */
	USHORT TotalFrameLen;			/* Total length of all frames want to send-out in one batch */

	QUEUE_HEADER TxPacketList;
	MAC_TABLE_ENTRY *pMacEntry;		/* NULL: packet with 802.11 RA field is multicast/broadcast address */
	HTTRANSMIT_SETTING	*pTransmit;

	/* Following structure used for the characteristics of a specific packet. */
	PNDIS_PACKET pPacket;
	UCHAR *pSrcBufHeader;			/* Reference to the head of sk_buff->data */
	UCHAR *pSrcBufData;			/* Reference to the sk_buff->data, will changed depends on hanlding progresss */
	UINT SrcBufLen;				/* Length of packet payload which not including Layer 2 header */

	PUCHAR pExtraLlcSnapEncap;		/* NULL means no extra LLC/SNAP is required */
#ifndef VENDOR_FEATURE1_SUPPORT
	/*
		Note: Can not insert any other new parameters
		between pExtraLlcSnapEncap & HeaderBuf; Or
		the start address of HeaderBuf will not be aligned by 4.

		But we can not change HeaderBuf[128] to HeaderBuf[32] because
		many codes use HeaderBuf[index].
	*/
	UCHAR HeaderBuf[128];			/* TempBuffer for TX_INFO + TX_WI + TSO_INFO + 802.11 Header + padding + AMSDU SubHeader + LLC/SNAP */
#else
	UINT32 HeaderBuffer[32];		/* total 128B, use UINT32 to avoid alignment problem */
	UCHAR *HeaderBuf;
#endif /* VENDOR_FEATURE1_SUPPORT */
	UCHAR MpduHeaderLen;			/* 802.11 header length NOT including the padding */
	UCHAR HdrPadLen;			/* recording Header Padding Length; */
	UCHAR UserPriority;			/* priority class of packet */
	UCHAR FrameGap;				/* what kind of IFS this packet use */
	UCHAR MpduReqNum;			/* number of fragments of this frame */
// TODO: shiang-6590, potential to remove
	UCHAR TxRate;				/* TODO: Obsoleted? Should change to MCS? */
	UCHAR CipherAlg;			/* cipher alogrithm */
	PCIPHER_KEY pKey;
	UCHAR KeyIdx;				/* Indicate the transmit key index */
	UCHAR OpMode;
	UCHAR Wcid;				/* The MAC entry associated to this packet */
	UCHAR apidx;				/* The interface associated to this packet */
	UCHAR wdev_idx;				/* Used to replace apidx */

#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
	UINT ApCliIfidx;
	PAPCLI_STRUCT pApCliEntry;
#endif /* APCLI_SUPPORT */

	MULTISSID_STRUCT *pMbss;

#endif /* CONFIG_AP_SUPPORT */
// TODO: ---End
	UINT32 Flags;				/*See following definitions for detail. */

	/*YOU SHOULD NOT TOUCH IT! Following parameters are used for hardware-depended layer. */
	ULONG Priv;				/* Hardware specific value saved in here. */

#ifdef TXBF_SUPPORT
	UCHAR TxSndgPkt; /* 1: sounding 2: NDP sounding */
	UCHAR TxNDPSndgBW;
	UCHAR TxNDPSndgMcs;
#endif /* TXBF_SUPPORT */

#ifdef TX_PKT_SG
	PACKET_INFO pkt_info;
#endif

#ifdef HDR_TRANS_SUPPORT
	BOOLEAN NeedTrans;	/* indicate the packet needs to do hw header translate */
#endif

	struct wifi_dev *wdev;
} TX_BLK;

#define fTX_bRtsRequired	0x0001	/* Indicate if need send RTS frame for protection. Not used in RT2860/RT2870. */
#define fTX_bAckRequired	0x0002	/* the packet need ack response */
#define fTX_bPiggyBack		0x0004	/* Legacy device use Piggback or not */
#define fTX_bHTRate		0x0008	/* allow to use HT rate */
#define fTX_bForceNonQoS	0x0010	/* force to transmit frame without WMM-QoS in HT mode */
#define fTX_bAllowFrag		0x0020	/* allow to fragment the packet, A-MPDU, A-MSDU, A-Ralink is not allowed to fragment */
#define fTX_bMoreData		0x0040	/* there are more data packets in PowerSave Queue */
#define fTX_bWMM		0x0080	/* QOS Data */
#define fTX_bClearEAPFrame	0x0100

#define	fTX_bSwEncrypt		0x0400	/* this packet need to be encrypted by software before TX */
#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
#define fTX_bApCliPacket	0x0200
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#ifdef UAPSD_SUPPORT
#define	fTX_bWMM_UAPSD_EOSP	0x0800	/* Used when UAPSD_SUPPORT */
#endif

#ifdef WFA_VHT_PF
#define fTX_AmsduInAmpdu	0x40000
#endif

#define TX_BLK_SET_FLAG(_pTxBlk, _flag)		(_pTxBlk->Flags |= _flag)
#define TX_BLK_TEST_FLAG(_pTxBlk, _flag)	(((_pTxBlk->Flags & _flag) == _flag) ? 1 : 0)
#define TX_BLK_CLEAR_FLAG(_pTxBlk, _flag)	(_pTxBlk->Flags &= ~(_flag))

//typedef struct dequeue_info{
//	UCHAR qidx;
//	UCHAR wcid;
//	int pkt_bytes;
//	int pkt_cnt;
//} DEQUE_INFO;

#ifdef RT_BIG_ENDIAN
/***************************************************************************
  *	Endian conversion related functions
  **************************************************************************/
/*
	========================================================================

	Routine Description:
		Endian conversion of Tx/Rx descriptor .

	Arguments:
		pAd 	Pointer to our adapter
		pData	Pointer to Tx/Rx descriptor
		DescriptorType	Direction of the frame

	Return Value:
		None

	Note:
		Call this function when read or update descriptor
	========================================================================
*/
static inline void RTMPWIEndianChange(RTMP_ADAPTER *pAd, PUCHAR pData,
	ULONG DescriptorType)
{
	int size;
	int i;
	UINT8 TXWISize = pAd->chipCap.TXWISize;
	UINT8 RXWISize = pAd->chipCap.RXWISize;

	size = ((DescriptorType == TYPE_TXWI) ? TXWISize : RXWISize);

	if (DescriptorType == TYPE_TXWI) {
		*((UINT32 *)(pData)) = SWAP32(*((UINT32 *)(pData)));		/* Byte 0~3 */
		*((UINT32 *)(pData + 4)) = SWAP32(*((UINT32 *)(pData+4)));	/* Byte 4~7 */
		if (size > 16)
			*((UINT32 *)(pData + 16)) = SWAP32(*((UINT32 *)(pData + 16)));	/* Byte 16~19 */
	} else {
		for (i = 0; i < size/4 ; i++)
			*(((UINT32 *)pData) +i) = SWAP32(*(((UINT32 *)pData)+i));
	}
}


/*
	========================================================================

	Routine Description:
		Endian conversion of Tx/Rx descriptor .

	Arguments:
		pAd 	Pointer to our adapter
		pData	Pointer to Tx/Rx descriptor
		DescriptorType	Direction of the frame

	Return Value:
		None

	Note:
		Call this function when read or update descriptor
	========================================================================
*/

#ifdef RTMP_MAC_USB
static inline void RTMPDescriptorEndianChange(UCHAR *pData, ULONG DescType)
{
	*((UINT32 *)(pData)) = SWAP32(*((UINT32 *)(pData)));
}
#endif /* RTMP_MAC_USB */
/*
	========================================================================

	Routine Description:
		Endian conversion of all kinds of 802.11 frames .

	Arguments:
		pAd 	Pointer to our adapter
		pData	Pointer to the 802.11 frame structure
		Dir 	Direction of the frame
		FromRxDoneInt	Caller is from RxDone interrupt

	Return Value:
		None

	Note:
		Call this function when read or update buffer data
	========================================================================
*/
static inline void RTMPFrameEndianChange(RTMP_ADAPTER *pAd, PUCHAR pData,
	ULONG Dir, BOOLEAN FromRxDoneInt)
{
	PHEADER_802_11 pFrame;
	PUCHAR pMacHdr;

	/* swab 16 bit fields - Frame Control field */
	if (Dir == DIR_READ) {
		*(USHORT *)pData = SWAP16(*(USHORT *)pData);
	}

	pFrame = (PHEADER_802_11) pData;
	pMacHdr = (PUCHAR) pFrame;

	/* swab 16 bit fields - Duration/ID field */
	*(USHORT *)(pMacHdr + 2) = SWAP16(*(USHORT *)(pMacHdr + 2));

	if (pFrame->FC.Type != FC_TYPE_CNTL) {
		/* swab 16 bit fields - Sequence Control field */
		*(USHORT *)(pMacHdr + 22) = SWAP16(*(USHORT *)(pMacHdr + 22));
	}

	if (pFrame->FC.Type == FC_TYPE_MGMT) {
		switch(pFrame->FC.SubType) {
		case SUBTYPE_ASSOC_REQ:
		case SUBTYPE_REASSOC_REQ:
			/* swab 16 bit fields - CapabilityInfo field */
			pMacHdr += sizeof(HEADER_802_11);
			*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);

			/* swab 16 bit fields - Listen Interval field */
			pMacHdr += 2;
			*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);
			break;

		case SUBTYPE_ASSOC_RSP:
		case SUBTYPE_REASSOC_RSP:
			/* swab 16 bit fields - CapabilityInfo field */
			pMacHdr += sizeof(HEADER_802_11);
			*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);

			/* swab 16 bit fields - Status Code field */
			pMacHdr += 2;
			*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);

			/* swab 16 bit fields - AID field */
			pMacHdr += 2;
			*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);
			break;

		case SUBTYPE_AUTH:
			 /* When the WEP bit is on, don't do the conversion here.
				This is only shared WEP can hit this condition.
				For AP, it shall do conversion after decryption.
				For STA, it shall do conversion before encryption. */
			if (pFrame->FC.Wep == 1) {
				break;
			} else {
				/* swab 16 bit fields - Auth Alg No. field */
				pMacHdr += sizeof(HEADER_802_11);
				*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);

				/* swab 16 bit fields - Auth Seq No. field */
				pMacHdr += 2;
				*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);

				/* swab 16 bit fields - Status Code field */
				pMacHdr += 2;
				*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);
			}
			break;

		case SUBTYPE_BEACON:
		case SUBTYPE_PROBE_RSP:
			/* swab 16 bit fields - BeaconInterval field */
			pMacHdr += (sizeof(HEADER_802_11) + TIMESTAMP_LEN);
			*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);
			/* swab 16 bit fields - CapabilityInfo field */
			pMacHdr += sizeof(USHORT);
			*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);
			break;

		case SUBTYPE_DEAUTH:
		case SUBTYPE_DISASSOC:
			/* If the PMF is negotiated, those frames shall be encrypted */
			if (!FromRxDoneInt && pFrame->FC.Wep == 1) {
				break;
			} else {
				/* swab 16 bit fields - Reason code field */
				pMacHdr += sizeof(HEADER_802_11);
				*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);
			}
			break;
		}
	} else if (pFrame->FC.Type == FC_TYPE_DATA ) {
	} else if (pFrame->FC.Type == FC_TYPE_CNTL) {
		switch(pFrame->FC.SubType) {
		case SUBTYPE_BLOCK_ACK_REQ:
			{
				PFRAME_BA_REQ pBAReq = (PFRAME_BA_REQ)pFrame;
				*(USHORT *)(&pBAReq->BARControl) = SWAP16(*(USHORT *)(&pBAReq->BARControl));
				pBAReq->BAStartingSeq.word = SWAP16(pBAReq->BAStartingSeq.word);
			}
			break;
		case SUBTYPE_BLOCK_ACK:
			/* For Block Ack packet, the HT_CONTROL field is in the same offset with Addr3 */
			*(UINT32 *)(&pFrame->Addr3[0]) = SWAP32(*(UINT32 *)(&pFrame->Addr3[0]));
			break;

		case SUBTYPE_ACK:
			/*For ACK packet, the HT_CONTROL field is in the same offset with Addr2 */
			*(UINT32 *)(&pFrame->Addr2[0])=	SWAP32(*(UINT32 *)(&pFrame->Addr2[0]));
			break;
		}
	} else {
		DBGPRINT(RT_DEBUG_ERROR,("Invalid Frame Type!!!\n"));
	}

	/* swab 16 bit fields - Frame Control */
	if (Dir == DIR_WRITE) {
		*(USHORT *)pData = SWAP16(*(USHORT *)pData);
	}
}
#endif /* RT_BIG_ENDIAN */

char *get_phymode_str(int phy_mode);
char *get_bw_str(int bandwidth);
BOOLEAN RTMPCheckForHang(NDIS_HANDLE MiniportAdapterContext);
NDIS_STATUS RTMPAllocTxRxRingMemory(RTMP_ADAPTER *pAd);

#ifdef RESOURCE_PRE_ALLOC
NDIS_STATUS RTMPInitTxRxRingMemory(RTMP_ADAPTER *pAd);
#endif

void get_dev_config_idx(RTMP_ADAPTER *pAd);
UCHAR *get_dev_name_prefix(RTMP_ADAPTER *pAd, int dev_type);
NDIS_STATUS RTMPReadParametersHook(RTMP_ADAPTER *pAd);
NDIS_STATUS RTMPSetProfileParameters(RTMP_ADAPTER *pAd, PSTRING pBuffer);

BOOLEAN RTMPGetKeyParameter(PSTRING key, PSTRING dest, int destsize, PSTRING buffer,
	BOOLEAN bTrimSpace);

#ifdef SINGLE_SKU_V2
BOOLEAN NDIS_STATUS RTMPSetSingleSKUParameters(RTMP_ADAPTER *pAd);
void UpdateSkuRatePwr(RTMP_ADAPTER *pAd, UCHAR ch, UCHAR bw, CHAR base_pwr);
#endif

#ifdef RTMP_RF_RW_SUPPORT
void NICInitRFRegisters(PRTMP_ADAPTER pAd);
void RTMP_WriteRF(RTMP_ADAPTER *pAd, UCHAR RegID, UCHAR Value, UCHAR BitMask);
NDIS_STATUS RT30xxWriteRFRegister(RTMP_ADAPTER *pAd, UCHAR regID, UCHAR value);
NDIS_STATUS RT30xxReadRFRegister(RTMP_ADAPTER *pAd, UCHAR regID, PUCHAR pValue);
BOOLEAN RTMPAdjustFrequencyOffset(RTMP_ADAPTER *pAd, PUCHAR pRefFreqOffset);
#endif /* RTMP_RF_RW_SUPPORT */

#ifdef RLT_RF
NDIS_STATUS rlt_rf_write(RTMP_ADAPTER *pAd, UCHAR bank, UCHAR regID, UCHAR value);
NDIS_STATUS rlt_rf_read(RTMP_ADAPTER *pAd, UCHAR bank, UCHAR regID, UCHAR *pValue);
#endif /* RLT_RF */

NDIS_STATUS rtmp_rf_write(RTMP_ADAPTER *pAd, UCHAR bank, UCHAR regID, UCHAR value);
NDIS_STATUS rtmp_rf_read(RTMP_ADAPTER *pAd, UCHAR bank, UCHAR regID, UCHAR *pValue);

void NICReadEEPROMParameters(RTMP_ADAPTER *pAd, PSTRING mac_addr);
void NICInitAsicFromEEPROM(RTMP_ADAPTER *pAd);
NDIS_STATUS NICInitializeAdapter(RTMP_ADAPTER *pAd, BOOLEAN bHardReset);
void RTMPRingCleanUp(RTMP_ADAPTER *pAd, UCHAR RingType);
void UserCfgExit(RTMP_ADAPTER *pAd);
void UserCfgInit(RTMP_ADAPTER *pAd);
int load_patch(RTMP_ADAPTER *ad);
NDIS_STATUS NICLoadFirmware(RTMP_ADAPTER *pAd);
void NICEraseFirmware(RTMP_ADAPTER *pAd);
void NICUpdateFifoStaCounters(RTMP_ADAPTER *pAd);
void NICUpdateRawCounters(RTMP_ADAPTER *pAd);

#ifdef FIFO_EXT_SUPPORT
BOOLEAN NicGetMacFifoTxCnt(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry);
void AsicFifoExtSet(RTMP_ADAPTER *pAd);
void AsicFifoExtEntryClean(RTMP_ADAPTER * pAd, MAC_TABLE_ENTRY *pEntry);
#endif /* FIFO_EXT_SUPPORT */

void NicResetRawCounters(RTMP_ADAPTER *pAd);

void NicGetTxRawCounters(RTMP_ADAPTER *pAd,TX_STA_CNT0_STRUC *pStaTxCnt0,
	TX_STA_CNT1_STRUC *pStaTxCnt1);

void RTMPZeroMemory(void *pSrc, ULONG Length);
ULONG RTMPCompareMemory(void *pSrc1, void *pSrc2, ULONG Length);
void RTMPMoveMemory(void *pDest, void *pSrc, ULONG Length);
void AtoH(PSTRING src, UCHAR *dest, int destlen);
void RTMP_AllTimerListRelease(RTMP_ADAPTER *pAd);

void RTMPInitTimer(RTMP_ADAPTER *pAd, RALINK_TIMER_STRUCT *pTimer,
	void *pTimerFunc, void *pData, BOOLEAN Repeat);

void RTMPSetTimer(RALINK_TIMER_STRUCT *pTimer, ULONG Value);
void RTMPModTimer(RALINK_TIMER_STRUCT *pTimer, ULONG Value);
void RTMPCancelTimer(RALINK_TIMER_STRUCT *pTimer, BOOLEAN *pCancelled);
void RTMPReleaseTimer(RALINK_TIMER_STRUCT *pTimer, BOOLEAN *pCancelled);
void RTMPEnableRxTx(RTMP_ADAPTER *pAd);

/* */
/* prototype in action.c */
/* */
void ActHeaderInit(
	RTMP_ADAPTER *pAd,
	HEADER_802_11 *pHdr80211,
	UCHAR *da,
	UCHAR *sa,
	UCHAR *bssid);

void ActionStateMachineInit(
	RTMP_ADAPTER *pAd,
	STATE_MACHINE *S,
	STATE_MACHINE_FUNC Trans[]);

void MlmeDELBAAction(
	RTMP_ADAPTER *pAd,
	MLME_QUEUE_ELEM *Elem);

void SendSMPSAction(
	RTMP_ADAPTER *pAd,
	UCHAR Wcid,
	UCHAR smps);

#ifdef CONFIG_AP_SUPPORT
void SendBeaconRequest(
	RTMP_ADAPTER *pAd,
	UCHAR Wcid);
#endif

#ifdef DOT11_N_SUPPORT
void RECBATimerTimeout(
	PVOID SystemSpecific1,
	PVOID FunctionContext,
	PVOID SystemSpecific2,
	PVOID SystemSpecific3);

void ORIBATimerTimeout(
	RTMP_ADAPTER *pAd);

#ifdef DOT11N_DRAFT3
void SendNotifyBWActionFrame(
	RTMP_ADAPTER *pAd,
	UCHAR  Wcid,
	UCHAR apidx);

void ChannelSwitchAction(
	RTMP_ADAPTER *pAd,
	UCHAR  Wcid,
	UCHAR  Channel,
	UCHAR  Secondary);

ULONG BuildIntolerantChannelRep(
	RTMP_ADAPTER *pAd,
	PUCHAR  pDest);

void Update2040CoexistFrameAndNotify(
	RTMP_ADAPTER *pAd,
	UCHAR  Wcid,
	BOOLEAN bAddIntolerantCha);

void UpdateBssScanParm(
	RTMP_ADAPTER *pAd,
	OVERLAP_BSS_SCAN_IE APBssScan);
#endif /* DOT11N_DRAFT3 */

BOOLEAN AsicSetRalinkBurstMode(RTMP_ADAPTER *pAd, BOOLEAN enable);
#endif /* DOT11_N_SUPPORT */

void AsicSetTxPreamble(RTMP_ADAPTER *pAd, USHORT TxPreamble);

void BarHeaderInit(
	RTMP_ADAPTER *pAd,
	PFRAME_BAR pCntlBar,
	PUCHAR pDA,
	PUCHAR pSA);

void InsertActField(
	RTMP_ADAPTER *pAd,
	PUCHAR pFrameBuf,
	PULONG pFrameLen,
	UINT8 Category,
	UINT8 ActCode);

BOOLEAN QosBADataParse(
	RTMP_ADAPTER *pAd,
	BOOLEAN bAMSDU,
	PUCHAR p8023Header,
	UCHAR WCID,
	UCHAR TID,
	USHORT Sequence,
	UCHAR DataOffset,
	USHORT Datasize,
	UINT   CurRxIndex);

#ifdef DOT11_N_SUPPORT
BOOLEAN CntlEnqueueForRecv(
	RTMP_ADAPTER *pAd,
	ULONG Wcid,
	ULONG MsgLen,
	PFRAME_BA_REQ pMsg);

void BaAutoManSwitch(
	RTMP_ADAPTER *pAd);
#endif /* DOT11_N_SUPPORT */

void HTIOTCheck(RTMP_ADAPTER *pAd, UCHAR BatRecIdx);
int rtmp_wdev_idx_reg(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
int rtmp_wdev_idx_unreg(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
void wdev_tx_pkts(NDIS_HANDLE dev_hnd, PPNDIS_PACKET pkt_list, UINT pkt_cnt,
	struct wifi_dev *wdev);

#ifdef DOT11_N_SUPPORT
void RTMP_BASetup(
	PRTMP_ADAPTER pAd,
	PMAC_TABLE_ENTRY pMacEntry,
	UINT8 UserPriority);
#endif /* DOT11_N_SUPPORT */

void RTMPDeQueuePacket(
	RTMP_ADAPTER *pAd,
   	BOOLEAN bIntContext,
	UCHAR QueIdx,
	int Max_Tx_Packets);

NDIS_STATUS RTMPFreeTXDRequest(
	 RTMP_ADAPTER *pAd,
	 UCHAR RingType,
	 UCHAR NumberRequired,
	PUCHAR FreeNumberIs);

USHORT RTMPCalcDuration(
	RTMP_ADAPTER *pAd,
	UCHAR Rate,
	ULONG Size);

void RTMPWriteTxWI(
	RTMP_ADAPTER *pAd,
	TXWI_STRUC *pTxWI,
	BOOLEAN FRAG,
	BOOLEAN CFACK,
	BOOLEAN InsTimestamp,
	BOOLEAN AMPDU,
	BOOLEAN Ack,
	BOOLEAN NSeq, /* HW new a sequence. */
	UCHAR BASize,
	UCHAR WCID,
	ULONG Length,
	UCHAR PID,
	UCHAR TID,
	UCHAR TxRate,
	UCHAR Txopmode,
	HTTRANSMIT_SETTING *pTransmit);

void RTMPWriteTxWI_Data(
	RTMP_ADAPTER *pAd,
	TXWI_STRUC *pTxWI,
	TX_BLK *pTxBlk);

void RTMPWriteTxWI_Cache(
	RTMP_ADAPTER *pAd,
	TXWI_STRUC *pTxWI,
	TX_BLK *pTxBlk);

void RTMPSuspendMsduTransmission(
	RTMP_ADAPTER *pAd);

void RTMPResumeMsduTransmission(
	RTMP_ADAPTER *pAd);

NDIS_STATUS MiniportMMRequest(
	RTMP_ADAPTER *pAd,
	UCHAR QueIdx,
	UCHAR *pData,
	UINT Length);

void RTMPSendNullFrame(
	RTMP_ADAPTER *pAd,
	UCHAR TxRate,
	BOOLEAN bQosNull,
	USHORT PwrMgmt);

#ifdef APCLI_SUPPORT
void ApCliRTMPReportMicError(
	PRTMP_ADAPTER pAd,
	PCIPHER_KEY pWpaKey,
	int ifIndex);

void  ApCliWpaDisassocApAndBlockAssoc(
	PVOID SystemSpecific1,
	PVOID FunctionContext,
	PVOID SystemSpecific2,
	PVOID SystemSpecific3);
#endif

BOOLEAN RTMPFreeTXDUponTxDmaDone(RTMP_ADAPTER *pAd, UCHAR QueIdx);

BOOLEAN RTMPCheckEtherType(RTMP_ADAPTER *pAd, PNDIS_PACKET pPacket,
	MAC_TABLE_ENTRY *pMacEntry, struct wifi_dev *wdev, PUCHAR pUserPriority,
	PUCHAR pQueIdx);

void RTMPCckBbpTuning(RTMP_ADAPTER *pAd, UINT TxRate);

/*
	MLME routines
*/

/* Asic/RF/BBP related functions */
void AsicGetTxPowerOffset(PRTMP_ADAPTER pAd,PULONG TxPwr);
void AsicExtraPowerOverMAC(RTMP_ADAPTER *pAd);
void AsicAdjustTxPower(RTMP_ADAPTER *pAd);
void AsicUpdateProtect(RTMP_ADAPTER *pAd,USHORT OperaionMode, UCHAR SetMask,
	BOOLEAN bDisableBGProtect, BOOLEAN bNonGFExist);

void AsicBBPAdjust(RTMP_ADAPTER *pAd);
void AsicSwitchChannel(RTMP_ADAPTER *pAd, UCHAR Channel, BOOLEAN bScan);

int AsicSetChannel(RTMP_ADAPTER *pAd, UCHAR ch, UINT8 bw, UINT8 ext_ch,
	BOOLEAN bScan);

#ifdef THERMAL_PROTECT_SUPPORT
void thermal_protection(PRTMP_ADAPTER pAd);
#endif

void AsicLockChannel(RTMP_ADAPTER *pAd, UCHAR Channel);
void AsicAntennaSelect(RTMP_ADAPTER *pAd, UCHAR Channel);
void AsicResetBBPAgent(RTMP_ADAPTER *pAd);

#ifdef CONFIG_STA_SUPPORT
void AsicSleepThenAutoWakeup(RTMP_ADAPTER *pAd, USHORT TbttNumToNextWakeUp);
void AsicForceWakeup(RTMP_ADAPTER *pAd, BOOLEAN bFromTx);
#endif

BOOLEAN AsicSetDevMac(RTMP_ADAPTER *pAd, UCHAR *addr);
void AsicSetBssid(RTMP_ADAPTER *pAd, UCHAR *pBssid);
void AsicDelWcidTab(RTMP_ADAPTER *pAd, UCHAR Wcid);

#ifdef MAC_APCLI_SUPPORT
void AsicSetApCliBssid(RTMP_ADAPTER *pAd, UCHAR *pBssid, UCHAR index);
#endif

BOOLEAN AsicSetRxFilter(RTMP_ADAPTER *pAd);

#ifdef DOT11_N_SUPPORT
BOOLEAN AsicSetRDG(RTMP_ADAPTER *pAd, BOOLEAN bEnable);
#endif

void AsicCtrlBcnMask(PRTMP_ADAPTER pAd, int mask);
BOOLEAN AsicSetPreTbtt(RTMP_ADAPTER *pAd, BOOLEAN enable);
void AsicDisableSync(RTMP_ADAPTER *pAd);
void AsicEnableBssSync(RTMP_ADAPTER *pAd);
void AsicEnableApBssSync(RTMP_ADAPTER *pAd);
void AsicEnableIbssSync(RTMP_ADAPTER *pAd);
void AsicSetEdcaParm(RTMP_ADAPTER *pAd, PEDCA_PARM pEdcaParm);
BOOLEAN AsicSetRetryLimit(RTMP_ADAPTER *pAd, UINT32 type, UINT32 limit);
UINT32 AsicGetRetryLimit(RTMP_ADAPTER *pAd, UINT32 type);
void AsicSetSlotTime(RTMP_ADAPTER *pAd, BOOLEAN bUseShortSlotTime);

void AsicAddSharedKeyEntry(RTMP_ADAPTER *pAd, UCHAR BssIndex, UCHAR KeyIdx,
	PCIPHER_KEY pCipherKey);

void AsicRemoveSharedKeyEntry(RTMP_ADAPTER *pAd, UCHAR BssIndex, UCHAR KeyIdx);
void AsicUpdateWCIDIVEIV(RTMP_ADAPTER *pAd, USHORT WCID, ULONG uIV, ULONG uEIV);
void AsicUpdateRxWCIDTable(RTMP_ADAPTER *pAd, USHORT WCID, PUCHAR pAddr);

void AsicUpdateWcidAttributeEntry(RTMP_ADAPTER *pAd, UCHAR BssIdx, UCHAR KeyIdx,
	UCHAR CipherAlg, UINT8 Wcid, UINT8 KeyTabFlag);

void AsicAddPairwiseKeyEntry(RTMP_ADAPTER *pAd, UCHAR WCID,
	PCIPHER_KEY pCipherKey);

void AsicRemovePairwiseKeyEntry(RTMP_ADAPTER *pAd, UCHAR Wcid);

#ifdef CONFIG_AP_SUPPORT
void AsicSetMbssMode(RTMP_ADAPTER *pAd, UCHAR NumOfBcns);
#endif

BOOLEAN AsicSendCommandToMcu(RTMP_ADAPTER *pAd, UCHAR Command, UCHAR Token,
	UCHAR Arg0, UCHAR Arg1, BOOLEAN in_atomic);

BOOLEAN AsicSendCmdToMcuAndWait(RTMP_ADAPTER *pAd, UCHAR Command, UCHAR Token,
	UCHAR Arg0, UCHAR Arg1, BOOLEAN in_atomic);

#ifdef MCS_LUT_SUPPORT
void asic_mcs_lut_update(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry);
#endif

#ifdef STREAM_MODE_SUPPORT
UINT32 StreamModeRegVal(RTMP_ADAPTER *pAd);
void AsicSetStreamMode(RTMP_ADAPTER *pAd, PUCHAR pMacAddr, int chainIdx, BOOLEAN bEnabled);
void RtmpStreamModeInit(RTMP_ADAPTER *pAd);
BOOLEAN Set_StreamMode_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_StreamModeMCS_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
#endif /* STREAM_MODE_SUPPORT */

#ifdef WOW_SUPPORT
#ifdef RTMP_MAC_USB
/* For WOW, 8051 MUC send full frame */
void AsicWOWSendNullFrame(RTMP_ADAPTER *pAd, UCHAR TxRate, BOOLEAN bQosNull);
void AsicLoadWOWFirmware(RTMP_ADAPTER *pAd, BOOLEAN WOW);
#endif /* RTMP_MAC_USB */
#endif /* WOW_SUPPORT */

void MacAddrRandomBssid(RTMP_ADAPTER *pAd, PUCHAR pAddr);

void MgtMacHeaderInit(RTMP_ADAPTER *pAd, HEADER_802_11 *pHdr80211, UCHAR SubType,
	UCHAR ToDs, UCHAR *pDA, UCHAR *pSA, UCHAR *pBssid);

void MgtMacHeaderInitExt(RTMP_ADAPTER *pAd, PHEADER_802_11 pHdr80211,
	UCHAR SubType, UCHAR ToDs, PUCHAR pDA, PUCHAR pSA, PUCHAR pBssid);

void MlmeRadioOff(RTMP_ADAPTER *pAd);
void MlmeRadioOn(RTMP_ADAPTER *pAd);
void BssTableInit(BSS_TABLE *Tab);

#ifdef DOT11_N_SUPPORT
void BATableInit(RTMP_ADAPTER *pAd, BA_TABLE *Tab);
void BATableExit(RTMP_ADAPTER *pAd);
#endif

#ifdef ED_MONITOR
ULONG BssChannelAPCount(BSS_TABLE *Tab, UCHAR Channel);
#endif

ULONG BssTableSearch(BSS_TABLE *Tab, PUCHAR pBssid, UCHAR Channel);

ULONG BssSsidTableSearch(BSS_TABLE *Tab, PUCHAR pBssid, PUCHAR pSsid,
	UCHAR SsidLen, UCHAR Channel);

ULONG BssTableSearchWithSSID(BSS_TABLE *Tab, PUCHAR Bssid, PUCHAR pSsid,
	UCHAR SsidLen, UCHAR Channel);

ULONG BssSsidTableSearchBySSID(BSS_TABLE *Tab, PUCHAR pSsid, UCHAR SsidLen);
void BssTableDeleteEntry(PBSS_TABLE pTab, PUCHAR pBssid, UCHAR Channel);

ULONG BssTableSetEntry(RTMP_ADAPTER *pAd, BSS_TABLE *Tab, BCN_IE_LIST *ie_list,
	CHAR Rssi, USHORT LengthVIE, PNDIS_802_11_VARIABLE_IEs pVIE);

#ifdef DOT11_N_SUPPORT
void BATableInsertEntry(RTMP_ADAPTER *pAd, USHORT Aid, USHORT TimeOutValue,
	USHORT StartingSeq, UCHAR TID, UCHAR BAWinSize, UCHAR OriginatorStatus,
	BOOLEAN IsRecipient);

#ifdef DOT11N_DRAFT3
void Bss2040CoexistTimeOut(PVOID SystemSpecific1, PVOID FunctionContext,
	PVOID SystemSpecific2, PVOID SystemSpecific3);

void TriEventInit(RTMP_ADAPTER *pAd);

int TriEventTableSetEntry(RTMP_ADAPTER *pAd, TRIGGER_EVENT_TAB *Tab,
	PUCHAR pBssid,HT_CAPABILITY_IE *pHtCapability, UCHAR HtCapabilityLen,
	UCHAR RegClass, UCHAR ChannelNo);

#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */

void BssTableSsidSort(RTMP_ADAPTER *pAd, BSS_TABLE *OutTab, CHAR Ssid[],
	UCHAR SsidLen);

void BssTableSortByRssi(BSS_TABLE *OutTab, BOOLEAN isInverseOrder);
NDIS_STATUS MlmeQueueInit(RTMP_ADAPTER *pAd, MLME_QUEUE *Queue);

BOOLEAN MlmeEnqueue( RTMP_ADAPTER *pAd, ULONG Machine, ULONG MsgType,
	ULONG MsgLen, void *Msg, ULONG Priv);

BOOLEAN MlmeEnqueueForRecv(PRTMP_ADAPTER pAd, ULONG Wcid, ULONG TimeStampHigh,
	ULONG TimeStampLow, UCHAR Rssi0, UCHAR Rssi1, UCHAR Rssi2, ULONG MsgLen,
	PVOID Msg, UCHAR Signal, UCHAR OpMode);

BOOLEAN  MsgTypeSubst(RTMP_ADAPTER *pAd, PFRAME_802_11 pFrame, int *Machine,
	int *MsgType);

void StateMachineInit(STATE_MACHINE *Sm, STATE_MACHINE_FUNC Trans[],
	ULONG StNr, ULONG MsgNr, STATE_MACHINE_FUNC DefFunc, ULONG InitState,
	ULONG Base);

void StateMachineSetAction(STATE_MACHINE *S, ULONG St, ULONG Msg,
	STATE_MACHINE_FUNC F);

void StateMachinePerformAction(RTMP_ADAPTER *pAd, STATE_MACHINE *S,
	MLME_QUEUE_ELEM *Elem, ULONG CurrState);

void Drop(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem);

void AssocStateMachineInit(RTMP_ADAPTER *pAd, STATE_MACHINE *Sm,
	STATE_MACHINE_FUNC Trans[]);

void ReassocTimeout(PVOID SystemSpecific1, PVOID FunctionContext,
	PVOID SystemSpecific2, PVOID SystemSpecific3);

void AssocTimeout(PVOID SystemSpecific1, PVOID FunctionContext,
	PVOID SystemSpecific2, PVOID SystemSpecific3);

void DisassocTimeout(PVOID SystemSpecific1, PVOID FunctionContext,
	PVOID SystemSpecific2, PVOID SystemSpecific3);

/*---------------------------------------------- */
void MlmeDisassocReqAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem);
void MlmeDisassocReqAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem);

#ifdef RTMP_MAC_USB
void MlmeCntlConfirm(RTMP_ADAPTER *pAd, ULONG MsgType, USHORT Msg);
#endif

void ComposePsPoll(RTMP_ADAPTER *pAd);
void ComposeNullFrame(RTMP_ADAPTER *pAd);

void  AssocPostProc(
	 RTMP_ADAPTER *pAd,
	 PUCHAR pAddr2,
	 USHORT CapabilityInfo,
	 USHORT Aid,
	 UCHAR SupRate[],
	 UCHAR SupRateLen,
	 UCHAR ExtRate[],
	 UCHAR ExtRateLen,
	PEDCA_PARM pEdcaParm,
	IE_LISTS *ie_list,
	HT_CAPABILITY_IE *pHtCapability,
	 UCHAR HtCapabilityLen,
	ADD_HT_INFO_IE *pAddHtInfo);

void AuthStateMachineInit(
	 RTMP_ADAPTER *pAd,
	PSTATE_MACHINE sm,
	STATE_MACHINE_FUNC Trans[]);

void AuthTimeout(
	PVOID SystemSpecific1,
	PVOID FunctionContext,
	PVOID SystemSpecific2,
	PVOID SystemSpecific3);

void MlmeAuthReqAction(
	 RTMP_ADAPTER *pAd,
	 MLME_QUEUE_ELEM *Elem);

void PeerAuthRspAtSeq2Action(
	 RTMP_ADAPTER *pAd,
	 MLME_QUEUE_ELEM *Elem);

void PeerAuthRspAtSeq4Action(
	 RTMP_ADAPTER *pAd,
	 MLME_QUEUE_ELEM *Elem);

void AuthTimeoutAction(
	 RTMP_ADAPTER *pAd,
	 MLME_QUEUE_ELEM *Elem);

void Cls2errAction(
	 RTMP_ADAPTER *pAd,
	 PUCHAR pAddr);

void MlmeDeauthReqAction(
	 RTMP_ADAPTER *pAd,
	 MLME_QUEUE_ELEM *Elem);

void InvalidStateWhenAuth(
	 RTMP_ADAPTER *pAd,
	 MLME_QUEUE_ELEM *Elem);

/*============================================= */

void AuthRspStateMachineInit(RTMP_ADAPTER *pAd, PSTATE_MACHINE Sm,
	 STATE_MACHINE_FUNC Trans[]);

void PeerDeauthAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem);

void PeerAuthSimpleRspGenAndSend(RTMP_ADAPTER *pAd, PHEADER_802_11 pHdr80211,
	USHORT Alg, USHORT Seq, USHORT Reason, USHORT Status);

BOOLEAN PeerProbeReqSanity(RTMP_ADAPTER *pAd, void *Msg, ULONG MsgLen,
	PEER_PROBE_REQ_PARAM *Param);

/*======================================== */

void SyncStateMachineInit(RTMP_ADAPTER *pAd, STATE_MACHINE *Sm,
	STATE_MACHINE_FUNC Trans[]);

void BeaconTimeout(PVOID SystemSpecific1, PVOID FunctionContext,
	PVOID SystemSpecific2, PVOID SystemSpecific3);

void ScanTimeout(PVOID SystemSpecific1, PVOID FunctionContext,
	PVOID SystemSpecific2, PVOID SystemSpecific3);

void MlmeScanReqAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem);
void InvalidStateWhenScan(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem);
void InvalidStateWhenJoin(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem);
void InvalidStateWhenStart(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem);
void PeerBeacon(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem);
void EnqueueProbeRequest(RTMP_ADAPTER *pAd);
BOOLEAN ScanRunning(RTMP_ADAPTER *pAd);

/*========================================= */

void MlmeCntlInit(RTMP_ADAPTER *pAd, STATE_MACHINE *S,
	STATE_MACHINE_FUNC Trans[]);

void MlmeCntlMachinePerformAction(RTMP_ADAPTER *pAd, STATE_MACHINE *S,
	MLME_QUEUE_ELEM *Elem);

void CntlOidScanProc(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem);
void CntlOidRTBssidProc(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM * Elem);
void LinkUp(RTMP_ADAPTER *pAd, UCHAR BssType);
void LinkDown(RTMP_ADAPTER *pAd, BOOLEAN IsReqFromAP);
void IterateOnBssTab(RTMP_ADAPTER *pAd);

void AssocParmFill(RTMP_ADAPTER *pAd, MLME_ASSOC_REQ_STRUCT *AssocReq,
	PUCHAR pAddr, USHORT CapabilityInfo, ULONG Timeout, USHORT ListenIntv);

void ScanParmFill(RTMP_ADAPTER *pAd, MLME_SCAN_REQ_STRUCT *ScanReq,
	STRING Ssid[], UCHAR SsidLen, UCHAR BssType, UCHAR ScanType);

void DisassocParmFill(RTMP_ADAPTER *pAd, MLME_DISASSOC_REQ_STRUCT *DisassocReq,
	PUCHAR pAddr, USHORT Reason);

void EnqueuePsPoll(RTMP_ADAPTER *pAd);
void EnqueueBeaconFrame(RTMP_ADAPTER *pAd);
void MlmeJoinReqAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem);
void MlmeScanReqAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem);
void MlmeStartReqAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem);
void MlmeForceJoinReqAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem);
void MlmeForceScanReqAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem);
void ScanTimeoutAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem);
void BeaconTimeoutAtJoinAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem);
void PeerBeaconAtScanAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem);
void PeerBeaconAtJoinAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem);
void PeerBeacon(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem);
void PeerProbeReqAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem);
void ScanNextChannel(RTMP_ADAPTER *pAd, UCHAR OpMode);
ULONG MakeIbssBeacon(RTMP_ADAPTER *pAd);

BOOLEAN MlmeScanReqSanity(RTMP_ADAPTER *pAd, void *Msg, ULONG MsgLen,
	UCHAR *BssType, CHAR ssid[], UCHAR *SsidLen, UCHAR *ScanType);

BOOLEAN PeerBeaconAndProbeRspSanity(RTMP_ADAPTER *pAd, void *Msg, ULONG MsgLen,
	UCHAR MsgChannel, BCN_IE_LIST *ie_list, USHORT *LengthVIE,
	PNDIS_802_11_VARIABLE_IEs pVIE);

#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
BOOLEAN PeerBeaconAndProbeRspSanity2(RTMP_ADAPTER *pAd, void *Msg, ULONG MsgLen,
	OVERLAP_BSS_SCAN_IE *BssScan, UCHAR *RegClass);
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */

BOOLEAN PeerAddBAReqActionSanity(RTMP_ADAPTER *pAd, void *pMsg, ULONG MsgLen,
	PUCHAR pAddr2);

BOOLEAN PeerAddBARspActionSanity(RTMP_ADAPTER *pAd, void *pMsg, ULONG MsgLen);

BOOLEAN PeerDelBAActionSanity(RTMP_ADAPTER *pAd, UCHAR Wcid, void *pMsg,
	ULONG MsgLen);

BOOLEAN MlmeAssocReqSanity(RTMP_ADAPTER *pAd, void *Msg, ULONG MsgLen,
	PUCHAR pApAddr, USHORT *CapabilityInfo, ULONG *Timeout,
	USHORT *ListenIntv);

BOOLEAN MlmeAuthReqSanity(RTMP_ADAPTER *pAd, void *Msg, ULONG MsgLen,
	PUCHAR pAddr, ULONG *Timeout, USHORT *Alg);

BOOLEAN MlmeStartReqSanity(RTMP_ADAPTER *pAd, void *Msg, ULONG MsgLen,
	CHAR Ssid[], UCHAR *Ssidlen);

BOOLEAN PeerAuthSanity(RTMP_ADAPTER *pAd, void *Msg, ULONG MsgLen, PUCHAR pAddr,
	USHORT *Alg, USHORT *Seq, USHORT *Status, CHAR ChlgText[]);

BOOLEAN PeerAssocRspSanity(RTMP_ADAPTER *pAd, void *pMsg, ULONG MsgLen,
	PUCHAR pAddr2, USHORT *pCapabilityInfo, USHORT *pStatus, USHORT *pAid,
	UCHAR SupRate[], UCHAR *pSupRateLen, UCHAR ExtRate[], UCHAR *pExtRateLen,
	HT_CAPABILITY_IE *pHtCapability, ADD_HT_INFO_IE *pAddHtInfo,	/* AP might use this additional ht info IE */
	UCHAR *pHtCapabilityLen, UCHAR *pAddHtInfoLen, UCHAR *pNewExtChannelOffset,
	PEDCA_PARM pEdcaParm, EXT_CAP_INFO_ELEMENT *pExtCapInfo, UCHAR *pCkipFlag,
	IE_LISTS *ie_list);

BOOLEAN PeerDisassocSanity(RTMP_ADAPTER *pAd, void *Msg, ULONG MsgLen,
	PUCHAR pAddr2, USHORT *Reason);

BOOLEAN PeerDeauthSanity(RTMP_ADAPTER *pAd, void *Msg, ULONG MsgLen,
	PUCHAR pAddr1, PUCHAR pAddr2, PUCHAR pAddr3, USHORT *Reason);

BOOLEAN GetTimBit(CHAR *Ptr, USHORT Aid, UCHAR *TimLen, UCHAR *BcastFlag,
	UCHAR *DtimCount, UCHAR *DtimPeriod, UCHAR *MessageToMe);

UCHAR ChannelSanity(RTMP_ADAPTER *pAd, UCHAR channel);
NDIS_802_11_NETWORK_TYPE NetworkTypeInUseSanity(BSS_ENTRY *pBss);
BOOLEAN MlmeDelBAReqSanity(RTMP_ADAPTER *pAd, void *Msg, ULONG MsgLen);

BOOLEAN MlmeAddBAReqSanity(RTMP_ADAPTER *pAd, void *Msg, ULONG MsgLen,
	PUCHAR pAddr2);

ULONG MakeOutgoingFrame(UCHAR *Buffer, ULONG *Length, ...);

#ifdef DOT11_VHT_AC
BOOLEAN RTMPCheckVht(RTMP_ADAPTER *pAd, UCHAR Wcid, VHT_CAP_IE *vht_cap,
	VHT_OP_IE *vht_op);
#endif /* DOT11_VHT_AC */

UCHAR RandomByte(RTMP_ADAPTER *pAd);
UCHAR RandomByte2(RTMP_ADAPTER *pAd);

void AsicUpdateAutoFallBackTable(RTMP_ADAPTER *pAd, UCHAR *pTxRate);
BOOLEAN AsicSetAutoFallBack(RTMP_ADAPTER *pAd, BOOLEAN enable);

void MlmePeriodicExec(PVOID SystemSpecific1, PVOID FunctionContext,
	PVOID SystemSpecific2, PVOID SystemSpecific3);

void LinkDownExec(PVOID SystemSpecific1, PVOID FunctionContext,
	PVOID SystemSpecific2, PVOID SystemSpecific3);

void STAMlmePeriodicExec(RTMP_ADAPTER *pAd);
void MlmeAutoReconnectLastSSID(RTMP_ADAPTER *pAd);
BOOLEAN MlmeValidateSSID(PUCHAR pSsid, UCHAR SsidLen);

#ifdef TXBF_SUPPORT
BOOLEAN MlmeTxBfAllowed(PRTMP_ADAPTER pAd, PMAC_TABLE_ENTRY pEntry,
	struct _RTMP_RA_LEGACY_TB *pTxRate);
#endif

void MlmeCheckPsmChange(RTMP_ADAPTER *pAd, ULONG Now32);
void MlmeSetPsmBit(RTMP_ADAPTER *pAd, USHORT psm);
void MlmeSetTxPreamble(RTMP_ADAPTER *pAd, USHORT TxPreamble);
void MlmeUpdateTxRates(RTMP_ADAPTER *pAd, BOOLEAN bLinkUp, UCHAR apidx);

#ifdef DOT11_N_SUPPORT
void MlmeUpdateHtTxRates(PRTMP_ADAPTER pAd, UCHAR apidx);
#endif

void RTMPCheckRates(RTMP_ADAPTER *pAd, UCHAR SupRate[], UCHAR *SupRateLen);

BOOLEAN RTMPCheckHt(RTMP_ADAPTER *pAd, UCHAR Wcid,
	HT_CAPABILITY_IE *pHtCapability, ADD_HT_INFO_IE *pAddHtInfo);

void RTMPUpdateMlmeRate(RTMP_ADAPTER *pAd);
CHAR RTMPMaxRssi(RTMP_ADAPTER *pAd, CHAR Rssi0, CHAR Rssi1, CHAR Rssi2);
CHAR RTMPAvgRssi(RTMP_ADAPTER *pAd, RSSI_SAMPLE *pRssi);
CHAR RTMPMinSnr(RTMP_ADAPTER *pAd, CHAR Snr0, CHAR Snr1);
void AsicSetRxAnt(RTMP_ADAPTER *pAd, UCHAR Ant);

#ifdef MICROWAVE_OVEN_SUPPORT
BOOLEAN Set_MO_FalseCCATh_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
void AsicMeasureFalseCCA(RTMP_ADAPTER *pAd);
void AsicMitigateMicrowave(RTMP_ADAPTER *pAd);
#endif /* MICROWAVE_OVEN_SUPPORT */

#ifdef RTMP_EFUSE_SUPPORT
BOOLEAN set_eFuseGetFreeBlockCount_Proc(PRTMP_ADAPTER pAd, PSTRING arg);
BOOLEAN set_eFuseBufferModeWriteBack_Proc(PRTMP_ADAPTER pAd, PSTRING arg);
BOOLEAN set_eFusedump_Proc(PRTMP_ADAPTER pAd, PSTRING arg);
BOOLEAN set_eFuseLoadFromBin_Proc(PRTMP_ADAPTER pAd, PSTRING arg);
UCHAR eFuseReadRegisters(PRTMP_ADAPTER pAd, USHORT Offset, USHORT Length,
	USHORT* pData);

void eFusePhysicalReadRegisters(PRTMP_ADAPTER pAd, USHORT Offset,
	USHORT Length, USHORT *pData);

int RtmpEfuseSupportCheck(RTMP_ADAPTER *pAd);
void eFuseGetFreeBlockCount(PRTMP_ADAPTER pAd, PUINT EfuseFreeBlock);
BOOLEAN eFuse_init(PRTMP_ADAPTER pAd);
int efuse_probe(RTMP_ADAPTER *pAd);

#ifdef RALINK_ATE
BOOLEAN Set_LoadEepromBufferFromEfuse_Proc(PRTMP_ADAPTER pAd, PSTRING arg);
BOOLEAN set_eFuseBufferModeWriteBack_Proc(PRTMP_ADAPTER pAd, PSTRING arg);
BOOLEAN Set_LoadEepromBufferFromEfuse_Proc(PRTMP_ADAPTER pAd, PSTRING arg);
BOOLEAN set_BinModeWriteBack_Proc(PRTMP_ADAPTER pAd, PSTRING arg);
#endif

#endif /* RTMP_EFUSE_SUPPORT */

void AsicRxAntEvalTimeout(PVOID SystemSpecific1, PVOID FunctionContext,
	PVOID SystemSpecific2, PVOID SystemSpecific3);

void APSDPeriodicExec(PVOID SystemSpecific1, PVOID FunctionContext,
	PVOID SystemSpecific2, PVOID SystemSpecific3);

void RTMPSetPiggyBack(RTMP_ADAPTER *pAd, BOOLEAN bPiggyBack);
BOOLEAN RTMPCheckEntryEnableAutoRateSwitch(RTMP_ADAPTER *pAd, PMAC_TABLE_ENTRY pEntry);
UCHAR RTMPStaFixedTxMode(RTMP_ADAPTER *pAd, PMAC_TABLE_ENTRY pEntry);
void RTMPUpdateLegacyTxSetting(UCHAR fixed_tx_mode, PMAC_TABLE_ENTRY pEntry);

#ifdef CONFIG_STA_SUPPORT
#ifdef RTMP_FREQ_CALIBRATION_SUPPORT
void InitFrequencyCalibration(RTMP_ADAPTER *pAd);
void StopFrequencyCalibration(RTMP_ADAPTER *pAd);
void FrequencyCalibration(RTMP_ADAPTER *pAd);
CHAR GetFrequencyOffset(RTMP_ADAPTER *pAd, RXWI_STRUC *pRxWI);
#endif /* RTMP_FREQ_CALIBRATION_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */

#ifdef RTMP_TEMPERATURE_COMPENSATION
void InitLookupTable(RTMP_ADAPTER *pAd);
#endif /* RTMP_TEMPERATURE_COMPENSATION */

void MlmeHalt(RTMP_ADAPTER *pAd);
NDIS_STATUS MlmeInit(RTMP_ADAPTER *pAd);
void MlmeResetRalinkCounters(RTMP_ADAPTER *pAd);
void BuildChannelList(RTMP_ADAPTER *pAd);
UCHAR FirstChannel(RTMP_ADAPTER *pAd);
UCHAR NextChannel(RTMP_ADAPTER *pAd, UCHAR channel);
void ChangeToCellPowerLimit(RTMP_ADAPTER *pAd, UCHAR AironetCellPowerLimit);

BOOLEAN RTMPTkipCompareMICValue(RTMP_ADAPTER *pAd, PUCHAR pSrc, PUCHAR pDA,
	PUCHAR pSA, PUCHAR pMICKey, UCHAR UserPriority, UINT Len);

void RTMPCalculateMICValue(RTMP_ADAPTER *pAd, PNDIS_PACKET pPacket,
	PUCHAR pEncap, PCIPHER_KEY pKey, UCHAR apidx);

BOOLEAN RT_CfgSetCountryRegion(RTMP_ADAPTER *pAd, PSTRING arg, int band);
BOOLEAN RT_CfgSetWirelessMode(RTMP_ADAPTER *pAd, PSTRING arg);

RT_802_11_PHY_MODE wmode_2_cfgmode(UCHAR wmode);
UCHAR cfgmode_2_wmode(UCHAR cfg_mode);
UCHAR *wmode_2_str(UCHAR wmode);

#ifdef CONFIG_AP_SUPPORT
#ifdef MBSS_SUPPORT
int RT_CfgSetMbssWirelessMode(RTMP_ADAPTER *pAd, PSTRING arg);
#endif /* MBSS_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

BOOLEAN RT_CfgSetShortSlot(RTMP_ADAPTER *pAd, PSTRING arg);

BOOLEAN RT_CfgSetWepKey(RTMP_ADAPTER *pAd, PSTRING keyString,
	CIPHER_KEY *pSharedKey, int keyIdx);

BOOLEAN RT_CfgSetWPAPSKKey(RTMP_ADAPTER *pAd, PSTRING keyString, int keyStringLen,
	UCHAR *pHashStr, int hashStrLen, PUCHAR pPMKBuf);

int RT_CfgSetFixedTxPhyMode(PSTRING arg);
BOOLEAN RT_CfgSetMacAddress(RTMP_ADAPTER *pAd, PSTRING arg);
int RT_CfgSetTxMCSProc(PSTRING arg, BOOLEAN *pAutoRate);
BOOLEAN RT_CfgSetAutoFallBack(RTMP_ADAPTER *pAd, PSTRING arg);

BOOLEAN set_tssi_enable(RTMP_ADAPTER *pAd, PSTRING arg);

#ifdef CONFIG_ANDES_SUPPORT
BOOLEAN set_fw_debug(RTMP_ADAPTER *pAd, PSTRING arg);
#endif

BOOLEAN set_cal_test(RTMP_ADAPTER *pAd, PSTRING arg);
NDIS_STATUS RTMPWPARemoveKeyProc(RTMP_ADAPTER *pAd, PVOID pBuf);
void RTMPWPARemoveAllKeys(RTMP_ADAPTER *pAd);
void RTMPSetPhyMode(RTMP_ADAPTER *pAd, ULONG phymode);

void RTMPUpdateHTIE(RT_HT_CAPABILITY *pRtHt, UCHAR *pMcsSet,
	HT_CAPABILITY_IE *pHtCapability, ADD_HT_INFO_IE *pAddHtInfo);

void RTMPAddWcidAttributeEntry(RTMP_ADAPTER *pAd, UCHAR BssIdx, UCHAR KeyIdx,
	UCHAR CipherAlg, MAC_TABLE_ENTRY *pEntry);

PSTRING GetEncryptType(CHAR enc);
PSTRING GetAuthMode(CHAR auth);

#ifdef DOT11_N_SUPPORT
void RTMPSetHT(RTMP_ADAPTER *pAd, OID_SET_HT_PHYMODE *pHTPhyMode);

UCHAR get_cent_ch_by_htinfo(RTMP_ADAPTER *pAd, ADD_HT_INFO_IE *ht_op,
	HT_CAPABILITY_IE *ht_cap);

int get_ht_cent_ch(RTMP_ADAPTER *pAd, UINT8 *rf_bw, UINT8 *ext_ch);
int ht_mode_adjust(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, HT_CAPABILITY_IE *peer,
	RT_HT_CAPABILITY *my);
int set_ht_fixed_mcs(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, UCHAR fixed_mcs,
	UCHAR mcs_bound);
int get_ht_max_mcs(RTMP_ADAPTER *pAd, UCHAR *desire_mcs, UCHAR *cap_mcs);
#endif /* DOT11_N_SUPPORT */

#ifdef SYSTEM_LOG_SUPPORT
void RtmpDrvSendWirelessEvent(void *pAdSrc, USHORT Event_flag, PUCHAR pAddr,
	UCHAR BssIdx, CHAR Rssi);
#else
#define RtmpDrvSendWirelessEvent(_pAd, _Event_flag, _pAddr, _BssIdx, _Rssi)
#endif /* SYSTEM_LOG_SUPPORT */

CHAR ConvertToRssi(PRTMP_ADAPTER pAd, CHAR Rssi, UCHAR RssiNumber);
CHAR ConvertToSnr(RTMP_ADAPTER *pAd, UCHAR Snr);

#ifdef DOT11N_DRAFT3
void BuildEffectedChannelList(RTMP_ADAPTER *pAd);
void DeleteEffectedChannelList(RTMP_ADAPTER *pAd);
void CntlChannelWidth(RTMP_ADAPTER *pAd, UCHAR PrimaryChannel, UCHAR CentralChannel,
	UCHAR ChannelWidth, UCHAR SecondaryChannelOffset);
#endif /* DOT11N_DRAFT3 */

void APAsicEvaluateRxAnt(RTMP_ADAPTER *pAd);
void APAsicRxAntEvalTimeout(RTMP_ADAPTER *pAd);
void RTMPGetTxTscFromAsic(RTMP_ADAPTER *pAd, UCHAR apidx, UCHAR *pTxTsc);

MAC_TABLE_ENTRY *PACInquiry(RTMP_ADAPTER *pAd, UCHAR Wcid);

UINT APValidateRSNIE(RTMP_ADAPTER *pAd, PMAC_TABLE_ENTRY pEntry,
	PUCHAR pRsnIe, UCHAR rsnie_len);

void HandleCounterMeasure(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry);

void WPAStart4WayHS(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, ULONG TimeInterval);

void WPAStart2WayGroupHS(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry);

void CMTimerExec(PVOID SystemSpecific1, PVOID FunctionContext, PVOID SystemSpecific2,
	PVOID SystemSpecific3);

void WPARetryExec(PVOID SystemSpecific1, PVOID FunctionContext, PVOID SystemSpecific2,
	PVOID SystemSpecific3);

#ifdef TXBF_SUPPORT
void eTxBfProbeTimerExec(PVOID SystemSpecific1, PVOID FunctionContext,
	PVOID SystemSpecific2, PVOID SystemSpecific3);
#endif /* TXBF_SUPPORT */

void EnqueueStartForPSKExec(PVOID SystemSpecific1, PVOID FunctionContext,
	PVOID SystemSpecific2, PVOID SystemSpecific3);

void RTMPHandleSTAKey(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY  *pEntry,
	MLME_QUEUE_ELEM  *Elem);

void MlmeDeAuthAction(RTMP_ADAPTER *pAd, PMAC_TABLE_ENTRY pEntry, USHORT Reason,
	BOOLEAN bDataFrameFirst);

#ifdef DOT11W_PMF_SUPPORT
void PMF_SAQueryTimeOut(PVOID SystemSpecific1, PVOID FunctionContext,
	PVOID SystemSpecific2, PVOID SystemSpecific3);

void PMF_SAQueryConfirmTimeOut(PVOID SystemSpecific1, PVOID FunctionContext,
	PVOID SystemSpecific2, PVOID SystemSpecific3);
#endif /* DOT11W_PMF_SUPPORT */

//void GREKEYPeriodicExec(PVOID SystemSpecific1, PVOID FunctionContext,
//	PVOID SystemSpecific2, PVOID SystemSpecific3);

//void AES_128_CMAC(
//	PUCHAR key,
//	PUCHAR input,
//	int len,
//	PUCHAR mac);

#ifdef DOT1X_SUPPORT
void WpaSend(RTMP_ADAPTER *pAd, PUCHAR pPacket, ULONG Len);

void RTMPAddPMKIDCache(RTMP_ADAPTER *pAd, int apidx, PUCHAR pAddr,
	UCHAR *PMKID, UCHAR *PMK);

int RTMPSearchPMKIDCache(RTMP_ADAPTER *pAd, int apidx, PUCHAR pAddr);
void RTMPDeletePMKIDCache(RTMP_ADAPTER *pAd, int apidx, int idx);
void RTMPMaintainPMKIDCache(RTMP_ADAPTER *pAd);
#else
#define RTMPMaintainPMKIDCache(_pAd)
#endif /* DOT1X_SUPPORT */

#ifdef RESOURCE_PRE_ALLOC
void RTMPResetTxRxRingMemory(RTMP_ADAPTER *pAd);
#endif

void RTMPFreeTxRxRingMemory(RTMP_ADAPTER *pAd);
BOOLEAN RTMP_FillTxBlkInfo(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk);
void announce_802_3_packet(void *pAdSrc, PNDIS_PACKET pPacket, UCHAR OpMode);

#ifdef DOT11_N_SUPPORT
UINT BA_Reorder_AMSDU_Annnounce(RTMP_ADAPTER *pAd, PNDIS_PACKET pPacket,
	UCHAR OpMode);
#endif /* DOT11_N_SUPPORT */

PNET_DEV get_netdev_from_bssid(RTMP_ADAPTER *pAd, UCHAR FromWhichBSSID);

#ifdef DOT11_N_SUPPORT
void ba_flush_reordering_timeout_mpdus(RTMP_ADAPTER *pAd, PBA_REC_ENTRY pBAEntry,
	ULONG Now32);

void BAOriSessionSetUp(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, UCHAR TID,
	USHORT TimeOut, ULONG DelayTime, BOOLEAN isForced);

void BASessionTearDownALL(RTMP_ADAPTER *pAd, UCHAR Wcid);

void BAOriSessionTearDown(RTMP_ADAPTER *pAd, UCHAR Wcid, UCHAR TID,
	BOOLEAN bPassive, BOOLEAN bForceSend);

void BARecSessionTearDown(RTMP_ADAPTER *pAd, UCHAR Wcid, UCHAR TID, BOOLEAN bPassive);
#endif /* DOT11_N_SUPPORT */

BOOLEAN ba_reordering_resource_init(RTMP_ADAPTER *pAd, int num);
void ba_reordering_resource_release(RTMP_ADAPTER *pAd);
int ComputeChecksum(UINT PIN);
UINT GenerateWpsPinCode(RTMP_ADAPTER *pAd, BOOLEAN bFromApcli, UCHAR apidx);

BOOLEAN rtstrmactohex(PSTRING s1, PSTRING s2);
BOOLEAN rtstrcasecmp(PSTRING s1, PSTRING s2);
PSTRING rtstrstruncasecmp(PSTRING s1, PSTRING s2);
PSTRING rtstrstr( const PSTRING s1, const PSTRING s2);
PSTRING rstrtok( PSTRING s, const PSTRING ct);
int rtinet_aton(const PSTRING cp, unsigned int *addr);

/*//////// common ioctl functions ////////*/
BOOLEAN Set_DriverVersion_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_CountryRegion_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_CountryRegionABand_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_WirelessMode_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_MBSS_WirelessMode_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_Channel_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
#ifdef RT_CFG80211_SUPPORT
BOOLEAN Set_DisableCfg2040Scan_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
#endif
BOOLEAN Set_ShortSlot_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_TxPower_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_BGProtection_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_TxPreamble_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_RTSThreshold_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_FragThreshold_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_TxBurst_Proc(RTMP_ADAPTER *pAd, PSTRING arg);

#ifdef AGGREGATION_SUPPORT
BOOLEAN Set_PktAggregate_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
#endif

#ifdef INF_PPA_SUPPORT
BOOLEAN Set_INF_AMAZON_SE_PPA_Proc(RTMP_ADAPTER *pAd, PSTRING arg);

int ifx_ra_start_xmit (struct net_device *rx_dev, struct net_device *tx_dev,
	struct sk_buff *skb, int len);
#endif /* INF_PPA_SUPPORT */

BOOLEAN Set_IEEE80211H_Proc(RTMP_ADAPTER *pAd, PSTRING arg);

#ifdef EXT_BUILD_CHANNEL_LIST
BOOLEAN Set_ExtCountryCode_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_ExtDfsType_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_ChannelListAdd_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_ChannelListShow_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_ChannelListDel_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
#endif /* EXT_BUILD_CHANNEL_LIST */

#ifdef DBG
BOOLEAN Set_Debug_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_DebugFunc_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
#endif

#ifdef TXBF_SUPPORT
BOOLEAN Set_ReadITxBf_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_ReadETxBf_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_WriteITxBf_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_WriteETxBf_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_StatITxBf_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_StatETxBf_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_TxBfTag_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_ITxBfTimeout_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_ETxBfTimeout_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_InvTxBfTag_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_ITxBfCal_Proc(RTMP_ADAPTER *pAd, PSTRING arg);

#ifdef MT76x2
BOOLEAN mt76x2_Set_ITxBfCal_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
#endif

BOOLEAN Set_ITxBfDivCal_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_ITxBfLnaCal_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_ETxBfEnCond_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_ETxBfCodebook_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_ETxBfCoefficient_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_ETxBfGrouping_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_ETxBfNoncompress_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_ETxBfIncapable_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_StaETxBfEnCond_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_NoSndgCntThrd_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_NdpSndgStreams_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_Trigger_Sounding_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_ITxBfEn_Proc(RTMP_ADAPTER *pAd, PSTRING arg);

#ifdef MT76x2
BOOLEAN Set_TxBfProfileTag_Help(PRTMP_ADAPTER pAd, PSTRING arg);
BOOLEAN Set_TxBfProfileTagValid(PRTMP_ADAPTER pAd, PSTRING arg);
BOOLEAN Set_TxBfProfileTagRead(PRTMP_ADAPTER pAd, PSTRING arg);
BOOLEAN Set_TxBfProfileTagWrite(PRTMP_ADAPTER pAd, PSTRING arg);
BOOLEAN Set_TxBfProfileDataRead(PRTMP_ADAPTER pAd, PSTRING arg);
BOOLEAN Set_TxBfProfileDataWrite(PRTMP_ADAPTER pAd, PSTRING arg);
BOOLEAN Set_TxBfProfileDataWriteAll(PRTMP_ADAPTER pAd, PSTRING arg);
BOOLEAN Set_TxBfProfileDataReadAll(PRTMP_ADAPTER pAd, PSTRING arg);
BOOLEAN Set_TxBfProfileTag_TimeOut(PRTMP_ADAPTER pAd, PSTRING arg);
BOOLEAN Set_TxBfProfileTag_Matrix(PRTMP_ADAPTER pAd, PSTRING arg);
BOOLEAN Set_TxBfProfileTag_SNR(PRTMP_ADAPTER pAd, PSTRING arg);
BOOLEAN Set_TxBfProfileTag_TxScale(PRTMP_ADAPTER pAd, PSTRING arg);
BOOLEAN Set_TxBfProfileTag_MAC(PRTMP_ADAPTER pAd, PSTRING arg);
BOOLEAN Set_TxBfProfileTag_Flg(PRTMP_ADAPTER pAd, PSTRING arg);
#endif

#endif /* TXBF_SUPPORT */

#ifdef VHT_TXBF_SUPPORT
BOOLEAN Set_VhtNDPA_Sounding_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
#endif

BOOLEAN Set_RateAdaptInterval(RTMP_ADAPTER *pAd, PSTRING arg);

#ifdef PRE_ANT_SWITCH
BOOLEAN Set_PreAntSwitch_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_PreAntSwitchRSSI_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_PreAntSwitchTimeout_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
#endif /* PRE_ANT_SWITCH */

#ifdef CFO_TRACK
BOOLEAN Set_CFOTrack_Proc(RTMP_ADAPTER *pAd, PSTRING arg);

#ifdef CFO_TRACK
#ifdef CONFIG_AP_SUPPORT
BOOLEAN rtmp_cfo_track(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, int lastClient);
#endif /* CONFIG_AP_SUPPORT */
#endif /* CFO_TRACK */

#endif // CFO_TRACK //

#ifdef DBG_CTRL_SUPPORT
int Set_DebugFlags_Proc(RTMP_ADAPTER *pAd, PSTRING arg);

#ifdef INCLUDE_DEBUG_QUEUE
int Set_DebugQueue_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
void dbQueueEnqueue(UCHAR type, UCHAR *data);
void dbQueueEnqueueTxFrame(UCHAR *pTxWI, UCHAR *pDot11Hdr);
void dbQueueEnqueueRxFrame(UCHAR *pRxWI, UCHAR *pDot11Hdr ULONG flags);
#endif /* INCLUDE_DEBUG_QUEUE */
#endif /* DBG_CTRL_SUPPORT */

BOOLEAN Show_DescInfo_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Show_MacTable_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Show_sta_tr_proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN show_devinfo_proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN show_sysinfo_proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN show_trinfo_proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Show_TxInfo_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_ResetStatCounter_Proc(RTMP_ADAPTER *pAd, PSTRING arg);

#ifdef DOT11_N_SUPPORT
BOOLEAN Set_BASetup_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_BADecline_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_BAOriTearDown_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_BARecTearDown_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_HtBw_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_HtMcs_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_HtGi_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_HtOpMode_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_HtStbc_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_HtHtc_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_HtExtcha_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_HtMpduDensity_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_HtBaWinSize_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_HtRdg_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_HtLinkAdapt_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_HtAmsdu_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_HtAutoBa_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_HtProtect_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_HtMimoPs_Proc(RTMP_ADAPTER *pAd, PSTRING arg);

#ifdef DOT11N_DRAFT3
BOOLEAN Set_HT_BssCoex_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_HT_BssCoexApCntThr_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
#endif

#ifdef CONFIG_AP_SUPPORT
BOOLEAN Set_HtTxStream_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_HtRxStream_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
#ifdef DOT11_N_SUPPORT
#ifdef GREENAP_SUPPORT
BOOLEAN Set_GreenAP_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
#endif /* GREENAP_SUPPORT */
#endif /* DOT11_N_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

BOOLEAN SetCommonHT(RTMP_ADAPTER *pAd);

BOOLEAN Set_ForceShortGI_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_ForceGF_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_SendSMPSAction_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_HtMIMOPSmode_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_HtTxBASize_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_HtDisallowTKIP_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_BurstMode_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
#endif /* DOT11_N_SUPPORT */

#ifdef DOT11_VHT_AC
BOOLEAN Set_VhtBw_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_VhtStbc_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN set_VhtBwSignal_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_VhtDisallowNonVHT_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
#endif

#ifdef APCLI_SUPPORT
BOOLEAN RTMPIoctlConnStatus(RTMP_ADAPTER *pAd, PSTRING arg);
#endif

#ifdef CONFIG_STA_SUPPORT
void RTMPSendDLSTearDownFrame(RTMP_ADAPTER *pAd, UCHAR *pDA);

#ifdef DOT11_N_SUPPORT
void QueryBATABLE(PRTMP_ADAPTER pAd, PQUERYBA_TABLE pBAT);
#endif

#ifdef WPA_SUPPLICANT_SUPPORT
int WpaCheckEapCode(PRTMP_ADAPTER pAd, PUCHAR pFrame, USHORT FrameLen,
	USHORT OffSet);
#endif /* WPA_SUPPLICANT_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
void detect_wmm_traffic(RTMP_ADAPTER *pAd, UCHAR up, UCHAR bOutput);
void dynamic_tune_be_tx_op(RTMP_ADAPTER *pAd, ULONG nonBEpackets);
#endif /* CONFIG_AP_SUPPORT */

#ifdef DOT11_N_SUPPORT
void build_ext_channel_switch_ie(RTMP_ADAPTER *pAd,
	HT_EXT_CHANNEL_SWITCH_ANNOUNCEMENT_IE *pIE);

void assoc_ht_info_debugshow(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry,
	UCHAR ht_cap_len, HT_CAPABILITY_IE *pHTCapability);
#endif /* DOT11_N_SUPPORT */

BOOLEAN rtmp_rx_done_handle(RTMP_ADAPTER *pAd);

#ifdef DOT11_N_SUPPORT
void Indicate_AMPDU_Packet(PRTMP_ADAPTER pAd, RX_BLK *pRxBlk,
	UCHAR FromWhichBSSID);

#ifdef HDR_TRANS_SUPPORT
void Indicate_AMPDU_Packet_Hdr_Trns(PRTMP_ADAPTER pAd, RX_BLK *pRxBlk,
	UCHAR FromWhichBSSID);
#endif

/* AMSDU packet indication */
void Indicate_AMSDU_Packet(PRTMP_ADAPTER pAd, RX_BLK *pRxBlk,
	UCHAR FromWhichBSSID);

#endif /* DOT11_N_SUPPORT */

/* Normal legacy Rx packet indication */
void Indicate_Legacy_Packet(PRTMP_ADAPTER pAd, RX_BLK *pRxBlk,
	UCHAR FromWhichBSSID);

#ifdef HDR_TRANS_SUPPORT
void Indicate_Legacy_Packet_Hdr_Trns(PRTMP_ADAPTER pAd, RX_BLK *pRxBlk,
	UCHAR FromWhichBSSID);
#endif

void Indicate_EAPOL_Packet(PRTMP_ADAPTER pAd, RX_BLK *pRxBlk,
	UCHAR FromWhichBSSID);

#ifdef TXBF_SUPPORT
BOOLEAN clientSupportsETxBF(RTMP_ADAPTER *pAd, HT_BF_CAP *pTxBFCap);
void setETxBFCap(RTMP_ADAPTER *pAd, HT_BF_CAP *pTxBFCap);

#ifdef VHT_TXBF_SUPPORT
BOOLEAN clientSupportsVHTETxBF(RTMP_ADAPTER *pAd, VHT_CAP_INFO *pTxBFCapInfo);
void setVHTETxBFCap(RTMP_ADAPTER *pAd, VHT_CAP_INFO *pTxBFCap);
#endif

void handleBfFb(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk);
void TxBFInit(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, BOOLEAN supETxBF);
void eTxBFProbing(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry);

void Trigger_Sounding_Packet(RTMP_ADAPTER *pAd, UCHAR SndgType, UCHAR SndgBW,
	UCHAR SndgMcs, MAC_TABLE_ENTRY *pEntry);

void rtmp_asic_set_bf(RTMP_ADAPTER *pAd);
BOOLEAN rtmp_chk_itxbf_calibration(RTMP_ADAPTER *pAd);

#endif /* TXBF_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
/* remove LLC and get 802_3 Header */
#define  RTMP_AP_802_11_REMOVE_LLC_AND_CONVERT_TO_802_3(_pRxBlk, _pHeader802_3)		\
{											\
	PUCHAR _pRemovedLLCSNAP = NULL, _pDA, _pSA;					\
											\
	if (RX_BLK_TEST_FLAG(_pRxBlk, fRX_WDS) || RX_BLK_TEST_FLAG(_pRxBlk, fRX_MESH))	\
	{										\
		_pDA = _pRxBlk->pHeader->Addr3;						\
		_pSA = (PUCHAR)_pRxBlk->pHeader + sizeof(HEADER_802_11);		\
	}										\
	else if (RX_BLK_TEST_FLAG(_pRxBlk, fRX_APCLI))					\
	{										\
		_pDA = _pRxBlk->pHeader->Addr1; 					\
		_pSA = _pRxBlk->pHeader->Addr3;						\
	}										\
	else										\
	{										\
		_pDA = _pRxBlk->pHeader->Addr3;						\
		_pSA = _pRxBlk->pHeader->Addr2;						\
	}										\
											\
	CONVERT_TO_802_3(_pHeader802_3, _pDA, _pSA, _pRxBlk->pData,			\
		_pRxBlk->DataSize, _pRemovedLLCSNAP);					\
}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
/* remove LLC and get 802_3 Header */
#define  RTMP_802_11_REMOVE_LLC_AND_CONVERT_TO_802_3(_pRxBlk, _pHeader802_3)		\
{											\
	PUCHAR _pRemovedLLCSNAP = NULL, _pDA, _pSA; 					\
											\
	if (RX_BLK_TEST_FLAG(_pRxBlk, fRX_WDS) || RX_BLK_TEST_FLAG(_pRxBlk, fRX_MESH))	\
	{										\
		_pDA = _pRxBlk->pHeader->Addr3;						\
		_pSA = (PUCHAR)_pRxBlk->pHeader + sizeof(HEADER_802_11);		\
	}										\
	else 										\
	{										\
		if (RX_BLK_TEST_FLAG(_pRxBlk, fRX_INFRA))				\
		{									\
			_pDA = _pRxBlk->pHeader->Addr1;					\
		if (RX_BLK_TEST_FLAG(_pRxBlk, fRX_DLS))					\
			_pSA = _pRxBlk->pHeader->Addr2;					\
		else									\
			_pSA = _pRxBlk->pHeader->Addr3;					\
		}									\
		else									\
		{									\
			_pDA = _pRxBlk->pHeader->Addr1;					\
			_pSA = _pRxBlk->pHeader->Addr2;					\
		}									\
	}										\
											\
	CONVERT_TO_802_3(_pHeader802_3, _pDA, _pSA, _pRxBlk->pData, 			\
		_pRxBlk->DataSize, _pRemovedLLCSNAP);					\
}
#endif /* CONFIG_STA_SUPPORT */

BOOLEAN APFowardWirelessStaToWirelessSta(PRTMP_ADAPTER pAd, PNDIS_PACKET pPacket,
	ULONG FromWhichBSSID);

void Announce_or_Forward_802_3_Packet(PRTMP_ADAPTER pAd, PNDIS_PACKET pPacket,
	UCHAR FromWhichBSSID);

void Sta_Announce_or_Forward_802_3_Packet(PRTMP_ADAPTER pAd, PNDIS_PACKET pPacket,
	UCHAR FromWhichBSSID);

#ifdef CONFIG_AP_SUPPORT
#define AP_ANNOUNCE_OR_FORWARD_802_3_PACKET(_pAd, _pPacket, _FromWhichBSS)\
		Announce_or_Forward_802_3_Packet(_pAd, _pPacket, _FromWhichBSS);
#endif

#ifdef CONFIG_STA_SUPPORT
#define ANNOUNCE_OR_FORWARD_802_3_PACKET(_pAd, _pPacket, _FromWhichBSS)\
		Sta_Announce_or_Forward_802_3_Packet(_pAd, _pPacket, _FromWhichBSS);
		/*announce_802_3_packet(_pAd, _pPacket); */
#endif

/* Normal, AMPDU or AMSDU */
void CmmRxnonRalinkFrameIndicate(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk,
	UCHAR FromWhichBSSID);

#ifdef HDR_TRANS_SUPPORT
void CmmRxnonRalinkFrameIndicate_Hdr_Trns(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk,
	UCHAR FromWhichBSSID);
#endif

void CmmRxRalinkFrameIndicate(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, RX_BLK *pRxBlk,
	UCHAR FromWhichBSSID);

void Update_Rssi_Sample(RTMP_ADAPTER *pAd, RSSI_SAMPLE *pRssi, RXWI_STRUC *pRxWI);

PNDIS_PACKET GetPacketFromRxRing(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk,
	BOOLEAN *pbReschedule, UINT32 *pRxPending, BOOLEAN *bCmdRspPacket,
	UCHAR RxRingNo);

PNDIS_PACKET RTMPDeFragmentDataFrame(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk);

/*////////////////////////////////////*/

#if defined (AP_SCAN_SUPPORT) || defined (CONFIG_STA_SUPPORT)
void RTMPIoctlGetSiteSurvey(RTMP_ADAPTER *pAd, RTMP_IOCTL_INPUT_STRUCT *wrq);
#endif

#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
BOOLEAN Set_ApCli_AuthMode_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_ApCli_EncrypType_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_ApCli_Enable_Proc(PRTMP_ADAPTER pAd, PSTRING arg);
BOOLEAN Set_ApCli_Ssid_Proc(PRTMP_ADAPTER pAd, PSTRING arg);
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
typedef struct CountryCodeToCountryRegion {
	USHORT CountryNum;
	UCHAR IsoName[3];
	/*UCHAR CountryName[40]; */
	PSTRING pCountryName;
	BOOLEAN SupportABand;
	/*ULONG RegDomainNum11A; */
	UCHAR RegDomainNum11A;
	BOOLEAN SupportGBand;
	/*ULONG RegDomainNum11G; */
	UCHAR RegDomainNum11G;
} COUNTRY_CODE_TO_COUNTRY_REGION;
#endif /* CONFIG_AP_SUPPORT */

#ifdef SNMP_SUPPORT
/*for snmp */
typedef struct _DefaultKeyIdxValue
{
	UCHAR KeyIdx;
	UCHAR Value[16];
} DefaultKeyIdxValue, *PDefaultKeyIdxValue;
#endif

void STA_MonPktSend(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk);

#ifdef CONFIG_STA_SUPPORT
void RTMPSetDesiredRates(RTMP_ADAPTER *pAd, LONG Rates);
#endif

BOOLEAN Set_FixedTxMode_Proc(RTMP_ADAPTER *pAd, PSTRING arg);

#ifdef CONFIG_APSTA_MIXED_SUPPORT
BOOLEAN Set_OpMode_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
#endif

BOOLEAN Set_LongRetryLimit_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_ShortRetryLimit_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_AutoFallBack_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
void RT28XXDMAEnable(RTMP_ADAPTER *pAd);

void RT28xx_UpdateBeaconToAsic(RTMP_ADAPTER * pAd, int apidx, ULONG BeaconLen,
	ULONG UpdatePos);

NDIS_STATUS RtmpNetTaskInit(RTMP_ADAPTER *pAd);
NDIS_STATUS RtmpMgmtTaskInit(RTMP_ADAPTER *pAd);
void RtmpNetTaskExit(RTMP_ADAPTER *pAd);
void RtmpMgmtTaskExit(RTMP_ADAPTER *pAd);

void tbtt_tasklet(unsigned long data);

#ifdef CONFIG_STA_SUPPORT
#ifdef CREDENTIAL_STORE
NDIS_STATUS RecoverConnectInfo(RTMP_ADAPTER *pAd);
NDIS_STATUS StoreConnectInfo(RTMP_ADAPTER *pAd);
#endif /* CREDENTIAL_STORE */
#endif /* CONFIG_STA_SUPPORT */

void AsicTurnOffRFClk(RTMP_ADAPTER *pAd, UCHAR Channel);

#ifdef RTMP_TIMER_TASK_SUPPORT
int RtmpTimerQThread(ULONG Context);

RTMP_TIMER_TASK_ENTRY *RtmpTimerQInsert(RTMP_ADAPTER *pAd,
	RALINK_TIMER_STRUCT *pTimer);

BOOLEAN RtmpTimerQRemove(RTMP_ADAPTER *pAd, RALINK_TIMER_STRUCT *pTimer);

void RtmpTimerQExit(RTMP_ADAPTER *pAd);
void RtmpTimerQInit(RTMP_ADAPTER *pAd);
#endif /* RTMP_TIMER_TASK_SUPPORT */

#ifdef NEW_WOW_SUPPORT
void RT28xxAndesWOWEnable(RTMP_ADAPTER *pAd);
void RT28xxAndesWOWDisable(RTMP_ADAPTER *pAd);
#endif

#if (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT)
void RT28xxAsicWOWEnable(RTMP_ADAPTER *pAd);
void RT28xxAsicWOWDisable(RTMP_ADAPTER *pAd);
#endif /* (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) */

/*////////////////////////////////////*/

#ifdef AP_QLOAD_SUPPORT
void QBSS_LoadInit(RTMP_ADAPTER *pAd);
void QBSS_LoadAlarmReset(RTMP_ADAPTER *pAd);
void QBSS_LoadAlarmResume(RTMP_ADAPTER *pAd);
UINT32 QBSS_LoadBusyTimeGet(RTMP_ADAPTER *pAd);
BOOLEAN QBSS_LoadIsAlarmIssued(RTMP_ADAPTER *pAd);
BOOLEAN QBSS_LoadIsBusyTimeAccepted(RTMP_ADAPTER *pAd, UINT32 BusyTime);
UINT32 QBSS_LoadElementAppend(RTMP_ADAPTER *pAd, UINT8 *buf_p);
UINT32 QBSS_LoadElementParse(RTMP_ADAPTER *pAd, UINT8 *pElement, UINT16 *pStationCount,
	UINT8 *pChanUtil, UINT16 *pAvalAdmCap);

void QBSS_LoadUpdate(RTMP_ADAPTER *pAd, ULONG UpTime);
void QBSS_LoadStatusClear(RTMP_ADAPTER *pAd);

int Show_QoSLoad_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
#endif /* AP_QLOAD_SUPPORT */

int RTMPShowCfgValue(RTMP_ADAPTER *pAd, PSTRING name, PSTRING buf, UINT32 MaxLen);
PSTRING RTMPGetRalinkAuthModeStr(NDIS_802_11_AUTHENTICATION_MODE authMode);
PSTRING RTMPGetRalinkEncryModeStr(USHORT encryMode);

#ifdef CONFIG_STA_SUPPORT
BOOLEAN StaUpdateMacTableEntry(RTMP_ADAPTER *pAd, PMAC_TABLE_ENTRY pEntry,
	UCHAR MaxSupportedRateIn500Kbps, HT_CAPABILITY_IE *pHtCapability,
	UCHAR HtCapabilityLen, ADD_HT_INFO_IE *pAddHtInfo, UCHAR AddHtInfoLen,
	IE_LISTS *ie_list, USHORT CapabilityInfo);

BOOLEAN AUTH_ReqSend(PRTMP_ADAPTER pAd, PMLME_QUEUE_ELEM pElem,
	PRALINK_TIMER_STRUCT pAuthTimer, PSTRING pSMName, USHORT SeqNo,
	PUCHAR pNewElement, ULONG ElementLen);
#endif

void ReSyncBeaconTime(RTMP_ADAPTER *pAd);
void RTMPSetAGCInitValue(RTMP_ADAPTER *pAd, UCHAR BandWidth);

#ifdef TXBF_SUPPORT
void handleHtcField(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk);
#endif

#ifdef MFB_SUPPORT
void MFB_PerPareMRQ(RTMP_ADAPTER *pAd, void* pBuf, MAC_TABLE_ENTRY *pEntry);
void MFB_PerPareMFB(RTMP_ADAPTER *pAd, void *pBuf, MAC_TABLE_ENTRY *pEntry);
#endif

#define VIRTUAL_IF_INC(__pAd) ((__pAd)->VirtualIfCnt++)
#define VIRTUAL_IF_DEC(__pAd) ((__pAd)->VirtualIfCnt--)
#define VIRTUAL_IF_NUM(__pAd) ((__pAd)->VirtualIfCnt)

#ifdef RTMP_USB_SUPPORT
/*
 * Function Prototype in rtusb_bulk.c
 */

#ifdef INF_AMAZON_SE
void SoftwareFlowControl(RTMP_ADAPTER *pAd) ;
#endif

void RTUSBInitTxDesc(RTMP_ADAPTER *pAd, PTX_CONTEXT pTxContext,
	UCHAR BulkOutPipeId, usb_complete_t Func);

void RTUSBInitHTTxDesc(RTMP_ADAPTER *pAd, PHT_TX_CONTEXT pTxContext,
	UCHAR BulkOutPipeId, ULONG BulkOutSize, usb_complete_t Func);

void RTUSBInitRxDesc(RTMP_ADAPTER *pAd, RX_CONTEXT *pRxContext);
void RTUSBCleanUpDataBulkOutQueue(RTMP_ADAPTER *pAd);
void RTUSBCancelPendingBulkOutIRP(RTMP_ADAPTER *pAd);
void RTUSBCancelPendingBulkInIRP(RTMP_ADAPTER *pAd);
void RTUSBCancelPendingIRPs(RTMP_ADAPTER *pAd);
void RTUSBKickBulkOut(RTMP_ADAPTER *pAd);
void RTUSBBulkReceive(RTMP_ADAPTER *pAd);
void RTUSBBulkCmdRspEventReceive(RTMP_ADAPTER *pAd);
void RTUSBInitRxDesc(RTMP_ADAPTER *pAd, RX_CONTEXT *pRxContext);
void RTUSBBulkRxHandle(ULONG data);
void InitUSBDevice(RT_CMD_USB_INIT *pConfig, void *pAd);
#endif /* RTMP_USB_SUPPORT */

#ifdef SOFT_ENCRYPT
BOOLEAN RTMPExpandPacketForSwEncrypt(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk);
void RTMPUpdateSwCacheCipherInfo(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk, UCHAR *pHdr);
#endif

/*
	OS Related funciton prototype definitions.
	TODO: Maybe we need to move these function prototypes to other proper place.
*/
void RTInitializeCmdQ(PCmdQ cmdq);
void CMDHandler(RTMP_ADAPTER *pAd);
void RTThreadDequeueCmd(PCmdQ cmdq, PCmdQElmt *pcmdqelmt);

NDIS_STATUS RTEnqueueInternalCmd(RTMP_ADAPTER *pAd,NDIS_OID Oid,
	PVOID pInformationBuffer, UINT32 InformationBufferLength);

#ifdef HOSTAPD_SUPPORT
void ieee80211_notify_michael_failure(RTMP_ADAPTER *pAd, PHEADER_802_11 pHeader,
	UINT keyix, int report);

const CHAR* ether_sprintf(const UINT8 *mac);
#endif

#ifdef VENDOR_FEATURE3_SUPPORT
void RTMP_IO_WRITE32(RTMP_ADAPTER *pAd, UINT32 Offset, UINT32 Value);
#endif

BOOLEAN WaitForAsicReady(RTMP_ADAPTER *pAd);
BOOLEAN CHAN_PropertyCheck(RTMP_ADAPTER *pAd, UINT32 ChanNum, UCHAR Property);

#ifdef CONFIG_STA_SUPPORT

/* command */
BOOLEAN Set_SSID_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_NetworkType_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_AuthMode_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_EncrypType_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_Wep_Key_Proc(RTMP_ADAPTER  *pAd, PSTRING Key, int KeyLen, int KeyId);

#ifdef WPA_SUPPLICANT_SUPPORT
BOOLEAN Set_Wpa_Support(RTMP_ADAPTER *pAd, PSTRING arg);
#endif /* WPA_SUPPLICANT_SUPPORT */

#ifdef DBG
void RTMPIoctlMAC(RTMP_ADAPTER *pAd, RTMP_IOCTL_INPUT_STRUCT *wrq);
void RTMPIoctlE2PROM(RTMP_ADAPTER *pAd, RTMP_IOCTL_INPUT_STRUCT *wrq);
#endif /* DBG */

NDIS_STATUS RTMPWPANoneAddKeyProc(RTMP_ADAPTER *pAd, void *pBuf);

BOOLEAN Set_LongRetryLimit_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_ShortRetryLimit_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_SiteSurvey_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
void StaSiteSurvey(RTMP_ADAPTER *pAd, PNDIS_802_11_SSID pSsid, UCHAR ScanType);

int RtmpIoctl_rt_ioctl_siwauth(RTMP_ADAPTER *pAd, void *pData, ULONG Data);
#endif /* CONFIG_STA_SUPPORT */

void  getRate(HTTRANSMIT_SETTING HTSetting, ULONG* fLastTxRxRate);

#ifdef APCLI_SUPPORT
#ifdef WPA_SUPPLICANT_SUPPORT
void ApcliSendAssocIEsToWpaSupplicant(RTMP_ADAPTER *pAd, UINT ifIndex);

int ApcliWpaCheckEapCode(PRTMP_ADAPTER pAd, PUCHAR pFrame, USHORT FrameLen,
	USHORT OffSet);

void ApcliWpaSendEapolStart(RTMP_ADAPTER *pAd, PUCHAR pBssid,
	PMAC_TABLE_ENTRY pMacEntry, PAPCLI_STRUCT pApCliEntry);
#endif/*WPA_SUPPLICANT_SUPPORT*/

void ApCliRTMPSendNullFrame(RTMP_ADAPTER *pAd, UCHAR TxRate, BOOLEAN bQosNull,
	MAC_TABLE_ENTRY *pMacEntry, USHORT PwrMgmt);

#endif/*APCLI_SUPPORT*/

void RTMP_IndicateMediaState(RTMP_ADAPTER *pAd, NDIS_MEDIA_STATE media_state);

#if defined(RT3350) || defined(RT33xx)
void RTMP_TxEvmCalibration(RTMP_ADAPTER *pAd);
#endif

int RTMP_COM_IoctlHandle(void *pAdSrc, int cmd, void *pData, ULONG Data);

#ifdef CONFIG_AP_SUPPORT
int RTMP_AP_IoctlPrepare(RTMP_ADAPTER *pAd, void *pCB);
#endif

BOOLEAN Set_VcoPeriod_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
BOOLEAN Set_RateAlg_Proc(RTMP_ADAPTER *pAd, PSTRING arg);

#ifdef SINGLE_SKU
BOOLEAN Set_ModuleTxpower_Proc(RTMP_ADAPTER *pAd, PSTRING arg);
#endif

void RtmpEnqueueNullFrame(RTMP_ADAPTER *pAd, UCHAR *pAddr, UCHAR TxRate,
	UCHAR AID, UCHAR apidx, BOOLEAN bQosNull, BOOLEAN bEOSP, UCHAR OldUP);

void RtmpCleanupPsQueue(RTMP_ADAPTER *pAd, PQUEUE_HEADER   pQueue);

NDIS_STATUS RtmpInsertPsQueue(RTMP_ADAPTER *pAd, PNDIS_PACKET pPacket,
	MAC_TABLE_ENTRY *pMacEntry, UCHAR QueIdx);

void RtmpHandleRxPsPoll(RTMP_ADAPTER *pAd, UCHAR *pAddr, USHORT Aid, BOOLEAN isActive);
BOOLEAN RtmpPsIndicate(RTMP_ADAPTER *pAd, UCHAR *pAddr, UCHAR wcid, UCHAR Psm);

#ifdef CONFIG_STA_SUPPORT
BOOLEAN RtmpPktPmBitCheck(RTMP_ADAPTER *pAd);
void RtmpPsActiveExtendCheck(RTMP_ADAPTER *pAd);
void RtmpPsModeChange(RTMP_ADAPTER *pAd, UINT32 PsMode);
#endif

#ifdef DOT11_N_SUPPORT
void DisplayTxAgg (RTMP_ADAPTER *pAd);
#endif
void set_default_ap_edca_param(RTMP_ADAPTER *pAd);
void set_default_sta_edca_param(RTMP_ADAPTER *pAd);

UCHAR dot11_2_ra_rate(UCHAR MaxSupportedRateIn500Kbps);
UCHAR dot11_max_sup_rate(int SupRateLen, UCHAR *SupRate, int ExtRateLen, UCHAR *ExtRate);

void mgmt_tb_set_mcast_entry(RTMP_ADAPTER *pAd);
void set_entry_phy_cfg(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry);
void MacTableReset(RTMP_ADAPTER *pAd);
MAC_TABLE_ENTRY *MacTableLookup(RTMP_ADAPTER *pAd, UCHAR *pAddr);
BOOLEAN MacTableDeleteEntry(RTMP_ADAPTER *pAd, USHORT wcid, UCHAR *pAddr);

MAC_TABLE_ENTRY *MacTableInsertEntry(RTMP_ADAPTER *pAd, UCHAR *pAddr,
	struct wifi_dev *wdev, UCHAR apidx, UCHAR OpMode, BOOLEAN CleanAll);

#ifdef DBG
void dump_txwi(RTMP_ADAPTER *pAd, TXWI_STRUC *pTxWI);
void dump_rxwi(RTMP_ADAPTER *pAd, RXWI_STRUC *pRxWI);
void dump_rxinfo(RTMP_ADAPTER *pAd, RXINFO_STRUC *pRxInfo);
#endif

#ifdef WFA_VHT_PF
/* for SIGMA */
int set_vht_nss_mcs_cap(RTMP_ADAPTER *pAd, PSTRING arg);
int set_vht_nss_mcs_opt(RTMP_ADAPTER *pAd, PSTRING arg);
int set_vht_opmode_notify_ie(RTMP_ADAPTER *pAd, PSTRING arg);
int set_force_operating_mode(RTMP_ADAPTER *pAd, PSTRING arg);
int set_force_amsdu(RTMP_ADAPTER *pAd, PSTRING arg);
int set_force_noack(RTMP_ADAPTER *pAd, PSTRING arg);
int set_force_vht_sgi(RTMP_ADAPTER *pAd, PSTRING arg);
int set_force_vht_tx_stbc(RTMP_ADAPTER *pAd, PSTRING arg);
int set_force_ext_cca(RTMP_ADAPTER *pAd, PSTRING arg);
int set_rx_rts_cts(RTMP_ADAPTER *pAd, PSTRING arg);
#endif /* WFA_VHT_PF */

#ifdef DROP_MASK_SUPPORT
void asic_set_drop_mask(RTMP_ADAPTER *ad, USHORT wcid, BOOLEAN enable);
void asic_drop_mask_reset(RTMP_ADAPTER *ad);
void drop_mask_init_per_client(RTMP_ADAPTER *ad, MAC_TABLE_ENTRY *entry);
void drop_mask_release_per_client(RTMP_ADAPTER *ad, MAC_TABLE_ENTRY *entry);
void drop_mask_per_client_reset(RTMP_ADAPTER *ad);

void set_drop_mask_per_client(RTMP_ADAPTER *ad, MAC_TABLE_ENTRY *entry,
	UINT8 type, BOOLEAN enable);

void drop_mask_timer_action(PVOID SystemSpecific1, PVOID FunctionContext,
	PVOID SystemSpecific2, PVOID SystemSpecific3);
#endif /* DROP_MASK_SUPPORT */

#ifdef PEER_DELBA_TX_ADAPT
void Peer_DelBA_Tx_Adapt_Init(RTMP_ADAPTER *pAd, PMAC_TABLE_ENTRY pEntry);

void PeerDelBATxAdaptTimeOut(PVOID SystemSpecific1, PVOID FunctionContext,
	PVOID SystemSpecific2, PVOID SystemSpecific3);
#endif /* PEER_DELBA_TX_ADAPT */

#ifdef MULTI_CLIENT_SUPPORT
void asic_change_tx_retry(RTMP_ADAPTER *pAd, USHORT num);
void pkt_aggr_num_change(RTMP_ADAPTER *pAd, USHORT num);
void asic_tune_be_wmm(RTMP_ADAPTER *pAd, USHORT num);
#endif /* MULTI_CLIENT_SUPPORT */

BOOLEAN set_rf(RTMP_ADAPTER *pAd, PSTRING arg);
int write_reg(RTMP_ADAPTER *ad, u32 base, u16 offset, u32 value);
int read_reg(struct _RTMP_ADAPTER *ad, u32 base, u16 offset, u32 *value);
BOOLEAN show_pwr_info(RTMP_ADAPTER *ad, PSTRING arg);
#ifdef DBG_DIAGNOSE
BOOLEAN Show_Diag_Proc(PRTMP_ADAPTER pAd, PSTRING arg);
#endif

#endif  /* __RTMP_H__ */

