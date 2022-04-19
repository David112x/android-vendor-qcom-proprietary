/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "RadioConfigImpl.h"
#include <android/hardware/radio/config/1.1/IRadioConfig.h>
#include <android/hardware/radio/config/1.1/IRadioConfigIndication.h>
#include <android/hardware/radio/config/1.1/IRadioConfigResponse.h>
#include <android/hardware/radio/config/1.1/types.h>
#include "request/SetPreferredDataModemRequestMessage.h"

using namespace android::hardware::radio;
class RadioConfigImpl_1_1 : public RadioConfigImpl,
                            public config::V1_1::IRadioConfig {
protected:
    sp<config::V1_1::IRadioConfigIndication> mIndicationCb_1_1;
    sp<config::V1_1::IRadioConfigResponse> mResponseCb_1_1;

public:
    virtual void clearCallbacks() override;
    sp<config::V1_1::IRadioConfigResponse> getResponseCallback_1_1();
    virtual void createRadioConfigModule() override;
    virtual ::android::status_t registerAsService(const std::string &serviceName) override;
    static const QcrildConfigVersion &getVersion();
    virtual void setPreferredDataModemResponse(int32_t serial,
                                               std::shared_ptr<rildata::SetPreferredDataModemResponse_t> response);
    virtual Return<void> setResponseFunctions_nolock(const sp<IRadioConfigResponse>& radioConfigResponse,
                                                     const sp<IRadioConfigIndication>& radioConfigIndication) override;
    Return<void> setResponseFunctions(const sp<IRadioConfigResponse>& radioConfigResponse,
                                      const sp<IRadioConfigIndication>& radioConfigIndication);
    Return<void> getPhoneCapability(int32_t serial);
    Return<void> setPreferredDataModem(int32_t serial, uint8_t modemId);

    Return<void> getSimSlotsStatus(int32_t serial) {
        return RadioConfigImpl::getSimSlotsStatus(serial);
    }
    Return<void> setSimSlotsMapping(int32_t serial, const hidl_vec<uint32_t>& slotMap) {
        return RadioConfigImpl::setSimSlotsMapping(serial, slotMap);
    }

    Return<void> setModemsConfig(int32_t /*serial*/, const ::android::hardware::radio::config::V1_1::ModemsConfig& /*modemconfig*/);
    Return<void> getModemsConfig(int32_t /*serial*/);

};
