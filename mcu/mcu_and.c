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
	mcu_and.c

	Abstract:
	on-chip CPU related codes

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#include "rt_config.h"

static void andes_init_cmd_msg(struct cmd_msg *msg, u8 type, BOOLEAN need_wait, u16 timeout,
   BOOLEAN need_retransmit, BOOLEAN need_rsp, u16 rsp_payload_len,
   char *rsp_payload, MSG_RSP_HANDLER rsp_handler);

static struct cmd_msg *andes_alloc_cmd_msg(struct _RTMP_ADAPTER *ad, unsigned int length);
static void andes_append_cmd_msg(struct cmd_msg *msg, char *data, unsigned int len);
static void andes_bh_schedule(struct _RTMP_ADAPTER *ad);

#ifdef RTMP_MAC_USB
static int andes_usb_enable_patch(RTMP_ADAPTER *ad)
{
	int ret = NDIS_STATUS_SUCCESS;
	RTMP_CHIP_CAP *cap = &ad->chipCap;

	/* enable patch command */
	u8 cmd[11];
	cmd[0] = 0x6F;
	cmd[1] = 0xFC;
	cmd[2] = 0x08;
	cmd[3] = 0x01;
	cmd[4] = 0x20;
	cmd[5] = 0x04;
	cmd[6] = 0x00;
	cmd[7] = (cap->rom_patch_offset & 0xFF);
	cmd[8] = (cap->rom_patch_offset & 0xFF00) >> 8;
	cmd[9] = (cap->rom_patch_offset & 0xFF0000) >> 16;
	cmd[10] = (cap->rom_patch_offset & 0xFF000000) >> 24;

	DBGPRINT(RT_DEBUG_TRACE, ("%s\n", __FUNCTION__));

	ret = RTUSB_VendorRequest(ad, USBD_TRANSFER_DIRECTION_OUT,
		DEVICE_CLASS_REQUEST_OUT, 0x01, 0x12, 0x00, cmd, 11);

	return ret;
}

static int andes_usb_reset_wmt(RTMP_ADAPTER *ad)
{
	int ret = NDIS_STATUS_SUCCESS;

	/* reset command */
	u8 cmd[8] = {0x6F, 0xFC, 0x05, 0x01, 0x07, 0x01, 0x00, 0x04};

	DBGPRINT(RT_DEBUG_OFF, ("%s\n", __FUNCTION__));

	RTUSB_VendorRequest(ad, USBD_TRANSFER_DIRECTION_OUT,
			DEVICE_CLASS_REQUEST_OUT, 0x01, 0x12, 0x00, cmd, 8);

	return ret;
}

static u16 checksume16(u8 *pData, int len)
{
	int sum = 0;

	while (len > 1) {
		sum += *((u16*)pData);
		pData = pData + 2;

		if (sum & 0x80000000)
			sum = (sum & 0xFFFF) + (sum >> 16);

		len -= 2;
	}

	if (len)
		sum += *((u8*)pData);

	while (sum >> 16) {
		sum = (sum & 0xFFFF) + (sum >> 16);
	}

	return ~sum;
}

static int andes_usb_chk_crc(RTMP_ADAPTER *ad, u32 checksum_len)
{
	int ret = 0;
	u8 cmd[8];
	RTMP_CHIP_CAP *cap = &ad->chipCap;

	DBGPRINT(RT_DEBUG_OFF, ("%s\n", __FUNCTION__));

	NdisMoveMemory(cmd, &cap->rom_patch_offset, 4);
	NdisMoveMemory(&cmd[4], &checksum_len, 4);

	ret = RTUSB_VendorRequest(ad, USBD_TRANSFER_DIRECTION_OUT,
			DEVICE_VENDOR_REQUEST_OUT, 0x01, 0x20, 0x00, cmd, 8);

	return ret;
}

static u16 andes_usb_get_crc(RTMP_ADAPTER *ad)
{
	int ret = 0;
	u16 crc, count = 0;

	while (1) {
		ret = RTUSB_VendorRequest(ad,
			(USBD_TRANSFER_DIRECTION_IN | USBD_SHORT_TRANSFER_OK),
			DEVICE_VENDOR_REQUEST_IN, 0x01, 0x21, 0x00, &crc, 2);

		if (crc != 0xFFFF)
			break;

		RtmpOsMsDelay(100);

		if (count++ > 100) {
			DBGPRINT(RT_DEBUG_ERROR, ("Query CRC over %d times\n", count));
			break;
		}
	}

	return crc;
}

static void usb_upload_rom_patch_complete(purbb_t urb, pregs *pt_regs)
{
	struct completion *load_rom_patch_done =
			(struct completion *)RTMP_OS_USB_CONTEXT_GET(urb);
	complete(load_rom_patch_done);
}

int andes_usb_load_rom_patch(RTMP_ADAPTER *ad)
{
	PURB urb;
	POS_COOKIE obj = (POS_COOKIE)ad->OS_Cookie;
	ra_dma_addr_t rom_patch_dma;
	PUCHAR rom_patch_data;
	TXINFO_NMAC_CMD *tx_info;
	s32 sent_len;
	u32 cur_len = 0;
	u32 mac_value;
	int i;
	u16 value;
	int ret = 0, total_checksum = 0;
	RTMP_CHIP_CAP *cap = &ad->chipCap;
	USB_DMA_CFG_STRUC cfg;
	u32 patch_len = 0;
	struct completion load_rom_patch_done;

	if (cap->rom_code_protect) {
		for (i = 0; i < GET_SEMAPHORE_RETRY_MAX; i++) {
			RTUSBReadMACRegister(ad, SEMAPHORE_03, &mac_value);
			if ((mac_value & 0x01) == 0x01)
				break;
			RtmpOsMsDelay(1);
		}

		if (i >= GET_SEMAPHORE_RETRY_MAX) {
			DBGPRINT(RT_DEBUG_ERROR,
					("%s: Failed to obtain SEMAPHORE_03\n",
					__FUNCTION__));
			return NDIS_STATUS_FAILURE;
		}
	}

	/* Check rom patch if ready */
	if (MT_REV_GTE(ad, MT76x2, REV_MT76x2E3)) {
		RTUSBReadMACRegister(ad, CLOCK_CTL, &mac_value);

		if (((mac_value & 0x01) == 0x01) && (cap->rom_code_protect)) {
			goto error0;
		}
	} else {
		RTUSBReadMACRegister(ad, COM_REG0, &mac_value);

		if (((mac_value & 0x02) == 0x02) && (cap->rom_code_protect)) {
			goto error0;
		}
	}

	/* Enable USB_DMA_CFG */
	USB_CFG_READ(ad, &cfg.word);
	cfg.word |= 0x00c00020;
	USB_CFG_WRITE(ad, cfg.word);

	if (cap->load_code_method == BIN_FILE_METHOD)
		OS_LOAD_CODE_FROM_BIN(&cap->rom_patch, cap->rom_patch_bin_file_name,
				obj->pUsb_Dev, &cap->rom_patch_len);
	else
		cap->rom_patch = cap->rom_patch_header_image;

	if (!cap->rom_patch) {
		if (cap->load_code_method == BIN_FILE_METHOD) {
			DBGPRINT(RT_DEBUG_ERROR,
					("%s:Please assign a rom patch(/lib/firmware/%s), load_method(%d)\n",
					__FUNCTION__, cap->rom_patch_bin_file_name, cap->load_code_method));
		} else {
			DBGPRINT(RT_DEBUG_ERROR, ("%s:Please assign a rom patch, load_method(%d)\n",
				__FUNCTION__, cap->load_code_method));
		}
		ret = NDIS_STATUS_FAILURE;
		goto error0;
	}

	RTUSBVendorReset(ad);

	/* get rom patch information */
	DBGPRINT(RT_DEBUG_TRACE, ("build time = \n"));

	for (i = 0; i < 16; i++)
		DBGPRINT(RT_DEBUG_TRACE, ("%c", *(cap->rom_patch + i)));

	if (IS_MT76x2(ad)) {
		if (((strncmp(cap->rom_patch, "20130809", 8) >= 0)) &&
			(MT_REV_GTE(ad, MT76x2, REV_MT76x2E3))) {
			DBGPRINT(RT_DEBUG_TRACE, ("rom patch for E3 IC\n"));
		} else if (((strncmp(cap->rom_patch, "20130809", 8) < 0)) &&
			(MT_REV_LT(ad, MT76x2, REV_MT76x2E3))){
			DBGPRINT(RT_DEBUG_TRACE, ("rom patch for E2 IC\n"));
		} else {
			DBGPRINT(RT_DEBUG_ERROR, ("rom patch does not match IC version\n"));
			RTMP_IO_READ32(ad, 0x0, &mac_value);
			DBGPRINT(RT_DEBUG_ERROR, ("IC version(%x)\n", mac_value));
			ret = NDIS_STATUS_FAILURE;
			goto error0;
		}
	}

#ifdef DBG
	DBGPRINT(RT_DEBUG_TRACE, ("\nplatform = "));

	for (i = 0; i < 4; i++)
		DBGPRINT(RT_DEBUG_TRACE, ("%c", *(cap->rom_patch + 16 + i)));

	DBGPRINT(RT_DEBUG_TRACE, ("\nhw/sw version = "));

	for (i = 0; i < 4; i++)
		DBGPRINT(RT_DEBUG_TRACE, ("0x%2x ", *(cap->rom_patch + 20 + i)));

	DBGPRINT(RT_DEBUG_TRACE, ("\npatch version = "));

	for (i = 0; i < 4; i++)
		DBGPRINT(RT_DEBUG_TRACE, ("%c", *(cap->rom_patch + 24 + i)));

	DBGPRINT(RT_DEBUG_TRACE, ("\n"));
#endif

	/* Enable FCE */
	RTUSBWriteMACRegister(ad, FCE_PSE_CTRL, 0x01, FALSE);

	/* FCE tx_fs_base_ptr */
	RTUSBWriteMACRegister(ad, TX_CPU_PORT_FROM_FCE_BASE_PTR, 0x400230, FALSE);

	/* FCE tx_fs_max_cnt */
	RTUSBWriteMACRegister(ad, TX_CPU_PORT_FROM_FCE_MAX_COUNT, 0x01, FALSE);

	/* FCE pdma enable */
	RTUSBWriteMACRegister(ad, FCE_PDMA_GLOBAL_CONF, 0x44, FALSE);

	/* FCE skip_fs_en */
	RTUSBWriteMACRegister(ad, FCE_SKIP_FS, 0x03, FALSE);

	/* Allocate URB */
	urb = RTUSB_ALLOC_URB(0);
	if (!urb) {
		DBGPRINT(RT_DEBUG_ERROR, ("cannot allocate URB\n"));
		ret = NDIS_STATUS_RESOURCES;
		goto error0;
	}

	/* Allocate TransferBuffer */
	rom_patch_data = RTUSB_URB_ALLOC_BUFFER(obj->pUsb_Dev, UPLOAD_PATCH_UNIT, &rom_patch_dma);

	if (!rom_patch_data) {
		ret = NDIS_STATUS_RESOURCES;
		goto error1;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("loading rom patch\n"));

	init_completion(&load_rom_patch_done);
	cur_len = 0x00;
	patch_len = cap->rom_patch_len - PATCH_INFO_SIZE;

	/* loading rom patch */
	while (1) {
		s32 sent_len_max = UPLOAD_PATCH_UNIT - sizeof(*tx_info) - USB_END_PADDING;
		sent_len = (patch_len - cur_len) >=  sent_len_max ? sent_len_max : (patch_len - cur_len);

		DBGPRINT(RT_DEBUG_TRACE, ("patch_len = %d\n", patch_len));
		DBGPRINT(RT_DEBUG_TRACE, ("cur_len = %d\n", cur_len));
		DBGPRINT(RT_DEBUG_TRACE, ("sent_len = %d\n", sent_len));

		if (sent_len > 0) {
			tx_info = (TXINFO_NMAC_CMD *)rom_patch_data;
			tx_info->info_type = CMD_PACKET;
			tx_info->pkt_len = sent_len;
			tx_info->d_port = CPU_TX_PORT;

#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((PUCHAR)tx_info, TYPE_TXINFO);
#endif
			NdisMoveMemory(rom_patch_data + sizeof(*tx_info), cap->rom_patch + PATCH_INFO_SIZE + cur_len, sent_len);

			/* four zero bytes for end padding */
			NdisZeroMemory(rom_patch_data + sizeof(*tx_info) + sent_len, 4);

			value = (cur_len + cap->rom_patch_offset) & 0xFFFF;

			DBGPRINT(RT_DEBUG_TRACE, ("rom_patch_offset = %x\n", cap->rom_patch_offset));

			/* Set FCE DMA descriptor */
			ret = RTUSB_VendorRequest(ad, USBD_TRANSFER_DIRECTION_OUT,
					DEVICE_VENDOR_REQUEST_OUT, 0x42, value, 0x230, NULL, 0);
			if (ret) {
				DBGPRINT(RT_DEBUG_ERROR,
						("%s: set fce dma descriptor fail (0x230)\n",
						__FUNCTION__));
				goto error2;
			}

			value = (((cur_len + cap->rom_patch_offset) & 0xFFFF0000) >> 16);

			/* Set FCE DMA descriptor */
			ret = RTUSB_VendorRequest(ad, USBD_TRANSFER_DIRECTION_OUT,
					DEVICE_VENDOR_REQUEST_OUT, 0x42, value, 0x232, NULL, 0);
			if (ret) {
				DBGPRINT(RT_DEBUG_ERROR,
						("%s: set fce dma descriptor fail (0x232)\n",
						__FUNCTION__));
				goto error2;
			}

			cur_len += sent_len;

			while ((sent_len % 4) != 0)
				sent_len++;

			value = ((sent_len << 16) & 0xFFFF);

			/* Set FCE DMA length */
			ret = RTUSB_VendorRequest(ad, USBD_TRANSFER_DIRECTION_OUT,
					DEVICE_VENDOR_REQUEST_OUT, 0x42, value, 0x234, NULL, 0);
			if (ret) {
				DBGPRINT(RT_DEBUG_ERROR,
						("%s: set fce dma length fail (0x234)\n",
						__FUNCTION__));
				goto error2;
			}

			value = (((sent_len << 16) & 0xFFFF0000) >> 16);

			/* Set FCE DMA length */
			ret = RTUSB_VendorRequest(ad, USBD_TRANSFER_DIRECTION_OUT,
					DEVICE_VENDOR_REQUEST_OUT, 0x42, value, 0x236, NULL, 0);
			if (ret) {
				DBGPRINT(RT_DEBUG_ERROR,
						("%s: set fce dma length fail (0x236)\n",
						__FUNCTION__));
				goto error2;
			}

			/* Initialize URB descriptor */
			RTUSB_FILL_HTTX_BULK_URB(urb, obj->pUsb_Dev, cap->CommandBulkOutAddr,
				rom_patch_data, sent_len + sizeof(*tx_info) + 4,
				(usb_complete_t)usb_upload_rom_patch_complete, &load_rom_patch_done, rom_patch_dma);

			ret = RTUSB_SUBMIT_URB(urb);
			if (ret) {
				DBGPRINT(RT_DEBUG_ERROR, ("%s: submit urb fail\n",
						__FUNCTION__));
				goto error2;
			}

			DBGPRINT(RT_DEBUG_INFO,
					("%s: submit urb, sent_len = %d, patch_ilm = %d, cur_len = %d\n",
					__FUNCTION__, sent_len, patch_len, cur_len));

			if (!wait_for_completion_timeout(&load_rom_patch_done,
				msecs_to_jiffies(UPLOAD_FW_TIMEOUT_MS))) {
				usb_kill_urb(urb);
				ret = NDIS_STATUS_FAILURE;
				DBGPRINT(RT_DEBUG_ERROR,
						("%s: upload fw timeout\n",
						__FUNCTION__));
				goto error2;
			}

			DBGPRINT(RT_DEBUG_OFF, ("."));

			RTUSBReadMACRegister(ad, TX_CPU_PORT_FROM_FCE_CPU_DESC_INDEX, &mac_value);
			mac_value++;
			RTUSBWriteMACRegister(ad, TX_CPU_PORT_FROM_FCE_CPU_DESC_INDEX, mac_value, FALSE);
			RtmpOsMsDelay(5);
		} else {
			break;
		}
	}

	total_checksum = checksume16(cap->rom_patch + PATCH_INFO_SIZE, patch_len);

	RtmpOsMsDelay(5);
	DBGPRINT(RT_DEBUG_TRACE, ("Send checksum req..\n"));
	andes_usb_chk_crc(ad, patch_len);
	RtmpOsMsDelay(20);

	if (total_checksum != andes_usb_get_crc(ad)) {
		DBGPRINT(RT_DEBUG_ERROR, ("checksum fail!, local(0x%x) <> fw(0x%x)\n",
				total_checksum, andes_usb_get_crc(ad)));

		ret = NDIS_STATUS_FAILURE;
		goto error2;
	}

	ret = andes_usb_enable_patch(ad);
	if (ret) {
		ret = NDIS_STATUS_FAILURE;
		goto error2;
	}

	ret = andes_usb_reset_wmt(ad);

	RtmpOsMsDelay(20);

	/* Check ROM_PATCH if ready */
	i = 0;

	do {
		if (MT_REV_GTE(ad, MT76x2, REV_MT76x2E3)) {
			RTUSBReadMACRegister(ad, CLOCK_CTL, &mac_value);
			if ((mac_value & 0x01) == 0x1)
				break;
		} else {
			RTUSBReadMACRegister(ad, COM_REG0, &mac_value);
			if ((mac_value & 0x02) == 0x2)
				break;
		}

		RtmpOsMsDelay(10);
		i++;
	} while (i <= 100);

	WARN_ON(i >= 100);

	if (MT_REV_GTE(ad, MT76x2, REV_MT76x2E3)) {
		DBGPRINT(RT_DEBUG_TRACE, ("%s: CLOCK_CTL(0x%x) = 0x%x\n",
				__FUNCTION__, CLOCK_CTL, mac_value));

		if ((mac_value & 0x01) != 0x1)
			ret = NDIS_STATUS_FAILURE;

	} else {
		DBGPRINT(RT_DEBUG_TRACE, ("%s: CLOCK_CTL(0x%x) = 0x%x\n",
				__FUNCTION__, COM_REG0, mac_value));

		if ((mac_value & 0x02) != 0x2)
			ret = NDIS_STATUS_FAILURE;
	}

error2:
	/* Free TransferBuffer */
	RTUSB_URB_FREE_BUFFER(obj->pUsb_Dev, UPLOAD_PATCH_UNIT, rom_patch_data, rom_patch_dma);

error1:
	/* Free URB */
	usb_free_urb(urb);

error0:
	if (cap->rom_code_protect)
		RTUSBWriteMACRegister(ad, SEMAPHORE_03, 0x1, FALSE);

	return ret;
}

