/******************************************************************************
  @file framework/hidl_gen/ims/vendor/qti/hardware/radio/ims/1.2/IImsRadio.h
  DESCRIPTION
  ---------------------------------------------------------------------------
  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
******************************************************************************/
#ifndef HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_IMS_V1_2_IIMSRADIO_H
#define HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_IMS_V1_2_IIMSRADIO_H

#include <vendor/qti/hardware/radio/ims/1.1/IImsRadio.h>
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

struct IImsRadio : public ::vendor::qti::hardware::radio::ims::V1_1::IImsRadio {
    typedef android::hardware::details::i_tag _hidl_tag;

    // Forward declaration for forward reference support:

    virtual bool isRemote() const override { return false; }


    virtual ::android::hardware::Return<void> setCallback(const ::android::sp<::vendor::qti::hardware::radio::ims::V1_0::IImsRadioResponse>& imsRadioResponse, const ::android::sp<::vendor::qti::hardware::radio::ims::V1_0::IImsRadioIndication>& imsRadioIndication) = 0;

    virtual ::android::hardware::Return<void> dial(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_0::DialRequest& dialRequest) = 0;

    virtual ::android::hardware::Return<void> addParticipant(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_0::DialRequest& dialRequest) = 0;

    virtual ::android::hardware::Return<void> getImsRegistrationState(int32_t token) = 0;

    virtual ::android::hardware::Return<void> answer(int32_t token, ::vendor::qti::hardware::radio::ims::V1_0::CallType callType, ::vendor::qti::hardware::radio::ims::V1_0::IpPresentation presentation, ::vendor::qti::hardware::radio::ims::V1_0::RttMode mode) = 0;

    virtual ::android::hardware::Return<void> hangup(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_0::HangupRequestInfo& hangup) = 0;

    virtual ::android::hardware::Return<void> requestRegistrationChange(int32_t token, ::vendor::qti::hardware::radio::ims::V1_0::RegState state) = 0;

    virtual ::android::hardware::Return<void> queryServiceStatus(int32_t token) = 0;

    virtual ::android::hardware::Return<void> setServiceStatus(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_0::ServiceStatusInfo& srvStatusInfo) = 0;

    virtual ::android::hardware::Return<void> hold(int32_t token, uint32_t callId) = 0;

    virtual ::android::hardware::Return<void> resume(int32_t token, uint32_t callId) = 0;

    virtual ::android::hardware::Return<void> setConfig(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_0::ConfigInfo& config) = 0;

    virtual ::android::hardware::Return<void> getConfig(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_0::ConfigInfo& config) = 0;

    virtual ::android::hardware::Return<void> conference(int32_t token) = 0;

    virtual ::android::hardware::Return<void> getClip(int32_t token) = 0;

    virtual ::android::hardware::Return<void> getClir(int32_t token) = 0;

    virtual ::android::hardware::Return<void> setClir(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_0::ClirInfo& clirInfo) = 0;

    virtual ::android::hardware::Return<void> getColr(int32_t token) = 0;

    virtual ::android::hardware::Return<void> setColr(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_0::ColrInfo& colrInfo) = 0;

    virtual ::android::hardware::Return<void> exitEmergencyCallbackMode(int32_t token) = 0;

    virtual ::android::hardware::Return<void> sendDtmf(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_0::DtmfInfo& dtmfInfo) = 0;

    virtual ::android::hardware::Return<void> startDtmf(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_0::DtmfInfo& dtmfInfo) = 0;

    virtual ::android::hardware::Return<void> stopDtmf(int32_t token) = 0;

    virtual ::android::hardware::Return<void> setUiTtyMode(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_0::TtyInfo& ttyInfo) = 0;

    virtual ::android::hardware::Return<void> modifyCallInitiate(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_0::CallModifyInfo& callModifyInfo) = 0;

    virtual ::android::hardware::Return<void> modifyCallConfirm(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_0::CallModifyInfo& callModifyInfo) = 0;

    virtual ::android::hardware::Return<void> queryCallForwardStatus(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_0::CallForwardInfo& callForwardInfo) = 0;

    virtual ::android::hardware::Return<void> setCallForwardStatus(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_0::CallForwardInfo& callForwardInfo) = 0;

    virtual ::android::hardware::Return<void> getCallWaiting(int32_t token, uint32_t serviceClass) = 0;

    virtual ::android::hardware::Return<void> setCallWaiting(int32_t token, ::vendor::qti::hardware::radio::ims::V1_0::ServiceClassStatus serviceStatus, uint32_t serviceClass) = 0;

    virtual ::android::hardware::Return<void> setSuppServiceNotification(int32_t token, ::vendor::qti::hardware::radio::ims::V1_0::ServiceClassStatus status) = 0;

    virtual ::android::hardware::Return<void> explicitCallTransfer(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_0::ExplicitCallTransferInfo& ectInfo) = 0;

