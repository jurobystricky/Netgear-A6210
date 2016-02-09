/****************************************************************************

    Module Name:
	rt_os_util.h

	Abstract:
	All function prototypes are provided from UTIL modules.

	Note:
	But can not use any OS key word and compile option here.
	All functions are provided from UTIL modules.


***************************************************************************/

#ifndef __RT_OS_UTIL_H__
#define __RT_OS_UTIL_H__

/* ============================ rt_linux.c ================================== */
/* General */
void RtmpUtilInit(void);

/* OS Time */
void RtmpusecDelay(ULONG usec);
void RtmpOsMsDelay(ULONG msec);
void RTMP_GetCurrentSystemTime(LARGE_INTEGER *time);
void RTMP_GetCurrentSystemTick(ULONG *pNow);
void RtmpOsWait(UINT32 Time);
UINT32 RtmpOsTimerAfter(ULONG a, ULONG b);
UINT32 RtmpOsTimerBefore(ULONG a, ULONG b);
void RtmpOsGetSystemUpTime(ULONG *pTime);
UINT32 RtmpOsTickUnitGet(void);
void *os_alloc_mem(size_t size);
NDIS_STATUS os_alloc_mem_suspend(void *pReserved, UCHAR **mem, ULONG size);
void os_free_mem(void *mem);
void RtmpOsVfree(void *pMem);
ULONG RtmpOsCopyFromUser(void *to, const void *from, ULONG n);
ULONG RtmpOsCopyToUser(void *to, const void *from, ULONG n);
BOOLEAN RtmpOsStatsAlloc(void **ppStats, void **ppIwStats);

/* OS Packet */

PNDIS_PACKET RTMP_AllocateFragPacketBuffer(void *pReserved, ULONG Length);

NDIS_STATUS RTMPAllocateNdisPacket(void *pReserved, PNDIS_PACKET *ppPacket,
	PUCHAR pHeader, UINT HeaderLen, PUCHAR pData, UINT DataLen);

void RTMP_QueryPacketInfo(PNDIS_PACKET pPacket, PACKET_INFO *pPacketInfo,
	PUCHAR *pSrcBufVA, UINT *pSrcBufLen);

PNDIS_PACKET DuplicatePacket(PNET_DEV pNetDev, PNDIS_PACKET pPacket,
	UCHAR FromWhichBSSID);

PNDIS_PACKET duplicate_pkt(PNET_DEV pNetDev, PUCHAR pHeader802_3, UINT HdrLen,
	PUCHAR pData, ULONG DataSize, UCHAR FromWhichBSSID);

PNDIS_PACKET duplicate_pkt_with_TKIP_MIC(void *pReserved, PNDIS_PACKET pOldPkt);

PNDIS_PACKET duplicate_pkt_with_VLAN(PNET_DEV pNetDev, USHORT VLAN_VID,
	USHORT VLAN_Priority, PUCHAR pHeader802_3, UINT HdrLen, PUCHAR pData,
	ULONG DataSize, UCHAR FromWhichBSSID, UCHAR *TPID);

typedef void (*RTMP_CB_8023_PACKET_ANNOUNCE)(void *pCtrlBkPtr, PNDIS_PACKET pPacket,
	UCHAR OpMode);

BOOLEAN RTMPL2FrameTxAction(void *pCtrlBkPtr, PNET_DEV pNetDev,
	RTMP_CB_8023_PACKET_ANNOUNCE _announce_802_3_packet, UCHAR apidx,
	PUCHAR pData, UINT32 data_len, UCHAR OpMode);

PNDIS_PACKET ExpandPacket(void *pReserved, PNDIS_PACKET pPacket, UINT32 ext_head_len,
	UINT32 ext_tail_len);

PNDIS_PACKET ClonePacket(void *pReserved, PNDIS_PACKET pPacket, PUCHAR pData,
	ULONG DataSize);

void wlan_802_11_to_802_3_packet(PNET_DEV pNetDev, UCHAR OpMode, USHORT VLAN_VID,
	USHORT VLAN_Priority, PNDIS_PACKET pRxPacket, UCHAR *pData, ULONG DataSize,
	PUCHAR pHeader802_3, UCHAR FromWhichBSSID, UCHAR *TPID);

UCHAR VLAN_8023_Header_Copy(USHORT VLAN_VID, USHORT VLAN_Priority, PUCHAR pHeader802_3,
	UINT HdrLen, PUCHAR pData, UCHAR FromWhichBSSID, UCHAR *TPID);

void RtmpOsPktRcvHandle(PNDIS_PACKET pNetPkt);
void RtmpOsPktInit(PNDIS_PACKET pNetPkt, PNET_DEV pNetDev, UCHAR *buf, USHORT len);

