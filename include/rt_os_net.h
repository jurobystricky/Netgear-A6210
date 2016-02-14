/****************************************************************************

	Module Name:
	rt_os_net.h

	Abstract:
	All function prototypes are defined in NETIF modules.


***************************************************************************/

#ifndef __RT_OS_NET_H__
#define __RT_OS_NET_H__

#include "chip/chip_id.h"

typedef void *(*RTMP_NET_ETH_CONVERT_DEV_SEARCH)(void *net_dev, UCHAR *pData);
typedef int (*RTMP_NET_PACKET_TRANSMIT)(void *pPacket);


/* ========================================================================== */

NDIS_STATUS RTMPAllocAdapterBlock(PVOID handle, void **ppAdapter);
void RTMPFreeAdapter(void *pAd);
BOOLEAN RtmpRaDevCtrlExit(void *pAd);
BOOLEAN RtmpRaDevCtrlInit(void *pAd, RTMP_INF_TYPE infType);
void RTMPHandleInterrupt(void *pAd);

int RTMP_COM_IoctlHandle(void *pAd, int cmd, void *pData, ULONG Data);

int RTMPSendPackets(NDIS_HANDLE MiniportAdapterContext, PPNDIS_PACKET ppPacketArray,
	UINT NumberOfPackets, UINT32 PktTotalLen, 
	RTMP_NET_ETH_CONVERT_DEV_SEARCH Func);

int P2P_PacketSend(PNDIS_PACKET pPktSrc, PNET_DEV pDev,
	RTMP_NET_PACKET_TRANSMIT Func);

#ifdef CONFIG_AP_SUPPORT
int RTMP_AP_IoctlHandle(void *pAd, RTMP_IOCTL_INPUT_STRUCT *wrq, int cmd,
	USHORT subcmd, void *pData, ULONG Data);
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
int RTMP_STA_IoctlHandle(void *pAd, RTMP_IOCTL_INPUT_STRUCT *wrq, int cmd,
	USHORT subcmd, void *pData, ULONG Data, USHORT priv_flags);
#endif /* CONFIG_STA_SUPPORT */

void RTMPDrvOpen(void *pAd);
void RTMPDrvClose(void *pAd, void *net_dev);
void RTMPInfClose(void *pAd);

BOOLEAN rt28xx_init(void *pAd, PSTRING pDefaultMac, PSTRING pHostName);

PNET_DEV RtmpPhyNetDevMainCreate(void *pAd);

/* ========================================================================== */
int rt28xx_close(void *dev);
int rt28xx_open(void *dev);

__inline int VIRTUAL_IF_UP(void *pAd)
{
	RT_CMD_INF_UP_DOWN InfConf = { rt28xx_open, rt28xx_close };
	if (RTMP_COM_IoctlHandle(pAd, CMD_RTPRIV_IOCTL_VIRTUAL_INF_UP,
			&InfConf, 0) != NDIS_STATUS_SUCCESS)
		return -1;
	return 0;
}

__inline void VIRTUAL_IF_DOWN(void *pAd)
{
	RT_CMD_INF_UP_DOWN InfConf = { rt28xx_open, rt28xx_close };
	RTMP_COM_IoctlHandle(pAd, CMD_RTPRIV_IOCTL_VIRTUAL_INF_DOWN,
			&InfConf, 0);
	return;
}

#ifdef RTMP_MODULE_OS

#ifdef CONFIG_AP_SUPPORT
int rt28xx_ap_ioctl(PNET_DEV net_dev, struct ifreq *rq, int cmd);
#endif

#ifdef CONFIG_STA_SUPPORT
int rt28xx_sta_ioctl(PNET_DEV net_dev, struct ifreq *rq, int cmd);
#endif

PNET_DEV RtmpPhyNetDevInit(void *pAd, RTMP_OS_NETDEV_OP_HOOK *pNetHook);
BOOLEAN RtmpPhyNetDevExit(void *pAd, PNET_DEV net_dev);

#endif /* RTMP_MODULE_OS */

