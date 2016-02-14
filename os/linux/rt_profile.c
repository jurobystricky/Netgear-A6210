/****************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ****************************************************************************

	Module Name:
	rt_profile.c

 */

#include "rt_config.h"

#if defined (CONFIG_RA_HW_NAT)  || defined (CONFIG_RA_HW_NAT_MODULE)
#include "../../../../../../net/nat/hw_nat/ra_nat.h"
#include "../../../../../../net/nat/hw_nat/frame_engine.h"
#endif

struct dev_type_name_map{
	int type;
	PSTRING prefix[2];
};


#define SECOND_INF_MAIN_DEV_NAME		"rai"

#if 0
#define SECOND_INF_MBSSID_DEV_NAME		"rai"
#define SECOND_INF_WDS_DEV_NAME			"wdsi"
#define SECOND_INF_APCLI_DEV_NAME		"apclii"
#define SECOND_INF_MESH_DEV_NAME		"meshi"
#define SECOND_INF_P2P_DEV_NAME			"p2pi"
#define SECOND_INF_MONITOR_DEV_NAME		"moni"
#endif

#define xdef_to_str(s)   def_to_str(s)
#define def_to_str(s)    #s

#if 0
#define FIRST_EEPROM_FILE_PATH	"/etc_ro/Wireless/RT2860/"
#define FIRST_AP_PROFILE_PATH	"/etc/Wireless/RT2860/RT2860.dat"
#define FIRST_CHIP_ID	xdef_to_str(CONFIG_RT_FIRST_CARD)

#define SECOND_EEPROM_FILE_PATH	"/etc_ro/Wireless/iNIC/"
#define SECOND_AP_PROFILE_PATH	"/etc/Wireless/iNIC/iNIC_ap.dat"
#define SECOND_CHIP_ID	xdef_to_str(CONFIG_RT_SECOND_CARD)
#endif

static struct dev_type_name_map prefix_map[] =
{
	{INT_MAIN, 		{INF_MAIN_DEV_NAME, SECOND_INF_MAIN_DEV_NAME}},
#ifdef CONFIG_AP_SUPPORT
#ifdef MBSS_SUPPORT
	{INT_MBSSID, 		{INF_MBSSID_DEV_NAME, SECOND_INF_MBSSID_DEV_NAME}},
#endif /* MBSS_SUPPORT */
#ifdef APCLI_SUPPORT
	{INT_APCLI, 		{INF_APCLI_DEV_NAME, SECOND_INF_APCLI_DEV_NAME}},
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
	{0},
};

struct dev_id_name_map{
	int chip_id;
	PSTRING chip_name;
};

static const struct dev_id_name_map id_name_list[]=
{
	{7610, "7610, 7610e 7610u"},

};

void get_dev_config_idx(RTMP_ADAPTER *pAd)
{
	int idx = 0;
#if defined(CONFIG_RT_FIRST_CARD) && defined(CONFIG_RT_SECOND_CARD)
	static int probe_cnt = 1;
	int first_card = 0, second_card = 0;

	A2Hex(first_card, FIRST_CHIP_ID);
	A2Hex(second_card, SECOND_CHIP_ID);
	DBGPRINT(RT_DEBUG_TRACE, ("chip_id1=0x%x, chip_id2=0x%x, pAd->MACVersion=0x%x\n",
			first_card, second_card, pAd->MACVersion));

//	if ((pAd->MACVersion & chip_id) == CONFIG_RT_SECOND_CARD)
//		pAd->flash_offset = SECOND_RF_OFFSET;

	if ((first_card == second_card) || IS_MT76x2(pAd)) {
		idx = probe_cnt;
		probe_cnt--;
	} else {
		if (IS_RT8592(pAd))
			idx = 0;
		else if (IS_RT5392(pAd) || IS_MT76x0(pAd))
			idx = 1;
	}
#endif /* defined(CONFIG_RT_FIRST_CARD) && defined(CONFIG_RT_SECOND_CARD) */

	pAd->dev_idx = idx;
}


