/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2006, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	rtusb_bulk.c

*/

#include "rt_config.h"

static INT RTUSBCmdThread(ULONG Context);

/*
========================================================================
Routine Description:
	Create kernel threads & tasklets.

Arguments:
	*net_dev	Pointer to wireless net device interface

Return Value:
	NDIS_STATUS_SUCCESS
	NDIS_STATUS_FAILURE

Note:
========================================================================
*/
NDIS_STATUS RtmpMgmtTaskInit(RTMP_ADAPTER *pAd)
{
	RTMP_OS_TASK *pTask;
	NDIS_STATUS status;

	/*
	 Create TimerQ Thread, We need init timerQ related structure
	 before create the timer thread.
	*/
	RtmpTimerQInit(pAd);

	pTask = &pAd->timerTask;
	RTMP_OS_TASK_INIT(pTask, "RtmpTimerTask", pAd);
	status = RtmpOSTaskAttach(pTask, RtmpTimerQThread, (ULONG)pTask);
	if (status == NDIS_STATUS_FAILURE) {
		printk (KERN_WARNING "%s: unable to start RtmpTimerQThread\n",
				RTMP_OS_NETDEV_GET_DEVNAME(pAd->net_dev));
		return NDIS_STATUS_FAILURE;
	}

	/* Creat Command Thread */
	pTask = &pAd->cmdQTask;
	RTMP_OS_TASK_INIT(pTask, "RtmpCmdQTask", pAd);
	status = RtmpOSTaskAttach(pTask, RTUSBCmdThread, (ULONG)pTask);
	if (status == NDIS_STATUS_FAILURE) {
		printk (KERN_WARNING "%s: unable to start RTUSBCmdThread\n",
				RTMP_OS_NETDEV_GET_DEVNAME(pAd->net_dev));
		return NDIS_STATUS_FAILURE;
	}

	return NDIS_STATUS_SUCCESS;
}



/*
========================================================================
Routine Description:
	Close kernel threads.

Arguments:
	*pAd		the raxx interface data pointer

Return Value:
	NONE

Note:
========================================================================
*/
VOID RtmpMgmtTaskExit(RTMP_ADAPTER *pAd)
{
	INT ret;
	RTMP_OS_TASK *pTask;

	/* Sleep 50 milliseconds so pending io might finish normally */
	RtmpusecDelay(50000);

	/* We want to wait until all pending receives and sends to the */
	/* device object. We cancel any */
	/* irps. Wait until sends and receives have stopped. */
	RTUSBCancelPendingIRPs(pAd);

	/* We need clear timerQ related structure before exits of the timer thread. */
	RtmpTimerQExit(pAd);

	/* Terminate cmdQ thread */
	pTask = &pAd->cmdQTask;
	RTMP_OS_TASK_LEGALITY(pTask) {
		NdisAcquireSpinLock(&pAd->CmdQLock);
		pAd->CmdQ.CmdQState = RTMP_TASK_STAT_STOPPED;
		NdisReleaseSpinLock(&pAd->CmdQLock);

		/*RTUSBCMDUp(&pAd->cmdQTask); */
		ret = RtmpOSTaskKill(pTask);
		if (ret == NDIS_STATUS_FAILURE) {
			DBGPRINT(RT_DEBUG_ERROR, ("kill command task failed!\n"));
		}
		pAd->CmdQ.CmdQState = RTMP_TASK_STAT_UNKNOWN;
	}

	/* Terminate timer thread */
	pTask = &pAd->timerTask;
	ret = RtmpOSTaskKill(pTask);
	if (ret == NDIS_STATUS_FAILURE) {
		DBGPRINT(RT_DEBUG_ERROR, ("kill timer task failed!\n"));
	}
}