void RT28xx_MBSS_Init(void *pAd, PNET_DEV main_dev_p);
int MBSS_VirtualIF_Open(PNET_DEV dev_p);
int MBSS_VirtualIF_Close(PNET_DEV dev_p);
void RT28xx_MBSS_Remove(void *pAd);
void RT28xx_WDS_Init(void *pAd, PNET_DEV net_dev);
int WdsVirtualIF_open(PNET_DEV dev);
int WdsVirtualIF_close(PNET_DEV dev);
void RT28xx_WDS_Remove(void *pAd);
void RT28xx_Monitor_Init(void *pAd, PNET_DEV main_dev_p);
void RT28xx_Monitor_Remove(void *pAd);
void RT28xx_ApCli_Init(void *pAd, PNET_DEV main_dev_p);
int ApCli_VirtualIF_Open(PNET_DEV dev_p);
int ApCli_VirtualIF_Close(PNET_DEV dev_p);
void RT28xx_ApCli_Remove(void *pAd);
void RTMP_Mesh_Init(void *pAd, PNET_DEV main_dev_p, PSTRING pHostName);
int Mesh_VirtualIF_Open(PNET_DEV pDev);
int Mesh_VirtualIF_Close(PNET_DEV pDev);
void RTMP_Mesh_Remove(void *pAd);
void RTMP_P2P_Init(void *pAd, PNET_DEV main_dev_p);
int P2P_VirtualIF_Open(PNET_DEV dev_p);
int P2P_VirtualIF_Close(PNET_DEV dev_p);
int P2P_VirtualIF_PacketSend(PNDIS_PACKET skb_p, PNET_DEV dev_p);
void RTMP_P2P_Remove(void *pAd);

#ifdef RT_CFG80211_P2P_SUPPORT
#define CFG_P2PGO_ON(__pAd)  RTMP_CFG80211_VIF_P2P_GO_ON(__pAd)
#define CFG_P2PCLI_ON(__pAd) RTMP_CFG80211_VIF_P2P_CLI_ON(__pAd)

BOOLEAN RTMP_CFG80211_VIF_P2P_GO_ON(void pAdSrc);
BOOLEAN RTMP_CFG80211_VIF_P2P_CLI_ON(void *pAdSrc);

#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
void RTMP_CFG80211_DummyP2pIf_Init(void *pAdSrc);
void RTMP_CFG80211_DummyP2pIf_Remove(void *pAdSrc);

BOOLEAN RTMP_CFG80211_VIF_P2P_CLI_ON(void *pAdSrc);
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */
#endif /* RT_CFG80211_P2P_SUPPORT */

PNET_DEV RTMP_CFG80211_FindVifEntry_ByType(void *pAdSrc, UINT32 devType);
PWIRELESS_DEV RTMP_CFG80211_FindVifEntryWdev_ByType(void *pAdSrc, UINT32 devType);
void RTMP_CFG80211_AddVifEntry(void *pAdSrc, PNET_DEV pNewNetDev, UINT32 DevType);
void RTMP_CFG80211_RemoveVifEntry(void *pAdSrc, PNET_DEV pNewNetDev);

PNET_DEV RTMP_CFG80211_VirtualIF_Get(void *pAdSrc);
void RTMP_CFG80211_VirtualIF_CancelP2pClient(void *pAdSrc);
void RTMP_CFG80211_VirtualIF_Init(void *pAd, CHAR *pIfName, UINT32 DevType);
void RTMP_CFG80211_VirtualIF_Remove(void *pAd,PNET_DEV dev_p, UINT32 DevType);
void RTMP_CFG80211_AllVirtualIF_Remove(void *pAdSrc);

#ifdef RT_CFG80211_ANDROID_PRIV_LIB_SUPPORT
int rt_android_private_command_entry(
	void *pAdSrc, struct net_device *net_dev, struct ifreq *ifr, int cmd);
#endif


/* FOR communication with RALINK DRIVER module in NET module */
/* general */
#define RTMP_DRIVER_NET_DEV_GET(__pAd, __pNetDev)	\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_NETDEV_GET, __pNetDev, 0)