int andes_usb_erase_rom_patch(RTMP_ADAPTER *ad)
{
	RTMP_CHIP_CAP *cap = &ad->chipCap;

	DBGPRINT(RT_DEBUG_OFF, ("%s\n", __FUNCTION__));

	if (cap->load_code_method == BIN_FILE_METHOD) {
		if (cap->rom_patch)
			os_free_mem(cap->rom_patch);
		cap->rom_patch = NULL;
	}

	return 0;
}

static void usb_uploadfw_complete(purbb_t urb, pregs *pt_regs)
{
	struct completion *load_fw_done =
			(struct completion *)RTMP_OS_USB_CONTEXT_GET(urb);

	complete(load_fw_done);
}

static NDIS_STATUS usb_load_ivb(RTMP_ADAPTER *ad)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	RTMP_CHIP_CAP *cap = &ad->chipCap;

	if (cap->load_iv) {
		Status = RTUSB_VendorRequest(ad,
			USBD_TRANSFER_DIRECTION_OUT, DEVICE_VENDOR_REQUEST_OUT,
			0x01, 0x12, 0x00, cap->FWImageName + 32, 64);
	} else {
		Status = RTUSB_VendorRequest(ad,
			USBD_TRANSFER_DIRECTION_OUT, DEVICE_VENDOR_REQUEST_OUT,
			0x01, 0x12, 0x00, NULL, 0x00);
	}

	if (Status) {
		DBGPRINT(RT_DEBUG_ERROR, ("Upload IVB Fail\n"));
		return Status;
	}

	return Status;
}

