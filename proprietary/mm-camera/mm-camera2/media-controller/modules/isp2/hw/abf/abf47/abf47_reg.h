/*============================================================================

  Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __ABF47_REG_H__
#define __ABF47_REG_H__

/* changed from 440
 * In the VFE 4.0 ABF registers were part of the demosaic register definations.
 * In VFE 4.4 the ABF has its own register i.e. and could either use the ABF 2.0
 * i.e the one used in the VFE 4.0 or use ABF 3.0 used in the VFE 4.4.
 */

#define ISP_ABF3_OFF 0x000005E8
#define ISP_ABF3_LEN 52

#define WTABLE_ADJ_VAL 4

#define SHORT_NOISE_SCALE 4.0f
#define NOISE_OFFSET_MAX_VAL (1<<16) - 1
#define DENOISE_STRENGTH_MAX_VAL 511
#define NOISE_OFFSET_SCALE 4
#define MAX_FPN_NOISE_SCALE_LEVEL 65535
#define ABF_CELL_BITWIDTH 11
#define ABF_INVCELL_BITWIDTH 12
#define ABF_INVCELL_BITSHIFT 16
#define ABF_HORI_GRID_NUM 12
#define ABF_VERT_GRID_NUM 9

#define CROSSP1LEVEL0EN  4
#define CROSSP1LEVEL1EN  5
#define MINMAXEN         6

#define ABF_CGC_OVERRIDE TRUE
#define ABF_CGC_OVERRIDE_REGISTER 0x2C
#define ABF_CGC_OVERRIDE_BIT 7

/* Registers for VFE 4.7 */
typedef struct ISP_ABF3_Cfg {
  /* Demosaic Config */
  uint32_t    /* reserved */             :  2;
  uint32_t    lutBankSel                 :  1;
  uint32_t    crossPlLevel0En            :  1;
  uint32_t    crossPlLevel1En            :  1;
  uint32_t    minMaxEn                   :  1;
  uint32_t    /*reserved*/               :  26;
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
  uint32_t    noiseScale0Lv0            : 12;
  uint32_t    wTableAdjLv0              :  4;
  uint32_t    noiseScale0Lv1            : 12;
  uint32_t    wTableAdjLv1              :  4;
  /*   CFG_1  */
  uint32_t    noiseScale1Lv0            : 16;
  uint32_t    noiseScale1Lv1            : 16;
  /*   CFG_2  */
  uint32_t    noiseScale2Lv0            : 12;
  uint32_t    /* reserved */            :  4;
  uint32_t    noiseScale2Lv1            : 12;
  uint32_t    /* reserved */            :  4;
  /*   CFG_3  */
  uint32_t    noiseOffLv0               : 16;
  uint32_t    noiseOffLv1               : 16;
  /*   CFG_4  */
  uint32_t    curvOffLv0                :  7;
  uint32_t    /* reserved */            :  9;
  uint32_t    curvOffLv1                :  7;
  uint32_t    /* reserved */            :  9;
  /*   CFG_5  */
  uint32_t    sftThrdNoiseScaleLv0      : 12;
  uint32_t    sftThrdNoiseShiftLv0      :  4;
  uint32_t    sftThrdNoiseScaleLv1      : 12;
  uint32_t    sftThrdNoiseShiftLv1      :  4;
  /* CFG_6 */
  /* Updated to 9uQ9. Old HW document says 9uQ8 */
  uint32_t    filterNoisePreserveL_Lv0  : 9;
  uint32_t    /* reserved */            : 7;
  uint32_t    filterNoisePreserveR_Lv0  : 9;
  uint32_t    /* reserved */            : 7;
  /* CFG_7 */
  uint32_t    filterNoisePreserveL_Lv1  : 9;
  uint32_t    /* reserved */            : 7;
  uint32_t    filterNoisePreserveR_Lv1  : 9;
  uint32_t    /* reserved */            : 7;
  /* CFG_8 */
  uint32_t    spatialScaleL_Lv0         : 9;
  uint32_t    /* reserved */            : 7;
  uint32_t    spatialScaleR_Lv0         : 9;
  uint32_t    /* reserved */            : 7;
  /* CFG_9 */
  uint32_t    spatialScaleL_Lv1         : 9;
  uint32_t    /* reserved */            : 7;
  uint32_t    spatialScaleR_Lv1         : 9;
  uint32_t    /* reserved */            : 7;
}__attribute__((packed, aligned(4))) ISP_ABF3_ChannelCfg;

/* Spatial CFG */
typedef struct ISP_ABF3_SpatialCfg {
  /* CFG_0 */
  uint32_t    init_CellNum_X_L             : 4;
  uint32_t    init_CellNum_X_R             : 4;
  uint32_t    init_CellNum_Y_L             : 4;
  uint32_t    init_CellNum_Y_R             : 4;
  uint32_t    /* reserved */               : 16;

  /* CFG_1 */
  uint32_t    init_DX_L                    : 11;
  uint32_t    /* reserved */               : 5;
  uint32_t    init_DX_R                    : 11;
  uint32_t    /* reserved */               : 5;

  /* CFG_2 */
  uint32_t    init_DY_L                    : 11;
  uint32_t    /* reserved */               : 5;
  uint32_t    init_DY_R                    : 11;
  uint32_t    /* reserved */               : 5;

  /* CFG_3 */
  uint32_t    init_PX_L                    : 16;
  uint32_t    init_PX_R                    : 16;

  /* CFG_4 */
  uint32_t    init_PY_L                    : 16;
  uint32_t    init_PY_R                    : 16;

  /* CFG_5 */
  uint32_t    inv_CellWidth_L             : 12;
  uint32_t    /* reserved */              : 4;
  uint32_t    inv_CellWidth_R             : 12;
  uint32_t    /* reserved */              : 4;

  /* CFG_6 */
  uint32_t    inv_CellHeight_L            : 12;
  uint32_t    /* reserved */              : 4;
  uint32_t    inv_CellHeight_R            : 12;
  uint32_t    /* reserved */              : 4;

  /* CFG_7 */
  uint32_t    cellWidth_L                  : 11;
  uint32_t    /* reserved */               : 5;
  uint32_t    cellWidth_R                  : 11;
  uint32_t    /* reserved */               : 5;

  /* CFG_8 */
  uint32_t    cellHeight_L                 : 11;
  uint32_t    /* reserved */               : 5;
  uint32_t    cellHeight_R                 : 11;
  uint32_t    /* reserved */               : 5;
}__attribute__((packed, aligned(4))) ISP_ABF3_SpatialCfg;

typedef struct ISP_ABF3_CmdType {
  /** These are in order of HW registers. Do not add variables or change order
    * That will affect hw write
   **/
  ISP_ABF3_Cfg        cfg;
  ISP_ABF3_Lv1Cfg     level1;
  ISP_ABF3_ChannelCfg grCfg;
  ISP_ABF3_ChannelCfg gbCfg;
  ISP_ABF3_ChannelCfg rCfg;
  ISP_ABF3_ChannelCfg bCfg;
  ISP_ABF3_SpatialCfg spCfg;
}__attribute__((packed, aligned(4))) ISP_ABF3_CmdType;

#endif /* __ABF44_REG_H__ */
