
#ifndef __SPECTRUM_H__
#define __SPECTRUM_H__

#include "rtmp_type.h"
#include "spectrum_def.h"

void PeerSpectrumAction(PRTMP_ADAPTER pAd, MLME_QUEUE_ELEM *Elem);
NDIS_STATUS MeasureReqTabInit(PRTMP_ADAPTER pAd);
void MeasureReqTabExit(PRTMP_ADAPTER pAd);
NDIS_STATUS TpcReqTabInit(PRTMP_ADAPTER pAd);
void TpcReqTabExit(PRTMP_ADAPTER pAd);
void NotifyChSwAnnToPeerAPs(PRTMP_ADAPTER pAd, PUCHAR pRA, PUCHAR pTA,
	UINT8 ChSwMode, UINT8 Channel);

#endif /* __SPECTRUM_H__ */