NDIS_STATUS andes_usb_loadfw(RTMP_ADAPTER *ad)
{
	PURB urb;
	POS_COOKIE obj = (POS_COOKIE)ad->OS_Cookie;
	ra_dma_addr_t fw_dma;
	PUCHAR fw_data;
	TXINFO_NMAC_CMD *tx_info;
	s32 sent_len;
	u32 cur_len;
	u32 mac_value, loop;
	u16 value;
	int ret = 0;
	RTMP_CHIP_CAP *cap = &ad->chipCap;
	USB_DMA_CFG_STRUC cfg;
	u32 ilm_len = 0, dlm_len = 0;
	u16 fw_ver, build_ver;
	struct completion load_fw_done;

	if (cap->ram_code_protect) {
		for (loop = 0; loop < GET_SEMAPHORE_RETRY_MAX; loop++) {
			RTUSBReadMACRegister(ad, SEMAPHORE_00, &mac_value);
			if ((mac_value & 0x01) == 0x01)
				break;
			RtmpOsMsDelay(1);
		}

		if (loop >= GET_SEMAPHORE_RETRY_MAX) {
			DBGPRINT(RT_DEBUG_ERROR,
					("%s: Failed to obtain SEMAPHORE_00\n",
					__FUNCTION__));
			return NDIS_STATUS_FAILURE;
		}
	}

	/* Check MCU if ready */
	RTUSBReadMACRegister(ad, COM_REG0, &mac_value);

	if (((mac_value & 0x01) == 0x01) && (cap->ram_code_protect)) {
		goto error0;
	}

	RTUSBVendorReset(ad);

	/* Enable USB_DMA_CFG */
	USB_CFG_READ(ad, &cfg.word);
	cfg.word |= 0x00c00020;
	USB_CFG_WRITE(ad, cfg.word);

	if (cap->load_code_method == BIN_FILE_METHOD)
		OS_LOAD_CODE_FROM_BIN(&cap->FWImageName, cap->fw_bin_file_name,
				obj->pUsb_Dev, &cap->fw_len);
	else
		cap->FWImageName = cap->fw_header_image;

	if (!cap->FWImageName) {
		if (cap->load_code_method == BIN_FILE_METHOD) {
			DBGPRINT(RT_DEBUG_ERROR,
					("%s:Please assign a fw image(/lib/firmware/%s), load_method(%d)\n",
					__FUNCTION__, cap->fw_bin_file_name,
					cap->load_code_method));
		} else {
			DBGPRINT(RT_DEBUG_ERROR,
					("%s:Please assign a fw image, load_method(%d)\n",
					__FUNCTION__, cap->load_code_method));
		}
		ret = NDIS_STATUS_FAILURE;
		goto error0;
	}

	/* Get FW information */
	ilm_len = (*(cap->FWImageName + 3) << 24) | (*(cap->FWImageName + 2) << 16) |
			 (*(cap->FWImageName + 1) << 8) | (*cap->FWImageName);

	dlm_len = (*(cap->FWImageName + 7) << 24) | (*(cap->FWImageName + 6) << 16) |
			 (*(cap->FWImageName + 5) << 8) | (*(cap->FWImageName + 4));

	fw_ver = (*(cap->FWImageName + 11) << 8) | (*(cap->FWImageName + 10));

	build_ver = (*(cap->FWImageName + 9) << 8) | (*(cap->FWImageName + 8));

	DBGPRINT(RT_DEBUG_OFF, ("fw version:%d.%d.%02d ",
			(fw_ver & 0xf000) >> 8, (fw_ver & 0x0f00) >> 8,
			fw_ver & 0x00ff));
	DBGPRINT(RT_DEBUG_OFF, ("build:%x\n", build_ver));
	DBGPRINT(RT_DEBUG_OFF, ("build time:"));

	for (loop = 0; loop < 16; loop++)
		DBGPRINT(RT_DEBUG_OFF, ("%c", *(cap->FWImageName + 16 + loop)));

	DBGPRINT(RT_DEBUG_OFF, ("\n"));

	if (IS_MT76x2(ad)) {
		if (((strncmp(cap->FWImageName + 16, "20130811", 8) >= 0)) &&
			(MT_REV_GTE(ad, MT76x2, REV_MT76x2E3))) {
			DBGPRINT(RT_DEBUG_OFF, ("fw for E3 IC\n"));

		} else if (((strncmp(cap->FWImageName + 16, "20130811", 8) < 0)) &&
			(MT_REV_LT(ad, MT76x2, REV_MT76x2E3))){

			DBGPRINT(RT_DEBUG_OFF, ("fw for E2 IC\n"));
		} else {
			DBGPRINT(RT_DEBUG_OFF, ("fw do not match IC version\n"));
			RTMP_IO_READ32(ad, 0x0, &mac_value);
			DBGPRINT(RT_DEBUG_OFF, ("IC version(%x)\n", mac_value));
			ret = NDIS_STATUS_FAILURE;
			goto error0;
		}
	}

	DBGPRINT(RT_DEBUG_OFF, ("ilm length = %d(bytes)\n", ilm_len));
	DBGPRINT(RT_DEBUG_OFF, ("dlm length = %d(bytes)\n", dlm_len));

	/* Enable FCE to send in-band cmd */
	RTUSBWriteMACRegister(ad, FCE_PSE_CTRL, 0x01, FALSE);

	/* FCE tx_fs_base_ptr */
	RTUSBWriteMACRegister(ad, TX_CPU_PORT_FROM_FCE_BASE_PTR, 0x400230, FALSE);

	/* FCE tx_fs_max_cnt */
	RTUSBWriteMACRegister(ad, TX_CPU_PORT_FROM_FCE_MAX_COUNT, 0x01, FALSE);

	/* FCE pdma enable */
	RTUSBWriteMACRegister(ad, FCE_PDMA_GLOBAL_CONF, 0x44, FALSE);

	/* FCE skip_fs_en */
	RTUSBWriteMACRegister(ad, FCE_SKIP_FS, 0x03, FALSE);


	/* Allocate URB */
	urb = RTUSB_ALLOC_URB(0);
	if (!urb) {
		DBGPRINT(RT_DEBUG_ERROR, ("can not allocate URB\n"));
		ret = NDIS_STATUS_RESOURCES;
		goto error0;
	}

	/* Allocate TransferBuffer */
	fw_data = RTUSB_URB_ALLOC_BUFFER(obj->pUsb_Dev, UPLOAD_FW_UNIT, &fw_dma);
	if (!fw_data) {
		ret = NDIS_STATUS_RESOURCES;
		goto error1;
	}

	DBGPRINT(RT_DEBUG_OFF, ("loading fw"));

	init_completion(&load_fw_done);

	if (cap->load_iv)
		cur_len = 0x40;
	else
		cur_len = 0x00;

	/* Loading ILM */
	while (1) {
		s32 sent_len_max = UPLOAD_FW_UNIT - sizeof(*tx_info) - USB_END_PADDING;
		sent_len = (ilm_len - cur_len) >=  sent_len_max ? sent_len_max : (ilm_len - cur_len);

		if (sent_len > 0) {
			tx_info = (TXINFO_NMAC_CMD *)fw_data;
			tx_info->info_type = CMD_PACKET;
			tx_info->pkt_len = sent_len;
			tx_info->d_port = CPU_TX_PORT;

#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((PUCHAR)tx_info, TYPE_TXINFO);
#endif
			NdisMoveMemory(fw_data + sizeof(*tx_info), cap->FWImageName + FW_INFO_SIZE + cur_len, sent_len);

			/* four zero bytes for end padding */
			NdisZeroMemory(fw_data + sizeof(*tx_info) + sent_len, USB_END_PADDING);

			value = (cur_len + cap->ilm_offset) & 0xFFFF;

			/* Set FCE DMA descriptor */
			ret = RTUSB_VendorRequest(ad,
				USBD_TRANSFER_DIRECTION_OUT, DEVICE_VENDOR_REQUEST_OUT,
				0x42, value, 0x230, NULL, 0);

			if (ret) {
				DBGPRINT(RT_DEBUG_ERROR,
						("%s: set fce dma descriptor fail (0x230)\n",
						__FUNCTION__));
				goto error2;
			}

			value = (((cur_len + cap->ilm_offset) & 0xFFFF0000) >> 16);

			/* Set FCE DMA descriptor */
			ret = RTUSB_VendorRequest(ad, USBD_TRANSFER_DIRECTION_OUT,
					DEVICE_VENDOR_REQUEST_OUT, 0x42, value,
					0x232, NULL, 0);

			if (ret) {
				DBGPRINT(RT_DEBUG_ERROR,
						("%s: set fce dma descriptor fail (0x232)\n",
						__FUNCTION__));
				goto error2;
			}

			cur_len += sent_len;

			while ((sent_len % 4) != 0)
				sent_len++;

			value = ((sent_len << 16) & 0xFFFF);

			/* Set FCE DMA length */
			ret = RTUSB_VendorRequest(ad, USBD_TRANSFER_DIRECTION_OUT,
					DEVICE_VENDOR_REQUEST_OUT, 0x42, value,
					0x234, NULL, 0);

			if (ret) {
				DBGPRINT(RT_DEBUG_ERROR,
						("%s: set fce dma descriptor fail (0x234)\n",
						__FUNCTION__));
				goto error2;
			}

			value = (((sent_len << 16) & 0xFFFF0000) >> 16);

			/* Set FCE DMA length */
			ret = RTUSB_VendorRequest(ad, USBD_TRANSFER_DIRECTION_OUT,
					DEVICE_VENDOR_REQUEST_OUT, 0x42, value,
					0x236, NULL, 0);

			if (ret) {
				DBGPRINT(RT_DEBUG_ERROR,
						("%s: set fce dma descriptor fail (0x236)\n",
						__FUNCTION__));
				goto error2;
			}

			/* Initialize URB descriptor */
			RTUSB_FILL_HTTX_BULK_URB(urb, obj->pUsb_Dev,
					cap->CommandBulkOutAddr, fw_data,
					sent_len + sizeof(*tx_info) + USB_END_PADDING,
					(usb_complete_t)usb_uploadfw_complete,
					&load_fw_done, fw_dma);

			ret = RTUSB_SUBMIT_URB(urb);

			if (ret) {
				DBGPRINT(RT_DEBUG_ERROR, ("%s: submit urb fail\n",
					__FUNCTION__));
				goto error2;
			}

			DBGPRINT(RT_DEBUG_INFO,
					("%s: submit urb, sent_len = %d, ilm_ilm = %d, cur_len = %d\n",
					__FUNCTION__, sent_len, ilm_len, cur_len));

			if (!wait_for_completion_timeout(&load_fw_done, msecs_to_jiffies(UPLOAD_FW_TIMEOUT_MS))) {
				usb_kill_urb(urb);
				ret = NDIS_STATUS_FAILURE;
				DBGPRINT(RT_DEBUG_ERROR,
						("%s: upload fw timeout(%dms)\n",
						__FUNCTION__, UPLOAD_FW_TIMEOUT_MS));
				DBGPRINT(RT_DEBUG_INFO,
						("%s: submit urb, sent_len = %d, ilm_ilm = %d, cur_len = %d\n",
						__FUNCTION__, sent_len, ilm_len, cur_len));

				goto error2;
			}
			DBGPRINT(RT_DEBUG_OFF, ("."));

			RTUSBReadMACRegister(ad, TX_CPU_PORT_FROM_FCE_CPU_DESC_INDEX, &mac_value);
			mac_value++;
			RTUSBWriteMACRegister(ad, TX_CPU_PORT_FROM_FCE_CPU_DESC_INDEX, mac_value, FALSE);

			RtmpOsMsDelay(5);
		} else {
			break;
		}
	}

	/* Re-Initialize completion */
	init_completion(&load_fw_done);

	cur_len = 0x00;

	/* Loading DLM */
	while (1) {
		s32 sent_len_max = UPLOAD_FW_UNIT - sizeof(*tx_info) - USB_END_PADDING;
		sent_len = (dlm_len - cur_len) >= sent_len_max ? sent_len_max : (dlm_len - cur_len);

		if (sent_len > 0) {
			tx_info = (TXINFO_NMAC_CMD *)fw_data;
			tx_info->info_type = CMD_PACKET;
			tx_info->pkt_len = sent_len;
			tx_info->d_port = CPU_TX_PORT;

#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((PUCHAR)tx_info, TYPE_TXINFO);
#endif
			NdisMoveMemory(fw_data + sizeof(*tx_info), cap->FWImageName + FW_INFO_SIZE + ilm_len + cur_len, sent_len);

			NdisZeroMemory(fw_data + sizeof(*tx_info) + sent_len, USB_END_PADDING);

			if (MT_REV_GTE(ad, MT76x2, REV_MT76x2E3))
				value = ((cur_len + (cap->dlm_offset + 0x800)) & 0xFFFF);
			else
				value = ((cur_len + (cap->dlm_offset)) & 0xFFFF);

			/* Set FCE DMA descriptor */
			ret = RTUSB_VendorRequest(ad, USBD_TRANSFER_DIRECTION_OUT,
					DEVICE_VENDOR_REQUEST_OUT, 0x42, value,
					0x230, NULL, 0);

			if (ret) {
				DBGPRINT(RT_DEBUG_ERROR,
					("%s: set fce dma descriptor fail (DLM: 0x230)\n",
					__FUNCTION__));
				goto error2;
			}

			if (MT_REV_GTE(ad, MT76x2, REV_MT76x2E3))
				value = (((cur_len + (cap->dlm_offset + 0x800)) & 0xFFFF0000) >> 16);
			else
				value = (((cur_len + (cap->dlm_offset)) & 0xFFFF0000) >> 16);

			/* Set FCE DMA descriptor */
			ret = RTUSB_VendorRequest(ad, USBD_TRANSFER_DIRECTION_OUT,
					DEVICE_VENDOR_REQUEST_OUT, 0x42, value,
					0x232, NULL, 0);

			if (ret) {
				DBGPRINT(RT_DEBUG_ERROR,
						("%s: set fce dma descriptor fail (DLM: 0x232)\n",
						__FUNCTION__));
				goto error2;
			}

			cur_len += sent_len;

			while ((sent_len % 4) != 0)
				sent_len++;

			value = ((sent_len << 16) & 0xFFFF);

			/* Set FCE DMA length */
			ret = RTUSB_VendorRequest(ad,
					USBD_TRANSFER_DIRECTION_OUT, DEVICE_VENDOR_REQUEST_OUT,
					0x42, value, 0x234, NULL, 0);

			if (ret) {
				DBGPRINT(RT_DEBUG_ERROR,
						("%s: set fce dma descriptor fail (DLM: 0x234)\n",
						__FUNCTION__));
				goto error2;
			}
			value = (((sent_len << 16) & 0xFFFF0000) >> 16);

			/* Set FCE DMA length */
			ret = RTUSB_VendorRequest(ad, USBD_TRANSFER_DIRECTION_OUT,
					DEVICE_VENDOR_REQUEST_OUT, 0x42, value,
					0x236, NULL, 0);

			if (ret) {
				DBGPRINT(RT_DEBUG_ERROR, ("set fce dma length fail\n"));
				goto error2;
			}

			/* Initialize URB descriptor */
			RTUSB_FILL_HTTX_BULK_URB(urb, obj->pUsb_Dev,
					cap->CommandBulkOutAddr, fw_data,
					sent_len + sizeof(*tx_info) + USB_END_PADDING,
					(usb_complete_t)usb_uploadfw_complete,
					&load_fw_done, fw_dma);

			ret = RTUSB_SUBMIT_URB(urb);

			if (ret) {
				DBGPRINT(RT_DEBUG_ERROR, ("submit urb fail\n"));
				goto error2;
			}

			DBGPRINT(RT_DEBUG_INFO,
					("%s: submit urb, sent_len = %d, dlm_len = %d, cur_len = %d\n",
					__FUNCTION__, sent_len, dlm_len, cur_len));

			if (!wait_for_completion_timeout(&load_fw_done, msecs_to_jiffies(UPLOAD_FW_TIMEOUT_MS))) {
				usb_kill_urb(urb);
				ret = NDIS_STATUS_FAILURE;
				DBGPRINT(RT_DEBUG_ERROR,
						("%s: upload fw timeout(%dms)\n",
						__FUNCTION__,UPLOAD_FW_TIMEOUT_MS));
				DBGPRINT(RT_DEBUG_INFO,
						("%s: submit urb, sent_len = %d, dlm_len = %d, cur_len = %d\n",
						__FUNCTION__, sent_len, dlm_len, cur_len));

				goto error2;
			}
			DBGPRINT(RT_DEBUG_OFF, ("."));

			RTUSBReadMACRegister(ad, TX_CPU_PORT_FROM_FCE_CPU_DESC_INDEX, &mac_value);
			mac_value++;
			RTUSBWriteMACRegister(ad, TX_CPU_PORT_FROM_FCE_CPU_DESC_INDEX, mac_value, FALSE);
			RtmpOsMsDelay(5);
		} else {
			break;
		}
	}

	/* Upload new 64 bytes interrupt vector or reset andes */
	DBGPRINT(RT_DEBUG_OFF, ("\n"));
	usb_load_ivb(ad);

	/* Check MCU if ready */
	loop = 0;
	do {
		RTUSBReadMACRegister(ad, COM_REG0, &mac_value);
		if ((mac_value & 0x01) == 0x01)
			break;
		RtmpOsMsDelay(10);
		loop++;
	} while (loop <= 100);


	WARN_ON(loop >= 100);

	DBGPRINT(RT_DEBUG_TRACE, ("%s: COM_REG0(0x%x) = 0x%x\n", __FUNCTION__, COM_REG0, mac_value));

	RTMP_IO_READ32(ad, COM_REG0, &mac_value);
	mac_value |= (1 << 1);
	RTMP_IO_WRITE32(ad, COM_REG0, mac_value);

	if ((mac_value & 0x01) != 0x01)
		ret = NDIS_STATUS_FAILURE;

error2:
	/* Free TransferBuffer */
	RTUSB_URB_FREE_BUFFER(obj->pUsb_Dev, UPLOAD_FW_UNIT, fw_data, fw_dma);

error1:
	/* Free URB */
	usb_free_urb(urb);

error0:
	if (cap->ram_code_protect)
		RTUSBWriteMACRegister(ad, SEMAPHORE_00, 0x1, FALSE);

	/* Enable FCE to send in-band cmd */
	RTUSBWriteMACRegister(ad, FCE_PSE_CTRL, 0x01, FALSE);

	return ret;
}
#endif

int andes_usb_erasefw(RTMP_ADAPTER *ad)
{
	RTMP_CHIP_CAP *cap = &ad->chipCap;

	DBGPRINT(RT_DEBUG_OFF, ("%s\n", __FUNCTION__));

	if (cap->load_code_method == BIN_FILE_METHOD) {
		if (cap->FWImageName)
			os_free_mem(cap->FWImageName);
		cap->FWImageName = NULL;
	}

	return 0;
}

static struct cmd_msg *andes_alloc_cmd_msg(RTMP_ADAPTER *ad, unsigned int length)
{
	struct cmd_msg *msg = NULL;
	RTMP_CHIP_CAP *cap = &ad->chipCap;
	struct MCU_CTRL *ctl = &ad->MCUCtrl;
	PNDIS_PACKET net_pkt = NULL;
#ifdef RTMP_USB_SUPPORT
	PURB urb = NULL;
#endif

	net_pkt = RTMP_AllocateFragPacketBuffer(ad, cap->cmd_header_len + length + cap->cmd_padding_len);
	if (!net_pkt) {
		DBGPRINT(RT_DEBUG_ERROR, ("can not allocate net_pkt\n"));
		goto error0;
	}

	OS_PKT_RESERVE(net_pkt, cap->cmd_header_len);

	msg = os_alloc_mem(sizeof(*msg));
	if (!msg) {
		goto error1;
	}

	CMD_MSG_CB(net_pkt)->msg = msg;

	memset(msg, 0x00, sizeof(*msg));

#ifdef RTMP_USB_SUPPORT
	urb = RTUSB_ALLOC_URB(0);
	if (!urb) {
		DBGPRINT(RT_DEBUG_ERROR, ("can not allocate urb\n"));
		goto error2;
	}

	msg->urb = urb;
#endif

	msg->priv = (void *)ad;
	msg->net_pkt = net_pkt;
	ctl->alloc_cmd_msg++;

	return msg;

#ifdef RTMP_USB_SUPPORT
error2:
#endif
	os_free_mem(msg);
error1:
	dev_kfree_skb_any(net_pkt);
error0:
	return NULL;
}

