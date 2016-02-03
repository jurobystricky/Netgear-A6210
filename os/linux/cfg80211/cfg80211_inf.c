/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2012, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	cfg80211_inf.c

	Abstract:

	Revision History:
	Who		When			What
	--------	----------		-----------------------------
	YF Luo		06-28-2012		Init version
			12-26-2013		Integration of NXTC
*/
#define RTMP_MODULE_OS

#include <linux/rtnetlink.h>
#include "rt_config.h"
#include "rtmp_comm.h"
#include "rt_os_util.h"
#include "rt_os_net.h"

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,28))
#ifdef RT_CFG80211_SUPPORT

extern int ApCliAllowToSendPacket(RTMP_ADAPTER *pAd, struct wifi_dev *wdev,
	PNDIS_PACKET pPacket, UCHAR *pWcid);

BOOLEAN CFG80211DRV_OpsChgVirtualInf(RTMP_ADAPTER *pAd, void *pData)
{
#ifdef RT_CFG80211_P2P_SINGLE_DEVICE
	PCFG80211_CTRL pCfg80211_ctrl = &pAd->cfg80211_ctrl;
#endif
	UINT newType, oldType;
	CMD_RTPRIV_IOCTL_80211_VIF_PARM *pVifParm;
	pVifParm = (CMD_RTPRIV_IOCTL_80211_VIF_PARM *)pData;

	newType = pVifParm->newIfType;
	oldType = pVifParm->oldIfType;

#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
	/* After P2P NEGO phase, the device type may be change from GC to GO
	   or no change. We remove the GC in VIF list if nego as GO case.
	 */
	if ((newType == RT_CMD_80211_IFTYPE_P2P_GO) &&
	   (oldType == RT_CMD_80211_IFTYPE_P2P_CLIENT)) {
		RTMP_CFG80211_VirtualIF_CancelP2pClient(pAd);
	}
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */

#ifdef RT_CFG80211_P2P_SINGLE_DEVICE
	CFG80211DBG(RT_DEBUG_TRACE, ("80211> @@@ Change from %u  to %u Mode\n",oldType,newType));

	pCfg80211_ctrl->P2POpStatusFlags = CFG_P2P_DISABLE;
	if (newType == RT_CMD_80211_IFTYPE_P2P_CLIENT) {
		pCfg80211_ctrl->P2POpStatusFlags = CFG_P2P_CLI_UP;
	} else if (newType == RT_CMD_80211_IFTYPE_P2P_GO) {
		pCfg80211_ctrl->P2POpStatusFlags = CFG_P2P_GO_UP;
	}
#endif /* RT_CFG80211_P2P_SINGLE_DEVICE */

#ifdef CONFIG_STA_SUPPORT
	/* Change Device Type */
	if (newType == RT_CMD_80211_IFTYPE_ADHOC) {
		Set_NetworkType_Proc(pAd, "Adhoc");
	} else if ((newType == RT_CMD_80211_IFTYPE_STATION) ||
		(newType == RT_CMD_80211_IFTYPE_P2P_CLIENT)) {
		CFG80211DBG(RT_DEBUG_TRACE, ("80211> Change the Interface to STA Mode\n"));

#ifdef CONFIG_AP_SUPPORT
		if (pAd->cfg80211_ctrl.isCfgInApMode == RT_CMD_80211_IFTYPE_AP && RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP))
			CFG80211DRV_DisableApInterface(pAd);
#endif /* CONFIG_AP_SUPPORT */

		pAd->cfg80211_ctrl.isCfgInApMode = RT_CMD_80211_IFTYPE_STATION;
	} else
#endif /*CONFIG_STA_SUPPORT*/
		if ((newType == RT_CMD_80211_IFTYPE_AP) ||
			 (newType == RT_CMD_80211_IFTYPE_P2P_GO)) {
		CFG80211DBG(RT_DEBUG_TRACE, ("80211> Change the Interface to AP Mode\n"));
		pAd->cfg80211_ctrl.isCfgInApMode = RT_CMD_80211_IFTYPE_AP;
	}
#ifdef CONFIG_STA_SUPPORT
	else if (newType == RT_CMD_80211_IFTYPE_MONITOR) {
		/* set packet filter */
		Set_NetworkType_Proc(pAd, "Monitor");

		if (pVifParm->MonFilterFlag != 0) {
			UINT32 Filter;

			RTMP_IO_READ32(pAd, RX_FILTR_CFG, &Filter);

			if ((pVifParm->MonFilterFlag & RT_CMD_80211_FILTER_FCSFAIL) == RT_CMD_80211_FILTER_FCSFAIL) {
				Filter = Filter & (~0x01);
			} else {
				Filter = Filter | 0x01;
			}

			if ((pVifParm->MonFilterFlag & RT_CMD_80211_FILTER_PLCPFAIL) == RT_CMD_80211_FILTER_PLCPFAIL) {
				Filter = Filter & (~0x02);
			} else {
				Filter = Filter | 0x02;
			}

			if ((pVifParm->MonFilterFlag & RT_CMD_80211_FILTER_CONTROL) == RT_CMD_80211_FILTER_CONTROL) {
				Filter = Filter & (~0xFF00);
			} else {
				Filter = Filter | 0xFF00;
			}

			if ((pVifParm->MonFilterFlag & RT_CMD_80211_FILTER_OTHER_BSS) == RT_CMD_80211_FILTER_OTHER_BSS) {
				Filter = Filter & (~0x08);
			} else {
				Filter = Filter | 0x08;
			}

			RTMP_IO_WRITE32(pAd, RX_FILTR_CFG, Filter);
			pVifParm->MonFilterFlag = Filter;
		}
	}
