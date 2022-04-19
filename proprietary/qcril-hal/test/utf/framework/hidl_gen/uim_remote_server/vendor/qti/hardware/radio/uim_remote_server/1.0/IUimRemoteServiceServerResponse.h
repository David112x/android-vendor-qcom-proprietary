/******************************************************************************
#  Copyright (c) 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file framework/hidl_gen/uim_remote_server/vendor/qti/hardware/radio/uim_remote_server/1.0/IUimRemoteServiceServerResponse.h
  DESCRIPTION
  ---------------------------------------------------------------------------
******************************************************************************/
#ifndef HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_UIM_REMOTE_SERVER_V1_0_IUIMREMOTESERVICESERVERRESPONSE_H
#define HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_UIM_REMOTE_SERVER_V1_0_IUIMREMOTESERVICESERVERRESPONSE_H

#include <android/hidl/base/1.0/IBase.h>
#include <vendor/qti/hardware/radio/uim_remote_server/1.0/types.h>

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
namespace uim_remote_server {
namespace V1_0 {

struct IUimRemoteServiceServerResponse : public ::android::hidl::base::V1_0::IBase {
    virtual bool isRemote() const override { return false; }


    virtual ::android::hardware::Return<void> uimRemoteServiceServerConnectResponse(int32_t token, UimRemoteServiceServerConnectRsp sapConnectRsp, int32_t maxMsgSize) = 0;

    virtual ::android::hardware::Return<void> uimRemoteServiceServerDisconnectResponse(int32_t token) = 0;

    virtual ::android::hardware::Return<void> uimRemoteServiceServerApduResponse(int32_t token, UimRemoteServiceServerResultCode resultCode, const ::android::hardware::hidl_vec<uint8_t>& apduRsp) = 0;

    virtual ::android::hardware::Return<void> uimRemoteServiceServerTransferAtrResponse(int32_t token, UimRemoteServiceServerResultCode resultCode, const ::android::hardware::hidl_vec<uint8_t>& atr) = 0;

    virtual ::android::hardware::Return<void> uimRemoteServiceServerPowerResponse(int32_t token, UimRemoteServiceServerResultCode resultCode) = 0;

    virtual ::android::hardware::Return<void> uimRemoteServiceServerResetSimResponse(int32_t token, UimRemoteServiceServerResultCode resultCode) = 0;

    virtual ::android::hardware::Return<void> uimRemoteServiceServerTransferCardReaderStatusResponse(int32_t token, UimRemoteServiceServerResultCode resultCode, int32_t cardReaderStatus) = 0;

    virtual ::android::hardware::Return<void> uimRemoteServiceServerErrorResponse(int32_t token) = 0;

    virtual ::android::hardware::Return<void> uimRemoteServiceServerTransferProtocolResponse(int32_t token, UimRemoteServiceServerResultCode resultCode) = 0;

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
    static ::android::hardware::Return<::android::sp<IUimRemoteServiceServerResponse>> castFrom(const ::android::sp<IUimRemoteServiceServerResponse>& parent, bool emitError = false);
    static ::android::hardware::Return<::android::sp<IUimRemoteServiceServerResponse>> castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError = false);

    static const char* descriptor;

    static ::android::sp<IUimRemoteServiceServerResponse> tryGetService(const std::string &serviceName="default", bool getStub=false);
    static ::android::sp<IUimRemoteServiceServerResponse> tryGetService(const char serviceName[], bool getStub=false)  { std::string str(serviceName ? serviceName : "");      return tryGetService(str, getStub); }
    static ::android::sp<IUimRemoteServiceServerResponse> tryGetService(const ::android::hardware::hidl_string& serviceName, bool getStub=false)  { std::string str(serviceName.c_str());      return tryGetService(str, getStub); }
    static ::android::sp<IUimRemoteServiceServerResponse> tryGetService(bool getStub) { return tryGetService("default", getStub); }
    static ::android::sp<IUimRemoteServiceServerResponse> getService(const std::string &serviceName="default", bool getStub=false);
    static ::android::sp<IUimRemoteServiceServerResponse> getService(const char serviceName[], bool getStub=false)  { std::string str(serviceName ? serviceName : "");      return getService(str, getStub); }
    static ::android::sp<IUimRemoteServiceServerResponse> getService(const ::android::hardware::hidl_string& serviceName, bool getStub=false)  { std::string str(serviceName.c_str());      return getService(str, getStub); }
    static ::android::sp<IUimRemoteServiceServerResponse> getService(bool getStub) { return getService("default", getStub); }
    ::android::status_t registerAsService(const std::string &serviceName="default");
    static bool registerForNotifications(
            const std::string &serviceName,
            const ::android::sp<::android::hidl::manager::V1_0::IServiceNotification> &notification);
};

std::string toString(const ::android::sp<IUimRemoteServiceServerResponse>&);

}  // namespace V1_0
}  // namespace uim_remote_server
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor

#endif  // HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_UIM_REMOTE_SERVER_V1_0_IUIMREMOTESERVICESERVERRESPONSE_H
