/* isp_pipeline_reg.h
 *
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __ISP_PIPELINE_REG_H_
#define __ISP_PIPELINE_REG_H_

#define ISP_SCALER_ENC_EN_BIT 23
#define ISP_SCALER_ENC_OFF_0 0x0000075C
#define ISP_SCALER_ENC_LEN_0 18
#define ISP_SCALER_ENC_OFF_1 0x00000AB8
#define ISP_SCALER_ENC_LEN_1 2

#define ISP_SCALER_VIEW_EN_BIT 24
#define ISP_SCALER_VIEW_OFF_0 0x000007A4
#define ISP_SCALER_VIEW_LEN_0 18
#define ISP_SCALER_VIEW_OFF_1 0x00000AC0
#define ISP_SCALER_VIEW_LEN_1 2

#define ISP_SCALER_EN_OFF   0x18
#define ISP_SCALER_ENC_OFF  ISP_SCALER_ENC_OFF_0
#define ISP_SCALER_VIEW_OFF ISP_SCALER_VIEW_OFF_0
#define ISP_SCALER_HVINOUT_DIFF 0

#define ISP_CLAMP_ENC_OFF    0x00000874
#define ISP_CLAMP_ENC_LEN    2

#define ISP_CLAMP_VIEW_OFF   0x0000087C
#define ISP_CLAMP_VIEW_LEN   2

#define ISP_DEMUX_EN_BIT     2
#define ISP_DEMUX_EN_OFF     0x18
#define ISP_DEMUX_OFF        0x00000424
#define ISP_DEMUX_LEN        7

#define ISP_CC_EN_BIT  17
#define ISP_CC_EN_OFF  0x18
#define ISP_CC_OFF 0x00000640
#define ISP_CC_LEN 9

#define ISP_DEMOSAIC_EN_BIT  4
#define ISP_DEMOSAIC_EN_OFF  0x18
#define ISP_DEMOSAIC_CLASSIFIER_CNT 18
#define ISP_DEMOSAIC_CFG_OFF 0x00000440
#define ISP_DEMOSAIC_OFF     0x00000518

#define ISP_TESTGEN_OFF 0x934

#define ISP_REG_UPDATE_OFF 0x378

#define ISP_BG_STATS_OFF 0x00000898
#define ISP_STATS_BG_BUF_SIZE   ((72 * 54 * 7) * 8)

/* Y Scaler Config Cmd*/
typedef struct ISP_Y_ScaleCfgCmdType {
  /* Y Scale Config */
  uint32_t     hEnable                        : 1;
  uint32_t     vEnable                        : 1;
  uint32_t     /* reserved */                 :30;

  /* Y Scale H Image Size Config */
  uint32_t     hIn                            :13;
  uint32_t     /* reserved */                 : 3;
  uint32_t     hOut                           :13;
  uint32_t     /* reserved */                 : 3;
  /* Y Scale H Phase Config */
  uint32_t     horizPhaseMult                 :19;
  uint32_t     /* reserved */                 : 1;
  uint32_t     horizInterResolution           : 2;
  uint32_t     /* reserved */                 :10;
  /* Y Scale H Stripe Config */
  uint32_t     horizMNInit                    :13;
  uint32_t     /* reserved */                 : 1;
  uint32_t     horizPhaseInitMsb              : 2;
  uint32_t     horizPhaseInit                 :16;

  /* Y Scale V Image Size Config */
  uint32_t     vIn                            :14;
  uint32_t     /* reserved */                 : 2;
  uint32_t     vOut                           :14;
  uint32_t     /* reserved */                 : 2;
  /* Y Scale V Phase Config */
  uint32_t     vertPhaseMult                  :19;
  uint32_t     /* reserved */                 : 1;
  uint32_t     vertInterResolution            : 2;
  uint32_t     /* reserved */                 :10;
  /* Y Scale V Stripe Config */
  uint32_t     vertMNInit                     :14;
  uint32_t     vertPhaseInitMsb               : 2;
  uint32_t     vertPhaseInit                  :16;
}__attribute__((packed, aligned(4))) ISP_Y_ScaleCfgCmdType;

/* Y Scaler Pad Config Cmd*/
typedef struct ISP_Y_ScalePadCfgCmdType {

  /* Y Scale H Pad Config */
  uint32_t     scaleYInWidth                  :13;
  uint32_t     /* reserved */                 : 3;
  uint32_t     hSkipCount                     :13;
  uint32_t     /* reserved */                 : 2;
  uint32_t     rightPadEnable                 : 1;

  /* Y Scale V Pad Config */
  uint32_t     scaleYInHeight                 :14;
  uint32_t     /* reserved */                 : 2;
  uint32_t     vSkipCount                     :14;
  uint32_t     /* reserved */                 : 1;
  uint32_t     bottomPadEnable                : 1;
} __attribute__((packed, aligned(4))) ISP_Y_ScalePadCfgCmdType;

