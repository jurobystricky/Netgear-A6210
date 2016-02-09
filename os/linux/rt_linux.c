/****************************************************************************

	Module Name:
	UTIL/rt_linux.c

	Abstract:
	All functions provided from OS module are put here.

	Note:
	1. Can not use sizeof() for a structure with any parameter included
	by any compile option, such as RTMP_ADAPTER.

	Because the RTMP_ADAPTER size in the UTIL module is different with
	DRIVER/NETIF.

	2. Do not use any structure with any parameter included by PCI/USB/RBUS/
	AP/STA.

	Because the structure size in the UTIL module is different with
	DRIVER/NETIF.

	3. Do not use any structure defined in DRIVER module, EX: pAd.
	So we can do module partition.

	Revision History:
	Who        When          What
	---------  ----------    -------------------------------------------

***************************************************************************/

#define RTMP_MODULE_OS
#define RTMP_MODULE_OS_UTIL

#include "rtmp_comm.h"
#include "rt_os_util.h"
#include "dot11i_wpa.h"
#include <linux/rtnetlink.h>

#if defined(CONFIG_RA_HW_NAT) || defined(CONFIG_RA_HW_NAT_MODULE)
#include "../../../../../../net/nat/hw_nat/ra_nat.h"
#include "../../../../../../net/nat/hw_nat/frame_engine.h"
#endif

/* TODO */
#undef RT_CONFIG_IF_OPMODE_ON_AP
#undef RT_CONFIG_IF_OPMODE_ON_STA

#if defined(CONFIG_AP_SUPPORT) && defined(CONFIG_STA_SUPPORT)
#define RT_CONFIG_IF_OPMODE_ON_AP(__OpMode)	if (__OpMode == OPMODE_AP)
#define RT_CONFIG_IF_OPMODE_ON_STA(__OpMode)	if (__OpMode == OPMODE_STA)
#else
#define RT_CONFIG_IF_OPMODE_ON_AP(__OpMode)
#define RT_CONFIG_IF_OPMODE_ON_STA(__OpMode)
#endif


#ifdef OS_ABL_FUNC_SUPPORT
static ULONG RTPktOffsetData = 0, RTPktOffsetLen = 0, RTPktOffsetCB = 0;
#endif

/*
 * the lock will not be used in TX/RX
 * path so throughput should not be impacted
 */
static BOOLEAN FlgIsUtilInit = FALSE;
spinlock_t UtilSemLock;

static void RtmpOSNetDeviceRefPut(PNET_DEV pNetDev);

/*
========================================================================
Routine Description:
	Initialize something in UTIL module.

Arguments:
	None

Return Value:
	None

Note:
========================================================================
*/
void RtmpUtilInit(void)
{
	if (FlgIsUtilInit == FALSE) {
		OS_NdisAllocateSpinLock(&UtilSemLock);
		FlgIsUtilInit = TRUE;
	}
}

/* timeout -- ms */
static inline void __RTMP_SetPeriodicTimer(struct timer_list * pTimer,
	unsigned long timeout)
{
	timeout = ((timeout * OS_HZ) / 1000);
	pTimer->expires = jiffies + timeout;
	add_timer(pTimer);
}

/* convert NdisMInitializeTimer --> RTMP_OS_Init_Timer */
static inline void __RTMP_OS_Init_Timer(void *pReserved,
	struct timer_list * pTimer, TIMER_FUNCTION function,
	PVOID data)
{
	if (!timer_pending(pTimer)) {
		init_timer(pTimer);
		pTimer->data = (unsigned long)data;
		pTimer->function = function;
	}
}

static inline void __RTMP_OS_Add_Timer(struct timer_list * pTimer,
	unsigned long timeout)
{
	if (timer_pending(pTimer))
		return;

	timeout = ((timeout * OS_HZ) / 1000);
	pTimer->expires = jiffies + timeout;
	add_timer(pTimer);
}

static inline void __RTMP_OS_Mod_Timer(struct timer_list * pTimer,
	unsigned long timeout)
{
	timeout = ((timeout * OS_HZ) / 1000);
	mod_timer(pTimer, jiffies + timeout);
}

static inline void __RTMP_OS_Del_Timer(struct timer_list * pTimer,
	BOOLEAN *pCancelled)
{
	if (timer_pending(pTimer))
		*pCancelled = del_timer_sync(pTimer);
	else
		*pCancelled = TRUE;
}


/* Unify all delay routine by using udelay */
void RtmpusecDelay(ULONG usec)
{
	ULONG i;

	for (i = 0; i < (usec / 50); i++)
		udelay(50);

	if (usec % 50)
		udelay(usec % 50);
}


void RtmpOsMsDelay(ULONG msec)
{
	mdelay(msec);
}

void RTMP_GetCurrentSystemTime(LARGE_INTEGER * time)
{
	time->u.LowPart = jiffies;
}

void RTMP_GetCurrentSystemTick(ULONG *pNow)
{
	*pNow = jiffies;
}


/* pAd MUST allow to be NULL */

void *os_alloc_mem(size_t size)
{
	return kmalloc(size, GFP_ATOMIC);
}

NDIS_STATUS os_alloc_mem_suspend(void *pReserved, UCHAR **mem, ULONG size)
{
	*mem = (PUCHAR) kmalloc(size, GFP_KERNEL);
	if (*mem) {
		return NDIS_STATUS_SUCCESS;
	} else
		return NDIS_STATUS_FAILURE;
}

/* pAd MUST allow to be NULL */
void os_free_mem(void *mem)
{
	ASSERT(mem);
	kfree(mem);
}

#if defined(RTMP_RBUS_SUPPORT) || defined(RTMP_FLASH_SUPPORT)
/* The flag "CONFIG_RALINK_FLASH_API" is used for APSoC Linux SDK */

#ifdef CONFIG_RALINK_FLASH_API
int32_t FlashRead(uint32_t *dst, uint32_t *src, uint32_t count);
int32_t FlashWrite(uint16_t *source, uint16_t *destination, uint32_t numBytes);
#else /* CONFIG_RALINK_FLASH_API */

#ifdef RA_MTD_RW_BY_NUM
#if defined(CONFIG_RT2880_FLASH_32M)
#define MTD_NUM_FACTORY 5
#else
#define MTD_NUM_FACTORY 2
#endif
extern int ra_mtd_write(int num, loff_t to, size_t len, const u_char *buf);
extern int ra_mtd_read(int num, loff_t from, size_t len, u_char *buf);
#else
extern int ra_mtd_write_nm(char *name, loff_t to, size_t len, const u_char *buf);
extern int ra_mtd_read_nm(char *name, loff_t from, size_t len, u_char *buf);
#endif

#endif /* CONFIG_RALINK_FLASH_API */

void RtmpFlashRead(UCHAR *p, ULONG a, ULONG b)
{
#ifdef CONFIG_RALINK_FLASH_API
	FlashRead((uint32_t *) p, (uint32_t *) a, (uint32_t) b);
#else
#ifdef RA_MTD_RW_BY_NUM
	ra_mtd_read(MTD_NUM_FACTORY, 0, (size_t) b, p);
#else
	ra_mtd_read_nm("Factory", a&0xFFFF, (size_t) b, p);
#endif
#endif /* CONFIG_RALINK_FLASH_API */
}

void RtmpFlashWrite(UCHAR * p, ULONG a, ULONG b)
{
#ifdef CONFIG_RALINK_FLASH_API
	FlashWrite((uint16_t *) p, (uint16_t *) a, (uint32_t) b);
#else
#ifdef RA_MTD_RW_BY_NUM
	ra_mtd_write(MTD_NUM_FACTORY, 0, (size_t) b, p);
#else
	ra_mtd_write_nm("Factory", a&0xFFFF, (size_t) b, p);
#endif
#endif /* CONFIG_RALINK_FLASH_API */
}
#endif /* defined(RTMP_RBUS_SUPPORT) || defined(RTMP_FLASH_SUPPORT) */


#if 0 //JB removed
static PNDIS_PACKET RtmpOSNetPktAlloc(void *dummy, int size)
{
	struct sk_buff *skb;
	/* Add 2 more bytes for ip header alignment */
	skb = dev_alloc_skb(size + 2);

	return ((PNDIS_PACKET) skb);
}
#endif

PNDIS_PACKET RTMP_AllocateFragPacketBuffer(void *dummy, ULONG len)
{
	struct sk_buff *pkt;

	pkt = dev_alloc_skb(len);

	if (pkt == NULL) {
		DBGPRINT(RT_DEBUG_ERROR,
			 ("can't allocate frag rx %ld size packet\n", len));
	}

	return (PNDIS_PACKET) pkt;
}