#define RTMP_DRIVER_NET_DEV_SET(__pAd, __pNetDev)	\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_NETDEV_SET, __pNetDev, 0)

#define RTMP_DRIVER_OP_MODE_GET(__pAd, __pOpMode)	\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_OPMODE_GET, __pOpMode, 0)

#define RTMP_DRIVER_IW_STATS_GET(__pAd, __pIwStats)	\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_INF_IW_STATUS_GET, __pIwStats, 0)

#define RTMP_DRIVER_INF_STATS_GET(__pAd, __pInfStats)	\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_INF_STATS_GET, __pInfStats, 0)

#define RTMP_DRIVER_INF_TYPE_GET(__pAd, __pInfType)	\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_INF_TYPE_GET, __pInfType, 0)

#define RTMP_DRIVER_TASK_LIST_GET(__pAd, __pList)	\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_TASK_LIST_GET, __pList, 0)

#define RTMP_DRIVER_NIC_NOT_EXIST_SET(__pAd)	\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_NIC_NOT_EXIST, NULL, 0)

#define RTMP_DRIVER_MCU_SLEEP_CLEAR(__pAd)	\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_MCU_SLEEP_CLEAR, NULL, 0)

#ifdef CONFIG_STA_SUPPORT
#ifdef CONFIG_PM
#ifdef USB_SUPPORT_SELECTIVE_SUSPEND

#define RTMP_DRIVER_USB_DEV_GET(__pAd, __pUsbDev) \
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_USB_DEV_GET, __pUsbDev, 0)

#define RTMP_DRIVER_USB_INTF_GET(__pAd, __pUsbIntf) \
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_USB_INTF_GET, __pUsbIntf, 0)

#define RTMP_DRIVER_ADAPTER_SUSPEND_SET(__pAd)	\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_ADAPTER_SUSPEND_SET, NULL, 0)

#define RTMP_DRIVER_ADAPTER_SUSPEND_CLEAR(__pAd)	\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_ADAPTER_SUSPEND_CLEAR, NULL, 0)

#define RTMP_DRIVER_ADAPTER_END_DISSASSOCIATE(__pAd)	\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_ADAPTER_SEND_DISSASSOCIATE, NULL, 0)

#define RTMP_DRIVER_ADAPTER_SUSPEND_TEST(__pAd, __flag)	\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_ADAPTER_SUSPEND_TEST,  __flag, 0)

#define RTMP_DRIVER_ADAPTER_IDLE_RADIO_OFF_TEST(__pAd, __flag)	\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_ADAPTER_IDLE_RADIO_OFF_TEST,  __flag, 0)
#endif /* USB_SUPPORT_SELECTIVE_SUSPEND */

#if (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT)
#define RTMP_DRIVER_ADAPTER_RT28XX_WOW_STATUS(__pAd, __flag)	\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_ADAPTER_RT28XX_WOW_STATUS, __flag, 0)

#define RTMP_DRIVER_ADAPTER_RT28XX_WOW_ENABLE(__pAd)	\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_ADAPTER_RT28XX_WOW_ENABLE, NULL, 0)

#define RTMP_DRIVER_ADAPTER_RT28XX_WOW_DISABLE(__pAd)	\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_ADAPTER_RT28XX_WOW_DISABLE, NULL, 0)
#endif /* (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) */

#endif /* CONFIG_PM */

#define RTMP_DRIVER_AP_SSID_GET(__pAd, pData)	\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_AP_BSSID_GET, pData, 0)
#endif /* CONFIG_STA_SUPPORT */

#define RTMP_DRIVER_ADAPTER_RT28XX_USB_ASICRADIO_OFF(__pAd)				\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_ADAPTER_RT28XX_USB_ASICRADIO_OFF, NULL, 0)

#define RTMP_DRIVER_ADAPTER_RT28XX_USB_ASICRADIO_ON(__pAd)				\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_ADAPTER_RT28XX_USB_ASICRADIO_ON, NULL, 0)