UCHAR *get_dev_name_prefix(RTMP_ADAPTER *pAd, int dev_type)
{
	struct dev_type_name_map *map;
	int type_idx = 0, dev_idx = pAd->dev_idx;

	do {
		map = &prefix_map[type_idx];
		if (map->type == dev_type) {
			DBGPRINT(RT_DEBUG_TRACE, 
					("%s(): dev_idx = %d, dev_name_prefix=%s\n",
					__FUNCTION__, dev_idx, map->prefix[dev_idx]));
			return map->prefix[dev_idx];
		}
		type_idx++;
	} while (prefix_map[type_idx].type != 0);

	return NULL;
}


static UCHAR *get_dev_profile(RTMP_ADAPTER *pAd)
{
	UCHAR *src = NULL;

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#if defined(CONFIG_RT_FIRST_CARD) && defined(CONFIG_RT_SECOND_CARD)
			int card_idx = pAd->dev_idx;

			if (card_idx == 0)
				src = FIRST_AP_PROFILE_PATH;
			else if (card_idx == 1)
				src = SECOND_AP_PROFILE_PATH;
			else
#endif /* defined(CONFIG_RT_FIRST_CARD) || defined(CONFIG_RT_SECOND_CARD) */
				src = AP_PROFILE_PATH;
		}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			src = STA_PROFILE_PATH;
		}
#endif /* CONFIG_STA_SUPPORT */

#ifdef MULTIPLE_CARD_SUPPORT
	src = (PSTRING)pAd->MC_FileName;
#endif /* MULTIPLE_CARD_SUPPORT */

	return src;
}


NDIS_STATUS RTMPReadParametersHook(RTMP_ADAPTER *pAd)
{
	PSTRING src = NULL;
	RTMP_OS_FD srcf;
	RTMP_OS_FS_INFO osFSInfo;
	NDIS_STATUS retval = NDIS_STATUS_FAILURE;
	ULONG buf_size = MAX_INI_BUFFER_SIZE, fsize;
	PSTRING buffer = NULL;

#ifdef HOSTAPD_SUPPORT
	int i;
#endif
	src = get_dev_profile(pAd);
	if (src && *src) {
		RtmpOSFSInfoChange(&osFSInfo, TRUE);
		srcf = RtmpOSFileOpen(src, O_RDONLY, 0);
		if (IS_FILE_OPEN_ERR(srcf)) {
			DBGPRINT(RT_DEBUG_ERROR, ("Open file \"%s\" failed!\n", src));
		} else {
			// TODO: need to roll back when convert into OSABL code
			fsize = (ULONG)file_inode(srcf)->i_size;
			DBGPRINT(RT_DEBUG_INFO, ("buffer size %lu file size %lu\n",
				buf_size, fsize));
				if (buf_size < (fsize + 1))
					buf_size = fsize + 1;

			buffer = os_alloc_mem(buf_size);
			if (buffer) {
				memset(buffer, 0x00, buf_size);
				retval = RtmpOSFileRead(srcf, buffer, buf_size - 1);
				if (retval > 0) {
					RTMPSetProfileParameters(pAd, buffer);
					retval = NDIS_STATUS_SUCCESS;
				} else {
					DBGPRINT(RT_DEBUG_ERROR,
							("Read file \"%s\" failed(errCode=%d)!\n",
							src, retval));
				}
				os_free_mem(buffer);
			} else {
				retval = NDIS_STATUS_FAILURE;
			}

			if (RtmpOSFileClose(srcf) != 0) {
				retval = NDIS_STATUS_FAILURE;
				DBGPRINT(RT_DEBUG_ERROR,
						("Close file \"%s\" failed(errCode=%d)!\n",
						src, retval));
			}
		}

		RtmpOSFSInfoChange(&osFSInfo, FALSE);
	}

#ifdef HOSTAPD_SUPPORT
		for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
			pAd->ApCfg.MBSSID[i].Hostapd=Hostapd_Diable;
			DBGPRINT(RT_DEBUG_TRACE, 
					("Reset ra%d hostapd support=FLASE",
					i));
		}
#endif /*HOSTAPD_SUPPORT */

#ifdef SINGLE_SKU_V2
	RTMPSetSingleSKUParameters(pAd);
#endif
	return retval;
}


