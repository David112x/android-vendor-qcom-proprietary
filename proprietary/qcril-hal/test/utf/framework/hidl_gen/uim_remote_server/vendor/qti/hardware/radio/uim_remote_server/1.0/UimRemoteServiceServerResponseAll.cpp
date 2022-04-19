/******************************************************************************
#  Copyright (c) 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file framework/hidl_gen/uim_remote_server/vendor/qti/hardware/radio/uim_remote_server/1.0/UimRemoteServiceServerResponseAll.cpp
  DESCRIPTION
  ---------------------------------------------------------------------------
******************************************************************************/
#define LOG_TAG "vendor.qti.hardware.radio.uim_remote_server@1.0::UimRemoteServiceServerResponse"

#undef UNUSED
#include <vendor/qti/hardware/radio/uim_remote_server/1.0/IUimRemoteServiceServerResponse.h>
#include <android/log.h>
#include <cutils/trace.h>
#include "ril_utf_hidl_services.h"


namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace uim_remote_server {
namespace V1_0 {

std::string toString(const ::android::sp<IUimRemoteServiceServerResponse>& o) {
    std::string os = "[class or subclass of ";
    os += IUimRemoteServiceServerResponse::descriptor;
    os += "]";
    os += o->isRemote() ? "@remote" : "@local";
    return os;
}

const char* IUimRemoteServiceServerResponse::descriptor("vendor.qti.hardware.radio.uim_remote_server@1.0::IUimRemoteServiceServerResponse");

// Methods from IUimRemoteServiceServerResponse follow.
// no default implementation for: ::android::hardware::Return<void> IUimRemoteServiceServerResponse::uimRemoteServiceServerConnectResponse(int32_t token, UimRemoteServiceServerConnectRsp sapConnectRsp, int32_t maxMsgSize)
// no default implementation for: ::android::hardware::Return<void> IUimRemoteServiceServerResponse::uimRemoteServiceServerDisconnectResponse(int32_t token)
// no default implementation for: ::android::hardware::Return<void> IUimRemoteServiceServerResponse::uimRemoteServiceServerApduResponse(int32_t token, UimRemoteServiceServerResultCode resultCode, const ::android::hardware::hidl_vec<uint8_t>& apduRsp)
// no default implementation for: ::android::hardware::Return<void> IUimRemoteServiceServerResponse::uimRemoteServiceServerTransferAtrResponse(int32_t token, UimRemoteServiceServerResultCode resultCode, const ::android::hardware::hidl_vec<uint8_t>& atr)
// no default implementation for: ::android::hardware::Return<void> IUimRemoteServiceServerResponse::uimRemoteServiceServerPowerResponse(int32_t token, UimRemoteServiceServerResultCode resultCode)
// no default implementation for: ::android::hardware::Return<void> IUimRemoteServiceServerResponse::uimRemoteServiceServerResetSimResponse(int32_t token, UimRemoteServiceServerResultCode resultCode)
// no default implementation for: ::android::hardware::Return<void> IUimRemoteServiceServerResponse::uimRemoteServiceServerTransferCardReaderStatusResponse(int32_t token, UimRemoteServiceServerResultCode resultCode, int32_t cardReaderStatus)
// no default implementation for: ::android::hardware::Return<void> IUimRemoteServiceServerResponse::uimRemoteServiceServerErrorResponse(int32_t token)
// no default implementation for: ::android::hardware::Return<void> IUimRemoteServiceServerResponse::uimRemoteServiceServerTransferProtocolResponse(int32_t token, UimRemoteServiceServerResultCode resultCode)

// Methods from ::android::hidl::base::V1_0::IBase follow.
::android::hardware::Return<void> IUimRemoteServiceServerResponse::interfaceChain(interfaceChain_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IUimRemoteServiceServerResponse::debug(const ::android::hardware::hidl_handle& fd, const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& options){
    (void)fd;
    (void)options;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IUimRemoteServiceServerResponse::interfaceDescriptor(interfaceDescriptor_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IUimRemoteServiceServerResponse::getHashChain(getHashChain_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IUimRemoteServiceServerResponse::setHALInstrumentation(){
    return ::android::hardware::Void();
}

::android::hardware::Return<bool> IUimRemoteServiceServerResponse::linkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient, uint64_t cookie){
    (void)cookie;
    return (recipient != nullptr);
}

::android::hardware::Return<void> IUimRemoteServiceServerResponse::ping(){
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IUimRemoteServiceServerResponse::getDebugInfo(getDebugInfo_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IUimRemoteServiceServerResponse::notifySyspropsChanged(){
    return ::android::hardware::Void();}

::android::hardware::Return<bool> IUimRemoteServiceServerResponse::unlinkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient){
    return (recipient != nullptr);
}


// static 
::android::hardware::Return<::android::sp<IUimRemoteServiceServerResponse>> IUimRemoteServiceServerResponse::castFrom(const ::android::sp<IUimRemoteServiceServerResponse>& parent, bool /* emitError */) {
    return parent;
}

// static 
::android::hardware::Return<::android::sp<IUimRemoteServiceServerResponse>> IUimRemoteServiceServerResponse::castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError) {
    return nullptr;
}

// static
::android::sp<IUimRemoteServiceServerResponse> IUimRemoteServiceServerResponse::tryGetService(const std::string &serviceName, const bool getStub) {
    return nullptr;
}

// static
::android::sp<IUimRemoteServiceServerResponse> IUimRemoteServiceServerResponse::getService(const std::string &serviceName, const bool getStub) {
    return nullptr;
}

::android::status_t IUimRemoteServiceServerResponse::registerAsService(const std::string &serviceName) {
    return ::android::OK;
}

bool IUimRemoteServiceServerResponse::registerForNotifications(
        const std::string &serviceName,
        const ::android::sp<::android::hidl::manager::V1_0::IServiceNotification> &notification) {
    return true;
}

}  // namespace V1_0
}  // namespace uim_remote_server
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
