/*
  Copyright (c) 2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#define LOG_TAG "vendor.qti.hardware.radio.qtiradio@2.1::QtiRadioResponse"

#undef UNUSED

#include <vendor/qti/hardware/radio/qtiradio/2.1/IQtiRadioResponse.h>
#include <android/log.h>
#include <cutils/trace.h>
#include "ril_utf_hidl_services.h"


namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace qtiradio {
namespace V2_1 {

const char* IQtiRadioResponse::descriptor("vendor.qti.hardware.radio.qtiradio@2.1::IQtiRadioResponse");

// Methods from ::vendor::qti::hardware::radio::qtiradio::V1_0::IQtiRadioResponse follow.
// no default implementation for: ::android::hardware::Return<void> IQtiRadioResponse::getAtrResponse(const ::vendor::qti::hardware::radio::qtiradio::V1_0::QtiRadioResponseInfo& info, const ::android::hardware::hidl_string& atr)

// Methods from ::vendor::qti::hardware::radio::qtiradio::V2_0::IQtiRadioResponse follow.
// no default implementation for: ::android::hardware::Return<void> IQtiRadioResponse::onEnable5gResponse(int32_t serial, uint32_t errorCode, ::vendor::qti::hardware::radio::qtiradio::V2_0::Status status)
// no default implementation for: ::android::hardware::Return<void> IQtiRadioResponse::onDisable5gResponse(int32_t serial, uint32_t errorCode, ::vendor::qti::hardware::radio::qtiradio::V2_0::Status status)
// no default implementation for: ::android::hardware::Return<void> IQtiRadioResponse::onEnable5gOnlyResponse(int32_t serial, uint32_t errorCode, ::vendor::qti::hardware::radio::qtiradio::V2_0::Status status)
// no default implementation for: ::android::hardware::Return<void> IQtiRadioResponse::on5gStatusResponse(int32_t serial, uint32_t errorCode, ::vendor::qti::hardware::radio::qtiradio::V2_0::EnableStatus enabled)
// no default implementation for: ::android::hardware::Return<void> IQtiRadioResponse::onNrDcParamResponse(int32_t serial, uint32_t errorCode, const ::vendor::qti::hardware::radio::qtiradio::V2_0::DcParam& dcParam)
// no default implementation for: ::android::hardware::Return<void> IQtiRadioResponse::onNrBearerAllocationResponse(int32_t serial, uint32_t errorCode, ::vendor::qti::hardware::radio::qtiradio::V2_0::BearerStatus bearerStatus)
// no default implementation for: ::android::hardware::Return<void> IQtiRadioResponse::onSignalStrengthResponse(int32_t serial, uint32_t errorCode, const ::vendor::qti::hardware::radio::qtiradio::V2_0::SignalStrength& signalStrength)
// no default implementation for: ::android::hardware::Return<void> IQtiRadioResponse::sendCdmaSmsResponse(const ::vendor::qti::hardware::radio::qtiradio::V1_0::QtiRadioResponseInfo& info, const ::android::hardware::radio::V1_0::SendSmsResult& sms)

// Methods from ::vendor::qti::hardware::radio::qtiradio::V2_1::IQtiRadioResponse follow.
// no default implementation for: ::android::hardware::Return<void> IQtiRadioResponse::onUpperLayerIndInfoResponse(int32_t serial, uint32_t errorCode, const ::vendor::qti::hardware::radio::qtiradio::V2_1::UpperLayerIndInfo& uliInfo)
// no default implementation for: ::android::hardware::Return<void> IQtiRadioResponse::onNrBearerAllocationResponse_2_1(int32_t serial, uint32_t errorCode, ::vendor::qti::hardware::radio::qtiradio::V2_1::BearerStatus bearerStatus)
// no default implementation for: ::android::hardware::Return<void> IQtiRadioResponse::on5gConfigInfoResponse(int32_t serial, uint32_t errorCode, ::vendor::qti::hardware::radio::qtiradio::V2_1::ConfigType config)

// Methods from ::android::hidl::base::V1_0::IBase follow.
::android::hardware::Return<void> IQtiRadioResponse::interfaceChain(interfaceChain_cb _hidl_cb){
    return ::android::hardware::Void();}

::android::hardware::Return<void> IQtiRadioResponse::debug(const ::android::hardware::hidl_handle& fd, const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& options){
    (void)fd;
    (void)options;
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IQtiRadioResponse::interfaceDescriptor(interfaceDescriptor_cb _hidl_cb){
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IQtiRadioResponse::getHashChain(getHashChain_cb _hidl_cb){
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IQtiRadioResponse::setHALInstrumentation(){
    return ::android::hardware::Void();
}

::android::hardware::Return<bool> IQtiRadioResponse::linkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient, uint64_t cookie){
    (void)cookie;
    return (recipient != nullptr);
}

::android::hardware::Return<void> IQtiRadioResponse::ping(){
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IQtiRadioResponse::getDebugInfo(getDebugInfo_cb _hidl_cb){
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IQtiRadioResponse::notifySyspropsChanged(){
    return ::android::hardware::Void();
}

::android::hardware::Return<bool> IQtiRadioResponse::unlinkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient){
    return (recipient != nullptr);
}


::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_1::IQtiRadioResponse>> IQtiRadioResponse::castFrom(const ::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_1::IQtiRadioResponse>& parent, bool /* emitError */) {
    return parent;
}

::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_1::IQtiRadioResponse>> IQtiRadioResponse::castFrom(const ::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_0::IQtiRadioResponse>& parent, bool emitError) {
    return ::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_1::IQtiRadioResponse>(static_cast<::vendor::qti::hardware::radio::qtiradio::V2_1::IQtiRadioResponse*>(parent.get()));
}

::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_1::IQtiRadioResponse>> IQtiRadioResponse::castFrom(const ::android::sp<::vendor::qti::hardware::radio::qtiradio::V1_0::IQtiRadioResponse>& parent, bool emitError) {
    return ::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_1::IQtiRadioResponse>(static_cast<::vendor::qti::hardware::radio::qtiradio::V2_1::IQtiRadioResponse*>(parent.get()));
}

::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_1::IQtiRadioResponse>> IQtiRadioResponse::castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError) {
    return ::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_1::IQtiRadioResponse>(static_cast<::vendor::qti::hardware::radio::qtiradio::V2_1::IQtiRadioResponse*>(parent.get()));
}

}  // namespace V2_1
}  // namespace qtiradio
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
