/*
  Copyright (c) 2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
#ifndef HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_QTIRADIO_V2_2_IQTIRADIO_H
#define HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_QTIRADIO_V2_2_IQTIRADIO_H

#include <vendor/qti/hardware/radio/qtiradio/2.1/IQtiRadio.h>

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
namespace V2_2 {

struct IQtiRadio : public ::vendor::qti::hardware::radio::qtiradio::V2_1::IQtiRadio {
    typedef android::hardware::details::i_tag _hidl_tag;

    // Forward declaration for forward reference support:

    virtual bool isRemote() const override { return false; }


    /**
     * Set callback for qtiradio requests and qtiradio indications
     *
     * @param responseCallback Object containing response callback functions
     * @param indicationCallback Object containing oem hook indication callback functions
     */
    virtual ::android::hardware::Return<void> setCallback(const ::android::sp<::vendor::qti::hardware::radio::qtiradio::V1_0::IQtiRadioResponse>& responseCallback, const ::android::sp<::vendor::qti::hardware::radio::qtiradio::V1_0::IQtiRadioIndication>& indicationCallback) = 0;

    /**
     * @param serial Serial number of request.
     *
     * Response callback is IRadioResponse.getAtrResponse()
     */
    virtual ::android::hardware::Return<void> getAtr(int32_t serial) = 0;

    /**
     * QTIRADIO_REQUEST_ENABLE_5G
     *
     * @param serial to match request/response. Responses must include the same token as requests.
     *
     * Response callback is IQtiRadioResponse.enable5gResponse()
     */
    virtual ::android::hardware::Return<void> enable5g(int32_t serial) = 0;

    /**
     * QTIRADIO_REQUEST_DISABLE_5G
     *
     * @param serial to match request/response. Responses must include the same token as requests.
     *
     * Response callback is IQtiRadioResponse.disable5gResponse()
     */
    virtual ::android::hardware::Return<void> disable5g(int32_t serial) = 0;

    /**
     * QTIRADIO_REQUEST_ENABLE_5G_ONLY
     *
     * @param serial to match request/response. Responses must include the same token as requests.
     *
     * Response callback is IQtiRadioResponse.enable5gOnlyResponse()
     */
    virtual ::android::hardware::Return<void> enable5gOnly(int32_t serial) = 0;

    /**
     * QTIRADIO_REQUEST_QUERY_5G_STATUS
     *
     * @param serial to match request/response. Responses must include the same token as requests.
     *
     * Response callback is IQtiRadioResponse.on5gStatusResponse()
     */
    virtual ::android::hardware::Return<void> query5gStatus(int32_t serial) = 0;

    /**
     * QTIRADIO_REQUEST_QUERY_ENDC_DCNR_PARAM
     *
     * @param serial to match request/response. Responses must include the same token as requests.
     *
     * Response callback is IQtiRadioResponse.onNrDcParamResponse()
     */
    virtual ::android::hardware::Return<void> queryNrDcParam(int32_t serial) = 0;

    /**
     * QTIRADIO_REQUEST_QUERY_NR_BEARER_ALLOCATION
     *
     * @param serial to match request/response. Responses must include the same token as requests.
     *
     * Response callback is IQtiRadioResponse.onNrBearerAllocationResponse()
     */
    virtual ::android::hardware::Return<void> queryNrBearerAllocation(int32_t serial) = 0;

    /**
     * QTIRADIO_REQUEST_QUERY_NR_SIGNAL_STRENGTH
     *
     * @param serial to match request/response. Responses must include the same token as requests.
     *
     * Response callback is IQtiRadioResponse.onNrSignalStrengthResponse()
     */
    virtual ::android::hardware::Return<void> queryNrSignalStrength(int32_t serial) = 0;

    /**
     * Send a CDMA SMS message
     *
     * @param serial Serial number of request.
     * @param sms Cdma Sms to be sent described by CdmaSmsMessage in types.hal
     * @param expectMore more messages are expected to be sent or not
     *
     * Response callback is IRadioResponse.sendCdmaSmsResponse()
     */
    virtual ::android::hardware::Return<void> sendCdmaSms(int32_t serial, const ::android::hardware::radio::V1_0::CdmaSmsMessage& sms, bool expectMore) = 0;

    virtual ::android::hardware::Return<void> queryUpperLayerIndInfo(int32_t serial) = 0;

    virtual ::android::hardware::Return<void> query5gConfigInfo(int32_t serial) = 0;

    virtual ::android::hardware::Return<void> queryNrIconType(int32_t serial) = 0;

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
    static ::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_2::IQtiRadio>> castFrom(const ::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_2::IQtiRadio>& parent, bool emitError = false);
    static ::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_2::IQtiRadio>> castFrom(const ::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_1::IQtiRadio>& parent, bool emitError = false);
    static ::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_2::IQtiRadio>> castFrom(const ::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_0::IQtiRadio>& parent, bool emitError = false);
    static ::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_2::IQtiRadio>> castFrom(const ::android::sp<::vendor::qti::hardware::radio::qtiradio::V1_0::IQtiRadio>& parent, bool emitError = false);
    static ::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_2::IQtiRadio>> castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError = false);

    static const char* descriptor;

    static ::android::sp<IQtiRadio> tryGetService(const std::string &serviceName="default", bool getStub=false);
    static ::android::sp<IQtiRadio> tryGetService(const char serviceName[], bool getStub=false)  { std::string str(serviceName ? serviceName : "");      return tryGetService(str, getStub); }
    static ::android::sp<IQtiRadio> tryGetService(const ::android::hardware::hidl_string& serviceName, bool getStub=false)  { std::string str(serviceName.c_str());      return tryGetService(str, getStub); }
    static ::android::sp<IQtiRadio> tryGetService(bool getStub) { return tryGetService("default", getStub); }
    static ::android::sp<IQtiRadio> getService(const std::string &serviceName="default", bool getStub=false);
    static ::android::sp<IQtiRadio> getService(const char serviceName[], bool getStub=false)  { std::string str(serviceName ? serviceName : "");      return getService(str, getStub); }
    static ::android::sp<IQtiRadio> getService(const ::android::hardware::hidl_string& serviceName, bool getStub=false)  { std::string str(serviceName.c_str());      return getService(str, getStub); }
    static ::android::sp<IQtiRadio> getService(bool getStub) { return getService("default", getStub); }
    __attribute__ ((warn_unused_result))::android::status_t registerAsService(const std::string &serviceName="default");
    static bool registerForNotifications(
            const std::string &serviceName,
            const ::android::sp<::android::hidl::manager::V1_0::IServiceNotification> &notification);
};

static inline std::string toString(const ::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_2::IQtiRadio>& o) {
    std::string os = "[class or subclass of ";
    os += ::vendor::qti::hardware::radio::qtiradio::V2_2::IQtiRadio::descriptor;
    os += "]";
    os += o->isRemote() ? "@remote" : "@local";
    return os;
}


}  // namespace V2_2
}  // namespace qtiradio
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor

#endif  // HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_QTIRADIO_V2_2_IQTIRADIO_H
