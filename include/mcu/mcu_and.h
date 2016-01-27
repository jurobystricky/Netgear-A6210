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
	mcu_and.h

*/

#ifndef __MCU_AND_H__
#define __MCU_AND_H__

#include "mcu.h"

#ifndef WORKQUEUE_BH
#include <linux/interrupt.h>
#endif

struct _RTMP_ADAPTER;
struct _BANK_RF_REG_PAIR;
struct _R_M_W_REG;
struct _RF_R_M_W_REG;

#define CPU_CTL			0x0704
#define CLOCK_CTL		0x0708
#define RESET_CTL		0x070C
#define INT_LEVEL		0x0718
#define COM_REG0		0x0730
#define COM_REG1		0x0734
#define COM_REG2		0x0738
#define COM_REG3		0x073C
#define PCIE_REMAP_BASE1	0x0740
#define PCIE_REMAP_BASE2	0x0744
#define PCIE_REMAP_BASE3	0x0748
#define PCIE_REMAP_BASE4	0x074C
#define LED_CTRL		0x0770
#define LED_TX_BLINK_0		0x0774
#define LED_TX_BLINK_1		0x0778
#define LED0_S0			0x077C
#define LED0_S1			0x0780
#define SEMAPHORE_00		0x07B0
#define SEMAPHORE_01		0x07B4
#define SEMAPHORE_02		0x07B8
#define SEMAPHORE_03		0x07BC

#define MCU_WAIT_ACK_CMD_THRESHOLD 0x0f
#define MCU_RX_CMD_THRESHOLD 	0x0f

#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _TXINFO_NMAC_CMD_PKT{
	UINT32 info_type:2;
	UINT32 d_port:3;
	UINT32 cmd_type:7;
	UINT32 cmd_seq:4;
	UINT32 pkt_len:16;
}TXINFO_NMAC_CMD_PKT;
#else
typedef struct GNU_PACKED _TXINFO_NMAC_CMD_PKT {
	UINT32 pkt_len:16;
	UINT32 cmd_seq:4;
	UINT32 cmd_type:7;
	UINT32 d_port:3;
	UINT32 info_type:2;
}TXINFO_NMAC_CMD_PKT;
#endif /* RT_BIG_ENDIAN */

enum cmd_msg_state {
	illegal,
	tx_start,
	tx_kickout_fail,
	tx_timeout_fail,
	tx_retransmit,
	tx_done,
	wait_cmd_out,
	wait_cmd_out_and_ack,
	wait_ack,
	rx_start,
	rx_receive_fail,
	rx_done,
};

enum {
	MCU_INIT,
	MCU_TX_HALT,
	MCU_RX_HALT,
};

enum {
	CMD_ACK,
};

enum cmd_msg_error_type {
	error_tx_kickout_fail,
	error_tx_timeout_fail,
	error_rx_receive_fail,
};

struct MCU_CTRL {
	u8 cmd_seq;
	unsigned long flags;
#ifndef WORKQUEUE_BH
	RTMP_NET_TASK_STRUCT cmd_msg_task;
#else
	struct tasklet_struct cmd_msg_task;
#endif
	spinlock_t txq_lock;
	DL_LIST txq;
	spinlock_t rxq_lock;
	DL_LIST rxq;
	spinlock_t ackq_lock;
	DL_LIST ackq;
	spinlock_t kickq_lock;
	DL_LIST kickq;
	spinlock_t tx_doneq_lock;
	DL_LIST tx_doneq;
	spinlock_t rx_doneq_lock;
	DL_LIST rx_doneq;
	unsigned long tx_kickout_fail_count;
	unsigned long tx_timeout_fail_count;
	unsigned long rx_receive_fail_count;
	unsigned long alloc_cmd_msg;
	unsigned long free_cmd_msg;
	BOOLEAN power_on;
	BOOLEAN dpd_on;
	struct _RTMP_ADAPTER *ad;
};

struct cmd_msg;
typedef void (*MSG_RSP_HANDLER)(struct cmd_msg *msg, char *payload, u16 payload_len);
typedef void (*MSG_EVENT_HANDLER)(struct _RTMP_ADAPTER *ad, char *payload, u16 payload_len);

