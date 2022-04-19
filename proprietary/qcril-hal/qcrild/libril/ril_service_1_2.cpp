/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*
* Not a Contribution.
* Apache license notifications and license are retained
* for attribution purposes only.
*/
/*
 * Copyright (c) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#define LOG_TAG "RILC"
#define TAG "RILQ_ril_1_2"

#include <memory>
#include <android/hardware/radio/1.2/IRadio.h>
#include <android/hardware/radio/1.2/IRadioResponse.h>
#include <android/hardware/radio/1.2/IRadioIndication.h>
#include <android/hardware/radio/1.2/types.h>

#include <android/hardware/radio/deprecated/1.0/IOemHook.h>

#include "ril_service_1_2.h"
#include "RilServiceModule_1_2.h"
#include <utils/SystemClock.h>
#include <log/log.h>
#include <inttypes.h>
#include <framework/Log.h>
#include <framework/Module.h>
#include <framework/QcrilInitMessage.h>
#include <framework/UnSolicitedMessage.h>
#include <request/LinkCapIndMessage.h>
#include <modules/nas/NasModule.h>

#include <interfaces/voice/QcRilRequestGetCurrentCallsMessage.h>
#include "interfaces/nas/nas_types.h"
#include "interfaces/nas/RilRequestGetVoiceRegistrationMessage.h"
#include "interfaces/nas/RilRequestGetDataRegistrationMessage.h"
#include "interfaces/nas/RilRequestStartNetworkScanMessage.h"

using rildata::LinkCapIndMessage;
using rildata::LinkCapEstimate_t;

#define INVALID_HEX_CHAR 16

using namespace android::hardware::radio;
using namespace android::hardware::radio::V1_0;
using namespace android::hardware::radio::deprecated::V1_0;
using ::android::hardware::Return;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::hidl_array;
using ::android::hardware::Void;
using android::CommandInfo;
using android::RequestInfo;
using android::requestToString;
using android::sp;
using RadioContext = RadioImpl_1_2::RadioContext;

extern bool dispatchStrings(int serial, int slotId, int request, bool allowEmpty, int countStrings, ...);
int convertRilSignalStrengthToHal(const RIL_SignalStrength& rilSignalStrength,
    V1_2::SignalStrength& signalStrength);
int convertRilSignalStrengthToHal(void *response, size_t responseLen, V1_2::SignalStrength& signalStrength);

static RadioError convertMsgToRadioError(Message::Callback::Status status, RIL_Errno e);
sp<RadioImpl_1_2> radioImpl_1_2;

void RadioImpl_1_2::clearCallbacks() {
    RLOGD("RadioImpl_1_2::clearCallbacks");
    mRadioResponseV1_2 = NULL;
    mRadioIndicationV1_2 = NULL;
    RadioImpl::clearCallbacks();
}

Return<void> RadioImpl_1_2::setResponseFunctions_nolock(
        const ::android::sp<IRadioResponse>& radioResponseParam,
        const ::android::sp<IRadioIndication>& radioIndicationParam) {
    RLOGD("RadioImpl_1_2::setResponseFunctions_nolock");

    mRadioResponseV1_2 = V1_2::IRadioResponse::castFrom(radioResponseParam).withDefault(nullptr);
    mRadioIndicationV1_2 = V1_2::IRadioIndication::castFrom(radioIndicationParam).withDefault(nullptr);

    if (mRadioResponseV1_2 == nullptr || mRadioIndicationV1_2 == nullptr) {
        mRadioResponseV1_2 = nullptr;
        mRadioIndicationV1_2 = nullptr;
    }

    QCRIL_LOG_DEBUG("radioResponseParamV1_2: %s. radioIndicationV1_2: %s",
            mRadioResponseV1_2 ? "<not null>" : "<null>",
            mRadioIndicationV1_2 ? "<not null>" : "<null>");

    RadioImpl::setResponseFunctions_nolock(
            radioResponseParam,
            radioIndicationParam);

    return Void();
}

Return<void> RadioImpl_1_2::setResponseFunctions(
        const ::android::sp<IRadioResponse>& radioResponseParam,
        const ::android::sp<IRadioIndication>& radioIndicationParam) {
    QCRIL_LOG_DEBUG("setResponseFunctions_1_2. radioResponseParam: %s. radioIndicationParam: %s",
            radioResponseParam ? "<not null>" : "<null>",
            radioIndicationParam ? "<not null>" : "<null>");
    RLOGD("setResponseFunctions_1_2");

    radioImpl_1_2 = this;

    qtimutex::QtiSharedMutex *radioServiceRwlockPtr = radio::getRadioServiceRwlock(mSlotId);
    radioServiceRwlockPtr->lock();

    RadioImpl_1_2::setResponseFunctions_nolock(
            radioResponseParam,
            radioIndicationParam);

    radioServiceRwlockPtr->unlock();

#ifndef QMI_RIL_UTF
    // client is connected. Send initial indications.
    android::onNewCommandConnect((RIL_SOCKET_ID) mSlotId);
#endif

    return Void();
}

Return<void> RadioImpl_1_2::getVoiceRegistrationState(int32_t serial) {
#if VDBG
    RLOGD("getVoiceRegistrationState: serial %d", serial);
#endif

    qtimutex::QtiSharedMutex *radioServiceRwlockPtr = radio::getRadioServiceRwlock(mSlotId);
    radioServiceRwlockPtr->lock_shared();
    if (mRadioResponseV1_2 == nullptr)
    {
      radioServiceRwlockPtr->unlock_shared();
      QCRIL_LOG_ERROR("mRadioResponseV1_2 is null , fallback to V1_0 Request");
      return  RadioImpl::getVoiceRegistrationState(serial);
    }
    radioServiceRwlockPtr->unlock_shared();

    std::shared_ptr<RadioContext> ctx = getContext(serial);
    auto msg = std::make_shared<RilRequestGetVoiceRegistrationMessage>(ctx);
    if (msg != nullptr) {
        GenericCallback<QcRilRequestMessageCallbackPayload> cb(
            [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                          std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
            if (resp != nullptr) {
                RadioResponseInfo respInfo = {};
                V1_2::VoiceRegStateResult voiceRegResponse = {};
                populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, resp->errorCode);
                auto rilRegResult = std::static_pointer_cast<qcril::interfaces::RilGetVoiceRegResult_t>(resp->data);
                if (rilRegResult != nullptr) {
                    fillVoiceRegistrationStateResponse(voiceRegResponse, rilRegResult->respData);
                }
                RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponseV1_2,
                        getVoiceRegistrationStateResponse_1_2, respInfo, voiceRegResponse);
            }
        });
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        V1_2::VoiceRegStateResult voiceRegResponse = {};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponseV1_2,
                getVoiceRegistrationStateResponse_1_2, respInfo, voiceRegResponse);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> RadioImpl_1_2::getDataRegistrationState(int32_t serial) {
#if VDBG
    RLOGD("getDataRegistrationState: serial %d", serial);
#endif

    qtimutex::QtiSharedMutex *radioServiceRwlockPtr = radio::getRadioServiceRwlock(mSlotId);
    radioServiceRwlockPtr->lock_shared();
    if (mRadioResponseV1_2 == nullptr)
    {
      radioServiceRwlockPtr->unlock_shared();
      QCRIL_LOG_ERROR("mRadioResponseV1_2 is null , fallback to V1_0 Request");
      return  RadioImpl::getDataRegistrationState(serial);
    }
    radioServiceRwlockPtr->unlock_shared();

    std::shared_ptr<RadioContext> ctx = getContext(serial);
    auto msg = std::make_shared<RilRequestGetDataRegistrationMessage>(ctx);
    if (msg != nullptr) {
        GenericCallback<QcRilRequestMessageCallbackPayload> cb(
            [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                          std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
            if (resp != nullptr) {
                RadioResponseInfo respInfo = {};
                V1_2::DataRegStateResult dataRegResponse = {};
                populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, resp->errorCode);
                auto rilRegResult = std::static_pointer_cast<qcril::interfaces::RilGetDataRegResult_t>(resp->data);
                if (rilRegResult != nullptr) {
                    fillDataRegistrationStateResponse(dataRegResponse, rilRegResult->respData);
                }
                RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponseV1_2,
                        getDataRegistrationStateResponse_1_2, respInfo, dataRegResponse);
            }
        });
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        V1_2::DataRegStateResult dataRegResponse = {};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponseV1_2,
                getDataRegistrationStateResponse_1_2, respInfo, dataRegResponse);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> RadioImpl_1_2::getSignalStrength(int32_t serial) {
#if VDBG
    RLOGD("getSignalStrength: serial %d", serial);
#endif
    qtimutex::QtiSharedMutex *radioServiceRwlockPtr = radio::getRadioServiceRwlock(mSlotId);
    radioServiceRwlockPtr->lock_shared();
    if (mRadioResponseV1_2 == nullptr)
    {
      radioServiceRwlockPtr->unlock_shared();
      QCRIL_LOG_ERROR("mRadioResponseV1_2 is null , fallback to V1_0 Request");
      return  RadioImpl::getSignalStrength(serial);
    }
    radioServiceRwlockPtr->unlock_shared();

    std::shared_ptr<RadioContext> ctx = getContext(serial);
    auto msg = std::make_shared<RilRequestGetSignalStrengthMessage>(ctx);
    if (msg != nullptr) {
        GenericCallback<QcRilRequestMessageCallbackPayload> cb(
            [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                          std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
            if (resp != nullptr) {
                RadioResponseInfo respInfo = {};
                V1_2::SignalStrength signalStrength = {};
                populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, resp->errorCode);
                auto rilSigResult = std::static_pointer_cast<qcril::interfaces::RilGetSignalStrengthResult_t>(resp->data);
                if (rilSigResult != nullptr) {
                    convertRilSignalStrengthToHal(rilSigResult->respData, signalStrength);
                }
                RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponseV1_2,
                        getSignalStrengthResponse_1_2, respInfo, signalStrength);
            }
        });
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        V1_2::SignalStrength signalStrength = {};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponseV1_2,
                getSignalStrengthResponse_1_2, respInfo, signalStrength);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

RadioError fillNetworkScanRequest_1_2(const V1_2::NetworkScanRequest& request,
        RIL_NetworkScanRequest &scanRequest) {

    if (request.mccMncs.size() > MAX_MCC_MNC_LIST_SIZE) {
        return RadioError::INVALID_ARGUMENTS;
    }

    auto res = fillNetworkScanRequest_1_1(
            (const V1_1::NetworkScanRequest&)request, scanRequest);
    if (res != RadioError::NONE) {
        return res;
    }

    scanRequest.maxSearchTime = request.maxSearchTime;
    scanRequest.incrementalResults = request.incrementalResults;
    scanRequest.incrementalResultsPeriodicity = request.incrementalResultsPeriodicity;
    scanRequest.mccMncLength = request.mccMncs.size();

    for (size_t i = 0; i < request.mccMncs.size(); ++i) {
        strlcpy(scanRequest.mccMncs[i], request.mccMncs[i].c_str(), MAX_MCC_MNC_LEN+1);
    }

    return RadioError::NONE;
}


Return<void> RadioImpl_1_2::getIccCardStatus(int32_t serial) {
#if VDBG
    RLOGD("getIccCardStatus: serial %d", serial);
#endif

    qtimutex::QtiSharedMutex *radioServiceRwlockPtr = radio::getRadioServiceRwlock(mSlotId);
    radioServiceRwlockPtr->lock_shared();

    if (mRadioResponseV1_2 == nullptr)
    {
      radioServiceRwlockPtr->unlock_shared();
      return  RadioImpl::getIccCardStatus(serial);
    }

    radioServiceRwlockPtr->unlock_shared();

    auto msg = std::make_shared<UimGetCardStatusRequestMsg>(qmi_ril_get_process_instance_id());
    if (msg) {
        GenericCallback<RIL_UIM_CardStatus> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                            std::shared_ptr<RIL_UIM_CardStatus> responseDataPtr) -> void {
                RadioResponseInfo responseInfo{RadioResponseType::SOLICITED, serial, RadioError::INTERNAL_ERR};
                V1_2::CardStatus cardStatus = {};

                if (solicitedMsg && responseDataPtr &&
                    status == Message::Callback::Status::SUCCESS) {
                    convertGetIccCardStatusResponse(responseInfo, cardStatus.base,
                            RESPONSE_SOLICITED, serial, responseDataPtr);
                    cardStatus.physicalSlotId = responseDataPtr->physical_slot_id;
                    cardStatus.atr = responseDataPtr->atr;
                    cardStatus.iccid = responseDataPtr->iccid;
                }
                RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponseV1_2, getIccCardStatusResponse_1_2,
                        responseInfo, cardStatus);
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo rsp{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        V1_2::CardStatus cardStatus = {};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponseV1_2, getIccCardStatusResponse_1_2, rsp, cardStatus);
    }
    QCRIL_LOG_FUNC_RETURN();
    return Void();
}


Return<void> RadioImpl_1_2::startNetworkScan_1_2(int32_t serial,
        const V1_2::NetworkScanRequest& request) {
#if VDBG
    RLOGD("startNetworkScan_1_2: serial %d", serial);
#endif
    std::shared_ptr<RilRequestStartNetworkScanMessage> msg;
    RIL_NetworkScanRequest scanRequest = {};
    auto res = fillNetworkScanRequest_1_2(request, scanRequest);
    if (res == RadioError::NONE) {
        std::shared_ptr<RadioContext> ctx = getContext(serial);
        if ((msg = std::make_shared<RilRequestStartNetworkScanMessage>(ctx, scanRequest)) == nullptr) {
            res = RadioError::NO_MEMORY;
        }
    }

    if (msg != nullptr) {
        GenericCallback<QcRilRequestMessageCallbackPayload> cb(
            [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                          std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
            RadioResponseInfo respInfo = {};
            if (resp != nullptr) {
                populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, resp->errorCode);
            } else {
                populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, RIL_E_NO_MEMORY);
            }
            RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponseV1_1, startNetworkScanResponse, respInfo);
        });
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, res};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponseV1_1, startNetworkScanResponse, respInfo);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}


Return<void> RadioImpl_1_2::setIndicationFilter_1_2(int32_t serial, ::android::hardware::hidl_bitfield<V1_2::IndicationFilter> indicationFilter){
    QCRIL_LOG_FUNC_ENTRY("serial=%d, indicationFilter=%d", serial, indicationFilter);
    int32_t int32Filter = indicationFilter;
    auto ctx = getContext(serial);
    auto msg = std::make_shared<NasEnablePhysChanConfigReporting>((int32Filter & V1_2::IndicationFilter::PHYSICAL_CHANNEL_CONFIG) != 0, ctx,
            [this, serial, indicationFilter]
            (std::shared_ptr<Message> msg, Message::Callback::Status status, std::shared_ptr<NasSettingResultCode> rc ) -> void {

            QCRIL_LOG_DEBUG("Callback for NasEnablePhysChanConfigReporting. rc: %d", rc?static_cast<int>(*rc):-1);
            RadioError err = convertMsgToRadioError(status,rc?static_cast<RIL_Errno>(*rc):RIL_E_INTERNAL_ERR);
            if (err != RadioError::NONE) {
                QCRIL_LOG_ERROR("Error enabling ChanConfigReporting: %d", static_cast<int>(err));
            } else {
                std::shared_ptr<NasPhysChanConfigReportingStatus> phyChanConfigReportingStatusMsg =
                    std::make_shared<NasPhysChanConfigReportingStatus>(
                        (int32_t) indicationFilter & V1_2::IndicationFilter::PHYSICAL_CHANNEL_CONFIG);
                if (phyChanConfigReportingStatusMsg) {
                    phyChanConfigReportingStatusMsg->broadcast();
                } else {
                    QCRIL_LOG_ERROR("Failed to create message NasPhysChanConfigReportingStatus.");
                }
            }

            int32_t int32Filter = indicationFilter &
                (RIL_UR_SIGNAL_STRENGTH
                 | RIL_UR_FULL_NETWORK_STATE
                 | RIL_UR_DATA_CALL_DORMANCY_CHANGED
                 | RIL_UR_LINK_CAPACITY_ESTIMATE);
            setIndicationFilter(serial, int32Filter);
    });
    msg->dispatch();
    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

RIL_Errno convertLcResultToRilError(rildata::LinkCapCriteriaResult_t result) {
    RIL_Errno ret = RIL_Errno::RIL_E_SUCCESS;

    switch(result) {
        case rildata::radio_not_available:
            ret = RIL_Errno::RIL_E_RADIO_NOT_AVAILABLE;
            break;
        case rildata::request_not_supported:
            ret = RIL_Errno::RIL_E_REQUEST_NOT_SUPPORTED;
            break;
        case rildata::internal_err:
            ret = RIL_Errno::RIL_E_INTERNAL_ERR;
            break;
        case rildata::invalid_arguments:
            ret = RIL_Errno::RIL_E_INVALID_ARGUMENTS;
            break;
        case rildata::success:
        default:
            ret = RIL_Errno::RIL_E_SUCCESS;
            break;
    }

    return ret;
}

Return<void> RadioImpl_1_2::setLinkCapacityReportingCriteria(
    int32_t serial,
    int32_t hysteresisMs,
    int32_t hysteresisDlKbps,
    int32_t hysteresisUlKbps,
    const hidl_vec<int32_t> &thresholdsDownlinkKbps,
    const hidl_vec<int32_t> &thresholdsUplinkKbps,
    V1_2::AccessNetwork ran)
{
    QCRIL_LOG_DEBUG("serial: %d, hysteresisMs: %d, hysteresisDlKbps: %d, hysteresisUlKbps: %d, ran: %d",
                    serial, hysteresisMs, hysteresisDlKbps, hysteresisUlKbps, ran);
    QCRIL_LOG_DEBUG("thresholdsDownlinkKbps: {");
    for (int thrD : thresholdsDownlinkKbps) {
        QCRIL_LOG_DEBUG("| %d", thrD);
    }
    QCRIL_LOG_DEBUG("}");
#if VDBG
    RLOGD("%s(): %d", __FUNCTION__, serial);
#endif

#ifndef QMI_RIL_UTF
    rildata::LinkCapCriteria_t c = {
        .hysteresisMs = hysteresisMs,
        .hysteresisUlKbps = hysteresisUlKbps,
        .hysteresisDlKbps = hysteresisDlKbps,
        .thresholdsUplinkKbps = thresholdsUplinkKbps,
        .thresholdsDownlinkKbps = thresholdsDownlinkKbps,
    };

    switch (ran) {
        case V1_2::AccessNetwork::GERAN:
            c.ran = rildata::RAN_t::GERAN;
            break;
        case V1_2::AccessNetwork::UTRAN:
            c.ran = rildata::RAN_t::WCDMA;
            break;
        case V1_2::AccessNetwork::EUTRAN:
            c.ran = rildata::RAN_t::LTE;
            break;
        case V1_2::AccessNetwork::CDMA2000:
            c.ran = rildata::RAN_t::CDMA;
            break;
        case V1_2::AccessNetwork::IWLAN:
            c.ran = rildata::RAN_t::IWLAN;
            break;
        default:
            Log::getInstance().d("Unknown AccessNetwork ran = "+std::to_string((int)ran));
            break;
    }

    auto msg = std::make_shared<rildata::SetLinkCapRptCriteriaMessage>(c);

    GenericCallback<rildata::LinkCapCriteriaResult_t> cb(
        ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                        std::shared_ptr<rildata::LinkCapCriteriaResult_t> responseDataPtr) -> void {
            if (solicitedMsg && responseDataPtr) {
                RIL_Errno re = convertLcResultToRilError(*responseDataPtr);
                RadioError e = convertMsgToRadioError(status, re);
                RadioResponseInfo responseInfo = {
                    .serial = serial, .type = RadioResponseType::SOLICITED, .error = e};
                Log::getInstance().d(
                    "[RilSvcDataSetLinkCapCriteriaCallback]: Callback[msg = " +
                    solicitedMsg->dump() + "] executed. client data = " +
                    "set-ril-service-cb-token status = " + std::to_string((int)status));

                qtimutex::QtiSharedMutex *radioServiceRwlockPtr = radio::getRadioServiceRwlock(mSlotId);
                radioServiceRwlockPtr->lock_shared();

                auto responseV1_2 = mRadioResponseV1_2;
                if (responseV1_2 != NULL) {
                    Return<void> retStatus =
                        responseV1_2->setLinkCapacityReportingCriteriaResponse(responseInfo);
                    checkReturnStatus(retStatus);
                } else {
                    RLOGE("%s: radioService[%d]->mRadioResponseV1_2 == NULL", __FUNCTION__,
                          mSlotId);
                }

                radioServiceRwlockPtr->unlock_shared();
            }
        }));
    msg->setCallback(&cb);
    if (msg)
        msg->dispatch();
#endif
    return Void();
}

Return<void> RadioImpl_1_2::startLceService(int32_t serial, int32_t reportInterval, bool pullMode) {
    sp<RadioImpl_1_2> ri = radioImpl_1_2;

    if( ri != NULL )
    {
      RequestInfo *pRI = android::addRequestToList(serial, ri->mSlotId, RIL_REQUEST_START_LCE);
      if (pRI != NULL) {
        sendErrorResponse(pRI, RIL_E_LCE_NOT_SUPPORTED);
      }
    } else {
        QCRIL_LOG_DEBUG("radioImpl_1_2 is null");
        return RadioImpl::startLceService(serial, reportInterval, pullMode);
    }
    return Void();
}

Return<void> RadioImpl_1_2::stopLceService(int32_t serial) {
    sp<RadioImpl_1_2> ri = radioImpl_1_2;

    if( ri != NULL )
    {
      RequestInfo *pRI = android::addRequestToList(serial, ri->mSlotId, RIL_REQUEST_STOP_LCE);
      if (pRI != NULL) {
         sendErrorResponse(pRI, RIL_E_LCE_NOT_SUPPORTED);
      }
    } else {
        QCRIL_LOG_DEBUG("radioImpl_1_2 is null");
        return RadioImpl::stopLceService(serial);
    }
    return Void();
}

Return<void> RadioImpl_1_2::pullLceData(int32_t serial) {
    sp<RadioImpl_1_2> ri = radioImpl_1_2;

    if( ri != NULL )
    {
      RequestInfo *pRI = android::addRequestToList(serial, ri->mSlotId, RIL_REQUEST_PULL_LCEDATA);
      if (pRI != NULL) {
        sendErrorResponse(pRI, RIL_E_LCE_NOT_SUPPORTED);
      }
    } else {
        QCRIL_LOG_DEBUG("radioImpl_1_2 is null");
        return RadioImpl::pullLceData(serial);
    }
    return Void();
}

RadioError convertMsgToRadioError(Message::Callback::Status status, RIL_Errno e) {
    RadioError ret = RadioError::INTERNAL_ERR;
    switch(status) {
        case Message::Callback::Status::FAILURE:
        case Message::Callback::Status::SUCCESS:
            ret = (RadioError) e;
            break;
        case Message::Callback::Status::CANCELLED:
            ret = RadioError::CANCELLED;
            break;
        case Message::Callback::Status::NO_HANDLER_FOUND:
            ret = RadioError::REQUEST_NOT_SUPPORTED;
            break;
        default:
            ret = RadioError::INTERNAL_ERR;
            break;
    }
    return ret;
}

RadioTechnology getRadioTechnologyFromAccessNetwork(V1_2::AccessNetwork accessNetwork) {

    switch(accessNetwork) {

        case V1_2::AccessNetwork::GERAN:
            return RadioTechnology::EDGE;

        case V1_2::AccessNetwork::UTRAN:
            return RadioTechnology::UMTS;

        case V1_2::AccessNetwork::EUTRAN:
            return RadioTechnology::LTE;

        case V1_2::AccessNetwork::CDMA2000:
            return RadioTechnology::IS95A;

        case V1_2::AccessNetwork::IWLAN:
            return RadioTechnology::IWLAN;

        default: {
            break;
        }
    }

    return RadioTechnology::UNKNOWN;
}

RIL_RadioAccessNetworks RadioImpl_1_2::convertHidlAccessNetworkToRilAccessNetwork(V1_2::AccessNetwork ran)
{
    RIL_RadioAccessNetworks ret = RIL_RadioAccessNetworks::UNKNOWN;
    switch(ran) {
        case V1_2::AccessNetwork::GERAN:
            ret = RIL_RadioAccessNetworks::GERAN;
            break;
        case V1_2::AccessNetwork::UTRAN:
            ret = RIL_RadioAccessNetworks::UTRAN;
            break;
        case V1_2::AccessNetwork::EUTRAN:
            ret = RIL_RadioAccessNetworks::EUTRAN;
            break;
        case V1_2::AccessNetwork::CDMA2000:
            ret = RIL_RadioAccessNetworks::CDMA;
            break;
        case V1_2::AccessNetwork::IWLAN:
            ret = RIL_RadioAccessNetworks::IWLAN;
            break;
        default:
            ret = RIL_RadioAccessNetworks::UNKNOWN;
            break;
    }
    return ret;
}
void RadioImpl_1_2::fillInSignalStrengthCriteria(std::vector<SignalStrengthCriteriaEntry> &out,
        int32_t hysteresisMs,
        int32_t hysteresisDb,
        const hidl_vec<int32_t> &thresholdsDbm,
        V1_2::AccessNetwork ran)
{
    RIL_RadioAccessNetworks ril_ran = convertHidlAccessNetworkToRilAccessNetwork(ran);
    out.push_back({ril_ran, hysteresisDb, thresholdsDbm});
}

RadioError sanityCheckSignalStrengthCriteriaParams(
    int32_t hysteresisMs,
    int32_t hysteresisDb,
    const hidl_vec<int32_t> &thresholdsDbm,
    V1_2::AccessNetwork ran)
{
    int i = 0;
    int size = thresholdsDbm.size();
    int diff = INT_MAX;
    RadioError ret = RadioError::NONE;

    QCRIL_LOG_FUNC_ENTRY();

    QCRIL_LOG_DEBUG("hysteresisMs: %d. hysteresisDb", hysteresisMs, hysteresisDb);

    // hysteresisDb must be smaller than the smallest threshold delta.
    for (i=0; i < size-1; i++)
    {
       if (abs(thresholdsDbm[i+1] - thresholdsDbm[i]) < diff)
        {
           diff = abs(thresholdsDbm[i+1] - thresholdsDbm[i]);
        }
    }

    QCRIL_LOG_DEBUG("diff: %d", diff);

    if((hysteresisDb != 0) && (diff <= hysteresisDb))
    {
        ret = RadioError::INVALID_ARGUMENTS;
    }

    QCRIL_LOG_FUNC_RETURN_WITH_RET(ret);
    return ret;
}

std::shared_ptr<RadioContext> RadioImpl_1_2::getContext(int32_t serial)
{
    std::shared_ptr<RadioContext> ctx;
    ctx = std::make_shared<RadioContext>(sp<RadioImpl_1_2>(this), serial);

    return ctx;
}

Return<void> RadioImpl_1_2::setSignalStrengthReportingCriteria(
    int32_t serial,
    int32_t hysteresisMs,
    int32_t hysteresisDb,
    const hidl_vec<int32_t> &thresholdsDbm,
    V1_2::AccessNetwork ran)
{
    QCRIL_LOG_FUNC_ENTRY();
    RadioError ret = RadioError::NONE;
    std::vector<SignalStrengthCriteriaEntry> args;
    std::shared_ptr<RadioContext> ctx = getContext(serial);
    QCRIL_LOG_DEBUG("New context with serial %d: %s (%s)", serial, ctx ? "<not null>" : "<null>",
                    ctx ? ctx->toString().c_str() : "<null>");

    if (RadioError::NONE ==
        sanityCheckSignalStrengthCriteriaParams(hysteresisMs, hysteresisDb, thresholdsDbm, ran))
    {
        fillInSignalStrengthCriteria(args, hysteresisMs, hysteresisDb, thresholdsDbm, ran);

        std::shared_ptr<NasSetSignalStrengthCriteria> msg =
            std::make_shared<NasSetSignalStrengthCriteria>(ctx, args);

        if (msg) {
            GenericCallback<RIL_Errno> cb([this, serial](std::shared_ptr<Message> msg,
                                                         Message::Callback::Status status,
                                                         std::shared_ptr<RIL_Errno> rsp) -> void {
                RadioError err = convertMsgToRadioError(status, rsp ? *rsp : RIL_E_INTERNAL_ERR);
                RadioResponseInfo info{RadioResponseType::SOLICITED, serial, err};
                QCRIL_LOG_DEBUG("serial: %d. status: %d rsp: %d. result: %d", serial, (int)status,
                                rsp ? *rsp : 0, (int)err);

                qtimutex::QtiSharedMutex *radioServiceRwlockPtr = radio::getRadioServiceRwlock(mSlotId);
                radioServiceRwlockPtr->lock_shared();

                auto response = mRadioResponseV1_2;
                if (response) {
                    Return<void> retStatus = response->setSignalStrengthReportingCriteriaResponse(info);
                    checkReturnStatus(retStatus);
                } else {
                    QCRIL_LOG_ERROR("%s: radioService[%d]->mRadioResponseV1_2 == NULL", __FUNCTION__,
                                    mSlotId);
                }

                radioServiceRwlockPtr->unlock_shared();
            });
            msg->setCallback(&cb);
            msg->dispatch();
        } else {
            ret = RadioError::NO_MEMORY;
        }
    }
    else
    {
        ret = RadioError::INVALID_ARGUMENTS;
        QCRIL_LOG_DEBUG("Invalid arguments passed");
    }


    if (ret != RadioError::NONE)
    {
        qtimutex::QtiSharedMutex *radioServiceRwlockPtr = radio::getRadioServiceRwlock(mSlotId);
        radioServiceRwlockPtr->lock_shared();

        auto response = mRadioResponseV1_2;
        if (response) {
            RadioResponseInfo rsp{RadioResponseType::SOLICITED, serial, ret};
            Return<void> retStatus = response->setSignalStrengthReportingCriteriaResponse(rsp);
            checkReturnStatus(retStatus);
        } else {
            QCRIL_LOG_ERROR("%s: radioService[%d]->mRadioResponseV1_2 == NULL", __FUNCTION__,
                            mSlotId);
        }

        radioServiceRwlockPtr->unlock_shared();
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

static bool convertMvnoTypeToString(MvnoType type, char *&str) {
    switch (type) {
        case MvnoType::IMSI:
            str = (char *)"imsi";
            return true;
        case MvnoType::GID:
            str = (char *)"gid";
            return true;
        case MvnoType::SPN:
            str = (char *)"spn";
            return true;
        case MvnoType::NONE:
            str = (char *)"";
            return true;
    }
    return false;
}

Return<void> RadioImpl_1_2::setupDataCall_1_2(int32_t serial, V1_2::AccessNetwork accessNetwork, const V1_0::DataProfileInfo& dataProfileInfo, bool modemCognitive, bool roamingAllowed, bool isRoaming, V1_2::DataRequestReason reason, const hidl_vec<hidl_string>& addresses, const hidl_vec<hidl_string>& dnses) {
#if VDBG
    RLOGD("setupDataCall_1_2: serial %d", serial);
#endif
    if (s_vendorFunctions->version >= 15) {
        char *mvnoTypeStr = NULL;
        if (!convertMvnoTypeToString(dataProfileInfo.mvnoType, mvnoTypeStr)) {
            RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
                    RIL_REQUEST_SETUP_DATA_CALL);
            if (pRI != NULL) {
                sendErrorResponse(pRI, RIL_E_INVALID_ARGUMENTS);
            }
            return Void();
        }

        std::string IpAddresses("");
        for(size_t j = 0; j < addresses.size(); j++) {
            IpAddresses += addresses[j];
            if (j+1<addresses.size()) {IpAddresses += " ";}
        }
        std::string DnsAddresses("");
        for(size_t j = 0; j < dnses.size(); j++) {
            DnsAddresses += dnses[j];
            if (j+1<dnses.size()) {DnsAddresses += " ";}
        }

        dispatchStrings(serial, mSlotId, RIL_REQUEST_SETUP_DATA_CALL, true, 19,
            std::to_string((int) RadioTechnology::UNKNOWN + 2).c_str(),
            std::to_string((int) dataProfileInfo.profileId).c_str(),
            dataProfileInfo.apn.c_str(),
            dataProfileInfo.user.c_str(),
            dataProfileInfo.password.c_str(),
            std::to_string((int) dataProfileInfo.authType).c_str(),
            dataProfileInfo.protocol.c_str(),
            dataProfileInfo.roamingProtocol.c_str(),
            std::to_string(dataProfileInfo.supportedApnTypesBitmap).c_str(),
            std::to_string(dataProfileInfo.bearerBitmap).c_str(),
            modemCognitive ? "1" : "0",
            std::to_string(dataProfileInfo.mtu).c_str(),
            mvnoTypeStr,
            dataProfileInfo.mvnoMatchData.c_str(),
            roamingAllowed ? "1" : "0",
            std::to_string((int) accessNetwork).c_str(),
            std::to_string((int) reason).c_str(),
            IpAddresses.c_str(),
            DnsAddresses.c_str());
    } else {
        RLOGE("Unsupported RIL version %d", s_vendorFunctions->version);
        RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
                RIL_REQUEST_SETUP_DATA_CALL);
        if (pRI != NULL) {
            sendErrorResponse(pRI, RIL_E_REQUEST_NOT_SUPPORTED);
        }
    }
    return Void();
}

Return<void> RadioImpl_1_2::deactivateDataCall_1_2(int32_t serial, int32_t cid, V1_2::DataRequestReason reason) {
#if VDBG
    RLOGD("deactivateDataCall_1_2: serial %d", serial);
#endif
    bool isRadioShutdown = false;

    if (V1_2::DataRequestReason::SHUTDOWN == reason) {
        isRadioShutdown = true;
    }

    dispatchStrings(serial, mSlotId, RIL_REQUEST_DEACTIVATE_DATA_CALL, true, 3,
        std::to_string(cid).c_str(),
        isRadioShutdown ? "1" : "0",
        std::to_string((int) reason).c_str());
    return Void();
}

Return<void> RadioImpl_1_2::getCurrentCalls(int32_t serial) {
#if VDBG
  RLOGD("getCurrentCalls: serial %d", serial);
#endif

  qtimutex::QtiSharedMutex *radioServiceRwlockPtr = radio::getRadioServiceRwlock(mSlotId);
  radioServiceRwlockPtr->lock_shared();
  if (mRadioResponseV1_2 == nullptr)
  {
    radioServiceRwlockPtr->unlock_shared();
    QCRIL_LOG_ERROR("mRadioResponseV1_2 is null , fallback to V1_0 Request");
    return  RadioImpl::getCurrentCalls(serial);
  }
  radioServiceRwlockPtr->unlock_shared();

  QCRIL_LOG_FUNC_ENTRY();
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<QcRilRequestGetCurrentCallsMessage>(getContext(serial));
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                       std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          hidl_vec<V1_2::Call> calls;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
            auto currentCalls =
                std::static_pointer_cast<qcril::interfaces::GetCurrentCallsRespData>(resp->data);
            if (currentCalls) {
              auto callList = currentCalls->getCallInfoList();
              if (!callList.empty()) {
                calls.resize(callList.size());
                for (uint32_t i = 0; i < callList.size(); i++) {
                  convertToHidl(calls[i], callList[i]);
                }
              }
            }
          }
          RadioResponseInfo respInfo = {};
          populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, errorCode);
          QCRIL_LOG_ERROR("getCurrentCallsResponse_1_2 : calls = %s", toString(calls).c_str());
          RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponseV1_2, getCurrentCallsResponse_1_2, respInfo,
                                   calls);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    hidl_vec<V1_2::Call> calls;
    RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
    RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponseV1_2, getCurrentCallsResponse_1_2, respInfo, calls);
  }
  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

/***************************************************************************************************
 * RESPONSE FUNCTIONS
 * Functions above are used for requests going from framework to vendor code. The ones below are
 * responses for those requests coming back from the vendor code.
 **************************************************************************************************/

