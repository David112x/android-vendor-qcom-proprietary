/******************************************************************************
#  Copyright (c) 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file framework/hidl_gen/lpa/vendor/qti/hardware/radio/lpa/1.0/UimLpaAll.cpp
  DESCRIPTION
  ---------------------------------------------------------------------------
******************************************************************************/
#define LOG_TAG "vendor.qti.hardware.radio.lpa@1.0::UimLpa"

#undef UNUSED

#include <vendor/qti/hardware/radio/lpa/1.0/IUimLpa.h>
#include <android/log.h>
#include <cutils/trace.h>
#include "ril_utf_hidl_services.h"

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace lpa {
namespace V1_0 {

std::string toString(const ::android::sp<IUimLpa>& o) {
    std::string os = "[class or subclass of ";
    os += IUimLpa::descriptor;
    os += "]";
    os += o->isRemote() ? "@remote" : "@local";
    return os;
}

const char* IUimLpa::descriptor("vendor.qti.hardware.radio.lpa@1.0::IUimLpa");

// Methods from IUimLpa follow.
// no default implementation for: ::android::hardware::Return<void> IUimLpa::setCallback(const ::android::sp<IUimLpaResponse>& responseCallback, const ::android::sp<IUimLpaIndication>& indicationCallback)
// no default implementation for: ::android::hardware::Return<void> IUimLpa::UimLpaUserRequest(int32_t token, const UimLpaUserReq& userReq)
// no default implementation for: ::android::hardware::Return<void> IUimLpa::UimLpaHttpTxnCompletedRequest(int32_t token, UimLpaResult result, const ::android::hardware::hidl_vec<uint8_t>& responsePayload, const ::android::hardware::hidl_vec<UimLpaHttpCustomHeader>& customHeaders)

// Methods from ::android::hidl::base::V1_0::IBase follow.
::android::hardware::Return<void> IUimLpa::interfaceChain(interfaceChain_cb _hidl_cb){
    return ::android::hardware::Void();}

::android::hardware::Return<void> IUimLpa::debug(const ::android::hardware::hidl_handle& fd, const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& options){
    (void)fd;
    (void)options;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IUimLpa::interfaceDescriptor(interfaceDescriptor_cb _hidl_cb){
    return ::android::hardware::Void();}

::android::hardware::Return<void> IUimLpa::getHashChain(getHashChain_cb _hidl_cb){
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IUimLpa::setHALInstrumentation(){
    return ::android::hardware::Void();
}

::android::hardware::Return<bool> IUimLpa::linkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient, uint64_t cookie){
    (void)cookie;
    return (recipient != nullptr);
}

::android::hardware::Return<void> IUimLpa::ping(){
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IUimLpa::getDebugInfo(getDebugInfo_cb _hidl_cb){
    return ::android::hardware::Void();}

::android::hardware::Return<void> IUimLpa::notifySyspropsChanged(){
    return ::android::hardware::Void();}

::android::hardware::Return<bool> IUimLpa::unlinkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient){
    return (recipient != nullptr);
}


// static 
::android::hardware::Return<::android::sp<IUimLpa>> IUimLpa::castFrom(const ::android::sp<IUimLpa>& parent, bool /* emitError */) {
    return parent;
}

// static 
::android::hardware::Return<::android::sp<IUimLpa>> IUimLpa::castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError) {
    return nullptr;
}

// static
::android::sp<IUimLpa> IUimLpa::tryGetService(const std::string &serviceName, const bool getStub) {
    return nullptr;
}

// static
::android::sp<IUimLpa> IUimLpa::getService(const std::string &serviceName, const bool getStub) {
    return nullptr;
}

::android::status_t IUimLpa::registerAsService(const std::string &serviceName) {
    return ::android::OK;
}

bool IUimLpa::registerForNotifications(
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
