/*===========================================================================
 *
 *    Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
 *    All Rights Reserved.
 *    Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 *===========================================================================*/

#include "ril_service_1_4.h"
#include "ril_utf_hidl_services.h"
#include "ril_utf_ril_api.h"
#include "platform/common/CommonPlatform.h"
#include "platform/android/IRadioResponseClientImpl.h"
#include "platform/android/IRadioIndicationClientImpl.h"
#include "platform/android/SmsHidlUtil.h"
#include "platform/android/NasHidlUtil.h"
#include "platform/android/VoiceHidlUtil.h"
#include "platform/android/SimHidlUtil.h"
#include <typeinfo>

#include <android/hardware/radio/1.2/IRadio.h>
#include <android/hardware/radio/1.3/IRadio.h>
#include <android/hardware/radio/1.4/IRadio.h>

#ifdef OFF
#undef OFF
#endif

class AndroidIRadio : public CommonPlatform {
public:
    AndroidIRadio() {
        utfIRadio = nullptr;
        bFirstCall = true;
    }
    void Register(RIL_RadioFunctions *callbacks) override;
    int OnRequest(int request, void *data, size_t datalen,
        RIL_Token t) override;
private:
    ::android::sp<::android::hardware::radio::V1_4::IRadio> utfIRadio;
    bool bFirstCall;
};

void AndroidIRadio::Register(RIL_RadioFunctions *callbacks) {
    if (!bFirstCall) return;
    ::radio::registerService(callbacks, nullptr);
    utfIRadio = ::android::hardware::radio::V1_4::IRadio::getService("slot1", false);
    if (utfIRadio != nullptr) {
        // create the HIDL clients
        sp<IRadioResponseClientImpl> respClient = new IRadioResponseClientImpl();
        sp<IRadioIndicationClientImpl> indClient = new IRadioIndicationClientImpl();
        utfIRadio->setResponseFunctions(respClient, indClient);
    }
    bFirstCall = false;
}

