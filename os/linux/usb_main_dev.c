/****************************************************************************
 * Ralink Tech Inc.
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************/

#define RTMP_MODULE_OS

#include "rtmp_comm.h"
#include "rt_os_util.h"
#include "rt_os_net.h"

extern USB_DEVICE_ID rtusb_dev_id[];

static BOOLEAN USBDevConfigInit(struct usb_device *dev, struct usb_interface *intf, void *pAd);

#ifndef PF_NOFREEZE
#define PF_NOFREEZE  0
#endif


static void rtusb_vendor_specific_check(struct usb_device *dev, void *pAd)
{
	RT_CMD_USB_MORE_FLAG_CONFIG Config = {
		dev->descriptor.idVendor, dev->descriptor.idProduct };
	RTMP_DRIVER_USB_MORE_FLAG_SET(pAd, &Config);
}


static int rt2870_probe(struct usb_interface *intf, struct usb_device *usb_dev,
	const USB_DEVICE_ID *dev_id, void **ppAd)
{
	struct net_device *net_dev = NULL;
	void *pAd = (void *) NULL;
	int status, rv;
	PVOID handle;
	RTMP_OS_NETDEV_OP_HOOK netDevHook;
	ULONG OpMode;
#ifdef CONFIG_PM
#ifdef USB_SUPPORT_SELECTIVE_SUSPEND
	int res = 1 ;
#endif /* USB_SUPPORT_SELECTIVE_SUSPEND */
#endif /* CONFIG_PM */

	DBGPRINT(RT_DEBUG_TRACE, ("===>rt2870_probe()!\n"));

#ifdef CONFIG_PM
#ifdef USB_SUPPORT_SELECTIVE_SUSPEND
        res = usb_autopm_get_interface(intf);
	if (res) {
		DBGPRINT(RT_DEBUG_ERROR, ("rt2870_probe autopm_resume fail ------\n"));
		return -EIO;
	}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
	atomic_set(&intf->pm_usage_cnt, 1);
	printk(" rt2870_probe ====> pm_usage_cnt %d \n", atomic_read(&intf->pm_usage_cnt));
#else
	intf->pm_usage_cnt = 1;
	printk(" rt2870_probe ====> pm_usage_cnt %d \n", intf->pm_usage_cnt);
#endif

#endif /* USB_SUPPORT_SELECTIVE_SUSPEND */
#endif /* CONFIG_PM */

	handle = os_alloc_mem(sizeof(struct os_cookie));
	if (handle == NULL) {
		printk("rt2870_probe(): Allocate memory for os handle failed!\n");
		return -ENOMEM;
	}
	memset(handle, 0, sizeof(struct os_cookie));

	((POS_COOKIE)handle)->pUsb_Dev = usb_dev;

#ifdef CONFIG_PM
#ifdef USB_SUPPORT_SELECTIVE_SUSPEND
	((POS_COOKIE)handle)->intf = intf;
#endif /* USB_SUPPORT_SELECTIVE_SUSPEND */
#endif /* CONFIG_PM */

	rv = RTMPAllocAdapterBlock(handle, &pAd);
	if (rv != NDIS_STATUS_SUCCESS) {
		os_free_mem(handle);
		goto err_out;
	}

	if (USBDevConfigInit(usb_dev, intf, pAd) == FALSE)
		goto err_out_free_radev;

	RTMP_DRIVER_USB_INIT(pAd, usb_dev, dev_id->driver_info);

	net_dev = RtmpPhyNetDevInit(pAd, &netDevHook);
	if (net_dev == NULL)
		goto err_out_free_radev;

	/* Here are the net_device structure with usb specific parameters. */
#ifdef NATIVE_WPA_SUPPLICANT_SUPPORT
	/* for supporting Network Manager.
	  * Set the sysfs physical device reference for the network logical device if set prior to registration will
	  * cause a symlink during initialization.
	 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
	SET_NETDEV_DEV(net_dev, &(usb_dev->dev));
#endif
#endif /* NATIVE_WPA_SUPPLICANT_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
/*    pAd->StaCfg.OriDevType = net_dev->type; */
	RTMP_DRIVER_STA_DEV_TYPE_SET(pAd, net_dev->type);
#endif

/*All done, it's time to register the net device to linux kernel. */
	/* Register this device */
#ifdef RT_CFG80211_SUPPORT
/*	pAd->pCfgDev = &(usb_dev->dev); */
/*	pAd->CFG80211_Register = CFG80211_Register; */
/*	RTMP_DRIVER_CFG80211_INIT(pAd, usb_dev); */

	/*
		In 2.6.32, cfg80211 register must be before register_netdevice();
		We can not put the register in rt28xx_open();
		Or you will suffer NULL pointer in list_add of
		cfg80211_netdev_notifier_call().
	*/
	CFG80211_Register(pAd, &(usb_dev->dev), net_dev);
#endif /* RT_CFG80211_SUPPORT */

	RTMP_DRIVER_OP_MODE_GET(pAd, &OpMode);
	status = RtmpOSNetDevAttach(OpMode, net_dev, &netDevHook);
	if (status != 0)
		goto err_out_free_netdev;

	*ppAd = pAd;

#ifdef INF_PPA_SUPPORT
	RTMP_DRIVER_INF_PPA_INIT(pAd);
#endif

#ifdef PRE_ASSIGN_MAC_ADDR
{
	UCHAR PermanentAddress[MAC_ADDR_LEN];
	RTMP_DRIVER_MAC_ADDR_GET(pAd, &PermanentAddress[0]);
	DBGPRINT(RT_DEBUG_TRACE, ("%s():MAC Addr - %02x:%02x:%02x:%02x:%02x:%02x\n",
			__FUNCTION__, PermanentAddress[0], PermanentAddress[1],
			PermanentAddress[2],PermanentAddress[3],
			PermanentAddress[4],PermanentAddress[5]));

	/* Set up the Mac address */
	RtmpOSNetDevAddrSet(OpMode, net_dev, &PermanentAddress[0], NULL);
}
#endif /* PRE_ASSIGN_MAC_ADDR */

#ifdef EXT_BUILD_CHANNEL_LIST
	RTMP_DRIVER_SET_PRECONFIG_VALUE(pAd);
#endif

	DBGPRINT(RT_DEBUG_TRACE, ("<===rt2870_probe()!\n"));

	return 0;

err_out_free_netdev:
	RtmpOSNetDevFree(net_dev);

err_out_free_radev:
	RTMPFreeAdapter(pAd);

err_out:
	*ppAd = NULL;

	return -1;
}