struct cmd_msg_cb {
	struct cmd_msg *msg;
};

#define USB_END_PADDING			4
#define UPLOAD_PATCH_UNIT		2048
#define PATCH_INFO_SIZE			30
#define FW_INFO_SIZE			32
#define IV_SIZE				0x40
#define GET_SEMAPHORE_RETRY_MAX		600
#define UPLOAD_FW_UNIT			14592
#define UPLOAD_FW_TIMEOUT_MS		2000
#define CMD_MSG_CB(pkt) ((struct cmd_msg_cb *)(GET_OS_PKT_CB(pkt)))
#define CMD_MSG_RETRANSMIT_TIMES	3
#define CMD_MSG_TIMEOUT 		500

struct cmd_msg {
	DL_LIST list;
	u8 type;
	u8 seq;
	u16 timeout;
	u16 rsp_payload_len;
	BOOLEAN need_wait;
	BOOLEAN need_rsp;
	BOOLEAN need_retransmit;

#ifdef RTMP_USB_SUPPORT
	struct completion ack_done; 
#endif
	char *rsp_payload;
	MSG_RSP_HANDLER rsp_handler;
	enum cmd_msg_state state;
	void *priv;
	PNDIS_PACKET net_pkt;
#ifdef RTMP_USB_SUPPORT
	PURB urb;
#endif
	int retransmit_times;
};

/*
 * Calibration ID
 */
#define MAX_CALIBRATION_WAIT_TIME	100

enum CALIBRATION_ID {
	R_CALIBRATION = 1,
	RXDCOC_CALIBRATION = 2,
	LC_CALIBRATION = 3,
	LOFT_CALIBRATION = 4,
	TXIQ_CALIBRATION = 5,
	BW_CALIBRATION = 6,
	DPD_CALIBRATION = 7,
	RXIQ_CALIBRATION = 8,
	TXDCOC_CALIBRATION = 9,
	RX_GROUP_DELAY_CALIBRATION = 10,
	TX_GROUP_DELAY_CALIBRATION = 11,
	FULL_CALIBRATION_ID = 0xFF,
};

enum CALIBRATION_ID_7662 {
	R_CALIBRATION_7662 = 1,
	TEMP_SENSOR_CALIBRATION_7662,
	RXDCOC_CALIBRATION_7662,
	RC_CALIBRATION_7662,
	SX_LOGEN_CALIBRATION_7662,
	LC_CALIBRATION_7662,
	TX_LOFT_CALIBRATION_7662,
	TXIQ_CALIBRATION_7662,
	TSSI_CALIBRATION_7662,
	TSSI_COMPENSATION_7662,
	DPD_CALIBRATION_7662,
	RXIQC_FI_CALIBRATION_7662,
	RXIQC_FD_CALIBRATION_7662,
};

enum CALIBRATION_TYPE {
	FULL_CALIBRATION,
	PARTIAL_CALIBRATION,
};

enum SWITCH_CHANNEL_STAGE {
	NORMAL_OPERATING = 0x01,
	SCANNING = 0x02,
	TEMPERATURE_CHANGE = 0x04,
};

/*
 * Function set ID
 */
enum FUN_ID {
	Q_SELECT = 1,
	BW_SETTING = 2,
	USB2_SW_DISCONNECT = 2,
	USB3_SW_DISCONNECT = 3,
	LOG_FW_DEBUG_MSG = 4,
	GET_FW_VERSION = 5,
};

/*
 * Command response RX Ring selection
 */
enum RX_RING_ID {
	RX_RING0,
	RX_RING1,
};

enum BW_SETTING {
	BW20 = 1,
	BW40 = 2,
	BW10 = 4,
	BW80 = 8,
};

/*
 * FW debug message parameters
 */
enum FW_DEBUG_SETTING {
	DISABLE_DEBUG_MSG = 0,
	DEBUG_MSG_TO_UART,
	DEBUG_MSG_TO_HOST,
};

enum CR_TYPE {
	RF_CR,
	BBP_CR,
	RF_BBP_CR,
	HL_TEMP_CR_UPDATE,
};