void convertRilCellInfoToHal_1_2(V1_2::CellInfo &hidlCellInfo, const RIL_CellInfo_v12 &rilCellInfo) {
        switch(rilCellInfo.cellInfoType) {
            case RIL_CELL_INFO_TYPE_GSM: {
                hidlCellInfo.gsm.resize(1);
                V1_2::CellInfoGsm *cellInfoGsm = &hidlCellInfo.gsm[0];

                new (&cellInfoGsm->cellIdentityGsm.base.mcc) hidl_string(
                        rilCellInfo.CellInfo.gsm.cellIdentityGsm.mcc,
                        strnlen(rilCellInfo.CellInfo.gsm.cellIdentityGsm.mcc,
                                sizeof(rilCellInfo.CellInfo.gsm.cellIdentityGsm.mcc) - 1));

                new (&cellInfoGsm->cellIdentityGsm.base.mnc) hidl_string(
                        rilCellInfo.CellInfo.gsm.cellIdentityGsm.mnc,
                        strnlen(rilCellInfo.CellInfo.gsm.cellIdentityGsm.mnc,
                                sizeof(rilCellInfo.CellInfo.gsm.cellIdentityGsm.mnc) - 1));

                cellInfoGsm->cellIdentityGsm.base.lac =
                        rilCellInfo.CellInfo.gsm.cellIdentityGsm.lac;
                cellInfoGsm->cellIdentityGsm.base.cid =
                        rilCellInfo.CellInfo.gsm.cellIdentityGsm.cid;
                cellInfoGsm->cellIdentityGsm.base.arfcn =
                        rilCellInfo.CellInfo.gsm.cellIdentityGsm.arfcn;
                cellInfoGsm->cellIdentityGsm.base.bsic =
                        rilCellInfo.CellInfo.gsm.cellIdentityGsm.bsic;
                cellInfoGsm->signalStrengthGsm.signalStrength =
                        rilCellInfo.CellInfo.gsm.signalStrengthGsm.signalStrength;
                cellInfoGsm->signalStrengthGsm.bitErrorRate =
                        rilCellInfo.CellInfo.gsm.signalStrengthGsm.bitErrorRate;
                cellInfoGsm->signalStrengthGsm.timingAdvance =
                        rilCellInfo.CellInfo.gsm.signalStrengthGsm.timingAdvance;

                new (&cellInfoGsm->cellIdentityGsm.operatorNames.alphaLong) hidl_string(
                        rilCellInfo.CellInfo.gsm.cellIdentityGsm.operatorNames.alphaLong,
                        strnlen(rilCellInfo.CellInfo.gsm.cellIdentityGsm.operatorNames.alphaLong,
                                sizeof(rilCellInfo.CellInfo.gsm.cellIdentityGsm.operatorNames.alphaLong) - 1));

                new (&cellInfoGsm->cellIdentityGsm.operatorNames.alphaShort) hidl_string(
                        rilCellInfo.CellInfo.gsm.cellIdentityGsm.operatorNames.alphaShort,
                        strnlen(rilCellInfo.CellInfo.gsm.cellIdentityGsm.operatorNames.alphaShort,
                                sizeof(rilCellInfo.CellInfo.gsm.cellIdentityGsm.operatorNames.alphaShort) - 1));
                break;
            }

            case RIL_CELL_INFO_TYPE_WCDMA: {
                hidlCellInfo.wcdma.resize(1);
                V1_2::CellInfoWcdma *cellInfoWcdma = &hidlCellInfo.wcdma[0];

                new (&cellInfoWcdma->cellIdentityWcdma.base.mcc) hidl_string(
                        rilCellInfo.CellInfo.wcdma.cellIdentityWcdma.mcc,
                        strnlen(rilCellInfo.CellInfo.wcdma.cellIdentityWcdma.mcc,
                                sizeof(rilCellInfo.CellInfo.wcdma.cellIdentityWcdma.mcc) - 1));

                new (&cellInfoWcdma->cellIdentityWcdma.base.mnc) hidl_string(
                        rilCellInfo.CellInfo.wcdma.cellIdentityWcdma.mnc,
                        strnlen(rilCellInfo.CellInfo.wcdma.cellIdentityWcdma.mnc,
                                sizeof(rilCellInfo.CellInfo.wcdma.cellIdentityWcdma.mnc) - 1));

                cellInfoWcdma->cellIdentityWcdma.base.lac =
                        rilCellInfo.CellInfo.wcdma.cellIdentityWcdma.lac;
                cellInfoWcdma->cellIdentityWcdma.base.cid =
                        rilCellInfo.CellInfo.wcdma.cellIdentityWcdma.cid;
                cellInfoWcdma->cellIdentityWcdma.base.psc =
                        rilCellInfo.CellInfo.wcdma.cellIdentityWcdma.psc;
                cellInfoWcdma->cellIdentityWcdma.base.uarfcn =
                        rilCellInfo.CellInfo.wcdma.cellIdentityWcdma.uarfcn;
                cellInfoWcdma->signalStrengthWcdma.base.signalStrength =
                        rilCellInfo.CellInfo.wcdma.signalStrengthWcdma.signalStrength;
                cellInfoWcdma->signalStrengthWcdma.base.bitErrorRate =
                        rilCellInfo.CellInfo.wcdma.signalStrengthWcdma.bitErrorRate;
                cellInfoWcdma->signalStrengthWcdma.rscp =
                        rilCellInfo.CellInfo.wcdma.signalStrengthWcdma.rscp;
                cellInfoWcdma->signalStrengthWcdma.ecno =
                        rilCellInfo.CellInfo.wcdma.signalStrengthWcdma.ecio;

                new (&cellInfoWcdma->cellIdentityWcdma.operatorNames.alphaLong) hidl_string(
                        rilCellInfo.CellInfo.wcdma.cellIdentityWcdma.operatorNames.alphaLong,
                        strnlen(rilCellInfo.CellInfo.wcdma.cellIdentityWcdma.operatorNames.alphaLong,
                                sizeof(rilCellInfo.CellInfo.wcdma.cellIdentityWcdma.operatorNames.alphaLong) - 1));

                new (&cellInfoWcdma->cellIdentityWcdma.operatorNames.alphaShort) hidl_string(
                        rilCellInfo.CellInfo.wcdma.cellIdentityWcdma.operatorNames.alphaShort,
                        strnlen(rilCellInfo.CellInfo.wcdma.cellIdentityWcdma.operatorNames.alphaShort,
                                sizeof(rilCellInfo.CellInfo.wcdma.cellIdentityWcdma.operatorNames.alphaShort) - 1));
                break;
            }

            case RIL_CELL_INFO_TYPE_CDMA: {
                hidlCellInfo.cdma.resize(1);
                V1_2::CellInfoCdma *cellInfoCdma = &hidlCellInfo.cdma[0];
                cellInfoCdma->cellIdentityCdma.base.networkId =
                        rilCellInfo.CellInfo.cdma.cellIdentityCdma.networkId;
                cellInfoCdma->cellIdentityCdma.base.systemId =
                        rilCellInfo.CellInfo.cdma.cellIdentityCdma.systemId;
                cellInfoCdma->cellIdentityCdma.base.baseStationId =
                        rilCellInfo.CellInfo.cdma.cellIdentityCdma.basestationId;
                cellInfoCdma->cellIdentityCdma.base.longitude =
                        rilCellInfo.CellInfo.cdma.cellIdentityCdma.longitude;
                cellInfoCdma->cellIdentityCdma.base.latitude =
                        rilCellInfo.CellInfo.cdma.cellIdentityCdma.latitude;
                cellInfoCdma->signalStrengthCdma.dbm =
                        rilCellInfo.CellInfo.cdma.signalStrengthCdma.dbm;
                cellInfoCdma->signalStrengthCdma.ecio =
                        rilCellInfo.CellInfo.cdma.signalStrengthCdma.ecio;
                cellInfoCdma->signalStrengthEvdo.dbm =
                        rilCellInfo.CellInfo.cdma.signalStrengthEvdo.dbm;
                cellInfoCdma->signalStrengthEvdo.ecio =
                        rilCellInfo.CellInfo.cdma.signalStrengthEvdo.ecio;
                cellInfoCdma->signalStrengthEvdo.signalNoiseRatio =
                        rilCellInfo.CellInfo.cdma.signalStrengthEvdo.signalNoiseRatio;

                new (&cellInfoCdma->cellIdentityCdma.operatorNames.alphaLong) hidl_string(
                        rilCellInfo.CellInfo.cdma.cellIdentityCdma.operatorNames.alphaLong,
                        strnlen(rilCellInfo.CellInfo.cdma.cellIdentityCdma.operatorNames.alphaLong,
                                sizeof(rilCellInfo.CellInfo.cdma.cellIdentityCdma.operatorNames.alphaLong) - 1));

                new (&cellInfoCdma->cellIdentityCdma.operatorNames.alphaShort) hidl_string(
                        rilCellInfo.CellInfo.cdma.cellIdentityCdma.operatorNames.alphaShort,
                        strnlen(rilCellInfo.CellInfo.cdma.cellIdentityCdma.operatorNames.alphaShort,
                                sizeof(rilCellInfo.CellInfo.cdma.cellIdentityCdma.operatorNames.alphaShort) - 1));
                break;
            }

            case RIL_CELL_INFO_TYPE_LTE: {
                hidlCellInfo.lte.resize(1);
                V1_2::CellInfoLte *cellInfoLte = &hidlCellInfo.lte[0];

                new (&cellInfoLte->cellIdentityLte.base.mcc) hidl_string(
                        rilCellInfo.CellInfo.lte.cellIdentityLte.mcc,
                        strnlen(rilCellInfo.CellInfo.lte.cellIdentityLte.mcc,
                                sizeof(rilCellInfo.CellInfo.lte.cellIdentityLte.mcc) - 1));

                new (&cellInfoLte->cellIdentityLte.base.mnc) hidl_string(
                        rilCellInfo.CellInfo.lte.cellIdentityLte.mnc,
                        strnlen(rilCellInfo.CellInfo.lte.cellIdentityLte.mnc,
                                sizeof(rilCellInfo.CellInfo.lte.cellIdentityLte.mnc) - 1));

                cellInfoLte->cellIdentityLte.base.ci =
                        rilCellInfo.CellInfo.lte.cellIdentityLte.ci;
                cellInfoLte->cellIdentityLte.base.pci =
                        rilCellInfo.CellInfo.lte.cellIdentityLte.pci;
                cellInfoLte->cellIdentityLte.base.tac =
                        rilCellInfo.CellInfo.lte.cellIdentityLte.tac;
                cellInfoLte->cellIdentityLte.base.earfcn =
                        rilCellInfo.CellInfo.lte.cellIdentityLte.earfcn;
                cellInfoLte->signalStrengthLte.signalStrength =
                        rilCellInfo.CellInfo.lte.signalStrengthLte.signalStrength;
                cellInfoLte->signalStrengthLte.rsrp =
                        rilCellInfo.CellInfo.lte.signalStrengthLte.rsrp;
                cellInfoLte->signalStrengthLte.rsrq =
                        rilCellInfo.CellInfo.lte.signalStrengthLte.rsrq;
                cellInfoLte->signalStrengthLte.rssnr =
                        rilCellInfo.CellInfo.lte.signalStrengthLte.rssnr;
                cellInfoLte->signalStrengthLte.cqi =
                        rilCellInfo.CellInfo.lte.signalStrengthLte.cqi;
                cellInfoLte->signalStrengthLte.timingAdvance =
                        rilCellInfo.CellInfo.lte.signalStrengthLte.timingAdvance;

                new (&cellInfoLte->cellIdentityLte.operatorNames.alphaLong) hidl_string(
                        rilCellInfo.CellInfo.lte.cellIdentityLte.operatorNames.alphaLong,
                        strnlen(rilCellInfo.CellInfo.lte.cellIdentityLte.operatorNames.alphaLong,
                                sizeof(rilCellInfo.CellInfo.lte.cellIdentityLte.operatorNames.alphaLong) - 1));

                new (&cellInfoLte->cellIdentityLte.operatorNames.alphaShort) hidl_string(
                        rilCellInfo.CellInfo.lte.cellIdentityLte.operatorNames.alphaShort,
                        strnlen(rilCellInfo.CellInfo.lte.cellIdentityLte.operatorNames.alphaShort,
                                sizeof(rilCellInfo.CellInfo.lte.cellIdentityLte.operatorNames.alphaShort) - 1));

                cellInfoLte->cellIdentityLte.bandwidth =
                    rilCellInfo.CellInfo.lte.cellIdentityLte.bandwidth;
                break;
            }

            case RIL_CELL_INFO_TYPE_TD_SCDMA: {
                hidlCellInfo.tdscdma.resize(1);
                V1_2::CellInfoTdscdma *cellInfoTdscdma = &hidlCellInfo.tdscdma[0];

                new (&cellInfoTdscdma->cellIdentityTdscdma.base.mcc) hidl_string(
                        rilCellInfo.CellInfo.tdscdma.cellIdentityTdscdma.mcc,
                        strnlen(rilCellInfo.CellInfo.tdscdma.cellIdentityTdscdma.mcc,
                                sizeof(rilCellInfo.CellInfo.tdscdma.cellIdentityTdscdma.mcc) - 1));

                new (&cellInfoTdscdma->cellIdentityTdscdma.base.mnc) hidl_string(
                        rilCellInfo.CellInfo.tdscdma.cellIdentityTdscdma.mnc,
                        strnlen(rilCellInfo.CellInfo.tdscdma.cellIdentityTdscdma.mnc,
                                sizeof(rilCellInfo.CellInfo.tdscdma.cellIdentityTdscdma.mnc) - 1));

                cellInfoTdscdma->cellIdentityTdscdma.base.lac =
                        rilCellInfo.CellInfo.tdscdma.cellIdentityTdscdma.lac;
                cellInfoTdscdma->cellIdentityTdscdma.base.cid =
                        rilCellInfo.CellInfo.tdscdma.cellIdentityTdscdma.cid;
                cellInfoTdscdma->cellIdentityTdscdma.base.cpid =
                        rilCellInfo.CellInfo.tdscdma.cellIdentityTdscdma.cpid;
                cellInfoTdscdma->signalStrengthTdscdma.rscp =
                        rilCellInfo.CellInfo.tdscdma.signalStrengthTdscdma.rscp;
                break;
            }

            default: {
                break;
            }
        }
}