/*
========================================================================
Routine Description:
    Release allocated resources.

Arguments:
	*dev		Point to the PCI or USB device
	pAd		driver control block pointer

Return Value:
    None

Note:
========================================================================
*/
static void rt2870_disconnect(struct usb_device *dev, void *pAd)
{
	struct net_device *net_dev;

	DBGPRINT(RT_DEBUG_WARN,
			("%s: unregister usbnet usb-%s-%s\n",
			__FUNCTION__,dev->bus->bus_name, dev->devpath));
	if (!pAd) {
		usb_put_dev(dev);
		printk("rtusb_disconnect: pAd == NULL!\n");
		return;
	}
/*	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST); */
	RTMP_DRIVER_NIC_NOT_EXIST_SET(pAd);

	/* for debug, wait to show some messages to /proc system */
	udelay(1);

	RTMP_DRIVER_NET_DEV_GET(pAd, &net_dev);
	RtmpPhyNetDevExit(pAd, net_dev);

	/* FIXME: Shall we need following delay and flush the schedule?? */
	udelay(1);
	flush_scheduled_work();
	udelay(1);

#ifdef RT_CFG80211_SUPPORT
	RTMP_DRIVER_80211_UNREGISTER(pAd, net_dev);
#endif

	/* free the root net_device */
//	RtmpOSNetDevFree(net_dev);

	RtmpRaDevCtrlExit(pAd);

	/* free the root net_device */
	RtmpOSNetDevFree(net_dev);

	/* release a use of the usb device structure */
	usb_put_dev(dev);
	udelay(1);

	DBGPRINT(RT_DEBUG_OFF, (" RTUSB disconnect successfully\n"));
}


