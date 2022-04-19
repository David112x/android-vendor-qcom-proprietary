/*
  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#define LOG_TAG "vendor.qti.hardware.radio.ims@1.4::ImsRadio"

#undef UNUSED

#include <vendor/qti/hardware/radio/ims/1.4/IImsRadio.h>
#include <android/log.h>
#include <cutils/trace.h>
#include "ril_utf_hidl_services.h"


namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace ims {
namespace V1_4 {

const char* IImsRadio::descriptor("vendor.qti.hardware.radio.ims@1.4::IImsRadio");

// Methods from ::vendor::qti::hardware::radio::ims::V1_0::IImsRadio follow.
// no default implementation for: ::android::hardware::Return<void> IImsRadio::setCallback(const ::android::sp<::vendor::qti::hardware::radio::ims::V1_0::IImsRadioResponse>& imsRadioResponse, const ::android::sp<::vendor::qti::hardware::radio::ims::V1_0::IImsRadioIndication>& imsRadioIndication)
// no default implementation for: ::android::hardware::Return<void> IImsRadio::dial(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_0::DialRequest& dialRequest)
// no default implementation for: ::android::hardware::Return<void> IImsRadio::addParticipant(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_0::DialRequest& dialRequest)
// no default implementation for: ::android::hardware::Return<void> IImsRadio::getImsRegistrationState(int32_t token)
// no default implementation for: ::android::hardware::Return<void> IImsRadio::answer(int32_t token, ::vendor::qti::hardware::radio::ims::V1_0::CallType callType, ::vendor::qti::hardware::radio::ims::V1_0::IpPresentation presentation, ::vendor::qti::hardware::radio::ims::V1_0::RttMode mode)
// no default implementation for: ::android::hardware::Return<void> IImsRadio::hangup(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_0::HangupRequestInfo& hangup)
// no default implementation for: ::android::hardware::Return<void> IImsRadio::requestRegistrationChange(int32_t token, ::vendor::qti::hardware::radio::ims::V1_0::RegState state)
// no default implementation for: ::android::hardware::Return<void> IImsRadio::queryServiceStatus(int32_t token)
// no default implementation for: ::android::hardware::Return<void> IImsRadio::setServiceStatus(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_0::ServiceStatusInfo& srvStatusInfo)
// no default implementation for: ::android::hardware::Return<void> IImsRadio::hold(int32_t token, uint32_t callId)
// no default implementation for: ::android::hardware::Return<void> IImsRadio::resume(int32_t token, uint32_t callId)
// no default implementation for: ::android::hardware::Return<void> IImsRadio::setConfig(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_0::ConfigInfo& config)
// no default implementation for: ::android::hardware::Return<void> IImsRadio::getConfig(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_0::ConfigInfo& config)
// no default implementation for: ::android::hardware::Return<void> IImsRadio::conference(int32_t token)
// no default implementation for: ::android::hardware::Return<void> IImsRadio::getClip(int32_t token)
// no default implementation for: ::android::hardware::Return<void> IImsRadio::getClir(int32_t token)
// no default implementation for: ::android::hardware::Return<void> IImsRadio::setClir(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_0::ClirInfo& clirInfo)
// no default implementation for: ::android::hardware::Return<void> IImsRadio::getColr(int32_t token)
// no default implementation for: ::android::hardware::Return<void> IImsRadio::setColr(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_0::ColrInfo& colrInfo)
// no default implementation for: ::android::hardware::Return<void> IImsRadio::exitEmergencyCallbackMode(int32_t token)
// no default implementation for: ::android::hardware::Return<void> IImsRadio::sendDtmf(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_0::DtmfInfo& dtmfInfo)
// no default implementation for: ::android::hardware::Return<void> IImsRadio::startDtmf(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_0::DtmfInfo& dtmfInfo)
// no default implementation for: ::android::hardware::Return<void> IImsRadio::stopDtmf(int32_t token)
// no default implementation for: ::android::hardware::Return<void> IImsRadio::setUiTtyMode(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_0::TtyInfo& ttyInfo)
// no default implementation for: ::android::hardware::Return<void> IImsRadio::modifyCallInitiate(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_0::CallModifyInfo& callModifyInfo)
// no default implementation for: ::android::hardware::Return<void> IImsRadio::modifyCallConfirm(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_0::CallModifyInfo& callModifyInfo)
// no default implementation for: ::android::hardware::Return<void> IImsRadio::queryCallForwardStatus(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_0::CallForwardInfo& callForwardInfo)
// no default implementation for: ::android::hardware::Return<void> IImsRadio::setCallForwardStatus(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_0::CallForwardInfo& callForwardInfo)
// no default implementation for: ::android::hardware::Return<void> IImsRadio::getCallWaiting(int32_t token, uint32_t serviceClass)
// no default implementation for: ::android::hardware::Return<void> IImsRadio::setCallWaiting(int32_t token, ::vendor::qti::hardware::radio::ims::V1_0::ServiceClassStatus serviceStatus, uint32_t serviceClass)
// no default implementation for: ::android::hardware::Return<void> IImsRadio::setSuppServiceNotification(int32_t token, ::vendor::qti::hardware::radio::ims::V1_0::ServiceClassStatus status)
// no default implementation for: ::android::hardware::Return<void> IImsRadio::explicitCallTransfer(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_0::ExplicitCallTransferInfo& ectInfo)
// no default implementation for: ::android::hardware::Return<void> IImsRadio::suppServiceStatus(int32_t token, int32_t operationType, ::vendor::qti::hardware::radio::ims::V1_0::FacilityType facilityType, const ::vendor::qti::hardware::radio::ims::V1_0::CbNumListInfo& cbNumListInfo)
// no default implementation for: ::android::hardware::Return<void> IImsRadio::getRtpStatistics(int32_t token)
// no default implementation for: ::android::hardware::Return<void> IImsRadio::getRtpErrorStatistics(int32_t token)
// no default implementation for: ::android::hardware::Return<void> IImsRadio::deflectCall(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_0::DeflectRequestInfo& deflectRequestInfo)
// no default implementation for: ::android::hardware::Return<void> IImsRadio::sendGeolocationInfo(int32_t token, double lat, double lon, const ::vendor::qti::hardware::radio::ims::V1_0::AddressInfo& addressInfo)
// no default implementation for: ::android::hardware::Return<void> IImsRadio::getImsSubConfig(int32_t token)
// no default implementation for: ::android::hardware::Return<void> IImsRadio::sendRttMessage(int32_t token, const ::android::hardware::hidl_string& message)
// no default implementation for: ::android::hardware::Return<void> IImsRadio::cancelModifyCall(int32_t token, uint32_t callId)

// Methods from ::vendor::qti::hardware::radio::ims::V1_1::IImsRadio follow.
// no default implementation for: ::android::hardware::Return<void> IImsRadio::hangup_1_1(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_1::HangupRequestInfo& hangup)

// Methods from ::vendor::qti::hardware::radio::ims::V1_2::IImsRadio follow.
// no default implementation for: ::android::hardware::Return<void> IImsRadio::hangup_1_2(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_2::HangupRequestInfo& hangup)
// no default implementation for: ::android::hardware::Return<void> IImsRadio::sendImsSms(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsMessage& imsSms)
// no default implementation for: ::android::hardware::Return<void> IImsRadio::acknowledgeSms(int32_t token, uint32_t messageRef, ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsDeliverStatusResult smsResult)
// no default implementation for: ::android::hardware::Return<void> IImsRadio::acknowledgeSmsReport(int32_t token, uint32_t messageRef, ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsStatusReportResult smsReport)
// no default implementation for: ::android::hardware::Return<void> IImsRadio::getSmsFormat(getSmsFormat_cb _hidl_cb)
// no default implementation for: ::android::hardware::Return<void> IImsRadio::sendGeolocationInfo_1_2(int32_t token, double lat, double lon, const ::vendor::qti::hardware::radio::ims::V1_2::AddressInfo& addressInfo)

// Methods from ::vendor::qti::hardware::radio::ims::V1_3::IImsRadio follow.
// no default implementation for: ::android::hardware::Return<void> IImsRadio::hangup_1_3(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_3::HangupRequestInfo& hangup)
// no default implementation for: ::android::hardware::Return<void> IImsRadio::setColr_1_3(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_3::ColrInfo& colrInfo)

// Methods from ::vendor::qti::hardware::radio::ims::V1_4::IImsRadio follow.
// no default implementation for: ::android::hardware::Return<void> IImsRadio::registerMultiIdentityLines(int32_t token, const ::android::hardware::hidl_vec<::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityLineInfoHal>& info)
// no default implementation for: ::android::hardware::Return<void> IImsRadio::queryVirtualLineInfo(int32_t token, const ::android::hardware::hidl_string& msisdn)
// no default implementation for: ::android::hardware::Return<void> IImsRadio::dial_1_4(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_4::DialRequest& dialRequest)
// no default implementation for: ::android::hardware::Return<void> IImsRadio::addParticipant_1_4(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_4::DialRequest& dialRequest)

// Methods from ::android::hidl::base::V1_0::IBase follow.
::android::hardware::Return<void> IImsRadio::interfaceChain(interfaceChain_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IImsRadio::debug(const ::android::hardware::hidl_handle& fd, const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& options){
    (void)fd;
    (void)options;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IImsRadio::interfaceDescriptor(interfaceDescriptor_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IImsRadio::getHashChain(getHashChain_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IImsRadio::setHALInstrumentation(){
    return ::android::hardware::Void();
}

::android::hardware::Return<bool> IImsRadio::linkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient, uint64_t cookie){
    (void)cookie;
    return (recipient != nullptr);
}

::android::hardware::Return<void> IImsRadio::ping(){
    return ::android::hardware::Void();
}


::android::hardware::Return<void> IImsRadio::notifySyspropsChanged(){
    return ::android::hardware::Void();}


::android::hardware::Return<void> IImsRadio::getDebugInfo(getDebugInfo_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();
}

::android::hardware::Return<bool> IImsRadio::unlinkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient){
    return (recipient != nullptr);
}


// static 
::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::ims::V1_4::IImsRadio>> IImsRadio::castFrom(const ::android::sp<::vendor::qti::hardware::radio::ims::V1_4::IImsRadio>& parent, bool /* emitError */) {
  return ::android::sp<::vendor::qti::hardware::radio::ims::V1_4::IImsRadio>(static_cast<::vendor::qti::hardware::radio::ims::V1_4::IImsRadio *>(parent.get()));
}

