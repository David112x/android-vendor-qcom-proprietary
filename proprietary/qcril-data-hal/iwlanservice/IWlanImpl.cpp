/*==============================================================================
              Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
              All Rights Reserved.
              Confidential and Proprietary - Qualcomm Technologies, Inc.
==============================================================================*/
/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/
#include <IWlanImpl.h>
#include <framework/Log.h>
#include <hardware_legacy/power.h>

#include "DataModule.h"

#include "GetAllQualifiedNetworkRequestMessage.h"
#include "GetIWlanDataRegistrationStateRequestMessage.h"
#include "GetIWlanDataCallListRequestMessage.h"
#include "DeactivateDataCallRequestMessage.h"
#include "IWLANCapabilityHandshake.h"

namespace vendor {
namespace qti {
namespace hardware {
namespace data {
namespace iwlan {

static sp<IWlanImpl> IWlanService = nullptr;
static string iwlanSvcWakelock;

std::mutex IWlanImpl::wakelockMutex;
uint8_t IWlanImpl::wakelockRefCount = 0;
timer_t IWlanImpl::wakelockTimerId = NULL;

#define IWLAN_WAKE_LOCK_NSECS 200000000

#define TAG "IWLAN"

#define SPACE_DELIMITER 0x20

/*
 * Register IWlanService service with Service Manager
 */
void IWlanServiceInit(int instanceId) {
  android::status_t ret = android::OK;
  QCRIL_LOG_INFO("IWlanServiceInit");
  IWlanService = new IWlanImpl((uint8_t)instanceId);
  std::string serviceName = "slot";
  QCRIL_LOG_INFO("IWlanServiceInit adding for slot %d", instanceId);
  ret = IWlanService->registerAsService(serviceName + std::to_string(instanceId + 1));
  QCRIL_LOG_INFO("IWlanServiceRegisterService instanceId=%d status=%d", instanceId, ret);
}

IWlanImpl::IWlanImpl(uint8_t instance) {
    iwlanSvcWakelock = "iwlan_svc_wl_" + std::to_string(instance);
}

void onDataRegistrationStateChange() {
    if (IWlanService != NULL) {
        IWlanService->onDataRegistrationStateChange();
    }
}

void onDataCallListChange(std::vector<DataCallResult_t> dcList) {
    if (IWlanService != NULL) {
        IWlanService->onDataCallListChange(dcList);
    }
}

void onQualifiedNetworksChange(std::vector<QualifiedNetwork_t> qnList) {
    if (IWlanService != NULL) {
        IWlanService->onQualifiedNetworksChange(qnList);
    }
}

void onSetupDataCallIWlanResponseIndMessage(SetupDataCallResponse_t response, int32_t serial, Message::Callback::Status status) {
    if (IWlanService != NULL) {
        IWlanService->onSetupDataCallIWlanResponseIndMessage(response, serial, status);
    }
}

void onDeactivateDataCallIWlanResponseIndMessage(ResponseError_t response, int32_t serial, Message::Callback::Status status) {
    if (IWlanService != NULL) {
        IWlanService->onDeactivateDataCallIWlanResponseIndMessage(response, serial, status);
    }
}

vector<hidl_string> convertAddrStringToHidlStringVector(const string &addr) {
    string tmpString;
    char delimiter = SPACE_DELIMITER;
    vector<hidl_string> hidlAddressesVector;
    stringstream ssAddresses(addr);
    while(getline(ssAddresses, tmpString, delimiter)) {
        hidlAddressesVector.push_back(tmpString);
    }
    return hidlAddressesVector;
}

V1_4::PdpProtocolType convertStringToPdpProtocolType(const string &type) {
    V1_4::PdpProtocolType protocolType = V1_4::PdpProtocolType::UNKNOWN;

    if (type.compare("IP") == 0) {
        protocolType = V1_4::PdpProtocolType::IP;
    }
    else if (type.compare("IPV6") == 0) {
        protocolType = V1_4::PdpProtocolType::IPV6;
    }
    else if (type.compare("IPV4V6") == 0) {
        protocolType = V1_4::PdpProtocolType::IPV4V6;
    }
    else if (type.compare("PPP") == 0) {
        protocolType = V1_4::PdpProtocolType::PPP;
    }
    else if (type.compare("NON_IP") == 0) {
        protocolType = V1_4::PdpProtocolType::NON_IP;
    }
    else if (type.compare("UNSTRUCTURED") == 0) {
        protocolType = V1_4::PdpProtocolType::UNSTRUCTURED;
    }
    else {
        protocolType = V1_4::PdpProtocolType::UNKNOWN;
    }

    return protocolType;
}

string convertPdpProtocolTypeToString(const V1_4::PdpProtocolType protocol) {
    string protocolType;
    switch(protocol) {
        case V1_4::PdpProtocolType::IP:
        protocolType = string("IP");
        break;

        case V1_4::PdpProtocolType::IPV6:
        protocolType = string("IPV6");
        break;

        case V1_4::PdpProtocolType::IPV4V6:
        protocolType = string("IPV4V6");
        break;

        case V1_4::PdpProtocolType::PPP:
        protocolType = string("PPP");
        break;

        case V1_4::PdpProtocolType::UNKNOWN:
        case V1_4::PdpProtocolType::NON_IP:
        case V1_4::PdpProtocolType::UNSTRUCTURED:
        default:
        protocolType = string("UNKNOWN");
        break;
    }
    return protocolType;
}
/*----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants and Static member initialization
 * -------------------------------------------------------------------------*/

sp<IIWlanResponse> IWlanImpl::getIWlanResponse() {
    std::shared_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
    return mIWlanResponse;
}

sp<IIWlanIndication> IWlanImpl::getIWlanIndication() {
    std::shared_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
    return mIWlanIndication;
}

/*----------------------------------------------------------------------------
 * FUNCTION      setResponseFunctions

 * DESCRIPTION   Sets the response and indication function callbacks.

 * DEPENDENCIES

 * RETURN VALUE

 * SIDE EFFECTS
 *--------------------------------------------------------------------------*/
Return<void> IWlanImpl::setResponseFunctions(
    const ::android::sp<IIWlanResponse>& iwlanResponse,
    const ::android::sp<IIWlanIndication>& iwlanIndication){

    QCRIL_LOG_INFO("IWlanImpl::setResponseFunctions");
    {
        std::unique_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
        IWlanImpl::setResponseFunctions_nolock(iwlanResponse, iwlanIndication);
    }
    return Void();
}

Return<void> IWlanImpl::setResponseFunctions_nolock(
    const ::android::sp<IIWlanResponse>& iwlanResponse,
    const ::android::sp<IIWlanIndication>& iwlanIndication){

    if (mIWlanResponse != nullptr) {
        mIWlanResponse->unlinkToDeath(iwlanDeathRecipient);
    }

    QCRIL_LOG_DEBUG("setResponseFunctions_nolock. iwlanResponseParam: %s. iwlanIndicationParam: %s",
            iwlanResponse ? "<not null>" : "<null>",
            iwlanIndication ? "<not null>" : "<null>");
    mIWlanResponse = iwlanResponse;
    mIWlanIndication = iwlanIndication;
    iwlanDeathRecipient = new IWlanDeathRecipient(this);
    iwlanResponse->linkToDeath(iwlanDeathRecipient, 0);

    //Since at least one of the services has been created, which means QTI
    //IWLAN is being used and we need to tell modem to turn on AP assist mode
    //This is for initial handshake

    auto msg =
        std::make_shared<IWLANCapabilityHandshake>(true);

    if (msg) {
          GenericCallback<ModemIWLANCapability_t> cb([this](std::shared_ptr<Message> msg,
                                                       Message::Callback::Status status,
                                                       std::shared_ptr<ModemIWLANCapability_t> rsp) -> void {
              if (msg && rsp) {

                  QCRIL_LOG_DEBUG("IWLANCapabilityHandshake cb invoked status %d ", status);

                  auto indicationCb = getIWlanIndication();
                  if (indicationCb != NULL) {

                      if ((status != Message::Callback::Status::SUCCESS) ||
                          (*rsp == ModemIWLANCapability_t::not_present)) {
                          Return<void> retStatus =
                          indicationCb->modemSupportNotPresent();
                          if (!retStatus.isOk()) {
                              QCRIL_LOG_ERROR("Unable to send support notification ind. Exception : %s",
                                              retStatus.description().c_str());
                          }
                      }


                  } else {
                      QCRIL_LOG_ERROR("%s: IWlanService ind cb is NULL", __FUNCTION__);
                  }
              }

        });
        msg->setCallback(&cb);
        msg->dispatch();
    }
    return Void();
}

/*----------------------------------------------------------------------------
 * FUNCTION      serviceDied

 * DESCRIPTION   Redirects call to class object to handle cleanup

 * DEPENDENCIES

 * RETURN VALUE

 * SIDE EFFECTS
 *--------------------------------------------------------------------------*/

void IWlanImpl::IWlanDeathRecipient::serviceDied(uint64_t cookie, const wp<IBase>& who) {
    QCRIL_LOG_DEBUG("IWlan Service Cb died. cookie: %llu, who: %p",
                  static_cast<unsigned long long>(cookie), &who);
    mIIWlan->clearResponseFunctions();
}

void IWlanImpl::clearResponseFunctions() {
    mIWlanResponse = NULL;
    mIWlanIndication = NULL;
    IWlanImpl::resetWakeLock();
}

void IWlanImpl::acknowledgeRequest(int32_t serial)
{
    QCRIL_LOG_ERROR("acknowledgeRequest %d", serial);
    auto responseCb = getIWlanResponse();
    if (responseCb != NULL) {
        Return<void> retStatus = responseCb->acknowledgeRequest(serial);
        if (!retStatus.isOk()) {
            QCRIL_LOG_ERROR("Unable to send acknowledgeRequest. Exception : %s", retStatus.description().c_str());
        }
    }
    else {
        QCRIL_LOG_ERROR("Response cb ptr is NULL");
    }
}

/*----------------------------------------------------------------------------
 * FUNCTION      setupDataCall

 * DESCRIPTION   Setup a packet data connection.

 * DEPENDENCIES

 * RETURN VALUE

 * SIDE EFFECTS
 *--------------------------------------------------------------------------*/
Return<void> IWlanImpl::setupDataCall(int32_t serial, const DataProfileInfo& dataProfile, bool allowRoaming,
                                      DataRequestReason reason, const hidl_vec<hidl_string>& addresses,
                                      const hidl_vec<hidl_string>& dnses){

    // TODO: consider to use DataProfileInfo_t in SetupDataCallRequestMessage
    std::vector<std::string> iwlanAddresses;
    std::vector<std::string> iwlanDnses;

    for (size_t i = 0 ; i < addresses.size(); i++ ) {
        iwlanAddresses.push_back(addresses[i]);
    }
    for (size_t i = 0 ; i < dnses.size(); i++ ) {
        iwlanDnses.push_back(dnses[i]);
    }

    auto msg =
      std::make_shared<SetupDataCallRequestMessage>(
        serial,
        RequestSource_t::IWLAN,
        AccessNetwork_t::IWLAN,
        (DataProfileId_t)dataProfile.profileId,
        dataProfile.apn,
        convertPdpProtocolTypeToString(dataProfile.protocol),
        convertPdpProtocolTypeToString(dataProfile.roamingProtocol),
        (ApnAuthType_t)dataProfile.authType,
        dataProfile.user,
        dataProfile.password,
        (DataProfileInfoType_t)dataProfile.type,
        dataProfile.maxConnsTime,
        dataProfile.maxConns,
        dataProfile.waitTime,
        dataProfile.enabled,
        (ApnTypes_t)dataProfile.supportedApnTypesBitmap,
        (RadioAccessFamily_t)dataProfile.bearerBitmap,
        dataProfile.mtu,
        false,
        false,
        allowRoaming,
        (DataRequestReason_t)reason,
        iwlanAddresses,
        iwlanDnses,
        std::make_shared<std::function<void(int32_t)>>(std::bind(&IWlanImpl::acknowledgeRequest, this, std::placeholders::_1)));

        if (msg) {
            GenericCallback<SetupDataCallResponse_t> cb([serial](std::shared_ptr<Message>,
                                                                 Message::Callback::Status status,
                                                                 std::shared_ptr<SetupDataCallResponse_t> rsp) -> void {
            if (rsp != nullptr) {
                auto indMsg = std::make_shared<SetupDataCallIWlanResponseIndMessage>(*rsp, serial, status);
                if (indMsg != nullptr) {
                    indMsg->broadcast();
                } else {
                    QCRIL_LOG_DEBUG("IWLAN setup data call cb failed to allocate message status %d respErr %d", status, rsp->respErr);
                }
            } else {
                QCRIL_LOG_ERROR("IWLAN setupDataCall resp is nullptr");
            }
        });
        msg->setCallback(&cb);
        msg->dispatch();
    }

    return Void();
}

/*----------------------------------------------------------------------------
 * FUNCTION      deactivateDataCall

 * DESCRIPTION   Deactivate packet data connection.

 * DEPENDENCIES

 * RETURN VALUE

 * SIDE EFFECTS
 *--------------------------------------------------------------------------*/
Return<void> IWlanImpl::deactivateDataCall(int32_t serial, int32_t cid, DataRequestReason reason){

   auto msg =
        std::make_shared<DeactivateDataCallRequestMessage>(
            serial,
            cid,
            (DataRequestReason_t)reason,
            std::make_shared<std::function<void(int32_t)>>(std::bind(&IWlanImpl::acknowledgeRequest, this, std::placeholders::_1)));
    if (msg) {
        GenericCallback<ResponseError_t> cb([serial](std::shared_ptr<Message>,
                                                     Message::Callback::Status status,
                                                     std::shared_ptr<ResponseError_t> rsp) -> void {
            if (rsp != nullptr) {
                auto indMsg = std::make_shared<DeactivateDataCallIWlanResponseIndMessage>(*rsp, serial, status);
                if (indMsg != nullptr) {
                    indMsg->broadcast();
                } else {
                    QCRIL_LOG_DEBUG("IWLAN deactivate data call cb failed to allocate message status %d respErr %d", status, *rsp);
                }
            } else {
                QCRIL_LOG_ERROR("IWLAN deactivateDataCall resp is nullptr");
            }
        });
        msg->setCallback(&cb);
        msg->dispatch();
    }

    return Void();
}

/*----------------------------------------------------------------------------
 * FUNCTION      getDataCallList

 * DESCRIPTION   Returns the data call list.

 * DEPENDENCIES

 * RETURN VALUE

 * SIDE EFFECTS
 *--------------------------------------------------------------------------*/
Return<void> IWlanImpl::getDataCallList(int32_t serial){
    auto msg = std::make_shared<GetIWlanDataCallListRequestMessage>();

    GenericCallback<DataCallListResult_t> cb(
        ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                        std::shared_ptr<DataCallListResult_t> responseDataPtr) -> void {
            if (solicitedMsg && responseDataPtr) {
                RadioError e = convertMsgToRadioError(status, responseDataPtr->respErr);
                IWlanResponseInfo responseInfo = {
                    .serial = serial, .error = e};
                QCRIL_LOG_DEBUG("getIWlanDataCallList cb invoked status %d respErr %d", status, responseDataPtr->respErr);
                QCRIL_LOG_DEBUG("Call list size = %d", responseDataPtr->call.size());
                for(size_t i=0;i<responseDataPtr->call.size();i++) {
                    DataCallResult_t *callinst = &(responseDataPtr->call[i]);
                    QCRIL_LOG_DEBUG("cid %d", callinst->cid);
                    QCRIL_LOG_DEBUG("cause %d", callinst->cause);
                    QCRIL_LOG_DEBUG("suggestedRetryTime %d", callinst->suggestedRetryTime);
                    QCRIL_LOG_DEBUG("type %s", callinst->type.c_str());
                    QCRIL_LOG_DEBUG("ifname %s", callinst->ifname.c_str());
                    QCRIL_LOG_DEBUG("addresses %s", callinst->addresses.c_str());
                    QCRIL_LOG_DEBUG("dnses %s", callinst->dnses.c_str());
                    QCRIL_LOG_DEBUG("gateways %s", callinst->gateways.c_str());
                    QCRIL_LOG_DEBUG("pcscf %s", callinst->pcscf.c_str());
                    QCRIL_LOG_DEBUG("mtu %d", callinst->mtu);
                }

                ::android::hardware::hidl_vec<SetupDataCallResult> dcResultList;
                dcResultList.resize(responseDataPtr->call.size());
                int i=0;
                for (DataCallResult_t entry: responseDataPtr->call) {
                    convertDCResultToHAL(entry, dcResultList[i++]);
                }

                auto responseCb = getIWlanResponse();
                if (responseCb != NULL) {
                    Return<void> retStatus =
                        responseCb->getDataCallListResponse(responseInfo, dcResultList);
                    if (!retStatus.isOk()) {
                        QCRIL_LOG_ERROR("Unable to send getDataCallList response. Exception : %s", retStatus.description().c_str());
                    }
                } else {
                    QCRIL_LOG_ERROR("%s: IWlanService resp cb is NULL", __FUNCTION__);
                }

            }
        }));
    if (msg) {
        msg->setCallback(&cb);
        msg->dispatch();
    }
    else {
    }
    return Void();
}

/*----------------------------------------------------------------------------
 * FUNCTION      getDataRegistrationState

 * DESCRIPTION   Returns the network registration state.

 * DEPENDENCIES

 * RETURN VALUE

 * SIDE EFFECTS
 *--------------------------------------------------------------------------*/
Return<void> IWlanImpl::getDataRegistrationState(int32_t serial){
    auto msg = std::make_shared<GetIWlanDataRegistrationStateRequestMessage>();

    GenericCallback<IWlanDataRegistrationStateResult_t> cb(
        ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                        std::shared_ptr<IWlanDataRegistrationStateResult_t> responseDataPtr) -> void {
            if (solicitedMsg && responseDataPtr) {
                RadioError e = convertMsgToRadioError(status, responseDataPtr->respErr);
                IWlanResponseInfo responseInfo = {
                    .serial = serial, .error = e};
                QCRIL_LOG_DEBUG("getDataRegistrationState cb invoked status %d respErr %d", status, responseDataPtr->respErr);

                IWlanDataRegStateResult regResult = {
                    .regState = (::android::hardware::radio::V1_0::RegState)responseDataPtr->regState,
                    .reasonForDenial = responseDataPtr->reasonForDenial};

                auto responseCb = getIWlanResponse();
                if (responseCb != NULL) {
                    Return<void> retStatus =
                        responseCb->getDataRegistrationStateResponse(responseInfo, regResult);
                    if (!retStatus.isOk()) {
                        QCRIL_LOG_ERROR("Unable to send data reg state response. Exception : %s", retStatus.description().c_str());
                    }
                } else {
                    QCRIL_LOG_ERROR("%s: IWlanService resp cb is NULL", __FUNCTION__);
                }

            }
        }));
    if (msg) {
        msg->setCallback(&cb);
        msg->dispatch();
    }
    else {
    }
    return Void();
}

/*----------------------------------------------------------------------------
 * FUNCTION      getAllQualifiedNetworks

 * DESCRIPTION   slotID for request.

 * DEPENDENCIES

 * RETURN VALUE

 * SIDE EFFECTS
 *--------------------------------------------------------------------------*/
Return<void> IWlanImpl::getAllQualifiedNetworks(int32_t serial){

    auto msg = std::make_shared<GetAllQualifiedNetworkRequestMessage>();

    GenericCallback<QualifiedNetworkResult_t> cb(
        ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                        std::shared_ptr<QualifiedNetworkResult_t> responseDataPtr) -> void {
            if (solicitedMsg && responseDataPtr) {
                RadioError e = convertMsgToRadioError(status, responseDataPtr->respErr);
                IWlanResponseInfo responseInfo = {
                    .serial = serial, .error = e};
                QCRIL_LOG_DEBUG("getAllQualifiedNetworks cb invoked status %d respErr %d", status, responseDataPtr->respErr);

                ::android::hardware::hidl_vec<::vendor::qti::hardware::data::iwlan::V1_0::QualifiedNetworks> qNetworks;
                convertQualifiedNetworksToHAL(responseDataPtr->qualifiedNetwork, qNetworks);

                auto responseCb = getIWlanResponse();
                if (responseCb != NULL) {
                    Return<void> retStatus =
                        responseCb->getAllQualifiedNetworksResponse(responseInfo, qNetworks);
                    if (!retStatus.isOk()) {
                        QCRIL_LOG_ERROR("Unable to send qualifiedNetworks response. Exception : %s", retStatus.description().c_str());
                    }
                } else {
                    QCRIL_LOG_ERROR("%s: IWlanService resp cb is NULL", __FUNCTION__);
                }

            }
        }));
    if (msg) {
        msg->setCallback(&cb);
        msg->dispatch();
    }
    else {
    }
    return Void();
}

