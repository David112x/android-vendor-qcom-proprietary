/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Copyright (c) 2017, The Linux Foundation. All rights reserved.
 *
 */

#include "pa_qti_effect.h"

static const pa_qti_effect_uuid_t SL_IID_AUDIOSPHERE_ = { 0x2f03ade8, 0xd92b, 0x4172, 0x9eea,
        { 0x52, 0x0c, 0xde, 0xfa, 0x3c, 0x1d } };
static const pa_qti_effect_uuid_t * const SL_IID_AUDIOSPHERE = &SL_IID_AUDIOSPHERE_;

static const pa_qti_effect_uuid_t SL_IID_AUDIOSPHERE_UUID = { 0x184e62ab, 0x2d19, 0x4364, 0x9d1b,
        { 0xc0, 0xa4, 0x07, 0x33, 0x86, 0x6c } };

/* enumerated parameter settings for BassBoost effect */
typedef enum {
    ASPHERE_PARAM_ENABLE,
    ASPHERE_PARAM_STRENGTH,
    ASPHERE_PARAM_STATUS,
} pa_qti_asphere_params;
