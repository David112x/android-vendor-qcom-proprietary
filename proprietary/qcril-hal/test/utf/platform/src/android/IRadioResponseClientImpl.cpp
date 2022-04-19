/*===========================================================================
 *
 *    Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
 *    All Rights Reserved.
 *    Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 *===========================================================================*/

#include "platform/android/IRadioResponseClientImpl.h"
#include "ril_utf_rild_sim.h"
#include "platform/android/SmsHidlUtil.h"
#include "platform/android/NasHidlUtil.h"
#include "platform/android/VoiceHidlUtil.h"
#include "platform/android/SimHidlUtil.h"

Return<void> IRadioResponseClientImpl::sendSmsResponse(const V1_0::RadioResponseInfo& info, const V1_0::SendSmsResult& sms) {
    // use malloc, since framework will use free() to release
    auto data = (RIL_SMS_Response*)malloc(sizeof(RIL_SMS_Response));
    if (data != nullptr) {
        convertGsmSmsHidltoRilPayload(sms, *data);

        int slot = acquire_expectation_table();
        enqueue_ril_expect(
                data,                  // payload
                sizeof(*data),
                RIL_REQUEST_SEND_SMS,   // msg_id
                info.serial,            // serial_id
                (RIL_Errno)info.error,  // RIL error
                ril_utf_ril_response);  // type

        release_expectation_table(slot);
    }
    return Void();
}

Return<void> IRadioResponseClientImpl::acknowledgeLastIncomingGsmSmsResponse(const V1_0::RadioResponseInfo& info) {
    int slot = acquire_expectation_table();
    enqueue_ril_expect(
                nullptr,                        // payload
                0,
                RIL_REQUEST_SMS_ACKNOWLEDGE,    // msg_id
                info.serial,                    // serial_id
                (RIL_Errno)info.error,          // RIL error
                ril_utf_ril_response);          // type
    release_expectation_table(slot);
    return Void();
}

Return<void> IRadioResponseClientImpl::getImsRegistrationStateResponse(const V1_0::RadioResponseInfo& info, bool isRegistered, V1_0::RadioTechnologyFamily ratFamily) {
    auto payload = (int*)malloc(sizeof(int)*2);
    if (payload != nullptr) {
        payload[0] = isRegistered ? 1 : 0;
        payload[1] = (ratFamily == V1_0::RadioTechnologyFamily::THREE_GPP) ?
            RADIO_TECH_3GPP: RADIO_TECH_3GPP2;

        int slot = acquire_expectation_table();
        enqueue_ril_expect(
                payload,                            // payload
                sizeof(int)*2,
                RIL_REQUEST_IMS_REGISTRATION_STATE, // msg_id
                info.serial,                        // serial_id
                (RIL_Errno)info.error,              // RIL error
                ril_utf_ril_response);              // type
        release_expectation_table(slot);
    }
    return Void();
}
Return<void> IRadioResponseClientImpl::setSmscAddressResponse(const V1_0::RadioResponseInfo& info) {
    int slot = acquire_expectation_table();
    enqueue_ril_expect(
                nullptr,                        // payload
                0,
                RIL_REQUEST_SET_SMSC_ADDRESS,   // msg_id
                info.serial,                    // serial_id
                (RIL_Errno)info.error,          // RIL error
                ril_utf_ril_response);          // type
    release_expectation_table(slot);
    return Void();
}
Return<void> IRadioResponseClientImpl::getSmscAddressResponse(
                                const V1_0::RadioResponseInfo& info, const hidl_string& smsc) {
    auto data = (ril_request_get_smsc_address_resp_t*)qcril_malloc(
                                                  sizeof(ril_request_get_smsc_address_resp_t));
    if(data) {
        int slot = acquire_expectation_table();
        if(smsc.size()) {
            strlcpy(data->SMSC_address, smsc.c_str(),
                    std::min(sizeof(data->SMSC_address), smsc.size()+1));
        }
        enqueue_ril_expect(
                    nullptr,                        // payload
                    0,
                    RIL_REQUEST_GET_SMSC_ADDRESS,   // msg_id
                    info.serial,                    // serial_id
                    (RIL_Errno)info.error,          // RIL error
                    ril_utf_ril_response);          // type
        release_expectation_table(slot);
    }
    return Void();
}
Return<void> IRadioResponseClientImpl::deleteSmsOnSimResponse(const V1_0::RadioResponseInfo& info) {
    int slot = acquire_expectation_table();
    enqueue_ril_expect(
                nullptr,                        // payload
                0,
                RIL_REQUEST_DELETE_SMS_ON_SIM,  // msg_id
                info.serial,                    // serial_id
                (RIL_Errno)info.error,          // RIL error
                ril_utf_ril_response);          // type
    release_expectation_table(slot);
    return Void();
}

Return<void> IRadioResponseClientImpl::deleteSmsOnRuimResponse(const V1_0::RadioResponseInfo& info) {
    int slot = acquire_expectation_table();
    enqueue_ril_expect(
                nullptr,                        // payload
                0,
                RIL_REQUEST_CDMA_DELETE_SMS_ON_RUIM, // msg_id
                info.serial,                    // serial_id
                (RIL_Errno)info.error,          // RIL error
                ril_utf_ril_response);          // type
    release_expectation_table(slot);
    return Void();
}

Return<void> IRadioResponseClientImpl::getVoiceRegistrationStateResponse(
        const V1_0::RadioResponseInfo& info, const V1_0::VoiceRegStateResult& voiceRegResponse) {
    // use malloc, since framework will use free() to release
    auto data = (RIL_VoiceRegistrationStateResponse*)malloc(sizeof(RIL_VoiceRegistrationStateResponse));
    if (data != nullptr) {
        convertHidlVoiceRegistrationStateResponseToRil(voiceRegResponse, *data);

        int slot = acquire_expectation_table();
        enqueue_ril_expect(
                data,                                 // payload
                sizeof(*data),
                RIL_REQUEST_VOICE_REGISTRATION_STATE, // msg_id
                info.serial,                          // serial_id
                (RIL_Errno)info.error,                // RIL error
                ril_utf_ril_response);                // type

        release_expectation_table(slot);
    }
    return Void();
}

Return<void> IRadioResponseClientImpl::getVoiceRegistrationStateResponse_1_2(
        const V1_0::RadioResponseInfo& info, const V1_2::VoiceRegStateResult& voiceRegResponse) {
    // use malloc, since framework will use free() to release
    auto data = (RIL_VoiceRegistrationStateResponse*)malloc(sizeof(RIL_VoiceRegistrationStateResponse));
    if (data != nullptr) {
        convertHidlVoiceRegistrationStateResponseToRil(voiceRegResponse, *data);

        int slot = acquire_expectation_table();
        enqueue_ril_expect(
                data,                                 // payload
                sizeof(*data),
                RIL_REQUEST_VOICE_REGISTRATION_STATE, // msg_id
                info.serial,                          // serial_id
                (RIL_Errno)info.error,                // RIL error
                ril_utf_ril_response);                // type

        release_expectation_table(slot);
    }
    return Void();
}

Return<void> IRadioResponseClientImpl::getDataRegistrationStateResponse(
        const V1_0::RadioResponseInfo& info, const V1_0::DataRegStateResult& dataRegResponse) {
    // use malloc, since framework will use free() to release
    auto data = (RIL_DataRegistrationStateResponse*)malloc(sizeof(RIL_DataRegistrationStateResponse));
    if (data != nullptr) {
        convertHidlDataRegistrationStateResponseToRil(dataRegResponse, *data);

        int slot = acquire_expectation_table();
        enqueue_ril_expect(
                data,                                 // payload
                sizeof(*data),
                RIL_REQUEST_DATA_REGISTRATION_STATE,  // msg_id
                info.serial,                          // serial_id
                (RIL_Errno)info.error,                // RIL error
                ril_utf_ril_response);                // type

        release_expectation_table(slot);
    }
    return Void();
}

Return<void> IRadioResponseClientImpl::getDataRegistrationStateResponse_1_2(
        const V1_0::RadioResponseInfo& info, const V1_2::DataRegStateResult& dataRegResponse) {
    // use malloc, since framework will use free() to release
    auto data = (RIL_DataRegistrationStateResponse*)malloc(sizeof(RIL_DataRegistrationStateResponse));
    if (data != nullptr) {
        convertHidlDataRegistrationStateResponseToRil(dataRegResponse, *data);

        int slot = acquire_expectation_table();
        enqueue_ril_expect(
                data,                                 // payload
                sizeof(*data),
                RIL_REQUEST_DATA_REGISTRATION_STATE,  // msg_id
                info.serial,                          // serial_id
                (RIL_Errno)info.error,                // RIL error
                ril_utf_ril_response);                // type

        release_expectation_table(slot);
    }
    return Void();
}