/*----------------------------------------------------------------------------
 * FUNCTION      responseAcknowledgement

 * DESCRIPTION   acknowledge to release wakelock which is aquired for
 *               asynchronous (setupDataCall, deactivateDataCall) response

 * DEPENDENCIES

 * RETURN VALUE

 * SIDE EFFECTS
 *--------------------------------------------------------------------------*/
Return<void> IWlanImpl::responseAcknowledgement(){
    releaseWakeLock();
    return Void();
}

/*----------------------------------------------------------------------------
 * FUNCTION      iwlanDisabled

 * DESCRIPTION   Services are unbound on java, send message to modem to
 *               disabled IWLAN

 * DEPENDENCIES

 * RETURN VALUE

 * SIDE EFFECTS
 *--------------------------------------------------------------------------*/
Return<void> IWlanImpl::iwlanDisabled(){
    auto msg =
        std::make_shared<IWLANCapabilityHandshake>(false);

    if (msg) {
        msg->dispatch();
    }
    return Void();
}

Return<void> IWlanImpl::debug(const hidl_handle& handle, const hidl_vec<hidl_string>&){
    if (handle != nullptr && handle->numFds >= 1) {
        int fd = handle->data[0];
        getDataModule().dump(fd);
    }
    return Void();
}

void IWlanImpl::onDataRegistrationStateChange() {
    QCRIL_LOG_DEBUG("IWLAN Data Registration State Change indication");
    auto indicationCb = getIWlanIndication();
    if (indicationCb != NULL) {
        acquireWakeLock();
        Return<void> retStatus =
             indicationCb->dataRegistrationStateChangeIndication();
        if (!retStatus.isOk()) {
            QCRIL_LOG_ERROR("Unable to send dataRegistrationStateChange ind. Exception : %s", retStatus.description().c_str());
        }
    }else {
        QCRIL_LOG_ERROR("%s: IWlanService ind cb is NULL", __FUNCTION__);
    }
}

