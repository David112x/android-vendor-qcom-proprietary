/*============================================================================

  Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __DEMOSAIC_ABF44_REG_H__
#define __DEMOSAIC_ABF44_REG_H__

/* changed from 440
   In the VFE 4.0 ABF registers were part of the demosaic register definations.
   In VFE 4.4 the ABF has its own register i.e. and could either use the ABF 2.0 i.e
   the one used in the VFE 4.0 or use ABF 3.0 used in the VFE 4.4. */


#define ISP_ABF3_OFF 0x00000524
#define ISP_ABF3_LEN 31

#define WTABLE_ADJ_VAL 4

#define SHORT_NOISE_SCALE 4.0f
#define NOISE_OFFSET_MAX_VAL (1<<16) - 1
#define DENOISE_STRENGTH_MAX_VAL 256
#define NOISE_OFFSET_SCALE 4

#define    IIRLEVEL0EN      1
#define    IIRLEVEL1EN      2
#define    CROSSP1LEVEL0EN  4
#define    CROSSP1LEVEL1EN  5
#define    MINMAXEN         6

#define ABF_CGC_OVERRIDE TRUE
#define ABF_CGC_OVERRIDE_REGISTER 0x2C
#define ABF_CGC_OVERRIDE_BIT 7

/* Registers for VFE 4.4 */
typedef struct ISP_ABF3_Cfg {
  /* Demosaic Config */
  uint32_t    iirLevel0En                :  1;
  uint32_t    iirLevel1En                :  1;
  uint32_t    lutBankSel                 :  1;
  uint32_t    crossPlLevel0En            :  1;
  uint32_t    crossPlLevel1En            :  1;
  uint32_t    minMaxEn                   :  1;
  /* reserved 6-31 */
  uint32_t    abf3En                     :  1;
  uint32_t    pipeFlushOvd               :  1;
  uint32_t    flushHaltOvd               :  1;
  uint32_t    /*reserved*/               :  7;
  uint32_t    pipeFlushCnt               : 13;
  uint32_t    /*reserved*/               :  3;
}__attribute__((packed, aligned(4))) ISP_ABF3_Cfg;


typedef struct ISP_ABF3_Lv1Cfg {
  /*   CFG_0  */
  uint32_t    distGrGb_0                : 3;
  uint32_t    /* reserved */            : 1;
  uint32_t    distGrGb_1                : 3;
  uint32_t    /* reserved */            : 1;
  uint32_t    distGrGb_2                : 3;
  uint32_t    /* reserved */            : 1;
  uint32_t    distGrGb_3                : 3;
  uint32_t    /* reserved */            : 1;
  uint32_t    distGrGb_4                : 3;
  uint32_t    /* reserved */            : 5;
  uint32_t    minMaxSelGrGb0            : 2;
  uint32_t    /* reserved */            : 2;
  uint32_t    minMaxSelGrGb1            : 2;
  uint32_t    /* reserved */            : 2;
  /*   CFG_1  */
  uint32_t    distRb0                   : 3;
  uint32_t    /* reserved */            : 1;
  uint32_t    distRb1                   : 3;
  uint32_t    /* reserved */            : 1;
  uint32_t    distRb2                   : 3;
  uint32_t    /* reserved */            : 1;
  uint32_t    distRb3                   : 3;
  uint32_t    /* reserved */            : 1;
  uint32_t    distRb4                   : 3;
  uint32_t    /* reserved */            : 5;
  uint32_t    minMaxSelRb0              : 2;
  uint32_t    /* reserved */            : 2;
  uint32_t    minMaxSelRb1              : 2;
  uint32_t    /* reserved */            : 2;
}__attribute__((packed, aligned(4))) ISP_ABF3_Lv1Cfg;

