/*===========================================================================

   Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
   All Rights Reserved.
   Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

#define LOG_TAG "vendor.qti.hardware.radio.ims@1.5::ImsRadioResponse"

#undef UNUSED

#include <vendor/qti/hardware/radio/ims/1.5/IImsRadioResponse.h>
#include <android/log.h>
#include <cutils/trace.h>
#include "ril_utf_hidl_services.h"

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace ims {
namespace V1_5 {

const char* IImsRadioResponse::descriptor("vendor.qti.hardware.radio.ims@1.5::IImsRadioResponse");

// Methods from ::vendor::qti::hardware::radio::ims::V1_0::IImsRadioResponse follow.
// no default implementation for: ::android::hardware::Return<void> IImsRadioResponse::dialResponse(int32_t token, uint32_t errorCode)
// no default implementation for: ::android::hardware::Return<void> IImsRadioResponse::answerResponse(int32_t token, uint32_t errorCode)
// no default implementation for: ::android::hardware::Return<void> IImsRadioResponse::hangupResponse(int32_t token, uint32_t errorCode)
// no default implementation for: ::android::hardware::Return<void> IImsRadioResponse::requestRegistrationChangeResponse(int32_t token, uint32_t errorCode)
// no default implementation for: ::android::hardware::Return<void> IImsRadioResponse::queryServiceStatusResponse(int32_t token, uint32_t errorCode, const ::android::hardware::hidl_vec<::vendor::qti::hardware::radio::ims::V1_0::ServiceStatusInfo>& srvStatusList)
// no default implementation for: ::android::hardware::Return<void> IImsRadioResponse::setServiceStatusResponse(int32_t token, uint32_t errorCode)
// no default implementation for: ::android::hardware::Return<void> IImsRadioResponse::holdResponse(int32_t token, uint32_t errorCode, const ::vendor::qti::hardware::radio::ims::V1_0::SipErrorInfo& sipError)
// no default implementation for: ::android::hardware::Return<void> IImsRadioResponse::resumeResponse(int32_t token, uint32_t errorCode, const ::vendor::qti::hardware::radio::ims::V1_0::SipErrorInfo& sipError)
// no default implementation for: ::android::hardware::Return<void> IImsRadioResponse::setConfigResponse(int32_t token, uint32_t errorCode, const ::vendor::qti::hardware::radio::ims::V1_0::ConfigInfo& config)
// no default implementation for: ::android::hardware::Return<void> IImsRadioResponse::getConfigResponse(int32_t token, uint32_t errorCode, const ::vendor::qti::hardware::radio::ims::V1_0::ConfigInfo& config)
// no default implementation for: ::android::hardware::Return<void> IImsRadioResponse::getRegistrationResponse(int32_t token, uint32_t errorCode, const ::vendor::qti::hardware::radio::ims::V1_0::RegistrationInfo& registration)
// no default implementation for: ::android::hardware::Return<void> IImsRadioResponse::suppServiceStatusResponse(int32_t token, uint32_t errorCode, const ::vendor::qti::hardware::radio::ims::V1_0::SuppServiceStatus& suppServiceStatus)
// no default implementation for: ::android::hardware::Return<void> IImsRadioResponse::conferenceResponse(int32_t token, uint32_t errorCode, const ::vendor::qti::hardware::radio::ims::V1_0::SipErrorInfo& errorInfo)
// no default implementation for: ::android::hardware::Return<void> IImsRadioResponse::getClipResponse(int32_t token, uint32_t errorCode, const ::vendor::qti::hardware::radio::ims::V1_0::ClipProvisionStatus& clipProvisionStatus)
// no default implementation for: ::android::hardware::Return<void> IImsRadioResponse::getClirResponse(int32_t token, uint32_t errorCode, const ::vendor::qti::hardware::radio::ims::V1_0::ClirInfo& clirInfo, bool hasClirInfo)
// no default implementation for: ::android::hardware::Return<void> IImsRadioResponse::setClirResponse(int32_t token, uint32_t errorCode)
// no default implementation for: ::android::hardware::Return<void> IImsRadioResponse::getColrResponse(int32_t token, uint32_t errorCode, const ::vendor::qti::hardware::radio::ims::V1_0::ColrInfo& colrInfo)
// no default implementation for: ::android::hardware::Return<void> IImsRadioResponse::exitEmergencyCallbackModeResponse(int32_t token, uint32_t errorCode)
// no default implementation for: ::android::hardware::Return<void> IImsRadioResponse::sendDtmfResponse(int32_t token, uint32_t errorCode)
// no default implementation for: ::android::hardware::Return<void> IImsRadioResponse::startDtmfResponse(int32_t token, uint32_t errorCode)
// no default implementation for: ::android::hardware::Return<void> IImsRadioResponse::stopDtmfResponse(int32_t token, uint32_t errorCode)
// no default implementation for: ::android::hardware::Return<void> IImsRadioResponse::setUiTTYModeResponse(int32_t token, uint32_t errorCode)
// no default implementation for: ::android::hardware::Return<void> IImsRadioResponse::modifyCallInitiateResponse(int32_t token, uint32_t errorCode)
// no default implementation for: ::android::hardware::Return<void> IImsRadioResponse::modifyCallConfirmResponse(int32_t token, uint32_t errorCode)
// no default implementation for: ::android::hardware::Return<void> IImsRadioResponse::queryCallForwardStatusResponse(int32_t token, uint32_t errorCode, const ::android::hardware::hidl_vec<::vendor::qti::hardware::radio::ims::V1_0::CallForwardInfo>& callForwardInfoList, const ::vendor::qti::hardware::radio::ims::V1_0::SipErrorInfo& errorDetails)
// no default implementation for: ::android::hardware::Return<void> IImsRadioResponse::getCallWaitingResponse(int32_t token, uint32_t errorCode, ::vendor::qti::hardware::radio::ims::V1_0::ServiceClassStatus serviceStatus, uint32_t serviceClass, const ::vendor::qti::hardware::radio::ims::V1_0::SipErrorInfo& errorDetails)
// no default implementation for: ::android::hardware::Return<void> IImsRadioResponse::explicitCallTransferResponse(int32_t token, uint32_t errorCode)
// no default implementation for: ::android::hardware::Return<void> IImsRadioResponse::setSuppServiceNotificationResponse(int32_t token, uint32_t errorCode, ::vendor::qti::hardware::radio::ims::V1_0::ServiceClassStatus serviceStatus)
// no default implementation for: ::android::hardware::Return<void> IImsRadioResponse::getRtpStatisticsResponse(int32_t token, uint32_t errorCode, uint64_t packetCount)
// no default implementation for: ::android::hardware::Return<void> IImsRadioResponse::getRtpErrorStatisticsResponse(int32_t token, uint32_t errorCode, uint64_t packetErrorCount)
// no default implementation for: ::android::hardware::Return<void> IImsRadioResponse::addParticipantResponse(int32_t token, uint32_t errorCode)
// no default implementation for: ::android::hardware::Return<void> IImsRadioResponse::deflectCallResponse(int32_t token, uint32_t errorCode)
// no default implementation for: ::android::hardware::Return<void> IImsRadioResponse::sendGeolocationInfoResponse(int32_t token, uint32_t errorCode)
// no default implementation for: ::android::hardware::Return<void> IImsRadioResponse::getImsSubConfigResponse(int32_t token, uint32_t errorCode, const ::vendor::qti::hardware::radio::ims::V1_0::ImsSubConfigInfo& subConfigInfo)
// no default implementation for: ::android::hardware::Return<void> IImsRadioResponse::sendRttMessageResponse(int32_t token, uint32_t errorCode)
// no default implementation for: ::android::hardware::Return<void> IImsRadioResponse::cancelModifyCallResponse(int32_t token, uint32_t errorCode)

// Methods from ::vendor::qti::hardware::radio::ims::V1_2::IImsRadioResponse follow.
// no default implementation for: ::android::hardware::Return<void> IImsRadioResponse::sendImsSmsResponse(int32_t token, uint32_t msgeRef, ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendStatusResult smsStatus, ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason reason)

// Methods from ::vendor::qti::hardware::radio::ims::V1_3::IImsRadioResponse follow.
// no default implementation for: ::android::hardware::Return<void> IImsRadioResponse::suppServiceStatusResponse_1_3(int32_t token, uint32_t errorCode, const ::vendor::qti::hardware::radio::ims::V1_3::SuppServiceStatus& suppServiceStatus)
// no default implementation for: ::android::hardware::Return<void> IImsRadioResponse::getColrResponse_1_3(int32_t token, uint32_t errorCode, const ::vendor::qti::hardware::radio::ims::V1_3::ColrInfo& colrInfo)

// Methods from ::vendor::qti::hardware::radio::ims::V1_4::IImsRadioResponse follow.
// no default implementation for: ::android::hardware::Return<void> IImsRadioResponse::registerMultiIdentityLinesResponse(int32_t token, uint32_t errorCode)
// no default implementation for: ::android::hardware::Return<void> IImsRadioResponse::queryVirtualLineInfoResponse(int32_t token, const ::android::hardware::hidl_string& msisdn, const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& virtualLineInfo)

// Methods from ::vendor::qti::hardware::radio::ims::V1_5::IImsRadioResponse follow.
// no default implementation for: ::android::hardware::Return<void> IImsRadioResponse::setCallForwardStatusResponse(int32_t token, uint32_t errorCode, const ::vendor::qti::hardware::radio::ims::V1_5::CallForwardStatusInfo& callForwardStatus)
// no default implementation for: ::android::hardware::Return<void> IImsRadioResponse::sendImsSmsResponse_1_5(int32_t token, uint32_t msgRef, ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendStatusResult smsStatus, ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason reason)

// Methods from ::android::hidl::base::V1_0::IBase follow.
::android::hardware::Return<void> IImsRadioResponse::interfaceChain(interfaceChain_cb _hidl_cb){
    return ::android::hardware::Void();}

::android::hardware::Return<void> IImsRadioResponse::debug(const ::android::hardware::hidl_handle& fd, const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& options){
    (void)fd;
    (void)options;
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IImsRadioResponse::interfaceDescriptor(interfaceDescriptor_cb _hidl_cb){
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IImsRadioResponse::getHashChain(getHashChain_cb _hidl_cb){
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IImsRadioResponse::setHALInstrumentation(){
    return ::android::hardware::Void();
}

::android::hardware::Return<bool> IImsRadioResponse::linkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient, uint64_t cookie){
    (void)cookie;
    return (recipient != nullptr);
}

::android::hardware::Return<void> IImsRadioResponse::ping(){
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IImsRadioResponse::getDebugInfo(getDebugInfo_cb _hidl_cb){
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IImsRadioResponse::notifySyspropsChanged(){
    return ::android::hardware::Void();
}

::android::hardware::Return<bool> IImsRadioResponse::unlinkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient){
    return (recipient != nullptr);
}


::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::ims::V1_5::IImsRadioResponse>> IImsRadioResponse::castFrom(const ::android::sp<::vendor::qti::hardware::radio::ims::V1_5::IImsRadioResponse>& parent, bool /* emitError */) {
    return parent;
}

