/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2008, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    rtmp_timer.c

    Abstract:
    task for timer handling

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    Name          Date          Modification logs
    Shiang Tu     08-28-2008    init version

*/

#include "rt_config.h"


BUILD_TIMER_FUNCTION(MlmePeriodicExec);
/*BUILD_TIMER_FUNCTION(MlmeRssiReportExec);*/
BUILD_TIMER_FUNCTION(AsicRxAntEvalTimeout);
BUILD_TIMER_FUNCTION(APSDPeriodicExec);
BUILD_TIMER_FUNCTION(EnqueueStartForPSKExec);

#ifdef DOT11W_PMF_SUPPORT
BUILD_TIMER_FUNCTION(PMF_SAQueryTimeOut);
BUILD_TIMER_FUNCTION(PMF_SAQueryConfirmTimeOut);
#endif /* DOT11W_PMF_SUPPORT */

#ifdef RTMP_MAC_USB
BUILD_TIMER_FUNCTION(BeaconUpdateExec);
#endif

#ifdef CONFIG_AP_SUPPORT
extern void APDetectOverlappingExec(PVOID SystemSpecific1, PVOID FunctionContext,
	PVOID SystemSpecific2, PVOID SystemSpecific3);

BUILD_TIMER_FUNCTION(APDetectOverlappingExec);

#ifdef DOT11N_DRAFT3
BUILD_TIMER_FUNCTION(Bss2040CoexistTimeOut);
#endif

BUILD_TIMER_FUNCTION(GREKEYPeriodicExec);
BUILD_TIMER_FUNCTION(CMTimerExec);
BUILD_TIMER_FUNCTION(WPARetryExec);

#ifdef AP_SCAN_SUPPORT
BUILD_TIMER_FUNCTION(APScanTimeout);
#endif

BUILD_TIMER_FUNCTION(APQuickResponeForRateUpExec);

#ifdef DROP_MASK_SUPPORT
BUILD_TIMER_FUNCTION(drop_mask_timer_action);
#endif

#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
BUILD_TIMER_FUNCTION(BeaconTimeout);
BUILD_TIMER_FUNCTION(ScanTimeout);
BUILD_TIMER_FUNCTION(AuthTimeout);
BUILD_TIMER_FUNCTION(AssocTimeout);
BUILD_TIMER_FUNCTION(ReassocTimeout);
BUILD_TIMER_FUNCTION(DisassocTimeout);
BUILD_TIMER_FUNCTION(LinkDownExec);
BUILD_TIMER_FUNCTION(StaQuickResponeForRateUpExec);
BUILD_TIMER_FUNCTION(WpaDisassocApAndBlockAssoc);

#ifdef RTMP_MAC_USB
BUILD_TIMER_FUNCTION(RtmpUsbStaAsicForceWakeupTimeout);
#endif

#endif /* CONFIG_STA_SUPPORT */

#ifdef TXBF_SUPPORT
BUILD_TIMER_FUNCTION(eTxBfProbeTimerExec);
#endif

#ifdef RT_CFG80211_P2P_SUPPORT
BUILD_TIMER_FUNCTION(CFG80211_P2PCTWindowTimer);
BUILD_TIMER_FUNCTION(CFG80211_P2pSwNoATimeOut);
BUILD_TIMER_FUNCTION(CFG80211_P2pPreAbsenTimeOut);
#endif

#ifdef RALINK_ATE
BUILD_TIMER_FUNCTION(ATEPeriodicExec);
#endif

#ifdef APCLI_SUPPORT
BUILD_TIMER_FUNCTION(ApCliWpaDisassocApAndBlockAssoc);
#endif

#ifdef PEER_DELBA_TX_ADAPT
BUILD_TIMER_FUNCTION(PeerDelBATxAdaptTimeOut);
#endif