void IWlanImpl::onDataCallListChange(std::vector<DataCallResult_t> dcList) {
    QCRIL_LOG_DEBUG("IWLAN Data Call List Change indication");
    auto indicationCb = getIWlanIndication();
    if (indicationCb != NULL) {
        acquireWakeLock();
        ::android::hardware::hidl_vec<SetupDataCallResult> dcResultList;
        dcResultList.resize(dcList.size());
        int i=0;
        for (DataCallResult_t entry: dcList) {
            convertDCResultToHAL(entry, dcResultList[i++]);
        }
        Return<void> retStatus =
             indicationCb->dataCallListChangeIndication(dcResultList);
        if (!retStatus.isOk()) {
            QCRIL_LOG_ERROR("Unable to send dataCallListChange ind. Exception : %s", retStatus.description().c_str());
        }
    }else {
        QCRIL_LOG_ERROR("%s: IWlanService ind cb is NULL", __FUNCTION__);
    }
}

void IWlanImpl::onQualifiedNetworksChange(std::vector<QualifiedNetwork_t> qnList) {
    QCRIL_LOG_DEBUG("IWLAN Qualified Networks Change indication");
    auto indicationCb = getIWlanIndication();
    if (indicationCb != NULL) {
        acquireWakeLock();
        ::android::hardware::hidl_vec<::vendor::qti::hardware::data::iwlan::V1_0::QualifiedNetworks> qNetworks;
        convertQualifiedNetworksToHAL(qnList, qNetworks);
        Return<void> retStatus =
             indicationCb->qualifiedNetworksChangeIndication(qNetworks);
        if (!retStatus.isOk()) {
            QCRIL_LOG_ERROR("Unable to send QualifiedNetworksChange ind. Exception : %s", retStatus.description().c_str());
        }
    }else {
        QCRIL_LOG_ERROR("%s: IWlanService ind cb is NULL", __FUNCTION__);
    }
}