::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::ims::V1_5::IImsRadioResponse>> IImsRadioResponse::castFrom(const ::android::sp<::vendor::qti::hardware::radio::ims::V1_4::IImsRadioResponse>& parent, bool emitError) {
  return ::android::sp<::vendor::qti::hardware::radio::ims::V1_5::IImsRadioResponse>(static_cast<::vendor::qti::hardware::radio::ims::V1_5::IImsRadioResponse *>(parent.get()));
}

::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::ims::V1_5::IImsRadioResponse>> IImsRadioResponse::castFrom(const ::android::sp<::vendor::qti::hardware::radio::ims::V1_3::IImsRadioResponse>& parent, bool emitError) {
  return ::android::sp<::vendor::qti::hardware::radio::ims::V1_5::IImsRadioResponse>(static_cast<::vendor::qti::hardware::radio::ims::V1_5::IImsRadioResponse *>(parent.get()));
}

::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::ims::V1_5::IImsRadioResponse>> IImsRadioResponse::castFrom(const ::android::sp<::vendor::qti::hardware::radio::ims::V1_2::IImsRadioResponse>& parent, bool emitError) {
  return ::android::sp<::vendor::qti::hardware::radio::ims::V1_5::IImsRadioResponse>(static_cast<::vendor::qti::hardware::radio::ims::V1_5::IImsRadioResponse *>(parent.get()));
}

::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::ims::V1_5::IImsRadioResponse>> IImsRadioResponse::castFrom(const ::android::sp<::vendor::qti::hardware::radio::ims::V1_0::IImsRadioResponse>& parent, bool emitError) {
  return ::android::sp<::vendor::qti::hardware::radio::ims::V1_5::IImsRadioResponse>(static_cast<::vendor::qti::hardware::radio::ims::V1_5::IImsRadioResponse *>(parent.get()));
}


::android::sp<IImsRadioResponse> IImsRadioResponse::tryGetService(const std::string &serviceName, const bool getStub) {
    return nullptr;
}

::android::sp<IImsRadioResponse> IImsRadioResponse::getService(const std::string &serviceName, const bool getStub) {
    return nullptr;
}

::android::status_t IImsRadioResponse::registerAsService(const std::string &serviceName) {
    return ::android::OK;
}

bool IImsRadioResponse::registerForNotifications(
        const std::string &serviceName,
        const ::android::sp<::android::hidl::manager::V1_0::IServiceNotification> &notification) {
    return true;
}

}  // namespace V1_5
}  // namespace ims
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