#ifdef CONFIG_PM
static int rtusb_suspend(struct usb_interface *intf, pm_message_t state)
{
	struct net_device *net_dev;
	void *pAd = usb_get_intfdata(intf);

#ifdef USB_SUPPORT_SELECTIVE_SUSPEND
	UCHAR Flag;
	DBGPRINT(RT_DEBUG_ERROR, ("%s():=>autosuspend\n", __FUNCTION__));
#ifdef WOW_SUPPORT
	RTMP_DRIVER_ADAPTER_RT28XX_USB_WOW_STATUS(pAd, &Flag);
	if (Flag == TRUE)
		RTMP_DRIVER_ADAPTER_RT28XX_USB_WOW_ENABLE(pAd);
	else
#endif /* WOW_SUPPORT */
	{
/*	if(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF)) */
		RTMP_DRIVER_ADAPTER_END_DISSASSOCIATE(pAd);
		RTMP_DRIVER_ADAPTER_IDLE_RADIO_OFF_TEST(pAd, &Flag);
		if (!Flag) {
			/*RT28xxUsbAsicRadioOff(pAd); */
			RTMP_DRIVER_ADAPTER_RT28XX_USB_ASICRADIO_OFF(pAd);
		}
	}
	/*RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_SUSPEND); */
	RTMP_DRIVER_ADAPTER_SUSPEND_SET(pAd);
	return 0;
#endif /* USB_SUPPORT_SELECTIVE_SUSPEND */

	DBGPRINT(RT_DEBUG_TRACE, ("%s()=>\n", __FUNCTION__));
/*	net_dev = pAd->net_dev; */
	RTMP_DRIVER_NET_DEV_GET(pAd, &net_dev);
	netif_device_detach(net_dev);

	RTMP_DRIVER_USB_SUSPEND(pAd, netif_running(net_dev));
	DBGPRINT(RT_DEBUG_TRACE, ("<=%s()\n", __FUNCTION__));
	return 0;
}


static int rtusb_resume(struct usb_interface *intf)
{
	struct net_device *net_dev;
	void *pAd = usb_get_intfdata(intf);

#ifdef USB_SUPPORT_SELECTIVE_SUSPEND
	int pm_usage_cnt;
	UCHAR Flag;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
	pm_usage_cnt = atomic_read(&intf->pm_usage_cnt);
#else
	pm_usage_cnt = intf->pm_usage_cnt;
#endif
	if(pm_usage_cnt  <= 0)
		usb_autopm_get_interface(intf);

	DBGPRINT(RT_DEBUG_ERROR, ("%s():=>autosuspend\n", __FUNCTION__));

	/*RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_SUSPEND); */
	RTMP_DRIVER_ADAPTER_SUSPEND_CLEAR(pAd);

#ifdef WOW_SUPPORT
	RTMP_DRIVER_ADAPTER_RT28XX_USB_WOW_STATUS(pAd, &Flag);
	if (Flag == TRUE)
		RTMP_DRIVER_ADAPTER_RT28XX_USB_WOW_DISABLE(pAd);
	else
#endif /* WOW_SUPPORT */
		/*RT28xxUsbAsicRadioOn(pAd); */
		RTMP_DRIVER_ADAPTER_RT28XX_USB_ASICRADIO_ON(pAd);

	DBGPRINT(RT_DEBUG_ERROR, ("%s(): <=autosuspend\n", __FUNCTION__));

	return 0;
#endif /* USB_SUPPORT_SELECTIVE_SUSPEND */

	DBGPRINT(RT_DEBUG_TRACE, ("%s()=>\n", __FUNCTION__));

/*	pAd->PM_FlgSuspend = 0; */
	RTMP_DRIVER_USB_RESUME(pAd);

/*	net_dev = pAd->net_dev; */
	RTMP_DRIVER_NET_DEV_GET(pAd, &net_dev);
	netif_device_attach(net_dev);
	netif_start_queue(net_dev);
	netif_carrier_on(net_dev);
	netif_wake_queue(net_dev);

	DBGPRINT(RT_DEBUG_TRACE, ("<=%s()\n", __FUNCTION__));
	return 0;
}
#endif /* CONFIG_PM */