static void andes_init_cmd_msg(struct cmd_msg *msg, u8 type, BOOLEAN need_wait,
	u16 timeout, BOOLEAN need_retransmit, BOOLEAN need_rsp,
	u16 rsp_payload_len, char *rsp_payload, MSG_RSP_HANDLER rsp_handler)
{
	msg->type = type;
#ifdef RTMP_USB_SUPPORT
	msg->need_wait= need_wait;
#else
	msg->need_wait= FALSE;
#endif
	msg->timeout = timeout;

	if (need_wait) {

#ifdef RTMP_USB_SUPPORT
		init_completion(&msg->ack_done);
#endif
	}

#ifdef RTMP_USB_SUPPORT
	msg->need_retransmit = need_retransmit;
#else
	msg->need_retransmit = 0;
#endif

#ifdef RTMP_USB_SUPPORT
	if (need_retransmit)
		msg->retransmit_times = CMD_MSG_RETRANSMIT_TIMES;
#else
	msg->retransmit_times = 0;
#endif

#ifdef RTMP_USB_SUPPORT
	msg->need_rsp = need_rsp;
#else
	msg->need_rsp = FALSE;
#endif
	msg->rsp_payload_len = rsp_payload_len;
	msg->rsp_payload = rsp_payload;
	msg->rsp_handler = rsp_handler;
}

static void andes_append_cmd_msg(struct cmd_msg *msg, char *data,
	unsigned int len)
{
	PNDIS_PACKET net_pkt = msg->net_pkt;

	if (data)
		memcpy(OS_PKT_TAIL_BUF_EXTEND(net_pkt, len), data, len);
}

static void andes_free_cmd_msg(struct cmd_msg *msg)
{
	PNDIS_PACKET net_pkt = msg->net_pkt;
	RTMP_ADAPTER *ad = (RTMP_ADAPTER *)(msg->priv);
	struct MCU_CTRL *ctl = &ad->MCUCtrl;

#ifdef RTMP_USB_SUPPORT
	usb_free_urb(msg->urb);
#endif

	os_free_mem(msg);

	dev_kfree_skb_any(net_pkt);
	ctl->free_cmd_msg++;
}

BOOLEAN is_inband_cmd_processing(RTMP_ADAPTER *ad)
{
	return FALSE;
}

static inline void andes_inc_error_count(struct MCU_CTRL *ctl,
	enum cmd_msg_error_type type)
{
	if (OS_TEST_BIT(MCU_INIT, &ctl->flags)) {
		switch (type) {
		case error_tx_kickout_fail:
			ctl->tx_kickout_fail_count++;
			break;
		case error_tx_timeout_fail:
			ctl->tx_timeout_fail_count++;
			break;
		case error_rx_receive_fail:
			ctl->rx_receive_fail_count++;
			break;
		default:
			DBGPRINT(RT_DEBUG_ERROR,
					("%s:unknown cmd_msg_error_type(%d)\n",
					__FUNCTION__, type));
		}
	}
}

static spinlock_t *andes_get_spin_lock(struct MCU_CTRL *ctl, DL_LIST *list)
{
	spinlock_t *lock = NULL;

	if (list == &ctl->txq)
		lock = &ctl->txq_lock;
	else if (list == &ctl->rxq)
		lock = &ctl->rxq_lock;
	else if (list == &ctl->ackq)
		lock = &ctl->ackq_lock;
	else if (list == &ctl->kickq)
		lock = &ctl->kickq_lock;
	else if (list == &ctl->tx_doneq)
		lock = &ctl->tx_doneq_lock;
	else if (list == &ctl->rx_doneq)
		lock = &ctl->rx_doneq_lock;
	else
		DBGPRINT(RT_DEBUG_ERROR, ("%s:illegal list\n", __FUNCTION__));

	return lock;
}

static inline UCHAR andes_get_cmd_msg_seq(RTMP_ADAPTER *ad)
{
	struct MCU_CTRL *ctl = &ad->MCUCtrl;
	struct cmd_msg *msg;
	unsigned long flags;

	RTMP_SPIN_LOCK_IRQSAVE(&ctl->ackq_lock, &flags);
get_seq:
	ctl->cmd_seq >= 0xf ? ctl->cmd_seq = 1 : ctl->cmd_seq++;
	DlListForEach(msg, &ctl->ackq, struct cmd_msg, list) {
		if (msg->seq == ctl->cmd_seq) {
			DBGPRINT(RT_DEBUG_ERROR, ("command(seq: %d) is still running\n", ctl->cmd_seq));
			goto get_seq;
		}
	}
	RTMP_SPIN_UNLOCK_IRQRESTORE(&ctl->ackq_lock, &flags);

	return ctl->cmd_seq;
}

static void _andes_queue_tail_cmd_msg(DL_LIST *list, struct cmd_msg *msg,
	enum cmd_msg_state state)
{
	msg->state = state;
	DlListAddTail(list, &msg->list);
}

static void andes_queue_tail_cmd_msg(DL_LIST *list, struct cmd_msg *msg,
	enum cmd_msg_state state)
{
	unsigned long flags;
	spinlock_t *lock;
	RTMP_ADAPTER *ad = (RTMP_ADAPTER *)msg->priv;
	struct MCU_CTRL *ctl = &ad->MCUCtrl;

	lock = andes_get_spin_lock(ctl, list);

	RTMP_SPIN_LOCK_IRQSAVE(lock, &flags);
	_andes_queue_tail_cmd_msg(list, msg, state);
	RTMP_SPIN_UNLOCK_IRQRESTORE(lock, &flags);
}

static void _andes_queue_head_cmd_msg(DL_LIST *list, struct cmd_msg *msg,
	enum cmd_msg_state state)
{
	msg->state = state;
	DlListAdd(list, &msg->list);
}

static void andes_queue_head_cmd_msg(DL_LIST *list, struct cmd_msg *msg,
	enum cmd_msg_state state)
{
	unsigned long flags;
	spinlock_t *lock;
	RTMP_ADAPTER *ad = (RTMP_ADAPTER *)msg->priv;
	struct MCU_CTRL *ctl = &ad->MCUCtrl;

	lock = andes_get_spin_lock(ctl, list);

	RTMP_SPIN_LOCK_IRQSAVE(lock, &flags);
	_andes_queue_head_cmd_msg(list, msg, state);
	RTMP_SPIN_UNLOCK_IRQRESTORE(lock, &flags);
}

static u32 andes_queue_len(struct MCU_CTRL *ctl, DL_LIST *list)
{
	u32 qlen;
	unsigned long flags;
	spinlock_t *lock;

	lock = andes_get_spin_lock(ctl, list);

	RTMP_SPIN_LOCK_IRQSAVE(lock, &flags);
	qlen = DlListLen(list);
	RTMP_SPIN_UNLOCK_IRQRESTORE(lock, &flags);

	return qlen;
}

#if 0 //JB removed
static int andes_queue_empty(struct MCU_CTRL *ctl, DL_LIST *list)
{
	unsigned long flags;
	int is_empty;
	spinlock_t *lock;

	lock = andes_get_spin_lock(ctl, list);

	RTMP_SPIN_LOCK_IRQSAVE(lock, &flags);
	is_empty = DlListEmpty(list);
	RTMP_SPIN_UNLOCK_IRQRESTORE(lock, &flags);

	return is_empty;
}
#endif //0

static void andes_queue_init(struct MCU_CTRL *ctl, DL_LIST *list)
{

	unsigned long flags;
	spinlock_t *lock;

	lock = andes_get_spin_lock(ctl, list);

	RTMP_SPIN_LOCK_IRQSAVE(lock, &flags);
	DlListInit(list);
	RTMP_SPIN_UNLOCK_IRQRESTORE(lock, &flags);
}

static void _andes_unlink_cmd_msg(struct cmd_msg *msg, DL_LIST *list)
{
	if (!msg)
		return;

	DlListDel(&msg->list);
}

static void andes_unlink_cmd_msg(struct cmd_msg *msg, DL_LIST *list)
{
	unsigned long flags;
	spinlock_t *lock;
	RTMP_ADAPTER *ad = (RTMP_ADAPTER *)msg->priv;
	struct MCU_CTRL *ctl = &ad->MCUCtrl;

	lock = andes_get_spin_lock(ctl, list);

	RTMP_SPIN_LOCK_IRQSAVE(lock, &flags);
	_andes_unlink_cmd_msg(msg, list);
	RTMP_SPIN_UNLOCK_IRQRESTORE(lock, &flags);
}

static struct cmd_msg *_andes_dequeue_cmd_msg(DL_LIST *list)
{
	struct cmd_msg *msg;

	msg = DlListFirst(list, struct cmd_msg, list);
	_andes_unlink_cmd_msg(msg, list);

	return msg;
}

static struct cmd_msg *andes_dequeue_cmd_msg(struct MCU_CTRL *ctl, DL_LIST *list)
{
	unsigned long flags;
	struct cmd_msg *msg;
	spinlock_t *lock;

	lock = andes_get_spin_lock(ctl, list);

	RTMP_SPIN_LOCK_IRQSAVE(lock, &flags);
	msg = _andes_dequeue_cmd_msg(list);
	RTMP_SPIN_UNLOCK_IRQRESTORE(lock, &flags);

	return msg;
}

void andes_rx_process_cmd_msg(RTMP_ADAPTER *ad, struct cmd_msg *rx_msg)
{
	PNDIS_PACKET net_pkt = rx_msg->net_pkt;
	struct cmd_msg *msg, *msg_tmp;
	RXFCE_INFO_CMD *rx_info = (RXFCE_INFO_CMD *)GET_OS_PKT_DATAPTR(net_pkt);
	struct MCU_CTRL *ctl = &ad->MCUCtrl;

#ifdef RT_BIG_ENDIAN
	RTMPDescriptorEndianChange((PUCHAR)rx_info, TYPE_RXINFO);
#endif

	DBGPRINT(RT_DEBUG_INFO, ("(andex_rx_cmd)info_type=%d,evt_type=%d,d_port=%d,"
			"qsel=%d,pcie_intr=%d,cmd_seq=%d,"
			"self_gen=%d,pkt_len=%d\n",
			rx_info->info_type, rx_info->evt_type,rx_info->d_port,
			rx_info->qsel, rx_info->pcie_intr, rx_info->cmd_seq,
			rx_info->self_gen, rx_info->pkt_len));

	if ((rx_info->info_type != CMD_PACKET)) {
		DBGPRINT(RT_DEBUG_ERROR, ("packet is not command response/self event\n"));
		return;
	}

	if (rx_info->self_gen) {
		/* if have callback function */
		RTEnqueueInternalCmd(ad, CMDTHREAD_RESPONSE_EVENT_CALLBACK,
				GET_OS_PKT_DATAPTR(net_pkt) + sizeof(*rx_info),
				rx_info->pkt_len);
	} else {
#ifdef RTMP_USB_SUPPORT
		RTMP_SPIN_LOCK_IRQ(&ctl->ackq_lock);
#endif

		DlListForEachSafe(msg, msg_tmp, &ctl->ackq, struct cmd_msg, list) {
			if (msg->seq == rx_info->cmd_seq) {
				_andes_unlink_cmd_msg(msg, &ctl->ackq);
#ifdef RTMP_USB_SUPPORT
				RTMP_SPIN_UNLOCK_IRQ(&ctl->ackq_lock);
#endif
				if ((msg->rsp_payload_len == rx_info->pkt_len) && (msg->rsp_payload_len != 0)) {
					msg->rsp_handler(msg, GET_OS_PKT_DATAPTR(net_pkt) + sizeof(*rx_info), rx_info->pkt_len);
				} else if ((msg->rsp_payload_len == 0) && (rx_info->pkt_len == 8)) {
					DBGPRINT(RT_DEBUG_INFO, ("command response(ack) success\n"));
				} else {
					DBGPRINT(RT_DEBUG_ERROR,
							("expect response len(%d), command response len(%d) invalid\n",
							msg->rsp_payload_len, rx_info->pkt_len));
					msg->rsp_payload_len = rx_info->pkt_len;
				}

				if (msg->need_wait) {
#ifdef RTMP_USB_SUPPORT
					complete(&msg->ack_done);
#endif
				} else {
					andes_free_cmd_msg(msg);
				}
#ifdef RTMP_USB_SUPPORT
				RTMP_SPIN_LOCK_IRQ(&ctl->ackq_lock);
#endif
				break;
			}
		}

#ifdef RTMP_USB_SUPPORT
		RTMP_SPIN_UNLOCK_IRQ(&ctl->ackq_lock);
#endif
	}
}


#ifdef RTMP_USB_SUPPORT
static void usb_rx_cmd_msg_complete(PURB urb)
{
	PNDIS_PACKET net_pkt = (PNDIS_PACKET)RTMP_OS_USB_CONTEXT_GET(urb);
	struct cmd_msg *msg = CMD_MSG_CB(net_pkt)->msg;
	RTMP_ADAPTER *ad = (RTMP_ADAPTER *)msg->priv;
	POS_COOKIE pObj = (POS_COOKIE)ad->OS_Cookie;
	RTMP_CHIP_CAP *pChipCap = &ad->chipCap;
	struct MCU_CTRL *ctl = &ad->MCUCtrl;
	enum cmd_msg_state state;
	unsigned long flags;
	int ret = 0;

	andes_unlink_cmd_msg(msg, &ctl->rxq);

	OS_PKT_TAIL_BUF_EXTEND(net_pkt, RTMP_USB_URB_LEN_GET(urb));

	if (RTMP_USB_URB_STATUS_GET(urb) == 0) {
		state = rx_done;
	} else {
		state = rx_receive_fail;
		andes_inc_error_count(ctl, error_rx_receive_fail);
		DBGPRINT(RT_DEBUG_ERROR, ("receive cmd msg fail(%d)\n", RTMP_USB_URB_STATUS_GET(urb)));
	}

	RTMP_SPIN_LOCK_IRQSAVE(&ctl->rx_doneq_lock, &flags);
	_andes_queue_tail_cmd_msg(&ctl->rx_doneq, msg, state);
	RTMP_SPIN_UNLOCK_IRQRESTORE(&ctl->rx_doneq_lock, &flags);

	if (OS_TEST_BIT(MCU_INIT, &ctl->flags)) {
		msg = andes_alloc_cmd_msg(ad, 512);

		if (!msg) {
			return;
		}

		net_pkt = msg->net_pkt;

		usb_fill_bulk_urb(msg->urb, pObj->pUsb_Dev,
				usb_rcvbulkpipe(pObj->pUsb_Dev, pChipCap->CommandRspBulkInAddr),
				GET_OS_PKT_DATAPTR(net_pkt), 512,
				usb_rx_cmd_msg_complete, net_pkt);

		andes_queue_tail_cmd_msg(&ctl->rxq, msg, rx_start);

		ret = RTUSB_SUBMIT_URB(msg->urb);
		if (ret) {
			andes_unlink_cmd_msg(msg, &ctl->rxq);
			andes_inc_error_count(ctl, error_rx_receive_fail);
			DBGPRINT(RT_DEBUG_ERROR,
					("%s:submit urb fail(%d)\n",
					__FUNCTION__, ret));
			andes_queue_tail_cmd_msg(&ctl->rx_doneq, msg, rx_receive_fail);
		}
	}

	andes_bh_schedule(ad);
}

