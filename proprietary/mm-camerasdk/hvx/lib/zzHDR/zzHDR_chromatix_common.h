/*===========================================================================

  Copyright (c) 2015 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

============================================================================*/

#ifndef CHROMATIX_HDR_EXTN_COMMON_H
#define CHROMATIX_HDR_EXTN_COMMON_H


#define MAX_SETS_FOR_TONE_NOISE_ADJ 6
#define MAX_SETS_FOR_GTM_EXP_ADJ    3
/// Tuning Control Flag.
typedef unsigned char tuning_control_type;

typedef struct
{
    float start;    // trigger point Start
    float end;      // trigger point End
} trigger_point2_type;

/// This type holds both luma index and gain based trigger points to be
/// used in configuration of VFE blocks.
typedef struct
{
    float gain_start;      ///< Gain based trigger point.
    float gain_end;        ///< Gain based end point.
    long lux_index_start; ///< Lux index based trigger point.
    long lux_index_end;   ///< Lux index based end point.
} trigger_point_linear_type;


#endif
