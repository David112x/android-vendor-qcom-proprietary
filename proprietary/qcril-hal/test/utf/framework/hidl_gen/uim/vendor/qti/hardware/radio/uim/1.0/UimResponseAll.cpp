/******************************************************************************
#  Copyright (c) 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file framework/hidl_gen/uim/vendor/qti/hardware/radio/uim/1.0/UimResponseAll.cpp
  DESCRIPTION
  ---------------------------------------------------------------------------
******************************************************************************/
#define LOG_TAG "vendor.qti.hardware.radio.uim@1.0::UimResponse"

#undef UNUSED

#include <vendor/qti/hardware/radio/uim/1.0/IUimResponse.h>
#include <android/log.h>
#include <cutils/trace.h>
#include "ril_utf_hidl_services.h"

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace uim {
namespace V1_0 {

std::string toString(const ::android::sp<IUimResponse>& o) {
    std::string os = "[class or subclass of ";
    os += IUimResponse::descriptor;
    os += "]";
    os += o->isRemote() ? "@remote" : "@local";
    return os;
}

const char* IUimResponse::descriptor("vendor.qti.hardware.radio.uim@1.0::IUimResponse");

// Methods from IUimResponse follow.
// no default implementation for: ::android::hardware::Return<void> IUimResponse::UimRemoteSimlockResponse(int32_t token, UimRemoteSimlockResponseType response, UimRemoteSimlockOperationType simlockOp, const ::android::hardware::hidl_vec<uint8_t>& simlockRspData, const UimRemoteSimlockVersion& version, const UimRemoteSimlockStatus& status)

// Methods from ::android::hidl::base::V1_0::IBase follow.
::android::hardware::Return<void> IUimResponse::interfaceChain(interfaceChain_cb _hidl_cb){
    return ::android::hardware::Void();}

::android::hardware::Return<void> IUimResponse::debug(const ::android::hardware::hidl_handle& fd, const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& options){
    (void)fd;
    (void)options;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IUimResponse::interfaceDescriptor(interfaceDescriptor_cb _hidl_cb){
    return ::android::hardware::Void();}

::android::hardware::Return<void> IUimResponse::getHashChain(getHashChain_cb _hidl_cb){
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IUimResponse::setHALInstrumentation(){
    return ::android::hardware::Void();
}

::android::hardware::Return<bool> IUimResponse::linkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient, uint64_t cookie){
    (void)cookie;
    return (recipient != nullptr);
}

::android::hardware::Return<void> IUimResponse::ping(){
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IUimResponse::getDebugInfo(getDebugInfo_cb _hidl_cb){
    return ::android::hardware::Void();}

::android::hardware::Return<void> IUimResponse::notifySyspropsChanged(){
    return ::android::hardware::Void();}

::android::hardware::Return<bool> IUimResponse::unlinkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient){
    return (recipient != nullptr);
}


// static 
::android::hardware::Return<::android::sp<IUimResponse>> IUimResponse::castFrom(const ::android::sp<IUimResponse>& parent, bool /* emitError */) {
    return parent;
}

// static 
::android::hardware::Return<::android::sp<IUimResponse>> IUimResponse::castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError) {
    return nullptr;
}

// static
::android::sp<IUimResponse> IUimResponse::tryGetService(const std::string &serviceName, const bool getStub) {
    return nullptr;
}

// static
::android::sp<IUimResponse> IUimResponse::getService(const std::string &serviceName, const bool getStub) {
    return nullptr;
}

::android::status_t IUimResponse::registerAsService(const std::string &serviceName) {
    return ::android::OK;
}

bool IUimResponse::registerForNotifications(
        const std::string &serviceName,
        const ::android::sp<::android::hidl::manager::V1_0::IServiceNotification> &notification) {
    return true;
}

}  // namespace V1_0
}  // namespace uim
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