void convertRilCellInfoListToHal_1_2(int slotId, void *response, size_t responseLen, hidl_vec<V1_2::CellInfo>& records) {
    int num = responseLen / sizeof(RIL_CellInfo_v12);
    records.resize(num);

    RIL_CellInfo_v12 *rillCellInfo = (RIL_CellInfo_v12 *) response;
    if (rillCellInfo) {
        for (int i = 0; i < num; i++) {
            records[i].cellInfoType = (CellInfoType) rillCellInfo->cellInfoType;
            records[i].registered = rillCellInfo->registered;
            records[i].timeStampType = (TimeStampType) rillCellInfo->timeStampType;
            records[i].timeStamp = rillCellInfo->timeStamp;
            records[i].connectionStatus = (V1_2::CellConnectionStatus ) rillCellInfo->connStatus;
            // All vectors should be size 0 except one which will be size 1. Set everything to
            // size 0 initially.
            records[i].gsm.resize(0);
            records[i].wcdma.resize(0);
            records[i].cdma.resize(0);
            records[i].lte.resize(0);
            records[i].tdscdma.resize(0);

            convertRilCellInfoToHal_1_2(records[i], *rillCellInfo);

            rillCellInfo += 1;
        }
    }
}

void convertRilCellInfoListToHal_1_2(const std::vector<RIL_CellInfo_v12>& rillCellInfo, hidl_vec<V1_2::CellInfo>& records) {
    auto num = rillCellInfo.size();
    records.resize(num);

    for (unsigned int i = 0; i < num; i++) {
        records[i].cellInfoType = (CellInfoType) rillCellInfo[i].cellInfoType;
        records[i].registered = rillCellInfo[i].registered;
        records[i].timeStampType = (TimeStampType) rillCellInfo[i].timeStampType;
        records[i].timeStamp = rillCellInfo[i].timeStamp;
        records[i].connectionStatus = (V1_2::CellConnectionStatus ) rillCellInfo[i].connStatus;
        // All vectors should be size 0 except one which will be size 1. Set everything to
        // size 0 initially.
        records[i].gsm.resize(0);
        records[i].wcdma.resize(0);
        records[i].cdma.resize(0);
        records[i].lte.resize(0);
        records[i].tdscdma.resize(0);

        convertRilCellInfoToHal_1_2(records[i], rillCellInfo[i]);
    }
}