void RTMP_IndicateMediaState(PRTMP_ADAPTER pAd, NDIS_MEDIA_STATE media_state)
{
	pAd->IndicateMediaState = media_state;

#ifdef SYSTEM_LOG_SUPPORT
	if (pAd->IndicateMediaState == NdisMediaStateConnected) {
		RTMPSendWirelessEvent(pAd, IW_STA_LINKUP_EVENT_FLAG, 
				pAd->MacTab.Content[BSSID_WCID].Addr, BSS0, 0);
	} else {
		RTMPSendWirelessEvent(pAd, IW_STA_LINKDOWN_EVENT_FLAG, 
				pAd->MacTab.Content[BSSID_WCID].Addr, BSS0, 0);
	}
#endif /* SYSTEM_LOG_SUPPORT */
}


void tbtt_tasklet(unsigned long data)
{
#ifdef CONFIG_AP_SUPPORT
#ifdef WORKQUEUE_BH
	struct work_struct *work = (struct work_struct *)data;
	POS_COOKIE pObj = container_of(work, struct os_cookie, tbtt_task);
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pObj->pAd_va;
#else
	PRTMP_ADAPTER pAd = (RTMP_ADAPTER *)data;
#endif /* WORKQUEUE_BH */

#ifdef RT_CFG80211_P2P_SUPPORT
		if (pAd->cfg80211_ctrl.isCfgInApMode == RT_CMD_80211_IFTYPE_AP)
#else
	if (pAd->OpMode == OPMODE_AP)
#endif /* RT_CFG80211_P2P_SUPPORT */
	{
		/* step 7 - if DTIM, then move backlogged bcast/mcast frames 
		 * from PSQ to TXQ whenever DtimCount==0 */
#ifdef RTMP_MAC_USB
		if ((pAd->ApCfg.DtimCount + 1) == pAd->ApCfg.DtimPeriod)
#endif /* RTMP_MAC_USB */
		{
			QUEUE_ENTRY *pEntry;
			BOOLEAN bPS = FALSE;
			UINT count = 0;
			unsigned long IrqFlags;

			RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);
			while (pAd->MacTab.McastPsQueue.Head) {
				bPS = TRUE;
				if (pAd->TxSwQueue[QID_AC_BE].Number <= (pAd->TxSwQMaxLen + MAX_PACKETS_IN_MCAST_PS_QUEUE)) {
					pEntry = RemoveHeadQueue(&pAd->MacTab.McastPsQueue);
					/*if (pAd->MacTab.McastPsQueue.Number) */
					if (count) {
						RTMP_SET_PACKET_MOREDATA(pEntry, TRUE);
					}
					InsertHeadQueue(&pAd->TxSwQueue[QID_AC_BE], pEntry);
					count++;
				} else {
					break;
				}
			}
			RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);

			if (pAd->MacTab.McastPsQueue.Number == 0) {
				UINT bss_index;

				/* clear MCAST/BCAST backlog bit for all BSS */
				for (bss_index = BSS0; bss_index < pAd->ApCfg.BssidNum; bss_index++)
					WLAN_MR_TIM_BCMC_CLEAR(bss_index);
			}
			pAd->MacTab.PsQIdleCount = 0;

			if (bPS == TRUE) {
				RTMPDeQueuePacket(pAd, FALSE, NUM_OF_TX_RING, 
						/*MAX_TX_IN_TBTT*/MAX_PACKETS_IN_MCAST_PS_QUEUE);
			}
		}
	}
#endif /* CONFIG_AP_SUPPORT */
}

/*
========================================================================
Routine Description:
	Assign protocol to the packet.

Arguments:
	pPkt		- the packet

Return Value:
	None

Note:
========================================================================
*/
static void RtmpOsPktProtocolAssign(PNDIS_PACKET pNetPkt)
{
	struct sk_buff *pRxPkt = RTPKT_TO_OSPKT(pNetPkt);
	pRxPkt->protocol = eth_type_trans(pRxPkt, pRxPkt->dev);
}

