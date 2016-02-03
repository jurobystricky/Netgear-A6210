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

*/


#include "rt_config.h"

int rtmp_wdev_idx_unreg(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	int idx;
	ULONG flags;

	if (!wdev)
		return -1;

	RTMP_INT_LOCK(&pAd->irq_lock, flags);
	for (idx = 0; idx < WDEV_NUM_MAX; idx++) {
		if (pAd->wdev_list[idx] == wdev) {
			DBGPRINT(RT_DEBUG_WARN,
					("unregister wdev(type:%d, idx:%d) from wdev_list\n",
					wdev->wdev_type, wdev->wdev_idx));
			pAd->wdev_list[idx] = NULL;
			wdev->wdev_idx = WDEV_NUM_MAX;
			break;
		}
	}

	if (idx == WDEV_NUM_MAX) {
		DBGPRINT(RT_DEBUG_ERROR,
				("Cannot found wdev(%p, type:%d, idx:%d) in wdev_list\n",
				wdev, wdev->wdev_type, wdev->wdev_idx));
		DBGPRINT(RT_DEBUG_OFF, ("Dump wdev_list:\n"));
		for (idx = 0; idx < WDEV_NUM_MAX; idx++) {
			DBGPRINT(RT_DEBUG_OFF, ("Idx %d: 0x%p\n", idx, pAd->wdev_list[idx]));
		}
	}
	RTMP_INT_UNLOCK(&pAd->irq_lock, flags);

	return ((idx < WDEV_NUM_MAX) ? 0 : -1);
}


int rtmp_wdev_idx_reg(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	int idx;
	ULONG flags;

	if (!wdev)
		return -1;

	RTMP_INT_LOCK(&pAd->irq_lock, flags);
	for (idx = 0; idx < WDEV_NUM_MAX; idx++) {
		if (pAd->wdev_list[idx] == wdev) {
			DBGPRINT(RT_DEBUG_WARN,
					("wdev(type:%d) already registered and idx(%d) %smatch\n",
					wdev->wdev_type, wdev->wdev_idx,
					((idx != wdev->wdev_idx) ? "mis" : "")));
			break;
		}
		if (pAd->wdev_list[idx] == NULL) {
			pAd->wdev_list[idx] = wdev;
			break;
		}
	}

	wdev->wdev_idx = idx;
	if (idx < WDEV_NUM_MAX) {
		DBGPRINT(RT_DEBUG_TRACE, ("Assign wdev_idx=%d\n", idx));
	}
	RTMP_INT_UNLOCK(&pAd->irq_lock, flags);

	return ((idx < WDEV_NUM_MAX) ? idx : -1);
}


/*
========================================================================
Routine Description:
    Early checking and OS-depened parsing for Tx packet to AP device.

Arguments:
    NDIS_HANDLE MiniportAdapterContext	Pointer refer to the device handle, i.e., the pAd.
	PPNDIS_PACKET ppPacketArray	The packet array need to do transmission.
	UINT NumberOfPackets		Number of packet in packet array.

Return Value:
	NONE

Note:
	This function do early checking and classification for send-out packet.
	You only can put OS-depened & AP related code in here.
========================================================================
*/
void wdev_tx_pkts(NDIS_HANDLE dev_hnd, PPNDIS_PACKET pkt_list, UINT pkt_cnt,
	struct wifi_dev *wdev)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)dev_hnd;
	PNDIS_PACKET pPacket;
	BOOLEAN allowToSend;
	UCHAR wcid = MCAST_WCID;
	UINT Index;

	for (Index = 0; Index < pkt_cnt; Index++) {
		pPacket = pkt_list[Index];

		if (RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_RESET_IN_PROGRESS |
			fRTMP_ADAPTER_HALT_IN_PROGRESS |
			fRTMP_ADAPTER_RADIO_OFF |
			fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))) {
			/* Drop send request since hardware is in reset state */
			dev_kfree_skb_any(pPacket);
			continue;
		}

#ifdef RT_CFG80211_SUPPORT
	if (pAd->cfg80211_ctrl.isCfgInApMode == RT_CMD_80211_IFTYPE_AP && 
		(wdev->Hostapd != Hostapd_CFG)
#ifdef RT_CFG80211_P2P_SUPPORT
	&& (!RTMP_CFG80211_VIF_P2P_GO_ON(pAd))
#endif 	/*RT_CFG80211_P2P_SUPPORT*/
		) {
			/* Drop send request since hardware is in reset state */
			dev_kfree_skb_any(pPacket);
			continue;
		}
#endif /*RT_CFG80211_SUPPORT*/

		if ((wdev->allow_data_tx == TRUE) && (wdev->tx_pkt_allowed))
			allowToSend = wdev->tx_pkt_allowed(pAd, wdev, pPacket, &wcid);
		else
			allowToSend = FALSE;

		if (allowToSend == TRUE) {
			RTMP_SET_PACKET_WCID(pPacket, wcid);
			RTMP_SET_PACKET_WDEV(pPacket, wdev->wdev_idx);
			NDIS_SET_PACKET_STATUS(pPacket, NDIS_STATUS_PENDING);
			pAd->RalinkCounters.PendingNdisPacketCount++;

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			APSendPacket(pAd, pPacket);
		}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
#ifdef RT_CFG80211_P2P_SUPPORT
			if (RTMP_GET_PACKET_OPMODE(pPacket))
				APSendPacket(pAd, pPacket);
			else
#endif /* RT_CFG80211_P2P_SUPPORT */
			STASendPacket(pAd, pPacket);
		}
#endif /* CONFIG_STA_SUPPORT */
		} else {
			dev_kfree_skb_any(pPacket);
		}
	}

	/* Dequeue outgoing frames from TxSwQueue[] and process it */
	RTMPDeQueuePacket(pAd, FALSE, NUM_OF_TX_RING, MAX_TX_PROCESS);
}