/*
	The allocated NDIS PACKET must be freed via dev_kfree_skb_any()
*/
NDIS_STATUS RTMPAllocateNdisPacket(void *pReserved, PNDIS_PACKET *ppPacket,
	UCHAR *pHeader, UINT HeaderLen, UCHAR *pData, UINT DataLen)
{
	struct sk_buff *pPacket;

	ASSERT(pData);
	ASSERT(DataLen);

	/* Add LEN_CCMP_HDR + LEN_CCMP_MIC for PMF */
	pPacket = dev_alloc_skb(HeaderLen + DataLen + RTMP_PKT_TAIL_PADDING + LEN_CCMP_HDR + LEN_CCMP_MIC);
	if (pPacket == NULL) {
		*ppPacket = NULL;
#ifdef DEBUG
		printk(KERN_ERR "RTMPAllocateNdisPacket Fail\n\n");
#endif
		return NDIS_STATUS_FAILURE;
	}

	/* Clone the frame content and update the length of packet */
	if (HeaderLen > 0)
		NdisMoveMemory(pPacket->data, pHeader, HeaderLen);
	if (DataLen > 0)
		NdisMoveMemory(pPacket->data + HeaderLen, pData, DataLen);

	skb_put(pPacket, HeaderLen + DataLen);
	*ppPacket = (PNDIS_PACKET)pPacket;

	return NDIS_STATUS_SUCCESS;
}


void RTMP_QueryPacketInfo(PNDIS_PACKET pPacket, PACKET_INFO *info,
	UCHAR **pSrcBufVA, UINT *pSrcBufLen)
{
	info->BufferCount = 1;
	info->pFirstBuffer = (PNDIS_BUFFER) GET_OS_PKT_DATAPTR(pPacket);
	info->PhysicalBufferCount = 1;
	info->TotalPacketLength = GET_OS_PKT_LEN(pPacket);

	*pSrcBufVA = GET_OS_PKT_DATAPTR(pPacket);
	*pSrcBufLen = GET_OS_PKT_LEN(pPacket);

#ifdef TX_PKT_SG
	if (RTMP_GET_PKT_SG(pPacket)) {
		struct sk_buff *skb = (struct sk_buff *)pPacket;
		int i, nr_frags = skb_shinfo(skb)->nr_frags;

		info->BufferCount =  nr_frags + 1;
		info->PhysicalBufferCount = info->BufferCount;
		info->sg_list[0].data = (void *)GET_OS_PKT_DATAPTR(pPacket);
		info->sg_list[0].len = skb_headlen(skb);
		for (i = 0; i < nr_frags; i++) {
			skb_frag_t *frag = &skb_shinfo(skb)->frags[i];

			info->sg_list[i+1].data = ((void *) page_address(frag->page) +
									frag->page_offset);
			info->sg_list[i+1].len = frag->size;
		}
	}
#endif /* TX_PKT_SG */
}

PNDIS_PACKET DuplicatePacket(PNET_DEV pNetDev, PNDIS_PACKET pPacket,
	UCHAR FromWhichBSSID)
{
	struct sk_buff *skb;
	PNDIS_PACKET pRetPacket = NULL;
	USHORT DataSize;
	UCHAR *pData;

	DataSize = (USHORT) GET_OS_PKT_LEN(pPacket);
	pData = (PUCHAR) GET_OS_PKT_DATAPTR(pPacket);

	skb = skb_clone(RTPKT_TO_OSPKT(pPacket), MEM_ALLOC_FLAG);
	if (skb) {
		skb->dev = pNetDev;
		pRetPacket = OSPKT_TO_RTPKT(skb);
	}

	return pRetPacket;
}


PNDIS_PACKET duplicate_pkt(PNET_DEV pNetDev, PUCHAR pHeader802_3, UINT HdrLen,
	PUCHAR pData, ULONG DataSize, UCHAR FromWhichBSSID)
{
	struct sk_buff *skb;
	PNDIS_PACKET pPacket = NULL;

	if ((skb =
		 __dev_alloc_skb(HdrLen + DataSize + 2, MEM_ALLOC_FLAG)) != NULL) {

		skb_reserve(skb, 2);
		NdisMoveMemory((unsigned char*)skb_tail_pointer(skb), pHeader802_3, HdrLen);
		skb_put(skb, HdrLen);
		NdisMoveMemory((unsigned char*)skb_tail_pointer(skb), pData, DataSize);
		skb_put(skb, DataSize);
		skb->dev = pNetDev;
		pPacket = OSPKT_TO_RTPKT(skb);
	}

	return pPacket;
}


#define TKIP_TX_MIC_SIZE 8
PNDIS_PACKET duplicate_pkt_with_TKIP_MIC(void *pReserved, PNDIS_PACKET pPacket)
{
	struct sk_buff *skb, *newskb;

	skb = RTPKT_TO_OSPKT(pPacket);
	if (skb_tailroom(skb) < TKIP_TX_MIC_SIZE) {
		/* alloc a new skb and copy the packet */
		newskb = skb_copy_expand(skb, skb_headroom(skb), TKIP_TX_MIC_SIZE, GFP_ATOMIC);

		dev_kfree_skb_any(skb);

		if (newskb == NULL) {
			DBGPRINT(RT_DEBUG_ERROR, 
					("Extend Tx.MIC for packet failed!, dropping packet!\n"));
			return NULL;
		}
		skb = newskb;
	}

	return OSPKT_TO_RTPKT(skb);
}

#ifdef CONFIG_AP_SUPPORT
PNDIS_PACKET duplicate_pkt_with_VLAN(PNET_DEV pNetDev, USHORT VLAN_VID,
	USHORT VLAN_Priority, PUCHAR pHeader802_3, UINT HdrLen, PUCHAR pData,
	ULONG DataSize, UCHAR FromWhichBSSID, UCHAR *TPID)
{
	struct sk_buff *skb;
	PNDIS_PACKET pPacket = NULL;
	UINT16 VLAN_Size;

	if ((skb = __dev_alloc_skb(HdrLen + DataSize + LENGTH_802_1Q + 2,
		   MEM_ALLOC_FLAG)) != NULL) {

		skb_reserve(skb, 2);

		/* copy header (maybe +VLAN tag) */
		VLAN_Size = VLAN_8023_Header_Copy(VLAN_VID, VLAN_Priority,
				  pHeader802_3, HdrLen,
				  skb->tail, FromWhichBSSID,
				  TPID);
		skb_put(skb, HdrLen + VLAN_Size);

		/* copy data body */
		NdisMoveMemory(skb->tail, pData, DataSize);
		skb_put(skb, DataSize);
		skb->dev = pNetDev;	/*get_netdev_from_bssid(pAd, FromWhichBSSID); */
		pPacket = OSPKT_TO_RTPKT(skb);
	}

	return pPacket;
}
#endif /* CONFIG_AP_SUPPORT */

/*
	========================================================================

	Routine Description:
		Send a L2 frame to upper daemon to trigger state machine

	Arguments:
		pAd - pointer to our pAdapter context

	Return Value:

	Note:

	========================================================================
*/
BOOLEAN RTMPL2FrameTxAction(void * pCtrlBkPtr, PNET_DEV pNetDev,
	RTMP_CB_8023_PACKET_ANNOUNCE _announce_802_3_packet, UCHAR apidx,
	UCHAR *pData, UINT32 data_len, UCHAR OpMode)
{
	struct sk_buff *skb = dev_alloc_skb(data_len + 2);

	if (!skb) {
		DBGPRINT(RT_DEBUG_ERROR,
				("%s : Error! Can't allocate a skb.\n",
				__FUNCTION__));
		return FALSE;
	}

	SET_OS_PKT_NETDEV(skb, pNetDev);

	/* 16 byte align the IP header */
	skb_reserve(skb, 2);

	/* Insert the frame content */
	NdisMoveMemory(GET_OS_PKT_DATAPTR(skb), pData, data_len);

	/* End this frame */
	skb_put(GET_OS_PKT_TYPE(skb), data_len);

	_announce_802_3_packet(pCtrlBkPtr, skb, OpMode);

	DBGPRINT(RT_DEBUG_TRACE, ("%s done\n", __FUNCTION__));
	return TRUE;
}


PNDIS_PACKET ExpandPacket(void *pReserved, PNDIS_PACKET pPacket,
	UINT32 ext_head_len, UINT32 ext_tail_len)
{
	struct sk_buff *skb, *newskb;

	skb = RTPKT_TO_OSPKT(pPacket);
	if (skb_cloned(skb) || (skb_headroom(skb) < ext_head_len)
		|| (skb_tailroom(skb) < ext_tail_len)) {
		UINT32 head_len =
			(skb_headroom(skb) <
			 ext_head_len) ? ext_head_len : skb_headroom(skb);
		UINT32 tail_len =
			(skb_tailroom(skb) <
			 ext_tail_len) ? ext_tail_len : skb_tailroom(skb);

		/* alloc a new skb and copy the packet */
		newskb = skb_copy_expand(skb, head_len, tail_len, GFP_ATOMIC);

		dev_kfree_skb_any(skb);

		if (newskb == NULL) {
			DBGPRINT(RT_DEBUG_ERROR,
					("Extend Tx buffer for WPI failed!, dropping packet!\n"));
			return NULL;
		}
		skb = newskb;
	}

	return OSPKT_TO_RTPKT(skb);

}