static void rtusb_dataout_complete(unsigned long data)
{
	PRTMP_ADAPTER pAd;
	purbb_t pUrb;
	POS_COOKIE pObj;
	PHT_TX_CONTEXT pHTTXContext;
	UCHAR BulkOutPipeId;
	NTSTATUS Status;
	unsigned long IrqFlags;

	pUrb = (purbb_t)data;
	pHTTXContext = (PHT_TX_CONTEXT)RTMP_USB_URB_DATA_GET(pUrb);
	Status = RTMP_USB_URB_STATUS_GET(pUrb);
	pAd = pHTTXContext->pAd;
	pObj = (POS_COOKIE) pAd->OS_Cookie;

	/* Store BulkOut PipeId */
	BulkOutPipeId = pHTTXContext->BulkOutPipeId;
	pAd->BulkOutDataOneSecCount++;

	RTMP_IRQ_LOCK(&pAd->BulkOutLock[BulkOutPipeId], IrqFlags);
	pAd->BulkOutPending[BulkOutPipeId] = FALSE;
	pHTTXContext->IRPPending = FALSE;
	pAd->watchDogTxPendingCnt[BulkOutPipeId] = 0;

	if (Status == USB_ST_NOERROR) {
		pAd->BulkOutComplete++;

		RTMP_IRQ_UNLOCK(&pAd->BulkOutLock[BulkOutPipeId], IrqFlags);

		pAd->Counters8023.GoodTransmits++;
		/*RTMP_IRQ_LOCK(&pAd->TxContextQueueLock[BulkOutPipeId], IrqFlags); */
		FREE_HTTX_RING(pAd, BulkOutPipeId, pHTTXContext);
		/*RTMP_IRQ_UNLOCK(&pAd->TxContextQueueLock[BulkOutPipeId], IrqFlags); */

#ifdef UAPSD_SUPPORT
#ifdef CONFIG_AP_SUPPORT
#ifdef RT_CFG80211_P2P_SUPPORT
		if (RTMP_CFG80211_VIF_P2P_GO_ON(pAd))
#else
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
#endif /* RT_CFG80211_P2P_SUPPORT */
		{
			UAPSD_UnTagFrame(pAd, BulkOutPipeId, pHTTXContext->NextBulkOutPosition, pHTTXContext->ENextBulkOutPosition);
		}
#endif /* CONFIG_AP_SUPPORT */
#endif /* UAPSD_SUPPORT */

	} else {
		/* STATUS_OTHER */
		PUCHAR pBuf;

		pAd->BulkOutCompleteOther++;
		pBuf = &pHTTXContext->TransferBuffer->field.WirelessPacket[pHTTXContext->NextBulkOutPosition];

		if (!RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_RESET_IN_PROGRESS |
			fRTMP_ADAPTER_HALT_IN_PROGRESS |
			fRTMP_ADAPTER_NIC_NOT_EXIST |
			fRTMP_ADAPTER_BULKOUT_RESET))){
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET);
			pAd->bulkResetPipeid = BulkOutPipeId;
			pAd->bulkResetReq[BulkOutPipeId] = pAd->BulkOutReq;
		}
		RTMP_IRQ_UNLOCK(&pAd->BulkOutLock[BulkOutPipeId], IrqFlags);

		DBGPRINT(RT_DEBUG_ERROR,
				("BulkOutDataPacket failed: ReasonCode=%d!\n",
				Status));
		DBGPRINT(RT_DEBUG_ERROR,
				("\t>>BulkOut Req=0x%lx, Complete=0x%lx, Other=0x%lx\n",
				 pAd->BulkOutReq, pAd->BulkOutComplete,
				 pAd->BulkOutCompleteOther));
		DBGPRINT(RT_DEBUG_ERROR,
				("\t>>BulkOut Header:%x %x %x %x %x %x %x %x\n",
				pBuf[0], pBuf[1], pBuf[2], pBuf[3], pBuf[4],
				pBuf[5], pBuf[6], pBuf[7]));
	}

	/* bInUse = TRUE, means some process are filling TX data, after that must turn on bWaitingBulkOut */
	/* bWaitingBulkOut = TRUE, means the TX data are waiting for bulk out. */
	/*RTMP_IRQ_LOCK(&pAd->TxContextQueueLock[BulkOutPipeId], IrqFlags); */
	if (((pHTTXContext->ENextBulkOutPosition != pHTTXContext->CurWritePosition) &&
		(pHTTXContext->ENextBulkOutPosition != (pHTTXContext->CurWritePosition+8)) &&
		!RTUSB_TEST_BULK_FLAG(pAd, (fRTUSB_BULK_OUT_DATA_FRAG << BulkOutPipeId)))
#ifdef USB_BULK_BUF_ALIGMENT
		|| (pHTTXContext->NextBulkIdx != pHTTXContext->CurWriteIdx)
#endif /* USB_BULK_BUF_ALIGMENT */
	) {
		/* Indicate There is data avaliable */
		RTUSB_SET_BULK_FLAG(pAd, (fRTUSB_BULK_OUT_DATA_NORMAL << BulkOutPipeId));
	}
	/*RTMP_IRQ_UNLOCK(&pAd->TxContextQueueLock[BulkOutPipeId], IrqFlags); */

	/* Always call Bulk routine, even reset bulk. */
	/* The protection of rest bulk should be in BulkOut routine */
	RTUSBKickBulkOut(pAd);
}


static void rtusb_null_frame_done_tasklet(unsigned long data)
{
	PRTMP_ADAPTER pAd;
	PTX_CONTEXT pNullContext;
	purbb_t pUrb;
	NTSTATUS Status;
	unsigned long irqFlag;

	pUrb = (purbb_t)data;
	pNullContext = (PTX_CONTEXT)RTMP_USB_URB_DATA_GET(pUrb);
	Status = RTMP_USB_URB_STATUS_GET(pUrb);
	pAd = pNullContext->pAd;

	/* Reset Null frame context flags */
	RTMP_IRQ_LOCK(&pAd->BulkOutLock[0], irqFlag);
	pNullContext->IRPPending = FALSE;
	pNullContext->InUse = FALSE;
	pAd->BulkOutPending[0] = FALSE;
	pAd->watchDogTxPendingCnt[0] = 0;

	if (Status == USB_ST_NOERROR) {
		RTMP_IRQ_UNLOCK(&pAd->BulkOutLock[0], irqFlag);
		RTMPDeQueuePacket(pAd, FALSE, NUM_OF_TX_RING, MAX_TX_PROCESS);
	} else {
		/* STATUS_OTHER */
		if ((!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS)) &&
			(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) &&
			(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) &&
			(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET))){
			DBGPRINT(RT_DEBUG_ERROR,
					("Bulk Out Null Frame Failed, ReasonCode=%d!\n",
					Status));
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET);
			pAd->bulkResetPipeid = (MGMTPIPEIDX | BULKOUT_MGMT_RESET_FLAG);
			RTMP_IRQ_UNLOCK(&pAd->BulkOutLock[0], irqFlag);
			RTEnqueueInternalCmd(pAd, CMDTHREAD_RESET_BULK_OUT, NULL, 0);
		} else {
			RTMP_IRQ_UNLOCK(&pAd->BulkOutLock[0], irqFlag);
		}
	}

	/* Always call Bulk routine, even reset bulk. */
	/* The protectioon of rest bulk should be in BulkOut routine */
	RTUSBKickBulkOut(pAd);
}

