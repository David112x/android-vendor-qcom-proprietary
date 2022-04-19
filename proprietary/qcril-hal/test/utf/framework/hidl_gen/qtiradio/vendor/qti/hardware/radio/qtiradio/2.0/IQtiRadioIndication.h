/*
  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#ifndef HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_QTIRADIO_V2_0_IQTIRADIOINDICATION_H
#define HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_QTIRADIO_V2_0_IQTIRADIOINDICATION_H

#include <vendor/qti/hardware/radio/qtiradio/1.0/IQtiRadioIndication.h>
#include <vendor/qti/hardware/radio/qtiradio/2.0/types.h>

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
namespace qtiradio {
namespace V2_0 {

struct IQtiRadioIndication : public ::vendor::qti::hardware::radio::qtiradio::V1_0::IQtiRadioIndication {
    typedef android::hardware::details::i_tag _hidl_tag;

    // Forward declaration for forward reference support:

    virtual bool isRemote() const override { return false; }


    virtual ::android::hardware::Return<void> qtiRadioIndication(::vendor::qti::hardware::radio::qtiradio::V1_0::QtiRadioIndicationType type) = 0;

    virtual ::android::hardware::Return<void> on5gStatusChange(::vendor::qti::hardware::radio::qtiradio::V2_0::EnableStatus status) = 0;

    virtual ::android::hardware::Return<void> onNrDcParamChange(const ::vendor::qti::hardware::radio::qtiradio::V2_0::DcParam& dcParam) = 0;

    virtual ::android::hardware::Return<void> onNrBearerAllocationChange(::vendor::qti::hardware::radio::qtiradio::V2_0::BearerStatus bearerStatus) = 0;

    virtual ::android::hardware::Return<void> onSignalStrengthChange(const ::vendor::qti::hardware::radio::qtiradio::V2_0::SignalStrength& signalStrength) = 0;

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
    static ::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_0::IQtiRadioIndication>> castFrom(const ::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_0::IQtiRadioIndication>& parent, bool emitError = false);
    static ::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_0::IQtiRadioIndication>> castFrom(const ::android::sp<::vendor::qti::hardware::radio::qtiradio::V1_0::IQtiRadioIndication>& parent, bool emitError = false);
    static ::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_0::IQtiRadioIndication>> castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError = false);

    static const char* descriptor;

    static ::android::sp<IQtiRadioIndication> tryGetService(const std::string &serviceName="default", bool getStub=false);
    static ::android::sp<IQtiRadioIndication> tryGetService(const char serviceName[], bool getStub=false)  { std::string str(serviceName ? serviceName : "");      return tryGetService(str, getStub); }
    static ::android::sp<IQtiRadioIndication> tryGetService(const ::android::hardware::hidl_string& serviceName, bool getStub=false)  { std::string str(serviceName.c_str());      return tryGetService(str, getStub); }
    static ::android::sp<IQtiRadioIndication> tryGetService(bool getStub) { return tryGetService("default", getStub); }
    static ::android::sp<IQtiRadioIndication> getService(const std::string &serviceName="default", bool getStub=false);
    static ::android::sp<IQtiRadioIndication> getService(const char serviceName[], bool getStub=false)  { std::string str(serviceName ? serviceName : "");      return getService(str, getStub); }
    static ::android::sp<IQtiRadioIndication> getService(const ::android::hardware::hidl_string& serviceName, bool getStub=false)  { std::string str(serviceName.c_str());      return getService(str, getStub); }
    static ::android::sp<IQtiRadioIndication> getService(bool getStub) { return getService("default", getStub); }
    __attribute__ ((warn_unused_result))::android::status_t registerAsService(const std::string &serviceName="default");
    static bool registerForNotifications(
            const std::string &serviceName,
            const ::android::sp<::android::hidl::manager::V1_0::IServiceNotification> &notification);
};

static inline std::string toString(const ::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_0::IQtiRadioIndication>& o) {
    std::string os = "[class or subclass of ";
    os += ::vendor::qti::hardware::radio::qtiradio::V2_0::IQtiRadioIndication::descriptor;
    os += "]";
    os += o->isRemote() ? "@remote" : "@local";
    return os;
}


}  // namespace V2_0
}  // namespace qtiradio
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor

#endif  // HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_QTIRADIO_V2_0_IQTIRADIOINDICATION_H