int RadioImpl_1_2::sendGetCellInfoListResponse(int slotId,
                                   int responseType, int serial, RIL_Errno e,
                                   void *response, size_t responseLen) {
#if VDBG
    RLOGD("getCellInfoListResponse: serial %d", serial);
#endif
    auto rsp = mRadioResponseV1_2;
    if (rsp) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);

        hidl_vec<CellInfo> ret;
        hidl_vec<V1_2::CellInfo> ret_1_2;
        if ((response == NULL && responseLen != 0)
                || responseLen % sizeof(RIL_CellInfo_v12) != 0) {
            RLOGE("getCellInfoListResponse: Invalid response");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            convertRilCellInfoListToHal_1_2(slotId, response, responseLen, ret_1_2);
        }

        Return<void> retStatus;
            retStatus = rsp->getCellInfoListResponse_1_2(
                    responseInfo, ret_1_2);
            checkReturnStatus(retStatus);

    } else {
        RadioImpl::sendGetCellInfoListResponse(slotId,
                responseType,
                serial,
                e,
                response,
                responseLen);
    }
    return 0;
}

int RadioImpl_1_2::sendCellInfoListInd(int slotId,
        int indicationType,
        int token,
        RIL_Errno e,
        void *response,
        size_t responseLen)
{
    if ( mRadioIndicationV1_2 ) {
            hidl_vec<V1_2::CellInfo> records_1_2;

            convertRilCellInfoListToHal_1_2(slotId, response, responseLen, records_1_2);
            Return<void> retStatus = mRadioIndicationV1_2->cellInfoList_1_2(
                    convertIntToRadioIndicationType(indicationType), records_1_2);
            checkReturnStatus(retStatus);
    } else {
        RLOGE("cellInfoListInd: radioService[%d]->mRadioIndication == NULL", slotId);
        RadioImpl::sendCellInfoListInd(slotId,
                indicationType,
                token,
                e,
                response,
                responseLen);
    }
    return 0;
}
int radio::cellInfoListInd(int slotId,
                           int indicationType, int token, RIL_Errno e, void *response,
                           size_t responseLen) {
    auto rs = getRadioService(static_cast<qcril_instance_id_e_type>(slotId));
    int ret = 1;
    if (rs != NULL) {
        ret = rs->sendCellInfoListInd(slotId,
                indicationType,
                token,
                e,
                response,
                responseLen);

    } else {
        RLOGE("cellInfoListInd: radioService[%d] == NULL", slotId);
    }

    return 0;
}