PNDIS_PACKET ClonePacket(void *pReserved, PNDIS_PACKET pPacket,
	PUCHAR pData, ULONG DataSize)
{
	struct sk_buff *pRxPkt;
	struct sk_buff *pClonedPkt;

	ASSERT(pPacket);
	pRxPkt = RTPKT_TO_OSPKT(pPacket);

	/* clone the packet */
	pClonedPkt = skb_clone(pRxPkt, MEM_ALLOC_FLAG);

	if (pClonedPkt) {
		/* set the correct dataptr and data len */
		pClonedPkt->dev = pRxPkt->dev;
		pClonedPkt->data = pData;
		pClonedPkt->len = DataSize;
		skb_set_tail_pointer(pClonedPkt, DataSize);
		ASSERT(DataSize < 1530);
	}
	return pClonedPkt;
}

void RtmpOsPktInit(PNDIS_PACKET pNetPkt, PNET_DEV pNetDev, UCHAR *pData,
	USHORT DataSize)
{
	PNDIS_PACKET pRxPkt = RTPKT_TO_OSPKT(pNetPkt);

	SET_OS_PKT_NETDEV(pRxPkt, pNetDev);
	SET_OS_PKT_DATAPTR(pRxPkt, pData);
	SET_OS_PKT_LEN(pRxPkt, DataSize);
	SET_OS_PKT_DATATAIL(pRxPkt, pData, DataSize);
}


void wlan_802_11_to_802_3_packet(PNET_DEV pNetDev, UCHAR OpMode, USHORT VLAN_VID,
	USHORT VLAN_Priority, PNDIS_PACKET pRxPacket, UCHAR *pData, ULONG DataSize,
	PUCHAR pHeader802_3, UCHAR FromWhichBSSID, UCHAR *TPID)
{
	struct sk_buff *pOSPkt;

	ASSERT(pHeader802_3);

	pOSPkt = RTPKT_TO_OSPKT(pRxPacket);
	pOSPkt->dev = pNetDev;
	pOSPkt->data = pData;
	pOSPkt->len = DataSize;
	skb_set_tail_pointer(pOSPkt, DataSize);

	/* copy 802.3 header */
#ifdef CONFIG_AP_SUPPORT
	RT_CONFIG_IF_OPMODE_ON_AP(OpMode) {
		/* maybe insert VLAN tag to the received packet */
		UCHAR VLAN_Size = 0;
		UCHAR *data_p;

		if (VLAN_VID != 0)
			VLAN_Size = LENGTH_802_1Q;

		data_p = skb_push(pOSPkt, LENGTH_802_3 + VLAN_Size);

		VLAN_8023_Header_Copy(VLAN_VID, VLAN_Priority,
				pHeader802_3, LENGTH_802_3, data_p,
				FromWhichBSSID, TPID);
	}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	RT_CONFIG_IF_OPMODE_ON_STA(OpMode) {
		NdisMoveMemory(skb_push(pOSPkt, LENGTH_802_3), pHeader802_3, LENGTH_802_3);
	}
#endif /* CONFIG_STA_SUPPORT */
}


void hex_dump(char *str, UCHAR *pSrcBufVA, UINT SrcBufLen)
{
#ifdef DBG
	unsigned char *pt;
	int x;

	if (RTDebugLevel < RT_DEBUG_TRACE)
		return;

	pt = pSrcBufVA;
	printk("%s: %p, len = %d\n", str, pSrcBufVA, SrcBufLen);
	for (x = 0; x < SrcBufLen; x++) {
		if (x % 16 == 0)
			printk("0x%04x : ", x);
		printk("%02x ", ((unsigned char)pt[x]));
		if (x % 16 == 15)
			printk("\n");
	}
	printk("\n");
#endif
}

#ifdef SYSTEM_LOG_SUPPORT
/*
	========================================================================

	Routine Description:
		Send log message through wireless event

		Support standard iw_event with IWEVCUSTOM. It is used below.

		iwreq_data.data.flags is used to store event_flag that is
		defined by user. iwreq_data.data.length is the length of the
		event log.

		The format of the event log is composed of the entry's MAC
		address and the desired log message (refer to
		pWirelessEventText).

			ex: 11:22:33:44:55:66 has associated successfully

		p.s. The requirement of Wireless Extension is v15 or newer.

	========================================================================
*/
void RtmpOsSendWirelessEvent(void *pAd, USHORT Event_flag, PUCHAR pAddr,
	UCHAR BssIdx, CHAR Rssi, RTMP_OS_SEND_WLAN_EVENT pFunc)
{
#if WIRELESS_EXT >= 15
	pFunc(pAd, Event_flag, pAddr, BssIdx, Rssi);
#else
	DBGPRINT(RT_DEBUG_ERROR,
			("%s : The Wireless Extension MUST be v15 or newer.\n",
			__FUNCTION__));
#endif /* WIRELESS_EXT >= 15 */
}
#endif /* SYSTEM_LOG_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
void SendSignalToDaemon(int sig, RTMP_OS_PID pid, unsigned long pid_no)
{
}
#endif /* CONFIG_AP_SUPPORT */


/*******************************************************************************

	File open/close related functions.

 *******************************************************************************/
RTMP_OS_FD RtmpOSFileOpen(char *pPath, int flag, int mode)
{
	struct file *filePtr;
	mm_segment_t oldfs;

	if (flag == RTMP_FILE_RDONLY)
		flag = O_RDONLY;
	else if (flag == RTMP_FILE_WRONLY)
		flag = O_WRONLY;
	else if (flag == RTMP_FILE_CREAT)
		flag = O_CREAT;
	else if (flag == RTMP_FILE_TRUNC)
		flag = O_TRUNC;

	oldfs = get_fs();
	set_fs(get_ds());
	filePtr = filp_open(pPath, flag, 0);
	set_fs(oldfs);

	if (IS_ERR(filePtr)) {
		DBGPRINT(RT_DEBUG_ERROR,
			 ("%s(): Error %ld opening %s\n", __FUNCTION__,
			  -PTR_ERR(filePtr), pPath));
	}

	return (RTMP_OS_FD) filePtr;
}


int RtmpOSFileClose(RTMP_OS_FD osfd)
{
	filp_close(osfd, NULL);
	return 0;
}


void RtmpOSFileSeek(RTMP_OS_FD osfd, int offset)
{
	osfd->f_pos = offset;
}


int RtmpOSFileRead(RTMP_OS_FD osfd, char *pDataPtr, int readLen)
{
	int ret;
	mm_segment_t fs = get_fs();

	set_fs(get_ds());
	ret = vfs_read(osfd, pDataPtr, readLen, &osfd->f_pos);
	set_fs(fs);
	return ret;
}


int RtmpOSFileWrite(RTMP_OS_FD osfd, char *pDataPtr, int writeLen)
{
	int ret;
	mm_segment_t oldfs = get_fs();

	set_fs(get_ds());
	ret = vfs_write(osfd, pDataPtr, writeLen, &osfd->f_pos);
	set_fs(oldfs);
	return ret;
}


static inline void __RtmpOSFSInfoChange(OS_FS_INFO * pOSFSInfo, BOOLEAN bSet)
{
	if (bSet) {
		/* Save uid and gid used for filesystem access. */
		/* Set user and group to 0 (root) */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,29)
		pOSFSInfo->fsuid = current->fsuid;
		pOSFSInfo->fsgid = current->fsgid;
		current->fsuid = current->fsgid = 0;
#else
		pOSFSInfo->fsuid = current_fsuid();
		pOSFSInfo->fsgid = current_fsgid();
#endif
		pOSFSInfo->fs = get_fs();
		set_fs(KERNEL_DS);
	} else {
		set_fs(pOSFSInfo->fs);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,29)
		current->fsuid = pOSFSInfo->fsuid;
		current->fsgid = pOSFSInfo->fsgid;
#endif
	}
}


/*******************************************************************************

	Task create/management/kill related functions.

 *******************************************************************************/