#define RTMP_DRIVER_ADAPTER_SUSPEND_SET(__pAd)						\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_ADAPTER_SUSPEND_SET, NULL, 0)

#define RTMP_DRIVER_ADAPTER_SUSPEND_CLEAR(__pAd)					\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_ADAPTER_SUSPEND_CLEAR, NULL, 0)

#define RTMP_DRIVER_VIRTUAL_INF_NUM_GET(__pAd, __pIfNum)				\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_VIRTUAL_INF_GET, __pIfNum, 0)

#define RTMP_DRIVER_CHANNEL_GET(__pAd, __Channel)					\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_SIOCGIWFREQ, __Channel, 0)

#define RTMP_DRIVER_IOCTL_SANITY_CHECK(__pAd, __SetCmd)					\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_SANITY_CHECK, __SetCmd, 0)

#define RTMP_DRIVER_BITRATE_GET(__pAd, __pBitRate)					\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_AP_SIOCGIWRATEQ, __pBitRate, 0)

#define RTMP_DRIVER_MAIN_INF_CREATE(__pAd, __ppNetDev)					\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_INF_MAIN_CREATE, __ppNetDev, 0)

#define RTMP_DRIVER_MAIN_INF_GET(__pAd, __pInfId)					\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_INF_MAIN_ID_GET, __pInfId, 0)

#define RTMP_DRIVER_MAIN_INF_CHECK(__pAd, __InfId)					\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_INF_MAIN_CHECK, NULL, __InfId)

#define RTMP_DRIVER_P2P_INF_CHECK(__pAd, __InfId)					\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_INF_P2P_CHECK, NULL, __InfId)

#ifdef EXT_BUILD_CHANNEL_LIST
#define RTMP_DRIVER_SET_PRECONFIG_VALUE(__pAd)						\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_SET_PRECONFIG_VALUE, NULL, 0)
#endif /* EXT_BUILD_CHANNEL_LIST */


#ifdef RT_CFG80211_SUPPORT
/* General Part */
#define RTMP_DRIVER_CFG80211_START(__pAd)						\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_CFG80211_CFG_START, NULL, 0)

#define RTMP_DRIVER_80211_UNREGISTER(__pAd, __pNetDev)					\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_80211_UNREGISTER, __pNetDev, 0)

#define RTMP_DRIVER_80211_CB_GET(__pAd, __ppCB)						\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_80211_CB_GET, __ppCB, 0)

#define RTMP_DRIVER_80211_CB_SET(__pAd, __pCB)						\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_80211_CB_SET, __pCB, 0)

#define RTMP_DRIVER_80211_RESET(__pAd)							\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_80211_RESET, NULL, 0)

/* STA Part */
#define RTMP_DRIVER_80211_SCAN_CHANNEL_LIST_SET(__pAd, __pData, __Len ) 		\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_80211_CHANNEL_LIST_SET, __pData, __Len)

#define RTMP_DRIVER_80211_SCAN(__pAd, __IfType)						\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_80211_SCAN, NULL, __IfType)

#define RTMP_DRIVER_80211_SCAN_STATUS_LOCK_INIT(__pAd, __isInit)			\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_80211_SCAN_STATUS_LOCK_INIT, NULL, __isInit)

#define RTMP_DRIVER_80211_SCAN_EXTRA_IE_SET(__pAd)					\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_80211_EXTRA_IES_SET,  NULL, 0)

#define RTMP_DRIVER_80211_GEN_IE_SET(__pAd, __pData, __Len)				\
	RTMP_STA_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_STA_SIOCSIWGENIE, 0, __pData, __Len, 0)

#define RTMP_DRIVER_80211_STA_KEY_ADD(__pAd, __pKeyInfo)				\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_80211_STA_KEY_ADD, __pKeyInfo, 0)

#define RTMP_DRIVER_80211_STA_KEY_DEFAULT_SET(__pAd, __KeyId)				\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_80211_STA_KEY_DEFAULT_SET, NULL, __KeyId)