#endif /*CONFIG_STA_SUPPORT*/

	if ((newType == RT_CMD_80211_IFTYPE_P2P_CLIENT) ||
	   (newType == RT_CMD_80211_IFTYPE_P2P_GO)) {
		COPY_MAC_ADDR(pAd->cfg80211_ctrl.P2PCurrentAddress, pVifParm->net_dev->dev_addr);
	} else {
#ifdef RT_CFG80211_P2P_SUPPORT
		pCfg80211_ctrl->bP2pCliPmEnable = FALSE;
		pCfg80211_ctrl->bPreKeepSlient = FALSE;
		pCfg80211_ctrl->bKeepSlient = FALSE;
		pCfg80211_ctrl->NoAIndex = MAX_LEN_OF_MAC_TABLE;
		pCfg80211_ctrl->MyGOwcid = MAX_LEN_OF_MAC_TABLE;
		pCfg80211_ctrl->CTWindows= 0;   /* CTWindows and OppPS parameter field */
#endif /* RT_CFG80211_P2P_SUPPORT */
	}

	return TRUE;
}

#ifdef RT_CFG80211_P2P_SUPPORT
BOOLEAN RTMP_CFG80211_VIF_P2P_GO_ON(void *pAdSrc)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
	PNET_DEV pNetDev = NULL;

	if ((pAd->cfg80211_ctrl.Cfg80211VifDevSet.vifDevList.size > 0) &&
		((pNetDev = RTMP_CFG80211_FindVifEntry_ByType(pAd, RT_CMD_80211_IFTYPE_P2P_GO)) != NULL))
		return TRUE;
	else
		return FALSE;
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */

#ifdef RT_CFG80211_P2P_SINGLE_DEVICE
	if(CFG80211_P2P_TEST_BIT(pAd->cfg80211_ctrl.P2POpStatusFlags, CFG_P2P_GO_UP))
		return TRUE;
	else
		return FALSE;
#endif /* RT_CFG80211_P2P_SINGLE_DEVICE */

	return FALSE;
}

BOOLEAN RTMP_CFG80211_VIF_P2P_CLI_ON(void *pAdSrc)
{
		PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
		PNET_DEV pNetDev = NULL;

	if ((pAd->cfg80211_ctrl.Cfg80211VifDevSet.vifDevList.size > 0) &&
		((pNetDev = RTMP_CFG80211_FindVifEntry_ByType(pAd, RT_CMD_80211_IFTYPE_P2P_CLIENT)) != NULL))
		return TRUE;
	else
		return FALSE;
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */

#ifdef RT_CFG80211_P2P_SINGLE_DEVICE
	if(CFG80211_P2P_TEST_BIT(pAd->cfg80211_ctrl.P2POpStatusFlags, CFG_P2P_CLI_UP))
		return TRUE;
	else
		return FALSE;
#endif /* RT_CFG80211_P2P_SINGLE_DEVICE */

	return FALSE;
}
#endif /* RT_CFG80211_P2P_SUPPORT */


BOOLEAN CFG80211DRV_OpsVifAdd(void *pAdOrg, void *pData)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	CMD_RTPRIV_IOCTL_80211_VIF_SET *pVifInfo;
	pVifInfo = (CMD_RTPRIV_IOCTL_80211_VIF_SET *)pData;

	/* Already one VIF in list */
	if (pAd->cfg80211_ctrl.Cfg80211VifDevSet.isGoingOn)
		return FALSE;

	pAd->cfg80211_ctrl.Cfg80211VifDevSet.isGoingOn = TRUE;
	RTMP_CFG80211_VirtualIF_Init(pAd, pVifInfo->vifName, pVifInfo->vifType);
	return TRUE;
}

BOOLEAN RTMP_CFG80211_VIF_ON(void *pAdSrc)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	return pAd->cfg80211_ctrl.Cfg80211VifDevSet.isGoingOn;
}


