/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#ifndef HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_UIM_V1_1_IUIMRESPONSE_H
#define HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_UIM_V1_1_IUIMRESPONSE_H

#include <vendor/qti/hardware/radio/uim/1.0/IUimResponse.h>
#include <vendor/qti/hardware/radio/uim/1.0/types.h>
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

struct IUimResponse : public ::vendor::qti::hardware::radio::uim::V1_0::IUimResponse {
    typedef android::hardware::details::i_tag _hidl_tag;

    // Forward declaration for forward reference support:

    virtual bool isRemote() const override { return false; }


    /**
     * UIM_REMOTE_SIMLOCK_RESPONSE
     * 
     * @param token Id to match req-resp. Value must match the one in req.
     * @param response Remote simlock response
     * @param simlockOp remote simlock requested operation
     * @param simlockRspData simlock response data valid for requested operation
     *        Encrypted Key for UIM_REMOTE_SIMLOCK_GENERATE_ENCRYPTED_KEY
     *        Simlock response BLOB for UIM_REMOTE_SIMLOCK_PROCESS_SIMLOCK_DATA
     *        HMAC data for UIM_REMOTE_SIMLOCK_GENERATE_HMAC
     * @param version Simlock blob highest version supported valid for requested
     * operation UIM_REMOTE_SIMLOCK_GET_MAX_SUPPORTED_VERSION
     * @param status Status of the simlock valid for requested operation
     * UIM_REMOTE_SIMLOCK_GET_STATUS
     */
    virtual ::android::hardware::Return<void> UimRemoteSimlockResponse(int32_t token, ::vendor::qti::hardware::radio::uim::V1_0::UimRemoteSimlockResponseType response, ::vendor::qti::hardware::radio::uim::V1_0::UimRemoteSimlockOperationType simlockOp, const ::android::hardware::hidl_vec<uint8_t>& simlockRspData, const ::vendor::qti::hardware::radio::uim::V1_0::UimRemoteSimlockVersion& version, const ::vendor::qti::hardware::radio::uim::V1_0::UimRemoteSimlockStatus& status) = 0;

    /**
     * UIM_REMOTE_SIMLOCK_RESPONSE_1_1
     * 
     * @param token Id to match req-resp. Value must match the one in req.
     * @param response Remote simlock response
     * @param simlockOp remote simlock requested operation
     * @param simlockRspData simlock response data valid for requested operation
     *        Encrypted Key for UIM_REMOTE_SIMLOCK_GENERATE_ENCRYPTED_KEY
     *        Simlock response BLOB for UIM_REMOTE_SIMLOCK_PROCESS_SIMLOCK_DATA
     *        HMAC data for UIM_REMOTE_SIMLOCK_GENERATE_HMAC
     *        BLOB data for UIM_REMOTE_SIMLOCK_GENERATE_BLOB_REQUEST
     * @param version Simlock blob highest version supported valid for requested
     * operation UIM_REMOTE_SIMLOCK_GET_MAX_SUPPORTED_VERSION
     * @param status Status of the simlock valid for requested operation
     * UIM_REMOTE_SIMLOCK_GET_STATUS
     */
    virtual ::android::hardware::Return<void> uimRemoteSimlockResponse_1_1(int32_t token, ::vendor::qti::hardware::radio::uim::V1_0::UimRemoteSimlockResponseType response, ::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType simlockOp, const ::android::hardware::hidl_vec<uint8_t>& simlockRspData, const ::vendor::qti::hardware::radio::uim::V1_0::UimRemoteSimlockVersion& version, const ::vendor::qti::hardware::radio::uim::V1_0::UimRemoteSimlockStatus& status) = 0;

    /**
     * UIM_REMOTE_SIMLOCK_TIMER_RESPONSE_1_1
     * 
     * @param token Id to match req-resp. Value must match the one in req.
     * @param response Remote simlock response
     * @param timerValue timer value started by simlock engine
     */
    virtual ::android::hardware::Return<void> uimRemoteSimlockTimerResponse_1_1(int32_t token, ::vendor::qti::hardware::radio::uim::V1_0::UimRemoteSimlockResponseType response, int32_t timerValue) = 0;

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
    static ::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::uim::V1_1::IUimResponse>> castFrom(const ::android::sp<::vendor::qti::hardware::radio::uim::V1_1::IUimResponse>& parent, bool emitError = false);
    static ::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::uim::V1_1::IUimResponse>> castFrom(const ::android::sp<::vendor::qti::hardware::radio::uim::V1_0::IUimResponse>& parent, bool emitError = false);
    static ::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::uim::V1_1::IUimResponse>> castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError = false);

    static const char* descriptor;

    static ::android::sp<IUimResponse> tryGetService(const std::string &serviceName="default", bool getStub=false);
    static ::android::sp<IUimResponse> tryGetService(const char serviceName[], bool getStub=false)  { std::string str(serviceName ? serviceName : "");      return tryGetService(str, getStub); }
    static ::android::sp<IUimResponse> tryGetService(const ::android::hardware::hidl_string& serviceName, bool getStub=false)  { std::string str(serviceName.c_str());      return tryGetService(str, getStub); }
    static ::android::sp<IUimResponse> tryGetService(bool getStub) { return tryGetService("default", getStub); }
    static ::android::sp<IUimResponse> getService(const std::string &serviceName="default", bool getStub=false);
    static ::android::sp<IUimResponse> getService(const char serviceName[], bool getStub=false)  { std::string str(serviceName ? serviceName : "");      return getService(str, getStub); }
    static ::android::sp<IUimResponse> getService(const ::android::hardware::hidl_string& serviceName, bool getStub=false)  { std::string str(serviceName.c_str());      return getService(str, getStub); }
    static ::android::sp<IUimResponse> getService(bool getStub) { return getService("default", getStub); }
    __attribute__ ((warn_unused_result))::android::status_t registerAsService(const std::string &serviceName="default");
    static bool registerForNotifications(
            const std::string &serviceName,
            const ::android::sp<::android::hidl::manager::V1_0::IServiceNotification> &notification);
};

static inline std::string toString(const ::android::sp<::vendor::qti::hardware::radio::uim::V1_1::IUimResponse>& o) {
    std::string os = "[class or subclass of ";
    os += ::vendor::qti::hardware::radio::uim::V1_1::IUimResponse::descriptor;
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

#endif  // HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_UIM_V1_1_IUIMRESPONSE_H