int RadioImpl_1_2::sendNetworkScanResult(
        std::shared_ptr<RilUnsolNetworkScanResultMessage> msg) {

    auto ind = mRadioIndicationV1_2;
    QCRIL_LOG_INFO("ind null: %s", ind ? "false" : "true" );

    if (ind) {
        V1_2::NetworkScanResult resultV1_2;
        resultV1_2.status = (V1_1::ScanStatus)msg->getStatus();
        resultV1_2.error = (RadioError)msg->getError();

        convertRilCellInfoListToHal_1_2(msg->getNetworkInfo(), resultV1_2.networkInfos);
        Return<void> retStatus = ind->networkScanResult_1_2(
                RadioIndicationType::UNSOLICITED, resultV1_2);
        checkReturnStatus(retStatus);
    } else {
        RadioImpl::sendNetworkScanResult(msg);
    }
    return 0;
}

int RadioImpl_1_2::networkScanResultInd
(
 int slotId,
 int indicationType,
 int token,
 RIL_Errno e,
 void *response,
 size_t responseLen)
{
    V1_2::NetworkScanResult resultV1_2;
    Return<void> retStatus;
    RIL_NetworkScanResult *networkScanResult = (RIL_NetworkScanResult *) response;

    if ( mRadioIndicationV1_2 != NULL ) {
        resultV1_2.status = (V1_1::ScanStatus) networkScanResult->status;
        resultV1_2.error = (RadioError) networkScanResult->error;
        convertRilCellInfoListToHal_1_2(
                slotId,
                networkScanResult->network_infos,
                networkScanResult->network_infos_length * sizeof(RIL_CellInfo_v12),
                resultV1_2.networkInfos);
        retStatus = mRadioIndicationV1_2->networkScanResult_1_2(
                convertIntToRadioIndicationType(indicationType), resultV1_2);
        checkReturnStatus(retStatus);
    } else {
        RadioImpl::networkScanResultInd(slotId,
                indicationType,
                token,
                e,
                response,
                responseLen);
    }
    return 0;
}