static
PCFG80211_VIF_DEV RTMP_CFG80211_FindVifEntry_ByMac(void *pAdSrc, PNET_DEV pNewNetDev)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	PLIST_HEADER  pCacheList = &pAd->cfg80211_ctrl.Cfg80211VifDevSet.vifDevList;
	PCFG80211_VIF_DEV pDevEntry = NULL;
	PLIST_ENTRY pListEntry = NULL;

	pListEntry = pCacheList->pHead;
	pDevEntry = (PCFG80211_VIF_DEV)pListEntry;
	while (pDevEntry != NULL) {
		if (RTMPEqualMemory(pDevEntry->net_dev->dev_addr, pNewNetDev->dev_addr, MAC_ADDR_LEN))
			return pDevEntry;

		pListEntry = pListEntry->pNext;
		pDevEntry = (PCFG80211_VIF_DEV)pListEntry;
	}

	return NULL;
}

PNET_DEV RTMP_CFG80211_FindVifEntry_ByType(void *pAdSrc, UINT32 devType)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	PLIST_HEADER  pCacheList = &pAd->cfg80211_ctrl.Cfg80211VifDevSet.vifDevList;
	PCFG80211_VIF_DEV pDevEntry = NULL;
	PLIST_ENTRY pListEntry = NULL;

	pListEntry = pCacheList->pHead;
	pDevEntry = (PCFG80211_VIF_DEV)pListEntry;
	while (pDevEntry != NULL) {
		if (pDevEntry->devType == devType)
			return pDevEntry->net_dev;

		pListEntry = pListEntry->pNext;
		pDevEntry = (PCFG80211_VIF_DEV)pListEntry;
	}

	return NULL;
}

PWIRELESS_DEV RTMP_CFG80211_FindVifEntryWdev_ByType(void *pAdSrc, UINT32 devType)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	PLIST_HEADER  pCacheList = &pAd->cfg80211_ctrl.Cfg80211VifDevSet.vifDevList;
	PCFG80211_VIF_DEV pDevEntry = NULL;
	PLIST_ENTRY pListEntry = NULL;

	pListEntry = pCacheList->pHead;
	pDevEntry = (PCFG80211_VIF_DEV)pListEntry;
	while (pDevEntry != NULL) {
		if (pDevEntry->devType == devType)
			return pDevEntry->net_dev->ieee80211_ptr;

		pListEntry = pListEntry->pNext;
		pDevEntry = (PCFG80211_VIF_DEV)pListEntry;
	}

	return NULL;
}

void RTMP_CFG80211_AddVifEntry(void *pAdSrc, PNET_DEV pNewNetDev, UINT32 DevType)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	PCFG80211_VIF_DEV pNewVifDev;

	pNewVifDev = os_alloc_mem(sizeof(CFG80211_VIF_DEV));
	if (pNewVifDev) {
		NdisZeroMemory(pNewVifDev, sizeof(CFG80211_VIF_DEV));

		pNewVifDev->pNext = NULL;
		pNewVifDev->net_dev = pNewNetDev;
		pNewVifDev->devType = DevType;
		NdisZeroMemory(pNewVifDev->CUR_MAC, MAC_ADDR_LEN);
		NdisCopyMemory(pNewVifDev->CUR_MAC, pNewNetDev->dev_addr, MAC_ADDR_LEN);

		insertTailList(&pAd->cfg80211_ctrl.Cfg80211VifDevSet.vifDevList, (PLIST_ENTRY)pNewVifDev);
		DBGPRINT(RT_DEBUG_TRACE, ("Add CFG80211 VIF Device, Type: %d.\n", pNewVifDev->devType));
	}
}

void RTMP_CFG80211_RemoveVifEntry(void *pAdSrc, PNET_DEV pNewNetDev)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	PLIST_ENTRY pListEntry = NULL;

	pListEntry = (PLIST_ENTRY)RTMP_CFG80211_FindVifEntry_ByMac(pAd, pNewNetDev);
	if (pListEntry) {
		delEntryList(&pAd->cfg80211_ctrl.Cfg80211VifDevSet.vifDevList, pListEntry);
		os_free_mem(pListEntry);
	} else {
		DBGPRINT(RT_DEBUG_ERROR, ("Error in RTMP_CFG80211_RemoveVifEntry.\n"));
	}
}

PNET_DEV RTMP_CFG80211_VirtualIF_Get(void *pAdSrc)
{
	//PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	//return pAd->Cfg80211VifDevSet.Cfg80211VifDev[0].net_dev;
	return NULL;
}