Return<void> IRadioResponseClientImpl::getDataRegistrationStateResponse_1_4(
        const V1_0::RadioResponseInfo& info, const V1_4::DataRegStateResult& dataRegResponse) {
    // use malloc, since framework will use free() to release
    auto data = (RIL_DataRegistrationStateResponse*)malloc(sizeof(RIL_DataRegistrationStateResponse));
    if (data != nullptr) {
        convertHidlDataRegistrationStateResponseToRil_1_4(dataRegResponse, *data);

        int slot = acquire_expectation_table();
        enqueue_ril_expect(
                data,                                 // payload
                sizeof(*data),
                RIL_REQUEST_DATA_REGISTRATION_STATE,  // msg_id
                info.serial,                          // serial_id
                (RIL_Errno)info.error,                // RIL error
                ril_utf_ril_response);                // type

        release_expectation_table(slot);
    }
    return Void();
}

Return<void> IRadioResponseClientImpl::getOperatorResponse(
        const V1_0::RadioResponseInfo& info, const hidl_string& longName,
        const hidl_string& shortName, const hidl_string& numeric) {
    auto data = convertHidlOperatorReponseToRil(longName, shortName, numeric);
    if (data != nullptr) {
        int slot = acquire_expectation_table();
        enqueue_ril_expect(
                data,                                 // payload
                sizeof(*data),
                RIL_REQUEST_OPERATOR,                 // msg_id
                info.serial,                          // serial_id
                (RIL_Errno)info.error,                // RIL error
                ril_utf_ril_response);                // type

        release_expectation_table(slot);
    }
    return Void();
}

Return<void> IRadioResponseClientImpl::getAvailableNetworksResponse(
        const V1_0::RadioResponseInfo& info, const hidl_vec<V1_0::OperatorInfo>& networkInfos) {
    auto data = convertHidlLegacyNetworkscanResponseToRil(networkInfos);
    if (data != nullptr) {
        int slot = acquire_expectation_table();
        enqueue_ril_expect(
                data,                                 // payload
                sizeof(*data),
                RIL_REQUEST_QUERY_AVAILABLE_NETWORKS, // msg_id
                info.serial,                          // serial_id
                (RIL_Errno)info.error,                // RIL error
                ril_utf_ril_response);                // type

        release_expectation_table(slot);
    }
    return Void();
}

Return<void> IRadioResponseClientImpl::getNetworkSelectionModeResponse(
        const V1_0::RadioResponseInfo& info, bool manual) {
    int* data = nullptr;
    int  data_len = 0;

    if (info.error == V1_0::RadioError::NONE) {
        data = (int*)malloc(sizeof(int));
        if (data == nullptr) return Void();
        *data = manual ? 1 : 0;
        data_len = sizeof(*data);
    }

    int slot = acquire_expectation_table();
    enqueue_ril_expect(
                data,                           // payload
                data_len,
                RIL_REQUEST_QUERY_NETWORK_SELECTION_MODE,
                info.serial,                    // serial_id
                (RIL_Errno)info.error,          // RIL error
                ril_utf_ril_response);          // type
    release_expectation_table(slot);
    return Void();
}

Return<void> IRadioResponseClientImpl::setBandModeResponse(
        const V1_0::RadioResponseInfo& info) {

    int slot = acquire_expectation_table();
    enqueue_ril_expect(
                nullptr,                        // payload
                0,
                RIL_REQUEST_SET_BAND_MODE,
                info.serial,                    // serial_id
                (RIL_Errno)info.error,          // RIL error
                ril_utf_ril_response);          // type
    release_expectation_table(slot);
    return Void();
}

Return<void> IRadioResponseClientImpl::getAvailableBandModesResponse(
        const V1_0::RadioResponseInfo& info, const hidl_vec<V1_0::RadioBandMode>& bandModes) {

    int num_bands = bandModes.size();
    size_t payload_len = 0;
    int *payload = nullptr;
    if (num_bands > 0) {
        payload = (int*)malloc(sizeof(int) * (num_bands + 1));
        payload_len = sizeof(int) * (num_bands + 1);
        memset(payload, 0, payload_len);
        payload[0] = num_bands;

        for(int i=0; i < num_bands; ++i) {
            payload[i+1] = static_cast<int>(bandModes[i]);
        }
    }

    int slot = acquire_expectation_table();
    enqueue_ril_expect(
                payload,                           // payload
                payload_len,
                RIL_REQUEST_QUERY_AVAILABLE_BAND_MODE,
                info.serial,                    // serial_id
                (RIL_Errno)info.error,          // RIL error
                ril_utf_ril_response);          // type
    release_expectation_table(slot);
    return Void();
}

Return<void> IRadioResponseClientImpl::setPreferredNetworkTypeResponse(const V1_0::RadioResponseInfo& info) {
    int slot = acquire_expectation_table();
    enqueue_ril_expect(
                nullptr,                        // payload
                0,
                RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE,    // msg_id
                info.serial,                    // serial_id
                (RIL_Errno)info.error,          // RIL error
                ril_utf_ril_response);          // type
    release_expectation_table(slot);
    return Void();
}

Return<void> IRadioResponseClientImpl::getPreferredNetworkTypeResponse(const V1_0::RadioResponseInfo& info,
        V1_0::PreferredNetworkType nwType) {
    int* data = nullptr;
    int  data_len = 0;

    // No matter failure or success, the HIDL interface will give the nw_type.
    // But the legacy interface UTF is using expects null payload in case of failure
    if (info.error == V1_0::RadioError::NONE) {
        data = (int*)malloc(sizeof(int));
        if (data == nullptr) return Void();
        *data = (int)nwType;
        data_len = sizeof(*data);
    }

    int slot = acquire_expectation_table();
    enqueue_ril_expect(
                data,                           // payload
                data_len,
                RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE,    // msg_id
                info.serial,                    // serial_id
                (RIL_Errno)info.error,          // RIL error
                ril_utf_ril_response);          // type
    release_expectation_table(slot);
    return Void();
}

Return<void> IRadioResponseClientImpl::getSignalStrengthResponse(const V1_0::RadioResponseInfo& info,
        const V1_0::SignalStrength& sigStrength) {
    // use malloc, since framework will use free() to release
    auto data = (RIL_SignalStrength_v10*)malloc(sizeof(RIL_SignalStrength_v10));
    if (data != nullptr) {
        convertHidlSignalStrengthToRil(sigStrength, *data);

        int slot = acquire_expectation_table();
        enqueue_ril_expect(
                    data,                           // payload
                    sizeof(*data),
                    RIL_REQUEST_SIGNAL_STRENGTH,
                    info.serial,                    // serial_id
                    (RIL_Errno)info.error,          // RIL error
                    ril_utf_ril_response);          // type
        release_expectation_table(slot);
    }
    return Void();
}

Return<void> IRadioResponseClientImpl::getSignalStrengthResponse_1_2(const V1_0::RadioResponseInfo& info,
        const V1_2::SignalStrength& sigStrength) {
    // use malloc, since framework will use free() to release
    auto data = (RIL_SignalStrength_v10*)malloc(sizeof(RIL_SignalStrength_v10));
    if (data != nullptr) {
        convertHidlSignalStrengthToRil(sigStrength, *data);

        int slot = acquire_expectation_table();
        enqueue_ril_expect(
                    data,                           // payload
                    sizeof(*data),
                    RIL_REQUEST_SIGNAL_STRENGTH,
                    info.serial,                    // serial_id
                    (RIL_Errno)info.error,          // RIL error
                    ril_utf_ril_response);          // type
        release_expectation_table(slot);
    }
    return Void();
}

Return<void> IRadioResponseClientImpl::getSignalStrengthResponse_1_4(const V1_0::RadioResponseInfo& info,
        const V1_4::SignalStrength& sigStrength) {
    // use malloc, since framework will use free() to release
    auto data = (RIL_SignalStrength_v10*)malloc(sizeof(RIL_SignalStrength_v10));
    if (data != nullptr) {
        convertHidlSignalStrengthToRil(sigStrength, *data);

        int slot = acquire_expectation_table();
        enqueue_ril_expect(
                    data,                           // payload
                    sizeof(*data),
                    RIL_REQUEST_SIGNAL_STRENGTH,
                    info.serial,                    // serial_id
                    (RIL_Errno)info.error,          // RIL error
                    ril_utf_ril_response);          // type
        release_expectation_table(slot);
    }
    return Void();
}