#ifdef RTMP_TIMER_TASK_SUPPORT
static void RtmpTimerQHandle(RTMP_ADAPTER *pAd)
{
	int status;
	RALINK_TIMER_STRUCT *pTimer;
	RTMP_TIMER_TASK_ENTRY *pEntry;
	unsigned long irqFlag;
	RTMP_OS_TASK *pTask;

	pTask = &pAd->timerTask;
	while (!RTMP_OS_TASK_IS_KILLED(pTask)) {
		pTimer = NULL;

		if (RtmpOSTaskWait(pAd, pTask, &status) == FALSE) {
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);
			break;
		}

		if (pAd->TimerQ.status == RTMP_TASK_STAT_STOPPED)
			break;

		/* event happened.*/
		while (pAd->TimerQ.pQHead) {
			RTMP_INT_LOCK(&pAd->TimerQLock, irqFlag);
			pEntry = pAd->TimerQ.pQHead;
			if (pEntry) {
				pTimer = pEntry->pRaTimer;

				/* update pQHead*/
				pAd->TimerQ.pQHead = pEntry->pNext;
				if (pEntry == pAd->TimerQ.pQTail)
					pAd->TimerQ.pQTail = NULL;

				/* return this queue entry to timerQFreeList.*/
				pEntry->pNext = pAd->TimerQ.pQPollFreeList;
				pAd->TimerQ.pQPollFreeList = pEntry;
			}
			RTMP_INT_UNLOCK(&pAd->TimerQLock, irqFlag);

			if (pTimer) {
				if ((pTimer->handle != NULL) && (!pAd->PM_FlgSuspend))
					pTimer->handle(NULL, (PVOID) pTimer->cookie, NULL, pTimer);
				if ((pTimer->Repeat) && (pTimer->State == FALSE))
					RTMP_OS_Add_Timer(&pTimer->TimerObj, pTimer->TimerValue);
			}
		}

		if (status != 0) {
			pAd->TimerQ.status = RTMP_TASK_STAT_STOPPED;
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);
			break;
		}
	}
}


int RtmpTimerQThread(ULONG Context)
{
	RTMP_OS_TASK *pTask;
	PRTMP_ADAPTER pAd;

	pTask = (RTMP_OS_TASK *)Context;

	pAd = (PRTMP_ADAPTER)RTMP_OS_TASK_DATA_GET(pTask);
	if (pAd == NULL) {
		DBGPRINT(RT_DEBUG_ERROR,( "%s:: pAd is NULL!\n",__FUNCTION__));
		return 0;
	}

	RtmpOSTaskCustomize(pTask);
	RtmpTimerQHandle(pAd);

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
	RtmpOSTaskNotifyToExit(pTask);

	DBGPRINT(RT_DEBUG_TRACE,( "<---%s\n",__FUNCTION__));
	return 0;
}


RTMP_TIMER_TASK_ENTRY *RtmpTimerQInsert(RTMP_ADAPTER *pAd,
	RALINK_TIMER_STRUCT *pTimer)
{
	RTMP_TIMER_TASK_ENTRY *pQNode = NULL, *pQTail;
	unsigned long irqFlags;
	RTMP_OS_TASK *pTask = &pAd->timerTask;

	RTMP_INT_LOCK(&pAd->TimerQLock, irqFlags);
	if (pAd->TimerQ.status & RTMP_TASK_CAN_DO_INSERT) {
		if(pAd->TimerQ.pQPollFreeList) {
			pQNode = pAd->TimerQ.pQPollFreeList;
			pAd->TimerQ.pQPollFreeList = pQNode->pNext;

			pQNode->pRaTimer = pTimer;
			pQNode->pNext = NULL;

			pQTail = pAd->TimerQ.pQTail;
			if (pAd->TimerQ.pQTail != NULL)
				pQTail->pNext = pQNode;
			pAd->TimerQ.pQTail = pQNode;
			if (pAd->TimerQ.pQHead == NULL)
				pAd->TimerQ.pQHead = pQNode;
		}
	}
	RTMP_INT_UNLOCK(&pAd->TimerQLock, irqFlags);

	if (pQNode) {
		RTMP_OS_TASK_WAKE_UP(pTask);
	}

	return pQNode;
}


