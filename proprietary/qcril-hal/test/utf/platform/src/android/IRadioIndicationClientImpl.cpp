/*===========================================================================
 *
 *    Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
 *    All Rights Reserved.
 *    Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 *===========================================================================*/

#include "platform/android/IRadioIndicationClientImpl.h"
#include "ril_utf_rild_sim.h"
#include "platform/android/SmsHidlUtil.h"
#include "platform/android/NasHidlUtil.h"

Return<void> IRadioIndicationClientImpl::newSms(V1_0::RadioIndicationType type, const hidl_vec<uint8_t>& pdu) {
    auto size = pdu.size();
    uint8_t* payload = (uint8_t*)malloc(size);
    if (payload != nullptr) {
        memcpy(payload, pdu.data(), size);
        int my_expect_slot = acquire_expectation_table();
        enqueue_ril_expect(
                payload,                    // payload
                size,
                RIL_UNSOL_RESPONSE_NEW_SMS, // msg_id
                -1,                         // serial_id
                RIL_E_SUCCESS,              // RIL error
                ril_utf_ril_unsol_response);// type
        release_expectation_table(my_expect_slot);
    }
    return Void();
}

Return<void> IRadioIndicationClientImpl::newBroadcastSms(V1_0::RadioIndicationType type,
        const hidl_vec<uint8_t>& pdu) {
    auto size = pdu.size();
    uint8_t* payload = (uint8_t*)malloc(size);
    if (payload != nullptr) {
        memcpy(payload, pdu.data(), size);
        int my_expect_slot = acquire_expectation_table();
        enqueue_ril_expect(
                payload,                    // payload
                size,
                RIL_UNSOL_RESPONSE_NEW_BROADCAST_SMS, // msg_id
                -1,                         // serial_id
                RIL_E_SUCCESS,              // RIL error
                ril_utf_ril_unsol_response);// type
        release_expectation_table(my_expect_slot);
    }
    return Void();
}

Return<void> IRadioIndicationClientImpl::imsNetworkStateChanged(V1_0::RadioIndicationType type) {
    int my_expect_slot = acquire_expectation_table();
    enqueue_ril_expect(
                nullptr,                                        // payload
                0,
                RIL_UNSOL_RESPONSE_IMS_NETWORK_STATE_CHANGED,   // msg_id
                -1,                                             // serial_id
                RIL_E_SUCCESS,                                  // RIL error
                ril_utf_ril_unsol_response);                    // type
    release_expectation_table(my_expect_slot);

    return Void();
}

Return<void> IRadioIndicationClientImpl::networkStateChanged(V1_0::RadioIndicationType type) {
    int my_expect_slot = acquire_expectation_table();
    enqueue_ril_expect(
                nullptr,                                        // payload
                0,
                RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED, // msg_id
                -1,                                             // serial_id
                RIL_E_SUCCESS,                                  // RIL error
                ril_utf_ril_unsol_response);                    // type
    release_expectation_table(my_expect_slot);

    return Void();
}

Return<void> IRadioIndicationClientImpl::simStatusChanged(V1_0::RadioIndicationType type) {
    int my_expect_slot = acquire_expectation_table();
    enqueue_ril_expect(
                nullptr,                                        // payload
                0,
                RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED,          // msg_id
                -1,                                             // serial_id
                RIL_E_SUCCESS,                                  // RIL error
                ril_utf_ril_unsol_response);                    // type
    release_expectation_table(my_expect_slot);

    return Void();
}

Return<void> IRadioIndicationClientImpl::simSmsStorageFull(V1_0::RadioIndicationType type) {
    int my_expect_slot = acquire_expectation_table();
    enqueue_ril_expect(
                nullptr,                                        // payload
                0,
                RIL_UNSOL_SIM_SMS_STORAGE_FULL,                 // msg_id
                -1,                                             // serial_id
                RIL_E_SUCCESS,                                  // RIL error
                ril_utf_ril_unsol_response);                    // type
    release_expectation_table(my_expect_slot);

    return Void();
}

