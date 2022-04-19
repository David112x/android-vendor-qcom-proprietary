/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "RadioConfigImpl_1_1.h"
#include <android/hardware/radio/config/1.2/IRadioConfigIndication.h>
#include <android/hardware/radio/config/1.2/IRadioConfigResponse.h>
#include <android/hardware/radio/config/1.2/types.h>

using namespace android::hardware::radio;
class RadioConfigImpl_1_2 : public RadioConfigImpl_1_1 {
protected:
    sp<config::V1_2::IRadioConfigIndication> mIndicationCb_1_2;
    sp<config::V1_2::IRadioConfigResponse>   mResponseCb_1_2;

public:
    sp<config::V1_2::IRadioConfigResponse> getResponseCallback_1_2();
    sp<config::V1_2::IRadioConfigIndication> getIndicationCallback_1_2();
    virtual void createRadioConfigModule() override;

    virtual void clearCallbacks() override;
    virtual ::android::status_t registerAsService(const std::string &serviceName) override;
    static const QcrildConfigVersion &getVersion();
    Return<void> setResponseFunctions(const sp<IRadioConfigResponse>& radioConfigResponse,
                                               const sp<IRadioConfigIndication>& radioConfigIndication);
    Return<void> getPhoneCapability(int32_t serial)
    {
      return RadioConfigImpl_1_1::getPhoneCapability(serial);
    }
    Return<void> setPreferredDataModem(int32_t serial, uint8_t modemId)
    {
      return RadioConfigImpl_1_1::setPreferredDataModem(serial, modemId);
    }
    Return<void> getSimSlotsStatus(int32_t serial);
    Return<void> setSimSlotsMapping(int32_t serial, const hidl_vec<uint32_t>& slotMap)
    {
      return RadioConfigImpl_1_1::setSimSlotsMapping(serial, slotMap);
    }
    Return<void> setModemsConfig(int32_t serial, const config::V1_1::ModemsConfig& modemconfig)
    {
      return RadioConfigImpl_1_1::setModemsConfig(serial, modemconfig);
    }
    Return<void> getModemsConfig(int32_t serial)
    {
      return RadioConfigImpl_1_1::getModemsConfig(serial);
    }
    virtual void sendSlotStatusIndication(const std::shared_ptr<UimSlotStatusInd> msg_ptr);
};