BOOLEAN RtmpTimerQRemove(RTMP_ADAPTER *pAd, RALINK_TIMER_STRUCT *pTimer)
{
	RTMP_TIMER_TASK_ENTRY *pNode, *pPrev = NULL;
	unsigned long irqFlags;

	RTMP_INT_LOCK(&pAd->TimerQLock, irqFlags);
	if (pAd->TimerQ.status >= RTMP_TASK_STAT_INITED) {
		pNode = pAd->TimerQ.pQHead;
		while (pNode) {
			if (pNode->pRaTimer == pTimer)
				break;
			pPrev = pNode;
			pNode = pNode->pNext;
		}

		/* Now move it to freeList queue.*/
		if (pNode) {
			if (pNode == pAd->TimerQ.pQHead)
				pAd->TimerQ.pQHead = pNode->pNext;
			if (pNode == pAd->TimerQ.pQTail)
				pAd->TimerQ.pQTail = pPrev;
			if (pPrev != NULL)
				pPrev->pNext = pNode->pNext;

			/* return this queue entry to timerQFreeList.*/
			pNode->pNext = pAd->TimerQ.pQPollFreeList;
			pAd->TimerQ.pQPollFreeList = pNode;
		}
	}
	RTMP_INT_UNLOCK(&pAd->TimerQLock, irqFlags);

	return TRUE;
}


void RtmpTimerQExit(RTMP_ADAPTER *pAd)
{
	RTMP_TIMER_TASK_ENTRY *pTimerQ;
	unsigned long irqFlags;

	RTMP_INT_LOCK(&pAd->TimerQLock, irqFlags);
	while (pAd->TimerQ.pQHead) {
		pTimerQ = pAd->TimerQ.pQHead;
		pAd->TimerQ.pQHead = pTimerQ->pNext;
		/* remove the timeQ*/
	}
	pAd->TimerQ.pQPollFreeList = NULL;
	os_free_mem(pAd->TimerQ.pTimerQPoll);
	pAd->TimerQ.pQTail = NULL;
	pAd->TimerQ.pQHead = NULL;
/*#ifndef KTHREAD_SUPPORT*/
	pAd->TimerQ.status = RTMP_TASK_STAT_STOPPED;
/*#endif*/
	RTMP_INT_UNLOCK(&pAd->TimerQLock, irqFlags);
/*	NdisFreeSpinLock(&pAd->TimerQLock); */
}


void RtmpTimerQInit(RTMP_ADAPTER *pAd)
{
	int i;
	RTMP_TIMER_TASK_ENTRY *pQNode, *pEntry;
	unsigned long irqFlags;

	NdisAllocateSpinLock(pAd, &pAd->TimerQLock);

	NdisZeroMemory(&pAd->TimerQ, sizeof(pAd->TimerQ));

	pAd->TimerQ.pTimerQPoll = os_alloc_mem(sizeof(RTMP_TIMER_TASK_ENTRY) * TIMER_QUEUE_SIZE_MAX);
	if (pAd->TimerQ.pTimerQPoll) {
		pEntry = NULL;
		pQNode = (RTMP_TIMER_TASK_ENTRY *)pAd->TimerQ.pTimerQPoll;
		NdisZeroMemory(pAd->TimerQ.pTimerQPoll, sizeof(RTMP_TIMER_TASK_ENTRY) * TIMER_QUEUE_SIZE_MAX);

		RTMP_INT_LOCK(&pAd->TimerQLock, irqFlags);
		for (i = 0; i <TIMER_QUEUE_SIZE_MAX; i++) {
			pQNode->pNext = pEntry;
			pEntry = pQNode;
			pQNode++;
		}
		pAd->TimerQ.pQPollFreeList = pEntry;
		pAd->TimerQ.pQHead = NULL;
		pAd->TimerQ.pQTail = NULL;
		pAd->TimerQ.status = RTMP_TASK_STAT_INITED;
		RTMP_INT_UNLOCK(&pAd->TimerQLock, irqFlags);
	}
}

void rtmp_timer_task_callback(struct timer_list *tl)
{
	RALINK_TIMER_STRUCT *pTimer = from_timer(pTimer, tl, TimerObj);
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *) pTimer->pAd;
	RTMP_TIMER_TASK_ENTRY *pQNode = RtmpTimerQInsert(pAd, pTimer);

	if (!pQNode && (pAd->TimerQ.status & RTMP_TASK_CAN_DO_INSERT))
		RTMP_OS_Add_Timer(&pTimer->TimerObj, OS_HZ);
}
#endif /* RTMP_TIMER_TASK_SUPPORT */