#ifdef RT_CFG80211_P2P_SUPPORT
void RTMP_CFG80211_VirtualIF_CancelP2pClient(void *pAdSrc)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	PLIST_HEADER  pCacheList = &pAd->cfg80211_ctrl.Cfg80211VifDevSet.vifDevList;
	PCFG80211_VIF_DEV pDevEntry = NULL;
	PLIST_ENTRY pListEntry = NULL;

	DBGPRINT(RT_DEBUG_TRACE, ("==> %s\n", __FUNCTION__));

	pListEntry = pCacheList->pHead;
	pDevEntry = (PCFG80211_VIF_DEV)pListEntry;
	while (pDevEntry != NULL) {
		if (pDevEntry->devType == RT_CMD_80211_IFTYPE_P2P_CLIENT) {
			DBGPRINT(RT_DEBUG_ERROR, ("==> RTMP_CFG80211_VirtualIF_CancelP2pClient HIT.\n"));
			pDevEntry->devType = RT_CMD_80211_IFTYPE_P2P_GO;
			break;
		}

		pListEntry = pListEntry->pNext;
		pDevEntry = (PCFG80211_VIF_DEV)pListEntry;
	}

	pAd->flg_apcli_init = FALSE;
	pAd->ApCfg.ApCliTab[MAIN_MBSSID].wdev.if_dev = NULL;

	DBGPRINT(RT_DEBUG_TRACE, ("<== %s\n", __FUNCTION__));
}
#endif /* RT_CFG80211_P2P_SUPPORT */

static int CFG80211_VirtualIF_Open(PNET_DEV dev_p) {
	void *pAdSrc;
	PRTMP_ADAPTER pAd;
	pAdSrc = RtmpOsGetNetDevPriv(dev_p);
	ASSERT(pAdSrc);
	pAd = (PRTMP_ADAPTER)pAdSrc;

	DBGPRINT(RT_DEBUG_TRACE, ("%s: ===> %d,%s\n", __FUNCTION__,
			dev_p->ifindex, RTMP_OS_NETDEV_GET_DEVNAME(dev_p)));

	//if (VIRTUAL_IF_UP(pAd) != 0)
	//	return -1;

	/* increase MODULE use count */
	RT_MOD_INC_USE_COUNT();

#ifdef RT_CFG80211_P2P_SUPPORT
	if (dev_p->ieee80211_ptr->iftype == RT_CMD_80211_IFTYPE_P2P_CLIENT) {
		DBGPRINT(RT_DEBUG_TRACE, ("CFG80211_VirtualIF_Open\n"));
		pAd->flg_apcli_init = TRUE;
		ApCli_Open(pAd, dev_p);
		return 0;
	}
#endif /* RT_CFG80211_P2P_SUPPORT */

	RTMP_OS_NETDEV_START_QUEUE(dev_p);
	DBGPRINT(RT_DEBUG_TRACE, ("%s: <=== %s\n", __FUNCTION__, RTMP_OS_NETDEV_GET_DEVNAME(dev_p)));

	return 0;
}

static int CFG80211_VirtualIF_Close(PNET_DEV dev_p)
{
	void *pAdSrc;
	PRTMP_ADAPTER pAd;

	pAdSrc = RtmpOsGetNetDevPriv(dev_p);
	ASSERT(pAdSrc);
	pAd = (PRTMP_ADAPTER)pAdSrc;

#ifdef RT_CFG80211_P2P_SUPPORT
	if (dev_p->ieee80211_ptr->iftype == RT_CMD_80211_IFTYPE_P2P_CLIENT) {
		DBGPRINT(RT_DEBUG_TRACE, ("CFG80211_VirtualIF_Close\n"));
		CFG80211OS_ScanEnd(pAd->pCfg80211_CB, TRUE);
		RT_MOD_DEC_USE_COUNT();
		return ApCli_Close(pAd, dev_p);
	}
#endif /* RT_CFG80211_P2P_SUPPORT */
	DBGPRINT(RT_DEBUG_TRACE, ("%s: ===> %s\n", __FUNCTION__, RTMP_OS_NETDEV_GET_DEVNAME(dev_p)));

	RTMP_OS_NETDEV_STOP_QUEUE(dev_p);

	if (netif_carrier_ok(dev_p))
		netif_carrier_off(dev_p);
#ifdef CONFIG_STA_SUPPORT
	if (INFRA_ON(pAd))
		AsicEnableBssSync(pAd);
	else if (ADHOC_ON(pAd))
		AsicEnableIbssSync(pAd);
#else
	else
		AsicDisableSync(pAd);
#endif

	//VIRTUAL_IF_DOWN(pAd);

	RT_MOD_DEC_USE_COUNT();
	return 0;
}