int usb_rx_cmd_msg_submit(RTMP_ADAPTER *ad)
{
	RTMP_CHIP_CAP *pChipCap = &ad->chipCap;
	POS_COOKIE pObj = (POS_COOKIE)ad->OS_Cookie;
	struct MCU_CTRL *ctl = &ad->MCUCtrl;
	struct cmd_msg *msg = NULL;
	PNDIS_PACKET net_pkt = NULL;
	int ret = 0;

	if (!OS_TEST_BIT(MCU_INIT, &ctl->flags))
		return ret;

	msg =  andes_alloc_cmd_msg(ad, 512);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		return ret;
	}

	net_pkt = msg->net_pkt;

	usb_fill_bulk_urb(msg->urb, pObj->pUsb_Dev, usb_rcvbulkpipe(pObj->pUsb_Dev,
			pChipCap->CommandRspBulkInAddr), GET_OS_PKT_DATAPTR(net_pkt),
			512, usb_rx_cmd_msg_complete, net_pkt);

	andes_queue_tail_cmd_msg(&ctl->rxq, msg, rx_start);

	ret = RTUSB_SUBMIT_URB(msg->urb);
	if (ret) {
		andes_unlink_cmd_msg(msg, &ctl->rxq);
		andes_inc_error_count(ctl, error_rx_receive_fail);
		DBGPRINT(RT_DEBUG_ERROR, ("%s:submit urb fail(%d)\n", __FUNCTION__, ret));
		andes_queue_tail_cmd_msg(&ctl->rx_doneq, msg, rx_receive_fail);
	}

	return ret;
}

int usb_rx_cmd_msgs_receive(RTMP_ADAPTER *ad)
{
	int ret = 0;
	int i;
	struct MCU_CTRL *ctl = &ad->MCUCtrl;

	for (i = 0; (i < 1) && (andes_queue_len(ctl, &ctl->rxq) < 1); i++) {
		ret = usb_rx_cmd_msg_submit(ad);
		if (ret)
			break;
	}

	return ret;
}
#endif

void andes_cmd_msg_bh(unsigned long param)
{
	RTMP_ADAPTER *ad = (RTMP_ADAPTER *)param;
	struct MCU_CTRL *ctl = &ad->MCUCtrl;
	struct cmd_msg *msg = NULL;

	while ((msg = andes_dequeue_cmd_msg(ctl, &ctl->rx_doneq))) {
		switch (msg->state) {
		case rx_done:
			andes_rx_process_cmd_msg(ad, msg);
			andes_free_cmd_msg(msg);
			continue;
		case rx_receive_fail:
			andes_free_cmd_msg(msg);
			continue;
		default:
			DBGPRINT(RT_DEBUG_ERROR, ("unknow msg state(%d)\n", msg->state));
		}
	}

	while ((msg = andes_dequeue_cmd_msg(ctl, &ctl->tx_doneq))) {
		switch (msg->state) {
		case tx_done:
		case tx_kickout_fail:
		case tx_timeout_fail:
			andes_free_cmd_msg(msg);
			continue;
		default:
			DBGPRINT(RT_DEBUG_ERROR, ("unknow msg state(%d)\n", msg->state));
		}
	}

	if (OS_TEST_BIT(MCU_INIT, &ctl->flags)) {
		andes_bh_schedule(ad);
#ifdef RTMP_USB_SUPPORT
		usb_rx_cmd_msgs_receive(ad);
#endif
	}
}

static void andes_bh_schedule(RTMP_ADAPTER *ad)
{
	struct MCU_CTRL *ctl = &ad->MCUCtrl;

	if (!OS_TEST_BIT(MCU_INIT, &ctl->flags))
		return;

	if (((andes_queue_len(ctl, &ctl->rx_doneq) > 0) ||
		(andes_queue_len(ctl, &ctl->tx_doneq) > 0)) &&
		OS_TEST_BIT(MCU_INIT, &ctl->flags)) {
#ifndef WORKQUEUE_BH
		RTMP_NET_TASK_DATA_ASSIGN(&ctl->cmd_msg_task, (unsigned long)(ad));
		RTMP_OS_TASKLET_SCHE(&ctl->cmd_msg_task);
#else
		tasklet_hi_schedule(&ctl->cmd_msg_task);
#endif
	}
}

#ifdef RTMP_USB_SUPPORT
static void usb_kick_out_cmd_msg_complete(PURB urb)
{
	PNDIS_PACKET net_pkt = (PNDIS_PACKET)RTMP_OS_USB_CONTEXT_GET(urb);
	struct cmd_msg *msg = CMD_MSG_CB(net_pkt)->msg;
	RTMP_ADAPTER *ad = (RTMP_ADAPTER *)msg->priv;
	struct MCU_CTRL *ctl = &ad->MCUCtrl;

	if (!OS_TEST_BIT(MCU_INIT, &ctl->flags))
		return;

	if (RTMP_USB_URB_STATUS_GET(urb) == 0) {
		if (!msg->need_rsp) {
			andes_unlink_cmd_msg(msg, &ctl->kickq);
			andes_queue_tail_cmd_msg(&ctl->tx_doneq, msg, tx_done);
		} else {
			msg->state = wait_ack;
		}
	} else {
		if (!msg->need_rsp) {
			andes_unlink_cmd_msg(msg, &ctl->kickq);
			andes_queue_tail_cmd_msg(&ctl->tx_doneq, msg, tx_kickout_fail);
			andes_inc_error_count(ctl, error_tx_kickout_fail);
		} else {
			andes_unlink_cmd_msg(msg, &ctl->ackq);
			msg->state = tx_kickout_fail;
			andes_inc_error_count(ctl, error_tx_kickout_fail);
			complete(&msg->ack_done);
		}

		DBGPRINT(RT_DEBUG_ERROR, ("kick out cmd msg fail(%d)\n",
				RTMP_USB_URB_STATUS_GET(urb)));
	}

	andes_bh_schedule(ad);
}

static int usb_kick_out_cmd_msg(PRTMP_ADAPTER ad, struct cmd_msg *msg)
{
	struct MCU_CTRL *ctl = &ad->MCUCtrl;
	POS_COOKIE pObj = (POS_COOKIE)ad->OS_Cookie;
	int ret = 0;
	PNDIS_PACKET net_pkt = msg->net_pkt;
	RTMP_CHIP_CAP *pChipCap = &ad->chipCap;

	if (msg->state != tx_retransmit) {
		/* append four zero bytes padding when usb aggregate enable */
		memset(OS_PKT_TAIL_BUF_EXTEND(net_pkt, USB_END_PADDING), 0x00, USB_END_PADDING);
	}

	usb_fill_bulk_urb(msg->urb, pObj->pUsb_Dev, usb_sndbulkpipe(pObj->pUsb_Dev,
			pChipCap->CommandBulkOutAddr), GET_OS_PKT_DATAPTR(net_pkt),
			GET_OS_PKT_LEN(net_pkt), usb_kick_out_cmd_msg_complete, net_pkt);

	if (msg->need_rsp)
		andes_queue_tail_cmd_msg(&ctl->ackq, msg, wait_cmd_out_and_ack);
	else
		andes_queue_tail_cmd_msg(&ctl->kickq, msg, wait_cmd_out);

	if (!OS_TEST_BIT(MCU_INIT, &ctl->flags))
		return -1;

	ret = RTUSB_SUBMIT_URB(msg->urb);
	if (ret) {
		if (!msg->need_rsp) {
			andes_unlink_cmd_msg(msg, &ctl->kickq);
			andes_queue_tail_cmd_msg(&ctl->tx_doneq, msg, tx_kickout_fail);
			andes_inc_error_count(ctl, error_tx_kickout_fail);
		} else {
			andes_unlink_cmd_msg(msg, &ctl->ackq);
			msg->state = tx_kickout_fail;
			andes_inc_error_count(ctl, error_tx_kickout_fail);
			complete(&msg->ack_done);
		}

		DBGPRINT(RT_DEBUG_ERROR, ("%s:submit urb fail(%d)\n", __FUNCTION__, ret));
	}

	return ret;
}

static void andes_usb_unlink_urb(RTMP_ADAPTER *ad, DL_LIST *list)
{
	unsigned long flags;
	struct cmd_msg *msg, *msg_tmp;
	spinlock_t *lock;
	struct MCU_CTRL *ctl = &ad->MCUCtrl;

	lock = andes_get_spin_lock(ctl, list);

	RTMP_SPIN_LOCK_IRQSAVE(lock, &flags);
	DlListForEachSafe(msg, msg_tmp, list, struct cmd_msg, list) {
		RTMP_SPIN_UNLOCK_IRQRESTORE(lock, &flags);
		if ((msg->state == wait_cmd_out_and_ack) ||
			(msg->state == wait_cmd_out) ||
			(msg->state == tx_start) || (msg->state == rx_start) ||
			(msg->state == tx_retransmit))
			usb_kill_urb(msg->urb);
		RTMP_SPIN_LOCK_IRQSAVE(lock, &flags);
	}
	RTMP_SPIN_UNLOCK_IRQRESTORE(lock, &flags);
}

#endif

static void andes_cleanup_cmd_msg(RTMP_ADAPTER *ad, DL_LIST *list)
{
	unsigned long flags;
	struct cmd_msg *msg, *msg_tmp;
	spinlock_t *lock;
	struct MCU_CTRL *ctl = &ad->MCUCtrl;

	lock = andes_get_spin_lock(ctl, list);

	RTMP_SPIN_LOCK_IRQSAVE(lock, &flags);
	DlListForEachSafe(msg, msg_tmp, list, struct cmd_msg, list) {
		_andes_unlink_cmd_msg(msg, list);
		andes_free_cmd_msg(msg);
	}
	DlListInit(list);
	RTMP_SPIN_UNLOCK_IRQRESTORE(lock, &flags);
}


#ifdef RTMP_USB_SUPPORT
static void andes_ctrl_usb_init(RTMP_ADAPTER *ad)
{
	struct MCU_CTRL *ctl = &ad->MCUCtrl;
	int ret = 0;

	RTMP_SEM_EVENT_WAIT(&(ad->mcu_atomic), ret);
	RTMP_CLEAR_FLAG(ad, fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD);
	ctl->cmd_seq = 0;
	RTMP_OS_TASKLET_INIT(ad, &ctl->cmd_msg_task, andes_cmd_msg_bh, (unsigned long)ad);
	NdisAllocateSpinLock(ad, &ctl->txq_lock);
	andes_queue_init(ctl, &ctl->txq);
	NdisAllocateSpinLock(ad, &ctl->rxq_lock);
	andes_queue_init(ctl, &ctl->rxq);
	NdisAllocateSpinLock(ad, &ctl->ackq_lock);
	andes_queue_init(ctl, &ctl->ackq);
	NdisAllocateSpinLock(ad, &ctl->kickq_lock);
	andes_queue_init(ctl, &ctl->kickq);
	NdisAllocateSpinLock(ad, &ctl->tx_doneq_lock);
	andes_queue_init(ctl, &ctl->tx_doneq);
	NdisAllocateSpinLock(ad, &ctl->rx_doneq_lock);
	andes_queue_init(ctl, &ctl->rx_doneq);
	ctl->tx_kickout_fail_count = 0;
	ctl->tx_timeout_fail_count = 0;
	ctl->rx_receive_fail_count = 0;
	ctl->alloc_cmd_msg = 0;
	ctl->free_cmd_msg = 0;
	ctl->ad = ad;
	OS_SET_BIT(MCU_INIT, &ctl->flags);
	usb_rx_cmd_msgs_receive(ad);
	RTMP_SEM_EVENT_UP(&(ad->mcu_atomic));
}
#endif

void andes_ctrl_init(RTMP_ADAPTER *ad)
{
	struct MCU_CTRL *ctl = &ad->MCUCtrl;

	if (!OS_TEST_BIT(MCU_INIT, &ctl->flags)) {

#ifdef RTMP_USB_SUPPORT
		andes_ctrl_usb_init(ad);
#endif
	}

	ctl->power_on = FALSE;
	ctl->dpd_on = FALSE;
}

