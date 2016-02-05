#ifndef __RTMP_USB_H__
#define __RTMP_USB_H__

#include "rtusb_io.h"

extern UCHAR EpToQueue[6];

// TODO: shiang-usw, fine tune BULKAGGRE_SIZE, original is 60
#ifdef INF_AMAZON_SE
#define BULKAGGRE_SIZE 	30
#else
#define BULKAGGRE_SIZE 	100
#endif

#define RXBULKAGGRE_SIZE		12
#define MAX_TXBULK_LIMIT		(LOCAL_TXBUF_SIZE*(BULKAGGRE_SIZE-1))
#define MAX_TXBULK_SIZE			(LOCAL_TXBUF_SIZE*BULKAGGRE_SIZE)
#define MAX_RXBULK_SIZE			(LOCAL_TXBUF_SIZE*RXBULKAGGRE_SIZE)
#define MAX_MLME_HANDLER_MEMORY 	20
#define CMD_RSP_BULK_SIZE		1024

/*Power saving */
#define PowerWakeCID			3
#define CID0MASK			0x000000ff
#define CID1MASK			0x0000ff00
#define CID2MASK			0x00ff0000
#define CID3MASK			0xff000000

/* Flags for Bulkflags control for bulk out data */

#define fRTUSB_BULK_OUT_DATA_NULL	0x00000001
#define fRTUSB_BULK_OUT_RTS		0x00000002
#define fRTUSB_BULK_OUT_MLME		0x00000004

#define fRTUSB_BULK_OUT_PSPOLL		0x00000010
#define fRTUSB_BULK_OUT_DATA_FRAG	0x00000020
#define fRTUSB_BULK_OUT_DATA_FRAG_2	0x00000040
#define fRTUSB_BULK_OUT_DATA_FRAG_3	0x00000080
#define fRTUSB_BULK_OUT_DATA_FRAG_4	0x00000100

#define fRTUSB_BULK_OUT_DATA_NORMAL	0x00010000
#define fRTUSB_BULK_OUT_DATA_NORMAL_2	0x00020000
#define fRTUSB_BULK_OUT_DATA_NORMAL_3	0x00040000
#define fRTUSB_BULK_OUT_DATA_NORMAL_4	0x00080000

/* TODO:move to ./ate/include/iface/ate_usb.h */
#ifdef RALINK_ATE
#define fRTUSB_BULK_OUT_DATA_ATE	0x00100000
#endif

#define FREE_HTTX_RING(_pCookie, _pipeId, _txContext)			\
{									\
	if ((_txContext)->ENextBulkOutPosition == (_txContext)->CurWritePosition)	\
	{								\
		(_txContext)->bRingEmpty = TRUE;			\
	}								\
	/*NdisInterlockedDecrement(&(_p)->TxCount); */\
}

#define NT_SUCCESS(status)	(((status) >=0) ? (TRUE):(FALSE))
#define PIRP			PVOID

#ifndef USB_ST_NOERROR
#define USB_ST_NOERROR		0
#endif

/* vendor-specific control operations */
#define USB_REQUEST_TIMEOUT_MS		300
#define VENDOR_RESET_DELAY_MS		20

#define DEVICE_CLASS_REQUEST_OUT	0x20
#define DEVICE_VENDOR_REQUEST_OUT	0x40
#define DEVICE_VENDOR_REQUEST_IN	0xc0
#define BULKOUT_MGMT_RESET_FLAG		0x80

#define RTUSB_SET_BULK_FLAG(_M, _F)	((_M)->BulkFlags |= (_F))
#define RTUSB_CLEAR_BULK_FLAG(_M, _F)	((_M)->BulkFlags &= ~(_F))
#define RTUSB_TEST_BULK_FLAG(_M, _F)	(((_M)->BulkFlags & (_F)) != 0)

//struct _MGMT_STRUC;
struct _TX_BLK;

NTSTATUS RTUSB_VendorRequest(struct _RTMP_ADAPTER *pAd, UINT32 TxFlags,
	UCHAR ReservedBits, UCHAR Request, USHORT val, USHORT idx, PVOID txbuf,
	UINT32 txbuf_len);

