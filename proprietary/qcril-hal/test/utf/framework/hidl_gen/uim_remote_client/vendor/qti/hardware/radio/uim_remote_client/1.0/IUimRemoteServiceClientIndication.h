/******************************************************************************
#  Copyright (c) 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file framework/hidl_gen/uim_remote_client/vendor/qti/hardware/radio/uim_remote_client/1.0/IUimRemoteServiceClientIndication.h
  DESCRIPTION
  ---------------------------------------------------------------------------
******************************************************************************/
#ifndef HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_UIM_REMOTE_CLIENT_V1_0_IUIMREMOTESERVICECLIENTINDICATION_H
#define HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_UIM_REMOTE_CLIENT_V1_0_IUIMREMOTESERVICECLIENTINDICATION_H

#include <android/hidl/base/1.0/IBase.h>
#include <vendor/qti/hardware/radio/uim_remote_client/1.0/types.h>

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
namespace uim_remote_client {
namespace V1_0 {

struct IUimRemoteServiceClientIndication : public ::android::hidl::base::V1_0::IBase {
    virtual bool isRemote() const override { return false; }


    virtual ::android::hardware::Return<void> UimRemoteServiceClientApduInd(const ::android::hardware::hidl_vec<uint8_t>& apduInd) = 0;

    virtual ::android::hardware::Return<void> UimRemoteServiceClientConnectInd() = 0;

    virtual ::android::hardware::Return<void> UimRemoteServiceClientDisconnectInd() = 0;

    virtual ::android::hardware::Return<void> UimRemoteServiceClientPowerUpInd(bool hasTimeOut, int32_t timeOut, bool hasVoltageClass, UimRemoteClientVoltageClass powerUpVoltageClass) = 0;

    virtual ::android::hardware::Return<void> UimRemoteServiceClientPowerDownInd(bool hasPowerDownMode, UimRemoteClientPowerDownMode powerDownMode) = 0;

    virtual ::android::hardware::Return<void> UimRemoteServiceClientResetInd() = 0;

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
    static ::android::hardware::Return<::android::sp<IUimRemoteServiceClientIndication>> castFrom(const ::android::sp<IUimRemoteServiceClientIndication>& parent, bool emitError = false);
    static ::android::hardware::Return<::android::sp<IUimRemoteServiceClientIndication>> castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError = false);

    static const char* descriptor;

    static ::android::sp<IUimRemoteServiceClientIndication> tryGetService(const std::string &serviceName="default", bool getStub=false);
    static ::android::sp<IUimRemoteServiceClientIndication> tryGetService(const char serviceName[], bool getStub=false)  { std::string str(serviceName ? serviceName : "");      return tryGetService(str, getStub); }
    static ::android::sp<IUimRemoteServiceClientIndication> tryGetService(const ::android::hardware::hidl_string& serviceName, bool getStub=false)  { std::string str(serviceName.c_str());      return tryGetService(str, getStub); }
    static ::android::sp<IUimRemoteServiceClientIndication> tryGetService(bool getStub) { return tryGetService("default", getStub); }
    static ::android::sp<IUimRemoteServiceClientIndication> getService(const std::string &serviceName="default", bool getStub=false);
    static ::android::sp<IUimRemoteServiceClientIndication> getService(const char serviceName[], bool getStub=false)  { std::string str(serviceName ? serviceName : "");      return getService(str, getStub); }
    static ::android::sp<IUimRemoteServiceClientIndication> getService(const ::android::hardware::hidl_string& serviceName, bool getStub=false)  { std::string str(serviceName.c_str());      return getService(str, getStub); }
    static ::android::sp<IUimRemoteServiceClientIndication> getService(bool getStub) { return getService("default", getStub); }
    ::android::status_t registerAsService(const std::string &serviceName="default");
    static bool registerForNotifications(
            const std::string &serviceName,
            const ::android::sp<::android::hidl::manager::V1_0::IServiceNotification> &notification);
};

std::string toString(const ::android::sp<IUimRemoteServiceClientIndication>&);

}  // namespace V1_0
}  // namespace uim_remote_client
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor

#endif  // HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_UIM_REMOTE_CLIENT_V1_0_IUIMREMOTESERVICECLIENTINDICATION_H