static BOOLEAN USBDevConfigInit(struct usb_device *dev, struct usb_interface *intf, void *pAd)
{
	struct usb_host_interface *iface_desc;
	ULONG BulkOutIdx;
	ULONG BulkInIdx;
	UINT32 i;
	RT_CMD_USB_DEV_CONFIG Config, *pConfig = &Config;

	/* get the active interface descriptor */
	iface_desc = intf->cur_altsetting;

	/* get # of enpoints  */
	pConfig->NumberOfPipes = iface_desc->desc.bNumEndpoints;
	DBGPRINT(RT_DEBUG_TRACE, ("NumEndpoints=%d\n", iface_desc->desc.bNumEndpoints));

	/* Configure Pipes */
	BulkOutIdx = 0;
	BulkInIdx = 0;

	for (i = 0; i < pConfig->NumberOfPipes; i++) {
		if ((iface_desc->endpoint[i].desc.bmAttributes == USB_ENDPOINT_XFER_BULK) &&
			((iface_desc->endpoint[i].desc.bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_IN)) {
			if (BulkInIdx < 2) {
				pConfig->BulkInEpAddr[BulkInIdx++] = iface_desc->endpoint[i].desc.bEndpointAddress;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,11)
				pConfig->BulkInMaxPacketSize = le2cpu16(iface_desc->endpoint[i].desc.wMaxPacketSize);
#else
				pConfig->BulkInMaxPacketSize = iface_desc->endpoint[i].desc.wMaxPacketSize;
#endif /* LINUX_VERSION_CODE */

				DBGPRINT(RT_DEBUG_TRACE,
						("BULK IN MaxPacketSize = %d\n",
						pConfig->BulkInMaxPacketSize));
				DBGPRINT(RT_DEBUG_TRACE,
						("EP address = 0x%2x\n",
						iface_desc->endpoint[i].desc.bEndpointAddress));
			} else {
				DBGPRINT(RT_DEBUG_ERROR, ("Bulk IN endpoint nums large than 2\n"));
			}
		} else if ((iface_desc->endpoint[i].desc.bmAttributes == USB_ENDPOINT_XFER_BULK) &&
			((iface_desc->endpoint[i].desc.bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_OUT)) {
			if (BulkOutIdx < 6) {
				/* there are 6 bulk out EP. EP6 highest priority. */
				/* EP1-4 is EDCA.  EP5 is HCCA. */
				pConfig->BulkOutEpAddr[BulkOutIdx++] = iface_desc->endpoint[i].desc.bEndpointAddress;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,11)
				pConfig->BulkOutMaxPacketSize = le2cpu16(iface_desc->endpoint[i].desc.wMaxPacketSize);
#else
				pConfig->BulkOutMaxPacketSize = iface_desc->endpoint[i].desc.wMaxPacketSize;
#endif
				DBGPRINT(RT_DEBUG_TRACE,
						("BULK OUT MaxPacketSize = %d\n",
						pConfig->BulkOutMaxPacketSize));
				DBGPRINT(RT_DEBUG_TRACE,
						("EP address = 0x%2x  \n",
						iface_desc->endpoint[i].desc.bEndpointAddress));
			} else {
				DBGPRINT(RT_DEBUG_ERROR,
						("Bulk Out endpoint nums large than 6\n"));
			}
		}
	}

	if (!(pConfig->BulkInEpAddr && pConfig->BulkOutEpAddr[0])) {
		printk("%s: Could not find both bulk-in and bulk-out endpoints\n", __FUNCTION__);
		return FALSE;
	}

	pConfig->pConfig = &dev->config->desc;
	usb_set_intfdata(intf, pAd);
	RTMP_DRIVER_USB_CONFIG_INIT(pAd, pConfig);
	rtusb_vendor_specific_check(dev, pAd);

	return TRUE;
}


