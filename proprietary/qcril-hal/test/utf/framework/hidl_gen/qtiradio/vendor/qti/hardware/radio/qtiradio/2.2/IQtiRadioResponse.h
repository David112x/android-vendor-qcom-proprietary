/*
  Copyright (c) 2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
#ifndef HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_QTIRADIO_V2_2_IQTIRADIORESPONSE_H
#define HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_QTIRADIO_V2_2_IQTIRADIORESPONSE_H

#include <vendor/qti/hardware/radio/qtiradio/2.1/IQtiRadioResponse.h>
#include <vendor/qti/hardware/radio/qtiradio/2.2/types.h>

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

struct IQtiRadioResponse : public ::vendor::qti::hardware::radio::qtiradio::V2_1::IQtiRadioResponse {
    typedef android::hardware::details::i_tag _hidl_tag;

    // Forward declaration for forward reference support:

    virtual bool isRemote() const override { return false; }


    /**
     * @param info Response info struct containing response type, serial no. and error
     * @param atr String containing the SIM ATR (Answer To Reset; as per ISO/IEC 7816-4)
     *
     * Valid errors returned:
     *   RadioError:NONE
     *   RadioError:RADIO_NOT_AVAILABLE
     *   RadioError:RIL_E_REQUEST_NOT_SUPPORTED
     */
    virtual ::android::hardware::Return<void> getAtrResponse(const ::vendor::qti::hardware::radio::qtiradio::V1_0::QtiRadioResponseInfo& info, const ::android::hardware::hidl_string& atr) = 0;

    /**
     * Response to IQtiRadio.enable5g
     *
     * @param serial to match request/response. Response must inclue same serial as request.
     * @param errorCode - errorCode as per types.hal returned from RIL.
     * @param status SUCCESS/FAILURE of the request.
     */
    virtual ::android::hardware::Return<void> onEnable5gResponse(int32_t serial, uint32_t errorCode, ::vendor::qti::hardware::radio::qtiradio::V2_0::Status status) = 0;

    /**
     * Response to IQtiRadio.disable5g
     *
     * @param serial to match request/response. Response must inclue same serial as request.
     * @param errorCode - errorCode as per types.hal returned from RIL.
     * @param status SUCCESS/FAILURE of the request.
     */
    virtual ::android::hardware::Return<void> onDisable5gResponse(int32_t serial, uint32_t errorCode, ::vendor::qti::hardware::radio::qtiradio::V2_0::Status status) = 0;

    /**
     * Response to IQtiRadio.enable5gOnly
     *
     * @param serial to match request/response. Response must inclue same serial as request.
     * @param errorCode - errorCode as per types.hal returned from RIL.
     * @param status SUCCESS/FAILURE of the request.
     */
    virtual ::android::hardware::Return<void> onEnable5gOnlyResponse(int32_t serial, uint32_t errorCode, ::vendor::qti::hardware::radio::qtiradio::V2_0::Status status) = 0;

    /**
     * Response to IQtiRadio.query5gStatus
     *
     * @param serial to match request/response. Response must inclue same serial as request.
     * @param errorCode - errorCode as per types.hal returned from RIL.
     * @param enabled values as per types.hal to indicate status of 5g in NSA or SA mode.
     */
    virtual ::android::hardware::Return<void> on5gStatusResponse(int32_t serial, uint32_t errorCode, ::vendor::qti::hardware::radio::qtiradio::V2_0::EnableStatus enabled) = 0;

    /**
     * Response to IQtiRadio.queryNrDcParam
     *
     * @param serial to match request/response. Response must inclue same serial as request.
     * @param errorCode - errorCode as per types.hal returned from RIL.
     * @param dcParam info about EN-DC and restrict-DCNR..
     */
    virtual ::android::hardware::Return<void> onNrDcParamResponse(int32_t serial, uint32_t errorCode, const ::vendor::qti::hardware::radio::qtiradio::V2_0::DcParam& dcParam) = 0;

    /**
     * Response to IQtiRadio.queryNrBearerAllocation
     *
     * @param serial to match request/response. Response must inclue same serial as request.
     * @param errorCode - errorCode as per types.hal returned from RIL.
     * @param bearerStatus values as per types.hal to indicate status of 5g bearer allocation..
     */
    virtual ::android::hardware::Return<void> onNrBearerAllocationResponse(int32_t serial, uint32_t errorCode, ::vendor::qti::hardware::radio::qtiradio::V2_0::BearerStatus bearerStatus) = 0;

    /**
     * Response to IQtiRadio.queryNrSignalStrength
     *
     * @param serial to match request/response. Response must inclue same serial as request.
     * @param errorCode - errorCode as per types.hal returned from RIL.
     * @param signalStrength values as per types.hal to indicate 5g signal strength parameters.
     */
    virtual ::android::hardware::Return<void> onSignalStrengthResponse(int32_t serial, uint32_t errorCode, const ::vendor::qti::hardware::radio::qtiradio::V2_0::SignalStrength& signalStrength) = 0;

    /**
     * @param info Response info struct containing response type, serial no. and error
     * @param sms Sms result struct as defined by SendSmsResult in types.hal
     *
     * Valid errors returned:
     *   RadioError:NONE
     *   RadioError:RADIO_NOT_AVAILABLE
     *   RadioError:INVALID_ARGUMENTS
     *   RadioError:SMS_SEND_FAIL_RETRY
     *   RadioError:NETWORK_REJECT
     *   RadioError:INVALID_STATE
     *   RadioError:NO_MEMORY
     *   RadioError:REQUEST_RATE_LIMITED
     *   RadioError:INVALID_SMS_FORMAT
     *   RadioError:SYSTEM_ERR
     *   RadioError:FDN_CHECK_FAILURE
     *   RadioError:MODEM_ERR
     *   RadioError:NETWORK_ERR
     *   RadioError:ENCODING_ERR
     *   RadioError:INVALID_SMSC_ADDRESS
     *   RadioError:INTERNAL_ERR
     *   RadioError:SYSTEM_ERR
     *   RadioError:REQUEST_NOT_SUPPORTED
     *   RadioError:OPERATION_NOT_ALLOWED
     *   RadioError:ENCODING_ERR
     *   RadioError:NO_RESOURCES
     *   RadioError:CANCELLED
     *   RadioError:SIM_ABSENT
     */
    virtual ::android::hardware::Return<void> sendCdmaSmsResponse(const ::vendor::qti::hardware::radio::qtiradio::V1_0::QtiRadioResponseInfo& info, const ::android::hardware::radio::V1_0::SendSmsResult& sms) = 0;

    virtual ::android::hardware::Return<void> onUpperLayerIndInfoResponse(int32_t serial, uint32_t errorCode, const ::vendor::qti::hardware::radio::qtiradio::V2_1::UpperLayerIndInfo& uliInfo) = 0;

    /**
     * Response to IQtiRadio.queryNrBearerAllocation
     *
     * @param serial to match request/response. Response must inclue same serial as request.
     * @param errorCode - errorCode as per types.hal returned from RIL.
     * @param bearerStatus values as per types.hal to indicate status of 5g bearer allocation..
     */
    virtual ::android::hardware::Return<void> onNrBearerAllocationResponse_2_1(int32_t serial, uint32_t errorCode, ::vendor::qti::hardware::radio::qtiradio::V2_1::BearerStatus bearerStatus) = 0;

    /**
     * Response to IQtiRadio.queryNrBearerAllocation
     *
     * @param serial to match request/response. Response must inclue same serial as request.
     * @param errorCode - errorCode as per types.hal returned from RIL.
     * @param config values as per types.hal to indicate status of 5g configuration type NSA/SA.
     */
    virtual ::android::hardware::Return<void> on5gConfigInfoResponse(int32_t serial, uint32_t errorCode, ::vendor::qti::hardware::radio::qtiradio::V2_1::ConfigType config) = 0;

    /**
     * Response to IQtiRadio.queryNrIconType
     *
     * @param serial to match request/response. Response must include same serial as request.
     * @param errorCode - errorCode as per types.hal returned from RIL.
     * @param NrIconType as per types.hal to indicate 5G icon - NONE(Non-5G) or
     *        5G BASIC or 5G UWB shown on the UI.
     */
    virtual ::android::hardware::Return<void> onNrIconTypeResponse(int32_t serial, uint32_t errorCode, ::vendor::qti::hardware::radio::qtiradio::V2_2::NrIconType iconType) = 0;

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
    static ::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_2::IQtiRadioResponse>> castFrom(const ::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_2::IQtiRadioResponse>& parent, bool emitError = false);
    static ::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_2::IQtiRadioResponse>> castFrom(const ::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_1::IQtiRadioResponse>& parent, bool emitError = false);
    static ::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_2::IQtiRadioResponse>> castFrom(const ::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_0::IQtiRadioResponse>& parent, bool emitError = false);
    static ::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_2::IQtiRadioResponse>> castFrom(const ::android::sp<::vendor::qti::hardware::radio::qtiradio::V1_0::IQtiRadioResponse>& parent, bool emitError = false);
    static ::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_2::IQtiRadioResponse>> castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError = false);

    static const char* descriptor;

    static ::android::sp<IQtiRadioResponse> tryGetService(const std::string &serviceName="default", bool getStub=false);
    static ::android::sp<IQtiRadioResponse> tryGetService(const char serviceName[], bool getStub=false)  { std::string str(serviceName ? serviceName : "");      return tryGetService(str, getStub); }
    static ::android::sp<IQtiRadioResponse> tryGetService(const ::android::hardware::hidl_string& serviceName, bool getStub=false)  { std::string str(serviceName.c_str());      return tryGetService(str, getStub); }
    static ::android::sp<IQtiRadioResponse> tryGetService(bool getStub) { return tryGetService("default", getStub); }
    static ::android::sp<IQtiRadioResponse> getService(const std::string &serviceName="default", bool getStub=false);
    static ::android::sp<IQtiRadioResponse> getService(const char serviceName[], bool getStub=false)  { std::string str(serviceName ? serviceName : "");      return getService(str, getStub); }
    static ::android::sp<IQtiRadioResponse> getService(const ::android::hardware::hidl_string& serviceName, bool getStub=false)  { std::string str(serviceName.c_str());      return getService(str, getStub); }
    static ::android::sp<IQtiRadioResponse> getService(bool getStub) { return getService("default", getStub); }
    __attribute__ ((warn_unused_result))::android::status_t registerAsService(const std::string &serviceName="default");
    static bool registerForNotifications(
            const std::string &serviceName,
            const ::android::sp<::android::hidl::manager::V1_0::IServiceNotification> &notification);
};

static inline std::string toString(const ::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_2::IQtiRadioResponse>& o) {
    std::string os = "[class or subclass of ";
    os += ::vendor::qti::hardware::radio::qtiradio::V2_2::IQtiRadioResponse::descriptor;
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

#endif  // HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_QTIRADIO_V2_2_IQTIRADIORESPONSE_H
