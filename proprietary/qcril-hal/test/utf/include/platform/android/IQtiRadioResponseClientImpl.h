/*===========================================================================
 *
 *    Copyright (c) 2019 Qualcomm Technologies, Inc.
 *    All Rights Reserved.
 *    Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 *===========================================================================*/

#pragma once

#include "vendor/qti/hardware/radio/qtiradio/2.3/IQtiRadioResponse.h"

using namespace ::android::hardware;
using namespace ::vendor::qti::hardware::radio::qtiradio;
using ::android::hardware::Return;

class IQtiRadioResponseClientImpl : public V2_3::IQtiRadioResponse {
    virtual Return<void> getAtrResponse(const V1_0::QtiRadioResponseInfo& info, const hidl_string& atr) override {
        return Void();
    }

    virtual Return<void> onEnable5gResponse(int32_t serial, uint32_t errorCode, V2_0::Status status) override;

    virtual Return<void> onDisable5gResponse(int32_t serial, uint32_t errorCode, V2_0::Status status) override {
        return Void();
    }

    virtual Return<void> onEnable5gOnlyResponse(int32_t serial, uint32_t errorCode, V2_0::Status status) override {
        return Void();
    }

    virtual Return<void> on5gStatusResponse(int32_t serial, uint32_t errorCode, V2_0::EnableStatus enabled) override {
        return Void();
    }

    virtual Return<void> onNrDcParamResponse(int32_t serial, uint32_t errorCode, const V2_0::DcParam& dcParam) override {
        return Void();
    }

    virtual Return<void> onNrBearerAllocationResponse(int32_t serial, uint32_t errorCode, V2_0::BearerStatus bearerStatus) override {
        return Void();
    }

    virtual Return<void> onSignalStrengthResponse(int32_t serial, uint32_t errorCode, const V2_0::SignalStrength& signalStrength) override {
        return Void();
    }

    virtual Return<void> sendCdmaSmsResponse(const V1_0::QtiRadioResponseInfo& info, const radio::V1_0::SendSmsResult& sms) override {
        return Void();
    }

    virtual Return<void> onUpperLayerIndInfoResponse(int32_t serial, uint32_t errorCode, const V2_1::UpperLayerIndInfo& uliInfo) override {
        return Void();
    }

    virtual Return<void> onNrBearerAllocationResponse_2_1(int32_t serial, uint32_t errorCode, V2_1::BearerStatus bearerStatus) override {
        return Void();
    }

    virtual Return<void> on5gConfigInfoResponse(int32_t serial, uint32_t errorCode, V2_1::ConfigType config) override {
        return Void();
    }

    virtual Return<void> onNrIconTypeResponse(int32_t serial, uint32_t errorCode, V2_2::NrIconType iconType) override {
        return Void();
    }

    virtual Return<void> onEnableEndcResponse(int32_t serial, uint32_t errorCode, V2_0::Status status) override {
        return Void();
    }

    virtual Return<void> onEndcStatusResponse(int32_t serial, uint32_t errorCode, V2_3::EndcStatus endcStatus) override {
        return Void();
    }
};