Return<void> IRadioResponseClientImpl::getVoiceRadioTechnologyResponse(const V1_0::RadioResponseInfo& info,
        V1_0::RadioTechnology rat) {
    auto data = (int*)malloc(sizeof(int));
    if (data != nullptr) {
        *data = static_cast<int>(rat);

        int slot = acquire_expectation_table();
        enqueue_ril_expect(
                    data,                           // payload
                    sizeof(*data),
                    RIL_REQUEST_SIGNAL_STRENGTH,
                    info.serial,                    // serial_id
                    (RIL_Errno)info.error,          // RIL error
                    ril_utf_ril_response);          // type
        release_expectation_table(slot);
    }
    return Void();
}

Return<void> IRadioResponseClientImpl::setCdmaSubscriptionSourceResponse(const V1_0::RadioResponseInfo& info) {
    int slot = acquire_expectation_table();
    enqueue_ril_expect(
                nullptr,                        // payload
                0,
                RIL_REQUEST_CDMA_SET_SUBSCRIPTION_SOURCE, // msg_id
                info.serial,                    // serial_id
                (RIL_Errno)info.error,          // RIL error
                ril_utf_ril_response);          // type
    release_expectation_table(slot);
    return Void();
}

Return<void> IRadioResponseClientImpl::setCdmaRoamingPreferenceResponse(const V1_0::RadioResponseInfo& info) {
    int slot = acquire_expectation_table();
    enqueue_ril_expect(
                nullptr,                        // payload
                0,
                RIL_REQUEST_CDMA_SET_ROAMING_PREFERENCE, // msg_id
                info.serial,                    // serial_id
                (RIL_Errno)info.error,          // RIL error
                ril_utf_ril_response);          // type
    release_expectation_table(slot);
    return Void();
}

Return<void> IRadioResponseClientImpl::getCdmaRoamingPreferenceResponse(const V1_0::RadioResponseInfo& info, V1_0::CdmaRoamingType type) {
    auto data = (ril_request_cdma_query_roaming_preference_resp_t*)malloc(sizeof(ril_request_cdma_query_roaming_preference_resp_t));
    if (data != nullptr) {
        data->roaming_preference = static_cast<ril_cdma_roaming_preference_enum_type>(type);

        int slot = acquire_expectation_table();
        enqueue_ril_expect(
                    data,                           // payload
                    sizeof(*data),
                    RIL_REQUEST_CDMA_QUERY_ROAMING_PREFERENCE,
                    info.serial,                    // serial_id
                    (RIL_Errno)info.error,          // RIL error
                    ril_utf_ril_response);          // type
        release_expectation_table(slot);
    }
    return Void();
}

Return<void> IRadioResponseClientImpl::getCdmaSubscriptionSourceResponse(const V1_0::RadioResponseInfo& info, V1_0::CdmaSubscriptionSource source) {
    auto data = (RIL_CdmaSubscriptionSource*)malloc(sizeof(RIL_CdmaSubscriptionSource));
    if (data != nullptr) {
        *data = static_cast<RIL_CdmaSubscriptionSource>(source);

        int slot = acquire_expectation_table();
        enqueue_ril_expect(
                    data,                           // payload
                    sizeof(*data),
                    RIL_REQUEST_CDMA_GET_SUBSCRIPTION_SOURCE,
                    info.serial,                    // serial_id
                    (RIL_Errno)info.error,          // RIL error
                    ril_utf_ril_response);          // type
        release_expectation_table(slot);
    }
    return Void();
}

Return<void> IRadioResponseClientImpl::requestShutdownResponse(const V1_0::RadioResponseInfo& info) {
    int slot = acquire_expectation_table();
    enqueue_ril_expect(
                nullptr,                        // payload
                0,
                RIL_REQUEST_SHUTDOWN,           // msg_id
                info.serial,                    // serial_id
                (RIL_Errno)info.error,          // RIL error
                ril_utf_ril_response);          // type
    release_expectation_table(slot);
    return Void();
}

Return<void> IRadioResponseClientImpl::dialResponse(const V1_0::RadioResponseInfo& info) {
    int slot = acquire_expectation_table();
    enqueue_ril_expect(
                nullptr,                        // payload
                0,
                RIL_REQUEST_DIAL,               // msg_id
                info.serial,                    // serial_id
                (RIL_Errno)info.error,          // RIL error
                ril_utf_ril_response);          // type
    release_expectation_table(slot);
    return Void();
}

Return<void> IRadioResponseClientImpl::hangupConnectionResponse(const V1_0::RadioResponseInfo& info) {
    int slot = acquire_expectation_table();
    enqueue_ril_expect(
                nullptr,                        // payload
                0,
                RIL_REQUEST_HANGUP,             // msg_id
                info.serial,                    // serial_id
                (RIL_Errno)info.error,          // RIL error
                ril_utf_ril_response);          // type
    release_expectation_table(slot);
    return Void();
}

Return<void> IRadioResponseClientImpl::acceptCallResponse(const V1_0::RadioResponseInfo& info) {
    int slot = acquire_expectation_table();
    enqueue_ril_expect(
                nullptr,                        // payload
                0,
                RIL_REQUEST_ANSWER,             // msg_id
                info.serial,                    // serial_id
                (RIL_Errno)info.error,          // RIL error
                ril_utf_ril_response);          // type
    release_expectation_table(slot);
    return Void();
}
Return<void> IRadioResponseClientImpl::hangupWaitingOrBackgroundResponse(const V1_0::RadioResponseInfo& info) {
    int slot = acquire_expectation_table();
    enqueue_ril_expect(
                nullptr,                        // payload
                0,
                RIL_REQUEST_HANGUP_WAITING_OR_BACKGROUND, // msg_id
                info.serial,                    // serial_id
                (RIL_Errno)info.error,          // RIL error
                ril_utf_ril_response);          // type
    release_expectation_table(slot);
    return Void();
}
Return<void> IRadioResponseClientImpl::hangupForegroundResumeBackgroundResponse(const V1_0::RadioResponseInfo& info) {
    int slot = acquire_expectation_table();
    enqueue_ril_expect(
                nullptr,                        // payload
                0,
                RIL_REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND, // msg_id
                info.serial,                    // serial_id
                (RIL_Errno)info.error,          // RIL error
                ril_utf_ril_response);          // type
    release_expectation_table(slot);
    return Void();
}
Return<void> IRadioResponseClientImpl::rejectCallResponse(const V1_0::RadioResponseInfo& info) {
    int slot = acquire_expectation_table();
    enqueue_ril_expect(
                nullptr,                        // payload
                0,
                RIL_REQUEST_UDUB,               // msg_id
                info.serial,                    // serial_id
                (RIL_Errno)info.error,          // RIL error
                ril_utf_ril_response);          // type
    release_expectation_table(slot);
    return Void();
}
Return<void> IRadioResponseClientImpl::switchWaitingOrHoldingAndActiveResponse(const V1_0::RadioResponseInfo& info) {
    int slot = acquire_expectation_table();
    enqueue_ril_expect(
                nullptr,                        // payload
                0,
                RIL_REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE, // msg_id
                info.serial,                    // serial_id
                (RIL_Errno)info.error,          // RIL error
                ril_utf_ril_response);          // type
    release_expectation_table(slot);
    return Void();
}
Return<void> IRadioResponseClientImpl::conferenceResponse(const V1_0::RadioResponseInfo& info) {
    int slot = acquire_expectation_table();
    enqueue_ril_expect(
                nullptr,                        // payload
                0,
                RIL_REQUEST_CONFERENCE,         // msg_id
                info.serial,                    // serial_id
                (RIL_Errno)info.error,          // RIL error
                ril_utf_ril_response);          // type
    release_expectation_table(slot);
    return Void();
}
Return<void> IRadioResponseClientImpl::sendDtmfResponse(const V1_0::RadioResponseInfo& info) {
    int slot = acquire_expectation_table();
    enqueue_ril_expect(
                nullptr,                        // payload
                0,
                RIL_REQUEST_DTMF,               // msg_id
                info.serial,                    // serial_id
                (RIL_Errno)info.error,          // RIL error
                ril_utf_ril_response);          // type
    release_expectation_table(slot);
    return Void();
}
Return<void> IRadioResponseClientImpl::startDtmfResponse(const V1_0::RadioResponseInfo& info) {
    int slot = acquire_expectation_table();
    enqueue_ril_expect(
                nullptr,                        // payload
                0,
                RIL_REQUEST_DTMF_START,         // msg_id
                info.serial,                    // serial_id
                (RIL_Errno)info.error,          // RIL error
                ril_utf_ril_response);          // type
    release_expectation_table(slot);
    return Void();
}

