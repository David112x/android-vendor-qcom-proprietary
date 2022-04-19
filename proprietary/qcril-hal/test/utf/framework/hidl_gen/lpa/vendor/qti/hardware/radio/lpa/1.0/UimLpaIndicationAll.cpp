/******************************************************************************
#  Copyright (c) 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file framework/hidl_gen/lpa/vendor/qti/hardware/radio/lpa/1.0/UimLpaIndicationAll.cpp
  DESCRIPTION
  ---------------------------------------------------------------------------
******************************************************************************/
#define LOG_TAG "vendor.qti.hardware.radio.lpa@1.0::UimLpaIndication"

#undef UNUSED

#include <vendor/qti/hardware/radio/lpa/1.0/IUimLpaIndication.h>
#include <android/log.h>
#include <cutils/trace.h>
#include "ril_utf_hidl_services.h"

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace lpa {
namespace V1_0 {

std::string toString(const ::android::sp<IUimLpaIndication>& o) {
    std::string os = "[class or subclass of ";
    os += IUimLpaIndication::descriptor;
    os += "]";
    os += o->isRemote() ? "@remote" : "@local";
    return os;
}

const char* IUimLpaIndication::descriptor("vendor.qti.hardware.radio.lpa@1.0::IUimLpaIndication");

// Methods from IUimLpaIndication follow.
// no default implementation for: ::android::hardware::Return<void> IUimLpaIndication::UimLpaHttpTxnIndication(const UimLpaHttpTransactionInd& txnInd)
// no default implementation for: ::android::hardware::Return<void> IUimLpaIndication::UimLpaAddProfileProgressIndication(const UimLpaAddProfileProgressInd& progressInd)

// Methods from ::android::hidl::base::V1_0::IBase follow.
::android::hardware::Return<void> IUimLpaIndication::interfaceChain(interfaceChain_cb _hidl_cb){
    return ::android::hardware::Void();}

::android::hardware::Return<void> IUimLpaIndication::debug(const ::android::hardware::hidl_handle& fd, const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& options){
    (void)fd;
    (void)options;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IUimLpaIndication::interfaceDescriptor(interfaceDescriptor_cb _hidl_cb){
    return ::android::hardware::Void();}

::android::hardware::Return<void> IUimLpaIndication::getHashChain(getHashChain_cb _hidl_cb){
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IUimLpaIndication::setHALInstrumentation(){
    return ::android::hardware::Void();
}

::android::hardware::Return<bool> IUimLpaIndication::linkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient, uint64_t cookie){
    (void)cookie;
    return (recipient != nullptr);
}

::android::hardware::Return<void> IUimLpaIndication::ping(){
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IUimLpaIndication::getDebugInfo(getDebugInfo_cb _hidl_cb){
    return ::android::hardware::Void();}

::android::hardware::Return<void> IUimLpaIndication::notifySyspropsChanged(){
    return ::android::hardware::Void();}

::android::hardware::Return<bool> IUimLpaIndication::unlinkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient){
    return (recipient != nullptr);
}


// static 
::android::hardware::Return<::android::sp<IUimLpaIndication>> IUimLpaIndication::castFrom(const ::android::sp<IUimLpaIndication>& parent, bool /* emitError */) {
    return parent;
}

// static 
::android::hardware::Return<::android::sp<IUimLpaIndication>> IUimLpaIndication::castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError) {
    return nullptr;
}

// static
::android::sp<IUimLpaIndication> IUimLpaIndication::tryGetService(const std::string &serviceName, const bool getStub) {
    return nullptr;
}

// static
::android::sp<IUimLpaIndication> IUimLpaIndication::getService(const std::string &serviceName, const bool getStub) {
    return nullptr;
}

::android::status_t IUimLpaIndication::registerAsService(const std::string &serviceName) {
    return ::android::OK;
}

bool IUimLpaIndication::registerForNotifications(
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