/* OS File */
RTMP_OS_FD RtmpOSFileOpen(char *pPath,  int flag, int mode);
int RtmpOSFileClose(RTMP_OS_FD osfd);
void RtmpOSFileSeek(RTMP_OS_FD osfd, int offset);
int RtmpOSFileRead(RTMP_OS_FD osfd, char *pDataPtr, int readLen);
int RtmpOSFileWrite(RTMP_OS_FD osfd, char *pDataPtr, int writeLen);
void RtmpOSFSInfoChange(RTMP_OS_FS_INFO *pOSFSInfoOrg, BOOLEAN bSet);

/* OS Network Interface */
int RtmpOSNetDevAddrSet(UCHAR OpMode, PNET_DEV pNetDev, PUCHAR pMacAddr,
	PUCHAR dev_name);

void RtmpOSNetDevFree(PNET_DEV pNetDev);

#ifdef CONFIG_STA_SUPPORT
int RtmpOSNotifyRawData(PNET_DEV pNetDev, UCHAR *buf, int len, ULONG type, USHORT proto);
#endif

void RtmpOSNetDevDetach(PNET_DEV pNetDev);
int RtmpOSNetDevAttach(UCHAR OpMode, PNET_DEV pNetDev, RTMP_OS_NETDEV_OP_HOOK *pDevOpHook);

PNET_DEV RtmpOSNetDevCreate(INT32 MC_RowID, UINT32 *pIoctlIF, int devType,
	int devNum, int privMemSize, PSTRING pNamePrefix);

BOOLEAN RtmpOSNetDevIsUp(void *pDev);
char *RtmpOsGetNetDevName(void *pDev);
UINT32 RtmpOsGetNetIfIndex( void *pDev);
void RtmpOsSetNetDevPriv(void *pDev, void *pPriv);
void *RtmpOsGetNetDevPriv(void *pDev);
void RtmpOsSetNetDevWdev(void *net_dev, void *wdev);
void *RtmpOsGetNetDevWdev(void *pDev);
USHORT RtmpDevPrivFlagsGet(void *pDev);
void RtmpDevPrivFlagsSet(void *pDev, USHORT PrivFlags);
UCHAR get_sniffer_mode(void *pDev);
void RtmpOsCmdUp(RTMP_OS_TASK *pCmdQTask);
void RtmpOsMlmeUp(RTMP_OS_TASK *pMlmeQTask);
NDIS_STATUS RtmpOSTaskKill(RTMP_OS_TASK *pTaskOrg);
int RtmpOSTaskNotifyToExit(RTMP_OS_TASK *pTaskOrg);
void RtmpOSTaskCustomize(RTMP_OS_TASK *pTaskOrg);

NDIS_STATUS RtmpOSTaskAttach(RTMP_OS_TASK *pTaskOrg, RTMP_OS_TASK_CALLBACK fn,
	ULONG arg);

NDIS_STATUS RtmpOSTaskInit(RTMP_OS_TASK *pTaskOrg, PSTRING pTaskName, void *pPriv,
	LIST_HEADER *pTaskList, LIST_HEADER *pSemList);

BOOLEAN RtmpOSTaskWait(void *pReserved, RTMP_OS_TASK *pTaskOrg, INT32 *pStatus);

/* OS Cache */
void RtmpOsDCacheFlush(ULONG AddrStart, ULONG Size);

/* OS Timer */
void RTMP_SetPeriodicTimer(struct timer_list *pTimerOrg, unsigned long timeout);

void RTMP_OS_Init_Timer(void *pReserved, struct timer_list *pTimerOrg,
	TIMER_FUNCTION function, PVOID data, LIST_HEADER *pTimerList);

void RTMP_OS_Add_Timer(struct timer_list *pTimerOrg, unsigned long timeout);
void RTMP_OS_Mod_Timer(struct timer_list *pTimerOrg, unsigned long timeout);
void RTMP_OS_Del_Timer(struct timer_list *pTimerOrg, BOOLEAN *pCancelled);

/* OS Utility */
void hex_dump(char *str, unsigned char *pSrcBufVA, unsigned int SrcBufLen);

typedef void (*RTMP_OS_SEND_WLAN_EVENT)(void *pAdSrc, USHORT Event_flag,
	PUCHAR pAddr, UCHAR BssIdx, CHAR Rssi);

void RtmpOsSendWirelessEvent(void *pAd, USHORT Event_flag, PUCHAR pAddr,
	UCHAR BssIdx, CHAR Rssi, RTMP_OS_SEND_WLAN_EVENT pFunc);

