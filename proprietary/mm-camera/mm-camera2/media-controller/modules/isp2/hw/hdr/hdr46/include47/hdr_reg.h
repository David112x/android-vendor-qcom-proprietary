/* hdr_reg.h
 *
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __HDR_REG_H__
#define __HDR_REG_H__

#define  ISP_HDR_OFF 0x57C
#define  ISP_HDR_LEN 5

#define  ISP_HDR_RECON_OFF 0x590
#define  ISP_HDR_RECON_LEN 4

#define  ISP_HDR_MAC_OFF 0x5A4
#define  ISP_HDR_MAC_LEN 8

/* limitation from HLD */
#define ISP_HDR_WB_RATIO_MAX 16383
#define ISP_HDR_WB_RATIO_MIN 1024

#define ISP_HDR_EXP_RATIO_MAX 16384
#define ISP_HDR_EXP_RATIO_MIN 1024

#define ISP_HDR_EXP_RECIP_RATIO_MAX 256
#define ISP_HDR_EXP_RECIP_RATIO_MIN 16

#define ISP_HDR_MAC_SQRT_ANALOG_GAIN_MAX 90
#define ISP_HDR_MAC_SQRT_ANALOG_GAIN_MIN 16

/* HDR Config */
typedef struct ISP_HdrCfgCmdType {
  /* HDR  0 Config */
  uint32_t     ReconFirstFiled           :1;
  uint32_t     /* reserved */            :3;
  uint32_t     ExpRatio                  :15;
  uint32_t     /* reserved */            :13;

  /* HDR  1 Config */
  uint32_t     RgWbGainRatio             :14;
  uint32_t     /* reserved */            :2;
  uint32_t     BgWbGainRatio             :14;
  uint32_t     /* reserved */            :2;

  /* HDR  2 Config */
  uint32_t      GrWbGainRatio            :14;
  uint32_t     /* reserved */            :2;
  uint32_t      GbWbGainRatio            :14;
  uint32_t     /* reserved */            :2;

  /* HDR  3 Config */
  uint32_t      R_RgWbGainRatio          :14;
  uint32_t     /* reserved */            :2;
  uint32_t      R_BgWbGainRatio          :14;
  uint32_t     /* reserved */            :2;

  /* HDR  4 Config */
  uint32_t      R_GrWbGainRatio          :14;
  uint32_t     /* reserved */            :2;
  uint32_t      R_GbWbGainRatio          :14;
  uint32_t     /* reserved */            :2;
}__attribute__((packed, aligned(4))) ISP_HdrCfgCmdType;

/* HDR RECON Config */
typedef struct ISP_HdrReconCfgCmdType {
  /* HDR Recon 0 Config */
  uint32_t     ReconHedgeTH1               :10;
  uint32_t     /* reserved */              :2;
  uint32_t     ReconHedgeDthLog2           :4;
  uint32_t     ReconMotionTH1              :10;
  uint32_t     /* reserved */              :2;
  uint32_t     ReconMotionDthLog2          :4;

  /* HDR Recon 1 Config */
  uint32_t      ReconDarkTH1               :10;
  uint32_t     /* reserved */              :2;
  uint32_t      ReconDarkDthLog2           :3;
  uint32_t     /* reserved */              :1;
  uint32_t      ReconFlatRegionTH          :10;
  uint32_t     /* reserved */              :2;
  uint32_t      ReconEdgeLpfTap0           :3;
  uint32_t     /* reserved */              :1;

  /* HDR Recon 2 Config */
  uint32_t     R_ReconHedgeTH1             :10;
  uint32_t     /* reserved */              :2;
  uint32_t     R_ReconHedgeDthLog2         :4;
  uint32_t     R_ReconMotionTH1            :10;
  uint32_t     /* reserved */              :2;
  uint32_t     R_ReconMotionDthLog2        :4;

  /* HDR Recon 3 Config */
  uint32_t     R_ReconDarkTH1              :10;
  uint32_t     /* reserved */              :2;
  uint32_t     R_ReconDarkDthLog2          :3;
  uint32_t     /* reserved */              :1;
  uint32_t     ReconMinFactor              :5;
  uint32_t     /* reserved */              :3;
  uint32_t     R_ReconMinFactor            :5;
  uint32_t     /* reserved */              :1;
  uint32_t     ReconLinearMode             :1;
  uint32_t     /* reserved */              :1;
}__attribute__((packed, aligned(4))) ISP_HdrReconCfgCmdType;

/* HDR MAC Config */
typedef struct ISP_HdrMacCfgCmdType {
  /* HDR MAC 0 Config */
  uint32_t     MacMotion0TH1               :10;
  uint32_t     /* reserved */              :2;
  uint32_t     R_MacMotion0TH1             :10;
  uint32_t     /* reserved */              :2;
  uint32_t     MacMotion0TH2               :8;

  /* HDR MAC 1 Config */
  uint32_t     R_MacMotion0TH2             :8;
  uint32_t     MacSqrtAnalogGain           :7;
  uint32_t     /* reserved */              :1;
  uint32_t     R_MacSqrtAnalogGain         :7;
  uint32_t     /* reserved */              :1;
  uint32_t      MacDilation                :3;
  uint32_t     /* reserved */              :5;

  /* HDR MAC 2 Config */
  uint32_t     MacMotion0Dt0               :6;
  uint32_t     /* reserved */              :2;
  uint32_t     R_MacMotion0Dt0             :6;
  uint32_t     /* reserved */              :2;
  uint32_t     MacMotionStength            :5;
  uint32_t     /* reserved */              :3;
  uint32_t     R_MacMotionStength          :5;
  uint32_t     /* reserved */              :3;

  /* HDR MAC 3 Config */
  uint32_t     MacLowLightTH1              :14;
  uint32_t     /* reserved */              :2;
  uint32_t     R_MacLowLightTH1              :14;
  uint32_t     /* reserved */              :2;

  /* HDR MAC 4 Config */
  uint32_t     MacLowLightDthLog2          :4;
  uint32_t     R_MacLowLightDthLog         :4;
  uint32_t     MacLowLightStrength_H       :5;
  uint32_t     /* reserved */              :3;
  uint32_t     R_MacLowLightStrength_H      :5;
  uint32_t     /* reserved */              :3;
  uint32_t     MacHighLightDthLog2         :4;
  uint32_t     R_MacHighLightDthLog        :4;

  /* HDR MAC 5 Config */
  uint32_t     MacHighLightTH1             :14;
  uint32_t     /* reserved */              :2;
  uint32_t     R_MacHighLightTH1           :14;
  uint32_t     /* reserved */              :2;

  /* HDR MAC 6 Config */
  uint32_t     MacSmoothTH1                :9;
  uint32_t     /* reserved */              :3;
  uint32_t     R_MacSmoothTH1              :9;
  uint32_t     /* reserved */              :1;
  uint32_t     MacSmoothEnable             :1;
  uint32_t     /* reserved */              :1;
  uint32_t     MacSmoothDthLog2            :4;
  uint32_t     R_MacSmoothDthLog2          :4;

  /* HDR MAC 7 Config */
  uint32_t     MacSmoothTap0               :3;
  uint32_t     /* reserved */              :1;
  uint32_t     R_MacSmoothTap0             :3;
  uint32_t     /* reserved */              :1;
  uint32_t     ExpRatioRecip               :9;
  uint32_t     MsbAligned                  :1;
  uint32_t     MacLinearMode               :1;
  uint32_t     /* reserved */              :13;
}__attribute__((packed, aligned(4))) ISP_HdrMacCfgCmdType;

#endif /* __HDR_REG_H__ */