#ifdef INF_PPA_SUPPORT
static int process_nbns_packet(PRTMP_ADAPTER pAd, struct sk_buff *skb)
{
	UCHAR *data;
	USHORT *eth_type;

	data = (UCHAR *)eth_hdr(skb);
	if (data == 0) {
		data = (UCHAR *)skb->data;
		if (data == 0) {
			DBGPRINT(RT_DEBUG_ERROR, ("%s::Error\n", __FUNCTION__));
			return 1;
		}
	}

	eth_type = (USHORT *)&data[12];
	if (*eth_type == cpu_to_be16(ETH_P_IP)) {
		int ip_h_len;
		UCHAR *ip_h;
		UCHAR *udp_h;
		USHORT dport, host_dport;

		ip_h = data + 14;
		ip_h_len = (ip_h[0] & 0x0f)*4;

		if (ip_h[9] == 0x11) {
			/* UDP */
			udp_h = ip_h + ip_h_len;
			memcpy(&dport, udp_h + 2, 2);
			host_dport = ntohs(dport);
			if ((host_dport == 67) || (host_dport == 68)) {
				/* DHCP */
				return 0;
			}
		}
	} else if ((data[12] == 0x88) && (data[13] == 0x8e)) {
		/* EAPOL */
		return 0;
		}
	return 1;
}
#endif /* INF_PPA_SUPPORT */

void announce_802_3_packet(void *pAdSrc, PNDIS_PACKET pPacket, UCHAR OpMode)
{
//	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pAdSrc;
	PNDIS_PACKET pRxPkt = pPacket;

	ASSERT(pPacket);

#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
	}
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

	/* Push up the protocol stack */
#ifdef CONFIG_AP_SUPPORT
#if defined(PLATFORM_BL2348) || defined(PLATFORM_BL23570)
	{
		extern int (*pToUpperLayerPktSent)(PNDIS_PACKET *pSkb);
		RtmpOsPktProtocolAssign(pRxPkt);
		pToUpperLayerPktSent(pRxPkt);
		return;
	}
#endif /* defined(PLATFORM_BL2348) || defined(PLATFORM_BL23570) */
#endif /* CONFIG_AP_SUPPORT */

#ifdef INF_PPA_SUPPORT
	{
		if (ppa_hook_directpath_send_fn && (pAd->PPAEnable == TRUE)) {
			int retVal, ret;
			UINT ppa_flags = 0;

			retVal = process_nbns_packet(pAd, pRxPkt);

			if (retVal > 0) {
				ret = ppa_hook_directpath_send_fn(pAd->g_if_id, pRxPkt, pRxPkt->len, ppa_flags);
				if (ret == 0) {
					pRxPkt = NULL;
					return;
				}
				RtmpOsPktRcvHandle(pRxPkt);
			} else if (retVal == 0) {
				RtmpOsPktProtocolAssign(pRxPkt);
				RtmpOsPktRcvHandle(pRxPkt);
			} else {
				dev_kfree_skb_any(pRxPkt);
			}
		} else {
			RtmpOsPktProtocolAssign(pRxPkt);
			RtmpOsPktRcvHandle(pRxPkt);
		}

		return;
	}
#endif /* INF_PPA_SUPPORT */

	{
#ifdef CONFIG_RT2880_BRIDGING_ONLY
		PACKET_CB_ASSIGN(pRxPkt, 22) = 0xa8;
#endif

#if defined(CONFIG_RA_CLASSIFIER) || defined(CONFIG_RA_CLASSIFIER_MODULE)
		if (ra_classifier_hook_rx!= NULL) {
			unsigned int flags;

			RTMP_IRQ_LOCK(&pAd->page_lock, flags);
			ra_classifier_hook_rx(pRxPkt, classifier_cur_cycle);
			RTMP_IRQ_UNLOCK(&pAd->page_lock, flags);
		}
#endif /* CONFIG_RA_CLASSIFIER */

#if !defined(CONFIG_RA_NAT_NONE)
#if defined (CONFIG_RA_HW_NAT) || defined (CONFIG_RA_HW_NAT_MODULE)
		RtmpOsPktNatMagicTag(pRxPkt);
#endif
		/* bruce+
			ra_sw_nat_hook_rx return 1 --> continue
			ra_sw_nat_hook_rx return 0 --> FWD & without netif_rx
		*/
		if (ra_sw_nat_hook_rx!= NULL) {
			unsigned int flags;

			RtmpOsPktProtocolAssign(pRxPkt);

			RTMP_IRQ_LOCK(&pAd->page_lock, flags);
			if (ra_sw_nat_hook_rx(pRxPkt)) {
				RtmpOsPktRcvHandle(pRxPkt);
			}
			RTMP_IRQ_UNLOCK(&pAd->page_lock, flags);
			return;
		}
#else
		{
#if defined (CONFIG_RA_HW_NAT)  || defined (CONFIG_RA_HW_NAT_MODULE)
			RtmpOsPktNatNone(pRxPkt);
#endif /* CONFIG_RA_HW_NAT */
		}
#endif /* CONFIG_RA_NAT_NONE */
	}

