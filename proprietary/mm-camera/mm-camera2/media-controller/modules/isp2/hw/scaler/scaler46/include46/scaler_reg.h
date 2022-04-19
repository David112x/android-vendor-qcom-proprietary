/*============================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __SCALER_REG_H__
#define __SCALER_REG_H__

#define ISP_SCALER46_ENC_OFF 0x00000824
#define ISP_SCALER46_ENC_LEN 22

#define ISP_SCALER46_VIEW_OFF 0x000008B4
#define ISP_SCALER46_VIEW_LEN 22

#define ISP_SCALER46_VIDEO_OFF 0x00000944
#define ISP_SCALER46_VIDEO_LEN 22

#define ISP_SCALER46_HW_SCALER_LIMIT 68
/* In worst case Chroma needs to be downscaled twice as much as luma */
#define ISP_SCALER46_MAX_SCALER_FACTOR ((ISP_SCALER46_HW_SCALER_LIMIT - 1) / 2)

#define INTERP_SHIFT     14

/* Y Scaler Config Cmd*/
typedef struct ISP_Y_ScaleCfgCmdType {
  /* Y Scale Config */
  uint32_t     hEnable                        : 1;
  uint32_t     vEnable                        : 1;
  uint32_t     /* reserved */                 :30;

  /* Y Scale H Image Size Config */
  uint32_t     hIn                            :14;
  uint32_t     /* reserved */                 : 2;
  uint32_t     hOut                           :14;
  uint32_t     /* reserved */                 : 2;
  /* Y Scale H Phase Config */
  uint32_t     horizPhaseMult                 :21;
  uint32_t     /* reserved */                 : 7;
  uint32_t     horizInterResolution           : 2;
  uint32_t     /* reserved */                 : 2;
  /* Y Scale H Stripe Config_0 */
  uint32_t     horizMNInit                    :14;
  uint32_t     /* reserved */                 :18;
  /* Y Scale H Stripe Config_1 */
  uint32_t     horizPhaseInit                 :21;
  uint32_t     /* reserved */                 :11;

  /* Y Scale H Pad Config */
  uint32_t     scaleYInWidth                  :14;
  uint32_t     /* reserved */                 : 2;
  uint32_t     hSkipCount                     :14;
  uint32_t     /* reserved */                 : 1;
  uint32_t     rightPadEnable                 : 1;

  /* Y Scale V Image Size Config */
  uint32_t     vIn                            :14;
  uint32_t     /* reserved */                 : 2;
  uint32_t     vOut                           :14;
  uint32_t     /* reserved */                 : 2;
  /* Y Scale V Phase Config */
  uint32_t     vertPhaseMult                  :21;
  uint32_t     /* reserved */                 : 7;
  uint32_t     vertInterResolution            : 2;
  uint32_t     /* reserved */                 : 2;
  /* Y Scale V Stripe Config_0 */
  uint32_t     vertMNInit                     :14;
  uint32_t     /* reserved */                 :18;
  /* Y Scale V Stripe Config_1 */
  uint32_t     vertPhaseInit                  :21;
  uint32_t     /* reserved */                 :11;

  /* Y Scale V Pad Config */
  uint32_t     scaleYInHeight                 :14;
  uint32_t     /* reserved */                 : 2;
  uint32_t     vSkipCount                     :14;
  uint32_t     /* reserved */                 : 1;
  uint32_t     bottomPadEnable                : 1;
}__attribute__((packed, aligned(4))) ISP_Y_ScaleCfgCmdType;


/* CbCr Scaler Config Cmd*/
typedef struct ISP_CbCr_ScaleCfgCmdType {
  /* CbCr config*/
  uint32_t     hEnable                        : 1;
  uint32_t     vEnable                        : 1;
  uint32_t     /* reserved */                 :30;

  /* CbCr H Image Size config */
  uint32_t     hIn                            :16;
  uint32_t     hOut                           :16;
  /* CbCr H Phase config */
  uint32_t     horizPhaseMult                 :21;
  uint32_t     /* reserved */                 : 7;
  uint32_t     horizInterResolution           : 2;
  uint32_t     /* reserved */                 : 2;
  /* CbCr H Stripe config 0*/
  uint32_t     horizMNInit                    :16;
  uint32_t     /* reserved */                 :16;
  /* CbCr H Stripe config 1*/
  uint32_t     horizPhaseInit                 :21;
  uint32_t     /* reserved */                 :11;
  /* CbCr H Pad config*/
  uint32_t     scaleCbCrInWidth               :14;
  uint32_t     /* reserved */                 : 2;
  uint32_t     hSkipCount                     :14;
  uint32_t     /* reserved */                 : 1;
  uint32_t     rightPadEnable                 : 1;

  /* CbCr V Image Size config */
  uint32_t     vIn                            :16;
  uint32_t     vOut                           :16;
  /* CbCr V Phase config */
  uint32_t     vertPhaseMult                  :21;
  uint32_t     /* reserved */                 : 7;
  uint32_t     vertInterResolution            : 2;
  uint32_t     /* reserved */                 : 2;
  /* CbCr V Stripe config 0*/
  uint32_t     vertMNInit                     :16;
  uint32_t     /* reserved */                 :16;
  /* CbCr V Stripe config 1*/
  uint32_t     vertPhaseInit                  :21;
  uint32_t     /* reserved */                 :11;
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
}__attribute__((packed, aligned(4))) ISP_ScaleCfgCmdType;

#endif /* __SCALER_REG_H__ */