Return<void> IRadioIndicationClientImpl::cdmaRuimSmsStorageFull(V1_0::RadioIndicationType type) {
    int my_expect_slot = acquire_expectation_table();
    enqueue_ril_expect(
                nullptr,                                        // payload
                0,
                RIL_UNSOL_CDMA_RUIM_SMS_STORAGE_FULL,           // msg_id
                -1,                                             // serial_id
                RIL_E_SUCCESS,                                  // RIL error
                ril_utf_ril_unsol_response);                    // type
    release_expectation_table(my_expect_slot);

    return Void();
}

Return<void> IRadioIndicationClientImpl::voiceRadioTechChanged(V1_0::RadioIndicationType type,
        V1_0::RadioTechnology rat) {
    auto payload = (uint32_t*)malloc(sizeof(uint32_t));
    if (payload != nullptr) {
        *payload = static_cast<int32_t>(rat);
        int my_expect_slot = acquire_expectation_table();
        enqueue_ril_expect(
                payload,                            // payload
                sizeof(*payload),
                RIL_UNSOL_VOICE_RADIO_TECH_CHANGED, // msg_id
                -1,                                 // serial_id
                RIL_E_SUCCESS,                      // RIL error
                ril_utf_ril_unsol_response);        // type
        release_expectation_table(my_expect_slot);
    }
    return Void();
}

Return<void> IRadioIndicationClientImpl::currentSignalStrength(V1_0::RadioIndicationType type,
        const V1_0::SignalStrength& signalStrength) {
    auto payload = (RIL_SignalStrength_v10*)malloc(sizeof(RIL_SignalStrength_v10));
    if (payload != nullptr) {
        convertHidlSignalStrengthToRil(signalStrength, *payload);

        int my_expect_slot = acquire_expectation_table();
        enqueue_ril_expect(
                payload,                            // payload
                sizeof(*payload),
                RIL_UNSOL_SIGNAL_STRENGTH,          // msg_id
                -1,                                 // serial_id
                RIL_E_SUCCESS,                      // RIL error
                ril_utf_ril_unsol_response);        // type
        release_expectation_table(my_expect_slot);
    }
    return Void();
}

Return<void> IRadioIndicationClientImpl::currentSignalStrength_1_2(V1_0::RadioIndicationType type,
        const V1_2::SignalStrength& signalStrength) {
    auto payload = (RIL_SignalStrength_v10*)malloc(sizeof(RIL_SignalStrength_v10));
    if (payload != nullptr) {
        convertHidlSignalStrengthToRil(signalStrength, *payload);

        int my_expect_slot = acquire_expectation_table();
        enqueue_ril_expect(
                payload,                            // payload
                sizeof(*payload),
                RIL_UNSOL_SIGNAL_STRENGTH,          // msg_id
                -1,                                 // serial_id
                RIL_E_SUCCESS,                      // RIL error
                ril_utf_ril_unsol_response);        // type
        release_expectation_table(my_expect_slot);
    }
    return Void();
}

Return<void> IRadioIndicationClientImpl::currentSignalStrength_1_4(V1_0::RadioIndicationType type,
        const V1_4::SignalStrength& signalStrength) {
    auto payload = (RIL_SignalStrength_v10*)malloc(sizeof(RIL_SignalStrength_v10));
    if (payload != nullptr) {
        convertHidlSignalStrengthToRil(signalStrength, *payload);

        int my_expect_slot = acquire_expectation_table();
        enqueue_ril_expect(
                payload,                            // payload
                sizeof(*payload),
                RIL_UNSOL_SIGNAL_STRENGTH,          // msg_id
                -1,                                 // serial_id
                RIL_E_SUCCESS,                      // RIL error
                ril_utf_ril_unsol_response);        // type
        release_expectation_table(my_expect_slot);
    }
    return Void();
}

