/*===========================================================================
 *
 *    Copyright (c) 2019 Qualcomm Technologies, Inc.
 *    All Rights Reserved.
 *    Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 *===========================================================================*/

#pragma once

#include "vendor/qti/hardware/radio/qtiradio/2.2/IQtiRadioIndication.h"

using namespace ::android::hardware;
using namespace ::vendor::qti::hardware::radio::qtiradio;
using ::android::hardware::Return;

class IQtiRadioIndicationClientImpl : public V2_2::IQtiRadioIndication {
public:

    virtual Return<void> qtiRadioIndication(V1_0::QtiRadioIndicationType type) override {
        return Void();
    }

    virtual Return<void> on5gStatusChange(V2_0::EnableStatus status) override {
        return Void();
    }

    virtual Return<void> onNrDcParamChange(const V2_0::DcParam& dcParam) override {
        return Void();
    }

    virtual Return<void> onNrBearerAllocationChange(V2_0::BearerStatus bearerStatus) override {
        return Void();
    }

    virtual Return<void> onSignalStrengthChange(const V2_0::SignalStrength& signalStrength) override {
        return Void();
    }

    virtual Return<void> onUpperLayerIndInfoChange(const V2_1::UpperLayerIndInfo& uliInfo) override {
        return Void();
    }

    virtual Return<void> onNrBearerAllocationChange_2_1(V2_1::BearerStatus bearerStatus) override {
        return Void();
    }

    virtual Return<void> on5gConfigInfoChange(V2_1::ConfigType config) override {
        return Void();
    }

    virtual Return<void> onNrIconTypeChange(V2_2::NrIconType iconType) override {
        return Void();
    }
};
