/****************************************************************************
 * Ralink Tech Inc.
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2013, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************/

/****************************************************************************

	Abstract:

	All CFG80211 Function Prototype.

***************************************************************************/

#ifndef __CFG80211EXTR_H__
#define __CFG80211EXTR_H__

#ifdef RT_CFG80211_SUPPORT

#define RT_CFG80211_DEBUG 		/* debug use */

#ifdef RT_CFG80211_DEBUG
#define CFG80211DBG(__Flg, __pMsg)	DBGPRINT(__Flg, __pMsg)
#else
#define CFG80211DBG(__Flg, __pMsg)
#endif

//CFG_TODO
#include "wfa_p2p.h"

#define RT_CFG80211_REGISTER(__pDev, __pNetDev)					\
	CFG80211_Register(__pDev, __pNetDev);

#define RT_CFG80211_BEACON_CR_PARSE(__pAd, __pVIE, __LenVIE)			\
	CFG80211_BeaconCountryRegionParse((void *)__pAd, __pVIE, __LenVIE);

#define RT_CFG80211_BEACON_TIM_UPDATE(__pAd)					\
        CFG80211_UpdateBeacon((void *)__pAd, NULL, 0, NULL, 0, FALSE);

#define RT_CFG80211_CRDA_REG_HINT(__pAd, __pCountryIe, __CountryIeLen)		\
	CFG80211_RegHint((void *)__pAd, __pCountryIe, __CountryIeLen);

#define RT_CFG80211_CRDA_REG_HINT11D(__pAd, __pCountryIe, __CountryIeLen)	\
	CFG80211_RegHint11D((void *)__pAd, __pCountryIe, __CountryIeLen);

#define RT_CFG80211_CRDA_REG_RULE_APPLY(__pAd)					\
	CFG80211_RegRuleApply((void *)__pAd, NULL, __pAd->cfg80211_ctrl.Cfg80211_Alpha2);

#define RT_CFG80211_CONN_RESULT_INFORM(__pAd, __pBSSID, __pReqIe, 		\
			__ReqIeLen,	__pRspIe, __RspIeLen, __FlgIsSuccess)	\
	CFG80211_ConnectResultInform((void *)__pAd, __pBSSID,			\
			__pReqIe, __ReqIeLen, __pRspIe, __RspIeLen, __FlgIsSuccess);

#define RT_CFG80211_SCANNING_INFORM(__pAd, __BssIdx, __ChanId, __pFrame,	\
			__FrameLen, __RSSI)					\
	CFG80211_Scaning((void *)__pAd, __BssIdx, __ChanId, __pFrame,		\
						__FrameLen, __RSSI);

#define RT_CFG80211_SCAN_END(__pAd, __FlgIsAborted)				\
	CFG80211_ScanEnd((void *)__pAd, __FlgIsAborted);
#ifdef CONFIG_STA_SUPPORT
#define RT_CFG80211_LOST_AP_INFORM(__pAd) 					\
	CFG80211_LostApInform((void *)__pAd);
#endif /*CONFIG_STA_SUPPORT*/
#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
#define RT_CFG80211_LOST_GO_INFORM(__pAd) 					\
	CFG80211_LostP2pGoInform((void *)__pAd);
#endif /*RT_CFG80211_P2P_CONCURRENT_DEVICE*/
#define RT_CFG80211_REINIT(__pAd)						\
	CFG80211_SupBandReInit((void *)__pAd);

#define RT_CFG80211_RFKILL_STATUS_UPDATE(_pAd, _active) 			\
	CFG80211_RFKillStatusUpdate(_pAd, _active);

#define RT_CFG80211_P2P_CLI_CONN_RESULT_INFORM(__pAd, __pBSSID, __pReqIe,	\
			__ReqIeLen,	__pRspIe, __RspIeLen, __FlgIsSuccess)	\
	CFG80211_P2pClientConnectResultInform(__pAd, __pBSSID,			\
			__pReqIe, __ReqIeLen, __pRspIe, __RspIeLen, __FlgIsSuccess);

#define RT_CFG80211_P2P_CLI_SEND_NULL_FRAME(_pAd, _PwrMgmt)			\
	CFG80211_P2pClientSendNullFrame(_pAd, _PwrMgmt);


