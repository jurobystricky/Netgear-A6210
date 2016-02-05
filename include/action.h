/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source	code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	action.h

*/

#ifndef __ACTION_H__
#define __ACTION_H__

#ifdef DOT11_N_SUPPORT
void PeerAddBAReqAction(PRTMP_ADAPTER pAd, MLME_QUEUE_ELEM *Elem);
void PeerAddBARspAction(PRTMP_ADAPTER pAd, MLME_QUEUE_ELEM *Elem);
void PeerDelBAAction(PRTMP_ADAPTER pAd, MLME_QUEUE_ELEM *Elem);
#endif

#endif /* __ACTION_H__ */
