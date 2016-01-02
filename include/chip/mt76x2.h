#ifndef __MT76X2_H__
#define __MT76X2_H__

#include "../mcu/mcu_and.h"
#include "../phy/mt_rf.h"

struct _RTMP_ADAPTER;

#define MAX_RF_ID	127
#define MAC_RF_BANK 7

void mt76x2_init(struct _RTMP_ADAPTER *ad);
void mt76x2_adjust_per_rate_pwr_delta(struct _RTMP_ADAPTER *ad, u8 channel, char delta_pwr);
void mt76x2_get_tx_pwr_per_rate(struct _RTMP_ADAPTER *ad);
int mt76x2_read_chl_pwr(struct _RTMP_ADAPTER *ad);
void mt76x2_pwrOn(struct _RTMP_ADAPTER *ad);
void mt76x2_calibration(struct _RTMP_ADAPTER *ad, u8 channel);
void mt76x2_external_pa_rf_dac_control(struct _RTMP_ADAPTER *ad, u8 channel);
void mt76x2_tssi_calibration(struct _RTMP_ADAPTER *ad, u8 channel);
void mt76x2_tssi_compensation(struct _RTMP_ADAPTER *ad, u8 channel);
int mt76x2_set_ed_cca(struct _RTMP_ADAPTER *ad, u8 enable);
int mt76x2_reinit_agc_gain(struct _RTMP_ADAPTER *ad, u8 channel);
int mt76x2_reinit_hi_lna_gain(struct _RTMP_ADAPTER *ad, u8 channel);
void mt76x2_get_external_lna_gain(struct _RTMP_ADAPTER *ad);
void mt76x2_get_agc_gain(struct _RTMP_ADAPTER *ad, BOOLEAN init_phase);
//jb was char get_chl_grp(u8 channel);
int get_chl_grp(u8 channel);
void percentage_delta_pwr(struct _RTMP_ADAPTER *ad);

void mt76x2_get_current_temp(struct _RTMP_ADAPTER *ad);
void mt76x2_read_temp_info_from_eeprom(struct _RTMP_ADAPTER *ad);
#ifdef RTMP_TEMPERATURE_TX_ALC
void mt76x2_read_tx_alc_info_from_eeprom(struct _RTMP_ADAPTER *ad);
void mt76x2_temp_tx_alc(struct _RTMP_ADAPTER *ad);
#endif /* RTMP_TEMPERATURE_TX_ALC */

#ifdef SINGLE_SKU_V2
void mt76x2_single_sku(struct _RTMP_ADAPTER *ad, u8 channel);
void mt76x2_read_single_sku_info_from_eeprom(struct _RTMP_ADAPTER *ad);
void mt76x2_make_up_rate_pwr_table(struct _RTMP_ADAPTER *ad);
UCHAR mt76x2_get_sku_channel_base_pwr(struct _RTMP_ADAPTER *ad, u8 channel);
void mt76x2_update_per_rate_pwr(struct _RTMP_ADAPTER *ad);
UCHAR mt76x2_update_sku_pwr(struct _RTMP_ADAPTER *ad, u8 channel);
#endif /* SINGLE_SKU_V2 */

#ifdef ED_MONITOR
void mt7612_set_ed_cca(struct _RTMP_ADAPTER *ad, BOOLEAN enable);
#endif /* ED_MONITOR */

#ifdef RALINK_ATE
VOID mt76x2_ate_do_calibration(
	struct _RTMP_ADAPTER *ad, UINT32 cal_id, UINT32 param);
#endif /* RALINK_ATE */

struct mt76x2_frequency_item {
	u8 channel;
	u32 fcal_target;
	u32 sdm_integer;
	u32 sdm_fraction;
};

typedef struct _MT76x2_RATE_PWR_ITEM {
	CHAR mcs_pwr;
} MT76x2_RATE_PWR_ITEM, *PMT76x2_RATE_PWR_ITEM;

typedef struct _MT76x2_RATE_PWR_TABLE {
	MT76x2_RATE_PWR_ITEM CCK[4];
	MT76x2_RATE_PWR_ITEM OFDM[8];
	MT76x2_RATE_PWR_ITEM HT[16];
	MT76x2_RATE_PWR_ITEM VHT1SS[10];
	MT76x2_RATE_PWR_ITEM VHT2SS[10];
	MT76x2_RATE_PWR_ITEM STBC[10];
	MT76x2_RATE_PWR_ITEM MCS32;
} MT76x2_RATE_PWR_Table, *PMT76x2_RATE_PWR_Table;


#endif