Return<void> IRadioIndicationClientImpl::radioStateChanged(V1_0::RadioIndicationType type,
        V1_0::RadioState radioState) {
    auto payload = (int32_t*)malloc(sizeof(int32_t));
    if (payload != nullptr) {
        *payload = static_cast<int32_t>(radioState);
        int my_expect_slot = acquire_expectation_table();
        enqueue_ril_expect(
                payload,                            // payload
                sizeof(*payload),
                RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED, // msg_id
                -1,                                 // serial_id
                RIL_E_SUCCESS,                      // RIL error
                ril_utf_ril_unsol_response);        // type
        release_expectation_table(my_expect_slot);
    }
    return Void();
}

Return<void> IRadioIndicationClientImpl::subscriptionStatusChanged(V1_0::RadioIndicationType type,
        bool activate) {
    auto payload = (int32_t*)malloc(sizeof(int32_t));
    if (payload != nullptr) {
        *payload = static_cast<int32_t>(activate);
        int my_expect_slot = acquire_expectation_table();
        enqueue_ril_expect(
                payload,                            // payload
                sizeof(*payload),
                RIL_UNSOL_UICC_SUBSCRIPTION_STATUS_CHANGED, // msg_id
                -1,                                 // serial_id
                RIL_E_SUCCESS,                      // RIL error
                ril_utf_ril_unsol_response);        // type
        release_expectation_table(my_expect_slot);
    }
    return Void();
}

Return<void> IRadioIndicationClientImpl::restrictedStateChanged(V1_0::RadioIndicationType type,
        V1_0::PhoneRestrictedState state) {
    auto payload = (int32_t*)malloc(sizeof(int32_t));
    if (payload != nullptr) {
        *payload = static_cast<int32_t>(state);
        int my_expect_slot = acquire_expectation_table();
        enqueue_ril_expect(
                payload,                            // payload
                sizeof(*payload),
                RIL_UNSOL_RESTRICTED_STATE_CHANGED, // msg_id
                -1,                                 // serial_id
                RIL_E_SUCCESS,                      // RIL error
                ril_utf_ril_unsol_response);        // type
        release_expectation_table(my_expect_slot);
    }
    return Void();
}

Return<void> IRadioIndicationClientImpl::callStateChanged(V1_0::RadioIndicationType type) {
    int my_expect_slot = acquire_expectation_table();
    enqueue_ril_expect(
                nullptr,                                        // payload
                0,
                RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED,          // msg_id
                -1,                                             // serial_id
                RIL_E_SUCCESS,                                  // RIL error
                ril_utf_ril_unsol_response);                    // type
    release_expectation_table(my_expect_slot);

    return Void();
}

Return<void> IRadioIndicationClientImpl::indicateRingbackTone(V1_0::RadioIndicationType type, bool start) {
    ril_unsol_ringback_tone_t* payload = (ril_unsol_ringback_tone_t*)malloc(sizeof(ril_unsol_ringback_tone_t));
    if (payload != nullptr) {
    payload->start = start;
    int my_expect_slot = acquire_expectation_table();
    enqueue_ril_expect(
                payload,                                        // payload
                sizeof(ril_unsol_ringback_tone_t),
                RIL_UNSOL_RINGBACK_TONE,                        // msg_id
                -1,                                             // serial_id
                RIL_E_SUCCESS,                                  // RIL error
                ril_utf_ril_unsol_response);                    // type
    release_expectation_table(my_expect_slot);
    }

    return Void();
}

