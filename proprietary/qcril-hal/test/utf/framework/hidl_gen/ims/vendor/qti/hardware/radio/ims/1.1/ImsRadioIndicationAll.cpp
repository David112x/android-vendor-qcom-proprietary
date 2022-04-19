/******************************************************************************
  @file framework/hidl_gen/ims/vendor/qti/hardware/radio/ims/1.1/ImsRadioIndicationAll.cpp
  DESCRIPTION
  ---------------------------------------------------------------------------
  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
******************************************************************************/
#define LOG_TAG "vendor.qti.hardware.radio.ims@1.1::ImsRadioIndication"

#define LOG_TAG "vendor.qti.hardware.radio.ims@1.1::ImsRadio"

#undef UNUSED

#include <vendor/qti/hardware/radio/ims/1.1/IImsRadioIndication.h>
#include <android/log.h>
#include <cutils/trace.h>
#include "ril_utf_hidl_services.h"

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace ims {
namespace V1_1 {

std::string toString(const ::android::sp<::vendor::qti::hardware::radio::ims::V1_1::IImsRadioIndication>& o) {
    std::string os = "[class or subclass of ";
    os += ::vendor::qti::hardware::radio::ims::V1_1::IImsRadioIndication::descriptor;
    os += "]";
    os += o->isRemote() ? "@remote" : "@local";
    return os;
}

const char* IImsRadioIndication::descriptor("vendor.qti.hardware.radio.ims@1.1::IImsRadioIndication");

// Methods from ::vendor::qti::hardware::radio::ims::V1_0::IImsRadioIndication follow.
// no default implementation for: ::android::hardware::Return<void> IImsRadioIndication::onCallStateChanged(const ::android::hardware::hidl_vec<::vendor::qti::hardware::radio::ims::V1_0::CallInfo>& callList)
// no default implementation for: ::android::hardware::Return<void> IImsRadioIndication::onRing()
// no default implementation for: ::android::hardware::Return<void> IImsRadioIndication::onRingbackTone(::vendor::qti::hardware::radio::ims::V1_0::ToneOperation tone)
// no default implementation for: ::android::hardware::Return<void> IImsRadioIndication::onRegistrationChanged(const ::vendor::qti::hardware::radio::ims::V1_0::RegistrationInfo& registration)
// no default implementation for: ::android::hardware::Return<void> IImsRadioIndication::onHandover(const ::vendor::qti::hardware::radio::ims::V1_0::HandoverInfo& handover)
// no default implementation for: ::android::hardware::Return<void> IImsRadioIndication::onServiceStatusChanged(const ::android::hardware::hidl_vec<::vendor::qti::hardware::radio::ims::V1_0::ServiceStatusInfo>& srvStatusList)
// no default implementation for: ::android::hardware::Return<void> IImsRadioIndication::onRadioStateChanged(::vendor::qti::hardware::radio::ims::V1_0::RadioState radioState)
// no default implementation for: ::android::hardware::Return<void> IImsRadioIndication::onEnterEmergencyCallBackMode()
// no default implementation for: ::android::hardware::Return<void> IImsRadioIndication::onExitEmergencyCallBackMode()
// no default implementation for: ::android::hardware::Return<void> IImsRadioIndication::onTtyNotification(const ::vendor::qti::hardware::radio::ims::V1_0::TtyInfo& ttyInfo)
// no default implementation for: ::android::hardware::Return<void> IImsRadioIndication::onRefreshConferenceInfo(const ::vendor::qti::hardware::radio::ims::V1_0::ConferenceInfo& conferenceInfo)
// no default implementation for: ::android::hardware::Return<void> IImsRadioIndication::onRefreshViceInfo(const ::vendor::qti::hardware::radio::ims::V1_0::ViceInfo& viceInfo)
// no default implementation for: ::android::hardware::Return<void> IImsRadioIndication::onModifyCall(const ::vendor::qti::hardware::radio::ims::V1_0::CallModifyInfo& callModifyInfo)
// no default implementation for: ::android::hardware::Return<void> IImsRadioIndication::onSuppServiceNotification(const ::vendor::qti::hardware::radio::ims::V1_0::SuppServiceNotification& suppServiceNotification)
// no default implementation for: ::android::hardware::Return<void> IImsRadioIndication::onMessageWaiting(const ::vendor::qti::hardware::radio::ims::V1_0::MessageWaitingIndication& messageWaitingIndication)
// no default implementation for: ::android::hardware::Return<void> IImsRadioIndication::onGeolocationInfoRequested(double lat, double lon)
// no default implementation for: ::android::hardware::Return<void> IImsRadioIndication::onImsSubConfigChanged(const ::vendor::qti::hardware::radio::ims::V1_0::ImsSubConfigInfo& config)
// no default implementation for: ::android::hardware::Return<void> IImsRadioIndication::onParticipantStatusInfo(const ::vendor::qti::hardware::radio::ims::V1_0::ParticipantStatusInfo& participantStatusInfo)
// no default implementation for: ::android::hardware::Return<void> IImsRadioIndication::onRegistrationBlockStatus(bool hasBlockStatusOnWwan, const ::vendor::qti::hardware::radio::ims::V1_0::BlockStatus& blockStatusOnWwan, bool hasBlockStatusOnWlan, const ::vendor::qti::hardware::radio::ims::V1_0::BlockStatus& blockStatusOnWlan)
// no default implementation for: ::android::hardware::Return<void> IImsRadioIndication::onRttMessageReceived(const ::android::hardware::hidl_string& message)
// no default implementation for: ::android::hardware::Return<void> IImsRadioIndication::onVoWiFiCallQuality(::vendor::qti::hardware::radio::ims::V1_0::VoWiFiCallQualityInfo voWiFiCallQualityInfo)
// no default implementation for: ::android::hardware::Return<void> IImsRadioIndication::onSupplementaryServiceIndication(const ::vendor::qti::hardware::radio::ims::V1_0::StkCcUnsolSsResult& ss)

// Methods from ::vendor::qti::hardware::radio::ims::V1_1::IImsRadioIndication follow.
// no default implementation for: ::android::hardware::Return<void> IImsRadioIndication::onCallStateChanged_1_1(const ::android::hardware::hidl_vec<::vendor::qti::hardware::radio::ims::V1_1::CallInfo>& callList)

// Methods from ::android::hidl::base::V1_0::IBase follow.
::android::hardware::Return<void> IImsRadioIndication::interfaceChain(interfaceChain_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IImsRadioIndication::debug(const ::android::hardware::hidl_handle& fd, const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& options){
    (void)fd;
    (void)options;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IImsRadioIndication::interfaceDescriptor(interfaceDescriptor_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IImsRadioIndication::getHashChain(getHashChain_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IImsRadioIndication::setHALInstrumentation(){
    return ::android::hardware::Void();
}

::android::hardware::Return<bool> IImsRadioIndication::linkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient, uint64_t cookie){
    (void)cookie;
    return (recipient != nullptr);
}

::android::hardware::Return<void> IImsRadioIndication::ping(){
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IImsRadioIndication::getDebugInfo(getDebugInfo_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IImsRadioIndication::notifySyspropsChanged(){
    return ::android::hardware::Void();}

::android::hardware::Return<bool> IImsRadioIndication::unlinkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient){
    return (recipient != nullptr);
}


// static 
::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::ims::V1_1::IImsRadioIndication>> IImsRadioIndication::castFrom(const ::android::sp<::vendor::qti::hardware::radio::ims::V1_1::IImsRadioIndication>& parent, bool /* emitError */) {
    return parent;
}

// static 
::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::ims::V1_1::IImsRadioIndication>> IImsRadioIndication::castFrom(const ::android::sp<::vendor::qti::hardware::radio::ims::V1_0::IImsRadioIndication>& parent, bool emitError) {
  return ::android::sp<::vendor::qti::hardware::radio::ims::V1_1::IImsRadioIndication>(static_cast<::vendor::qti::hardware::radio::ims::V1_1::IImsRadioIndication *>(parent.get()));
}

// static 
::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::ims::V1_1::IImsRadioIndication>> IImsRadioIndication::castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError) {
  return ::android::sp<::vendor::qti::hardware::radio::ims::V1_1::IImsRadioIndication>(static_cast<::vendor::qti::hardware::radio::ims::V1_1::IImsRadioIndication *>(parent.get()));
}

// static
::android::sp<IImsRadioIndication> IImsRadioIndication::tryGetService(const std::string &serviceName, const bool getStub) {
    return nullptr;
}

// static
::android::sp<IImsRadioIndication> IImsRadioIndication::getService(const std::string &serviceName, const bool getStub) {
    return nullptr;
}

::android::status_t IImsRadioIndication::registerAsService(const std::string &serviceName) {
    return ::android::OK;
}

bool IImsRadioIndication::registerForNotifications(
        const std::string &serviceName,
        const ::android::sp<::android::hidl::manager::V1_0::IServiceNotification> &notification) {
    return true;
}

}  // namespace V1_1
}  // namespace ims
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