#ifdef RTMP_USB_SUPPORT
static void andes_ctrl_usb_exit(RTMP_ADAPTER *ad)
{
	struct MCU_CTRL *ctl = &ad->MCUCtrl;
	int ret = 0;

	RTMP_SEM_EVENT_WAIT(&(ad->mcu_atomic), ret);
	RTMP_CLEAR_FLAG(ad, fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD);
	OS_CLEAR_BIT(MCU_INIT, &ctl->flags);
	andes_usb_unlink_urb(ad, &ctl->txq);
	andes_usb_unlink_urb(ad, &ctl->kickq);
	andes_usb_unlink_urb(ad, &ctl->ackq);
	andes_usb_unlink_urb(ad, &ctl->rxq);
	RTMP_OS_TASKLET_KILL(&ctl->cmd_msg_task);
	andes_cleanup_cmd_msg(ad, &ctl->txq);
	NdisFreeSpinLock(&ctl->txq_lock);
	andes_cleanup_cmd_msg(ad, &ctl->ackq);
	NdisFreeSpinLock(&ctl->ackq_lock);
	andes_cleanup_cmd_msg(ad, &ctl->rxq);
	NdisFreeSpinLock(&ctl->rxq_lock);
	andes_cleanup_cmd_msg(ad, &ctl->kickq);
	NdisFreeSpinLock(&ctl->kickq_lock);
	andes_cleanup_cmd_msg(ad, &ctl->tx_doneq);
	NdisFreeSpinLock(&ctl->tx_doneq_lock);
	andes_cleanup_cmd_msg(ad, &ctl->rx_doneq);
	NdisFreeSpinLock(&ctl->rx_doneq_lock);
	DBGPRINT(RT_DEBUG_OFF, ("tx_kickout_fail_count = %ld\n", ctl->tx_kickout_fail_count));
	DBGPRINT(RT_DEBUG_OFF, ("tx_timeout_fail_count = %ld\n", ctl->tx_timeout_fail_count));
	DBGPRINT(RT_DEBUG_OFF, ("rx_receive_fail_count = %ld\n", ctl->rx_receive_fail_count));
	DBGPRINT(RT_DEBUG_OFF, ("alloc_cmd_msg = %ld\n", ctl->alloc_cmd_msg));
	DBGPRINT(RT_DEBUG_OFF, ("free_cmd_msg = %ld\n", ctl->free_cmd_msg));
	RTMP_SEM_EVENT_UP(&(ad->mcu_atomic));
}
#endif


void andes_ctrl_exit(RTMP_ADAPTER *ad)
{
	struct MCU_CTRL *ctl = &ad->MCUCtrl;

	if (OS_TEST_BIT(MCU_INIT, &ctl->flags)) {

#ifdef RTMP_USB_SUPPORT
		andes_ctrl_usb_exit(ad);
#endif
	}

	ctl->power_on = FALSE;
	ctl->dpd_on = FALSE;
}

static int andes_dequeue_and_kick_out_cmd_msgs(RTMP_ADAPTER *ad)
{
	struct cmd_msg *msg = NULL;
	PNDIS_PACKET net_pkt = NULL;
	struct MCU_CTRL *ctl = &ad->MCUCtrl;
	int ret = NDIS_STATUS_SUCCESS;
	TXINFO_NMAC_CMD *tx_info;

	while ((msg = andes_dequeue_cmd_msg(ctl, &ctl->txq)) != NULL) {
		if (!RTMP_TEST_FLAG(ad, fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD)
				|| RTMP_TEST_FLAG(ad, fRTMP_ADAPTER_NIC_NOT_EXIST)
				|| RTMP_TEST_FLAG(ad, fRTMP_ADAPTER_SUSPEND)) {
			if (!msg->need_rsp)
				andes_free_cmd_msg(msg);
			continue;
		}

		if (andes_queue_len(ctl, &ctl->ackq) > 0) {
			andes_queue_head_cmd_msg(&ctl->txq, msg, msg->state);
			ret = NDIS_STATUS_FAILURE;
			continue;
		}

		net_pkt = msg->net_pkt;

		if (msg->state != tx_retransmit) {
			if (msg->need_rsp)
				msg->seq = andes_get_cmd_msg_seq(ad);
			else
				msg->seq = 0;

			tx_info = (TXINFO_NMAC_CMD *)OS_PKT_HEAD_BUF_EXTEND(net_pkt, sizeof(*tx_info));
			tx_info->info_type = CMD_PACKET;
			tx_info->d_port = CPU_TX_PORT;
			tx_info->cmd_type = msg->type;
			tx_info->cmd_seq = msg->seq;
			tx_info->pkt_len = GET_OS_PKT_LEN(net_pkt) - sizeof(*tx_info);

#ifdef RT_BIG_ENDIAN
			*(UINT32 *)tx_info = le2cpu32(*(UINT32 *)tx_info);
			//RTMPDescriptorEndianChange((PUCHAR)tx_info, TYPE_TXINFO);
#endif
		}

#ifdef RTMP_USB_SUPPORT
		ret = usb_kick_out_cmd_msg(ad, msg);
#endif

		if (ret) {
			DBGPRINT(RT_DEBUG_ERROR, ("kick out msg fail\n"));
			break;
		}
	}

	andes_bh_schedule(ad);

	return ret;
}

static int andes_wait_for_complete_timeout(struct cmd_msg *msg, long timeout)
{
	int ret = 0;
#ifdef RTMP_USB_SUPPORT
	long expire;
	expire = timeout ? msecs_to_jiffies(timeout) : msecs_to_jiffies(CMD_MSG_TIMEOUT);
	ret = wait_for_completion_timeout(&msg->ack_done, expire);
#endif

	return ret;
}

int andes_send_cmd_msg(PRTMP_ADAPTER ad, struct cmd_msg *msg)
{
	struct MCU_CTRL *ctl = &ad->MCUCtrl;
	int ret = 0;
	BOOLEAN need_wait = msg->need_wait;

#ifdef RTMP_USB_SUPPORT
	RTMP_SEM_EVENT_WAIT(&(ad->mcu_atomic), ret);
#endif

	if (!RTMP_TEST_FLAG(ad, fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD) ||
		RTMP_TEST_FLAG(ad, fRTMP_ADAPTER_NIC_NOT_EXIST) ||
		RTMP_TEST_FLAG(ad, fRTMP_ADAPTER_SUSPEND)) {
		andes_free_cmd_msg(msg);

#ifdef RTMP_USB_SUPPORT
		RTMP_SEM_EVENT_UP(&(ad->mcu_atomic));
#endif
		return NDIS_STATUS_FAILURE;
	}

	andes_queue_tail_cmd_msg(&ctl->txq, msg, tx_start);

retransmit:
	andes_dequeue_and_kick_out_cmd_msgs(ad);

	/* Wait for response */
	if (need_wait) {
		enum cmd_msg_state state;
		if (!andes_wait_for_complete_timeout(msg, msg->timeout)) {
			ret = NDIS_STATUS_FAILURE;
			DBGPRINT(RT_DEBUG_ERROR, ("command (%d) timeout(%dms)\n", msg->type, CMD_MSG_TIMEOUT));
			DBGPRINT(RT_DEBUG_ERROR, ("txq qlen = %d\n", andes_queue_len(ctl, &ctl->txq)));
			DBGPRINT(RT_DEBUG_ERROR, ("rxq qlen = %d\n", andes_queue_len(ctl, &ctl->rxq)));
			DBGPRINT(RT_DEBUG_ERROR, ("kickq qlen = %d\n", andes_queue_len(ctl, &ctl->kickq)));
			DBGPRINT(RT_DEBUG_ERROR, ("ackq qlen = %d\n", andes_queue_len(ctl, &ctl->ackq)));
			DBGPRINT(RT_DEBUG_ERROR, ("tx_doneq.qlen = %d\n", andes_queue_len(ctl, &ctl->tx_doneq)));
			DBGPRINT(RT_DEBUG_ERROR, ("rx_done qlen = %d\n", andes_queue_len(ctl, &ctl->rx_doneq)));
			if (OS_TEST_BIT(MCU_INIT, &ctl->flags)) {
				if (msg->state == wait_cmd_out_and_ack) {
#ifdef RTMP_USB_SUPPORT
					usb_kill_urb(msg->urb);
#endif
				} else if (msg->state == wait_ack) {
					andes_unlink_cmd_msg(msg, &ctl->ackq);
				}
			}

			andes_inc_error_count(ctl, error_tx_timeout_fail);
			state = tx_timeout_fail;
			if (msg->retransmit_times > 0)
				msg->retransmit_times--;
			DBGPRINT(RT_DEBUG_ERROR, ("msg->retransmit_times = %d\n", msg->retransmit_times));
		} else {
			if (msg->state == tx_kickout_fail) {
				state = tx_kickout_fail;
				msg->retransmit_times--;
			} else {
				state = tx_done;
				msg->retransmit_times = 0;
			}
		}

		if (OS_TEST_BIT(MCU_INIT, &ctl->flags)) {
			if (msg->need_retransmit && (msg->retransmit_times > 0)) {
#ifdef RTMP_USB_SUPPORT
				init_completion(&msg->ack_done);
#endif
				state = tx_retransmit;
				andes_queue_head_cmd_msg(&ctl->txq, msg, state);
				goto retransmit;
			} else {
				andes_queue_tail_cmd_msg(&ctl->tx_doneq, msg, state);
			}
		} else {
			andes_free_cmd_msg(msg);
		}
	}

#ifdef RTMP_USB_SUPPORT
	RTMP_SEM_EVENT_UP(&(ad->mcu_atomic));
#endif

	return ret;
}


int andes_burst_write(RTMP_ADAPTER *ad, u32 offset, u32 *data, u32 cnt)
{
	struct cmd_msg *msg;
	unsigned int var_len, offset_num, cur_len = 0, sent_len;
	u32 value, i, cur_index = 0;
	RTMP_CHIP_CAP *cap = &ad->chipCap;
	int ret = 0;
	BOOLEAN last_packet = FALSE;

	if (!data)
		return -1;

	offset_num = cnt / ((cap->InbandPacketMaxLen - sizeof(offset)) / 4);

	if (cnt % ((cap->InbandPacketMaxLen - sizeof(offset)) / 4))
		var_len = sizeof(offset) * (offset_num + 1) + 4 * cnt;
	else
		var_len = sizeof(offset) * offset_num + 4 * cnt;

	while (cur_len < var_len) {
		sent_len = (var_len - cur_len) > cap->InbandPacketMaxLen
				? cap->InbandPacketMaxLen : (var_len - cur_len);

		if (((sent_len < cap->InbandPacketMaxLen) || ((cur_len + cap->InbandPacketMaxLen) == var_len)))
			last_packet = TRUE;

		msg = andes_alloc_cmd_msg(ad, sent_len);
		if (!msg) {
			ret = NDIS_STATUS_RESOURCES;
			goto error;
		}

		if (last_packet) {
			andes_init_cmd_msg(msg, CMD_BURST_WRITE, TRUE, 0, TRUE, TRUE, 0, NULL, NULL);
		} else {
			andes_init_cmd_msg(msg, CMD_BURST_WRITE, FALSE, 0, FALSE, FALSE, 0, NULL, NULL);
		}

		value = cpu2le32(offset + cap->WlanMemmapOffset + cur_index * 4);
		andes_append_cmd_msg(msg, (char *)&value, 4);

		for (i = 0; i < ((sent_len - 4) / 4); i++) {
			value = cpu2le32(data[i + cur_index]);
			andes_append_cmd_msg(msg, (char *)&value, 4);
		}

		ret = andes_send_cmd_msg(ad, msg);
		cur_index += ((sent_len - 4) / 4);
		cur_len += cap->InbandPacketMaxLen;
	}

error:
	return ret;
}

static void andes_burst_read_callback(struct cmd_msg *msg, char *rsp_payload, u16 rsp_payload_len)
{
	u32 i;
	u32 *data;
	NdisMoveMemory(msg->rsp_payload, rsp_payload + 4, rsp_payload_len - 4);

	for (i = 0; i < (msg->rsp_payload_len - 4) / 4; i++) {
		data = (u32 *)(msg->rsp_payload + i * 4);
		*data = le2cpu32(*data);
	}
}

int andes_burst_read(RTMP_ADAPTER *ad, u32 offset, u32 cnt, u32 *data)
{
	struct cmd_msg *msg;
	unsigned int cur_len = 0, rsp_len, offset_num, receive_len;
	u32 value, cur_index = 0;
	RTMP_CHIP_CAP *cap = &ad->chipCap;
	int ret = 0;

	if (!data)
		return -1;

	offset_num = cnt / ((cap->InbandPacketMaxLen - sizeof(offset)) / 4);

	if (cnt % ((cap->InbandPacketMaxLen - sizeof(offset)) / 4))
		rsp_len = sizeof(offset) * (offset_num + 1) + 4 * cnt;
	else
		rsp_len = sizeof(offset) * offset_num + 4 * cnt;

	while (cur_len < rsp_len) {
		receive_len = (rsp_len - cur_len) > cap->InbandPacketMaxLen
			   ? cap->InbandPacketMaxLen : (rsp_len - cur_len);

		msg = andes_alloc_cmd_msg(ad, 8);
		if (!msg) {
			ret = NDIS_STATUS_RESOURCES;
			goto error;
		}

		andes_init_cmd_msg(msg, CMD_BURST_READ, TRUE, 0, TRUE, TRUE, receive_len,
				(char *)(&data[cur_index]), andes_burst_read_callback);

		value = cpu2le32(offset + cap->WlanMemmapOffset + cur_index * 4);
		andes_append_cmd_msg(msg, (char *)&value, 4);

		value = cpu2le32((receive_len - 4) / 4);
		andes_append_cmd_msg(msg, (char *)&value, 4);

		ret = andes_send_cmd_msg(ad, msg);
		if (ret) {
			if (cnt == 1)
				*data = 0xffffffff;
		}

		cur_index += ((receive_len - 4) / 4);
		cur_len += cap->InbandPacketMaxLen;
	}

error:
	return ret;
}

static void andes_random_read_callback(struct cmd_msg *msg, char *rsp_payload, u16 rsp_payload_len)
{
	u32 i;
	RTMP_REG_PAIR *reg_pair = (RTMP_REG_PAIR *)msg->rsp_payload;

	for (i = 0; i < msg->rsp_payload_len / 8; i++) {
		NdisMoveMemory(&reg_pair[i].Value, rsp_payload + 8 * i + 4, 4);
		reg_pair[i].Value = le2cpu32(reg_pair[i].Value);
	}
}