/* CbCr Scaler Config Cmd*/
typedef struct ISP_CbCr_ScaleCfgCmdType {
  /* CbCr config*/
  uint32_t     hEnable                        : 1;
  uint32_t     vEnable                        : 1;
  uint32_t     /* reserved */                 :30;

  /* CbCr H Image Size config */
  uint32_t     hIn                            :15;
  uint32_t     /* reserved */                 : 1;
  uint32_t     hOut                           :14;
  uint32_t     /* reserved */                 : 2;
  /* CbCr H Phase config */
  uint32_t     horizPhaseMult                 :19;
  uint32_t     /* reserved */                 : 1;
  uint32_t     horizInterResolution           : 2;
  uint32_t     /* reserved */                 :10;
  /* CbCr H Stripe config 0*/
  uint32_t     horizMNInit                    :15;
  uint32_t     /* reserved */                 :17;
  /* CbCr H Stripe config 1*/
  uint32_t     horizPhaseInit                 :18;
  uint32_t     /* reserved */                 :14;
  /* CbCr H Pad config*/
  uint32_t     scaleCbCrInWidth               :13;
  uint32_t     /* reserved */                 : 3;
  uint32_t     hSkipCount                     :13;
  uint32_t     /* reserved */                 : 2;
  uint32_t     rightPadEnable                 : 1;

  /* CbCr V Image Size config */
  uint32_t     vIn                            :16;
  uint32_t     vOut                           :15;
  uint32_t     /* reserved */                 : 1;
  /* CbCr V Phase config */
  uint32_t     vertPhaseMult                  :20;
  uint32_t     vertInterResolution            : 2;
  uint32_t     /* reserved */                 :10;
  /* CbCr V Stripe config 0*/
  uint32_t     vertMNInit                     :16;
  uint32_t     /* reserved */                 :16;
  /* CbCr V Stripe config 1*/
  uint32_t     vertPhaseInit                  :19;
  uint32_t     /* reserved */                 :13;
  /* CbCr V Pad config*/
  uint32_t     scaleCbCrInHeight              :14;
  uint32_t     /* reserved */                 : 2;
  uint32_t     vSkipCount                     :14;
  uint32_t     /* reserved */                 : 1;
  uint32_t     bottomPadEnable                : 1;
}__attribute__((packed, aligned(4))) ISP_CbCr_ScaleCfgCmdType;

typedef struct {
  ISP_Y_ScaleCfgCmdType     y;
  ISP_CbCr_ScaleCfgCmdType  cbcr;
} ISP_ScaleCfgCmdType;

/* Output Clamp Config Command */
typedef struct ISP_OutputClampConfigCmdType {
  /* Output Clamp Maximums */
  uint32_t  yChanMax         :  8;
  uint32_t  cbChanMax        :  8;
  uint32_t  crChanMax        :  8;
  uint32_t  /* reserved */   :  8;
  /* Output Clamp Minimums */
  uint32_t  yChanMin         :  8;
  uint32_t  cbChanMin        :  8;
  uint32_t  crChanMin        :  8;
  uint32_t  /* reserved */   :  8;
}__attribute__((packed, aligned(4))) ISP_OutputClampConfigCmdType;

typedef struct ISP_DemuxConfigCmdType {
  /*  period  */
  uint32_t  period         : 3;
  uint32_t  /* reserved */ :29;
  /* Demux Gain 0 Config */
  uint32_t  ch0EvenGain    :12;
  uint32_t  /* reserved */ : 4;
  uint32_t  ch0OddGain     :12;
  uint32_t  /* reserved */ : 4;
  /* Demux Gain 1 Config */
  uint32_t  ch1Gain        :12;
  uint32_t  /* reserved */ : 4;
  uint32_t  ch2Gain        :12;
  uint32_t  /* reserved */ : 4;
  /* Demux Gain 0 Config */
  uint32_t  R_ch0EvenGain  :12;
  uint32_t  /* reserved */ : 4;
  uint32_t  R_ch0OddGain   :12;
  uint32_t  /* reserved */ : 4;
  /* Demux Gain 1 Config */
  uint32_t  R_ch1Gain      :12;
  uint32_t  /* reserved */ : 4;
  uint32_t  R_ch2Gain      :12;
  uint32_t  /* reserved */ : 4;
  /* Demux Gain 1 Config */
  uint32_t  evenCfg        :32;
  /* Demux Gain 1 Config */
  uint32_t  oddCfg         :32;
} __attribute__((packed, aligned(4))) ISP_DemuxConfigCmdType;

