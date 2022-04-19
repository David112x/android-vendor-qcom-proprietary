/*
  Copyright (c) 2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#define LOG_TAG "vendor.qti.hardware.radio.qtiradio@2.1::QtiRadioIndication"

#undef UNUSED

#include <vendor/qti/hardware/radio/qtiradio/2.1/IQtiRadioIndication.h>
#include <android/log.h>
#include <cutils/trace.h>
#include "ril_utf_hidl_services.h"


namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace qtiradio {
namespace V2_1 {

const char* IQtiRadioIndication::descriptor("vendor.qti.hardware.radio.qtiradio@2.1::IQtiRadioIndication");

// Methods from ::vendor::qti::hardware::radio::qtiradio::V1_0::IQtiRadioIndication follow.
// no default implementation for: ::android::hardware::Return<void> IQtiRadioIndication::qtiRadioIndication(::vendor::qti::hardware::radio::qtiradio::V1_0::QtiRadioIndicationType type)

// Methods from ::vendor::qti::hardware::radio::qtiradio::V2_0::IQtiRadioIndication follow.
// no default implementation for: ::android::hardware::Return<void> IQtiRadioIndication::on5gStatusChange(::vendor::qti::hardware::radio::qtiradio::V2_0::EnableStatus status)
// no default implementation for: ::android::hardware::Return<void> IQtiRadioIndication::onNrDcParamChange(const ::vendor::qti::hardware::radio::qtiradio::V2_0::DcParam& dcParam)
// no default implementation for: ::android::hardware::Return<void> IQtiRadioIndication::onNrBearerAllocationChange(::vendor::qti::hardware::radio::qtiradio::V2_0::BearerStatus bearerStatus)
// no default implementation for: ::android::hardware::Return<void> IQtiRadioIndication::onSignalStrengthChange(const ::vendor::qti::hardware::radio::qtiradio::V2_0::SignalStrength& signalStrength)

// Methods from ::vendor::qti::hardware::radio::qtiradio::V2_1::IQtiRadioIndication follow.
// no default implementation for: ::android::hardware::Return<void> IQtiRadioIndication::onUpperLayerIndInfoChange(const ::vendor::qti::hardware::radio::qtiradio::V2_1::UpperLayerIndInfo& uliInfo)
// no default implementation for: ::android::hardware::Return<void> IQtiRadioIndication::onNrBearerAllocationChange_2_1(::vendor::qti::hardware::radio::qtiradio::V2_1::BearerStatus bearerStatus)
// no default implementation for: ::android::hardware::Return<void> IQtiRadioIndication::on5gConfigInfoChange(::vendor::qti::hardware::radio::qtiradio::V2_1::ConfigType config)

// Methods from ::android::hidl::base::V1_0::IBase follow.
::android::hardware::Return<void> IQtiRadioIndication::interfaceChain(interfaceChain_cb _hidl_cb){
    return ::android::hardware::Void();}

::android::hardware::Return<void> IQtiRadioIndication::debug(const ::android::hardware::hidl_handle& fd, const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& options){
    (void)fd;
    (void)options;
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IQtiRadioIndication::interfaceDescriptor(interfaceDescriptor_cb _hidl_cb){
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IQtiRadioIndication::getHashChain(getHashChain_cb _hidl_cb){
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IQtiRadioIndication::setHALInstrumentation(){
    return ::android::hardware::Void();
}

::android::hardware::Return<bool> IQtiRadioIndication::linkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient, uint64_t cookie){
    (void)cookie;
    return (recipient != nullptr);
}

::android::hardware::Return<void> IQtiRadioIndication::ping(){
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IQtiRadioIndication::getDebugInfo(getDebugInfo_cb _hidl_cb){
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IQtiRadioIndication::notifySyspropsChanged(){
    return ::android::hardware::Void();
}

::android::hardware::Return<bool> IQtiRadioIndication::unlinkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient){
    return (recipient != nullptr);
}


::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_1::IQtiRadioIndication>> IQtiRadioIndication::castFrom(const ::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_1::IQtiRadioIndication>& parent, bool /* emitError */) {
    return parent;
}

::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_1::IQtiRadioIndication>> IQtiRadioIndication::castFrom(const ::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_0::IQtiRadioIndication>& parent, bool emitError) {
    return ::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_1::IQtiRadioIndication>(static_cast<::vendor::qti::hardware::radio::qtiradio::V2_1::IQtiRadioIndication*>(parent.get()));
}

::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_1::IQtiRadioIndication>> IQtiRadioIndication::castFrom(const ::android::sp<::vendor::qti::hardware::radio::qtiradio::V1_0::IQtiRadioIndication>& parent, bool emitError) {
    return ::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_1::IQtiRadioIndication>(static_cast<::vendor::qti::hardware::radio::qtiradio::V2_1::IQtiRadioIndication*>(parent.get()));
}

::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_1::IQtiRadioIndication>> IQtiRadioIndication::castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError) {
    return ::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_1::IQtiRadioIndication>(static_cast<::vendor::qti::hardware::radio::qtiradio::V2_1::IQtiRadioIndication*>(parent.get()));
}

}  // namespace V2_1
}  // namespace qtiradio
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