int andes_random_read(RTMP_ADAPTER *ad, RTMP_REG_PAIR *reg_pair, u32 num)
{
	struct cmd_msg *msg;
	unsigned int var_len = num * 8, cur_len = 0, receive_len;
	u32 i, value, cur_index = 0;
	RTMP_CHIP_CAP *cap = &ad->chipCap;
	int ret = 0;

	if (!reg_pair)
		return -1;

	while (cur_len < var_len) {
		receive_len = (var_len - cur_len) > cap->InbandPacketMaxLen
		   ? cap->InbandPacketMaxLen : (var_len - cur_len);

		msg = andes_alloc_cmd_msg(ad, receive_len);
		if (!msg) {
			ret = NDIS_STATUS_RESOURCES;
			goto error;
		}

		andes_init_cmd_msg(msg, CMD_RANDOM_READ, TRUE, 0, TRUE, TRUE, receive_len,
				(char *)&reg_pair[cur_index], andes_random_read_callback);

		for (i = 0; i < receive_len / 8; i++) {
			value = cpu2le32(reg_pair[i + cur_index].Register + cap->WlanMemmapOffset);
			andes_append_cmd_msg(msg, (char *)&value, 4);
			value = 0;
			andes_append_cmd_msg(msg, (char *)&value, 4);
		}


		ret = andes_send_cmd_msg(ad, msg);
		cur_index += receive_len / 8;
		cur_len += cap->InbandPacketMaxLen;
	}

error:
	return ret;
}

static void andes_rf_random_read_callback(struct cmd_msg *msg, char *rsp_payload, u16 rsp_payload_len)
{
	u32 i;
	BANK_RF_REG_PAIR *reg_pair = (BANK_RF_REG_PAIR *)msg->rsp_payload;

	for (i = 0; i < msg->rsp_payload_len / 8; i++) {
		NdisMoveMemory(&reg_pair[i].Value, rsp_payload + 8 * i + 4, 1);
	}
}

int andes_rf_random_read(RTMP_ADAPTER *ad, BANK_RF_REG_PAIR *reg_pair, u32 num)
{
	struct cmd_msg *msg;
	unsigned int var_len = num * 8, cur_len = 0, receive_len;
	u32 i, value, cur_index = 0;
	RTMP_CHIP_CAP *cap = &ad->chipCap;
	int ret = 0;

	if (!reg_pair)
		return -1;

	while (cur_len < var_len) {
		receive_len = (var_len - cur_len) > cap->InbandPacketMaxLen
			   ? cap->InbandPacketMaxLen : (var_len - cur_len);

		msg = andes_alloc_cmd_msg(ad, receive_len);
		if (!msg) {
			ret = NDIS_STATUS_RESOURCES;
			goto error;
		}

		andes_init_cmd_msg(msg, CMD_RANDOM_READ, TRUE, 0, TRUE, TRUE, receive_len,
				(char *)&reg_pair[cur_index], andes_rf_random_read_callback);

		for (i = 0; i < (receive_len) / 8; i++) {
			value = 0;

			/* RF selection */
			value = (value & ~0x80000000) | 0x80000000;

			/* RF bank */
			value = (value & ~0x00ff0000) | (reg_pair[i + cur_index].Bank << 16);

			/* RF Index */
			value = (value & ~0x0000ffff) | reg_pair[i + cur_index].Register;

			value = cpu2le32(value);
			andes_append_cmd_msg(msg, (char *)&value, 4);
			value = 0;
			andes_append_cmd_msg(msg, (char *)&value, 4);
		}

		ret = andes_send_cmd_msg(ad, msg);
		cur_index += receive_len / 8;
		cur_len += cap->InbandPacketMaxLen;
	}

error:
	return ret;
}

int andes_read_modify_write(RTMP_ADAPTER *ad, R_M_W_REG *reg_pair, u32 num)
{
	struct cmd_msg *msg;
	unsigned int var_len = num * 12, cur_len = 0, sent_len;
	u32 value, i, cur_index = 0;
	RTMP_CHIP_CAP *cap = &ad->chipCap;
	int ret = 0;
	BOOLEAN last_packet = FALSE;

	if (!reg_pair)
		return -1;

	while (cur_len < var_len) {
		sent_len = (var_len - cur_len) > cap->InbandPacketMaxLen
				? cap->InbandPacketMaxLen : (var_len - cur_len);

		if ((sent_len < cap->InbandPacketMaxLen) ||
			(cur_len + cap->InbandPacketMaxLen) == var_len)
			last_packet = TRUE;

		msg = andes_alloc_cmd_msg(ad, sent_len);
		if (!msg) {
			ret = NDIS_STATUS_RESOURCES;
			goto error;
		}

		if (last_packet)
			andes_init_cmd_msg(msg, CMD_READ_MODIFY_WRITE, TRUE, 0, TRUE, TRUE, 0, NULL, NULL);
		else
			andes_init_cmd_msg(msg, CMD_READ_MODIFY_WRITE, FALSE, 0, FALSE, FALSE, 0, NULL, NULL);

		for (i = 0; i < (sent_len / 12); i++) {
			/* Address */
			value = cpu2le32(reg_pair[i + cur_index].Register + cap->WlanMemmapOffset);
			andes_append_cmd_msg(msg, (char *)&value, 4);

			/* ClearBitMask */
			value = cpu2le32(reg_pair[i + cur_index].ClearBitMask);
			andes_append_cmd_msg(msg, (char *)&value, 4);

			/* UpdateData */
			value = cpu2le32(reg_pair[i + cur_index].Value);
			andes_append_cmd_msg(msg, (char *)&value, 4);
		}

		ret = andes_send_cmd_msg(ad, msg);
		cur_index += (sent_len / 12);
		cur_len += cap->InbandPacketMaxLen;
	}

error:
	return ret;
}

int andes_rf_read_modify_write(RTMP_ADAPTER *ad, RF_R_M_W_REG *reg_pair, u32 num)
{
	struct cmd_msg *msg;
	unsigned int var_len = num * 12, cur_len = 0, sent_len;
	u32 value, i, cur_index = 0;
	RTMP_CHIP_CAP *cap = &ad->chipCap;
	int ret = 0;
	BOOLEAN last_packet = FALSE;

	if (!reg_pair)
		return -1;

	while (cur_len < var_len) {
		sent_len = (var_len - cur_len) > cap->InbandPacketMaxLen
				? cap->InbandPacketMaxLen : (var_len - cur_len);

		if ((sent_len < cap->InbandPacketMaxLen) ||
			(cur_len + cap->InbandPacketMaxLen) == var_len)
			last_packet = TRUE;

		msg = andes_alloc_cmd_msg(ad, sent_len);
		if (!msg) {
			ret = NDIS_STATUS_RESOURCES;
			goto error;
		}

		if (last_packet)
			andes_init_cmd_msg(msg, CMD_READ_MODIFY_WRITE, TRUE, 0, TRUE, TRUE, 0, NULL, NULL);
		else
			andes_init_cmd_msg(msg, CMD_READ_MODIFY_WRITE, FALSE, 0, FALSE, FALSE, 0, NULL, NULL);

		for (i = 0; i < sent_len / 12; i++) {
			value = 0;
			/* RF selection */
			value = (value & ~0x80000000) | 0x80000000;

			/* RF bank */
			value = (value & ~0x00ff0000) | (reg_pair[i + cur_index].Bank << 16);

			/* RF Index */
			value = (value & ~0x000000ff) | reg_pair[i + cur_index].Register;
			value = cpu2le32(value);
			andes_append_cmd_msg(msg, (char *)&value, 4);

			value = 0;
			/* ClearBitMask */
			value = (value & ~0x000000ff) | reg_pair[i + cur_index].ClearBitMask;
			value = cpu2le32(value);
			andes_append_cmd_msg(msg, (char *)&value, 4);

			value = 0;
			/* UpdateData */
			value = (value & ~0x000000ff) | reg_pair[i + cur_index].Value;
			value = cpu2le32(value);
			andes_append_cmd_msg(msg, (char *)&value, 4);
		}

		ret = andes_send_cmd_msg(ad, msg);
		cur_index += (sent_len / 12);
		cur_len += cap->InbandPacketMaxLen;
	}

error:
	return ret;
}

int andes_random_write(RTMP_ADAPTER *ad, RTMP_REG_PAIR *reg_pair, u32 num)
{
	struct cmd_msg *msg;
	unsigned int var_len = num * 8, cur_len = 0, sent_len;
	u32 value, i, cur_index = 0;
	RTMP_CHIP_CAP *cap = &ad->chipCap;
	int ret = 0;
	BOOLEAN last_packet = FALSE;

	if (!reg_pair)
		return -1;

	while (cur_len < var_len) {
		sent_len = (var_len - cur_len) > cap->InbandPacketMaxLen
			? cap->InbandPacketMaxLen : (var_len - cur_len);

		if ((sent_len < cap->InbandPacketMaxLen) || (cur_len + cap->InbandPacketMaxLen) == var_len)
			last_packet = TRUE;

		msg = andes_alloc_cmd_msg(ad, sent_len);
		if (!msg) {
			ret = NDIS_STATUS_RESOURCES;
			goto error;
		}

		if (last_packet)
			andes_init_cmd_msg(msg, CMD_RANDOM_WRITE, TRUE, 0, TRUE, TRUE, 0, NULL, NULL);
		else
			andes_init_cmd_msg(msg, CMD_RANDOM_WRITE, FALSE, 0, FALSE, FALSE, 0, NULL, NULL);

		for (i = 0; i < (sent_len / 8); i++) {
			/* Address */
			value = cpu2le32(reg_pair[i + cur_index].Register + cap->WlanMemmapOffset);
			andes_append_cmd_msg(msg, (char *)&value, 4);

			/* UpdateData */
			value = cpu2le32(reg_pair[i + cur_index].Value);
			andes_append_cmd_msg(msg, (char *)&value, 4);
		};

		ret = andes_send_cmd_msg(ad, msg);
		cur_index += (sent_len / 8);
		cur_len += cap->InbandPacketMaxLen;
	}

error:
	return ret;
}

int andes_rf_random_write(RTMP_ADAPTER *ad, BANK_RF_REG_PAIR *reg_pair, u32 num)
{
	struct cmd_msg *msg;
	unsigned int var_len = num * 8, cur_len = 0, sent_len;
	u32 value, i, cur_index = 0;
	RTMP_CHIP_CAP *cap = &ad->chipCap;
	int ret = 0;
	BOOLEAN last_packet = FALSE;

	if (!reg_pair)
		return -1;

	while (cur_len < var_len) {
		sent_len = (var_len - cur_len) > cap->InbandPacketMaxLen
				? cap->InbandPacketMaxLen : (var_len - cur_len);

		if ((sent_len < cap->InbandPacketMaxLen) ||
			(cur_len + cap->InbandPacketMaxLen) == var_len)
			last_packet = TRUE;

		msg = andes_alloc_cmd_msg(ad, sent_len);

		if (!msg) {
			ret = NDIS_STATUS_RESOURCES;
			goto error;
		}

		if (last_packet)
			andes_init_cmd_msg(msg, CMD_RANDOM_WRITE, TRUE, 0, TRUE, TRUE, 0, NULL, NULL);
		else
			andes_init_cmd_msg(msg, CMD_RANDOM_WRITE, FALSE, 0, FALSE, FALSE, 0, NULL, NULL);

		for (i = 0; i < (sent_len / 8); i++) {
			value = 0;
			/* RF selection */
			value = (value & ~0x80000000) | 0x80000000;

			/* RF bank */
			value = (value & ~0x00ff0000) | (reg_pair[i + cur_index].Bank << 16);

			/* RF Index */
			value = (value & ~0x000000ff) | reg_pair[i + cur_index].Register;

			value = cpu2le32(value);
			andes_append_cmd_msg(msg, (char *)&value, 4);

			value = 0;
			/* UpdateData */
			value = (value & ~0x000000ff) | reg_pair[i + cur_index].Value;
			value = cpu2le32(value);
			andes_append_cmd_msg(msg, (char *)&value, 4);
		}

		ret = andes_send_cmd_msg(ad, msg);
		cur_index += (sent_len / 8);
		cur_len += cap->InbandPacketMaxLen;
	}

error:
	return ret;
}

int andes_sc_random_write(RTMP_ADAPTER *ad, CR_REG *table, u32 nums, u32 flags)
{
	u32 varlen = 0, i, j;
	RTMP_REG_PAIR *sw_ch_table = NULL, temp;

	if (!table)
		return -1;

	for (i = 0; i < nums; i++) {
		if ((table[i].flags & (_BAND | _BW | _TX_RX_SETTING)) == flags) {
			varlen += sizeof(RTMP_REG_PAIR);
		}
	}

	sw_ch_table = os_alloc_mem(varlen);
	if (!sw_ch_table) {
		return -1;
	}

	for (i = 0, j = 0; i < nums; i++) {
		if ((table[i].flags & (_BAND | _BW | _TX_RX_SETTING)) == flags) {
			temp.Register = table[i].offset;
			temp.Value = table[i].value;
			NdisMoveMemory(&sw_ch_table[j], &temp, sizeof(temp));
			j++;
		}
	}

	andes_random_write(ad, sw_ch_table, varlen / sizeof(RTMP_REG_PAIR));
	os_free_mem(sw_ch_table);

	return 0;
}

int andes_sc_rf_random_write(RTMP_ADAPTER *ad, BANK_RF_CR_REG *table, u32 nums, u32 flags)
{
	u32 varlen = 0, i, j;
	BANK_RF_REG_PAIR *sw_ch_table = NULL, temp;

	if (!table)
		return -1;

	for (i = 0; i < nums; i++) {
		if ((table[i].flags & (_BAND | _BW | _TX_RX_SETTING)) == flags) {
			varlen += sizeof(BANK_RF_REG_PAIR);
		}
	}

	sw_ch_table = os_alloc_mem(varlen);
	if (!sw_ch_table) {
		return -1;
	}

	for (i = 0, j = 0; i < nums; i++) {
		if ((table[i].flags & (_BAND | _BW | _TX_RX_SETTING)) == flags) {
			temp.Bank = table[i].bank;
			temp.Register = table[i].offset;
			temp.Value = table[i].value;
			NdisMoveMemory(&sw_ch_table[j], &temp, sizeof(temp));
			j++;
		}
	}

	andes_rf_random_write(ad, sw_ch_table, varlen / sizeof(BANK_RF_REG_PAIR));
	os_free_mem(sw_ch_table);

	return 0;
}