// static 
::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::ims::V1_4::IImsRadio>> IImsRadio::castFrom(const ::android::sp<::vendor::qti::hardware::radio::ims::V1_3::IImsRadio>& parent, bool emitError) {
  return ::android::sp<::vendor::qti::hardware::radio::ims::V1_4::IImsRadio>(static_cast<::vendor::qti::hardware::radio::ims::V1_4::IImsRadio *>(parent.get()));
}

// static 
::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::ims::V1_4::IImsRadio>> IImsRadio::castFrom(const ::android::sp<::vendor::qti::hardware::radio::ims::V1_2::IImsRadio>& parent, bool emitError) {
  return ::android::sp<::vendor::qti::hardware::radio::ims::V1_4::IImsRadio>(static_cast<::vendor::qti::hardware::radio::ims::V1_4::IImsRadio *>(parent.get()));
}

// static 
::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::ims::V1_4::IImsRadio>> IImsRadio::castFrom(const ::android::sp<::vendor::qti::hardware::radio::ims::V1_1::IImsRadio>& parent, bool emitError) {
  return ::android::sp<::vendor::qti::hardware::radio::ims::V1_4::IImsRadio>(static_cast<::vendor::qti::hardware::radio::ims::V1_4::IImsRadio *>(parent.get()));
}