void IWlanImpl::onSetupDataCallIWlanResponseIndMessage(SetupDataCallResponse_t rsp, int32_t serial, Message::Callback::Status status) {
    QCRIL_LOG_DEBUG("IWLAN SetupDataCall response indication");
    auto responseCb = getIWlanResponse();
    if (responseCb != NULL) {
        acquireWakeLock();
        SetupDataCallResult dcResult = {};
        IWlanResponseInfo responseInfo {.serial = serial, .error = RadioError::NO_MEMORY};
        RadioError e = convertMsgToRadioError(status, rsp.respErr);
        QCRIL_LOG_DEBUG("status %d respErr %d", status, rsp.respErr);
        QCRIL_LOG_DEBUG("cause = %d", rsp.call.cause);
        QCRIL_LOG_DEBUG("suggestedRetryTime = %d", rsp.call.suggestedRetryTime);
        QCRIL_LOG_DEBUG("cid = %d", rsp.call.cid);
        QCRIL_LOG_DEBUG("active = %d", rsp.call.active);
        QCRIL_LOG_DEBUG("type = %s", rsp.call.type.c_str());
        QCRIL_LOG_DEBUG("ifname = %s", rsp.call.ifname.c_str());
        QCRIL_LOG_DEBUG("addresses = %s", rsp.call.addresses.c_str());
        QCRIL_LOG_DEBUG("dnses = %s", rsp.call.dnses.c_str());
        QCRIL_LOG_DEBUG("gateways = %s", rsp.call.gateways.c_str());
        QCRIL_LOG_DEBUG("pcscf = %s", rsp.call.pcscf.c_str());
        QCRIL_LOG_DEBUG("mtu = %d", rsp.call.mtu);
        convertDCResultToHAL(rsp.call, dcResult);
        responseInfo = {.serial = serial, .error = e};
        QCRIL_LOG_DEBUG("setup data call cb invoked serial %d error %d", responseInfo.serial, responseInfo.error);
        Return<void> retStatus =
                responseCb->setupDataCallResponse(responseInfo, dcResult);
        if (!retStatus.isOk()) {
            QCRIL_LOG_ERROR("Unable to send setupDataCall response. Exception : %s", retStatus.description().c_str());
        }
    }else {
        QCRIL_LOG_ERROR("%s: IWlanService response cb is NULL", __FUNCTION__);
    }
}

