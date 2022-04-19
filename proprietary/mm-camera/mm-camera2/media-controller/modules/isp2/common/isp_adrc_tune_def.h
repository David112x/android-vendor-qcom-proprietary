/*============================================================================

 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.

============================================================================*/
#ifndef __CHROMATIX_ADRC_H__
#define __CHROMATIX_ADRC_H__

#include "chromatix.h"

#define ISP_ADRC_LUT_LENGTH  64
#define ISP_ADRC_GAIN_TABLE_LENGTH 47
#define ISP_ADRC_GAIN_LEVELS 3
#define UNIT_GAIN_STEP 1.03

#define ISP_ADRC_GAIN_LUT_KNEE_LENGTH          7

/* KNEE Gain LUT for LA/LTM/GTM gain */
typedef struct {
  float         knee_gain[ISP_ADRC_GAIN_LUT_KNEE_LENGTH];
  float         knee_index[ISP_ADRC_GAIN_LUT_KNEE_LENGTH]; /* LUT for LA/LTM/GTM*/
}isp_adrc_knee_gain_lut;

typedef struct {
  int knee_points;
  isp_adrc_knee_gain_lut adrc_knee_gain_lut[ISP_ADRC_GAIN_TABLE_LENGTH];
} isp_adrc_knee_lut_type;
#endif