static inline NDIS_STATUS __RtmpOSTaskKill(OS_TASK *pTask)
{
	int ret = NDIS_STATUS_FAILURE;

#ifdef KTHREAD_SUPPORT
	if (pTask->kthread_task) {
		kthread_stop(pTask->kthread_task);
		ret = NDIS_STATUS_SUCCESS;
	}
#else
	CHECK_PID_LEGALITY(pTask->taskPID) {
		DBGPRINT(RT_DEBUG_TRACE,
				("Terminate the task(%s) with pid(%d)!\n",
				pTask->taskName, GET_PID_NUMBER(pTask->taskPID)));
		mb();
		pTask->task_killed = 1;
		mb();
		ret = KILL_THREAD_PID(pTask->taskPID, SIGTERM, 1);
		if (ret) {
			printk(KERN_WARNING
					"kill task(%s) with pid(%d) failed(retVal=%d)!\n",
					pTask->taskName, GET_PID_NUMBER(pTask->taskPID),
					ret);
		} else {
			wait_for_completion(&pTask->taskComplete);
			pTask->taskPID = THREAD_PID_INIT_VALUE;
			pTask->task_killed = 0;
			RTMP_SEM_EVENT_DESTORY(&pTask->taskSema);
			ret = NDIS_STATUS_SUCCESS;
		}
	}
#endif

	return ret;
}


static inline int __RtmpOSTaskNotifyToExit(OS_TASK *pTask)
{
#ifndef KTHREAD_SUPPORT
	pTask->taskPID = THREAD_PID_INIT_VALUE;
	complete_and_exit(&pTask->taskComplete, 0);
#endif

	return 0;
}


static inline void __RtmpOSTaskCustomize(OS_TASK *pTask)
{
#ifndef KTHREAD_SUPPORT

	daemonize((PSTRING) &pTask->taskName[0] /*"%s",pAd->net_dev->name */ );

	allow_signal(SIGTERM);
	allow_signal(SIGKILL);
	current->flags |= PF_NOFREEZE;

	RTMP_GET_OS_PID(pTask->taskPID, current->pid);

	/* signal that we've started the thread */
	complete(&pTask->taskComplete);

#endif
}


static inline NDIS_STATUS __RtmpOSTaskAttach(OS_TASK *pTask,
	RTMP_OS_TASK_CALLBACK fn, ULONG arg)
{
	NDIS_STATUS status = NDIS_STATUS_SUCCESS;
#ifndef KTHREAD_SUPPORT
	pid_t pid_number = -1;
#endif

#ifdef KTHREAD_SUPPORT
	pTask->task_killed = 0;
	pTask->kthread_task = NULL;
	pTask->kthread_task =
		kthread_run((cast_fn) fn, (void *)arg, pTask->taskName);
	if (IS_ERR(pTask->kthread_task))
		status = NDIS_STATUS_FAILURE;
#else
	pid_number =
		kernel_thread((cast_fn) fn, (void *)arg, RTMP_OS_MGMT_TASK_FLAGS);
	if (pid_number < 0) {
		DBGPRINT(RT_DEBUG_ERROR,
			 ("Attach task(%s) failed!\n", pTask->taskName));
		status = NDIS_STATUS_FAILURE;
	} else {
		/* Wait for the thread to start */
		wait_for_completion(&pTask->taskComplete);
		status = NDIS_STATUS_SUCCESS;
	}
#endif
	return status;
}


static inline NDIS_STATUS __RtmpOSTaskInit(OS_TASK *pTask, PSTRING pTaskName,
	void *pPriv, LIST_HEADER *pSemList)
{
	int len;

	ASSERT(pTask);

#ifndef KTHREAD_SUPPORT
	NdisZeroMemory((PUCHAR) (pTask), sizeof (OS_TASK));
#endif

	len = strlen(pTaskName);
	len = len > (RTMP_OS_TASK_NAME_LEN - 1) ? (RTMP_OS_TASK_NAME_LEN - 1) : len;
	NdisMoveMemory(&pTask->taskName[0], pTaskName, len);
	pTask->priv = pPriv;

#ifndef KTHREAD_SUPPORT
	RTMP_SEM_EVENT_INIT_LOCKED(&(pTask->taskSema), pSemList);
	pTask->taskPID = THREAD_PID_INIT_VALUE;
	init_completion(&pTask->taskComplete);
#endif

#ifdef KTHREAD_SUPPORT
	init_waitqueue_head(&(pTask->kthread_q));
#endif

	return NDIS_STATUS_SUCCESS;
}


BOOLEAN __RtmpOSTaskWait(void *pReserved, OS_TASK *pTask, INT32 *pStatus)
{
#ifdef KTHREAD_SUPPORT
	RTMP_WAIT_EVENT_INTERRUPTIBLE((*pStatus), pTask);

	if ((pTask->task_killed == 1) || ((*pStatus) != 0))
		return FALSE;
#else
	RTMP_SEM_EVENT_WAIT(&(pTask->taskSema), (*pStatus));

	/* unlock the device pointers */
	if ((*pStatus) != 0) {
/*		RTMP_SET_FLAG_(*pFlags, fRTMP_ADAPTER_HALT_IN_PROGRESS); */
		return FALSE;
	}
#endif /* KTHREAD_SUPPORT */

	return TRUE;
}

static UINT32 RtmpOSWirelessEventTranslate(IN UINT32 eventType)
{
	switch (eventType) {
	case RT_WLAN_EVENT_CUSTOM:
		eventType = IWEVCUSTOM;
		break;

	case RT_WLAN_EVENT_CGIWAP:
		eventType = SIOCGIWAP;
		break;

#if WIRELESS_EXT > 17
	case RT_WLAN_EVENT_ASSOC_REQ_IE:
		eventType = IWEVASSOCREQIE;
		break;
#endif /* WIRELESS_EXT */

#if WIRELESS_EXT >= 14
	case RT_WLAN_EVENT_SCAN:
		eventType = SIOCGIWSCAN;
		break;
#endif /* WIRELESS_EXT */

	case RT_WLAN_EVENT_EXPIRED:
		eventType = IWEVEXPIRED;
		break;

	default:
		printk("Unknown event: 0x%x\n", eventType);
		break;
	}

	return eventType;
}


int RtmpOSWrielessEventSend(PNET_DEV pNetDev, UINT32 eventType, int flags,
	PUCHAR pSrcMac, PUCHAR pData, UINT32 dataLen)
{
	union iwreq_data wrqu;

	/* translate event type */
	eventType = RtmpOSWirelessEventTranslate(eventType);

	memset(&wrqu, 0, sizeof (wrqu));

	if (flags > -1)
		wrqu.data.flags = flags;

	if (pSrcMac)
		memcpy(wrqu.ap_addr.sa_data, pSrcMac, MAC_ADDR_LEN);

	if ((pData != NULL) && (dataLen > 0))
		wrqu.data.length = dataLen;
	else
		wrqu.data.length = 0;

	wireless_send_event(pNetDev, eventType, &wrqu, (char *)pData);
	return 0;
}


int RtmpOSWrielessEventSendExt(PNET_DEV pNetDev, UINT32 eventType, int flags,
	PUCHAR pSrcMac, PUCHAR pData, UINT32 dataLen, UINT32 family)
{
	union iwreq_data wrqu;

	/* translate event type */
	eventType = RtmpOSWirelessEventTranslate(eventType);

	/* translate event type */
	memset(&wrqu, 0, sizeof (wrqu));

	if (flags > -1)
		wrqu.data.flags = flags;

	if (pSrcMac)
		memcpy(wrqu.ap_addr.sa_data, pSrcMac, MAC_ADDR_LEN);

	if ((pData != NULL) && (dataLen > 0))
		wrqu.data.length = dataLen;

	wrqu.addr.sa_family = family;

	wireless_send_event(pNetDev, eventType, &wrqu, (char *)pData);
	return 0;
}


/*
========================================================================
Routine Description:
	Check if the network interface is up.

Arguments:
	*pDev		- Network Interface

Return Value:
	None

Note:
========================================================================
*/
BOOLEAN RtmpOSNetDevIsUp(void *pDev)
{
	struct net_device *pNetDev = (struct net_device *)pDev;

	if ((pNetDev == NULL) || !(pNetDev->flags & IFF_UP))
		return FALSE;

	return TRUE;
}


/*
========================================================================
Routine Description:
	Assign sys_handle data pointer (pAd) to the priv info structured linked to
	the OS network interface.

Arguments:
	pDev		- the os net device data structure
	pPriv		- the sys_handle want to assigned

Return Value:
	None

Note:
========================================================================
*/
void RtmpOsSetNetDevPriv(void *pDev, void *pPriv)
{
	struct mt_dev_priv *priv_info = NULL;

	priv_info = (struct mt_dev_priv *)netdev_priv((struct net_device *)pDev);
	priv_info->sys_handle = (void *)pPriv;
	priv_info->priv_flags = 0;
}


/*
========================================================================
Routine Description:
	Get wifi_dev from the priv info linked to OS network interface data structure.

Arguments:
	pDev		- the device

Return Value:
	sys_handle

Note:
========================================================================
*/
void *RtmpOsGetNetDevPriv(void *pDev)
{
	return ((struct mt_dev_priv *)netdev_priv((struct net_device *)pDev))->sys_handle;
}


void RtmpOsSetNetDevWdev(void *net_dev, void *wdev)
{
	struct mt_dev_priv *priv_info;

	priv_info = (struct mt_dev_priv *)netdev_priv((struct net_device *)net_dev);
	priv_info->wifi_dev = wdev;
}