Return<void> IRadioResponseClientImpl::stopDtmfResponse(const V1_0::RadioResponseInfo& info) {
    int slot = acquire_expectation_table();
    enqueue_ril_expect(
                nullptr,                        // payload
                0,
                RIL_REQUEST_DTMF_STOP,          // msg_id
                info.serial,                    // serial_id
                (RIL_Errno)info.error,          // RIL error
                ril_utf_ril_response);          // type
    release_expectation_table(slot);
    return Void();
}

Return<void> IRadioResponseClientImpl::getClirResponse(const V1_0::RadioResponseInfo& info, int32_t n, int32_t m) {
    auto payload = (ril_request_get_clir_resp_t *)malloc(sizeof(ril_request_get_clir_resp_t));
    if (payload != nullptr) {
        payload->n_parameter = n;
        payload->m_parameter = m;
        int slot = acquire_expectation_table();
        enqueue_ril_expect(
                    payload,                        // payload
                    sizeof(ril_request_get_clir_resp_t),
                    RIL_REQUEST_GET_CLIR,           // msg_id
                    info.serial,                    // serial_id
                    (RIL_Errno)info.error,          // RIL error
                    ril_utf_ril_response);          // type
        release_expectation_table(slot);
    }
    return Void();
}

Return<void> IRadioResponseClientImpl::setClirResponse(const V1_0::RadioResponseInfo& info) {
    int slot = acquire_expectation_table();
    enqueue_ril_expect(
                nullptr,                        // payload
                0,
                RIL_REQUEST_SET_CLIR,           // msg_id
                info.serial,                    // serial_id
                (RIL_Errno)info.error,          // RIL error
                ril_utf_ril_response);          // type
    release_expectation_table(slot);
    return Void();
}

Return<void> IRadioResponseClientImpl::getMuteResponse(const V1_0::RadioResponseInfo& info, bool enable) {
    auto payload = (ril_request_mute_t *)malloc(sizeof(ril_request_mute_t));
    if (payload != nullptr) {
        payload->enable = enable;
        int slot = acquire_expectation_table();
        enqueue_ril_expect(
                    payload,                        // payload
                    sizeof(ril_request_mute_t),
                    RIL_REQUEST_GET_MUTE,           // msg_id
                    info.serial,                    // serial_id
                    (RIL_Errno)info.error,          // RIL error
                    ril_utf_ril_response);          // type
        release_expectation_table(slot);
    }
    return Void();
}

Return<void> IRadioResponseClientImpl::setMuteResponse(const V1_0::RadioResponseInfo& info) {
    int slot = acquire_expectation_table();
    enqueue_ril_expect(
                nullptr,                        // payload
                0,
                RIL_REQUEST_SET_MUTE,           // msg_id
                info.serial,                    // serial_id
                (RIL_Errno)info.error,          // RIL error
                ril_utf_ril_response);          // type
    release_expectation_table(slot);
    return Void();
}

Return<void> IRadioResponseClientImpl::setCellInfoListRateResponse(const V1_0::RadioResponseInfo& info) {
    int slot = acquire_expectation_table();
    enqueue_ril_expect(
                nullptr,                        // payload
                0,
                RIL_REQUEST_SET_UNSOL_CELL_INFO_LIST_RATE, // msg_id
                info.serial,                    // serial_id
                (RIL_Errno)info.error,          // RIL error
                ril_utf_ril_response);          // type
    release_expectation_table(slot);
    return Void();
}

Return<void> IRadioResponseClientImpl::getClipResponse(const V1_0::RadioResponseInfo& info, V1_0::ClipStatus status) {
    auto payload = (int *)malloc(sizeof(int));
    if (payload != nullptr) {
      if (status == V1_0::ClipStatus::CLIP_PROVISIONED) {
        *payload = 0;
      } else if (status == V1_0::ClipStatus::CLIP_UNPROVISIONED) {
        *payload = 1;
      } else {
        *payload = 2;
      }
    int slot = acquire_expectation_table();
    enqueue_ril_expect(
                payload,                        // payload
                sizeof(int),
                RIL_REQUEST_QUERY_CLIP,         // msg_id
                info.serial,                    // serial_id
                (RIL_Errno)info.error,          // RIL error
                ril_utf_ril_response);          // type
    release_expectation_table(slot);
    }
    return Void();
}

Return<void> IRadioResponseClientImpl::getCellInfoListResponse(
    const V1_0::RadioResponseInfo& info, const hidl_vec<V1_0::CellInfo>& cellInfo) {
  size_t payload_len = cellInfo.size() * sizeof(RIL_CellInfo_v12);
  auto payload = (RIL_CellInfo_v12*)malloc(payload_len);
  if (payload != nullptr) {
    memset(payload, 0x0, payload_len);
    // TODO copy payload
    int slot = acquire_expectation_table();
    enqueue_ril_expect(payload,  // payload
                       sizeof(RIL_CellInfo_v12),
                       RIL_REQUEST_GET_CELL_INFO_LIST,  // msg_id
                       info.serial,                     // serial_id
                       (RIL_Errno)info.error,           // RIL error
                       ril_utf_ril_response);           // type
    release_expectation_table(slot);
  }
  return Void();
}

Return<void> IRadioResponseClientImpl::getCellInfoListResponse_1_2(
    const V1_0::RadioResponseInfo& info, const hidl_vec<V1_2::CellInfo>& cellInfo) {
  size_t payload_len = cellInfo.size() * sizeof(RIL_CellInfo_v12);
  auto payload = (RIL_CellInfo_v12*)malloc(payload_len);
  if (payload != nullptr) {
    memset(payload, 0x0, payload_len);
    // TODO copy payload
    int slot = acquire_expectation_table();
    enqueue_ril_expect(payload,  // payload
                       sizeof(RIL_CellInfo_v12),
                       RIL_REQUEST_GET_CELL_INFO_LIST,  // msg_id
                       info.serial,                     // serial_id
                       (RIL_Errno)info.error,           // RIL error
                       ril_utf_ril_response);           // type
    release_expectation_table(slot);
  }
  return Void();
}

Return<void> IRadioResponseClientImpl::getCellInfoListResponse_1_4(
    const ::android::hardware::radio::V1_0::RadioResponseInfo& info,
    const ::android::hardware::hidl_vec<::android::hardware::radio::V1_4::CellInfo>& cellInfo) {
  size_t payload_len = cellInfo.size() * sizeof(RIL_CellInfo_v12);
  auto payload = (RIL_CellInfo_v12*)malloc(payload_len);
  if (payload != nullptr) {
    memset(payload, 0x0, payload_len);
    // TODO copy payload
  }
  int slot = acquire_expectation_table();
  enqueue_ril_expect(payload,  // payload
                     sizeof(RIL_CellInfo_v12),
                     RIL_REQUEST_GET_CELL_INFO_LIST,  // msg_id
                     info.serial,                     // serial_id
                     (RIL_Errno)info.error,           // RIL error
                     ril_utf_ril_response);           // type
  release_expectation_table(slot);
  return Void();
}

Return<void> IRadioResponseClientImpl::separateConnectionResponse(const V1_0::RadioResponseInfo& info) {
    int slot = acquire_expectation_table();
    enqueue_ril_expect(
                nullptr,                        // payload
                0,
                RIL_REQUEST_SEPARATE_CONNECTION, // msg_id
                info.serial,                    // serial_id
                (RIL_Errno)info.error,          // RIL error
                ril_utf_ril_response);          // type
    release_expectation_table(slot);
    return Void();
}

Return<void> IRadioResponseClientImpl::explicitCallTransferResponse(const V1_0::RadioResponseInfo& info) {
    int slot = acquire_expectation_table();
    enqueue_ril_expect(
                nullptr,                        // payload
                0,
                RIL_REQUEST_EXPLICIT_CALL_TRANSFER, // msg_id
                info.serial,                    // serial_id
                (RIL_Errno)info.error,          // RIL error
                ril_utf_ril_response);          // type
    release_expectation_table(slot);
    return Void();
}

