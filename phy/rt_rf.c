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
	rt_rf.c

	Abstract:
	Ralink Wireless driver RF related functions

*/


#include "rt_config.h"


#ifdef RTMP_RF_RW_SUPPORT
/*
	========================================================================

	Routine Description: Read RF register through MAC with specified bit mask

	Arguments:
		pAd	- pointer to the adapter structure
		regID	- RF register ID
		pValue1	- (RF value & BitMask)
		pValue2	- (RF value & (~BitMask))
		BitMask	- bit wise mask

	Return Value:

	Note:

	========================================================================
*/
static void RTMP_ReadRF(PRTMP_ADAPTER pAd, UCHAR RegID, PUCHAR pValue1, 
	PUCHAR pValue2, UCHAR BitMask)
{
	UCHAR RfReg = 0;
	RT30xxReadRFRegister(pAd, RegID, &RfReg);
	if (pValue1 != NULL)
		*pValue1 = RfReg & BitMask;
	if (pValue2 != NULL)
		*pValue2 = RfReg & (~BitMask);
}

/*
	========================================================================

	Routine Description: Write RF register through MAC with specified bit mask

	Arguments:
		pAd	- pointer to the adapter structure
		regID	- RF register ID
		Value	- only write the part of (Value & BitMask) to RF register
		BitMask	- bit wise mask

	Return Value:

	Note:

	========================================================================
*/
void RTMP_WriteRF(PRTMP_ADAPTER pAd, UCHAR RegID, UCHAR Value, UCHAR BitMask)
{
	UCHAR RfReg = 0;
	RTMP_ReadRF(pAd, RegID, NULL, &RfReg, BitMask);
	RfReg |= ((Value) & BitMask);
	RT30xxWriteRFRegister(pAd, RegID, RfReg);
}

/*
	========================================================================

	Routine Description: Write RF register through MAC

	========================================================================
*/
NDIS_STATUS RT30xxWriteRFRegister(PRTMP_ADAPTER pAd, UCHAR regID, UCHAR value)
{
	RF_CSR_CFG_STRUC rfcsr = { { 0 } };
	UINT i = 0;
	NDIS_STATUS ret;

	ASSERT((regID <= pAd->chipCap.MaxNumOfRfId));

#ifdef RTMP_MAC_USB
	if (IS_USB_INF(pAd)) {
		RTMP_SEM_EVENT_WAIT(&pAd->reg_atomic, ret);
		if (ret != 0) {
			DBGPRINT(RT_DEBUG_ERROR, 
					("reg_atomic get failed(ret=%d)\n", 
					ret));
			return STATUS_UNSUCCESSFUL;
		}
	}
#endif /* RTMP_MAC_USB */

	ret = STATUS_UNSUCCESSFUL;
	do {
		RTMP_IO_READ32(pAd, RF_CSR_CFG, &rfcsr.word);

		if (!rfcsr.non_bank.RF_CSR_KICK)
			break;
		i++;
	} while ((i < MAX_BUSY_COUNT) && (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)));

	if ((i == MAX_BUSY_COUNT) || 
		(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))) {
		DBGPRINT(RT_DEBUG_ERROR, 
				("%s():RF Write failed(RetryCnt=%d, DevNotExistFlag=%d)\n",
				__FUNCTION__, i, RTMP_TEST_FLAG(pAd, 
				fRTMP_ADAPTER_NIC_NOT_EXIST)));
		goto done;
	}

	rfcsr.non_bank.RF_CSR_WR = 1;
	rfcsr.non_bank.RF_CSR_KICK = 1;
	rfcsr.non_bank.TESTCSR_RFACC_REGNUM = regID;

	if ((pAd->chipCap.RfReg17WtMethod == RF_REG_WT_METHOD_STEP_ON) && 
		(regID == RF_R17)) {
		UCHAR IdRf;
		UCHAR RfValue;
		BOOLEAN beAdd;

		RT30xxReadRFRegister(pAd, RF_R17, &RfValue);
		beAdd =  (RfValue < value) ? TRUE : FALSE;
		IdRf = RfValue;
		while (IdRf != value) {
			if (beAdd)
				IdRf++;
			else
				IdRf--;

			rfcsr.non_bank.RF_CSR_DATA = IdRf;
			RTMP_IO_WRITE32(pAd, RF_CSR_CFG, rfcsr.word);
			RtmpOsMsDelay(1);
		}
	}

	rfcsr.non_bank.RF_CSR_DATA = value;
	RTMP_IO_WRITE32(pAd, RF_CSR_CFG, rfcsr.word);
	ret = NDIS_STATUS_SUCCESS;