int RadioImpl_1_2::sendLinkCapInd(std::shared_ptr<LinkCapIndMessage> msg)
{
    QCRIL_LOG_FUNC_ENTRY();

    auto ind = mRadioIndicationV1_2;
    QCRIL_LOG_DEBUG("ind null: %s", ind ? "false" : "true" );
    if ( ind ) {
        LinkCapEstimate_t &ilce = msg->getParams();
        V1_2::LinkCapacityEstimate lce = {
            .downlinkCapacityKbps = ilce.downlinkCapacityKbps,
            .uplinkCapacityKbps = ilce.uplinkCapacityKbps };
        QCRIL_LOG_DEBUG("Sending CURRENT_LINK_CAPACITY_ESTIMATE dl: %d ul: %d",
                        ilce.downlinkCapacityKbps, ilce.uplinkCapacityKbps );
        Return<void> ret = ind->currentLinkCapacityEstimate(RadioIndicationType::UNSOLICITED, lce);
        checkReturnStatus(ret);
    }

    QCRIL_LOG_FUNC_RETURN();
    return 0;
}

int RadioImpl_1_2::convertToHidl(V1_2::AudioQuality &aq, enum RIL_AudioQuality rilAudioQuality) {
    switch(rilAudioQuality) {
        case RIL_AUDIO_QUAL_UNSPECIFIED:
        default:
            aq = V1_2::AudioQuality::UNSPECIFIED;
            break;
        case RIL_AUDIO_QUAL_AMR:
            aq = V1_2::AudioQuality::AMR;
            break;
        case RIL_AUDIO_QUAL_AMR_WB:
            aq = V1_2::AudioQuality::AMR_WB;
            break;
        case RIL_AUDIO_QUAL_GSM_EFR:
            aq = V1_2::AudioQuality::GSM_EFR;
            break;
        case RIL_AUDIO_QUAL_GSM_FR:
            aq = V1_2::AudioQuality::GSM_FR;
            break;
        case RIL_AUDIO_QUAL_GSM_HR:
            aq = V1_2::AudioQuality::GSM_HR;
            break;
        case RIL_AUDIO_QUAL_EVRC:
            aq = V1_2::AudioQuality::EVRC;
            break;
        case RIL_AUDIO_QUAL_EVRC_B:
            aq = V1_2::AudioQuality::EVRC_B;
            break;
        case RIL_AUDIO_QUAL_EVRC_WB:
            aq = V1_2::AudioQuality::EVRC_WB;
            break;
        case RIL_AUDIO_QUAL_EVRC_NW:
            aq = V1_2::AudioQuality::EVRC_NW;
            break;
    }
    return 0;
}
int RadioImpl_1_2::convertToHidl(V1_2::Call &hidlCall, RIL_Call &rilCall) {
    RadioImpl::convertToHidl(hidlCall.base, rilCall);
    convertToHidl(hidlCall.audioQuality, rilCall.audioQuality);
    return 0;
}
int RadioImpl_1_2::convertToHidl(V1_2::Call &out, qcril::interfaces::CallInfo &in) {
    RadioImpl::convertToHidl(out.base, in);
    if (in.hasAudioQuality()) {
      convertToHidl(out.audioQuality, in.getAudioQuality());
    }
    return 0;
}
int RadioImpl_1_2::convertGetCurrentCallsResponse(RadioResponseInfo &responseInfo,
        hidl_vec<V1_2::Call> &calls,
        int responseType, int serial, RIL_Errno e,
        void *response, size_t responseLen) {
    populateResponseInfo(responseInfo, serial, responseType, e);
    if (!response || !responseLen) {
        RLOGE("Empty response");
    } else {
        size_t nrCalls = responseLen / sizeof(RIL_Call *);
        calls.resize(nrCalls);
        for (size_t i = 0; i < nrCalls; i++) {
            RIL_Call *p_cur = static_cast<RIL_Call **>(response)[i];
            convertToHidl(calls[i], *p_cur);
        }
    }
    return 0;
}