Return<void> IRadioResponseClientImpl::getLastCallFailCauseResponse(const V1_0::RadioResponseInfo& info, const V1_0::LastCallFailCauseInfo& failCauseinfo) {
    auto payload = (RIL_LastCallFailCauseInfo *)malloc(sizeof(RIL_LastCallFailCauseInfo));
    if (payload != nullptr) {
        payload->cause_code = (RIL_LastCallFailCause)failCauseinfo.causeCode;
        if (!failCauseinfo.vendorCause.empty()) {
            payload->vendor_cause = strndup(failCauseinfo.vendorCause.c_str(), failCauseinfo.vendorCause.size()+1);
        }

        int slot = acquire_expectation_table();
        enqueue_ril_expect(
                payload,                            // payload
                sizeof(RIL_LastCallFailCauseInfo),
                RIL_REQUEST_LAST_CALL_FAIL_CAUSE,   // msg_id
                info.serial,                        // serial_id
                (RIL_Errno)info.error,              // RIL error
                ril_utf_ril_response);              // type
        release_expectation_table(slot);
    }
    return Void();
}

Return<void> IRadioResponseClientImpl::getCurrentCallsResponse(const V1_0::RadioResponseInfo& info, const hidl_vec<V1_0::Call>& calls) {
    int num_calls = calls.size();
    RIL_Call **payload = nullptr;
    size_t payload_len = 0;
    if (num_calls > 0) {
      payload = (RIL_Call **)malloc(sizeof(RIL_Call*) * num_calls);
      payload_len = sizeof(RIL_Call*)*num_calls;

      for (int i = 0; i < num_calls; ++i) {
        payload[i] = (RIL_Call*)malloc(sizeof(RIL_Call));
        memset(payload[i], 0, sizeof(RIL_Call));
        payload[i]->state = (RIL_CallState)(calls[i].state);
        payload[i]->index = calls[i].index;
        payload[i]->toa = calls[i].toa;
        payload[i]->isMpty = calls[i].isMpty;
        payload[i]->isMT = calls[i].isMT;
        payload[i]->als = calls[i].als;
        payload[i]->isVoice = calls[i].isVoice;
        payload[i]->isVoicePrivacy = calls[i].isVoicePrivacy;
        payload[i]->number = strndup(calls[i].number.c_str(), calls[i].number.size()+1);
        payload[i]->numberPresentation = (int)calls[i].numberPresentation;
        payload[i]->name = strndup(calls[i].name.c_str(), calls[i].name.size()+1);
        payload[i]->namePresentation = (int)calls[i].namePresentation;
        // TODO UusInfo
      }
    }
    int slot = acquire_expectation_table();
    enqueue_ril_expect(
                payload,                        // payload
                payload_len,
                RIL_REQUEST_GET_CURRENT_CALLS,  // msg_id
                info.serial,                    // serial_id
                (RIL_Errno)info.error,          // RIL error
                ril_utf_ril_response);          // type
    release_expectation_table(slot);
    return Void();
}

Return<void> IRadioResponseClientImpl::getCurrentCallsResponse_1_2(const V1_0::RadioResponseInfo& info, const hidl_vec<V1_2::Call>& calls) {
    int num_calls = calls.size();
    RIL_Call **payload = nullptr;
    size_t payload_len = 0;
    if (num_calls > 0) {
      payload = (RIL_Call **)malloc(sizeof(RIL_Call*) * num_calls);
      payload_len = sizeof(RIL_Call*)*num_calls;

      for (int i = 0; i < num_calls; ++i) {
        payload[i] = (RIL_Call*)malloc(sizeof(RIL_Call));
        memset(payload[i], 0, sizeof(RIL_Call));
        payload[i]->state = (RIL_CallState)(calls[i].base.state);
        payload[i]->index = calls[i].base.index;
        payload[i]->toa = calls[i].base.toa;
        payload[i]->isMpty = calls[i].base.isMpty;
        payload[i]->isMT = calls[i].base.isMT;
        payload[i]->als = calls[i].base.als;
        payload[i]->isVoice = calls[i].base.isVoice;
        payload[i]->isVoicePrivacy = calls[i].base.isVoicePrivacy;
        payload[i]->number = strndup(calls[i].base.number.c_str(), calls[i].base.number.size()+1);
        payload[i]->numberPresentation = (int)calls[i].base.numberPresentation;
        payload[i]->name = strndup(calls[i].base.name.c_str(), calls[i].base.name.size()+1);
        payload[i]->namePresentation = (int)calls[i].base.namePresentation;
        // TODO UusInfo
        payload[i]->audioQuality = (RIL_AudioQuality)calls[i].audioQuality;
      }
    }
    int slot = acquire_expectation_table();
    enqueue_ril_expect(
                payload,                        // payload
                payload_len,
                RIL_REQUEST_GET_CURRENT_CALLS,  // msg_id
                info.serial,                    // serial_id
                (RIL_Errno)info.error,          // RIL error
                ril_utf_ril_response);          // type
    release_expectation_table(slot);
    return Void();
}

Return<void> IRadioResponseClientImpl::sendUssdResponse(const V1_0::RadioResponseInfo& info) {
    int slot = acquire_expectation_table();
    enqueue_ril_expect(
                nullptr,                        // payload
                0,
                RIL_REQUEST_SEND_USSD,          // msg_id
                info.serial,                    // serial_id
                (RIL_Errno)info.error,          // RIL error
                ril_utf_ril_response);          // type
    release_expectation_table(slot);
    return Void();
}

Return<void> IRadioResponseClientImpl::cancelPendingUssdResponse(const V1_0::RadioResponseInfo& info) {
    int slot = acquire_expectation_table();
    enqueue_ril_expect(
                nullptr,                        // payload
                0,
                RIL_REQUEST_CANCEL_USSD,        // msg_id
                info.serial,                    // serial_id
                (RIL_Errno)info.error,          // RIL error
                ril_utf_ril_response);          // type
    release_expectation_table(slot);
    return Void();
}

Return<void> IRadioResponseClientImpl::getCallForwardStatusResponse(const V1_0::RadioResponseInfo& info, const hidl_vec<V1_0::CallForwardInfo>& callForwardInfos) {
  RIL_CallForwardInfo** payload = nullptr;
  int num_info = callForwardInfos.size();
  int size = num_info * sizeof(RIL_CallForwardInfo *);
  if (num_info) {
    payload = (RIL_CallForwardInfo **)malloc(sizeof(RIL_CallForwardInfo *) * num_info);
    for (int i = 0; i < num_info; ++i) {
      payload[i] = (RIL_CallForwardInfo *)malloc(sizeof(RIL_CallForwardInfo));
      convertCallForwardInfotoUtf(*(payload[i]), callForwardInfos[i]);
    }
  }

  int slot = acquire_expectation_table();
  enqueue_ril_expect(payload,               // payload
                     size,
                     RIL_REQUEST_QUERY_CALL_FORWARD_STATUS,  // msg_id
                     info.serial,           // serial_id
                     (RIL_Errno)info.error, // RIL error
                     ril_utf_ril_response); // type
  release_expectation_table(slot);
  return Void();
}

Return<void> IRadioResponseClientImpl::setCallForwardResponse(const V1_0::RadioResponseInfo& info) {
    int slot = acquire_expectation_table();
    enqueue_ril_expect(
                nullptr,                        // payload
                0,
                RIL_REQUEST_SET_CALL_FORWARD,   // msg_id
                info.serial,                    // serial_id
                (RIL_Errno)info.error,          // RIL error
                ril_utf_ril_response);          // type
    release_expectation_table(slot);
    return Void();
}

Return<void> IRadioResponseClientImpl::getCallWaitingResponse(const V1_0::RadioResponseInfo& info, bool enable, int32_t serviceClass) {
    auto payload = (ril_request_query_call_waiting_resp_t*)malloc(sizeof(ril_request_query_call_waiting_resp_t));
    if (payload != nullptr) {
        payload->status = (ril_call_waiting_status_enum_type)enable;
        int slot = acquire_expectation_table();
        enqueue_ril_expect(
                payload,                            // payload
                sizeof(ril_request_query_call_waiting_resp_t),
                RIL_REQUEST_QUERY_CALL_WAITING,     // msg_id
                info.serial,                        // serial_id
                (RIL_Errno)info.error,              // RIL error
                ril_utf_ril_response);              // type
        release_expectation_table(slot);
    }
    return Void();
}