done:
#ifdef RTMP_MAC_USB
	if (IS_USB_INF(pAd)) {
		RTMP_SEM_EVENT_UP(&pAd->reg_atomic);
	}
#endif /* RTMP_MAC_USB */

	return ret;
}


/*
	========================================================================

	Routine Description: Read RF register through MAC

	========================================================================
*/
NDIS_STATUS RT30xxReadRFRegister(PRTMP_ADAPTER pAd, UCHAR regID, PUCHAR pValue)
{
	RF_CSR_CFG_STRUC rfcsr = { { 0 } };
	UINT i = 0, k = 0;
	NDIS_STATUS ret = STATUS_UNSUCCESSFUL;

	ASSERT((regID <= pAd->chipCap.MaxNumOfRfId));

#ifdef RTMP_MAC_USB
	if (IS_USB_INF(pAd)) {
		RTMP_SEM_EVENT_WAIT(&pAd->reg_atomic, i);
		if (i != 0) {
			DBGPRINT(RT_DEBUG_ERROR, 
					("reg_atomic get failed(ret=%d)\n", i));
			return STATUS_UNSUCCESSFUL;
		}
	}
#endif /* RTMP_MAC_USB */

	for (i = 0; i < MAX_BUSY_COUNT; i++) {
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
			goto done;

		RTMP_IO_READ32(pAd, RF_CSR_CFG, &rfcsr.word);

		if (rfcsr.non_bank.RF_CSR_KICK == BUSY)
				continue;

		rfcsr.word = 0;
		rfcsr.non_bank.RF_CSR_WR = 0;
		rfcsr.non_bank.RF_CSR_KICK = 1;
		rfcsr.non_bank.TESTCSR_RFACC_REGNUM = regID;
		RTMP_IO_WRITE32(pAd, RF_CSR_CFG, rfcsr.word);

		for (k = 0; k < MAX_BUSY_COUNT; k++) {
			if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
				goto done;

			RTMP_IO_READ32(pAd, RF_CSR_CFG, &rfcsr.word);

			if (rfcsr.non_bank.RF_CSR_KICK == IDLE)
				break;
		}

		if ((rfcsr.non_bank.RF_CSR_KICK == IDLE) &&
			(rfcsr.non_bank.TESTCSR_RFACC_REGNUM == regID)) {
			*pValue = (UCHAR)(rfcsr.non_bank.RF_CSR_DATA);
			break;
		}
	}

	if (rfcsr.non_bank.RF_CSR_KICK == BUSY) {
		DBGPRINT_ERR(("%s(): RF read R%d=0x%X fail, i[%d], k[%d]\n",
				__FUNCTION__, regID, rfcsr.word,i,k));
		goto done;
	}
	ret = STATUS_SUCCESS;

done:
#ifdef RTMP_MAC_USB
	if (IS_USB_INF(pAd)) {
		RTMP_SEM_EVENT_UP(&pAd->reg_atomic);
	}
#endif /* RTMP_MAC_USB */

	return ret;
}



/*
	========================================================================
	Routine Description:
		Adjust frequency offset when do channel switching or frequency calabration.

	Arguments:
		pAd		- Adapter pointer
		pRefFreqOffset	in: referenced Frequency offset   
				out: adjusted frequency offset

	========================================================================
*/
BOOLEAN RTMPAdjustFrequencyOffset(RTMP_ADAPTER *pAd,UCHAR *pRefFreqOffset)
{
	BOOLEAN RetVal = TRUE;
	UCHAR RFValue;
	UCHAR PreRFValue;
	UCHAR FreqOffset = 0;
	UCHAR HighCurrentBit = 0;

	RTMP_ReadRF(pAd, RF_R17, &FreqOffset, &HighCurrentBit, 0x7F);
	PreRFValue =  HighCurrentBit | FreqOffset;
	FreqOffset = min((*pRefFreqOffset & 0x7F), 0x5F);
	RFValue = HighCurrentBit | FreqOffset;

	if (PreRFValue != RFValue) {
#ifdef RTMP_MAC_USB
		RetVal = AsicSendCommandToMcu(pAd, 0x74, 0xff, FreqOffset, 
				PreRFValue, FALSE);
#else
		RetVal = (RT30xxWriteRFRegister(pAd, RF_R17, RFValue) == 
				STATUS_SUCCESS ? TRUE:FALSE);
#endif /* !RTMP_MAC_USB */
	}

	if (RetVal == FALSE)
		DBGPRINT(RT_DEBUG_TRACE, 
				("%s(): Error in tuning frequency offset !!\n", 
				__FUNCTION__));
	else
		*pRefFreqOffset = FreqOffset;

	return RetVal;

}

#endif /* RTMP_RF_RW_SUPPORT */