NTSTATUS RTUSBMultiRead(struct _RTMP_ADAPTER *pAd, USHORT Offset, UCHAR *buf, USHORT len);
NTSTATUS RTUSBMultiWrite(struct _RTMP_ADAPTER *pAd, USHORT Offset, UCHAR *buf, USHORT len, BOOLEAN bWriteHigh);
NTSTATUS RTUSBMultiWrite_nBytes(struct _RTMP_ADAPTER *pAd, USHORT Offset, UCHAR *buf, USHORT len, USHORT batchLen);
NTSTATUS RTUSBMultiWrite_OneByte(struct _RTMP_ADAPTER *pAd, USHORT Offset, UCHAR *pData);
NTSTATUS RTUSBSingleWrite(struct _RTMP_ADAPTER *pAd, USHORT Offset, USHORT val, BOOLEAN bWriteHigh);
NTSTATUS RTUSBReadBBPRegister(struct _RTMP_ADAPTER *pAd, UCHAR Id, UCHAR *pValue);
NTSTATUS RTUSBWriteBBPRegister(struct _RTMP_ADAPTER *pAd, UCHAR Id, UCHAR Value);
NTSTATUS RTUSBWriteRFRegister(struct _RTMP_ADAPTER *pAd, UINT32 Value);
NTSTATUS RTUSBWriteMACRegister(struct _RTMP_ADAPTER *pAd, USHORT Offset, UINT32 val, BOOLEAN bWriteHigh);
NTSTATUS RTUSBReadMACRegister(struct _RTMP_ADAPTER *pAd, USHORT Offset, UINT32 *val);
NTSTATUS RTUSBReadEEPROM(struct _RTMP_ADAPTER *pAd, USHORT Offset, UCHAR *buf, USHORT len);
NTSTATUS RTUSBWriteEEPROM(struct _RTMP_ADAPTER *pAd, USHORT Offset, UCHAR *buf, USHORT len);
NTSTATUS RTUSBFirmwareWrite(struct _RTMP_ADAPTER *pAd, UCHAR *pFwImage, ULONG FwLen);
NTSTATUS RTUSBVendorReset(struct _RTMP_ADAPTER *pAd);

void RTUSBDequeueCmd(PCmdQ cmdq, PCmdQElmt *pcmdqelmt);
void RTUSBBssBeaconExit(struct _RTMP_ADAPTER *pAd);
void RTUSBBssBeaconStop(struct _RTMP_ADAPTER *pAd);
void RTUSBBssBeaconStart(struct _RTMP_ADAPTER * pAd);
void RTUSBBssBeaconInit(struct _RTMP_ADAPTER *pAd);

NDIS_STATUS RTUSBFreeDescRequest(struct _RTMP_ADAPTER *pAd, UCHAR BulkOutPipeId, UINT32 req_cnt);
BOOLEAN RTUSBNeedQueueBackForAgg(struct _RTMP_ADAPTER *pAd, UCHAR BulkOutPipeId);

USHORT RtmpUSB_WriteSingleTxResource(struct _RTMP_ADAPTER *pAd, struct _TX_BLK *pTxBlk, BOOLEAN bIsLast, USHORT *freeCnt);
USHORT RtmpUSB_WriteFragTxResource(struct _RTMP_ADAPTER *pAd, struct _TX_BLK *pTxBlk, UCHAR fragNum, USHORT *freeCnt);
USHORT RtmpUSB_WriteMultiTxResource(struct _RTMP_ADAPTER *pAd, struct _TX_BLK *pTxBlk, UCHAR frmNum, USHORT *freeCnt);
void RtmpUSB_FinalWriteTxResource(struct _RTMP_ADAPTER *pAd, struct _TX_BLK *pTxBlk, USHORT mpdu_len, USHORT TxIdx);

void RtmpUSBDataLastTxIdx(struct _RTMP_ADAPTER *pAd, UCHAR QueIdx, USHORT TxIdx);
void RtmpUSBDataKickOut(struct _RTMP_ADAPTER *pAd, struct _TX_BLK *pTxBlk, UCHAR QueIdx);
int  RtmpUSBMgmtKickOut(struct _RTMP_ADAPTER *pAd, UCHAR QIdx, PNDIS_PACKET pkt, UCHAR *pSrcBufVA, UINT SrcBufLen);
void RtmpUSBNullFrameKickOut(struct _RTMP_ADAPTER *pAd, UCHAR QIdx, UCHAR *pNullFrm, UINT32 frmLen);
void RTUSBWatchDog(struct _RTMP_ADAPTER *pAd);
void RtmpUsbStaAsicForceWakeupTimeout(PVOID arg1, PVOID FuncContext, PVOID arg2, PVOID arg3);
void RT28xxUsbStaAsicForceWakeup(struct _RTMP_ADAPTER *pAd, BOOLEAN bFromTx);
void RT28xxUsbStaAsicSleepThenAutoWakeup(struct _RTMP_ADAPTER *pAd, USHORT TbttNumToNextWakeUp);
void RT28xxUsbMlmeRadioOn(struct _RTMP_ADAPTER *pAd);
void RT28xxUsbMlmeRadioOFF(struct _RTMP_ADAPTER *pAd);
void RT28xxUsbAsicRadioOn(struct _RTMP_ADAPTER *pAd);
void RT28xxUsbAsicRadioOff(struct _RTMP_ADAPTER *pAd);

struct usb_control {
	BOOLEAN usb_aggregation;
};

#endif /* __RTMP_USB_H__ */