Return<void> IRadioResponseClientImpl::setCallWaitingResponse(const V1_0::RadioResponseInfo& info) {
    int slot = acquire_expectation_table();
    enqueue_ril_expect(
                nullptr,                        // payload
                0,
                RIL_REQUEST_SET_CALL_WAITING,   // msg_id
                info.serial,                    // serial_id
                (RIL_Errno)info.error,          // RIL error
                ril_utf_ril_response);          // type
    release_expectation_table(slot);
    return Void();
}
Return<void> IRadioResponseClientImpl::getFacilityLockForAppResponse(const V1_0::RadioResponseInfo& info, int32_t response) {
    auto payload = (int *) malloc(sizeof(int));
    if (payload != nullptr) {
        *payload = response;
        int slot = acquire_expectation_table();
        enqueue_ril_expect(
                payload,                            // payload
                sizeof(int),
                RIL_REQUEST_QUERY_FACILITY_LOCK,    // msg_id
                info.serial,                        // serial_id
                (RIL_Errno)info.error,              // RIL error
                ril_utf_ril_response);              // type
        release_expectation_table(slot);
    }
    return Void();
}
Return<void> IRadioResponseClientImpl::setFacilityLockForAppResponse(const V1_0::RadioResponseInfo& info, int32_t retry) {
    (void)retry;
    int slot = acquire_expectation_table();
    enqueue_ril_expect(
            nullptr,                            // payload
            0,
            RIL_REQUEST_QUERY_FACILITY_LOCK,    // msg_id
            info.serial,                        // serial_id
            (RIL_Errno)info.error,              // RIL error
            ril_utf_ril_response);              // type
    release_expectation_table(slot);
    return Void();
}
Return<void> IRadioResponseClientImpl::setBarringPasswordResponse(const V1_0::RadioResponseInfo& info) {
    int slot = acquire_expectation_table();
    enqueue_ril_expect(
            nullptr,                            // payload
            0,
            RIL_REQUEST_CHANGE_BARRING_PASSWORD,// msg_id
            info.serial,                        // serial_id
            (RIL_Errno)info.error,              // RIL error
            ril_utf_ril_response);              // type
    release_expectation_table(slot);
    return Void();
}
Return<void> IRadioResponseClientImpl::setSuppServiceNotificationsResponse(const V1_0::RadioResponseInfo& info) {
    int slot = acquire_expectation_table();
    enqueue_ril_expect(
            nullptr,                            // payload
            0,
            RIL_REQUEST_SET_SUPP_SVC_NOTIFICATION, // msg_id
            info.serial,                        // serial_id
            (RIL_Errno)info.error,              // RIL error
            ril_utf_ril_response);              // type
    release_expectation_table(slot);
    return Void();
}
Return<void> IRadioResponseClientImpl::writeSmsToSimResponse(const V1_0::RadioResponseInfo& info, int32_t index) {
    auto payload = (ril_request_write_sms_to_sim_resp_t *) malloc(sizeof(ril_request_write_sms_to_sim_resp_t));
    if (payload != nullptr) {
        int slot = acquire_expectation_table();
        payload->response = index;
        enqueue_ril_expect(
                payload,                            // payload
                sizeof(ril_request_write_sms_to_sim_resp_t),
                RIL_REQUEST_WRITE_SMS_TO_SIM,       // msg_id
                info.serial,                        // serial_id
                (RIL_Errno)info.error,              // RIL error
                ril_utf_ril_response);              // type
        release_expectation_table(slot);
    }
    return Void();
}
Return<void> IRadioResponseClientImpl::setTTYModeResponse(const V1_0::RadioResponseInfo& info) {
    int slot = acquire_expectation_table();
    enqueue_ril_expect(
            nullptr,                            // payload
            0,
            RIL_REQUEST_SET_TTY_MODE,           // msg_id
            info.serial,                        // serial_id
            (RIL_Errno)info.error,              // RIL error
            ril_utf_ril_response);              // type
    release_expectation_table(slot);
    return Void();
}
Return<void> IRadioResponseClientImpl::getTTYModeResponse(const V1_0::RadioResponseInfo& info, V1_0::TtyMode mode) {
    auto payload = (ril_request_get_tty_mode_t *) malloc(sizeof(ril_request_get_tty_mode_t));
    if (payload != nullptr) {
        payload->tty_mode = (ril_tty_mode_enum_type)mode;
        int slot = acquire_expectation_table();
        enqueue_ril_expect(
                payload,                            // payload
                sizeof(ril_request_get_tty_mode_t),
                RIL_REQUEST_QUERY_TTY_MODE,         // msg_id
                info.serial,                        // serial_id
                (RIL_Errno)info.error,              // RIL error
                ril_utf_ril_response);              // type
        release_expectation_table(slot);
    }
    return Void();
}
Return<void> IRadioResponseClientImpl::setPreferredVoicePrivacyResponse(const V1_0::RadioResponseInfo& info) {
    int slot = acquire_expectation_table();
    enqueue_ril_expect(
            nullptr,                            // payload
            0,
            RIL_REQUEST_CDMA_SET_PREFERRED_VOICE_PRIVACY_MODE, // msg_id
            info.serial,                        // serial_id
            (RIL_Errno)info.error,              // RIL error
            ril_utf_ril_response);              // type
    release_expectation_table(slot);
    return Void();
}
Return<void> IRadioResponseClientImpl::getPreferredVoicePrivacyResponse(const V1_0::RadioResponseInfo& info, bool enable) {
    auto payload = (ril_request_cdma_query_preferred_voice_privacy_mode_t *)
              malloc(sizeof(ril_request_cdma_query_preferred_voice_privacy_mode_t));
    if (payload != nullptr) {
        payload->voice_privacy_mode = STANDARD_PRIVACY_MODE;
        if (enable) {
            payload->voice_privacy_mode = ENHANCED_PRIVACY_MODE;
        }
        int slot = acquire_expectation_table();
        enqueue_ril_expect(
                payload,                            // payload
                sizeof(ril_request_cdma_query_preferred_voice_privacy_mode_t),
                RIL_REQUEST_CDMA_QUERY_PREFERRED_VOICE_PRIVACY_MODE, // msg_id
                info.serial,                        // serial_id
                (RIL_Errno)info.error,              // RIL error
                ril_utf_ril_response);              // type
        release_expectation_table(slot);
    }
    return Void();
}
Return<void> IRadioResponseClientImpl::sendCDMAFeatureCodeResponse(const V1_0::RadioResponseInfo& info) {
    int slot = acquire_expectation_table();
    enqueue_ril_expect(
            nullptr,                            // payload
            0,
            RIL_REQUEST_CDMA_FLASH,             // msg_id
            info.serial,                        // serial_id
            (RIL_Errno)info.error,              // RIL error
            ril_utf_ril_response);              // type
    release_expectation_table(slot);
    return Void();
}
Return<void> IRadioResponseClientImpl::sendBurstDtmfResponse(const V1_0::RadioResponseInfo& info) {
    int slot = acquire_expectation_table();
    enqueue_ril_expect(
            nullptr,                            // payload
            0,
            RIL_REQUEST_CDMA_BURST_DTMF,        // msg_id
            info.serial,                        // serial_id
            (RIL_Errno)info.error,              // RIL error
            ril_utf_ril_response);              // type
    release_expectation_table(slot);
    return Void();
}
Return<void> IRadioResponseClientImpl::acknowledgeLastIncomingCdmaSmsResponse(const V1_0::RadioResponseInfo& info) {
    int slot = acquire_expectation_table();
    enqueue_ril_expect(
            nullptr,                            // payload
            0,
            RIL_REQUEST_CDMA_SMS_ACKNOWLEDGE,   // msg_id
            info.serial,                        // serial_id
            (RIL_Errno)info.error,              // RIL error
            ril_utf_ril_response);              // type
    release_expectation_table(slot);
    return Void();
}
Return<void> IRadioResponseClientImpl::getGsmBroadcastConfigResponse(
      const V1_0::RadioResponseInfo& info, const hidl_vec<V1_0::GsmBroadcastSmsConfigInfo>& configs) {
    int slot = acquire_expectation_table();
    int num_congigs = configs.size();
    bool allocation_failure = false;
    RIL_GSM_BroadcastSmsConfigInfo **payload = nullptr;
    size_t payload_len = 0;
    if(num_congigs > 0) {
        payload = (RIL_GSM_BroadcastSmsConfigInfo **)
                            malloc(sizeof(RIL_GSM_BroadcastSmsConfigInfo*) * num_congigs);
        if(payload) {
            payload_len = sizeof(RIL_GSM_BroadcastSmsConfigInfo*)*num_congigs;

            for(int i=0; i<num_congigs; i++) {
                payload[i] = (RIL_GSM_BroadcastSmsConfigInfo*)
                                malloc(sizeof(RIL_GSM_BroadcastSmsConfigInfo));
                if(payload[i]) {
                    memset(payload[i], 0, sizeof(RIL_GSM_BroadcastSmsConfigInfo));
                    payload[i]->fromServiceId = (int)(configs[i].fromServiceId);
                    payload[i]->toServiceId = (int)(configs[i].toServiceId);
                    payload[i]->fromCodeScheme = (int)(configs[i].fromCodeScheme);
                    payload[i]->toCodeScheme = (int)(configs[i].toCodeScheme);
                    payload[i]->selected = (unsigned char)(configs[i].selected);
                } else {
                    allocation_failure = true;
                }
            }
        } else {
            allocation_failure = true;
        }
    }
    if(allocation_failure) {
        if(payload) {
            for(int i=0; i<num_congigs; i++) {
                if(payload[i]) {
                    free(payload[i]);
                }
            }
            free(payload);
        }
        payload = nullptr;
        payload_len = 0;
    }
    enqueue_ril_expect(
            payload,                            // payload
            payload_len,
            RIL_REQUEST_GSM_GET_BROADCAST_SMS_CONFIG,   // msg_id
            info.serial,                        // serial_id
            (RIL_Errno)info.error,              // RIL error
            ril_utf_ril_response);              // type
    release_expectation_table(slot);
    return Void();
}
Return<void> IRadioResponseClientImpl::setRadioPowerResponse(const V1_0::RadioResponseInfo& info) {
    int slot = acquire_expectation_table();
    enqueue_ril_expect(
            nullptr,                            // payload
            0,
            RIL_REQUEST_RADIO_POWER,            // msg_id
            info.serial,                        // serial_id
            (RIL_Errno)info.error,              // RIL error
            ril_utf_ril_response);              // type
    release_expectation_table(slot);
    return Void();
}
Return<void> IRadioResponseClientImpl::getBasebandVersionResponse(const V1_0::RadioResponseInfo& info,
        const hidl_string& version) {
    int slot = acquire_expectation_table();
    auto payload = (ril_request_get_baseband_version_type_t*)malloc(sizeof(ril_request_get_baseband_version_type_t));
    size_t payload_len = sizeof(ril_request_get_baseband_version_type_t);
    memset(payload, 0, payload_len);
    strlcpy(payload->swver, version.c_str(), SW_VERSION_MAX);
    enqueue_ril_expect(
            payload,                            // payload
            payload_len,
            RIL_REQUEST_BASEBAND_VERSION,       // msg_id
            info.serial,                        // serial_id
            (RIL_Errno)info.error,              // RIL error
            ril_utf_ril_response);              // type
    release_expectation_table(slot);
    return Void();
}
Return<void> IRadioResponseClientImpl::setGsmBroadcastActivationResponse(const V1_0::RadioResponseInfo& info) {
    int slot = acquire_expectation_table();
    enqueue_ril_expect(
            nullptr,                             // payload
            0,
            RIL_REQUEST_GSM_SMS_BROADCAST_ACTIVATION, // msg_id
            info.serial,                         // serial_id
            (RIL_Errno)info.error,               // RIL error
            ril_utf_ril_response);               // type
    release_expectation_table(slot);
    return Void();
}
Return<void> IRadioResponseClientImpl::getCdmaBroadcastConfigResponse(
    const V1_0::RadioResponseInfo& info, const hidl_vec<V1_0::CdmaBroadcastSmsConfigInfo>& configs) {
    int slot = acquire_expectation_table();
    int num_congigs = configs.size();
    RIL_CDMA_BroadcastSmsConfigInfo **payload = nullptr;
    size_t payload_len = 0;
    if(num_congigs > 0) {
        payload = (RIL_CDMA_BroadcastSmsConfigInfo **)
                            malloc(sizeof(RIL_CDMA_BroadcastSmsConfigInfo*) * num_congigs);
        payload_len = sizeof(RIL_CDMA_BroadcastSmsConfigInfo*)*num_congigs;

        for(int i=0; i<num_congigs; i++) {
            payload[i] = (RIL_CDMA_BroadcastSmsConfigInfo*)
                            malloc(sizeof(RIL_CDMA_BroadcastSmsConfigInfo));
            memset(payload[i], 0, sizeof(RIL_CDMA_BroadcastSmsConfigInfo));
            payload[i]->service_category = (int)(configs[i].serviceCategory);
            payload[i]->language = (int)(configs[i].language);
            payload[i]->selected = (unsigned char)(configs[i].selected);
        }
    }
    enqueue_ril_expect(
            nullptr,                             // payload
            0,
            RIL_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG, // msg_id
            info.serial,                         // serial_id
            (RIL_Errno)info.error,               // RIL error
            ril_utf_ril_response);               // type
    release_expectation_table(slot);
    return Void();
}
Return<void> IRadioResponseClientImpl::setCdmaBroadcastActivationResponse(
                                                     const V1_0::RadioResponseInfo& info) {
    int slot = acquire_expectation_table();
    enqueue_ril_expect(
            nullptr,                             // payload
            0,
            RIL_REQUEST_CDMA_SMS_BROADCAST_ACTIVATION, // msg_id
            info.serial,                         // serial_id
            (RIL_Errno)info.error,               // RIL error
            ril_utf_ril_response);               // type
    release_expectation_table(slot);
    return Void();
}
Return<void> IRadioResponseClientImpl::writeSmsToRuimResponse(const V1_0::RadioResponseInfo& info, uint32_t index) {
    int slot = acquire_expectation_table();
    auto payload = (int *) malloc(sizeof(int));
    if (payload != nullptr) {
        *payload = index;
    }
    enqueue_ril_expect(
            payload,                            // payload
            sizeof(int),
            RIL_REQUEST_CDMA_WRITE_SMS_TO_RUIM, // msg_id
            info.serial,                        // serial_id
            (RIL_Errno)info.error,              // RIL error
            ril_utf_ril_response);              // type
    release_expectation_table(slot);
    return Void();
}
Return<void> IRadioResponseClientImpl::getDeviceIdentityResponse(const V1_0::RadioResponseInfo& info,
        const hidl_string& imei, const hidl_string& imeisv, const hidl_string& esn, const hidl_string& meid) {
    int slot = acquire_expectation_table();
    auto payload = (ril_request_device_identity_type_t *)malloc(sizeof(ril_request_device_identity_type_t));
    size_t payload_len = sizeof(ril_request_device_identity_type_t);
    memset(payload, 0, payload_len);
    convertToRil(imei, imeisv, esn, meid, payload);
    enqueue_ril_expect(
            payload,                            // payload
            payload_len,
            RIL_REQUEST_DEVICE_IDENTITY,        // msg_id
            info.serial,                        // serial_id
            (RIL_Errno)info.error,              // RIL error
            ril_utf_ril_response);              // type
    release_expectation_table(slot);
    return Void();
}
Return<void> IRadioResponseClientImpl::setGsmBroadcastConfigResponse(
                                                        const V1_0::RadioResponseInfo& info) {
    int slot = acquire_expectation_table();
    enqueue_ril_expect(
            nullptr,                             // payload
            0,
            RIL_REQUEST_GSM_SET_BROADCAST_SMS_CONFIG, // msg_id
            info.serial,                         // serial_id
            (RIL_Errno)info.error,               // RIL error
            ril_utf_ril_response);               // type
    release_expectation_table(slot);
    return Void();
}
Return<void> IRadioResponseClientImpl::setCdmaBroadcastConfigResponse(
                                                        const V1_0::RadioResponseInfo& info) {
    int slot = acquire_expectation_table();
    enqueue_ril_expect(
            nullptr,                             // payload
            0,
            RIL_REQUEST_CDMA_SET_BROADCAST_SMS_CONFIG, // msg_id
            info.serial,                         // serial_id
            (RIL_Errno)info.error,               // RIL error
            ril_utf_ril_response);               // type
    release_expectation_table(slot);
    return Void();
}
Return<void> IRadioResponseClientImpl::reportSmsMemoryStatusResponse(
                                                        const V1_0::RadioResponseInfo& info) {
    int slot = acquire_expectation_table();
    enqueue_ril_expect(
            nullptr,                             // payload
            0,
            RIL_REQUEST_REPORT_SMS_MEMORY_STATUS, // msg_id
            info.serial,                         // serial_id
            (RIL_Errno)info.error,               // RIL error
            ril_utf_ril_response);               // type
    release_expectation_table(slot);
    return Void();
}
Return<void> IRadioResponseClientImpl::getModemActivityInfoResponse(const V1_0::RadioResponseInfo& info,
        const V1_0::ActivityStatsInfo& activityInfo) {
    auto payload = (RIL_ActivityStatsInfo*) malloc(sizeof(RIL_ActivityStatsInfo));
    if (payload != nullptr) {
        memset(payload, 0, sizeof(RIL_ActivityStatsInfo));
        int slot = acquire_expectation_table();
        enqueue_ril_expect(
                payload,                            // payload
                sizeof(RIL_ActivityStatsInfo),
                RIL_REQUEST_GET_ACTIVITY_INFO,        // msg_id
                info.serial,                        // serial_id
                (RIL_Errno)info.error,              // RIL error
                ril_utf_ril_response);              // type
        release_expectation_table(slot);
    }
    return Void();
}