void *RtmpOsGetNetDevWdev(void *pDev)
{
	return ((struct mt_dev_priv *)netdev_priv((struct net_device *)pDev))->wifi_dev;
}


/*
========================================================================
Routine Description:
	Get private flags from the network interface.

Arguments:
	pDev		- the device

Return Value:
	pPriv		- the pointer

Note:
========================================================================
*/
USHORT RtmpDevPrivFlagsGet(void *pDev)
{
	return ((struct mt_dev_priv *)netdev_priv((struct net_device *)pDev))->priv_flags;
}


/*
========================================================================
Routine Description:
	Get private flags from the network interface.

Arguments:
	pDev		- the device

Return Value:
	pPriv		- the pointer

Note:
========================================================================
*/
void RtmpDevPrivFlagsSet(void *pDev, USHORT PrivFlags)
{
	struct mt_dev_priv *priv_info;

	priv_info = (struct mt_dev_priv *)netdev_priv((struct net_device *)pDev);
	priv_info->priv_flags = PrivFlags;
}

UCHAR get_sniffer_mode(void *pDev)
{
	struct mt_dev_priv *priv_info;

	priv_info = (struct mt_dev_priv *)netdev_priv((struct net_device *)pDev);
	return priv_info->sniffer_mode;
}

#if 0 //JB removed
static void set_sniffer_mode(void *net_dev, UCHAR sniffer_mode)
{
	struct mt_dev_priv *priv_info;

	priv_info = (struct mt_dev_priv *)netdev_priv((struct net_device *)net_dev);
	priv_info->sniffer_mode = sniffer_mode;
}
#endif //0

/*
========================================================================
Routine Description:
	Get network interface name.

Arguments:
	pDev		- the device

Return Value:
	the name

Note:
========================================================================
*/
char *RtmpOsGetNetDevName(void *pDev)
{
	return ((PNET_DEV) pDev)->name;
}


UINT32 RtmpOsGetNetIfIndex(IN void *pDev)
{
	return ((PNET_DEV) pDev)->ifindex;
}


int RtmpOSNetDevAddrSet(UCHAR OpMode, PNET_DEV pNetDev, PUCHAR pMacAddr,
	PUCHAR dev_name)
{
	struct net_device *net_dev = (struct net_device *)pNetDev;

#ifdef CONFIG_STA_SUPPORT
	/* work-around for the SuSE due to it has it's own interface name management system. */
	RT_CONFIG_IF_OPMODE_ON_STA(OpMode) {
		if (dev_name != NULL) {
			NdisZeroMemory(dev_name, 16);
			NdisMoveMemory(dev_name, net_dev->name, strlen(net_dev->name));
		}
	}
#endif /* CONFIG_STA_SUPPORT */

	NdisMoveMemory(net_dev->dev_addr, pMacAddr, 6);

	return 0;
}

static PNET_DEV RtmpOSNetDevGetByName(PNET_DEV pNetDev, PSTRING pDevName)
{
	PNET_DEV pTargetNetDev = NULL;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)
	pTargetNetDev = dev_get_by_name(dev_net(pNetDev), pDevName);
#else
	ASSERT(pNetDev);
	pTargetNetDev = dev_get_by_name(pNetDev->nd_net, pDevName);
#endif
#else
	pTargetNetDev = dev_get_by_name(pDevName);
#endif /* KERNEL_VERSION(2,6,24) */

	return pTargetNetDev;
}

/*
  *	Assign the network dev name for created Ralink WiFi interface.
  */
static int RtmpOSNetDevRequestName(INT32 MC_RowID, UINT32 *pIoctlIF,
	PNET_DEV dev, PSTRING pPrefixStr, int devIdx)
{
	PNET_DEV existNetDev;
	STRING suffixName[IFNAMSIZ];
	STRING desiredName[IFNAMSIZ];
	int ifNameIdx, prefixLen, slotNameLen;
	int Status;

	prefixLen = strlen(pPrefixStr);
	ASSERT((prefixLen < IFNAMSIZ));

	for (ifNameIdx = devIdx; ifNameIdx < 32; ifNameIdx++) {
		memset(suffixName, 0, IFNAMSIZ);
		memset(desiredName, 0, IFNAMSIZ);
		strncpy(&desiredName[0], pPrefixStr, prefixLen);

#ifdef MULTIPLE_CARD_SUPPORT
#ifdef RT_SOC_SUPPORT
		if (MC_RowID > 0)
			sprintf(suffixName, "i%d", ifNameIdx);
		else
#else
		if (MC_RowID >= 0)
			sprintf(suffixName, "%02d_%d", MC_RowID, ifNameIdx);
		else
#endif /* RT_SOC_SUPPORT */
#endif /* MULTIPLE_CARD_SUPPORT */
			sprintf(suffixName, "%d", ifNameIdx);

		slotNameLen = strlen(suffixName);
		ASSERT(((slotNameLen + prefixLen) < IFNAMSIZ));
		strcat(desiredName, suffixName);

		existNetDev = RtmpOSNetDevGetByName(dev, &desiredName[0]);
		if (existNetDev == NULL)
			break;
		else
			RtmpOSNetDeviceRefPut(existNetDev);
	}

	if (ifNameIdx < 32) {
#ifdef HOSTAPD_SUPPORT
		*pIoctlIF = ifNameIdx;
#endif /*HOSTAPD_SUPPORT */
		strcpy(&dev->name[0], &desiredName[0]);
		Status = NDIS_STATUS_SUCCESS;
	} else {
		DBGPRINT(RT_DEBUG_ERROR,
				("Cannot request DevName with preifx(%s) and in range(0~32) as suffix from OS!\n",
				pPrefixStr));
		Status = NDIS_STATUS_FAILURE;
	}

	return Status;
}

#if 0 //JB removed
static void RtmpOSNetDevClose(PNET_DEV pNetDev)
{
	dev_close(pNetDev);
}
#endif

void RtmpOSNetDevFree(PNET_DEV pNetDev)
{
	ASSERT(pNetDev);

	free_netdev(pNetDev);
	DBGPRINT(RT_DEBUG_TRACE, ("%s: freed %p\n", __FUNCTION__, pNetDev));

}

static int RtmpOSNetDevAlloc(PNET_DEV *new_dev_p, UINT32 privDataSize)
{
	*new_dev_p = NULL;

	DBGPRINT(RT_DEBUG_TRACE,
			("Allocate a net device with private data size=%d\n",
			privDataSize));

	*new_dev_p = alloc_etherdev(privDataSize);

	if (*new_dev_p)
		return NDIS_STATUS_SUCCESS;
	else
		return NDIS_STATUS_FAILURE;
}


#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,31)
static int RtmpOSNetDevOpsAlloc(PVOID *pNetDevOps)
{
	*pNetDevOps = (PVOID) vmalloc(sizeof (struct net_device_ops));
	if (*pNetDevOps) {
		NdisZeroMemory(*pNetDevOps, sizeof (struct net_device_ops));
		return NDIS_STATUS_SUCCESS;
	} else {
		return NDIS_STATUS_FAILURE;
	}
}
#endif


static void RtmpOSNetDeviceRefPut(PNET_DEV pNetDev)
{
	/*
	   every time dev_get_by_name is called, and it has returned a valid struct
	   net_device*, dev_put should be called afterwards, because otherwise the
	   machine hangs when the device is unregistered (since dev->refcnt > 1).
	 */
	if (pNetDev)
		dev_put(pNetDev);
}

#if 0 //JB removed
static int RtmpOSNetDevDestroy(void *pReserved, PNET_DEV pNetDev)
{
	/* TODO: Need to fix this */
	printk("WARNING: This function(%s) not implement yet!!!\n",
			__FUNCTION__);
	return 0;
}
#endif


void RtmpOSNetDevDetach(PNET_DEV pNetDev)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,31)
	struct net_device_ops *pNetDevOps = (struct net_device_ops *)pNetDev->netdev_ops;
#endif
	DBGPRINT(RT_DEBUG_WARN, ("--> RtmpOSNetDevDetach\n"));

	unregister_netdev(pNetDev);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,31)
	vfree(pNetDevOps);
#endif
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
static void RALINK_ET_DrvInfoGet(struct net_device *pDev,
	struct ethtool_drvinfo *pInfo)
{
	strcpy(pInfo->driver, "RALINK WLAN");
	sprintf(pInfo->bus_info, "CSR 0x%lx", pDev->base_addr);
}

static struct ethtool_ops RALINK_Ethtool_Ops = {
	.get_drvinfo = RALINK_ET_DrvInfoGet,
};
#endif

#define ASSERT_RTNL_UNLOCKED() do { \
       if (unlikely(rtnl_is_locked())) { \
               printk(KERN_ERR "RTNL: assertion failed at %s (%d)\n", \
                       __FILE__,  __LINE__); \
                dump_stack(); \
        } \
} while(0)