int RadioImpl_1_2::sendGetCurrentCallsResponse(int slotId,
                              int responseType, int serial, RIL_Errno e,
                              void *response, size_t responseLen) {
    auto rsp = mRadioResponseV1_2;
    QCRIL_LOG_FUNC_ENTRY();
    if (rsp) {
        hidl_vec<V1_2::Call> calls = {};
        RadioResponseInfo responseInfo = {};
        if (convertGetCurrentCallsResponse(responseInfo,
                calls,
                responseType,
                serial,
                e,
                response,
                responseLen) != 0) {
            responseInfo.error = RadioError::INTERNAL_ERR;
        }
        QCRIL_LOG_DEBUG("Calling getCurrentCallsResponse_1_2. Serial: %d", serial);
        Return<void> retStatus = rsp->
                getCurrentCallsResponse_1_2(responseInfo, calls);
        checkReturnStatus(retStatus);
    } else {
        RLOGE("radioService[%d]->mRadioResponseV1_2 == NULL", slotId);
        RadioImpl::sendGetCurrentCallsResponse(slotId,
                responseType,
                serial,
                e,
                response,
                responseLen);
    }
    QCRIL_LOG_FUNC_RETURN();
    return 0;
}

int convertToHal(WcdmaSignalStrength &out, const RIL_WCDMA_SignalStrength &in) {
    out.signalStrength = in.signalStrength;
    out.bitErrorRate = in.bitErrorRate;
    return 0;
}

int convertToHal(V1_2::WcdmaSignalStrength &out, const RIL_WCDMA_SignalStrength &in) {
    int ret = 1;
    ret = convertToHal(out.base, in);
    if (ret) return ret;
    out.rscp = in.rscp;
    out.ecno = in.ecio;
    ret = 0;
    return ret;
}

void convertToHal(V1_2::TdscdmaSignalStrength &out, const RIL_TD_SCDMA_SignalStrength &in) {
    out.rscp = in.rscp;
    out.signalStrength = INT_MAX;
    out.bitErrorRate = INT_MAX;
}

int convertRilSignalStrengthToHal(const RIL_SignalStrength& rilSignalStrength,
        V1_2::SignalStrength& signalStrength)
{
    convertToHal(signalStrength.gsm, rilSignalStrength.GW_SignalStrength);
    convertToHal(signalStrength.wcdma, rilSignalStrength.WCDMA_SignalStrength);
    convertToHal(signalStrength.cdma, rilSignalStrength.CDMA_SignalStrength);
    convertToHal(signalStrength.evdo, rilSignalStrength.EVDO_SignalStrength);
    convertToHal(signalStrength.lte, rilSignalStrength.LTE_SignalStrength);
    convertToHal(signalStrength.tdScdma, rilSignalStrength.TD_SCDMA_SignalStrength);
    return 0;
}

int convertRilSignalStrengthToHal(void *response, size_t responseLen,
        V1_2::SignalStrength& signalStrength)
{
    RIL_SignalStrength_v10 *rilSignalStrength = (RIL_SignalStrength_v10 *) response;

    if (response == NULL || responseLen != sizeof(RIL_SignalStrength_v10)) {
        RLOGE("invalid response");
        return 1;
    }
    convertToHal(signalStrength.gsm, rilSignalStrength->GW_SignalStrength);
    convertToHal(signalStrength.wcdma, rilSignalStrength->WCDMA_SignalStrength);
    convertToHal(signalStrength.cdma, rilSignalStrength->CDMA_SignalStrength);
    convertToHal(signalStrength.evdo, rilSignalStrength->EVDO_SignalStrength);
    convertToHal(signalStrength.lte, rilSignalStrength->LTE_SignalStrength);
    convertToHal(signalStrength.tdScdma, rilSignalStrength->TD_SCDMA_SignalStrength);
    return 0;
}

int RadioImpl_1_2::sendSignalStrength(
        std::shared_ptr<RilUnsolSignalStrengthMessage> msg) {
    auto ind = mRadioIndicationV1_2;
    QCRIL_LOG_INFO("ind null: %s", ind ? "false" : "true" );

    if (ind) {
        V1_2::SignalStrength signalStrength = {};
        convertRilSignalStrengthToHal(msg->getSignalStrength(), signalStrength);
        Return<void> retStatus = ind->currentSignalStrength_1_2(
                RadioIndicationType::UNSOLICITED, signalStrength);
        checkReturnStatus(retStatus);
    } else {
        RadioImpl::sendSignalStrength(msg);
    }
    return 0;
}

int RadioImpl_1_2::sendGetSignalStrengthResponse(int slotId,
                                 int responseType, int serial, RIL_Errno e,
                                 void *response, size_t responseLen) {
    auto rsp = mRadioResponseV1_2;
#if VDBG
    RLOGD("sendGetSignalStrengthResponse: serial %d", serial);
#endif
    if (rsp) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        V1_2::SignalStrength signalStrength = {};
        if (response == NULL || responseLen != sizeof(RIL_SignalStrength_v10)) {
            RLOGE("sendGetSignalStrengthResponse: Invalid response");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            convertRilSignalStrengthToHal(response, responseLen, signalStrength);
        }

        Return<void> retStatus = rsp->getSignalStrengthResponse_1_2(
                responseInfo, signalStrength);
        checkReturnStatus(retStatus);
    } else {
        RLOGE("sendGetSignalStrengthResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
        RadioImpl::sendGetSignalStrengthResponse(slotId,
                responseType,
                serial,
                e,
                response,
                responseLen);
    }

    return 0;
}