static int CFG80211_PacketSend(PNDIS_PACKET pPktSrc, PNET_DEV pDev, RTMP_NET_PACKET_TRANSMIT Func)
{
	PRTMP_ADAPTER pAd;
	pAd = RtmpOsGetNetDevPriv(pDev);
	ASSERT(pAd);

	/* To Indicate from Which VIF */
	switch (pDev->ieee80211_ptr->iftype) {
	case RT_CMD_80211_IFTYPE_AP:
		//minIdx = MIN_NET_DEVICE_FOR_CFG80211_VIF_AP;
		RTMP_SET_PACKET_OPMODE(pPktSrc, OPMODE_AP);
		break;

	case RT_CMD_80211_IFTYPE_P2P_GO:;
		//minIdx = MIN_NET_DEVICE_FOR_CFG80211_VIF_P2P_GO;
		if (!OPSTATUS_TEST_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED)) {
			DBGPRINT(RT_DEBUG_TRACE,
					("Drop the Packet due P2P GO not in ready state\n"));
			dev_kfree_skb_any(pPktSrc);
			return 0;
		}
		RTMP_SET_PACKET_OPMODE(pPktSrc, OPMODE_AP);
		break;

	case RT_CMD_80211_IFTYPE_P2P_CLIENT:
		//minIdx = MIN_NET_DEVICE_FOR_CFG80211_VIF_P2P_CLI;
		RTMP_SET_PACKET_OPMODE(pPktSrc, OPMODE_AP);
		break;

	case RT_CMD_80211_IFTYPE_STATION:
	default:
		DBGPRINT(RT_DEBUG_TRACE, ("Unknown CFG80211 I/F Type(%d)\n",
				pDev->ieee80211_ptr->iftype));
		dev_kfree_skb_any(pPktSrc);
		return 0;
	}

	DBGPRINT(RT_DEBUG_INFO, ("CFG80211 Packet Type  [%s](%d)\n",
			pDev->name, pDev->ieee80211_ptr->iftype));

	RTMP_SET_PACKET_NET_DEVICE_MBSSID(pPktSrc, MAIN_MBSSID);

	return Func(RTPKT_TO_OSPKT(pPktSrc));
}

static int CFG80211_VirtualIF_PacketSend(struct sk_buff *skb, PNET_DEV dev_p)
{
	struct wifi_dev *wdev;

	DBGPRINT(RT_DEBUG_INFO, ("%s ---> %d\n",
			__FUNCTION__, dev_p->ieee80211_ptr->iftype));

	if (!(RTMP_OS_NETDEV_STATE_RUNNING(dev_p))) {
		/* the interface is down */
		dev_kfree_skb_any(skb);
		return 0;
	}

	/* The device not ready to send packt. */
	wdev = RtmpOsGetNetDevWdev(dev_p);
	ASSERT(wdev);
	if (!wdev)
		return -1;

	return CFG80211_PacketSend(skb, dev_p, rt28xx_packet_xmit);
}

static int CFG80211_VirtualIF_Ioctl(PNET_DEV dev_p, void *rq_p, int cmd)
{
	RTMP_ADAPTER *pAd = RtmpOsGetNetDevPriv(dev_p);
	ASSERT(pAd);

	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
		return -ENETDOWN;

	DBGPRINT(RT_DEBUG_TRACE, ("%s --->\n", __FUNCTION__));

	return rt28xx_ioctl(dev_p, rq_p, cmd);

}

void RTMP_CFG80211_VirtualIF_Init(void *pAdSrc, CHAR *pDevName, UINT32 DevType)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	RTMP_OS_NETDEV_OP_HOOK	netDevHook, *pNetDevOps;
	PNET_DEV new_dev_p;
#ifdef RT_CFG80211_P2P_SUPPORT
	APCLI_STRUCT *pApCliEntry;
#endif /* RT_CFG80211_P2P_SUPPORT */

	CHAR preIfName[12];
	UINT devNameLen = strlen(pDevName);
	UINT preIfIndex = pDevName[devNameLen-1] - 48;
	CFG80211_CB *p80211CB = pAd->pCfg80211_CB;
	struct wireless_dev *pWdev;
	UINT32 MC_RowID = 0, IoctlIF = 0, Inf = INT_P2P;

	memset(preIfName, 0, sizeof(preIfName));
	NdisCopyMemory(preIfName, pDevName, devNameLen-1);
	pNetDevOps = &netDevHook;

	DBGPRINT(RT_DEBUG_TRACE, ("%s ---> (%s, %s, %d)\n",
			__FUNCTION__, pDevName, preIfName, preIfIndex));

	/* init operation functions and flags */
	NdisZeroMemory(&netDevHook, sizeof(netDevHook));
	netDevHook.open = CFG80211_VirtualIF_Open;	/* device opem hook point */
	netDevHook.stop = CFG80211_VirtualIF_Close;	/* device close hook point */
	netDevHook.xmit = CFG80211_VirtualIF_PacketSend;/* hard transmit hook point */
	netDevHook.ioctl = CFG80211_VirtualIF_Ioctl;	/* ioctl hook point */

#if WIRELESS_EXT >= 12
	//netDevHook.iw_handler = (void *)&rt28xx_ap_iw_handler_def;