Return<void> IRadioResponseClientImpl::iccIOForAppResponse(const V1_0::RadioResponseInfo& info,
                 const V1_0::IccIoResult& iccIo) {
  auto payload = (RIL_SIM_IO_Response*) malloc(sizeof(RIL_SIM_IO_Response));
    if (payload != nullptr) {
        memset(payload, 0, sizeof(RIL_SIM_IO_Response));
        convertSimIoRespHidltoRilPayload(iccIo, *payload);

        int slot = acquire_expectation_table();

        enqueue_ril_expect(
                payload,                 // payload
                sizeof(RIL_SIM_IO_Response),
                RIL_REQUEST_SIM_IO,      // msg_id
                info.serial,             // serial_id
                (RIL_Errno)info.error,   // RIL error
                ril_utf_ril_response);   // type

        release_expectation_table(slot);
    }

  return Void();
}

Return<void> IRadioResponseClientImpl::getIccCardStatusResponse_1_2
                 (const V1_0::RadioResponseInfo& info, const V1_2::CardStatus& cardStatus) {

  auto payload = (RIL_CardStatus_v6*) malloc(sizeof(RIL_CardStatus_v6));
    if (payload != nullptr) {
        memset(payload, 0, sizeof(RIL_CardStatus_v6));
        convertGetIccCardStatusRespHidltoRilPayload(cardStatus, *payload);

        int slot = acquire_expectation_table();

        enqueue_ril_expect(
                payload,                      // payload
                sizeof(RIL_CardStatus_v6),
                RIL_REQUEST_GET_SIM_STATUS,   // msg_id
                info.serial,                  // serial_id
                (RIL_Errno)info.error,        // RIL error
                ril_utf_ril_response);        // type

        release_expectation_table(slot);
    }

  return Void();
}

