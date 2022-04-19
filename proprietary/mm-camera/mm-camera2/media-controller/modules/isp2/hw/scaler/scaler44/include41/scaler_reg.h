/*============================================================================

  Copyright (c) 2013-2015 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __SCALER44_REG_H__
#define __SCALER44_REG_H__

#define ISP_SCALER44_ENC_OFF_0 0x0000075C
#define ISP_SCALER44_ENC_LEN_0 18
#define ISP_SCALER44_ENC_OFF_1 0x00000978
#define ISP_SCALER44_ENC_LEN_1 2

#define ISP_SCALER44_VIEW_OFF_0 0x000007A4
#define ISP_SCALER44_VIEW_LEN_0 18
#define ISP_SCALER44_VIEW_OFF_1 0x00000980
#define ISP_SCALER44_VIEW_LEN_1 2

#define ISP_SCALER44_MAX_VIEW_H_OUT    2112
#define ISP_SCALER44_MAX_SCALER_FACTOR 15

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

/* Scaler Config Cmd Type*/
typedef struct ISP_ScaleCfgCmdType {
  ISP_Y_ScaleCfgCmdType     Y_ScaleCfg;
  ISP_CbCr_ScaleCfgCmdType  CbCr_ScaleCfg;
  ISP_Y_ScalePadCfgCmdType  Y_ScalePadCfg;
}__attribute__((packed, aligned(4))) ISP_ScaleCfgCmdType;

#endif /* __SCALER44_REG_H__ */
