/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#include "ImsVoiceModule.h"
#include <framework/Module.h>
#include <framework/PolicyManager.h>
#include <framework/SingleDispatchAndFamilyPairTimerRestriction.h>
#include <interfaces/ims/ims.h>
#include <modules/nas/qcril_qmi_nas.h>
#include <modules/qmi/EndpointStatusIndMessage.h>
#include <modules/voice/qcril_qmi_voice.h>
#include "qcril_am.h"
#include "qcril_cm_ss.h"
#include "qcril_other.h"

#define TAG "RILQ"

ImsVoiceModule::ImsVoiceModule() : VoiceModule("ImsVoiceModule") {
  using std::placeholders::_1;

  std::unordered_map<message_id_t, std::function<void(std::shared_ptr<Message>)>>
      subMessageHandler = {
          HANDLER(QcRilRequestImsDialMessage, ImsVoiceModule::handleQcRilRequestImsDialMessage),
          HANDLER(QcRilRequestImsHangupMessage, ImsVoiceModule::handleQcRilRequestImsHangupMessage),
          HANDLER(QcRilRequestImsHoldMessage, ImsVoiceModule::handleQcRilRequestImsHoldMessage),
          HANDLER(QcRilRequestImsResumeMessage, ImsVoiceModule::handleQcRilRequestImsResumeMessage),
          HANDLER(QcRilRequestImsAnswerMessage, ImsVoiceModule::handleQcRilRequestImsAnswerMessage),
          HANDLER(QcRilRequestImsAddParticipantMessage,
                  ImsVoiceModule::handleQcRilRequestImsAddParticipantMessage),
          HANDLER(QcRilRequestImsConferenceMessage,
                  ImsVoiceModule::handleQcRilRequestImsConferenceMessage),
          HANDLER(QcRilRequestImsModifyCallInitiateMessage,
                  ImsVoiceModule::handleQcRilRequestImsModifyCallInitiateMessage),
          HANDLER(QcRilRequestImsCancelModifyCallMessage,
                  ImsVoiceModule::handleQcRilRequestImsCancelModifyCallMessage),
          HANDLER(QcRilRequestImsModifyCallConfirmMessage,
                  ImsVoiceModule::handleQcRilRequestImsModifyCallConfirmMessage),
          HANDLER(QcRilRequestImsExplicitCallTransferMessage,
                  ImsVoiceModule::handleQcRilRequestImsExplicitCallTransferMessage),
          HANDLER(QcRilRequestImsDeflectCallMessage,
                  ImsVoiceModule::handleQcRilRequestImsDeflectCallMessage),
          HANDLER(QcRilRequestImsSetUiTtyModeMessage,
                  ImsVoiceModule::handleQcRilRequestImsSetUiTtyModeMessage),
          HANDLER(QcRilRequestImsSendDtmfMessage,
                  ImsVoiceModule::handleQcRilRequestImsSendDtmfMessage),
          HANDLER(QcRilRequestImsStartDtmfMessage,
                  ImsVoiceModule::handleQcRilRequestImsStartDtmfMessage),
          HANDLER(QcRilRequestImsStopDtmfMessage,
                  ImsVoiceModule::handleQcRilRequestImsStopDtmfMessage),
      };
  mMessageHandler.insert(subMessageHandler.begin(), subMessageHandler.end());
}

ImsVoiceModule::~ImsVoiceModule() {}

void ImsVoiceModule::init() {
  QCRIL_LOG_FUNC_ENTRY();
  /* Call base init before doing any other stuff.*/
  VoiceModule::init();

  // ring1: DTMF related. Set DTMF_START and DTMF_STOP as family pair
  std::vector<std::string> ring1{
      QcRilRequestImsSendDtmfMessage::MESSAGE_NAME,
      QcRilRequestImsStartDtmfMessage::MESSAGE_NAME,
      QcRilRequestImsStopDtmfMessage::MESSAGE_NAME,
  };
  auto rest = std::make_shared<SingleDispatchAndFamilyPairTimerRestriction>();
  if (rest) {
    rest->setMessagePair(QcRilRequestImsStartDtmfMessage::MESSAGE_NAME,
                         QcRilRequestImsStopDtmfMessage::MESSAGE_NAME);
    int timer = 0;
    // Start to Stop time
    qmi_ril_get_property_value_from_integer(QCRIL_QMI_VOICE_DTMF_INTERVAL, &timer, 0);
    rest->setMessageTimer(QcRilRequestImsStartDtmfMessage::MESSAGE_NAME, timer);
    // Pause time
    qmi_ril_get_property_value_from_integer(QCRIL_QMI_VOICE_DTMF_PAUSE_INTERVAL, &timer, 0);
    rest->setMessageTimer(QcRilRequestImsStopDtmfMessage::MESSAGE_NAME, timer);
  }
  PolicyManager::getInstance().setMessageRestriction(ring1, rest);

  QCRIL_LOG_FUNC_RETURN();
}

