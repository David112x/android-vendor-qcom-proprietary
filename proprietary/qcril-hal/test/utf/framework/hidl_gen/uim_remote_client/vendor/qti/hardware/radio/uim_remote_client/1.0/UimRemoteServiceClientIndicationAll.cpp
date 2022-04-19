/******************************************************************************
#  Copyright (c) 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file framework/hidl_gen/uim_remote_client/vendor/qti/hardware/radio/uim_remote_client/1.0/UimRemoteServiceClientIndicationAll.cpp
  DESCRIPTION
  ---------------------------------------------------------------------------
******************************************************************************/
#define LOG_TAG "vendor.qti.hardware.radio.uim_remote_client@1.0::UimRemoteServiceClientIndication"

#undef UNUSED

#include <vendor/qti/hardware/radio/uim_remote_client/1.0/IUimRemoteServiceClientIndication.h>
#include <android/log.h>
#include <cutils/trace.h>
#include "ril_utf_hidl_services.h"

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace uim_remote_client {
namespace V1_0 {

std::string toString(const ::android::sp<IUimRemoteServiceClientIndication>& o) {
    std::string os = "[class or subclass of ";
    os += IUimRemoteServiceClientIndication::descriptor;
    os += "]";
    os += o->isRemote() ? "@remote" : "@local";
    return os;
}

const char* IUimRemoteServiceClientIndication::descriptor("vendor.qti.hardware.radio.uim_remote_client@1.0::IUimRemoteServiceClientIndication");

// Methods from IUimRemoteServiceClientIndication follow.
// no default implementation for: ::android::hardware::Return<void> IUimRemoteServiceClientIndication::UimRemoteServiceClientApduInd(const ::android::hardware::hidl_vec<uint8_t>& apduInd)
// no default implementation for: ::android::hardware::Return<void> IUimRemoteServiceClientIndication::UimRemoteServiceClientConnectInd()
// no default implementation for: ::android::hardware::Return<void> IUimRemoteServiceClientIndication::UimRemoteServiceClientDisconnectInd()
// no default implementation for: ::android::hardware::Return<void> IUimRemoteServiceClientIndication::UimRemoteServiceClientPowerUpInd(bool hasTimeOut, int32_t timeOut, bool hasVoltageClass, UimRemoteClientVoltageClass powerUpVoltageClass)
// no default implementation for: ::android::hardware::Return<void> IUimRemoteServiceClientIndication::UimRemoteServiceClientPowerDownInd(bool hasPowerDownMode, UimRemoteClientPowerDownMode powerDownMode)
// no default implementation for: ::android::hardware::Return<void> IUimRemoteServiceClientIndication::UimRemoteServiceClientResetInd()

// Methods from ::android::hidl::base::V1_0::IBase follow.
::android::hardware::Return<void> IUimRemoteServiceClientIndication::interfaceChain(interfaceChain_cb _hidl_cb){
    return ::android::hardware::Void();}

::android::hardware::Return<void> IUimRemoteServiceClientIndication::debug(const ::android::hardware::hidl_handle& fd, const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& options){
    (void)fd;
    (void)options;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IUimRemoteServiceClientIndication::interfaceDescriptor(interfaceDescriptor_cb _hidl_cb){
    return ::android::hardware::Void();}

::android::hardware::Return<void> IUimRemoteServiceClientIndication::getHashChain(getHashChain_cb _hidl_cb){
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IUimRemoteServiceClientIndication::setHALInstrumentation(){
    return ::android::hardware::Void();
}

::android::hardware::Return<bool> IUimRemoteServiceClientIndication::linkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient, uint64_t cookie){
    (void)cookie;
    return (recipient != nullptr);
}

::android::hardware::Return<void> IUimRemoteServiceClientIndication::ping(){
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IUimRemoteServiceClientIndication::getDebugInfo(getDebugInfo_cb _hidl_cb){
    return ::android::hardware::Void();}

::android::hardware::Return<void> IUimRemoteServiceClientIndication::notifySyspropsChanged(){
    return ::android::hardware::Void();}

::android::hardware::Return<bool> IUimRemoteServiceClientIndication::unlinkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient){
    return (recipient != nullptr);
}


// static 
::android::hardware::Return<::android::sp<IUimRemoteServiceClientIndication>> IUimRemoteServiceClientIndication::castFrom(const ::android::sp<IUimRemoteServiceClientIndication>& parent, bool /* emitError */) {
    return parent;
}

// static 
::android::hardware::Return<::android::sp<IUimRemoteServiceClientIndication>> IUimRemoteServiceClientIndication::castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError) {
    return nullptr;
}

// static
::android::sp<IUimRemoteServiceClientIndication> IUimRemoteServiceClientIndication::tryGetService(const std::string &serviceName, const bool getStub) {
    return nullptr;
}

// static
::android::sp<IUimRemoteServiceClientIndication> IUimRemoteServiceClientIndication::getService(const std::string &serviceName, const bool getStub) {
    return nullptr;
}

::android::status_t IUimRemoteServiceClientIndication::registerAsService(const std::string &serviceName) {
    return ::android::OK;
}

bool IUimRemoteServiceClientIndication::registerForNotifications(
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