enum TEMPERATURE_LEVEL_7662 {
	NORMAL_TEMP_7662, /* 0~60 */
	LOW_TEMP_7662, /* < 0 */
	HIGH_TEMP_7662, /* > 60 */
};

#define LOAD_CR_MODE_MASK (0xff)
#define LOAD_CR_MODE(p) (((p) & 0xff))
#define LOAD_CR_TEMP_LEVEL_MASK (0xff << 8)
#define LOAD_CR_TEMP_LEVEL(p) (((p) & 0xff) << 8)
#define LOAD_CR_CHL_MASK (0xff << 16)
#define LOAD_CR_CHL(p) (((p) & 0xff) << 16)

#define SC_PARAM1_CHL_MASK (0xff)
#define SC_PARAM1_CHL(p) (((p) & 0xff))
#define SC_PARAM1_SCAN_MASK (0xff << 8)
#define SC_PARAM1_SCAN(p) (((p) & 0xff) << 8)
#define SC_PARAM1_BW_MASK (0xff << 16)
#define SC_PARAM1_BW(p) (((p) & 0xff) << 16)

#define SC_PARAM2_TR_SETTING_MASK (0xffff)
#define SC_PARAM2_TR_SETTING(p) (((p) & 0xffff))
#define SC_PARAM2_EXTENSION_CHL_MASK (0xff << 16)
#define SC_PARAM2_EXTENSION_CHL(p) (((p) & 0xff) << 16)

#define TSSI_PARAM2_SLOPE0_MASK (0xff)
#define TSSI_PARAM2_SLOPE0(p) (((p) & 0xff))
#define TSSI_PARAM2_SLOPE1_MASK (0xff << 8)
#define TSSI_PARAM2_SLOPE1(p) (((p) & 0xff) << 8)
#define TSSI_PARAM2_OFFSET0_MASK (0xff << 16)
#define TSSI_PARAM2_OFFSET0(p) (((p) & 0xff) << 16)
#define TSSI_PARAM2_OFFSET1_MASK (0xff << 24)
#define TSSI_PARAM2_OFFSET1(p) (((p) & 0xff) << 24)

/*
 * Command type table 
 */
enum CMD_TYPE {
	CMD_FUN_SET_OP = 1,
	CMD_LOAD_CR,
	CMD_INIT_GAIN_OP = 3,
	CMD_DYNC_VGA_OP = 6,
	CMD_TDLS_CH_SW = 7,
	CMD_BURST_WRITE = 8,
	CMD_READ_MODIFY_WRITE,
	CMD_RANDOM_READ,
	CMD_BURST_READ,
	CMD_RANDOM_WRITE = 12,
	CMD_LED_MODE_OP = 16,
	CMD_POWER_SAVING_OP = 20,
	CMD_WOW_CONFIG,
	CMD_WOW_QUERY,
	CMD_WOW_FEATURE = 24,
	CMD_CARRIER_DETECT_OP = 28,
	CMD_RADOR_DETECT_OP,
	CMD_SWITCH_CHANNEL_OP,
	CMD_CALIBRATION_OP,
	CMD_BEACON_OP,
	CMD_ANTENNA_OP
};

/*
 * Event type table
 */
enum EVENT_TYPE {
	CMD_DONE,
	CMD_ERROR,
	CMD_RETRY,
	EVENT_PWR_RSP,
	EVENT_WOW_RSP,
	EVENT_CARRIER_DETECT_RSP,
	EVENT_DFS_DETECT_RSP,
};

enum mcu_skb_state {
	ILLEAGAL = 0,
	MCU_CMD_START,
	MCU_CMD_DONE,
	MCU_RSP_START,
	MCU_RSP_DONE,
	MCU_RSP_CLEANUP,
	UNLINK_START,
};

struct mcu_skb_data {
	enum mcu_skb_state state;
};

typedef	union _ANDES_CALIBRATION_PARAM {
	UINT32 generic;
#ifdef MT76x2
	struct {
		UINT32 pa_mode;
		UINT32 tssi_slope_offset;
	} mt76x2_tssi_comp_param;
#endif
} ANDES_CALIBRATION_PARAM;

