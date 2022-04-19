/*===========================================================================
 *
 *    Copyright (c) 2018 Qualcomm Technologies, Inc.
 *    All Rights Reserved.
 *    Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 *===========================================================================*/

#pragma once

#include "android/hardware/radio/1.2/IRadio.h"
#include "ril.h"

using namespace ::android::hardware::radio;

int convertGsmSmsPayloadtoHidl(void *data, size_t datalen, V1_0::GsmSmsMessage& sms);
int convertGsmSmsHidltoRilPayload(const V1_0::SendSmsResult& sms, RIL_SMS_Response& data);
int convertGsmSmsWritePayloadtoHidl(void *in, size_t datalen, V1_0::SmsWriteArgs& out);
int convertCdmaSmsWritePayloadtoHidl(void *in, size_t datalen, V1_0::CdmaSmsWriteArgs& out);
int convertCdmaSmsAcktoHidl(void *in, size_t datalen, V1_0::CdmaSmsAck& out);
int convertCdmaBcConfigtoHidl(void *in, size_t datalen,
    android::hardware::hidl_vec<android::hardware::radio::V1_0::CdmaBroadcastSmsConfigInfo>& out);
int convertGsmBcConfigtoHidl(void *in, size_t datalen,
    android::hardware::hidl_vec<android::hardware::radio::V1_0::GsmBroadcastSmsConfigInfo>& out);
