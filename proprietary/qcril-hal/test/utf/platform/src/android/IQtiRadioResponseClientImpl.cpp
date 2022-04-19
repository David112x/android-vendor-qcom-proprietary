/*===========================================================================
 *
 *    Copyright (c) 2019 Qualcomm Technologies, Inc.
 *    All Rights Reserved.
 *    Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 *===========================================================================*/

#include "platform/android/IQtiRadioResponseClientImpl.h"
#include "ril_utf_rild_sim.h"

Return<void> IQtiRadioResponseClientImpl::onEnable5gResponse(int32_t serial, uint32_t errorCode, V2_0::Status status) {

    int slot = acquire_expectation_table();
    enqueue_ril_expect(
            nullptr,                // payload
            0,
            RIL_REQUEST_ENABLE_5G,  // msg_id
            serial,                 // serial_id
            (RIL_Errno)errorCode,   // RIL error
            ril_utf_ril_response);  // type

    release_expectation_table(slot);
    return Void();
}