typedef struct ISP_Chroma_Enhance_CfgCmdType {
  /* Color Conversion (RGB to Y) Config */
  int32_t      RGBtoYConversionV0      :12;
  int32_t     /* reserved */           :20;

  /* Conversion Coefficient 1 */
  int32_t      RGBtoYConversionV1      :12;
  int32_t     /* reserved */           :20;

  /* Conversion Coefficient 2 */
  int32_t      RGBtoYConversionV2      :12;
  int32_t     /* reserved */           :20;

  /* Conversion Offset */
  uint32_t     RGBtoYConversionOffset  : 8;
  uint32_t     /* reserved */          :24;

  /* Chroma Enhance A Config */
  int32_t      ap                      :12;
  int32_t     /* reserved */           : 4;
  int32_t      am                      :12;
  int32_t     /* reserved */           : 4;

  /* Chroma Enhance B Config */
  int32_t      bp                      :12;
  int32_t     /* reserved */           : 4;
  int32_t      bm                      :12;
  int32_t     /* reserved */           : 4;

  /* Chroma Enhance C Config */
  int32_t      cp                      :12;
  int32_t     /* reserved */           : 4;
  int32_t      cm                      :12;
  int32_t     /* reserved */           : 4;

  /* Chroma Enhance D Config */
  int32_t      dp                      :12;
  int32_t     /* reserved */           : 4;
  int32_t      dm                      :12;
  int32_t     /* reserved */           : 4;

  /* Chroma Enhance K Config */
  int32_t      kcb                     :11;
  int32_t     /* reserved */           : 5;
  int32_t      kcr                     :11;
  int32_t     /* reserved */           : 5;
} __attribute__((packed, aligned(4))) ISP_Chroma_Enhance_CfgCmdType;

typedef struct ISP_DemosaicInterpClassifierType {
  uint32_t w_n                             : 10;
  uint32_t   /* reserved */                : 2;
  uint32_t t_n                             : 10;
  uint32_t   /* reserved */                : 2;
  uint32_t l_n                             : 5;
  uint32_t   /* reserved */                : 2;
  uint32_t b_n                             : 1;
} __attribute__((packed, aligned(4))) ISP_DemosaicInterpClassifierType;

typedef struct ISP_DemosaicCmdType {
  /* Interp WB Gain 0 */
  uint32_t rgWbGain                        : 9;
  uint32_t wgr1                            : 3;
  uint32_t wgr2                            : 3;
  uint32_t bgWbGain                        : 9;
  uint32_t lambdaG                         : 8;

  /* Interp WB Gain 1 */
  uint32_t grWbGain                        : 9;
  uint32_t   /* reserved */                : 6;
  uint32_t gbWbGain                        : 9;
  uint32_t lambdaRB                        : 8;

  /* Interp Classifier */
  ISP_DemosaicInterpClassifierType    interpClassifier[ISP_DEMOSAIC_CLASSIFIER_CNT];

  /* Interp G 0 */
  uint32_t bl                              : 8;
  uint32_t bu                              : 8;
  uint32_t   /* reserved */                : 4;
  uint32_t noiseLevelG                     :12;

  /* Interp G 1 */
  uint32_t dblu                            : 9;
  uint32_t   /* reserved */                : 3;
  uint32_t a                               : 6;
  uint32_t   /* reserved */                : 2;
  uint32_t noiseLevelRB                    :12;
} __attribute__((packed, aligned(4))) ISP_DemosaicCmdType;

typedef struct ISP_DemosaicCfgCmdType {
  /* Demosaic Config */
  uint32_t demoV4Enable                    : 1;

  /* directional interpolation disable */
  uint32_t dirGInterpDisable               : 1;
  uint32_t dirRBInterpDisable              : 1;
  uint32_t  /*reserve*/                    : 1;

   /* cosited rgb enable */
  uint32_t cositedRgbEnable                : 1;
  uint32_t   /* reserved */                : 3;

  /* dynamic clamp enable */
  uint32_t dynGClampEnable                 : 1;
  uint32_t dynRBClampEnable                : 1;

  /* center clamp exclusion bit */
  uint32_t dynGClampCDisable               : 1;
  uint32_t dynRBClampCDisable              : 1;
  uint32_t   /* reserved */                : 4;

  /* pipe flush count */
  uint32_t pipeFlushCount                  : 13;
  uint32_t   /* reserved */                : 1;
  uint32_t pipeFlushOvd                    : 1;
  uint32_t flushHaltOvd                    : 1;
} __attribute__((packed, aligned(4))) ISP_DemosaicCfgCmdType;