#ifdef SINGLE_SKU
#define CFG80211_BANDINFO_FILL(__pAd, __pBandInfo)				\
{										\
	(__pBandInfo)->RFICType = __pAd->phy_ctrl.rf_band_cap;			\
	(__pBandInfo)->MpduDensity = __pAd->CommonCfg.BACapability.field.MpduDensity;\
	(__pBandInfo)->TxStream = __pAd->CommonCfg.TxStream;			\
	(__pBandInfo)->RxStream = __pAd->CommonCfg.RxStream;			\
	(__pBandInfo)->MaxTxPwr = __pAd->CommonCfg.DefineMaxTxPwr;		\
	if (WMODE_EQUAL(__pAd->CommonCfg.PhyMode, WMODE_B))			\
		(__pBandInfo)->FlgIsBMode = TRUE;				\
	else									\
		(__pBandInfo)->FlgIsBMode = FALSE;				\
	(__pBandInfo)->MaxBssTable = MAX_LEN_OF_BSS_TABLE;			\
	(__pBandInfo)->RtsThreshold = pAd->CommonCfg.RtsThreshold;		\
	(__pBandInfo)->FragmentThreshold = pAd->CommonCfg.FragmentThreshold;	\
	(__pBandInfo)->RetryMaxCnt = 0;						\
	RTMP_IO_READ32(__pAd, TX_RTY_CFG, &((__pBandInfo)->RetryMaxCnt));	\
}
#else
#define CFG80211_BANDINFO_FILL(__pAd, __pBandInfo)				\
{										\
	(__pBandInfo)->RFICType = __pAd->phy_ctrl.rf_band_cap;			\
	(__pBandInfo)->MpduDensity = __pAd->CommonCfg.BACapability.field.MpduDensity;\
	(__pBandInfo)->TxStream = __pAd->CommonCfg.TxStream;			\
	(__pBandInfo)->RxStream = __pAd->CommonCfg.RxStream;			\
	(__pBandInfo)->MaxTxPwr = 0;						\
	if (WMODE_EQUAL(__pAd->CommonCfg.PhyMode, WMODE_B))			\
		(__pBandInfo)->FlgIsBMode = TRUE;				\
	else									\
		(__pBandInfo)->FlgIsBMode = FALSE;				\
	(__pBandInfo)->MaxBssTable = MAX_LEN_OF_BSS_TABLE;			\
	(__pBandInfo)->RtsThreshold = pAd->CommonCfg.RtsThreshold;		\
	(__pBandInfo)->FragmentThreshold = pAd->CommonCfg.FragmentThreshold;	\
	(__pBandInfo)->RetryMaxCnt = 0;						\
	RTMP_IO_READ32(__pAd, TX_RTY_CFG, &((__pBandInfo)->RetryMaxCnt));	\
}
#endif /* SINGLE_SKU */

/* Scan Releated */
#ifdef CONFIG_STA_SUPPORT
BOOLEAN CFG80211DRV_OpsScanRunning(void *pAdOrg);
#endif /*CONFIG_STA_SUPPORT*/
BOOLEAN CFG80211DRV_OpsScanSetSpecifyChannel(
	void *pAdOrg, void *pData, UINT8 dataLen);

BOOLEAN CFG80211DRV_OpsScanCheckStatus(
	void *pAdOrg, UINT8 IfType);

BOOLEAN CFG80211DRV_OpsScanExtraIesSet(void *pAdOrg);

void CFG80211DRV_OpsScanInLinkDownAction(void *pAdOrg);

int CFG80211DRV_OpsScanGetNextChannel(void *pAdOrg);

void CFG80211_ScanStatusLockInit(void *pAdCB, UINT init);

void CFG80211_Scaning(
	void *pAdCB, UINT32 BssIdx, UINT32 ChanId, UCHAR *pFrame, UINT32 FrameLen, INT32 RSSI);

void CFG80211_ScanEnd(void *pAdCB, BOOLEAN FlgIsAborted);

/* Connect Releated */

void CFG80211_P2pClientConnectResultInform(void *pAdCB, UCHAR *pBSSID, UCHAR *pReqIe, 
	UINT32 ReqIeLen, UCHAR *pRspIe, UINT32 RspIeLen, UCHAR FlgIsSuccess);

void CFG80211_ConnectResultInform(void *pAdCB, UCHAR *pBSSID, UCHAR *pReqIe, 
	UINT32 ReqIeLen, UCHAR *pRspIe, UINT32 RspIeLen, UCHAR FlgIsSuccess);

void CFG80211DRV_PmkidConfig(void *pAdOrg, void *pData);

#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
void CFG80211_LostP2pGoInform(void *pAdCB);
#endif

void CFG80211_LostApInform(void *pAdCB);
int CFG80211_StaPortSecured(void *pAdCB, UCHAR *pMac, UINT flag);

/* AP Related*/
int CFG80211_ApStaDel(void *pAdCB, UCHAR *pMac);

void CFG80211_UpdateBeacon(void *pAdOrg, UCHAR *beacon_head_buf, UINT32 beacon_head_len,
	UCHAR *beacon_tail_buf, UINT32 beacon_tail_len, BOOLEAN isAllUpdate);

