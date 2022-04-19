/******************************************************************************
#  Copyright (c) 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file framework/hidl_gen/uim_remote_client/vendor/qti/hardware/radio/uim_remote_client/1.0/UimRemoteServiceClientResponseAll.cpp
  DESCRIPTION
  ---------------------------------------------------------------------------
******************************************************************************/
#define LOG_TAG "vendor.qti.hardware.radio.uim_remote_client@1.0::UimRemoteServiceClientResponse"

#undef UNUSED

#include <vendor/qti/hardware/radio/uim_remote_client/1.0/IUimRemoteServiceClientResponse.h>
#include <android/log.h>
#include <cutils/trace.h>
#include "ril_utf_hidl_services.h"

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace uim_remote_client {
namespace V1_0 {

std::string toString(const ::android::sp<IUimRemoteServiceClientResponse>& o) {
    std::string os = "[class or subclass of ";
    os += IUimRemoteServiceClientResponse::descriptor;
    os += "]";
    os += o->isRemote() ? "@remote" : "@local";
    return os;
}

const char* IUimRemoteServiceClientResponse::descriptor("vendor.qti.hardware.radio.uim_remote_client@1.0::IUimRemoteServiceClientResponse");

// Methods from IUimRemoteServiceClientResponse follow.
// no default implementation for: ::android::hardware::Return<void> IUimRemoteServiceClientResponse::UimRemoteServiceClientEventResp(int32_t token, UimRemoteClientEventRsp eventResp)
// no default implementation for: ::android::hardware::Return<void> IUimRemoteServiceClientResponse::UimRemoteServiceClientApduResp(int32_t token, UimRemoteClientApduRsp apduResp)

// Methods from ::android::hidl::base::V1_0::IBase follow.
::android::hardware::Return<void> IUimRemoteServiceClientResponse::interfaceChain(interfaceChain_cb _hidl_cb){
    return ::android::hardware::Void();}

::android::hardware::Return<void> IUimRemoteServiceClientResponse::debug(const ::android::hardware::hidl_handle& fd, const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& options){
    (void)fd;
    (void)options;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IUimRemoteServiceClientResponse::interfaceDescriptor(interfaceDescriptor_cb _hidl_cb){
    return ::android::hardware::Void();}

::android::hardware::Return<void> IUimRemoteServiceClientResponse::getHashChain(getHashChain_cb _hidl_cb){
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IUimRemoteServiceClientResponse::setHALInstrumentation(){
    return ::android::hardware::Void();
}

::android::hardware::Return<bool> IUimRemoteServiceClientResponse::linkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient, uint64_t cookie){
    (void)cookie;
    return (recipient != nullptr);
}

::android::hardware::Return<void> IUimRemoteServiceClientResponse::ping(){
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IUimRemoteServiceClientResponse::getDebugInfo(getDebugInfo_cb _hidl_cb){
    return ::android::hardware::Void();}

::android::hardware::Return<void> IUimRemoteServiceClientResponse::notifySyspropsChanged(){
    return ::android::hardware::Void();}

::android::hardware::Return<bool> IUimRemoteServiceClientResponse::unlinkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient){
    return (recipient != nullptr);
}


// static 
::android::hardware::Return<::android::sp<IUimRemoteServiceClientResponse>> IUimRemoteServiceClientResponse::castFrom(const ::android::sp<IUimRemoteServiceClientResponse>& parent, bool /* emitError */) {
    return parent;
}

// static 
::android::hardware::Return<::android::sp<IUimRemoteServiceClientResponse>> IUimRemoteServiceClientResponse::castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError) {
    return nullptr;
}

// static
::android::sp<IUimRemoteServiceClientResponse> IUimRemoteServiceClientResponse::tryGetService(const std::string &serviceName, const bool getStub) {
    return nullptr;
}

// static
::android::sp<IUimRemoteServiceClientResponse> IUimRemoteServiceClientResponse::getService(const std::string &serviceName, const bool getStub) {
    return nullptr;
}

::android::status_t IUimRemoteServiceClientResponse::registerAsService(const std::string &serviceName) {
    return ::android::OK;
}

bool IUimRemoteServiceClientResponse::registerForNotifications(
        const std::string &serviceName,
        const ::android::sp<::android::hidl::manager::V1_0::IServiceNotification> &notification) {
    return true;
}

}  // namespace V1_0
}  // namespace uim_remote_client
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