enum CALIBRATION_TEST_TYPE {
	CAL_ROBUST_TEST=0,

};

#ifdef RTMP_MAC_USB
int andes_usb_loadfw(struct _RTMP_ADAPTER *ad);
int andes_usb_erasefw(struct _RTMP_ADAPTER *ad);
int andes_usb_load_rom_patch(struct _RTMP_ADAPTER *ad);
int andes_usb_erase_rom_patch(struct _RTMP_ADAPTER *ad);
void andes_usb_fw_init(struct _RTMP_ADAPTER *ad);
#endif /* RTMP_MAC_USB */
void andes_ctrl_init(struct _RTMP_ADAPTER *ad);
void andes_ctrl_enable(struct _RTMP_ADAPTER *ad);
void andes_ctrl_disable(struct _RTMP_ADAPTER *ad);
void andes_ctrl_exit(struct _RTMP_ADAPTER *ad);
int andes_send_cmd_msg(struct _RTMP_ADAPTER *ad, struct cmd_msg *msg);
int andes_burst_write(struct _RTMP_ADAPTER *ad, u32 offset, u32 *data, u32 cnt);
int andes_burst_read(struct _RTMP_ADAPTER *ad, u32 offset, u32 cnt, u32 *data);
int andes_random_read(struct _RTMP_ADAPTER *ad, RTMP_REG_PAIR *reg_pair, u32 num);
int andes_rf_random_read(struct _RTMP_ADAPTER *ad, struct _BANK_RF_REG_PAIR *reg_pair, u32 num);
int andes_read_modify_write(struct _RTMP_ADAPTER *ad, struct _R_M_W_REG *reg_pair, u32 num);
int andes_rf_read_modify_write(struct _RTMP_ADAPTER *ad, struct _RF_R_M_W_REG *reg_pair, u32 num);
int andes_random_write(struct _RTMP_ADAPTER *ad, RTMP_REG_PAIR *reg_pair, u32 num);
int andes_rf_random_write(struct _RTMP_ADAPTER *ad, struct _BANK_RF_REG_PAIR *reg_pair, u32 num);
int andes_sc_random_write(struct _RTMP_ADAPTER *ad, CR_REG *table, u32 nums, u32 flags);
int andes_sc_rf_random_write(struct _RTMP_ADAPTER *ad, BANK_RF_CR_REG *table, u32 nums, u32 flags);
int andes_fun_set(struct _RTMP_ADAPTER *ad, u32 fun_id, u32 param);
int andes_pwr_saving(struct _RTMP_ADAPTER *ad, u32 op, u32 level, u32 listen_interval, 
	u32 pre_tbtt_lead_time, u8 tim_byte_offset, u8 tim_byte_pattern);
int andes_calibration(struct _RTMP_ADAPTER *ad, u32 cal_id, ANDES_CALIBRATION_PARAM *param);
BOOLEAN is_inband_cmd_processing(struct _RTMP_ADAPTER *ad);
void andes_cmd_msg_bh(unsigned long param);
int usb_rx_cmd_msg_submit(struct _RTMP_ADAPTER *ad);
int usb_rx_cmd_msgs_receive(struct _RTMP_ADAPTER *ad);
void pci_kick_out_cmd_msg_complete(PNDIS_PACKET net_pkt);
int andes_load_cr(struct _RTMP_ADAPTER *ad, u32 cr_type, UINT8 temp_level, UINT8 channel);
int andes_switch_channel(struct _RTMP_ADAPTER *ad, u8 channel, BOOLEAN scan, unsigned int bw, unsigned int tx_rx_setting, u8 bbp_ch_idx);
int andes_init_gain(struct _RTMP_ADAPTER *ad, UINT8 channel, BOOLEAN force_mode, unsigned int gain_from_e2p);
int andes_dynamic_vga(struct _RTMP_ADAPTER *ad, UINT8 channel, BOOLEAN mode, BOOLEAN ext, int rssi, unsigned int false_cca);

#endif /* __MCU_AND_H__ */

