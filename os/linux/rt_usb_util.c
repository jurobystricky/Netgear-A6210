/****************************************************************************

    Module Name:
	rt_usb_util.c

    Abstract:
	Any utility is used in UTIL module for USB function.


***************************************************************************/

#define RTMP_MODULE_OS
#define RTMP_MODULE_OS_UTIL

#include "rtmp_comm.h"
#include "rt_os_util.h"

#ifdef RTMP_MAC_USB

/*
========================================================================
Routine Description:
	Dump URB information.

Arguments:
	purb_org	- the URB

Return Value:
	None

Note:
========================================================================
*/
#if 0 //JB removed
void dump_urb(void *purb_org)
{
	struct urb *purb = (struct urb *)purb_org;

	printk("urb                  :0x%08lx\n", (unsigned long)purb);
	printk("\tdev                   :0x%08lx\n", (unsigned long)purb->dev);
	printk("\t\tdev->state          :0x%d\n", purb->dev->state);
	printk("\tpipe                  :0x%08x\n", purb->pipe);
	printk("\tstatus                :%d\n", purb->status);
	printk("\ttransfer_flags        :0x%08x\n", purb->transfer_flags);
	printk("\ttransfer_buffer       :0x%08lx\n", (unsigned long)purb->transfer_buffer);
	printk("\ttransfer_buffer_length:%d\n", purb->transfer_buffer_length);
	printk("\tactual_length         :%d\n", purb->actual_length);
	printk("\tsetup_packet          :0x%08lx\n", (unsigned long)purb->setup_packet);
	printk("\tstart_frame           :%d\n", purb->start_frame);
	printk("\tnumber_of_packets     :%d\n", purb->number_of_packets);
	printk("\tinterval              :%d\n", purb->interval);
	printk("\terror_count           :%d\n", purb->error_count);
	printk("\tcontext               :0x%08lx\n", (unsigned long)purb->context);
	printk("\tcomplete              :0x%08lx\n\n", (unsigned long)purb->complete);
}
#endif

#ifdef CONFIG_STA_SUPPORT
#ifdef CONFIG_PM
#ifdef USB_SUPPORT_SELECTIVE_SUSPEND

static void rausb_autopm_put_interface(void *intf)
{
	usb_autopm_put_interface((struct usb_interface *)intf);
}

static int rausb_autopm_get_interface(void *intf)
{
	return usb_autopm_get_interface((struct usb_interface *)intf);
}


/*
========================================================================
Routine Description:
	RTMP_Usb_AutoPM_Put_Interface

Arguments:


Return Value:


Note:
========================================================================
*/

int RTMP_Usb_AutoPM_Put_Interface (void *pUsb_Devsrc, void *intfsrc)
{
	int pm_usage_cnt;
	struct usb_interface *intf = (struct usb_interface *)intfsrc;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
		pm_usage_cnt = atomic_read(&intf->pm_usage_cnt);
#else
		pm_usage_cnt = intf->pm_usage_cnt;
#endif

		if (pm_usage_cnt == 1) {
			rausb_autopm_put_interface(intf);
		}

	return 0;
}

EXPORT_SYMBOL(RTMP_Usb_AutoPM_Put_Interface);

/*
========================================================================
Routine Description:
	RTMP_Usb_AutoPM_Get_Interface

Arguments:


Return Value:
		(-1)  error (resume fail )
		1 success (resume success)
		2 (do nothing)

Note:
========================================================================
*/
#warning "This needs to be fixed"
/* http://www.spinics.net/lists/linux-usb/msg35888.html */
int RTMP_Usb_AutoPM_Get_Interface(void *pUsb_Devsrc, void *intfsrc)
{
	int pm_usage_cnt;
	struct usb_interface *intf = (struct usb_interface *)intfsrc;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
	pm_usage_cnt = (int)atomic_read(&intf->pm_usage_cnt);
#else
	pm_usage_cnt = intf->pm_usage_cnt;
#endif

	if (pm_usage_cnt == 0) {
		int res = rausb_autopm_get_interface(intf);
		if (res) {
			DBGPRINT(RT_DEBUG_ERROR,
					("AsicSwitchChannel autopm_resume fail ------\n"));
			return (-1);
		}
	}
	return 2;
}

EXPORT_SYMBOL(RTMP_Usb_AutoPM_Get_Interface);

#endif /* USB_SUPPORT_SELECTIVE_SUSPEND */
#endif /* CONFIG_PM */
#endif /* CONFIG_STA_SUPPORT */


/*
========================================================================
Routine Description:
	Get USB Vendor ID.

Arguments:
	pUsbDev		- the usb device

Return Value:
	the name

Note:
========================================================================
*/
UINT32 RtmpOsGetUsbDevVendorID(void *pUsbDev) {
	return ((struct usb_device *) pUsbDev)->descriptor.idVendor;
}

/*
========================================================================
Routine Description:
	Get USB Product ID.

Arguments:
	pUsbDev		- the usb device

Return Value:
	the name

Note:
========================================================================
*/
UINT32 RtmpOsGetUsbDevProductID(void *pUsbDev) {
	return ((struct usb_device *) pUsbDev)->descriptor.idProduct;
}

#endif /* RTMP_MAC_USB */