typedef struct ISP_TestGenCmdType {
  /* TESTGEN_CFG */
  uint32_t testGenSel                      : 1;
  uint32_t /* reserved */                  :31;
  /* SW_TESTGEN_CMD */
  uint32_t pixelData                       :12;
  uint32_t pixelDataEn                     : 1;
  uint32_t hSync                           : 1;
  uint32_t vSync                           : 1;
  uint32_t  /*reserve*/                    :17;
  /* HW_TESTGEN_CMD */
  uint32_t goCmd                           : 1;
  uint32_t stopCmd                         : 1;
  uint32_t  /*reserve*/                    :30;
  /* HW_TESTGEN_CFG */
  uint32_t numFrame                        :10;
  uint32_t  /*reserve*/                    : 2;
  uint32_t pixelDataSel                    : 1;
  uint32_t systematicDataSel               : 1;
  uint32_t  /*reserve*/                    : 2;
  uint32_t pixelDataSize                   : 2;
  uint32_t hSyncEdge                       : 1;
  uint32_t vSyncEdge                       : 1;
  uint32_t  /*reserve*/                    :12;
  /* HW_TESTGEN_IMAGE_CFG */
  uint32_t imageWidth                      :14;
  uint32_t   /* reserved */                : 2;
  uint32_t imageHeight                     :14;
  uint32_t   /* reserved */                : 2;
  /* HW_TESTGEN_SOF_OFFSET_CFG */
  uint32_t sofOffset                       :24;
  uint32_t   /* reserved */                : 8;
  /* HW_TESTGEN_EOF_NOFFSET_CFG */
  uint32_t eofNOffset                      :24;
  uint32_t   /* reserved */                : 8;
  /* HW_TESTGEN_SOL_OFFSET_CFG */
  uint32_t solOffset                       : 9;
  uint32_t   /* reserved */                :23;
  /* HW_TESTGEN_EOL_NOFFSET_CFG */
  uint32_t eolNOffset                      : 9;
  uint32_t   /* reserved */                :23;
  /* HW_TESTGEN_HBI_CFG */
  uint32_t hbi                             :14;
  uint32_t   /* reserved */                :18;
  /* HW_TESTGEN_VBL_CFG */
  uint32_t vbl                             :14;
  uint32_t   /* reserved */                : 2;
  uint32_t vblEn                           : 1;
  uint32_t   /* reserved */                :15;
  /* HW_TESTGEN_SOF_DUMMY_LINE_CFG */
  uint32_t sofDummy                        : 8;
  uint32_t   /* reserved */                :24;
  /* HW_TESTGEN_EOF_DUMMY_LINE_CFG */
  uint32_t eofDummy                        : 8;
  uint32_t   /* reserved */                :24;
  /* HW_TESTGEN_COLOR_BARS_CFG */
  uint32_t unicolorBarSel                  : 3;
  uint32_t   /* reserved */                : 1;
  uint32_t unicolorBarEn                   : 1;
  uint32_t splitEn                         : 1;
  uint32_t pixelPattern                    : 2;
  uint32_t rotatePeriod                    : 6;
  uint32_t   /* reserved */                :18;
  /* HW_TESTGEN_RANDOM_CFG */
  uint32_t randSeed                        :16;
  uint32_t   /* reserved */                :16;

} __attribute__((packed, aligned(4))) ISP_TestGenCmdType;

typedef struct ISP_StatsBg_CfgCmdType {
  /*  VFE_STATS_BG_RGN_OFFSET_CFG   */
  uint32_t        rgnHOffset            :   13;
  uint32_t      /* reserved */          :    3;
  uint32_t        rgnVOffset            :   12;
  uint32_t       /*reserved */          :    4;
  /*  VFE_STATS_BG_RGN_SIZE_CFG */
  uint32_t        rgnWidth              :    9;
  uint32_t      /* reserved */          :    1;
  uint32_t        rgnHeight             :    8;
  uint32_t        rgnHNum               :    7;
  uint32_t        rgnVNum               :    6;
  uint32_t      /* reserved */          :    1;
  /* VFE_STATS_BG_THRESHOLD_CFG */
  uint32_t        rMax                  :    8;
  uint32_t        grMax                 :    8;
  uint32_t        bMax                  :    8;
  uint32_t        gbMax                 :    8;
} __attribute__((packed, aligned(4))) ISP_StatsBg_CfgCmdType;

#endif /* __ISP_PIPELINE_REG_H_ */