static int rtusb_probe(struct usb_interface *intf, const USB_DEVICE_ID *id)
{
	void *pAd;
	struct usb_device *dev;
	int rv;

	dev = interface_to_usbdev(intf);
	dev = usb_get_dev(dev);

	rv = rt2870_probe(intf, dev, id, &pAd);
	if (rv != 0)
		usb_put_dev(dev);
#ifdef IFUP_IN_PROBE
	else
	{
		if (VIRTUAL_IF_UP(pAd) != 0) {
			pAd = usb_get_intfdata(intf);
			usb_set_intfdata(intf, NULL);
			rt2870_disconnect(dev, pAd);
			rv = -ENOMEM;
		}
	}
#endif /* IFUP_IN_PROBE */
	return rv;
}


static void rtusb_disconnect(struct usb_interface *intf)
{
	struct usb_device *dev = interface_to_usbdev(intf);
	void *pAd;

	pAd = usb_get_intfdata(intf);
#ifdef IFUP_IN_PROBE
	VIRTUAL_IF_DOWN(pAd);
#endif
	usb_set_intfdata(intf, NULL);
	rt2870_disconnect(dev, pAd);

#ifdef CONFIG_PM
#ifdef USB_SUPPORT_SELECTIVE_SUSPEND
	printk("rtusb_disconnect usb_autopm_put_interface \n");
	usb_autopm_put_interface(intf);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
	printk("%s() => pm_usage_cnt %d \n", __FUNCTION__,
			atomic_read(&intf->pm_usage_cnt));
#else
	printk("%s() => pm_usage_cnt %d \n", __FUNCTION__, intf->pm_usage_cnt);
#endif
#endif /* USB_SUPPORT_SELECTIVE_SUSPEND */
#endif /* CONFIG_PM */
}

static struct usb_driver rtusb_driver = {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,15)
	.owner = THIS_MODULE,
#endif
	.name = RTMP_DRV_NAME,
	.probe = rtusb_probe,
	.disconnect = rtusb_disconnect,
	.id_table = rtusb_dev_id,

#ifdef CONFIG_PM
#ifdef USB_SUPPORT_SELECTIVE_SUSPEND
	.supports_autosuspend = 1,
#endif /* USB_SUPPORT_SELECTIVE_SUSPEND */
	.suspend = rtusb_suspend,
	.resume = rtusb_resume,
#endif /* CONFIG_PM */
};

#ifdef DBG
u32 RTDebugLevel = RT_DEBUG_ERROR;
ULONG RTDebugFunc = 0;

// This directory entry will point to `/sys/kernel/debug/rt2870xx.
static struct dentry *dbgfs_dir = 0;
#endif

int __init rtusb_init(void)
{
#ifdef DBG
	struct dentry *tmp;

	printk("rtusb init %s --->\n", RTMP_DRV_NAME);
	dbgfs_dir = debugfs_create_dir(RTMP_DRV_NAME, 0);
	if (!dbgfs_dir) {
		printk(KERN_ALERT "debugfs_create_dir failed\n");
	}

	tmp = debugfs_create_u32("RTDebugLevel", S_IWUSR | S_IRUGO, dbgfs_dir,
			&RTDebugLevel);
	if (!tmp) {
		printk(KERN_ALERT "debugfs_create_u32 failed\n");
	}
#endif
	return usb_register(&rtusb_driver);
}


void __exit rtusb_exit(void)
{
	usb_deregister(&rtusb_driver);
#ifdef DBG
	debugfs_remove_recursive(dbgfs_dir);
#endif
	printk("<--- rtusb exit\n");
}

#ifndef MULTI_INF_SUPPORT

module_init(rtusb_init);
module_exit(rtusb_exit);

/* Following information will be show when you run 'modinfo' */
/* *** If you have a solution for the bug in current version of driver, please mail to me. */
/* Otherwise post to forum in ralinktech's web site(www.ralinktech.com) and let all users help you. *** */
MODULE_AUTHOR("Ralink");
MODULE_DESCRIPTION("Ralink Wireless Lan Linux Driver");
MODULE_LICENSE("GPL");

#ifdef CONFIG_STA_SUPPORT
#ifdef MODULE_VERSION
MODULE_VERSION(STA_DRIVER_VERSION);
#endif
#endif /* CONFIG_STA_SUPPORT */

#endif /* MULTI_INF_SUPPORT */