int RtmpOSNetDevAttach(UCHAR OpMode, PNET_DEV pNetDev,
	RTMP_OS_NETDEV_OP_HOOK *pDevOpHook)
{
	int ret;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,31)
	struct net_device_ops *pNetDevOps = (struct net_device_ops *)pNetDev->netdev_ops;
#endif

	DBGPRINT(RT_DEBUG_WARN, ("RtmpOSNetDevAttach()--->\n"));

	/* If we need hook some callback function to the net device structrue,
	 * do it now. */
	if (pDevOpHook) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,31)
		pNetDevOps->ndo_set_mac_address = eth_mac_addr;
		pNetDevOps->ndo_open = pDevOpHook->open;
		pNetDevOps->ndo_stop = pDevOpHook->stop;
		pNetDevOps->ndo_start_xmit =
				(HARD_START_XMIT_FUNC) (pDevOpHook->xmit);
		pNetDevOps->ndo_do_ioctl = pDevOpHook->ioctl;
#else
		pNetDev->open = pDevOpHook->open;
		pNetDev->stop = pDevOpHook->stop;
		pNetDev->hard_start_xmit =
				(HARD_START_XMIT_FUNC) (pDevOpHook->xmit);
		pNetDev->do_ioctl = pDevOpHook->ioctl;
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
		pNetDev->ethtool_ops = &RALINK_Ethtool_Ops;
#endif
		/* if you don't implement get_stats, just leave the callback
		 * function as NULL, a dummy function will make kernel panic.
		 */
		if (pDevOpHook->get_stats)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,31)
			pNetDevOps->ndo_get_stats = pDevOpHook->get_stats;
#else
			pNetDev->get_stats = pDevOpHook->get_stats;
#endif

		/* OS specific flags, here we used to indicate if we are
		 * virtual interface */
/*		pNetDev->priv_flags = pDevOpHook->priv_flags; */
		RtmpDevPrivFlagsSet(pNetDev, pDevOpHook->priv_flags);

#if (WIRELESS_EXT < 21) && (WIRELESS_EXT >= 12)
/*		pNetDev->get_wireless_stats = rt28xx_get_wireless_stats; */
		pNetDev->get_wireless_stats = pDevOpHook->get_wstats;
#endif

#ifdef CONFIG_STA_SUPPORT
#if WIRELESS_EXT >= 12
		if (OpMode == OPMODE_STA) {
/*			pNetDev->wireless_handlers = &rt28xx_iw_handler_def; */
			pNetDev->wireless_handlers = pDevOpHook->iw_handler;
		}
#endif /*WIRELESS_EXT >= 12 */
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_APSTA_MIXED_SUPPORT
#if WIRELESS_EXT >= 12
		if (OpMode == OPMODE_AP) {
/*			pNetDev->wireless_handlers = &rt28xx_ap_iw_handler_def; */
			pNetDev->wireless_handlers = pDevOpHook->iw_handler;
		}
#endif /*WIRELESS_EXT >= 12 */
#endif /* CONFIG_APSTA_MIXED_SUPPORT */

		/* copy the net device mac address to the net_device structure. */
		NdisMoveMemory(pNetDev->dev_addr, &pDevOpHook->devAddr[0],
				   MAC_ADDR_LEN);
		// rtnl_locked = pDevOpHook->needProtcted;
	}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,31)
	pNetDevOps->ndo_validate_addr = NULL;
	/*pNetDev->netdev_ops = ops; */
#else
	pNetDev->validate_addr = NULL;
#endif
#endif
	ASSERT_RTNL_UNLOCKED();
	ret = register_netdev(pNetDev);
	netif_stop_queue(pNetDev);

	DBGPRINT(RT_DEBUG_WARN, ("<---RtmpOSNetDevAttach(), ret=%d\n", ret));
	if (ret == 0)
		return NDIS_STATUS_SUCCESS;
	else
		return NDIS_STATUS_FAILURE;
}


PNET_DEV RtmpOSNetDevCreate(INT32 MC_RowID, UINT32 *pIoctlIF, int devType,
	int devNum, int privMemSize, PSTRING pNamePrefix)
{
	struct net_device *pNetDev = NULL;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,31)
	struct net_device_ops *pNetDevOps = NULL;
#endif
	int status;

	/* allocate a new network device */
	status = RtmpOSNetDevAlloc(&pNetDev, privMemSize);
	if (status != NDIS_STATUS_SUCCESS) {
		DBGPRINT(RT_DEBUG_ERROR,
				("Allocate network device fail (%s)...\n",
				pNamePrefix));
		return NULL;
	}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,31)
	status = RtmpOSNetDevOpsAlloc((PVOID) &pNetDevOps);
	if (status != NDIS_STATUS_SUCCESS) {
		DBGPRINT(RT_DEBUG_TRACE, ("Allocate net device ops fail!\n"));
		RtmpOSNetDevFree(pNetDev);
		return NULL;
	} else {
		DBGPRINT(RT_DEBUG_TRACE, ("Allocate net device ops success!\n"));
		pNetDev->netdev_ops = pNetDevOps;
	}
#endif
	/* find a available interface name, max 32 interfaces */
	status = RtmpOSNetDevRequestName(MC_RowID, pIoctlIF, pNetDev, pNamePrefix, devNum);
	if (status != NDIS_STATUS_SUCCESS) {
		/* error! no any available ra name can be used! */
		DBGPRINT(RT_DEBUG_ERROR,
				("Assign inf name (%s with suffix 0~32) failed\n",
				pNamePrefix));
		RtmpOSNetDevFree(pNetDev);
		return NULL;
	} else {
		DBGPRINT(RT_DEBUG_TRACE,
				("%s: The name of the new %s interface is %s\n",
				__FUNCTION__, pNamePrefix, pNetDev->name));
	}

	return pNetDev;
}


#ifdef CONFIG_AP_SUPPORT
UCHAR VLAN_8023_Header_Copy(USHORT VLAN_VID, USHORT VLAN_Priority, PUCHAR pHeader802_3,
	UINT HdrLen, PUCHAR pData, UCHAR FromWhichBSSID, UCHAR *TPID)
{
	UINT16 TCI;
	UCHAR VLAN_Size = 0;

	if (VLAN_VID != 0) {
		/* need to insert VLAN tag */
		VLAN_Size = LENGTH_802_1Q;

		/* make up TCI field */
		TCI = (VLAN_VID & 0x0fff) | ((VLAN_Priority & 0x7) << 13);

#ifndef RT_BIG_ENDIAN
		TCI = SWAP16(TCI);
#endif
		/* copy dst + src MAC (12B) */
		memcpy(pData, pHeader802_3, LENGTH_802_3_NO_TYPE);

		/* copy VLAN tag (4B) */
		/* do NOT use memcpy to speed up */
		*(UINT16 *) (pData + LENGTH_802_3_NO_TYPE) = *(UINT16 *) TPID;
		*(UINT16 *) (pData + LENGTH_802_3_NO_TYPE + 2) = TCI;

		/* copy type/len (2B) */
		*(UINT16 *) (pData + LENGTH_802_3_NO_TYPE + LENGTH_802_1Q) =
			*(UINT16 *) & pHeader802_3[LENGTH_802_3 -
						   LENGTH_802_3_TYPE];

		/* copy tail if exist */
		if (HdrLen > LENGTH_802_3)
			memcpy(pData + LENGTH_802_3 + LENGTH_802_1Q, pHeader802_3 
					+ LENGTH_802_3, HdrLen - LENGTH_802_3);
	} else {
		/* no VLAN tag is needed to insert */
		memcpy(pData, pHeader802_3, HdrLen);
	}

	return VLAN_Size;
}
#endif /* CONFIG_AP_SUPPORT */


/*
========================================================================
Routine Description:
	Allocate memory for adapter control block.

Arguments:
	pAd		Pointer to our adapter

Return Value:
	NDIS_STATUS_SUCCESS
	NDIS_STATUS_FAILURE
	NDIS_STATUS_RESOURCES

Note:
========================================================================
*/
//NDIS_STATUS AdapterBlockAllocateMemory(void *handle, void **ppAd, UINT32 SizeOfpAd)
//{
//	*ppAd = (PVOID) vmalloc(SizeOfpAd);
//	if (*ppAd) {
//		NdisZeroMemory(*ppAd, SizeOfpAd);
//		return NDIS_STATUS_SUCCESS;
//	} else
//		return NDIS_STATUS_FAILURE;
//}


/* ========================================================================== */

UINT RtmpOsWirelessExtVerGet(void)
{
	return WIRELESS_EXT;
}