int CFG80211_ApStaDelSendEvent(PRTMP_ADAPTER pAd, const PUCHAR mac_addr);

/* Key Releated */
BOOLEAN CFG80211DRV_ApKeyAdd(void *pAdOrg, void *pData);
BOOLEAN CFG80211DRV_ApKeyDel(void *pAdOrg, void *pData);
void CFG80211DRV_RtsThresholdAdd(void *pAdOrg, UINT threshold);
void CFG80211DRV_FragThresholdAdd(void *pAdOrg, UINT threshold);
int CFG80211_setApDefaultKey(void *pAdCB, UINT Data);
int CFG80211_setPowerMgmt(void *pAdCB, UINT Enable);

/* General Releated */

BOOLEAN CFG80211DRV_OpsChgVirtualInf(RTMP_ADAPTER *pAd, void *pData);

int CFG80211DRV_IoctlHandle(void *pAdSrc, int cmd, void *pData, ULONG Data);

UCHAR CFG80211_getCenCh(RTMP_ADAPTER *pAd, UCHAR prim_ch);
void CFG80211_RegHint(void *pAdCB, UCHAR *pCountryIe, ULONG CountryIeLen);
void CFG80211_RegHint11D(void *pAdCB, UCHAR *pCountryIe, ULONG CountryIeLen);
void CFG80211_RegRuleApply(void *pAdCB, void *pWiphy, UCHAR *pAlpha2);
BOOLEAN CFG80211_SupBandReInit(void *pAdCB);

#ifdef RFKILL_HW_SUPPORT
void CFG80211_RFKillStatusUpdate(PVOID pAd, BOOLEAN active);
#endif /* RFKILL_HW_SUPPORT */

/* P2P Related */
void CFG80211DRV_SetP2pCliAssocIe(void *pAdOrg, void *pData, UINT ie_len);
void CFG80211DRV_P2pClientKeyAdd(void *pAdOrg, void *pData);
BOOLEAN CFG80211DRV_P2pClientConnect(void *pAdOrg, void *pData);
BOOLEAN CFG80211_checkScanTable(void *pAdCB);
void CFG80211_P2pClientSendNullFrame(void *pAdCB, int PwrMgmt);
BOOLEAN CFG80211DRV_OpsRemainOnChannel(void *pAdOrg, void *pData, UINT32 duration);
BOOLEAN CFG80211DRV_OpsCancelRemainOnChannel(void *pAdOrg, UINT32 cookie);
BOOLEAN CFG80211_CheckActionFrameType(RTMP_ADAPTER  *pAd, PUCHAR preStr, 
		void *pData, UINT32 length);
BOOLEAN CFG80211_SyncPacketWmmIe(RTMP_ADAPTER *pAd, void *pData, ULONG dataLen);
BOOLEAN CFG80211_HandleP2pMgmtFrame(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, UCHAR OpMode);
int CFG80211_SendMgmtFrame(RTMP_ADAPTER *pAd, void *pData, ULONG Data);

#ifdef RT_CFG80211_P2P_SUPPORT
void CFG80211_PeerP2pBeacon(PRTMP_ADAPTER pAd,PUCHAR pAddr2, LME_QUEUE_ELEM *Elem,
	LARGE_INTEGER TimeStamp);

BOOLEAN CFG80211_P2pResetNoATimer(PRTMP_ADAPTER pAd, ULONG DiffTimeInus);

BOOLEAN CFG80211_P2pHandleNoAAttri(PRTMP_ADAPTER pAd, PMAC_TABLE_ENTRY pMacClient,
	PUCHAR pData);
void CFG80211_ParseBeaconIE(RTMP_ADAPTER *pAd, MULTISSID_STRUCT *pMbss, 
	struct wifi_dev *wdev,UCHAR *wpa_ie,UCHAR *rsn_ie);

#endif /* RT_CFG80211_P2P_SUPPORT */

//--------------------------------
void CFG80211_Convert802_3Packet(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, UCHAR *pHeader802_3);
void CFG80211_Announce802_3Packet(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, UCHAR FromWhichBSSID);
void CFG80211_SendMgmtFrameDone(RTMP_ADAPTER *pAd, USHORT Sequence);
void CFG80211_SwitchTxChannel(RTMP_ADAPTER *pAd, ULONG Data);
BOOLEAN CFG80211DRV_OpsBeaconSet(void *pAdOrg, void *pData);
BOOLEAN CFG80211DRV_OpsBeaconAdd(void *pAdOrg, void *pData);
void CFG80211DRV_DisableApInterface(PRTMP_ADAPTER pAd);
BOOLEAN CFG80211DRV_OpsVifAdd(void *pAdOrg, void *pData);

#endif /* RT_CFG80211_SUPPORT */

#endif /* __CFG80211EXTR_H__ */
