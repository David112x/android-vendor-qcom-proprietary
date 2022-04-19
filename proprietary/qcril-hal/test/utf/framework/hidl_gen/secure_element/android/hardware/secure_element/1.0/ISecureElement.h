/******************************************************************************
  @file framework/hidl_gen/secure_element/android/hardware/secure_element/1.0/ISecureElement.h
  DESCRIPTION
  ---------------------------------------------------------------------------
  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
******************************************************************************/
#ifndef HIDL_GENERATED_ANDROID_HARDWARE_SECURE_ELEMENT_V1_0_ISECUREELEMENT_H
#define HIDL_GENERATED_ANDROID_HARDWARE_SECURE_ELEMENT_V1_0_ISECUREELEMENT_H

#include <android/hardware/secure_element/1.0/ISecureElementHalCallback.h>
#include <android/hardware/secure_element/1.0/types.h>
#include <android/hidl/base/1.0/IBase.h>

#include <android/hidl/manager/1.0/IServiceNotification.h>

#include <hidl/HidlSupport.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <utils/NativeHandle.h>
#include <utils/misc.h>

namespace android {
namespace hardware {
namespace secure_element {
namespace V1_0 {

struct ISecureElement : public ::android::hidl::base::V1_0::IBase {
    typedef android::hardware::details::i_tag _hidl_tag;

    // Forward declaration for forward reference support:

    virtual bool isRemote() const override { return false; }


    virtual ::android::hardware::Return<void> init(const ::android::sp<::android::hardware::secure_element::V1_0::ISecureElementHalCallback>& clientCallback) = 0;

    using getAtr_cb = std::function<void(const ::android::hardware::hidl_vec<uint8_t>& response)>;
    virtual ::android::hardware::Return<void> getAtr(getAtr_cb _hidl_cb) = 0;

    virtual ::android::hardware::Return<bool> isCardPresent() = 0;

    using transmit_cb = std::function<void(const ::android::hardware::hidl_vec<uint8_t>& response)>;
    virtual ::android::hardware::Return<void> transmit(const ::android::hardware::hidl_vec<uint8_t>& data, transmit_cb _hidl_cb) = 0;

    using openLogicalChannel_cb = std::function<void(const ::android::hardware::secure_element::V1_0::LogicalChannelResponse& response, ::android::hardware::secure_element::V1_0::SecureElementStatus status)>;
    virtual ::android::hardware::Return<void> openLogicalChannel(const ::android::hardware::hidl_vec<uint8_t>& aid, uint8_t p2, openLogicalChannel_cb _hidl_cb) = 0;

    using openBasicChannel_cb = std::function<void(const ::android::hardware::hidl_vec<uint8_t>& selectResponse, ::android::hardware::secure_element::V1_0::SecureElementStatus status)>;
    virtual ::android::hardware::Return<void> openBasicChannel(const ::android::hardware::hidl_vec<uint8_t>& aid, uint8_t p2, openBasicChannel_cb _hidl_cb) = 0;

    virtual ::android::hardware::Return<::android::hardware::secure_element::V1_0::SecureElementStatus> closeChannel(uint8_t channelNumber) = 0;

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
    static ::android::hardware::Return<::android::sp<::android::hardware::secure_element::V1_0::ISecureElement>> castFrom(const ::android::sp<::android::hardware::secure_element::V1_0::ISecureElement>& parent, bool emitError = false);
    static ::android::hardware::Return<::android::sp<::android::hardware::secure_element::V1_0::ISecureElement>> castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError = false);

    static const char* descriptor;

    static ::android::sp<ISecureElement> tryGetService(const std::string &serviceName="default", bool getStub=false);
    static ::android::sp<ISecureElement> tryGetService(const char serviceName[], bool getStub=false)  { std::string str(serviceName ? serviceName : "");      return tryGetService(str, getStub); }
    static ::android::sp<ISecureElement> tryGetService(const ::android::hardware::hidl_string& serviceName, bool getStub=false)  { std::string str(serviceName.c_str());      return tryGetService(str, getStub); }
    static ::android::sp<ISecureElement> tryGetService(bool getStub) { return tryGetService("default", getStub); }
    static ::android::sp<ISecureElement> getService(const std::string &serviceName="default", bool getStub=false);
    static ::android::sp<ISecureElement> getService(const char serviceName[], bool getStub=false)  { std::string str(serviceName ? serviceName : "");      return getService(str, getStub); }
    static ::android::sp<ISecureElement> getService(const ::android::hardware::hidl_string& serviceName, bool getStub=false)  { std::string str(serviceName.c_str());      return getService(str, getStub); }
    static ::android::sp<ISecureElement> getService(bool getStub) { return getService("default", getStub); }
    __attribute__ ((warn_unused_result))::android::status_t registerAsService(const std::string &serviceName="default");
    static bool registerForNotifications(
            const std::string &serviceName,
            const ::android::sp<::android::hidl::manager::V1_0::IServiceNotification> &notification);
};

std::string toString(const ::android::sp<::android::hardware::secure_element::V1_0::ISecureElement>&);

}  // namespace V1_0
}  // namespace secure_element
}  // namespace hardware
}  // namespace android

#endif  // HIDL_GENERATED_ANDROID_HARDWARE_SECURE_ELEMENT_V1_0_ISECUREELEMENT_H