#endif /* WIRELESS_EXT >= 12 */

	new_dev_p = RtmpOSNetDevCreate(MC_RowID, &IoctlIF, Inf, preIfIndex, sizeof(PRTMP_ADAPTER), preIfName);
	if (new_dev_p == NULL) {
		/* allocation fail, exit */
		DBGPRINT(RT_DEBUG_ERROR, ("Allocate network device fail (CFG80211)...\n"));
		return;
	} else {
		DBGPRINT(RT_DEBUG_TRACE, ("Register CFG80211 I/F (%s)\n",
				RTMP_OS_NETDEV_GET_DEVNAME(new_dev_p)));
	}

	new_dev_p->destructor = free_netdev;
	RtmpOsSetNetDevPriv(new_dev_p, pAd);
	NdisMoveMemory(&pNetDevOps->devAddr[0], &pAd->CurrentAddress[0], MAC_ADDR_LEN);

	//CFG_TODO
	/*
		Bit1 of MAC address Byte0 is local administration bit
		and should be set to 1 in extended multiple BSSIDs'
		Bit3~ of MAC address Byte0 is extended multiple BSSID index.
	*/
	if (pAd->chipCap.MBSSIDMode == MBSSID_MODE1) {
		pNetDevOps->devAddr[0] += 2; /* NEW BSSID */
	} else {
#ifdef P2P_ODD_MAC_ADJUST
		if (pNetDevOps->devAddr[5] & 0x01 == 0x01)
			pNetDevOps->devAddr[5] -= 1;
		else
#endif /* P2P_ODD_MAC_ADJUST */
		pNetDevOps->devAddr[5] += FIRST_MBSSID;
	}

	switch (DevType) {
	case RT_CMD_80211_IFTYPE_MONITOR:
		DBGPRINT(RT_DEBUG_ERROR, ("CFG80211 I/F Monitor Type\n"));

		//RTMP_OS_NETDEV_SET_TYPE_MONITOR(new_dev_p);
		break;
#ifdef RT_CFG80211_P2P_SUPPORT
	case RT_CMD_80211_IFTYPE_P2P_CLIENT:
		pApCliEntry = &pAd->ApCfg.ApCliTab[MAIN_MBSSID];
		wdev = &pApCliEntry->wdev;
		wdev->wdev_type = WDEV_TYPE_STA;
		wdev->func_dev = pApCliEntry;
		wdev->sys_handle = (void *)pAd;
		wdev->if_dev = new_dev_p;
		wdev->tx_pkt_allowed = ApCliAllowToSendPacket;
		RtmpOsSetNetDevPriv(new_dev_p, pAd);
		RTMP_OS_NETDEV_SET_WDEV(new_dev_p, wdev);
		if (rtmp_wdev_idx_reg(pAd, wdev) < 0) {
			DBGPRINT(RT_DEBUG_ERROR,
					("%s: Assign wdev idx for %s failed, free net device!\n",
					__FUNCTION__,RTMP_OS_NETDEV_GET_DEVNAME(new_dev_p)));
			RtmpOSNetDevFree(new_dev_p);
			break;
		}

		/* init MAC address of virtual network interface */
		COPY_MAC_ADDR(wdev->if_addr, pNetDevOps->devAddr);
		break;
	case RT_CMD_80211_IFTYPE_P2P_GO:
		pNetDevOps->priv_flags = INT_P2P;
		pAd->ApCfg.MBSSID[MAIN_MBSSID].MSSIDDev = NULL;
		/* The Behivaor in SetBeacon Ops	*/
		//pAd->ApCfg.MBSSID[MAIN_MBSSID].MSSIDDev = new_dev_p;
		pAd->cfg80211_ctrl.isCfgInApMode = RT_CMD_80211_IFTYPE_AP;
		COPY_MAC_ADDR(pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev.if_addr, pNetDevOps->devAddr);
		COPY_MAC_ADDR(pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev.bssid, pNetDevOps->devAddr);
		break;
#endif /* RT_CFG80211_P2P_SUPPORT */
	default:
		DBGPRINT(RT_DEBUG_ERROR, ("Unknown CFG80211 I/F Type (%d)\n", DevType));
	}

	//CFG_TODO : should be move to VIF_CHG
	if ((DevType == RT_CMD_80211_IFTYPE_P2P_CLIENT) ||
		(DevType == RT_CMD_80211_IFTYPE_P2P_GO)) {
		COPY_MAC_ADDR(pAd->cfg80211_ctrl.P2PCurrentAddress, pNetDevOps->devAddr);
	}

	pWdev = kzalloc(sizeof(*pWdev), GFP_KERNEL);
	new_dev_p->ieee80211_ptr = pWdev;
	pWdev->wiphy = p80211CB->pCfg80211_Wdev->wiphy;
	SET_NETDEV_DEV(new_dev_p, wiphy_dev(pWdev->wiphy));
	pWdev->netdev = new_dev_p;
	pWdev->iftype = DevType;
	RtmpOSNetDevAttach(pAd->OpMode, new_dev_p, pNetDevOps);
	AsicSetBssid(pAd, pAd->CurrentAddress);

	/* Record the pNetDevice to Cfg80211VifDevList */
	RTMP_CFG80211_AddVifEntry(pAd, new_dev_p, DevType);

	DBGPRINT(RT_DEBUG_TRACE, ("%s <---\n", __FUNCTION__));
}

