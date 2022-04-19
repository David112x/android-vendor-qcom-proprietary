/* abf48_reg.h
 *
 * Copyright (c) 2013-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __ABF48_REG_H__
#define __ABF48_REG_H__

/* changed from 440
 * In the VFE 4.0 ABF registers were part of the demosaic register definations.
 * In VFE 4.4 the ABF has its own register i.e. and could either use the ABF 2.0
 * i.e the one used in the VFE 4.0 or use ABF 3.0 used in the VFE 4.4.
 */

#define ISP_ABF34_OFF 0x000005E8
#define ISP_ABF34_LEN 23


/* ABF registers */
#define ISP_ABF34_CFG        0x000005E8
#define ISP_ABF34_CFG_LEN    1
#define ISP_ABF34_CFG2       0x000005F4
#define ISP_ABF34_CFG2_LEN   22

#define ABF_CGC_OVERRIDE TRUE
#define ABF_CGC_OVERRIDE_REGISTER 0x2C
#define ABF_CGC_OVERRIDE_BIT 7

#define ABFV34_NOISE_STD_LENGTH 65
typedef uint32_t noise_std2_lut_t[ABFV34_NOISE_STD_LENGTH];

/* Registers for VFE 4.8 */
typedef struct ISP_ABF34_Cfg {
  /* Demosaic Config */
  uint32_t    filterEn                  :  1;
  uint32_t    lutBankSel                :  1;
  uint32_t    crossPlEn                 :  1;
  uint32_t    singleBpcEn               :  1;
  uint32_t    pixelMatchLvGrGb          :  3;
  uint32_t    /* reserved */            :  1;
  uint32_t    pixelMatchLvRb            :  3;
  uint32_t    /* reserved */            :  1;
  uint32_t    distGrGb_0                :  3;
  uint32_t    /* reserved */            :  1;
  uint32_t    distGrGb_1                :  2;
  uint32_t    distGrGb_2                :  2;
  uint32_t    distRb_0                  :  3;
  uint32_t    /* reserved */            :  1;
  uint32_t    distRb_1                  :  2;
  uint32_t    distRb_2                  :  2;
  uint32_t    /* reserved */            :  4;
}__attribute__((packed, aligned(4))) ISP_ABF34_Cfg;

typedef struct ISP_ABF34_RnrCfg {
  /*   CFG_0  */
  uint32_t    bx                         : 14;
  uint32_t    /* reserved */            :  2;
  uint32_t    by                         : 14;
  uint32_t    /* reserved */            :  2;
  /*   CFG_1  */
  uint32_t    initSquare                : 28;
  uint32_t    /* reserved */            :  4;
  /*   CFG_2  */
  uint32_t    anchor_0                  : 12;
  uint32_t    /* reserved */            :  4;
  uint32_t    anchor_1                  : 12;
  uint32_t    /* reserved */            :  4;
  /*   CFG_3  */
  uint32_t    anchor_2                  : 12;
  uint32_t    /* reserved */            :  4;
  uint32_t    anchor_3                  : 12;
  uint32_t    /* reserved */            :  4;
  /*   CFG_4  */
  uint32_t    coeffBase_0               :  8;
  uint32_t    coeffSlope_0              :  8;
  uint32_t    coeffShift_0              :  4;
  uint32_t    /* reserved */            : 12;
  /*   CFG_5  */
  uint32_t    coeffBase_1               :  8;
  uint32_t    coeffSlope_1              :  8;
  uint32_t    coeffShift_1              :  4;
  uint32_t    /* reserved */            : 12;
  /*   CFG_6  */
  uint32_t    coeffBase_2               :  8;
  uint32_t    coeffSlope_2              :  8;
  uint32_t    coeffShift_2              :  4;
  uint32_t    /* reserved */            : 12;
  /*   CFG_7  */
  uint32_t    coeffBase_3               :  8;
  uint32_t    coeffSlope_3              :  8;
  uint32_t    coeffShift_3              :  4;
  uint32_t    /* reserved */            : 12;
  /*   CFG_8  */
  uint32_t    threshBase_0              :  8;
  uint32_t    threshSlope_0             :  8;
  uint32_t    threshShift_0             :  4;
  uint32_t    /* reserved */            : 12;
  /*   CFG_9  */
  uint32_t    threshBase_1              :  8;
  uint32_t    threshSlope_1             :  8;
  uint32_t    threshShift_1             :  4;
  uint32_t    /* reserved */            : 12;
  /*   CFG_10  */
  uint32_t    threshBase_2              :  8;
  uint32_t    threshSlope_2             :  8;
  uint32_t    threshShift_2             :  4;
  uint32_t    /* reserved */            : 12;
  /*   CFG_11  */
  uint32_t    threshBase_3              :  8;
  uint32_t    threshSlope_3             :  8;
  uint32_t    threshShift_3             :  4;
  uint32_t    /* reserved */            : 12;
  /*   CFG_12  */
  uint32_t    rsquareShift              :  4;
  uint32_t    /* reserved */            : 28;
}__attribute__((packed, aligned(4))) ISP_ABF34_RnrCfg;

