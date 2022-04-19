/******************************************************************************
#  Copyright (c) 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file framework/hidl_gen/uim_remote_server/vendor/qti/hardware/radio/uim_remote_server/1.0/UimRemoteServiceServerAll.cpp
  DESCRIPTION
  ---------------------------------------------------------------------------
******************************************************************************/
#define LOG_TAG "vendor.qti.hardware.radio.uim_remote_server@1.0::UimRemoteServiceServer"

#undef UNUSED

#include <vendor/qti/hardware/radio/uim_remote_server/1.0/IUimRemoteServiceServer.h>
#include <android/log.h>
#include <cutils/trace.h>
#include "ril_utf_hidl_services.h"

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace uim_remote_server {
namespace V1_0 {

std::string toString(const ::android::sp<IUimRemoteServiceServer>& o) {
    std::string os = "[class or subclass of ";
    os += IUimRemoteServiceServer::descriptor;
    os += "]";
    os += o->isRemote() ? "@remote" : "@local";
    return os;
}

const char* IUimRemoteServiceServer::descriptor("vendor.qti.hardware.radio.uim_remote_server@1.0::IUimRemoteServiceServer");

// Methods from IUimRemoteServiceServer follow.
// no default implementation for: ::android::hardware::Return<void> IUimRemoteServiceServer::setCallback(const ::android::sp<IUimRemoteServiceServerResponse>& responseCallback, const ::android::sp<IUimRemoteServiceServerIndication>& indicationCallback)
// no default implementation for: ::android::hardware::Return<void> IUimRemoteServiceServer::uimRemoteServiceServerConnectReq(int32_t token, int32_t maxMsgSize)
// no default implementation for: ::android::hardware::Return<void> IUimRemoteServiceServer::uimRemoteServiceServerDisconnectReq(int32_t token)
// no default implementation for: ::android::hardware::Return<void> IUimRemoteServiceServer::uimRemoteServiceServerApduReq(int32_t token, UimRemoteServiceServerApduType type, const ::android::hardware::hidl_vec<uint8_t>& command)
// no default implementation for: ::android::hardware::Return<void> IUimRemoteServiceServer::uimRemoteServiceServerTransferAtrReq(int32_t token)
// no default implementation for: ::android::hardware::Return<void> IUimRemoteServiceServer::uimRemoteServiceServerPowerReq(int32_t token, bool state)
// no default implementation for: ::android::hardware::Return<void> IUimRemoteServiceServer::uimRemoteServiceServerResetSimReq(int32_t token)
// no default implementation for: ::android::hardware::Return<void> IUimRemoteServiceServer::uimRemoteServiceServerTransferCardReaderStatusReq(int32_t token)
// no default implementation for: ::android::hardware::Return<void> IUimRemoteServiceServer::uimRemoteServiceServerSetTransferProtocolReq(int32_t token, UimRemoteServiceServerTransferProtocol transferProtocol)

// Methods from ::android::hidl::base::V1_0::IBase follow.
::android::hardware::Return<void> IUimRemoteServiceServer::interfaceChain(interfaceChain_cb _hidl_cb){
    return ::android::hardware::Void();}

::android::hardware::Return<void> IUimRemoteServiceServer::debug(const ::android::hardware::hidl_handle& fd, const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& options){
    (void)fd;
    (void)options;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IUimRemoteServiceServer::interfaceDescriptor(interfaceDescriptor_cb _hidl_cb){
    return ::android::hardware::Void();}

::android::hardware::Return<void> IUimRemoteServiceServer::getHashChain(getHashChain_cb _hidl_cb){
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IUimRemoteServiceServer::setHALInstrumentation(){
    return ::android::hardware::Void();
}

::android::hardware::Return<bool> IUimRemoteServiceServer::linkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient, uint64_t cookie){
    (void)cookie;
    return (recipient != nullptr);
}

::android::hardware::Return<void> IUimRemoteServiceServer::ping(){
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IUimRemoteServiceServer::getDebugInfo(getDebugInfo_cb _hidl_cb){
    return ::android::hardware::Void();}

::android::hardware::Return<void> IUimRemoteServiceServer::notifySyspropsChanged(){
    return ::android::hardware::Void();}

::android::hardware::Return<bool> IUimRemoteServiceServer::unlinkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient){
    return (recipient != nullptr);
}


// static 
::android::hardware::Return<::android::sp<IUimRemoteServiceServer>> IUimRemoteServiceServer::castFrom(const ::android::sp<IUimRemoteServiceServer>& parent, bool /* emitError */) {
    return parent;
}

// static 
::android::hardware::Return<::android::sp<IUimRemoteServiceServer>> IUimRemoteServiceServer::castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError) {
    return nullptr;
}

// static
::android::sp<IUimRemoteServiceServer> IUimRemoteServiceServer::tryGetService(const std::string &serviceName, const bool getStub) {
    return nullptr;
}

// static
::android::sp<IUimRemoteServiceServer> IUimRemoteServiceServer::getService(const std::string &serviceName, const bool getStub) {
    return nullptr;
}

::android::status_t IUimRemoteServiceServer::registerAsService(const std::string &serviceName) {
    return android::OK;
}

bool IUimRemoteServiceServer::registerForNotifications(
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