#ifdef CONFIG_AP_SUPPORT
void SendSignalToDaemon(int sig, RTMP_OS_PID pid, unsigned long pid_no);
#endif

int RtmpOSWrielessEventSend(PNET_DEV pNetDev, UINT32 eventType, int flags,
	PUCHAR pSrcMac, PUCHAR pData, UINT32 dataLen);

int RtmpOSWrielessEventSendExt(PNET_DEV pNetDev, UINT32 eventType, int flags,
	PUCHAR pSrcMac, PUCHAR pData, UINT32 dataLen, UINT32 family);

UINT RtmpOsWirelessExtVerGet(void);

void RtmpDrvAllMacPrint(void *pReserved, UINT32 *pBufMac, UINT32 AddrStart,
	UINT32 AddrEnd, UINT32 AddrStep);

void RtmpDrvAllE2PPrint(void *pReserved, USHORT *pMacContent, UINT32 AddrEnd,
	UINT32 AddrStep);

void RtmpDrvAllRFPrint(void *pReserved, UCHAR *pBuf, UINT32 BufLen);
void RtmpOsWlanEventSet(void *pReserved, BOOLEAN *pCfgWEnt, BOOLEAN FlgIsWEntSup);
UINT16 RtmpOsGetUnaligned(UINT16 *pWord);
UINT32 RtmpOsGetUnaligned32(UINT32 *pWord);
ULONG RtmpOsGetUnalignedlong(ULONG *pWord);
long RtmpOsSimpleStrtol(const char *cp, char **endp, unsigned int base);

/* ============================ rt_os_util.c ================================ */
void RtmpDrvRateGet(void *pReserved, UINT8 MODE, UINT8 ShortGI, UINT8 BW,
	UINT8 MCS, UINT8 Antenna, ULONG *pRate);

char * rtstrchr(const char * s, int c);

void WpaSendMicFailureToWpaSupplicant(PNET_DEV pNetDev, const PUCHAR src_addr,
	BOOLEAN bUnicast, int key_id, const PUCHAR tsc);

int wext_notify_event_assoc(PNET_DEV pNetDev, UCHAR *ReqVarIEs, UINT32 ReqVarIELen);

void SendAssocIEsToWpaSupplicant(PNET_DEV pNetDev, UCHAR *ReqVarIEs, UINT32 ReqVarIELen);

#ifdef CONFIG_STA_SUPPORT
#ifdef CONFIG_PM
#ifdef USB_SUPPORT_SELECTIVE_SUSPEND

int RTMP_Usb_AutoPM_Put_Interface(void *pUsb_Dev, void *intf);
int RTMP_Usb_AutoPM_Get_Interface(void *pUsb_Dev, void *intf);

#endif /* USB_SUPPORT_SELECTIVE_SUSPEND */
#endif /* CONFIG_PM */
#endif /* CONFIG_STA_SUPPORT */

/* ============================ rt_usb_util.c =============================== */

#if defined(RTMP_RBUS_SUPPORT) || defined(RTMP_FLASH_SUPPORT)
void RtmpFlashRead(UCHAR *p, ULONG a, ULONG b);
void RtmpFlashWrite(UCHAR *p, ULONG a, ULONG b);
#endif /* defined(RTMP_RBUS_SUPPORT) || defined(RTMP_FLASH_SUPPORT) */

UINT32 RtmpOsGetUsbDevVendorID(void *pUsbDev);
UINT32 RtmpOsGetUsbDevProductID(void *pUsbDev);

/* CFG80211 */
#ifdef RT_CFG80211_SUPPORT
typedef struct __CFG80211_BAND {
	UINT8 RFICType;
	UINT8 MpduDensity;
	UINT8 TxStream;
	UINT8 RxStream;
	UINT32 MaxTxPwr;
	UINT32 MaxBssTable;
	UINT16 RtsThreshold;
	UINT16 FragmentThreshold;
	UINT32 RetryMaxCnt; /* bit0~7: short; bit8 ~ 15: long */
	BOOLEAN FlgIsBMode;
} CFG80211_BAND;

void CFG80211OS_UnRegister(void *pCB, void *pNetDev);

BOOLEAN CFG80211_SupBandInit(void *pCB, CFG80211_BAND *pBandInfo, void *pWiphyOrg,
	void *pChannelsOrg, void *pRatesOrg);

BOOLEAN CFG80211OS_SupBandReInit(void *pCB, CFG80211_BAND *pBandInfo);
void CFG80211OS_RegHint(void *pCB, UCHAR *pCountryIe, ULONG CountryIeLen);
void CFG80211OS_RegHint11D(void *pCB, UCHAR *pCountryIe, ULONG CountryIeLen);