void IWlanImpl::onDeactivateDataCallIWlanResponseIndMessage(ResponseError_t rsp, int32_t serial, Message::Callback::Status status) {
    QCRIL_LOG_DEBUG("IWLAN DeactivateDataCall response indication");
    auto responseCb = getIWlanResponse();
    if (responseCb != NULL) {
        acquireWakeLock();
        IWlanResponseInfo responseInfo = {.serial = serial, .error = RadioError::NO_MEMORY};
        RadioError e = convertMsgToRadioError(status, rsp);
        responseInfo = {.serial = serial, .error = e};
        QCRIL_LOG_DEBUG("IWLAN deactivate data call cb invoked serial %d error %d", responseInfo.serial, responseInfo.error);
        Return<void> retStatus =
            responseCb->deactivateDataCallResponse(responseInfo);
        if (!retStatus.isOk()) {
            QCRIL_LOG_ERROR("Unable to send deactivateDataCall response. Exception : %s", retStatus.description().c_str());
        }
    }else {
        QCRIL_LOG_ERROR("%s: IWlanService response cb is NULL", __FUNCTION__);
    }
}

RadioError IWlanImpl::convertMsgToRadioError(Message::Callback::Status status, ResponseError_t respErr) {
    RadioError ret = RadioError::INTERNAL_ERR;
    if (status == Message::Callback::Status::SUCCESS) {
      switch (respErr) {
        case ResponseError_t::NO_ERROR: ret = RadioError::NONE; break;
        case ResponseError_t::NOT_SUPPORTED: ret = RadioError::REQUEST_NOT_SUPPORTED; break;
        case ResponseError_t::INVALID_ARGUMENT: ret = RadioError::INVALID_ARGUMENTS; break;
        default: break;
      }
    }
    return ret;
}