typedef struct ISP_ABF34_ChannelCfg {
  /*   CFG_0  */
  uint32_t    curveOffset               :  7;
  uint32_t    /* reserved */            : 25;
}__attribute__((packed, aligned(4))) ISP_ABF34_ChannelCfg;

/* Bpc CFG */
typedef struct ISP_ABF34_BpcCfg {
  /* CFG_0 */
  uint32_t    fmax                      :  6;
  uint32_t    /* reserved */            :  2;
  uint32_t    fmin                      :  6;
  uint32_t    /* reserved */            :  2;
  uint32_t    offset                    : 12;
  uint32_t    /* reserved */            :  4;
  /* CFG_1 */
  uint32_t    minShift                  :  4;
  uint32_t    maxShift                  :  4;
  uint32_t    bls                       : 12;
  uint32_t    /* reserved */            : 12;
  }__attribute__((packed, aligned(4))) ISP_ABF34_BpcCfg;

 /* Bpc CFG */
typedef struct ISP_ABF34_NoisePrsvCfg {
  /* CFG_0 */
  uint32_t    anchor_lo                 :  10;
  uint32_t    /* reserved */            :  6;
  uint32_t    anchor_gap                :  10;
  uint32_t    /* reserved */            :  6;
  /* CFG_1 */
  uint32_t    lo_grgb                   :  9;
  uint32_t    /* reserved */            :  3;
  uint32_t    slope_grgb                :  10;
  uint32_t    /* reserved */            :  2;
  uint32_t    shift_grgb                :  4;
  uint32_t    /* reserved */            :  4;
  /* CFG_2 */
  uint32_t    lo_rb                     :  9;
  uint32_t    /* reserved */            :  3;
  uint32_t    slope_rb                  :  10;
  uint32_t    /* reserved */            :  2;
  uint32_t    shift_rb                  :  4;
  uint32_t    /* reserved */            :  4;
  }__attribute__((packed, aligned(4))) ISP_ABF34_NoisePrsvCfg;

typedef struct ISP_ABF34_Padding {
  /*   CFG_0  */
  uint32_t    /* reserved */            : 32;
}__attribute__((packed, aligned(4))) ISP_ABF34_Padding;


typedef struct ISP_ABF34_cfg_cmd_t {
  /** These are in order of HW registers. Do not add variables or change order
    * That will affect hw write
   **/
  ISP_ABF34_Cfg        cfg;

}__attribute__((packed, aligned(4))) ISP_ABF34_cfg_cmd_t;

typedef struct ISP_ABF34_cfg_2_cmd_t {
  /** These are in order of HW registers. Do not add variables or change order
    * That will affect hw write
   **/
  ISP_ABF34_ChannelCfg grCfg;
  ISP_ABF34_ChannelCfg gbCfg;
  ISP_ABF34_ChannelCfg rCfg;
  ISP_ABF34_ChannelCfg bCfg;
  ISP_ABF34_RnrCfg     rnrCfg;
  ISP_ABF34_BpcCfg     bpcCfg;
  ISP_ABF34_NoisePrsvCfg noisePrsvCfg;
}__attribute__((packed, aligned(4))) ISP_ABF34_cfg_2_cmd_t;


typedef struct ISP_ABF34_CmdType {
  /** These are in order of HW registers. Do not add variables or change order
    * That will affect hw write
   **/
  ISP_ABF34_cfg_cmd_t      cfg_cmd;
  ISP_ABF34_cfg_2_cmd_t    cfg_cmd2;

}__attribute__((packed, aligned(4))) ISP_ABF34_CmdType;




#endif /* __ABF44_REG_H__ */
