/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#define LOG_TAG "vendor.qti.hardware.radio.qtiradio@1.0::QtiRadioResponse"

#undef UNUSED

#include <vendor/qti/hardware/radio/qtiradio/1.0/IQtiRadioResponse.h>
#include <android/log.h>
#include <cutils/trace.h>
#include "ril_utf_hidl_services.h"

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace qtiradio {
namespace V1_0 {

std::string toString(const ::android::sp<IQtiRadioResponse>& o) {
    std::string os = "[class or subclass of ";
    os += IQtiRadioResponse::descriptor;
    os += "]";
    os += o->isRemote() ? "@remote" : "@local";
    return os;
}

const char* IQtiRadioResponse::descriptor("vendor.qti.hardware.radio.qtiradio@1.0::IQtiRadioResponse");

// Methods from IQtiRadioResponse follow.
// no default implementation for: ::android::hardware::Return<void> IQtiRadioResponse::getAtrResponse(const QtiRadioResponseInfo& info, const ::android::hardware::hidl_string& atr)

// Methods from ::android::hidl::base::V1_0::IBase follow.
::android::hardware::Return<void> IQtiRadioResponse::interfaceChain(interfaceChain_cb _hidl_cb){
    return ::android::hardware::Void();}

::android::hardware::Return<void> IQtiRadioResponse::debug(const ::android::hardware::hidl_handle& fd, const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& options){
    (void)fd;
    (void)options;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IQtiRadioResponse::interfaceDescriptor(interfaceDescriptor_cb _hidl_cb){
    return ::android::hardware::Void();}

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
    return ::android::hardware::Void();}

::android::hardware::Return<void> IQtiRadioResponse::notifySyspropsChanged(){
    return ::android::hardware::Void();}

::android::hardware::Return<bool> IQtiRadioResponse::unlinkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient){
    return (recipient != nullptr);
}


// static 
::android::hardware::Return<::android::sp<IQtiRadioResponse>> IQtiRadioResponse::castFrom(const ::android::sp<IQtiRadioResponse>& parent, bool /* emitError */) {
    return parent;
}

// static
::android::sp<IQtiRadioResponse> IQtiRadioResponse::tryGetService(const std::string &serviceName, const bool getStub) {
    return nullptr;
}

// static
::android::sp<IQtiRadioResponse> IQtiRadioResponse::getService(const std::string &serviceName, const bool getStub) {
    return nullptr;
}

::android::status_t IQtiRadioResponse::registerAsService(const std::string &serviceName) {
    return ::android::OK;
}

bool IQtiRadioResponse::registerForNotifications(
        const std::string &serviceName,
        const ::android::sp<::android::hidl::manager::V1_0::IServiceNotification> &notification) {
    return true;
}

static_assert(sizeof(::android::hardware::MQDescriptor<char, ::android::hardware::kSynchronizedReadWrite>) == 32, "wrong size");
static_assert(sizeof(::android::hardware::hidl_handle) == 16, "wrong size");
static_assert(sizeof(::android::hardware::hidl_memory) == 40, "wrong size");
static_assert(sizeof(::android::hardware::hidl_string) == 16, "wrong size");
static_assert(sizeof(::android::hardware::hidl_vec<char>) == 16, "wrong size");

}  // namespace V1_0
}  // namespace qtiradio
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
