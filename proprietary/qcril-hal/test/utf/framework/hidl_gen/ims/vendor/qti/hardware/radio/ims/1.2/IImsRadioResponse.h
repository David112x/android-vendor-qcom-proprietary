/******************************************************************************
  @file framework/hidl_gen/ims/vendor/qti/hardware/radio/ims/1.2/IImsRadioResponse.h
  DESCRIPTION
  ---------------------------------------------------------------------------
  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
******************************************************************************/
#ifndef HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_IMS_V1_2_IIMSRADIORESPONSE_H
#define HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_IMS_V1_2_IIMSRADIORESPONSE_H

#include <vendor/qti/hardware/radio/ims/1.0/IImsRadioResponse.h>
#include <vendor/qti/hardware/radio/ims/1.2/types.h>

#include <android/hidl/manager/1.0/IServiceNotification.h>

#include <hidl/HidlSupport.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <utils/NativeHandle.h>
#include <utils/misc.h>

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace ims {
namespace V1_2 {

struct IImsRadioResponse : public ::vendor::qti::hardware::radio::ims::V1_0::IImsRadioResponse {
    typedef android::hardware::details::i_tag _hidl_tag;

    // Forward declaration for forward reference support:

    virtual bool isRemote() const override { return false; }


    virtual ::android::hardware::Return<void> dialResponse(int32_t token, uint32_t errorCode) = 0;

    virtual ::android::hardware::Return<void> answerResponse(int32_t token, uint32_t errorCode) = 0;

    virtual ::android::hardware::Return<void> hangupResponse(int32_t token, uint32_t errorCode) = 0;

    virtual ::android::hardware::Return<void> requestRegistrationChangeResponse(int32_t token, uint32_t errorCode) = 0;

    virtual ::android::hardware::Return<void> queryServiceStatusResponse(int32_t token, uint32_t errorCode, const ::android::hardware::hidl_vec<::vendor::qti::hardware::radio::ims::V1_0::ServiceStatusInfo>& srvStatusList) = 0;

    virtual ::android::hardware::Return<void> setServiceStatusResponse(int32_t token, uint32_t errorCode) = 0;

    virtual ::android::hardware::Return<void> holdResponse(int32_t token, uint32_t errorCode, const ::vendor::qti::hardware::radio::ims::V1_0::SipErrorInfo& sipError) = 0;

    virtual ::android::hardware::Return<void> resumeResponse(int32_t token, uint32_t errorCode, const ::vendor::qti::hardware::radio::ims::V1_0::SipErrorInfo& sipError) = 0;

    virtual ::android::hardware::Return<void> setConfigResponse(int32_t token, uint32_t errorCode, const ::vendor::qti::hardware::radio::ims::V1_0::ConfigInfo& config) = 0;

    virtual ::android::hardware::Return<void> getConfigResponse(int32_t token, uint32_t errorCode, const ::vendor::qti::hardware::radio::ims::V1_0::ConfigInfo& config) = 0;

    virtual ::android::hardware::Return<void> getRegistrationResponse(int32_t token, uint32_t errorCode, const ::vendor::qti::hardware::radio::ims::V1_0::RegistrationInfo& registration) = 0;

    virtual ::android::hardware::Return<void> suppServiceStatusResponse(int32_t token, uint32_t errorCode, const ::vendor::qti::hardware::radio::ims::V1_0::SuppServiceStatus& suppServiceStatus) = 0;

    virtual ::android::hardware::Return<void> conferenceResponse(int32_t token, uint32_t errorCode, const ::vendor::qti::hardware::radio::ims::V1_0::SipErrorInfo& errorInfo) = 0;

    virtual ::android::hardware::Return<void> getClipResponse(int32_t token, uint32_t errorCode, const ::vendor::qti::hardware::radio::ims::V1_0::ClipProvisionStatus& clipProvisionStatus) = 0;

    virtual ::android::hardware::Return<void> getClirResponse(int32_t token, uint32_t errorCode, const ::vendor::qti::hardware::radio::ims::V1_0::ClirInfo& clirInfo, bool hasClirInfo) = 0;

    virtual ::android::hardware::Return<void> setClirResponse(int32_t token, uint32_t errorCode) = 0;

    virtual ::android::hardware::Return<void> getColrResponse(int32_t token, uint32_t errorCode, const ::vendor::qti::hardware::radio::ims::V1_0::ColrInfo& colrInfo) = 0;

    virtual ::android::hardware::Return<void> exitEmergencyCallbackModeResponse(int32_t token, uint32_t errorCode) = 0;

    virtual ::android::hardware::Return<void> sendDtmfResponse(int32_t token, uint32_t errorCode) = 0;

    virtual ::android::hardware::Return<void> startDtmfResponse(int32_t token, uint32_t errorCode) = 0;

    virtual ::android::hardware::Return<void> stopDtmfResponse(int32_t token, uint32_t errorCode) = 0;

    virtual ::android::hardware::Return<void> setUiTTYModeResponse(int32_t token, uint32_t errorCode) = 0;

    virtual ::android::hardware::Return<void> modifyCallInitiateResponse(int32_t token, uint32_t errorCode) = 0;

    virtual ::android::hardware::Return<void> modifyCallConfirmResponse(int32_t token, uint32_t errorCode) = 0;

    virtual ::android::hardware::Return<void> queryCallForwardStatusResponse(int32_t token, uint32_t errorCode, const ::android::hardware::hidl_vec<::vendor::qti::hardware::radio::ims::V1_0::CallForwardInfo>& callForwardInfoList, const ::vendor::qti::hardware::radio::ims::V1_0::SipErrorInfo& errorDetails) = 0;

    virtual ::android::hardware::Return<void> getCallWaitingResponse(int32_t token, uint32_t errorCode, ::vendor::qti::hardware::radio::ims::V1_0::ServiceClassStatus serviceStatus, uint32_t serviceClass, const ::vendor::qti::hardware::radio::ims::V1_0::SipErrorInfo& errorDetails) = 0;

    virtual ::android::hardware::Return<void> explicitCallTransferResponse(int32_t token, uint32_t errorCode) = 0;

    virtual ::android::hardware::Return<void> setSuppServiceNotificationResponse(int32_t token, uint32_t errorCode, ::vendor::qti::hardware::radio::ims::V1_0::ServiceClassStatus serviceStatus) = 0;

    virtual ::android::hardware::Return<void> getRtpStatisticsResponse(int32_t token, uint32_t errorCode, uint64_t packetCount) = 0;

    virtual ::android::hardware::Return<void> getRtpErrorStatisticsResponse(int32_t token, uint32_t errorCode, uint64_t packetErrorCount) = 0;

    virtual ::android::hardware::Return<void> addParticipantResponse(int32_t token, uint32_t errorCode) = 0;

    virtual ::android::hardware::Return<void> deflectCallResponse(int32_t token, uint32_t errorCode) = 0;

    virtual ::android::hardware::Return<void> sendGeolocationInfoResponse(int32_t token, uint32_t errorCode) = 0;

    virtual ::android::hardware::Return<void> getImsSubConfigResponse(int32_t token, uint32_t errorCode, const ::vendor::qti::hardware::radio::ims::V1_0::ImsSubConfigInfo& subConfigInfo) = 0;

    virtual ::android::hardware::Return<void> sendRttMessageResponse(int32_t token, uint32_t errorCode) = 0;

    virtual ::android::hardware::Return<void> cancelModifyCallResponse(int32_t token, uint32_t errorCode) = 0;

    virtual ::android::hardware::Return<void> sendImsSmsResponse(int32_t token, uint32_t msgeRef, ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendStatusResult smsStatus, ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason reason) = 0;

    using interfaceChain_cb = std::function<void(const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& descriptors)>;
    virtual ::android::hardware::Return<void> interfaceChain(interfaceChain_cb _hidl_cb) override;

    virtual ::android::hardware::Return<void> debug(const ::android::hardware::hidl_handle& fd, const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& options) override;

    using interfaceDescriptor_cb = std::function<void(const ::android::hardware::hidl_string& descriptor)>;
    virtual ::android::hardware::Return<void> interfaceDescriptor(interfaceDescriptor_cb _hidl_cb) override;

    using getHashChain_cb = std::function<void(const ::android::hardware::hidl_vec<::android::hardware::hidl_array<uint8_t, 32>>& hashchain)>;
    virtual ::android::hardware::Return<void> getHashChain(getHashChain_cb _hidl_cb) override;

    virtual ::android::hardware::Return<void> setHALInstrumentation() override;

    virtual ::android::hardware::Return<bool> linkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient, uint64_t cookie) override;

    virtual ::android::hardware::Return<void> ping() override;

    using getDebugInfo_cb = std::function<void(const ::android::hidl::base::V1_0::DebugInfo& info)>;
    virtual ::android::hardware::Return<void> getDebugInfo(getDebugInfo_cb _hidl_cb) override;

    virtual ::android::hardware::Return<void> notifySyspropsChanged() override;

    virtual ::android::hardware::Return<bool> unlinkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient) override;
    // cast static functions
    static ::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::ims::V1_2::IImsRadioResponse>> castFrom(const ::android::sp<::vendor::qti::hardware::radio::ims::V1_2::IImsRadioResponse>& parent, bool emitError = false);
    static ::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::ims::V1_2::IImsRadioResponse>> castFrom(const ::android::sp<::vendor::qti::hardware::radio::ims::V1_0::IImsRadioResponse>& parent, bool emitError = false);
    static ::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::ims::V1_2::IImsRadioResponse>> castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError = false);

    static const char* descriptor;

    static ::android::sp<IImsRadioResponse> tryGetService(const std::string &serviceName="default", bool getStub=false);
    static ::android::sp<IImsRadioResponse> tryGetService(const char serviceName[], bool getStub=false)  { std::string str(serviceName ? serviceName : "");      return tryGetService(str, getStub); }
    static ::android::sp<IImsRadioResponse> tryGetService(const ::android::hardware::hidl_string& serviceName, bool getStub=false)  { std::string str(serviceName.c_str());      return tryGetService(str, getStub); }
    static ::android::sp<IImsRadioResponse> tryGetService(bool getStub) { return tryGetService("default", getStub); }
    static ::android::sp<IImsRadioResponse> getService(const std::string &serviceName="default", bool getStub=false);
    static ::android::sp<IImsRadioResponse> getService(const char serviceName[], bool getStub=false)  { std::string str(serviceName ? serviceName : "");      return getService(str, getStub); }
    static ::android::sp<IImsRadioResponse> getService(const ::android::hardware::hidl_string& serviceName, bool getStub=false)  { std::string str(serviceName.c_str());      return getService(str, getStub); }
    static ::android::sp<IImsRadioResponse> getService(bool getStub) { return getService("default", getStub); }
    __attribute__ ((warn_unused_result))::android::status_t registerAsService(const std::string &serviceName="default");
    static bool registerForNotifications(
            const std::string &serviceName,
            const ::android::sp<::android::hidl::manager::V1_0::IServiceNotification> &notification);
};

static inline std::string toString(const ::android::sp<::vendor::qti::hardware::radio::ims::V1_2::IImsRadioResponse>& o) {
    std::string os = "[class or subclass of ";
    os += ::vendor::qti::hardware::radio::ims::V1_2::IImsRadioResponse::descriptor;
    os += "]";
    os += o->isRemote() ? "@remote" : "@local";
    return os;
}


}  // namespace V1_2
}  // namespace ims
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor

#endif  // HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_IMS_V1_2_IIMSRADIORESPONSE_H