typedef struct ISP_ABF3_ChannelCfg {
  /*   CFG_0  */
  uint32_t    gr_noiseScale0Lv0            : 12;
  uint32_t    gr_wTableAdjLv0              :  4;
  uint32_t    gr_noiseScale0Lv1            : 12;
  uint32_t    gr_wTableAdjLv1              :  4;
  /*   CFG_1  */
  uint32_t    gr_noiseScale1Lv0            : 16;
  uint32_t    gr_noiseScale1Lv1            : 16;
  /*   CFG_2  */
  uint32_t    gr_noiseScale2Lv0            : 12;
  uint32_t    /* reserved */               :  4;
  uint32_t    gr_noiseScale2Lv1            : 12;
  uint32_t    /* reserved */               :  4;
  /*   CFG_3  */
  uint32_t    gr_noiseOffLv0               : 16;
  uint32_t    gr_noiseOffLv1               : 16;
  /*   CFG_4  */
  uint32_t    gr_curvOffLv0                :  7;
  uint32_t    /* reserved */               :  9;
  uint32_t    gr_curvOffLv1                :  7;
  uint32_t    /* reserved */               :  9;
  /*   CFG_5  */
  uint32_t    gr_sftThrdNoiseScaleLv0      : 12;
  uint32_t    gr_sftThrdNoiseShiftLv0      :  4;
  uint32_t    gr_sftThrdNoiseScaleLv1      : 12;
  uint32_t    gr_sftThrdNoiseShiftLv1      :  4;


  /*   CFG_0  */
  uint32_t    gb_noiseScale0Lv0            : 12;
  uint32_t    gb_wTableAdjLv0              :  4;
  uint32_t    gb_noiseScale0Lv1            : 12;
  uint32_t    gb_wTableAdjLv1              :  4;
  /*   CFG_1  */
  uint32_t    gb_noiseScale1Lv0            : 16;
  uint32_t    gb_noiseScale1Lv1            : 16;
  /*   CFG_2  */
  uint32_t    gb_noiseScale2Lv0            : 12;
  uint32_t    /* reserved */               :  4;
  uint32_t    gb_noiseScale2Lv1            : 12;
  uint32_t    /* reserved */               :  4;
  /*   CFG_3  */
  uint32_t    gb_noiseOffLv0               : 16;
  uint32_t    gb_noiseOffLv1               : 16;
  /*   CFG_4  */
  uint32_t    gb_curvOffLv0                :  7;
  uint32_t    /* reserved */               :  9;
  uint32_t    gb_curvOffLv1                :  7;
  uint32_t    /* reserved */               :  9;
  /*   CFG_5  */
  uint32_t    gb_sftThrdNoiseScaleLv0      : 12;
  uint32_t    gb_sftThrdNoiseShiftLv0      :  4;
  uint32_t    gb_sftThrdNoiseScaleLv1      : 12;
  uint32_t    gb_sftThrdNoiseShiftLv1      :  4;


  /*   CFG_0  */
  uint32_t    r_noiseScale0Lv0             : 12;
  uint32_t    r_wTableAdjLv0               :  4;
  uint32_t    r_noiseScale0Lv1             : 12;
  uint32_t    r_wTableAdjLv1               :  4;
  /*   CFG_1  */
  uint32_t    r_noiseScale1Lv0             : 16;
  uint32_t    r_noiseScale1Lv1             : 16;
  /*   CFG_2  */
  uint32_t    r_noiseScale2Lv0             : 12;
  uint32_t    /* reserved */               :  4;
  uint32_t    r_noiseScale2Lv1             : 12;
  uint32_t    /* reserved */               :  4;
  /*   CFG_3  */
  uint32_t    r_noiseOffLv0                : 16;
  uint32_t    r_noiseOffLv1                : 16;
  /*   CFG_4  */
  uint32_t    r_curvOffLv0                 :  7;
  uint32_t    /* reserved */               :  9;
  uint32_t    r_curvOffLv1                 :  7;
  uint32_t    /* reserved */               :  9;
  /*   CFG_5  */
  uint32_t    r_sftThrdNoiseScaleLv0       : 12;
  uint32_t    r_sftThrdNoiseShiftLv0       :  4;
  uint32_t    r_sftThrdNoiseScaleLv1       : 12;
  uint32_t    r_sftThrdNoiseShiftLv1       :  4;



  /*   CFG_0  */
  uint32_t    b_noiseScale0Lv0             : 12;
  uint32_t    b_wTableAdjLv0               :  4;
  uint32_t    b_noiseScale0Lv1             : 12;
  uint32_t    b_wTableAdjLv1               :  4;
  /*   CFG_1  */
  uint32_t    b_noiseScale1Lv0             : 16;
  uint32_t    b_noiseScale1Lv1             : 16;
  /*   CFG_2  */
  uint32_t    b_noiseScale2Lv0             : 12;
  uint32_t    /* reserved */               :  4;
  uint32_t    b_noiseScale2Lv1             : 12;
  uint32_t    /* reserved */               :  4;
  /*   CFG_3  */
  uint32_t    b_noiseOffLv0                : 16;
  uint32_t    b_noiseOffLv1                : 16;
  /*   CFG_4  */
  uint32_t    b_curvOffLv0                 :  7;
  uint32_t    /* reserved */               :  9;
  uint32_t    b_curvOffLv1                 :  7;
  uint32_t    /* reserved */               :  9;
  /*   CFG_5  */
  uint32_t    b_sftThrdNoiseScaleLv0       : 12;
  uint32_t    b_sftThrdNoiseShiftLv0       :  4;
  uint32_t    b_sftThrdNoiseScaleLv1       : 12;
  uint32_t    b_sftThrdNoiseShiftLv1       :  4;
  /*  gr CFG_5  */
  uint32_t    gr_filterStrLv0              :  9;
  uint32_t    /* reserved */               :  7;
  uint32_t    gr_filterStrLv1              :  9;
  uint32_t    /* reserved */               :  7;
  /*  gb CFG_5  */
  uint32_t    gb_filterStrLv0              :  9;
  uint32_t    /* reserved */               :  7;
  uint32_t    gb_filterStrLv1              :  9;
  uint32_t    /* reserved */               :  7;
  /*  r CFG_5  */
  uint32_t    r_filterStrLv0               :  9;
  uint32_t    /* reserved */               :  7;
  uint32_t    r_filterStrLv1               :  9;
  uint32_t    /* reserved */               :  7;
  /*  gb CFG_5  */
  uint32_t    b_filterStrLv0               :  9;
  uint32_t    /* reserved */               :  7;
  uint32_t    b_filterStrLv1               :  9;
  uint32_t    /* reserved */               :  7;
}__attribute__((packed, aligned(4))) ISP_ABF3_ChannelCfg;


typedef struct ISP_ABF3_CmdType {
  ISP_ABF3_Cfg        cfg;
  ISP_ABF3_Lv1Cfg     level1;
  ISP_ABF3_ChannelCfg ch;
}__attribute__((packed, aligned(4))) ISP_ABF3_CmdType;

#endif //__DEMOSAIC_ABF44_REG_H__