#ifdef CONFIG_AP_SUPPORT
#ifdef BG_FT_SUPPORT
	if (BG_FTPH_PacketFromApHandle(pRxPkt) == 0)
		return;
#endif /* BG_FT_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

	RtmpOsPktProtocolAssign(pRxPkt);

#if defined (CONFIG_WIFI_PKT_FWD)
	if (wf_fwd_rx_hook!= NULL) {
		unsigned int flags;
		RTMP_IRQ_LOCK(&pAd->page_lock, flags);

		if (wf_fwd_rx_hook(pRxPkt) == 0) {
			RTMP_IRQ_UNLOCK(&pAd->page_lock, flags);
			return;
		}

		RTMP_IRQ_UNLOCK(&pAd->page_lock, flags);
	}
#endif /* CONFIG_WIFI_PKT_FWD */

	RtmpOsPktRcvHandle(pRxPkt);
}

extern spinlock_t TimerSemLock;

void RTMPFreeAdapter(void *pAdSrc)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	POS_COOKIE os_cookie;
	int index;

	os_cookie = (POS_COOKIE)pAd->OS_Cookie;

	if (pAd->BeaconBuf)
		os_free_mem(pAd->BeaconBuf);

#ifdef MULTIPLE_CARD_SUPPORT
#ifdef RTMP_FLASH_SUPPORT
	if (pAd->eebuf && (pAd->eebuf != pAd->chipCap.EEPROM_DEFAULT_BIN)) {
		os_free_mem(pAd->eebuf);
		pAd->eebuf = NULL;
	}
#endif /* RTMP_FLASH_SUPPORT */
#endif /* MULTIPLE_CARD_SUPPORT */

	NdisFreeSpinLock(&pAd->MgmtRingLock);

#if defined(RT3290) || defined(RLT_MAC)
//	NdisFreeSpinLock(&pAd->WlanEnLock);
#endif /* defined(RT3290) || defined(RLT_MAC) */

	for (index = 0 ; index < NUM_OF_TX_RING; index++) {
		NdisFreeSpinLock(&pAd->TxSwQueueLock[index]);
		NdisFreeSpinLock(&pAd->DeQueueLock[index]);
		pAd->DeQueueRunning[index] = FALSE;
	}

	NdisFreeSpinLock(&pAd->irq_lock);

#ifdef UAPSD_SUPPORT
	NdisFreeSpinLock(&pAd->UAPSDEOSPLock); /* OS_ABL_SUPPORT */
#endif /* UAPSD_SUPPORT */

#ifdef DOT11_N_SUPPORT
	NdisFreeSpinLock(&pAd->mpdu_blk_pool.lock);
#endif
	if (pAd->iw_stats) {
		os_free_mem(pAd->iw_stats);
		pAd->iw_stats = NULL;
	}

	if (pAd->stats) {
		os_free_mem(pAd->stats);
		pAd->stats = NULL;
	}

	NdisFreeSpinLock(&TimerSemLock);

#ifdef RALINK_ATE
#ifdef RTMP_MAC_USB
	RTMP_OS_ATMOIC_DESTROY(&pAd->BulkOutRemained);
	RTMP_OS_ATMOIC_DESTROY(&pAd->BulkInRemained);
#endif /* RTMP_MAC_USB */
#endif /* RALINK_ATE */

	RTMP_OS_FREE_TIMER(pAd);
	RTMP_OS_FREE_LOCK(pAd);
	RTMP_OS_FREE_TASKLET(pAd);
	RTMP_OS_FREE_TASK(pAd);
	RTMP_OS_FREE_SEM(pAd);
	RTMP_OS_FREE_ATOMIC(pAd);

	RtmpOsVfree(pAd);
	if (os_cookie)
		os_free_mem(os_cookie);
}