static void rtusb_pspoll_frame_done_tasklet(unsigned long data)
{
	PRTMP_ADAPTER pAd;
	PTX_CONTEXT pPsPollContext;
	purbb_t pUrb;
	NTSTATUS Status;

	pUrb = (purbb_t)data;
	pPsPollContext = (PTX_CONTEXT)RTMP_USB_URB_DATA_GET(pUrb);
	Status = RTMP_USB_URB_STATUS_GET(pUrb);
	pAd = pPsPollContext->pAd;

	/* Reset PsPoll context flags */
	pPsPollContext->IRPPending = FALSE;
	pPsPollContext->InUse = FALSE;
	pAd->watchDogTxPendingCnt[0] = 0;

	if (Status == USB_ST_NOERROR) {
		RTMPDeQueuePacket(pAd, FALSE, NUM_OF_TX_RING, MAX_TX_PROCESS);
	} else {
		/* STATUS_OTHER */
		if ((!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS)) &&
			(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) &&
			(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) &&
			(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET))) {
			DBGPRINT(RT_DEBUG_ERROR, ("Bulk Out PSPoll Failed\n"));
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET);
			pAd->bulkResetPipeid = (MGMTPIPEIDX | BULKOUT_MGMT_RESET_FLAG);
			RTEnqueueInternalCmd(pAd, CMDTHREAD_RESET_BULK_OUT, NULL, 0);
		}
	}

	RTMP_SEM_LOCK(&pAd->BulkOutLock[0]);
	pAd->BulkOutPending[0] = FALSE;
	RTMP_SEM_UNLOCK(&pAd->BulkOutLock[0]);

	/* Always call Bulk routine, even reset bulk. */
	/* The protectioon of rest bulk should be in BulkOut routine */
	RTUSBKickBulkOut(pAd);
}


/*
========================================================================
Routine Description:
	Handle received packets.

Arguments:
	data		- URB information pointer

Return Value:
	None

Note:
========================================================================
*/
static void rx_done_tasklet(unsigned long data)
{
	purbb_t pUrb;
	PRX_CONTEXT pRxContext;
	PRTMP_ADAPTER pAd;
	NTSTATUS Status;
	unsigned long IrqFlags;
	pUrb = (purbb_t)data;
	pRxContext = (PRX_CONTEXT)RTMP_USB_URB_DATA_GET(pUrb);
	Status = RTMP_USB_URB_STATUS_GET(pUrb);
	pAd = pRxContext->pAd;

	RTMP_IRQ_LOCK(&pAd->BulkInLock, IrqFlags);
	pRxContext->InUse = FALSE;
	pRxContext->IRPPending = FALSE;
	pRxContext->BulkInOffset += RTMP_USB_URB_LEN_GET(pUrb); /*pUrb->actual_length; */
	/*NdisInterlockedDecrement(&pAd->PendingRx); */

	if (pAd->PendingRx > 0)
		pAd->PendingRx--;

	if (Status == USB_ST_NOERROR) {
		pAd->BulkInComplete++;
		pAd->NextRxBulkInPosition = 0;
		if (pRxContext->BulkInOffset) {
			/* As jan's comment, it may bulk-in success but size is zero. */
			pRxContext->Readable = TRUE;
			INC_RING_INDEX(pAd->NextRxBulkInIndex, RX_RING_SIZE);
		}
		RTMP_IRQ_UNLOCK(&pAd->BulkInLock, IrqFlags);
	} else {
		/* STATUS_OTHER */
		pAd->BulkInCompleteFail++;
		/* Still read this packet although it may comtain wrong bytes. */
		pRxContext->Readable = FALSE;
		RTMP_IRQ_UNLOCK(&pAd->BulkInLock, IrqFlags);

		/* Parsing all packets. because after reset, the index will reset to all zero. */
		if ((!RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_RESET_IN_PROGRESS |
			fRTMP_ADAPTER_BULKIN_RESET | fRTMP_ADAPTER_HALT_IN_PROGRESS |
			fRTMP_ADAPTER_RADIO_OFF |fRTMP_ADAPTER_NIC_NOT_EXIST)))) {

			DBGPRINT(RT_DEBUG_ERROR,
					("Bulk In Failed. Status=%d, BIIdx=0x%x, BIRIdx=0x%x, actual_length= 0x%x\n",
					Status, pAd->NextRxBulkInIndex,
					pAd->NextRxBulkInReadIndex,
					RTMP_USB_URB_LEN_GET(pRxContext->pUrb))); /*->actual_length)); */

			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_BULKIN_RESET);
			RTEnqueueInternalCmd(pAd, CMDTHREAD_RESET_BULK_IN, NULL, 0);
		}
	}

	ASSERT((pRxContext->InUse == pRxContext->IRPPending));