void RtmpDrvAllMacPrint(void *pReserved, UINT32 *pBufMac, UINT32 AddrStart,
	UINT32 AddrEnd, UINT32 AddrStep)
{
	struct file *file_w;
	PSTRING fileName = "MacDump.txt";
	mm_segment_t orig_fs;
	STRING *msg;
	UINT32 macAddr = 0, macValue = 0;

	msg = os_alloc_mem(1024);
	if (!msg)
		return;

	orig_fs = get_fs();
	set_fs(KERNEL_DS);

	/* open file */
	file_w = filp_open(fileName, O_WRONLY | O_CREAT, 0);
	if (IS_ERR(file_w)) {
		DBGPRINT(RT_DEBUG_TRACE,
				("-->2) %s: Error %ld opening %s\n", __FUNCTION__,
				-PTR_ERR(file_w), fileName));
	} else {
		if (file_w->f_op && file_w->f_op->write) {
			file_w->f_pos = 0;
			macAddr = AddrStart;

			while (macAddr <= AddrEnd) {
/*				RTMP_IO_READ32(pAd, macAddr, &macValue); // sample */
				macValue = *pBufMac++;
				sprintf(msg, "%04x = %08x\n", macAddr, macValue);

				/* write data to file */
				file_w->f_op->write(file_w, msg, strlen(msg), &file_w->f_pos);

				printk("%s", msg);
				macAddr += AddrStep;
			}
			sprintf(msg, "\nDump all MAC values to %s\n", fileName);
		}
		filp_close(file_w, NULL);
	}
	set_fs(orig_fs);
	os_free_mem(msg);
}


void RtmpDrvAllE2PPrint(void *pReserved, USHORT *pMacContent, UINT32 AddrEnd,
	UINT32 AddrStep)
{
	struct file *file_w;
	PSTRING fileName = "EEPROMDump.txt";
	mm_segment_t orig_fs;
	STRING *msg;
	USHORT eepAddr = 0;
	USHORT eepValue;

	msg = os_alloc_mem(1024);
	if (!msg)
		return;

	orig_fs = get_fs();
	set_fs(KERNEL_DS);

	/* open file */
	file_w = filp_open(fileName, O_WRONLY | O_CREAT, 0);
	if (IS_ERR(file_w)) {
		DBGPRINT(RT_DEBUG_TRACE,
			 ("-->2) %s: Error %ld opening %s\n", __FUNCTION__,
			  -PTR_ERR(file_w), fileName));
	} else {
		if (file_w->f_op && file_w->f_op->write) {
			file_w->f_pos = 0;
			eepAddr = 0x00;

			while (eepAddr <= AddrEnd) {
				eepValue = *pMacContent;
				sprintf(msg, "%08x = %04x\n", eepAddr, eepValue);

				/* write data to file */
				file_w->f_op->write(file_w, msg, strlen(msg), &file_w->f_pos);

				printk("%s", msg);
				eepAddr += AddrStep;
				pMacContent += (AddrStep >> 1);
			}
			sprintf(msg, "\nDump all EEPROM values to %s\n",
				fileName);
		}
		filp_close(file_w, NULL);
	}
	set_fs(orig_fs);
	os_free_mem(msg);
}


void RtmpDrvAllRFPrint(void *pReserved, UCHAR *pBuf, UINT32 BufLen)
{
	struct file *file_w;
	PSTRING fileName = "RFDump.txt";
	mm_segment_t orig_fs;

	orig_fs = get_fs();
	set_fs(KERNEL_DS);

	/* open file */
	file_w = filp_open(fileName, O_WRONLY | O_CREAT, 0);
	if (IS_ERR(file_w)) {
		DBGPRINT(RT_DEBUG_TRACE,
			 ("-->2) %s: Error %ld opening %s\n", __FUNCTION__,
			  -PTR_ERR(file_w), fileName));
	} else {
		if (file_w->f_op && file_w->f_op->write) {
			file_w->f_pos = 0;
			/* write data to file */
			file_w->f_op->write(file_w, pBuf, BufLen, &file_w->f_pos);
		}
		filp_close(file_w, NULL);
	}
	set_fs(orig_fs);
}


/*
========================================================================
Routine Description:
	Wake up the command thread.

Arguments:
	pAd		- WLAN control block pointer

Return Value:
	None

Note:
========================================================================
*/
void RtmpOsCmdUp(RTMP_OS_TASK *pCmdQTask)
{
	OS_TASK *pTask = RTMP_OS_TASK_GET(pCmdQTask);
#ifdef KTHREAD_SUPPORT
	pTask->kthread_running = TRUE;
	wake_up(&pTask->kthread_q);
#else
	CHECK_PID_LEGALITY(pTask->taskPID) {
		RTMP_SEM_EVENT_UP(&(pTask->taskSema));
	}
#endif /* KTHREAD_SUPPORT */
}


/*
========================================================================
Routine Description:
	Wake up USB Mlme thread.

Arguments:
	pAd		- WLAN control block pointer

Return Value:
	None

Note:
========================================================================
*/
void RtmpOsMlmeUp(IN RTMP_OS_TASK *pMlmeQTask)
{
	OS_TASK *pTask = RTMP_OS_TASK_GET(pMlmeQTask);

#ifdef KTHREAD_SUPPORT
	if ((pTask != NULL) && (pTask->kthread_task)) {
		pTask->kthread_running = TRUE;
		wake_up(&pTask->kthread_q);
	}
#else
	if (pTask != NULL) {
		CHECK_PID_LEGALITY(pTask->taskPID) {
			RTMP_SEM_EVENT_UP(&(pTask->taskSema));
		}
	}
#endif /* KTHREAD_SUPPORT */
}


/*
========================================================================
Routine Description:
	Check if the file is error.

Arguments:
	pFile		- the file

Return Value:
	OK or any error

Note:
	rt_linux.h, not rt_drv.h
========================================================================
*/
//static INT32 RtmpOsFileIsErr(IN void *pFile)
//{
//	return IS_FILE_OPEN_ERR(pFile);
//}


/*
========================================================================
Routine Description:
	Enable or disable wireless event sent.

Arguments:
	pReserved	- Reserved
	FlgIsWEntSup	- TRUE or FALSE

Return Value:
	None

Note:
========================================================================
*/
void RtmpOsWlanEventSet(void *pReserved, BOOLEAN *pCfgWEnt, BOOLEAN FlgIsWEntSup)
{
#if WIRELESS_EXT >= 15
/*	pAd->CommonCfg.bWirelessEvent = FlgIsWEntSup; */
	*pCfgWEnt = FlgIsWEntSup;
#else
	*pCfgWEnt = 0;		/* disable */
#endif
}

/*
========================================================================
Routine Description:
	vmalloc

Arguments:
	Size		- memory size

Return Value:
	the memory

Note:
========================================================================
*/
//void *RtmpOsVmalloc(ULONG Size)
//{
//	return vmalloc(Size);
//}

/*
========================================================================
Routine Description:
	vfree

Arguments:
	pMem		- the memory

Return Value:
	None

Note:
========================================================================
*/
void RtmpOsVfree(void *pMem)
{
	if (pMem != NULL)
		vfree(pMem);
}





BOOLEAN RtmpOsStatsAlloc(void **ppStats, void **ppIwStats)
{
	*ppStats = os_alloc_mem(sizeof (struct net_device_stats));
	if ((*ppStats) == NULL)
		return FALSE;
	NdisZeroMemory((UCHAR*) *ppStats, sizeof (struct net_device_stats));

#if WIRELESS_EXT >= 12
	*ppIwStats = os_alloc_mem(sizeof (struct iw_statistics));
	if ((*ppIwStats) == NULL) {
		os_free_mem(*ppStats);
		return FALSE;
	}
	NdisZeroMemory((UCHAR*)* ppIwStats, sizeof (struct iw_statistics));
#endif

	return TRUE;
}


/*
========================================================================
Routine Description:
	Pass the received packet to OS.

Arguments:
	pPkt		- the packet

Return Value:
	None

Note:
========================================================================
*/
void RtmpOsPktRcvHandle(PNDIS_PACKET pNetPkt)
{
	struct sk_buff *pRxPkt = RTPKT_TO_OSPKT(pNetPkt);

	netif_rx(pRxPkt);
}


#ifdef IAPP_SUPPORT
/* Layer 2 Update frame to switch/bridge */
/* For any Layer2 devices, e.g., bridges, switches and other APs, the frame
   can update their forwarding tables with the correct port to reach the new
   location of the STA */
typedef struct GNU_PACKED _RT_IAPP_L2_UPDATE_FRAME {

	UCHAR DA[ETH_ALEN];	/* broadcast MAC address */
	UCHAR SA[ETH_ALEN];	/* the MAC address of the STA that has just associated
				   or reassociated */
	USHORT Len;		/* 8 octets */
	UCHAR DSAP;		/* null */
	UCHAR SSAP;		/* null */
	UCHAR Control;		/* reference to IEEE Std 802.2 */
	UCHAR XIDInfo[3];	/* reference to IEEE Std 802.2 */
} RT_IAPP_L2_UPDATE_FRAME, *PRT_IAPP_L2_UPDATE_FRAME;