void IWlanImpl::convertDCResultToHAL(DataCallResult_t result,  SetupDataCallResult& halResult) {
    halResult.cause = (::android::hardware::radio::V1_4::DataCallFailCause) result.cause;
    halResult.suggestedRetryTime = result.suggestedRetryTime;
    halResult.cid = result.cid;
    halResult.active = (::android::hardware::radio::V1_4::DataConnActiveStatus) result.active;
    halResult.type = convertStringToPdpProtocolType(result.type);
    halResult.ifname = result.ifname;
    halResult.addresses = convertAddrStringToHidlStringVector(result.addresses);
    halResult.dnses = convertAddrStringToHidlStringVector(result.dnses);
    halResult.gateways = convertAddrStringToHidlStringVector(result.gateways);
    halResult.pcscf = convertAddrStringToHidlStringVector(result.pcscf);
    halResult.mtu = result.mtu;
}

void IWlanImpl::convertQualifiedNetworksToHAL(std::vector<QualifiedNetwork_t> qnList,
           ::android::hardware::hidl_vec<::vendor::qti::hardware::data::iwlan::V1_0::QualifiedNetworks>& qNetworks) {
    qNetworks.resize(qnList.size());
    int i=0;
    for (QualifiedNetwork_t entry: qnList) {
        qNetworks[i].apnType = (::android::hardware::radio::V1_0::ApnTypes)entry.apnType;
        qNetworks[i].networks = entry.network;
        i++;
    }
}