Return<void> IRadioIndicationClientImpl::callRing(V1_0::RadioIndicationType type, bool isGsm, const V1_0::CdmaSignalInfoRecord& record) {
    int my_expect_slot = acquire_expectation_table();
    enqueue_ril_expect(
                nullptr,                                        // payload
                0,
                RIL_UNSOL_CALL_RING,                            // msg_id
                -1,                                             // serial_id
                RIL_E_SUCCESS,                                  // RIL error
                ril_utf_ril_unsol_response);                    // type
    release_expectation_table(my_expect_slot);

    return Void();
}
Return<void> IRadioIndicationClientImpl::srvccStateNotify(V1_0::RadioIndicationType type, V1_0::SrvccState state) {
    RIL_SrvccState* payload = (RIL_SrvccState*)malloc(sizeof(RIL_SrvccState));
    if (payload != nullptr) {
        *payload = (RIL_SrvccState)state;
        int my_expect_slot = acquire_expectation_table();
        enqueue_ril_expect(
                    payload,                                        // payload
                    sizeof(RIL_SrvccState),
                    RIL_UNSOL_SRVCC_STATE_NOTIFY,                   // msg_id
                    -1,                                             // serial_id
                    RIL_E_SUCCESS,                                  // RIL error
                    ril_utf_ril_unsol_response);                    // type
        release_expectation_table(my_expect_slot);
    }
    return Void();
}
Return<void> IRadioIndicationClientImpl::cdmaCallWaiting(V1_0::RadioIndicationType type, const V1_0::CdmaCallWaiting& callWaitingRecord) {
    RIL_CDMA_CallWaiting_v6* payload = (RIL_CDMA_CallWaiting_v6*)malloc(sizeof(RIL_CDMA_CallWaiting_v6));
    if (payload != nullptr) {
        if (callWaitingRecord.number.empty()) {
          payload->number = strndup(callWaitingRecord.number.c_str(), callWaitingRecord.number.size()+1);
        }
        payload->numberPresentation = (int)callWaitingRecord.numberPresentation;
        if (callWaitingRecord.name.empty()) {
          payload->name = strndup(callWaitingRecord.name.c_str(), callWaitingRecord.name.size()+1);
        }
        payload->signalInfoRecord.isPresent = callWaitingRecord.signalInfoRecord.isPresent ? TRUE : FALSE;
        payload->signalInfoRecord.signalType = callWaitingRecord.signalInfoRecord.signalType;
        payload->signalInfoRecord.alertPitch = callWaitingRecord.signalInfoRecord.alertPitch;
        payload->signalInfoRecord.signal = callWaitingRecord.signalInfoRecord.signal;
        payload->number_type = (int)callWaitingRecord.numberType;
        payload->number_plan = (int)callWaitingRecord.numberPlan;

        int my_expect_slot = acquire_expectation_table();
        enqueue_ril_expect(
                    payload,                                        // payload
                    sizeof(RIL_CDMA_CallWaiting_v6),
                    RIL_UNSOL_CDMA_CALL_WAITING,                    // msg_id
                    -1,                                             // serial_id
                    RIL_E_SUCCESS,                                  // RIL error
                    ril_utf_ril_unsol_response);                    // type
        release_expectation_table(my_expect_slot);
    }
    return Void();
}
Return<void> IRadioIndicationClientImpl::cdmaOtaProvisionStatus(V1_0::RadioIndicationType type, V1_0::CdmaOtaProvisionStatus status) {
    RIL_CDMA_OTA_ProvisionStatus* payload = (RIL_CDMA_OTA_ProvisionStatus*)malloc(sizeof(RIL_CDMA_OTA_ProvisionStatus));
    if (payload != nullptr) {
        *payload = (RIL_CDMA_OTA_ProvisionStatus)status;
        int my_expect_slot = acquire_expectation_table();
        enqueue_ril_expect(
                    payload,                                        // payload
                    sizeof(RIL_CDMA_OTA_ProvisionStatus),
                    RIL_UNSOL_CDMA_OTA_PROVISION_STATUS,            // msg_id
                    -1,                                             // serial_id
                    RIL_E_SUCCESS,                                  // RIL error
                    ril_utf_ril_unsol_response);                    // type
        release_expectation_table(my_expect_slot);
    }
    return Void();
}

