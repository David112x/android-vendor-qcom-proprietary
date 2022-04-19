/*==========================================================================

                     FTM WLAN Header File

Description
  The header file includes enums, struct definitions for WLAN FTM packets

 Copyright (c) 2010, 2014-2020 Qualcomm Technologies, Inc.
 All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

/*===========================================================================

                         Edit History


when       who       what, where, why
--------   ---       ----------------------------------------------------------
07/11/11   karthikm  Created header file to include enums, struct for WLAN FTM
                     for Atheros support
========================================================================*/

#ifndef  FTM_WLAN_H_
#define  FTM_WLAN_H_

#ifdef CONFIG_FTM_WLAN

#include <sys/types.h>
#include <stdint.h>

#define FTM_WLAN_CMD_CODE 22

extern char g_ifname[];
/* TLV2.0 flag */
extern int tlv2_enabled;

/* Various ERROR CODES supported by the FTM WLAN module*/
typedef enum {
    FTM_ERR_CODE_PASS = 0,
    FTM_ERR_CODE_IOCTL_FAIL,
    FTM_ERR_CODE_SOCK_FAIL,
    FTM_ERR_CODE_UNRECOG_FTM
}FTM_WLAN_ERROR_CODES;


#define CONFIG_HOST_TCMD_SUPPORT  1
#define AR6000_IOCTL_SUPPORTED    1

#define ATH_MAC_LEN               6

#define HALF_SPEED_MODE           50
#define QUARTER_SPEED_MODE        51


typedef enum
{
    AT_WIENABLE_OFF  = 0,   /*OFF */
    AT_WIENABLE_ON   = 1,   /*ON signaling mode*/
    AT_WIENABLE_TEST = 2    /*Test mode¡]for WT and MT test¡^default setting after startup*/
}WLAN_AT_WIENABLE_TYPE;

typedef enum
{
    AT_WIMODE_CW      = 0,  /*CW      */
    AT_WIMODE_80211a  = 1,  /*802.11a */
    AT_WIMODE_80211b  = 2,  /*802.11b */
    AT_WIMODE_80211g  = 3,  /*802.11g */
    AT_WIMODE_80211n  = 4,  /*802.11n */
    AT_WIMODE_80211ac = 5,  /*802.11ac*/
}WLAN_AT_WIMODE_TYPE;

typedef enum
{
    AT_WIBAND_20M   = 0,  /* 20MHz*/
    AT_WIBAND_40M   = 1,  /* 40MHz*/
    AT_WIBAND_80M   = 2,  /* 80MHz*/
    AT_WIBAND_160M  = 3,  /*160MHz*/
}WLAN_AT_WIBAND_TYPE;

typedef enum
{
    AT_WIFREQ_24G   = 0,  /*2.4G*/
    AT_WIFREQ_50G   = 1,  /* 5G*/
}WLAN_AT_WIFREQ_TYPE;

typedef struct _WIFREQ_STRU
{
    uint16_t value;   /*frequency unit(MHz)¡Arange( 0¡ã65535)*/
    int16_t offset;  /*offset, unit(KHz)¡Arange(-32768¡ã32767)¡Adefault is 0*/
}WLAN_AT_WIFREQ_STRU;

typedef enum
{
    AT_FEATURE_TYPE_OFF  = 0,   /*OFF */
    AT_FEATURE_TYPE_ON   = 1,   /*ON */
}WLAN_AT_FEATURE_TYPE;

typedef enum
{
	ATH_RATE_1M=1,			//0
	ATH_RATE_2M=2,
	ATH_RATE_5_5M=3,
	ATH_RATE_6M = 4,		//4
	ATH_RATE_9M = 5,
	ATH_RATE_11M = 6,		//3
	ATH_RATE_12M = 7,
	ATH_RATE_18M = 8,
	ATH_RATE_24M = 10,
	ATH_RATE_36M = 12,
	ATH_RATE_48M = 13,
	ATH_RATE_54M = 14,		//11

	ATH_RATE_MCS0 = 15,
	ATH_RATE_MCS1 = 16,
	ATH_RATE_MCS2 = 17,
	ATH_RATE_MCS3 = 18,
	ATH_RATE_MCS4 = 19,
	ATH_RATE_MCS5 = 20,
	ATH_RATE_MCS6 = 21,
	ATH_RATE_MCS7 = 22,
	ATH_RATE_MCS8 = 23,
	ATH_RATE_MCS9 = 24,
	ATH_RATE_MCS10=25,
	ATH_RATE_MCS11=26,
	ATH_RATE_MCS12=27,
	ATH_RATE_MCS13=28,
	ATH_RATE_MCS14=29,
	ATH_RATE_MCS15=30,
}WLAN_AT_RATE_VALUE;