void ImsVoiceModule::handleQcRilRequestImsDialMessage(
    std::shared_ptr<QcRilRequestImsDialMessage> msg) {
  QCRIL_LOG_FUNC_ENTRY("msg = %s", (msg ? msg->dump().c_str() : "nullptr"));

  voice_dial_call_req_msg_v02 dial_call_req = {};
  RIL_Errno rilErr = RIL_E_SUCCESS;
  qcril_qmi_voice_voip_call_info_elaboration_type elaboration = 0;
  auto pendingMsgStatus = std::make_pair(0, false);

  if (msg != nullptr) {
    do {
      if (!(msg->hasAddress() || msg->getAddress().empty()) &&
          !(msg->hasIsConferenceUri() || msg->getIsConferenceUri())) {
        QCRIL_LOG_ERROR("Invalid number");
        break;
      }
      if (msg->hasIsConferenceUri()) {
        if (msg->getAddress().size() > QMI_VOICE_CONF_URI_LIST_MAX_LEN_V02) {
          QCRIL_LOG_ERROR("Invalid conf uri");
          break;
        }
        memcpy(dial_call_req.calling_number, "Conference Call", strlen("Conference Call"));
        dial_call_req.conf_uri_list_valid = TRUE;
        memcpy(dial_call_req.conf_uri_list, msg->getAddress().c_str(),
               std::min(sizeof(dial_call_req.calling_number), msg->getAddress().size()));
      } else {
        string subAddress = "";
        string sipUriOverflow = "";
        string callingNumber = "";
        rilErr = convert_dial_call_req_number(msg->getAddress(), callingNumber, sipUriOverflow,
                                              subAddress);
        if (rilErr != RIL_E_SUCCESS) {
          QCRIL_LOG_ERROR("Invalid number");
          break;
        }
        if (!subAddress.empty()) {
          dial_call_req.called_party_subaddress_valid = TRUE;
          // Always set to 1 according to spec Table 10.5.119/3GPP TS 24.008
          dial_call_req.called_party_subaddress.extension_bit = 1;
          dial_call_req.called_party_subaddress.subaddress_type = SUBADDRESS_TYPE_NSAP_V02;
          dial_call_req.called_party_subaddress.odd_even_ind = subAddress.size() % 2;
          memcpy(dial_call_req.called_party_subaddress.subaddress, subAddress.c_str(),
                 std::min(sizeof(dial_call_req.called_party_subaddress.subaddress),
                          subAddress.size()));
          dial_call_req.called_party_subaddress.subaddress_len = subAddress.size();
        }
        if (!callingNumber.empty()) {
          memcpy(dial_call_req.calling_number, callingNumber.c_str(),
                 std::min(sizeof(dial_call_req.calling_number), callingNumber.size()));
        }
        if (!sipUriOverflow.empty()) {
          dial_call_req.sip_uri_overflow_valid = TRUE;
          memcpy(dial_call_req.sip_uri_overflow, sipUriOverflow.c_str(),
                 std::min(sizeof(dial_call_req.sip_uri_overflow), sipUriOverflow.size()));
        }
      }
      if (msg->hasClir()) {
        dial_call_req.clir_type_valid =
            convert_dial_call_req_clir_type(msg->getClir(), dial_call_req.clir_type);
        if (dial_call_req.clir_type_valid) {
          dial_call_req.pi_valid = TRUE;
          dial_call_req.pi = (dial_call_req.clir_type == CLIR_INVOCATION_V02)
                                 ? IP_PRESENTATION_NUM_RESTRICTED_V02
                                 : IP_PRESENTATION_NUM_ALLOWED_V02;
        }
      }
      if (msg->hasIsCallPull()) {
        dial_call_req.call_pull_valid = TRUE;
        dial_call_req.call_pull = msg->getIsCallPull() ? TRUE : FALSE;
      }
      if (msg->hasIsEncrypted()) {
        dial_call_req.is_secure_call_valid = TRUE;
        dial_call_req.is_secure_call = msg->getIsEncrypted() ? TRUE : FALSE;
      }
      if (msg->hasRttMode()) {
        dial_call_req.rtt_mode_valid =
            convert_rtt_mode_to_qmi(msg->getRttMode(), dial_call_req.rtt_mode);
      }
      elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_AUTO_DOMAIN;
      if (msg->hasCallType() && msg->hasCallDomain()) {
        dial_call_req.call_type_valid = TRUE;
        convert_call_info_to_qmi(msg->getCallType(), msg->getCallDomain(), false,
                                 dial_call_req.call_type, dial_call_req.audio_attrib_valid,
                                 dial_call_req.audio_attrib, dial_call_req.video_attrib_valid,
                                 dial_call_req.video_attrib);
        if (msg->getCallDomain() == qcril::interfaces::CallDomain::PS) {
          elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_PS_DOMAIN;
        }
        if (msg->getCallDomain() == qcril::interfaces::CallDomain::AUTOMATIC) {
          dial_call_req.service_type_valid = TRUE;
          dial_call_req.service_type =
              qcril_qmi_nas_setting_srv_type_based_on_elaboration_and_rat(elaboration);
        }
      }
      if (msg->hasDisplayText()) {
        dial_call_req.display_text_valid = TRUE;
        dial_call_req.display_text_len = qcril_cm_ss_convert_utf8_to_ucs2(
            msg->getDisplayText().c_str(), msg->getDisplayText().size(),
            (char *)dial_call_req.display_text, sizeof(dial_call_req.display_text));
        if (dial_call_req.display_text_len) {
          dial_call_req.display_text_len /= 2;
        }
      }
      if (msg->hasRetryCallFailReason()) {
        dial_call_req.call_failure_reason_valid = convert_call_fail_reason_to_qmi(
            msg->getRetryCallFailReason(), dial_call_req.call_failure_reason);
      }
      if (msg->hasRetryCallFailMode()) {
        dial_call_req.call_failure_mode_valid =
            convert_call_mode_to_qmi(msg->getRetryCallFailMode(), dial_call_req.call_failure_mode);
      }
      if (msg->hasOriginatingNumber() && msg->getOriginatingNumber().size() > 0) {
        dial_call_req.origination_number_valid = TRUE;
        strlcpy(dial_call_req.origination_number, msg->getOriginatingNumber().c_str(),
                sizeof(dial_call_req.origination_number));
        dial_call_req.is_secondary_valid = TRUE;
        dial_call_req.is_secondary = (msg->hasIsSecondary() && msg->getIsSecondary());
      }

      pendingMsgStatus = getPendingMessageList().insert(msg);
      if (pendingMsgStatus.second != true) {
        QCRIL_LOG_ERROR("getPendingMessageList().insert failed!!");
        rilErr = RIL_E_GENERIC_FAILURE;
        break;
      }
      uint16_t req_id = pendingMsgStatus.first;

      rilErr = qcril_qmi_voice_process_dial_call_req(
          req_id, dial_call_req,
          std::bind(&ImsVoiceModule::processDialCallResponse, this, std::placeholders::_1),
          std::bind(&ImsVoiceModule::processDialCommandOversightCompletion, this,
                    std::placeholders::_1),
          elaboration);
    } while (0);

    if (rilErr != RIL_E_SUCCESS) {
      if (pendingMsgStatus.second) {
        getPendingMessageList().erase(pendingMsgStatus.first);
      }
      auto respPayload = std::make_shared<QcRilRequestMessageCallbackPayload>(rilErr, nullptr);
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

void ImsVoiceModule::handleQcRilRequestImsHangupMessage(
    std::shared_ptr<QcRilRequestImsHangupMessage> msg) {
  QCRIL_LOG_FUNC_ENTRY("msg = %s", (msg ? msg->dump().c_str() : "nullptr"));

  RIL_Errno rilErr = RIL_E_SUCCESS;
  auto pendingMsgStatus = std::make_pair(0, false);

  if (msg != nullptr) {
    do {
      pendingMsgStatus = getPendingMessageList().insert(msg);
      if (pendingMsgStatus.second != true) {
        QCRIL_LOG_ERROR("getPendingMessageList().insert failed!!");
        rilErr = RIL_E_GENERIC_FAILURE;
        break;
      }
      uint16_t req_id = pendingMsgStatus.first;

      if (msg->hasConnectionUri() && !msg->getConnectionUri().empty()) {
        if (msg->getConnectionUri().size() > QMI_VOICE_SIP_URI_MAX_V02) {
          QCRIL_LOG_ERROR("ConnectionUri len > QMI_VOICE_SIP_URI_MAX_V02");
          rilErr = RIL_E_INVALID_ARGUMENTS;
          break;
        }
        voice_manage_ip_calls_req_msg_v02 manage_ip_calls_req = {};
        manage_ip_calls_req.sups_type = VOIP_SUPS_TYPE_RELEASE_SPECIFIED_CALL_FROM_CONFERENCE_V02;
        manage_ip_calls_req.sip_uri_valid = TRUE;
        memcpy(manage_ip_calls_req.sip_uri, msg->getConnectionUri().c_str(),
               msg->getConnectionUri().size());

        rilErr = qcril_qmi_voice_process_manage_ip_calls_req(
            req_id, manage_ip_calls_req,
            std::bind(&ImsVoiceModule::processReleaseParticipantResponse, this,
                      std::placeholders::_1));
      }
      if (msg->hasCallIndex()) {
        if (qcril_qmi_voice_cancel_pending_call(true)) {
          QCRIL_LOG_INFO("Cancelled a pending call request; send failure response");
          rilErr = RIL_E_GENERIC_FAILURE;
          break;
        }
        int connId = msg->getCallIndex();
        if (connId == -1) {
          if (qcril_qmi_process_hangup_on_call_being_setup(&connId) != RIL_E_SUCCESS) {
            /* Could not retrieve call id */
            rilErr = RIL_E_INVALID_ARGUMENTS;
            break;
          } else if (connId == -1) {
            /* Have not received call id, hence we will wait to receive call id. */
            // The msg stored in getPendingMessageList() will be poped and re-evalue once the call id
            // is available.
            rilErr = RIL_E_SUCCESS;
            break;
          }
        }
        voice_reject_cause_enum_v02 qmi_rej_cause = (voice_reject_cause_enum_v02)0xFF;
        bool need_to_reject_incoming_call = false;

        if (msg->hasRejectCause() &&
            msg->getRejectCause() != qcril::interfaces::CallFailCause::MISC) {
          need_to_reject_incoming_call =
              qcril_qmi_ims_map_ims_failcause_qmi(msg->getRejectCause(), qmi_rej_cause);
        } else if (msg->hasRejectCauseRaw()) {
          qmi_rej_cause = (voice_reject_cause_enum_v02)msg->getRejectCauseRaw();
          need_to_reject_incoming_call = true;
        }

        qcril_qmi_voice_voip_call_info_entry_type *call_info_entry =
            qcril_qmi_voice_voip_find_call_info_entry_by_call_android_id(connId);
        if (call_info_entry == nullptr) {
          QCRIL_LOG_ERROR("Failed to find call entry, aborting!");
          rilErr = RIL_E_INVALID_ARGUMENTS;
          break;
        }

        QCRIL_LOG_INFO(
            "Is pending incoming call: %s",
            call_info_entry->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_PENDING_INCOMING
                ? "TRUE"
                : "FALSE");

        if (need_to_reject_incoming_call &&
            !(call_info_entry->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_PENDING_INCOMING)) {
          need_to_reject_incoming_call = false;
        }

        if (need_to_reject_incoming_call) {
          voice_answer_call_req_msg_v02 ans_call_req = {};
          ans_call_req.call_id = call_info_entry->qmi_call_id;
          ans_call_req.reject_call_valid = TRUE;
          ans_call_req.reject_call = TRUE;
          ans_call_req.reject_cause_valid = TRUE;
          ans_call_req.reject_cause = qmi_rej_cause;

          rilErr = qcril_qmi_voice_process_answer_call_req(
              req_id, ans_call_req,
              std::bind(&ImsVoiceModule::processAnswerResponse, this, std::placeholders::_1),
              std::bind(&ImsVoiceModule::processAnswerCommandOversightCompletion, this,
                        std::placeholders::_1));
        } else {
          voice_end_call_req_msg_v02 end_call_req = {};
          end_call_req.call_id = call_info_entry->qmi_call_id;
          if (qmi_rej_cause != 0xFF) {
            end_call_req.end_cause_valid = TRUE;
            end_call_req.end_cause = qmi_rej_cause;
          }

          rilErr = qcril_qmi_voice_process_end_call_req(
              req_id, end_call_req,
              std::bind(&ImsVoiceModule::processEndCallResponse, this, std::placeholders::_1),
              std::bind(&ImsVoiceModule::processHangupCommandOversightCompletion, this,
                        std::placeholders::_1));
        }
      }
    } while (0);

    if (rilErr != RIL_E_SUCCESS) {
      if (pendingMsgStatus.second) {
        getPendingMessageList().erase(pendingMsgStatus.first);
      }
      std::shared_ptr<QcRilRequestMessageCallbackPayload> respPayload =
          std::make_shared<QcRilRequestMessageCallbackPayload>(rilErr, nullptr);
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

void ImsVoiceModule::handleQcRilRequestImsAnswerMessage(
    std::shared_ptr<QcRilRequestImsAnswerMessage> msg) {
  QCRIL_LOG_FUNC_ENTRY("msg = %s", (msg ? msg->dump().c_str() : "nullptr"));

  RIL_Errno rilErr = RIL_E_SUCCESS;
  qcril_qmi_voice_voip_call_info_entry_type *call_info_entry = nullptr;
  voice_answer_call_req_msg_v02 ans_call_req = {};
  auto pendingMsgStatus = std::make_pair(0, false);

  if (msg != nullptr) {
    do {
      call_info_entry = qcril_qmi_voice_voip_find_call_info_entry_by_elaboration(
          QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_PENDING_INCOMING, TRUE);
      if (NULL == call_info_entry) {
        QCRIL_LOG_INFO(".. pending incoming call record entry not found");
        rilErr = RIL_E_INVALID_STATE;
        break;
      }
      ans_call_req.call_id = call_info_entry->qmi_call_id;
      if (msg->hasCallType()) {
        /* In case of voice call, call type is not expected in answer */
        if (call_info_entry->voice_scv_info.call_type == CALL_TYPE_VT_V02) {
          if (msg->getCallType() == qcril::interfaces::CallType::VT) {
            ans_call_req.call_type_valid = TRUE;
            ans_call_req.call_type = CALL_TYPE_VT_V02;

            ans_call_req.audio_attrib_valid = TRUE;
            ans_call_req.audio_attrib = (VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);

            ans_call_req.video_attrib_valid = TRUE;
            ans_call_req.video_attrib = (VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);
          } else if (msg->getCallType() == qcril::interfaces::CallType::VT_TX) {
            ans_call_req.call_type_valid = TRUE;
            ans_call_req.call_type = CALL_TYPE_VT_V02;

            ans_call_req.audio_attrib_valid = TRUE;
            ans_call_req.audio_attrib = (VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);

            ans_call_req.video_attrib_valid = TRUE;
            ans_call_req.video_attrib = VOICE_CALL_ATTRIB_TX_V02;
          } else if (msg->getCallType() == qcril::interfaces::CallType::VT_RX) {
            ans_call_req.call_type_valid = TRUE;
            ans_call_req.call_type = CALL_TYPE_VT_V02;

            ans_call_req.audio_attrib_valid = TRUE;
            ans_call_req.audio_attrib = (VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);

            ans_call_req.video_attrib_valid = TRUE;
            ans_call_req.video_attrib = VOICE_CALL_ATTRIB_RX_V02;
          } else {
            ans_call_req.call_type_valid = TRUE;
            ans_call_req.call_type = CALL_TYPE_VOICE_IP_V02;
          }
          // Cache the user specified call type in answer.
          call_info_entry->answered_call_type_valid = TRUE;
          call_info_entry->answered_call_type = msg->getCallType();
        }
        QCRIL_LOG_DEBUG("modem call type = %d", ans_call_req.call_type);
      }

      if (msg->hasPresentation()) {
        ans_call_req.pi_valid =
            convert_presentation_to_qmi(msg->getPresentation(), ans_call_req.pi);
      }
      if (msg->hasRttMode()) {
        ans_call_req.rtt_mode_valid =
            convert_rtt_mode_to_qmi(msg->getRttMode(), ans_call_req.rtt_mode);
      }

      pendingMsgStatus = getPendingMessageList().insert(msg);
      if (pendingMsgStatus.second != true) {
        QCRIL_LOG_ERROR("getPendingMessageList().insert failed!!");
        rilErr = RIL_E_GENERIC_FAILURE;
        break;
      }
      uint16_t req_id = pendingMsgStatus.first;

      rilErr = qcril_qmi_voice_process_answer_call_req(
          req_id, ans_call_req,
          std::bind(&ImsVoiceModule::processAnswerResponse, this, std::placeholders::_1),
          std::bind(&ImsVoiceModule::processAnswerCommandOversightCompletion, this,
                    std::placeholders::_1));
      if (rilErr == RIL_E_SUCCESS) {
        qcril_am_handle_event(QCRIL_AM_EVENT_IMS_ANSWER, NULL);
      }
    } while (0);
    if (rilErr != RIL_E_SUCCESS) {
      if (pendingMsgStatus.second) {
        getPendingMessageList().erase(pendingMsgStatus.first);
      }
      std::shared_ptr<QcRilRequestMessageCallbackPayload> respPayload =
          std::make_shared<QcRilRequestMessageCallbackPayload>(rilErr, nullptr);
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
    }
  }

  QCRIL_LOG_FUNC_RETURN();
}

void ImsVoiceModule::handleQcRilRequestImsHoldMessage(
    std::shared_ptr<QcRilRequestImsHoldMessage> msg) {
  QCRIL_LOG_FUNC_ENTRY("msg = %s", (msg ? msg->dump().c_str() : "nullptr"));

  RIL_Errno rilErr = RIL_E_SUCCESS;
  qcril_qmi_voice_voip_call_info_entry_type *call_info = nullptr;
  qcril_qmi_voice_voip_current_call_summary_type calls_summary = {};
  voice_manage_ip_calls_req_msg_v02 manage_ip_calls_req = {};
  auto pendingMsgStatus = std::make_pair(0, false);

  if (msg != nullptr) {
    do {
      if (!msg->hasCallId()) {
        QCRIL_LOG_ERROR("Invalid callId");
        rilErr = RIL_E_INVALID_STATE;
        break;
      }

      qcril_qmi_voice_voip_generate_summary(&calls_summary);
      QCRIL_LOG_ERROR("Number of voip calls = %d", calls_summary.nof_voip_calls);
      if (calls_summary.nof_voip_calls <= QMI_RIL_ZERO) {
        QCRIL_LOG_ERROR("Do not have any voip active calls");
        rilErr = RIL_E_INVALID_STATE;
        break;
      }

      uint8_t call_id = msg->getCallId();
      call_info = qcril_qmi_voice_voip_find_call_info_entry_by_call_android_id(call_id);
      if (!call_info) {
        QCRIL_LOG_ERROR("Do not have any voip active calls or invalid call id in request");
        rilErr = RIL_E_GENERIC_FAILURE;
        break;
      }

      manage_ip_calls_req.sups_type = VOIP_SUPS_TYPE_CALL_HOLD_V02;
      manage_ip_calls_req.call_id_valid = TRUE;
      manage_ip_calls_req.call_id = call_info->voice_scv_info.call_id;

      pendingMsgStatus = getPendingMessageList().insert(msg);
      if (pendingMsgStatus.second != true) {
        QCRIL_LOG_ERROR("getPendingMessageList().insert failed!!");
        rilErr = RIL_E_GENERIC_FAILURE;
        break;
      }
      uint16_t req_id = pendingMsgStatus.first;

      rilErr = qcril_qmi_voice_process_manage_ip_calls_req(
          req_id, manage_ip_calls_req,
          std::bind(&ImsVoiceModule::processHoldResponse, this, std::placeholders::_1));
    } while (0);

    if (rilErr != RIL_E_SUCCESS) {
      if (pendingMsgStatus.second) {
        getPendingMessageList().erase(pendingMsgStatus.first);
      }
      std::shared_ptr<QcRilRequestMessageCallbackPayload> respPayload =
          std::make_shared<QcRilRequestMessageCallbackPayload>(rilErr, nullptr);
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

void ImsVoiceModule::handleQcRilRequestImsResumeMessage(
    std::shared_ptr<QcRilRequestImsResumeMessage> msg) {
  QCRIL_LOG_FUNC_ENTRY("msg = %s", (msg ? msg->dump().c_str() : "nullptr"));

  RIL_Errno rilErr = RIL_E_SUCCESS;
  qcril_qmi_voice_voip_call_info_entry_type *call_info = nullptr;
  qcril_qmi_voice_voip_current_call_summary_type calls_summary = {};
  voice_manage_ip_calls_req_msg_v02 manage_ip_calls_req = {};
  auto pendingMsgStatus = std::make_pair(0, false);

  if (msg != nullptr) {
    do {
      if (!msg->hasCallId()) {
        QCRIL_LOG_ERROR("Invalid callId");
        rilErr = RIL_E_INVALID_STATE;
        break;
      }

      qcril_qmi_voice_voip_generate_summary(&calls_summary);

      QCRIL_LOG_ERROR("Number of voip calls = %d", calls_summary.nof_voip_calls);
      if (calls_summary.nof_voip_calls <= QMI_RIL_ZERO) {
        QCRIL_LOG_ERROR("Do not have any voip active calls");
        rilErr = RIL_E_INVALID_STATE;
        break;
      }

      uint8_t call_id = msg->getCallId();
      call_info = qcril_qmi_voice_voip_find_call_info_entry_by_call_android_id(call_id);
      if (!call_info) {
        QCRIL_LOG_ERROR("Do not have any voip active calls or invalid call id in request");
        rilErr = RIL_E_GENERIC_FAILURE;
        break;
      }

      manage_ip_calls_req.sups_type = VOIP_SUPS_TYPE_CALL_RESUME_V02;
      manage_ip_calls_req.call_id_valid = TRUE;
      manage_ip_calls_req.call_id = call_info->voice_scv_info.call_id;

      pendingMsgStatus = getPendingMessageList().insert(msg);
      if (pendingMsgStatus.second != true) {
        QCRIL_LOG_ERROR("getPendingMessageList().insert failed!!");
        rilErr = RIL_E_GENERIC_FAILURE;
        break;
      }
      uint16_t req_id = pendingMsgStatus.first;

      rilErr = qcril_qmi_voice_process_manage_ip_calls_req(
          req_id, manage_ip_calls_req,
          std::bind(&ImsVoiceModule::processResumeResponse, this, std::placeholders::_1));
    } while (0);

    if (rilErr != RIL_E_SUCCESS) {
      if (pendingMsgStatus.second) {
        getPendingMessageList().erase(pendingMsgStatus.first);
      }
      std::shared_ptr<QcRilRequestMessageCallbackPayload> respPayload =
          std::make_shared<QcRilRequestMessageCallbackPayload>(rilErr, nullptr);
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
    } else {
      if (call_info) {
        call_info->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_SWITCHING_CALL_TO_ACTIVE;
        qcril_am_handle_event(QCRIL_AM_EVENT_SWITCH_CALL, NULL);
      }
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

void ImsVoiceModule::handleQcRilRequestImsAddParticipantMessage(
    std::shared_ptr<QcRilRequestImsAddParticipantMessage> msg) {
  QCRIL_LOG_FUNC_ENTRY("msg = %s", (msg ? msg->dump().c_str() : "nullptr"));

  RIL_Errno rilErr = RIL_E_SUCCESS;
  qcril_qmi_voice_voip_current_call_summary_type calls_summary = {};
  voice_manage_ip_calls_req_msg_v02 manage_ip_calls_req = {};
  qcril_qmi_voice_voip_call_info_entry_type *call_info_entry = nullptr;
  auto pendingMsgStatus = std::make_pair(0, false);

  if (msg != nullptr) {
    do {
      if (msg->getAddress().size() > QMI_VOICE_SIP_URI_MAX_V02) {
        QCRIL_LOG_ERROR("invalid parameter");
        rilErr = RIL_E_INVALID_STATE;
        break;
      }

      qcril_qmi_voice_voip_generate_summary(&calls_summary);
      QCRIL_LOG_DEBUG("number of calls = %d", calls_summary.nof_calls_overall);

      if (calls_summary.nof_calls_overall == 1) {
        call_info_entry = calls_summary.active_or_single_call;
      } else {
        // Get the call in conversation state if more than one call
        call_info_entry = qcril_qmi_voice_voip_find_call_info_entry_by_qmi_call_state(
            CALL_STATE_CONVERSATION_V02);
      }

      QCRIL_LOG_INFO(".. call_info_entry %p", call_info_entry);

      if (!call_info_entry || (VOICE_INVALID_CALL_ID == call_info_entry->qmi_call_id) ||
          !(CALL_STATE_CONVERSATION_V02 == call_info_entry->voice_scv_info.call_state ||
            CALL_STATE_HOLD_V02 == call_info_entry->voice_scv_info.call_state) ||
          !(CALL_TYPE_VOICE_IP_V02 == call_info_entry->voice_scv_info.call_type ||
            CALL_TYPE_VT_V02 == call_info_entry->voice_scv_info.call_type)) {
        QCRIL_LOG_ERROR("No valid call info entry");
        qcril_qmi_voice_voip_call_info_dump(call_info_entry);
        rilErr = RIL_E_INVALID_STATE;
        break;
      }

      manage_ip_calls_req.sups_type = VOIP_SUPS_TYPE_ADD_PARTICIPANT_V02;
      manage_ip_calls_req.call_id_valid = TRUE;
      manage_ip_calls_req.call_id = call_info_entry->voice_scv_info.call_id;
      manage_ip_calls_req.sip_uri_valid = TRUE;
      memcpy(&manage_ip_calls_req.sip_uri, msg->getAddress().c_str(), msg->getAddress().size());

      pendingMsgStatus = getPendingMessageList().insert(msg);
      if (pendingMsgStatus.second != true) {
        QCRIL_LOG_ERROR("getPendingMessageList().insert failed!!");
        rilErr = RIL_E_GENERIC_FAILURE;
        break;
      }
      uint16_t req_id = pendingMsgStatus.first;

      rilErr = qcril_qmi_voice_process_manage_ip_calls_req(
          req_id, manage_ip_calls_req,
          std::bind(&ImsVoiceModule::processAddParticipantResponse, this, std::placeholders::_1));
    } while (0);

    if (rilErr != RIL_E_SUCCESS) {
      if (pendingMsgStatus.second) {
        getPendingMessageList().erase(pendingMsgStatus.first);
      }
      std::shared_ptr<QcRilRequestMessageCallbackPayload> respPayload =
          std::make_shared<QcRilRequestMessageCallbackPayload>(rilErr, nullptr);
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
    }
  }

  QCRIL_LOG_FUNC_RETURN();
}

void ImsVoiceModule::handleQcRilRequestImsConferenceMessage(
    std::shared_ptr<QcRilRequestImsConferenceMessage> msg) {
  QCRIL_LOG_FUNC_ENTRY("msg = %s", (msg ? msg->dump().c_str() : "nullptr"));

  RIL_Errno rilErr = RIL_E_SUCCESS;
  qcril_qmi_voice_voip_current_call_summary_type calls_summary = {};
  voice_manage_ip_calls_req_msg_v02 manage_ip_calls_req = {};
  auto pendingMsgStatus = std::make_pair(0, false);

  if (msg != nullptr) {
    do {
      qcril_qmi_voice_voip_generate_summary(&calls_summary);

      QCRIL_LOG_ERROR("Number of voice calls = %d, Number of voip calls = %d",
                      calls_summary.nof_voice_calls, calls_summary.nof_voip_calls);
      if (calls_summary.nof_voip_calls <= QMI_RIL_ZERO) {
        rilErr = RIL_E_INVALID_STATE;
        break;
      }

      manage_ip_calls_req.sups_type = VOIP_SUPS_TYPE_MAKE_CONFERENCE_CALL_V02;

      pendingMsgStatus = getPendingMessageList().insert(msg);
      if (pendingMsgStatus.second != true) {
        QCRIL_LOG_ERROR("getPendingMessageList().insert failed!!");
        rilErr = RIL_E_GENERIC_FAILURE;
        break;
      }
      uint16_t req_id = pendingMsgStatus.first;

      rilErr = qcril_qmi_voice_process_manage_ip_calls_req(
          req_id, manage_ip_calls_req,
          std::bind(&ImsVoiceModule::processConferenceResponse, this, std::placeholders::_1));
    } while (0);

    if (rilErr != RIL_E_SUCCESS) {
      if (pendingMsgStatus.second) {
        getPendingMessageList().erase(pendingMsgStatus.first);
      }
      std::shared_ptr<QcRilRequestMessageCallbackPayload> respPayload =
          std::make_shared<QcRilRequestMessageCallbackPayload>(rilErr, nullptr);
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

void ImsVoiceModule::handleQcRilRequestImsModifyCallInitiateMessage(
    std::shared_ptr<QcRilRequestImsModifyCallInitiateMessage> msg) {
  QCRIL_LOG_FUNC_ENTRY("msg = %s", (msg ? msg->dump().c_str() : "nullptr"));

  RIL_Errno rilErr = RIL_E_SUCCESS;
  voice_manage_ip_calls_req_msg_v02 manage_ip_calls_req = {};
  qcril_qmi_voice_voip_call_info_entry_type *call_info = nullptr;
  auto pendingMsgStatus = std::make_pair(0, false);
  boolean is_emergency_ip = FALSE;

  if (msg != nullptr) {
    do {
      if (msg->hasCallId() && msg->getCallId() >= VOICE_LOWEST_CALL_ID &&
          msg->getCallId() <= VOICE_HIGHEST_CALL_ID) {
        call_info = qcril_qmi_voice_voip_find_call_info_entry_by_call_android_id(msg->getCallId());
      } else {
        QCRIL_LOG_ERROR("Invalid CallId");
        rilErr = RIL_E_GENERIC_FAILURE;
        break;
      }

      if (call_info == NULL) {
        QCRIL_LOG_DEBUG("android call-id = %d not found, rejecting modify initiate request",
                        msg->getCallId());
        rilErr = RIL_E_GENERIC_FAILURE;
        break;
      }

      if (call_info->voice_scv_info.call_type == CALL_TYPE_EMERGENCY_IP_V02 ||
          call_info->voice_scv_info.call_type == CALL_TYPE_EMERGENCY_VT_V02) {
        is_emergency_ip = TRUE;
      }
      if (!msg->hasCallType() || msg->getCallType() == qcril::interfaces::CallType::UNKNOWN) {
        QCRIL_LOG_DEBUG("Invalid CallType");
        rilErr = RIL_E_GENERIC_FAILURE;
        break;
      }
      if (!msg->hasCallDomain() || msg->getCallDomain() == qcril::interfaces::CallDomain::UNKNOWN) {
        QCRIL_LOG_DEBUG("Invalid CallDomain");
        rilErr = RIL_E_GENERIC_FAILURE;
        break;
      }
      uint8_t ret = convert_call_info_to_qmi(
          msg->getCallType(), msg->getCallDomain(), is_emergency_ip, manage_ip_calls_req.call_type,
          manage_ip_calls_req.audio_attrib_valid, manage_ip_calls_req.audio_attrib,
          manage_ip_calls_req.video_attrib_valid, manage_ip_calls_req.video_attrib);

      if (!ret) {
        QCRIL_LOG_DEBUG("Failed to convert the call type details");
        rilErr = RIL_E_GENERIC_FAILURE;
        break;
      }
      if (msg->hasRttMode()) {
        manage_ip_calls_req.rtt_mode_valid =
            convert_rtt_mode_to_qmi(msg->getRttMode(), manage_ip_calls_req.rtt_mode);
      }
      manage_ip_calls_req.call_type_valid = TRUE;
      manage_ip_calls_req.call_id_valid = TRUE;
      manage_ip_calls_req.call_id = call_info->qmi_call_id;
      manage_ip_calls_req.sups_type = VOIP_SUPS_TYPE_MODIFY_CALL_V02;

      pendingMsgStatus = getPendingMessageList().insert(msg);
      if (pendingMsgStatus.second != true) {
        QCRIL_LOG_ERROR("getPendingMessageList().insert failed!!");
        rilErr = RIL_E_GENERIC_FAILURE;
        break;
      }
      msg->setPendingMessageState(PendingMessageState::AWAITING_CALLBACK);
      uint16_t req_id = pendingMsgStatus.first;

      rilErr = qcril_qmi_voice_process_manage_ip_calls_req(
          req_id, manage_ip_calls_req,
          std::bind(&ImsVoiceModule::processModifyCallInitiateResponse, this,
                    std::placeholders::_1));
    } while (0);

    if (rilErr != RIL_E_SUCCESS) {
      if (pendingMsgStatus.second) {
        getPendingMessageList().erase(pendingMsgStatus.first);
      }
      std::shared_ptr<QcRilRequestMessageCallbackPayload> respPayload =
          std::make_shared<QcRilRequestMessageCallbackPayload>(rilErr, nullptr);
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

void ImsVoiceModule::handleQcRilRequestImsCancelModifyCallMessage(
    std::shared_ptr<QcRilRequestImsCancelModifyCallMessage> msg) {
  QCRIL_LOG_FUNC_ENTRY("msg = %s", (msg ? msg->dump().c_str() : "nullptr"));

  RIL_Errno rilErr = RIL_E_SUCCESS;
  voice_ims_call_cancel_req_msg_v02 call_cancel_req = {};
  qcril_qmi_voice_voip_call_info_entry_type *call_info = nullptr;
  auto pendingMsgStatus = std::make_pair(0, false);

  if (msg != nullptr) {
    do {
      if (!msg->hasCallId()) {
        QCRIL_LOG_ERROR("Invalid call id");
        rilErr = RIL_E_GENERIC_FAILURE;
        break;
      }
      call_info = qcril_qmi_voice_voip_find_call_info_entry_by_call_android_id(msg->getCallId());

      if (call_info == nullptr) {
        QCRIL_LOG_ERROR("android call-id = %d not found, rejecting modify initiate request",
                        msg->getCallId());
        rilErr = RIL_E_GENERIC_FAILURE;
        break;
      }

      call_cancel_req.call_id = call_info->qmi_call_id;
      call_cancel_req.cancel_type = VOIP_CANCEL_TYPE_CANCEL_CALL_UPGRADE_V02;

      pendingMsgStatus = getPendingMessageList().insert(msg);
      if (pendingMsgStatus.second != true) {
        QCRIL_LOG_ERROR("getPendingMessageList().insert failed!!");
        rilErr = RIL_E_GENERIC_FAILURE;
        break;
      }
      uint16_t req_id = pendingMsgStatus.first;

      rilErr = qcril_qmi_voice_process_ims_call_cancel_req(
          req_id, call_cancel_req,
          std::bind(&ImsVoiceModule::processCancelModifyCallResponse, this, std::placeholders::_1));
    } while (0);

    if (rilErr != RIL_E_SUCCESS) {
      if (pendingMsgStatus.second) {
        getPendingMessageList().erase(pendingMsgStatus.first);
      }
      std::shared_ptr<QcRilRequestMessageCallbackPayload> respPayload =
          std::make_shared<QcRilRequestMessageCallbackPayload>(rilErr, nullptr);
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

void ImsVoiceModule::handleQcRilRequestImsModifyCallConfirmMessage(
    std::shared_ptr<QcRilRequestImsModifyCallConfirmMessage> msg) {
  QCRIL_LOG_FUNC_ENTRY("msg = %s", (msg ? msg->dump().c_str() : "nullptr"));

  RIL_Errno rilErr = RIL_E_SUCCESS;
  voice_manage_ip_calls_req_msg_v02 manage_ip_calls_req = {};
  qcril_qmi_voice_voip_call_info_entry_type *call_info = nullptr;
  auto pendingMsgStatus = std::make_pair(0, false);
  boolean is_audio_attrib_valid, is_video_attrib_valid, accept;
  boolean is_emergency_ip = FALSE;

  if (msg != nullptr) {
    do {
      if (msg->hasCallId() && msg->getCallId() >= VOICE_LOWEST_CALL_ID &&
          msg->getCallId() <= VOICE_HIGHEST_CALL_ID) {
        call_info = qcril_qmi_voice_voip_find_call_info_entry_by_call_android_id(msg->getCallId());
      } else {
        QCRIL_LOG_ERROR("Invalid CallId");
        rilErr = RIL_E_GENERIC_FAILURE;
        break;
      }

      if (call_info == NULL) {
        QCRIL_LOG_DEBUG("android call-id = %d not found, rejecting modify initiate request",
                        msg->getCallId());
        rilErr = RIL_E_GENERIC_FAILURE;
        break;
      }

      if (call_info->voice_scv_info.call_type == CALL_TYPE_EMERGENCY_IP_V02 ||
          call_info->voice_scv_info.call_type == CALL_TYPE_EMERGENCY_VT_V02) {
        is_emergency_ip = TRUE;
      }
      if (!msg->hasCallType() || msg->getCallType() == qcril::interfaces::CallType::UNKNOWN) {
        QCRIL_LOG_DEBUG("Invalid CallType");
        rilErr = RIL_E_GENERIC_FAILURE;
        break;
      }
      if (!msg->hasCallDomain() || msg->getCallDomain() == qcril::interfaces::CallDomain::UNKNOWN) {
        QCRIL_LOG_DEBUG("Invalid CallDomain");
        rilErr = RIL_E_GENERIC_FAILURE;
        break;
      }
      uint8_t ret = convert_call_info_to_qmi(
          msg->getCallType(), msg->getCallDomain(), is_emergency_ip, manage_ip_calls_req.call_type,
          manage_ip_calls_req.audio_attrib_valid, manage_ip_calls_req.audio_attrib,
          manage_ip_calls_req.video_attrib_valid, manage_ip_calls_req.video_attrib);
      manage_ip_calls_req.call_type_valid = ret;

      if (!ret) {
        QCRIL_LOG_DEBUG("Failed to convert the call type details");
        rilErr = RIL_E_GENERIC_FAILURE;
        break;
      }

      if (msg->hasRttMode()) {
        manage_ip_calls_req.rtt_mode_valid =
            convert_rtt_mode_to_qmi(msg->getRttMode(), manage_ip_calls_req.rtt_mode);
      }

      is_audio_attrib_valid =
          (call_info->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_AUDIO_ATTR_VALID) ? TRUE
                                                                                        : FALSE;
      is_video_attrib_valid =
          (call_info->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_VIDEO_ATTR_VALID) ? TRUE
                                                                                        : FALSE;

      accept = !qcril_qmi_voice_match_modem_call_type(
          manage_ip_calls_req.call_type, manage_ip_calls_req.audio_attrib_valid,
          manage_ip_calls_req.audio_attrib, manage_ip_calls_req.video_attrib_valid,
          manage_ip_calls_req.video_attrib, manage_ip_calls_req.rtt_mode_valid,
          manage_ip_calls_req.rtt_mode, call_info->voice_scv_info.call_type, is_audio_attrib_valid,
          call_info->voice_audio_attrib.call_attributes, is_video_attrib_valid,
          call_info->voice_video_attrib.call_attributes, call_info->rtt_mode_valid,
          call_info->rtt_mode.rtt_mode);

      if (accept) {
        manage_ip_calls_req.sups_type = VOIP_SUPS_TYPE_MODIFY_ACCEPT_V02;
      } else {
        manage_ip_calls_req.sups_type = VOIP_SUPS_TYPE_MODIFY_REJECT_V02;
      }

      manage_ip_calls_req.call_id_valid = TRUE;
      manage_ip_calls_req.call_id = call_info->qmi_call_id;

      pendingMsgStatus = getPendingMessageList().insert(msg);
      if (pendingMsgStatus.second != true) {
        QCRIL_LOG_ERROR("getPendingMessageList().insert failed!!");
        rilErr = RIL_E_GENERIC_FAILURE;
        break;
      }
      uint16_t req_id = pendingMsgStatus.first;

      rilErr = qcril_qmi_voice_process_manage_ip_calls_req(
          req_id, manage_ip_calls_req,
          std::bind(&ImsVoiceModule::processModifyCallConfirmResponse, this,
                    std::placeholders::_1));

      if (rilErr == RIL_E_SUCCESS) {
        call_info->elaboration &= ~QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_CALL_MODIFY_CONFIRM_PENDING;
      }
    } while (0);

    if (rilErr != RIL_E_SUCCESS) {
      if (pendingMsgStatus.second) {
        getPendingMessageList().erase(pendingMsgStatus.first);
      }
      std::shared_ptr<QcRilRequestMessageCallbackPayload> respPayload =
          std::make_shared<QcRilRequestMessageCallbackPayload>(rilErr, nullptr);
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

void ImsVoiceModule::handleQcRilRequestImsExplicitCallTransferMessage(
    std::shared_ptr<QcRilRequestImsExplicitCallTransferMessage> msg) {
  QCRIL_LOG_FUNC_ENTRY("msg = %s", (msg ? msg->dump().c_str() : "nullptr"));

  RIL_Errno rilErr = RIL_E_SUCCESS;
  qcril_qmi_voice_voip_call_info_entry_type *call_info = nullptr;
  voice_manage_ip_calls_req_msg_v02 manage_ip_calls_req = {};
  auto pendingMsgStatus = std::make_pair(0, false);

  if (msg != nullptr) {
    do {
      if (!msg->hasCallId() || !msg->hasEctType()) {
        QCRIL_LOG_ERROR("Mandatory params missing");
        rilErr = RIL_E_INVALID_ARGUMENTS;
        break;
      }

      uint8_t call_id = msg->getCallId();
      QCRIL_LOG_ERROR("call id = %d", call_id);
      call_info = qcril_qmi_voice_voip_find_call_info_entry_by_call_android_id(call_id);
      if (!call_info) {
        QCRIL_LOG_ERROR("Do not have any voip active calls or invalid call id in request");
        rilErr = RIL_E_GENERIC_FAILURE;
        break;
      }

      manage_ip_calls_req.sups_type = VOIP_SUPS_TYPE_EXPLICIT_CALL_TRANSFER_V02;
      manage_ip_calls_req.call_id_valid = TRUE;
      manage_ip_calls_req.call_id = call_info->voice_scv_info.call_id;

      if (msg->hasTargetCallId()) {
        QCRIL_LOG_INFO("targetCallId: %d", msg->getTargetCallId());
        qcril_qmi_voice_voip_call_info_entry_type *target_call_info_entry =
            qcril_qmi_voice_voip_find_call_info_entry_by_call_android_id(msg->getTargetCallId());
        if (target_call_info_entry == nullptr) {
          QCRIL_LOG_ERROR("Failed to find target call entry, aborting!");
          rilErr = RIL_E_GENERIC_FAILURE;
          break;
        }
        manage_ip_calls_req.transfer_target_call_id_valid = TRUE;
        manage_ip_calls_req.transfer_target_call_id = target_call_info_entry->qmi_call_id;
      }

      if (qcril_qmi_voice_map_ril_ect_type_to_qmi_ect_type(manage_ip_calls_req.ect_type,
                                                           msg->getEctType())) {
        manage_ip_calls_req.ect_type_valid = TRUE;
      } else {
        QCRIL_LOG_ERROR("Failed to map ims ect_type %d to qmi ect_type", msg->getEctType());
        rilErr = RIL_E_INVALID_ARGUMENTS;
        break;
      }

      if (msg->hasTargetAddress() && msg->getTargetAddress().size() > 0) {
        manage_ip_calls_req.sip_uri_valid = TRUE;
        strlcpy(manage_ip_calls_req.sip_uri, msg->getTargetAddress().c_str(),
                sizeof(manage_ip_calls_req.sip_uri));
      }

      pendingMsgStatus = getPendingMessageList().insert(msg);
      if (pendingMsgStatus.second != true) {
        QCRIL_LOG_ERROR("getPendingMessageList().insert failed!!");
        rilErr = RIL_E_GENERIC_FAILURE;
        break;
      }
      uint16_t req_id = pendingMsgStatus.first;

      rilErr = qcril_qmi_voice_process_manage_ip_calls_req(
          req_id, manage_ip_calls_req,
          std::bind(&ImsVoiceModule::processExplicitCallTransferResponse, this,
                    std::placeholders::_1));
    } while (0);

    if (rilErr != RIL_E_SUCCESS) {
      if (pendingMsgStatus.second) {
        getPendingMessageList().erase(pendingMsgStatus.first);
      }
      std::shared_ptr<QcRilRequestMessageCallbackPayload> respPayload =
          std::make_shared<QcRilRequestMessageCallbackPayload>(rilErr, nullptr);
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

void ImsVoiceModule::handleQcRilRequestImsDeflectCallMessage(
    std::shared_ptr<QcRilRequestImsDeflectCallMessage> msg) {
  QCRIL_LOG_FUNC_ENTRY("msg = %s", (msg ? msg->dump().c_str() : "nullptr"));

  RIL_Errno rilErr = RIL_E_SUCCESS;
  qcril_qmi_voice_voip_call_info_entry_type *call_info = nullptr;
  voice_manage_ip_calls_req_msg_v02 manage_ip_calls_req = {};
  auto pendingMsgStatus = std::make_pair(0, false);

  if (msg != nullptr) {
    do {
      if (!msg->hasCallId() || !msg->hasNumber()) {
        QCRIL_LOG_ERROR("Mandatory params missing");
        rilErr = RIL_E_INVALID_ARGUMENTS;
        break;
      }

      uint8_t call_id = msg->getCallId();
      QCRIL_LOG_ERROR("call id = %d", call_id);
      call_info = qcril_qmi_voice_voip_find_call_info_entry_by_call_android_id(call_id);
      if (!call_info) {
        QCRIL_LOG_ERROR("Do not have any voip active calls or invalid call id in request");
        rilErr = RIL_E_GENERIC_FAILURE;
        break;
      }

      manage_ip_calls_req.sups_type = VOIP_SUPS_TYPE_CALL_DEFLECTION_V02;
      manage_ip_calls_req.call_id_valid = TRUE;
      manage_ip_calls_req.call_id = call_info->voice_scv_info.call_id;

      if (msg->hasNumber() && msg->getNumber().size() > 0) {
        manage_ip_calls_req.sip_uri_valid = TRUE;
        strlcpy(manage_ip_calls_req.sip_uri, msg->getNumber().c_str(),
                sizeof(manage_ip_calls_req.sip_uri));
      }

      pendingMsgStatus = getPendingMessageList().insert(msg);
      if (pendingMsgStatus.second != true) {
        QCRIL_LOG_ERROR("getPendingMessageList().insert failed!!");
        rilErr = RIL_E_GENERIC_FAILURE;
        break;
      }
      uint16_t req_id = pendingMsgStatus.first;

      rilErr = qcril_qmi_voice_process_manage_ip_calls_req(
          req_id, manage_ip_calls_req,
          std::bind(&ImsVoiceModule::processDeflectCallResponse, this, std::placeholders::_1));
    } while (0);

    if (rilErr != RIL_E_SUCCESS) {
      if (pendingMsgStatus.second) {
        getPendingMessageList().erase(pendingMsgStatus.first);
      }
      std::shared_ptr<QcRilRequestMessageCallbackPayload> respPayload =
          std::make_shared<QcRilRequestMessageCallbackPayload>(rilErr, nullptr);
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

void ImsVoiceModule::handleQcRilRequestImsSetUiTtyModeMessage(
    std::shared_ptr<QcRilRequestImsSetUiTtyModeMessage> msg) {
  QCRIL_LOG_FUNC_ENTRY("msg = %s", (msg ? msg->dump().c_str() : "nullptr"));

  RIL_Errno rilErr = RIL_E_SUCCESS;
  voice_set_config_req_msg_v02 set_config_req = {};
  auto pendingMsgStatus = std::make_pair(0, false);

  if (msg != nullptr) {
    do {
      bool isValid = false;
      if (msg->hasTtyMode()) {
        isValid = qcril_qmi_voice_map_ril_tty_mode_to_qmi_tty_mode(set_config_req.ui_tty_setting,
                                                                   msg->getTtyMode());
        set_config_req.ui_tty_setting_valid = isValid ? TRUE : FALSE;
      }
      if (!isValid) {
        QCRIL_LOG_ERROR("Invalid parameter: ttyMode");
        rilErr = RIL_E_INVALID_ARGUMENTS;
        break;
      }

      pendingMsgStatus = getPendingMessageList().insert(msg);
      if (pendingMsgStatus.second != true) {
        QCRIL_LOG_ERROR("getPendingMessageList().insert failed!!");
        rilErr = RIL_E_GENERIC_FAILURE;
        break;
      }
      uint16_t req_id = pendingMsgStatus.first;

      rilErr = qcril_qmi_voice_process_set_config_req_sync(
          req_id, set_config_req,
          std::bind(&ImsVoiceModule::processSetUiTtyModeResponse, this, std::placeholders::_1));
    } while (0);

    if (rilErr != RIL_E_SUCCESS) {
      if (pendingMsgStatus.second) {
        getPendingMessageList().erase(pendingMsgStatus.first);
      }
      std::shared_ptr<QcRilRequestMessageCallbackPayload> respPayload =
          std::make_shared<QcRilRequestMessageCallbackPayload>(rilErr, nullptr);
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

void ImsVoiceModule::handleQcRilRequestImsSendDtmfMessage(
    std::shared_ptr<QcRilRequestImsSendDtmfMessage> msg) {
  QCRIL_LOG_FUNC_ENTRY("msg = %s", (msg ? msg->dump().c_str() : "nullptr"));

  RIL_Errno rilErr = RIL_E_SUCCESS;
  voice_start_cont_dtmf_req_msg_v02 start_cont_dtmf_req = {};
  auto pendingMsgStatus = std::make_pair(0, false);

  if (msg != nullptr) {
    do {
      if (!msg->hasDigit()) {
        QCRIL_LOG_ERROR("Invalid parameter: digit");
        rilErr = RIL_E_INVALID_ARGUMENTS;
        break;
      }

      start_cont_dtmf_req.cont_dtmf_info.digit = msg->getDigit();
      start_cont_dtmf_req.cont_dtmf_info.call_id = QCRIL_QMI_VOICE_UNKNOWN_ACTIVE_CONN_ID;

      pendingMsgStatus = getPendingMessageList().insert(msg);
      if (pendingMsgStatus.second != true) {
        QCRIL_LOG_ERROR("getPendingMessageList().insert failed!!");
        break;
      }
      uint16_t req_id = pendingMsgStatus.first;

      rilErr = qcril_qmi_voice_process_start_cont_dtmf_req(
          req_id, start_cont_dtmf_req,
          std::bind(&ImsVoiceModule::processStartContDtmfResponse, this, std::placeholders::_1));
    } while (0);

    if (rilErr != RIL_E_SUCCESS) {
      if (pendingMsgStatus.second) {
        getPendingMessageList().erase(pendingMsgStatus.first);
      }
      std::shared_ptr<QcRilRequestMessageCallbackPayload> respPayload =
          std::make_shared<QcRilRequestMessageCallbackPayload>(rilErr, nullptr);
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

void ImsVoiceModule::handleQcRilRequestImsStartDtmfMessage(
    std::shared_ptr<QcRilRequestImsStartDtmfMessage> msg) {
  QCRIL_LOG_FUNC_ENTRY("msg = %s", (msg ? msg->dump().c_str() : "nullptr"));

  RIL_Errno rilErr = RIL_E_SUCCESS;
  voice_start_cont_dtmf_req_msg_v02 start_cont_dtmf_req = {};
  auto pendingMsgStatus = std::make_pair(0, false);

  if (msg != nullptr) {
    do {
      if (!msg->hasDigit()) {
        QCRIL_LOG_ERROR("Invalid parameter: digit");
        rilErr = RIL_E_INVALID_ARGUMENTS;
        break;
      }

      start_cont_dtmf_req.cont_dtmf_info.digit = msg->getDigit();
      start_cont_dtmf_req.cont_dtmf_info.call_id = QCRIL_QMI_VOICE_UNKNOWN_ACTIVE_CONN_ID;

      pendingMsgStatus = getPendingMessageList().insert(msg);
      if (pendingMsgStatus.second != true) {
        QCRIL_LOG_ERROR("getPendingMessageList().insert failed!!");
        break;
      }
      uint16_t req_id = pendingMsgStatus.first;

      rilErr = qcril_qmi_voice_process_start_cont_dtmf_req(
          req_id, start_cont_dtmf_req,
          std::bind(&ImsVoiceModule::processStartContDtmfResponse, this, std::placeholders::_1));
    } while (0);

    if (rilErr != RIL_E_SUCCESS) {
      if (pendingMsgStatus.second) {
        getPendingMessageList().erase(pendingMsgStatus.first);
      }
      std::shared_ptr<QcRilRequestMessageCallbackPayload> respPayload =
          std::make_shared<QcRilRequestMessageCallbackPayload>(rilErr, nullptr);
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

void ImsVoiceModule::handleQcRilRequestImsStopDtmfMessage(
    std::shared_ptr<QcRilRequestImsStopDtmfMessage> msg) {
  QCRIL_LOG_FUNC_ENTRY("msg = %s", (msg ? msg->dump().c_str() : "nullptr"));

  RIL_Errno rilErr = RIL_E_SUCCESS;
  voice_stop_cont_dtmf_req_msg_v02 stop_cont_dtmf_req = {
      .call_id = QCRIL_QMI_VOICE_UNKNOWN_ACTIVE_CONN_ID};
  auto pendingMsgStatus = std::make_pair(0, false);

  if (msg != nullptr) {
    do {
      pendingMsgStatus = getPendingMessageList().insert(msg);
      if (pendingMsgStatus.second != true) {
        QCRIL_LOG_ERROR("getPendingMessageList().insert failed!!");
        break;
      }
      uint16_t req_id = pendingMsgStatus.first;

      rilErr = qcril_qmi_voice_process_stop_cont_dtmf_req(
          req_id, stop_cont_dtmf_req,
          std::bind(&ImsVoiceModule::processStopContDtmfResponse, this, std::placeholders::_1));
    } while (0);

    if (rilErr != RIL_E_SUCCESS) {
      if (pendingMsgStatus.second) {
        getPendingMessageList().erase(pendingMsgStatus.first);
      }
      std::shared_ptr<QcRilRequestMessageCallbackPayload> respPayload =
          std::make_shared<QcRilRequestMessageCallbackPayload>(rilErr, nullptr);
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}


/*===============================================================================================
 * Callback functions
 *===============================================================================================*/

void ImsVoiceModule::processDialCallResponse(CommonVoiceResponseData *data) {
  QCRIL_LOG_FUNC_ENTRY();
  (void)data;
  // No action required
  // processDialCommandOversightCompletion will send the response
  // to telephony in all DIAL_CALL_RESP cases.
  QCRIL_LOG_FUNC_RETURN();
}

void ImsVoiceModule::processReleaseParticipantResponse(CommonVoiceResponseData *data) {
  QCRIL_LOG_FUNC_ENTRY();
  if (data) {
    auto pendingMsg = getPendingMessageList().extract(data->req_id);

    auto respPayload = std::make_shared<QcRilRequestMessageCallbackPayload>(data->errNo, nullptr);
    if (pendingMsg) {
      auto msg(std::static_pointer_cast<QcRilRequestMessage>(pendingMsg));
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

void ImsVoiceModule::processEndCallResponse(CommonVoiceResponseData *data) {
  QCRIL_LOG_FUNC_ENTRY();
  (void)data;
  // No action required
  // processHangupCommandOversightCompletion will send the response to telephony
  QCRIL_LOG_FUNC_RETURN();
}

void ImsVoiceModule::processAddParticipantResponse(CommonVoiceResponseData *data) {
  QCRIL_LOG_FUNC_ENTRY();
  if (data) {
    auto pendingMsg = getPendingMessageList().extract(data->req_id);

    auto respPayload = std::make_shared<QcRilRequestMessageCallbackPayload>(data->errNo, nullptr);
    if (pendingMsg) {
      auto msg(std::static_pointer_cast<QcRilRequestMessage>(pendingMsg));
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

void ImsVoiceModule::processConferenceResponse(CommonVoiceResponseData *data) {
  QCRIL_LOG_FUNC_ENTRY();
  std::shared_ptr<qcril::interfaces::SipErrorInfo> errorDetails = nullptr;
  voice_manage_ip_calls_resp_msg_v02 *qmiResp = nullptr;

  if (data) {
    // Notify call state changed indication
    if (data->errNo == RIL_E_SUCCESS) {
#ifndef QMI_RIL_UTF
      qcril_qmi_voice_send_ims_unsol_call_state_changed();
#endif
    } else {
      uint32_t failureCauseDescriptionLen = 0;
      uint16_t *failureCauseDescription = nullptr;
      qmiResp = static_cast<voice_manage_ip_calls_resp_msg_v02 *>(data->respData);
      if (qmiResp) {
        if (qmiResp->end_reason_text_valid && qmiResp->end_reason_text_len > 0) {
          for (uint32_t i = 0; i < qmiResp->end_reason_text_len; i++) {
            if (qmiResp->end_reason_text[i].end_reason_text_len > 0) {
              failureCauseDescriptionLen = qmiResp->end_reason_text[i].end_reason_text_len;
              failureCauseDescription = qmiResp->end_reason_text[i].end_reason_text;
            }
          }
        }
        errorDetails = buildSipErrorInfo(qmiResp->sip_error_code_valid, qmiResp->sip_error_code,
                                         failureCauseDescriptionLen, failureCauseDescription);
      }
    }

    auto pendingMsg = getPendingMessageList().extract(data->req_id);

    auto respPayload =
        std::make_shared<QcRilRequestMessageCallbackPayload>(data->errNo, errorDetails);
    if (pendingMsg) {
      auto msg(std::static_pointer_cast<QcRilRequestMessage>(pendingMsg));
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

void ImsVoiceModule::processAnswerResponse(CommonVoiceResponseData *data) {
  QCRIL_LOG_FUNC_ENTRY();
  if (data) {
    auto pendingMsg = getPendingMessageList().find(data->req_id);
    if (pendingMsg &&
        pendingMsg->get_message_id() == QcRilRequestImsAnswerMessage::get_class_message_id()) {
      if (data->errNo != RIL_E_SUCCESS) {
        qcril_am_handle_event(QCRIL_AM_EVENT_IMS_ANSWER_FAIL, NULL);
      }
    }
    // No action required
    // processAnswerCommandOversightCompletion will send the response to telephony
  }
  QCRIL_LOG_FUNC_RETURN();
}

void ImsVoiceModule::processHoldResponse(CommonVoiceResponseData *data) {
  QCRIL_LOG_FUNC_ENTRY();
  std::shared_ptr<qcril::interfaces::SipErrorInfo> errorDetails = nullptr;
  voice_manage_ip_calls_resp_msg_v02 *qmiResp = nullptr;

  if (data) {
    auto pendingMsg = getPendingMessageList().extract(data->req_id);

    if (data->errNo != RIL_E_SUCCESS) {
      uint32_t failureCauseDescriptionLen = 0;
      uint16_t *failureCauseDescription = nullptr;
      qmiResp = static_cast<voice_manage_ip_calls_resp_msg_v02 *>(data->respData);
      if (qmiResp) {
        if (qmiResp->end_reason_text_valid && qmiResp->end_reason_text_len > 0) {
          for (uint32_t i = 0; i < qmiResp->end_reason_text_len; i++) {
            if (qmiResp->end_reason_text[i].end_reason_text_len > 0) {
              failureCauseDescriptionLen = qmiResp->end_reason_text[i].end_reason_text_len;
              failureCauseDescription = qmiResp->end_reason_text[i].end_reason_text;
            }
          }
        }
        errorDetails = buildSipErrorInfo(qmiResp->sip_error_code_valid, qmiResp->sip_error_code,
                                         failureCauseDescriptionLen, failureCauseDescription);
      }
    }

    auto respPayload =
        std::make_shared<QcRilRequestMessageCallbackPayload>(data->errNo, errorDetails);
    if (pendingMsg) {
      auto msg(std::static_pointer_cast<QcRilRequestMessage>(pendingMsg));
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

void ImsVoiceModule::processExplicitCallTransferResponse(CommonVoiceResponseData *data) {
  QCRIL_LOG_FUNC_ENTRY();
  if (data) {
    auto pendingMsg = getPendingMessageList().extract(data->req_id);
    if (pendingMsg) {
      auto respPayload = std::make_shared<QcRilRequestMessageCallbackPayload>(data->errNo, nullptr);
      auto msg(std::static_pointer_cast<QcRilRequestMessage>(pendingMsg));
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

void ImsVoiceModule::processDeflectCallResponse(CommonVoiceResponseData *data) {
  QCRIL_LOG_FUNC_ENTRY();
  if (data) {
    auto pendingMsg = getPendingMessageList().extract(data->req_id);
    if (pendingMsg) {
      auto respPayload = std::make_shared<QcRilRequestMessageCallbackPayload>(data->errNo, nullptr);
      auto msg(std::static_pointer_cast<QcRilRequestMessage>(pendingMsg));
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

void ImsVoiceModule::processResumeResponse(CommonVoiceResponseData *data) {
  QCRIL_LOG_FUNC_ENTRY();
  std::shared_ptr<qcril::interfaces::SipErrorInfo> errorDetails = nullptr;
  voice_manage_ip_calls_resp_msg_v02 *qmiResp = nullptr;

  if (data) {
    auto pendingMsg = getPendingMessageList().extract(data->req_id);

    if (data->errNo != RIL_E_SUCCESS) {
      uint32_t failureCauseDescriptionLen = 0;
      uint16_t *failureCauseDescription = nullptr;
      qmiResp = static_cast<voice_manage_ip_calls_resp_msg_v02 *>(data->respData);
      if (qmiResp) {
        if (qmiResp->end_reason_text_valid && qmiResp->end_reason_text_len > 0) {
          for (uint32_t i = 0; i < qmiResp->end_reason_text_len; i++) {
            if (qmiResp->end_reason_text[i].end_reason_text_len > 0) {
              failureCauseDescriptionLen = qmiResp->end_reason_text[i].end_reason_text_len;
              failureCauseDescription = qmiResp->end_reason_text[i].end_reason_text;
            }
          }
        }
        errorDetails = buildSipErrorInfo(qmiResp->sip_error_code_valid, qmiResp->sip_error_code,
                                         failureCauseDescriptionLen, failureCauseDescription);
      }
    }

    if (pendingMsg) {
      auto respPayload =
          std::make_shared<QcRilRequestMessageCallbackPayload>(data->errNo, errorDetails);
      auto msg(std::static_pointer_cast<QcRilRequestMessage>(pendingMsg));
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

void ImsVoiceModule::processModifyCallInitiateResponse(CommonVoiceResponseData *data) {
  QCRIL_LOG_FUNC_ENTRY();
  voice_manage_ip_calls_resp_msg_v02 *qmiResp = nullptr;

  if (data) {
    auto pendingMsg = getPendingMessageList().find(data->req_id);

    qmiResp = static_cast<voice_manage_ip_calls_resp_msg_v02 *>(data->respData);
    if (data->errNo == RIL_E_SUCCESS) {
      /* wait for modify indication, update state to wait for more events */
      if (pendingMsg) {
        auto msg(std::static_pointer_cast<QcRilRequestMessage>(pendingMsg));
        msg->setPendingMessageState(PendingMessageState::AWAITING_INDICATION);
      }
    } else {
      getPendingMessageList().erase(data->req_id);

      if (qmiResp && (qmiResp->failure_cause_valid == TRUE) &&
          (qmiResp->failure_cause == QMI_FAILURE_CAUSE_UPGRADE_DOWNGRADE_REJ_V02)) {
        data->errNo = RIL_E_REJECTED_BY_REMOTE;
      }
      if (pendingMsg) {
        auto respPayload =
            std::make_shared<QcRilRequestMessageCallbackPayload>(data->errNo, nullptr);
        auto msg(std::static_pointer_cast<QcRilRequestMessage>(pendingMsg));
        msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
      }
    }
    if (qmiResp && qmiResp->call_modified_cause_valid && qmiResp->call_id_valid) {
      qcril_qmi_voice_call_info_entry_set_call_modified_cause(
          qcril_qmi_voice_voip_find_call_info_entry_by_call_qmi_id(qmiResp->call_id),
          qmiResp->call_modified_cause);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

void ImsVoiceModule::processModifyCallConfirmResponse(CommonVoiceResponseData *data) {
  QCRIL_LOG_FUNC_ENTRY();
  voice_manage_ip_calls_resp_msg_v02 *qmiResp = nullptr;

  if (data) {
    auto pendingMsg = getPendingMessageList().find(data->req_id);
    qmiResp = static_cast<voice_manage_ip_calls_resp_msg_v02 *>(data->respData);
    if (data->errNo == RIL_E_SUCCESS) {
      /* wait for modify indication, update state to wait for more events */
      if (pendingMsg) {
        auto msg(std::static_pointer_cast<QcRilRequestMessage>(pendingMsg));
        msg->setPendingMessageState(PendingMessageState::AWAITING_INDICATION);
      }
    } else {
      getPendingMessageList().erase(data->req_id);

      if (pendingMsg) {
        auto respPayload =
            std::make_shared<QcRilRequestMessageCallbackPayload>(data->errNo, nullptr);
        auto msg(std::static_pointer_cast<QcRilRequestImsModifyCallConfirmMessage>(pendingMsg));
        msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
      }
    }
    if (qmiResp && qmiResp->call_modified_cause_valid && qmiResp->call_id_valid) {
      qcril_qmi_voice_call_info_entry_set_call_modified_cause(
          qcril_qmi_voice_voip_find_call_info_entry_by_call_qmi_id(qmiResp->call_id),
          qmiResp->call_modified_cause);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

void ImsVoiceModule::processCancelModifyCallResponse(CommonVoiceResponseData *data) {
  QCRIL_LOG_FUNC_ENTRY();
  if (data) {
    auto pendingMsg = getPendingMessageList().extract(data->req_id);

    if (pendingMsg) {
      auto respPayload = std::make_shared<QcRilRequestMessageCallbackPayload>(data->errNo, nullptr);
      auto msg(std::static_pointer_cast<QcRilRequestMessage>(pendingMsg));
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

void ImsVoiceModule::processSetUiTtyModeResponse(CommonVoiceResponseData *data) {
  QCRIL_LOG_FUNC_ENTRY();
  voice_set_config_resp_msg_v02 *qmiResp = nullptr;
  if (data) {
    auto pendingMsg = getPendingMessageList().extract(data->req_id);
    qmiResp = static_cast<voice_set_config_resp_msg_v02 *>(data->respData);
    if (qmiResp && (data->errNo == RIL_E_SUCCESS)) {
      QCRIL_LOG_INFO(".. outcome  %d, %d", qmiResp->ui_tty_setting_outcome_valid,
                     qmiResp->ui_tty_setting_outcome);
      if (qmiResp->ui_tty_setting_outcome_valid && qmiResp->ui_tty_setting_outcome != 0x00) {
        data->errNo = RIL_E_MODEM_ERR;
      }
    }
    if (pendingMsg) {
      auto respPayload = std::make_shared<QcRilRequestMessageCallbackPayload>(data->errNo, nullptr);
      auto msg(std::static_pointer_cast<QcRilRequestMessage>(pendingMsg));
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

void ImsVoiceModule::processStartContDtmfResponse(CommonVoiceResponseData *data) {
  QCRIL_LOG_FUNC_ENTRY();
  if (data) {
    bool sendResp = true;
    auto pendingMsg = getPendingMessageList().find(data->req_id);
    if (pendingMsg &&
        (pendingMsg->get_message_id() == QcRilRequestImsSendDtmfMessage::get_class_message_id())) {
      sendResp = false;
      TimeKeeper::millisec timer = qcril_qmi_voice_dtmf_rtp_event_interval();
      uint16_t req_id = data->req_id;
      RIL_Errno errNo = data->errNo;
      TimeKeeper::getInstance().set_timer(
          [this, req_id, errNo](void *user_data) {
            QCRIL_NOTUSED(user_data);
            voice_stop_cont_dtmf_req_msg_v02 stop_cont_dtmf_req = {
                .call_id = QCRIL_QMI_VOICE_UNKNOWN_ACTIVE_CONN_ID};
            RIL_Errno rilErr = qcril_qmi_voice_process_stop_cont_dtmf_req(
                req_id, stop_cont_dtmf_req,
                std::bind(&ImsVoiceModule::processStopContDtmfResponse, this,
                          std::placeholders::_1));
            if (rilErr != RIL_E_SUCCESS) {
              auto pendingMsg = getPendingMessageList().extract(req_id);
              if (pendingMsg) {
                auto respPayload =
                    std::make_shared<QcRilRequestMessageCallbackPayload>(errNo, nullptr);
                auto msg(std::static_pointer_cast<QcRilRequestMessage>(pendingMsg));
                msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
              }
            }
          },
          nullptr, timer);
    }
    if (sendResp) {
      auto pendingMsg = getPendingMessageList().extract(data->req_id);
      if (pendingMsg) {
        auto respPayload =
            std::make_shared<QcRilRequestMessageCallbackPayload>(data->errNo, nullptr);
        auto msg(std::static_pointer_cast<QcRilRequestMessage>(pendingMsg));
        msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
      }
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

void ImsVoiceModule::processStopContDtmfResponse(CommonVoiceResponseData *data) {
  QCRIL_LOG_FUNC_ENTRY();
  if (data) {
    auto pendingMsg = getPendingMessageList().extract(data->req_id);
    if (pendingMsg) {
      auto respPayload = std::make_shared<QcRilRequestMessageCallbackPayload>(data->errNo, nullptr);
      auto msg(std::static_pointer_cast<QcRilRequestMessage>(pendingMsg));
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

void ImsVoiceModule::processDialCommandOversightCompletion(CommonVoiceResponseData *data) {
  int idx = 0;
  qcril_qmi_voice_voip_call_info_entry_type *call_entry = nullptr;
  qmi_ril_voice_ims_command_exec_oversight_link_type *link = nullptr;
  qmi_ril_voice_ims_command_exec_oversight_type *command_oversight = nullptr;

  QCRIL_LOG_FUNC_ENTRY();

  if (data) {
    auto pendingMsg = getPendingMessageList().extract(data->req_id);

    // Send response
    if (pendingMsg) {
      auto respPayload = std::make_shared<QcRilRequestMessageCallbackPayload>(data->errNo, nullptr);
      auto msg(std::static_pointer_cast<QcRilRequestMessage>(pendingMsg));
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
    }

    // Free the call info entry if required.
    command_oversight =
        static_cast<qmi_ril_voice_ims_command_exec_oversight_type *>(data->respData);

    QCRIL_LOG_DEBUG("command_oversight = %p, ril_req_res = %d", command_oversight, data->errNo);

    if (command_oversight &&
        (data->errNo != RIL_E_SUCCESS && data->errNo != RIL_E_DIAL_MODIFIED_TO_DIAL)) {
      link = command_oversight->impacted;
      // Delete the call_entry if DIAL is returned with error due to timeout
      for (idx = 0; idx < command_oversight->nof_impacted; idx++) {
        if (QMI_RIL_VOICE_IMS_EXEC_OVERSIGHT_LINKAGE_ELABORATION_PATTERN == link->linkage_type) {
          call_entry = qcril_qmi_voice_voip_find_call_info_entry_by_elaboration(
              link->linkage.elaboration_pattern, TRUE);
          if (call_entry) {
            if ((link->exec_state ==
                 QMI_RIL_VOICE_IMS_EXEC_INTERMED_STATE_TIMEOUT_READY_FOR_COMPLETION) ||
                (link->exec_state ==
                 QMI_RIL_VOICE_IMS_EXEC_INTERMED_STATE_ABANDONED_READY_FOR_COMPLETION)) {
              call_entry->voice_scv_info.call_state = CALL_STATE_END_V02;
              break;
            }
            call_entry = nullptr;
          }
        }
        link++;
      }
      if (call_entry) {
        qcril_qmi_voice_send_ims_unsol_call_state_changed();
      }
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

void ImsVoiceModule::processAnswerCommandOversightCompletion(CommonVoiceResponseData *data) {
  QCRIL_LOG_FUNC_ENTRY();
  if (data) {
    auto pendingMsg = getPendingMessageList().extract(data->req_id);
    if (pendingMsg) {
      auto respPayload = std::make_shared<QcRilRequestMessageCallbackPayload>(data->errNo, nullptr);
      auto msg(std::static_pointer_cast<QcRilRequestMessage>(pendingMsg));
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

void ImsVoiceModule::processHangupCommandOversightCompletion(CommonVoiceResponseData *data) {
  QCRIL_LOG_FUNC_ENTRY();
  if (data) {
    auto pendingMsg = getPendingMessageList().extract(data->req_id);

    if (pendingMsg) {
      auto respPayload = std::make_shared<QcRilRequestMessageCallbackPayload>(data->errNo, nullptr);
      auto msg(std::static_pointer_cast<QcRilRequestMessage>(pendingMsg));
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}
