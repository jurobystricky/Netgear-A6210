/*

*/


#ifdef SYSTEM_LOG_SUPPORT

#include "rt_config.h"

/* for wireless system event message */
static char const *pWirelessSysEventText[IW_SYS_EVENT_TYPE_NUM] = {
	/* system status event */
	"had associated successfully",				/* IW_ASSOC_EVENT_FLAG */
	"had disassociated",					/* IW_DISASSOC_EVENT_FLAG */
	"had deauthenticated",					/* IW_DEAUTH_EVENT_FLAG */
	"had been aged-out and disassociated",			/* IW_AGEOUT_EVENT_FLAG */
	"occurred CounterMeasures attack",			/* IW_COUNTER_MEASURES_EVENT_FLAG */
	"occurred replay counter different in Key Handshaking",	/* IW_REPLAY_COUNTER_DIFF_EVENT_FLAG */
	"occurred RSNIE different in Key Handshaking",		/* IW_RSNIE_DIFF_EVENT_FLAG */
	"occurred MIC different in Key Handshaking",		/* IW_MIC_DIFF_EVENT_FLAG */
	"occurred ICV error in RX",				/* IW_ICV_ERROR_EVENT_FLAG */
	"occurred MIC error in RX",				/* IW_MIC_ERROR_EVENT_FLAG */
	"Group Key Handshaking timeout",			/* IW_GROUP_HS_TIMEOUT_EVENT_FLAG */
	"Pairwise Key Handshaking timeout",			/* IW_PAIRWISE_HS_TIMEOUT_EVENT_FLAG */
	"RSN IE sanity check failure",				/* IW_RSNIE_SANITY_FAIL_EVENT_FLAG */
	"set key done in WPA/WPAPSK",				/* IW_SET_KEY_DONE_WPA1_EVENT_FLAG */
	"set key done in WPA2/WPA2PSK",				/* IW_SET_KEY_DONE_WPA2_EVENT_FLAG */
	"connects with our wireless client",			/* IW_STA_LINKUP_EVENT_FLAG */
	"disconnects with our wireless client",			/* IW_STA_LINKDOWN_EVENT_FLAG */
	"scan completed",					/* IW_SCAN_COMPLETED_EVENT_FLAG */
	"scan terminate!! Busy!! Enqueue fail!!",		/* IW_SCAN_ENQUEUE_FAIL_EVENT_FLAG */
	"channel switch to ",					/* IW_CHANNEL_CHANGE_EVENT_FLAG */
	"wireless mode is not support",				/* IW_STA_MODE_EVENT_FLAG */
	"blacklisted in MAC filter list",			/* IW_MAC_FILTER_LIST_EVENT_FLAG */
	"Authentication rejected because of challenge failure",	/* IW_AUTH_REJECT_CHALLENGE_FAILURE */
	"Scanning",						/* IW_SCANNING_EVENT_FLAG */
	"Start a new IBSS",					/* IW_START_IBSS_FLAG */
	"Join the IBSS",					/* IW_JOIN_IBSS_FLAG */
	"Shared WEP fail",					/* IW_SHARED_WEP_FAIL*/
	};

/*
	========================================================================

	Routine Description:
		Send log message through wireless event

		Support standard iw_event with IWEVCUSTOM. It is used below.

		iwreq_data.data.flags is used to store event_flag that is defined by user.
		iwreq_data.data.length is the length of the event log.

		The format of the event log is composed of the entry's MAC address and
		the desired log message (refer to pWirelessEventText).

			ex: 11:22:33:44:55:66 has associated successfully

		p.s. The requirement of Wireless Extension is v15 or newer.

	========================================================================
*/
void RtmpDrvSendWirelessEvent(void *pAdSrc, USHORT Event_flag, PUCHAR pAddr,
	UCHAR BssIdx, CHAR Rssi)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	PSTRING pBuf, pBufPtr = NULL;
	USHORT event, type, BufLen;
	UCHAR event_table_len = 0;

	if (pAd->CommonCfg.bWirelessEvent == FALSE)
		return;

	type = Event_flag & 0xFF00;
	event = Event_flag & 0x00FF;

	switch (type) {
	case IW_SYS_EVENT_FLAG_START:
		event_table_len = IW_SYS_EVENT_TYPE_NUM;
		break;
	}

	if (event_table_len == 0) {
		DBGPRINT(RT_DEBUG_ERROR, (
				"%s : The type(%0x02x) is not valid.\n", 
				__FUNCTION__, type));
		return;
	}

	if (event >= event_table_len) {
		DBGPRINT(RT_DEBUG_ERROR, 
				("%s : The event(%0x02x) is not valid.\n", 
				__FUNCTION__, event));
		return;
	}

	/*Allocate memory and copy the msg. */
/*	if((pBuf = kmalloc(IW_CUSTOM_MAX_LEN, GFP_ATOMIC)) != NULL) */
	pBuf = os_alloc_mem(IW_CUSTOM_MAX_LEN);
	if (pBuf != NULL) {
		/*Prepare the payload */
		memset(pBuf, 0, IW_CUSTOM_MAX_LEN);
		pBufPtr = pBuf;

		if (pAddr)
			pBufPtr += sprintf(pBufPtr, 
					"(RT2860) STA(%02x:%02x:%02x:%02x:%02x:%02x) ",
					PRINT_MAC(pAddr));
		else if (BssIdx < MAX_MBSSID_NUM(pAd))
			pBufPtr += sprintf(pBufPtr, 
					"(RT2860) BSS(ra%d) ", BssIdx);
		else
			pBufPtr += sprintf(pBufPtr, "(RT2860) ");

		if (type == IW_SYS_EVENT_FLAG_START) {
			pBufPtr += sprintf(pBufPtr, "%s", pWirelessSysEventText[event]);

			if (Event_flag == IW_CHANNEL_CHANGE_EVENT_FLAG) {
			 	pBufPtr += sprintf(pBufPtr, "%3d", Rssi);
			}
		} else {
			pBufPtr += sprintf(pBufPtr, "%s", "unknown event");
		}

		pBufPtr[pBufPtr - pBuf] = '\0';
		BufLen = pBufPtr - pBuf;

		RtmpOSWrielessEventSend(pAd->net_dev, RT_WLAN_EVENT_CUSTOM, 
				Event_flag, NULL, (PUCHAR)pBuf, BufLen);
		/*DBGPRINT(RT_DEBUG_TRACE, ("%s : %s\n", __FUNCTION__, pBuf)); */

		os_free_mem(pBuf);
	} else {
		DBGPRINT(RT_DEBUG_ERROR, 
				("%s : Can't allocate memory for wireless event.\n", 
				__FUNCTION__));
	}
}

#endif /* SYSTEM_LOG_SUPPORT */