int RTMPSendPackets(NDIS_HANDLE dev_hnd, PPNDIS_PACKET pkt_list, UINT pkt_cnt,
	UINT32 pkt_total_len, RTMP_NET_ETH_CONVERT_DEV_SEARCH Func)
{
	struct wifi_dev *wdev = (struct wifi_dev *)dev_hnd;
	RTMP_ADAPTER *pAd;
	PNDIS_PACKET pPacket = pkt_list[0];

	ASSERT(wdev->sys_handle);
	pAd = (RTMP_ADAPTER *)wdev->sys_handle;
	INC_COUNTER64(pAd->WlanCounters.TransmitCountFrmOs);

	if (!pPacket)
		return 0;

	if (pkt_total_len < 14) {
		hex_dump("bad packet", GET_OS_PKT_DATAPTR(pPacket), pkt_total_len);
		dev_kfree_skb_any(pPacket);
		return 0;
	}

#ifdef RALINK_ATE
	if (ATE_ON(pAd)) {
		dev_kfree_skb_any(pPacket);
		return 0;
	}
#endif /* RALINK_ATE */

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		/* Drop send request since we are in monitor mode */
		if (MONITOR_ON(pAd)) {
			dev_kfree_skb_any(pPacket);
			return 0;
		}
	}
#endif /* CONFIG_STA_SUPPORT */

#if !defined(CONFIG_RA_NAT_NONE)
	if (ra_sw_nat_hook_tx != NULL) {
		unsigned long flags;

		RTMP_INT_LOCK(&pAd->page_lock, flags);
		ra_sw_nat_hook_tx(pPacket);
		RTMP_INT_UNLOCK(&pAd->page_lock, flags);
	}
#endif

#ifdef CONFIG_5VT_ENHANCE
	RTMP_SET_PACKET_5VT(pPacket, 0);
	if (*(int*)(GET_OS_PKT_CB(pPacket)) == BRIDGE_TAG) {
		RTMP_SET_PACKET_5VT(pPacket, 1);
	}
#endif

	wdev_tx_pkts((NDIS_HANDLE)pAd, (PPNDIS_PACKET) &pPacket, 1, wdev);
	return 0;
}


PNET_DEV get_netdev_from_bssid(RTMP_ADAPTER *pAd, UCHAR FromWhichBSSID)
{
	PNET_DEV dev_p = NULL;
#ifdef CONFIG_AP_SUPPORT
	UCHAR infRealIdx;
#endif

	do {
#ifdef CONFIG_STA_SUPPORT
#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
		if (FromWhichBSSID >= MIN_NET_DEVICE_FOR_CFG80211_VIF_P2P_GO) {
			dev_p = pAd->ApCfg.MBSSID[BSS0].wdev.if_dev;
			break;
		} else if (FromWhichBSSID >= MIN_NET_DEVICE_FOR_CFG80211_VIF_P2P_CLI) {
			//CFG_TODO
			break;
		}
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
		infRealIdx = FromWhichBSSID & (NET_DEVICE_REAL_IDX_MASK);
#ifdef APCLI_SUPPORT
		if (FromWhichBSSID >= MIN_NET_DEVICE_FOR_APCLI) {
			dev_p = (infRealIdx >= MAX_APCLI_NUM ? NULL : pAd->ApCfg.ApCliTab[infRealIdx].wdev.if_dev);
			break;
		}
#endif /* APCLI_SUPPORT */

		if ((FromWhichBSSID > 0) &&
			(FromWhichBSSID < pAd->ApCfg.BssidNum) &&
			(FromWhichBSSID < MAX_MBSSID_NUM(pAd)) &&
			(FromWhichBSSID < HW_BEACON_MAX_NUM)) {
			dev_p = pAd->ApCfg.MBSSID[FromWhichBSSID].wdev.if_dev;
		} else
#endif /* CONFIG_AP_SUPPORT */
		{
			dev_p = pAd->net_dev;
		}
	} while (FALSE);

	ASSERT(dev_p);
	return dev_p;
}