#ifdef RALINK_ATE
	if (ATE_ON(pAd)) {
		/* If the driver is in ATE mode and Rx frame is set into here. */
		if (pAd->ContinBulkIn == TRUE) {
			RTUSBBulkReceive(pAd);
		}
	}
	else
#endif /* RALINK_ATE */
	RTUSBBulkReceive(pAd);
}



static void rtusb_mgmt_dma_done_tasklet(unsigned long data)
{
	PRTMP_ADAPTER pAd;
	PTX_CONTEXT pMLMEContext;
	int index;
	PNDIS_PACKET pPacket;
	purbb_t pUrb;
	NTSTATUS Status;
	unsigned long IrqFlags;

	pUrb = (purbb_t)data;
	pMLMEContext = (PTX_CONTEXT)RTMP_USB_URB_DATA_GET(pUrb);
	Status = RTMP_USB_URB_STATUS_GET(pUrb);
	pAd = pMLMEContext->pAd;
	index = pMLMEContext->SelfIdx;

	ASSERT((pAd->MgmtRing.TxDmaIdx == index));

	RTMP_IRQ_LOCK(&pAd->BulkOutLock[MGMTPIPEIDX], IrqFlags);

#ifdef UAPSD_SUPPORT
	/* Qos Null frame with EOSP shall have valid Wcid value. reference RtmpUSBMgmtKickOut() API. */
	/* otherwise will be value of MCAST_WCID. */
#ifdef CONFIG_AP_SUPPORT
#ifdef RT_CFG80211_P2P_SUPPORT
	if (RTMP_CFG80211_VIF_P2P_GO_ON(pAd))
#else
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
#endif /* RT_CFG80211_P2P_SUPPORT */
	{
		/* Qos Null frame with EOSP shall have valid Wcid value. reference RtmpUSBMgmtKickOut() API. */
		/* otherwise will be value of MCAST_WCID. */
		if ((pMLMEContext->Wcid != MCAST_WCID) && (pMLMEContext->Wcid < MAX_LEN_OF_MAC_TABLE)) {
			MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[pMLMEContext->Wcid];

			UAPSD_SP_Close(pAd, pEntry);
			pMLMEContext->Wcid = MCAST_WCID;
		}
	}
#endif /* CONFIG_AP_SUPPORT */
#endif /* UAPSD_SUPPORT */

	if (Status != USB_ST_NOERROR) {
		/*Bulk-Out fail status handle */
		if ((!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS)) &&
			(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) &&
			(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) &&
			(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET))) {
			DBGPRINT(RT_DEBUG_ERROR,
					("Bulk Out MLME Failed, Status=%d!\n",
					Status));
			/* TODO: How to handle about the MLMEBulkOut failed issue.
			 * Need to resend the mgmt pkt?
			 */
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET);
			pAd->bulkResetPipeid = (MGMTPIPEIDX | BULKOUT_MGMT_RESET_FLAG);
		}
	}

	pAd->BulkOutPending[MGMTPIPEIDX] = FALSE;
	RTMP_IRQ_UNLOCK(&pAd->BulkOutLock[MGMTPIPEIDX], IrqFlags);

	RTMP_IRQ_LOCK(&pAd->MLMEBulkOutLock, IrqFlags);
	/* Reset MLME context flags */
	pMLMEContext->IRPPending = FALSE;
	pMLMEContext->InUse = FALSE;
	pMLMEContext->bWaitingBulkOut = FALSE;
	pMLMEContext->BulkOutSize = 0;

	pPacket = pAd->MgmtRing.Cell[index].pNdisPacket;
	pAd->MgmtRing.Cell[index].pNdisPacket = NULL;

	/* Increase MgmtRing Index */
	INC_RING_INDEX(pAd->MgmtRing.TxDmaIdx, MGMT_RING_SIZE);
	pAd->MgmtRing.TxSwFreeIdx++;
	RTMP_IRQ_UNLOCK(&pAd->MLMEBulkOutLock, IrqFlags);


#if RT_CFG80211_SUPPORT
	{
	HEADER_802_11  *pHeader;
	pHeader = (HEADER_802_11 *)(GET_OS_PKT_DATAPTR(pPacket)+
			TXINFO_SIZE + pAd->chipCap.TXWISize);
	CFG80211_SendMgmtFrameDone(pAd, pHeader->Sequence);
	}