BOOLEAN CFG80211OS_BandInfoGet(void *pCB, void *pWiphyOrg, void **ppBand24,
	void **ppBand5);

UINT32 CFG80211OS_ChanNumGet(void *pCB, void *pWiphyOrg, UINT32 IdBand);

BOOLEAN CFG80211OS_ChanInfoGet(void *pCB, void *pWiphyOrg, UINT32 IdBand,
	UINT32 IdChan, UINT32 *pChanId, UINT32 *pPower, BOOLEAN *pFlgIsRadar);

BOOLEAN CFG80211OS_ChanInfoInit(void *pCB, UINT32 InfoIndex, UCHAR ChanId,
	UCHAR MaxTxPwr, BOOLEAN FlgIsNMode, BOOLEAN FlgIsBW20M);

void CFG80211OS_Scaning(void *pCB, UINT32 ChanId, UCHAR *pFrame, UINT32 FrameLen,
	INT32 RSSI, BOOLEAN FlgIsNMode, UINT8 BW);

void CFG80211OS_ScanEnd(void *pCB, BOOLEAN FlgIsAborted);

void CFG80211OS_ConnectResultInform(void *pCB, UCHAR *pBSSID, UCHAR *pReqIe,
	UINT32 ReqIeLen, UCHAR *pRspIe, UINT32 RspIeLen, UCHAR FlgIsSuccess);

void CFG80211OS_P2pClientConnectResultInform(PNET_DEV pNetDev, UCHAR *pBSSID,
	UCHAR *pReqIe, UINT32 ReqIeLen, UCHAR *pRspIe, UINT32 RspIeLen,
	UCHAR FlgIsSuccess);

BOOLEAN CFG80211OS_RxMgmt(PNET_DEV pNetDev, INT32 freq, PUCHAR frame, UINT32 len);

void CFG80211OS_TxStatus(PNET_DEV pNetDev, INT32 cookie, PUCHAR frame,
	UINT32 len, BOOLEAN ack);

#ifdef WPA_SUPPLICANT_SUPPORT
void CFG80211OS_MICFailReport(PNET_DEV pNetDev, const PUCHAR src_addr,
	BOOLEAN unicast, int key_id, const PUCHAR tsc);
#endif

#endif /* RT_CFG80211_SUPPORT */


/* ================================ MACRO =================================== */
#define RTMP_UTIL_DCACHE_FLUSH(__AddrStart, __Size)

/* ================================ EXTERN ================================== */
extern UCHAR SNAP_802_1H[6];
extern UCHAR SNAP_BRIDGE_TUNNEL[6];
extern UCHAR EAPOL[2];
extern UCHAR TPID[];
extern UCHAR IPX[2];
extern UCHAR APPLE_TALK[2];
extern UCHAR NUM_BIT8[8];
extern UINT32 RalinkRate_Legacy[];
extern UINT32 RalinkRate_HT_1NSS[Rate_BW_MAX][Rate_GI_MAX][Rate_MCS];
extern UINT32 RalinkRate_VHT_1NSS[Rate_BW_MAX][Rate_GI_MAX][Rate_MCS];
extern UINT8 newRateGetAntenna(UINT8 MCS);

INT32  RtPrivIoctlSetVal(void);

void OS_SPIN_LOCK(spinlock_t *lock);
void OS_SPIN_UNLOCK(spinlock_t *lock);
void OS_SPIN_LOCK_IRQSAVE(spinlock_t *lock, unsigned long *flags);
void OS_SPIN_UNLOCK_IRQRESTORE(spinlock_t *lock, unsigned long *flags);
void OS_SPIN_LOCK_IRQ(spinlock_t *lock);
void OS_SPIN_UNLOCK_IRQ(spinlock_t *lock);
void RtmpOsSpinLockIrqSave(spinlock_t *lock, unsigned long *flags);
void RtmpOsSpinUnlockIrqRestore(spinlock_t *lock, unsigned long *flags);
void RtmpOsSpinLockIrq(spinlock_t *lock);
void RtmpOsSpinUnlockIrq(spinlock_t *lock);
int  OS_TEST_BIT(int bit, unsigned long *flags);
void OS_SET_BIT(int bit, unsigned long *flags);
void OS_CLEAR_BIT(int bit, unsigned long *flags);
void OS_LOAD_CODE_FROM_BIN(unsigned char **image, char *bin_name, void *inf_dev, UINT32 *code_len);

#endif /* __RT_OS_UTIL_H__ */
