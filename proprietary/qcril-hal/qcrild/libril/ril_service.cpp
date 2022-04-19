/*
* Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
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
#define TAG LOG_TAG

#include "QcrildServiceFactory.h"
#ifndef QMI_RIL_UTF
#include "RadioConfigFactory.h"
#endif
#include <framework/Log.h>
#include <android/hardware/radio/1.1/IRadio.h>
#include <android/hardware/radio/1.1/IRadioResponse.h>
#include <android/hardware/radio/1.1/IRadioIndication.h>
#include <android/hardware/radio/1.1/types.h>
#include <android/hardware/radio/1.0/types.h>

#include <android/hardware/radio/deprecated/1.0/IOemHook.h>

#include "ril_service.h"
#ifndef QMI_RIL_UTF
#include "RadioConfigImpl.h"
#endif
#include <utils/SystemClock.h>
#include <inttypes.h>
#include <log/log.h>

#ifndef QMI_RIL_UTF
#include <hidl/HidlTransportSupport.h>
#endif

#include "SetLinkCapRptCriteriaMessage.h"
#include "modules/nas/NasDataCallbacks.h"
#include "modules/sms/RilRequestSendSmsMessage.h"
#include "modules/sms/RilRequestCdmaSendSmsMessage.h"
#include "modules/sms/RilRequestAckGsmSmsMessage.h"
#include "modules/sms/RilRequestAckCdmaSmsMessage.h"
#include "modules/sms/RilRequestDeleteSmsOnSimMessage.h"
#include "modules/sms/RilRequestWriteSmsToSimMessage.h"
#include "modules/sms/RilRequestCdmaWriteSmsToRuimMessage.h"
#include "modules/sms/RilRequestCdmaDeleteSmsOnRuimMessage.h"
#include "modules/sms/RilRequestGetSmscAddressMessage.h"
#include "modules/sms/RilRequestSetSmscAddressMessage.h"
#include "modules/sms/RilRequestGetGsmBroadcastConfigMessage.h"
#include "modules/sms/RilRequestGetCdmaBroadcastConfigMessage.h"
#include "modules/sms/RilRequestGsmSetBroadcastSmsConfigMessage.h"
#include "modules/sms/RilRequestCdmaSetBroadcastSmsConfigMessage.h"
#include "modules/sms/RilRequestGsmSmsBroadcastActivateMessage.h"
#include "modules/sms/RilRequestCdmaSmsBroadcastActivateMessage.h"
#include "modules/sms/RilRequestReportSmsMemoryStatusMessage.h"
#include "modules/sms/RilRequestGetImsRegistrationMessage.h"
#include "modules/sms/RilRequestImsSendSmsMessage.h"

#include <interfaces/voice/QcRilRequestConferenceMessage.h>
#include <interfaces/voice/QcRilRequestLastCallFailCauseMessage.h>
#include <interfaces/voice/QcRilRequestExplicitCallTransferMessage.h>
#include <interfaces/voice/QcRilRequestGetCallWaitingMessage.h>
#include <interfaces/voice/QcRilRequestGetClipMessage.h>
#include <interfaces/voice/QcRilRequestGetClirMessage.h>
#include <interfaces/voice/QcRilRequestHangupForegroundResumeBackgroundMessage.h>
#include <interfaces/voice/QcRilRequestSendUssdMessage.h>
#include <interfaces/voice/QcRilRequestCancelUssdMessage.h>
#include <interfaces/voice/QcRilRequestCsHangupMessage.h>
#include <interfaces/voice/QcRilRequestGetCurrentCallsMessage.h>
#include <interfaces/voice/QcRilRequestCsDialMessage.h>
#include <interfaces/voice/QcRilRequestCsAnswerMessage.h>
#include <interfaces/voice/QcRilRequestHangupWaitingOrBackgroundMessage.h>
#include <interfaces/voice/QcRilRequestQueryCallForwardMessage.h>
#include <interfaces/voice/QcRilRequestGetCallBarringMessage.h>
#include <interfaces/voice/QcRilRequestSendDtmfMessage.h>
#include <interfaces/voice/QcRilRequestCdmaBurstDtmfMessage.h>
#include <interfaces/voice/QcRilRequestCdmaFlashMessage.h>
#include <interfaces/voice/QcRilRequestGetPreferredVoicePrivacyMessage.h>
#include <interfaces/voice/QcRilRequestSetPreferredVoicePrivacyMessage.h>
#include <interfaces/voice/QcRilRequestSetTtyModeMessage.h>
#include <interfaces/voice/QcRilRequestGetTtyModeMessage.h>
#include <interfaces/voice/QcRilRequestSeparateConnectionMessage.h>
#include <interfaces/voice/QcRilRequestSetCallBarringPasswordMessage.h>
#include <interfaces/voice/QcRilRequestSetCallForwardMessage.h>
#include <interfaces/voice/QcRilRequestSetCallWaitingMessage.h>
#include <interfaces/voice/QcRilRequestSetClirMessage.h>
#include <interfaces/voice/QcRilRequestSetSuppSvcNotificationMessage.h>
#include <interfaces/voice/QcRilRequestSetSupsServiceMessage.h>
#include <interfaces/voice/QcRilRequestStartDtmfMessage.h>
#include <interfaces/voice/QcRilRequestStopDtmfMessage.h>
#include <interfaces/voice/QcRilRequestSwitchWaitingOrHoldingAndActiveMessage.h>
#include <interfaces/voice/QcRilRequestUdubMessage.h>
#include <interfaces/voice/QcRilRequestSetMuteMessage.h>
#include <interfaces/voice/QcRilRequestGetMuteMessage.h>

#include "interfaces/nas/nas_types.h"
#include "interfaces/nas/RilRequestGetVoiceRegistrationMessage.h"
#include "interfaces/nas/RilRequestGetDataRegistrationMessage.h"
#include "interfaces/nas/RilRequestOperatorMessage.h"
#include "interfaces/nas/RilRequestQueryNetworkSelectModeMessage.h"
#include "interfaces/nas/RilRequestAllowDataMessage.h"
#include "interfaces/nas/RilRequestQueryAvailNetworkMessage.h"
#include "interfaces/nas/RilRequestSetNetworkSelectionManualMessage.h"
#include "interfaces/nas/RilRequestSetNetworkSelectionAutoMessage.h"
#include "interfaces/nas/RilRequestStopNetworkScanMessage.h"
#include "interfaces/nas/RilRequestStartNetworkScanMessage.h"
#include "interfaces/nas/RilRequestGetPrefNetworkTypeMessage.h"
#include "interfaces/nas/RilRequestSetPrefNetworkTypeMessage.h"
#include "interfaces/nas/RilRequestGetSignalStrengthMessage.h"
#include "interfaces/nas/RilRequestGetVoiceRadioTechMessage.h"
#include "interfaces/nas/RilRequestSetCdmaSubscriptionSourceMessage.h"
#include "interfaces/nas/RilRequestExitEmergencyCallbackMessage.h"
#include "interfaces/nas/RilRequestScreenStateMessage.h"
#include "interfaces/nas/RilRequestSendDeviceStateMessage.h"
#include "interfaces/nas/RilRequestGetNeighborCellIdsMessage.h"
#include "interfaces/nas/RilRequestCdmaSubscriptionMessage.h"
#include "interfaces/nas/RilRequestSetCdmaRoamingPrefMessage.h"
#include "interfaces/nas/RilRequestSetLocationUpdateMessage.h"
#include "interfaces/nas/RilRequestQueryAvailBandModeMessage.h"
#include "interfaces/nas/RilRequestSetBandModeMessage.h"
#include "interfaces/nas/RilRequestSetUiccSubsMessage.h"
#include "interfaces/nas/RilRequestGetRadioCapMessage.h"
#include "interfaces/nas/RilRequestSetRadioCapMessage.h"
#include "interfaces/nas/RilRequestShutDownMessage.h"
#include "interfaces/nas/RilRequestSetUnsolRespFilterMessage.h"
#include <interfaces/nas/RilRequestSetUnsolCellInfoListRateMessage.h>
#include "interfaces/nas/RilRequestQueryCdmaRoamingPrefMessage.h"
#include "interfaces/nas/RilRequestGetCdmaSubscriptionSourceMessage.h"

#include "modules/uim/UimGetFacilityLockRequestMsg.h"
#include "modules/uim/UimSetFacilityLockRequestMsg.h"
#include "modules/uim/UimGetCardStatusRequestMsg.h"
#include "modules/uim/UimEnterSimPinRequestMsg.h"
#include "modules/uim/UimEnterSimPukRequestMsg.h"
#include "modules/uim/UimChangeSimPinRequestMsg.h"
#include "modules/uim/UimEnterDePersoRequestMsg.h"
#include "modules/uim/UimSIMAuthenticationRequestMsg.h"
#include "modules/uim/UimISIMAuthenticationRequestMsg.h"
#include "modules/uim/UimGetImsiRequestMsg.h"
#include "modules/uim/UimSIMIORequestMsg.h"
#include "modules/uim/UimTransmitAPDURequestMsg.h"
#include "modules/uim/UimSIMOpenChannelRequestMsg.h"
#include "modules/uim/UimSIMCloseChannelRequestMsg.h"
#include "modules/uim/UimCardPowerReqMsg.h"

#include "modules/gstk/GstkSendEnvelopeRequestMsg.h"
#include "modules/gstk/GstkSendTerminalResponseRequestMsg.h"
#include "modules/gstk/GstkSendSetupCallResponseRequestMsg.h"
#include "modules/gstk/GstkReportSTKIsRunningMsg.h"

#include "interfaces/dms/dms_types.h"
#include "interfaces/dms/RilRequestRadioPowerMessage.h"
#include "interfaces/dms/RilRequestGetModemActivityMessage.h"
#include "interfaces/dms/RilRequestGetBaseBandVersionMessage.h"
#include "interfaces/dms/RilRequestGetDeviceIdentityMessage.h"

#include "framework/QcrilRadioClientConnectedMessage.h"

#define INVALID_HEX_CHAR 16

using namespace android::hardware::radio;
using namespace android::hardware::radio::V1_0;
using namespace android::hardware::radio::deprecated::V1_0;
#ifndef QMI_RIL_UTF
using ::android::hardware::configureRpcThreadpool;
using ::android::hardware::joinRpcThreadpool;
#endif
using ::android::hardware::Return;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::hidl_array;
using ::android::hardware::Void;
using android::CommandInfo;
using android::RequestInfo;
using android::requestToString;
using android::sp;

using RadioContext = RadioImpl::RadioContext;

#if defined(OEM_HOOK_DISABLED) || defined(QMI_RIL_UTF)
constexpr bool kOemHookEnabled = false;
#else
constexpr bool kOemHookEnabled = true;
#endif

RIL_RadioFunctions *s_vendorFunctions = NULL;
static CommandInfo *s_commands;

//class RadioImpl;
struct OemHookImpl;

#if (SIM_COUNT >= 2)
sp<RadioImpl> radioService[SIM_COUNT];
sp<OemHookImpl> oemHookService[SIM_COUNT];
// counter used for synchronization. It is incremented every time response callbacks are updated.
volatile int32_t mCounterRadio[SIM_COUNT];
volatile int32_t mCounterOemHook[SIM_COUNT];
#else
sp<RadioImpl> radioService[1];
sp<OemHookImpl> oemHookService[1];
// counter used for synchronization. It is incremented every time response callbacks are updated.
volatile int32_t mCounterRadio[1];
volatile int32_t mCounterOemHook[1];
#endif

#ifndef QMI_RIL_UTF
sp<RadioConfigImpl> radioConfigService;
#endif
static qtimutex::QtiSharedMutex radioServiceRwlock;

#if (SIM_COUNT >= 2)
static qtimutex::QtiSharedMutex radioServiceRwlock2;
#if (SIM_COUNT >= 3)
static qtimutex::QtiSharedMutex radioServiceRwlock3;
#if (SIM_COUNT >= 4)
static qtimutex::QtiSharedMutex radioServiceRwlock4;
#endif
#endif
#endif

void convertRilHardwareConfigListToHal(void *response, size_t responseLen,
        hidl_vec<HardwareConfig>& records);

void convertRilRadioCapabilityToHal(void *response, size_t responseLen, RadioCapability& rc);
void convertRilRadioCapabilityToHal(RIL_RadioCapability& rilRc, RadioCapability& hidlRc);

void convertRilLceDataInfoToHal(void *response, size_t responseLen, LceDataInfo& lce);

int convertRilSignalStrengthToHal(void *response, size_t responseLen,
        SignalStrength& signalStrength);

int convertRilSignalStrengthToHal(const RIL_SignalStrength& rilSignalStrength,
        SignalStrength& signalStrength);

void convertRilDataCallToHal(RIL_Data_Call_Response_v11 *dcResponse,
        SetupDataCallResult& dcResult);

void convertRilDataCallListToHal(void *response, size_t responseLen,
        hidl_vec<SetupDataCallResult>& dcResultList);

void convertRilCellInfoListToHal(int slotId, void *response, size_t responseLen, hidl_vec<CellInfo>& records);
void convertRilCellInfoListToHal(const std::vector<RIL_CellInfo_v12>& rillCellInfo, hidl_vec<CellInfo>& records);

int convertNwTypeToRadioAccessFamily(PreferredNetworkType nwType);
PreferredNetworkType convertRadioAccessFamilyToNwType(int radioAccessFamily);

static inline unsigned indexFromQcrilInstanceId(qcril_instance_id_e_type instance)
{
    int unsigned ret = 0;
    /* For one RILD for each SIM card, return always 0
       as radio service instance will be stored at radioService[0] */
#if (SIM_COUNT >= 2)
    ret = static_cast<unsigned>(instance);
#else
    (void)instance;
#endif
    return ret;
}

sp<RadioImpl> getRadioService(qcril_instance_id_e_type instance)
{
    unsigned slot = indexFromQcrilInstanceId(instance);
    sp<RadioImpl> ret = nullptr;

    if (slot < sizeof(radioService) / sizeof(radioService[0]))
    {
        ret = radioService[slot];
    }
    return ret;
}
struct OemHookImpl : public IOemHook {
    int32_t mSlotId;
    sp<IOemHookResponse> mOemHookResponse;
    sp<IOemHookIndication> mOemHookIndication;

    Return<void> setResponseFunctions(
            const ::android::sp<IOemHookResponse>& oemHookResponse,
            const ::android::sp<IOemHookIndication>& oemHookIndication);

    Return<void> sendRequestRaw(int32_t serial,
            const ::android::hardware::hidl_vec<uint8_t>& data);

    Return<void> sendRequestStrings(int32_t serial,
            const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& data);
};

void memsetAndFreeStrings(int numPointers, ...) {
    va_list ap;
    va_start(ap, numPointers);
    for (int i = 0; i < numPointers; i++) {
        char *ptr = va_arg(ap, char *);
        if (ptr) {
#ifdef MEMSET_FREED
#define MAX_STRING_LENGTH 4096
            memset(ptr, 0, strnlen(ptr, MAX_STRING_LENGTH));
#endif
            free(ptr);
        }
    }
    va_end(ap);
}

void sendErrorResponse(RequestInfo *pRI, RIL_Errno err) {
    if (pRI && pRI->pCI)
    {
        pRI->pCI->responseFunction((int) pRI->socket_id,
                (int) RadioResponseType::SOLICITED, pRI->token, err, NULL, 0);
    }
}

/**
 * Copies over src to dest. If memory allocation fails, responseFunction() is called for the
 * request with error RIL_E_NO_MEMORY. The size() method is used to determine the size of the
 * destination buffer into which the HIDL string is copied. If there is a discrepancy between
 * the string length reported by the size() method, and the length of the string returned by
 * the c_str() method, the function will return false indicating a failure.
 *
 * Returns true on success, and false on failure.
 */
bool copyHidlStringToRil(char **dest, const hidl_string &src, RequestInfo *pRI, bool allowEmpty) {
    size_t len = src.size();
    if (len == 0 && !allowEmpty) {
        *dest = NULL;
        return true;
    }
    *dest = (char *) calloc(len + 1, sizeof(char));
    if (*dest == NULL) {
        RLOGE("Memory allocation failed for request %s", requestToString(pRI->pCI->requestNumber));
        sendErrorResponse(pRI, RIL_E_NO_MEMORY);
        return false;
    }
    if (strlcpy(*dest, src.c_str(), len + 1) >= (len + 1)) {
        RLOGE("Copy of the HIDL string has been truncated, as "
              "the string length reported by size() does not "
              "match the length of string returned by c_str().");
        return false;
    }
    return true;
}

bool copyHidlStringToRil(char **dest, const hidl_string &src, RequestInfo *pRI) {
    return copyHidlStringToRil(dest, src, pRI, false);
}

hidl_string convertCharPtrToHidlString(const char *ptr) {
    hidl_string ret;
    if (ptr != NULL) {
        // TODO: replace this with strnlen
        ret.setToExternal(ptr, strlen(ptr));
    }
    return ret;
}

CallForwardInfoStatus convertCallForwardInfoStatus(int in) {
  switch (in) {
    case 0:  // disable
      return CallForwardInfoStatus::DISABLE;
    case 1:  // enable
      return CallForwardInfoStatus::ENABLE;
    case 2:  // interrogate
      return CallForwardInfoStatus::INTERROGATE;
    case 3:  // registeration
      return CallForwardInfoStatus::REGISTRATION;
    case 4:  // erasure
      return CallForwardInfoStatus::ERASURE;
  }
  return CallForwardInfoStatus::DISABLE;
}

int convertCallForwardInfoStatus(CallForwardInfoStatus in) {
  switch (in) {
    case CallForwardInfoStatus::DISABLE:
      return 0;  // disable
    case CallForwardInfoStatus::ENABLE:
      return 1;  // enable
    case CallForwardInfoStatus::INTERROGATE:
      return 2;  // interrogate
    case CallForwardInfoStatus::REGISTRATION:
      return 3;  // registeration
    case CallForwardInfoStatus::ERASURE:
      return 4;  // erasure
  }
  return 0;
}

qcril::interfaces::FacilityType convertFacilityType(std::string in) {
  if (in == "AO") {
    return qcril::interfaces::FacilityType::BAOC;
  } else if (in == "OI") {
    return qcril::interfaces::FacilityType::BAOIC;
  } else if (in == "OX") {
    return qcril::interfaces::FacilityType::BAOICxH;
  } else if (in == "AI") {
    return qcril::interfaces::FacilityType::BAIC;
  } else if (in == "IR") {
    return qcril::interfaces::FacilityType::BAICr;
  } else if (in == "AB") {
    return qcril::interfaces::FacilityType::BA_ALL;
  } else if (in == "AG") {
    return qcril::interfaces::FacilityType::BA_MO;
  } else if (in == "AC") {
    return qcril::interfaces::FacilityType::BA_MT;
  }
  return qcril::interfaces::FacilityType::UNKNOWN;
}

bool dispatchVoid(int serial, int slotId, int request) {
    RequestInfo *pRI = android::addRequestToList(serial, slotId, request);
    if (pRI == NULL) {
        return false;
    }
    CALL_ONREQUEST(request, NULL, 0, pRI, slotId);
    return true;
}

bool dispatchString(int serial, int slotId, int request, const char * str) {
    RequestInfo *pRI = android::addRequestToList(serial, slotId, request);
    if (pRI == NULL) {
        return false;
    }

    char *pString;
    if (!copyHidlStringToRil(&pString, str, pRI)) {
        return false;
    }

    CALL_ONREQUEST(request, pString, sizeof(char *), pRI, slotId);

    memsetAndFreeStrings(1, pString);
    return true;
}

bool dispatchStrings(int serial, int slotId, int request, bool allowEmpty, int countStrings, ...) {
    RequestInfo *pRI = android::addRequestToList(serial, slotId, request);
    if (pRI == NULL) {
        return false;
    }

    char **pStrings;
    pStrings = (char **)calloc(countStrings, sizeof(char *));
    if (pStrings == NULL) {
        RLOGE("Memory allocation failed for request %s", requestToString(request));
        sendErrorResponse(pRI, RIL_E_NO_MEMORY);
        return false;
    }
    va_list ap;
    va_start(ap, countStrings);
    for (int i = 0; i < countStrings; i++) {
        const char* str = va_arg(ap, const char *);
        if (!copyHidlStringToRil(&pStrings[i], hidl_string(str), pRI, allowEmpty)) {
            va_end(ap);
            for (int j = 0; j < i; j++) {
                memsetAndFreeStrings(1, pStrings[j]);
            }
            free(pStrings);
            return false;
        }
    }
    va_end(ap);

    CALL_ONREQUEST(request, pStrings, countStrings * sizeof(char *), pRI, slotId);

    if (pStrings != NULL) {
        for (int i = 0 ; i < countStrings ; i++) {
            memsetAndFreeStrings(1, pStrings[i]);
        }

#ifdef MEMSET_FREED
        memset(pStrings, 0, countStrings * sizeof(char *));
#endif
        free(pStrings);
    }
    return true;
}

bool dispatchStrings(int serial, int slotId, int request, const hidl_vec<hidl_string>& data) {
    RequestInfo *pRI = android::addRequestToList(serial, slotId, request);
    if (pRI == NULL) {
        return false;
    }

    int countStrings = data.size();
    char **pStrings;
    pStrings = (char **)calloc(countStrings, sizeof(char *));
    if (pStrings == NULL) {
        RLOGE("Memory allocation failed for request %s", requestToString(request));
        sendErrorResponse(pRI, RIL_E_NO_MEMORY);
        return false;
    }

    for (int i = 0; i < countStrings; i++) {
        if (!copyHidlStringToRil(&pStrings[i], data[i], pRI)) {
            for (int j = 0; j < i; j++) {
                memsetAndFreeStrings(1, pStrings[j]);
            }
            free(pStrings);
            return false;
        }
    }

    CALL_ONREQUEST(request, pStrings, countStrings * sizeof(char *), pRI, slotId);

    if (pStrings != NULL) {
        for (int i = 0 ; i < countStrings ; i++) {
            memsetAndFreeStrings(1, pStrings[i]);
        }

#ifdef MEMSET_FREED
        memset(pStrings, 0, countStrings * sizeof(char *));
#endif
        free(pStrings);
    }
    return true;
}

bool dispatchInts(int serial, int slotId, int request, int countInts, ...) {
    RequestInfo *pRI = android::addRequestToList(serial, slotId, request);
    if (pRI == NULL) {
        return false;
    }

    int *pInts = (int *)calloc(countInts, sizeof(int));

    if (pInts == NULL) {
        RLOGE("Memory allocation failed for request %s", requestToString(request));
        sendErrorResponse(pRI, RIL_E_NO_MEMORY);
        return false;
    }
    va_list ap;
    va_start(ap, countInts);
    for (int i = 0; i < countInts; i++) {
        pInts[i] = va_arg(ap, int);
    }
    va_end(ap);

    CALL_ONREQUEST(request, pInts, countInts * sizeof(int), pRI, slotId);

    if (pInts != NULL) {
#ifdef MEMSET_FREED
        memset(pInts, 0, countInts * sizeof(int));
#endif
        free(pInts);
    }
    return true;
}

bool dispatchCallForwardStatus(int serial, int slotId, int request,
                              const CallForwardInfo& callInfo) {
    RequestInfo *pRI = android::addRequestToList(serial, slotId, request);
    if (pRI == NULL) {
        return false;
    }

    RIL_CallForwardInfo cf;
    cf.status = (int) callInfo.status;
    cf.reason = callInfo.reason;
    cf.serviceClass = callInfo.serviceClass;
    cf.toa = callInfo.toa;
    cf.timeSeconds = callInfo.timeSeconds;

    if (!copyHidlStringToRil(&cf.number, callInfo.number, pRI)) {
        return false;
    }

    CALL_ONREQUEST(request, &cf, sizeof(cf), pRI, slotId);

    memsetAndFreeStrings(1, cf.number);

    return true;
}

bool dispatchRaw(int serial, int slotId, int request, const hidl_vec<uint8_t>& rawBytes) {
    RequestInfo *pRI = android::addRequestToList(serial, slotId, request);
    if (pRI == NULL) {
        return false;
    }

    const uint8_t *uData = rawBytes.data();

    CALL_ONREQUEST(request, (void *) uData, rawBytes.size(), pRI, slotId);

    return true;
}

void checkReturnStatus(int32_t slotId, Return<void>& ret, bool isRadioService) {
    if (ret.isOk() == false) {
        RLOGE("checkReturnStatus: unable to call response/indication callback");
#if 1
        (void)slotId;
        (void)isRadioService;
#else
        // Remote process hosting the callbacks must be dead. Reset the callback objects;
        // there's no other recovery to be done here. When the client process is back up, it will
        // call setResponseFunctions()

        // Caller should already hold read lock, release that first
        // note the current counter to avoid overwriting updates made by another thread before
        // write lock is acquired.
        int counter = isRadioService ? mCounterRadio[slotId] : mCounterOemHook[slotId];
        qtimutex::QtiSharedMutex *radioServiceRwlockPtr = radio::getRadioServiceRwlock(slotId);
        radioServiceRwlockPtr->unlock_shared();

        // acquire write lock
        radioServiceRwlockPtr->lock();

        // make sure the counter value has not changed
        if (counter == (isRadioService ? mCounterRadio[slotId] : mCounterOemHook[slotId])) {
            if (isRadioService) {
                radioService[slotId]->clearCallbacks();
            } else {
                oemHookService[slotId]->mOemHookResponse = NULL;
                oemHookService[slotId]->mOemHookIndication = NULL;
            }
            isRadioService ? mCounterRadio[slotId]++ : mCounterOemHook[slotId]++;
        } else {
            RLOGE("checkReturnStatus: not resetting responseFunctions as they likely "
                    "got updated on another thread");
        }

        // release write lock
        radioServiceRwlockPtr->unlock();

        // Reacquire read lock
        radioServiceRwlockPtr->lock_shared();
#endif
    }
}

void RadioImpl::clearCallbacks() {
    RLOGD("RadioImpl::clearCallbacks");
    mRadioResponse = NULL;
    mRadioIndication = NULL;
    mRadioResponseV1_1 = NULL;
    mRadioIndicationV1_1 = NULL;
}

Module* RadioImpl::getRilServiceModule() {
    return nullptr;
}

void RadioImpl::serviceDied(uint64_t cookie,
        const ::android::wp<::android::hidl::base::V1_0::IBase>& who) {
    RLOGD("RadioImpl::serviceDied: Client died. Cleaning up callbacks");
    std::unique_lock<qtimutex::QtiSharedMutex> sl(*radio::getRadioServiceRwlock(mSlotId));
    clearCallbacks();
}

const QcrildServiceVersion &RadioImpl::getVersion() {
    static QcrildServiceVersion version(1,1);
    return version;
}
void RadioImpl::checkReturnStatus(Return<void>& ret) {
    ::checkReturnStatus(mSlotId, ret, true);
}

Return<void> RadioImpl::setResponseFunctions_nolock(
        const ::android::sp<IRadioResponse>& radioResponseParam,
        const ::android::sp<IRadioIndication>& radioIndicationParam) {

    if (mRadioResponse != nullptr) {
        mRadioResponse->unlinkToDeath(this);
    }

    mRadioResponse = radioResponseParam;
    mRadioIndication = radioIndicationParam;
    mRadioResponseV1_1 = V1_1::IRadioResponse::castFrom(mRadioResponse).withDefault(nullptr);
    mRadioIndicationV1_1 = V1_1::IRadioIndication::castFrom(mRadioIndication).withDefault(nullptr);

    QCRIL_LOG_DEBUG("radioResponseParamV1_1: %s. radioIndicationV1_1: %s",
            mRadioResponseV1_1 ? "<not null>" : "<null>",
            mRadioIndicationV1_1 ? "<not null>" : "<null>");

    if (mRadioResponseV1_1 == nullptr || mRadioIndicationV1_1 == nullptr) {
        mRadioResponseV1_1 = nullptr;
        mRadioIndicationV1_1 = nullptr;
    }

    if (mRadioResponse != nullptr) {
        mRadioResponse->linkToDeath(this, 0);
    }

    mCounterRadio[mSlotId]++;

    auto message = std::make_shared<QcrilRadioClientConnectedMessage>();
    if(message) {
        message->broadcast();
    }

    return Void();
}

Return<void> RadioImpl::setResponseFunctions(
        const ::android::sp<IRadioResponse>& radioResponseParam,
        const ::android::sp<IRadioIndication>& radioIndicationParam) {
    QCRIL_LOG_DEBUG("setResponseFunctions. radioResponseParam: %s. radioIndicationParam: %s",
            radioResponseParam ? "<not null>" : "<null>",
            radioIndicationParam ? "<not null>" : "<null>");
    RLOGD("setResponseFunctions");

    qtimutex::QtiSharedMutex *radioServiceRwlockPtr = radio::getRadioServiceRwlock(mSlotId);
    radioServiceRwlockPtr->lock();

    setResponseFunctions_nolock(radioResponseParam, radioIndicationParam);

    radioServiceRwlockPtr->unlock();

#ifndef QMI_RIL_UTF
    // client is connected. Send initial indications.
    android::onNewCommandConnect((RIL_SOCKET_ID) mSlotId);
#endif

    return Void();
}

Return<void> RadioImpl::getIccCardStatus(int32_t serial) {
#if VDBG
    RLOGD("getIccCardStatus: serial %d", serial);
#endif
    auto msg = std::make_shared<UimGetCardStatusRequestMsg>(qmi_ril_get_process_instance_id());
    if (msg) {
        GenericCallback<RIL_UIM_CardStatus> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                            std::shared_ptr<RIL_UIM_CardStatus> responseDataPtr) -> void {
                RadioResponseInfo responseInfo{RadioResponseType::SOLICITED, serial, RadioError::INTERNAL_ERR};
                V1_0::CardStatus cardStatus = {};

                if (solicitedMsg && responseDataPtr &&
                    status == Message::Callback::Status::SUCCESS) {
                    convertGetIccCardStatusResponse(responseInfo, cardStatus,
                            RESPONSE_SOLICITED, serial, responseDataPtr);
                }
                RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getIccCardStatusResponse, responseInfo, cardStatus);
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo rsp{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        V1_0::CardStatus cardStatus = {};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getIccCardStatusResponse, rsp, cardStatus);
    }
    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> RadioImpl::supplyIccPinForApp(int32_t serial, const hidl_string& pin,
        const hidl_string& aid) {
#if VDBG
    RLOGD("supplyIccPinForApp: serial %d", serial);
#endif
    auto msg = std::make_shared<UimEnterSimPinRequestMsg>(RIL_UIM_SIM_PIN1, pin, aid);
    if (msg) {
        GenericCallback<RIL_UIM_SIM_PIN_Response> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                            std::shared_ptr<RIL_UIM_SIM_PIN_Response> responseDataPtr) -> void {
                RadioResponseInfo responseInfo{RadioResponseType::SOLICITED, serial, RadioError::INTERNAL_ERR};
                int retries = -1;

                if (solicitedMsg && responseDataPtr &&
                    status == Message::Callback::Status::SUCCESS) {
                    responseInfo.error = (RadioError)responseDataPtr->err;
                    retries = responseDataPtr->no_of_retries;
                }
                RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, supplyIccPinForAppResponse,
                        responseInfo, retries);
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo rsp{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, supplyIccPinForAppResponse, rsp, -1);
    }
    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> RadioImpl::supplyIccPukForApp(int32_t serial, const hidl_string& puk,
                                           const hidl_string& pin, const hidl_string& aid) {
#if VDBG
    RLOGD("supplyIccPukForApp: serial %d", serial);
#endif
    auto msg = std::make_shared<UimEnterSimPukRequestMsg>(RIL_UIM_SIM_PUK1, puk, pin, aid);
    if (msg) {
        GenericCallback<RIL_UIM_SIM_PIN_Response> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                            std::shared_ptr<RIL_UIM_SIM_PIN_Response> responseDataPtr) -> void {
                RadioResponseInfo responseInfo{RadioResponseType::SOLICITED, serial, RadioError::INTERNAL_ERR};
                int retries = -1;

                if (solicitedMsg && responseDataPtr &&
                    status == Message::Callback::Status::SUCCESS) {
                    responseInfo.error = (RadioError)responseDataPtr->err;
                    retries = responseDataPtr->no_of_retries;
                }
                RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, supplyIccPukForAppResponse,
                        responseInfo, retries);
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo rsp{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, supplyIccPukForAppResponse, rsp, -1);
    }
    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> RadioImpl::supplyIccPin2ForApp(int32_t serial, const hidl_string& pin2,
                                            const hidl_string& aid) {
#if VDBG
    RLOGD("supplyIccPin2ForApp: serial %d", serial);
#endif
    auto msg = std::make_shared<UimEnterSimPinRequestMsg>(RIL_UIM_SIM_PIN2, pin2, aid);
    if (msg) {
        GenericCallback<RIL_UIM_SIM_PIN_Response> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                            std::shared_ptr<RIL_UIM_SIM_PIN_Response> responseDataPtr) -> void {
                RadioResponseInfo responseInfo{RadioResponseType::SOLICITED, serial, RadioError::INTERNAL_ERR};
                int retries = -1;

                if (solicitedMsg && responseDataPtr &&
                    status == Message::Callback::Status::SUCCESS) {
                    responseInfo.error = (RadioError)responseDataPtr->err;
                    retries = responseDataPtr->no_of_retries;
                }
                RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, supplyIccPin2ForAppResponse,
                        responseInfo, retries);
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo rsp{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, supplyIccPin2ForAppResponse, rsp, -1);
    }
    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> RadioImpl::supplyIccPuk2ForApp(int32_t serial, const hidl_string& puk2,
                                            const hidl_string& pin2, const hidl_string& aid) {
#if VDBG
    RLOGD("supplyIccPuk2ForApp: serial %d", serial);
#endif
    auto msg = std::make_shared<UimEnterSimPukRequestMsg>(RIL_UIM_SIM_PUK2, puk2, pin2, aid);
    if (msg) {
        GenericCallback<RIL_UIM_SIM_PIN_Response> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                            std::shared_ptr<RIL_UIM_SIM_PIN_Response> responseDataPtr) -> void {
                RadioResponseInfo responseInfo{RadioResponseType::SOLICITED, serial, RadioError::INTERNAL_ERR};
                int retries = -1;

                if (solicitedMsg && responseDataPtr &&
                    status == Message::Callback::Status::SUCCESS) {
                    responseInfo.error = (RadioError)responseDataPtr->err;
                    retries = responseDataPtr->no_of_retries;
                }
                RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, supplyIccPuk2ForAppResponse,
                        responseInfo, retries);
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo rsp{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, supplyIccPuk2ForAppResponse, rsp, -1);
    }
    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> RadioImpl::changeIccPinForApp(int32_t serial, const hidl_string& oldPin,
                                           const hidl_string& newPin, const hidl_string& aid) {
#if VDBG
    RLOGD("changeIccPinForApp: serial %d", serial);
#endif
    auto msg = std::make_shared<UimChangeSimPinRequestMsg>(RIL_UIM_SIM_PIN1, newPin, oldPin, aid);
    if (msg) {
        GenericCallback<RIL_UIM_SIM_PIN_Response> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                            std::shared_ptr<RIL_UIM_SIM_PIN_Response> responseDataPtr) -> void {
                RadioResponseInfo responseInfo{RadioResponseType::SOLICITED, serial, RadioError::INTERNAL_ERR};
                int retries = -1;

                if (solicitedMsg && responseDataPtr &&
                    status == Message::Callback::Status::SUCCESS) {
                    responseInfo.error = (RadioError)responseDataPtr->err;
                    retries = responseDataPtr->no_of_retries;
                }
                RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, changeIccPinForAppResponse,
                        responseInfo, retries);
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo rsp{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, changeIccPinForAppResponse, rsp, -1);
    }
    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> RadioImpl::changeIccPin2ForApp(int32_t serial, const hidl_string& oldPin2,
                                            const hidl_string& newPin2, const hidl_string& aid) {
#if VDBG
    RLOGD("changeIccPin2ForApp: serial %d", serial);
#endif
    auto msg = std::make_shared<UimChangeSimPinRequestMsg>(RIL_UIM_SIM_PIN2, newPin2, oldPin2, aid);
    if (msg) {
        GenericCallback<RIL_UIM_SIM_PIN_Response> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                            std::shared_ptr<RIL_UIM_SIM_PIN_Response> responseDataPtr) -> void {
                RadioResponseInfo responseInfo{RadioResponseType::SOLICITED, serial, RadioError::INTERNAL_ERR};
                int retries = -1;

                if (solicitedMsg && responseDataPtr &&
                    status == Message::Callback::Status::SUCCESS) {
                    responseInfo.error = (RadioError)responseDataPtr->err;
                    retries = responseDataPtr->no_of_retries;
                }
                RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, changeIccPin2ForAppResponse,
                        responseInfo, retries);
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo rsp{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, changeIccPin2ForAppResponse, rsp, -1);
    }
    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> RadioImpl::supplyNetworkDepersonalization(int32_t serial,
                                                       const hidl_string& netPin) {
#if VDBG
    RLOGD("supplyNetworkDepersonalization: serial %d", serial);
#endif
    auto msg = std::make_shared<UimEnterDePersoRequestMsg>(netPin, RIL_UIM_PERSOSUBSTATE_SIM_NETWORK);
    if (msg) {
        GenericCallback<RIL_UIM_PersoResponse> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                            std::shared_ptr<RIL_UIM_PersoResponse> responseDataPtr) -> void {
                RadioResponseInfo responseInfo{RadioResponseType::SOLICITED, serial, RadioError::INTERNAL_ERR};
                int retries = -1;

                if (solicitedMsg && responseDataPtr &&
                    status == Message::Callback::Status::SUCCESS) {
                    responseInfo.error = (RadioError)responseDataPtr->err;
                    retries = responseDataPtr->no_of_retries;
                }
                RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, supplyNetworkDepersonalizationResponse,
                        responseInfo, retries);
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo rsp{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, supplyNetworkDepersonalizationResponse, rsp, -1);
    }
    QCRIL_LOG_FUNC_RETURN();
    return Void();
}
CallState convertCallState(qcril::interfaces::CallState in) {
  switch(in) {
    case qcril::interfaces::CallState::ACTIVE:
      return CallState::ACTIVE;
    case qcril::interfaces::CallState::HOLDING:
      return CallState::HOLDING;
    case qcril::interfaces::CallState::DIALING:
      return CallState::DIALING;
    case qcril::interfaces::CallState::ALERTING:
      return CallState::ALERTING;
    case qcril::interfaces::CallState::INCOMING:
      return CallState::INCOMING;
    case qcril::interfaces::CallState::WAITING:
      return CallState::WAITING;
    default:
      return CallState::ACTIVE;
  }
  return CallState::ACTIVE;
}
CallPresentation convertCallPresentation(uint32_t in) {
  switch(in) {
    case 0:
      return CallPresentation::ALLOWED;
    case 1:
      return CallPresentation::RESTRICTED;
    case 2:
    default:
      return CallPresentation::UNKNOWN;
    case 3:
      return CallPresentation::PAYPHONE;
  }
  return CallPresentation::UNKNOWN;
}

int RadioImpl::convertToHidl(Call &out, qcril::interfaces::CallInfo &in) {
    /* each call info */
    out.state = convertCallState(in.getCallState());
    out.index = in.getIndex();
    out.toa = in.getToa();
    out.isMpty = in.getIsMpty();
    out.isMT = in.getIsMt();
    out.als = in.getAls();
    out.isVoice = in.getIsVoice();
    out.isVoicePrivacy = in.getIsVoicePrivacy();
    out.number = in.getNumber().c_str();
    out.numberPresentation = convertCallPresentation(in.getNumberPresentation());
    out.name = in.getName().c_str();
    out.namePresentation = convertCallPresentation(in.getNamePresentation());
    if (in.hasUusInfo() && in.getUusInfo() != NULL) {
      std::shared_ptr<qcril::interfaces::UusInfo> uusInfo = in.getUusInfo();
      if (uusInfo->hasData() && !uusInfo->getData().empty()) {
        out.uusInfo.resize(1);
        out.uusInfo[0].uusType = (UusType) uusInfo->getType();
        out.uusInfo[0].uusDcs = (UusDcs) uusInfo->getDcs();
        out.uusInfo[0].uusData = uusInfo->getData().c_str();
      }
    }
    return 0;
}
Return<void> RadioImpl::getCurrentCalls(int32_t serial) {
#if VDBG
  RLOGD("getCurrentCalls: serial %d", serial);
#endif
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
          hidl_vec<Call> calls;
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
          QCRIL_LOG_ERROR("getCurrentCallsResponse : calls = %s", toString(calls).c_str());
          RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getCurrentCallsResponse, respInfo,
                                   calls);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    hidl_vec<Call> calls;
    RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
    RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getCurrentCallsResponse, respInfo, calls);
  }
  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

Return<void> RadioImpl::dial(int32_t serial, const Dial& dialInfo) {
#if VDBG
    RLOGD("dial: serial %d", serial);
#endif
  QCRIL_LOG_FUNC_ENTRY();
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<QcRilRequestCsDialMessage>(getContext(serial));
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    if (!dialInfo.address.empty()) {
      msg->setAddress(dialInfo.address.c_str());
    }
    msg->setClir((int)dialInfo.clir);
    if (dialInfo.uusInfo.size() != 0) {
      auto uusInfo = std::make_shared<qcril::interfaces::UusInfo>();
      if (uusInfo) {
        uusInfo->setType((RIL_UUS_Type) dialInfo.uusInfo[0].uusType);
        uusInfo->setDcs((RIL_UUS_DCS) dialInfo.uusInfo[0].uusDcs);
        if (dialInfo.uusInfo[0].uusData.size() > 0) {
          uusInfo->setData(std::string(dialInfo.uusInfo[0].uusData.c_str(),
                                       dialInfo.uusInfo[0].uusData.size()));
        }
        msg->setUusInfo(uusInfo);
      }
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                       std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
          }
          RadioResponseInfo respInfo = {};
          populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, errorCode);
          RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, dialResponse, respInfo);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
    RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, dialResponse, respInfo);
  }
  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

Return<void> RadioImpl::getImsiForApp(int32_t serial, const hidl_string& aid) {
#if VDBG
    RLOGD("getImsiForApp: serial %d", serial);
#endif
    auto msg = std::make_shared<UimGetImsiRequestMsg>(qmi_ril_get_process_instance_id(), aid);
    if (msg) {
        GenericCallback<RIL_UIM_IMSI_Response> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                            std::shared_ptr<RIL_UIM_IMSI_Response> responseDataPtr) -> void {
                RadioResponseInfo responseInfo{RadioResponseType::SOLICITED, serial, RadioError::INTERNAL_ERR};
                string IMSI = {};

                if (solicitedMsg && responseDataPtr &&
                    status == Message::Callback::Status::SUCCESS) {
                    responseInfo.error = (RadioError)responseDataPtr->err;
                    IMSI = responseDataPtr->IMSI;
                }
                RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getIMSIForAppResponse,
                        responseInfo, IMSI);
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo rsp{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getIMSIForAppResponse, rsp, NULL);
    }
    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> RadioImpl::hangup(int32_t serial, int32_t gsmIndex) {
#if VDBG
  RLOGD("hangup: serial %d", serial);
#endif
  QCRIL_LOG_FUNC_ENTRY();
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<QcRilRequestCsHangupMessage>(getContext(serial));
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
    }
    msg->setCallIndex(gsmIndex);
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                       std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
          }
          RadioResponseInfo respInfo = {};
          populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, errorCode);
          RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, hangupConnectionResponse, respInfo);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
    RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, hangupConnectionResponse, respInfo);
  }
  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

Return<void> RadioImpl::hangupWaitingOrBackground(int32_t serial) {
#if VDBG
  RLOGD("hangupWaitingOrBackground: serial %d", serial);
#endif
  QCRIL_LOG_FUNC_ENTRY();
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<QcRilRequestHangupWaitingOrBackgroundMessage>(getContext(serial));
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                       std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
          }
          RadioResponseInfo respInfo = {};
          populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, errorCode);
          RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, hangupWaitingOrBackgroundResponse,
                                   respInfo);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
    RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, hangupWaitingOrBackgroundResponse, respInfo);
  }
  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

Return<void> RadioImpl::hangupForegroundResumeBackground(int32_t serial) {
#if VDBG
  RLOGD("hangupForegroundResumeBackground: serial %d", serial);
#endif
  QCRIL_LOG_FUNC_ENTRY();
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<QcRilRequestHangupForegroundResumeBackgroundMessage>(getContext(serial));
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                       std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
          }
          RadioResponseInfo respInfo = {};
          populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, errorCode);
          RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse,
                                   hangupForegroundResumeBackgroundResponse, respInfo);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
    RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, hangupForegroundResumeBackgroundResponse,
                             respInfo);
  }
  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

Return<void> RadioImpl::switchWaitingOrHoldingAndActive(int32_t serial) {
#if VDBG
    RLOGD("switchWaitingOrHoldingAndActive: serial %d", serial);
#endif
  QCRIL_LOG_FUNC_ENTRY();
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<QcRilRequestSwitchWaitingOrHoldingAndActiveMessage>(getContext(serial));
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                       std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
          }
          RadioResponseInfo respInfo = {};
          populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, errorCode);
          RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse,
                                   switchWaitingOrHoldingAndActiveResponse, respInfo);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
    RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, switchWaitingOrHoldingAndActiveResponse,
                             respInfo);
  }
  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

Return<void> RadioImpl::conference(int32_t serial) {
#if VDBG
    RLOGD("conference: serial %d", serial);
#endif
  QCRIL_LOG_FUNC_ENTRY();
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<QcRilRequestConferenceMessage>(getContext(serial));
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                       std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
          }
          RadioResponseInfo respInfo = {};
          populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, errorCode);
          RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, conferenceResponse, respInfo);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
    RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, conferenceResponse, respInfo);
  }
  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

Return<void> RadioImpl::rejectCall(int32_t serial) {
#if VDBG
    RLOGD("rejectCall: serial %d", serial);
#endif
  QCRIL_LOG_FUNC_ENTRY();
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<QcRilRequestUdubMessage>(getContext(serial));
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                       std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
          }
          RadioResponseInfo respInfo = {};
          populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, errorCode);
          RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, rejectCallResponse, respInfo);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
    RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, rejectCallResponse, respInfo);
  }
  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

Return<void> RadioImpl::getLastCallFailCause(int32_t serial) {
#if VDBG
    RLOGD("getLastCallFailCause: serial %d", serial);
#endif
  QCRIL_LOG_FUNC_ENTRY();
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<QcRilRequestLastCallFailCauseMessage>(getContext(serial));
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                       std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          LastCallFailCauseInfo info = {};
          info.vendorCause = hidl_string();
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
            auto failCause = std::static_pointer_cast<qcril::interfaces::LastCallFailCauseInfo>(resp->data);
            if (failCause) {
              if (failCause->hasCallFailCause()) {
                info.causeCode = (LastCallFailCause) failCause->getCallFailCause();
              }
              if (failCause->hasCallFailCauseDescription()) {
                info.vendorCause = failCause->getCallFailCauseDescription().c_str();
              }
            }
          }
          RadioResponseInfo respInfo = {};
          populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, errorCode);
          RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getLastCallFailCauseResponse, respInfo, info);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    LastCallFailCauseInfo info = {};
    RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
    RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getLastCallFailCauseResponse, respInfo, info);
  }
  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

Return<void> RadioImpl::getSignalStrength(int32_t serial) {
#if VDBG
    RLOGD("getSignalStrength: serial %d", serial);
#endif
    std::shared_ptr<RadioContext> ctx = getContext(serial);
    auto msg = std::make_shared<RilRequestGetSignalStrengthMessage>(ctx);
    if (msg != nullptr) {
        GenericCallback<QcRilRequestMessageCallbackPayload> cb(
            [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                          std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
            if (resp != nullptr) {
                RadioResponseInfo respInfo = {};
                SignalStrength signalStrength = {};
                populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, resp->errorCode);
                auto rilSigResult = std::static_pointer_cast<qcril::interfaces::RilGetSignalStrengthResult_t>(resp->data);
                if (rilSigResult != nullptr) {
                    convertRilSignalStrengthToHal(rilSigResult->respData, signalStrength);
                }
                RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse,
                        getSignalStrengthResponse, respInfo, signalStrength);
            }
        });
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        SignalStrength signalStrength = {};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse,
                getSignalStrengthResponse, respInfo, signalStrength);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> RadioImpl::getVoiceRegistrationState(int32_t serial) {
#if VDBG
    RLOGD("getVoiceRegistrationState: serial %d", serial);
#endif
    std::shared_ptr<RadioContext> ctx = getContext(serial);
    auto msg = std::make_shared<RilRequestGetVoiceRegistrationMessage>(ctx);
    if (msg != nullptr) {
        GenericCallback<QcRilRequestMessageCallbackPayload> cb(
            [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                          std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
            if (resp != nullptr) {
                RadioResponseInfo respInfo = {};
                VoiceRegStateResult voiceRegResponse = {};
                populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, resp->errorCode);
                auto rilRegResult = std::static_pointer_cast<qcril::interfaces::RilGetVoiceRegResult_t>(resp->data);
                if (rilRegResult != nullptr) {
                    fillVoiceRegistrationStateResponse(voiceRegResponse, rilRegResult->respData);
                }
                RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse,
                        getVoiceRegistrationStateResponse, respInfo, voiceRegResponse);
            }
        });
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        VoiceRegStateResult voiceRegResponse = {};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse,
                getVoiceRegistrationStateResponse, respInfo, voiceRegResponse);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> RadioImpl::getDataRegistrationState(int32_t serial) {
#if VDBG
    RLOGD("getDataRegistrationState: serial %d", serial);
#endif
    std::shared_ptr<RadioContext> ctx = getContext(serial);
    auto msg = std::make_shared<RilRequestGetDataRegistrationMessage>(ctx);
    if (msg != nullptr) {
        GenericCallback<QcRilRequestMessageCallbackPayload> cb(
            [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                          std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
            if (resp != nullptr) {
                RadioResponseInfo respInfo = {};
                DataRegStateResult dataRegResponse = {};
                populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, resp->errorCode);
                auto rilRegResult = std::static_pointer_cast<qcril::interfaces::RilGetDataRegResult_t>(resp->data);
                if (rilRegResult != nullptr) {
                    fillDataRegistrationStateResponse(dataRegResponse, rilRegResult->respData);
                }
                RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse,
                        getDataRegistrationStateResponse, respInfo, dataRegResponse);
            }
        });
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        DataRegStateResult dataRegResponse = {};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse,
                getDataRegistrationStateResponse, respInfo, dataRegResponse);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> RadioImpl::getOperator(int32_t serial) {
#if VDBG
    RLOGD("getOperator: serial %d", serial);
#endif
    std::shared_ptr<RadioContext> ctx = getContext(serial);
    auto msg = std::make_shared<RilRequestOperatorMessage>(ctx);
    if (msg != nullptr) {
        GenericCallback<QcRilRequestMessageCallbackPayload> cb(
            [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                          std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
            RadioResponseInfo respInfo = {};
            std::shared_ptr<qcril::interfaces::RilGetOperatorResult_t> rilOperatorResult;
            if (resp != nullptr) {
                populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, resp->errorCode);
                rilOperatorResult = std::static_pointer_cast<qcril::interfaces::RilGetOperatorResult_t>(resp->data);
            } else {
                populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, RIL_E_NO_MEMORY);
            }
            if (rilOperatorResult != nullptr) {
                RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getOperatorResponse,
                    respInfo, rilOperatorResult->longName, rilOperatorResult->shortName,
                    rilOperatorResult->numeric);
            } else {
                RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getOperatorResponse,
                    respInfo, hidl_string(), hidl_string(), hidl_string());
            }
        });
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getOperatorResponse,
            respInfo, hidl_string(), hidl_string(), hidl_string());
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> RadioImpl::setRadioPower(int32_t serial, bool on) {
    RLOGD("setRadioPower: serial %d on %d", serial, on);

    std::shared_ptr<RadioContext> ctx = getContext(serial);
    auto msg = std::make_shared<RilRequestRadioPowerMessage>(ctx, on);
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
            RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, setRadioPowerResponse, respInfo);
        });
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, setRadioPowerResponse, respInfo);
    }
    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> RadioImpl::sendDtmf(int32_t serial, const hidl_string &s) {
#if VDBG
  RLOGD("sendDtmf: serial %d", serial);
#endif
  QCRIL_LOG_FUNC_ENTRY();
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<QcRilRequestSendDtmfMessage>(getContext(serial));
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    if (s.empty()) {
      QCRIL_LOG_ERROR("Invalid parameter: s");
      sendFailure = true;
      break;
    }
    msg->setDigit(s.c_str()[0]);
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                       std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
          }
          RadioResponseInfo respInfo = {};
          populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, errorCode);
          RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, sendDtmfResponse, respInfo);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
    RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, sendDtmfResponse, respInfo);
  }
  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

SendSmsResult makeSendSmsResult(RadioResponseInfo& responseInfo, int serial, int responseType,
                                RIL_Errno e, void *response, size_t responseLen) {
    populateResponseInfo(responseInfo, serial, responseType, e);
    SendSmsResult result = {};

    if (response == NULL || responseLen != sizeof(RIL_SMS_Response)) {
        RLOGE("Invalid response: NULL");
        if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        result.ackPDU = hidl_string();
    } else {
        RIL_SMS_Response *resp = (RIL_SMS_Response *) response;
        result.messageRef = resp->messageRef;
        result.ackPDU = convertCharPtrToHidlString(resp->ackPDU);
        result.errorCode = resp->errorCode;
   }
   return result;
}

SendSmsResult makeSendSmsResult(RadioResponseInfo& responseInfo, int serial, int responseType,
                                std::shared_ptr<RilSendSmsResult_t> responseDataPtr) {

    SendSmsResult result = {};
    if (!responseDataPtr) {
        RLOGE("Invalid response: NULL");
        populateResponseInfo(responseInfo, serial, responseType,
                RIL_E_INVALID_RESPONSE);
        result.ackPDU = hidl_string();
    } else {
        populateResponseInfo(responseInfo, serial, responseType,
                responseDataPtr->rilErr);
        result.messageRef = responseDataPtr->messageRef;
        result.ackPDU = responseDataPtr->ackPDU;
        result.errorCode = responseDataPtr->errorCode;
    }
    return result;
}

int radio::sendSmsResponse(int slotId,
                          int responseType, int serial, RIL_Errno e, void *response,
                          size_t responseLen) {
#if VDBG
    RLOGD("sendSmsResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        SendSmsResult result = makeSendSmsResult(responseInfo, serial, responseType, e, response,
                responseLen);

        Return<void> retStatus = radioService[slotId]->mRadioResponse->sendSmsResponse(responseInfo,
                result);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("sendSmsResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::sendSMSExpectMoreResponse(int slotId,
                                    int responseType, int serial, RIL_Errno e, void *response,
                                    size_t responseLen) {
#if VDBG
    RLOGD("sendSMSExpectMoreResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        SendSmsResult result = makeSendSmsResult(responseInfo, serial, responseType, e, response,
                responseLen);

        Return<void> retStatus = radioService[slotId]->mRadioResponse->sendSMSExpectMoreResponse(
                responseInfo, result);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("sendSMSExpectMoreResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::setupDataCallResponse(int slotId,
                                 int responseType, int serial, RIL_Errno e, void *response,
                                 size_t responseLen) {
#if VDBG
    RLOGD("setupDataCallResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);

        SetupDataCallResult result = {};
        if (response == NULL || (responseLen % sizeof(RIL_Data_Call_Response_v11)) != 0) {
            if (response != NULL) {
                RLOGE("setupDataCallResponse: Invalid response");
                if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
            }
            result.status = DataCallFailCause::ERROR_UNSPECIFIED;
            result.type = hidl_string();
            result.ifname = hidl_string();
            result.addresses = hidl_string();
            result.dnses = hidl_string();
            result.gateways = hidl_string();
            result.pcscf = hidl_string();
        } else {
            convertRilDataCallToHal((RIL_Data_Call_Response_v11 *) response, result);
        }

        Return<void> retStatus = radioService[slotId]->mRadioResponse->setupDataCallResponse(
                responseInfo, result);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("setupDataCallResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

Return<void> RadioImpl::sendSms(int32_t serial, const GsmSmsMessage& message) {
    QCRIL_LOG_FUNC_ENTRY();
#if VDBG
    RLOGD("sendSms: serial %d", serial);
#endif
    auto msg = std::make_shared<RilRequestSendSmsMessage>(message.smscPdu, message.pdu);
    if (msg) {
        GenericCallback<RilSendSmsResult_t> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                            std::shared_ptr<RilSendSmsResult_t> responseDataPtr) -> void {
                if (solicitedMsg && responseDataPtr) {
                    RadioResponseInfo responseInfo = {};
                    SendSmsResult result = makeSendSmsResult(responseInfo, serial,
                            RESPONSE_SOLICITED, responseDataPtr);

                    RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, sendSmsResponse,
                            responseInfo, result);
                }
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo rsp{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        SendSmsResult result = {-1, hidl_string(), -1};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, sendSmsResponse, rsp, result);
    }
    QCRIL_LOG_FUNC_RETURN();

    return Void();
}

Return<void> RadioImpl::sendSMSExpectMore(int32_t serial, const GsmSmsMessage& message) {
#if VDBG
    RLOGD("sendSMSExpectMore: serial %d", serial);
#endif
    auto msg = std::make_shared<RilRequestSendSmsMessage>(message.smscPdu, message.pdu, true);
    if (msg) {
        GenericCallback<RilSendSmsResult_t> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                            std::shared_ptr<RilSendSmsResult_t> responseDataPtr) -> void {
                if (solicitedMsg && responseDataPtr) {
                    RadioResponseInfo responseInfo = {};
                    SendSmsResult result = makeSendSmsResult(responseInfo, serial,
                            RESPONSE_SOLICITED, responseDataPtr);

                    RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, sendSMSExpectMoreResponse,
                            responseInfo, result);
                }
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo rsp{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        SendSmsResult result = {-1, hidl_string(), -1};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, sendSMSExpectMoreResponse, rsp, result);

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

Return<void> RadioImpl::setupDataCall(int32_t serial, RadioTechnology radioTechnology,
                                      const DataProfileInfo& dataProfileInfo, bool modemCognitive,
                                      bool roamingAllowed, bool isRoaming) {
#if VDBG
    RLOGD("setupDataCall: serial %d", serial);
#endif

    if (s_vendorFunctions->version >= 4 && s_vendorFunctions->version <= 14) {
        const hidl_string &protocol =
                (isRoaming ? dataProfileInfo.roamingProtocol : dataProfileInfo.protocol);
        dispatchStrings(serial, mSlotId, RIL_REQUEST_SETUP_DATA_CALL, true, 7,
            std::to_string((int) radioTechnology + 2).c_str(),
            std::to_string((int) dataProfileInfo.profileId).c_str(),
            dataProfileInfo.apn.c_str(),
            dataProfileInfo.user.c_str(),
            dataProfileInfo.password.c_str(),
            std::to_string((int) dataProfileInfo.authType).c_str(),
            protocol.c_str());
    } else if (s_vendorFunctions->version >= 15) {
        char *mvnoTypeStr = NULL;
        if (!convertMvnoTypeToString(dataProfileInfo.mvnoType, mvnoTypeStr)) {
            RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
                    RIL_REQUEST_SETUP_DATA_CALL);
            if (pRI != NULL) {
                sendErrorResponse(pRI, RIL_E_INVALID_ARGUMENTS);
            }
            return Void();
        }
        dispatchStrings(serial, mSlotId, RIL_REQUEST_SETUP_DATA_CALL, true, 19,
            std::to_string((int) radioTechnology + 2).c_str(),
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
            NULL,
            NULL,
            NULL,
            NULL);
    } else {
        RLOGE("Unsupported RIL version %d, min version expected 4", s_vendorFunctions->version);
        RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
                RIL_REQUEST_SETUP_DATA_CALL);
        if (pRI != NULL) {
            sendErrorResponse(pRI, RIL_E_REQUEST_NOT_SUPPORTED);
        }
    }
    return Void();
}

Return<void> RadioImpl::iccIOForApp(int32_t serial, const IccIo& iccIo) {
#if VDBG
    RLOGD("iccIOForApp: serial %d", serial);
#endif
    RIL_UIM_SIM_IO data = {};
    data.command = iccIo.command;
    data.fileid = iccIo.fileId;
    data.path = iccIo.path;
    data.p1 = iccIo.p1;
    data.p2 = iccIo.p2;
    data.p3 = iccIo.p3;
    data.data = iccIo.data;
    data.pin2 = iccIo.pin2;
    data.aidPtr = iccIo.aid;

    auto msg = std::make_shared<UimSIMIORequestMsg>(qmi_ril_get_process_instance_id(), data);
    if (msg) {
        GenericCallback<RIL_UIM_SIM_IO_Response> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                            std::shared_ptr<RIL_UIM_SIM_IO_Response> responseDataPtr) -> void {
                RadioResponseInfo responseInfo{RadioResponseType::SOLICITED, serial, RadioError::INTERNAL_ERR};
                IccIoResult  result = {};

                if (solicitedMsg && responseDataPtr &&
                    status == Message::Callback::Status::SUCCESS) {
                    responseInfo.error = (RadioError)responseDataPtr->err;
                    result.sw1 = responseDataPtr->sw1;
                    result.sw2 = responseDataPtr->sw2;
                    result.simResponse = responseDataPtr->simResponse;
                }
                RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, iccIOForAppResponse,
                        responseInfo, result);
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo rsp{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        IccIoResult  result = {};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, iccIOForAppResponse, rsp, result);
    }
    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> RadioImpl::sendUssd(int32_t serial, const hidl_string& ussd) {
#if VDBG
    RLOGD("sendUssd: serial %d", serial);
#endif
  QCRIL_LOG_FUNC_ENTRY();
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<QcRilRequestSendUssdMessage>(getContext(serial));
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    if (ussd.empty()) {
      QCRIL_LOG_ERROR("Invalid parameter: ussd");
      sendFailure = true;
      break;
    }
    msg->setUssd(ussd.c_str());
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                       std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
          }
          RadioResponseInfo respInfo = {};
          populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, errorCode);
          RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, sendUssdResponse, respInfo);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
    RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, sendUssdResponse, respInfo);
  }
  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

Return<void> RadioImpl::cancelPendingUssd(int32_t serial) {
#if VDBG
    RLOGD("cancelPendingUssd: serial %d", serial);
#endif
  QCRIL_LOG_FUNC_ENTRY();
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<QcRilRequestCancelUssdMessage>(getContext(serial));
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                       std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
          }
          RadioResponseInfo respInfo = {};
          populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, errorCode);
          RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, cancelPendingUssdResponse, respInfo);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
    RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, cancelPendingUssdResponse, respInfo);
  }
  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

Return<void> RadioImpl::getClir(int32_t serial) {
#if VDBG
  RLOGD("getClir: serial %d", serial);
#endif
  QCRIL_LOG_FUNC_ENTRY();
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<QcRilRequestGetClirMessage>(getContext(serial));
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                       std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          int n = -1, m = -1;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
            auto clirInfo = std::static_pointer_cast<qcril::interfaces::ClirInfo>(resp->data);
            if (clirInfo) {
              if (clirInfo->hasAction()) {
                n = clirInfo->getAction();
              }
              if (clirInfo->hasPresentation()) {
                m = clirInfo->getPresentation();
              }
            }
          }
          RadioResponseInfo respInfo = {};
          populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, errorCode);
          RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getClirResponse, respInfo, n, m);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
    RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getClirResponse, respInfo, -1, -1);
  }
  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

Return<void> RadioImpl::setClir(int32_t serial, int32_t status) {
#if VDBG
  RLOGD("setClir: serial %d", serial);
#endif
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<QcRilRequestSetClirMessage>(getContext(serial));
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    msg->setParamN(status);
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                      std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
          }
          RadioResponseInfo respInfo = {};
          populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, errorCode);
          RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, setClirResponse, respInfo);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
    RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, setClirResponse, respInfo);
  }
  return Void();
}

Return<void> RadioImpl::getCallForwardStatus(int32_t serial, const CallForwardInfo &callInfo) {
#if VDBG
  RLOGD("getCallForwardStatus: serial %d", serial);
#endif
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<QcRilRequestQueryCallForwardMessage>(getContext(serial));
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    // Set parameters
    if (callInfo.reason != INT32_MAX) {
      msg->setReason(callInfo.reason);
    }
    if (callInfo.serviceClass != INT32_MAX) {
      msg->setServiceClass(callInfo.serviceClass);
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                       std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          hidl_vec<CallForwardInfo> callForwardInfos;
          std::shared_ptr<qcril::interfaces::GetCallForwardRespData> data = nullptr;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
            data = std::static_pointer_cast<qcril::interfaces::GetCallForwardRespData>(resp->data);
            auto cfInfoList = data->getCallForwardInfo();
            if (data && !cfInfoList.empty()) {
              uint32_t count = cfInfoList.size();
              callForwardInfos.resize(count);
              for (uint32_t i = 0; i < count; i++) {
                auto &resp = cfInfoList[i];
                if (resp.hasStatus()) {
                  callForwardInfos[i].status = convertCallForwardInfoStatus(resp.getStatus());
                }
                if (resp.hasReason()) {
                  callForwardInfos[i].reason = resp.getReason();
                }
                if (resp.hasServiceClass()) {
                  callForwardInfos[i].serviceClass = resp.getServiceClass();
                }
                if (resp.hasToa()) {
                  callForwardInfos[i].toa = resp.getToa();
                }
                if (resp.hasNumber() && !resp.getNumber().empty()) {
                  callForwardInfos[i].number = resp.getNumber().c_str();
                }
                if (resp.hasTimeSeconds()) {
                  callForwardInfos[i].timeSeconds = resp.getTimeSeconds();
                }
              }
            }
          }
          RadioResponseInfo respInfo = {};
          populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, errorCode);
          RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getCallForwardStatusResponse, respInfo,
                                   callForwardInfos);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    hidl_vec<CallForwardInfo> callForwardInfos;
    RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
    RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getCallForwardStatusResponse, respInfo,
                             callForwardInfos);
  }
  return Void();
}

Return<void> RadioImpl::setCallForward(int32_t serial, const CallForwardInfo &callInfo) {
#if VDBG
  RLOGD("setCallForward: serial %d", serial);
#endif
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<QcRilRequestSetCallForwardMessage>(getContext(serial));
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    // Set parameters
    msg->setStatus(convertCallForwardInfoStatus(callInfo.status));
    msg->setReason(callInfo.reason);
    msg->setServiceClass(callInfo.serviceClass);
    msg->setToa(callInfo.toa);
    if (!callInfo.number.empty()) {
      msg->setNumber(callInfo.number.c_str());
    }
    msg->setTimeSeconds(callInfo.timeSeconds);
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                       std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
          }
          RadioResponseInfo respInfo = {};
          populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, errorCode);
          RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, setCallForwardResponse, respInfo);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
    RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, setCallForwardResponse, respInfo);
  }
  return Void();
}

Return<void> RadioImpl::getCallWaiting(int32_t serial, int32_t serviceClass) {
#if VDBG
  RLOGD("getCallWaiting: serial %d", serial);
#endif
  QCRIL_LOG_FUNC_ENTRY();
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<QcRilRequestGetCallWaitingMessage>(getContext(serial));
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    // Set parameters
    msg->setServiceClass(serviceClass);
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                      std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          bool enable = false;
          int serviceClass = -1;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
            auto cwInfo = std::static_pointer_cast<qcril::interfaces::CallWaitingInfo>(resp->data);
            if (cwInfo->hasStatus()) {
              enable =
                  ((cwInfo->getStatus() == qcril::interfaces::ServiceClassStatus::ENABLED) ? true
                                                                                           : false);
            }
            if (cwInfo->hasServiceClass()) {
              serviceClass = cwInfo->getServiceClass();
            }
          }
          RadioResponseInfo respInfo = {};
          populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, errorCode);
          RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getCallWaitingResponse, respInfo,
                                   enable, serviceClass);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
    RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getCallWaitingResponse, respInfo, false, -1);
  }
  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

Return<void> RadioImpl::setCallWaiting(int32_t serial, bool enable, int32_t serviceClass) {
#if VDBG
  RLOGD("setCallWaiting: serial %d", serial);
#endif
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<QcRilRequestSetCallWaitingMessage>(getContext(serial));
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    // Set parameters
    if (enable) {
      msg->setServiceStatus(qcril::interfaces::ServiceClassStatus::ENABLED);
    } else {
      msg->setServiceStatus(qcril::interfaces::ServiceClassStatus::DISABLED);
    }
    msg->setServiceClass(serviceClass);
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                       std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
          }
          RadioResponseInfo respInfo = {};
          populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, errorCode);
          RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, setCallWaitingResponse, respInfo);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
    RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, setCallWaitingResponse, respInfo);
  }
  return Void();
}

Return<void> RadioImpl::acknowledgeLastIncomingGsmSms(int32_t serial,
                                                      bool success, SmsAcknowledgeFailCause cause) {
#if VDBG
    RLOGD("acknowledgeLastIncomingGsmSms: serial %d", serial);
#endif
    auto msg = std::make_shared<RilRequestAckGsmSmsMessage>(success,
            static_cast<int32_t>(cause));
    if (msg) {
        GenericCallback<RIL_Errno> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                            std::shared_ptr<RIL_Errno> responseDataPtr) -> void {
                if (solicitedMsg && responseDataPtr) {
                    RadioResponseInfo respInfo = {};
                    populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, *responseDataPtr);
                    RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse,
                            acknowledgeLastIncomingGsmSmsResponse, respInfo);
                }
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse,
                acknowledgeLastIncomingGsmSmsResponse, respInfo);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> RadioImpl::acceptCall(int32_t serial) {
#if VDBG
    RLOGD("acceptCall: serial %d", serial);
#endif
  QCRIL_LOG_FUNC_ENTRY("serial = %d", serial);
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<QcRilRequestCsAnswerMessage>(getContext(serial));
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                       std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
          }
          RadioResponseInfo respInfo = {};
          populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, errorCode);
          RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, acceptCallResponse, respInfo);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
    RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, acceptCallResponse, respInfo);
  }
  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

Return<void> RadioImpl::deactivateDataCall(int32_t serial,
                                           int32_t cid, bool reasonRadioShutDown) {
#if VDBG
    RLOGD("deactivateDataCall: serial %d", serial);
#endif
    dispatchStrings(serial, mSlotId, RIL_REQUEST_DEACTIVATE_DATA_CALL, true, 3,
        std::to_string(cid).c_str(),
        reasonRadioShutDown ? "1" : "0",
        NULL);
    return Void();
}

Return<void> RadioImpl::getFacilityLockForApp(int32_t serial, const hidl_string &facility,
                                              const hidl_string &password, int32_t serviceClass,
                                              const hidl_string &appId) {
#if VDBG
  RLOGD("getFacilityLockForApp: serial %d", serial);
#endif
  QCRIL_LOG_FUNC_ENTRY("serial = %d, facility = %s, serviceClass = %d", serial, facility.c_str(),
                       serviceClass);
  bool sendFailure = false;
  RadioError errResp = RadioError::NO_MEMORY;

  do {
    if (facility == "SC" || facility == "FD") {
      GenericCallback<RIL_UIM_GetFacilityLockResponseMsg> cb(
          [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                         std::shared_ptr<RIL_UIM_GetFacilityLockResponseMsg> resp) -> void {
            RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
            int32_t lockStatus = 0;
            (void)msg;
            if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
              errorCode = (RIL_Errno)resp->ril_err;
              lockStatus = resp->lock_status;
            }
            RadioResponseInfo respInfo = {};
            populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, errorCode);
            RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getFacilityLockForAppResponse,
                                     respInfo, lockStatus);
          });
      auto msg = std::make_shared<UimGetFacilityLockRequestMsg>(
          password, appId, (facility == "SC" ? PIN1 : FDN));
      if (msg == nullptr) {
        QCRIL_LOG_ERROR("msg is nullptr");
        sendFailure = true;
        break;
      }
      msg->setCallback(&cb);
      msg->dispatch();
      break;
    }
    qcril::interfaces::FacilityType rilFacility = convertFacilityType(facility);
    if (rilFacility != qcril::interfaces::FacilityType::UNKNOWN) {
      auto queryMsg = std::make_shared<QcRilRequestGetCallBarringMessage>(getContext(serial));
      if (queryMsg == nullptr) {
        QCRIL_LOG_ERROR("queryMsg is nullptr");
        sendFailure = true;
        break;
      }
      // Set parameters
      queryMsg->setFacilityType(rilFacility);
      queryMsg->setServiceClass(serviceClass);
      GenericCallback<QcRilRequestMessageCallbackPayload> cb(
          [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                         std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
            RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
            int32_t serviceClass = 0;
            (void)msg;
            if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
              errorCode = resp->errorCode;
              auto data =
                  std::static_pointer_cast<qcril::interfaces::SuppServiceStatusInfo>(resp->data);
              if (data->hasServiceClass()) {
                serviceClass = data->getServiceClass();
              }
            }
            RadioResponseInfo respInfo = {};
            populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, errorCode);
            RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getFacilityLockForAppResponse,
                                     respInfo, serviceClass);
          });
      queryMsg->setCallback(&cb);
      queryMsg->dispatch();
    } else {
      QCRIL_LOG_ERROR("Invalid Facility Type");
      errResp = RadioError::INVALID_ARGUMENTS;
      sendFailure = true;
    }
  } while (FALSE);
  if (sendFailure) {
    RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, errResp};
    RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getFacilityLockForAppResponse, respInfo, 0);
  }
  return Void();
}

Return<void> RadioImpl::setFacilityLockForApp(int32_t serial, const hidl_string &facility,
                                              bool lockState, const hidl_string &password,
                                              int32_t serviceClass, const hidl_string &appId) {
#if VDBG
  RLOGD("setFacilityLockForApp: serial %d", serial);
#endif
  QCRIL_LOG_FUNC_ENTRY("serial = %d, facility = %s, lockState = %d, serviceClass = %d", serial,
                       facility.c_str(), lockState, serviceClass);
  bool sendFailure = false;
  RadioError errResp = RadioError::NO_MEMORY;

  do {
    if (facility == "SC" || facility == "FD") {
      GenericCallback<RIL_UIM_SIM_PIN_Response> cb(
          [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                         std::shared_ptr<RIL_UIM_SIM_PIN_Response> resp) -> void {
            RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
            int32_t retry = 0;
            (void)msg;
            if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
              errorCode = (RIL_Errno)resp->err;
              retry = resp->no_of_retries;
            }
            RadioResponseInfo respInfo = {};
            populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, errorCode);
            RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, setFacilityLockForAppResponse,
                                     respInfo, retry);
          });
      auto msg = std::make_shared<UimSetFacilityLockRequestMsg>(
          password, appId, lockState, (facility == "SC" ? PIN1 : FDN));
      if (msg == nullptr) {
        QCRIL_LOG_ERROR("msg is nullptr");
        sendFailure = true;
        break;
      }
      msg->setCallback(&cb);
      msg->dispatch();
      break;
    }
    qcril::interfaces::FacilityType rilFacility = convertFacilityType(facility);
    if (rilFacility != qcril::interfaces::FacilityType::UNKNOWN) {
      auto setSupsMsg = std::make_shared<QcRilRequestSetSupsServiceMessage>(getContext(serial));
      if (setSupsMsg == nullptr) {
        QCRIL_LOG_ERROR("setSupsMsg is nullptr");
        sendFailure = true;
        break;
      }
      // Set parameters
      if (lockState) {
        setSupsMsg->setOperationType(1);  // activate
      } else {
        setSupsMsg->setOperationType(2);  // deactivate
      }
      setSupsMsg->setFacilityType(rilFacility);
      setSupsMsg->setServiceClass(serviceClass);
      if (password.size()) {
        setSupsMsg->setPassword(password.c_str());
      }
      GenericCallback<QcRilRequestMessageCallbackPayload> cb(
          [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                         std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
            RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
            (void)msg;
            if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
              errorCode = resp->errorCode;
            }
            RadioResponseInfo respInfo = {};
            populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, errorCode);
            RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, setFacilityLockForAppResponse,
                                     respInfo, -1);
          });
      setSupsMsg->setCallback(&cb);
      setSupsMsg->dispatch();
    } else {
      QCRIL_LOG_ERROR("Invalid Facility Type");
      errResp = RadioError::INVALID_ARGUMENTS;
      sendFailure = true;
    }
  } while (FALSE);
  if (sendFailure) {
    RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, errResp};
    RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, setFacilityLockForAppResponse, respInfo, -1);
  }
  return Void();
}

Return<void> RadioImpl::setBarringPassword(int32_t serial, const hidl_string &facility,
                                           const hidl_string &oldPassword,
                                           const hidl_string &newPassword) {
#if VDBG
  RLOGD("setBarringPassword: serial %d", serial);
#endif
  QCRIL_LOG_FUNC_ENTRY("serial = %d, facility = %s", serial, facility.c_str());
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<QcRilRequestSetCallBarringPasswordMessage>(getContext(serial));
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    // Set parameters
    if (!facility.empty()) {
      msg->setFacilityType(convertFacilityType(facility));
    } else {
      QCRIL_LOG_ERROR("invalid parameter: facility");
      sendFailure = true;
      break;
    }
    if (!oldPassword.empty()) {
      msg->setOldPassword(oldPassword.c_str());
    }
    if (!newPassword.empty()) {
      msg->setNewPassword(newPassword.c_str());
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                       std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
          }
          RadioResponseInfo respInfo = {};
          populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, errorCode);
          RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, setBarringPasswordResponse, respInfo);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
    RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, setBarringPasswordResponse, respInfo);
  }
  return Void();
}

Return<void> RadioImpl::getNetworkSelectionMode(int32_t serial) {
#if VDBG
    RLOGD("getNetworkSelectionMode: serial %d", serial);
#endif
    std::shared_ptr<RadioContext> ctx = getContext(serial);
    auto msg = std::make_shared<RilRequestQueryNetworkSelectModeMessage>(ctx);
    if (msg != nullptr) {
        GenericCallback<QcRilRequestMessageCallbackPayload> cb(
            [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                          std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
            RadioResponseInfo respInfo = {};
            std::shared_ptr<qcril::interfaces::RilGetSelectModeResult_t> rilModeResult;
            if (resp != nullptr) {
                populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, resp->errorCode);
                rilModeResult = std::static_pointer_cast<qcril::interfaces::RilGetSelectModeResult_t>(resp->data);
            } else {
                populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, RIL_E_NO_MEMORY);
            }
            if (rilModeResult != nullptr) {
                RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getNetworkSelectionModeResponse,
                    respInfo, rilModeResult->bManual);
            } else {
                RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getNetworkSelectionModeResponse,
                    respInfo, false);
            }
        });
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getNetworkSelectionModeResponse,
            respInfo, false);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> RadioImpl::setNetworkSelectionModeAutomatic(int32_t serial) {
#if VDBG
    RLOGD("setNetworkSelectionModeAutomatic: serial %d", serial);
#endif
    std::shared_ptr<RadioContext> ctx = getContext(serial);
    auto msg = std::make_shared<RilRequestSetNetworkSelectionAutoMessage>(ctx);
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
            RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, setNetworkSelectionModeAutomaticResponse, respInfo);
        });
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, setNetworkSelectionModeAutomaticResponse, respInfo);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> RadioImpl::setNetworkSelectionModeManual(int32_t serial,
                                                      const hidl_string& operatorNumeric) {
#if VDBG
    RLOGD("setNetworkSelectionModeManual: serial %d", serial);
#endif
    std::shared_ptr<RadioContext> ctx = getContext(serial);
    auto msg = std::make_shared<RilRequestSetNetworkSelectionManualMessage>(ctx, operatorNumeric);
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
            RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, setNetworkSelectionModeManualResponse, respInfo);
        });
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, setNetworkSelectionModeManualResponse, respInfo);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}


static OperatorStatus convertRilNetworkStatusToHidl(qcril::interfaces::NetworkStatus status) {
    OperatorStatus res = OperatorStatus::UNKNOWN;
    switch (status) {
    case qcril::interfaces::NetworkStatus::UNKNOWN:
        res = OperatorStatus::UNKNOWN;
        break;
    case qcril::interfaces::NetworkStatus::AVAILABLE:
        res = OperatorStatus::AVAILABLE;
        break;
    case qcril::interfaces::NetworkStatus::CURRENT:
        res = OperatorStatus::CURRENT;
        break;
    case qcril::interfaces::NetworkStatus::FORBIDDEN:
        res = OperatorStatus::FORBIDDEN;
        break;
    default:
        res = OperatorStatus::UNKNOWN;
        break;
    }

    return res;
}

static void convertRilNetworkResultToHidl(std::vector<qcril::interfaces::NetworkInfo>& rilData,
        hidl_vec<OperatorInfo>& hidlData) {
    hidlData.resize(rilData.size());
    for (size_t i = 0; i < rilData.size(); i++) {
        hidlData[i].alphaLong = rilData[i].alphaLong;
        hidlData[i].alphaShort = rilData[i].alphaShort;
        hidlData[i].operatorNumeric = rilData[i].operatorNumeric;
        hidlData[i].status = convertRilNetworkStatusToHidl(rilData[i].status);
    }
}

Return<void> RadioImpl::getAvailableNetworks(int32_t serial) {
#if VDBG
    RLOGD("getAvailableNetworks: serial %d", serial);
#endif
    std::shared_ptr<RadioContext> ctx = getContext(serial);
    auto msg = std::make_shared<RilRequestQueryAvailNetworkMessage>(ctx);
    if (msg != nullptr) {
        GenericCallback<QcRilRequestMessageCallbackPayload> cb(
            [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                          std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
            RadioResponseInfo respInfo = {};
            std::shared_ptr<qcril::interfaces::RilGetAvailNetworkResult_t> rilNetworkResult;
            if (resp != nullptr) {
                populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, resp->errorCode);
                rilNetworkResult = std::static_pointer_cast<qcril::interfaces::RilGetAvailNetworkResult_t>(resp->data);
            } else {
                populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, RIL_E_NO_MEMORY);
            }

            hidl_vec<OperatorInfo> networks;
            if (rilNetworkResult != nullptr) {
                convertRilNetworkResultToHidl(rilNetworkResult->info, networks);
            }
            RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getAvailableNetworksResponse,
                    respInfo, networks);
        });
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        hidl_vec<OperatorInfo> networks;
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getAvailableNetworksResponse,
                respInfo, networks);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

RadioError fillNetworkScanRequest_1_1(const V1_1::NetworkScanRequest& request,
        RIL_NetworkScanRequest &scanRequest) {

    if (request.specifiers.size() > MAX_RADIO_ACCESS_NETWORKS) {
        return RadioError::INVALID_ARGUMENTS;
    }

    if (request.type == V1_1::ScanType::ONE_SHOT) {
      scanRequest.type = RIL_ONE_SHOT;
    } else if (request.type == V1_1::ScanType::PERIODIC) {
      scanRequest.type = RIL_PERIODIC;
    }
    scanRequest.interval = request.interval;
    scanRequest.specifiers_length = request.specifiers.size();

    for (size_t i = 0; i < request.specifiers.size(); ++i) {
        if (request.specifiers[i].geranBands.size() > MAX_BANDS ||
            request.specifiers[i].utranBands.size() > MAX_BANDS ||
            request.specifiers[i].eutranBands.size() > MAX_BANDS ||
            request.specifiers[i].channels.size() > MAX_CHANNELS) {
            return RadioError::INVALID_ARGUMENTS;
        }
        const V1_1::RadioAccessSpecifier& ras_from =
                request.specifiers[i];
        RIL_RadioAccessSpecifier& ras_to = scanRequest.specifiers[i];

        ras_to.radio_access_network = (RIL_RadioAccessNetworks) ras_from.radioAccessNetwork;
        ras_to.channels_length = ras_from.channels.size();

        std::copy(ras_from.channels.begin(), ras_from.channels.end(), ras_to.channels);
        const std::vector<uint32_t> * bands = nullptr;
        switch (request.specifiers[i].radioAccessNetwork) {
            case V1_1::RadioAccessNetworks::GERAN:
                ras_to.bands_length = ras_from.geranBands.size();
                bands = (std::vector<uint32_t> *) &ras_from.geranBands;
                break;
            case V1_1::RadioAccessNetworks::UTRAN:
                ras_to.bands_length = ras_from.utranBands.size();
                bands = (std::vector<uint32_t> *) &ras_from.utranBands;
                break;
            case V1_1::RadioAccessNetworks::EUTRAN:
                ras_to.bands_length = ras_from.eutranBands.size();
                bands = (std::vector<uint32_t> *) &ras_from.eutranBands;
                break;
            default:
                return RadioError::INVALID_ARGUMENTS;
        }
        // safe to copy to geran_bands because it's a union member
        for (size_t idx = 0; idx < ras_to.bands_length; ++idx) {
            ras_to.bands.geran_bands[idx] = (RIL_GeranBands) (*bands)[idx];
        }
    }

    return RadioError::NONE;
}

Return<void> RadioImpl::startNetworkScan(int32_t serial, const V1_1::NetworkScanRequest& request) {
#if VDBG
    RLOGD("startNetworkScan: serial %d", serial);
#endif
    std::shared_ptr<RilRequestStartNetworkScanMessage> msg;
    RIL_NetworkScanRequest scanRequest = {};
    auto res = fillNetworkScanRequest_1_1(request, scanRequest);
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

Return<void> RadioImpl::stopNetworkScan(int32_t serial) {
#if VDBG
    RLOGD("stopNetworkScan: serial %d", serial);
#endif
    std::shared_ptr<RadioContext> ctx = getContext(serial);
    auto msg = std::make_shared<RilRequestStopNetworkScanMessage>(ctx);
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
            RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponseV1_1, stopNetworkScanResponse, respInfo);
        });
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponseV1_1, stopNetworkScanResponse, respInfo);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> RadioImpl::startDtmf(int32_t serial, const hidl_string &s) {
#if VDBG
  RLOGD("startDtmf: serial %d", serial);
#endif
  QCRIL_LOG_FUNC_ENTRY("serial = %d, s = %s", serial, s.c_str());
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<QcRilRequestStartDtmfMessage>(getContext(serial));
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    if (s.empty()) {
      QCRIL_LOG_ERROR("Invalid parameter: s");
      sendFailure = true;
      break;
    }
    msg->setDigit(s.c_str()[0]);
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                       std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
          } else if (status == Message::Callback::Status::CANCELLED) {
            errorCode = RIL_E_CANCELLED;
          }
          RadioResponseInfo respInfo = {};
          populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, errorCode);
          RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, startDtmfResponse, respInfo);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
    RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, startDtmfResponse, respInfo);
  }
  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

Return<void> RadioImpl::stopDtmf(int32_t serial) {
#if VDBG
  RLOGD("stopDtmf: serial %d", serial);
#endif
  QCRIL_LOG_FUNC_ENTRY("serial = %d", serial);
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<QcRilRequestStopDtmfMessage>(getContext(serial));
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                       std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
          } else if (status == Message::Callback::Status::CANCELLED) {
            errorCode = RIL_E_CANCELLED;
          }
          RadioResponseInfo respInfo = {};
          populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, errorCode);
          RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, stopDtmfResponse, respInfo);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
    RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, stopDtmfResponse, respInfo);
  }
  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

Return<void> RadioImpl::getBasebandVersion(int32_t serial) {
#if VDBG
    RLOGD("getBasebandVersion: serial %d", serial);
#endif

    std::shared_ptr<RadioContext> ctx = getContext(serial);
    auto msg = std::make_shared<RilRequestGetBaseBandVersionMessage>(ctx);
    if (msg != nullptr) {
        GenericCallback<QcRilRequestMessageCallbackPayload> cb(
            [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                          std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
            RadioResponseInfo respInfo = {};
            std::shared_ptr<qcril::interfaces::RilGetBaseBandResult_t> result;
            if (resp != nullptr) {
                populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, resp->errorCode);
                result =
                    std::static_pointer_cast<qcril::interfaces::RilGetBaseBandResult_t>(resp->data);
            } else {
                populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, RIL_E_NO_MEMORY);
            }
            if (result != nullptr) {
                RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getBasebandVersionResponse,
                    respInfo, result->version);
            } else {
                RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getBasebandVersionResponse,
                    respInfo, hidl_string());
            }
        });
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getBasebandVersionResponse,
            respInfo, hidl_string());
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> RadioImpl::separateConnection(int32_t serial, int32_t gsmIndex) {
#if VDBG
    RLOGD("separateConnection: serial %d", serial);
#endif
  QCRIL_LOG_FUNC_ENTRY();
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<QcRilRequestSeparateConnectionMessage>(getContext(serial));
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    msg->setCallId(gsmIndex);
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                       std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
          }
          RadioResponseInfo respInfo = {};
          populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, errorCode);
          RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, separateConnectionResponse, respInfo);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
    RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, separateConnectionResponse, respInfo);
  }
  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

Return<void> RadioImpl::setMute(int32_t serial, bool enable) {
  QCRIL_LOG_FUNC_ENTRY("serial %d, enable = %d", serial, enable);
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<QcRilRequestSetMuteMessage>(getContext(serial));
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    msg->setEnable(enable);
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, serial](std::shared_ptr<Message> /*msg*/, Message::Callback::Status status,
                       std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
          }
          RadioResponseInfo respInfo = {};
          populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, errorCode);
          RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, setMuteResponse, respInfo);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
    RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, setMuteResponse, respInfo);
  }
  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

Return<void> RadioImpl::getMute(int32_t serial) {
  QCRIL_LOG_FUNC_ENTRY("serial %d", serial);
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<QcRilRequestGetMuteMessage>(getContext(serial));
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, serial](std::shared_ptr<Message> /*msg*/, Message::Callback::Status status,
                       std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          bool enable = false;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
            auto muteInfo = std::static_pointer_cast<qcril::interfaces::MuteInfo>(resp->data);
            if (muteInfo) {
              if (muteInfo->hasEnable()) {
                enable = muteInfo->getEnable();
              }
            }
          }
          RadioResponseInfo respInfo = {};
          populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, errorCode);
          RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getMuteResponse, respInfo, enable);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
    RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getMuteResponse, respInfo, false);
  }
  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

Return<void> RadioImpl::getClip(int32_t serial) {
#if VDBG
  RLOGD("getClip: serial %d", serial);
#endif
  QCRIL_LOG_FUNC_ENTRY();
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<QcRilRequestGetClipMessage>(getContext(serial));
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                       std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          ClipStatus clip = ClipStatus::UNKNOWN;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
            auto clipInfo = std::static_pointer_cast<qcril::interfaces::ClipInfo>(resp->data);
            if (clipInfo && clipInfo->hasClipStatus()) {
              // The radio/1.0/types.hal defines the enum ClipStatus as below.
              // enum ClipStatus : int32_t {
              //     CLIP_PROVISIONED,                     // CLIP provisioned
              //     CLIP_UNPROVISIONED,                   // CLIP not provisioned
              //     UNKNOWN,                              // unknown, e.g. no network etc
              // };
              // But telephony is expecting the values as below.
              // 0 : CLIP not provisioned
              // 1 : Provisioned
              // Since all the previous IRadio implementations and the telephony clients are
              // expecting the clip status as mentioned above, intentionally setting the values as
              // below.
              if (clipInfo->getClipStatus() == qcril::interfaces::ClipStatus::NOT_PROVISIONED) {
                clip = (ClipStatus)0;  // 0 : CLIP not provisioned
              } else if (clipInfo->getClipStatus() == qcril::interfaces::ClipStatus::PROVISIONED) {
                clip = (ClipStatus)1;  // 1 : Provisioned
              }
            }
          }
          RadioResponseInfo respInfo = {};
          populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, errorCode);
          RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getClipResponse, respInfo, clip);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
    RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getClipResponse, respInfo,
                             ClipStatus::UNKNOWN);
  }
  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

Return<void> RadioImpl::getDataCallList(int32_t serial) {
#if VDBG
    RLOGD("getDataCallList: serial %d", serial);
#endif
    dispatchVoid(serial, mSlotId, RIL_REQUEST_DATA_CALL_LIST);
    return Void();
}

Return<void> RadioImpl::setSuppServiceNotifications(int32_t serial, bool enable) {
#if VDBG
  RLOGD("setSuppServiceNotifications: serial %d", serial);
#endif
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<QcRilRequestSetSuppSvcNotificationMessage>(getContext(serial));
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    // Set parameters
    msg->setStatus(enable);
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                       std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
          }
          RadioResponseInfo respInfo = {};
          populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, errorCode);
          RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, setSuppServiceNotificationsResponse,
                                   respInfo);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
    RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, setSuppServiceNotificationsResponse,
                             respInfo);
  }
  return Void();
}

Return<void> RadioImpl::writeSmsToSim(int32_t serial, const SmsWriteArgs& smsWriteArgs) {
#if VDBG
    RLOGD("writeSmsToSim: serial %d", serial);
#endif
    auto msg = std::make_shared<RilRequestWriteSmsToSimMessage>(smsWriteArgs.smsc,
            smsWriteArgs.pdu, static_cast<int>(smsWriteArgs.status));
    if (msg) {
        GenericCallback<RilWriteSmsToSimResult_t> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                            std::shared_ptr<RilWriteSmsToSimResult_t> responseDataPtr) -> void {
                if (solicitedMsg && responseDataPtr) {
                        RadioResponseInfo respInfo = {};
                        populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, responseDataPtr->rilErr);
                        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse,
                                writeSmsToSimResponse, respInfo, responseDataPtr->recordNumber);
                }
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse,
                writeSmsToSimResponse, respInfo, -1);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> RadioImpl::deleteSmsOnSim(int32_t serial, int32_t index) {
#if VDBG
    RLOGD("deleteSmsOnSim: serial %d", serial);
#endif
    auto msg = std::make_shared<RilRequestDeleteSmsOnSimMessage>(index);
    if (msg) {
        GenericCallback<RIL_Errno> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                            std::shared_ptr<RIL_Errno> responseDataPtr) -> void {
                if (solicitedMsg && responseDataPtr) {
                        RadioResponseInfo respInfo = {};
                        populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, *responseDataPtr);
                        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse,
                                deleteSmsOnSimResponse, respInfo);
                }
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse,
                deleteSmsOnSimResponse, respInfo);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> RadioImpl::setBandMode(int32_t serial, RadioBandMode mode) {
#if VDBG
    RLOGD("setBandMode: serial %d", serial);
#endif
    std::shared_ptr<RadioContext> ctx = getContext(serial);
    auto msg = std::make_shared<RilRequestSetBandModeMessage>(ctx,
            static_cast<int>(mode));
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
            RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, setBandModeResponse, respInfo);
        });
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, setBandModeResponse, respInfo);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> RadioImpl::getAvailableBandModes(int32_t serial) {
#if VDBG
    RLOGD("getAvailableBandModes: serial %d", serial);
#endif
    std::shared_ptr<RadioContext> ctx = getContext(serial);
    auto msg = std::make_shared<RilRequestQueryAvailBandModeMessage>(ctx);
    if (msg != nullptr) {
        GenericCallback<QcRilRequestMessageCallbackPayload> cb(
            [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                          std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
            RadioResponseInfo respInfo = {};
            std::shared_ptr<qcril::interfaces::RilQueryAvailBandsResult_t> rilBandResult;
            if (resp != nullptr) {
                populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, resp->errorCode);
                rilBandResult = std::static_pointer_cast<qcril::interfaces::RilQueryAvailBandsResult_t>(resp->data);
            } else {
                populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, RIL_E_NO_MEMORY);
            }
            if (rilBandResult != nullptr) {
                hidl_vec<RadioBandMode> modes;
                modes.resize(rilBandResult->bandList.size());
                for (unsigned int i = 0; i < rilBandResult->bandList.size(); i++) {
                    modes[i] = (RadioBandMode)rilBandResult->bandList[i];
                }
                RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getAvailableBandModesResponse,
                    respInfo, modes);
            } else {
                RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getAvailableBandModesResponse,
                    respInfo, hidl_vec<RadioBandMode>());
            }
        });
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getAvailableBandModesResponse,
            respInfo,  hidl_vec<RadioBandMode>());
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> RadioImpl::sendEnvelope(int32_t serial, const hidl_string& command) {
#if VDBG
    RLOGD("sendEnvelope: serial %d", serial);
#endif
    auto msg = std::make_shared<GstkSendEnvelopeRequestMsg>(serial, command);
    if (msg) {
        GenericCallback<RIL_GSTK_EnvelopeResponse> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                            std::shared_ptr<RIL_GSTK_EnvelopeResponse> responseDataPtr) -> void {
                RadioResponseInfo responseInfo{RadioResponseType::SOLICITED, serial, RadioError::INTERNAL_ERR};
                std::string       rsp_data = {};

                if (solicitedMsg && responseDataPtr &&
                    status == Message::Callback::Status::SUCCESS) {
                    responseInfo.error = (RadioError)responseDataPtr->err;
                    rsp_data = responseDataPtr->rsp;
                }
                RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, sendEnvelopeResponse, responseInfo, rsp_data);
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo rsp{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        std::string       rsp_data = {};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, sendEnvelopeResponse, rsp, rsp_data);
    }
    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> RadioImpl::sendTerminalResponseToSim(int32_t serial,
                                                  const hidl_string& commandResponse) {
#if VDBG
    RLOGD("sendTerminalResponseToSim: serial %d", serial);
#endif
    auto msg = std::make_shared<GstkSendTerminalResponseRequestMsg>(serial, commandResponse);
    if (msg) {
        GenericCallback<RIL_GSTK_Errno> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                            std::shared_ptr<RIL_GSTK_Errno> responseDataPtr) -> void {
                RadioResponseInfo responseInfo{RadioResponseType::SOLICITED, serial, RadioError::INTERNAL_ERR};

                if (solicitedMsg && responseDataPtr &&
                    status == Message::Callback::Status::SUCCESS) {
                    responseInfo.error = (RadioError)*responseDataPtr;
                }
                RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, sendTerminalResponseToSimResponse, responseInfo);
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo rsp{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, sendTerminalResponseToSimResponse, rsp);
    }
    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> RadioImpl::handleStkCallSetupRequestFromSim(int32_t serial, bool accept) {
#if VDBG
    RLOGD("handleStkCallSetupRequestFromSim: serial %d", serial);
#endif
    auto msg = std::make_shared<GstkSendSetupCallResponseRequestMsg>(accept);
    if (msg) {
        GenericCallback<RIL_GSTK_Errno> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                            std::shared_ptr<RIL_GSTK_Errno> responseDataPtr) -> void {
                RadioResponseInfo responseInfo{RadioResponseType::SOLICITED, serial, RadioError::INTERNAL_ERR};

                if (solicitedMsg && responseDataPtr &&
                    status == Message::Callback::Status::SUCCESS) {
                    responseInfo.error = (RadioError)*responseDataPtr;
                }
                RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, handleStkCallSetupRequestFromSimResponse, responseInfo);
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo rsp{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, handleStkCallSetupRequestFromSimResponse, rsp);
    }
    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> RadioImpl::explicitCallTransfer(int32_t serial) {
#if VDBG
    RLOGD("explicitCallTransfer: serial %d", serial);
#endif
  QCRIL_LOG_FUNC_ENTRY();
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<QcRilRequestExplicitCallTransferMessage>(getContext(serial));
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                       std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
          }
          RadioResponseInfo respInfo = {};
          populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, errorCode);
          RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, separateConnectionResponse, respInfo);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
    RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, separateConnectionResponse, respInfo);
  }
  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

Return<void> RadioImpl::setPreferredNetworkType(int32_t serial, PreferredNetworkType nwType) {
#if VDBG
    RLOGD("setPreferredNetworkType: serial %d", serial);
#endif
    uint32_t radioAccessFamily = convertNwTypeToRadioAccessFamily(nwType);
    std::shared_ptr<RadioContext> ctx = getContext(serial);
    auto msg = std::make_shared<RilRequestSetPrefNetworkTypeMessage>(ctx,
            radioAccessFamily);
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
            RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, setPreferredNetworkTypeResponse, respInfo);
        });
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, setPreferredNetworkTypeResponse, respInfo);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> RadioImpl::getPreferredNetworkType(int32_t serial) {
#if VDBG
    RLOGD("getPreferredNetworkType: serial %d", serial);
#endif
    std::shared_ptr<RadioContext> ctx = getContext(serial);
    auto msg = std::make_shared<RilRequestGetPrefNetworkTypeMessage>(ctx);
    if (msg != nullptr) {
        GenericCallback<QcRilRequestMessageCallbackPayload> cb(
            [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                          std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
            RadioResponseInfo respInfo = {};
            std::shared_ptr<qcril::interfaces::RilGetModePrefResult_t> prefResult;
            if (resp != nullptr) {
                populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, resp->errorCode);
                prefResult = std::static_pointer_cast<qcril::interfaces::RilGetModePrefResult_t>(resp->data);
            } else {
                populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, RIL_E_NO_MEMORY);
            }
            if (prefResult != nullptr) {
                PreferredNetworkType nwType = convertRadioAccessFamilyToNwType(prefResult->pref);
                RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getPreferredNetworkTypeResponse,
                    respInfo, nwType);
            } else {
                RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getPreferredNetworkTypeResponse,
                    respInfo, PreferredNetworkType::GSM_WCDMA/*defalut vaue */);
            }
        });
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getPreferredNetworkTypeResponse,
            respInfo, PreferredNetworkType::GSM_WCDMA/*defalut vaue */);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

void convertRilNeighboringCidResultToHidl(
        const std::vector<qcril::interfaces::RilNeighboringCell_t>& rilCell,
        hidl_vec<NeighboringCell>& hidlCell) {

    auto num = rilCell.size();
    hidlCell.resize(num);
    for (unsigned int i = 0 ; i < num; i++) {
        hidlCell[i].cid = rilCell[i].cid;
        hidlCell[i].rssi = rilCell[i].rssi;
    }
}

Return<void> RadioImpl::getNeighboringCids(int32_t serial) {
#if VDBG
    RLOGD("getNeighboringCids: serial %d", serial);
#endif
    std::shared_ptr<RadioContext> ctx = getContext(serial);
    auto msg = std::make_shared<RilRequestGetNeighborCellIdsMessage>(ctx);
    if (msg != nullptr) {
        GenericCallback<QcRilRequestMessageCallbackPayload> cb(
            [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                          std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
            RadioResponseInfo respInfo = {};
            std::shared_ptr<qcril::interfaces::RilGetNeighborCellIdResult_t> rilCellResult;
            if (resp != nullptr) {
                populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, resp->errorCode);
                rilCellResult = std::static_pointer_cast<qcril::interfaces::RilGetNeighborCellIdResult_t>(
                        resp->data);
            } else {
                populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, RIL_E_NO_MEMORY);
            }

            hidl_vec<NeighboringCell> cells;
            if (rilCellResult != nullptr) {
                convertRilNeighboringCidResultToHidl(rilCellResult->cellList, cells);
            }
            RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getNeighboringCidsResponse,
                    respInfo, cells);
        });
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        hidl_vec<NeighboringCell> cells;
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getNeighboringCidsResponse,
                respInfo, cells);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> RadioImpl::setLocationUpdates(int32_t serial, bool enable) {
#if VDBG
    RLOGD("setLocationUpdates: serial %d", serial);
#endif
    std::shared_ptr<RadioContext> ctx = getContext(serial);
    auto msg = std::make_shared<RilRequestSetLocationUpdateMessage>(ctx, enable);
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
            RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, setLocationUpdatesResponse, respInfo);
        });
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, setLocationUpdatesResponse, respInfo);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> RadioImpl::setCdmaSubscriptionSource(int32_t serial, CdmaSubscriptionSource cdmaSub) {
#if VDBG
    RLOGD("setCdmaSubscriptionSource: serial %d", serial);
#endif
    std::shared_ptr<RadioContext> ctx = getContext(serial);
    auto source = (cdmaSub == CdmaSubscriptionSource::RUIM_SIM) ?
        RilRequestSetCdmaSubscriptionSourceMessage::SubscriptionSource::SIM:
        RilRequestSetCdmaSubscriptionSourceMessage::SubscriptionSource::NV;
    auto msg = std::make_shared<RilRequestSetCdmaSubscriptionSourceMessage>(ctx, source);
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
            RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, setCdmaSubscriptionSourceResponse, respInfo);
        });
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, setCdmaSubscriptionSourceResponse, respInfo);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> RadioImpl::setCdmaRoamingPreference(int32_t serial, CdmaRoamingType type) {
#if VDBG
    RLOGD("setCdmaRoamingPreference: serial %d", serial);
#endif
    std::shared_ptr<RadioContext> ctx = getContext(serial);
    auto msg = std::make_shared<RilRequestSetCdmaRoamingPrefMessage>(ctx,
            static_cast<int>(type));
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
            RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, setCdmaRoamingPreferenceResponse, respInfo);
        });
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, setCdmaRoamingPreferenceResponse, respInfo);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> RadioImpl::getCdmaRoamingPreference(int32_t serial) {
#if VDBG
  RLOGD("getCdmaRoamingPreference: serial %d", serial);
#endif
  QCRIL_LOG_FUNC_ENTRY("serial %d", serial);
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<RilRequestQueryCdmaRoamingPrefMessage>(getContext(serial));
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, serial](std::shared_ptr<Message> /*msg*/, Message::Callback::Status status,
                       std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          CdmaRoamingType pref = CdmaRoamingType::HOME_NETWORK;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
            auto roamPref =
                std::static_pointer_cast<qcril::interfaces::RilQueryCdmaRoamingPrefResult_t>(
                    resp->data);
            if (roamPref) {
              pref = (CdmaRoamingType)roamPref->mPrefType;
            }
          }
          RadioResponseInfo respInfo = {};
          populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, errorCode);
          RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getCdmaRoamingPreferenceResponse,
                                   respInfo, pref);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    RadioResponseInfo respInfo{ RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY };
    RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getCdmaRoamingPreferenceResponse, respInfo,
                             CdmaRoamingType::HOME_NETWORK);
  }
  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

Return<void> RadioImpl::setTTYMode(int32_t serial, TtyMode mode) {
#if VDBG
  RLOGD("setTTYMode: serial %d", serial);
#endif
  QCRIL_LOG_FUNC_ENTRY();
  bool sendFailure = false;
  RadioError radioError = RadioError::NO_MEMORY;
  do {
    auto msg = std::make_shared<QcRilRequestSetTtyModeMessage>(getContext(serial));
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      radioError = RadioError::NO_MEMORY;
      sendFailure = true;
      break;
    }
#undef OFF
    qcril::interfaces::TtyMode ttyMode = qcril::interfaces::TtyMode::UNKNOWN;
    switch (mode) {
      case TtyMode::OFF:
        ttyMode = qcril::interfaces::TtyMode::MODE_OFF;
        break;
      case TtyMode::FULL:
        ttyMode = qcril::interfaces::TtyMode::FULL;
        break;
      case TtyMode::HCO:
        ttyMode = qcril::interfaces::TtyMode::HCO;
        break;
      case TtyMode::VCO:
        ttyMode = qcril::interfaces::TtyMode::VCO;
        break;
      default:
        QCRIL_LOG_ERROR("Invlid TtyMode");
        radioError = RadioError::INVALID_ARGUMENTS;
        sendFailure = true;
        break;
    }
    if (sendFailure) {
      break;
    }
    msg->setTtyMode(ttyMode);
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                       std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
          }
          RadioResponseInfo respInfo = {};
          populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, errorCode);
          RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, setTTYModeResponse, respInfo);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, radioError};
    RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, setTTYModeResponse, respInfo);
  }
  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

Return<void> RadioImpl::getTTYMode(int32_t serial) {
#if VDBG
  RLOGD("getTTYMode: serial %d", serial);
#endif
  QCRIL_LOG_FUNC_ENTRY();
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<QcRilRequestGetTtyModeMessage>(getContext(serial));
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                       std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          TtyMode mode = TtyMode::OFF;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
            auto ttyModeResp = std::static_pointer_cast<qcril::interfaces::TtyModeResp>(resp->data);
            if (ttyModeResp && ttyModeResp->hasTtyMode()) {
              switch (ttyModeResp->getTtyMode()) {
                case qcril::interfaces::TtyMode::MODE_OFF:
                  mode = TtyMode::OFF;
                  break;
                case qcril::interfaces::TtyMode::FULL:
                  mode = TtyMode::FULL;
                  break;
                case qcril::interfaces::TtyMode::HCO:
                  mode = TtyMode::HCO;
                  break;
                case qcril::interfaces::TtyMode::VCO:
                  mode = TtyMode::VCO;
                  break;
                case qcril::interfaces::TtyMode::UNKNOWN:
                  mode = TtyMode::OFF;
                  break;
              }
            }
          }
          RadioResponseInfo respInfo = {};
          populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, errorCode);
          RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getTTYModeResponse, respInfo, mode);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
    RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getTTYModeResponse, respInfo, TtyMode::OFF);
  }
  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

Return<void> RadioImpl::setPreferredVoicePrivacy(int32_t serial, bool enable) {
#if VDBG
  RLOGD("setPreferredVoicePrivacy: serial %d", serial);
#endif
  QCRIL_LOG_FUNC_ENTRY();
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<QcRilRequestSetPreferredVoicePrivacyMessage>(getContext(serial));
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    if (enable) {
      msg->setPrivacyMode(qcril::interfaces::PrivacyMode::ENHANCED);
    } else {
      msg->setPrivacyMode(qcril::interfaces::PrivacyMode::STANDARD);
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                       std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
          }
          RadioResponseInfo respInfo = {};
          populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, errorCode);
          RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, setPreferredVoicePrivacyResponse,
                                   respInfo);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
    RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, setPreferredVoicePrivacyResponse, respInfo);
  }
  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

Return<void> RadioImpl::getPreferredVoicePrivacy(int32_t serial) {
#if VDBG
  RLOGD("getPreferredVoicePrivacy: serial %d", serial);
#endif
  QCRIL_LOG_FUNC_ENTRY();
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<QcRilRequestGetPreferredVoicePrivacyMessage>(getContext(serial));
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                       std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          bool enable = false;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
            auto privacyModeResp =
                std::static_pointer_cast<qcril::interfaces::PrivacyModeResp>(resp->data);
            if (privacyModeResp) {
              if (privacyModeResp->hasPrivacyMode()) {
                if (privacyModeResp->getPrivacyMode() == qcril::interfaces::PrivacyMode::ENHANCED) {
                  enable = true;
                }
              }
            }
          }
          RadioResponseInfo respInfo = {};
          populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, errorCode);
          RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getPreferredVoicePrivacyResponse,
                                   respInfo, enable);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
    RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getPreferredVoicePrivacyResponse, respInfo,
                             false);
  }
  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

Return<void> RadioImpl::sendCDMAFeatureCode(int32_t serial, const hidl_string& featureCode) {
#if VDBG
    RLOGD("sendCDMAFeatureCode: serial %d", serial);
#endif
  QCRIL_LOG_FUNC_ENTRY();
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<QcRilRequestCdmaFlashMessage>(getContext(serial));
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    if (!featureCode.empty()) {
      msg->setFeatureCode(featureCode.c_str());
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                       std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
          }
          RadioResponseInfo respInfo = {};
          populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, errorCode);
          RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, sendCDMAFeatureCodeResponse, respInfo);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
    RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, sendCDMAFeatureCodeResponse, respInfo);
  }
  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

Return<void> RadioImpl::sendBurstDtmf(int32_t serial, const hidl_string& dtmf, int32_t on,
                                      int32_t off) {
#if VDBG
    RLOGD("sendBurstDtmf: serial %d", serial);
#endif
  QCRIL_LOG_FUNC_ENTRY();
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<QcRilRequestCdmaBurstDtmfMessage>(getContext(serial));
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    if (!dtmf.empty()) {
      msg->setDigitBuffer(dtmf.c_str());
    }
    msg->setDtmfOnLength(on);
    msg->setDtmfOffLength(off);
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                       std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
          }
          RadioResponseInfo respInfo = {};
          populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, errorCode);
          RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, sendBurstDtmfResponse, respInfo);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
    RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, sendBurstDtmfResponse, respInfo);
  }
  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

void constructCdmaSms(RIL_CDMA_SMS_Message &rcsm, const CdmaSmsMessage& sms) {
    rcsm.uTeleserviceID = sms.teleserviceId;
    rcsm.bIsServicePresent = BOOL_TO_INT(sms.isServicePresent);
    rcsm.uServicecategory = sms.serviceCategory;
    rcsm.sAddress.digit_mode = (RIL_CDMA_SMS_DigitMode) sms.address.digitMode;
    rcsm.sAddress.number_mode = (RIL_CDMA_SMS_NumberMode) sms.address.numberMode;
    rcsm.sAddress.number_type = (RIL_CDMA_SMS_NumberType) sms.address.numberType;
    rcsm.sAddress.number_plan = (RIL_CDMA_SMS_NumberPlan) sms.address.numberPlan;

    rcsm.sAddress.number_of_digits = sms.address.digits.size();
    int digitLimit= MIN((rcsm.sAddress.number_of_digits), RIL_CDMA_SMS_ADDRESS_MAX);
    for (int i = 0; i < digitLimit; i++) {
        rcsm.sAddress.digits[i] = sms.address.digits[i];
    }

    rcsm.sSubAddress.subaddressType = (RIL_CDMA_SMS_SubaddressType) sms.subAddress.subaddressType;
    rcsm.sSubAddress.odd = BOOL_TO_INT(sms.subAddress.odd);

    rcsm.sSubAddress.number_of_digits = sms.subAddress.digits.size();
    digitLimit= MIN((rcsm.sSubAddress.number_of_digits), RIL_CDMA_SMS_SUBADDRESS_MAX);
    for (int i = 0; i < digitLimit; i++) {
        rcsm.sSubAddress.digits[i] = sms.subAddress.digits[i];
    }

    rcsm.uBearerDataLen = sms.bearerData.size();
    digitLimit= MIN((rcsm.uBearerDataLen), RIL_CDMA_SMS_BEARER_DATA_MAX);
    for (int i = 0; i < digitLimit; i++) {
        rcsm.aBearerData[i] = sms.bearerData[i];
    }
}

Return<void> RadioImpl::sendCdmaSms(int32_t serial, const CdmaSmsMessage& sms) {
#if VDBG
    RLOGD("sendCdmaSms: serial %d", serial);
#endif
    RIL_CDMA_SMS_Message rcsm = {};
    constructCdmaSms(rcsm, sms);

    auto msg = std::make_shared<RilRequestCdmaSendSmsMessage>(rcsm);
    if (msg) {
        GenericCallback<RilSendSmsResult_t> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                            std::shared_ptr<RilSendSmsResult_t> responseDataPtr) -> void {
                if (solicitedMsg && responseDataPtr) {
                    RadioResponseInfo responseInfo = {};
                    SendSmsResult result = makeSendSmsResult(responseInfo, serial,
                            RESPONSE_SOLICITED, responseDataPtr);

                    RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, sendCdmaSmsResponse,
                            responseInfo, result);
                }
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo rsp{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        SendSmsResult result = {-1, hidl_string(), -1};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, sendCdmaSmsResponse, rsp, result);
    }
    QCRIL_LOG_FUNC_RETURN();

    return Void();
}

Return<void> RadioImpl::acknowledgeLastIncomingCdmaSms(int32_t serial, const CdmaSmsAck& smsAck) {
#if VDBG
    RLOGD("acknowledgeLastIncomingCdmaSms: serial %d", serial);
#endif
    auto msg = std::make_shared<RilRequestAckCdmaSmsMessage>(
            smsAck.errorClass == CdmaSmsErrorClass::NO_ERROR, smsAck.smsCauseCode);
    if (msg) {
        GenericCallback<RIL_Errno> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                            std::shared_ptr<RIL_Errno> responseDataPtr) -> void {
                if (solicitedMsg && responseDataPtr) {
                    RadioResponseInfo respInfo = {};
                    populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, *responseDataPtr);
                    RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse,
                            acknowledgeLastIncomingCdmaSmsResponse, respInfo);
                }
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse,
                acknowledgeLastIncomingCdmaSmsResponse, respInfo);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> RadioImpl::getGsmBroadcastConfig(int32_t serial) {
#if VDBG
    RLOGD("getGsmBroadcastConfig: serial %d", serial);
#endif
    auto msg = std::make_shared<RilRequestGetGsmBroadcastConfigMessage>();
    if (msg) {
        GenericCallback<RilGetGsmBroadcastConfigResult_t> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                            std::shared_ptr<RilGetGsmBroadcastConfigResult_t> responseDataPtr) -> void {
                if (solicitedMsg && responseDataPtr) {
                    // generate config list
                    hidl_vec<GsmBroadcastSmsConfigInfo> configs;
                    auto num = responseDataPtr->configList.size();
                    configs.resize(num);
                    for (unsigned int i = 0; i < num; i++) {
                        auto& item = responseDataPtr->configList[i];
                        configs[i].fromServiceId = item.fromServiceId;
                        configs[i].toServiceId = item.toServiceId;
                        configs[i].fromCodeScheme = item.fromCodeScheme;
                        configs[i].toCodeScheme = item.toCodeScheme;
                        configs[i].selected = item.selected == 1 ? true : false;
                    }
                    // generate the resp info
                    RadioResponseInfo respInfo = {};
                    populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, responseDataPtr->rilErr);

                    RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse,
                            getGsmBroadcastConfigResponse, respInfo, configs);
                }
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        hidl_vec<GsmBroadcastSmsConfigInfo> configs;
        RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse,
                getGsmBroadcastConfigResponse, respInfo, configs);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> RadioImpl::setGsmBroadcastConfig(int32_t serial,
                                              const hidl_vec<GsmBroadcastSmsConfigInfo>&
                                              configInfo) {
#if VDBG
    RLOGD("setGsmBroadcastConfig: serial %d", serial);
#endif
    int num = configInfo.size();
    std::vector<RIL_GSM_BroadcastSmsConfigInfo> configVec(num);

    for (int i = 0 ; i < num ; i++ ) {
        configVec[i].fromServiceId = configInfo[i].fromServiceId;
        configVec[i].toServiceId = configInfo[i].toServiceId;
        configVec[i].fromCodeScheme = configInfo[i].fromCodeScheme;
        configVec[i].toCodeScheme = configInfo[i].toCodeScheme;
        configVec[i].selected = BOOL_TO_INT(configInfo[i].selected);
    }

    auto msg = std::make_shared<RilRequestGsmSetBroadcastSmsConfigMessage>(
            std::move(configVec));
    if (msg) {
        GenericCallback<RIL_Errno> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                            std::shared_ptr<RIL_Errno> responseDataPtr) -> void {
                if (solicitedMsg && responseDataPtr) {
                        RadioResponseInfo respInfo = {};
                        populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, *responseDataPtr);
                        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse,
                                setGsmBroadcastConfigResponse, respInfo);
                }
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse,
                setGsmBroadcastConfigResponse, respInfo);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> RadioImpl::setGsmBroadcastActivation(int32_t serial, bool activate) {
#if VDBG
    RLOGD("setGsmBroadcastActivation: serial %d", serial);
#endif
    auto msg = std::make_shared<RilRequestGsmSmsBroadcastActivateMessage>(activate);
    if (msg) {
        GenericCallback<RIL_Errno> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                            std::shared_ptr<RIL_Errno> responseDataPtr) -> void {
                if (solicitedMsg && responseDataPtr) {
                    RadioResponseInfo respInfo = {};
                    populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, *responseDataPtr);
                    RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse,
                            setGsmBroadcastActivationResponse, respInfo);
                }
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse,
                setGsmBroadcastActivationResponse, respInfo);
    }

    return Void();
}

Return<void> RadioImpl::getCdmaBroadcastConfig(int32_t serial) {
#if VDBG
    RLOGD("getCdmaBroadcastConfig: serial %d", serial);
#endif
    auto msg = std::make_shared<RilRequestGetCdmaBroadcastConfigMessage>();
    if (msg) {
        GenericCallback<RilGetCdmaBroadcastConfigResult_t> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                            std::shared_ptr<RilGetCdmaBroadcastConfigResult_t> responseDataPtr) -> void {
                if (solicitedMsg && responseDataPtr) {
                    // generate config list
                    hidl_vec<CdmaBroadcastSmsConfigInfo> configs;
                    auto num = responseDataPtr->configList.size();
                    configs.resize(num);
                    for (unsigned int i = 0; i < num; i++) {
                        auto& item = responseDataPtr->configList[i];
                        configs[i].serviceCategory = item.service_category;
                        configs[i].language = item.language;
                        configs[i].selected = item.selected == 1 ? true : false;
                    }
                    // generate the resp info
                    RadioResponseInfo respInfo = {};
                    populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, responseDataPtr->rilErr);

                    RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse,
                            getCdmaBroadcastConfigResponse, respInfo, configs);
                }
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        hidl_vec<CdmaBroadcastSmsConfigInfo> configs;
        RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse,
                getCdmaBroadcastConfigResponse, respInfo, configs);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> RadioImpl::setCdmaBroadcastConfig(int32_t serial,
                                               const hidl_vec<CdmaBroadcastSmsConfigInfo>&
                                               configInfo) {
#if VDBG
    RLOGD("setCdmaBroadcastConfig: serial %d", serial);
#endif
    int num = configInfo.size();
    std::vector<RIL_CDMA_BroadcastSmsConfigInfo> configVec(num);

    for (int i = 0 ; i < num ; i++ ) {
        configVec[i].service_category = configInfo[i].serviceCategory;
        configVec[i].language = configInfo[i].language;
        configVec[i].selected = BOOL_TO_INT(configInfo[i].selected);
    }

    auto msg = std::make_shared<RilRequestCdmaSetBroadcastSmsConfigMessage>(
            std::move(configVec));
    if (msg) {
        GenericCallback<RIL_Errno> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                            std::shared_ptr<RIL_Errno> responseDataPtr) -> void {
                if (solicitedMsg && responseDataPtr) {
                        RadioResponseInfo respInfo = {};
                        populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, *responseDataPtr);
                        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse,
                                setCdmaBroadcastConfigResponse, respInfo);
                }
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse,
                setCdmaBroadcastConfigResponse, respInfo);
    }

    return Void();
}

Return<void> RadioImpl::setCdmaBroadcastActivation(int32_t serial, bool activate) {
#if VDBG
    RLOGD("setCdmaBroadcastActivation: serial %d", serial);
#endif
    auto msg = std::make_shared<RilRequestCdmaSmsBroadcastActivateMessage>(activate);
    if (msg) {
        GenericCallback<RIL_Errno> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                            std::shared_ptr<RIL_Errno> responseDataPtr) -> void {
                if (solicitedMsg && responseDataPtr) {
                    RadioResponseInfo respInfo = {};
                    populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, *responseDataPtr);
                    RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse,
                            setCdmaBroadcastActivationResponse, respInfo);
                }
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse,
                setCdmaBroadcastActivationResponse, respInfo);
    }

    return Void();
}

Return<void> RadioImpl::getCDMASubscription(int32_t serial) {
#if VDBG
    RLOGD("getCDMASubscription: serial %d", serial);
#endif
    std::shared_ptr<RadioContext> ctx = getContext(serial);
    auto msg = std::make_shared<RilRequestCdmaSubscriptionMessage>(ctx);
    if (msg != nullptr) {
        GenericCallback<QcRilRequestMessageCallbackPayload> cb(
            [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                          std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
            RadioResponseInfo respInfo = {};
            std::shared_ptr<qcril::interfaces::RilGetCdmaSubscriptionResult_t> rilCdmaSubResult;
            if (resp != nullptr) {
                populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, resp->errorCode);
                rilCdmaSubResult = std::static_pointer_cast<
                    qcril::interfaces::RilGetCdmaSubscriptionResult_t>(resp->data);
            } else {
                populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, RIL_E_NO_MEMORY);
            }

            if (rilCdmaSubResult != nullptr) {
                RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getCDMASubscriptionResponse,
                    respInfo, rilCdmaSubResult->mdn, rilCdmaSubResult->hSid,
                    rilCdmaSubResult->hNid, rilCdmaSubResult->min, rilCdmaSubResult->prl);
            } else {
                RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getCDMASubscriptionResponse,
                    respInfo, hidl_string(), hidl_string(), hidl_string(), hidl_string(),
                    hidl_string());
            }
        });
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getCDMASubscriptionResponse,
            respInfo, hidl_string(), hidl_string(), hidl_string(), hidl_string(), hidl_string());
    }

    QCRIL_LOG_FUNC_RETURN();

    return Void();
}

Return<void> RadioImpl::writeSmsToRuim(int32_t serial, const CdmaSmsWriteArgs& cdmaSms) {
#if VDBG
    RLOGD("writeSmsToRuim: serial %d", serial);
#endif
    RIL_CDMA_SMS_Message rcsm = {};
    constructCdmaSms(rcsm, cdmaSms.message);

    auto msg = std::make_shared<RilRequestCdmaWriteSmsToRuimMessage>(rcsm,
            static_cast<int>(cdmaSms.status));
    if (msg) {
        GenericCallback<RilWriteSmsToSimResult_t> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                            std::shared_ptr<RilWriteSmsToSimResult_t> responseDataPtr) -> void {
                if (solicitedMsg && responseDataPtr) {
                        RadioResponseInfo respInfo = {};
                        populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, responseDataPtr->rilErr);
                        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse,
                                writeSmsToRuimResponse, respInfo, responseDataPtr->recordNumber);
                }
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse,
                writeSmsToRuimResponse, respInfo, -1);
    }

    return Void();
}

Return<void> RadioImpl::deleteSmsOnRuim(int32_t serial, int32_t index) {
#if VDBG
    RLOGD("deleteSmsOnRuim: serial %d", serial);
#endif
    auto msg = std::make_shared<RilRequestCdmaDeleteSmsOnRuimMessage>(index);
    if (msg) {
        GenericCallback<RIL_Errno> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                            std::shared_ptr<RIL_Errno> responseDataPtr) -> void {
                if (solicitedMsg && responseDataPtr) {
                        RadioResponseInfo respInfo = {};
                        populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, *responseDataPtr);
                        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse,
                                deleteSmsOnRuimResponse, respInfo);
                }
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse,
                deleteSmsOnRuimResponse, respInfo);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> RadioImpl::getDeviceIdentity(int32_t serial) {
#if VDBG
    RLOGD("getDeviceIdentity: serial %d", serial);
#endif

    std::shared_ptr<RadioContext> ctx = getContext(serial);
    auto msg = std::make_shared<RilRequestGetDeviceIdentityMessage>(ctx);
    if (msg != nullptr) {
        GenericCallback<QcRilRequestMessageCallbackPayload> cb(
            [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                          std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
            RadioResponseInfo respInfo = {};
            std::shared_ptr<qcril::interfaces::RilDeviceIdentityResult_t> result;
            if (resp != nullptr) {
                populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, resp->errorCode);
                result =
                    std::static_pointer_cast<qcril::interfaces::RilDeviceIdentityResult_t>(resp->data);
            } else {
                populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, RIL_E_NO_MEMORY);
            }
            if (result != nullptr) {
                RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getDeviceIdentityResponse,
                    respInfo, result->imei, result->imeisv, result->esn, result->meid);
            } else {
                RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getDeviceIdentityResponse,
                    respInfo, hidl_string(), hidl_string(), hidl_string(), hidl_string());
            }
        });
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getDeviceIdentityResponse,
            respInfo, hidl_string(), hidl_string(), hidl_string(), hidl_string());
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> RadioImpl::exitEmergencyCallbackMode(int32_t serial) {
#if VDBG
    RLOGD("exitEmergencyCallbackMode: serial %d", serial);
#endif
    std::shared_ptr<RadioContext> ctx = getContext(serial);
    auto msg = std::make_shared<RilRequestExitEmergencyCallbackMessage>(ctx);
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
            RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, exitEmergencyCallbackModeResponse, respInfo);
        });
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, exitEmergencyCallbackModeResponse, respInfo);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> RadioImpl::getSmscAddress(int32_t serial) {
#if VDBG
    RLOGD("getSmscAddress: serial %d", serial);
#endif
    auto msg = std::make_shared<RilRequestGetSmscAddressMessage>();
    if (msg) {
        GenericCallback<RilGetSmscAddrResult_t> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                            std::shared_ptr<RilGetSmscAddrResult_t> responseDataPtr) -> void {
                if (solicitedMsg && responseDataPtr) {
                    RadioResponseInfo respInfo = {};
                    populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, responseDataPtr->rilErr);
                    RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse,
                            getSmscAddressResponse, respInfo, std::move(responseDataPtr->smscAddr))
                }
            }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse,
                getSmscAddressResponse, respInfo, hidl_string());
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> RadioImpl::setSmscAddress(int32_t serial, const hidl_string& smsc) {
#if VDBG
    RLOGD("setSmscAddress: serial %d", serial);
#endif
    auto msg = std::make_shared<RilRequestSetSmscAddressMessage>(smsc);
    if (msg) {
        GenericCallback<RIL_Errno> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                            std::shared_ptr<RIL_Errno> responseDataPtr) -> void {
                if (solicitedMsg && responseDataPtr) {
                    RadioResponseInfo respInfo = {};
                    populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, *responseDataPtr);
                    RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse,
                            setSmscAddressResponse, respInfo);
                }
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse,
                setSmscAddressResponse, respInfo);
    }

    return Void();
}

Return<void> RadioImpl::reportSmsMemoryStatus(int32_t serial, bool available) {
#if VDBG
    RLOGD("reportSmsMemoryStatus: serial %d", serial);
#endif
    auto msg = std::make_shared<RilRequestReportSmsMemoryStatusMessage>(available);
    if (msg) {
        GenericCallback<RIL_Errno> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                            std::shared_ptr<RIL_Errno> responseDataPtr) -> void {
                if (solicitedMsg && responseDataPtr) {
                        RadioResponseInfo respInfo = {};
                        populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, *responseDataPtr);
                        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse,
                                reportSmsMemoryStatusResponse, respInfo);
                }
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse,
                reportSmsMemoryStatusResponse, respInfo);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> RadioImpl::reportStkServiceIsRunning(int32_t serial) {
#if VDBG
    RLOGD("reportStkServiceIsRunning: serial %d", serial);
#endif
    auto msg = std::make_shared<GstkReportSTKIsRunningMsg>();
    if (msg) {
        GenericCallback<RIL_GSTK_Errno> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                            std::shared_ptr<RIL_GSTK_Errno> responseDataPtr) -> void {
                RadioResponseInfo responseInfo{RadioResponseType::SOLICITED, serial, RadioError::INTERNAL_ERR};

                if (solicitedMsg && responseDataPtr &&
                    status == Message::Callback::Status::SUCCESS) {
                    responseInfo.error = (RadioError)*responseDataPtr;
                }
                RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, reportStkServiceIsRunningResponse, responseInfo);
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo rsp{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, reportStkServiceIsRunningResponse, rsp);
    }
    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> RadioImpl::getCdmaSubscriptionSource(int32_t serial) {
#if VDBG
  RLOGD("getCdmaSubscriptionSource: serial %d", serial);
#endif
  QCRIL_LOG_FUNC_ENTRY("serial %d", serial);
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<RilRequestGetCdmaSubscriptionSourceMessage>(getContext(serial));
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, serial](std::shared_ptr<Message> /*msg*/, Message::Callback::Status status,
                       std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          CdmaSubscriptionSource pref = CdmaSubscriptionSource::RUIM_SIM;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
            auto srcResult =
                std::static_pointer_cast<qcril::interfaces::RilGetCdmaSubscriptionSourceResult_t>(
                    resp->data);
            if (srcResult) {
              pref = (CdmaSubscriptionSource)srcResult->mSource;
            }
          }
          RadioResponseInfo respInfo = {};
          populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, errorCode);
          RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getCdmaSubscriptionSourceResponse,
                                   respInfo, pref);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    RadioResponseInfo respInfo{ RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY };
    RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getCdmaSubscriptionSourceResponse, respInfo,
                             CdmaSubscriptionSource::RUIM_SIM);
  }
  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

Return<void> RadioImpl::requestIsimAuthentication(int32_t serial, const hidl_string& challenge) {
#if VDBG
    RLOGD("requestIsimAuthentication: serial %d", serial);
#endif
    auto msg = std::make_shared<UimISIMAuthenticationRequestMsg>(challenge);
    if (msg) {
        GenericCallback<RIL_UIM_SIM_IO_Response> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                            std::shared_ptr<RIL_UIM_SIM_IO_Response> responseDataPtr) -> void {
                RadioResponseInfo responseInfo{RadioResponseType::SOLICITED, serial, RadioError::INTERNAL_ERR};
                std::string rsp = {};

                if (solicitedMsg && responseDataPtr &&
                    status == Message::Callback::Status::SUCCESS) {
                    responseInfo.error = (RadioError)responseDataPtr->err;
                    rsp = responseDataPtr->simResponse;
                }
                RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, requestIsimAuthenticationResponse,
                  responseInfo, rsp);
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        std::string rsp = {};

        RadioResponseInfo rspInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, requestIsimAuthenticationResponse, rspInfo, rsp);
    }
    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> RadioImpl::acknowledgeIncomingGsmSmsWithPdu(int32_t serial, bool success,
                                                         const hidl_string& ackPdu) {
#if VDBG
    RLOGD("acknowledgeIncomingGsmSmsWithPdu: serial %d", serial);
#endif
    dispatchStrings(serial, mSlotId, RIL_REQUEST_ACKNOWLEDGE_INCOMING_GSM_SMS_WITH_PDU, false,
            2, success ? "1" : "0", ackPdu.c_str());
    return Void();
}

Return<void> RadioImpl::sendEnvelopeWithStatus(int32_t serial, const hidl_string& contents) {
#if VDBG
    RLOGD("sendEnvelopeWithStatus: serial %d", serial);
#endif
    auto msg = std::make_shared<GstkSendEnvelopeRequestMsg>(serial, contents);
    if (msg) {
        GenericCallback<RIL_GSTK_EnvelopeResponse> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                            std::shared_ptr<RIL_GSTK_EnvelopeResponse> responseDataPtr) -> void {
                RadioResponseInfo responseInfo{RadioResponseType::SOLICITED, serial, RadioError::INTERNAL_ERR};
                IccIoResult       rsp_data = {};

                if (solicitedMsg && responseDataPtr &&
                    status == Message::Callback::Status::SUCCESS) {
                    responseInfo.error = (RadioError)responseDataPtr->err;
                    rsp_data.simResponse = responseDataPtr->rsp;
                    rsp_data.sw1 = responseDataPtr->sw1;
                    rsp_data.sw2 = responseDataPtr->sw2;
                }
                RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, sendEnvelopeWithStatusResponse, responseInfo, rsp_data);
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo rsp{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        IccIoResult       rsp_data = {};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, sendEnvelopeWithStatusResponse, rsp, rsp_data);
    }
    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> RadioImpl::getVoiceRadioTechnology(int32_t serial) {
#if VDBG
    RLOGD("getVoiceRadioTechnology: serial %d", serial);
#endif
    std::shared_ptr<RadioContext> ctx = getContext(serial);
    auto msg = std::make_shared<RilRequestGetVoiceRadioTechMessage>(ctx);
    if (msg != nullptr) {
        GenericCallback<QcRilRequestMessageCallbackPayload> cb(
            [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                          std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
            RadioResponseInfo respInfo = {};
            std::shared_ptr<qcril::interfaces::RilGetVoiceTechResult_t> ratResult;
            if (resp != nullptr) {
                populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, resp->errorCode);
                ratResult = std::static_pointer_cast<qcril::interfaces::RilGetVoiceTechResult_t>(resp->data);
            } else {
                populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, RIL_E_NO_MEMORY);
            }
            if (ratResult != nullptr) {
                RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getVoiceRadioTechnologyResponse,
                    respInfo, (RadioTechnology)ratResult->rat);
            } else {
                RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getVoiceRadioTechnologyResponse,
                    respInfo, RadioTechnology::UNKNOWN/*defalut vaue */);
            }
        });
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getVoiceRadioTechnologyResponse,
            respInfo, RadioTechnology::UNKNOWN/*defalut vaue */);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> RadioImpl::getCellInfoList(int32_t serial) {
#if VDBG
    RLOGD("getCellInfoList: serial %d", serial);
#endif
    dispatchVoid(serial, mSlotId, RIL_REQUEST_GET_CELL_INFO_LIST);
    return Void();
}

Return<void> RadioImpl::setCellInfoListRate(int32_t serial, int32_t rate) {
  QCRIL_LOG_FUNC_ENTRY("serial %d, rate = %d", serial, rate);
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<RilRequestSetUnsolCellInfoListRateMessage>(getContext(serial));
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    msg->setRate(rate);
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, serial](std::shared_ptr<Message> /*msg*/, Message::Callback::Status status,
                       std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
          }
          RadioResponseInfo respInfo = {};
          populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, errorCode);
          RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, setCellInfoListRateResponse, respInfo);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
    RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, setCellInfoListRateResponse, respInfo);
  }
  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

Return<void> RadioImpl::setInitialAttachApn(int32_t serial, const DataProfileInfo& dataProfileInfo,
                                            bool modemCognitive, bool isRoaming) {
#if VDBG
    RLOGD("setInitialAttachApn: serial %d", serial);
#endif
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
            RIL_REQUEST_SET_INITIAL_ATTACH_APN);
    if (pRI == NULL) {
        return Void();
    }

    if (s_vendorFunctions->version <= 14) {
        RIL_InitialAttachApn iaa = {};

        if (!copyHidlStringToRil(&iaa.apn, dataProfileInfo.apn, pRI, true)) {
            return Void();
        }

        const hidl_string &protocol =
                (isRoaming ? dataProfileInfo.roamingProtocol : dataProfileInfo.protocol);

        if (!copyHidlStringToRil(&iaa.protocol, protocol, pRI)) {
            memsetAndFreeStrings(1, iaa.apn);
            return Void();
        }
        iaa.authtype = (int) dataProfileInfo.authType;
        if (!copyHidlStringToRil(&iaa.username, dataProfileInfo.user, pRI)) {
            memsetAndFreeStrings(2, iaa.apn, iaa.protocol);
            return Void();
        }
        if (!copyHidlStringToRil(&iaa.password, dataProfileInfo.password, pRI)) {
            memsetAndFreeStrings(3, iaa.apn, iaa.protocol, iaa.username);
            return Void();
        }

        CALL_ONREQUEST(RIL_REQUEST_SET_INITIAL_ATTACH_APN, &iaa, sizeof(iaa), pRI, mSlotId);

        memsetAndFreeStrings(4, iaa.apn, iaa.protocol, iaa.username, iaa.password);
    } else {
        RIL_InitialAttachApn_v15 iaa = {};

        if (!copyHidlStringToRil(&iaa.apn, dataProfileInfo.apn, pRI, true)) {
            return Void();
        }

        if (!copyHidlStringToRil(&iaa.protocol, dataProfileInfo.protocol, pRI)) {
            memsetAndFreeStrings(1, iaa.apn);
            return Void();
        }
        if (!copyHidlStringToRil(&iaa.roamingProtocol, dataProfileInfo.roamingProtocol, pRI)) {
            memsetAndFreeStrings(2, iaa.apn, iaa.protocol);
            return Void();
        }
        iaa.authtype = (int) dataProfileInfo.authType;
        if (!copyHidlStringToRil(&iaa.username, dataProfileInfo.user, pRI)) {
            memsetAndFreeStrings(3, iaa.apn, iaa.protocol, iaa.roamingProtocol);
            return Void();
        }
        if (!copyHidlStringToRil(&iaa.password, dataProfileInfo.password, pRI)) {
            memsetAndFreeStrings(4, iaa.apn, iaa.protocol, iaa.roamingProtocol, iaa.username);
            return Void();
        }
        iaa.supportedTypesBitmask = dataProfileInfo.supportedApnTypesBitmap;
        iaa.bearerBitmask = dataProfileInfo.bearerBitmap;
        iaa.modemCognitive = BOOL_TO_INT(modemCognitive);
        iaa.mtu = dataProfileInfo.mtu;

        if (!convertMvnoTypeToString(dataProfileInfo.mvnoType, iaa.mvnoType)) {
            sendErrorResponse(pRI, RIL_E_INVALID_ARGUMENTS);
            memsetAndFreeStrings(5, iaa.apn, iaa.protocol, iaa.roamingProtocol, iaa.username,
                    iaa.password);
            return Void();
        }

        if (!copyHidlStringToRil(&iaa.mvnoMatchData, dataProfileInfo.mvnoMatchData, pRI)) {
            memsetAndFreeStrings(5, iaa.apn, iaa.protocol, iaa.roamingProtocol, iaa.username,
                    iaa.password);
            return Void();
        }

        CALL_ONREQUEST(RIL_REQUEST_SET_INITIAL_ATTACH_APN, &iaa, sizeof(iaa), pRI, mSlotId);

        memsetAndFreeStrings(6, iaa.apn, iaa.protocol, iaa.roamingProtocol, iaa.username,
                iaa.password, iaa.mvnoMatchData);
    }

    return Void();
}

Return<void> RadioImpl::getImsRegistrationState(int32_t serial) {
#if VDBG
    RLOGD("getImsRegistrationState: serial %d", serial);
#endif
    auto msg = std::make_shared<RilRequestGetImsRegistrationMessage>();
    if (msg) {
        GenericCallback<RilGetImsRegistrationResult_t> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                            std::shared_ptr<RilGetImsRegistrationResult_t> responseDataPtr) -> void {
                if (solicitedMsg && responseDataPtr) {
                    RadioResponseInfo respInfo = {};
                    populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, responseDataPtr->rilErr);
                    RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getImsRegistrationStateResponse,
                            respInfo, responseDataPtr->isRegistered,
                            responseDataPtr->ratFamily == RADIO_TECH_3GPP ?
                            RadioTechnologyFamily::THREE_GPP : RadioTechnologyFamily::THREE_GPP2)
                }
            }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getImsRegistrationStateResponse,
                respInfo, false, RadioTechnologyFamily::THREE_GPP);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

bool dispatchImsGsmSms(const ImsSmsMessage& message, RequestInfo *pRI) {
    RIL_IMS_SMS_Message rism = {};
    char **pStrings;
    int countStrings = 2;
    int dataLen = sizeof(char *) * countStrings;

    rism.tech = RADIO_TECH_3GPP;
    rism.retry = BOOL_TO_INT(message.retry);
    rism.messageRef = message.messageRef;

    if (message.gsmMessage.size() != 1) {
        RLOGE("dispatchImsGsmSms: Invalid len %s", requestToString(pRI->pCI->requestNumber));
        sendErrorResponse(pRI, RIL_E_INVALID_ARGUMENTS);
        return false;
    }

    pStrings = (char **)calloc(countStrings, sizeof(char *));
    if (pStrings == NULL) {
        RLOGE("dispatchImsGsmSms: Memory allocation failed for request %s",
                requestToString(pRI->pCI->requestNumber));
        sendErrorResponse(pRI, RIL_E_NO_MEMORY);
        return false;
    }

    if (!copyHidlStringToRil(&pStrings[0], message.gsmMessage[0].smscPdu, pRI)) {
#ifdef MEMSET_FREED
        memset(pStrings, 0, dataLen);
#endif
        free(pStrings);
        return false;
    }

    if (!copyHidlStringToRil(&pStrings[1], message.gsmMessage[0].pdu, pRI)) {
        memsetAndFreeStrings(1, pStrings[0]);
#ifdef MEMSET_FREED
        memset(pStrings, 0, dataLen);
#endif
        free(pStrings);
        return false;
    }

    rism.message.gsmMessage = pStrings;
    CALL_ONREQUEST(pRI->pCI->requestNumber, &rism, sizeof(RIL_RadioTechnologyFamily) +
            sizeof(uint8_t) + sizeof(int32_t) + dataLen, pRI, pRI->socket_id);

    for (int i = 0 ; i < countStrings ; i++) {
        memsetAndFreeStrings(1, pStrings[i]);
    }

#ifdef MEMSET_FREED
    memset(pStrings, 0, dataLen);
#endif
    free(pStrings);

    return true;
}

struct ImsCdmaSms {
    RIL_IMS_SMS_Message imsSms;
    RIL_CDMA_SMS_Message cdmaSms;
};

bool dispatchImsCdmaSms(const ImsSmsMessage& message, RequestInfo *pRI) {
    ImsCdmaSms temp = {};

    if (message.cdmaMessage.size() != 1) {
        RLOGE("dispatchImsCdmaSms: Invalid len %s", requestToString(pRI->pCI->requestNumber));
        sendErrorResponse(pRI, RIL_E_INVALID_ARGUMENTS);
        return false;
    }

    temp.imsSms.tech = RADIO_TECH_3GPP2;
    temp.imsSms.retry = BOOL_TO_INT(message.retry);
    temp.imsSms.messageRef = message.messageRef;
    temp.imsSms.message.cdmaMessage = &temp.cdmaSms;

    constructCdmaSms(temp.cdmaSms, message.cdmaMessage[0]);

    // Vendor code expects payload length to include actual msg payload
    // (sizeof(RIL_CDMA_SMS_Message)) instead of (RIL_CDMA_SMS_Message *) + size of other fields in
    // RIL_IMS_SMS_Message
    int payloadLen = sizeof(RIL_RadioTechnologyFamily) + sizeof(uint8_t) + sizeof(int32_t)
            + sizeof(RIL_CDMA_SMS_Message);

    CALL_ONREQUEST(pRI->pCI->requestNumber, &temp.imsSms, payloadLen, pRI, pRI->socket_id);

    return true;
}

Return<void> RadioImpl::sendImsSms(int32_t serial, const ImsSmsMessage& message) {
#if VDBG
    RLOGD("sendImsSms: serial %d", serial);
#endif
    bool sendResponse = false;
    RadioError errorResponse = RadioError::GENERIC_FAILURE;
    do {
        RIL_RadioTechnologyFamily tech = (RIL_RadioTechnologyFamily) message.tech;
        auto msg = std::make_shared<RilRequestImsSendSmsMessage>(message.messageRef,
            tech, message.retry);
        if (msg == nullptr) {
            sendResponse = true;
            errorResponse = RadioError::NO_MEMORY;
            break;
        }
        if (tech == RADIO_TECH_3GPP) {
            std::string payload;
            if (message.gsmMessage.size() != 1) {
                errorResponse = RadioError::INVALID_ARGUMENTS;
                sendResponse = true;
                break;
            }
            string gsmPdu;
            string gsmSmsc;
            QCRIL_LOG_INFO("sendImsSms: pdu size = %d", message.gsmMessage[0].pdu.size());

            if (message.gsmMessage[0].pdu.size() >0) {
                gsmPdu = message.gsmMessage[0].pdu.c_str();
            } else {
                errorResponse = RadioError::INVALID_ARGUMENTS;
                sendResponse = true;
                break;
            }

            if (message.gsmMessage[0].smscPdu.size() >0) {
                gsmSmsc = message.gsmMessage[0].smscPdu.c_str();
            } else {
                errorResponse = RadioError::INVALID_ARGUMENTS;
                sendResponse = true;
                break;
            }
            msg->setGsmPayload(gsmSmsc, gsmPdu);
        } else {
            //set cmda payload
            if (message.cdmaMessage.size() != 1) {
                errorResponse = RadioError::INVALID_ARGUMENTS;
                sendResponse = true;
                break;
            }

            RIL_CDMA_SMS_Message rcsm = {};
            constructCdmaSms(rcsm, message.cdmaMessage[0]);
            msg->setCdmaPayload(rcsm);
        }

        GenericCallback<RilSendSmsResult_t> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                            std::shared_ptr<RilSendSmsResult_t> responseDataPtr) -> void {
                if (solicitedMsg && responseDataPtr) {
                    RadioResponseInfo responseInfo = {};
                    SendSmsResult result = makeSendSmsResult(responseInfo, serial,
                            RESPONSE_SOLICITED, responseDataPtr);

                    RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, sendImsSmsResponse,
                            responseInfo, result);
                }
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } while (false);

    if (sendResponse) {
        RadioResponseInfo rsp{RadioResponseType::SOLICITED, serial, errorResponse};
        SendSmsResult result = {-1, hidl_string(), -1};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, sendImsSmsResponse, rsp, result);
    }
    return Void();
}

Return<void> RadioImpl::iccTransmitApduBasicChannel(int32_t serial, const SimApdu& message) {
#if VDBG
    RLOGD("iccTransmitApduBasicChannel: serial %d", serial);
#endif
    RIL_UIM_SIM_APDU data = {};
    data.sessionid = message.sessionId;
    data.cla = message.cla;
    data.instruction = message.instruction;
    data.p1 = message.p1;
    data.p2 = message.p2;
    data.p3 = message.p3;
    data.data = message.data;

    auto msg = std::make_shared<UimTransmitAPDURequestMsg>(qmi_ril_get_process_instance_id(), true, true, data);
    if (msg) {
        GenericCallback<RIL_UIM_SIM_IO_Response> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                            std::shared_ptr<RIL_UIM_SIM_IO_Response> responseDataPtr) -> void {
                RadioResponseInfo responseInfo{RadioResponseType::SOLICITED, serial, RadioError::INTERNAL_ERR};
                IccIoResult  result = {};

                if (solicitedMsg && responseDataPtr &&
                    status == Message::Callback::Status::SUCCESS) {
                    responseInfo.error = (RadioError)responseDataPtr->err;
                    result.sw1 = responseDataPtr->sw1;
                    result.sw2 = responseDataPtr->sw2;
                    result.simResponse = responseDataPtr->simResponse;
                }
                RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, iccTransmitApduBasicChannelResponse,
                        responseInfo, result);
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo rsp{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        IccIoResult  result = {};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, iccTransmitApduBasicChannelResponse, rsp, result);
    }
    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> RadioImpl::iccOpenLogicalChannel(int32_t serial, const hidl_string& aid, int32_t p2) {
#if VDBG
    RLOGD("iccOpenLogicalChannel: serial %d", serial);
#endif
    auto msg = std::make_shared<UimSIMOpenChannelRequestMsg>(aid, p2);
    if (msg) {
        GenericCallback<RIL_UIM_OpenChannelResponse> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                            std::shared_ptr<RIL_UIM_OpenChannelResponse> responseDataPtr) -> void {
                RadioResponseInfo responseInfo{RadioResponseType::SOLICITED, serial, RadioError::INTERNAL_ERR};
                hidl_vec<int8_t> selectResponse = {};
                int32_t ch_id = -1;

                if (solicitedMsg && responseDataPtr &&
                    status == Message::Callback::Status::SUCCESS) {
                    responseInfo.error = (RadioError)responseDataPtr->err;
                    selectResponse.resize(responseDataPtr->selectResponse.size());
                    for (uint32_t i = 0; i < responseDataPtr->selectResponse.size(); i++) {
                        selectResponse[i] = responseDataPtr->selectResponse[i];
                    }
                    ch_id = responseDataPtr->channel_id;
                }
                RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, iccOpenLogicalChannelResponse,
                        responseInfo, ch_id, selectResponse);
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo rsp{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        hidl_vec<int8_t> selectResponse = {};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, iccOpenLogicalChannelResponse, rsp, -1, selectResponse);
    }
    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> RadioImpl::iccCloseLogicalChannel(int32_t serial, int32_t channelId) {
#if VDBG
    RLOGD("iccCloseLogicalChannel: serial %d", serial);
#endif
    auto msg = std::make_shared<UimSIMCloseChannelRequestMsg>(channelId);
    if (msg) {
        GenericCallback<RIL_UIM_Errno> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                            std::shared_ptr<RIL_UIM_Errno> responseDataPtr) -> void {
                RadioResponseInfo responseInfo{RadioResponseType::SOLICITED, serial, RadioError::INTERNAL_ERR};

                if (solicitedMsg && responseDataPtr && responseDataPtr.get() &&
                    status == Message::Callback::Status::SUCCESS) {
                    responseInfo.error = (RadioError)*(responseDataPtr.get());
                }
                RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, iccCloseLogicalChannelResponse,
                        responseInfo);
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo rsp{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, iccCloseLogicalChannelResponse, rsp);
    }
    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> RadioImpl::iccTransmitApduLogicalChannel(int32_t serial, const SimApdu& message) {
#if VDBG
    RLOGD("iccTransmitApduLogicalChannel: serial %d", serial);
#endif
    RIL_UIM_SIM_APDU data = {};
    data.sessionid = message.sessionId;
    data.cla = message.cla;
    data.instruction = message.instruction;
    data.p1 = message.p1;
    data.p2 = message.p2;
    data.p3 = message.p3;
    data.data = message.data;

    auto msg = std::make_shared<UimTransmitAPDURequestMsg>(qmi_ril_get_process_instance_id(), false, true, data);
    if (msg) {
        GenericCallback<RIL_UIM_SIM_IO_Response> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                            std::shared_ptr<RIL_UIM_SIM_IO_Response> responseDataPtr) -> void {
                RadioResponseInfo responseInfo{RadioResponseType::SOLICITED, serial, RadioError::INTERNAL_ERR};
                IccIoResult  result = {};

                if (solicitedMsg && responseDataPtr &&
                    status == Message::Callback::Status::SUCCESS) {
                    responseInfo.error = (RadioError)responseDataPtr->err;
                    result.sw1 = responseDataPtr->sw1;
                    result.sw2 = responseDataPtr->sw2;
                    result.simResponse = responseDataPtr->simResponse;
                }
                RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, iccTransmitApduLogicalChannelResponse,
                        responseInfo, result);
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo rsp{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        IccIoResult  result = {};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, iccTransmitApduLogicalChannelResponse, rsp, result);
    }
    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> RadioImpl::nvReadItem(int32_t serial, NvItem itemId) {
#if VDBG
    RLOGD("nvReadItem: serial %d", serial);
#endif
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId, RIL_REQUEST_NV_READ_ITEM);
    if (pRI == NULL) {
        return Void();
    }

    RIL_NV_ReadItem nvri = {};
    nvri.itemID = (RIL_NV_Item) itemId;

    CALL_ONREQUEST(pRI->pCI->requestNumber, &nvri, sizeof(nvri), pRI, mSlotId);
    return Void();
}

Return<void> RadioImpl::nvWriteItem(int32_t serial, const NvWriteItem& item) {
#if VDBG
    RLOGD("nvWriteItem: serial %d", serial);
#endif
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId, RIL_REQUEST_NV_WRITE_ITEM);
    if (pRI == NULL) {
        return Void();
    }

    RIL_NV_WriteItem nvwi = {};

    nvwi.itemID = (RIL_NV_Item) item.itemId;

    if (!copyHidlStringToRil(&nvwi.value, item.value, pRI)) {
        return Void();
    }

    CALL_ONREQUEST(pRI->pCI->requestNumber, &nvwi, sizeof(nvwi), pRI, mSlotId);

    memsetAndFreeStrings(1, nvwi.value);
    return Void();
}

Return<void> RadioImpl::nvWriteCdmaPrl(int32_t serial, const hidl_vec<uint8_t>& prl) {
#if VDBG
    RLOGD("nvWriteCdmaPrl: serial %d", serial);
#endif
    dispatchRaw(serial, mSlotId, RIL_REQUEST_NV_WRITE_CDMA_PRL, prl);
    return Void();
}

Return<void> RadioImpl::nvResetConfig(int32_t serial, ResetNvType resetType) {
    int rilResetType = -1;
#if VDBG
    RLOGD("nvResetConfig: serial %d", serial);
#endif
    /* Convert ResetNvType to RIL.h values
     * RIL_REQUEST_NV_RESET_CONFIG
     * 1 - reload all NV items
     * 2 - erase NV reset (SCRTN)
     * 3 - factory reset (RTN)
     */
    switch(resetType) {
      case ResetNvType::RELOAD:
        rilResetType = 1;
        break;
      case ResetNvType::ERASE:
        rilResetType = 2;
        break;
      case ResetNvType::FACTORY_RESET:
        rilResetType = 3;
        break;
    }
    dispatchInts(serial, mSlotId, RIL_REQUEST_NV_RESET_CONFIG, 1, rilResetType);
    return Void();
}

Return<void> RadioImpl::setUiccSubscription(int32_t serial, const SelectUiccSub& uiccSub) {
#if VDBG
    RLOGD("setUiccSubscription: serial %d", serial);
#endif
    // convert HIDL type to RIL internal structure
    RIL_SelectUiccSub rilUiccSub = {};
    rilUiccSub.slot = uiccSub.slot;
    rilUiccSub.app_index = uiccSub.appIndex;
    rilUiccSub.sub_type = (RIL_SubscriptionType) uiccSub.subType;
    rilUiccSub.act_status = (RIL_UiccSubActStatus) uiccSub.actStatus;

    // Create message and callback
    std::shared_ptr<RadioContext> ctx = getContext(serial);
    auto msg = std::make_shared<RilRequestSetUiccSubsMessage>(ctx, rilUiccSub);
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
            RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, setUiccSubscriptionResponse, respInfo);
        });
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, setUiccSubscriptionResponse, respInfo);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> RadioImpl::setDataAllowed(int32_t serial, bool allow) {
#if VDBG
    RLOGD("setDataAllowed: serial %d", serial);
#endif
    std::shared_ptr<RadioContext> ctx = getContext(serial);
    auto msg = std::make_shared<RilRequestAllowDataMessage>(ctx, allow);
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
            RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, setDataAllowedResponse, respInfo);
        });
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, setDataAllowedResponse, respInfo);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> RadioImpl::getHardwareConfig(int32_t serial) {
#if VDBG
    RLOGD("getHardwareConfig: serial %d", serial);
#endif
    dispatchVoid(serial, mSlotId, RIL_REQUEST_GET_HARDWARE_CONFIG);
    return Void();
}

Return<void> RadioImpl::requestIccSimAuthentication(int32_t serial, int32_t authContext,
        const hidl_string& authData, const hidl_string& aid) {
#if VDBG
    RLOGD("requestIccSimAuthentication: serial %d", serial);
#endif
    auto msg = std::make_shared<UimSIMAuthenticationRequestMsg>(authContext, authData, aid);
    if (msg) {
        GenericCallback<RIL_UIM_SIM_IO_Response> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                            std::shared_ptr<RIL_UIM_SIM_IO_Response> responseDataPtr) -> void {
                RadioResponseInfo responseInfo{RadioResponseType::SOLICITED, serial, RadioError::INTERNAL_ERR};
                IccIoResult  result = {};

                if (solicitedMsg && responseDataPtr &&
                    status == Message::Callback::Status::SUCCESS) {
                    responseInfo.error = (RadioError)responseDataPtr->err;
                    result.sw1 = responseDataPtr->sw1;
                    result.sw2 = responseDataPtr->sw2;
                    result.simResponse = responseDataPtr->simResponse;
                }
                RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, requestIccSimAuthenticationResponse,
                        responseInfo, result);
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo rsp{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        IccIoResult  result = {};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, requestIccSimAuthenticationResponse, rsp, result);
    }
    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

/**
 * @param numProfiles number of data profile
 * @param dataProfiles the pointer to the actual data profiles. The acceptable type is
          RIL_DataProfileInfo or RIL_DataProfileInfo_v15.
 * @param dataProfilePtrs the pointer to the pointers that point to each data profile structure
 * @param numfields number of string-type member in the data profile structure
 * @param ... the variadic parameters are pointers to each string-type member
 **/
template <typename T>
void freeSetDataProfileData(int numProfiles, T *dataProfiles, T **dataProfilePtrs,
                            int numfields, ...) {
    va_list args;
    va_start(args, numfields);

    // Iterate through each string-type field that need to be free.
    for (int i = 0; i < numfields; i++) {
        // Iterate through each data profile and free that specific string-type field.
        // The type 'char *T::*' is a type of pointer to a 'char *' member inside T structure.
        char *T::*ptr = va_arg(args, char *T::*);
        for (int j = 0; j < numProfiles; j++) {
            memsetAndFreeStrings(1, dataProfiles[j].*ptr);
        }
    }

    va_end(args);

#ifdef MEMSET_FREED
    memset(dataProfiles, 0, numProfiles * sizeof(T));
    memset(dataProfilePtrs, 0, numProfiles * sizeof(T *));
#endif
    free(dataProfiles);
    free(dataProfilePtrs);
}

Return<void> RadioImpl::setDataProfile(int32_t serial, const hidl_vec<DataProfileInfo>& profiles,
                                       bool isRoaming) {
#if VDBG
    RLOGD("setDataProfile: serial %d", serial);
#endif
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId, RIL_REQUEST_SET_DATA_PROFILE);
    if (pRI == NULL) {
        return Void();
    }

    size_t num = profiles.size();
    bool success = false;

    if (s_vendorFunctions->version <= 14) {

        RIL_DataProfileInfo *dataProfiles =
            (RIL_DataProfileInfo *) calloc(num, sizeof(RIL_DataProfileInfo));

        if (dataProfiles == NULL) {
            RLOGE("Memory allocation failed for request %s",
                    requestToString(pRI->pCI->requestNumber));
            sendErrorResponse(pRI, RIL_E_NO_MEMORY);
            return Void();
        }

        RIL_DataProfileInfo **dataProfilePtrs =
            (RIL_DataProfileInfo **) calloc(num, sizeof(RIL_DataProfileInfo *));
        if (dataProfilePtrs == NULL) {
            RLOGE("Memory allocation failed for request %s",
                    requestToString(pRI->pCI->requestNumber));
            free(dataProfiles);
            sendErrorResponse(pRI, RIL_E_NO_MEMORY);
            return Void();
        }

        for (size_t i = 0; i < num; i++) {
            dataProfilePtrs[i] = &dataProfiles[i];

            success = copyHidlStringToRil(&dataProfiles[i].apn, profiles[i].apn, pRI, true);

            const hidl_string &protocol =
                    (isRoaming ? profiles[i].roamingProtocol : profiles[i].protocol);

            if (success && !copyHidlStringToRil(&dataProfiles[i].protocol, protocol, pRI, true)) {
                success = false;
            }

            if (success && !copyHidlStringToRil(&dataProfiles[i].user, profiles[i].user, pRI,
                    true)) {
                success = false;
            }
            if (success && !copyHidlStringToRil(&dataProfiles[i].password, profiles[i].password,
                    pRI, true)) {
                success = false;
            }

            if (!success) {
                freeSetDataProfileData(num, dataProfiles, dataProfilePtrs, 4,
                    &RIL_DataProfileInfo::apn, &RIL_DataProfileInfo::protocol,
                    &RIL_DataProfileInfo::user, &RIL_DataProfileInfo::password);
                return Void();
            }

            dataProfiles[i].profileId = (RIL_DataProfile) profiles[i].profileId;
            dataProfiles[i].authType = (int) profiles[i].authType;
            dataProfiles[i].type = (int) profiles[i].type;
            dataProfiles[i].maxConnsTime = profiles[i].maxConnsTime;
            dataProfiles[i].maxConns = profiles[i].maxConns;
            dataProfiles[i].waitTime = profiles[i].waitTime;
            dataProfiles[i].enabled = BOOL_TO_INT(profiles[i].enabled);
        }

        CALL_ONREQUEST(RIL_REQUEST_SET_DATA_PROFILE, dataProfilePtrs,
                num * sizeof(RIL_DataProfileInfo *), pRI, mSlotId);

        freeSetDataProfileData(num, dataProfiles, dataProfilePtrs, 4,
                &RIL_DataProfileInfo::apn, &RIL_DataProfileInfo::protocol,
                &RIL_DataProfileInfo::user, &RIL_DataProfileInfo::password);
    } else {
        RIL_DataProfileInfo_v15 *dataProfiles =
            (RIL_DataProfileInfo_v15 *) calloc(num, sizeof(RIL_DataProfileInfo_v15));

        if (dataProfiles == NULL) {
            RLOGE("Memory allocation failed for request %s",
                    requestToString(pRI->pCI->requestNumber));
            sendErrorResponse(pRI, RIL_E_NO_MEMORY);
            return Void();
        }

        RIL_DataProfileInfo_v15 **dataProfilePtrs =
            (RIL_DataProfileInfo_v15 **) calloc(num, sizeof(RIL_DataProfileInfo_v15 *));
        if (dataProfilePtrs == NULL) {
            RLOGE("Memory allocation failed for request %s",
                    requestToString(pRI->pCI->requestNumber));
            free(dataProfiles);
            sendErrorResponse(pRI, RIL_E_NO_MEMORY);
            return Void();
        }

        for (size_t i = 0; i < num; i++) {
            dataProfilePtrs[i] = &dataProfiles[i];

            success = copyHidlStringToRil(&dataProfiles[i].apn, profiles[i].apn, pRI, true);
            if (success && !copyHidlStringToRil(&dataProfiles[i].protocol, profiles[i].protocol,
                    pRI)) {
                success = false;
            }
            if (success && !copyHidlStringToRil(&dataProfiles[i].roamingProtocol,
                    profiles[i].roamingProtocol, pRI, true)) {
                success = false;
            }
            if (success && !copyHidlStringToRil(&dataProfiles[i].user, profiles[i].user, pRI,
                    true)) {
                success = false;
            }
            if (success && !copyHidlStringToRil(&dataProfiles[i].password, profiles[i].password,
                    pRI, true)) {
                success = false;
            }
            if (success && !copyHidlStringToRil(&dataProfiles[i].mvnoMatchData,
                    profiles[i].mvnoMatchData, pRI, true)) {
                success = false;
            }

            if (success && !convertMvnoTypeToString(profiles[i].mvnoType,
                    dataProfiles[i].mvnoType)) {
                sendErrorResponse(pRI, RIL_E_INVALID_ARGUMENTS);
                success = false;
            }

            if (!success) {
                freeSetDataProfileData(num, dataProfiles, dataProfilePtrs, 6,
                    &RIL_DataProfileInfo_v15::apn, &RIL_DataProfileInfo_v15::protocol,
                    &RIL_DataProfileInfo_v15::roamingProtocol, &RIL_DataProfileInfo_v15::user,
                    &RIL_DataProfileInfo_v15::password, &RIL_DataProfileInfo_v15::mvnoMatchData);
                return Void();
            }

            dataProfiles[i].profileId = (RIL_DataProfile) profiles[i].profileId;
            dataProfiles[i].authType = (int) profiles[i].authType;
            dataProfiles[i].type = (int) profiles[i].type;
            dataProfiles[i].maxConnsTime = profiles[i].maxConnsTime;
            dataProfiles[i].maxConns = profiles[i].maxConns;
            dataProfiles[i].waitTime = profiles[i].waitTime;
            dataProfiles[i].enabled = BOOL_TO_INT(profiles[i].enabled);
            dataProfiles[i].supportedTypesBitmask = profiles[i].supportedApnTypesBitmap;
            dataProfiles[i].bearerBitmask = profiles[i].bearerBitmap;
            dataProfiles[i].mtu = profiles[i].mtu;
        }

        CALL_ONREQUEST(RIL_REQUEST_SET_DATA_PROFILE, dataProfilePtrs,
                num * sizeof(RIL_DataProfileInfo_v15 *), pRI, mSlotId);

        freeSetDataProfileData(num, dataProfiles, dataProfilePtrs, 6,
                &RIL_DataProfileInfo_v15::apn, &RIL_DataProfileInfo_v15::protocol,
                &RIL_DataProfileInfo_v15::roamingProtocol, &RIL_DataProfileInfo_v15::user,
                &RIL_DataProfileInfo_v15::password, &RIL_DataProfileInfo_v15::mvnoMatchData);
    }

    return Void();
}

Return<void> RadioImpl::requestShutdown(int32_t serial) {
#if VDBG
    RLOGD("requestShutdown: serial %d", serial);
#endif
    std::shared_ptr<RadioContext> ctx = getContext(serial);
    auto msg = std::make_shared<RilRequestShutDownMessage>(ctx);
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
            RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, requestShutdownResponse, respInfo);
        });
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, requestShutdownResponse, respInfo);
    }

    QCRIL_LOG_FUNC_RETURN();

    return Void();
}

Return<void> RadioImpl::getRadioCapability(int32_t serial) {
#if VDBG
    RLOGD("getRadioCapability: serial %d", serial);
#endif
    std::shared_ptr<RadioContext> ctx = getContext(serial);
    auto msg = std::make_shared<RilRequestGetRadioCapMessage>(ctx);
    if (msg != nullptr) {
        GenericCallback<QcRilRequestMessageCallbackPayload> cb(
            [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                          std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
            if (resp != nullptr) {
                RadioResponseInfo respInfo = {};
                RadioCapability result = {};
                populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, resp->errorCode);
                auto rilCapResult = std::static_pointer_cast<qcril::interfaces::RilRadioCapResult_t>(resp->data);
                if (rilCapResult != nullptr) {
                    convertRilRadioCapabilityToHal(rilCapResult->respData, result);
                }
                RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse,
                        getRadioCapabilityResponse, respInfo, result);
            }
        });
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        RadioCapability result = {};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse,
                getRadioCapabilityResponse, respInfo, result);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> RadioImpl::setRadioCapability(int32_t serial, const RadioCapability& rc) {
#if VDBG
    RLOGD("setRadioCapability: serial %d", serial);
#endif
    // convert HIDL structure to internel data structure
    RIL_RadioCapability rilRc = {};

    // TODO : set rilRc.version using HIDL version ?
    rilRc.session = rc.session;
    rilRc.phase = (int) rc.phase;
    rilRc.rat = (int) rc.raf;
    rilRc.status = (int) rc.status;
    strlcpy(rilRc.logicalModemUuid, rc.logicalModemUuid.c_str(), sizeof(rilRc.logicalModemUuid));

    // Create message and callback
    std::shared_ptr<RadioContext> ctx = getContext(serial);
    auto msg = std::make_shared<RilRequestSetRadioCapMessage>(ctx, rilRc);
    if (msg != nullptr) {
        GenericCallback<QcRilRequestMessageCallbackPayload> cb(
            [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                          std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
            if (resp != nullptr) {
                RadioResponseInfo respInfo = {};
                RadioCapability result = {};
                populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, resp->errorCode);
                auto rilCapResult = std::static_pointer_cast<qcril::interfaces::RilRadioCapResult_t>(resp->data);
                if (rilCapResult != nullptr) {
                    convertRilRadioCapabilityToHal(rilCapResult->respData, result);
                }
                RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse,
                        getRadioCapabilityResponse, respInfo, result);
            }
        });
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        RadioCapability result = {};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse,
                getRadioCapabilityResponse, respInfo, result);
    }

    QCRIL_LOG_FUNC_RETURN();

    return Void();
}

Return<void> RadioImpl::startLceService(int32_t serial, int32_t reportInterval, bool pullMode) {
#if VDBG
    RLOGD("startLceService: serial %d", serial);
#endif
    dispatchInts(serial, mSlotId, RIL_REQUEST_START_LCE, 2, reportInterval,
            BOOL_TO_INT(pullMode));
    return Void();
}

Return<void> RadioImpl::stopLceService(int32_t serial) {
#if VDBG
    RLOGD("stopLceService: serial %d", serial);
#endif
    dispatchVoid(serial, mSlotId, RIL_REQUEST_STOP_LCE);
    return Void();
}

Return<void> RadioImpl::pullLceData(int32_t serial) {
#if VDBG
    RLOGD("pullLceData: serial %d", serial);
#endif
    dispatchVoid(serial, mSlotId, RIL_REQUEST_PULL_LCEDATA);
    return Void();
}

Return<void> RadioImpl::getModemActivityInfo(int32_t serial) {
#if VDBG
    RLOGD("getModemActivityInfo: serial %d", serial);
#endif

    std::shared_ptr<RadioContext> ctx = getContext(serial);
    auto msg = std::make_shared<RilRequestGetModemActivityMessage>(ctx);
    if (msg != nullptr) {
        GenericCallback<QcRilRequestMessageCallbackPayload> cb(
            [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                          std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
            ActivityStatsInfo info = {};
            RadioResponseInfo respInfo = {};
            std::shared_ptr<qcril::interfaces::RilGetModemActivityResult_t> result;
            if (resp != nullptr) {
                populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, resp->errorCode);
                result =
                    std::static_pointer_cast<qcril::interfaces::RilGetModemActivityResult_t>(resp->data);

                if (result != nullptr) {
                    RIL_ActivityStatsInfo resp = (RIL_ActivityStatsInfo)result->respData;
                    info.sleepModeTimeMs = resp.sleep_mode_time_ms;
                    info.idleModeTimeMs = resp.idle_mode_time_ms;
                    for(int i = 0; i < RIL_NUM_TX_POWER_LEVELS; i++) {
                        info.txmModetimeMs[i] = resp.tx_mode_time_ms[i];
                    }
                    info.rxModeTimeMs = resp.rx_mode_time_ms;
                }
            } else {
                populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, RIL_E_NO_MEMORY);
            }
            if (result != nullptr) {
                RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getModemActivityInfoResponse,
                    respInfo, info);
            } else {
                RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getModemActivityInfoResponse,
                    respInfo, info);
            }
        });
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        ActivityStatsInfo info = {};
        RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, getModemActivityInfoResponse,
            respInfo, info);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> RadioImpl::setAllowedCarriers(int32_t serial, bool allAllowed,
                                           const CarrierRestrictions& carriers) {
#if VDBG
    RLOGD("setAllowedCarriers: serial %d", serial);
#endif
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
            RIL_REQUEST_SET_CARRIER_RESTRICTIONS);
    if (pRI == NULL) {
        return Void();
    }

    RIL_CarrierRestrictions cr = {};
    RIL_Carrier *allowedCarriers = NULL;
    RIL_Carrier *excludedCarriers = NULL;

    cr.len_allowed_carriers = carriers.allowedCarriers.size();
    allowedCarriers = (RIL_Carrier *)calloc(cr.len_allowed_carriers, sizeof(RIL_Carrier));
    if (allowedCarriers == NULL) {
        RLOGE("setAllowedCarriers: Memory allocation failed for request %s",
                requestToString(pRI->pCI->requestNumber));
        sendErrorResponse(pRI, RIL_E_NO_MEMORY);
        return Void();
    }
    cr.allowed_carriers = allowedCarriers;

    cr.len_excluded_carriers = carriers.excludedCarriers.size();
    excludedCarriers = (RIL_Carrier *)calloc(cr.len_excluded_carriers, sizeof(RIL_Carrier));
    if (excludedCarriers == NULL) {
        RLOGE("setAllowedCarriers: Memory allocation failed for request %s",
                requestToString(pRI->pCI->requestNumber));
        sendErrorResponse(pRI, RIL_E_NO_MEMORY);
#ifdef MEMSET_FREED
        memset(allowedCarriers, 0, cr.len_allowed_carriers * sizeof(RIL_Carrier));
#endif
        free(allowedCarriers);
        return Void();
    }
    cr.excluded_carriers = excludedCarriers;

    for (int i = 0; i < cr.len_allowed_carriers; i++) {
        allowedCarriers[i].mcc = carriers.allowedCarriers[i].mcc.c_str();
        allowedCarriers[i].mnc = carriers.allowedCarriers[i].mnc.c_str();
        allowedCarriers[i].match_type = (RIL_CarrierMatchType) carriers.allowedCarriers[i].matchType;
        allowedCarriers[i].match_data = carriers.allowedCarriers[i].matchData.c_str();
    }

    for (int i = 0; i < cr.len_excluded_carriers; i++) {
        excludedCarriers[i].mcc = carriers.excludedCarriers[i].mcc.c_str();
        excludedCarriers[i].mnc = carriers.excludedCarriers[i].mnc.c_str();
        excludedCarriers[i].match_type =
                (RIL_CarrierMatchType) carriers.excludedCarriers[i].matchType;
        excludedCarriers[i].match_data = carriers.excludedCarriers[i].matchData.c_str();
    }

    CALL_ONREQUEST(pRI->pCI->requestNumber, &cr, sizeof(RIL_CarrierRestrictions), pRI, mSlotId);

#ifdef MEMSET_FREED
    memset(allowedCarriers, 0, cr.len_allowed_carriers * sizeof(RIL_Carrier));
    memset(excludedCarriers, 0, cr.len_excluded_carriers * sizeof(RIL_Carrier));
#endif
    free(allowedCarriers);
    free(excludedCarriers);
    return Void();
}

Return<void> RadioImpl::getAllowedCarriers(int32_t serial) {
#if VDBG
    RLOGD("getAllowedCarriers: serial %d", serial);
#endif
    dispatchVoid(serial, mSlotId, RIL_REQUEST_GET_CARRIER_RESTRICTIONS);
    return Void();
}

Return<void> RadioImpl::sendDeviceState(int32_t serial, DeviceStateType deviceStateType,
                                        bool state) {
#if VDBG
    RLOGD("sendDeviceState: serial %d", serial);
#endif
    std::shared_ptr<QcRilRequestMessage> msg;
    std::shared_ptr<RadioContext> ctx = getContext(serial);

    if (s_vendorFunctions->version < 15) {
        if (deviceStateType ==  DeviceStateType::LOW_DATA_EXPECTED) {
            RLOGD("sendDeviceState: calling screen state %d", BOOL_TO_INT(!state));
            msg = std::make_shared<RilRequestScreenStateMessage>(ctx, state);
        } else {
            RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial,
                    RadioError::REQUEST_NOT_SUPPORTED };
            RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, sendDeviceStateResponse, respInfo);
            return Void();
        }
    } else {
        msg = std::make_shared<RilRequestSendDeviceStateMessage>(ctx,
                (RIL_DeviceStateType)deviceStateType, state);
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
            RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, sendDeviceStateResponse, respInfo);
        });
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, sendDeviceStateResponse, respInfo);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> RadioImpl::setIndicationFilter(int32_t serial, int32_t indicationFilter) {
    QCRIL_LOG_FUNC_ENTRY("serial=%d, indicationFilter=%d", serial, indicationFilter);
    bool sendFailure = false;
    RadioError radioError = RadioError::NO_MEMORY;
    do {
      if (s_vendorFunctions->version < 15) {
        sendFailure = true;
        radioError = RadioError::REQUEST_NOT_SUPPORTED;
        break;
      }
      std::shared_ptr<RadioContext> ctx = getContext(serial);
      auto msg =
          std::make_shared<RilRequestSetUnsolRespFilterMessage>(ctx, indicationFilter);
      if (msg == nullptr) {
        break;
      }
      GenericCallback<QcRilRequestMessageCallbackPayload> cb(
          [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                         std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
            RadioResponseInfo respInfo = {};
            if (resp != nullptr) {
              populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, resp->errorCode);
            } else {
              populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, RIL_E_NO_MEMORY);
            }
            QCRIL_LOG_DEBUG("setIndicationFilterResponse: serial=%d, error=%d", serial, respInfo.error);
            RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, setIndicationFilterResponse,
                                     respInfo);
          });
      msg->setCallback(&cb);
      msg->dispatch();
    } while (FALSE);

    if (sendFailure) {
        RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, setIndicationFilterResponse, respInfo);
    }

    QCRIL_LOG_FUNC_RETURN();

    return Void();
}

Return<void> RadioImpl::setSimCardPower(int32_t serial, bool powerUp) {
#if VDBG
    RLOGD("setSimCardPower: serial %d", serial);
#endif
    RIL_UIM_CardPowerState state = RIL_UIM_CARD_POWER_DOWN;

    if (powerUp) {
        state = RIL_UIM_CARD_POWER_UP;
    }

    auto msg = std::make_shared<UimCardPowerReqMsg>(qmi_ril_get_process_instance_id(), state);
    if (msg) {
        GenericCallback<RIL_UIM_Errno> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                            std::shared_ptr<RIL_UIM_Errno> responseDataPtr) -> void {
                RadioResponseInfo responseInfo{RadioResponseType::SOLICITED, serial, RadioError::INTERNAL_ERR};

                if (solicitedMsg && responseDataPtr && responseDataPtr.get() &&
                    status == Message::Callback::Status::SUCCESS) {
                    responseInfo.error = (RadioError)*(responseDataPtr.get());
                }
                RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, setSimCardPowerResponse,
                        responseInfo);
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo rsp{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        IccIoResult  result = {};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, setSimCardPowerResponse, rsp);
    }
    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> RadioImpl::setSimCardPower_1_1(int32_t serial, const V1_1::CardPowerState state) {
#if VDBG
    RLOGD("setSimCardPower_1_1: serial %d state %d", serial, state);
#endif
    auto msg = std::make_shared<UimCardPowerReqMsg>(qmi_ril_get_process_instance_id(), (RIL_UIM_CardPowerState)state);
    if (msg) {
        GenericCallback<RIL_UIM_Errno> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                            std::shared_ptr<RIL_UIM_Errno> responseDataPtr) -> void {
                RadioResponseInfo responseInfo{RadioResponseType::SOLICITED, serial, RadioError::INTERNAL_ERR};

                if (solicitedMsg && responseDataPtr && responseDataPtr.get() &&
                    status == Message::Callback::Status::SUCCESS) {
                    responseInfo.error = (RadioError)*(responseDataPtr.get());
                }
                RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponseV1_1, setSimCardPowerResponse_1_1,
                        responseInfo);
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo rsp{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        IccIoResult  result = {};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponseV1_1, setSimCardPowerResponse_1_1, rsp);
    }
    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> RadioImpl::setCarrierInfoForImsiEncryption(int32_t serial,
        const V1_1::ImsiEncryptionInfo& data) {
#if VDBG
    RLOGD("setCarrierInfoForImsiEncryption: serial %d", serial);
#endif
    RequestInfo *pRI = android::addRequestToList(
            serial, mSlotId, RIL_REQUEST_SET_CARRIER_INFO_IMSI_ENCRYPTION);
    if (pRI == NULL) {
        return Void();
    }

    RIL_CarrierInfoForImsiEncryption imsiEncryption = {};

    if (!copyHidlStringToRil(&imsiEncryption.mnc, data.mnc, pRI)) {
        return Void();
    }
    if (!copyHidlStringToRil(&imsiEncryption.mcc, data.mcc, pRI)) {
        memsetAndFreeStrings(1, imsiEncryption.mnc);
        return Void();
    }
    if (!copyHidlStringToRil(&imsiEncryption.keyIdentifier, data.keyIdentifier, pRI)) {
        memsetAndFreeStrings(2, imsiEncryption.mnc, imsiEncryption.mcc);
        return Void();
    }
    imsiEncryption.carrierKeyLength = data.carrierKey.size();
    imsiEncryption.carrierKey = new uint8_t[imsiEncryption.carrierKeyLength];
    if(imsiEncryption.carrierKey != nullptr) {
        memcpy(imsiEncryption.carrierKey, data.carrierKey.data(),
                                            imsiEncryption.carrierKeyLength);
        imsiEncryption.expirationTime = data.expirationTime;
        CALL_ONREQUEST(pRI->pCI->requestNumber, &imsiEncryption,
                sizeof(RIL_CarrierInfoForImsiEncryption), pRI, mSlotId);
        delete(imsiEncryption.carrierKey);
    }
    return Void();
}

Return<void> RadioImpl::startKeepalive(int32_t serial, const V1_1::KeepaliveRequest& keepalive) {
#if VDBG
    RLOGD("%s(): %d", __FUNCTION__, serial);
#endif
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId, RIL_REQUEST_START_KEEPALIVE);
    if (pRI == NULL) {
        return Void();
    }

    RIL_KeepaliveRequest kaReq = {};

    kaReq.type = static_cast<RIL_KeepaliveType>(keepalive.type);
    switch(kaReq.type) {
        case NATT_IPV4:
            if (keepalive.sourceAddress.size() != 4 ||
                    keepalive.destinationAddress.size() != 4) {
                RLOGE("Invalid address for keepalive!");
                sendErrorResponse(pRI, RIL_E_INVALID_ARGUMENTS);
                return Void();
            }
            break;
        case NATT_IPV6:
            if (keepalive.sourceAddress.size() != 16 ||
                    keepalive.destinationAddress.size() != 16) {
                RLOGE("Invalid address for keepalive!");
                sendErrorResponse(pRI, RIL_E_INVALID_ARGUMENTS);
                return Void();
            }
            break;
        default:
            RLOGE("Unknown packet keepalive type!");
            sendErrorResponse(pRI, RIL_E_INVALID_ARGUMENTS);
            return Void();
    }

    ::memcpy(kaReq.sourceAddress, keepalive.sourceAddress.data(), keepalive.sourceAddress.size());
    kaReq.sourcePort = keepalive.sourcePort;

    ::memcpy(kaReq.destinationAddress,
            keepalive.destinationAddress.data(), keepalive.destinationAddress.size());
    kaReq.destinationPort = keepalive.destinationPort;

    kaReq.maxKeepaliveIntervalMillis = keepalive.maxKeepaliveIntervalMillis;
    kaReq.cid = keepalive.cid; // This is the context ID of the data call

    CALL_ONREQUEST(pRI->pCI->requestNumber, &kaReq, sizeof(RIL_KeepaliveRequest), pRI, mSlotId);
    return Void();
}

Return<void> RadioImpl::stopKeepalive(int32_t serial, int32_t sessionHandle) {
#if VDBG
    RLOGD("%s(): %d", __FUNCTION__, serial);
#endif
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId, RIL_REQUEST_STOP_KEEPALIVE);
    if (pRI == NULL) {
        return Void();
    }

    CALL_ONREQUEST(pRI->pCI->requestNumber, &sessionHandle, sizeof(uint32_t), pRI, mSlotId);
    return Void();
}


Return<void> RadioImpl::responseAcknowledgement() {
    android::releaseWakeLock();
    return Void();
}

Return<void> OemHookImpl::setResponseFunctions(
        const ::android::sp<IOemHookResponse>& oemHookResponseParam,
        const ::android::sp<IOemHookIndication>& oemHookIndicationParam) {
#if VDBG
    RLOGD("OemHookImpl::setResponseFunctions");
#endif

    qtimutex::QtiSharedMutex *radioServiceRwlockPtr = radio::getRadioServiceRwlock(mSlotId);
    radioServiceRwlockPtr->lock();

    mOemHookResponse = oemHookResponseParam;
    mOemHookIndication = oemHookIndicationParam;
    mCounterOemHook[mSlotId]++;

    radioServiceRwlockPtr->unlock();

    return Void();
}

Return<void> OemHookImpl::sendRequestRaw(int32_t serial, const hidl_vec<uint8_t>& data) {
#if VDBG
    RLOGD("OemHookImpl::sendRequestRaw: serial %d", serial);
#endif
    dispatchRaw(serial, mSlotId, RIL_REQUEST_OEM_HOOK_RAW, data);
    return Void();
}

Return<void> OemHookImpl::sendRequestStrings(int32_t serial,
        const hidl_vec<hidl_string>& data) {
#if VDBG
    RLOGD("OemHookImpl::sendRequestStrings: serial %d", serial);
#endif
    dispatchStrings(serial, mSlotId, RIL_REQUEST_OEM_HOOK_STRINGS, data);
    return Void();
}

/***************************************************************************************************
 * RESPONSE FUNCTIONS
 * Functions above are used for requests going from framework to vendor code. The ones below are
 * responses for those requests coming back from the vendor code.
 **************************************************************************************************/

void radio::acknowledgeRequest(int slotId, int serial) {
    if (radioService[slotId]->mRadioResponse != NULL) {
        Return<void> retStatus = radioService[slotId]->mRadioResponse->acknowledgeRequest(serial);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("acknowledgeRequest: radioService[%d]->mRadioResponse == NULL", slotId);
    }
}

void populateResponseInfo(RadioResponseInfo& responseInfo, int serial, int responseType,
        V1_0::RadioError e) {
    responseInfo.serial = serial;
    switch (responseType) {
        case RESPONSE_SOLICITED:
            responseInfo.type = RadioResponseType::SOLICITED;
            break;
        case RESPONSE_SOLICITED_ACK_EXP:
            responseInfo.type = RadioResponseType::SOLICITED_ACK_EXP;
            break;
    }
    responseInfo.error = e;
}

void populateResponseInfo(RadioResponseInfo& responseInfo, int serial, int responseType,
                         RIL_Errno e) {
    responseInfo.serial = serial;
    switch (responseType) {
        case RESPONSE_SOLICITED:
            responseInfo.type = RadioResponseType::SOLICITED;
            break;
        case RESPONSE_SOLICITED_ACK_EXP:
            responseInfo.type = RadioResponseType::SOLICITED_ACK_EXP;
            break;
    }
    responseInfo.error = (RadioError) e;
}

int responseIntOrEmpty(RadioResponseInfo& responseInfo, int serial, int responseType, RIL_Errno e,
               void *response, size_t responseLen) {
    populateResponseInfo(responseInfo, serial, responseType, e);
    int ret = -1;

    if (response == NULL && responseLen == 0) {
        // Earlier RILs did not send a response for some cases although the interface
        // expected an integer as response. Do not return error if response is empty. Instead
        // Return -1 in those cases to maintain backward compatibility.
    } else if (response == NULL || responseLen != sizeof(int)) {
        RLOGE("responseIntOrEmpty: Invalid response");
        if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
    } else {
        int *p_int = (int *) response;
        ret = p_int[0];
    }
    return ret;
}

int responseInt(RadioResponseInfo& responseInfo, int serial, int responseType, RIL_Errno e,
               void *response, size_t responseLen) {
    populateResponseInfo(responseInfo, serial, responseType, e);
    int ret = -1;

    if (response == NULL || responseLen != sizeof(int)) {
        RLOGE("responseInt: Invalid response");
        if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
    } else {
        int *p_int = (int *) response;
        ret = p_int[0];
    }
    return ret;
}

void RadioImpl::convertGetIccCardStatusResponse(RadioResponseInfo &responseInfo,
        V1_0::CardStatus &cardStatus, int responseType,
        int serial, std::shared_ptr<RIL_UIM_CardStatus> p_cur) {
    if (p_cur == NULL)
    {
      populateResponseInfo(responseInfo, serial, responseType, RIL_E_INVALID_RESPONSE);
      return;
    }
    else
    {
      populateResponseInfo(responseInfo, serial, responseType, (RIL_Errno)p_cur->err);
    }

    if (p_cur->gsm_umts_subscription_app_index >= p_cur->num_applications
            || p_cur->cdma_subscription_app_index >= p_cur->num_applications
            || p_cur->ims_subscription_app_index >= p_cur->num_applications) {
        RLOGE("getIccCardStatusResponse: Invalid response");
        if (p_cur->err == RIL_UIM_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        return;
    } else {
        cardStatus.cardState = (CardState) p_cur->card_state;
        cardStatus.universalPinState = (PinState) p_cur->universal_pin_state;
        cardStatus.gsmUmtsSubscriptionAppIndex = p_cur->gsm_umts_subscription_app_index;
        cardStatus.cdmaSubscriptionAppIndex = p_cur->cdma_subscription_app_index;
        cardStatus.imsSubscriptionAppIndex = p_cur->ims_subscription_app_index;

        RIL_UIM_AppStatus *rilAppStatus = p_cur->applications;
        cardStatus.applications.resize(p_cur->num_applications);
        AppStatus *appStatus = cardStatus.applications.data();
#if VDBG
        RLOGD("getIccCardStatusResponse: num_applications %d", p_cur->num_applications);
#endif
        for (int i = 0; i < p_cur->num_applications; i++) {
            appStatus[i].appType = (AppType) rilAppStatus[i].app_type;
            appStatus[i].appState = (AppState) rilAppStatus[i].app_state;
            appStatus[i].persoSubstate = (PersoSubstate) rilAppStatus[i].perso_substate;
            appStatus[i].aidPtr = rilAppStatus[i].aid_ptr;
            appStatus[i].appLabelPtr = rilAppStatus[i].app_label_ptr;
            appStatus[i].pin1Replaced = rilAppStatus[i].pin1_replaced;
            appStatus[i].pin1 = (PinState) rilAppStatus[i].pin1;
            appStatus[i].pin2 = (PinState) rilAppStatus[i].pin2;
        }

    }
    return;
}

int RadioImpl::convertToHidl(Call &hidlCall, RIL_Call &rilCall) {
    /* each call info */
    hidlCall.state = (CallState) rilCall.state;
    hidlCall.index = rilCall.index;
    hidlCall.toa = rilCall.toa;
    hidlCall.isMpty = rilCall.isMpty;
    hidlCall.isMT = rilCall.isMT;
    hidlCall.als = rilCall.als;
    hidlCall.isVoice = rilCall.isVoice;
    hidlCall.isVoicePrivacy = rilCall.isVoicePrivacy;
    hidlCall.number = convertCharPtrToHidlString(rilCall.number);
    hidlCall.numberPresentation = (CallPresentation) rilCall.numberPresentation;
    hidlCall.name = convertCharPtrToHidlString(rilCall.name);
    hidlCall.namePresentation = (CallPresentation) rilCall.namePresentation;
    if (rilCall.uusInfo != NULL && rilCall.uusInfo->uusData != NULL) {
        RIL_UUS_Info *uusInfo = rilCall.uusInfo;
        hidlCall.uusInfo.resize(1);
        hidlCall.uusInfo[0].uusType = (UusType) uusInfo->uusType;
        hidlCall.uusInfo[0].uusDcs = (UusDcs) uusInfo->uusDcs;
        // convert uusInfo->uusData to a null-terminated string
        char *nullTermStr = strndup(uusInfo->uusData, uusInfo->uusLength);
        hidlCall.uusInfo[0].uusData = nullTermStr;
        free(nullTermStr);
    }
    return 0;
}
int RadioImpl::convertGetCurrentCallsResponse(RadioResponseInfo &responseInfo,
                                  hidl_vec<Call> &calls,
                                  int responseType, int serial, RIL_Errno e,
                                  void *response, size_t responseLen) {
#if VDBG
    RLOGD("getCurrentCallsResponse: serial %d", serial);
#endif

    populateResponseInfo(responseInfo, serial, responseType, e);

    if ((response == NULL && responseLen != 0)
            || (responseLen % sizeof(RIL_Call *)) != 0) {
        RLOGE("getCurrentCallsResponse: Invalid response");
        if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
    } else {
        int num = responseLen / sizeof(RIL_Call *);
        calls.resize(num);

        for (int i = 0 ; i < num ; i++) {
            RIL_Call *p_cur = ((RIL_Call **) response)[i];
            if (p_cur) {
                convertToHidl(calls[i], *p_cur);
            }
        }
    }
    return 0;
}
int RadioImpl::sendGetCurrentCallsResponse(int slotId,
                                  int responseType, int serial, RIL_Errno e,
                                  void *response, size_t responseLen) {

    auto rsp = mRadioResponse;
    if (rsp) {
        RadioResponseInfo responseInfo = {};
        hidl_vec<Call> calls;
        if (convertGetCurrentCallsResponse(responseInfo,
                    calls,
                    responseType,
                    serial,
                    e,
                    response,
                    responseLen) == 0 ) {
            Return<void> retStatus = rsp->
                    getCurrentCallsResponse(responseInfo, calls);
            checkReturnStatus(retStatus);
        }
    } else {
        RLOGE("getCurrentCallsResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::getCurrentCallsResponse(int slotId,
                                  int responseType, int serial, RIL_Errno e,
                                  void *response, size_t responseLen) {
    auto srvc = radioService[slotId];
    if (srvc) {
        return srvc->sendGetCurrentCallsResponse(slotId,
                responseType,
                serial,
                e,
                response,
                responseLen);
    } else {
        RLOGE("radioService[slotId = %d] is null!", slotId);
    }
    return 0;
}

int radio::dialResponse(int slotId,
                       int responseType, int serial, RIL_Errno e, void *response,
                       size_t responseLen) {
#if VDBG
    RLOGD("dialResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = radioService[slotId]->mRadioResponse->dialResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("dialResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::hangupConnectionResponse(int slotId,
                                   int responseType, int serial, RIL_Errno e,
                                   void *response, size_t responseLen) {
#if VDBG
    RLOGD("hangupConnectionResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = radioService[slotId]->mRadioResponse->hangupConnectionResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("hangupConnectionResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::hangupWaitingOrBackgroundResponse(int slotId,
                                            int responseType, int serial, RIL_Errno e,
                                            void *response, size_t responseLen) {
#if VDBG
    RLOGD("hangupWaitingOrBackgroundResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus =
                radioService[slotId]->mRadioResponse->hangupWaitingOrBackgroundResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("hangupWaitingOrBackgroundResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::hangupForegroundResumeBackgroundResponse(int slotId, int responseType, int serial,
                                                    RIL_Errno e, void *response,
                                                    size_t responseLen) {
#if VDBG
    RLOGD("hangupWaitingOrBackgroundResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus =
                radioService[slotId]->mRadioResponse->hangupWaitingOrBackgroundResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("hangupWaitingOrBackgroundResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::switchWaitingOrHoldingAndActiveResponse(int slotId, int responseType, int serial,
                                                   RIL_Errno e, void *response,
                                                   size_t responseLen) {
#if VDBG
    RLOGD("switchWaitingOrHoldingAndActiveResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus =
                radioService[slotId]->mRadioResponse->switchWaitingOrHoldingAndActiveResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("switchWaitingOrHoldingAndActiveResponse: radioService[%d]->mRadioResponse "
                "== NULL", slotId);
    }

    return 0;
}

int radio::conferenceResponse(int slotId, int responseType,
                             int serial, RIL_Errno e, void *response, size_t responseLen) {
#if VDBG
    RLOGD("conferenceResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = radioService[slotId]->mRadioResponse->conferenceResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("conferenceResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::rejectCallResponse(int slotId, int responseType,
                             int serial, RIL_Errno e, void *response, size_t responseLen) {
#if VDBG
    RLOGD("rejectCallResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = radioService[slotId]->mRadioResponse->rejectCallResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("rejectCallResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::getLastCallFailCauseResponse(int slotId,
                                       int responseType, int serial, RIL_Errno e, void *response,
                                       size_t responseLen) {
#if VDBG
    RLOGD("getLastCallFailCauseResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);

        LastCallFailCauseInfo info = {};
        info.vendorCause = hidl_string();
        if (response == NULL) {
            RLOGE("getCurrentCallsResponse Invalid response: NULL");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else if (responseLen == sizeof(int)) {
            int *pInt = (int *) response;
            info.causeCode = (LastCallFailCause) pInt[0];
        } else if (responseLen == sizeof(RIL_LastCallFailCauseInfo))  {
            RIL_LastCallFailCauseInfo *pFailCauseInfo = (RIL_LastCallFailCauseInfo *) response;
            info.causeCode = (LastCallFailCause) pFailCauseInfo->cause_code;
            info.vendorCause = convertCharPtrToHidlString(pFailCauseInfo->vendor_cause);
        } else {
            RLOGE("getCurrentCallsResponse Invalid response: NULL");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        }

        Return<void> retStatus = radioService[slotId]->mRadioResponse->getLastCallFailCauseResponse(
                responseInfo, info);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("getLastCallFailCauseResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::getSignalStrengthResponse(int slotId,
                                     int responseType, int serial, RIL_Errno e,
                                     void *response, size_t responseLen) {
    auto srvc = radioService[slotId];
    if (srvc) {
        return srvc->sendGetSignalStrengthResponse(slotId,
                responseType,
                serial,
                e,
                response,
                responseLen);
    } else {
        RLOGE("getSignalStrengthResponse: radioService[%d] == NULL",
                slotId);
        return 0;
    }
}

int RadioImpl::sendGetSignalStrengthResponse(int slotId,
                                     int responseType, int serial, RIL_Errno e,
                                     void *response, size_t responseLen) {
    auto rsp = mRadioResponse;
#if VDBG
    RLOGD("getSignalStrengthResponse: serial %d", serial);
#endif
    if (rsp) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        SignalStrength signalStrength = {};
        if (response == NULL || responseLen != sizeof(RIL_SignalStrength_v10)) {
            RLOGE("getSignalStrengthResponse: Invalid response");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            convertRilSignalStrengthToHal(response, responseLen, signalStrength);
        }

        Return<void> retStatus = rsp->getSignalStrengthResponse(
                responseInfo, signalStrength);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("getSignalStrengthResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int RadioImpl::sendNewSms(std::shared_ptr<RilUnsolIncoming3GppSMSMessage> msg)
{
    auto ind = mRadioIndication;
    QCRIL_LOG_INFO("ind null: %s", ind ? "false" : "true" );
    if (ind) {
        hidl_vec<uint8_t> pdu;
        pdu.setToExternal(msg->getPayload().data(), msg->getPayload().size());
        Return<void> ret = ind->newSms(RadioIndicationType::UNSOLICITED, pdu);
        checkReturnStatus(ret);
    }

    QCRIL_LOG_FUNC_RETURN();
    return 0;
}

int RadioImpl::sendNewCdmaSms(std::shared_ptr<RilUnsolIncoming3Gpp2SMSMessage> shared_msg)
{
    auto ind = mRadioIndication;
    QCRIL_LOG_INFO("ind null: %s", ind ? "false" : "true" );
    if (ind) {
        CdmaSmsMessage msg = {};
        auto& rilMsg = shared_msg->getCdmaSms();

        msg.teleserviceId = rilMsg.uTeleserviceID;
        msg.isServicePresent = rilMsg.bIsServicePresent;
        msg.serviceCategory = rilMsg.uServicecategory;
        msg.address.digitMode =
                (V1_0::CdmaSmsDigitMode) rilMsg.sAddress.digit_mode;
        msg.address.numberMode =
                (V1_0::CdmaSmsNumberMode) rilMsg.sAddress.number_mode;
        msg.address.numberType =
                (V1_0::CdmaSmsNumberType) rilMsg.sAddress.number_type;
        msg.address.numberPlan =
                (V1_0::CdmaSmsNumberPlan) rilMsg.sAddress.number_plan;

        int digitLimit = MIN((rilMsg.sAddress.number_of_digits), RIL_CDMA_SMS_ADDRESS_MAX);
        msg.address.digits.setToExternal(rilMsg.sAddress.digits, digitLimit);

        msg.subAddress.subaddressType = (V1_0::CdmaSmsSubaddressType)
                rilMsg.sSubAddress.subaddressType;
        msg.subAddress.odd = rilMsg.sSubAddress.odd;

        digitLimit= MIN((rilMsg.sSubAddress.number_of_digits), RIL_CDMA_SMS_SUBADDRESS_MAX);
        msg.subAddress.digits.setToExternal(rilMsg.sSubAddress.digits, digitLimit);

        digitLimit = MIN((rilMsg.uBearerDataLen), RIL_CDMA_SMS_BEARER_DATA_MAX);
        msg.bearerData.setToExternal(rilMsg.aBearerData, digitLimit);

        Return<void> ret = ind->cdmaNewSms(RadioIndicationType::UNSOLICITED, msg);

        checkReturnStatus(ret);
    }

    QCRIL_LOG_FUNC_RETURN();
    return 0;
}

int RadioImpl::sendNewSmsOnSim(std::shared_ptr<RilUnsolNewSmsOnSimMessage> msg)
{
    auto ind = mRadioIndication;
    QCRIL_LOG_INFO("ind null: %s", ind ? "false" : "true" );
    if (ind) {
        Return<void> ret = ind->newSmsOnSim(RadioIndicationType::UNSOLICITED,
                msg->getRecordNumber());
        checkReturnStatus(ret);
    }
    QCRIL_LOG_FUNC_RETURN();
    return 0;
}

int RadioImpl::sendNewSmsStatusReport(std::shared_ptr<RilUnsolNewSmsStatusReportMessage> msg)
{
    auto ind = mRadioIndication;
    QCRIL_LOG_INFO("ind null: %s", ind ? "false" : "true" );
    if (ind) {
        hidl_vec<uint8_t> pdu;
        pdu.setToExternal(msg->getPayload().data(), msg->getPayload().size());
        Return<void> ret = ind->newSmsStatusReport(
                RadioIndicationType::UNSOLICITED, pdu);
        checkReturnStatus(ret);
    }
    QCRIL_LOG_FUNC_RETURN();
    return 0;
}

int RadioImpl::sendNewBroadcastSms(std::shared_ptr<RilUnsolNewBroadcastSmsMessage> msg)
{
    auto ind = mRadioIndication;
    QCRIL_LOG_INFO("ind null: %s", ind ? "false" : "true" );
    if (ind) {
        hidl_vec<uint8_t> pdu;
        pdu.setToExternal(msg->getPayload().data(), msg->getPayload().size());
        Return<void> ret = ind->newBroadcastSms(
                RadioIndicationType::UNSOLICITED, pdu);
        checkReturnStatus(ret);
    }
    QCRIL_LOG_FUNC_RETURN();
    return 0;
}

int RadioImpl::sendStkCCAlphaNotify(std::shared_ptr<RilUnsolStkCCAlphaNotifyMessage> msg)
{
    auto ind = mRadioIndication;
    QCRIL_LOG_INFO("ind null: %s", ind ? "false" : "true" );
    if (ind) {
        hidl_string payload(msg->getPayload());
        Return<void> ret = ind->stkCallControlAlphaNotify(
                RadioIndicationType::UNSOLICITED, payload);
        checkReturnStatus(ret);
    }
    QCRIL_LOG_FUNC_RETURN();
    return 0;
}

int RadioImpl::sendCdmaRuimSmsStorageFull(std::shared_ptr<RilUnsolCdmaRuimSmsStorageFullMessage> msg)
{
    auto ind = mRadioIndication;
    QCRIL_LOG_INFO("ind null: %s", ind ? "false" : "true" );
    if (ind) {
        Return<void> ret = ind->cdmaRuimSmsStorageFull(RadioIndicationType::UNSOLICITED);
        checkReturnStatus(ret);
    }
    QCRIL_LOG_FUNC_RETURN();
    return 0;
}

int RadioImpl::sendSimSmsStorageFull(std::shared_ptr<RilUnsolSimSmsStorageFullMessage> msg)
{
    auto ind = mRadioIndication;
    QCRIL_LOG_INFO("ind null: %s", ind ? "false" : "true" );
    if (ind) {
        Return<void> ret = ind->simSmsStorageFull(RadioIndicationType::UNSOLICITED);
        checkReturnStatus(ret);
    }
    QCRIL_LOG_FUNC_RETURN();
    return 0;
}

int RadioImpl::sendImsNetworkStateChanged(std::shared_ptr<RilUnsolImsNetworkStateChangedMessage> msg)
{
    auto ind = mRadioIndication;
    QCRIL_LOG_INFO("ind null: %s", ind ? "false" : "true" );
    if (ind) {
        Return<void> ret = ind->imsNetworkStateChanged(RadioIndicationType::UNSOLICITED);
        checkReturnStatus(ret);
    }
    QCRIL_LOG_FUNC_RETURN();
    return 0;
}

void convertRilCdmaSignalInfoRecord(std::shared_ptr<RIL_CDMA_SignalInfoRecord> signalInfoRecord,
        CdmaSignalInfoRecord& record) {
    record.isPresent = signalInfoRecord->isPresent;
    record.signalType = signalInfoRecord->signalType;
    record.alertPitch = signalInfoRecord->alertPitch;
    record.signal = signalInfoRecord->signal;
}
int RadioImpl::sendCallRing(std::shared_ptr<QcRilUnsolCallRingingMessage> msg)
{
  QCRIL_LOG_FUNC_ENTRY();
  auto ind = mRadioIndication;
  if (ind && msg) {
    bool isGsm;
    CdmaSignalInfoRecord record = {};
    if (!msg->hasCdmaSignalInfoRecord()) {
      isGsm = true;
    } else {
      isGsm = false;
      if (msg->getCdmaSignalInfoRecord() == nullptr) {
        RLOGE("callRingInd: invalid response");
        return 0;
      }
      convertRilCdmaSignalInfoRecord(msg->getCdmaSignalInfoRecord(), record);
    }
#if VDBG
    RLOGD("callRingInd: isGsm %d", isGsm);
#endif
    Return<void> ret = ind->callRing(RadioIndicationType::UNSOLICITED, isGsm, record);
    checkReturnStatus(ret);
  } else {
    QCRIL_LOG_INFO("ind null: %s, msg null: %s", ind ? "false" : "true", msg ? "false" : "true");
  }
  QCRIL_LOG_FUNC_RETURN();
  return 0;
}

static UssdModeType convertUssdModeType(qcril::interfaces::UssdModeType in) {
  switch(in) {
    case qcril::interfaces::UssdModeType::NOTIFY:
      return UssdModeType::NOTIFY;
    case qcril::interfaces::UssdModeType::REQUEST:
      return UssdModeType::REQUEST;
    case qcril::interfaces::UssdModeType::NW_RELEASE:
      return UssdModeType::NW_RELEASE;
    case qcril::interfaces::UssdModeType::LOCAL_CLIENT:
      return UssdModeType::LOCAL_CLIENT;
    case qcril::interfaces::UssdModeType::NOT_SUPPORTED:
      return UssdModeType::NOT_SUPPORTED;
    case qcril::interfaces::UssdModeType::NW_TIMEOUT:
      return UssdModeType::NW_TIMEOUT;
    default:
      return UssdModeType::NOTIFY;
  }
  return UssdModeType::NOTIFY;
}

int RadioImpl::sendOnUssd(std::shared_ptr<QcRilUnsolOnUssdMessage> msg)
{
  QCRIL_LOG_FUNC_ENTRY();
  auto ind = mRadioIndication;
  if (ind && msg && !msg->isImsUpdate()) {
    if (msg->hasMode()) {
      UssdModeType modeType = convertUssdModeType(msg->getMode());
      hidl_string ussdMsg = "";
      if (msg->hasMessage() && !msg->getMessage().empty()) {
        ussdMsg = msg->getMessage().c_str();
      }
#if VDBG
      RLOGD("onUssdInd: mode %s", mode);
#endif
      Return<void> ret = ind->onUssd(RadioIndicationType::UNSOLICITED, modeType, ussdMsg);
      checkReturnStatus(ret);
    }
  } else {
    QCRIL_LOG_INFO("ind null: %s, msg null: %s, isImsUpdate: %s", ind ? "false" : "true",
                      msg ? "false" : "true", msg->isImsUpdate() ? "true" : "false");
  }
  QCRIL_LOG_FUNC_RETURN();
  return 0;
}

int RadioImpl::sendCdmaInfoRec(std::shared_ptr<QcRilUnsolCdmaInfoRecordMessage> msg)
{
    QCRIL_LOG_FUNC_ENTRY();
    auto ind = mRadioIndication;
    if (ind && msg) {
      if (msg->hasCdmaInfoRecords() && msg->getCdmaInfoRecords() != nullptr) {
        CdmaInformationRecords records = {};
        std::shared_ptr<RIL_CDMA_InformationRecords> recordsRil = msg->getCdmaInfoRecords();

        char* string8 = NULL;
        int num = MIN(recordsRil->numberOfInfoRecs, RIL_CDMA_MAX_NUMBER_OF_INFO_RECS);
        if (recordsRil->numberOfInfoRecs > RIL_CDMA_MAX_NUMBER_OF_INFO_RECS) {
            RLOGE("cdmaInfoRecInd: received %d recs which is more than %d, dropping "
                    "additional ones", recordsRil->numberOfInfoRecs,
                    RIL_CDMA_MAX_NUMBER_OF_INFO_RECS);
        }
        records.infoRec.resize(num);
        for (int i = 0 ; i < num ; i++) {
            CdmaInformationRecord *record = &records.infoRec[i];
            RIL_CDMA_InformationRecord *infoRec = &recordsRil->infoRec[i];
            record->name = (CdmaInfoRecName) infoRec->name;
            // All vectors should be size 0 except one which will be size 1. Set everything to
            // size 0 initially.
            record->display.resize(0);
            record->number.resize(0);
            record->signal.resize(0);
            record->redir.resize(0);
            record->lineCtrl.resize(0);
            record->clir.resize(0);
            record->audioCtrl.resize(0);
            switch (infoRec->name) {
                case RIL_CDMA_DISPLAY_INFO_REC:
                case RIL_CDMA_EXTENDED_DISPLAY_INFO_REC: {
                    if (infoRec->rec.display.alpha_len > CDMA_ALPHA_INFO_BUFFER_LENGTH) {
                        RLOGE("cdmaInfoRecInd: invalid display info response length %d "
                                "expected not more than %d", (int) infoRec->rec.display.alpha_len,
                                CDMA_ALPHA_INFO_BUFFER_LENGTH);
                        return 0;
                    }
                    string8 = (char*) malloc((infoRec->rec.display.alpha_len + 1) * sizeof(char));
                    if (string8 == NULL) {
                        RLOGE("cdmaInfoRecInd: Memory allocation failed for "
                                "responseCdmaInformationRecords");
                        return 0;
                    }
                    memcpy(string8, infoRec->rec.display.alpha_buf, infoRec->rec.display.alpha_len);
                    string8[(int)infoRec->rec.display.alpha_len] = '\0';

                    record->display.resize(1);
                    record->display[0].alphaBuf = string8;
                    free(string8);
                    string8 = NULL;
                    break;
                }

                case RIL_CDMA_CALLED_PARTY_NUMBER_INFO_REC:
                case RIL_CDMA_CALLING_PARTY_NUMBER_INFO_REC:
                case RIL_CDMA_CONNECTED_NUMBER_INFO_REC: {
                    if (infoRec->rec.number.len > CDMA_NUMBER_INFO_BUFFER_LENGTH) {
                        RLOGE("cdmaInfoRecInd: invalid display info response length %d "
                                "expected not more than %d", (int) infoRec->rec.number.len,
                                CDMA_NUMBER_INFO_BUFFER_LENGTH);
                        return 0;
                    }
                    string8 = (char*) malloc((infoRec->rec.number.len + 1) * sizeof(char));
                    if (string8 == NULL) {
                        RLOGE("cdmaInfoRecInd: Memory allocation failed for "
                                "responseCdmaInformationRecords");
                        return 0;
                    }
                    memcpy(string8, infoRec->rec.number.buf, infoRec->rec.number.len);
                    string8[(int)infoRec->rec.number.len] = '\0';

                    record->number.resize(1);
                    record->number[0].number = string8;
                    free(string8);
                    string8 = NULL;
                    record->number[0].numberType = infoRec->rec.number.number_type;
                    record->number[0].numberPlan = infoRec->rec.number.number_plan;
                    record->number[0].pi = infoRec->rec.number.pi;
                    record->number[0].si = infoRec->rec.number.si;
                    break;
                }

                case RIL_CDMA_SIGNAL_INFO_REC: {
                    record->signal.resize(1);
                    record->signal[0].isPresent = infoRec->rec.signal.isPresent;
                    record->signal[0].signalType = infoRec->rec.signal.signalType;
                    record->signal[0].alertPitch = infoRec->rec.signal.alertPitch;
                    record->signal[0].signal = infoRec->rec.signal.signal;
                    break;
                }

                case RIL_CDMA_REDIRECTING_NUMBER_INFO_REC: {
                    if (infoRec->rec.redir.redirectingNumber.len >
                                                  CDMA_NUMBER_INFO_BUFFER_LENGTH) {
                        RLOGE("cdmaInfoRecInd: invalid display info response length %d "
                                "expected not more than %d\n",
                                (int)infoRec->rec.redir.redirectingNumber.len,
                                CDMA_NUMBER_INFO_BUFFER_LENGTH);
                        return 0;
                    }
                    string8 = (char*) malloc((infoRec->rec.redir.redirectingNumber.len + 1) *
                            sizeof(char));
                    if (string8 == NULL) {
                        RLOGE("cdmaInfoRecInd: Memory allocation failed for "
                                "responseCdmaInformationRecords");
                        return 0;
                    }
                    memcpy(string8, infoRec->rec.redir.redirectingNumber.buf,
                            infoRec->rec.redir.redirectingNumber.len);
                    string8[(int)infoRec->rec.redir.redirectingNumber.len] = '\0';

                    record->redir.resize(1);
                    record->redir[0].redirectingNumber.number = string8;
                    free(string8);
                    string8 = NULL;
                    record->redir[0].redirectingNumber.numberType =
                            infoRec->rec.redir.redirectingNumber.number_type;
                    record->redir[0].redirectingNumber.numberPlan =
                            infoRec->rec.redir.redirectingNumber.number_plan;
                    record->redir[0].redirectingNumber.pi = infoRec->rec.redir.redirectingNumber.pi;
                    record->redir[0].redirectingNumber.si = infoRec->rec.redir.redirectingNumber.si;
                    record->redir[0].redirectingReason =
                            (CdmaRedirectingReason) infoRec->rec.redir.redirectingReason;
                    break;
                }

                case RIL_CDMA_LINE_CONTROL_INFO_REC: {
                    record->lineCtrl.resize(1);
                    record->lineCtrl[0].lineCtrlPolarityIncluded =
                            infoRec->rec.lineCtrl.lineCtrlPolarityIncluded;
                    record->lineCtrl[0].lineCtrlToggle = infoRec->rec.lineCtrl.lineCtrlToggle;
                    record->lineCtrl[0].lineCtrlReverse = infoRec->rec.lineCtrl.lineCtrlReverse;
                    record->lineCtrl[0].lineCtrlPowerDenial =
                            infoRec->rec.lineCtrl.lineCtrlPowerDenial;
                    break;
                }

                case RIL_CDMA_T53_CLIR_INFO_REC: {
                    record->clir.resize(1);
                    record->clir[0].cause = infoRec->rec.clir.cause;
                    break;
                }

                case RIL_CDMA_T53_AUDIO_CONTROL_INFO_REC: {
                    record->audioCtrl.resize(1);
                    record->audioCtrl[0].upLink = infoRec->rec.audioCtrl.upLink;
                    record->audioCtrl[0].downLink = infoRec->rec.audioCtrl.downLink;
                    break;
                }

                case RIL_CDMA_T53_RELEASE_INFO_REC:
                    RLOGE("cdmaInfoRecInd: RIL_CDMA_T53_RELEASE_INFO_REC: INVALID");
                    return 0;

                default:
                    RLOGE("cdmaInfoRecInd: Incorrect name value");
                    return 0;
            }
        }

#if VDBG
        RLOGD("cdmaInfoRecInd");
#endif
        Return<void> ret = ind->cdmaInfoRec(RadioIndicationType::UNSOLICITED, records);
        checkReturnStatus(ret);
      }
    } else {
      QCRIL_LOG_INFO("ind null: %s, msg null: %s", ind ? "false" : "true", msg ? "false" : "true");
    }
    QCRIL_LOG_FUNC_RETURN();
    return 0;
}

int RadioImpl::sendSuppSvcNotify(std::shared_ptr<QcRilUnsolSuppSvcNotificationMessage> msg)
{
  QCRIL_LOG_FUNC_ENTRY();
  auto ind = mRadioIndication;
  if (ind && msg) {
    SuppSvcNotification suppSvc = {};

    if (msg->hasNotificationType()) {
      if (msg->getNotificationType() == qcril::interfaces::NotificationType::MT) {
        suppSvc.isMT = true;
      } else {
        suppSvc.isMT = false;
      }
    }
    if (msg->hasCode()) {
      suppSvc.code = msg->getCode();
    }
    if (msg->hasIndex()) {
      suppSvc.index = msg->getIndex();
    }
    if (msg->hasType()) {
      suppSvc.type = msg->getType();
    }
    if (msg->hasNumber() && !msg->getNumber().empty()) {
      suppSvc.number = msg->getNumber().c_str();
    }

#if VDBG
    RLOGD("suppSvcNotifyInd: isMT %d code %d index %d type %d", suppSvc.isMT, suppSvc.code,
          suppSvc.index, suppSvc.type);
#endif
    Return<void> ret = ind->suppSvcNotify(RadioIndicationType::UNSOLICITED, suppSvc);
    checkReturnStatus(ret);
  } else {
    QCRIL_LOG_INFO("ind null: %s, msg null: %s", ind ? "false" : "true", msg ? "false" : "true");
  }
  QCRIL_LOG_FUNC_RETURN();
  return 0;
}

void convertRilCdmaSignalInfoRecordToHal(
    std::shared_ptr<RIL_CDMA_SignalInfoRecord> signalInfoRecord, CdmaSignalInfoRecord &record)
{
  record.isPresent = signalInfoRecord->isPresent;
  record.signalType = signalInfoRecord->signalType;
  record.alertPitch = signalInfoRecord->alertPitch;
  record.signal = signalInfoRecord->signal;
}

int RadioImpl::sendCdmaCallWaiting(std::shared_ptr<QcRilUnsolCdmaCallWaitingMessage> msg)
{
  QCRIL_LOG_FUNC_ENTRY();
  auto ind = mRadioIndication;
  if (ind && msg) {
    CdmaCallWaiting callWaitingRecord = {};

    if (msg->hasNumber() && !msg->getNumber().empty()) {
      callWaitingRecord.number = msg->getNumber().c_str();
    }

    if (msg->hasNumberPresentation()) {
      callWaitingRecord.numberPresentation =
          (CdmaCallWaitingNumberPresentation)msg->getNumberPresentation();
    }

    if (msg->hasName() && !msg->getName().empty()) {
      callWaitingRecord.name = msg->getName().c_str();
    }

    if (msg->hasNumberType()) {
      callWaitingRecord.numberType = (CdmaCallWaitingNumberType)msg->getNumberType();
    }

    if (msg->hasNumberPlan()) {
      callWaitingRecord.numberPlan = (CdmaCallWaitingNumberPlan)msg->getNumberPlan();
    }

    if (msg->hasSignalInfoRecord() && msg->getSignalInfoRecord() != nullptr) {
      convertRilCdmaSignalInfoRecordToHal(msg->getSignalInfoRecord(),
                                          callWaitingRecord.signalInfoRecord);
    }
#if VDBG
    RLOGD("cdmaCallWaitingInd");
#endif
    Return<void> ret = ind->cdmaCallWaiting(RadioIndicationType::UNSOLICITED, callWaitingRecord);
    checkReturnStatus(ret);
  } else {
    QCRIL_LOG_INFO("ind null: %s, msg null: %s", ind ? "false" : "true", msg ? "false" : "true");
  }
  QCRIL_LOG_FUNC_RETURN();
  return 0;
}

int RadioImpl::sendCdmaOtaProvisionStatus(
    std::shared_ptr<QcRilUnsolCdmaOtaProvisionStatusMessage> msg)
{
  QCRIL_LOG_FUNC_ENTRY();
  auto ind = mRadioIndication;
  if (ind && msg) {
    if (msg && msg->hasStatus()) {
      int32_t status = msg->getStatus();
#if VDBG
      RLOGD("cdmaOtaProvisionStatusInd: status %d", status);
#endif
      Return<void> ret = ind->cdmaOtaProvisionStatus(RadioIndicationType::UNSOLICITED,
                                                     (CdmaOtaProvisionStatus)status);
      checkReturnStatus(ret);
    }
  } else {
    QCRIL_LOG_INFO("ind null: %s, msg null: %s", ind ? "false" : "true", msg ? "false" : "true");
  }
  QCRIL_LOG_FUNC_RETURN();
  return 0;
}

int RadioImpl::sendIndicateRingbackTone(std::shared_ptr<QcRilUnsolRingbackToneMessage> msg)
{
  QCRIL_LOG_FUNC_ENTRY();
  auto ind = mRadioIndication;
  if (ind && msg) {
    if (msg->hasRingBackToneOperation()) {
      bool start =
          (msg->getRingBackToneOperation() == qcril::interfaces::RingBackToneOperation::START)
              ? true
              : false;
#if VDBG
      RLOGD("indicateRingbackToneInd: start %d", start);
#endif
      Return<void> ret = ind->indicateRingbackTone(RadioIndicationType::UNSOLICITED, start);
      checkReturnStatus(ret);
    }
  } else {
    QCRIL_LOG_INFO("ind null: %s, msg null: %s", ind ? "false" : "true", msg ? "false" : "true");
  }
  QCRIL_LOG_FUNC_RETURN();
  return 0;
}

int RadioImpl::sendSrvccStateNotify(std::shared_ptr<QcRilUnsolSrvccStatusMessage> msg) {
  auto ind = mRadioIndication;
  if (ind && msg) {
    if (msg->hasState()) {
      int32_t state = msg->getState();
#if VDBG
      RLOGD("srvccStateNotifyInd: state %d", state);
#endif
      Return<void> ret = ind->srvccStateNotify(RadioIndicationType::UNSOLICITED, (SrvccState)state);
      checkReturnStatus(ret);
    }
  } else {
    QCRIL_LOG_INFO("ind null: %s, msg null: %s", ind ? "false" : "true", msg ? "false" : "true");
  }
  QCRIL_LOG_FUNC_RETURN();
  return 0;
}

bool isServiceTypeCfQuery(RIL_SsServiceType serType, RIL_SsRequestType reqType) {
    if ((reqType == SS_INTERROGATION) &&
        (serType == SS_CFU ||
         serType == SS_CF_BUSY ||
         serType == SS_CF_NO_REPLY ||
         serType == SS_CF_NOT_REACHABLE ||
         serType == SS_CF_ALL ||
         serType == SS_CF_ALL_CONDITIONAL)) {
        return true;
    }
    return false;
}

int RadioImpl::sendOnSupplementaryServiceIndication(
    std::shared_ptr<QcRilUnsolSupplementaryServiceMessage> msg) {
  QCRIL_LOG_FUNC_ENTRY();
  auto ind = mRadioIndication;
  if (ind && msg) {
    StkCcUnsolSsResult ss = {};
    if (msg->hasServiceType()) {
      ss.serviceType = (SsServiceType)msg->getServiceType();
    }
    if (msg->hasRequestType()) {
      ss.requestType = (SsRequestType)msg->getRequestType();
    }
    if (msg->hasTeleserviceType()) {
      ss.teleserviceType = (SsTeleserviceType)msg->getTeleserviceType();
    }
    if (msg->hasServiceClass()) {
      ss.serviceClass = msg->getServiceClass();
    }
    if (msg->hasResult()) {
      ss.result = (RadioError)msg->getResult();
    }

      if (isServiceTypeCfQuery(msg->getServiceType(), msg->getRequestType()) &&
          msg->hasCallForwardInfoList() && !msg->getCallForwardInfoList().empty()) {
        std::vector<qcril::interfaces::CallForwardInfo> rilCfInfo = msg->getCallForwardInfoList();
#if VDBG
        RLOGD("onSupplementaryServiceIndicationInd CF type, num of Cf elements %d",
              rilCfInfo.size());
#endif
#if 0
        if (rilSsResponse.cfData.numValidIndexes > NUM_SERVICE_CLASSES) {
          RLOGE(
              "onSupplementaryServiceIndicationInd numValidIndexes is greater than "
              "max value %d, truncating it to max value",
              NUM_SERVICE_CLASSES);
          rilSsResponse.cfData.numValidIndexes = NUM_SERVICE_CLASSES;
        }
#endif

        ss.cfData.resize(1);
        ss.ssInfo.resize(0);

        /* number of call info's */
        ss.cfData[0].cfInfo.resize(rilCfInfo.size());

        for (int i = 0; i < rilCfInfo.size() && i < NUM_SERVICE_CLASSES; i++) {
          qcril::interfaces::CallForwardInfo cf = rilCfInfo[i];
          CallForwardInfo *cfInfo = &ss.cfData[0].cfInfo[i];

          if (cf.hasStatus()) {
            cfInfo->status = (CallForwardInfoStatus)cf.getStatus();
          }
          if (cf.hasReason()) {
            cfInfo->reason = cf.getReason();
          }
          if (cf.hasServiceClass()) {
            cfInfo->serviceClass = cf.getServiceClass();
          }
          if (cf.hasToa()) {
            cfInfo->toa = cf.getToa();
          }
          if (!cf.getNumber().empty()) {
            cfInfo->number = cf.getNumber().c_str();
          }
          if (cf.hasTimeSeconds()) {
            cfInfo->timeSeconds = cf.getTimeSeconds();
          }
#if 0
#if VDBG
          RLOGD(
              "onSupplementaryServiceIndicationInd: "
              "Data: %d,reason=%d,cls=%d,toa=%d,num=%s,tout=%d],",
              cf.status, cf.reason, cf.serviceClass, cf.toa, (char *)cf.number, cf.timeSeconds);
#endif
#endif
        }
      } else {
        ss.ssInfo.resize(1);
        ss.cfData.resize(0);

        if (msg->hasSuppSrvInfoList() && !msg->getSuppSrvInfoList().empty()) {
          /* each int */
          ss.ssInfo[0].ssInfo.resize(SS_INFO_MAX);
          for (int i = 0; i < msg->getSuppSrvInfoList().size() && i < SS_INFO_MAX; i++) {
#if VDBG
            RLOGD("onSupplementaryServiceIndicationInd: Data: %d", msg->getSuppSrvInfoList()[i]);
#endif
            ss.ssInfo[0].ssInfo[i] = msg->getSuppSrvInfoList()[i];
          }
        }
      }

#if VDBG
      RLOGD("onSupplementaryServiceIndicationInd");
#endif
      Return<void> ret =
          ind->onSupplementaryServiceIndication(RadioIndicationType::UNSOLICITED, ss);
      checkReturnStatus(ret);
  } else {
    QCRIL_LOG_INFO("ind null: %s, msg null: %s", ind ? "false" : "true", msg ? "false" : "true");
  }
  QCRIL_LOG_FUNC_RETURN();
  return 0;
}

int RadioImpl::sendCallStateChanged(std::shared_ptr<QcRilUnsolCallStateChangeMessage> msg)
{
  auto ind = mRadioIndication;
  if (ind && msg) {
#if VDBG
    RLOGD("callStateChangedInd");
#endif
    Return<void> ret = ind->callStateChanged(RadioIndicationType::UNSOLICITED);
    checkReturnStatus(ret);
  } else {
    QCRIL_LOG_INFO("ind null: %s, msg null: %s", ind ? "false" : "true", msg ? "false" : "true");
  }
  QCRIL_LOG_FUNC_RETURN();
  return 0;
}

int RadioImpl::sendAcknowledgeRequest(std::shared_ptr<RilAcknowledgeRequestMessage> msg)
{
    auto ctx = msg->getContext();
    if (ctx != nullptr) {
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponse, acknowledgeRequest, ctx->serial);
    }
    QCRIL_LOG_FUNC_RETURN();
    return 0;
}


int RadioImpl::sendNetworkStateChanged(std::shared_ptr<RilUnsolNetworkStateChangedMessage> msg)
{
    auto ind = mRadioIndication;
    QCRIL_LOG_INFO("ind null: %s", ind ? "false" : "true" );
    if (ind) {
        Return<void> ret = ind->networkStateChanged(RadioIndicationType::UNSOLICITED);
        checkReturnStatus(ret);
    }
    QCRIL_LOG_FUNC_RETURN();
    return 0;
}

int RadioImpl::sendNitzTimeReceived(std::shared_ptr<RilUnsolNitzTimeReceivedMessage> msg)
{
#ifndef QMI_RIL_UTF // TODO: find a way to get elapsedRealtime in host compilation
    auto ind = mRadioIndication;
    QCRIL_LOG_INFO("ind null: %s", ind ? "false" : "true" );
    if (ind) {
        auto& time = msg->getNitzTime();
        int64_t timeReceived = android::elapsedRealtime();
#if VDBG
        RLOGD("nitzTimeReceivedInd: nitzTime %s receivedTime %" PRId64, time.c_str(),
                timeReceived);
#endif
        Return<void> ret = ind->nitzTimeReceived(RadioIndicationType::UNSOLICITED,
                time, timeReceived);
        checkReturnStatus(ret);
    } else {
        RLOGE("nitzTimeReceivedInd: radioService[%d]->mRadioIndication == NULL", mSlotId);
        return -1;
    }
    QCRIL_LOG_FUNC_RETURN();
#endif
    return 0;
}

int RadioImpl::sendVoiceRadioTechChanged(std::shared_ptr<RilUnsolVoiceRadioTechChangedMessage> msg)
{
    auto ind = mRadioIndication;
    QCRIL_LOG_INFO("ind null: %s", ind ? "false" : "true" );
    if (ind) {
        Return<void> ret = ind->voiceRadioTechChanged(RadioIndicationType::UNSOLICITED,
                (RadioTechnology)msg->getVoiceRat());
        checkReturnStatus(ret);
    }
    QCRIL_LOG_FUNC_RETURN();
    return 0;
}

int RadioImpl::sendNetworkScanResult(std::shared_ptr<RilUnsolNetworkScanResultMessage> msg)
{
    auto ind = mRadioIndicationV1_1;
    QCRIL_LOG_INFO("ind null: %s", ind ? "false" : "true" );
    if (ind) {
        V1_1::NetworkScanResult result;
        result.status = (V1_1::ScanStatus)msg->getStatus();
        result.error = (RadioError)msg->getError();

        convertRilCellInfoListToHal(msg->getNetworkInfo(), result.networkInfos);
        Return<void> retStatus = ind->networkScanResult(
                RadioIndicationType::UNSOLICITED, result);
        checkReturnStatus(retStatus);
    }
    QCRIL_LOG_FUNC_RETURN();
    return 0;
}

int RadioImpl::sendSignalStrength(std::shared_ptr<RilUnsolSignalStrengthMessage> msg)
{
    auto ind = mRadioIndication;
    QCRIL_LOG_INFO("ind null: %s", ind ? "false" : "true" );
    if (ind) {
        SignalStrength signalStrength = {};
        convertRilSignalStrengthToHal(msg->getSignalStrength(), signalStrength);
        Return<void> ret = ind->currentSignalStrength(RadioIndicationType::UNSOLICITED,
                signalStrength);
        checkReturnStatus(ret);
    }
    QCRIL_LOG_FUNC_RETURN();
    return 0;
}

int RadioImpl::sendEmergencyCallbackMode(std::shared_ptr<RilUnsolEmergencyCallbackModeMessage> msg)
{
    auto ind = mRadioIndication;
    QCRIL_LOG_INFO("ind null: %s", ind ? "false" : "true" );
    if (ind) {
        auto mode = msg->getEmergencyCallbackMode();
        if (mode == RilUnsolEmergencyCallbackModeMessage::EmergencyCallbackMode::ENTER) {
            Return<void> ret = ind->enterEmergencyCallbackMode(RadioIndicationType::UNSOLICITED);
            checkReturnStatus(ret);
        } else {
            Return<void> ret = ind->exitEmergencyCallbackMode(RadioIndicationType::UNSOLICITED);
            checkReturnStatus(ret);
        }
    }
    QCRIL_LOG_FUNC_RETURN();
    return 0;
}

int RadioImpl::sendRadioCapability(std::shared_ptr<RilUnsolRadioCapabilityMessage> msg)
{
    auto ind = mRadioIndication;
    QCRIL_LOG_INFO("ind null: %s", ind ? "false" : "true" );
    if (ind) {
        RadioCapability rc = {};
        convertRilRadioCapabilityToHal(msg->getRadioCapability(), rc);
        Return<void> ret = ind->radioCapabilityIndication(
                RadioIndicationType::UNSOLICITED, rc);
        checkReturnStatus(ret);
    }
    QCRIL_LOG_FUNC_RETURN();
    return 0;
}

int RadioImpl::sendCdmaPrlChanged(std::shared_ptr<RilUnsolCdmaPrlChangedMessage> msg)
{
    auto ind = mRadioIndication;
    QCRIL_LOG_INFO("ind null: %s", ind ? "false" : "true" );
    if (ind) {
        int32_t version = msg->getPrl();
        Return<void> ret = ind->cdmaPrlChanged(RadioIndicationType::UNSOLICITED, version);
        checkReturnStatus(ret);
    }
    QCRIL_LOG_FUNC_RETURN();
    return 0;
}

int RadioImpl::sendRestrictedStateChanged(std::shared_ptr<RilUnsolRestrictedStateChangedMessage> msg)
{
    auto ind = mRadioIndication;
    QCRIL_LOG_INFO("ind null: %s", ind ? "false" : "true" );
    if (ind) {
        int32_t state = msg->getState();
        Return<void> ret = ind->restrictedStateChanged(RadioIndicationType::UNSOLICITED,
                (PhoneRestrictedState)state);
        checkReturnStatus(ret);
    }
    QCRIL_LOG_FUNC_RETURN();
    return 0;
}

int RadioImpl::sendUiccSubsStatusChanged(std::shared_ptr<RilUnsolUiccSubsStatusChangedMessage> msg)
{
    auto ind = mRadioIndication;
    QCRIL_LOG_INFO("ind null: %s", ind ? "false" : "true" );
    if (ind) {
        bool activate = msg->getStatus();
        Return<void> ret = ind->subscriptionStatusChanged(
                RadioIndicationType::UNSOLICITED, activate);
        checkReturnStatus(ret);
    }
    QCRIL_LOG_FUNC_RETURN();
    return 0;
}

int RadioImpl::sendRadioStateChanged(std::shared_ptr<RilUnsolRadioStateChangedMessage> msg)
{
    auto ind = mRadioIndication;
    QCRIL_LOG_INFO("ind null: %s", ind ? "false" : "true" );
    if (ind) {
        RadioState radioState =
                (RadioState) CALL_ONSTATEREQUEST(slotId);
        QCRIL_LOG_INFO("radioStateChangedInd: radioState %d", radioState);
        Return<void> ret = ind->radioStateChanged(
            RadioIndicationType::UNSOLICITED, radioState);
        checkReturnStatus(ret);
    }
    QCRIL_LOG_FUNC_RETURN();
    return 0;
}

int RadioImpl::sendSimStatusChanged(std::shared_ptr<UimSimStatusChangedInd> msg)
{
    auto ind = mRadioIndication;
    QCRIL_LOG_INFO("ind null: %s", ind ? "false" : "true" );
    if (ind) {
        Return<void> ret = ind->simStatusChanged(RadioIndicationType::UNSOLICITED);
        checkReturnStatus(ret);
    }
    QCRIL_LOG_FUNC_RETURN();
    return 0;
}

int RadioImpl::sendSimRefresh(std::shared_ptr<UimSimRefreshIndication> msg)
{
    auto ind = mRadioIndication;
    QCRIL_LOG_INFO("ind null: %s", ind ? "false" : "true" );
    if (ind && msg) {
        SimRefreshResult refreshResult = {};
        RIL_UIM_SIM_RefreshIndication simRefreshResponse = msg->get_refresh_ind();
        refreshResult.type =
                (V1_0::SimRefreshType) simRefreshResponse.result;
        refreshResult.efId = simRefreshResponse.ef_id;
        refreshResult.aid = simRefreshResponse.aid;

#if VDBG
        RLOGD("simRefreshInd: type %d efId %d", refreshResult.type, refreshResult.efId);
#endif
        Return<void> ret = ind->simRefresh(RadioIndicationType::UNSOLICITED, refreshResult);
        checkReturnStatus(ret);
    }
    QCRIL_LOG_FUNC_RETURN();
    return 0;
}

int RadioImpl::sendGstkIndication(std::shared_ptr<GstkUnsolIndMsg> msg)
{
    auto         ind = mRadioIndication;

    QCRIL_LOG_INFO("ind null: %s", ind ? "false" : "true" );
    if (ind && msg) {
        Return<void>       ret     = {};
        RIL_GSTK_UnsolData stk_ind = msg->get_unsol_data();

        switch(stk_ind.type) {
            case RIL_GSTK_UNSOL_SESSION_END:
                ret = ind->stkSessionEnd(RadioIndicationType::UNSOLICITED);
                break;
            case RIL_GSTK_UNSOL_PROACTIVE_COMMAND:
                ret = ind->stkProactiveCommand(RadioIndicationType::UNSOLICITED, stk_ind.cmd);
                break;
            case RIL_GSTK_UNSOL_EVENT_NOTIFY:
                ret = ind->stkEventNotify(RadioIndicationType::UNSOLICITED, stk_ind.cmd);
                break;
            case RIL_GSTK_UNSOL_STK_CALL_SETUP:
                // For call setup send event notify followed by call setup
                ret = ind->stkEventNotify(RadioIndicationType::UNSOLICITED, stk_ind.cmd);
                ret = ind->stkCallSetup(RadioIndicationType::UNSOLICITED,
                                        stk_ind.setupCallTimeout);
                break;
            default:
                QCRIL_LOG_FUNC_RETURN();
                return 0;
        }
        checkReturnStatus(ret);
    }
    QCRIL_LOG_FUNC_RETURN();
    return 0;
}

RIL_CellInfoType getCellInfoTypeRadioTechnology(char *rat) {
    if (rat == NULL) {
        return RIL_CELL_INFO_TYPE_NONE;
    }

    int radioTech = atoi(rat);

    switch(radioTech) {

        case RADIO_TECH_GPRS:
        case RADIO_TECH_EDGE:
        case RADIO_TECH_GSM: {
            return RIL_CELL_INFO_TYPE_GSM;
        }

        case RADIO_TECH_UMTS:
        case RADIO_TECH_HSDPA:
        case RADIO_TECH_HSUPA:
        case RADIO_TECH_HSPA:
        case RADIO_TECH_HSPAP: {
            return RIL_CELL_INFO_TYPE_WCDMA;
        }

        case RADIO_TECH_IS95A:
        case RADIO_TECH_IS95B:
        case RADIO_TECH_1xRTT:
        case RADIO_TECH_EVDO_0:
        case RADIO_TECH_EVDO_A:
        case RADIO_TECH_EVDO_B:
        case RADIO_TECH_EHRPD: {
            return RIL_CELL_INFO_TYPE_CDMA;
        }

        case RADIO_TECH_LTE:
        case RADIO_TECH_LTE_CA: {
            return RIL_CELL_INFO_TYPE_LTE;
        }

        case RADIO_TECH_TD_SCDMA: {
            return RIL_CELL_INFO_TYPE_TD_SCDMA;
        }

        default: {
            break;
        }
    }

    return RIL_CELL_INFO_TYPE_NONE;

}


int convertResponseStringEntryToInt(char **response, int index, int numStrings) {
    if ((response != NULL) &&  (numStrings > index) && (response[index] != NULL)) {
        return atoi(response[index]);
    }

    return -1;
}

int convertResponseHexStringEntryToInt(char **response, int index, int numStrings) {
    const int hexBase = 16;
    if ((response != NULL) &&  (numStrings > index) && (response[index] != NULL)) {
        return strtol(response[index], NULL, hexBase);
    }

    return -1;
}

/* Fill Cell Identity info from Voice Registration State Response.
 * This fucntion is applicable only for RIL Version < 15.
 * Response is a  "char **".
 * First and Second entries are in hex string format
 * and rest are integers represented in ascii format. */
void fillCellIdentityFromVoiceRegStateResponseString(CellIdentity &cellIdentity,
        int numStrings, char** response) {

    RIL_CellIdentity_v16 rilCellIdentity;
    memset(&rilCellIdentity, -1, sizeof(RIL_CellIdentity_v16));

    rilCellIdentity.cellInfoType = getCellInfoTypeRadioTechnology(response[3]);
    switch(rilCellIdentity.cellInfoType) {

        case RIL_CELL_INFO_TYPE_GSM: {
            /* valid LAC are hexstrings in the range 0x0000 - 0xffff */
            rilCellIdentity.cellIdentityGsm.lac =
                    convertResponseHexStringEntryToInt(response, 1, numStrings);

            /* valid CID are hexstrings in the range 0x00000000 - 0xffffffff */
            rilCellIdentity.cellIdentityGsm.cid =
                    convertResponseHexStringEntryToInt(response, 2, numStrings);
            break;
        }

        case RIL_CELL_INFO_TYPE_WCDMA: {
            /* valid LAC are hexstrings in the range 0x0000 - 0xffff */
            rilCellIdentity.cellIdentityWcdma.lac =
                    convertResponseHexStringEntryToInt(response, 1, numStrings);

            /* valid CID are hexstrings in the range 0x00000000 - 0xffffffff */
            rilCellIdentity.cellIdentityWcdma.cid =
                    convertResponseHexStringEntryToInt(response, 2, numStrings);
            rilCellIdentity.cellIdentityWcdma.psc =
                    convertResponseStringEntryToInt(response, 14, numStrings);
            break;
        }

        case RIL_CELL_INFO_TYPE_TD_SCDMA:{
            /* valid LAC are hexstrings in the range 0x0000 - 0xffff */
            rilCellIdentity.cellIdentityTdscdma.lac =
                    convertResponseHexStringEntryToInt(response, 1, numStrings);

            /* valid CID are hexstrings in the range 0x00000000 - 0xffffffff */
            rilCellIdentity.cellIdentityTdscdma.cid =
                    convertResponseHexStringEntryToInt(response, 2, numStrings);
            break;
        }

        case RIL_CELL_INFO_TYPE_CDMA:{
            rilCellIdentity.cellIdentityCdma.basestationId =
                    convertResponseStringEntryToInt(response, 4, numStrings);
            /* Order of Lat. and Long. swapped between RIL and HIDL interface versions. */
            rilCellIdentity.cellIdentityCdma.latitude =
                    convertResponseStringEntryToInt(response, 5, numStrings);
            rilCellIdentity.cellIdentityCdma.longitude =
                    convertResponseStringEntryToInt(response, 6, numStrings);
            rilCellIdentity.cellIdentityCdma.systemId =
                    convertResponseStringEntryToInt(response, 8, numStrings);
            rilCellIdentity.cellIdentityCdma.networkId =
                    convertResponseStringEntryToInt(response, 9, numStrings);
            break;
        }

        case RIL_CELL_INFO_TYPE_LTE:{
            /* valid TAC are hexstrings in the range 0x0000 - 0xffff */
            rilCellIdentity.cellIdentityLte.tac =
                    convertResponseHexStringEntryToInt(response, 1, numStrings);

            /* valid CID are hexstrings in the range 0x00000000 - 0xffffffff */
            rilCellIdentity.cellIdentityLte.ci =
                    convertResponseHexStringEntryToInt(response, 2, numStrings);
            break;
        }

        default: {
            break;
        }
    }

    fillCellIdentityResponse(cellIdentity, rilCellIdentity);
}

/* Fill Cell Identity info from Data Registration State Response.
 * This fucntion is applicable only for RIL Version < 15.
 * Response is a  "char **".
 * First and Second entries are in hex string format
 * and rest are integers represented in ascii format. */
void fillCellIdentityFromDataRegStateResponseString(CellIdentity &cellIdentity,
        int numStrings, char** response) {

    RIL_CellIdentity_v16 rilCellIdentity;
    memset(&rilCellIdentity, -1, sizeof(RIL_CellIdentity_v16));

    rilCellIdentity.cellInfoType = getCellInfoTypeRadioTechnology(response[3]);
    switch(rilCellIdentity.cellInfoType) {
        case RIL_CELL_INFO_TYPE_GSM: {
            /* valid LAC are hexstrings in the range 0x0000 - 0xffff */
            rilCellIdentity.cellIdentityGsm.lac =
                    convertResponseHexStringEntryToInt(response, 1, numStrings);

            /* valid CID are hexstrings in the range 0x00000000 - 0xffffffff */
            rilCellIdentity.cellIdentityGsm.cid =
                    convertResponseHexStringEntryToInt(response, 2, numStrings);
            break;
        }
        case RIL_CELL_INFO_TYPE_WCDMA: {
            /* valid LAC are hexstrings in the range 0x0000 - 0xffff */
            rilCellIdentity.cellIdentityWcdma.lac =
                    convertResponseHexStringEntryToInt(response, 1, numStrings);

            /* valid CID are hexstrings in the range 0x00000000 - 0xffffffff */
            rilCellIdentity.cellIdentityWcdma.cid =
                    convertResponseHexStringEntryToInt(response, 2, numStrings);
            break;
        }
        case RIL_CELL_INFO_TYPE_TD_SCDMA:{
            /* valid LAC are hexstrings in the range 0x0000 - 0xffff */
            rilCellIdentity.cellIdentityTdscdma.lac =
                    convertResponseHexStringEntryToInt(response, 1, numStrings);

            /* valid CID are hexstrings in the range 0x00000000 - 0xffffffff */
            rilCellIdentity.cellIdentityTdscdma.cid =
                    convertResponseHexStringEntryToInt(response, 2, numStrings);
            break;
        }
        case RIL_CELL_INFO_TYPE_LTE: {
            rilCellIdentity.cellIdentityLte.tac =
                    convertResponseStringEntryToInt(response, 6, numStrings);
            rilCellIdentity.cellIdentityLte.pci =
                    convertResponseStringEntryToInt(response, 7, numStrings);
            rilCellIdentity.cellIdentityLte.ci =
                    convertResponseStringEntryToInt(response, 8, numStrings);
            break;
        }
        default: {
            break;
        }
    }

    fillCellIdentityResponse(cellIdentity, rilCellIdentity);
}

int radio::getVoiceRegistrationStateResponse(int slotId,
                                            int responseType, int serial, RIL_Errno e,
                                            void *response, size_t responseLen) {
    auto srvc = radioService[slotId];
    if (srvc != NULL) {
        return srvc->sendGetVoiceRegistrationStateResponse(
                slotId,
                responseType,
                serial,
                e,
                response,
                responseLen);
    }
    return 0;
}

template <class T>
void fillCellIdentityGsm(T &out, const RIL_CellIdentityGsm_v12 &in) {
      out.mcc = in.mcc;
      out.mnc = in.mnc;
      out.lac = in.lac;
      out.cid = in.cid;
    out.arfcn = in.arfcn;
     out.bsic = in.bsic;
}

template <class T>
void fillCellIdentityWcdma(T &out, const RIL_CellIdentityWcdma_v12 &in) {
       out.mcc = in.mcc;
       out.mnc = in.mnc;
       out.lac = in.lac;
       out.cid = in.cid;
       out.psc = in.psc;
    out.uarfcn = in.uarfcn;
}

template <class T>
void fillCellIdentityCdma(T &out, const RIL_CellIdentityCdma &in) {
        out.networkId = in.networkId;
         out.systemId = in.systemId;
    out.baseStationId = in.basestationId;
        out.longitude = in.longitude;
         out.latitude = in.latitude;
}

template <class T>
void fillCellIdentityLte(T &out, const RIL_CellIdentityLte_v12 &in) {
       out.mcc = in.mcc;
       out.mnc = in.mnc;
        out.ci = in.ci;
       out.pci = in.pci;
       out.tac = in.tac;
    out.earfcn = in.earfcn;
}

template <class T>
void fillCellIdentityTdscdma(T &out, const RIL_CellIdentityTdscdma &in) {
     out.mcc = in.mcc;
     out.mnc = in.mnc;
     out.lac = in.lac;
     out.cid = in.cid;
    out.cpid = in.cpid;
}

int RadioImpl::sendGetVoiceRegistrationStateResponse(int slotId,
                                            int responseType, int serial, RIL_Errno e,
                                            void *response, size_t responseLen) {
#if VDBG
    RLOGD("sendGetVoiceRegistrationStateResponse: serial %d", serial);
#endif

    auto rsp = mRadioResponse;
    if (rsp != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);

        VoiceRegStateResult voiceRegResponse = {};
        int numStrings = responseLen / sizeof(char *);
        if (response == NULL) {
               RLOGE("sendGetVoiceRegistrationStateResponse Invalid response: NULL");
               if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else if (s_vendorFunctions->version <= 14) {
            if (numStrings != 15) {
                RLOGE("sendGetVoiceRegistrationStateResponse Invalid response: NULL");
                if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
            } else {
                char **resp = (char **) response;
                voiceRegResponse.regState = (RegState) ATOI_NULL_HANDLED_DEF(resp[0], 4);
                voiceRegResponse.rat = ATOI_NULL_HANDLED(resp[3]);
                voiceRegResponse.cssSupported = ATOI_NULL_HANDLED_DEF(resp[7], 0);
                voiceRegResponse.roamingIndicator = ATOI_NULL_HANDLED(resp[10]);
                voiceRegResponse.systemIsInPrl = ATOI_NULL_HANDLED_DEF(resp[11], 0);
                voiceRegResponse.defaultRoamingIndicator = ATOI_NULL_HANDLED_DEF(resp[12], 0);
                voiceRegResponse.reasonForDenial = ATOI_NULL_HANDLED_DEF(resp[13], 0);
                fillCellIdentityFromVoiceRegStateResponseString(voiceRegResponse.cellIdentity,
                        numStrings, resp);
            }
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
                rsp->getVoiceRegistrationStateResponse(
                responseInfo, voiceRegResponse);
        checkReturnStatus(retStatus);
    } else {
        RLOGE("sendGetVoiceRegistrationStateResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::getDataRegistrationStateResponse(int slotId,
                                           int responseType, int serial, RIL_Errno e,
                                           void *response, size_t responseLen) {
    auto srvc = radioService[slotId];
    if (srvc != NULL) {
        return srvc->sendGetDataRegistrationStateResponse(slotId,
                responseType,
                serial,
                e,
                response,
                responseLen);
    } else {
        RLOGE("getDataRegistrationStateResponse: radioService[%d] == NULL",
                slotId);
    }
    return 0;
}

int RadioImpl::sendGetDataRegistrationStateResponse(int slotId,
                                           int responseType, int serial, RIL_Errno e,
                                           void *response, size_t responseLen) {
#if VDBG
    RLOGD("getDataRegistrationStateResponse: serial %d", serial);
#endif

    auto rsp = mRadioResponse;
    if (rsp != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        DataRegStateResult dataRegResponse = {};
        if (response == NULL) {
            RLOGE("getDataRegistrationStateResponse Invalid response: NULL");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else if (s_vendorFunctions->version <= 14) {
            int numStrings = responseLen / sizeof(char *);
            if ((numStrings != 6) && (numStrings != 11)) {
                RLOGE("getDataRegistrationStateResponse Invalid response: NULL");
                if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
            } else {
                char **resp = (char **) response;
                dataRegResponse.regState = (RegState) ATOI_NULL_HANDLED_DEF(resp[0], 4);
                dataRegResponse.rat =  ATOI_NULL_HANDLED_DEF(resp[3], 0);
                dataRegResponse.reasonDataDenied =  ATOI_NULL_HANDLED(resp[4]);
                dataRegResponse.maxDataCalls =  ATOI_NULL_HANDLED_DEF(resp[5], 1);
                fillCellIdentityFromDataRegStateResponseString(dataRegResponse.cellIdentity,
                        numStrings, resp);
            }
        } else {
            RIL_DataRegistrationStateResponse *dataRegState =
                    (RIL_DataRegistrationStateResponse *)response;

            if (responseLen != sizeof(RIL_DataRegistrationStateResponse)) {
                RLOGE("getDataRegistrationStateResponse Invalid response: NULL");
                if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
            } else {
                fillDataRegistrationStateResponse(dataRegResponse, *dataRegState);
            }
        }

        Return<void> retStatus =
                rsp->getDataRegistrationStateResponse(responseInfo,
                dataRegResponse);
        checkReturnStatus(retStatus);
    } else {
        RLOGE("getDataRegistrationStateResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::getOperatorResponse(int slotId,
                              int responseType, int serial, RIL_Errno e, void *response,
                              size_t responseLen) {
#if VDBG
    RLOGD("getOperatorResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        hidl_string longName;
        hidl_string shortName;
        hidl_string numeric;
        int numStrings = responseLen / sizeof(char *);
        if (response == NULL || numStrings != 3) {
            RLOGE("getOperatorResponse Invalid response: NULL");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;

        } else {
            char **resp = (char **) response;
            longName = convertCharPtrToHidlString(resp[0]);
            shortName = convertCharPtrToHidlString(resp[1]);
            numeric = convertCharPtrToHidlString(resp[2]);
        }
        Return<void> retStatus = radioService[slotId]->mRadioResponse->getOperatorResponse(
                responseInfo, longName, shortName, numeric);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("getOperatorResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::setRadioPowerResponse(int slotId,
                                int responseType, int serial, RIL_Errno e, void *response,
                                size_t responseLen) {
    RLOGD("setRadioPowerResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = radioService[slotId]->mRadioResponse->setRadioPowerResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("setRadioPowerResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::sendDtmfResponse(int slotId,
                           int responseType, int serial, RIL_Errno e, void *response,
                           size_t responseLen) {
#if VDBG
    RLOGD("sendDtmfResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = radioService[slotId]->mRadioResponse->sendDtmfResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("sendDtmfResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::sendUssdResponse(int slotId,
                           int responseType, int serial, RIL_Errno e, void *response,
                           size_t responseLen) {
#if VDBG
    RLOGD("sendUssdResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = radioService[slotId]->mRadioResponse->sendUssdResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("sendUssdResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::cancelPendingUssdResponse(int slotId,
                                    int responseType, int serial, RIL_Errno e, void *response,
                                    size_t responseLen) {
#if VDBG
    RLOGD("cancelPendingUssdResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = radioService[slotId]->mRadioResponse->cancelPendingUssdResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("cancelPendingUssdResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

#if 0
int radio::acknowledgeLastIncomingGsmSmsResponse(int slotId,
                                                int responseType, int serial, RIL_Errno e,
                                                void *response, size_t responseLen) {
#if VDBG
    RLOGD("acknowledgeLastIncomingGsmSmsResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus =
                radioService[slotId]->mRadioResponse->acknowledgeLastIncomingGsmSmsResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("acknowledgeLastIncomingGsmSmsResponse: radioService[%d]->mRadioResponse "
                "== NULL", slotId);
    }

    return 0;
}
#endif

int radio::acceptCallResponse(int slotId,
                             int responseType, int serial, RIL_Errno e,
                             void *response, size_t responseLen) {
#if VDBG
    RLOGD("acceptCallResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = radioService[slotId]->mRadioResponse->acceptCallResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("acceptCallResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::deactivateDataCallResponse(int slotId,
                                                int responseType, int serial, RIL_Errno e,
                                                void *response, size_t responseLen) {
#if VDBG
    RLOGD("deactivateDataCallResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = radioService[slotId]->mRadioResponse->deactivateDataCallResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("deactivateDataCallResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::setFacilityLockForAppResponse(int slotId,
                                      int responseType, int serial, RIL_Errno e,
                                      void *response, size_t responseLen) {
#if VDBG
    RLOGD("setFacilityLockForAppResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        int ret = responseIntOrEmpty(responseInfo, serial, responseType, e, response, responseLen);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->setFacilityLockForAppResponse(responseInfo,
                ret);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("setFacilityLockForAppResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::getNetworkSelectionModeResponse(int slotId,
                                          int responseType, int serial, RIL_Errno e, void *response,
                                          size_t responseLen) {
#if VDBG
    RLOGD("getNetworkSelectionModeResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        bool manual = false;
        if (response == NULL || responseLen != sizeof(int)) {
            RLOGE("getNetworkSelectionModeResponse Invalid response: NULL");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            int *pInt = (int *) response;
            manual = pInt[0] == 1 ? true : false;
        }
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->getNetworkSelectionModeResponse(
                responseInfo,
                manual);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("getNetworkSelectionModeResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::setNetworkSelectionModeAutomaticResponse(int slotId, int responseType, int serial,
                                                    RIL_Errno e, void *response,
                                                    size_t responseLen) {
#if VDBG
    RLOGD("setNetworkSelectionModeAutomaticResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->setNetworkSelectionModeAutomaticResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("setNetworkSelectionModeAutomaticResponse: radioService[%d]->mRadioResponse "
                "== NULL", slotId);
    }

    return 0;
}

int radio::setNetworkSelectionModeManualResponse(int slotId,
                             int responseType, int serial, RIL_Errno e,
                             void *response, size_t responseLen) {
#if VDBG
    RLOGD("setNetworkSelectionModeManualResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->setNetworkSelectionModeManualResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("acceptCallResponse: radioService[%d]->setNetworkSelectionModeManualResponse "
                "== NULL", slotId);
    }

    return 0;
}

int convertOperatorStatusToInt(const char *str) {
    if (strncmp("unknown", str, 9) == 0) {
        return (int) OperatorStatus::UNKNOWN;
    } else if (strncmp("available", str, 9) == 0) {
        return (int) OperatorStatus::AVAILABLE;
    } else if (strncmp("current", str, 9) == 0) {
        return (int) OperatorStatus::CURRENT;
    } else if (strncmp("forbidden", str, 9) == 0) {
        return (int) OperatorStatus::FORBIDDEN;
    } else {
        return -1;
    }
}

int radio::getAvailableNetworksResponse(int slotId,
                              int responseType, int serial, RIL_Errno e, void *response,
                              size_t responseLen) {
#if VDBG
    RLOGD("getAvailableNetworksResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        hidl_vec<OperatorInfo> networks;
        if ((response == NULL && responseLen != 0)
                || responseLen % (4 * sizeof(char *))!= 0) {
            RLOGE("getAvailableNetworksResponse Invalid response: NULL");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            char **resp = (char **) response;
            int numStrings = responseLen / sizeof(char *);
            networks.resize(numStrings/4);
            for (int i = 0, j = 0; i < numStrings; i = i + 4, j++) {
                networks[j].alphaLong = convertCharPtrToHidlString(resp[i]);
                networks[j].alphaShort = convertCharPtrToHidlString(resp[i + 1]);
                networks[j].operatorNumeric = convertCharPtrToHidlString(resp[i + 2]);
                int status = convertOperatorStatusToInt(resp[i + 3]);
                if (status == -1) {
                    if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
                } else {
                    networks[j].status = (OperatorStatus) status;
                }
            }
        }
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->getAvailableNetworksResponse(responseInfo,
                networks);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("getAvailableNetworksResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::startDtmfResponse(int slotId,
                            int responseType, int serial, RIL_Errno e,
                            void *response, size_t responseLen) {
#if VDBG
    RLOGD("startDtmfResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->startDtmfResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("startDtmfResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::stopDtmfResponse(int slotId,
                           int responseType, int serial, RIL_Errno e,
                           void *response, size_t responseLen) {
#if VDBG
    RLOGD("stopDtmfResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->stopDtmfResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("stopDtmfResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::getBasebandVersionResponse(int slotId,
                                     int responseType, int serial, RIL_Errno e,
                                     void *response, size_t responseLen) {
#if VDBG
    RLOGD("getBasebandVersionResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->getBasebandVersionResponse(responseInfo,
                convertCharPtrToHidlString((char *) response));
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("getBasebandVersionResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::separateConnectionResponse(int slotId,
                                     int responseType, int serial, RIL_Errno e,
                                     void *response, size_t responseLen) {
#if VDBG
    RLOGD("separateConnectionResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->separateConnectionResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("separateConnectionResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::getDataCallListResponse(int slotId,
                                   int responseType, int serial, RIL_Errno e,
                                   void *response, size_t responseLen) {
#if VDBG
    RLOGD("getDataCallListResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);

        hidl_vec<SetupDataCallResult> ret;
        if ((response == NULL && responseLen != 0)
                || responseLen % sizeof(RIL_Data_Call_Response_v11) != 0) {
            RLOGE("getDataCallListResponse: invalid response");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            convertRilDataCallListToHal(response, responseLen, ret);
        }

        Return<void> retStatus = radioService[slotId]->mRadioResponse->getDataCallListResponse(
                responseInfo, ret);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("getDataCallListResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

#if 0
int radio::setSuppServiceNotificationsResponse(int slotId,
                                              int responseType, int serial, RIL_Errno e,
                                              void *response, size_t responseLen) {
#if VDBG
    RLOGD("setSuppServiceNotificationsResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->setSuppServiceNotificationsResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("setSuppServiceNotificationsResponse: radioService[%d]->mRadioResponse "
                "== NULL", slotId);
    }

    return 0;
}
#endif

#if 0
int radio::deleteSmsOnSimResponse(int slotId,
                                 int responseType, int serial, RIL_Errno e,
                                 void *response, size_t responseLen) {
#if VDBG
    RLOGD("deleteSmsOnSimResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->deleteSmsOnSimResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("deleteSmsOnSimResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}
#endif

int radio::setBandModeResponse(int slotId,
                              int responseType, int serial, RIL_Errno e,
                              void *response, size_t responseLen) {
#if VDBG
    RLOGD("setBandModeResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->setBandModeResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("setBandModeResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

#if 0
int radio::writeSmsToSimResponse(int slotId,
                                int responseType, int serial, RIL_Errno e,
                                void *response, size_t responseLen) {
#if VDBG
    RLOGD("writeSmsToSimResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        int ret = responseInt(responseInfo, serial, responseType, e, response, responseLen);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->writeSmsToSimResponse(responseInfo, ret);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("writeSmsToSimResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}
#endif

int radio::getAvailableBandModesResponse(int slotId,
                                        int responseType, int serial, RIL_Errno e, void *response,
                                        size_t responseLen) {
#if VDBG
    RLOGD("getAvailableBandModesResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        hidl_vec<RadioBandMode> modes;
        if ((response == NULL && responseLen != 0)|| responseLen % sizeof(int) != 0) {
            RLOGE("getAvailableBandModesResponse Invalid response: NULL");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            int *pInt = (int *) response;
            int numInts = responseLen / sizeof(int);
            modes.resize(numInts);
            for (int i = 0; i < numInts; i++) {
                modes[i] = (RadioBandMode) pInt[i];
            }
        }
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->getAvailableBandModesResponse(responseInfo,
                modes);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("getAvailableBandModesResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::explicitCallTransferResponse(int slotId,
                                       int responseType, int serial, RIL_Errno e,
                                       void *response, size_t responseLen) {
#if VDBG
    RLOGD("explicitCallTransferResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->explicitCallTransferResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("explicitCallTransferResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::setPreferredNetworkTypeResponse(int slotId,
                                 int responseType, int serial, RIL_Errno e,
                                 void *response, size_t responseLen) {
#if VDBG
    RLOGD("setPreferredNetworkTypeResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->setPreferredNetworkTypeResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("setPreferredNetworkTypeResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}


int radio::getPreferredNetworkTypeResponse(int slotId,
                                          int responseType, int serial, RIL_Errno e,
                                          void *response, size_t responseLen) {
#if VDBG
    RLOGD("getPreferredNetworkTypeResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        int ret = responseInt(responseInfo, serial, responseType, e, response, responseLen);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->getPreferredNetworkTypeResponse(
                responseInfo, (PreferredNetworkType) ret);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("getPreferredNetworkTypeResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::getNeighboringCidsResponse(int slotId,
                                     int responseType, int serial, RIL_Errno e,
                                     void *response, size_t responseLen) {
#if VDBG
    RLOGD("getNeighboringCidsResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        hidl_vec<NeighboringCell> cells;

        if ((response == NULL && responseLen != 0)
                || responseLen % sizeof(RIL_NeighboringCell *) != 0) {
            RLOGE("getNeighboringCidsResponse Invalid response: NULL");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            int num = responseLen / sizeof(RIL_NeighboringCell *);
            cells.resize(num);
            for (int i = 0 ; i < num; i++) {
                RIL_NeighboringCell *resp = ((RIL_NeighboringCell **) response)[i];
                cells[i].cid = convertCharPtrToHidlString(resp->cid);
                cells[i].rssi = resp->rssi;
            }
        }

        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->getNeighboringCidsResponse(responseInfo,
                cells);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("getNeighboringCidsResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::setLocationUpdatesResponse(int slotId,
                                     int responseType, int serial, RIL_Errno e,
                                     void *response, size_t responseLen) {
#if VDBG
    RLOGD("setLocationUpdatesResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->setLocationUpdatesResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("setLocationUpdatesResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::setCdmaSubscriptionSourceResponse(int slotId,
                                 int responseType, int serial, RIL_Errno e,
                                 void *response, size_t responseLen) {
#if VDBG
    RLOGD("setCdmaSubscriptionSourceResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->setCdmaSubscriptionSourceResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("setCdmaSubscriptionSourceResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::setCdmaRoamingPreferenceResponse(int slotId,
                                 int responseType, int serial, RIL_Errno e,
                                 void *response, size_t responseLen) {
#if VDBG
    RLOGD("setCdmaRoamingPreferenceResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->setCdmaRoamingPreferenceResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("setCdmaRoamingPreferenceResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::setTTYModeResponse(int slotId,
                             int responseType, int serial, RIL_Errno e,
                             void *response, size_t responseLen) {
#if VDBG
    RLOGD("setTTYModeResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->setTTYModeResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("setTTYModeResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::getTTYModeResponse(int slotId,
                             int responseType, int serial, RIL_Errno e,
                             void *response, size_t responseLen) {
#if VDBG
    RLOGD("getTTYModeResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        int ret = responseInt(responseInfo, serial, responseType, e, response, responseLen);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->getTTYModeResponse(responseInfo,
                (TtyMode) ret);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("getTTYModeResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::setPreferredVoicePrivacyResponse(int slotId,
                                 int responseType, int serial, RIL_Errno e,
                                 void *response, size_t responseLen) {
#if VDBG
    RLOGD("setPreferredVoicePrivacyResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->setPreferredVoicePrivacyResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("setPreferredVoicePrivacyResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::getPreferredVoicePrivacyResponse(int slotId,
                                           int responseType, int serial, RIL_Errno e,
                                           void *response, size_t responseLen) {
#if VDBG
    RLOGD("getPreferredVoicePrivacyResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        bool enable = false;
        int numInts = responseLen / sizeof(int);
        if (response == NULL || numInts != 1) {
            RLOGE("getPreferredVoicePrivacyResponse Invalid response: NULL");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            int *pInt = (int *) response;
            enable = pInt[0] == 1 ? true : false;
        }
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->getPreferredVoicePrivacyResponse(
                responseInfo, enable);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("getPreferredVoicePrivacyResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::sendCDMAFeatureCodeResponse(int slotId,
                                 int responseType, int serial, RIL_Errno e,
                                 void *response, size_t responseLen) {
#if VDBG
    RLOGD("sendCDMAFeatureCodeResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->sendCDMAFeatureCodeResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("sendCDMAFeatureCodeResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::sendBurstDtmfResponse(int slotId,
                                 int responseType, int serial, RIL_Errno e,
                                 void *response, size_t responseLen) {
#if VDBG
    RLOGD("sendBurstDtmfResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->sendBurstDtmfResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("sendBurstDtmfResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::sendCdmaSmsResponse(int slotId,
                              int responseType, int serial, RIL_Errno e, void *response,
                              size_t responseLen) {
#if VDBG
    RLOGD("sendCdmaSmsResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        SendSmsResult result = makeSendSmsResult(responseInfo, serial, responseType, e, response,
                responseLen);

        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->sendCdmaSmsResponse(responseInfo, result);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("sendCdmaSmsResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

#if 0
int radio::acknowledgeLastIncomingCdmaSmsResponse(int slotId,
                                                 int responseType, int serial, RIL_Errno e,
                                                 void *response, size_t responseLen) {
#if VDBG
    RLOGD("acknowledgeLastIncomingCdmaSmsResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->acknowledgeLastIncomingCdmaSmsResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("acknowledgeLastIncomingCdmaSmsResponse: radioService[%d]->mRadioResponse "
                "== NULL", slotId);
    }

    return 0;
}

int radio::getGsmBroadcastConfigResponse(int slotId,
                                        int responseType, int serial, RIL_Errno e,
                                        void *response, size_t responseLen) {
#if VDBG
    RLOGD("getGsmBroadcastConfigResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        hidl_vec<GsmBroadcastSmsConfigInfo> configs;

        if ((response == NULL && responseLen != 0)
                || responseLen % sizeof(RIL_GSM_BroadcastSmsConfigInfo *) != 0) {
            RLOGE("getGsmBroadcastConfigResponse Invalid response: NULL");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            int num = responseLen / sizeof(RIL_GSM_BroadcastSmsConfigInfo *);
            configs.resize(num);
            for (int i = 0 ; i < num; i++) {
                RIL_GSM_BroadcastSmsConfigInfo *resp =
                        ((RIL_GSM_BroadcastSmsConfigInfo **) response)[i];
                configs[i].fromServiceId = resp->fromServiceId;
                configs[i].toServiceId = resp->toServiceId;
                configs[i].fromCodeScheme = resp->fromCodeScheme;
                configs[i].toCodeScheme = resp->toCodeScheme;
                configs[i].selected = resp->selected == 1 ? true : false;
            }
        }

        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->getGsmBroadcastConfigResponse(responseInfo,
                configs);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("getGsmBroadcastConfigResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::setGsmBroadcastConfigResponse(int slotId,
                                        int responseType, int serial, RIL_Errno e,
                                        void *response, size_t responseLen) {
#if VDBG
    RLOGD("setGsmBroadcastConfigResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->setGsmBroadcastConfigResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("setGsmBroadcastConfigResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::setGsmBroadcastActivationResponse(int slotId,
                                            int responseType, int serial, RIL_Errno e,
                                            void *response, size_t responseLen) {
#if VDBG
    RLOGD("setGsmBroadcastActivationResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->setGsmBroadcastActivationResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("setGsmBroadcastActivationResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::getCdmaBroadcastConfigResponse(int slotId,
                                         int responseType, int serial, RIL_Errno e,
                                         void *response, size_t responseLen) {
#if VDBG
    RLOGD("getCdmaBroadcastConfigResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        hidl_vec<CdmaBroadcastSmsConfigInfo> configs;

        if ((response == NULL && responseLen != 0)
                || responseLen % sizeof(RIL_CDMA_BroadcastSmsConfigInfo *) != 0) {
            RLOGE("getCdmaBroadcastConfigResponse Invalid response: NULL");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            int num = responseLen / sizeof(RIL_CDMA_BroadcastSmsConfigInfo *);
            configs.resize(num);
            for (int i = 0 ; i < num; i++) {
                RIL_CDMA_BroadcastSmsConfigInfo *resp =
                        ((RIL_CDMA_BroadcastSmsConfigInfo **) response)[i];
                configs[i].serviceCategory = resp->service_category;
                configs[i].language = resp->language;
                configs[i].selected = resp->selected == 1 ? true : false;
            }
        }

        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->getCdmaBroadcastConfigResponse(responseInfo,
                configs);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("getCdmaBroadcastConfigResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::setCdmaBroadcastConfigResponse(int slotId,
                                         int responseType, int serial, RIL_Errno e,
                                         void *response, size_t responseLen) {
#if VDBG
    RLOGD("setCdmaBroadcastConfigResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->setCdmaBroadcastConfigResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("setCdmaBroadcastConfigResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}
#endif

#if 0
int radio::setCdmaBroadcastActivationResponse(int slotId,
                                             int responseType, int serial, RIL_Errno e,
                                             void *response, size_t responseLen) {
#if VDBG
    RLOGD("setCdmaBroadcastActivationResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->setCdmaBroadcastActivationResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("setCdmaBroadcastActivationResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}
#endif

int radio::getCDMASubscriptionResponse(int slotId,
                                      int responseType, int serial, RIL_Errno e, void *response,
                                      size_t responseLen) {
#if VDBG
    RLOGD("getCDMASubscriptionResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);

        int numStrings = responseLen / sizeof(char *);
        hidl_string emptyString;
        if (response == NULL || numStrings != 5) {
            RLOGE("getOperatorResponse Invalid response: NULL");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
            Return<void> retStatus
                    = radioService[slotId]->mRadioResponse->getCDMASubscriptionResponse(
                    responseInfo, emptyString, emptyString, emptyString, emptyString, emptyString);
            radioService[slotId]->checkReturnStatus(retStatus);
        } else {
            char **resp = (char **) response;
            Return<void> retStatus
                    = radioService[slotId]->mRadioResponse->getCDMASubscriptionResponse(
                    responseInfo,
                    convertCharPtrToHidlString(resp[0]),
                    convertCharPtrToHidlString(resp[1]),
                    convertCharPtrToHidlString(resp[2]),
                    convertCharPtrToHidlString(resp[3]),
                    convertCharPtrToHidlString(resp[4]));
            radioService[slotId]->checkReturnStatus(retStatus);
        }
    } else {
        RLOGE("getCDMASubscriptionResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

#if 0
int radio::writeSmsToRuimResponse(int slotId,
                                 int responseType, int serial, RIL_Errno e,
                                 void *response, size_t responseLen) {
#if VDBG
    RLOGD("writeSmsToRuimResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        int ret = responseInt(responseInfo, serial, responseType, e, response, responseLen);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->writeSmsToRuimResponse(responseInfo, ret);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("writeSmsToRuimResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::deleteSmsOnRuimResponse(int slotId,
                                  int responseType, int serial, RIL_Errno e,
                                  void *response, size_t responseLen) {
#if VDBG
    RLOGD("deleteSmsOnRuimResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->deleteSmsOnRuimResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("deleteSmsOnRuimResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}
#endif

int radio::getDeviceIdentityResponse(int slotId,
                                    int responseType, int serial, RIL_Errno e, void *response,
                                    size_t responseLen) {
#if VDBG
    RLOGD("getDeviceIdentityResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);

        int numStrings = responseLen / sizeof(char *);
        hidl_string emptyString;
        if (response == NULL || numStrings != 4) {
            RLOGE("getDeviceIdentityResponse Invalid response: NULL");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
            Return<void> retStatus
                    = radioService[slotId]->mRadioResponse->getDeviceIdentityResponse(responseInfo,
                    emptyString, emptyString, emptyString, emptyString);
            radioService[slotId]->checkReturnStatus(retStatus);
        } else {
            char **resp = (char **) response;
            Return<void> retStatus
                    = radioService[slotId]->mRadioResponse->getDeviceIdentityResponse(responseInfo,
                    convertCharPtrToHidlString(resp[0]),
                    convertCharPtrToHidlString(resp[1]),
                    convertCharPtrToHidlString(resp[2]),
                    convertCharPtrToHidlString(resp[3]));
            radioService[slotId]->checkReturnStatus(retStatus);
        }
    } else {
        RLOGE("getDeviceIdentityResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::exitEmergencyCallbackModeResponse(int slotId,
                                            int responseType, int serial, RIL_Errno e,
                                            void *response, size_t responseLen) {
#if VDBG
    RLOGD("exitEmergencyCallbackModeResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->exitEmergencyCallbackModeResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("exitEmergencyCallbackModeResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

#if 0
int radio::getSmscAddressResponse(int slotId,
                                  int responseType, int serial, RIL_Errno e,
                                  void *response, size_t responseLen) {
#if VDBG
    RLOGD("getSmscAddressResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->getSmscAddressResponse(responseInfo,
                convertCharPtrToHidlString((char *) response));
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("getSmscAddressResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}
#endif

#if 0
int radio::setSmscAddressResponse(int slotId,
                                             int responseType, int serial, RIL_Errno e,
                                             void *response, size_t responseLen) {
#if VDBG
    RLOGD("setSmscAddressResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->setSmscAddressResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("setSmscAddressResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}


int radio::reportSmsMemoryStatusResponse(int slotId,
                                        int responseType, int serial, RIL_Errno e,
                                        void *response, size_t responseLen) {
#if VDBG
    RLOGD("reportSmsMemoryStatusResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->reportSmsMemoryStatusResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("reportSmsMemoryStatusResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}
#endif

int radio::acknowledgeIncomingGsmSmsWithPduResponse(int slotId,
                                                   int responseType,
                                                   int serial, RIL_Errno e, void *response,
                                                   size_t responseLen) {
#if VDBG
    RLOGD("acknowledgeIncomingGsmSmsWithPduResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->acknowledgeIncomingGsmSmsWithPduResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("acknowledgeIncomingGsmSmsWithPduResponse: radioService[%d]->mRadioResponse "
                "== NULL", slotId);
    }

    return 0;
}


int radio::getVoiceRadioTechnologyResponse(int slotId,
                                          int responseType, int serial, RIL_Errno e,
                                          void *response, size_t responseLen) {
#if VDBG
    RLOGD("getVoiceRadioTechnologyResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        int ret = responseInt(responseInfo, serial, responseType, e, response, responseLen);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->getVoiceRadioTechnologyResponse(
                responseInfo, (RadioTechnology) ret);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("getVoiceRadioTechnologyResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int RadioImpl::sendGetCellInfoListResponse(int slotId,
                                   int responseType, int serial, RIL_Errno e,
                                   void *response, size_t responseLen) {
#if VDBG
    RLOGD("getCellInfoListResponse: serial %d", serial);
#endif

    auto rsp = mRadioResponse;
    if (rsp != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);

        hidl_vec<CellInfo> ret;
        if ((response == NULL && responseLen != 0)
                || responseLen % sizeof(RIL_CellInfo_v12) != 0) {
            RLOGE("getCellInfoListResponse: Invalid response");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            convertRilCellInfoListToHal(slotId, response, responseLen, ret);
        }

        Return<void> retStatus;
        retStatus = rsp->getCellInfoListResponse(
                responseInfo, ret);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("getCellInfoListResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::getCellInfoListResponse(int slotId,
                                   int responseType,
                                   int serial, RIL_Errno e, void *response,
                                   size_t responseLen) {
    auto srvc = radioService[slotId];
    if (srvc) {
        return srvc->sendGetCellInfoListResponse(slotId,
                responseType,
                serial,
                e,
                response,
                responseLen);
    } else {
        RLOGE("radioService[slotId = %d] is null!", slotId);
    }
    return 1;
}

int radio::setInitialAttachApnResponse(int slotId,
                                       int responseType, int serial, RIL_Errno e,
                                       void *response, size_t responseLen) {
#if VDBG
    RLOGD("setInitialAttachApnResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->setInitialAttachApnResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("setInitialAttachApnResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

#if 0
int radio::getImsRegistrationStateResponse(int slotId,
                                           int responseType, int serial, RIL_Errno e,
                                           void *response, size_t responseLen) {
#if VDBG
    RLOGD("getImsRegistrationStateResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        bool isRegistered = false;
        int ratFamily = 0;
        int numInts = responseLen / sizeof(int);
        if (response == NULL || numInts != 2) {
            RLOGE("getImsRegistrationStateResponse Invalid response: NULL");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            int *pInt = (int *) response;
            isRegistered = pInt[0] == 1 ? true : false;
            ratFamily = pInt[1];
        }
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->getImsRegistrationStateResponse(
                responseInfo, isRegistered, (RadioTechnologyFamily) ratFamily);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("getImsRegistrationStateResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}
#endif

int radio::sendImsSmsResponse(int slotId,
                              int responseType, int serial, RIL_Errno e, void *response,
                              size_t responseLen) {
#if VDBG
    RLOGD("sendImsSmsResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        SendSmsResult result = makeSendSmsResult(responseInfo, serial, responseType, e, response,
                responseLen);

        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->sendImsSmsResponse(responseInfo, result);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("sendSmsResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::nvReadItemResponse(int slotId,
                              int responseType, int serial, RIL_Errno e,
                              void *response, size_t responseLen) {
#if VDBG
    RLOGD("nvReadItemResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = radioService[slotId]->mRadioResponse->nvReadItemResponse(
                responseInfo,
                convertCharPtrToHidlString((char *) response));
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("nvReadItemResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::nvWriteItemResponse(int slotId,
                               int responseType, int serial, RIL_Errno e,
                               void *response, size_t responseLen) {
#if VDBG
    RLOGD("nvWriteItemResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->nvWriteItemResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("nvWriteItemResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::nvWriteCdmaPrlResponse(int slotId,
                                  int responseType, int serial, RIL_Errno e,
                                  void *response, size_t responseLen) {
#if VDBG
    RLOGD("nvWriteCdmaPrlResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->nvWriteCdmaPrlResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("nvWriteCdmaPrlResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::nvResetConfigResponse(int slotId,
                                 int responseType, int serial, RIL_Errno e,
                                 void *response, size_t responseLen) {
#if VDBG
    RLOGD("nvResetConfigResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->nvResetConfigResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("nvResetConfigResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::setUiccSubscriptionResponse(int slotId,
                                       int responseType, int serial, RIL_Errno e,
                                       void *response, size_t responseLen) {
#if VDBG
    RLOGD("setUiccSubscriptionResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->setUiccSubscriptionResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("setUiccSubscriptionResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::setDataAllowedResponse(int slotId,
                                  int responseType, int serial, RIL_Errno e,
                                  void *response, size_t responseLen) {
#if VDBG
    RLOGD("setDataAllowedResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->setDataAllowedResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("setDataAllowedResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::getHardwareConfigResponse(int slotId,
                                     int responseType, int serial, RIL_Errno e,
                                     void *response, size_t responseLen) {
#if VDBG
    RLOGD("getHardwareConfigResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);

        hidl_vec<HardwareConfig> result;
        if ((response == NULL && responseLen != 0)
                || responseLen % sizeof(RIL_HardwareConfig) != 0) {
            RLOGE("hardwareConfigChangedInd: invalid response");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            convertRilHardwareConfigListToHal(response, responseLen, result);
        }

        Return<void> retStatus = radioService[slotId]->mRadioResponse->getHardwareConfigResponse(
                responseInfo, result);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("getHardwareConfigResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::setDataProfileResponse(int slotId,
                                  int responseType, int serial, RIL_Errno e,
                                  void *response, size_t responseLen) {
#if VDBG
    RLOGD("setDataProfileResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->setDataProfileResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("setDataProfileResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::requestShutdownResponse(int slotId,
                                  int responseType, int serial, RIL_Errno e,
                                  void *response, size_t responseLen) {
#if VDBG
    RLOGD("requestShutdownResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->requestShutdownResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("requestShutdownResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

void responseRadioCapability(RadioResponseInfo& responseInfo, int serial,
        int responseType, RIL_Errno e, void *response, size_t responseLen, RadioCapability& rc) {
    populateResponseInfo(responseInfo, serial, responseType, e);

    if (response == NULL || responseLen != sizeof(RIL_RadioCapability)) {
        RLOGE("responseRadioCapability: Invalid response");
        if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        rc.logicalModemUuid = hidl_string();
    } else {
        convertRilRadioCapabilityToHal(response, responseLen, rc);
    }
}

int radio::getRadioCapabilityResponse(int slotId,
                                     int responseType, int serial, RIL_Errno e,
                                     void *response, size_t responseLen) {
#if VDBG
    RLOGD("getRadioCapabilityResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        RadioCapability result = {};
        responseRadioCapability(responseInfo, serial, responseType, e, response, responseLen,
                result);
        Return<void> retStatus = radioService[slotId]->mRadioResponse->getRadioCapabilityResponse(
                responseInfo, result);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("getRadioCapabilityResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::setRadioCapabilityResponse(int slotId,
                                     int responseType, int serial, RIL_Errno e,
                                     void *response, size_t responseLen) {
#if VDBG
    RLOGD("setRadioCapabilityResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        RadioCapability result = {};
        responseRadioCapability(responseInfo, serial, responseType, e, response, responseLen,
                result);
        Return<void> retStatus = radioService[slotId]->mRadioResponse->setRadioCapabilityResponse(
                responseInfo, result);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("setRadioCapabilityResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

LceStatusInfo responseLceStatusInfo(RadioResponseInfo& responseInfo, int serial, int responseType,
                                    RIL_Errno e, void *response, size_t responseLen) {
    populateResponseInfo(responseInfo, serial, responseType, e);
    LceStatusInfo result = {};

    if (response == NULL || responseLen != sizeof(RIL_LceStatusInfo)) {
        RLOGE("Invalid response: NULL");
        if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
    } else {
        RIL_LceStatusInfo *resp = (RIL_LceStatusInfo *) response;
        result.lceStatus = (LceStatus) resp->lce_status;
        result.actualIntervalMs = (uint8_t) resp->actual_interval_ms;
    }
    return result;
}

int radio::startLceServiceResponse(int slotId,
                                   int responseType, int serial, RIL_Errno e,
                                   void *response, size_t responseLen) {
#if VDBG
    RLOGD("startLceServiceResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        LceStatusInfo result = responseLceStatusInfo(responseInfo, serial, responseType, e,
                response, responseLen);

        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->startLceServiceResponse(responseInfo,
                result);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("startLceServiceResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::stopLceServiceResponse(int slotId,
                                  int responseType, int serial, RIL_Errno e,
                                  void *response, size_t responseLen) {
#if VDBG
    RLOGD("stopLceServiceResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        LceStatusInfo result = responseLceStatusInfo(responseInfo, serial, responseType, e,
                response, responseLen);

        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->stopLceServiceResponse(responseInfo,
                result);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("stopLceServiceResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::pullLceDataResponse(int slotId,
                               int responseType, int serial, RIL_Errno e,
                               void *response, size_t responseLen) {
#if VDBG
    RLOGD("pullLceDataResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);

        LceDataInfo result = {};
        if (response == NULL || responseLen != sizeof(RIL_LceDataInfo)) {
            RLOGE("pullLceDataResponse: Invalid response");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            convertRilLceDataInfoToHal(response, responseLen, result);
        }

        Return<void> retStatus = radioService[slotId]->mRadioResponse->pullLceDataResponse(
                responseInfo, result);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("pullLceDataResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::getModemActivityInfoResponse(int slotId,
                                        int responseType, int serial, RIL_Errno e,
                                        void *response, size_t responseLen) {
#if VDBG
    RLOGD("getModemActivityInfoResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        ActivityStatsInfo info;
        if (response == NULL || responseLen != sizeof(RIL_ActivityStatsInfo)) {
            RLOGE("getModemActivityInfoResponse Invalid response: NULL");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            RIL_ActivityStatsInfo *resp = (RIL_ActivityStatsInfo *)response;
            info.sleepModeTimeMs = resp->sleep_mode_time_ms;
            info.idleModeTimeMs = resp->idle_mode_time_ms;
            for(int i = 0; i < RIL_NUM_TX_POWER_LEVELS; i++) {
                info.txmModetimeMs[i] = resp->tx_mode_time_ms[i];
            }
            info.rxModeTimeMs = resp->rx_mode_time_ms;
        }

        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->getModemActivityInfoResponse(responseInfo,
                info);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("getModemActivityInfoResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::setAllowedCarriersResponse(int slotId,
                                      int responseType, int serial, RIL_Errno e,
                                      void *response, size_t responseLen) {
#if VDBG
    RLOGD("setAllowedCarriersResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        int ret = responseInt(responseInfo, serial, responseType, e, response, responseLen);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->setAllowedCarriersResponse(responseInfo,
                ret);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("setAllowedCarriersResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::getAllowedCarriersResponse(int slotId,
                                      int responseType, int serial, RIL_Errno e,
                                      void *response, size_t responseLen) {
#if VDBG
    RLOGD("getAllowedCarriersResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        CarrierRestrictions carrierInfo = {};
        bool allAllowed = true;
        if (response == NULL) {
#if VDBG
            RLOGD("getAllowedCarriersResponse response is NULL: all allowed");
#endif
            carrierInfo.allowedCarriers.resize(0);
            carrierInfo.excludedCarriers.resize(0);
        } else if (responseLen != sizeof(RIL_CarrierRestrictions)) {
            RLOGE("getAllowedCarriersResponse Invalid response");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            RIL_CarrierRestrictions *pCr = (RIL_CarrierRestrictions *)response;
            if (pCr->len_allowed_carriers > 0 || pCr->len_excluded_carriers > 0) {
                allAllowed = false;
            }

            carrierInfo.allowedCarriers.resize(pCr->len_allowed_carriers);
            for(int i = 0; i < pCr->len_allowed_carriers; i++) {
                RIL_Carrier *carrier = pCr->allowed_carriers + i;
                carrierInfo.allowedCarriers[i].mcc = convertCharPtrToHidlString(carrier->mcc);
                carrierInfo.allowedCarriers[i].mnc = convertCharPtrToHidlString(carrier->mnc);
                carrierInfo.allowedCarriers[i].matchType = (CarrierMatchType) carrier->match_type;
                carrierInfo.allowedCarriers[i].matchData =
                        convertCharPtrToHidlString(carrier->match_data);
            }

            carrierInfo.excludedCarriers.resize(pCr->len_excluded_carriers);
            for(int i = 0; i < pCr->len_excluded_carriers; i++) {
                RIL_Carrier *carrier = pCr->excluded_carriers + i;
                carrierInfo.excludedCarriers[i].mcc = convertCharPtrToHidlString(carrier->mcc);
                carrierInfo.excludedCarriers[i].mnc = convertCharPtrToHidlString(carrier->mnc);
                carrierInfo.excludedCarriers[i].matchType = (CarrierMatchType) carrier->match_type;
                carrierInfo.excludedCarriers[i].matchData =
                        convertCharPtrToHidlString(carrier->match_data);
            }
        }

        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->getAllowedCarriersResponse(responseInfo,
                allAllowed, carrierInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("getAllowedCarriersResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::sendDeviceStateResponse(int slotId,
                              int responseType, int serial, RIL_Errno e,
                              void *response, size_t responselen) {
#if VDBG
    RLOGD("sendDeviceStateResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->sendDeviceStateResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("sendDeviceStateResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::setCarrierInfoForImsiEncryptionResponse(int slotId,
                               int responseType, int serial, RIL_Errno e,
                               void *response, size_t responseLen) {
    RLOGD("setCarrierInfoForImsiEncryptionResponse: serial %d", serial);
    if (radioService[slotId]->mRadioResponseV1_1 != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = radioService[slotId]->mRadioResponseV1_1->
                setCarrierInfoForImsiEncryptionResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("setCarrierInfoForImsiEncryptionResponse: radioService[%d]->mRadioResponseV1_1 == "
                "NULL", slotId);
    }
    return 0;
}

int radio::setIndicationFilterResponse(int slotId,
                              int responseType, int serial, RIL_Errno e,
                              void *response, size_t responselen) {
    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->setIndicationFilterResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("setIndicationFilterResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::startNetworkScanResponse(int slotId, int responseType, int serial, RIL_Errno e,
                                    void *response, size_t responseLen) {
#if VDBG
    RLOGD("startNetworkScanResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponseV1_1 != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponseV1_1->startNetworkScanResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("startNetworkScanResponse: radioService[%d]->mRadioResponseV1_1 == NULL", slotId);
    }

    return 0;
}

int radio::stopNetworkScanResponse(int slotId, int responseType, int serial, RIL_Errno e,
                                   void *response, size_t responseLen) {
#if VDBG
    RLOGD("stopNetworkScanResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponseV1_1 != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponseV1_1->stopNetworkScanResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("stopNetworkScanResponse: radioService[%d]->mRadioResponseV1_1 == NULL", slotId);
    }

    return 0;
}

void convertRilKeepaliveStatusToHal(const RIL_KeepaliveStatus *rilStatus,
        V1_1::KeepaliveStatus& halStatus) {
    halStatus.sessionHandle = rilStatus->sessionHandle;
    halStatus.code = static_cast<V1_1::KeepaliveStatusCode>(rilStatus->code);
}

int radio::startKeepaliveResponse(int slotId, int responseType, int serial, RIL_Errno e,
                                    void *response, size_t responseLen) {
#if VDBG
    RLOGD("%s(): %d", __FUNCTION__, serial);
#endif
    RadioResponseInfo responseInfo = {};
    populateResponseInfo(responseInfo, serial, responseType, e);

    // If we don't have a radio service, there's nothing we can do
    if (radioService[slotId]->mRadioResponseV1_1 == NULL) {
        RLOGE("%s: radioService[%d]->mRadioResponseV1_1 == NULL", __FUNCTION__, slotId);
        return 0;
    }

    V1_1::KeepaliveStatus ks = {};
    if (response == NULL || responseLen != sizeof(V1_1::KeepaliveStatus)) {
        RLOGE("%s: invalid response - %d", __FUNCTION__, static_cast<int>(e));
        if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
    } else {
        convertRilKeepaliveStatusToHal(static_cast<RIL_KeepaliveStatus*>(response), ks);
    }

    Return<void> retStatus =
            radioService[slotId]->mRadioResponseV1_1->startKeepaliveResponse(responseInfo, ks);
    radioService[slotId]->checkReturnStatus(retStatus);
    return 0;
}

int radio::stopKeepaliveResponse(int slotId, int responseType, int serial, RIL_Errno e,
                                    void *response, size_t responseLen) {
#if VDBG
    RLOGD("%s(): %d", __FUNCTION__, serial);
#endif
    RadioResponseInfo responseInfo = {};
    populateResponseInfo(responseInfo, serial, responseType, e);

    // If we don't have a radio service, there's nothing we can do
    if (radioService[slotId]->mRadioResponseV1_1 == NULL) {
        RLOGE("%s: radioService[%d]->mRadioResponseV1_1 == NULL", __FUNCTION__, slotId);
        return 0;
    }

    Return<void> retStatus =
            radioService[slotId]->mRadioResponseV1_1->stopKeepaliveResponse(responseInfo);
    radioService[slotId]->checkReturnStatus(retStatus);
    return 0;
}

int radio::sendRequestRawResponse(int slotId,
                                  int responseType, int serial, RIL_Errno e,
                                  void *response, size_t responseLen) {
#if VDBG
   RLOGD("sendRequestRawResponse: serial %d", serial);
#endif

    if (!kOemHookEnabled) return 0;

    if (oemHookService[slotId]->mOemHookResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        hidl_vec<uint8_t> data;

        if (response == NULL) {
            RLOGE("sendRequestRawResponse: Invalid response");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            data.setToExternal((uint8_t *) response, responseLen);
        }
        Return<void> retStatus = oemHookService[slotId]->mOemHookResponse->
                sendRequestRawResponse(responseInfo, data);
        checkReturnStatus(slotId, retStatus, false);
    } else {
        RLOGE("sendRequestRawResponse: oemHookService[%d]->mOemHookResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::sendRequestStringsResponse(int slotId,
                                      int responseType, int serial, RIL_Errno e,
                                      void *response, size_t responseLen) {
#if VDBG
    RLOGD("sendRequestStringsResponse: serial %d", serial);
#endif

    if (!kOemHookEnabled) return 0;

    if (oemHookService[slotId]->mOemHookResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        hidl_vec<hidl_string> data;

        if ((response == NULL && responseLen != 0) || responseLen % sizeof(char *) != 0) {
            RLOGE("sendRequestStringsResponse Invalid response: NULL");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            char **resp = (char **) response;
            int numStrings = responseLen / sizeof(char *);
            data.resize(numStrings);
            for (int i = 0; i < numStrings; i++) {
                data[i] = convertCharPtrToHidlString(resp[i]);
            }
        }
        Return<void> retStatus
                = oemHookService[slotId]->mOemHookResponse->sendRequestStringsResponse(
                responseInfo, data);
        checkReturnStatus(slotId, retStatus, false);
    } else {
        RLOGE("sendRequestStringsResponse: oemHookService[%d]->mOemHookResponse == "
                "NULL", slotId);
    }

    return 0;
}

/***************************************************************************************************
 * INDICATION FUNCTIONS
 * The below function handle unsolicited messages coming from the Radio
 * (messages for which there is no pending request)
 **************************************************************************************************/

RadioIndicationType convertIntToRadioIndicationType(int indicationType) {
    return indicationType == RESPONSE_UNSOLICITED ? (RadioIndicationType::UNSOLICITED) :
            (RadioIndicationType::UNSOLICITED_ACK_EXP);
}

int radio::radioStateChangedInd(int slotId,
                                 int indicationType, int token, RIL_Errno e, void *response,
                                 size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        RadioState radioState =
                (RadioState) CALL_ONSTATEREQUEST(slotId);
        RLOGD("radioStateChangedInd: radioState %d", radioState);
        Return<void> retStatus = radioService[slotId]->mRadioIndication->radioStateChanged(
                convertIntToRadioIndicationType(indicationType), radioState);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("radioStateChangedInd: radioService[%d]->mRadioIndication == NULL", slotId);
    }

    return 0;
}

int radio::callStateChangedInd(int slotId,
                               int indicationType, int token, RIL_Errno e, void *response,
                               size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
#if VDBG
        RLOGD("callStateChangedInd");
#endif
        Return<void> retStatus = radioService[slotId]->mRadioIndication->callStateChanged(
                convertIntToRadioIndicationType(indicationType));
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("callStateChangedInd: radioService[%d]->mRadioIndication == NULL", slotId);
    }

    return 0;
}

int radio::networkStateChangedInd(int slotId,
                                  int indicationType, int token, RIL_Errno e, void *response,
                                  size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
#if VDBG
        RLOGD("networkStateChangedInd");
#endif
        Return<void> retStatus = radioService[slotId]->mRadioIndication->networkStateChanged(
                convertIntToRadioIndicationType(indicationType));
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("networkStateChangedInd: radioService[%d]->mRadioIndication == NULL",
                slotId);
    }

    return 0;
}

int radio::newSmsInd(int slotId, int indicationType,
                     int token, RIL_Errno e, void *response, size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        if (response == NULL || responseLen == 0) {
            RLOGE("newSmsInd: invalid response");
            return 0;
        }

        hidl_vec<uint8_t> pdu;
        pdu.setToExternal(static_cast<uint8_t*>(response), responseLen);
#if VDBG
        RLOGD("newSmsInd");
#endif
        Return<void> retStatus = radioService[slotId]->mRadioIndication->newSms(
                convertIntToRadioIndicationType(indicationType), pdu);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("newSmsInd: radioService[%d]->mRadioIndication == NULL", slotId);
    }

    return 0;
}

int radio::newSmsStatusReportInd(int slotId,
                                 int indicationType, int token, RIL_Errno e, void *response,
                                 size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        if (response == NULL || responseLen == 0) {
            RLOGE("newSmsStatusReportInd: invalid response");
            return 0;
        }

        hidl_vec<uint8_t> pdu;
        pdu.setToExternal(static_cast<uint8_t*>(response), responseLen);
#if VDBG
        RLOGD("newSmsStatusReportInd");
#endif
        Return<void> retStatus = radioService[slotId]->mRadioIndication->newSmsStatusReport(
                convertIntToRadioIndicationType(indicationType), pdu);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("newSmsStatusReportInd: radioService[%d]->mRadioIndication == NULL", slotId);
    }

    return 0;
}

int radio::newSmsOnSimInd(int slotId, int indicationType,
                          int token, RIL_Errno e, void *response, size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        if (response == NULL || responseLen != sizeof(int)) {
            RLOGE("newSmsOnSimInd: invalid response");
            return 0;
        }
        int32_t recordNumber = ((int32_t *) response)[0];
#if VDBG
        RLOGD("newSmsOnSimInd: slotIndex %d", recordNumber);
#endif
        Return<void> retStatus = radioService[slotId]->mRadioIndication->newSmsOnSim(
                convertIntToRadioIndicationType(indicationType), recordNumber);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("newSmsOnSimInd: radioService[%d]->mRadioIndication == NULL", slotId);
    }

    return 0;
}

int radio::onUssdInd(int slotId, int indicationType,
                     int token, RIL_Errno e, void *response, size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        if (response == NULL || responseLen != 2 * sizeof(char *)) {
            RLOGE("onUssdInd: invalid response");
            return 0;
        }
        char **strings = (char **) response;
        char *mode = strings[0];
        hidl_string msg = convertCharPtrToHidlString(strings[1]);
        UssdModeType modeType = (UssdModeType) atoi(mode);
#if VDBG
        RLOGD("onUssdInd: mode %s", mode);
#endif
        Return<void> retStatus = radioService[slotId]->mRadioIndication->onUssd(
                convertIntToRadioIndicationType(indicationType), modeType, msg);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("onUssdInd: radioService[%d]->mRadioIndication == NULL", slotId);
    }

    return 0;
}

int radio::nitzTimeReceivedInd(int slotId,
                               int indicationType, int token, RIL_Errno e, void *response,
                               size_t responseLen) {
#ifndef QMI_RIL_UTF // TODO: find a way to get elapsedRealtime in host compilation
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        if (response == NULL || responseLen == 0) {
            RLOGE("nitzTimeReceivedInd: invalid response");
            return 0;
        }
        hidl_string nitzTime = convertCharPtrToHidlString((char *) response);
        int64_t timeReceived = android::elapsedRealtime();
#if VDBG
        RLOGD("nitzTimeReceivedInd: nitzTime %s receivedTime %" PRId64, nitzTime.c_str(),
                timeReceived);
#endif
        Return<void> retStatus = radioService[slotId]->mRadioIndication->nitzTimeReceived(
                convertIntToRadioIndicationType(indicationType), nitzTime, timeReceived);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("nitzTimeReceivedInd: radioService[%d]->mRadioIndication == NULL", slotId);
        return -1;
    }
#endif
    return 0;
}

int convertToHal(GsmSignalStrength &out, const RIL_GW_SignalStrength &in) {
    out.signalStrength = in.signalStrength;
    out.bitErrorRate = in.bitErrorRate;
    return 0;
}

int convertToHal(GsmSignalStrength &out, const RIL_WCDMA_SignalStrength &in) {
    out.signalStrength = in.signalStrength;
    out.bitErrorRate = in.bitErrorRate;
    return 0;
}

int convertToHal(WcdmaSignalStrength &out, const RIL_GW_SignalStrength &in) {
    out.signalStrength = in.signalStrength;
    out.bitErrorRate = in.bitErrorRate;
    return 0;
}

int convertToHal(CdmaSignalStrength &out, const RIL_CDMA_SignalStrength &in) {
    out.dbm = in.dbm;
    out.ecio = in.ecio;
    return 0;
}

int convertToHal(EvdoSignalStrength &out, const RIL_EVDO_SignalStrength &in) {
    out.dbm = in.dbm;
    out.ecio = in.ecio;
    out.signalNoiseRatio = in.signalNoiseRatio;
    return 0;
}

int convertToHal(LteSignalStrength &out, RIL_LTE_SignalStrength_v8 in) {
    // Fixup LTE for backwards compatibility
    // signalStrength: -1 -> 99
    if (in.signalStrength == -1) {
        in.signalStrength = 99;
    }
    // rsrp: -1 -> INT_MAX all other negative value to positive.
    // So remap here
    if (in.rsrp == -1) {
        in.rsrp = INT_MAX;
    } else if (in.rsrp < -1) {
        in.rsrp = -in.rsrp;
    }
    // rsrq: -1 -> INT_MAX
    if (in.rsrq == -1) {
        in.rsrq = INT_MAX;
    }
    // Not remapping rssnr is already using INT_MAX
    // cqi: -1 -> INT_MAX
    if (in.cqi == -1) {
        in.cqi = INT_MAX;
    }

    out.signalStrength = in.signalStrength;
    out.rsrp = in.rsrp;
    out.rsrq = in.rsrq;
    out.rssnr = in.rssnr;
    out.cqi = in.cqi;
    out.timingAdvance = in.timingAdvance;
    return 0;
}

int convertToHal(TdScdmaSignalStrength &out, const RIL_TD_SCDMA_SignalStrength &in) {
    out.rscp = in.rscp;
    return 0;
}

int convertRilSignalStrengthToHal(const RIL_SignalStrength& rilSignalStrength,
        SignalStrength& signalStrength) {

    if (rilSignalStrength.WCDMA_SignalStrength.valid) {
        convertToHal(signalStrength.gw, rilSignalStrength.GW_SignalStrength);
    } else {
        convertToHal(signalStrength.gw, rilSignalStrength.WCDMA_SignalStrength);
    }
    convertToHal(signalStrength.cdma, rilSignalStrength.CDMA_SignalStrength);
    convertToHal(signalStrength.evdo, rilSignalStrength.EVDO_SignalStrength);
    convertToHal(signalStrength.lte, rilSignalStrength.LTE_SignalStrength);
    convertToHal(signalStrength.tdScdma, rilSignalStrength.TD_SCDMA_SignalStrength);
    return 0;
}

int convertRilSignalStrengthToHal(void *response, size_t responseLen,
        SignalStrength& signalStrength) {
    RIL_SignalStrength_v10 *rilSignalStrength = (RIL_SignalStrength_v10 *) response;

    if (response == NULL || responseLen != sizeof(RIL_SignalStrength_v10)) {
        RLOGE("invalid response");
        return 1;
    }
    if (!rilSignalStrength->WCDMA_SignalStrength.valid) {
        convertToHal(signalStrength.gw, rilSignalStrength->GW_SignalStrength);
    } else {
        convertToHal(signalStrength.gw, rilSignalStrength->WCDMA_SignalStrength);
    }
    convertToHal(signalStrength.cdma, rilSignalStrength->CDMA_SignalStrength);
    convertToHal(signalStrength.evdo, rilSignalStrength->EVDO_SignalStrength);
    convertToHal(signalStrength.lte, rilSignalStrength->LTE_SignalStrength);
    convertToHal(signalStrength.tdScdma, rilSignalStrength->TD_SCDMA_SignalStrength);
    return 0;
}

int RadioImpl::sendCurrentSignalStrengthInd(int slotId,
                                             int indicationType, int token, RIL_Errno e,
                                             void *response, size_t responseLen) {
    auto indication = mRadioIndication;
    if (indication != NULL) {
        if (response == NULL || responseLen != sizeof(RIL_SignalStrength_v10)) {
            RLOGE("invalid response");
            return 0;
        }
        SignalStrength signalStrength = {};
        convertRilSignalStrengthToHal(response, responseLen, signalStrength);

#if VDBG
        RLOGD("currentSignalStrengthInd");
#endif
        Return<void> retStatus = indication->currentSignalStrength(
                convertIntToRadioIndicationType(indicationType), signalStrength);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("currentSignalStrengthInd: radioService[%d]->mRadioIndication == NULL",
                slotId);
    }
    return 0;
}

int radio::currentSignalStrengthInd(int slotId,
                                    int indicationType, int token, RIL_Errno e,
                                    void *response, size_t responseLen) {
    int ret = 0;
    if (radioService[slotId] != NULL) {
        ret = radioService[slotId]->sendCurrentSignalStrengthInd(slotId,
                indicationType,
                token,
                e,
                response,
                responseLen);

    }

    return ret;
}

void convertRilDataCallToHal(RIL_Data_Call_Response_v11 *dcResponse,
        SetupDataCallResult& dcResult) {
    dcResult.status = (DataCallFailCause) dcResponse->status;
    dcResult.suggestedRetryTime = dcResponse->suggestedRetryTime;
    dcResult.cid = dcResponse->cid;
    dcResult.active = dcResponse->active;
    dcResult.type = convertCharPtrToHidlString(dcResponse->type);
    dcResult.ifname = convertCharPtrToHidlString(dcResponse->ifname);
    dcResult.addresses = convertCharPtrToHidlString(dcResponse->addresses);
    dcResult.dnses = convertCharPtrToHidlString(dcResponse->dnses);
    dcResult.gateways = convertCharPtrToHidlString(dcResponse->gateways);
    dcResult.pcscf = convertCharPtrToHidlString(dcResponse->pcscf);
    dcResult.mtu = dcResponse->mtu;
}

void convertRilDataCallListToHal(void *response, size_t responseLen,
        hidl_vec<SetupDataCallResult>& dcResultList) {
    int num = responseLen / sizeof(RIL_Data_Call_Response_v11);

    RIL_Data_Call_Response_v11 *dcResponse = (RIL_Data_Call_Response_v11 *) response;
    dcResultList.resize(num);
    for (int i = 0; i < num; i++) {
        convertRilDataCallToHal(&dcResponse[i], dcResultList[i]);
    }
}

int radio::dataCallListChangedInd(int slotId,
                                  int indicationType, int token, RIL_Errno e, void *response,
                                  size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        if ((response == NULL && responseLen != 0)
                || responseLen % sizeof(RIL_Data_Call_Response_v11) != 0) {
            RLOGE("dataCallListChangedInd: invalid response");
            return 0;
        }
        hidl_vec<SetupDataCallResult> dcList;
        convertRilDataCallListToHal(response, responseLen, dcList);
#if VDBG
        RLOGD("dataCallListChangedInd");
#endif
        Return<void> retStatus = radioService[slotId]->mRadioIndication->dataCallListChanged(
                convertIntToRadioIndicationType(indicationType), dcList);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("dataCallListChangedInd: radioService[%d]->mRadioIndication == NULL", slotId);
    }

    return 0;
}

int radio::suppSvcNotifyInd(int slotId, int indicationType,
                            int token, RIL_Errno e, void *response, size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        if (response == NULL || responseLen != sizeof(RIL_SuppSvcNotification)) {
            RLOGE("suppSvcNotifyInd: invalid response");
            return 0;
        }

        SuppSvcNotification suppSvc = {};
        RIL_SuppSvcNotification *ssn = (RIL_SuppSvcNotification *) response;
        suppSvc.isMT = ssn->notificationType;
        suppSvc.code = ssn->code;
        suppSvc.index = ssn->index;
        suppSvc.type = ssn->type;
        suppSvc.number = convertCharPtrToHidlString(ssn->number);

#if VDBG
        RLOGD("suppSvcNotifyInd: isMT %d code %d index %d type %d",
                suppSvc.isMT, suppSvc.code, suppSvc.index, suppSvc.type);
#endif
        Return<void> retStatus = radioService[slotId]->mRadioIndication->suppSvcNotify(
                convertIntToRadioIndicationType(indicationType), suppSvc);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("suppSvcNotifyInd: radioService[%d]->mRadioIndication == NULL", slotId);
    }

    return 0;
}

int radio::simSmsStorageFullInd(int slotId,
                                int indicationType, int token, RIL_Errno e, void *response,
                                size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
#if VDBG
        RLOGD("simSmsStorageFullInd");
#endif
        Return<void> retStatus = radioService[slotId]->mRadioIndication->simSmsStorageFull(
                convertIntToRadioIndicationType(indicationType));
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("simSmsStorageFullInd: radioService[%d]->mRadioIndication == NULL", slotId);
    }

    return 0;
}

void convertRilCdmaSignalInfoRecordToHal(RIL_CDMA_SignalInfoRecord *signalInfoRecord,
        CdmaSignalInfoRecord& record) {
    record.isPresent = signalInfoRecord->isPresent;
    record.signalType = signalInfoRecord->signalType;
    record.alertPitch = signalInfoRecord->alertPitch;
    record.signal = signalInfoRecord->signal;
}

int radio::callRingInd(int slotId, int indicationType,
                       int token, RIL_Errno e, void *response, size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        bool isGsm;
        CdmaSignalInfoRecord record = {};
        if (response == NULL || responseLen == 0) {
            isGsm = true;
        } else {
            isGsm = false;
            if (responseLen != sizeof (RIL_CDMA_SignalInfoRecord)) {
                RLOGE("callRingInd: invalid response");
                return 0;
            }
            convertRilCdmaSignalInfoRecordToHal((RIL_CDMA_SignalInfoRecord *) response, record);
        }

#if VDBG
        RLOGD("callRingInd: isGsm %d", isGsm);
#endif
        Return<void> retStatus = radioService[slotId]->mRadioIndication->callRing(
                convertIntToRadioIndicationType(indicationType), isGsm, record);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("callRingInd: radioService[%d]->mRadioIndication == NULL", slotId);
    }

    return 0;
}

int radio::cdmaNewSmsInd(int slotId, int indicationType,
                         int token, RIL_Errno e, void *response, size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        if (response == NULL || responseLen != sizeof(RIL_CDMA_SMS_Message)) {
            RLOGE("cdmaNewSmsInd: invalid response");
            return 0;
        }

        CdmaSmsMessage msg = {};
        RIL_CDMA_SMS_Message *rilMsg = (RIL_CDMA_SMS_Message *) response;
        msg.teleserviceId = rilMsg->uTeleserviceID;
        msg.isServicePresent = rilMsg->bIsServicePresent;
        msg.serviceCategory = rilMsg->uServicecategory;
        msg.address.digitMode =
                (V1_0::CdmaSmsDigitMode) rilMsg->sAddress.digit_mode;
        msg.address.numberMode =
                (V1_0::CdmaSmsNumberMode) rilMsg->sAddress.number_mode;
        msg.address.numberType =
                (V1_0::CdmaSmsNumberType) rilMsg->sAddress.number_type;
        msg.address.numberPlan =
                (V1_0::CdmaSmsNumberPlan) rilMsg->sAddress.number_plan;

        int digitLimit = MIN((rilMsg->sAddress.number_of_digits), RIL_CDMA_SMS_ADDRESS_MAX);
        msg.address.digits.setToExternal(rilMsg->sAddress.digits, digitLimit);

        msg.subAddress.subaddressType = (V1_0::CdmaSmsSubaddressType)
                rilMsg->sSubAddress.subaddressType;
        msg.subAddress.odd = rilMsg->sSubAddress.odd;

        digitLimit= MIN((rilMsg->sSubAddress.number_of_digits), RIL_CDMA_SMS_SUBADDRESS_MAX);
        msg.subAddress.digits.setToExternal(rilMsg->sSubAddress.digits, digitLimit);

        digitLimit = MIN((rilMsg->uBearerDataLen), RIL_CDMA_SMS_BEARER_DATA_MAX);
        msg.bearerData.setToExternal(rilMsg->aBearerData, digitLimit);

#if VDBG
        RLOGD("cdmaNewSmsInd");
#endif
        Return<void> retStatus = radioService[slotId]->mRadioIndication->cdmaNewSms(
                convertIntToRadioIndicationType(indicationType), msg);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("cdmaNewSmsInd: radioService[%d]->mRadioIndication == NULL", slotId);
    }

    return 0;
}

int radio::newBroadcastSmsInd(int slotId,
                              int indicationType, int token, RIL_Errno e, void *response,
                              size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        if (response == NULL || responseLen == 0) {
            RLOGE("newBroadcastSmsInd: invalid response");
            return 0;
        }

        hidl_vec<uint8_t> data;
        data.setToExternal((uint8_t *) response, responseLen);
#if VDBG
        RLOGD("newBroadcastSmsInd");
#endif
        Return<void> retStatus = radioService[slotId]->mRadioIndication->newBroadcastSms(
                convertIntToRadioIndicationType(indicationType), data);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("newBroadcastSmsInd: radioService[%d]->mRadioIndication == NULL", slotId);
    }

    return 0;
}

int radio::cdmaRuimSmsStorageFullInd(int slotId,
                                     int indicationType, int token, RIL_Errno e, void *response,
                                     size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
#if VDBG
        RLOGD("cdmaRuimSmsStorageFullInd");
#endif
        Return<void> retStatus = radioService[slotId]->mRadioIndication->cdmaRuimSmsStorageFull(
                convertIntToRadioIndicationType(indicationType));
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("cdmaRuimSmsStorageFullInd: radioService[%d]->mRadioIndication == NULL",
                slotId);
    }

    return 0;
}

int radio::restrictedStateChangedInd(int slotId,
                                     int indicationType, int token, RIL_Errno e, void *response,
                                     size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        if (response == NULL || responseLen != sizeof(int)) {
            RLOGE("restrictedStateChangedInd: invalid response");
            return 0;
        }
        int32_t state = ((int32_t *) response)[0];
#if VDBG
        RLOGD("restrictedStateChangedInd: state %d", state);
#endif
        Return<void> retStatus = radioService[slotId]->mRadioIndication->restrictedStateChanged(
                convertIntToRadioIndicationType(indicationType), (PhoneRestrictedState) state);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("restrictedStateChangedInd: radioService[%d]->mRadioIndication == NULL",
                slotId);
    }

    return 0;
}

int radio::enterEmergencyCallbackModeInd(int slotId,
                                         int indicationType, int token, RIL_Errno e, void *response,
                                         size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
#if VDBG
        RLOGD("enterEmergencyCallbackModeInd");
#endif
        Return<void> retStatus = radioService[slotId]->mRadioIndication->enterEmergencyCallbackMode(
                convertIntToRadioIndicationType(indicationType));
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("enterEmergencyCallbackModeInd: radioService[%d]->mRadioIndication == NULL",
                slotId);
    }

    return 0;
}

int radio::cdmaCallWaitingInd(int slotId,
                              int indicationType, int token, RIL_Errno e, void *response,
                              size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        if (response == NULL || responseLen != sizeof(RIL_CDMA_CallWaiting_v6)) {
            RLOGE("cdmaCallWaitingInd: invalid response");
            return 0;
        }

        CdmaCallWaiting callWaitingRecord = {};
        RIL_CDMA_CallWaiting_v6 *callWaitingRil = ((RIL_CDMA_CallWaiting_v6 *) response);
        callWaitingRecord.number = convertCharPtrToHidlString(callWaitingRil->number);
        callWaitingRecord.numberPresentation =
                (CdmaCallWaitingNumberPresentation) callWaitingRil->numberPresentation;
        callWaitingRecord.name = convertCharPtrToHidlString(callWaitingRil->name);
        convertRilCdmaSignalInfoRecordToHal(&callWaitingRil->signalInfoRecord,
                callWaitingRecord.signalInfoRecord);
        callWaitingRecord.numberType = (CdmaCallWaitingNumberType) callWaitingRil->number_type;
        callWaitingRecord.numberPlan = (CdmaCallWaitingNumberPlan) callWaitingRil->number_plan;

#if VDBG
        RLOGD("cdmaCallWaitingInd");
#endif
        Return<void> retStatus = radioService[slotId]->mRadioIndication->cdmaCallWaiting(
                convertIntToRadioIndicationType(indicationType), callWaitingRecord);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("cdmaCallWaitingInd: radioService[%d]->mRadioIndication == NULL", slotId);
    }

    return 0;
}

int radio::cdmaOtaProvisionStatusInd(int slotId,
                                     int indicationType, int token, RIL_Errno e, void *response,
                                     size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        if (response == NULL || responseLen != sizeof(int)) {
            RLOGE("cdmaOtaProvisionStatusInd: invalid response");
            return 0;
        }
        int32_t status = ((int32_t *) response)[0];
#if VDBG
        RLOGD("cdmaOtaProvisionStatusInd: status %d", status);
#endif
        Return<void> retStatus = radioService[slotId]->mRadioIndication->cdmaOtaProvisionStatus(
                convertIntToRadioIndicationType(indicationType), (CdmaOtaProvisionStatus) status);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("cdmaOtaProvisionStatusInd: radioService[%d]->mRadioIndication == NULL",
                slotId);
    }

    return 0;
}

int radio::cdmaInfoRecInd(int slotId,
                          int indicationType, int token, RIL_Errno e, void *response,
                          size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        if (response == NULL || responseLen != sizeof(RIL_CDMA_InformationRecords)) {
            RLOGE("cdmaInfoRecInd: invalid response");
            return 0;
        }

        CdmaInformationRecords records = {};
        RIL_CDMA_InformationRecords *recordsRil = (RIL_CDMA_InformationRecords *) response;

        char* string8 = NULL;
        int num = MIN(recordsRil->numberOfInfoRecs, RIL_CDMA_MAX_NUMBER_OF_INFO_RECS);
        if (recordsRil->numberOfInfoRecs > RIL_CDMA_MAX_NUMBER_OF_INFO_RECS) {
            RLOGE("cdmaInfoRecInd: received %d recs which is more than %d, dropping "
                    "additional ones", recordsRil->numberOfInfoRecs,
                    RIL_CDMA_MAX_NUMBER_OF_INFO_RECS);
        }
        records.infoRec.resize(num);
        for (int i = 0 ; i < num ; i++) {
            CdmaInformationRecord *record = &records.infoRec[i];
            RIL_CDMA_InformationRecord *infoRec = &recordsRil->infoRec[i];
            record->name = (CdmaInfoRecName) infoRec->name;
            // All vectors should be size 0 except one which will be size 1. Set everything to
            // size 0 initially.
            record->display.resize(0);
            record->number.resize(0);
            record->signal.resize(0);
            record->redir.resize(0);
            record->lineCtrl.resize(0);
            record->clir.resize(0);
            record->audioCtrl.resize(0);
            switch (infoRec->name) {
                case RIL_CDMA_DISPLAY_INFO_REC:
                case RIL_CDMA_EXTENDED_DISPLAY_INFO_REC: {
                    if (infoRec->rec.display.alpha_len > CDMA_ALPHA_INFO_BUFFER_LENGTH) {
                        RLOGE("cdmaInfoRecInd: invalid display info response length %d "
                                "expected not more than %d", (int) infoRec->rec.display.alpha_len,
                                CDMA_ALPHA_INFO_BUFFER_LENGTH);
                        return 0;
                    }
                    string8 = (char*) malloc((infoRec->rec.display.alpha_len + 1) * sizeof(char));
                    if (string8 == NULL) {
                        RLOGE("cdmaInfoRecInd: Memory allocation failed for "
                                "responseCdmaInformationRecords");
                        return 0;
                    }
                    memcpy(string8, infoRec->rec.display.alpha_buf, infoRec->rec.display.alpha_len);
                    string8[(int)infoRec->rec.display.alpha_len] = '\0';

                    record->display.resize(1);
                    record->display[0].alphaBuf = string8;
                    free(string8);
                    string8 = NULL;
                    break;
                }

                case RIL_CDMA_CALLED_PARTY_NUMBER_INFO_REC:
                case RIL_CDMA_CALLING_PARTY_NUMBER_INFO_REC:
                case RIL_CDMA_CONNECTED_NUMBER_INFO_REC: {
                    if (infoRec->rec.number.len > CDMA_NUMBER_INFO_BUFFER_LENGTH) {
                        RLOGE("cdmaInfoRecInd: invalid display info response length %d "
                                "expected not more than %d", (int) infoRec->rec.number.len,
                                CDMA_NUMBER_INFO_BUFFER_LENGTH);
                        return 0;
                    }
                    string8 = (char*) malloc((infoRec->rec.number.len + 1) * sizeof(char));
                    if (string8 == NULL) {
                        RLOGE("cdmaInfoRecInd: Memory allocation failed for "
                                "responseCdmaInformationRecords");
                        return 0;
                    }
                    memcpy(string8, infoRec->rec.number.buf, infoRec->rec.number.len);
                    string8[(int)infoRec->rec.number.len] = '\0';

                    record->number.resize(1);
                    record->number[0].number = string8;
                    free(string8);
                    string8 = NULL;
                    record->number[0].numberType = infoRec->rec.number.number_type;
                    record->number[0].numberPlan = infoRec->rec.number.number_plan;
                    record->number[0].pi = infoRec->rec.number.pi;
                    record->number[0].si = infoRec->rec.number.si;
                    break;
                }

                case RIL_CDMA_SIGNAL_INFO_REC: {
                    record->signal.resize(1);
                    record->signal[0].isPresent = infoRec->rec.signal.isPresent;
                    record->signal[0].signalType = infoRec->rec.signal.signalType;
                    record->signal[0].alertPitch = infoRec->rec.signal.alertPitch;
                    record->signal[0].signal = infoRec->rec.signal.signal;
                    break;
                }

                case RIL_CDMA_REDIRECTING_NUMBER_INFO_REC: {
                    if (infoRec->rec.redir.redirectingNumber.len >
                                                  CDMA_NUMBER_INFO_BUFFER_LENGTH) {
                        RLOGE("cdmaInfoRecInd: invalid display info response length %d "
                                "expected not more than %d\n",
                                (int)infoRec->rec.redir.redirectingNumber.len,
                                CDMA_NUMBER_INFO_BUFFER_LENGTH);
                        return 0;
                    }
                    string8 = (char*) malloc((infoRec->rec.redir.redirectingNumber.len + 1) *
                            sizeof(char));
                    if (string8 == NULL) {
                        RLOGE("cdmaInfoRecInd: Memory allocation failed for "
                                "responseCdmaInformationRecords");
                        return 0;
                    }
                    memcpy(string8, infoRec->rec.redir.redirectingNumber.buf,
                            infoRec->rec.redir.redirectingNumber.len);
                    string8[(int)infoRec->rec.redir.redirectingNumber.len] = '\0';

                    record->redir.resize(1);
                    record->redir[0].redirectingNumber.number = string8;
                    free(string8);
                    string8 = NULL;
                    record->redir[0].redirectingNumber.numberType =
                            infoRec->rec.redir.redirectingNumber.number_type;
                    record->redir[0].redirectingNumber.numberPlan =
                            infoRec->rec.redir.redirectingNumber.number_plan;
                    record->redir[0].redirectingNumber.pi = infoRec->rec.redir.redirectingNumber.pi;
                    record->redir[0].redirectingNumber.si = infoRec->rec.redir.redirectingNumber.si;
                    record->redir[0].redirectingReason =
                            (CdmaRedirectingReason) infoRec->rec.redir.redirectingReason;
                    break;
                }

                case RIL_CDMA_LINE_CONTROL_INFO_REC: {
                    record->lineCtrl.resize(1);
                    record->lineCtrl[0].lineCtrlPolarityIncluded =
                            infoRec->rec.lineCtrl.lineCtrlPolarityIncluded;
                    record->lineCtrl[0].lineCtrlToggle = infoRec->rec.lineCtrl.lineCtrlToggle;
                    record->lineCtrl[0].lineCtrlReverse = infoRec->rec.lineCtrl.lineCtrlReverse;
                    record->lineCtrl[0].lineCtrlPowerDenial =
                            infoRec->rec.lineCtrl.lineCtrlPowerDenial;
                    break;
                }

                case RIL_CDMA_T53_CLIR_INFO_REC: {
                    record->clir.resize(1);
                    record->clir[0].cause = infoRec->rec.clir.cause;
                    break;
                }

                case RIL_CDMA_T53_AUDIO_CONTROL_INFO_REC: {
                    record->audioCtrl.resize(1);
                    record->audioCtrl[0].upLink = infoRec->rec.audioCtrl.upLink;
                    record->audioCtrl[0].downLink = infoRec->rec.audioCtrl.downLink;
                    break;
                }

                case RIL_CDMA_T53_RELEASE_INFO_REC:
                    RLOGE("cdmaInfoRecInd: RIL_CDMA_T53_RELEASE_INFO_REC: INVALID");
                    return 0;

                default:
                    RLOGE("cdmaInfoRecInd: Incorrect name value");
                    return 0;
            }
        }

#if VDBG
        RLOGD("cdmaInfoRecInd");
#endif
        Return<void> retStatus = radioService[slotId]->mRadioIndication->cdmaInfoRec(
                convertIntToRadioIndicationType(indicationType), records);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("cdmaInfoRecInd: radioService[%d]->mRadioIndication == NULL", slotId);
    }

    return 0;
}

int radio::indicateRingbackToneInd(int slotId,
                                   int indicationType, int token, RIL_Errno e, void *response,
                                   size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        if (response == NULL || responseLen != sizeof(int)) {
            RLOGE("indicateRingbackToneInd: invalid response");
            return 0;
        }
        bool start = ((int32_t *) response)[0];
#if VDBG
        RLOGD("indicateRingbackToneInd: start %d", start);
#endif
        Return<void> retStatus = radioService[slotId]->mRadioIndication->indicateRingbackTone(
                convertIntToRadioIndicationType(indicationType), start);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("indicateRingbackToneInd: radioService[%d]->mRadioIndication == NULL", slotId);
    }

    return 0;
}

int radio::resendIncallMuteInd(int slotId,
                               int indicationType, int token, RIL_Errno e, void *response,
                               size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
#if VDBG
        RLOGD("resendIncallMuteInd");
#endif
        Return<void> retStatus = radioService[slotId]->mRadioIndication->resendIncallMute(
                convertIntToRadioIndicationType(indicationType));
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("resendIncallMuteInd: radioService[%d]->mRadioIndication == NULL", slotId);
    }

    return 0;
}

int radio::cdmaSubscriptionSourceChangedInd(int slotId,
                                            int indicationType, int token, RIL_Errno e,
                                            void *response, size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        if (response == NULL || responseLen != sizeof(int)) {
            RLOGE("cdmaSubscriptionSourceChangedInd: invalid response");
            return 0;
        }
        int32_t cdmaSource = ((int32_t *) response)[0];
#if VDBG
        RLOGD("cdmaSubscriptionSourceChangedInd: cdmaSource %d", cdmaSource);
#endif
        Return<void> retStatus = radioService[slotId]->mRadioIndication->
                cdmaSubscriptionSourceChanged(convertIntToRadioIndicationType(indicationType),
                (CdmaSubscriptionSource) cdmaSource);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("cdmaSubscriptionSourceChangedInd: radioService[%d]->mRadioIndication == NULL",
                slotId);
    }

    return 0;
}

int radio::cdmaPrlChangedInd(int slotId,
                             int indicationType, int token, RIL_Errno e, void *response,
                             size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        if (response == NULL || responseLen != sizeof(int)) {
            RLOGE("cdmaPrlChangedInd: invalid response");
            return 0;
        }
        int32_t version = ((int32_t *) response)[0];
#if VDBG
        RLOGD("cdmaPrlChangedInd: version %d", version);
#endif
        Return<void> retStatus = radioService[slotId]->mRadioIndication->cdmaPrlChanged(
                convertIntToRadioIndicationType(indicationType), version);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("cdmaPrlChangedInd: radioService[%d]->mRadioIndication == NULL", slotId);
    }

    return 0;
}

int radio::exitEmergencyCallbackModeInd(int slotId,
                                        int indicationType, int token, RIL_Errno e, void *response,
                                        size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
#if VDBG
        RLOGD("exitEmergencyCallbackModeInd");
#endif
        Return<void> retStatus = radioService[slotId]->mRadioIndication->exitEmergencyCallbackMode(
                convertIntToRadioIndicationType(indicationType));
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("exitEmergencyCallbackModeInd: radioService[%d]->mRadioIndication == NULL",
                slotId);
    }

    return 0;
}

int radio::rilConnectedInd(int slotId,
                           int indicationType, int token, RIL_Errno e, void *response,
                           size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        RLOGD("rilConnectedInd");
        Return<void> retStatus = radioService[slotId]->mRadioIndication->rilConnected(
                convertIntToRadioIndicationType(indicationType));
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("rilConnectedInd: radioService[%d]->mRadioIndication == NULL", slotId);
    }

    return 0;
}

int radio::voiceRadioTechChangedInd(int slotId,
                                    int indicationType, int token, RIL_Errno e, void *response,
                                    size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        if (response == NULL || responseLen != sizeof(int)) {
            RLOGE("voiceRadioTechChangedInd: invalid response");
            return 0;
        }
        int32_t rat = ((int32_t *) response)[0];
#if VDBG
        RLOGD("voiceRadioTechChangedInd: rat %d", rat);
#endif
        Return<void> retStatus = radioService[slotId]->mRadioIndication->voiceRadioTechChanged(
                convertIntToRadioIndicationType(indicationType), (RadioTechnology) rat);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("voiceRadioTechChangedInd: radioService[%d]->mRadioIndication == NULL",
                slotId);
    }

    return 0;
}

void convertRilCellInfoListToHal(const std::vector<RIL_CellInfo_v12>& rillCellInfo,
        hidl_vec<CellInfo>& records) {
    auto num = rillCellInfo.size();
    records.resize(num);

    for (unsigned int i = 0; i < num; i++) {
        records[i].cellInfoType = (CellInfoType) rillCellInfo[i].cellInfoType;
        records[i].registered = rillCellInfo[i].registered;
        records[i].timeStampType = (TimeStampType) rillCellInfo[i].timeStampType;
        records[i].timeStamp = rillCellInfo[i].timeStamp;
        // All vectors should be size 0 except one which will be size 1. Set everything to
        // size 0 initially.
        records[i].gsm.resize(0);
        records[i].wcdma.resize(0);
        records[i].cdma.resize(0);
        records[i].lte.resize(0);
        records[i].tdscdma.resize(0);
        switch(rillCellInfo[i].cellInfoType) {
            case RIL_CELL_INFO_TYPE_GSM: {
                records[i].gsm.resize(1);
                CellInfoGsm *cellInfoGsm = &records[i].gsm[0];

                new (&cellInfoGsm->cellIdentityGsm.mcc) hidl_string(
                        rillCellInfo[i].CellInfo.gsm.cellIdentityGsm.mcc,
                        strnlen(rillCellInfo[i].CellInfo.gsm.cellIdentityGsm.mcc,
                                sizeof(rillCellInfo[i].CellInfo.gsm.cellIdentityGsm.mcc) - 1));

                new (&cellInfoGsm->cellIdentityGsm.mnc) hidl_string(
                        rillCellInfo[i].CellInfo.gsm.cellIdentityGsm.mnc,
                        strnlen(rillCellInfo[i].CellInfo.gsm.cellIdentityGsm.mnc,
                                sizeof(rillCellInfo[i].CellInfo.gsm.cellIdentityGsm.mnc) - 1));

                cellInfoGsm->cellIdentityGsm.lac =
                        rillCellInfo[i].CellInfo.gsm.cellIdentityGsm.lac;
                cellInfoGsm->cellIdentityGsm.cid =
                        rillCellInfo[i].CellInfo.gsm.cellIdentityGsm.cid;
                cellInfoGsm->cellIdentityGsm.arfcn =
                        rillCellInfo[i].CellInfo.gsm.cellIdentityGsm.arfcn;
                cellInfoGsm->cellIdentityGsm.bsic =
                        rillCellInfo[i].CellInfo.gsm.cellIdentityGsm.bsic;
                cellInfoGsm->signalStrengthGsm.signalStrength =
                        rillCellInfo[i].CellInfo.gsm.signalStrengthGsm.signalStrength;
                cellInfoGsm->signalStrengthGsm.bitErrorRate =
                        rillCellInfo[i].CellInfo.gsm.signalStrengthGsm.bitErrorRate;
                cellInfoGsm->signalStrengthGsm.timingAdvance =
                        rillCellInfo[i].CellInfo.gsm.signalStrengthGsm.timingAdvance;
                break;
            }

            case RIL_CELL_INFO_TYPE_WCDMA: {
                records[i].wcdma.resize(1);
                CellInfoWcdma *cellInfoWcdma = &records[i].wcdma[0];

                new (&cellInfoWcdma->cellIdentityWcdma.mcc) hidl_string(
                        rillCellInfo[i].CellInfo.wcdma.cellIdentityWcdma.mcc,
                        strnlen(rillCellInfo[i].CellInfo.wcdma.cellIdentityWcdma.mcc,
                                sizeof(rillCellInfo[i].CellInfo.wcdma.cellIdentityWcdma.mcc) - 1));

                new (&cellInfoWcdma->cellIdentityWcdma.mnc) hidl_string(
                        rillCellInfo[i].CellInfo.wcdma.cellIdentityWcdma.mnc,
                        strnlen(rillCellInfo[i].CellInfo.wcdma.cellIdentityWcdma.mnc,
                                sizeof(rillCellInfo[i].CellInfo.wcdma.cellIdentityWcdma.mnc) - 1));

                cellInfoWcdma->cellIdentityWcdma.lac =
                        rillCellInfo[i].CellInfo.wcdma.cellIdentityWcdma.lac;
                cellInfoWcdma->cellIdentityWcdma.cid =
                        rillCellInfo[i].CellInfo.wcdma.cellIdentityWcdma.cid;
                cellInfoWcdma->cellIdentityWcdma.psc =
                        rillCellInfo[i].CellInfo.wcdma.cellIdentityWcdma.psc;
                cellInfoWcdma->cellIdentityWcdma.uarfcn =
                        rillCellInfo[i].CellInfo.wcdma.cellIdentityWcdma.uarfcn;
                cellInfoWcdma->signalStrengthWcdma.signalStrength =
                        rillCellInfo[i].CellInfo.wcdma.signalStrengthWcdma.signalStrength;
                cellInfoWcdma->signalStrengthWcdma.bitErrorRate =
                        rillCellInfo[i].CellInfo.wcdma.signalStrengthWcdma.bitErrorRate;
                break;
            }

            case RIL_CELL_INFO_TYPE_CDMA: {
                records[i].cdma.resize(1);
                CellInfoCdma *cellInfoCdma = &records[i].cdma[0];
                cellInfoCdma->cellIdentityCdma.networkId =
                        rillCellInfo[i].CellInfo.cdma.cellIdentityCdma.networkId;
                cellInfoCdma->cellIdentityCdma.systemId =
                        rillCellInfo[i].CellInfo.cdma.cellIdentityCdma.systemId;
                cellInfoCdma->cellIdentityCdma.baseStationId =
                        rillCellInfo[i].CellInfo.cdma.cellIdentityCdma.basestationId;
                cellInfoCdma->cellIdentityCdma.longitude =
                        rillCellInfo[i].CellInfo.cdma.cellIdentityCdma.longitude;
                cellInfoCdma->cellIdentityCdma.latitude =
                        rillCellInfo[i].CellInfo.cdma.cellIdentityCdma.latitude;
                cellInfoCdma->signalStrengthCdma.dbm =
                        rillCellInfo[i].CellInfo.cdma.signalStrengthCdma.dbm;
                cellInfoCdma->signalStrengthCdma.ecio =
                        rillCellInfo[i].CellInfo.cdma.signalStrengthCdma.ecio;
                cellInfoCdma->signalStrengthEvdo.dbm =
                        rillCellInfo[i].CellInfo.cdma.signalStrengthEvdo.dbm;
                cellInfoCdma->signalStrengthEvdo.ecio =
                        rillCellInfo[i].CellInfo.cdma.signalStrengthEvdo.ecio;
                cellInfoCdma->signalStrengthEvdo.signalNoiseRatio =
                        rillCellInfo[i].CellInfo.cdma.signalStrengthEvdo.signalNoiseRatio;
                break;
            }

            case RIL_CELL_INFO_TYPE_LTE: {
                records[i].lte.resize(1);
                CellInfoLte *cellInfoLte = &records[i].lte[0];

                new (&cellInfoLte->cellIdentityLte.mcc) hidl_string(
                        rillCellInfo[i].CellInfo.lte.cellIdentityLte.mcc,
                        strnlen(rillCellInfo[i].CellInfo.lte.cellIdentityLte.mcc,
                                sizeof(rillCellInfo[i].CellInfo.lte.cellIdentityLte.mcc) - 1));

                new (&cellInfoLte->cellIdentityLte.mnc) hidl_string(
                        rillCellInfo[i].CellInfo.lte.cellIdentityLte.mnc,
                        strnlen(rillCellInfo[i].CellInfo.lte.cellIdentityLte.mnc,
                                sizeof(rillCellInfo[i].CellInfo.lte.cellIdentityLte.mnc) - 1));

                cellInfoLte->cellIdentityLte.ci =
                        rillCellInfo[i].CellInfo.lte.cellIdentityLte.ci;
                cellInfoLte->cellIdentityLte.pci =
                        rillCellInfo[i].CellInfo.lte.cellIdentityLte.pci;
                cellInfoLte->cellIdentityLte.tac =
                        rillCellInfo[i].CellInfo.lte.cellIdentityLte.tac;
                cellInfoLte->cellIdentityLte.earfcn =
                        rillCellInfo[i].CellInfo.lte.cellIdentityLte.earfcn;
                cellInfoLte->signalStrengthLte.signalStrength =
                        rillCellInfo[i].CellInfo.lte.signalStrengthLte.signalStrength;
                cellInfoLte->signalStrengthLte.rsrp =
                        rillCellInfo[i].CellInfo.lte.signalStrengthLte.rsrp;
                cellInfoLte->signalStrengthLte.rsrq =
                        rillCellInfo[i].CellInfo.lte.signalStrengthLte.rsrq;
                cellInfoLte->signalStrengthLte.rssnr =
                        rillCellInfo[i].CellInfo.lte.signalStrengthLte.rssnr;
                cellInfoLte->signalStrengthLte.cqi =
                        rillCellInfo[i].CellInfo.lte.signalStrengthLte.cqi;
                cellInfoLte->signalStrengthLte.timingAdvance =
                        rillCellInfo[i].CellInfo.lte.signalStrengthLte.timingAdvance;
                break;
            }

            case RIL_CELL_INFO_TYPE_TD_SCDMA: {
                records[i].tdscdma.resize(1);
                CellInfoTdscdma *cellInfoTdscdma = &records[i].tdscdma[0];

                new (&cellInfoTdscdma->cellIdentityTdscdma.mcc) hidl_string(
                        rillCellInfo[i].CellInfo.tdscdma.cellIdentityTdscdma.mcc,
                        strnlen(rillCellInfo[i].CellInfo.tdscdma.cellIdentityTdscdma.mcc,
                                sizeof(rillCellInfo[i].CellInfo.tdscdma.cellIdentityTdscdma.mcc) - 1));

                new (&cellInfoTdscdma->cellIdentityTdscdma.mnc) hidl_string(
                        rillCellInfo[i].CellInfo.tdscdma.cellIdentityTdscdma.mnc,
                        strnlen(rillCellInfo[i].CellInfo.tdscdma.cellIdentityTdscdma.mnc,
                                sizeof(rillCellInfo[i].CellInfo.tdscdma.cellIdentityTdscdma.mnc) - 1));

                cellInfoTdscdma->cellIdentityTdscdma.lac =
                        rillCellInfo[i].CellInfo.tdscdma.cellIdentityTdscdma.lac;
                cellInfoTdscdma->cellIdentityTdscdma.cid =
                        rillCellInfo[i].CellInfo.tdscdma.cellIdentityTdscdma.cid;
                cellInfoTdscdma->cellIdentityTdscdma.cpid =
                        rillCellInfo[i].CellInfo.tdscdma.cellIdentityTdscdma.cpid;
                cellInfoTdscdma->signalStrengthTdscdma.rscp =
                        rillCellInfo[i].CellInfo.tdscdma.signalStrengthTdscdma.rscp;
                break;
            }

            default: {
                break;
            }
        }
    }
}

void convertRilCellInfoListToHal(int slotId, void *response, size_t responseLen, hidl_vec<CellInfo>& records) {
    int num = responseLen / sizeof(RIL_CellInfo_v12);
    records.resize(num);

    RIL_CellInfo_v12 *rillCellInfo = (RIL_CellInfo_v12 *) response;
    for (int i = 0; i < num; i++) {
        records[i].cellInfoType = (CellInfoType) rillCellInfo->cellInfoType;
        records[i].registered = rillCellInfo->registered;
        records[i].timeStampType = (TimeStampType) rillCellInfo->timeStampType;
        records[i].timeStamp = rillCellInfo->timeStamp;
        // All vectors should be size 0 except one which will be size 1. Set everything to
        // size 0 initially.
        records[i].gsm.resize(0);
        records[i].wcdma.resize(0);
        records[i].cdma.resize(0);
        records[i].lte.resize(0);
        records[i].tdscdma.resize(0);
        switch(rillCellInfo->cellInfoType) {
            case RIL_CELL_INFO_TYPE_GSM: {
                records[i].gsm.resize(1);
                CellInfoGsm *cellInfoGsm = &records[i].gsm[0];

                new (&cellInfoGsm->cellIdentityGsm.mcc) hidl_string(
                        rillCellInfo->CellInfo.gsm.cellIdentityGsm.mcc,
                        strnlen(rillCellInfo->CellInfo.gsm.cellIdentityGsm.mcc,
                                sizeof(rillCellInfo->CellInfo.gsm.cellIdentityGsm.mcc) - 1));

                new (&cellInfoGsm->cellIdentityGsm.mnc) hidl_string(
                        rillCellInfo->CellInfo.gsm.cellIdentityGsm.mnc,
                        strnlen(rillCellInfo->CellInfo.gsm.cellIdentityGsm.mnc,
                                sizeof(rillCellInfo->CellInfo.gsm.cellIdentityGsm.mnc) - 1));

                cellInfoGsm->cellIdentityGsm.lac =
                        rillCellInfo->CellInfo.gsm.cellIdentityGsm.lac;
                cellInfoGsm->cellIdentityGsm.cid =
                        rillCellInfo->CellInfo.gsm.cellIdentityGsm.cid;
                cellInfoGsm->cellIdentityGsm.arfcn =
                        rillCellInfo->CellInfo.gsm.cellIdentityGsm.arfcn;
                cellInfoGsm->cellIdentityGsm.bsic =
                        rillCellInfo->CellInfo.gsm.cellIdentityGsm.bsic;
                cellInfoGsm->signalStrengthGsm.signalStrength =
                        rillCellInfo->CellInfo.gsm.signalStrengthGsm.signalStrength;
                cellInfoGsm->signalStrengthGsm.bitErrorRate =
                        rillCellInfo->CellInfo.gsm.signalStrengthGsm.bitErrorRate;
                cellInfoGsm->signalStrengthGsm.timingAdvance =
                        rillCellInfo->CellInfo.gsm.signalStrengthGsm.timingAdvance;
                break;
            }

            case RIL_CELL_INFO_TYPE_WCDMA: {
                records[i].wcdma.resize(1);
                CellInfoWcdma *cellInfoWcdma = &records[i].wcdma[0];

                new (&cellInfoWcdma->cellIdentityWcdma.mcc) hidl_string(
                        rillCellInfo->CellInfo.wcdma.cellIdentityWcdma.mcc,
                        strnlen(rillCellInfo->CellInfo.wcdma.cellIdentityWcdma.mcc,
                                sizeof(rillCellInfo->CellInfo.wcdma.cellIdentityWcdma.mcc) - 1));

                new (&cellInfoWcdma->cellIdentityWcdma.mnc) hidl_string(
                        rillCellInfo->CellInfo.wcdma.cellIdentityWcdma.mnc,
                        strnlen(rillCellInfo->CellInfo.wcdma.cellIdentityWcdma.mnc,
                                sizeof(rillCellInfo->CellInfo.wcdma.cellIdentityWcdma.mnc) - 1));

                cellInfoWcdma->cellIdentityWcdma.lac =
                        rillCellInfo->CellInfo.wcdma.cellIdentityWcdma.lac;
                cellInfoWcdma->cellIdentityWcdma.cid =
                        rillCellInfo->CellInfo.wcdma.cellIdentityWcdma.cid;
                cellInfoWcdma->cellIdentityWcdma.psc =
                        rillCellInfo->CellInfo.wcdma.cellIdentityWcdma.psc;
                cellInfoWcdma->cellIdentityWcdma.uarfcn =
                        rillCellInfo->CellInfo.wcdma.cellIdentityWcdma.uarfcn;
                cellInfoWcdma->signalStrengthWcdma.signalStrength =
                        rillCellInfo->CellInfo.wcdma.signalStrengthWcdma.signalStrength;
                cellInfoWcdma->signalStrengthWcdma.bitErrorRate =
                        rillCellInfo->CellInfo.wcdma.signalStrengthWcdma.bitErrorRate;
                break;
            }

            case RIL_CELL_INFO_TYPE_CDMA: {
                records[i].cdma.resize(1);
                CellInfoCdma *cellInfoCdma = &records[i].cdma[0];
                cellInfoCdma->cellIdentityCdma.networkId =
                        rillCellInfo->CellInfo.cdma.cellIdentityCdma.networkId;
                cellInfoCdma->cellIdentityCdma.systemId =
                        rillCellInfo->CellInfo.cdma.cellIdentityCdma.systemId;
                cellInfoCdma->cellIdentityCdma.baseStationId =
                        rillCellInfo->CellInfo.cdma.cellIdentityCdma.basestationId;
                cellInfoCdma->cellIdentityCdma.longitude =
                        rillCellInfo->CellInfo.cdma.cellIdentityCdma.longitude;
                cellInfoCdma->cellIdentityCdma.latitude =
                        rillCellInfo->CellInfo.cdma.cellIdentityCdma.latitude;
                cellInfoCdma->signalStrengthCdma.dbm =
                        rillCellInfo->CellInfo.cdma.signalStrengthCdma.dbm;
                cellInfoCdma->signalStrengthCdma.ecio =
                        rillCellInfo->CellInfo.cdma.signalStrengthCdma.ecio;
                cellInfoCdma->signalStrengthEvdo.dbm =
                        rillCellInfo->CellInfo.cdma.signalStrengthEvdo.dbm;
                cellInfoCdma->signalStrengthEvdo.ecio =
                        rillCellInfo->CellInfo.cdma.signalStrengthEvdo.ecio;
                cellInfoCdma->signalStrengthEvdo.signalNoiseRatio =
                        rillCellInfo->CellInfo.cdma.signalStrengthEvdo.signalNoiseRatio;
                break;
            }

            case RIL_CELL_INFO_TYPE_LTE: {
                records[i].lte.resize(1);
                CellInfoLte *cellInfoLte = &records[i].lte[0];

                new (&cellInfoLte->cellIdentityLte.mcc) hidl_string(
                        rillCellInfo->CellInfo.lte.cellIdentityLte.mcc,
                        strnlen(rillCellInfo->CellInfo.lte.cellIdentityLte.mcc,
                                sizeof(rillCellInfo->CellInfo.lte.cellIdentityLte.mcc) - 1));

                new (&cellInfoLte->cellIdentityLte.mnc) hidl_string(
                        rillCellInfo->CellInfo.lte.cellIdentityLte.mnc,
                        strnlen(rillCellInfo->CellInfo.lte.cellIdentityLte.mnc,
                                sizeof(rillCellInfo->CellInfo.lte.cellIdentityLte.mnc) - 1));

                cellInfoLte->cellIdentityLte.ci =
                        rillCellInfo->CellInfo.lte.cellIdentityLte.ci;
                cellInfoLte->cellIdentityLte.pci =
                        rillCellInfo->CellInfo.lte.cellIdentityLte.pci;
                cellInfoLte->cellIdentityLte.tac =
                        rillCellInfo->CellInfo.lte.cellIdentityLte.tac;
                cellInfoLte->cellIdentityLte.earfcn =
                        rillCellInfo->CellInfo.lte.cellIdentityLte.earfcn;
                cellInfoLte->signalStrengthLte.signalStrength =
                        rillCellInfo->CellInfo.lte.signalStrengthLte.signalStrength;
                cellInfoLte->signalStrengthLte.rsrp =
                        rillCellInfo->CellInfo.lte.signalStrengthLte.rsrp;
                cellInfoLte->signalStrengthLte.rsrq =
                        rillCellInfo->CellInfo.lte.signalStrengthLte.rsrq;
                cellInfoLte->signalStrengthLte.rssnr =
                        rillCellInfo->CellInfo.lte.signalStrengthLte.rssnr;
                cellInfoLte->signalStrengthLte.cqi =
                        rillCellInfo->CellInfo.lte.signalStrengthLte.cqi;
                cellInfoLte->signalStrengthLte.timingAdvance =
                        rillCellInfo->CellInfo.lte.signalStrengthLte.timingAdvance;
                break;
            }

            case RIL_CELL_INFO_TYPE_TD_SCDMA: {
                records[i].tdscdma.resize(1);
                CellInfoTdscdma *cellInfoTdscdma = &records[i].tdscdma[0];

                new (&cellInfoTdscdma->cellIdentityTdscdma.mcc) hidl_string(
                        rillCellInfo->CellInfo.tdscdma.cellIdentityTdscdma.mcc,
                        strnlen(rillCellInfo->CellInfo.tdscdma.cellIdentityTdscdma.mcc,
                                sizeof(rillCellInfo->CellInfo.tdscdma.cellIdentityTdscdma.mcc) - 1));

                new (&cellInfoTdscdma->cellIdentityTdscdma.mnc) hidl_string(
                        rillCellInfo->CellInfo.tdscdma.cellIdentityTdscdma.mnc,
                        strnlen(rillCellInfo->CellInfo.tdscdma.cellIdentityTdscdma.mnc,
                                sizeof(rillCellInfo->CellInfo.tdscdma.cellIdentityTdscdma.mnc) - 1));

                cellInfoTdscdma->cellIdentityTdscdma.lac =
                        rillCellInfo->CellInfo.tdscdma.cellIdentityTdscdma.lac;
                cellInfoTdscdma->cellIdentityTdscdma.cid =
                        rillCellInfo->CellInfo.tdscdma.cellIdentityTdscdma.cid;
                cellInfoTdscdma->cellIdentityTdscdma.cpid =
                        rillCellInfo->CellInfo.tdscdma.cellIdentityTdscdma.cpid;
                cellInfoTdscdma->signalStrengthTdscdma.rscp =
                        rillCellInfo->CellInfo.tdscdma.signalStrengthTdscdma.rscp;
                break;
            }

            default: {
                break;
            }
        }
        rillCellInfo += 1;
    }
}

int RadioImpl::sendCellInfoListInd(int slotId,
        int indicationType,
        int token,
        RIL_Errno e,
        void *response,
        size_t responseLen)
{
    if ( (mRadioIndication != NULL)) {
        if ((response == NULL && responseLen != 0) || responseLen % sizeof(RIL_CellInfo_v12) != 0) {
            RLOGE("cellInfoListInd: invalid response");
            return 0;
        }

        Return<void> retStatus;

#if VDBG
        RLOGD("cellInfoListInd");
#endif

        hidl_vec<CellInfo> records;
        convertRilCellInfoListToHal(slotId, response, responseLen, records);
        retStatus = radioService[slotId]->mRadioIndication->cellInfoList(
                convertIntToRadioIndicationType(indicationType), records);

        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("cellInfoListInd: radioService[%d]->mRadioIndication == NULL", slotId);
    }

    return 0;
}
int radio::cellInfoListInd_1_1(int slotId,
                           int indicationType, int token, RIL_Errno e, void *response,
                           size_t responseLen) {
    if (radioService[slotId] != NULL &&
        (radioService[slotId]->mRadioIndication != NULL)) {
        if ((response == NULL && responseLen != 0) || responseLen % sizeof(RIL_CellInfo_v12) != 0) {
            RLOGE("cellInfoListInd: invalid response");
            return 0;
        }

        Return<void> retStatus;

#if VDBG
        RLOGD("cellInfoListInd");
#endif

        hidl_vec<CellInfo> records;
        convertRilCellInfoListToHal(slotId, response, responseLen, records);
        retStatus = radioService[slotId]->mRadioIndication->cellInfoList(
                convertIntToRadioIndicationType(indicationType), records);

        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("cellInfoListInd: radioService[%d]->mRadioIndication == NULL", slotId);
    }

    return 0;
}

int radio::imsNetworkStateChangedInd(int slotId,
                                     int indicationType, int token, RIL_Errno e, void *response,
                                     size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
#if VDBG
        RLOGD("imsNetworkStateChangedInd");
#endif
        Return<void> retStatus = radioService[slotId]->mRadioIndication->imsNetworkStateChanged(
                convertIntToRadioIndicationType(indicationType));
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("imsNetworkStateChangedInd: radioService[%d]->mRadioIndication == NULL",
                slotId);
    }

    return 0;
}

int radio::subscriptionStatusChangedInd(int slotId,
                                        int indicationType, int token, RIL_Errno e, void *response,
                                        size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        if (response == NULL || responseLen != sizeof(int)) {
            RLOGE("subscriptionStatusChangedInd: invalid response");
            return 0;
        }
        bool activate = ((int32_t *) response)[0];
#if VDBG
        RLOGD("subscriptionStatusChangedInd: activate %d", activate);
#endif
        Return<void> retStatus = radioService[slotId]->mRadioIndication->subscriptionStatusChanged(
                convertIntToRadioIndicationType(indicationType), activate);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("subscriptionStatusChangedInd: radioService[%d]->mRadioIndication == NULL",
                slotId);
    }

    return 0;
}

int radio::srvccStateNotifyInd(int slotId,
                               int indicationType, int token, RIL_Errno e, void *response,
                               size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        if (response == NULL || responseLen != sizeof(int)) {
            RLOGE("srvccStateNotifyInd: invalid response");
            return 0;
        }
        int32_t state = ((int32_t *) response)[0];
#if VDBG
        RLOGD("srvccStateNotifyInd: rat %d", state);
#endif
        Return<void> retStatus = radioService[slotId]->mRadioIndication->srvccStateNotify(
                convertIntToRadioIndicationType(indicationType), (SrvccState) state);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("srvccStateNotifyInd: radioService[%d]->mRadioIndication == NULL", slotId);
    }

    return 0;
}

void convertRilHardwareConfigListToHal(void *response, size_t responseLen,
        hidl_vec<HardwareConfig>& records) {
    int num = responseLen / sizeof(RIL_HardwareConfig);
    records.resize(num);

    RIL_HardwareConfig *rilHardwareConfig = (RIL_HardwareConfig *) response;
    for (int i = 0; i < num; i++) {
        records[i].type = (HardwareConfigType) rilHardwareConfig[i].type;
        records[i].uuid = convertCharPtrToHidlString(rilHardwareConfig[i].uuid);
        records[i].state = (HardwareConfigState) rilHardwareConfig[i].state;
        switch (rilHardwareConfig[i].type) {
            case RIL_HARDWARE_CONFIG_MODEM: {
                records[i].modem.resize(1);
                records[i].sim.resize(0);
                HardwareConfigModem *hwConfigModem = &records[i].modem[0];
                hwConfigModem->rat = rilHardwareConfig[i].cfg.modem.rat;
                hwConfigModem->maxVoice = rilHardwareConfig[i].cfg.modem.maxVoice;
                hwConfigModem->maxData = rilHardwareConfig[i].cfg.modem.maxData;
                hwConfigModem->maxStandby = rilHardwareConfig[i].cfg.modem.maxStandby;
                break;
            }

            case RIL_HARDWARE_CONFIG_SIM: {
                records[i].sim.resize(1);
                records[i].modem.resize(0);
                records[i].sim[0].modemUuid =
                        convertCharPtrToHidlString(rilHardwareConfig[i].cfg.sim.modemUuid);
                break;
            }
        }
    }
}

int radio::hardwareConfigChangedInd(int slotId,
                                    int indicationType, int token, RIL_Errno e, void *response,
                                    size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        if ((response == NULL && responseLen != 0)
                || responseLen % sizeof(RIL_HardwareConfig) != 0) {
            RLOGE("hardwareConfigChangedInd: invalid response");
            return 0;
        }

        hidl_vec<HardwareConfig> configs;
        convertRilHardwareConfigListToHal(response, responseLen, configs);

#if VDBG
        RLOGD("hardwareConfigChangedInd");
#endif
        Return<void> retStatus = radioService[slotId]->mRadioIndication->hardwareConfigChanged(
                convertIntToRadioIndicationType(indicationType), configs);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("hardwareConfigChangedInd: radioService[%d]->mRadioIndication == NULL",
                slotId);
    }

    return 0;
}

void convertRilRadioCapabilityToHal(RIL_RadioCapability& rilRc, RadioCapability& hidlRc) {
    hidlRc.session = rilRc.session;
    hidlRc.phase = (V1_0::RadioCapabilityPhase)rilRc.phase;
    hidlRc.raf = rilRc.rat;
    hidlRc.logicalModemUuid = convertCharPtrToHidlString(rilRc.logicalModemUuid);
    hidlRc.status = (V1_0::RadioCapabilityStatus)rilRc.status;
}

void convertRilRadioCapabilityToHal(void *response, size_t responseLen, RadioCapability& rc) {
    RIL_RadioCapability *rilRadioCapability = (RIL_RadioCapability *) response;
    convertRilRadioCapabilityToHal(*rilRadioCapability, rc);
}

int radio::radioCapabilityIndicationInd(int slotId,
                                        int indicationType, int token, RIL_Errno e, void *response,
                                        size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        if (response == NULL || responseLen != sizeof(RIL_RadioCapability)) {
            RLOGE("radioCapabilityIndicationInd: invalid response");
            return 0;
        }

        RadioCapability rc = {};
        convertRilRadioCapabilityToHal(response, responseLen, rc);

#if VDBG
        RLOGD("radioCapabilityIndicationInd");
#endif
        Return<void> retStatus = radioService[slotId]->mRadioIndication->radioCapabilityIndication(
                convertIntToRadioIndicationType(indicationType), rc);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("radioCapabilityIndicationInd: radioService[%d]->mRadioIndication == NULL",
                slotId);
    }

    return 0;
}

int radio::onSupplementaryServiceIndicationInd(int slotId,
                                               int indicationType, int token, RIL_Errno e,
                                               void *response, size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        if (response == NULL || responseLen != sizeof(RIL_StkCcUnsolSsResponse)) {
            RLOGE("onSupplementaryServiceIndicationInd: invalid response");
            return 0;
        }

        RIL_StkCcUnsolSsResponse *rilSsResponse = (RIL_StkCcUnsolSsResponse *) response;
        StkCcUnsolSsResult ss = {};
        ss.serviceType = (SsServiceType) rilSsResponse->serviceType;
        ss.requestType = (SsRequestType) rilSsResponse->requestType;
        ss.teleserviceType = (SsTeleserviceType) rilSsResponse->teleserviceType;
        ss.serviceClass = rilSsResponse->serviceClass;
        ss.result = (RadioError) rilSsResponse->result;

        if (isServiceTypeCfQuery(rilSsResponse->serviceType, rilSsResponse->requestType)) {
#if VDBG
            RLOGD("onSupplementaryServiceIndicationInd CF type, num of Cf elements %d",
                    rilSsResponse->cfData.numValidIndexes);
#endif
            if (rilSsResponse->cfData.numValidIndexes > NUM_SERVICE_CLASSES) {
                RLOGE("onSupplementaryServiceIndicationInd numValidIndexes is greater than "
                        "max value %d, truncating it to max value", NUM_SERVICE_CLASSES);
                rilSsResponse->cfData.numValidIndexes = NUM_SERVICE_CLASSES;
            }

            ss.cfData.resize(1);
            ss.ssInfo.resize(0);

            /* number of call info's */
            ss.cfData[0].cfInfo.resize(rilSsResponse->cfData.numValidIndexes);

            for (int i = 0; i < rilSsResponse->cfData.numValidIndexes; i++) {
                 RIL_CallForwardInfo cf = rilSsResponse->cfData.cfInfo[i];
                 CallForwardInfo *cfInfo = &ss.cfData[0].cfInfo[i];

                 cfInfo->status = (CallForwardInfoStatus) cf.status;
                 cfInfo->reason = cf.reason;
                 cfInfo->serviceClass = cf.serviceClass;
                 cfInfo->toa = cf.toa;
                 cfInfo->number = convertCharPtrToHidlString(cf.number);
                 cfInfo->timeSeconds = cf.timeSeconds;
#if VDBG
                 RLOGD("onSupplementaryServiceIndicationInd: "
                        "Data: %d,reason=%d,cls=%d,toa=%d,num=%s,tout=%d],", cf.status,
                        cf.reason, cf.serviceClass, cf.toa, (char*)cf.number, cf.timeSeconds);
#endif
            }
        } else {
            ss.ssInfo.resize(1);
            ss.cfData.resize(0);

            /* each int */
            ss.ssInfo[0].ssInfo.resize(SS_INFO_MAX);
            for (int i = 0; i < SS_INFO_MAX; i++) {
#if VDBG
                 RLOGD("onSupplementaryServiceIndicationInd: Data: %d",
                        rilSsResponse->ssInfo[i]);
#endif
                 ss.ssInfo[0].ssInfo[i] = rilSsResponse->ssInfo[i];
            }
        }

#if VDBG
        RLOGD("onSupplementaryServiceIndicationInd");
#endif
        Return<void> retStatus = radioService[slotId]->mRadioIndication->
                onSupplementaryServiceIndication(convertIntToRadioIndicationType(indicationType),
                ss);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("onSupplementaryServiceIndicationInd: "
                "radioService[%d]->mRadioIndication == NULL", slotId);
    }

    return 0;
}

int radio::stkCallControlAlphaNotifyInd(int slotId,
                                        int indicationType, int token, RIL_Errno e, void *response,
                                        size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        if (response == NULL || responseLen == 0) {
            RLOGE("stkCallControlAlphaNotifyInd: invalid response");
            return 0;
        }
#if VDBG
        RLOGD("stkCallControlAlphaNotifyInd");
#endif
        Return<void> retStatus = radioService[slotId]->mRadioIndication->stkCallControlAlphaNotify(
                convertIntToRadioIndicationType(indicationType),
                convertCharPtrToHidlString((char *) response));
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("stkCallControlAlphaNotifyInd: radioService[%d]->mRadioIndication == NULL",
                slotId);
    }

    return 0;
}

void convertRilLceDataInfoToHal(void *response, size_t responseLen, LceDataInfo& lce) {
    RIL_LceDataInfo *rilLceDataInfo = (RIL_LceDataInfo *)response;
    lce.lastHopCapacityKbps = rilLceDataInfo->last_hop_capacity_kbps;
    lce.confidenceLevel = rilLceDataInfo->confidence_level;
    lce.lceSuspended = rilLceDataInfo->lce_suspended;
}

int radio::lceDataInd(int slotId,
                      int indicationType, int token, RIL_Errno e, void *response,
                      size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        if (response == NULL || responseLen != sizeof(RIL_LceDataInfo)) {
            RLOGE("lceDataInd: invalid response");
            return 0;
        }

        LceDataInfo lce = {};
        convertRilLceDataInfoToHal(response, responseLen, lce);
#if VDBG
        RLOGD("lceDataInd");
#endif
        Return<void> retStatus = radioService[slotId]->mRadioIndication->lceData(
                convertIntToRadioIndicationType(indicationType), lce);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("lceDataInd: radioService[%d]->mRadioIndication == NULL", slotId);
    }

    return 0;
}

int radio::pcoDataInd(int slotId,
                      int indicationType, int token, RIL_Errno e, void *response,
                      size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        if (response == NULL || responseLen != sizeof(RIL_PCO_Data)) {
            RLOGE("pcoDataInd: invalid response");
            return 0;
        }

        PcoDataInfo pco = {};
        RIL_PCO_Data *rilPcoData = (RIL_PCO_Data *)response;
        pco.cid = rilPcoData->cid;
        pco.bearerProto = convertCharPtrToHidlString(rilPcoData->bearer_proto);
        pco.pcoId = rilPcoData->pco_id;
        pco.contents.setToExternal((uint8_t *) rilPcoData->contents, rilPcoData->contents_length);

#if VDBG
        RLOGD("pcoDataInd");
#endif
        Return<void> retStatus = radioService[slotId]->mRadioIndication->pcoData(
                convertIntToRadioIndicationType(indicationType), pco);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("pcoDataInd: radioService[%d]->mRadioIndication == NULL", slotId);
    }

    return 0;
}

int radio::modemResetInd(int slotId,
                         int indicationType, int token, RIL_Errno e, void *response,
                         size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        if (response == NULL || responseLen == 0) {
            RLOGE("modemResetInd: invalid response");
            return 0;
        }
#if VDBG
        RLOGD("modemResetInd");
#endif
        Return<void> retStatus = radioService[slotId]->mRadioIndication->modemReset(
                convertIntToRadioIndicationType(indicationType),
                convertCharPtrToHidlString((char *) response));
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("modemResetInd: radioService[%d]->mRadioIndication == NULL", slotId);
    }

    return 0;
}

int radio::networkScanResultInd(int slotId,
                                int indicationType, int token, RIL_Errno e, void *response,
                                size_t responseLen) {
    int ret = RIL_E_INTERNAL_ERR;
    if(radioService[slotId] != NULL) {
        ret = radioService[slotId]->networkScanResultInd(slotId,
                indicationType,
                token,
                e,
                response,
                responseLen);
    }
    return ret;
}

int RadioImpl::networkScanResultInd(int slotId,
                                int indicationType, int token, RIL_Errno e, void *response,
                                size_t responseLen) {
    if ( ( radioService[slotId]->mRadioIndicationV1_1 != NULL) ) {
#if VDBG
    RLOGD("networkScanResultInd");
#endif
        if (response == NULL || responseLen == 0) {
            RLOGE("networkScanResultInd: invalid response");
            return 0;
        }

        RIL_NetworkScanResult *networkScanResult = (RIL_NetworkScanResult *) response;

        V1_1::NetworkScanResult result;
        result.status = (V1_1::ScanStatus) networkScanResult->status;
        result.error = (RadioError) networkScanResult->error;
        Return<void> retStatus;

        convertRilCellInfoListToHal(
                slotId,
                networkScanResult->network_infos,
                networkScanResult->network_infos_length * sizeof(RIL_CellInfo_v12),
                result.networkInfos);
        retStatus = radioService[slotId]->mRadioIndicationV1_1->networkScanResult(
                convertIntToRadioIndicationType(indicationType), result);
        checkReturnStatus(retStatus);
    } else {
        RLOGE("networkScanResultInd: radioService[%d]->mRadioIndication == NULL", slotId);
    }
    return 0;
}

int radio::carrierInfoForImsiEncryption(int slotId,
                                  int indicationType, int token, RIL_Errno e, void *response,
                                  size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndicationV1_1 != NULL) {
        if (response == NULL || responseLen == 0) {
            RLOGE("carrierInfoForImsiEncryption: invalid response");
            return 0;
        }
        RLOGD("carrierInfoForImsiEncryption");
        Return<void> retStatus = radioService[slotId]->mRadioIndicationV1_1->
                carrierInfoForImsiEncryption(convertIntToRadioIndicationType(indicationType));
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        RLOGE("carrierInfoForImsiEncryption: radioService[%d]->mRadioIndicationV1_1 == NULL",
                slotId);
    }

    return 0;
}

int radio::keepaliveStatusInd(int slotId,
                         int indicationType, int token, RIL_Errno e, void *response,
                         size_t responseLen) {
#if VDBG
    RLOGD("%s(): token=%d", __FUNCTION__, token);
#endif
    if (radioService[slotId] == NULL || radioService[slotId]->mRadioIndication == NULL) {
        RLOGE("%s: radioService[%d]->mRadioIndication == NULL", __FUNCTION__, slotId);
        return 0;
    }

    auto ret = V1_1::IRadioIndication::castFrom(
        radioService[slotId]->mRadioIndication);
    if (!ret.isOk()) {
        RLOGE("%s: ret.isOk() == false for radioService[%d]", __FUNCTION__, slotId);
        return 0;
    }
    sp<V1_1::IRadioIndication> radioIndicationV1_1 = ret;

    if (response == NULL || responseLen != sizeof(V1_1::KeepaliveStatus)) {
        RLOGE("%s: invalid response", __FUNCTION__);
        return 0;
    }

    V1_1::KeepaliveStatus ks;
    convertRilKeepaliveStatusToHal(static_cast<RIL_KeepaliveStatus*>(response), ks);

    Return<void> retStatus = radioIndicationV1_1->keepaliveStatus(
            convertIntToRadioIndicationType(indicationType), ks);
    radioService[slotId]->checkReturnStatus(retStatus);
    return 0;
}

int radio::oemHookRawInd(int slotId,
                         int indicationType, int token, RIL_Errno e, void *response,
                         size_t responseLen) {
    if (!kOemHookEnabled) return 0;

    if (oemHookService[slotId] != NULL && oemHookService[slotId]->mOemHookIndication != NULL) {
        if (response == NULL || responseLen == 0) {
            RLOGE("oemHookRawInd: invalid response");
            return 0;
        }

        hidl_vec<uint8_t> data;
        data.setToExternal((uint8_t *) response, responseLen);
#if VDBG
        RLOGD("oemHookRawInd");
#endif
        Return<void> retStatus = oemHookService[slotId]->mOemHookIndication->oemHookRaw(
                convertIntToRadioIndicationType(indicationType), data);
        checkReturnStatus(slotId, retStatus, false);
    } else {
        RLOGE("oemHookRawInd: oemHookService[%d]->mOemHookIndication == NULL", slotId);
    }

    return 0;
}

QcrildServiceFactory &getQcrildServiceFactory()
{
    static QcrildServiceFactory factory{};
    return factory;
}

#ifndef QMI_RIL_UTF
extern RadioConfigFactory &getRadioConfigFactory();
#endif

void radio::registerService(RIL_RadioFunctions *callbacks, CommandInfo *commands) {
    using namespace android::hardware;
    int simCount = 1;
    QCRIL_LOG_DEBUG("registerService");
    getQcrildServiceFactory().registerRadioImpl<RadioImpl>(nullptr);
    const char *serviceNames[] = {
            android::RIL_getServiceName()
            #if (SIM_COUNT >= 2)
            , RIL2_SERVICE_NAME
            #if (SIM_COUNT >= 3)
            , RIL3_SERVICE_NAME
            #if (SIM_COUNT >= 4)
            , RIL4_SERVICE_NAME
            #endif
            #endif
            #endif
            };

    #if (SIM_COUNT >= 2)
    simCount = SIM_COUNT;
    #endif

    s_vendorFunctions = callbacks;
    s_commands = commands;
#ifndef QMI_RIL_UTF
    configureRpcThreadpool(1, true /* callerWillJoin */);
#endif
    for (int i = 0; i < simCount; i++) {
        qtimutex::QtiSharedMutex *radioServiceRwlockPtr = getRadioServiceRwlock(i);
        radioServiceRwlockPtr->lock();

        radioService[i] = getQcrildServiceFactory().getRadioImpl(static_cast<qcril_instance_id_e_type>(i));
        android::status_t status;
        if (radioService[i]) {
            radioService[i]->mSlotId = i;
            QCRIL_LOG_INFO("registerService: starting %s %s",
                    radioService[i]->getDescriptor(),
                    serviceNames[i]);
            status = radioService[i]->registerAsService(serviceNames[i]);
            if (status != android::OK) {
                QCRIL_LOG_INFO("Error registering service %s %s",
                        radioService[i]->descriptor,
                        serviceNames[i]);
            }
        } else {
            QCRIL_LOG_INFO("registerService: Unable to find implementation for IRadio");
        }
        if (kOemHookEnabled) {
            oemHookService[i] = new OemHookImpl;
            if (oemHookService[i]) {
                if (oemHookService[i]) {
                    oemHookService[i]->mSlotId = i;
                    status = oemHookService[i]->registerAsService(serviceNames[i]);
                    if (status != android::OK) {
                        QCRIL_LOG_INFO("Error registering service %s %s",
                                oemHookService[i]->descriptor,
                                serviceNames[i]);
                    }
                }
            }
        }

        radioServiceRwlockPtr->unlock();
    }
#ifndef QMI_RIL_UTF
    android::status_t status;
    if (qmi_ril_get_process_instance_id() == QCRIL_DEFAULT_INSTANCE_ID) {
        // Register RadioConfig only on default instance
        radioConfigService = getRadioConfigFactory().getRadioConfigImpl();
        if (radioConfigService) {
            status = radioConfigService->registerAsService("default");
            if (status != android::OK) {
                QCRIL_LOG_INFO("Error registering service %s %s",
                        radioConfigService->descriptor,
                        "default");
            }
        }
    }
#endif
}

void rilc_thread_pool() {
#ifndef QMI_RIL_UTF
    joinRpcThreadpool();
#endif
}

qtimutex::QtiSharedMutex * radio::getRadioServiceRwlock(int slotId) {
    qtimutex::QtiSharedMutex *radioServiceRwlockPtr = &radioServiceRwlock;

    #if (SIM_COUNT >= 2)
    if (slotId == 2) radioServiceRwlockPtr = &radioServiceRwlock2;
    #if (SIM_COUNT >= 3)
    if (slotId == 3) radioServiceRwlockPtr = &radioServiceRwlock3;
    #if (SIM_COUNT >= 4)
    if (slotId == 4) radioServiceRwlockPtr = &radioServiceRwlock4;
    #endif
    #endif
    #endif

    return radioServiceRwlockPtr;
}
::android::status_t RadioImpl::registerAsService( const std::string &serviceName) {
    QCRIL_LOG_INFO("RadioImpl[1.1]::registerAsService(%s)", serviceName.c_str());
    return V1_1::IRadio::registerAsService(serviceName);
}
const char *getDescriptor() {
    return V1_0::IRadio::descriptor;
}

int convertNwTypeToRadioAccessFamily(PreferredNetworkType nwType) {
    uint32_t radioAccessFamily = 0;
    switch (nwType) {
        case PreferredNetworkType::GSM_WCDMA:
            radioAccessFamily = QCRIL_RAF_GSM | QCRIL_RAF_WCDMA;
            break;
        case PreferredNetworkType::GSM_ONLY:
            radioAccessFamily = QCRIL_RAF_GSM;
            break;
        case PreferredNetworkType::WCDMA:
            radioAccessFamily = QCRIL_RAF_WCDMA;
            break;
        case PreferredNetworkType::GSM_WCDMA_AUTO:
            radioAccessFamily = QCRIL_RAF_GSM | QCRIL_RAF_WCDMA | QCRIL_RAF_AUTO;
            break;
        case PreferredNetworkType::CDMA_EVDO_AUTO:
            radioAccessFamily = QCRIL_RAF_CDMA | QCRIL_RAF_EVDO;
            break;
        case PreferredNetworkType::CDMA_ONLY:
            radioAccessFamily = QCRIL_RAF_CDMA;
            break;
        case PreferredNetworkType::EVDO_ONLY:
            radioAccessFamily = QCRIL_RAF_EVDO;
            break;
        case PreferredNetworkType::GSM_WCDMA_CDMA_EVDO_AUTO:
            radioAccessFamily = QCRIL_RAF_GSM | QCRIL_RAF_WCDMA | QCRIL_RAF_CDMA | QCRIL_RAF_EVDO;
            break;
        case PreferredNetworkType::LTE_CDMA_EVDO:
            radioAccessFamily = QCRIL_RAF_LTE | QCRIL_RAF_CDMA | QCRIL_RAF_EVDO;
            break;
        case PreferredNetworkType::LTE_GSM_WCDMA:
            radioAccessFamily = QCRIL_RAF_LTE | QCRIL_RAF_GSM | QCRIL_RAF_WCDMA;
            break;
        case PreferredNetworkType::LTE_CMDA_EVDO_GSM_WCDMA:
            radioAccessFamily = QCRIL_RAF_LTE | QCRIL_RAF_CDMA | QCRIL_RAF_EVDO | QCRIL_RAF_GSM | QCRIL_RAF_WCDMA;
            break;
        case PreferredNetworkType::LTE_ONLY:
            radioAccessFamily = QCRIL_RAF_LTE;
            break;
        case PreferredNetworkType::LTE_WCDMA:
            radioAccessFamily = QCRIL_RAF_LTE | QCRIL_RAF_WCDMA;
            break;
        case PreferredNetworkType::TD_SCDMA_ONLY:
            radioAccessFamily = RAF_TD_SCDMA;
            break;
        case PreferredNetworkType::TD_SCDMA_WCDMA:
            radioAccessFamily = RAF_TD_SCDMA | QCRIL_RAF_WCDMA;
            break;
        case PreferredNetworkType::TD_SCDMA_LTE:
            radioAccessFamily = RAF_TD_SCDMA | QCRIL_RAF_LTE;
            break;
        case PreferredNetworkType::TD_SCDMA_GSM:
            radioAccessFamily = RAF_TD_SCDMA | QCRIL_RAF_GSM;
            break;
        case PreferredNetworkType::TD_SCDMA_GSM_LTE:
            radioAccessFamily = RAF_TD_SCDMA | QCRIL_RAF_GSM | QCRIL_RAF_LTE;
            break;
        case PreferredNetworkType::TD_SCDMA_GSM_WCDMA:
            radioAccessFamily = RAF_TD_SCDMA | QCRIL_RAF_GSM | QCRIL_RAF_WCDMA;
            break;
        case PreferredNetworkType::TD_SCDMA_WCDMA_LTE:
            radioAccessFamily = RAF_TD_SCDMA | QCRIL_RAF_WCDMA | QCRIL_RAF_LTE;
            break;
        case PreferredNetworkType::TD_SCDMA_GSM_WCDMA_LTE:
            radioAccessFamily = RAF_TD_SCDMA | QCRIL_RAF_GSM | QCRIL_RAF_WCDMA | QCRIL_RAF_LTE;
            break;
        case PreferredNetworkType::TD_SCDMA_GSM_WCDMA_CDMA_EVDO_AUTO:
            radioAccessFamily = RAF_TD_SCDMA | QCRIL_RAF_GSM | QCRIL_RAF_WCDMA | QCRIL_RAF_CDMA | QCRIL_RAF_EVDO;
            break;
        case PreferredNetworkType::TD_SCDMA_LTE_CDMA_EVDO_GSM_WCDMA:
            radioAccessFamily = RAF_TD_SCDMA | QCRIL_RAF_LTE | QCRIL_RAF_CDMA | QCRIL_RAF_EVDO | QCRIL_RAF_GSM | QCRIL_RAF_WCDMA;
            break;
        default:
            radioAccessFamily = RAF_UNKNOWN;
            break;
    }
    return radioAccessFamily;
}

PreferredNetworkType convertRadioAccessFamilyToNwType(int radioAccessFamily) {
    PreferredNetworkType nwType = PreferredNetworkType::GSM_WCDMA;

    switch (radioAccessFamily) {
        case (QCRIL_RAF_GSM | QCRIL_RAF_WCDMA):
            nwType = PreferredNetworkType::GSM_WCDMA;
            break;
        case QCRIL_RAF_GSM:
            nwType = PreferredNetworkType::GSM_ONLY;
            break;
        case QCRIL_RAF_WCDMA:
            nwType = PreferredNetworkType::WCDMA;
            break;
        case (QCRIL_RAF_GSM | QCRIL_RAF_WCDMA | QCRIL_RAF_AUTO):
            nwType = PreferredNetworkType::GSM_WCDMA_AUTO;
            break;
        case (QCRIL_RAF_CDMA | QCRIL_RAF_EVDO):
            nwType = PreferredNetworkType::CDMA_EVDO_AUTO;
            break;
        case QCRIL_RAF_CDMA:
            nwType = PreferredNetworkType::CDMA_ONLY;
            break;
        case QCRIL_RAF_EVDO:
            nwType = PreferredNetworkType::EVDO_ONLY;
            break;
        case (QCRIL_RAF_GSM | QCRIL_RAF_WCDMA | QCRIL_RAF_CDMA | QCRIL_RAF_EVDO):
            nwType = PreferredNetworkType::GSM_WCDMA_CDMA_EVDO_AUTO;
            break;
        case (QCRIL_RAF_LTE | QCRIL_RAF_CDMA | QCRIL_RAF_EVDO):
            nwType = PreferredNetworkType::LTE_CDMA_EVDO;
            break;
        case (QCRIL_RAF_LTE | QCRIL_RAF_GSM | QCRIL_RAF_WCDMA):
            nwType = PreferredNetworkType::LTE_GSM_WCDMA;
            break;
        case (QCRIL_RAF_LTE | QCRIL_RAF_CDMA | QCRIL_RAF_EVDO | QCRIL_RAF_GSM | QCRIL_RAF_WCDMA):
            nwType = PreferredNetworkType::LTE_CMDA_EVDO_GSM_WCDMA;
            break;
        case (QCRIL_RAF_LTE):
            nwType = PreferredNetworkType::LTE_ONLY;
            break;
        case (QCRIL_RAF_LTE | QCRIL_RAF_WCDMA):
            nwType = PreferredNetworkType::LTE_WCDMA;
            break;
        case RAF_TD_SCDMA:
            nwType = PreferredNetworkType::TD_SCDMA_ONLY;
            break;
        case (RAF_TD_SCDMA | QCRIL_RAF_WCDMA):
            nwType = PreferredNetworkType::TD_SCDMA_WCDMA;
            break;
        case (RAF_TD_SCDMA | QCRIL_RAF_LTE):
            nwType = PreferredNetworkType::TD_SCDMA_LTE;
            break;
        case (RAF_TD_SCDMA | QCRIL_RAF_GSM):
            nwType = PreferredNetworkType::TD_SCDMA_GSM;
            break;
        case (RAF_TD_SCDMA | QCRIL_RAF_GSM | QCRIL_RAF_LTE):
            nwType = PreferredNetworkType::TD_SCDMA_GSM_LTE;
            break;
        case (RAF_TD_SCDMA | QCRIL_RAF_GSM | QCRIL_RAF_WCDMA):
            nwType = PreferredNetworkType::TD_SCDMA_GSM_WCDMA;
            break;
        case (RAF_TD_SCDMA | QCRIL_RAF_WCDMA | QCRIL_RAF_LTE):
            nwType = PreferredNetworkType::TD_SCDMA_WCDMA_LTE;
            break;
        case (RAF_TD_SCDMA | QCRIL_RAF_GSM | QCRIL_RAF_WCDMA | QCRIL_RAF_LTE):
            nwType = PreferredNetworkType::TD_SCDMA_GSM_WCDMA_LTE;
            break;
        case (RAF_TD_SCDMA | QCRIL_RAF_GSM | QCRIL_RAF_WCDMA | QCRIL_RAF_CDMA | QCRIL_RAF_EVDO):
            nwType = PreferredNetworkType::TD_SCDMA_GSM_WCDMA_CDMA_EVDO_AUTO;
            break;
        case (RAF_TD_SCDMA | QCRIL_RAF_LTE | QCRIL_RAF_CDMA | QCRIL_RAF_EVDO | QCRIL_RAF_GSM | QCRIL_RAF_WCDMA):
            nwType = PreferredNetworkType::TD_SCDMA_LTE_CDMA_EVDO_GSM_WCDMA;
            break;
        default:
            nwType = PreferredNetworkType::GSM_WCDMA;
            break;
    }
    return nwType;
}