Return<void> IRadioResponseClientImpl::getIccCardStatusResponse_1_4
                 (const V1_0::RadioResponseInfo& info, const V1_4::CardStatus& cardStatus) {

  auto payload = (RIL_CardStatus_v6*) malloc(sizeof(RIL_CardStatus_v6));
    if (payload != nullptr) {
        memset(payload, 0, sizeof(RIL_CardStatus_v6));
        convertGetIccCardStatusRespHidltoRilPayload_1_4(cardStatus, *payload);

        int slot = acquire_expectation_table();

        enqueue_ril_expect(
                payload,                      // payload
                sizeof(RIL_CardStatus_v6),
                RIL_REQUEST_GET_SIM_STATUS,   // msg_id
                info.serial,                  // serial_id
                (RIL_Errno)info.error,        // RIL error
                ril_utf_ril_response);        // type

        release_expectation_table(slot);
    }

  return Void();
}

Return<void> IRadioResponseClientImpl::iccOpenLogicalChannelResponse(
               const V1_0::RadioResponseInfo& info,
               int32_t channelId, const hidl_vec<int8_t>& selectResponse)
{
  /*Since channel value should be added to the response, length is increased by 1*/
  uint32_t selectResponse_length = selectResponse.size() + 1;
  if (selectResponse_length)
  {
    auto payload = (uint8_t*) malloc(sizeof(selectResponse));
      if (payload != nullptr) {
          convertOpenLogicalChannelRespHidlToRilPayload(selectResponse, channelId, payload);
       }
        int slot = acquire_expectation_table();

        enqueue_ril_expect(
                payload,                      // payload
                selectResponse_length,
                RIL_REQUEST_SIM_OPEN_CHANNEL,   // msg_id
                info.serial,                  // serial_id
                (RIL_Errno)info.error,        // RIL error
                ril_utf_ril_response);        // type

        release_expectation_table(slot);
    }
  return Void();
}

Return<void> IRadioResponseClientImpl::enableModemResponse
                 (const V1_0::RadioResponseInfo& info) {

    int slot = acquire_expectation_table();

    enqueue_ril_expect(
            nullptr,                      // payload
            0,
            RIL_REQUEST_ENABLE_MODEM,     // msg_id
            info.serial,                  // serial_id
            (RIL_Errno)info.error,        // RIL error
            ril_utf_ril_response);        // type

    release_expectation_table(slot);

    return Void();
}

Return<void> IRadioResponseClientImpl::getModemStackStatusResponse
                 (const V1_0::RadioResponseInfo& info, bool isEnabled) {

    auto payload = (int*)malloc(sizeof(int));
    if (payload != nullptr) {
        *payload = isEnabled ? 1 : 0;

        int slot = acquire_expectation_table();
        enqueue_ril_expect(
                payload,                            // payload
                sizeof(int),
                RIL_REQUEST_GET_MODEM_STACK_STATUS, // msg_id
                info.serial,                        // serial_id
                (RIL_Errno)info.error,              // RIL error
                ril_utf_ril_response);              // type
        release_expectation_table(slot);
    }
 
    return Void();
}

Return<void> IRadioResponseClientImpl::startNetworkScanResponse_1_4(const V1_0::RadioResponseInfo& info) {
    int slot = acquire_expectation_table();
    enqueue_ril_expect(
            nullptr,                        // payload
            0,
            RIL_REQUEST_START_NETWORK_SCAN, // msg_id
            info.serial,                    // serial_id
            (RIL_Errno)info.error,          // RIL error
            ril_utf_ril_response);          // type
    release_expectation_table(slot);
    return Void();
}

Return<void> IRadioResponseClientImpl::stopNetworkScanResponse(const V1_0::RadioResponseInfo& info) {
    int slot = acquire_expectation_table();
    enqueue_ril_expect(
            nullptr,                        // payload
            0,
            RIL_REQUEST_STOP_NETWORK_SCAN,  // msg_id
            info.serial,                    // serial_id
            (RIL_Errno)info.error,          // RIL error
            ril_utf_ril_response);          // type
    release_expectation_table(slot);
    return Void();
}
Return<void> IRadioResponseClientImpl::getNeighboringCidsResponse(const V1_0::RadioResponseInfo& info, const hidl_vec<V1_0::NeighboringCell>& cells) {
    int slot = acquire_expectation_table();
    int num_cells = cells.size();
    RIL_NeighboringCell **payload = nullptr;
    size_t payload_len = 0;
    if(num_cells > 0) {
        payload = (RIL_NeighboringCell **)malloc(sizeof(RIL_NeighboringCell*) * num_cells);
        payload_len = sizeof(RIL_NeighboringCell*)*num_cells;

        for(int i=0; i<num_cells; i++) {
            payload[i] = (RIL_NeighboringCell*)malloc(sizeof(RIL_NeighboringCell));
            memset(payload[i], 0, sizeof(RIL_NeighboringCell));
            payload[i]->cid = strdup(cells[i].cid.c_str());
            payload[i]->rssi = (int)(cells[i].rssi);
        }
    }
    enqueue_ril_expect(
            payload,                             // payload
            payload_len,
            RIL_REQUEST_GET_NEIGHBORING_CELL_IDS, // msg_id
            info.serial,                         // serial_id
            (RIL_Errno)info.error,               // RIL error
            ril_utf_ril_response);               // type
    release_expectation_table(slot);
    return Void();
}
