/******************************************************************************
  @file framework/hidl_gen/ims/vendor/qti/hardware/radio/ims/1.2/IImsRadioIndication.h
  DESCRIPTION
  ---------------------------------------------------------------------------
  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
******************************************************************************/
#ifndef HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_IMS_V1_2_IIMSRADIOINDICATION_H
#define HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_IMS_V1_2_IIMSRADIOINDICATION_H

#include <vendor/qti/hardware/radio/ims/1.1/IImsRadioIndication.h>
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

struct IImsRadioIndication : public ::vendor::qti::hardware::radio::ims::V1_1::IImsRadioIndication {
    typedef android::hardware::details::i_tag _hidl_tag;

    // Forward declaration for forward reference support:

    virtual bool isRemote() const override { return false; }


    virtual ::android::hardware::Return<void> onCallStateChanged(const ::android::hardware::hidl_vec<::vendor::qti::hardware::radio::ims::V1_0::CallInfo>& callList) = 0;

    virtual ::android::hardware::Return<void> onRing() = 0;

    virtual ::android::hardware::Return<void> onRingbackTone(::vendor::qti::hardware::radio::ims::V1_0::ToneOperation tone) = 0;

    virtual ::android::hardware::Return<void> onRegistrationChanged(const ::vendor::qti::hardware::radio::ims::V1_0::RegistrationInfo& registration) = 0;

    virtual ::android::hardware::Return<void> onHandover(const ::vendor::qti::hardware::radio::ims::V1_0::HandoverInfo& handover) = 0;

    virtual ::android::hardware::Return<void> onServiceStatusChanged(const ::android::hardware::hidl_vec<::vendor::qti::hardware::radio::ims::V1_0::ServiceStatusInfo>& srvStatusList) = 0;

    virtual ::android::hardware::Return<void> onRadioStateChanged(::vendor::qti::hardware::radio::ims::V1_0::RadioState radioState) = 0;

    virtual ::android::hardware::Return<void> onEnterEmergencyCallBackMode() = 0;

    virtual ::android::hardware::Return<void> onExitEmergencyCallBackMode() = 0;

    virtual ::android::hardware::Return<void> onTtyNotification(const ::vendor::qti::hardware::radio::ims::V1_0::TtyInfo& ttyInfo) = 0;

    virtual ::android::hardware::Return<void> onRefreshConferenceInfo(const ::vendor::qti::hardware::radio::ims::V1_0::ConferenceInfo& conferenceInfo) = 0;

    virtual ::android::hardware::Return<void> onRefreshViceInfo(const ::vendor::qti::hardware::radio::ims::V1_0::ViceInfo& viceInfo) = 0;

    virtual ::android::hardware::Return<void> onModifyCall(const ::vendor::qti::hardware::radio::ims::V1_0::CallModifyInfo& callModifyInfo) = 0;

    virtual ::android::hardware::Return<void> onSuppServiceNotification(const ::vendor::qti::hardware::radio::ims::V1_0::SuppServiceNotification& suppServiceNotification) = 0;

    virtual ::android::hardware::Return<void> onMessageWaiting(const ::vendor::qti::hardware::radio::ims::V1_0::MessageWaitingIndication& messageWaitingIndication) = 0;

    virtual ::android::hardware::Return<void> onGeolocationInfoRequested(double lat, double lon) = 0;

    virtual ::android::hardware::Return<void> onImsSubConfigChanged(const ::vendor::qti::hardware::radio::ims::V1_0::ImsSubConfigInfo& config) = 0;

    virtual ::android::hardware::Return<void> onParticipantStatusInfo(const ::vendor::qti::hardware::radio::ims::V1_0::ParticipantStatusInfo& participantStatusInfo) = 0;

    virtual ::android::hardware::Return<void> onRegistrationBlockStatus(bool hasBlockStatusOnWwan, const ::vendor::qti::hardware::radio::ims::V1_0::BlockStatus& blockStatusOnWwan, bool hasBlockStatusOnWlan, const ::vendor::qti::hardware::radio::ims::V1_0::BlockStatus& blockStatusOnWlan) = 0;