/* myftm tool command line options greater than ascii value
 * so, that it doesn't collide with ascii.
 */
typedef enum
{
	MYFTM_OPT_CMD_SAR = 501,
	MYFTM_OPT_PRM_SAR_INDEX8 = 502,
	MYFTM_OPT_PRM_SAR_CHAIN = 503,
	MYFTM_OPT_PRM_SAR_CCK2GLIMIT = 504,
	MYFTM_OPT_PRM_SAR_OFDM2GLIMIT = 505,
	MYFTM_OPT_PRM_SAR_OFDM5GLIMIT = 506,
	MYFTM_OPT_PRM_FLAG_DPD = 507,
	MYFTM_OPT_CMD_SETREGDMN = 508,
	MYFTM_OPT_CMD_DPDSTATUS = 509,
	MYFTM_OPT_CMD_SET_RATEBW = 510,
	MYFTM_OPT_CMD_SET_NSS = 511,
	MYFTM_OPT_CMD_SET_GI = 512,
	MYFTM_OPT_CMD_SET_OFDM_ADCM = 513,
	MYFTM_OPT_CMD_SET_OFDM_PPDU_TYPE = 514,
	MYFTM_OPT_CMD_SET_OFDM_LINKDIR = 515,
	MYFTM_OPT_CMD_SET_OFDMA_TONEPLAN = 516,
	MYFTM_OPT_CMD_SET_PREFECPAD = 517,
	MYFTM_OPT_CMD_SET_LDPCEXTRASYMBOL = 518,
	MYFTM_OPT_CMD_SET_OFDMAUL_TXCONFIG = 519,
	MYFTM_OPT_CMD_SET_DUTYCYCLE = 520,
	MYFTM_OPT_CMD_READ_REGISTER = 521,
	MYFTM_OPT_CMD_WRITE_REGISTER = 522,
	MYFTM_OPT_CMD_WRT_REGISTER_VAL = 523,
	MYFTM_OPT_CMD_SET_LOWPOWER = 524,
	MYFTM_OPT_CMD_SET_LOWPOWER_MODE = 525,
	MYFTM_OPT_CMD_SET_PHYIDMASK = 526,
	MYFTM_OPT_CMD_SET_LOWPOWER_FEATUREMASK = 527,
} myftm_cmd_line_options;

/*Enable WIFI, return value:  success: 0 fail: -1 */
int32_t WlanATSetWifiEnable(WLAN_AT_WIENABLE_TYPE onoff);
WLAN_AT_WIENABLE_TYPE WlanATGetWifiEnable(void);

/*set wifi mode, return value:  success: 0 fail: -1 */
int32_t WlanATSetWifiMode(WLAN_AT_WIMODE_TYPE mode);

/*set the bandwidth*/
int32_t WlanATSetWifiBand(WLAN_AT_WIBAND_TYPE width);

/*set the frequency*/
int32_t WlanATSetWifiFreq(WLAN_AT_WIFREQ_STRU *pFreq);

/*Set the Tx state*/
int32_t WlanATSetWifiTX(int txType);

/*Set the Rx state*/
int32_t WlanATSetWifiRX(WLAN_AT_FEATURE_TYPE onoff);
/*Get the tx control*/
WLAN_AT_FEATURE_TYPE WlanATGetWifiTX(void);

/*Set the Tx power*/
int WlanATSetWifiTxPower(char *val);

/*Get the Tx power*/
int WlanATGetWifiTxPower(void);

int WlanATSetRate(WLAN_AT_RATE_VALUE ratevalue);

