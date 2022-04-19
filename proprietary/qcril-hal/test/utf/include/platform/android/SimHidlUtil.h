/*===========================================================================
 *
 *    Copyright (c) 2019 Qualcomm Technologies, Inc.
 *    All Rights Reserved.
 *    Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 *===========================================================================*/

#pragma once

#include "android/hardware/radio/1.4/IRadio.h"
#include "ril.h"

using namespace ::android::hardware::radio;

int convertSimIoPayloadtoHidl(void *data, size_t datalen, V1_0::IccIo& iccIoData);
int convertSimIoRespHidltoRilPayload(const V1_0::IccIoResult& iccIoResult, RIL_SIM_IO_Response& data);
int convertGetIccCardStatusRespHidltoRilPayload(const V1_2::CardStatus& cardStatusData,
                                                RIL_CardStatus_v6& data);
int convertGetIccCardStatusRespHidltoRilPayload_1_4(const V1_4::CardStatus& cardStatusData,
                                                RIL_CardStatus_v6& data);
void convertOpenLogicalChannelRespHidlToRilPayload
       (const android::hardware::hidl_vec<int8_t>& selectResponse, int32_t channel,
        uint8_t *payload);
void convertOpenLogicalChannelPayloadtoHidl(void* data, size_t datalen,
       uint8_t* p2, android::hardware::hidl_string& aid);