#endif /* RT_CFG80211_SUPPORT */

	/* No-matter success or fail, we free the mgmt packet. */
	if (pPacket)
		dev_kfree_skb_any(pPacket);

	if ((RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_RESET_IN_PROGRESS |
		fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST)))) {
		/* do nothing and return directly. */
	} else {
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET) &&
			((pAd->bulkResetPipeid & BULKOUT_MGMT_RESET_FLAG) == BULKOUT_MGMT_RESET_FLAG)) {
			/* For Mgmt Bulk-Out failed, ignore it now. */
			RTEnqueueInternalCmd(pAd, CMDTHREAD_RESET_BULK_OUT, NULL, 0);
		} else {
			/* Always call Bulk routine, even reset bulk. */
			/* The protectioon of rest bulk should be in BulkOut routine */
			if (pAd->MgmtRing.TxSwFreeIdx < MGMT_RING_SIZE /* pMLMEContext->bWaitingBulkOut == TRUE */) {
				RTUSB_SET_BULK_FLAG(pAd, fRTUSB_BULK_OUT_MLME);
			}
			RTUSBKickBulkOut(pAd);
		}
	}
}


static void rtusb_hcca_dma_done_tasklet(unsigned long data)
{
	PRTMP_ADAPTER pAd;
	PHT_TX_CONTEXT pHTTXContext;
	UCHAR BulkOutPipeId = 4;
	purbb_t pUrb;

	DBGPRINT(RT_DEBUG_ERROR, ("--->hcca_dma_done_tasklet\n"));

	pUrb = (purbb_t)data;
	pHTTXContext = (PHT_TX_CONTEXT)RTMP_USB_URB_DATA_GET(pUrb);
	pAd = pHTTXContext->pAd;

	rtusb_dataout_complete((unsigned long)pUrb);

	if ((RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_RESET_IN_PROGRESS |
		fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST)))) {
		/* do nothing and return directly. */
	} else {
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET)) {
			RTEnqueueInternalCmd(pAd, CMDTHREAD_RESET_BULK_OUT, NULL, 0);
		} else {
			pHTTXContext = &pAd->TxContext[BulkOutPipeId];
			if ((pAd->TxSwQueue[BulkOutPipeId].Number > 0) &&
				(pAd->DeQueueRunning[BulkOutPipeId] == FALSE) &&
				(pHTTXContext->bCurWriting == FALSE)) {
				RTMPDeQueuePacket(pAd, FALSE, BulkOutPipeId, MAX_TX_PROCESS);
			}

			RTUSB_SET_BULK_FLAG(pAd, fRTUSB_BULK_OUT_DATA_NORMAL);
			RTUSBKickBulkOut(pAd);
		}
	}

	DBGPRINT(RT_DEBUG_ERROR, ("<---hcca_dma_done_tasklet\n"));
}


static void rtusb_ac3_dma_done_tasklet(unsigned long data)
{
	PRTMP_ADAPTER pAd;
	PHT_TX_CONTEXT pHTTXContext;
	UCHAR BulkOutPipeId = 3;
	purbb_t pUrb;

	pUrb = (purbb_t)data;
	pHTTXContext = (PHT_TX_CONTEXT)RTMP_USB_URB_DATA_GET(pUrb);
	pAd = pHTTXContext->pAd;

	rtusb_dataout_complete((unsigned long)pUrb);

	if ((RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_RESET_IN_PROGRESS |
		fRTMP_ADAPTER_HALT_IN_PROGRESS |fRTMP_ADAPTER_NIC_NOT_EXIST)))) {
		/* do nothing and return directly. */
	} else {
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET)) {
			RTEnqueueInternalCmd(pAd, CMDTHREAD_RESET_BULK_OUT, NULL, 0);
		} else {
			pHTTXContext = &pAd->TxContext[BulkOutPipeId];
			if ((pAd->TxSwQueue[BulkOutPipeId].Number > 0) &&
				(pAd->DeQueueRunning[BulkOutPipeId] == FALSE) &&
				(pHTTXContext->bCurWriting == FALSE)) {
				RTMPDeQueuePacket(pAd, FALSE, BulkOutPipeId, MAX_TX_PROCESS);
			}

			RTUSB_SET_BULK_FLAG(pAd, fRTUSB_BULK_OUT_DATA_NORMAL<<3);
			RTUSBKickBulkOut(pAd);
		}
	}
}


static void rtusb_ac2_dma_done_tasklet(unsigned long data)
{
	PRTMP_ADAPTER pAd;
	PHT_TX_CONTEXT pHTTXContext;
	UCHAR BulkOutPipeId = 2;
	purbb_t pUrb;

	pUrb = (purbb_t)data;
	pHTTXContext = (PHT_TX_CONTEXT)RTMP_USB_URB_DATA_GET(pUrb);
	pAd = pHTTXContext->pAd;

	rtusb_dataout_complete((unsigned long)pUrb);

	if ((RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_RESET_IN_PROGRESS |
		fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST)))) {
		/* do nothing and return directly. */
	} else {
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET)) {
			RTEnqueueInternalCmd(pAd, CMDTHREAD_RESET_BULK_OUT, NULL, 0);
		} else {
			pHTTXContext = &pAd->TxContext[BulkOutPipeId];
			if ((pAd->TxSwQueue[BulkOutPipeId].Number > 0) &&
				(pAd->DeQueueRunning[BulkOutPipeId] == FALSE) &&
				(pHTTXContext->bCurWriting == FALSE)) {
				RTMPDeQueuePacket(pAd, FALSE, BulkOutPipeId, MAX_TX_PROCESS);
			}

			RTUSB_SET_BULK_FLAG(pAd, fRTUSB_BULK_OUT_DATA_NORMAL<<2);
			RTUSBKickBulkOut(pAd);
		}
	}
}


