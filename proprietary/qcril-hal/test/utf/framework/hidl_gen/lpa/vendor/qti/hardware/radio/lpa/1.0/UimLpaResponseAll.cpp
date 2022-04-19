/******************************************************************************
#  Copyright (c) 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file framework/hidl_gen/lpa/vendor/qti/hardware/radio/lpa/1.0/UimLpaResponseAll.cpp
  DESCRIPTION
  ---------------------------------------------------------------------------
******************************************************************************/
#define LOG_TAG "vendor.qti.hardware.radio.lpa@1.0::UimLpaResponse"

#undef UNUSED

#include <vendor/qti/hardware/radio/lpa/1.0/IUimLpaResponse.h>
#include <android/log.h>
#include <cutils/trace.h>
#include "ril_utf_hidl_services.h"

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace lpa {
namespace V1_0 {

std::string toString(const ::android::sp<IUimLpaResponse>& o) {
    std::string os = "[class or subclass of ";
    os += IUimLpaResponse::descriptor;
    os += "]";
    os += o->isRemote() ? "@remote" : "@local";
    return os;
}

const char* IUimLpaResponse::descriptor("vendor.qti.hardware.radio.lpa@1.0::IUimLpaResponse");

// Methods from IUimLpaResponse follow.
// no default implementation for: ::android::hardware::Return<void> IUimLpaResponse::UimLpaUserResponse(int32_t token, const UimLpaUserResp& response)
// no default implementation for: ::android::hardware::Return<void> IUimLpaResponse::UimLpaHttpTxnCompletedResponse(int32_t token, UimLpaResult result)

// Methods from ::android::hidl::base::V1_0::IBase follow.
::android::hardware::Return<void> IUimLpaResponse::interfaceChain(interfaceChain_cb _hidl_cb){
    return ::android::hardware::Void();}

::android::hardware::Return<void> IUimLpaResponse::debug(const ::android::hardware::hidl_handle& fd, const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& options){
    (void)fd;
    (void)options;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IUimLpaResponse::interfaceDescriptor(interfaceDescriptor_cb _hidl_cb){
    return ::android::hardware::Void();}

::android::hardware::Return<void> IUimLpaResponse::getHashChain(getHashChain_cb _hidl_cb){
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IUimLpaResponse::setHALInstrumentation(){
    return ::android::hardware::Void();
}

::android::hardware::Return<bool> IUimLpaResponse::linkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient, uint64_t cookie){
    (void)cookie;
    return (recipient != nullptr);
}

::android::hardware::Return<void> IUimLpaResponse::ping(){
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IUimLpaResponse::getDebugInfo(getDebugInfo_cb _hidl_cb){
    return ::android::hardware::Void();}

::android::hardware::Return<void> IUimLpaResponse::notifySyspropsChanged(){
    return ::android::hardware::Void();}

::android::hardware::Return<bool> IUimLpaResponse::unlinkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient){
    return (recipient != nullptr);
}


// static 
::android::hardware::Return<::android::sp<IUimLpaResponse>> IUimLpaResponse::castFrom(const ::android::sp<IUimLpaResponse>& parent, bool /* emitError */) {
    return parent;
}

// static 
::android::hardware::Return<::android::sp<IUimLpaResponse>> IUimLpaResponse::castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError) {
    return nullptr;
}

// static
::android::sp<IUimLpaResponse> IUimLpaResponse::tryGetService(const std::string &serviceName, const bool getStub) {
    return nullptr;
}

// static
::android::sp<IUimLpaResponse> IUimLpaResponse::getService(const std::string &serviceName, const bool getStub) {
    return nullptr;
}

::android::status_t IUimLpaResponse::registerAsService(const std::string &serviceName) {
    return ::android::OK;
}

bool IUimLpaResponse::registerForNotifications(
        const std::string &serviceName,
        const ::android::sp<::android::hidl::manager::V1_0::IServiceNotification> &notification) {
    return true;
}

}  // namespace V1_0
}  // namespace lpa
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