int RadioImpl_1_2::sendCurrentSignalStrengthInd(int slotId,
                                             int indicationType, int token, RIL_Errno e,
                                             void *response, size_t responseLen) {
    auto indication = mRadioIndicationV1_2;
    if (indication != NULL) {
        if (response == NULL || responseLen != sizeof(RIL_SignalStrength_v10)) {
            RLOGE("invalid response");
            return 0;
        }
        V1_2::SignalStrength signalStrength = {};
        convertRilSignalStrengthToHal(response, responseLen, signalStrength);

#if VDBG
        RLOGD("sendCurrentSignalStrengthInd_1_2");
#endif
        Return<void> retStatus = indication->currentSignalStrength_1_2(
                convertIntToRadioIndicationType(indicationType), signalStrength);
        checkReturnStatus(retStatus);
    } else {
        RLOGE("sendCurrentSignalStrengthInd_1_2: radioService[%d]->mRadioIndication == NULL",
                slotId);
        RadioImpl::sendCurrentSignalStrengthInd(slotId,
                indicationType,
                token,
                e,
                response,
                responseLen);
    }
    return 0;
}


template <>
void fillCellIdentityGsm(V1_2::CellIdentityGsm &out, const RIL_CellIdentityGsm_v12 &in) {
    fillCellIdentityGsm(out.base, in);
    out.operatorNames.alphaLong = in.operatorNames.alphaLong;
    out.operatorNames.alphaShort = in.operatorNames.alphaShort;
}

template <>
void fillCellIdentityWcdma(V1_2::CellIdentityWcdma &out, const RIL_CellIdentityWcdma_v12 &in) {
    fillCellIdentityWcdma(out.base, in);
    out.operatorNames.alphaLong = in.operatorNames.alphaLong;
    out.operatorNames.alphaShort = in.operatorNames.alphaShort;
}

template <>
void fillCellIdentityCdma(V1_2::CellIdentityCdma &out, const RIL_CellIdentityCdma &in) {
    fillCellIdentityCdma(out.base, in);
    out.operatorNames.alphaLong = in.operatorNames.alphaLong;
    out.operatorNames.alphaShort = in.operatorNames.alphaShort;
}

template <>
void fillCellIdentityLte(V1_2::CellIdentityLte &out, const RIL_CellIdentityLte_v12 &in) {
    fillCellIdentityLte(out.base, in);
    out.operatorNames.alphaLong = in.operatorNames.alphaLong;
    out.operatorNames.alphaShort = in.operatorNames.alphaShort;
    out.bandwidth = in.bandwidth;
}

template <>
void fillCellIdentityTdscdma(V1_2::CellIdentityTdscdma &out, const RIL_CellIdentityTdscdma &in) {
    fillCellIdentityTdscdma(out.base, in);
    out.operatorNames.alphaLong = in.operatorNames.alphaLong;
    out.operatorNames.alphaShort = in.operatorNames.alphaShort;
    out.uarfcn = INT_MAX;
}

int RadioImpl_1_2::sendGetVoiceRegistrationStateResponse(int slotId,
                            int responseType, int serial, RIL_Errno e,
                            void *response, size_t responseLen) {
    auto rsp = mRadioResponseV1_2;
    if (rsp != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);

        V1_2::VoiceRegStateResult voiceRegResponse = {};
        if (response == NULL) {
               RLOGE("sendGetVoiceRegistrationStateResponse Invalid response: NULL");
               if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            RIL_VoiceRegistrationStateResponse *voiceRegState =
                    (RIL_VoiceRegistrationStateResponse *)response;

            if (responseLen != sizeof(RIL_VoiceRegistrationStateResponse)) {
                RLOGE("sendGetVoiceRegistrationStateResponse Invalid response: NULL");
                if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
            } else {
                fillVoiceRegistrationStateResponse(voiceRegResponse,
                        *voiceRegState);
            }
        }
        Return<void> retStatus =
                rsp->getVoiceRegistrationStateResponse_1_2(
                responseInfo, voiceRegResponse);
        checkReturnStatus(retStatus);
    } else {
        RLOGE("sendGetVoiceRegistrationStateResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
        RadioImpl::sendGetVoiceRegistrationStateResponse(slotId,
                responseType,
                serial,
                e,
                response,
                responseLen);
    }
    return 0;
}

int RadioImpl_1_2::sendGetDataRegistrationStateResponse(int slotId,
                            int responseType, int serial, RIL_Errno e,
                            void *response, size_t responseLen) {
    auto rsp = mRadioResponseV1_2;
    if (rsp != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);

        V1_2::DataRegStateResult dataRegResponse = {};
        if (response == NULL) {
               RLOGE("sendGetDataRegistrationStateResponse Invalid response: NULL");
               if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            RIL_DataRegistrationStateResponse *dataRegState =
                    (RIL_DataRegistrationStateResponse *)response;

            if (responseLen != sizeof(RIL_DataRegistrationStateResponse)) {
                RLOGE("sendGetDataRegistrationStateResponse Invalid response: NULL");
                if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
            } else {
                fillDataRegistrationStateResponse(dataRegResponse, *dataRegState);
            }
        }
        Return<void> retStatus =
                rsp->getDataRegistrationStateResponse_1_2(
                responseInfo, dataRegResponse);
        checkReturnStatus(retStatus);
    } else {
        RLOGE("sendGetDataRegistrationStateResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
        RadioImpl::sendGetDataRegistrationStateResponse(slotId,
                responseType,
                serial,
                e,
                response,
                responseLen);
    }
    return 0;
}

V1_2::CellConnectionStatus toHal(NasPhysChanInfo::Status status) {
    V1_2::CellConnectionStatus ret = V1_2::CellConnectionStatus::NONE;
    switch(status) {
        case NasPhysChanInfo::Status::NONE:
            ret = V1_2::CellConnectionStatus::NONE;
            break;
        case NasPhysChanInfo::Status::PRIMARY:
            ret = V1_2::CellConnectionStatus::PRIMARY_SERVING;
            break;
        case NasPhysChanInfo::Status::SECONDARY:
            ret = V1_2::CellConnectionStatus::SECONDARY_SERVING;
            break;
        default:
            QCRIL_LOG_ERROR("Unknown NasPhysChanInfo::Status value %d", static_cast<int>(status));
            break;
    }
    return ret;
}

V1_2::PhysicalChannelConfig toHal(const NasPhysChanInfo &info) {
    V1_2::PhysicalChannelConfig ret;
    ret.status = toHal(info.getStatus());
    ret.cellBandwidthDownlink = info.getBandwidth();
    return ret;
}

hidl_vec<V1_2::PhysicalChannelConfig> toHal(std::shared_ptr<const std::vector<NasPhysChanInfo>> info) {
    hidl_vec<V1_2::PhysicalChannelConfig> ret;
    if (info) {
        ret.resize(info->size());
        int i = 0;
        for (auto entry: *info) {
           ret[i++] = toHal(entry);
        }
    }
    return ret;
}

int RadioImpl_1_2::sendCurrentPhysicalChannelConfigs(std::shared_ptr<NasPhysChanConfigMessage> msg) {
    QCRIL_LOG_FUNC_ENTRY();

    auto rsp = mRadioIndicationV1_2;
    std::shared_ptr<const std::vector<NasPhysChanInfo>> phyChanInfo;

    if (msg) {
        phyChanInfo = msg->getInfo();
    } else {
        RLOGE("sendCurrentPhysicalChannelConfigs: msg is NULL");
    }

    if (rsp && phyChanInfo) {
        QCRIL_LOG_INFO("Sending current config. size: %lu", phyChanInfo->size());
        Return<void> retStatus = rsp->currentPhysicalChannelConfigs(
                RadioIndicationType::UNSOLICITED, toHal(phyChanInfo));
        checkReturnStatus(retStatus);
    } else {
        RLOGE("sendCurrentPhysicalChannelConfigs: radioService[%d]->mRadioResponse is NULL"
                " or phyChanInfo is NULL", mSlotId);
    }

    QCRIL_LOG_FUNC_RETURN();
    return 0;
}

::android::status_t RadioImpl_1_2::registerAsService(const std::string &serviceName) {
    QCRIL_LOG_INFO("RadioImpl_1_2::registerAsService(%s)", serviceName.c_str());
    createRilServiceModule();
    return V1_2::IRadio::registerAsService(serviceName);
}

static void __attribute__((constructor)) registerRadioImpl_1_2();
void registerRadioImpl_1_2()
{
    QCRIL_LOG_INFO("Calling registerRadioImpl_1_2");
    getQcrildServiceFactory().registerRadioImpl<RadioImpl_1_2>(nullptr);
}

Module* RadioImpl_1_2::getRilServiceModule() {
    return mModule;
}

void RadioImpl_1_2::createRilServiceModule() {
    mModule = new RilServiceModule_1_2(getInstanceId());
    if (mModule != nullptr) {
        mModule->init();
    }
}

const QcrildServiceVersion &RadioImpl_1_2::getVersion() {
    static QcrildServiceVersion version(1, 2);
    return version;
}

RadioImpl_1_2::RadioImpl_1_2(qcril_instance_id_e_type instance): RadioImpl(instance) {
    QCRIL_LOG_INFO("RadioImpl_1_2()");
}