static void rtusb_ac1_dma_done_tasklet(unsigned long data)
{
	PRTMP_ADAPTER pAd;
	PHT_TX_CONTEXT pHTTXContext;
	UCHAR BulkOutPipeId = 1;
	purbb_t pUrb;

	pUrb = (purbb_t)data;
	pHTTXContext = (PHT_TX_CONTEXT)RTMP_USB_URB_DATA_GET(pUrb);
	pAd = pHTTXContext->pAd;

	rtusb_dataout_complete((unsigned long)pUrb);

	if ((RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_RESET_IN_PROGRESS |
		fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST)))) {
		/* do nothing and return directly. */
	} else {
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET)) {
			RTEnqueueInternalCmd(pAd, CMDTHREAD_RESET_BULK_OUT, NULL, 0);
		} else {
			pHTTXContext = &pAd->TxContext[BulkOutPipeId];
			if ((pAd->TxSwQueue[BulkOutPipeId].Number > 0) &&
				(pAd->DeQueueRunning[BulkOutPipeId] == FALSE) &&
				(pHTTXContext->bCurWriting == FALSE)) {
				RTMPDeQueuePacket(pAd, FALSE, BulkOutPipeId, MAX_TX_PROCESS);
			}

			RTUSB_SET_BULK_FLAG(pAd, fRTUSB_BULK_OUT_DATA_NORMAL<<1);
			RTUSBKickBulkOut(pAd);
		}
	}
}


static void rtusb_ac0_dma_done_tasklet(unsigned long data)
{
	PRTMP_ADAPTER pAd;
	PHT_TX_CONTEXT pHTTXContext;
	UCHAR BulkOutPipeId = 0;
	purbb_t pUrb;

	pUrb = (purbb_t)data;
	pHTTXContext = (PHT_TX_CONTEXT)RTMP_USB_URB_DATA_GET(pUrb);
	pAd = pHTTXContext->pAd;

	rtusb_dataout_complete((unsigned long)pUrb);

	if ((RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_RESET_IN_PROGRESS |
		fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST)))) {
		/* do nothing and return directly. */
	} else {
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET)) {
			RTEnqueueInternalCmd(pAd, CMDTHREAD_RESET_BULK_OUT, NULL, 0);
		} else {
			pHTTXContext = &pAd->TxContext[BulkOutPipeId];
			if ((pAd->TxSwQueue[BulkOutPipeId].Number > 0) &&
				(pAd->DeQueueRunning[BulkOutPipeId] == FALSE) &&
				(pHTTXContext->bCurWriting == FALSE)) {
				RTMPDeQueuePacket(pAd, FALSE, BulkOutPipeId, MAX_TX_PROCESS);
			}

			RTUSB_SET_BULK_FLAG(pAd, fRTUSB_BULK_OUT_DATA_NORMAL);
			RTUSBKickBulkOut(pAd);
		}
	}
}

#ifdef RALINK_ATE
static void rtusb_ate_ac0_dma_done_tasklet(unsigned long data)
{
	PRTMP_ADAPTER pAd;
	PTX_CONTEXT pNullContext;
	UCHAR BulkOutPipeId;
	NTSTATUS Status;
	ULONG IrqFlags;
	ULONG OldValue;
	purbb_t pURB;

	pURB = (purbb_t)data;
	/*pNullContext = (PTX_CONTEXT)pURB->rtusb_urb_context; */
	pNullContext = (PTX_CONTEXT)RTMP_USB_URB_DATA_GET(pURB);
	pAd = pNullContext->pAd;

	/* Reset Null frame context flags */
	pNullContext->IRPPending = FALSE;
	pNullContext->InUse = FALSE;
	Status = RTMP_USB_URB_STATUS_GET(pURB);/*pURB->rtusb_urb_status; */

	/* Store BulkOut PipeId. */
	BulkOutPipeId = pNullContext->BulkOutPipeId;
	pAd->BulkOutDataOneSecCount++;

	if (Status == USB_ST_NOERROR) {
		pAd->BulkOutComplete++;
		pAd->Counters8023.GoodTransmits++;
		RTMPDeQueuePacket(pAd, TRUE, BulkOutPipeId, MAX_TX_PROCESS);
	} else {
		pAd->BulkOutCompleteOther++;

		DBGPRINT(RT_DEBUG_ERROR,
				("BulkOutDataPacket Failed STATUS_OTHER = 0x%x . \n",
				Status));
		DBGPRINT(RT_DEBUG_ERROR,
				(">>BulkOutReq=0x%lx, BulkOutComplete=0x%lx\n",
				pAd->BulkOutReq, pAd->BulkOutComplete));

		if ((!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS)) &&
			(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) &&
			(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) &&
			(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET))) {
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET);

			/* In 28xx, RT_OID_USB_RESET_BULK_OUT ==> CMDTHREAD_RESET_BULK_OUT */
			RTEnqueueInternalCmd(pAd, CMDTHREAD_RESET_BULK_OUT, NULL, 0);

			/* check */
			BULK_OUT_LOCK(&pAd->BulkOutLock[BulkOutPipeId], IrqFlags);
			pAd->BulkOutPending[BulkOutPipeId] = FALSE;
			pAd->bulkResetPipeid = BulkOutPipeId;
			BULK_OUT_UNLOCK(&pAd->BulkOutLock[BulkOutPipeId], IrqFlags);
			return;
		}
	}

	if (atomic_read(&pAd->BulkOutRemained) > 0) {
		atomic_dec(&pAd->BulkOutRemained);
	}

	/* 1st - Transmit Success */
	OldValue = pAd->WlanCounters.TransmittedFragmentCount.u.LowPart;
	pAd->WlanCounters.TransmittedFragmentCount.u.LowPart++;

	if (pAd->WlanCounters.TransmittedFragmentCount.u.LowPart < OldValue) {
		pAd->WlanCounters.TransmittedFragmentCount.u.HighPart++;
	}

	if (((pAd->ContinBulkOut == TRUE ) ||(atomic_read(&pAd->BulkOutRemained) > 0))
		&& (pAd->ate.Mode & ATE_TXFRAME)) {
		RTUSB_SET_BULK_FLAG(pAd, fRTUSB_BULK_OUT_DATA_ATE);
	} else {
		RTUSB_CLEAR_BULK_FLAG(pAd, fRTUSB_BULK_OUT_DATA_ATE);
	}

	BULK_OUT_LOCK(&pAd->BulkOutLock[BulkOutPipeId], IrqFlags);
	pAd->BulkOutPending[BulkOutPipeId] = FALSE;
	BULK_OUT_UNLOCK(&pAd->BulkOutLock[BulkOutPipeId], IrqFlags);

	/* Always call Bulk routine, even reset bulk. */
	/* The protection of rest bulk should be in BulkOut routine. */
	RTUSBKickBulkOut(pAd);
}
#endif /* RALINK_ATE */