PNDIS_PACKET RtmpOsPktIappMakeUp(PNET_DEV pNetDev, UINT8 *pMac)
{
	RT_IAPP_L2_UPDATE_FRAME frame_body;
	int size = sizeof (RT_IAPP_L2_UPDATE_FRAME);
	PNDIS_PACKET pNetBuf;

	if (pNetDev == NULL)
		return NULL;

	pNetBuf = RtmpOSNetPktAlloc(NULL, size);
	if (!pNetBuf) {
		DBGPRINT(RT_DEBUG_ERROR, ("Error! Can't allocate a skb.\n"));
		return NULL;
	}

	/* init the update frame body */
	NdisZeroMemory(&frame_body, size);

	memset(frame_body.DA, 0xFF, ETH_ALEN);
	memcpy(frame_body.SA, pMac, ETH_ALEN);

	frame_body.Len = OS_HTONS(ETH_ALEN);
	frame_body.DSAP = 0;
	frame_body.SSAP = 0x01;
	frame_body.Control = 0xAF;
	frame_body.XIDInfo[0] = 0x81;
	frame_body.XIDInfo[1] = 1;
	frame_body.XIDInfo[2] = 1 << 1;

	SET_OS_PKT_NETDEV(pNetBuf, pNetDev);
	skb_reserve(pNetBuf, 2);
	memcpy(skb_put(pNetBuf, size), &frame_body, size);
	return pNetBuf;
}
#endif /* IAPP_SUPPORT */


void RtmpOsPktNatMagicTag(IN PNDIS_PACKET pNetPkt)
{
#if !defined(CONFIG_RA_NAT_NONE)
#if defined (CONFIG_RA_HW_NAT)  || defined (CONFIG_RA_HW_NAT_MODULE)
	struct sk_buff *pRxPkt = RTPKT_TO_OSPKT(pNetPkt);
	FOE_MAGIC_TAG(pRxPkt) = FOE_MAGIC_WLAN;
#endif /* CONFIG_RA_HW_NAT || CONFIG_RA_HW_NAT_MODULE */
#endif /* CONFIG_RA_NAT_NONE */
}


void RtmpOsPktNatNone(IN PNDIS_PACKET pNetPkt)
{
#if defined(CONFIG_RA_NAT_NONE)
#if defined (CONFIG_RA_HW_NAT)  || defined (CONFIG_RA_HW_NAT_MODULE)
	FOE_AI(((struct sk_buff *)pNetPkt)) = UN_HIT;
#endif /* CONFIG_RA_HW_NAT || CONFIG_RA_HW_NAT_MODULE */
#endif /* CONFIG_RA_NAT_NONE */
}


/*
========================================================================
Routine Description:
	Flush a data cache line.

Arguments:
	AddrStart	- the start address
	Size		- memory size

Return Value:
	None

Note:
========================================================================
*/
void RtmpOsDCacheFlush(ULONG AddrStart, ULONG Size)
{
	RTMP_UTIL_DCACHE_FLUSH(AddrStart, Size);
}


#ifdef CONFIG_STA_SUPPORT
int RtmpOSNotifyRawData(PNET_DEV pNetDev, PUCHAR buff, int len, ULONG type,
	USHORT protocol)
{
	struct sk_buff *skb = NULL;

	skb = dev_alloc_skb(len + 2);

	if (!skb) {
		DBGPRINT(RT_DEBUG_ERROR,
				( "%s: failed to allocate sk_buff for notification\n",
				pNetDev->name));
		return -ENOMEM;
	} else {
		skb_reserve(skb, 2);
		memcpy(skb_put(skb, len), buff, len);
		skb->len = len;
		skb->dev = pNetDev;
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,21))
		skb->mac.raw = skb->data;
#else
		skb_set_mac_header(skb, 0);
#endif
		skb->ip_summed = CHECKSUM_UNNECESSARY;
		skb->pkt_type = PACKET_OTHERHOST;
		skb->protocol = htons(protocol);
		memset(skb->cb, 0, sizeof(skb->cb));

		netif_rx(skb);
	}
	return 0;
}

#endif /* CONFIG_STA_SUPPORT */


void OS_SPIN_LOCK_IRQSAVE(spinlock_t *lock, unsigned long *flags)
{
	spin_lock_irqsave(lock, *flags);
}

void OS_SPIN_UNLOCK_IRQRESTORE(spinlock_t *lock, unsigned long *flags)
{
	spin_unlock_irqrestore(lock, *flags);
}

void OS_SPIN_LOCK(spinlock_t *lock)
{
	spin_lock(lock);
}

void OS_SPIN_UNLOCK(spinlock_t *lock)
{
	spin_unlock(lock);
}

void OS_SPIN_LOCK_IRQ(spinlock_t *lock)
{
	spin_lock_irq(lock);
}

void OS_SPIN_UNLOCK_IRQ(spinlock_t *lock)
{
	spin_unlock_irq(lock);
}

int OS_TEST_BIT(int bit, unsigned long *flags)
{
	return test_bit(bit, flags);
}

void OS_SET_BIT(int bit, unsigned long *flags)
{
	set_bit(bit, flags);
}

void OS_CLEAR_BIT(int bit, unsigned long *flags)
{
	clear_bit(bit, flags);
}

#ifndef BB_SOC
void OS_LOAD_CODE_FROM_BIN(unsigned char **image, char *bin_name, void *inf_dev,
	UINT32 *code_len)
{
	struct device *dev;
	const struct firmware *fw_entry;

#ifdef RTMP_USB_SUPPORT
	dev = (struct device *)(&(((struct usb_device *)(inf_dev))->dev));
#endif

	if (request_firmware(&fw_entry, bin_name, dev) != 0) {
		DBGPRINT(RT_DEBUG_ERROR,
				("%s:fw not available(/lib/firmware/%s)\n",
				__FUNCTION__, bin_name));
		*image = NULL;
		return;
	}

	*image = kmalloc(fw_entry->size, GFP_KERNEL);
	memcpy(*image, fw_entry->data, fw_entry->size);
	*code_len = fw_entry->size;

	release_firmware(fw_entry);
}
#endif /* !BB_SOC */


void RtmpOSFSInfoChange(RTMP_OS_FS_INFO *pOSFSInfoOrg, BOOLEAN bSet)
{
	__RtmpOSFSInfoChange(pOSFSInfoOrg, bSet);
}


/* timeout -- ms */
void RTMP_SetPeriodicTimer(struct timer_list *pTimerOrg, unsigned long timeout)
{
	__RTMP_SetPeriodicTimer(pTimerOrg, timeout);
}


/* convert NdisMInitializeTimer --> RTMP_OS_Init_Timer */
void RTMP_OS_Init_Timer(void *pReserved, struct timer_list *pTimerOrg,
	TIMER_FUNCTION function, PVOID data, LIST_HEADER *pTimerList)
{
	__RTMP_OS_Init_Timer(pReserved, pTimerOrg, function, data);
}


void RTMP_OS_Add_Timer(struct timer_list *pTimerOrg, unsigned long timeout)
{
	__RTMP_OS_Add_Timer(pTimerOrg, timeout);
}


void RTMP_OS_Mod_Timer(struct timer_list *pTimerOrg, unsigned long timeout)
{
	__RTMP_OS_Mod_Timer(pTimerOrg, timeout);
}


void RTMP_OS_Del_Timer(struct timer_list *pTimerOrg, BOOLEAN *pCancelled)
{
	__RTMP_OS_Del_Timer(pTimerOrg, pCancelled);
}


NDIS_STATUS RtmpOSTaskKill(RTMP_OS_TASK *pTask)
{
	return __RtmpOSTaskKill(pTask);
}


int RtmpOSTaskNotifyToExit(RTMP_OS_TASK *pTask)
{
	return __RtmpOSTaskNotifyToExit(pTask);
}


void RtmpOSTaskCustomize(RTMP_OS_TASK *pTask)
{
	__RtmpOSTaskCustomize(pTask);
}


NDIS_STATUS RtmpOSTaskAttach(RTMP_OS_TASK *pTask, RTMP_OS_TASK_CALLBACK fn,
	ULONG arg)
{
	return __RtmpOSTaskAttach(pTask, fn, arg);
}


NDIS_STATUS RtmpOSTaskInit(RTMP_OS_TASK *pTask, PSTRING pTaskName, void *pPriv,
	LIST_HEADER *pTaskList, LIST_HEADER *pSemList)
{
	return __RtmpOSTaskInit(pTask, pTaskName, pPriv, pSemList);
}


BOOLEAN RtmpOSTaskWait(void *pReserved, RTMP_OS_TASK * pTask, INT32 *pStatus)
{
	return __RtmpOSTaskWait(pReserved, pTask, pStatus);
}




