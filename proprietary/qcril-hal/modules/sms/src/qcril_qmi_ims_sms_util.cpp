/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#
#  Not a Contribution.
#  Apache license notifications and license are retained
#  for attribution purposes only.
#
#  Copyright (C) 2008 The Android Open Source Project
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
#******************************************************************************/

#include "utilities/utils_common.h"
#include "qcril_qmi_ims_sms_util.h"
#include "modules/nas/qcril_qmi_nas.h"
#include "framework/Log.h"

#define TAG                 "SMS_UTILS"

// only for debug purpose
void dumpRILCdmaFormat(RIL_CDMA_SMS_Message& cdmaMsg)
{
    QCRIL_LOG_INFO("uTeleserviceID: %d", cdmaMsg.uTeleserviceID);
    QCRIL_LOG_INFO("bIsServicePresent: %d", cdmaMsg.bIsServicePresent);
    QCRIL_LOG_INFO("uServicecategory: %d", cdmaMsg.uServicecategory);

    QCRIL_LOG_INFO("address digit_mode: %d", cdmaMsg.sAddress.digit_mode);
    QCRIL_LOG_INFO("address number_mode: %d", cdmaMsg.sAddress.number_mode);
    QCRIL_LOG_INFO("address number_type: %d", cdmaMsg.sAddress.number_type);
    QCRIL_LOG_INFO("address number_plan: %d", cdmaMsg.sAddress.number_plan);
    QCRIL_LOG_INFO("address number_of_digits: %d", cdmaMsg.sAddress.number_of_digits);

    QCRIL_LOG_INFO("subaddress type: %d", cdmaMsg.sSubAddress.subaddressType);
    QCRIL_LOG_INFO("subaddress odd: %d", cdmaMsg.sSubAddress.odd);
    QCRIL_LOG_INFO("subaddress number_of_digits: %d", cdmaMsg.sSubAddress.number_of_digits);

    QCRIL_LOG_INFO("uBearerDataLen: %d", cdmaMsg.uBearerDataLen);
}