int AndroidIRadio::OnRequest(int request, void *data, size_t datalen,
        RIL_Token t) {
    switch (request) {
    case RIL_REQUEST_SEND_SMS:
        if (utfIRadio != nullptr) {
            V1_0::GsmSmsMessage sms;
            convertGsmSmsPayloadtoHidl(data, datalen, sms);
            RIL_UTF_DEBUG("before call HIDL handler");
            utfIRadio->sendSms(*static_cast<int32_t*>(t), sms);
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_SMS_ACKNOWLEDGE:
        if (utfIRadio != nullptr) {
            auto ackInfo = static_cast<int*>(data);
            utfIRadio->acknowledgeLastIncomingGsmSms(
                *static_cast<int32_t*>(t), !!ackInfo[0],
                static_cast<SmsAcknowledgeFailCause>(ackInfo[1]));
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_IMS_REGISTRATION_STATE:
        if (utfIRadio != nullptr) {
            utfIRadio->getImsRegistrationState(*static_cast<int32_t*>(t));
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_SET_SMSC_ADDRESS:
        if (utfIRadio != nullptr) {
            hidl_string addr = (const char*)data;
            utfIRadio->setSmscAddress(*static_cast<int32_t*>(t), std::move(addr));
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_DELETE_SMS_ON_SIM:
        if (utfIRadio != nullptr) {
            int index = ((ril_request_delete_sms_on_sim_t*)data)->record_index;
            utfIRadio->deleteSmsOnSim(*static_cast<int32_t*>(t), index);
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_CDMA_DELETE_SMS_ON_RUIM:
        if (utfIRadio != nullptr) {
            int index = ((ril_request_delete_sms_on_sim_t*)data)->record_index;
            utfIRadio->deleteSmsOnRuim(*static_cast<int32_t*>(t), index);
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_VOICE_REGISTRATION_STATE:
        if (utfIRadio != nullptr) {
            utfIRadio->getVoiceRegistrationState(*static_cast<int32_t*>(t));
            return 0;
        } else {
            return 1;
        }
        break;
    case RIL_REQUEST_DATA_REGISTRATION_STATE:
        if (utfIRadio != nullptr) {
            utfIRadio->getDataRegistrationState(*static_cast<int32_t*>(t));
            return 0;
        } else {
            return 1;
        }
        break;
    case RIL_REQUEST_OPERATOR:
        if (utfIRadio != nullptr) {
            utfIRadio->getOperator(*static_cast<int32_t*>(t));
            return 0;
        } else {
            return 1;
        }
        break;
    case RIL_REQUEST_QUERY_AVAILABLE_NETWORKS:
        if (utfIRadio != nullptr) {
            utfIRadio->getAvailableNetworks(*static_cast<int32_t*>(t));
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE:
        if (utfIRadio != nullptr) {
            int networkType = *static_cast<int*>(data);
            utfIRadio->setPreferredNetworkType(*static_cast<int32_t*>(t),
                    (V1_0::PreferredNetworkType)networkType);
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE:
        if (utfIRadio != nullptr) {
            utfIRadio->getPreferredNetworkType(*static_cast<int32_t*>(t));
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_VOICE_RADIO_TECH:
        if (utfIRadio != nullptr) {
            utfIRadio->getVoiceRadioTechnology(*static_cast<int32_t*>(t));
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_CDMA_SET_SUBSCRIPTION_SOURCE:
        if (utfIRadio != nullptr) {
            auto source = *static_cast<int*>(data);
            utfIRadio->setCdmaSubscriptionSource(*static_cast<int32_t*>(t),
                    static_cast<CdmaSubscriptionSource>(source));
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_CDMA_SET_ROAMING_PREFERENCE:
        if (utfIRadio != nullptr) {
            auto type = *static_cast<int*>(data);
            utfIRadio->setCdmaRoamingPreference(*static_cast<int32_t*>(t),
                    static_cast<CdmaRoamingType>(type));
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_CDMA_QUERY_ROAMING_PREFERENCE:
        if (utfIRadio != nullptr) {
            utfIRadio->getCdmaRoamingPreference(*static_cast<int32_t*>(t));
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_CDMA_GET_SUBSCRIPTION_SOURCE:
        if (utfIRadio != nullptr) {
            utfIRadio->getCdmaSubscriptionSource(*static_cast<int32_t*>(t));
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_SHUTDOWN:
        if (utfIRadio != nullptr) {
            utfIRadio->requestShutdown(*static_cast<int32_t*>(t));
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_DIAL:
        if (utfIRadio != nullptr) {
            V1_0::Dial dialInfo = {};
            convertDialInfotoHidl(data, datalen, dialInfo);
            utfIRadio->dial(*static_cast<int32_t*>(t), dialInfo);
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_GET_CURRENT_CALLS:
        if (utfIRadio != nullptr) {
            utfIRadio->getCurrentCalls(*static_cast<int32_t*>(t));
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_HANGUP:
        if (utfIRadio != nullptr) {
            int index = *static_cast<int*>(data);
            utfIRadio->hangup(*static_cast<int32_t*>(t), index);
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_ANSWER:
        if (utfIRadio != nullptr) {
            utfIRadio->acceptCall(*static_cast<int32_t*>(t));
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_HANGUP_WAITING_OR_BACKGROUND:
        if (utfIRadio != nullptr) {
            utfIRadio->hangupWaitingOrBackground(*static_cast<int32_t*>(t));
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND:
        if (utfIRadio != nullptr) {
            utfIRadio->hangupForegroundResumeBackground(*static_cast<int32_t*>(t));
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_UDUB:
        if (utfIRadio != nullptr) {
            utfIRadio->rejectCall(*static_cast<int32_t*>(t));
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE:
        if (utfIRadio != nullptr) {
            utfIRadio->switchWaitingOrHoldingAndActive(*static_cast<int32_t*>(t));
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_CONFERENCE:
        if (utfIRadio != nullptr) {
            utfIRadio->conference(*static_cast<int32_t*>(t));
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_DTMF:
        if (utfIRadio != nullptr) {
            auto dtmf_req = (ril_request_dtmf_t *)data;
            hidl_string s = dtmf_req->dtmf;
            utfIRadio->sendDtmf(*static_cast<int32_t*>(t), s);
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_DTMF_START:
        if (utfIRadio != nullptr) {
            auto dtmf_req = (ril_request_dtmf_t *)data;
            hidl_string s = dtmf_req->dtmf;
            utfIRadio->startDtmf(*static_cast<int32_t*>(t), s);
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_DTMF_STOP:
        if (utfIRadio != nullptr) {
            utfIRadio->stopDtmf(*static_cast<int32_t*>(t));
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_GET_CLIR:
        if (utfIRadio != nullptr) {
            utfIRadio->getClir(*static_cast<int32_t*>(t));
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_SET_CLIR:
        if (utfIRadio != nullptr) {
            auto req = (ril_request_set_clir_t *)data;
            utfIRadio->setClir(*static_cast<int32_t*>(t), req->n_parameter);
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_GET_MUTE:
        if (utfIRadio != nullptr) {
            utfIRadio->getMute(*static_cast<int32_t*>(t));
            return 0;
        }
    case RIL_REQUEST_SET_MUTE:
        if (utfIRadio != nullptr) {
            auto req = (ril_request_mute_t *)data;
            utfIRadio->setMute(*static_cast<int32_t*>(t), req->enable);
            return 0;
        }
    case RIL_REQUEST_SET_UNSOL_CELL_INFO_LIST_RATE:
        if (utfIRadio != nullptr) {
            auto req = (ril_request_unsol_cell_info_rate_t *)data;
            utfIRadio->setCellInfoListRate(*static_cast<int32_t*>(t), req->rate);
            return 0;
        }
    case RIL_REQUEST_QUERY_CLIP:
        if (utfIRadio != nullptr) {
            utfIRadio->getClip(*static_cast<int32_t*>(t));
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_GET_CELL_INFO_LIST:
        if (utfIRadio != nullptr) {
            utfIRadio->getCellInfoList(*static_cast<int32_t*>(t));
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_EXPLICIT_CALL_TRANSFER:
        if (utfIRadio != nullptr) {
            utfIRadio->explicitCallTransfer(*static_cast<int32_t*>(t));
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_SEPARATE_CONNECTION:
        if (utfIRadio != nullptr) {
            int index = *static_cast<int*>(data);
            utfIRadio->separateConnection(*static_cast<int32_t*>(t), index);
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_LAST_CALL_FAIL_CAUSE:
        if (utfIRadio != nullptr) {
            utfIRadio->getLastCallFailCause(*static_cast<int32_t*>(t));
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_SEND_USSD:
        if (utfIRadio != nullptr) {
            char *ussd_str = static_cast<char *>(data);
            hidl_string ussd = ussd_str;
            utfIRadio->sendUssd(*static_cast<int32_t*>(t), ussd);
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_CANCEL_USSD:
        if (utfIRadio != nullptr) {
            utfIRadio->cancelPendingUssd(*static_cast<int32_t*>(t));
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_QUERY_CALL_FORWARD_STATUS:
        if (utfIRadio != nullptr) {
            V1_0::CallForwardInfo cfInfo = {};
            convertCallForwardInfotoHidl(data, datalen, cfInfo);
            utfIRadio->getCallForwardStatus(*static_cast<int32_t*>(t), cfInfo);
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_SET_CALL_FORWARD:
        if (utfIRadio != nullptr) {
            V1_0::CallForwardInfo cfInfo = {};
            convertCallForwardInfotoHidl(data, datalen, cfInfo);
            utfIRadio->setCallForward(*static_cast<int32_t*>(t), cfInfo);
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_QUERY_CALL_WAITING:
        if (utfIRadio != nullptr) {
            auto req = (ril_request_query_call_waiting_t *)data;
            utfIRadio->getCallWaiting(*static_cast<int32_t*>(t), req->service_class);
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_SET_CALL_WAITING:
        if (utfIRadio != nullptr) {
            auto req = (ril_request_set_call_waiting_t *)data;
            bool enable = req->status ? true : false;
            int32_t serviceClass = req->service_class;
            utfIRadio->setCallWaiting(*static_cast<int32_t*>(t), enable, serviceClass);
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_QUERY_FACILITY_LOCK:
        if (utfIRadio != nullptr) {
            char **req = (char**)data;
            string facility = req[0] ? req[0] : "";
            string password = req[1] ? req[1] : "";
            int32_t serviceClass = req[2] ? atoi(req[2]) : 0;
            string appId = req[3] ? req[3] : "";
            utfIRadio->getFacilityLockForApp(*static_cast<int32_t*>(t), facility, password, serviceClass, appId);
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_SET_FACILITY_LOCK:
        if (utfIRadio != nullptr) {
            char **req = (char**)data;
            string facility = req[0] ? req[0] : "";
            bool lockState = req[1] ? (req[1][0] == '1' ? true : false) : false;
            string password = req[2] ? req[2] : "";
            int32_t serviceClass = req[3] ? atoi(req[3]) : 0;
            string appId = req[4] ? req[4] : "";
            utfIRadio->setFacilityLockForApp(*static_cast<int32_t*>(t), facility, lockState, password, serviceClass, appId);
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_CHANGE_BARRING_PASSWORD:
        if (utfIRadio != nullptr) {
            auto req = (ril_request_change_barring_password_t *)data;
            utfIRadio->setBarringPassword(*static_cast<int32_t*>(t), req->facility_string_code, req->old_password, req->new_password);
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_SET_SUPP_SVC_NOTIFICATION:
        if (utfIRadio != nullptr) {
            auto req = (ril_request_set_supp_svc_notification_t *)data;
            bool enable = req->notification_status ? true : false;
            utfIRadio->setSuppServiceNotifications(*static_cast<int32_t*>(t), enable);
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_SET_TTY_MODE:
        if (utfIRadio != nullptr) {
            auto req = (ril_request_set_tty_mode_t *)data;
            V1_0::TtyMode mode = (V1_0::TtyMode) req->tty_mode;
            switch(req->tty_mode) {
              case TTY_Off:
                mode = V1_0::TtyMode::OFF;
                break;
              case TTY_Full:
                mode = V1_0::TtyMode::FULL;
                break;
              case TTY_HCO:
                mode = V1_0::TtyMode::HCO;
                break;
              case TTY_VCO:
                mode = V1_0::TtyMode::VCO;
                break;
            }
            utfIRadio->setTTYMode(*static_cast<int32_t*>(t), mode);
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_QUERY_TTY_MODE:
        if (utfIRadio != nullptr) {
            utfIRadio->getTTYMode(*static_cast<int32_t*>(t));
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_CDMA_SET_PREFERRED_VOICE_PRIVACY_MODE:
        if (utfIRadio != nullptr) {
            auto req = (ril_request_cdma_set_preferred_voice_privacy_mode_t *)data;
            bool enable = req->voice_privacy_mode ? true : false;
            utfIRadio->setPreferredVoicePrivacy(*static_cast<int32_t*>(t), enable);
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_CDMA_QUERY_PREFERRED_VOICE_PRIVACY_MODE:
        if (utfIRadio != nullptr) {
            utfIRadio->getPreferredVoicePrivacy(*static_cast<int32_t*>(t));
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_CDMA_FLASH:
        if (utfIRadio != nullptr) {
            auto req = (ril_request_cdma_flash_t *)data;
            hidl_string featureCode = req->flash_string;
            utfIRadio->sendCDMAFeatureCode(*static_cast<int32_t*>(t), featureCode);
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_CDMA_BURST_DTMF:
        if (utfIRadio != nullptr) {
            auto req = (ril_request_cdma_burst_dtmf_t *)data;
            hidl_string dtmf = req->DTMF_string;
            int32_t on = atoi(req->DTMF_ON_length);
            int32_t off = atoi(req->DTMF_OFF_length);
            utfIRadio->sendBurstDtmf(*static_cast<int32_t*>(t), dtmf, on, off);
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_RADIO_POWER:
        if (utfIRadio != nullptr) {
            int state = *static_cast<int*>(data);
            bool on = state ? true : false;
            utfIRadio->setRadioPower(*static_cast<int32_t*>(t), on);
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_BASEBAND_VERSION:
        if (utfIRadio != nullptr) {
            utfIRadio->getBasebandVersion(*static_cast<int32_t*>(t));
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_DEVICE_IDENTITY:
        if (utfIRadio != nullptr) {
            utfIRadio->getDeviceIdentity(*static_cast<int32_t*>(t));
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_GET_ACTIVITY_INFO:
        if (utfIRadio != nullptr) {
            utfIRadio->getModemActivityInfo(*static_cast<int32_t*>(t));
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_SIM_IO:
        if (utfIRadio != nullptr) {
            V1_0::IccIo iccIoData;
            /* Construct the SIM IO request data */
            convertSimIoPayloadtoHidl(data, datalen, iccIoData);
            RIL_UTF_DEBUG("Calling iccIOForApp HIDL request");
            utfIRadio->iccIOForApp(*static_cast<int32_t*>(t), iccIoData);
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_GET_SIM_STATUS:
        if (utfIRadio != nullptr) {
            RIL_UTF_DEBUG("Calling getIccCardStatus HIDL request");
            utfIRadio->getIccCardStatus(*static_cast<int32_t*>(t));
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_SIM_OPEN_CHANNEL:
        if (utfIRadio != nullptr) {
            RIL_UTF_DEBUG("Calling iccOpenLogicalChannel HIDL request");
            uint8_t p2 = 0;
            hidl_string aid = {};
            convertOpenLogicalChannelPayloadtoHidl(data, datalen, &p2, aid);
            utfIRadio->iccOpenLogicalChannel(*static_cast<int32_t*>(t),aid ,p2);
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_QUERY_NETWORK_SELECTION_MODE:
        if (utfIRadio != nullptr) {
            RIL_UTF_DEBUG("Calling getNetworkSelectionMode HIDL request");
            utfIRadio->getNetworkSelectionMode(*static_cast<int32_t*>(t));
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_SET_BAND_MODE:
        if (utfIRadio != nullptr) {
            V1_0::RadioBandMode mode = *static_cast<V1_0::RadioBandMode*>(data);
            RIL_UTF_DEBUG("Calling setBandMode HIDL request");
            utfIRadio->setBandMode(*static_cast<int32_t*>(t), mode);
            return 0;
        } else {
            return 1;
        }
     case RIL_REQUEST_QUERY_AVAILABLE_BAND_MODE:
        if (utfIRadio != nullptr) {
            RIL_UTF_DEBUG("Calling getAvailableBandModes HIDL request");
            utfIRadio->getAvailableBandModes(*static_cast<int32_t*>(t));
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_ENABLE_MODEM:
         RIL_UTF_DEBUG("Calling RIL_REQUEST_ENABLE_MODEM");
         if (utfIRadio != nullptr) {
            int state = *static_cast<int*>(data);
            bool on = state ? true : false;
            RIL_UTF_DEBUG("Calling enableModem");
            utfIRadio->enableModem(*static_cast<int32_t*>(t), on);
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_GET_MODEM_STACK_STATUS:
         RIL_UTF_DEBUG("Calling RIL_REQUEST_ENABLE_MODEM");
         if (utfIRadio != nullptr) {
            RIL_UTF_DEBUG("Calling getModemStackStatus");
            utfIRadio->getModemStackStatus(*static_cast<int32_t*>(t));
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_GET_SMSC_ADDRESS:
        if (utfIRadio != nullptr) {
            RIL_UTF_DEBUG("Calling getSmscAddress HIDL request");
            utfIRadio->getSmscAddress(*static_cast<int32_t*>(t));
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_WRITE_SMS_TO_SIM:
         RIL_UTF_DEBUG("Calling RIL_REQUEST_WRITE_SMS_TO_SIM");
         if (utfIRadio != nullptr) {
            V1_0::SmsWriteArgs smswrite;
            convertGsmSmsWritePayloadtoHidl(data, datalen, smswrite);
            RIL_UTF_DEBUG("Calling writeSmsToSim");
            utfIRadio->writeSmsToSim(*static_cast<int32_t*>(t), smswrite);
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_CDMA_WRITE_SMS_TO_RUIM:
         RIL_UTF_DEBUG("Calling RIL_REQUEST_CDMA_WRITE_SMS_TO_RUIM");
         if (utfIRadio != nullptr) {
            V1_0::CdmaSmsWriteArgs smswrite;
            convertCdmaSmsWritePayloadtoHidl(data, datalen, smswrite);
            RIL_UTF_DEBUG("Calling writeSmsToRuim");
            utfIRadio->writeSmsToRuim(*static_cast<int32_t*>(t), smswrite);
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_CDMA_SMS_ACKNOWLEDGE:
         RIL_UTF_DEBUG("Calling RIL_REQUEST_CDMA_SMS_ACKNOWLEDGE");
         if (utfIRadio != nullptr) {
            V1_0::CdmaSmsAck smsack;
            convertCdmaSmsAcktoHidl(data, datalen, smsack);
            RIL_UTF_DEBUG("Calling acknowledgeLastIncomingCdmaSms");
            utfIRadio->acknowledgeLastIncomingCdmaSms(*static_cast<int32_t*>(t), smsack);
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_CDMA_SET_BROADCAST_SMS_CONFIG:
         RIL_UTF_DEBUG("Calling RIL_REQUEST_CDMA_SET_BROADCAST_SMS_CONFIG");
         if (utfIRadio != nullptr) {
            hidl_vec<CdmaBroadcastSmsConfigInfo> bconfigs;
            convertCdmaBcConfigtoHidl(data, datalen, bconfigs);
            RIL_UTF_DEBUG("Calling setCdmaBroadcastConfig");
            utfIRadio->setCdmaBroadcastConfig(*static_cast<int32_t*>(t), bconfigs);
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_GSM_SET_BROADCAST_SMS_CONFIG:
         RIL_UTF_DEBUG("Calling RIL_REQUEST_GSM_SET_BROADCAST_SMS_CONFIG");
         if (utfIRadio != nullptr) {
            hidl_vec<GsmBroadcastSmsConfigInfo> bconfigs;
            convertGsmBcConfigtoHidl(data, datalen, bconfigs);
            RIL_UTF_DEBUG("Calling setGsmBroadcastConfig");
            utfIRadio->setGsmBroadcastConfig(*static_cast<int32_t*>(t), bconfigs);
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_GSM_GET_BROADCAST_SMS_CONFIG:
         RIL_UTF_DEBUG("Calling RIL_REQUEST_GSM_GET_BROADCAST_SMS_CONFIG");
         if (utfIRadio != nullptr) {
            RIL_UTF_DEBUG("Calling getGsmBroadcastConfig");
            utfIRadio->getGsmBroadcastConfig(*static_cast<int32_t*>(t));
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG:
         RIL_UTF_DEBUG("Calling RIL_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG");
         if (utfIRadio != nullptr) {
            RIL_UTF_DEBUG("Calling getCdmaBroadcastConfig");
            utfIRadio->getCdmaBroadcastConfig(*static_cast<int32_t*>(t));
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_REPORT_SMS_MEMORY_STATUS:
        RIL_UTF_DEBUG("Calling RIL_REQUEST_REPORT_SMS_MEMORY_STATUS");
        if (utfIRadio != nullptr) {
            bool memavail = (((static_cast<int*>(data))[0] == 0)?false:true);
            RIL_UTF_DEBUG("Calling reportSmsMemoryStatus");
            utfIRadio->reportSmsMemoryStatus(*static_cast<int32_t*>(t), memavail);
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_GSM_SMS_BROADCAST_ACTIVATION:
        RIL_UTF_DEBUG("Calling RIL_REQUEST_GSM_SMS_BROADCAST_ACTIVATION");
        if (utfIRadio != nullptr) {
            bool activate = (((static_cast<int*>(data))[0] == 0)?false:true);
            RIL_UTF_DEBUG("Calling setGsmBroadcastActivation");
            utfIRadio->setGsmBroadcastActivation(*static_cast<int32_t*>(t), activate);
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_CDMA_SMS_BROADCAST_ACTIVATION:
        RIL_UTF_DEBUG("Calling RIL_REQUEST_CDMA_SMS_BROADCAST_ACTIVATION");
        if (utfIRadio != nullptr) {
            bool activate = (((static_cast<int*>(data))[0] == 0)?false:true);
            RIL_UTF_DEBUG("Calling setCdmaBroadcastActivation");
            utfIRadio->setCdmaBroadcastActivation(*static_cast<int32_t*>(t), activate);
            return 0;
        } else {
            return 1;
        }
    case RIL_REQUEST_START_NETWORK_SCAN:
        RIL_UTF_DEBUG("Calling RIL_REQUEST_START_NETWORK_SCAN");
        if (utfIRadio != nullptr) {
            RIL_UTF_DEBUG("Calling startNetworkScan_1_4");
            V1_2::NetworkScanRequest nsRequest = {};
            RIL_NetworkScanRequest *rilReq = static_cast<RIL_NetworkScanRequest*>(data);
            convertNetworkScanRequestToHidl(*rilReq, nsRequest);
            utfIRadio->startNetworkScan_1_4(*static_cast<int32_t*>(t), nsRequest);
            return 0;
        }
    case RIL_REQUEST_STOP_NETWORK_SCAN:
        RIL_UTF_DEBUG("Calling RIL_REQUEST_STOP_NETWORK_SCAN");
        if (utfIRadio != nullptr) {
            RIL_UTF_DEBUG("Calling stopNetworkScan");
            utfIRadio->stopNetworkScan(*static_cast<int32_t*>(t));
            return 0;
        }
    case RIL_REQUEST_GET_NEIGHBORING_CELL_IDS:
        RIL_UTF_DEBUG("Calling RIL_REQUEST_GET_NEIGHBORING_CELL_IDS");
        if (utfIRadio != nullptr) {
            RIL_UTF_DEBUG("Calling getNeighboringCids");
            utfIRadio->getNeighboringCids(*static_cast<int32_t*>(t));
            return 0;
        }

    default:
        return 1;
    }

    return 1;
}

static void __attribute__((constructor)) registerPlatform() {
    static AndroidIRadio iradio;
    setPlatform(&iradio);
}