void RTMP_CFG80211_VirtualIF_Remove(void *pAdSrc, PNET_DEV dev_p, UINT32 DevType)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
#ifdef RT_CFG80211_P2P_SUPPORT
	BOOLEAN isGoOn = FALSE;
	struct wifi_dev *wdev;
#endif

	if (dev_p) {
		RTMP_CFG80211_RemoveVifEntry(pAd, dev_p);
				RTMP_OS_NETDEV_STOP_QUEUE(dev_p);
#ifdef RT_CFG80211_P2P_SUPPORT
		pAd->cfg80211_ctrl.Cfg80211VifDevSet.isGoingOn = FALSE;
		isGoOn = RTMP_CFG80211_VIF_P2P_GO_ON(pAd);

		if (isGoOn) {
			RtmpOSNetDevDetach(dev_p);
			pAd->ApCfg.MBSSID[MAIN_MBSSID].MSSIDDev = NULL;
		} else if (pAd->flg_apcli_init) {
			wdev = &pAd->ApCfg.ApCliTab[MAIN_MBSSID].wdev;

			OPSTATUS_CLEAR_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);
			cfg80211_disconnected(dev_p, 0, NULL, 0, GFP_KERNEL);

			NdisZeroMemory(pAd->ApCfg.ApCliTab[MAIN_MBSSID].CfgApCliBssid, MAC_ADDR_LEN);
			RtmpOSNetDevDetach(dev_p);
			rtmp_wdev_idx_unreg(pAd, wdev);
			pAd->flg_apcli_init = FALSE;
			wdev->if_dev = NULL;
		} else
#endif /* RT_CFG80211_P2P_SUPPORT */
		{
			/* Never Opened When New Netdevice on */
			RtmpOSNetDevDetach(dev_p);
		}

		if (dev_p->ieee80211_ptr) {
			kfree(dev_p->ieee80211_ptr);
			dev_p->ieee80211_ptr = NULL;
		}
	}
}

void RTMP_CFG80211_AllVirtualIF_Remove(void *pAdSrc)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	PLIST_HEADER  pCacheList = &pAd->cfg80211_ctrl.Cfg80211VifDevSet.vifDevList;
	PCFG80211_VIF_DEV pDevEntry = NULL;
	PLIST_ENTRY pListEntry = NULL;
	pListEntry = pCacheList->pHead;
	pDevEntry = (PCFG80211_VIF_DEV)pListEntry;

	while ((pDevEntry != NULL) && (pCacheList->size != 0)) {
		rtnl_lock();
		RTMP_CFG80211_VirtualIF_Remove(pAd, pDevEntry->net_dev, pDevEntry->net_dev->ieee80211_ptr->iftype);
		rtnl_unlock();
		pListEntry = pListEntry->pNext;
		pDevEntry = (PCFG80211_VIF_DEV)pListEntry;
	}
}

#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
static int CFG80211_DummyP2pIf_Open(PNET_DEV dev_p)
{
	struct wireless_dev *wdev = dev_p->ieee80211_ptr;
	if (!wdev)
		return -EINVAL;

	wdev->wiphy->interface_modes |= (BIT(NL80211_IFTYPE_P2P_CLIENT)
					 | BIT(NL80211_IFTYPE_P2P_GO));
	return 0;
}

static int CFG80211_DummyP2pIf_Close(PNET_DEV dev_p)
{
	struct wireless_dev *wdev = dev_p->ieee80211_ptr;
	if (!wdev)
		return -EINVAL;

	wdev->wiphy->interface_modes = (wdev->wiphy->interface_modes)
			& (~(BIT(NL80211_IFTYPE_P2P_CLIENT)| BIT(NL80211_IFTYPE_P2P_GO)));
	return 0;
}

static int CFG80211_DummyP2pIf_Ioctl(PNET_DEV dev_p, void *rq_p, int cmd)
{
	RTMP_ADAPTER *pAd = RtmpOsGetNetDevPriv(dev_p);
	ASSERT(pAd);

	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
		return -ENETDOWN;

	DBGPRINT(RT_DEBUG_TRACE, ("%s --->\n", __FUNCTION__));

	return rt28xx_ioctl(dev_p, rq_p, cmd);
}

static int CFG80211_DummyP2pIf_PacketSend(PNDIS_PACKET skb_p, PNET_DEV dev_p)
{
	return 0;
}