#ifdef DOT11W_PMF_SUPPORT
#define RTMP_DRIVER_80211_STA_MGMT_KEY_DEFAULT_SET(__pAd, __KeyId)			\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_80211_STA_MGMT_KEY_DEFAULT_SET, NULL, __KeyId)
#endif /* DOT11W_PMF_SUPPORT */

#define RTMP_DRIVER_80211_POWER_MGMT_SET(__pAd, __enable)				\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_80211_POWER_MGMT_SET, NULL, __enable)

#define RTMP_DRIVER_80211_STA_LEAVE(__pAd, __ifType)					\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_80211_STA_LEAVE, NULL, __ifType)

#define RTMP_DRIVER_80211_STA_GET(__pAd, __pStaInfo)					\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_80211_STA_GET, __pStaInfo, 0)

#define RTMP_DRIVER_80211_CONNECT(__pAd, __pConnInfo, __devType)			\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_80211_CONNECT_TO, __pConnInfo, __devType)

#define RTMP_DRIVER_80211_IBSS_JOIN(__pAd, __pInfo)					\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_80211_IBSS_JOIN, __pInfo, 0)

#define RTMP_DRIVER_80211_PMKID_CTRL(__pAd, __pPmkidInfo)				\
	RTMP_STA_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_STA_SIOCSIWPMKSA, 0, __pPmkidInfo, 0, 0)

/* Information Part */
#define RTMP_DRIVER_80211_BANDINFO_GET(__pAd, __pBandInfo)				\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_80211_BANDINFO_GET, __pBandInfo, 0)

#define RTMP_DRIVER_80211_CHANGE_BSS_PARM(__pAd, __pBssInfo) \
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_80211_CHANGE_BSS_PARM, __pBssInfo, 0)

#define RTMP_DRIVER_80211_CHAN_SET(__pAd, __pChan)					\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_80211_CHAN_SET, __pChan, 0)

#define RTMP_DRIVER_80211_RFKILL(__pAd, __pActive)					\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_80211_RFKILL, __pActive, 0)

#define RTMP_DRIVER_80211_REG_NOTIFY(__pAd, __pNotify)					\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_80211_REG_NOTIFY_TO, __pNotify, 0)

#define RTMP_DRIVER_80211_SURVEY_GET(__pAd, __pSurveyInfo)				\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_80211_SURVEY_GET, __pSurveyInfo, 0)

#define RTMP_DRIVER_80211_NETDEV_EVENT(__pAd, __pDev, __state)				\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_80211_NETDEV_EVENT, __pDev, __state)

/* AP Part */
#define RTMP_DRIVER_80211_BEACON_DEL(__pAd) \
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_80211_BEACON_DEL,  NULL, 0)

#define RTMP_DRIVER_80211_BEACON_ADD(__pAd, __pBeacon) \
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_80211_BEACON_ADD, __pBeacon, 0)

#define RTMP_DRIVER_80211_BEACON_SET(__pAd, __pBeacon) \
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_80211_BEACON_SET, __pBeacon, 0)

#define RTMP_DRIVER_80211_BITRATE_SET(__pAd, __pMask) \
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_80211_BITRATE_SET, __pMask, 0)

#define RTMP_DRIVER_80211_AP_KEY_DEL(__pAd, __pKeyInfo) \
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_80211_AP_KEY_DEL, __pKeyInfo, 0)

#define RTMP_DRIVER_80211_AP_KEY_ADD(__pAd, __pKeyInfo) \
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_80211_AP_KEY_ADD, __pKeyInfo, 0)

#define RTMP_DRIVER_80211_RTS_THRESHOLD_ADD(__pAd, __Rts_thresold) \
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_80211_RTS_THRESHOLD_ADD, NULL, __Rts_thresold)

#define RTMP_DRIVER_80211_FRAG_THRESHOLD_ADD(__pAd, __Frag_thresold) \
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_80211_FRAG_THRESHOLD_ADD, NULL, __Frag_thresold)