NDIS_STATUS RtmpNetTaskInit(RTMP_ADAPTER *pAd)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;

	/* Create receive tasklet */
	RTMP_OS_TASKLET_INIT(pAd, &pObj->rx_done_task, rx_done_tasklet, (ULONG)pAd);
	//RTMP_OS_TASKLET_INIT(pAd, &pObj->cmd_rsp_event_task, cmd_rsp_event_tasklet, (ULONG)pAd);
	RTMP_OS_TASKLET_INIT(pAd, &pObj->mgmt_dma_done_task, rtusb_mgmt_dma_done_tasklet, (unsigned long)pAd);
	RTMP_OS_TASKLET_INIT(pAd, &pObj->ac0_dma_done_task, rtusb_ac0_dma_done_tasklet, (unsigned long)pAd);
#ifdef RALINK_ATE
	RTMP_OS_TASKLET_INIT(pAd, &pObj->ate_ac0_dma_done_task, rtusb_ate_ac0_dma_done_tasklet, (unsigned long)pAd);
#endif /* RALINK_ATE */
	RTMP_OS_TASKLET_INIT(pAd, &pObj->ac1_dma_done_task, rtusb_ac1_dma_done_tasklet, (unsigned long)pAd);
	RTMP_OS_TASKLET_INIT(pAd, &pObj->ac2_dma_done_task, rtusb_ac2_dma_done_tasklet, (unsigned long)pAd);
	RTMP_OS_TASKLET_INIT(pAd, &pObj->ac3_dma_done_task, rtusb_ac3_dma_done_tasklet, (unsigned long)pAd);
	RTMP_OS_TASKLET_INIT(pAd, &pObj->hcca_dma_done_task, rtusb_hcca_dma_done_tasklet, (unsigned long)pAd);
	RTMP_OS_TASKLET_INIT(pAd, &pObj->tbtt_task, tbtt_tasklet, (unsigned long)pAd);
	RTMP_OS_TASKLET_INIT(pAd, &pObj->null_frame_complete_task, rtusb_null_frame_done_tasklet, (unsigned long)pAd);
	RTMP_OS_TASKLET_INIT(pAd, &pObj->pspoll_frame_complete_task, rtusb_pspoll_frame_done_tasklet, (unsigned long)pAd);

	return NDIS_STATUS_SUCCESS;
}


void RtmpNetTaskExit(IN RTMP_ADAPTER *pAd)
{
	POS_COOKIE pObj;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	RTMP_OS_TASKLET_KILL(&pObj->rx_done_task);
	RTMP_OS_TASKLET_KILL(&pObj->cmd_rsp_event_task);
	RTMP_OS_TASKLET_KILL(&pObj->mgmt_dma_done_task);
	RTMP_OS_TASKLET_KILL(&pObj->ac0_dma_done_task);
#ifdef RALINK_ATE
	RTMP_OS_TASKLET_KILL(&pObj->ate_ac0_dma_done_task);
#endif
	RTMP_OS_TASKLET_KILL(&pObj->ac1_dma_done_task);
	RTMP_OS_TASKLET_KILL(&pObj->ac2_dma_done_task);
	RTMP_OS_TASKLET_KILL(&pObj->ac3_dma_done_task);
	RTMP_OS_TASKLET_KILL(&pObj->hcca_dma_done_task);
	RTMP_OS_TASKLET_KILL(&pObj->tbtt_task);
	RTMP_OS_TASKLET_KILL(&pObj->null_frame_complete_task);
	RTMP_OS_TASKLET_KILL(&pObj->pspoll_frame_complete_task);
}