Return<void> IRadioIndicationClientImpl::cdmaInfoRec(V1_0::RadioIndicationType type, const V1_0::CdmaInformationRecords& records) {
    RIL_CDMA_InformationRecords* payload = (RIL_CDMA_InformationRecords*)malloc(sizeof(RIL_CDMA_InformationRecords));
    if (payload != nullptr) {
        payload->numberOfInfoRecs = records.infoRec.size();
        // TODO

        int my_expect_slot = acquire_expectation_table();
        enqueue_ril_expect(
                    payload,                                        // payload
                    sizeof(RIL_CDMA_InformationRecords),
                    RIL_UNSOL_CDMA_INFO_REC,                        // msg_id
                    -1,                                             // serial_id
                    RIL_E_SUCCESS,                                  // RIL error
                    ril_utf_ril_unsol_response);                    // type
        release_expectation_table(my_expect_slot);
    }
    return Void();
}

Return<void> IRadioIndicationClientImpl::onSupplementaryServiceIndication(V1_0::RadioIndicationType type, const V1_0::StkCcUnsolSsResult& ss) {
    RIL_StkCcUnsolSsResponse* payload = (RIL_StkCcUnsolSsResponse*)malloc(sizeof(RIL_StkCcUnsolSsResponse));
    if (payload != nullptr) {
        // TODO payload
        int my_expect_slot = acquire_expectation_table();
        enqueue_ril_expect(
                    payload,                                        // payload
                    sizeof(RIL_StkCcUnsolSsResponse),
                    RIL_UNSOL_ON_SS,                                // msg_id
                    -1,                                             // serial_id
                    RIL_E_SUCCESS,                                  // RIL error
                    ril_utf_ril_unsol_response);                    // type
        release_expectation_table(my_expect_slot);
    }
    return Void();
}
Return<void> IRadioIndicationClientImpl::stkCallControlAlphaNotify(V1_0::RadioIndicationType type, const hidl_string& alpha) {
    auto payload = (ril_unsol_stk_cc_alpha_notify_t*)malloc(sizeof(ril_unsol_stk_cc_alpha_notify_t));
    if (payload != nullptr) {
        size_t payload_len = sizeof(ril_unsol_stk_cc_alpha_notify_t);
        memset(payload, 0, payload_len);
        strlcpy(payload->alpha, alpha.c_str(), sizeof(payload->alpha));
        int my_expect_slot = acquire_expectation_table();
        enqueue_ril_expect(
                    payload,                                        // payload
                    sizeof(ril_unsol_stk_cc_alpha_notify_t),
                    RIL_UNSOL_STK_CC_ALPHA_NOTIFY,                  // msg_id
                    -1,                                             // serial_id
                    RIL_E_SUCCESS,                                  // RIL error
                    ril_utf_ril_unsol_response);                    // type
        release_expectation_table(my_expect_slot);
    }
    return Void();
}

Return<void> IRadioIndicationClientImpl::onUssd(V1_0::RadioIndicationType type, V1_0::UssdModeType modeType, const hidl_string& msg) {
    char **payload = (char **)malloc(2*sizeof(char *));
    if (payload != nullptr) {
      const char *mode = nullptr;
      switch(modeType) {
        case V1_0::UssdModeType::NOTIFY:
          mode = "0";
          break;
        case V1_0::UssdModeType::REQUEST:
          mode = "1";
          break;
        case V1_0::UssdModeType::NW_RELEASE:
          mode = "2";
          break;
        case V1_0::UssdModeType::LOCAL_CLIENT:
          mode = "3";
          break;
        case V1_0::UssdModeType::NOT_SUPPORTED:
          mode = "4";
          break;
        case V1_0::UssdModeType::NW_TIMEOUT:
          mode = "5";
          break;
      }
        if (mode) {
          int size = strlen(mode)+1;
          payload[0] = (char *)malloc(sizeof(char) * size);
          memcpy(payload[0], mode, size);
        }
        if (!msg.empty()) {
          int size = msg.size()+1;
          payload[1] = (char *)malloc(sizeof(char) * size);
          memcpy(payload[1], msg.c_str(), size);
        }
        int my_expect_slot = acquire_expectation_table();
        enqueue_ril_expect(
                    payload,                                        // payload
                    2*sizeof(char),
                    RIL_UNSOL_ON_USSD,                              // msg_id
                    -1,                                             // serial_id
                    RIL_E_SUCCESS,                                  // RIL error
                    ril_utf_ril_unsol_response);                    // type
        release_expectation_table(my_expect_slot);
    }
    return Void();
}

