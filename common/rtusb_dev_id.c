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
    rtusb_dev_id.c

 */

#define RTMP_MODULE_OS

#include "rtmp_comm.h"
#include "rt_os_util.h"
#include "rt_os_net.h"

/* module table */
USB_DEVICE_ID rtusb_dev_id[] = {
#ifdef MT76x2
	{USB_DEVICE(0x0846, 0x9014), .driver_info = RLT_MAC_BASE}, /* Netgear WNDA3100v3 */
	{USB_DEVICE(0x0B05, 0x180B), .driver_info = RLT_MAC_BASE}, /* ASUS USB-N53 */
	{USB_DEVICE(0x0846, 0x9053), .driver_info = RLT_MAC_BASE}, /* MT7612U, Netgear A6210 */
	{USB_DEVICE(0x0B05, 0x17EB), .driver_info = RLT_MAC_BASE}, /* ASUS USB-AC55 */
	{USB_DEVICE(0x045e, 0x02e6), .driver_info = RLT_MAC_BASE}, /* Microsoft XBox One Wireless Adapter */
	{USB_DEVICE(0x0E8D, 0x7612), .driver_info = RLT_MAC_BASE},
	{USB_DEVICE_AND_INTERFACE_INFO(0x0E8D, 0x7632, 0xff, 0xff, 0xff), .driver_info = RLT_MAC_BASE},
	{USB_DEVICE_AND_INTERFACE_INFO(0x0E8D, 0x7662, 0xff, 0xff, 0xff), .driver_info = RLT_MAC_BASE},
#endif
	{ }/* Terminating entry */
};

MODULE_DEVICE_TABLE(usb, rtusb_dev_id);
