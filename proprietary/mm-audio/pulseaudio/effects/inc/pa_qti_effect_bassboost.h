/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Not a Contribution.
 * Apache license notifications and license are retained
 * for attribution purposes only.
 *
 * Copyright (c) 2017, The Linux Foundation. All rights reserved.
 * Not a Contribution.
 *
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "pa_qti_effect.h"

static const pa_qti_effect_uuid_t SL_IID_BASSBOOST_ = { 0x0634f220, 0xddd4, 0x11db, 0xa0fc,
        { 0x00, 0x02, 0xa5, 0xd5, 0xc5, 0x1b } };
static const pa_qti_effect_uuid_t * const SL_IID_BASSBOOST = &SL_IID_BASSBOOST_;

static const pa_qti_effect_uuid_t SL_IID_BASSBOOST_UUID = { 0x2c4a8c24, 0x1581, 0x487f, 0x94f6,
        { 0x00, 0x02, 0xa5, 0xd5, 0xc5, 0x1b } };

/* enumerated parameter settings for BassBoost effect */
typedef enum {
    BASSBOOST_PARAM_STRENGTH_SUPPORTED,
    BASSBOOST_PARAM_STRENGTH
} pa_qti_bassboost_params;