void RTMP_CFG80211_DummyP2pIf_Remove(void *pAdSrc)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	PCFG80211_CTRL cfg80211_ctrl = &pAd->cfg80211_ctrl;
	PNET_DEV dummy_p2p_net_dev = (PNET_DEV)cfg80211_ctrl->dummy_p2p_net_dev;

	DBGPRINT(RT_DEBUG_TRACE, (" %s =====> \n", __FUNCTION__));
	rtnl_lock();

	if (dummy_p2p_net_dev) {
		RTMP_OS_NETDEV_STOP_QUEUE(dummy_p2p_net_dev);
		RtmpOSNetDevDetach(dummy_p2p_net_dev);
		if (dummy_p2p_net_dev->ieee80211_ptr) {
			kfree(dummy_p2p_net_dev->ieee80211_ptr);
			dummy_p2p_net_dev->ieee80211_ptr = NULL;
		}

		rtnl_unlock();
		RtmpOSNetDevFree(dummy_p2p_net_dev);
		rtnl_lock();
		cfg80211_ctrl->flg_cfg_dummy_p2p_init = FALSE;
	}
	rtnl_unlock();
	DBGPRINT(RT_DEBUG_TRACE, (" %s <=====\n", __FUNCTION__));
}

void RTMP_CFG80211_DummyP2pIf_Init(void *pAdSrc)
{
#define INF_CFG80211_DUMMY_P2P_NAME "p2p"
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	CFG80211_CB *p80211CB = pAd->pCfg80211_CB;
	PCFG80211_CTRL cfg80211_ctrl = &pAd->cfg80211_ctrl;
	RTMP_OS_NETDEV_OP_HOOK	netDevHook, *pNetDevOps;
	PNET_DEV new_dev_p;
	UINT32 MC_RowID = 0, IoctlIF = 0, Inf = INT_P2P;
	UINT preIfIndex = 0;
	struct wireless_dev *pWdev;

	DBGPRINT(RT_DEBUG_TRACE, (" %s =====> \n", __FUNCTION__));
	if (cfg80211_ctrl->flg_cfg_dummy_p2p_init != FALSE)
		return;

#if RT_CFG80211_P2P_SUPPORT
	cfg80211_ctrl->bP2pCliPmEnable = FALSE;
	cfg80211_ctrl->bPreKeepSlient = FALSE;
	cfg80211_ctrl->bKeepSlient = FALSE;
	cfg80211_ctrl->NoAIndex = MAX_LEN_OF_MAC_TABLE;
	cfg80211_ctrl->MyGOwcid = MAX_LEN_OF_MAC_TABLE;
	cfg80211_ctrl->CTWindows=0;	/* CTWindows and OppPS parameter field */
#endif /* RT_CFG80211_P2P_SUPPORT */

	pNetDevOps=&netDevHook;

	/* init operation functions and flags */
	NdisZeroMemory(&netDevHook, sizeof(netDevHook));
	netDevHook.open = CFG80211_DummyP2pIf_Open;		/* device opem hook point */
	netDevHook.stop = CFG80211_DummyP2pIf_Close;		/* device close hook point */
	netDevHook.xmit = CFG80211_DummyP2pIf_PacketSend;	/* hard transmit hook point */
	netDevHook.ioctl = CFG80211_DummyP2pIf_Ioctl;		/* ioctl hook point */

	new_dev_p = RtmpOSNetDevCreate(MC_RowID, &IoctlIF, Inf, preIfIndex, sizeof(PRTMP_ADAPTER), INF_CFG80211_DUMMY_P2P_NAME);
	if (new_dev_p == NULL) {
		/* allocation fail, exit */
		DBGPRINT(RT_DEBUG_ERROR, ("Allocate network device fail (CFG80211: Dummy P2P IF)...\n"));
		return;
	} else {
		DBGPRINT(RT_DEBUG_TRACE, ("Register CFG80211 I/F (%s)\n", RTMP_OS_NETDEV_GET_DEVNAME(new_dev_p)));
	}

	RtmpOsSetNetDevPriv(new_dev_p, pAd);
	NdisMoveMemory(&pNetDevOps->devAddr[0], &pAd->CurrentAddress[0], MAC_ADDR_LEN);
	pNetDevOps->needProtcted = TRUE;

	pWdev = kzalloc(sizeof(*pWdev), GFP_KERNEL);
	if (unlikely(!pWdev)) {
		DBGPRINT(RT_DEBUG_ERROR, ("Could not allocate wireless device\n"));
		return;
	}

	new_dev_p->ieee80211_ptr = pWdev;
	pWdev->wiphy = p80211CB->pCfg80211_Wdev->wiphy;
	SET_NETDEV_DEV(new_dev_p, wiphy_dev(pWdev->wiphy));
	pWdev->netdev = new_dev_p;
	pWdev->iftype = RT_CMD_80211_IFTYPE_STATION;

	RtmpOSNetDevAttach(pAd->OpMode, new_dev_p, pNetDevOps);
	cfg80211_ctrl->dummy_p2p_net_dev = new_dev_p;
	cfg80211_ctrl->flg_cfg_dummy_p2p_init = TRUE;

	DBGPRINT(RT_DEBUG_TRACE, (" %s <=====\n", __FUNCTION__));
}
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */
#endif /* RT_CFG80211_SUPPORT */
#endif /* LINUX_VERSION_CODE */