void IWlanImpl::acquireWakeLock() {
    wakelockMutex.lock();
    if (wakelockRefCount == 0) {
        if (acquire_wake_lock(PARTIAL_WAKE_LOCK, iwlanSvcWakelock.c_str()) < 0)
        {
            QCRIL_LOG_ERROR("%s: failed to acquire wake lock [%d:%s]",
                             __func__, errno, strerror(errno));
            wakelockMutex.unlock();
            return;

        }
    }
    wakelockRefCount++;
    //Create a new timer if required

    struct sigevent sigev;
    struct itimerspec itimers;

    memset(&sigev, 0, sizeof(struct sigevent));
    memset(&itimers, 0,sizeof(struct itimerspec));

    sigev.sigev_notify            = SIGEV_THREAD;
    sigev.sigev_notify_attributes = NULL;
    sigev.sigev_value.sival_ptr   = NULL;
    sigev.sigev_notify_function   = wakeLockTimeoutHdlr;
    if (wakelockTimerId == NULL) {
        if (-1 == timer_create(CLOCK_MONOTONIC, &sigev, &wakelockTimerId) )
        {
            QCRIL_LOG_ERROR( "failed to create wakelock timer ");
            wakelockMutex.unlock();
            return;
        }
        else
        {
            QCRIL_LOG_DEBUG( "wakelock timer creation success:");
        }
    }

    itimers.it_value.tv_sec     = 0;
    itimers.it_value.tv_nsec    = IWLAN_WAKE_LOCK_NSECS;
    itimers.it_interval.tv_sec  = 0;
    itimers.it_interval.tv_nsec = 0;

    // Start the timer, value gets overwritten if timer had already been started
    if (-1 == timer_settime(wakelockTimerId, 0, &itimers, NULL))
    {
        QCRIL_LOG_ERROR( "failed to start timer for timer_id , deleting... ");
        timer_delete(wakelockTimerId);
        wakelockTimerId = NULL;
    }
    wakelockMutex.unlock();
}

