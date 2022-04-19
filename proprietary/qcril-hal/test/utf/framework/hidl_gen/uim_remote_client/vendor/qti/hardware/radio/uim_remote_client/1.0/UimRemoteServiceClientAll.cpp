/******************************************************************************
#  Copyright (c) 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file framework/hidl_gen/uim_remote_client/vendor/qti/hardware/radio/uim_remote_client/1.0/UimRemoteServiceClientAll.cpp
  DESCRIPTION
  ---------------------------------------------------------------------------
******************************************************************************/
#define LOG_TAG "vendor.qti.hardware.radio.uim_remote_client@1.0::UimRemoteServiceClient"

#undef UNUSED

#include <vendor/qti/hardware/radio/uim_remote_client/1.0/IUimRemoteServiceClient.h>
#include <android/log.h>
#include <cutils/trace.h>
#include "ril_utf_hidl_services.h"

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace uim_remote_client {
namespace V1_0 {

std::string toString(const ::android::sp<IUimRemoteServiceClient>& o) {
    std::string os = "[class or subclass of ";
    os += IUimRemoteServiceClient::descriptor;
    os += "]";
    os += o->isRemote() ? "@remote" : "@local";
    return os;
}

const char* IUimRemoteServiceClient::descriptor("vendor.qti.hardware.radio.uim_remote_client@1.0::IUimRemoteServiceClient");

// Methods from IUimRemoteServiceClient follow.
// no default implementation for: ::android::hardware::Return<void> IUimRemoteServiceClient::setCallback(const ::android::sp<IUimRemoteServiceClientResponse>& responseCallback, const ::android::sp<IUimRemoteServiceClientIndication>& indicationCallback)
// no default implementation for: ::android::hardware::Return<void> IUimRemoteServiceClient::UimRemoteServiceClientEventReq(int32_t token, const UimRemoteEventReqType& eventReq)
// no default implementation for: ::android::hardware::Return<void> IUimRemoteServiceClient::UimRemoteServiceClientApduReq(int32_t token, UimRemoteClientApduStatus apduStatus, const ::android::hardware::hidl_vec<uint8_t>& apduResponse)

// Methods from ::android::hidl::base::V1_0::IBase follow.
::android::hardware::Return<void> IUimRemoteServiceClient::interfaceChain(interfaceChain_cb _hidl_cb){
    return ::android::hardware::Void();}

::android::hardware::Return<void> IUimRemoteServiceClient::debug(const ::android::hardware::hidl_handle& fd, const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& options){
    (void)fd;
    (void)options;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IUimRemoteServiceClient::interfaceDescriptor(interfaceDescriptor_cb _hidl_cb){
    return ::android::hardware::Void();}

::android::hardware::Return<void> IUimRemoteServiceClient::getHashChain(getHashChain_cb _hidl_cb){
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IUimRemoteServiceClient::setHALInstrumentation(){
    return ::android::hardware::Void();
}

::android::hardware::Return<bool> IUimRemoteServiceClient::linkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient, uint64_t cookie){
    (void)cookie;
    return (recipient != nullptr);
}

::android::hardware::Return<void> IUimRemoteServiceClient::ping(){
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IUimRemoteServiceClient::getDebugInfo(getDebugInfo_cb _hidl_cb){
    return ::android::hardware::Void();}

::android::hardware::Return<void> IUimRemoteServiceClient::notifySyspropsChanged(){
    return ::android::hardware::Void();}

::android::hardware::Return<bool> IUimRemoteServiceClient::unlinkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient){
    return (recipient != nullptr);
}


// static 
::android::hardware::Return<::android::sp<IUimRemoteServiceClient>> IUimRemoteServiceClient::castFrom(const ::android::sp<IUimRemoteServiceClient>& parent, bool /* emitError */) {
    return parent;
}

// static 
::android::hardware::Return<::android::sp<IUimRemoteServiceClient>> IUimRemoteServiceClient::castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError) {
    return nullptr;
}

// static
::android::sp<IUimRemoteServiceClient> IUimRemoteServiceClient::tryGetService(const std::string &serviceName, const bool getStub) {
    return nullptr;
}

// static
::android::sp<IUimRemoteServiceClient> IUimRemoteServiceClient::getService(const std::string &serviceName, const bool getStub) {
    return nullptr;
}

::android::status_t IUimRemoteServiceClient::registerAsService(const std::string &serviceName) {
    return ::android::OK;
}

bool IUimRemoteServiceClient::registerForNotifications(
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