int WlanATGetRate(void);

int WlanATSetWifiAntenna(int antenna);

int WlanATGetWifiAntenna(void);

int myftm_wlan_set_scpc_cal(int cal_file_opt);

/*set the Wifi RX Mode*/
int32_t WlanATSetWifiRXMode(int rx_type);
/*set the frequency2*/
int32_t WlanATSetWifiFreq2(WLAN_AT_WIFREQ_STRU *pFreq);
/*set the DBS*/
void WlanATSetDBS(char *);
/*set the phy id*/
void WlanATSetPhyid(int);
/*set the Bssid*/
void WlanATSetBssid(char *);
/*set the TX STA Addr*/
void WlanATSetTXSta(char *);
/*set the RX STA Addr*/
void WlanATSetRXSta(char *);
/*set the STA Addr*/
void WlanATSetSTAAddr(char *);
/*set the BT Addr*/
void WlanATSetBTAddr(char *);
/* set ShortGuard */
void WlanATSetShortGuard(int);
/* Command SAR */
void WlanATCmdSAR(unsigned int );
/* set SAR Index */
void WlanATCmdSARIndex(char *);
/* set SAR Chain */
void WlanATCmdSARChain(unsigned int );
/* set SAR CCK2GLIMIT */
void WlanATCmdSARCCK2gLimit(char *);
/* set SAR OFDM2GLIMIT */
void WlanATCmdSAROFDM2gLimit(char *);
/* set SAR OFDM5GLIMIT */
void WlanATCmdSAROFDM5gLimit(char *);
/* set DPD flag in PARM_FLAGS */
void WlanATCmdFlagDPDEnable();
/* Command SETREGDMN */
void WlanATCmdSETREGDMN(char *val);
/* Command DPDStatus */
void WlanATCmdDPDStatus();
/* Command rateBw */
void WlanATCmdSETRateBW(uint8_t val);
/* Command nss */
void WlanATCmdSETNSS(uint8_t val);
/* Command nss */
void WlanATCmdSETGI(uint8_t val);
/* Command pfdmadcm */
void WlanATCmdSETADCM(uint8_t val);
/* Command ofdmappdutype */
void WlanATCmdSETPPDUTYPE(uint8_t val);
/* Command ofdmalinkdir */
void WlanATCmdSETLINKDIR(uint8_t val);
/* Command SETOFDMA TONEPLAN */
int WlanATCmdSET_TONEPLAN(char *val);
/* Command SET OFDMA UPLINK TX CONFIG */
int WlanATCmdSET_OFDMAULTX();

/*Command prefecpad */
void WlanATCmdSET_PREFECPAD(uint8_t val);

/*Command ldpc extra symbol */
void WlanATCmdSET_LDPCEXTRASYMBOL(uint8_t val);

/*Command ldpc extra symbol */
void WlanATCmdSET_DUTYCYCLE(uint8_t val);

/* Command read/write value from/to Register */
void WlanATCmdWRITE_REGVAL(uint32_t val);
int WlanATCmdWRITE_REGISTER(uint32_t reg);
int WlanATCmdREAD_REGISTER(uint32_t reg);

/*Command to configured low power mode */
int WlanATCmdSET_LOWPOWER();
void WlanATCmdSET_LOWPOWER_MODE(char *val);
void WlanATCmdSET_PHYIDMASK(uint8_t val);
void WlanATCmdSET_LOWEPOWER_FEATUREMASK(uint32_t mask);

/* for unit test of test usage */
void unittest(void);
int WlanATSetWifiTPC(int pmc);
int WlanATSetWifiPktSize(int size);
void WlanATSetPaCfg(int val);
void WlanATSetDacGain(int val);
void WlanATSetGainIdx(int val);
void WlanATSetNumPkt(int val);
void WlanATSetAgg(int val);
void WlanATSetStbc(int val);
void WlanATSetLdpc(int val);
void WlanATSetWlanMode(int val);
void WlanATSetLPreamble();
void WlanATinit();
#endif /* CONFIG_FTM_WLAN */
#endif /* FTM_WLAN_H_ */
