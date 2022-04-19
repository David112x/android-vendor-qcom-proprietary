/*
  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#define LOG_TAG "vendor.qti.hardware.radio.qtiradio@2.0::QtiRadio"

#undef UNUSED

#include <vendor/qti/hardware/radio/qtiradio/2.0/IQtiRadio.h>
#include <android/log.h>
#include <cutils/trace.h>
#include "ril_utf_hidl_services.h"

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace qtiradio {
namespace V2_0 {

const char* IQtiRadio::descriptor("vendor.qti.hardware.radio.qtiradio@2.0::IQtiRadio");

// Methods from ::vendor::qti::hardware::radio::qtiradio::V1_0::IQtiRadio follow.
// no default implementation for: ::android::hardware::Return<void> IQtiRadio::setCallback(const ::android::sp<::vendor::qti::hardware::radio::qtiradio::V1_0::IQtiRadioResponse>& responseCallback, const ::android::sp<::vendor::qti::hardware::radio::qtiradio::V1_0::IQtiRadioIndication>& indicationCallback)
// no default implementation for: ::android::hardware::Return<void> IQtiRadio::getAtr(int32_t serial)

// Methods from ::vendor::qti::hardware::radio::qtiradio::V2_0::IQtiRadio follow.
// no default implementation for: ::android::hardware::Return<void> IQtiRadio::enable5g(int32_t serial)
// no default implementation for: ::android::hardware::Return<void> IQtiRadio::disable5g(int32_t serial)
// no default implementation for: ::android::hardware::Return<void> IQtiRadio::enable5gOnly(int32_t serial)
// no default implementation for: ::android::hardware::Return<void> IQtiRadio::query5gStatus(int32_t serial)
// no default implementation for: ::android::hardware::Return<void> IQtiRadio::queryNrDcParam(int32_t serial)
// no default implementation for: ::android::hardware::Return<void> IQtiRadio::queryNrBearerAllocation(int32_t serial)
// no default implementation for: ::android::hardware::Return<void> IQtiRadio::queryNrSignalStrength(int32_t serial)

// Methods from ::android::hidl::base::V1_0::IBase follow.
::android::hardware::Return<void> IQtiRadio::interfaceChain(interfaceChain_cb _hidl_cb){
    return ::android::hardware::Void();}

::android::hardware::Return<void> IQtiRadio::debug(const ::android::hardware::hidl_handle& fd, const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& options){
    (void)fd;
    (void)options;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IQtiRadio::interfaceDescriptor(interfaceDescriptor_cb _hidl_cb){
    return ::android::hardware::Void();}

::android::hardware::Return<void> IQtiRadio::getHashChain(getHashChain_cb _hidl_cb){
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IQtiRadio::setHALInstrumentation(){
    return ::android::hardware::Void();
}

::android::hardware::Return<bool> IQtiRadio::linkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient, uint64_t cookie){
    (void)cookie;
    return (recipient != nullptr);
}

::android::hardware::Return<void> IQtiRadio::ping(){
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IQtiRadio::getDebugInfo(getDebugInfo_cb _hidl_cb){
    return ::android::hardware::Void();}

::android::hardware::Return<void> IQtiRadio::notifySyspropsChanged(){
    return ::android::hardware::Void();}

::android::hardware::Return<bool> IQtiRadio::unlinkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient){
    return (recipient != nullptr);
}


// static
::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_0::IQtiRadio>> IQtiRadio::castFrom(const ::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_0::IQtiRadio>& parent, bool /* emitError */) {
    return parent;
}

// static
::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_0::IQtiRadio>> IQtiRadio::castFrom(const ::android::sp<::vendor::qti::hardware::radio::qtiradio::V1_0::IQtiRadio>& parent, bool emitError) {
    return ::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_0::IQtiRadio>(static_cast<::vendor::qti::hardware::radio::qtiradio::V2_0::IQtiRadio*>(parent.get()));
}

// static
::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_0::IQtiRadio>> IQtiRadio::castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError) {
    return ::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_0::IQtiRadio>(static_cast<::vendor::qti::hardware::radio::qtiradio::V2_0::IQtiRadio*>(parent.get()));
}

static std::unordered_map<std::string, ::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_0::IQtiRadio>> mHidlServices;
::android::sp<IQtiRadio> IQtiRadio::tryGetService(const std::string &serviceName, const bool getStub) {
    return mHidlServices[serviceName];
}

::android::sp<IQtiRadio> IQtiRadio::getService(const std::string &serviceName, const bool getStub) {
    return mHidlServices[serviceName];
}

::android::status_t IQtiRadio::registerAsService(const std::string &serviceName) {
    mHidlServices[serviceName] = this;
    ::vendor::qti::hardware::radio::qtiradio::V1_0::IQtiRadio::registerAsService(serviceName);
    return ::android::OK;
}

}  // namespace V2_0
}  // namespace qtiradio
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
