/*==============================================================================
  Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
==============================================================================*/
#ifndef VENDOR_QTI_HARDWARE_DATA_IWLAN_V1_0_IIWLANIMPL_H
#define VENDOR_QTI_HARDWARE_DATA_IWLAN_V1_0_IIWLANIMPL_H

#include <mutex>
#include <time.h>
#include <signal.h>
#include <vendor/qti/hardware/data/iwlan/1.0/IIWlan.h>
#include <vendor/qti/hardware/data/iwlan/1.0/IIWlanIndication.h>
#include <vendor/qti/hardware/data/iwlan/1.0/IIWlanResponse.h>

#include "MessageCommon.h"
#include "QtiMutex.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic pop


using android::sp;
using android::wp;
using android::hidl::base::V1_0::IBase;
using android::hardware::Void;
using android::hardware::Return;
using android::hardware::hidl_vec;
using android::hardware::hidl_string;
using android::hardware::hidl_handle;
using android::hardware::hidl_death_recipient;
using android::hardware::radio::V1_0::RadioError;
using android::hardware::radio::V1_2::DataRequestReason;
using android::hardware::radio::V1_4::DataProfileInfo;
using android::hardware::radio::V1_4::SetupDataCallResult;
using namespace android::hardware::radio;
using namespace vendor::qti::hardware::data::iwlan::V1_0;
using namespace rildata;

namespace vendor {
namespace qti {
namespace hardware {
namespace data {
namespace iwlan {

void IWlanServiceInit(int instanceId);
void onDataRegistrationStateChange();
void onDataCallListChange(std::vector<DataCallResult_t> dcList);
void onQualifiedNetworksChange(std::vector<QualifiedNetwork_t> qnList);
void onSetupDataCallIWlanResponseIndMessage(SetupDataCallResponse_t response, int32_t serial, Message::Callback::Status status);
void onDeactivateDataCallIWlanResponseIndMessage(ResponseError_t response, int32_t serial, Message::Callback::Status status);

vector<hidl_string> convertAddrStringToHidlStringVector(const string &addr);
V1_4::PdpProtocolType convertStringToPdpProtocolType(const string &type);
string convertPdpProtocolTypeToString(const V1_4::PdpProtocolType protocol);

class IWlanImpl : public IIWlan {
public:

    IWlanImpl(uint8_t instance);
    ~IWlanImpl() {};
    Return<void> setResponseFunctions(
        const sp<IIWlanResponse>& iwlanResponse,
        const sp<IIWlanIndication>& iwlanIndication) override;
    virtual Return<void> setResponseFunctions_nolock(
        const sp<IIWlanResponse>& iwlanResponse,
        const sp<IIWlanIndication>& iwlanIndication);
    Return<void> setupDataCall(int32_t serial, const DataProfileInfo& dataProfile, bool allowRoaming,
                               DataRequestReason reason, const hidl_vec<hidl_string>& addresses,
                               const hidl_vec<hidl_string>& dnses) override;
    Return<void> deactivateDataCall(int32_t serial, int32_t cid, DataRequestReason reason) override;
    Return<void> getDataCallList(int32_t serial) override;
    Return<void> getDataRegistrationState(int32_t serial) override;
    Return<void> getAllQualifiedNetworks(int32_t serial) override;
    Return<void> responseAcknowledgement() override;
    Return<void> iwlanDisabled() override;
    Return<void> debug(const hidl_handle& handle, const hidl_vec<hidl_string>&) override;

    void onDataRegistrationStateChange();
    void onDataCallListChange(std::vector<DataCallResult_t> dcList);
    void onQualifiedNetworksChange(std::vector<QualifiedNetwork_t> qnList);
    void onSetupDataCallIWlanResponseIndMessage(SetupDataCallResponse_t response, int32_t serial, Message::Callback::Status status);
    void onDeactivateDataCallIWlanResponseIndMessage(ResponseError_t response, int32_t serial, Message::Callback::Status status);
    void clearResponseFunctions();
    void acknowledgeRequest(int32_t serial);

private:
    static std::mutex wakelockMutex;
    static uint8_t wakelockRefCount;
    static timer_t wakelockTimerId;

    qtimutex::QtiSharedMutex mCallbackLock;
    sp<IIWlanResponse> mIWlanResponse;
    sp<IIWlanIndication> mIWlanIndication;

    sp<IIWlanResponse> getIWlanResponse();
    sp<IIWlanIndication> getIWlanIndication();

    struct IWlanDeathRecipient : hidl_death_recipient {

        IWlanDeathRecipient(sp<IWlanImpl> setService) : mIIWlan(setService){};
        ~IWlanDeathRecipient() = default;

        virtual void serviceDied(uint64_t cookie, const wp<IBase>& who) override;
    private:
          sp<IWlanImpl> mIIWlan;
    };

    sp<IWlanDeathRecipient> iwlanDeathRecipient;

    RadioError convertMsgToRadioError(Message::Callback::Status status, ResponseError_t respErr);
    void convertDCResultToHAL(DataCallResult_t result,  SetupDataCallResult& halResult);
    void convertQualifiedNetworksToHAL(std::vector<QualifiedNetwork_t> qnList,
         ::android::hardware::hidl_vec<::vendor::qti::hardware::data::iwlan::V1_0::QualifiedNetworks>& qNetworks);
    static void acquireWakeLock();
    static void releaseWakeLock();
    static void wakeLockTimeoutHdlr(union sigval sval);
    static void resetWakeLock();
};

} //iwlan
} //data
} //hardware
} //qti
} //vendor

#endif //VENDOR_QTI_HARDWARE_DATA_IWLAN_V1_0_IIWLANIMPL_H