int andes_pwr_saving(RTMP_ADAPTER *ad, u32 op, u32 level,
	 u32 listen_interval, u32 pre_tbtt_lead_time,
	 u8 tim_byte_offset, u8 tim_byte_pattern)
{
	struct cmd_msg *msg;
	unsigned int var_len;
	u32 value;
	int ret = 0;

	/* Power operation and Power Level */
	var_len = 8;

	if (op == RADIO_OFF_ADVANCE) {
		/* Listen interval, Pre-TBTT, TIM info */
		var_len += 12;
	}

	msg = andes_alloc_cmd_msg(ad, var_len);
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	andes_init_cmd_msg(msg, CMD_POWER_SAVING_OP, FALSE, 0, FALSE, FALSE, 0, NULL, NULL);

	/* Power operation */
	value = cpu2le32(op);
	andes_append_cmd_msg(msg, (char *)&value, 4);

	/* Power Level */
	value = cpu2le32(level);
	andes_append_cmd_msg(msg, (char *)&value, 4);

	if (op == RADIO_OFF_ADVANCE) {
		/* Listen interval */
		value = cpu2le32(listen_interval);
		andes_append_cmd_msg(msg, (char *)&value, 4);

		/* Pre TBTT lead time */
		value = cpu2le32(pre_tbtt_lead_time);
		andes_append_cmd_msg(msg, (char*)&value, 4);

		/* TIM Info */
		value = (value & ~0x000000ff) | tim_byte_pattern;
		value = (value & ~0x0000ff00) | (tim_byte_offset << 8);
		value = cpu2le32(value);
		andes_append_cmd_msg(msg, (char *)&value, 4);
	}

	ret = andes_send_cmd_msg(ad, msg);

error:
	return ret;
}

int andes_fun_set(RTMP_ADAPTER *ad, u32 fun_id, u32 param)
{
	struct cmd_msg *msg;
	u32 value;
	int ret = 0;

	/* Function ID and Parameter */
	msg = andes_alloc_cmd_msg(ad, 8);
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	if (fun_id != Q_SELECT)
		andes_init_cmd_msg(msg, CMD_FUN_SET_OP, TRUE, 0, TRUE, TRUE, 0, NULL, NULL);
	else
		andes_init_cmd_msg(msg, CMD_FUN_SET_OP, FALSE, 0, FALSE, FALSE, 0, NULL, NULL);

	/* Function ID */
	value = cpu2le32(fun_id);
	andes_append_cmd_msg(msg, (char *)&value, 4);

	/* Parameter */
	value = cpu2le32(param);
	andes_append_cmd_msg(msg, (char *)&value, 4);

	ret = andes_send_cmd_msg(ad, msg);

error:
	return ret;
}

int andes_calibration(RTMP_ADAPTER *ad, u32 cal_id, ANDES_CALIBRATION_PARAM *param)
{
	struct cmd_msg *msg;
	u32 value;
	int ret = 0;

	DBGPRINT(RT_DEBUG_INFO, ("%s:cal_id(%d)\n ", __FUNCTION__, cal_id));

#ifdef MT76x2
	/* Calibration ID and Parameter */
	if (cal_id == TSSI_COMPENSATION_7662 && IS_MT76x2(ad))
		msg = andes_alloc_cmd_msg(ad, 12);
	else
#endif /* MT76x2 */
		msg = andes_alloc_cmd_msg(ad, 8);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	andes_init_cmd_msg(msg, CMD_CALIBRATION_OP, TRUE, 0, TRUE, TRUE, 0, NULL, NULL);

	/* Calibration ID */
	value = cpu2le32(cal_id);
	andes_append_cmd_msg(msg, (char *)&value, 4);

	/* Parameter */
#ifdef MT76x2
	if (cal_id == TSSI_COMPENSATION_7662 && IS_MT76x2(ad)) {
		value = cpu2le32(param->mt76x2_tssi_comp_param.pa_mode);
		andes_append_cmd_msg(msg, (char *)&value, 4);

		value = cpu2le32(param->mt76x2_tssi_comp_param.tssi_slope_offset);
		andes_append_cmd_msg(msg, (char *)&value, 4);
	} else
#endif /* MT76x2 */
	{
		value = cpu2le32(param->generic);
		andes_append_cmd_msg(msg, (char *)&value, 4);
	}

	ret = andes_send_cmd_msg(ad, msg);

#ifdef MT76x2
#endif /* MT76x2 */

error:
	return ret;
}

int andes_load_cr(RTMP_ADAPTER *ad, u32 cr_type, UINT8 temp_level, UINT8 channel)
{
	struct cmd_msg *msg;
	u32 value = 0;
	int ret = 0;

	DBGPRINT(RT_DEBUG_OFF, ("%s:cr_type(%d)\n", __FUNCTION__, cr_type));

	msg = andes_alloc_cmd_msg(ad, 8);
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	andes_init_cmd_msg(msg, CMD_LOAD_CR, TRUE, 0, TRUE, TRUE, 0, NULL, NULL);

	/* CR type */
	value &= ~LOAD_CR_MODE_MASK;
	value |= LOAD_CR_MODE(cr_type);

	if (cr_type == HL_TEMP_CR_UPDATE) {
		value &= ~LOAD_CR_TEMP_LEVEL_MASK;
		value |= LOAD_CR_TEMP_LEVEL(temp_level);

		value &= ~LOAD_CR_CHL_MASK;
		value |= LOAD_CR_CHL(channel);
	}

	value = cpu2le32(value);
	andes_append_cmd_msg(msg, (char *)&value, 4);

	value = 0x80000000;
	value |= ((ad->EEPROMDefaultValue[EEPROM_NIC_CFG1_OFFSET] >> 8) & 0xFF);
	value |= ((ad->EEPROMDefaultValue[EEPROM_NIC_CFG2_OFFSET] & 0xFF) << 8 );
	value = cpu2le32(value);
	andes_append_cmd_msg(msg, (char *)&value, 4);

	ret = andes_send_cmd_msg(ad, msg);

error:
	return ret;
}

int andes_switch_channel(RTMP_ADAPTER *ad, u8 channel, BOOLEAN scan, unsigned int bw, unsigned int tx_rx_setting, u8 bbp_ch_idx)
{
	struct cmd_msg *msg;
	u32 value = 0;
	int ret;

	DBGPRINT(RT_DEBUG_INFO, ("%s:channel(%d),scan(%d),bw(%d),trx(0x%x)\n",
			__FUNCTION__, channel, scan, bw, tx_rx_setting));

	msg = andes_alloc_cmd_msg(ad, 8);
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	andes_init_cmd_msg(msg, CMD_SWITCH_CHANNEL_OP, TRUE, 0, TRUE, TRUE, 0, NULL, NULL);

	/*
	 * switch channel related param
	 * channel, scan, bw, tx_rx_setting
	*/
	value &= ~SC_PARAM1_CHL_MASK;
	value |= SC_PARAM1_CHL(channel);
	value &= ~SC_PARAM1_SCAN_MASK;
	value |= SC_PARAM1_SCAN(scan);
	value &= ~SC_PARAM1_BW_MASK;
	value |= SC_PARAM1_BW(bw);
	value = cpu2le32(value);
	andes_append_cmd_msg(msg, (char *)&value, 4);

	value = 0;
	value |= SC_PARAM2_TR_SETTING(tx_rx_setting);
	value = cpu2le32(value);
	andes_append_cmd_msg(msg, (char *)&value, 4);

	ret = andes_send_cmd_msg(ad, msg);

	mdelay(5);

	msg = andes_alloc_cmd_msg(ad, 8);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	andes_init_cmd_msg(msg, CMD_SWITCH_CHANNEL_OP, TRUE, 0, TRUE, TRUE, 0, NULL, NULL);

	/*
	 * switch channel related param
	 * channel, scan, bw, tx_rx_setting, extension channel
	*/
	value &= ~SC_PARAM1_CHL_MASK;
	value |= SC_PARAM1_CHL(channel);
	value &= ~SC_PARAM1_SCAN_MASK;
	value |= SC_PARAM1_SCAN(scan);
	value &= ~SC_PARAM1_BW_MASK;
	value |= SC_PARAM1_BW(bw);
	value = cpu2le32(value);
	andes_append_cmd_msg(msg, (char *)&value, 4);

	value = 0;
	value |= SC_PARAM2_TR_SETTING(tx_rx_setting);
	value &= ~SC_PARAM2_EXTENSION_CHL_MASK;

	if (bbp_ch_idx == 0)
		value |= SC_PARAM2_EXTENSION_CHL(0xe0);
	else if (bbp_ch_idx == 1)
		value |= SC_PARAM2_EXTENSION_CHL(0xe1);
	else if (bbp_ch_idx == 2)
		value |= SC_PARAM2_EXTENSION_CHL(0xe2);
	else if (bbp_ch_idx == 3)
		value |= SC_PARAM2_EXTENSION_CHL(0xe3);

	value = cpu2le32(value);
	andes_append_cmd_msg(msg, (char *)&value, 4);
	ret = andes_send_cmd_msg(ad, msg);

error:
	return ret;
}

int andes_init_gain(RTMP_ADAPTER *ad, UINT8 channel, BOOLEAN force_mode, unsigned int gain_from_e2p)
{
	struct cmd_msg *msg;
	u32 value = 0;
	int ret = 0;

	DBGPRINT(RT_DEBUG_INFO,
			("%s:channel(%d), force mode(%d), init gain parameter(0x%08x)\n",
			__FUNCTION__, channel, force_mode, gain_from_e2p));

	msg = andes_alloc_cmd_msg(ad, 8);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	andes_init_cmd_msg(msg, CMD_INIT_GAIN_OP, TRUE, 0, TRUE, TRUE, 0, NULL, NULL);

	/* init gain parameter#1 */
	if (force_mode == TRUE)
		value = 0x80000000;

	value |= channel;
	value = cpu2le32(value);
	andes_append_cmd_msg(msg, (char *)&value, 4);

	/* init gain parameter#2 while force mode is enabled */
	value = gain_from_e2p;
	value = cpu2le32(value);
	andes_append_cmd_msg(msg, (char *)&value, 4);
	ret = andes_send_cmd_msg(ad, msg);

error:
	return ret;
}

int andes_dynamic_vga(RTMP_ADAPTER *ad, UINT8 channel, BOOLEAN mode, BOOLEAN ext,
	int rssi, unsigned int false_cca)
{
	struct cmd_msg *msg;
	u32 value = 0;
	int rssi_val = 0, ret = 0;

	DBGPRINT(RT_DEBUG_INFO,
			("%s:channel(%d), ap/sta mode(%d), extension(%d), rssi(%d), false cca count(%d)\n",
			__FUNCTION__, channel, mode, ext, rssi, false_cca));

	msg = andes_alloc_cmd_msg(ad, 8);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	andes_init_cmd_msg(msg, CMD_DYNC_VGA_OP, TRUE, 0, TRUE, TRUE, 0, NULL, NULL);

	/* dynamic VGA parameter#1: TRUE = AP mode ; FALSE = STA mode */
	if (mode == TRUE)
		value |= 0x80000000;

	if (ext == TRUE)
		value |= 0x40000000;

	value |= channel;
	value = cpu2le32(value);
	andes_append_cmd_msg(msg, (char *)&value, 4);

	/* dynamic VGA parameter#2: RSSI (signed value) */
	rssi_val = cpu2le32(rssi);
	andes_append_cmd_msg(msg, (char *)&rssi_val, 4);

	/* dynamic VGA parameter#3: false CCA count */
	value = cpu2le32(false_cca);
	andes_append_cmd_msg(msg, (char *)&value, 4);
	ret = andes_send_cmd_msg(ad, msg);

error:
	return ret;
}

#if 0 //JB removed
static int andes_led_op(RTMP_ADAPTER *ad, u32 led_idx, u32 link_status)
{
	struct cmd_msg *msg;
	u32 value = 0;
	int ret = 0;

	DBGPRINT(RT_DEBUG_INFO, ("%s:led_idx(%d), link_status(%d)\n ",
			__FUNCTION__, led_idx, link_status));

	msg = andes_alloc_cmd_msg(ad, 8);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	andes_init_cmd_msg(msg, CMD_LED_MODE_OP, FALSE, 0, FALSE, FALSE, 0, NULL, NULL);

	/* Led index */
	value = cpu2le32(led_idx);
	andes_append_cmd_msg(msg, (char *)&value, 4);

	/* Link status */
	value = cpu2le32(link_status);
	andes_append_cmd_msg(msg, (char *)&value, 4);
	ret = andes_send_cmd_msg(ad, msg);

error:
	return ret;
}
#endif //0

#ifdef RTMP_USB_SUPPORT
void andes_usb_fw_init(RTMP_ADAPTER *ad)
{
	DBGPRINT(RT_DEBUG_OFF, ("%s\n", __FUNCTION__));

#ifdef HDR_TRANS_SUPPORT
	RTMP_IO_WRITE32(ad, HEADER_TRANS_CTRL_REG, 0X2);
	RTMP_IO_WRITE32(ad, TSO_CTRL, 0x7050);
#else
	RTMP_IO_WRITE32(ad, HEADER_TRANS_CTRL_REG, 0x0);
	RTMP_IO_WRITE32(ad, TSO_CTRL, 0x0);
#endif

	RT28XXDMAEnable(ad);
	RTMP_SET_FLAG(ad, fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD);
	andes_fun_set(ad, Q_SELECT, ad->chipCap.CmdRspRxRing);
	usb_rx_cmd_msgs_receive(ad);
	PWR_SAVING_OP(ad, RADIO_ON, 0, 0, 0, 0, 0);
}
#endif /* RTMP_USB_SUPPORT */