/*
========================================================================
Routine Description:
	USB command kernel thread.

Arguments:
	*Context	the pAd, driver control block pointer

Return Value:
	0		close the thread

Note:
========================================================================
*/
static INT RTUSBCmdThread(ULONG Context)
{
	RTMP_ADAPTER *pAd;
	RTMP_OS_TASK *pTask;
	int status;
	status = 0;

	pTask = (RTMP_OS_TASK *)Context;
	pAd = (PRTMP_ADAPTER)RTMP_OS_TASK_DATA_GET(pTask);

	if (pAd == NULL)
		return 0;

	RtmpOSTaskCustomize(pTask);

	NdisAcquireSpinLock(&pAd->CmdQLock);
	pAd->CmdQ.CmdQState = RTMP_TASK_STAT_RUNNING;
	NdisReleaseSpinLock(&pAd->CmdQLock);

	while (pAd->CmdQ.CmdQState == RTMP_TASK_STAT_RUNNING) {
		if (RtmpOSTaskWait(pAd, pTask, &status) == FALSE) {
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);
			break;
		}

		if (pAd->CmdQ.CmdQState == RTMP_TASK_STAT_STOPPED)
			break;

		if (!pAd->PM_FlgSuspend)
			CMDHandler(pAd);
	}

	if (!pAd->PM_FlgSuspend) {
		/* Clear the CmdQElements. */
		CmdQElmt *pCmdQElmt = NULL;

		NdisAcquireSpinLock(&pAd->CmdQLock);
		pAd->CmdQ.CmdQState = RTMP_TASK_STAT_STOPPED;
		while (pAd->CmdQ.size) {
			RTThreadDequeueCmd(&pAd->CmdQ, &pCmdQElmt);
			if (pCmdQElmt) {
				if (pCmdQElmt->CmdFromNdis == TRUE) {
					if (pCmdQElmt->buffer != NULL)
						os_free_mem(pCmdQElmt->buffer);
					os_free_mem(pCmdQElmt);
				} else {
					if ((pCmdQElmt->buffer != NULL) && (pCmdQElmt->bufferlength != 0))
						os_free_mem(pCmdQElmt->buffer);
					os_free_mem(pCmdQElmt);
				}
			}
		}

		NdisReleaseSpinLock(&pAd->CmdQLock);
	}
	/* notify the exit routine that we're actually exiting now
	 *
	 * complete()/wait_for_completion() is similar to up()/down(),
	 * except that complete() is safe in the case where the structure
	 * is getting deleted in a parallel mode of execution (i.e. just
	 * after the down() -- that's necessary for the thread-shutdown
	 * case.
	 *
	 * complete_and_exit() goes even further than this -- it is safe in
	 * the case that the thread of the caller is going away (not just
	 * the structure) -- this is necessary for the module-remove case.
	 * This is important in preemption kernels, which transfer the flow
	 * of execution immediately upon a complete().
	 */
	DBGPRINT(RT_DEBUG_TRACE,( "<---RTUSBCmdThread\n"));

	RtmpOSTaskNotifyToExit(pTask);
	return 0;

}

void InitUSBDevice(RT_CMD_USB_INIT *config, VOID *ad_src)
{
	PRTMP_ADAPTER ad = (PRTMP_ADAPTER)ad_src;
	ad->infType = RTMP_DEV_INF_USB;

	RTMP_SEM_EVENT_INIT(&(ad->UsbVendorReq_semaphore), &ad->RscSemMemList);
	RTMP_SEM_EVENT_INIT(&(ad->reg_atomic), &ad->RscSemMemList);
	RTMP_SEM_EVENT_INIT(&(ad->hw_atomic), &ad->RscSemMemList);
	RTMP_SEM_EVENT_INIT(&(ad->mcu_atomic), &ad->RscSemMemList);

	ad->UsbVendorReqBuf = os_alloc_mem(MAX_PARAM_BUFFER_SIZE - 1);
	if (ad->UsbVendorReqBuf == NULL) {
		return;
	}

#ifdef RLT_MAC
	if (config->driver_info == RLT_MAC_BASE) {
		UINT32 value;
		RTMP_IO_READ32(ad, 0x00, &value);
		ad->ChipID = value;
#ifdef RT65xx
	if (IS_RT65XX(ad))
		rlt_wlan_chip_onoff(ad, TRUE, TRUE);
#endif
#ifdef MT76x2
	if (IS_MT76x2(ad))
		mt76x2_pwrOn(ad);
#endif
	}
#endif

	if (RtmpRaDevCtrlInit(ad, ad->infType)) {
		DBGPRINT(RT_DEBUG_TRACE, ("%s: Success\n",__FUNCTION__));
	} else {
		DBGPRINT(RT_DEBUG_ERROR, ("%s: ERROR ignored! \n",__FUNCTION__));
	}
}

