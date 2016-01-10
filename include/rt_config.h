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

	Module Name:
	rt_config.h

	Abstract:
	Central header file to maintain all include files for all NDIS
	miniport driver routines.

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
	Paul Lin    08-01-2002    created

*/
#ifndef	__RT_CONFIG_H__
#define	__RT_CONFIG_H__

#include "rtmp_comm.h"
#include "rtmp_def.h"
#include "rtmp_chip.h"
#include "rtmp_timer.h"

#ifdef AGS_SUPPORT
#include "ags.h"
#endif

#include "mlme.h"
#include "crypt_md5.h"
#include "crypt_sha2.h"
#include "crypt_hmac.h"
#include "crypt_aes.h"
#include "crypt_arc4.h"
#include "rtmp.h"
#include "wpa.h"
#include "chlist.h"
#include "spectrum.h"

#ifdef CONFIG_AP_SUPPORT
#include "ap.h"
#include "ap_autoChSel.h"
#endif

#include "rt_os_util.h"

#include "eeprom.h"
#if defined(RTMP_PCI_SUPPORT) || defined(RTMP_USB_SUPPORT)
#include "mcu/mcu.h"
#endif

#undef AP_WSC_INCLUDED
#undef STA_WSC_INCLUDED
#undef WSC_INCLUDED

#include "rt_os_net.h"

#ifdef UAPSD_SUPPORT
#include "uapsd.h"
#endif

#ifdef CONFIG_AP_SUPPORT
#ifdef MBSS_SUPPORT
#include "ap_mbss.h"
#endif

#ifdef APCLI_SUPPORT
#include "ap_apcli.h"
#endif

#include "ap_ids.h"
#include "ap_cfg.h"

#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
#include "sta.h"
#endif

#ifdef BLOCK_NET_IF
#include "netif_block.h"
#endif

#ifdef RALINK_ATE
#include "rt_ate.h"
#endif

#if defined(AP_WSC_INCLUDED) || defined(STA_WSC_INCLUDED)
#define WSC_INCLUDED
#endif

#ifdef APCLI_WPA_SUPPLICANT_SUPPORT
#ifndef APCLI_SUPPORT
#error "Build Apcli for being controlled by NetworkManager or wext, please set HAS_APCLI_SUPPORT=y and HAS_APCLI_WPA_SUPPLICANT=y"
#endif /* APCLI_SUPPORT */
#define WPA_SUPPLICANT_SUPPORT
#endif /* APCLI_WPA_SUPPLICANT_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
#ifdef NATIVE_WPA_SUPPLICANT_SUPPORT
#ifndef WPA_SUPPLICANT_SUPPORT
#error "Build for being controlled by NetworkManager or wext, please set HAS_WPA_SUPPLICANT=y and HAS_NATIVE_WPA_SUPPLICANT_SUPPORT=y"
#endif /* WPA_SUPPLICANT_SUPPORT */
#endif /* NATIVE_WPA_SUPPLICANT_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */

#ifdef DOT11W_PMF_SUPPORT
#include "pmf.h"
#endif

#ifdef DOT11_VHT_AC
#include "vht.h"
#endif

#ifdef CONFIG_STA_SUPPORT
#include "sta_cfg.h"
#endif

#ifdef WORKQUEUE_BH
#include <linux/workqueue.h>
#endif

#ifdef RTMP_FREQ_CALIBRATION_SUPPORT
#include "frq_cal.h"
#endif

#ifdef TXBF_SUPPORT
#include "rt_txbf.h"
#endif

#ifdef RLT_MAC
#include "mac_ral/fce.h"
#endif

#ifdef RT_CFG80211_SUPPORT
#include "cfg80211extr.h"
#include "cfg80211_cmm.h"
#endif

#endif	/* __RT_CONFIG_H__ */