    virtual ::android::hardware::Return<void> onRttMessageReceived(const ::android::hardware::hidl_string& message) = 0;

    virtual ::android::hardware::Return<void> onVoWiFiCallQuality(::vendor::qti::hardware::radio::ims::V1_0::VoWiFiCallQualityInfo voWiFiCallQualityInfo) = 0;

    virtual ::android::hardware::Return<void> onSupplementaryServiceIndication(const ::vendor::qti::hardware::radio::ims::V1_0::StkCcUnsolSsResult& ss) = 0;

    virtual ::android::hardware::Return<void> onCallStateChanged_1_1(const ::android::hardware::hidl_vec<::vendor::qti::hardware::radio::ims::V1_1::CallInfo>& callList) = 0;

    virtual ::android::hardware::Return<void> onCallStateChanged_1_2(const ::android::hardware::hidl_vec<::vendor::qti::hardware::radio::ims::V1_2::CallInfo>& callList) = 0;

    virtual ::android::hardware::Return<void> onImsSmsStatusReport(const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendStatusReport& smsStatusReport) = 0;

    virtual ::android::hardware::Return<void> onIncomingImsSms(const ::vendor::qti::hardware::radio::ims::V1_2::IncomingImsSms& imsSms) = 0;

    virtual ::android::hardware::Return<void> onVopsChanged(bool isVopsEnabled) = 0;

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
    static ::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::ims::V1_2::IImsRadioIndication>> castFrom(const ::android::sp<::vendor::qti::hardware::radio::ims::V1_2::IImsRadioIndication>& parent, bool emitError = false);
    static ::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::ims::V1_2::IImsRadioIndication>> castFrom(const ::android::sp<::vendor::qti::hardware::radio::ims::V1_1::IImsRadioIndication>& parent, bool emitError = false);
    static ::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::ims::V1_2::IImsRadioIndication>> castFrom(const ::android::sp<::vendor::qti::hardware::radio::ims::V1_0::IImsRadioIndication>& parent, bool emitError = false);
    static ::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::ims::V1_2::IImsRadioIndication>> castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError = false);

    static const char* descriptor;

    static ::android::sp<IImsRadioIndication> tryGetService(const std::string &serviceName="default", bool getStub=false);
    static ::android::sp<IImsRadioIndication> tryGetService(const char serviceName[], bool getStub=false)  { std::string str(serviceName ? serviceName : "");      return tryGetService(str, getStub); }
    static ::android::sp<IImsRadioIndication> tryGetService(const ::android::hardware::hidl_string& serviceName, bool getStub=false)  { std::string str(serviceName.c_str());      return tryGetService(str, getStub); }
    static ::android::sp<IImsRadioIndication> tryGetService(bool getStub) { return tryGetService("default", getStub); }
    static ::android::sp<IImsRadioIndication> getService(const std::string &serviceName="default", bool getStub=false);
    static ::android::sp<IImsRadioIndication> getService(const char serviceName[], bool getStub=false)  { std::string str(serviceName ? serviceName : "");      return getService(str, getStub); }
    static ::android::sp<IImsRadioIndication> getService(const ::android::hardware::hidl_string& serviceName, bool getStub=false)  { std::string str(serviceName.c_str());      return getService(str, getStub); }
    static ::android::sp<IImsRadioIndication> getService(bool getStub) { return getService("default", getStub); }
    __attribute__ ((warn_unused_result))::android::status_t registerAsService(const std::string &serviceName="default");
    static bool registerForNotifications(
            const std::string &serviceName,
            const ::android::sp<::android::hidl::manager::V1_0::IServiceNotification> &notification);
};

static inline std::string toString(const ::android::sp<::vendor::qti::hardware::radio::ims::V1_2::IImsRadioIndication>& o) {
    std::string os = "[class or subclass of ";
    os += ::vendor::qti::hardware::radio::ims::V1_2::IImsRadioIndication::descriptor;
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

#endif  // HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_IMS_V1_2_IIMSRADIOINDICATION_H