    virtual ::android::hardware::Return<void> suppServiceStatus(int32_t token, int32_t operationType, ::vendor::qti::hardware::radio::ims::V1_0::FacilityType facilityType, const ::vendor::qti::hardware::radio::ims::V1_0::CbNumListInfo& cbNumListInfo) = 0;

    virtual ::android::hardware::Return<void> getRtpStatistics(int32_t token) = 0;

    virtual ::android::hardware::Return<void> getRtpErrorStatistics(int32_t token) = 0;

    virtual ::android::hardware::Return<void> deflectCall(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_0::DeflectRequestInfo& deflectRequestInfo) = 0;

    virtual ::android::hardware::Return<void> sendGeolocationInfo(int32_t token, double lat, double lon, const ::vendor::qti::hardware::radio::ims::V1_0::AddressInfo& addressInfo) = 0;

    virtual ::android::hardware::Return<void> getImsSubConfig(int32_t token) = 0;

    virtual ::android::hardware::Return<void> sendRttMessage(int32_t token, const ::android::hardware::hidl_string& message) = 0;

    virtual ::android::hardware::Return<void> cancelModifyCall(int32_t token, uint32_t callId) = 0;

    virtual ::android::hardware::Return<void> hangup_1_1(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_1::HangupRequestInfo& hangup) = 0;

    virtual ::android::hardware::Return<void> hangup_1_2(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_2::HangupRequestInfo& hangup) = 0;

    virtual ::android::hardware::Return<void> sendImsSms(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsMessage& imsSms) = 0;

    virtual ::android::hardware::Return<void> acknowledgeSms(int32_t token, uint32_t messageRef, ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsDeliverStatusResult smsResult) = 0;

    virtual ::android::hardware::Return<void> acknowledgeSmsReport(int32_t token, uint32_t messageRef, ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsStatusReportResult smsReport) = 0;

    using getSmsFormat_cb = std::function<void(const ::android::hardware::hidl_string& format)>;
    virtual ::android::hardware::Return<void> getSmsFormat(getSmsFormat_cb _hidl_cb) = 0;

    virtual ::android::hardware::Return<void> sendGeolocationInfo_1_2(int32_t token, double lat, double lon, const ::vendor::qti::hardware::radio::ims::V1_2::AddressInfo& addressInfo) = 0;

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
    static ::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::ims::V1_2::IImsRadio>> castFrom(const ::android::sp<::vendor::qti::hardware::radio::ims::V1_2::IImsRadio>& parent, bool emitError = false);
    static ::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::ims::V1_2::IImsRadio>> castFrom(const ::android::sp<::vendor::qti::hardware::radio::ims::V1_1::IImsRadio>& parent, bool emitError = false);
    static ::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::ims::V1_2::IImsRadio>> castFrom(const ::android::sp<::vendor::qti::hardware::radio::ims::V1_0::IImsRadio>& parent, bool emitError = false);
    static ::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::ims::V1_2::IImsRadio>> castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError = false);

    static const char* descriptor;

    static ::android::sp<IImsRadio> tryGetService(const std::string &serviceName="default", bool getStub=false);
    static ::android::sp<IImsRadio> tryGetService(const char serviceName[], bool getStub=false)  { std::string str(serviceName ? serviceName : "");      return tryGetService(str, getStub); }
    static ::android::sp<IImsRadio> tryGetService(const ::android::hardware::hidl_string& serviceName, bool getStub=false)  { std::string str(serviceName.c_str());      return tryGetService(str, getStub); }
    static ::android::sp<IImsRadio> tryGetService(bool getStub) { return tryGetService("default", getStub); }
    static ::android::sp<IImsRadio> getService(const std::string &serviceName="default", bool getStub=false);
    static ::android::sp<IImsRadio> getService(const char serviceName[], bool getStub=false)  { std::string str(serviceName ? serviceName : "");      return getService(str, getStub); }
    static ::android::sp<IImsRadio> getService(const ::android::hardware::hidl_string& serviceName, bool getStub=false)  { std::string str(serviceName.c_str());      return getService(str, getStub); }
    static ::android::sp<IImsRadio> getService(bool getStub) { return getService("default", getStub); }
    __attribute__ ((warn_unused_result))::android::status_t registerAsService(const std::string &serviceName="default");
    static bool registerForNotifications(
            const std::string &serviceName,
            const ::android::sp<::android::hidl::manager::V1_0::IServiceNotification> &notification);
};

static inline std::string toString(const ::android::sp<::vendor::qti::hardware::radio::ims::V1_2::IImsRadio>& o) {
    std::string os = "[class or subclass of ";
    os += ::vendor::qti::hardware::radio::ims::V1_2::IImsRadio::descriptor;
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

#endif  // HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_IMS_V1_2_IIMSRADIO_H