#define RTMP_DRIVER_80211_AP_KEY_DEFAULT_SET(__pAd, __KeyId)				\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_80211_AP_KEY_DEFAULT_SET, NULL, __KeyId)

#define RTMP_DRIVER_80211_AP_PROBE_RSP(__pAd, __pFrame, __Len) \
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_80211_AP_PROBE_RSP_EXTRA_IE, __pFrame, __Len)

#define RTMP_DRIVER_80211_AP_ASSOC_RSP(__pAd, __pFrame, __Len) \
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_80211_AP_ASSOC_RSP_EXTRA_IE, __pFrame, __Len)

#define RTMP_DRIVER_80211_AP_MLME_PORT_SECURED(__pAd, __pMac, __Reg) \
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_80211_PORT_SECURED, __pMac, __Reg)

#define RTMP_DRIVER_80211_AP_STA_DEL(__pAd, __pMac) \
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_80211_AP_STA_DEL, __pMac, 0)

/* P2P Part */
#define RTMP_DRIVER_80211_ACTION_FRAME_REG(__pAd, __devPtr, __Reg) \
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_80211_ACTION_FRAME_REG, __devPtr, __Reg)

#define RTMP_DRIVER_80211_REMAIN_ON_CHAN_DUR_IMER_INIT(__pAd)                       \
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_80211_REMAIN_ON_CHAN_DUR_TIMER_INIT, NULL, 0)

#define RTMP_DRIVER_80211_REMAIN_ON_CHAN_SET(__pAd, __pChan, __Duration)  \
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_80211_REMAIN_ON_CHAN_SET, __pChan, __Duration)

#define RTMP_DRIVER_80211_CANCEL_REMAIN_ON_CHAN_SET(__pAd, __cookie) \
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_80211_CANCEL_REMAIN_ON_CHAN_SET, NULL, __cookie)

#define RTMP_DRIVER_80211_CHANNEL_LOCK(__pAd, __Chan)                   \
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_80211_CHANNEL_LOCK, NULL , __Chan)

#define RTMP_DRIVER_80211_MGMT_FRAME_REG(__pAd, __devPtr, __Reg) \
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_80211_MGMT_FRAME_REG, __devPtr, __Reg)

#define RTMP_DRIVER_80211_MGMT_FRAME_SEND(__pAd, __pFrame, __Len)                       \
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_80211_MGMT_FRAME_SEND, __pFrame, __Len)

#define RTMP_DRIVER_80211_P2P_CHANNEL_RESTORE(__pAd)				\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_80211_CHANNEL_RESTORE,  NULL, 0)

#define RTMP_DRIVER_80211_P2PCLI_ASSSOC_IE_SET(__pAd, __pFrame, __Len) \
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_80211_P2PCLI_ASSSOC_IE_SET, __pFrame, __Len)

#define RTMP_DRIVER_80211_P2P_CLIENT_KEY_ADD(__pAd, __pKeyInfo)					\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_80211_P2P_CLIENT_KEY_ADD, __pKeyInfo, 0)

/* VIF Part */
#define RTMP_DRIVER_80211_VIF_ADD(__pAd, __pInfo) \
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_80211_VIF_ADD, __pInfo, 0)

#define RTMP_DRIVER_80211_VIF_DEL(__pAd, __devPtr, __type) \
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_80211_VIF_DEL, __devPtr, __type)

#define RTMP_DRIVER_80211_VIF_CHG(__pAd, __pVifInfo)			\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_80211_VIF_CHG, __pVifInfo, 0)

#ifdef RT_P2P_SPECIFIC_WIRELESS_EVENT
#define RTMP_DRIVER_80211_SEND_WIRELESS_EVENT(__pAd, __pMacAddr)	\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_80211_SEND_WIRELESS_EVENT, __pMacAddr, 0)
#endif /* RT_P2P_SPECIFIC_WIRELESS_EVENT */
#endif /* RT_CFG80211_SUPPORT */

/* mesh */
#define RTMP_DRIVER_MESH_REMOVE(__pAd)		\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_MESH_REMOVE, NULL, 0)