// static 
::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::ims::V1_4::IImsRadio>> IImsRadio::castFrom(const ::android::sp<::vendor::qti::hardware::radio::ims::V1_0::IImsRadio>& parent, bool emitError) {
  return ::android::sp<::vendor::qti::hardware::radio::ims::V1_4::IImsRadio>(static_cast<::vendor::qti::hardware::radio::ims::V1_4::IImsRadio *>(parent.get()));
}

// static 
::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::ims::V1_4::IImsRadio>> IImsRadio::castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError) {
    return nullptr;
}

static std::unordered_map<std::string, ::android::sp<::vendor::qti::hardware::radio::ims::V1_4::IImsRadio>> mHidlServices;
// static
::android::sp<IImsRadio> IImsRadio::tryGetService(const std::string &serviceName, const bool getStub) {
    return mHidlServices[serviceName];
}

// static
::android::sp<IImsRadio> IImsRadio::getService(const std::string &serviceName, const bool getStub) {
    return mHidlServices[serviceName];
}

::android::status_t IImsRadio::registerAsService(const std::string &serviceName) {
    mHidlServices[serviceName] = this;
    ::vendor::qti::hardware::radio::ims::V1_3::IImsRadio::registerAsService(serviceName);
    return ::android::OK;
}

bool IImsRadio::registerForNotifications(
        const std::string &serviceName,
        const ::android::sp<::android::hidl::manager::V1_0::IServiceNotification> &notification) {
    return true;
}

}  // namespace V1_4
}  // namespace ims
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