Return<void> IRadioIndicationClientImpl::suppSvcNotify(V1_0::RadioIndicationType type, const V1_0::SuppSvcNotification& suppSvc) {
    RIL_SuppSvcNotification *payload = (RIL_SuppSvcNotification *)malloc(sizeof(RIL_SuppSvcNotification));
    if (payload != nullptr) {
        //memset(payload, 0, sizeof(RIL_SuppSvcNotification));
        payload->notificationType = (suppSvc.isMT ? 1 : 0);  //MO=0, MT=1
        payload->code = suppSvc.code;
        payload->index = suppSvc.index;
        payload->type = suppSvc.type;
        payload->number = nullptr;
        if (!suppSvc.number.empty()) {
          payload->number = strdup(suppSvc.number.c_str());
        }
        int my_expect_slot = acquire_expectation_table();
        enqueue_ril_expect(
                    payload,                                        // payload
                    sizeof(RIL_SuppSvcNotification),
                    RIL_UNSOL_SUPP_SVC_NOTIFICATION,                // msg_id
                    -1,                                             // serial_id
                    RIL_E_SUCCESS,                                  // RIL error
                    ril_utf_ril_unsol_response);                    // type
        release_expectation_table(my_expect_slot);
    }
    return Void();
}

Return<void> IRadioIndicationClientImpl::currentEmergencyNumberList(
        V1_0::RadioIndicationType type,
        const hidl_vec<V1_4::EmergencyNumber>& emergencyNumberList) {

    auto payload = (ril_unsol_emergency_list_t*)malloc(sizeof(ril_unsol_emergency_list_t));
    size_t payload_len = sizeof(ril_unsol_emergency_list_t);
    memset(payload, 0, payload_len);
    convertToRil(emergencyNumberList, payload);
    if (payload != nullptr) {
        int my_expect_slot = acquire_expectation_table();
        enqueue_ril_expect(
                    payload,                                        // payload
                    payload_len,
                    RIL_UNSOL_EMERGENCY_NUMBERS_LIST,                        // msg_id
                    -1,                                             // serial_id
                    RIL_E_SUCCESS,                                  // RIL error
                    ril_utf_ril_unsol_response);                    // type
        release_expectation_table(my_expect_slot);
    }
    return Void();
}

Return<void> IRadioIndicationClientImpl::networkScanResult_1_4(
    ::android::hardware::radio::V1_0::RadioIndicationType type,
    const ::android::hardware::radio::V1_4::NetworkScanResult& result) {
    auto payload = (RIL_NetworkScanResult*)malloc(sizeof(RIL_NetworkScanResult));
    size_t payload_len = sizeof(RIL_NetworkScanResult);
    memset(payload, 0, payload_len);
    convertNetworkScanResultToRil(result, *payload);
    if (payload != nullptr) {
        int my_expect_slot = acquire_expectation_table();
        enqueue_ril_expect(
                    payload,                                        // payload
                    payload_len,
                    RIL_UNSOL_NETWORK_SCAN_RESULT,                  // msg_id
                    -1,                                             // serial_id
                    RIL_E_SUCCESS,                                  // RIL error
                    ril_utf_ril_unsol_response);                    // type
        release_expectation_table(my_expect_slot);
    }
    return Void();
}
