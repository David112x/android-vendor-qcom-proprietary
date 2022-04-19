/*============================================================================

  Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __FOVCROP46_REG_H__
#define __FOVCROP46_REG_H__

#define ISP_FOV46_ENC_OFF 0x0000087C
#define ISP_FOV46_ENC_LEN 4

#define ISP_FOV46_VIEW_OFF 0x0000090C
#define ISP_FOV46_VIEW_LEN 4

#define ISP_FOV46_VIDEO_OFF 0x0000099C
#define ISP_FOV46_VIDEO_LEN 4

#define INTERP_SHIFT  14

/* Crop Config */
typedef struct ISP_FOV_CropConfig {
  uint32_t  lastPixel        : 13;
  uint32_t  /* reserved */   :  3;
  uint32_t  firstPixel       : 13;
  uint32_t  /* reserved */   :  3;

  uint32_t  lastLine         : 14;
  uint32_t  /* reserved */   :  2;
  uint32_t  firstLine        : 14;
  uint32_t  /* reserved */   :  2;
}__attribute__((packed, aligned(4))) ISP_FOV_CropConfig;

/* Field Of View (FOV) Crop Config Command */
typedef struct ISP_FOV_CropConfigCmdType {
  /* Y config */
  ISP_FOV_CropConfig  y_crop_cfg;
  /* CbCr Config */
  ISP_FOV_CropConfig  cbcr_crop_cfg;
}__attribute__((packed, aligned(4))) ISP_FOV_CropConfigCmdType;

#endif //__FOVCROP46_REG_H__