/* inf ppa */
#define RTMP_DRIVER_INF_PPA_INIT(__pAd)		\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_INF_PPA_INIT, NULL, 0)

#define RTMP_DRIVER_INF_PPA_EXIT(__pAd)		\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_INF_PPA_EXIT, NULL, 0)


#define RTMP_DRIVER_IRQ_INIT(__pAd)							\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_IRQ_INIT, NULL, 0)

#define RTMP_DRIVER_IRQ_RELEASE(__pAd)							\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_IRQ_RELEASE, NULL, 0)

/* usb */
#define RTMP_DRIVER_USB_MORE_FLAG_SET(__pAd, __pConfig)					\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_USB_MORE_FLAG_SET, __pConfig, 0)

#define RTMP_DRIVER_USB_CONFIG_INIT(__pAd, __pConfig)					\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_USB_CONFIG_INIT, __pConfig, 0)

#define RTMP_DRIVER_USB_SUSPEND(__pAd, __bIsRunning)					\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_USB_SUSPEND, NULL, __bIsRunning)

#define RTMP_DRIVER_USB_RESUME(__pAd)							\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_USB_RESUME, NULL, 0)

#define RTMP_DRIVER_USB_INIT(__pAd, __pUsbDev, __driver_info)	\
do {	\
	RT_CMD_USB_INIT __Config, *__pConfig = &__Config;	\
	__pConfig->pUsbDev = __pUsbDev;	\
	__pConfig->driver_info = __driver_info;	\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_USB_INIT, __pConfig, 0);	\
} while (0)

/* ap */
#define RTMP_DRIVER_AP_BITRATE_GET(__pAd, __pConfig)					\
	RTMP_AP_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_AP_SIOCGIWRATEQ, __pConfig, 0)

#define RTMP_DRIVER_AP_MAIN_OPEN(__pAd)							\
	RTMP_AP_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_MAIN_OPEN, 0, NULL, 0)

/* sta */
#define RTMP_DRIVER_STA_DEV_TYPE_SET(__pAd, __Type)					\
	RTMP_STA_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_ORI_DEV_TYPE_SET, 0, NULL, __Type, __Type)

#define RTMP_DRIVER_MAC_ADDR_GET(__pAd, __pMacAddr)					\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_MAC_ADDR_GET, 0, __pMacAddr, 0)

#define RTMP_DRIVER_ADAPTER_CSO_SUPPORT_TEST(__pAd, __flag)				\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_ADAPTER_CSO_SUPPORT_TEST, __flag, 0)

#define RTMP_DRIVER_ADAPTER_TSO_SUPPORT_TEST(__pAd, __flag)				\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_ADAPTER_TSO_SUPPORT_TEST, __flag, 0)

#ifdef CONFIG_HAS_EARLYSUSPEND
#define RTMP_DRIVER_SET_SUSPEND_FLAG(__pAd) \
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_SET_SUSPEND_FLAG, NULL, 0)

#define RTMP_DRIVER_LOAD_FIRMWARE_CHECK(__pAd)						\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_LOAD_FIRMWARE_CHECK, NULL, 0)

#define RTMP_DRIVER_OS_COOKIE_GET(__pAd, __os_cookie)					\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_OS_COOKIE_GET, __os_cookie, 0)

#define RTMP_DRIVER_ADAPTER_REGISTER_EARLYSUSPEND(__pAd)	\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_REGISTER_EARLYSUSPEND, 0, NULL, 0)

#define RTMP_DRIVER_ADAPTER_UNREGISTER_EARLYSUSPEND(__pAd)	\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_UNREGISTER_EARLYSUSPEND, NULL, 0)

#define RTMP_DRIVER_ADAPTER_CHECK_EARLYSUSPEND(__pAd, __flag)	\
	RTMP_COM_IoctlHandle(__pAd, CMD_RTPRIV_IOCTL_CHECK_EARLYSUSPEND, __flag, 0)
#endif /* CONFIG_HAS_EARLYSUSPEND */

#endif /* __RT_OS_NET_H__ */
