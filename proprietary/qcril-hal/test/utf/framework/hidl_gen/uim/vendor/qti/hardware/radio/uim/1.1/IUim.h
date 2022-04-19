/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#ifndef HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_UIM_V1_1_IUIM_H
#define HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_UIM_V1_1_IUIM_H

#include <vendor/qti/hardware/radio/uim/1.0/IUim.h>
#include <vendor/qti/hardware/radio/uim/1.1/types.h>

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
namespace uim {
namespace V1_1 {

struct IUim : public ::vendor::qti::hardware::radio::uim::V1_0::IUim {
    typedef android::hardware::details::i_tag _hidl_tag;

    // Forward declaration for forward reference support:

    virtual bool isRemote() const override { return false; }


    /**
     * Set callback for uim lpa requests and indications
     * 
     * @param responseCallback Object containing response callback functions
     * @param indicationCallback Object containing uim remote server indication callback functions
     */
    virtual ::android::hardware::Return<void> setCallback(const ::android::sp<::vendor::qti::hardware::radio::uim::V1_0::IUimResponse>& responseCallback, const ::android::sp<::vendor::qti::hardware::radio::uim::V1_0::IUimIndication>& indicationCallback) = 0;

    /**
     * UIM_REMOTE_SIMLOCK_REQUEST
     * 
     * @param token Id to match req-resp. Resp must include same token.
     * @param simLockOp simlock request type
     * @param simlockData simlock data
     */
    virtual ::android::hardware::Return<void> UimRemoteSimlockRequest(int32_t token, ::vendor::qti::hardware::radio::uim::V1_0::UimRemoteSimlockOperationType simlockOp, const ::android::hardware::hidl_vec<uint8_t>& simlockData) = 0;

    /**
     * UIM_REMOTE_SIMLOCK_REQUEST_1_1
     * 
     * @param token Id to match req-resp. Resp must include same token.
     * @param simLockOp simlock request type
     * @param simlockData simlock data
     */
    virtual ::android::hardware::Return<void> uimRemoteSimlockRequest_1_1(int32_t token, ::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType simlockOp, const ::android::hardware::hidl_vec<uint8_t>& simlockData) = 0;

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
    static ::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::uim::V1_1::IUim>> castFrom(const ::android::sp<::vendor::qti::hardware::radio::uim::V1_1::IUim>& parent, bool emitError = false);
    static ::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::uim::V1_1::IUim>> castFrom(const ::android::sp<::vendor::qti::hardware::radio::uim::V1_0::IUim>& parent, bool emitError = false);
    static ::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::uim::V1_1::IUim>> castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError = false);

    static const char* descriptor;

    static ::android::sp<IUim> tryGetService(const std::string &serviceName="default", bool getStub=false);
    static ::android::sp<IUim> tryGetService(const char serviceName[], bool getStub=false)  { std::string str(serviceName ? serviceName : "");      return tryGetService(str, getStub); }
    static ::android::sp<IUim> tryGetService(const ::android::hardware::hidl_string& serviceName, bool getStub=false)  { std::string str(serviceName.c_str());      return tryGetService(str, getStub); }
    static ::android::sp<IUim> tryGetService(bool getStub) { return tryGetService("default", getStub); }
    static ::android::sp<IUim> getService(const std::string &serviceName="default", bool getStub=false);
    static ::android::sp<IUim> getService(const char serviceName[], bool getStub=false)  { std::string str(serviceName ? serviceName : "");      return getService(str, getStub); }
    static ::android::sp<IUim> getService(const ::android::hardware::hidl_string& serviceName, bool getStub=false)  { std::string str(serviceName.c_str());      return getService(str, getStub); }
    static ::android::sp<IUim> getService(bool getStub) { return getService("default", getStub); }
    __attribute__ ((warn_unused_result))::android::status_t registerAsService(const std::string &serviceName="default");
    static bool registerForNotifications(
            const std::string &serviceName,
            const ::android::sp<::android::hidl::manager::V1_0::IServiceNotification> &notification);
};

static inline std::string toString(const ::android::sp<::vendor::qti::hardware::radio::uim::V1_1::IUim>& o) {
    std::string os = "[class or subclass of ";
    os += ::vendor::qti::hardware::radio::uim::V1_1::IUim::descriptor;
    os += "]";
    os += o->isRemote() ? "@remote" : "@local";
    return os;
}


}  // namespace V1_1
}  // namespace uim
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor

#endif  // HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_UIM_V1_1_IUIM_H
