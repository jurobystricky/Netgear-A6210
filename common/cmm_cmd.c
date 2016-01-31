/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2006, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in	any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

 	Module Name:
	cmm_cmd.c

	Abstract:
	All command related API.

	Revision History:
	Who	When	What
	------------------  ----------------------------------------------
	Name	Date    Modification logs
	Paul Lin    06-25-2004  created
*/

#include "rt_config.h"

void RTInitializeCmdQ(PCmdQ cmdq)
{
	cmdq->head = NULL;
	cmdq->tail = NULL;
	cmdq->size = 0;
	cmdq->CmdQState = RTMP_TASK_STAT_INITED;
}


void RTThreadDequeueCmd(PCmdQ cmdq, PCmdQElmt *pcmdqelmt)
{
	*pcmdqelmt = cmdq->head;

	if (*pcmdqelmt != NULL) {
		cmdq->head = cmdq->head->next;
		cmdq->size--;
		if (cmdq->size == 0)
			cmdq->tail = NULL;
	}
}

NDIS_STATUS RTEnqueueInternalCmd(PRTMP_ADAPTER pAd, NDIS_OID Oid,
	PVOID pInformationBuffer, UINT32 InformationBufferLength)
{
	NDIS_STATUS status;
	PCmdQElmt cmdqelmt;

	cmdqelmt = os_alloc_mem(sizeof(CmdQElmt));
	if (cmdqelmt == NULL)
		return NDIS_STATUS_RESOURCES;

	NdisZeroMemory(cmdqelmt, sizeof(CmdQElmt));

	if (InformationBufferLength > 0) {
		cmdqelmt->buffer = os_alloc_mem(InformationBufferLength);
		if (cmdqelmt->buffer == NULL) {
			os_free_mem(cmdqelmt);
			return NDIS_STATUS_RESOURCES;
		} else {
			NdisMoveMemory(cmdqelmt->buffer, pInformationBuffer,
					InformationBufferLength);
			cmdqelmt->bufferlength = InformationBufferLength;
		}
	} else {
		cmdqelmt->buffer = NULL;
		cmdqelmt->bufferlength = 0;
	}

	cmdqelmt->command = Oid;
	cmdqelmt->CmdFromNdis = FALSE;

	if (cmdqelmt != NULL) {
		NdisAcquireSpinLock(&pAd->CmdQLock);
		if (pAd->CmdQ.CmdQState & RTMP_TASK_CAN_DO_INSERT) {
			EnqueueCmd((&pAd->CmdQ), cmdqelmt);
			status = NDIS_STATUS_SUCCESS;
		} else {
			status = NDIS_STATUS_FAILURE;
		}
		NdisReleaseSpinLock(&pAd->CmdQLock);

		if (status == NDIS_STATUS_FAILURE) {
			if (cmdqelmt->buffer)
				os_free_mem(cmdqelmt->buffer);
			os_free_mem(cmdqelmt);
		} else {
			RtmpOsCmdUp(&pAd->cmdQTask);
		}
	}
	return NDIS_STATUS_SUCCESS;
}