#ifdef CONFIG_AP_SUPPORT
/*
========================================================================
Routine Description:
	Driver pre-Ioctl for AP.

Arguments:
	pAdSrc		- WLAN control block pointer
	pCB		- the IOCTL parameters

Return Value:
	0		No error
	Otherwise	-errno

Note:
========================================================================
*/
int RTMP_AP_IoctlPrepare(RTMP_ADAPTER *pAd, void *pCB)
{
	RT_CMD_AP_IOCTL_CONFIG *pConfig = (RT_CMD_AP_IOCTL_CONFIG *)pCB;
	POS_COOKIE pObj;
	USHORT index;
#ifdef CONFIG_APSTA_MIXED_SUPPORT
	int cmd = 0xff;
#endif /* CONFIG_APSTA_MIXED_SUPPORT */

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	if ((pConfig->priv_flags == INT_MAIN) &&
			!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE)) {
		if (pConfig->pCmdData == NULL)
			return 0;

		if (RtPrivIoctlSetVal() == pConfig->CmdId_RTPRIV_IOCTL_SET) {
			if (TRUE
#ifdef CONFIG_APSTA_MIXED_SUPPORT
				&& (strstr(pConfig->pCmdData, "OpMode") == NULL)
#endif /* CONFIG_APSTA_MIXED_SUPPORT */
#ifdef SINGLE_SKU
				&& (strstr(pConfig->pCmdData, "ModuleTxpower") == NULL)
#endif /* SINGLE_SKU */
			) {
				return -ENETDOWN;
			}
		} else {
			return -ENETDOWN;
		}
	}

	/* determine this ioctl command is comming from which interface. */
	if (pConfig->priv_flags == INT_MAIN) {
		pObj->ioctl_if_type = INT_MAIN;
		pObj->ioctl_if = MAIN_MBSSID;
	} else if (pConfig->priv_flags == INT_MBSSID) {
		pObj->ioctl_if_type = INT_MBSSID;
		/* if (!RTMPEqualMemory(net_dev->name, pAd->net_dev->name, 3))  // for multi-physical card, no MBSSID */
		if (strcmp(pConfig->name, RtmpOsGetNetDevName(pAd->net_dev)) != 0) {
			/* sample */
			for (index = 1; index < pAd->ApCfg.BssidNum; index++) {
				if (pAd->ApCfg.MBSSID[index].wdev.if_dev == pConfig->net_dev) {
					pObj->ioctl_if = index;
					break;
				}
			}
			/* Interface not found! */
			if (index == pAd->ApCfg.BssidNum)
				return -ENETDOWN;
		} else {
			   /* ioctl command from I/F(ra0) */
			pObj->ioctl_if = MAIN_MBSSID;
		}
		MBSS_MR_APIDX_SANITY_CHECK(pAd, pObj->ioctl_if);
	}
#ifdef APCLI_SUPPORT
	else if (pConfig->priv_flags == INT_APCLI) {
		pObj->ioctl_if_type = INT_APCLI;
		for (index = 0; index < MAX_APCLI_NUM; index++) {
			if (pAd->ApCfg.ApCliTab[index].wdev.if_dev == pConfig->net_dev) {
				pObj->ioctl_if = index;
				break;
			}

			if (index == MAX_APCLI_NUM) {
				DBGPRINT(RT_DEBUG_ERROR, 
						("%s(): can not find Apcli I/F\n", 
						__FUNCTION__));
				return -ENETDOWN;
			}
		}
		APCLI_MR_APIDX_SANITY_CHECK(pObj->ioctl_if);
	}
#endif /* APCLI_SUPPORT */
	else {
	/* DBGPRINT(RT_DEBUG_WARN, ("IOCTL is not supported in WDS interface\n")); */
		return -EOPNOTSUPP;
	}

	pConfig->apidx = pObj->ioctl_if;
	return 0;
}


static void AP_E2PROM_IOCTL_PostCtrl(RTMP_IOCTL_INPUT_STRUCT *wrq, PSTRING msg)
{
	wrq->u.data.length = strlen(msg);
	if (copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length)) {
		DBGPRINT(RT_DEBUG_TRACE, ("%s: copy_to_user() fail\n", __FUNCTION__));
	}
}


static void IAPP_L2_UpdatePostCtrl(PRTMP_ADAPTER pAd, IN UINT8 *mac_p, int bssid)
{
}
#endif /* CONFIG_AP_SUPPORT */