void IWlanImpl::releaseWakeLock() {
    wakelockMutex.lock();
    if (wakelockRefCount > 0) {
        wakelockRefCount--;
        if (wakelockRefCount == 0) {
            QCRIL_LOG_DEBUG( "ref count is 0, releasing wakelock");
            timer_delete(wakelockTimerId);
            wakelockTimerId = NULL;
            if (release_wake_lock(iwlanSvcWakelock.c_str()) < 0)
            {
                QCRIL_LOG_ERROR("%s: release wakelock %s failed. ",__func__,iwlanSvcWakelock.c_str() );
            }
        }
    }
    wakelockMutex.unlock();
}

void IWlanImpl::wakeLockTimeoutHdlr(union sigval sval) {
    std::ignore = sval;
    QCRIL_LOG_DEBUG( "wakelock timeout called, releasing wakelock");
    IWlanImpl::resetWakeLock();

}

void IWlanImpl::resetWakeLock() {
    wakelockMutex.lock();
    if (wakelockRefCount > 0) {
        wakelockRefCount = 0;
        timer_delete(wakelockTimerId);
        wakelockTimerId = NULL;
        if (release_wake_lock(iwlanSvcWakelock.c_str()) < 0)
        {
            QCRIL_LOG_ERROR("%s: release wakelock %s failed. ",__func__,iwlanSvcWakelock.c_str() );
        }
    }
    wakelockMutex.unlock();
}

} //iwlan
} //data
} //hardware
} //qti
} //vendor
