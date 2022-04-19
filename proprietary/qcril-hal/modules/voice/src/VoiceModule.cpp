/******************************************************************************
#  Copyright (c) 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include <unordered_map>

#include "framework/Dispatcher.h"
#include "framework/Looper.h"
#include "framework/ModuleLooper.h"
#include "framework/PolicyManager.h"
#include "framework/SingleDispatchRestriction.h"
#include "framework/SingleDispatchAndFamilyPairRestriction.h"
#include "framework/Log.h"
#include <framework/ThreadPoolManager.h>

#include "modules/qmi/ModemEndPointFactory.h"
#include "modules/qmi/QmiSetupRequestCallback.h"
#include "modules/voice/RilRequestDialSyncMessage.h"
#include "modules/qmi/QmiIndMessage.h"
#include "modules/qmi/EndpointStatusIndMessage.h"
#include "modules/qmi/VoiceModemEndPoint.h"

#include "modules/voice/VoiceModule.h"
#include "modules/android/ril_message_factory.h"
#include "modules/voice/qcril_qmi_voice.h"
#include "qcril_am.h"
#include "modules/nas/qcril_qmi_nas.h"
#include "qcril_cm_ss.h"

#ifdef USING_NAS_PUBLIC_API
// NAS interface
#include "modules/nas/NasModuleInterface.h"
#include "modules/nas/NasModuleGetSyncInterfaceMessage.h"
#include "modules/voice/NasApi.h"

// NAS indication
#include "modules/nas/VoiceRegistrationStatusChangeInd.h"
#include "modules/nas/NetworkServiceRteChangeInd.h"
#endif

#undef  TAG
#define TAG "VoiceModule"

VoiceModule::VoiceModule(string name) : AddPendingMessageList(name) {
  mName = name;
  mLooper = nullptr;
  mInitialized = false;
  ModemEndPointFactory<VoiceModemEndPoint>::getInstance().buildEndPoint();

  /* _1 is the place holder for argument to handler method
     TODO: wrap function handler into a macro to make it more pleasing
  */
  using std::placeholders::_1;
  mMessageHandler = {
      // Qcril init complete message
      HANDLER(QcrilInitMessage, VoiceModule::handleQcrilInit),
      HANDLER(QcrilImsClientConnected, VoiceModule::handleQcrilImsClientConnected),
      // RIL request with translators

      HANDLER(RilRequestDialSyncMessage, VoiceModule::handleDialSyncMessage),

      HANDLER(QcRilRequestCsDialMessage, VoiceModule::handleQcRilRequestCsDialMessage),
      HANDLER(QcRilRequestCsAnswerMessage, VoiceModule::handleQcRilRequestCsAnswerMessage),
      HANDLER(QcRilRequestGetCurrentCallsMessage, VoiceModule::handleQcRilRequestGetCurrentCallsMessage),
      HANDLER(QcRilRequestLastCallFailCauseMessage,
              VoiceModule::handleQcRilRequestLastCallFailCauseMessage),
      HANDLER(QcRilRequestCsHangupMessage, VoiceModule::handleQcRilRequestCsHangupMessage),
      HANDLER(QcRilRequestCdmaFlashMessage, VoiceModule::handleQcRilRequestCdmaFlashMessage),
      HANDLER(QcRilRequestGetPreferredVoicePrivacyMessage, VoiceModule::handleQcRilRequestGetPreferredVoicePrivacyMessage),
      HANDLER(QcRilRequestSetPreferredVoicePrivacyMessage, VoiceModule::handleQcRilRequestSetPreferredVoicePrivacyMessage),
      HANDLER(QcRilRequestSendDtmfMessage, VoiceModule::handleQcRilRequestSendDtmfMessage),
      HANDLER(QcRilRequestStartDtmfMessage, VoiceModule::handleQcRilRequestStartDtmfMessage),
      HANDLER(QcRilRequestStopDtmfMessage, VoiceModule::handleQcRilRequestStopDtmfMessage),
      HANDLER(QcRilRequestSendUssdMessage, VoiceModule::handleQcRilRequestSendUssdMessage),
      HANDLER(QcRilRequestCancelUssdMessage, VoiceModule::handleQcRilRequestCancelUssdMessage),
      HANDLER(QcRilRequestSetSuppSvcNotificationMessage,
              VoiceModule::handleQcRilRequestSetSuppSvcNotificationMessage),
      HANDLER(QcRilRequestGetClipMessage, VoiceModule::handleQcRilRequestGetClipMessage),
      HANDLER(QcRilRequestGetClirMessage, VoiceModule::handleQcRilRequestGetClirMessage),
      HANDLER(QcRilRequestSetClirMessage, VoiceModule::handleQcRilRequestSetClirMessage),
      HANDLER(QcRilRequestGetColrMessage, VoiceModule::handleQcRilRequestGetColrMessage),
      HANDLER(QcRilRequestSetColrMessage, VoiceModule::handleQcRilRequestSetColrMessage),
      HANDLER(QcRilRequestQueryCallForwardMessage,
              VoiceModule::handleQcRilRequestQueryCallForwardMessage),
      HANDLER(QcRilRequestSetCallForwardMessage,
              VoiceModule::handleQcRilRequestSetCallForwardMessage),
      HANDLER(QcRilRequestGetCallWaitingMessage,
              VoiceModule::handleQcRilRequestGetCallWaitingMessage),
      HANDLER(QcRilRequestSetCallWaitingMessage,
              VoiceModule::handleQcRilRequestSetCallWaitingMessage),
      HANDLER(QcRilRequestSetSupsServiceMessage,
              VoiceModule::handleQcRilRequestSetSupsServiceMessage),
      HANDLER(QcRilRequestQueryColpMessage, VoiceModule::handleQcRilRequestQueryColpMessage),
      HANDLER(QcRilRequestGetCallBarringMessage,
              VoiceModule::handleQcRilRequestGetCallBarringMessage),
      HANDLER(QcRilRequestSetCallBarringPasswordMessage,
              VoiceModule::handleQcRilRequestSetCallBarringPasswordMessage),
      HANDLER(QcRilRequestConferenceMessage, VoiceModule::handleQcRilRequestConferenceMessage),
      HANDLER(QcRilRequestHangupForegroundResumeBackgroundMessage,
              VoiceModule::handleQcRilRequestHangupForegroundResumeBackgroundMessage),
      HANDLER(QcRilRequestHangupWaitingOrBackgroundMessage,
              VoiceModule::handleQcRilRequestHangupWaitingOrBackgroundMessage),
      HANDLER(QcRilRequestSwitchWaitingOrHoldingAndActiveMessage,
              VoiceModule::handleQcRilRequestSwitchWaitingOrHoldingAndActiveMessage),
      HANDLER(QcRilRequestUdubMessage, VoiceModule::handleQcRilRequestUdubMessage),
      HANDLER(QcRilRequestSeparateConnectionMessage,
              VoiceModule::handleQcRilRequestSeparateConnectionMessage),
      HANDLER(QcRilRequestExplicitCallTransferMessage,
              VoiceModule::handleQcRilRequestExplicitCallTransferMessage),
      HANDLER(QcRilRequestSetTtyModeMessage,
              VoiceModule::handleQcRilRequestSetTtyModeMessage),
      HANDLER(QcRilRequestGetTtyModeMessage,
              VoiceModule::handleQcRilRequestGetTtyModeMessage),
      HANDLER(QcRilRequestCdmaBurstDtmfMessage,
              VoiceModule::handleQcRilRequestCdmaBurstDtmfMessage),
      HANDLER(QcRilRequestRejectIncomingCallMessage,
              VoiceModule::handleQcRilRequestRejectIncomingCallMessage),
      HANDLER(QcRilRequestGetCurrentSetupCallsMessage,
              VoiceModule::handleQcRilRequestGetCurrentSetupCallsMessage),
      HANDLER(QcRilRequestSetupAnswerMessage,
              VoiceModule::handleQcRilRequestSetupAnswerMessage),
      HANDLER(QcRilRequestSetLocalCallHoldMessage,
              VoiceModule::handleQcRilRequestSetLocalCallHoldMessage),
      HANDLER(QcRilRequestSetMuteMessage, VoiceModule::handleQcRilRequestSetMuteMessage),
      HANDLER(QcRilRequestGetMuteMessage, VoiceModule::handleQcRilRequestGetMuteMessage),

      // public APIs
      HANDLER(VoiceLimitIndMessage, VoiceModule::handleLimitedIndMessage),
      HANDLER(VoiceGetAnswerCallModeMessage, VoiceModule::handleGetAnswerCallModeMessage),
      HANDLER(VoiceCheckSpecificCallMessage, VoiceModule::handleCheckSpecificCallMessage),
      HANDLER(VoiceImsConnectedMessage, VoiceModule::handleImsConnectedMessage),
      HANDLER(VoiceRteProcessPendingCallMessage, VoiceModule::handleRteProcessPendingCall),
      HANDLER(VoiceCheckCallPresentMessage, VoiceModule::handleCheckCallPresentMessage),
      HANDLER(VoiceRequestPendingDialMessage, VoiceModule::handleVoiceRequestPendingDialMessage),

#ifdef USING_NAS_PUBLIC_API
      // NAS indication
      {VoiceRegistrationStatusChangeInd::get_class_message_id(),
       std::bind(&VoiceModule::HandleVoiceRegStatusIndMessage, this, _1)},
      {NetworkServiceRteChangeInd::get_class_message_id(),
       std::bind(&VoiceModule::HandleServiceRteChangeIndMessage, this, _1)},
#endif
      // QMI indication
      {REG_MSG("VOICE_QMI_IND"), std::bind(&VoiceModule::handleVoiceQmiIndMessage, this, _1)},
      // QMI Async response
      HANDLER(QmiAsyncResponseMessage, VoiceModule::handleQmiAsyncRespMessage),
      // End Point Status Indication
      {REG_MSG("VOICE_ENDPOINT_STATUS_IND"),
       std::bind(&VoiceModule::handleVoiceEndpointStatusIndMessage, this, _1)},
  };
}

VoiceModule::VoiceModule() : VoiceModule("VoiceModule") {
}

VoiceModule::~VoiceModule() {
}

void VoiceModule::init() {
  /* Call base init before doing any other stuff.*/
  Module::init();

  // move voice pre init here
  qcril_qmi_voice_pre_init();

  // flow control: single
  QCRIL_ANDROID_SET_MESSAGE_RESTRICTION(
      QcRilRequestSetSuppSvcNotificationMessage::MESSAGE_NAME,
      std::make_shared<SingleDispatchRestriction>());
  // flow control: single
  QCRIL_ANDROID_SET_MESSAGE_RESTRICTION(
      QcRilRequestGetCurrentCallsMessage::MESSAGE_NAME,
      std::make_shared<SingleDispatchRestriction>());
  // flow control: single
  QCRIL_ANDROID_SET_MESSAGE_RESTRICTION(
      QcRilRequestLastCallFailCauseMessage::MESSAGE_NAME,
      std::make_shared<SingleDispatchRestriction>());
  // flow control: single
  QCRIL_ANDROID_SET_MESSAGE_RESTRICTION(
      QcRilRequestCdmaFlashMessage::MESSAGE_NAME,
      std::make_shared<SingleDispatchRestriction>());
  // flow control: single - dedicated thread
  QCRIL_ANDROID_SET_MESSAGE_RESTRICTION(
      QcRilRequestSetTtyModeMessage::MESSAGE_NAME,
      std::make_shared<SingleDispatchRestriction>());
  // flow control: multiple - dedicated thread
  QCRIL_ANDROID_SET_MESSAGE_RESTRICTION(
      QcRilRequestGetTtyModeMessage::MESSAGE_NAME,
      std::make_shared<SingleDispatchRestriction>());
  // flow control: single
  QCRIL_ANDROID_SET_MESSAGE_RESTRICTION(
      QcRilRequestSetClirMessage::MESSAGE_NAME,
      std::make_shared<SingleDispatchRestriction>());
  // flow control: exempt
  // QcRilRequestSendUssdMessage
  // flow control: single
  QCRIL_ANDROID_SET_MESSAGE_RESTRICTION(
      QcRilRequestCancelUssdMessage::MESSAGE_NAME,
      std::make_shared<SingleDispatchRestriction>());
  // flow control: single
  QCRIL_ANDROID_SET_MESSAGE_RESTRICTION(
      QcRilRequestGetPreferredVoicePrivacyMessage::MESSAGE_NAME,
      std::make_shared<SingleDispatchRestriction>());
  // flow control: multiple - dedicated thread
  QCRIL_ANDROID_SET_MESSAGE_RESTRICTION(
      QcRilRequestSetPreferredVoicePrivacyMessage::MESSAGE_NAME,
      std::make_shared<SingleDispatchRestriction>());

  QCRIL_ANDROID_SET_MESSAGE_RESTRICTION(
      QcRilRequestCsDialMessage::MESSAGE_NAME,
      std::make_shared<SingleDispatchRestriction>());

  // ring1: DTMF related. Set DTMF_START and DTMF_STOP as family pair
  std::vector<std::string> ring1 {
        QcRilRequestSendDtmfMessage::MESSAGE_NAME,
        QcRilRequestStartDtmfMessage::MESSAGE_NAME,
        QcRilRequestStopDtmfMessage::MESSAGE_NAME,
        QcRilRequestCdmaBurstDtmfMessage::MESSAGE_NAME
  };
  auto rest = std::make_shared<SingleDispatchAndFamilyPairRestriction>();
  if (rest) {
    rest->setMessagePair(QcRilRequestStartDtmfMessage::MESSAGE_NAME,
        QcRilRequestStopDtmfMessage::MESSAGE_NAME);
  }
  PolicyManager::getInstance().setMessageRestriction(ring1, rest);

  // ring2: CALL related
  std::vector<std::string> ring2 {
        QcRilRequestCsHangupMessage::MESSAGE_NAME,
        QcRilRequestHangupWaitingOrBackgroundMessage::MESSAGE_NAME,
        QcRilRequestHangupForegroundResumeBackgroundMessage::MESSAGE_NAME,
        QcRilRequestSwitchWaitingOrHoldingAndActiveMessage::MESSAGE_NAME,
        QcRilRequestConferenceMessage::MESSAGE_NAME,
        QcRilRequestExplicitCallTransferMessage::MESSAGE_NAME,
        QcRilRequestSeparateConnectionMessage::MESSAGE_NAME,
        QcRilRequestUdubMessage::MESSAGE_NAME,
        QcRilRequestCsAnswerMessage::MESSAGE_NAME
  };
  PolicyManager::getInstance().setMessageRestriction(ring2,
        std::make_shared<SingleDispatchRestriction>());

  // ring3: Supplementary Service related
  std::vector<std::string> ring3 {
        QcRilRequestQueryCallForwardMessage::MESSAGE_NAME,
        QcRilRequestSetCallForwardMessage::MESSAGE_NAME,
        QcRilRequestSetCallBarringPasswordMessage::MESSAGE_NAME,
        QcRilRequestGetCallWaitingMessage::MESSAGE_NAME,
        QcRilRequestSetCallWaitingMessage::MESSAGE_NAME,
        QcRilRequestGetClipMessage::MESSAGE_NAME,
        QcRilRequestGetClirMessage::MESSAGE_NAME
  };
  PolicyManager::getInstance().setMessageRestriction(ring3,
        std::make_shared<SingleDispatchRestriction>());

  QCRIL_ANDROID_SET_MESSAGE_RESTRICTION(
      QcRilRequestRejectIncomingCallMessage::MESSAGE_NAME,
      std::make_shared<SingleDispatchRestriction>());

  QCRIL_ANDROID_SET_MESSAGE_RESTRICTION(
      QcRilRequestSetupAnswerMessage::MESSAGE_NAME,
      std::make_shared<SingleDispatchRestriction>());

  QCRIL_ANDROID_SET_MESSAGE_RESTRICTION(
      QcRilRequestSetLocalCallHoldMessage::MESSAGE_NAME,
      std::make_shared<SingleDispatchRestriction>());
}

void VoiceModule::handleQcrilInit(std::shared_ptr<QcrilInitMessage> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());

  /* Init QMI services.*/
  QmiSetupRequestCallback qmiVoiceSetupCallback("VoiceModule-Token");
  auto voiceEndPoint = ModemEndPointFactory<VoiceModemEndPoint>::getInstance().buildEndPoint();
  voiceEndPoint->requestSetup("Client-Voice-Serv-Token", &qmiVoiceSetupCallback);
}

void VoiceModule::handleVoiceEndpointStatusIndMessage(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  std::shared_ptr<EndpointStatusIndMessage> shared_msg(
    std::static_pointer_cast<EndpointStatusIndMessage>(msg));

  if (shared_msg->getState() == ModemEndPoint::State::OPERATIONAL) {
     if (!mInitialized) {
       mInitialized = true;
       qcril_qmi_voice_init();
       mSetAllCallForwardSupported = qcril_qmi_voice_is_set_all_call_forward_supported();
     }
  } else {
    // state is not operational
    mInitialized = false;
    qcril_qmi_voice_cleanup();
    clearPendingMessageList();
    // Reset QCRIL AM states
    qcril_am_state_reset();
  }
}

void VoiceModule::handleVoiceQmiIndMessage(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  std::shared_ptr<QmiIndMessage> shared_indMsg(
        std::static_pointer_cast<QmiIndMessage>(msg));

  QmiIndMsgDataStruct *indData = shared_indMsg->getData();
  if (indData != nullptr) {
    qcril_qmi_voice_unsol_ind_cb_helper(indData->msgId, indData->indData,
            indData->indSize);
  } else {
    Log::getInstance().d("Unexpected, null data from message");
  }
}

void VoiceModule::handleQmiAsyncRespMessage(std::shared_ptr<QmiAsyncResponseMessage> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  QmiAsyncRespData *asyncResp = msg->getData();
  if (asyncResp != nullptr && asyncResp->cb != nullptr) {
    asyncResp->cb(asyncResp->msgId, asyncResp->respData, asyncResp->respDataSize,
        asyncResp->cbData, asyncResp->traspErr);
  } else {
    Log::getInstance().d("Unexpected, null data from message");
  }
}

void VoiceModule::handleQcrilImsClientConnected(std::shared_ptr<QcrilImsClientConnected> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  qcril_qmi_voice_ims_client_connected();
}

// only for gtest
void VoiceModule::handleDialSyncMessage(std::shared_ptr<RilRequestDialSyncMessage> shared_dialMsg) {
  Log::getInstance().d("[VoiceModule]: Handling msg = " + shared_dialMsg->dump());
  std::shared_ptr<string> shared_pResponse = std::make_shared<string>("Thank you for dialing ");
  shared_dialMsg->sendResponse(
      shared_dialMsg, Message::Callback::Status::SUCCESS, shared_pResponse);
}

void VoiceModule::processDialCommandOversightCompletion(CommonVoiceResponseData *data) {
  int idx = 0;
  qcril_qmi_voice_voip_call_info_entry_type *call_entry = nullptr;
  qmi_ril_voice_ims_command_exec_oversight_link_type *link = nullptr;
  qmi_ril_voice_ims_command_exec_oversight_type *command_oversight = nullptr;

  QCRIL_LOG_FUNC_ENTRY();

  if (data) {
    auto pendingMsg = getPendingMessageList().extract(data->req_id);
    if (pendingMsg) {
      auto msg(std::static_pointer_cast<QcRilRequestMessage>(pendingMsg));
      auto respPayload = std::make_shared<QcRilRequestMessageCallbackPayload>(data->errNo, nullptr);
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
        qcril_qmi_voice_voip_destroy_call_info_entry(call_entry);
      }
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}


void VoiceModule::processHangupCommandOversightCompletion(CommonVoiceResponseData *data) {
  QCRIL_LOG_FUNC_ENTRY();
  if (data) {
    auto pendingMsg = getPendingMessageList().extract(data->req_id);
    if (pendingMsg) {
      auto msg(std::static_pointer_cast<QcRilRequestMessage>(pendingMsg));
      auto respPayload = std::make_shared<QcRilRequestMessageCallbackPayload>(data->errNo, nullptr);
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

void VoiceModule::processReleaseHeldOrWaitingCommandOversightCompletion(CommonVoiceResponseData *data) {
  QCRIL_LOG_FUNC_ENTRY();
  if (data) {
    auto pendingMsg = getPendingMessageList().extract(data->req_id);
    if (pendingMsg) {
      auto msg(std::static_pointer_cast<QcRilRequestMessage>(pendingMsg));
      auto respPayload = std::make_shared<QcRilRequestMessageCallbackPayload>(data->errNo, nullptr);
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
    }

    if (data->errNo != RIL_E_SUCCESS) {
      qcril_qmi_voice_voip_unmark_with_specified_call_state(
          QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_CALL_END_BY_USER, CALL_STATE_WAITING_V02);
      qcril_qmi_voice_voip_mark_with_specified_call_state(
          QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_PENDING_INCOMING, CALL_STATE_WAITING_V02);
      qcril_qmi_voice_voip_unmark_with_specified_call_state(
          QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_CALL_END_BY_USER, CALL_STATE_INCOMING_V02);
      qcril_qmi_voice_voip_mark_with_specified_call_state(
          QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_PENDING_INCOMING, CALL_STATE_INCOMING_V02);
      qcril_qmi_voice_voip_unmark_with_specified_call_state(
          QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_CALL_END_BY_USER, CALL_STATE_HOLD_V02);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

void VoiceModule::processAnswerCommandOversightCompletion(CommonVoiceResponseData *data) {
  QCRIL_LOG_FUNC_ENTRY();
  if (data) {
    auto pendingMsg = getPendingMessageList().extract(data->req_id);
    if (pendingMsg) {
      auto msg(std::static_pointer_cast<QcRilRequestMessage>(pendingMsg));
      auto respPayload = std::make_shared<QcRilRequestMessageCallbackPayload>(data->errNo, nullptr);
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

void VoiceModule::processSetupAnswerCommandOversightCompletion(CommonVoiceResponseData *data) {
  QCRIL_LOG_FUNC_ENTRY();
  if (data) {
    auto pendingMsg = getPendingMessageList().extract(data->req_id);
    if (pendingMsg) {
      auto msg(std::static_pointer_cast<QcRilRequestMessage>(pendingMsg));
      auto respPayload = std::make_shared<QcRilRequestMessageCallbackPayload>(data->errNo, nullptr);
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

void VoiceModule::processDialCallResponse(CommonVoiceResponseData *data) {
  QCRIL_LOG_FUNC_ENTRY();
  (void)data;
  // Send response from processDialCommandOversightCompletion
  QCRIL_LOG_FUNC_RETURN();
}

void VoiceModule::processHangupResponse(CommonVoiceResponseData *data) {
  QCRIL_LOG_FUNC_ENTRY();
  (void)data;
  // Send response from processHangupCommandOversightCompletion
  QCRIL_LOG_FUNC_RETURN();
}

void VoiceModule::processStartContDtmfResponse(CommonVoiceResponseData *data) {
  QCRIL_LOG_FUNC_ENTRY();
  if (data) {
    bool sendResp = true;
    auto pendingMsg = getPendingMessageList().find(data->req_id);
    if (pendingMsg &&
        (pendingMsg->get_message_id() == QcRilRequestSendDtmfMessage::get_class_message_id())) {
      voice_stop_cont_dtmf_req_msg_v02 stop_cont_dtmf_req = {
          .call_id = QCRIL_QMI_VOICE_UNKNOWN_ACTIVE_CONN_ID};
      RIL_Errno rilErr = qcril_qmi_voice_process_stop_cont_dtmf_req(
          data->req_id, stop_cont_dtmf_req,
          std::bind(&VoiceModule::processStopContDtmfResponse, this, std::placeholders::_1));
      sendResp = (rilErr != RIL_E_SUCCESS);
    }
    if (sendResp) {
      auto pendingMsg = getPendingMessageList().extract(data->req_id);
      if (pendingMsg) {
        auto msg(std::static_pointer_cast<QcRilRequestMessage>(pendingMsg));
        auto respPayload =
            std::make_shared<QcRilRequestMessageCallbackPayload>(data->errNo, nullptr);
        msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
      }
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

void VoiceModule::processStopContDtmfResponse(CommonVoiceResponseData *data) {
  QCRIL_LOG_FUNC_ENTRY();
  if (data) {
    auto pendingMsg = getPendingMessageList().extract(data->req_id);
    if (pendingMsg) {
      auto msg(std::static_pointer_cast<QcRilRequestMessage>(pendingMsg));
      auto respPayload = std::make_shared<QcRilRequestMessageCallbackPayload>(data->errNo, nullptr);
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

void VoiceModule::processReleaseHeldOrWaitingResponse(CommonVoiceResponseData *data) {
  QCRIL_LOG_FUNC_ENTRY();
  (void)data;
  // Send response from processReleaseHeldOrWaitingCommandOversightCompletion
  QCRIL_LOG_FUNC_RETURN();
}

void VoiceModule::processReleaseActiveAcceptHeldOrWaitingResponse(CommonVoiceResponseData *data) {
  QCRIL_LOG_FUNC_ENTRY();
  if (data) {
    voice_manage_calls_resp_msg_v02 *qmiResp =
        static_cast<voice_manage_calls_resp_msg_v02 *>(data->respData);
    if (qmiResp) {
      if (qmiResp->failure_cause_valid == TRUE) {
        QCRIL_LOG_ERROR("QCRIL QMI VOICE MNG CALLS RESP sups_failure_cause=%d",
                        qmiResp->failure_cause);
        qcril_qmi_send_ss_failure_cause_oem_hook_unsol_resp(qmiResp->failure_cause,
                                                            VOICE_INVALID_CALL_ID);
      }
    }

    auto pendingMsg = getPendingMessageList().extract(data->req_id);
    if (pendingMsg) {
      auto msg(std::static_pointer_cast<QcRilRequestMessage>(pendingMsg));
      auto respPayload = std::make_shared<QcRilRequestMessageCallbackPayload>(data->errNo, nullptr);
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
    }

    qcril_qmi_voice_voip_unmark_all_with(
        QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_SWITCHING_CALL_TO_ACTIVE);
    if (data->errNo != RIL_E_SUCCESS) {
      qcril_am_handle_event(QCRIL_AM_EVENT_SWITCH_CALL_FAIL, NULL);
      qcril_qmi_voice_voip_unmark_with_specified_call_state(
          QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_CALL_END_BY_USER, CALL_STATE_CONVERSATION_V02);
      qcril_qmi_voice_voip_mark_with_specified_call_state(
          QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_PENDING_INCOMING, CALL_STATE_WAITING_V02);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

void VoiceModule::processHoldActiveAcceptWaitingOrHeldResponse(CommonVoiceResponseData *data) {
  QCRIL_LOG_FUNC_ENTRY();
  if (data) {
    voice_manage_calls_resp_msg_v02 *qmiResp =
        static_cast<voice_manage_calls_resp_msg_v02 *>(data->respData);
    if (qmiResp) {
      if (qmiResp->failure_cause_valid == TRUE) {
        QCRIL_LOG_ERROR("QCRIL QMI VOICE MNG CALLS RESP sups_failure_cause=%d",
                        qmiResp->failure_cause);
        qcril_qmi_send_ss_failure_cause_oem_hook_unsol_resp(qmiResp->failure_cause,
                                                            VOICE_INVALID_CALL_ID);
      }
    }

    auto pendingMsg = getPendingMessageList().extract(data->req_id);
    if (pendingMsg) {
      auto msg(std::static_pointer_cast<QcRilRequestMessage>(pendingMsg));
      auto respPayload = std::make_shared<QcRilRequestMessageCallbackPayload>(data->errNo, nullptr);
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
    }

    qcril_qmi_voice_voip_unmark_all_with(
        QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_SWITCHING_CALL_TO_ACTIVE);
    if (data->errNo != RIL_E_SUCCESS) {
      qcril_am_handle_event(QCRIL_AM_EVENT_SWITCH_CALL_FAIL, NULL);
      qcril_qmi_voice_voip_mark_with_specified_call_state(
          QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_PENDING_INCOMING, CALL_STATE_WAITING_V02);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

void VoiceModule::processCommonManageCallsResponse(CommonVoiceResponseData *data) {
  QCRIL_LOG_FUNC_ENTRY();
  if (data) {
    voice_manage_calls_resp_msg_v02 *qmiResp =
        static_cast<voice_manage_calls_resp_msg_v02 *>(data->respData);
    if (qmiResp) {
      if (qmiResp->failure_cause_valid == TRUE) {
        QCRIL_LOG_ERROR("QCRIL QMI VOICE MNG CALLS RESP sups_failure_cause=%d",
                        qmiResp->failure_cause);
        /* Send UNSOL msg with SS error code first */
        qcril_qmi_send_ss_failure_cause_oem_hook_unsol_resp(qmiResp->failure_cause,
                                                            VOICE_INVALID_CALL_ID);
      }
    }
    auto pendingMsg = getPendingMessageList().extract(data->req_id);
    if (pendingMsg) {
      auto msg(std::static_pointer_cast<QcRilRequestMessage>(pendingMsg));
      auto respPayload = std::make_shared<QcRilRequestMessageCallbackPayload>(data->errNo, nullptr);
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

void VoiceModule::processAnswerResponse(CommonVoiceResponseData *data) {
  QCRIL_LOG_FUNC_ENTRY();
  if (data->errNo != RIL_E_SUCCESS) {
    qcril_am_handle_event(QCRIL_AM_EVENT_VOICE_ANSWER_FAIL, NULL);
  }
  QCRIL_LOG_FUNC_RETURN();
}

void VoiceModule::processSetupAnswerResponse(CommonVoiceResponseData * /*data*/) {
  QCRIL_LOG_FUNC_ENTRY();
  QCRIL_LOG_FUNC_RETURN();
}

void VoiceModule::processSendFlashResponse(CommonVoiceResponseData *data) {
  QCRIL_LOG_FUNC_ENTRY();
  if (data) {
    voice_send_flash_resp_msg_v02 *qmiResp =
        static_cast<voice_send_flash_resp_msg_v02 *>(data->respData);
    if (qmiResp) {
      if (data->errNo == RIL_E_MODEM_ERR && qmiResp->resp.error == QMI_ERR_INVALID_ID_V01) {
        // handle specific error
        data->errNo = RIL_E_INVALID_CALL_ID;
      }
    }
    auto pendingMsg = getPendingMessageList().extract(data->req_id);
    if (pendingMsg) {
      auto msg(std::static_pointer_cast<QcRilRequestMessage>(pendingMsg));
      auto respPayload = std::make_shared<QcRilRequestMessageCallbackPayload>(data->errNo, nullptr);
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

void VoiceModule::processBurstDtmfResponse(CommonVoiceResponseData *data) {
  QCRIL_LOG_FUNC_ENTRY();
  if (data) {
    voice_burst_dtmf_resp_msg_v02 *qmiResp =
        static_cast<voice_burst_dtmf_resp_msg_v02 *>(data->respData);
    if (qmiResp) {
      if (data->errNo == RIL_E_MODEM_ERR && qmiResp->resp.error == QMI_ERR_INVALID_ID_V01) {
        // handle specific error
        data->errNo = RIL_E_INVALID_CALL_ID;
      }
    }
    auto pendingMsg = getPendingMessageList().extract(data->req_id);
    if (pendingMsg) {
      auto msg(std::static_pointer_cast<QcRilRequestMessage>(pendingMsg));
      auto respPayload = std::make_shared<QcRilRequestMessageCallbackPayload>(data->errNo, nullptr);
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

void VoiceModule::processSetConfigResponse(CommonVoiceResponseData *data) {
  QCRIL_LOG_FUNC_ENTRY();
  if (data) {
    // Validate outcome parameter from qmi response
    voice_set_config_resp_msg_v02 *qmiResp =
        static_cast<voice_set_config_resp_msg_v02 *>(data->respData);
    if (qmiResp && (data->errNo == RIL_E_SUCCESS)) {
      QCRIL_LOG_INFO(".. outcome  %d, %d", qmiResp->tty_mode_outcome_valid,
                     qmiResp->tty_mode_outcome);
      if (qmiResp->tty_mode_outcome_valid && qmiResp->tty_mode_outcome != 0x00) {
        data->errNo = RIL_E_MODEM_ERR;
      }
    }
    auto pendingMsg = getPendingMessageList().extract(data->req_id);
    if (pendingMsg) {
      auto msg(std::static_pointer_cast<QcRilRequestMessage>(pendingMsg));
      auto respPayload = std::make_shared<QcRilRequestMessageCallbackPayload>(data->errNo, nullptr);
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

void VoiceModule::processGetConfigResponse(CommonVoiceResponseData *data) {
  QCRIL_LOG_FUNC_ENTRY();
  std::shared_ptr<qcril::interfaces::TtyModeResp> ttyMode = nullptr;
  if (data) {
    voice_get_config_resp_msg_v02 *qmiResp =
        static_cast<voice_get_config_resp_msg_v02 *>(data->respData);
    if (qmiResp && qmiResp->current_tty_mode_valid) {
      qcril::interfaces::TtyMode mode = qcril::interfaces::TtyMode::UNKNOWN;
      switch (qmiResp->current_tty_mode) {
        case TTY_MODE_FULL_V02:
          mode = qcril::interfaces::TtyMode::FULL;
          break;
        case TTY_MODE_VCO_V02:
          mode = qcril::interfaces::TtyMode::VCO;
          break;
        case TTY_MODE_HCO_V02:
          mode = qcril::interfaces::TtyMode::HCO;
          break;
        case TTY_MODE_OFF_V02:  // fallthrough
        default:
          mode = qcril::interfaces::TtyMode::MODE_OFF;
          break;
      }
      ttyMode = std::make_shared<qcril::interfaces::TtyModeResp>();
      if (ttyMode) {
        ttyMode->setTtyMode(mode);
      }
    }
    auto pendingMsg = getPendingMessageList().extract(data->req_id);
    if (pendingMsg) {
      auto msg(std::static_pointer_cast<QcRilRequestMessage>(pendingMsg));
      auto respPayload = std::make_shared<QcRilRequestMessageCallbackPayload>(data->errNo, ttyMode);
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

void VoiceModule::processGetPreferredVoicePrivacyResponse(CommonVoiceResponseData *data) {
  QCRIL_LOG_FUNC_ENTRY();
  std::shared_ptr<qcril::interfaces::PrivacyModeResp> privacyMode = nullptr;
  if (data) {
    voice_get_config_resp_msg_v02 *qmiResp =
        static_cast<voice_get_config_resp_msg_v02 *>(data->respData);
    if (qmiResp && qmiResp->current_voice_privacy_pref_valid) {
      qcril::interfaces::PrivacyMode mode = qcril::interfaces::PrivacyMode::UNKNOWN;
      switch (qmiResp->current_voice_privacy_pref) {
        case VOICE_PRIVACY_STANDARD_V02:
          mode = qcril::interfaces::PrivacyMode::STANDARD;
          break;
        case VOICE_PRIVACY_ENHANCED_V02:
          mode = qcril::interfaces::PrivacyMode::ENHANCED;
          break;
        default:
          mode = qcril::interfaces::PrivacyMode::UNKNOWN;
          break;
      }
      if (mode != qcril::interfaces::PrivacyMode::UNKNOWN) {
        privacyMode = std::make_shared<qcril::interfaces::PrivacyModeResp>();
        if (privacyMode) {
          privacyMode->setPrivacyMode(mode);
        }
      }
    }
    auto pendingMsg = getPendingMessageList().extract(data->req_id);
    if (pendingMsg) {
      auto msg(std::static_pointer_cast<QcRilRequestMessage>(pendingMsg));
      auto respPayload =
          std::make_shared<QcRilRequestMessageCallbackPayload>(data->errNo, privacyMode);
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

void VoiceModule::processSetPreferredVoicePrivacyResponse(CommonVoiceResponseData *data) {
  QCRIL_LOG_FUNC_ENTRY();
  if (data) {
    auto pendingMsg = getPendingMessageList().extract(data->req_id);
    if (pendingMsg) {
      auto msg(std::static_pointer_cast<QcRilRequestMessage>(pendingMsg));
      auto respPayload = std::make_shared<QcRilRequestMessageCallbackPayload>(data->errNo, nullptr);
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

void VoiceModule::processOrigUssdResponse(CommonVoiceResponseData *data) {
  QCRIL_LOG_FUNC_ENTRY();
  if (data) {
    auto pendingMsg = getPendingMessageList().extract(data->req_id);
    if (pendingMsg) {
      auto msg(std::static_pointer_cast<QcRilRequestMessage>(pendingMsg));
      auto respPayload = std::make_shared<QcRilRequestMessageCallbackPayload>(data->errNo, nullptr);
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

void VoiceModule::processAnswerUssdResponse(CommonVoiceResponseData *data) {
  QCRIL_LOG_FUNC_ENTRY();
  if (data) {
    auto pendingMsg = getPendingMessageList().extract(data->req_id);
    if (pendingMsg) {
      auto msg(std::static_pointer_cast<QcRilRequestMessage>(pendingMsg));
      auto respPayload = std::make_shared<QcRilRequestMessageCallbackPayload>(data->errNo, nullptr);
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

void VoiceModule::processCancelUssdResponse(CommonVoiceResponseData *data) {
  QCRIL_LOG_FUNC_ENTRY();
  if (data) {
    auto pendingMsg = getPendingMessageList().extract(data->req_id);
    if (pendingMsg) {
      auto msg(std::static_pointer_cast<QcRilRequestMessage>(pendingMsg));
      auto respPayload = std::make_shared<QcRilRequestMessageCallbackPayload>(data->errNo, nullptr);
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}


void VoiceModule::processGetClipResponse(CommonVoiceResponseData *data) {
  std::shared_ptr<qcril::interfaces::ClipInfo> clipInfo = nullptr;
  voice_get_clip_resp_msg_v02 *qmiResp = nullptr;
  QCRIL_LOG_FUNC_ENTRY();
  if (data) {
    auto pendingMsg = getPendingMessageList().extract(data->req_id);
    auto msg(std::static_pointer_cast<QcRilRequestMessage>(pendingMsg));
    qmiResp = static_cast<voice_get_clip_resp_msg_v02 *>(data->respData);
    if (qmiResp) {
      clipInfo = std::make_shared<qcril::interfaces::ClipInfo>();
      if (clipInfo) {
        if (qmiResp->clip_response_valid) {
          clipInfo->setClipStatus(
              mapActiveStatusToClipStatus(qmiResp->clip_response.active_status));
        }
        if (qmiResp->sip_error_code_valid || qmiResp->failure_cause_description_valid) {
          auto errorDetails = buildSipErrorInfo(
              qmiResp->sip_error_code_valid, qmiResp->sip_error_code,
              qmiResp->failure_cause_description_valid ? qmiResp->failure_cause_description_len : 0,
              (uint16_t *)&(qmiResp->failure_cause_description));
          clipInfo->setErrorDetails(errorDetails);
        }
      }
      if (data->errNo != RIL_E_SUCCESS) {
        if (msg && !msg->isImsRequest()) {
          if (qmiResp->failure_cause_valid == TRUE) {
            qcril_qmi_send_ss_failure_cause_oem_hook_unsol_resp(
                qmiResp->failure_cause,
                (qmiResp->call_id_valid ? qmiResp->call_id : VOICE_INVALID_CALL_ID));
          }
        }
      }
    }
    if (msg) {
      auto respPayload =
          std::make_shared<QcRilRequestMessageCallbackPayload>(data->errNo, clipInfo);
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

void VoiceModule::processGetClirResponse(CommonVoiceResponseData *data) {
  std::shared_ptr<qcril::interfaces::ClirInfo> clirInfo = nullptr;
  voice_get_clir_resp_msg_v02 *qmiResp = nullptr;
  QCRIL_LOG_FUNC_ENTRY();
  if (data) {
    auto pendingMsg = getPendingMessageList().extract(data->req_id);
    auto msg(std::static_pointer_cast<QcRilRequestMessage>(pendingMsg));
    qmiResp = static_cast<voice_get_clir_resp_msg_v02 *>(data->respData);
    if (qmiResp) {
      qmi_result_type_v01 qmi_result = qmiResp->resp.result;
      qmi_error_type_v01 qmi_error = qmiResp->resp.error;
      clirInfo = std::make_shared<qcril::interfaces::ClirInfo>();
      uint8 clir = qcril_qmi_voice_get_cached_clir();
      if (clirInfo) {
        if ((qmi_result == QMI_RESULT_SUCCESS_V01) &&
            (qmi_error == QMI_ERR_SUPS_FAILURE_CAUSE_V01)) {
          if (qmiResp->clir_response_valid == TRUE) {
            clirInfo->setAction(clir);
            clirInfo->setPresentation(QCRIL_QMI_VOICE_CLIR_SRV_NO_NETWORK);
            data->errNo = RIL_E_SUCCESS;
          }
        } else {
          if ((qmiResp->failure_cause_valid == TRUE) &&
              (qmiResp->failure_cause == QMI_FAILURE_CAUSE_FACILITY_NOT_SUPPORTED_V02)) {
            data->errNo = RIL_E_SUCCESS;
            int feature_fac_not_supp_as_no_nw = qcril_qmi_voice_feature_fac_not_supp_as_no_nw();
            QCRIL_LOG_DEBUG("feature_fac_not_supp_as_no_nw = %d\n", feature_fac_not_supp_as_no_nw);
            if (feature_fac_not_supp_as_no_nw) {
              clirInfo->setAction(QCRIL_QMI_VOICE_SS_CLIR_PRESENTATION_INDICATOR);
              clirInfo->setPresentation(QCRIL_QMI_VOICE_CLIR_SRV_NO_NETWORK);
            } else {
              // In case get_clir not supported by IMS at modem, then return cached value.
              if (clir == QCRIL_QMI_VOICE_SS_CLIR_INVOCATION_OPTION) {
                clirInfo->setPresentation(QCRIL_QMI_VOICE_CLIR_SRV_PRESENTATION_ALLOWED);
              } else if (clir == QCRIL_QMI_VOICE_SS_CLIR_SUPPRESSION_OPTION) {
                clirInfo->setPresentation(QCRIL_QMI_VOICE_CLIR_SRV_PRESENTATION_RESTRICTED);
              } else {
                clirInfo->setPresentation(QCRIL_QMI_VOICE_CLIR_SRV_PRESENTATION_ALLOWED);
              }
              clirInfo->setAction(clir);
            }
          } else if ((qmiResp->clir_response_valid == TRUE) && (qmi_error == QMI_ERR_NONE_V01)) {
            data->errNo = RIL_E_SUCCESS;
            clirInfo->setAction(clir);
            int ril_provision_status = QCRIL_QMI_VOICE_CLIR_SRV_NOT_PROVISIONED;
            boolean ret = qcril_qmi_voice_map_qmi_to_ril_provision_status(
                qmiResp->clir_response.provision_status, &ril_provision_status);
            if (ret) {
              clirInfo->setPresentation(ril_provision_status);
            }
          }
        }
      }
      if (data->errNo != RIL_E_SUCCESS) {
        if (msg && !msg->isImsRequest()) {
          if (qmiResp->failure_cause_valid == TRUE) {
            qcril_qmi_send_ss_failure_cause_oem_hook_unsol_resp(
                qmiResp->failure_cause,
                (qmiResp->call_id_valid ? qmiResp->call_id : VOICE_INVALID_CALL_ID));
          }
        }
      }
    }
    if (msg) {
      auto respPayload =
          std::make_shared<QcRilRequestMessageCallbackPayload>(data->errNo, clirInfo);
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

void VoiceModule::processGetColrResponse(CommonVoiceResponseData *data) {
  QCRIL_LOG_FUNC_ENTRY();
  std::shared_ptr<qcril::interfaces::ColrInfo> colrInfo = nullptr;
  voice_get_colr_resp_msg_v02 *qmiResp = nullptr;
  if (data) {
    qmiResp = static_cast<voice_get_colr_resp_msg_v02 *>(data->respData);
    if (qmiResp) {
      if (qmiResp->colr_pi_valid != TRUE && qmiResp->colr_response_valid != TRUE) {
        data->errNo = RIL_E_GENERIC_FAILURE;
      }
      colrInfo = buildColrInfo(qmiResp);
    }
    auto pendingMsg = getPendingMessageList().extract(data->req_id);
    if (pendingMsg) {
      auto respPayload =
          std::make_shared<QcRilRequestMessageCallbackPayload>(data->errNo, colrInfo);
      auto msg(std::static_pointer_cast<QcRilRequestMessage>(pendingMsg));
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

void VoiceModule::processGetCallWaitingResponse(CommonVoiceResponseData *data) {
  QCRIL_LOG_FUNC_ENTRY();
  std::shared_ptr<qcril::interfaces::CallWaitingInfo> callWaitingInfo = nullptr;
  voice_get_call_waiting_resp_msg_v02 *qmiResp = nullptr;
  if (data) {
    qmiResp = static_cast<voice_get_call_waiting_resp_msg_v02 *>(data->respData);
    if (qmiResp) {
      callWaitingInfo = std::make_shared<qcril::interfaces::CallWaitingInfo>();
      if (callWaitingInfo) {
        if (data->errNo == RIL_E_SUCCESS && qmiResp->service_class_valid) {
          if (qmiResp->service_class == 0x00) {
            callWaitingInfo->setStatus(qcril::interfaces::ServiceClassStatus::DISABLED);
          } else {
            callWaitingInfo->setStatus(qcril::interfaces::ServiceClassStatus::ENABLED);
            callWaitingInfo->setServiceClass(qmiResp->service_class);
          }
        } else {
          QCRIL_LOG_INFO("QCRIL QMI VOICE Query Call Waiting RESP: FAILURE");
          if (qmiResp->failure_cause_valid) {
            qcril_qmi_send_ss_failure_cause_oem_hook_unsol_resp(
                qmiResp->failure_cause,
                (qmiResp->call_id_valid ? qmiResp->call_id : VOICE_INVALID_CALL_ID));
          }
          if (qmiResp->sip_error_code_valid || qmiResp->failure_cause_description_valid) {
            auto errorDetails = buildSipErrorInfo(
                qmiResp->sip_error_code_valid, qmiResp->sip_error_code,
                qmiResp->failure_cause_description_valid ? qmiResp->failure_cause_description_len
                                                         : 0,
                (uint16_t *)&(qmiResp->failure_cause_description));
            callWaitingInfo->setErrorDetails(errorDetails);
          }
        }
      }
      if (qmiResp->resp.error == QMI_ERR_OP_NETWORK_UNSUPPORTED_V01) {
        data->errNo = RIL_E_NETWORK_NOT_SUPPORTED;
      }
    }
    auto pendingMsg = getPendingMessageList().extract(data->req_id);
    if (pendingMsg) {
      auto respPayload =
          std::make_shared<QcRilRequestMessageCallbackPayload>(data->errNo, callWaitingInfo);
      auto msg(std::static_pointer_cast<QcRilRequestMessage>(pendingMsg));
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

void VoiceModule::processSetSupsServiceResponse(CommonVoiceResponseData *data) {
  QCRIL_LOG_FUNC_ENTRY();
  std::shared_ptr<qcril::interfaces::SipErrorInfo> errorDetails = nullptr;
  voice_set_sups_service_resp_msg_v02 *qmiResp = nullptr;
  if (data) {
    qmiResp = static_cast<voice_set_sups_service_resp_msg_v02 *>(data->respData);
    if (qmiResp) {
      if (data->errNo != RIL_E_SUCCESS) {
        if (qmiResp->failure_cause_valid == TRUE) {
          qcril_qmi_send_ss_failure_cause_oem_hook_unsol_resp(
              qmiResp->failure_cause,
              (qmiResp->call_id_valid ? qmiResp->call_id : VOICE_INVALID_CALL_ID));
        }
      }
      if ((data->errNo == RIL_E_MODEM_ERR) && (qmiResp->failure_cause_valid) &&
          (qmiResp->failure_cause == QMI_FAILURE_CAUSE_NEGATIVE_PWD_CHECK_V02)) {
        data->errNo = RIL_E_PASSWORD_INCORRECT;
      }
      if (qmiResp->sip_error_code_valid || qmiResp->failure_cause_description_valid) {
        errorDetails = buildSipErrorInfo(
            qmiResp->sip_error_code_valid, qmiResp->sip_error_code,
            qmiResp->failure_cause_description_valid ? qmiResp->failure_cause_description_len : 0,
            (uint16_t *)&(qmiResp->failure_cause_description));
      }
    }
    auto pendingMsg = getPendingMessageList().extract(data->req_id);
    if (pendingMsg) {
      auto respPayload =
          std::make_shared<QcRilRequestMessageCallbackPayload>(data->errNo, errorDetails);
      auto msg(std::static_pointer_cast<QcRilRequestMessage>(pendingMsg));
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

void VoiceModule::processSetCallFwdSupsReqResponse(CommonVoiceResponseData *data) {
  QCRIL_LOG_FUNC_ENTRY();
  std::shared_ptr<qcril::interfaces::SetCallForwardRespData> setCfRespData = nullptr;
  voice_set_sups_service_resp_msg_v02 *qmiResp = nullptr;
  if (data) {
    qmiResp = static_cast<voice_set_sups_service_resp_msg_v02 *>(data->respData);
    setCfRespData = std::make_shared<qcril::interfaces::SetCallForwardRespData>();
    if (qmiResp && setCfRespData) {
      if (data->errNo != RIL_E_SUCCESS) {
        if (qmiResp->failure_cause_valid == TRUE) {
          qcril_qmi_send_ss_failure_cause_oem_hook_unsol_resp(
              qmiResp->failure_cause,
              (qmiResp->call_id_valid ? qmiResp->call_id : VOICE_INVALID_CALL_ID));
        }
      }
      if (qmiResp->sip_error_code_valid || qmiResp->failure_cause_description_valid) {
        auto errorDetails = buildSipErrorInfo(
            qmiResp->sip_error_code_valid, qmiResp->sip_error_code,
            qmiResp->failure_cause_description_valid ? qmiResp->failure_cause_description_len
                                                     : 0,
            (uint16_t *)&(qmiResp->failure_cause_description));
        setCfRespData->setErrorDetails(errorDetails);
      }
    }

    auto pendingMsg = getPendingMessageList().extract(data->req_id);
    if (pendingMsg) {
      auto respPayload =
          std::make_shared<QcRilRequestMessageCallbackPayload>(data->errNo, setCfRespData);
      auto msg(std::static_pointer_cast<QcRilRequestMessage>(pendingMsg));
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

void VoiceModule::processSetAllCallFwdSupsReqResponse(CommonVoiceResponseData *data) {
  QCRIL_LOG_FUNC_ENTRY();
  std::shared_ptr<qcril::interfaces::SetCallForwardRespData> setCfRespData = nullptr;
  voice_set_all_call_fwd_sups_resp_msg_v02 *qmiResp = nullptr;
  if (data) {
    qmiResp = static_cast<voice_set_all_call_fwd_sups_resp_msg_v02 *>(data->respData);
    setCfRespData = std::make_shared<qcril::interfaces::SetCallForwardRespData>();
    if (qmiResp && setCfRespData) {
      if (data->errNo != RIL_E_SUCCESS) {
        if (qmiResp->failure_cause_valid == TRUE) {
          qcril_qmi_send_ss_failure_cause_oem_hook_unsol_resp(
              qmiResp->failure_cause,
              (qmiResp->call_id_valid ? qmiResp->call_id : VOICE_INVALID_CALL_ID));
        }
      }
      if (qmiResp->sip_error_code_valid || qmiResp->failure_cause_description_valid) {
        auto errorDetails = buildSipErrorInfo(
            qmiResp->sip_error_code_valid, qmiResp->sip_error_code,
            qmiResp->failure_cause_description_valid ? QMI_VOICE_FAILURE_CAUSE_DESC_MAX_LEN_V02 : 0,
            (uint16_t *)&(qmiResp->failure_cause_description));
        setCfRespData->setErrorDetails(errorDetails);
      }
      qcril::interfaces::SetCallForwardStatus setCfStatus = {};
      if (qmiResp->cfu_result_valid) {
        setCfStatus.setReason(QCRIL_QMI_VOICE_CCFC_REASON_UNCOND);
        setCfStatus.setStatus(qmiResp->cfu_result == QMI_RESULT_SUCCESS_V01 ? true : false);
        auto errorDetails =
            buildSipErrorInfo(qmiResp->cfu_sip_error_code_valid, qmiResp->cfu_sip_error_code,
                              qmiResp->cfu_failure_cause_description_valid
                                  ? QMI_VOICE_FAILURE_CAUSE_DESC_MAX_LEN_V02
                                  : 0,
                              (uint16_t *)&(qmiResp->cfu_failure_cause_description));
        setCfStatus.setErrorDetails(errorDetails);
        setCfRespData->addToSetCallForwardStatus(setCfStatus);
      }
      setCfStatus = {};
      if (qmiResp->cfb_result_valid) {
        setCfStatus.setReason(QCRIL_QMI_VOICE_CCFC_REASON_BUSY);
        setCfStatus.setStatus(qmiResp->cfb_result == QMI_RESULT_SUCCESS_V01 ? true : false);
        auto errorDetails =
            buildSipErrorInfo(qmiResp->cfb_sip_error_code_valid, qmiResp->cfb_sip_error_code,
                              qmiResp->cfb_failure_cause_description_valid
                                  ? QMI_VOICE_FAILURE_CAUSE_DESC_MAX_LEN_V02
                                  : 0,
                              (uint16_t *)&(qmiResp->cfb_failure_cause_description));
        setCfStatus.setErrorDetails(errorDetails);
        setCfRespData->addToSetCallForwardStatus(setCfStatus);
      }
      setCfStatus = {};
      if (qmiResp->cfnry_result_valid) {
        setCfStatus.setReason(QCRIL_QMI_VOICE_CCFC_REASON_NOREPLY);
        setCfStatus.setStatus(qmiResp->cfnry_result == QMI_RESULT_SUCCESS_V01 ? true : false);
        auto errorDetails =
            buildSipErrorInfo(qmiResp->cfnry_sip_error_code_valid, qmiResp->cfnry_sip_error_code,
                              qmiResp->cfnry_failure_cause_description_valid
                                  ? QMI_VOICE_FAILURE_CAUSE_DESC_MAX_LEN_V02
                                  : 0,
                              (uint16_t *)&(qmiResp->cfnry_failure_cause_description));
        setCfStatus.setErrorDetails(errorDetails);
        setCfRespData->addToSetCallForwardStatus(setCfStatus);
      }
      setCfStatus = {};
      if (qmiResp->cfnrc_result_valid) {
        setCfStatus.setReason(QCRIL_QMI_VOICE_CCFC_REASON_NOTREACH);
        setCfStatus.setStatus(qmiResp->cfnrc_result == QMI_RESULT_SUCCESS_V01 ? true : false);
        auto errorDetails =
            buildSipErrorInfo(qmiResp->cfnrc_sip_error_code_valid, qmiResp->cfnrc_sip_error_code,
                              qmiResp->cfnrc_failure_cause_description_valid
                                  ? QMI_VOICE_FAILURE_CAUSE_DESC_MAX_LEN_V02
                                  : 0,
                              (uint16_t *)&(qmiResp->cfnrc_failure_cause_description));
        setCfStatus.setErrorDetails(errorDetails);
        setCfRespData->addToSetCallForwardStatus(setCfStatus);
      }
    }
    auto pendingMsg = getPendingMessageList().extract(data->req_id);
    if (pendingMsg) {
      auto respPayload =
          std::make_shared<QcRilRequestMessageCallbackPayload>(data->errNo, setCfRespData);
      auto msg(std::static_pointer_cast<QcRilRequestMessage>(pendingMsg));
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

void VoiceModule::processGetCallBarringResponse(CommonVoiceResponseData *data) {
  QCRIL_LOG_FUNC_ENTRY();
  auto suppSvc = std::make_shared<qcril::interfaces::SuppServiceStatusInfo>();
  voice_get_call_barring_resp_msg_v02 *qmiResp = nullptr;
  if (data) {
    qmiResp = static_cast<voice_get_call_barring_resp_msg_v02 *>(data->respData);
    if (qmiResp && suppSvc) {
      if (data->errNo != RIL_E_SUCCESS) {
        if (qmiResp->failure_cause_valid == TRUE) {
          qcril_qmi_send_ss_failure_cause_oem_hook_unsol_resp(
              qmiResp->failure_cause,
              (qmiResp->call_id_valid ? qmiResp->call_id : VOICE_INVALID_CALL_ID));
        }
      }
      if (data->errNo == RIL_E_SUCCESS) {
        if (qmiResp->service_class_valid) {
          if (qmiResp->service_class == 0x00) {
            suppSvc->setStatus(qcril::interfaces::ServiceClassStatus::DISABLED);
          } else {
            suppSvc->setStatus(qcril::interfaces::ServiceClassStatus::ENABLED);
          }
          suppSvc->setServiceClass(qmiResp->service_class);
          if (qmiResp->cc_sups_result_valid) {
            qcril::interfaces::FacilityType facility = qcril::interfaces::FacilityType::UNKNOWN;
            bool ret = mapFacilityType(facility,
                                       (voice_reason_enum_v02) qmiResp->cc_sups_result.reason);
            if (ret && facility != qcril::interfaces::FacilityType::UNKNOWN) {
              suppSvc->setFacilityType(facility);
            }
          }
          if (qmiResp->sc_barred_numbers_status_list_valid) {
            for (uint32_t i = 0; i < qmiResp->sc_barred_numbers_status_list_len; i++) {
              qcril::interfaces::CallBarringNumbersListInfo cbNumList = {};
              cbNumList.setServiceClass(
                  qmiResp->sc_barred_numbers_status_list[i].service_class_ext);
              for (uint32_t j = 0;
                   j < qmiResp->sc_barred_numbers_status_list[i].call_barring_numbers_list_len;
                   j++) {
                qcril::interfaces::CallBarringNumbersInfo cbNumInfo = {};
                cbNumInfo.setStatus(mapActiveStatus(qmiResp->sc_barred_numbers_status_list[i]
                                                        .call_barring_numbers_list[j]
                                                        .active_status));
                cbNumInfo.setNumber(qmiResp->sc_barred_numbers_status_list[i]
                                        .call_barring_numbers_list[j]
                                        .barred_number);
                cbNumList.addToCallBarringNumbersInfo(cbNumInfo);
              }
              suppSvc->addToCallBarringNumbersListInfo(cbNumList);
            }
          }
        }
      }

      if (qmiResp->sip_error_code_valid || qmiResp->failure_cause_description_valid) {
        auto errorDetails = buildSipErrorInfo(
            qmiResp->sip_error_code_valid, qmiResp->sip_error_code,
            qmiResp->failure_cause_description_valid ? qmiResp->failure_cause_description_len : 0,
            (uint16_t *)&(qmiResp->failure_cause_description));
        suppSvc->setErrorDetails(errorDetails);
      }
    }
    auto pendingMsg = getPendingMessageList().extract(data->req_id);
    if (pendingMsg) {
      auto respPayload = std::make_shared<QcRilRequestMessageCallbackPayload>(data->errNo, suppSvc);
      auto msg(std::static_pointer_cast<QcRilRequestMessage>(pendingMsg));
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

void VoiceModule::processGetColpResponse(CommonVoiceResponseData *data) {
  QCRIL_LOG_FUNC_ENTRY();
  auto suppSvc = std::make_shared<qcril::interfaces::SuppServiceStatusInfo>();
  voice_get_colp_resp_msg_v02 *qmiResp = nullptr;
  if (data) {
    qmiResp = static_cast<voice_get_colp_resp_msg_v02 *>(data->respData);
    if (qmiResp && suppSvc) {
      if (!qmiResp->colp_response_valid) {
        data->errNo = RIL_E_GENERIC_FAILURE;
      }
      suppSvc->setFacilityType(qcril::interfaces::FacilityType::COLP);
      if (qmiResp->colp_response_valid) {
        QCRIL_LOG_INFO("COLP RESP with status %d and provisionStatus %d",
                       (int)qmiResp->colp_response.active_status,
                       (int)qmiResp->colp_response.provision_status);
        suppSvc->setStatus(mapActiveStatus(qmiResp->colp_response.active_status));
        suppSvc->setProvisionStatus(mapProvisionStatus(qmiResp->colp_response.provision_status));
      }

      if (qmiResp->sip_error_code_valid || qmiResp->failure_cause_description_valid) {
        auto errorDetails = buildSipErrorInfo(
            qmiResp->sip_error_code_valid, qmiResp->sip_error_code,
            qmiResp->failure_cause_description_valid ? qmiResp->failure_cause_description_len : 0,
            (uint16_t *)&(qmiResp->failure_cause_description));
        suppSvc->setErrorDetails(errorDetails);
      }
    }
    auto pendingMsg = getPendingMessageList().extract(data->req_id);
    if (pendingMsg) {
      auto respPayload = std::make_shared<QcRilRequestMessageCallbackPayload>(data->errNo, suppSvc);
      auto msg(std::static_pointer_cast<QcRilRequestMessage>(pendingMsg));
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

void VoiceModule::processSetSuppSvcNotificationResponse(CommonVoiceResponseData *data) {
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

void VoiceModule::processSetCallBarringPasswordResponse(CommonVoiceResponseData *data) {
  QCRIL_LOG_FUNC_ENTRY();
  voice_set_call_barring_password_resp_msg_v02 *qmiResp = nullptr;
  if (data) {
    qmiResp = static_cast<voice_set_call_barring_password_resp_msg_v02 *>(data->respData);
    if (qmiResp) {
      if (data->errNo != RIL_E_SUCCESS) {
        QCRIL_LOG_INFO("QCRIL QMI VOICE Change Call barring pwd: FAILURE");
        if (qmiResp->failure_cause_valid == TRUE) {
          qcril_qmi_send_ss_failure_cause_oem_hook_unsol_resp(
              qmiResp->failure_cause,
              (qmiResp->call_id_valid ? qmiResp->call_id : VOICE_INVALID_CALL_ID));
        }
      }
      if ((data->errNo == RIL_E_MODEM_ERR) && (qmiResp->failure_cause_valid) &&
          (qmiResp->failure_cause == QMI_FAILURE_CAUSE_NEGATIVE_PWD_CHECK_V02)) {
        data->errNo = RIL_E_PASSWORD_INCORRECT;
      }
    }
    auto pendingMsg = getPendingMessageList().extract(data->req_id);
    if (pendingMsg) {
      auto respPayload = std::make_shared<QcRilRequestMessageCallbackPayload>(data->errNo, nullptr);
      auto msg(std::static_pointer_cast<QcRilRequestMessage>(pendingMsg));
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

void VoiceModule::processQueryCallForwardResponse(CommonVoiceResponseData *data) {
  QCRIL_LOG_FUNC_ENTRY();
  std::shared_ptr<qcril::interfaces::GetCallForwardRespData> cfRespData = nullptr;
  RIL_CallForwardInfo respBuffer[QCRIL_QMI_VOICE_CFW_RESPONSE_BUF_SZ] = {};
  voice_get_call_forwarding_resp_msg_v02 *qmiResp = nullptr;
  if (data) {
    auto pendingMsg = getPendingMessageList().extract(data->req_id);
    auto msg(std::static_pointer_cast<QcRilRequestQueryCallForwardMessage>(pendingMsg));
    qmiResp = static_cast<voice_get_call_forwarding_resp_msg_v02 *>(data->respData);
    cfRespData = std::make_shared<qcril::interfaces::GetCallForwardRespData>();
    if (qmiResp && msg && cfRespData) {
      if (data->errNo == RIL_E_SUCCESS && qmiResp->get_call_forwarding_info_valid) {
        uint32_t numInstances = qcril_qmi_voice_convert_qmi_to_ril_call_forwarding_info(
            msg->getReason(), qmiResp->get_call_forwarding_info_len,
            qmiResp->get_call_forwarding_info, QCRIL_QMI_VOICE_CFW_RESPONSE_BUF_SZ, respBuffer);
        QCRIL_LOG_INFO("num_of_instances = %u\n", numInstances);
        if (numInstances > 0) {
          for (uint32_t i = 0; i < numInstances; i++) {
            qcril::interfaces::CallForwardInfo cfInfo = {};
            cfInfo.setStatus(respBuffer[i].status);
            cfInfo.setReason(respBuffer[i].reason);
            cfInfo.setServiceClass(respBuffer[i].serviceClass);
            cfInfo.setToa(respBuffer[i].toa);
            if (respBuffer[i].number) {
              cfInfo.setNumber(respBuffer[i].number);
              qcril_free(respBuffer[i].number);
            }
            cfInfo.setTimeSeconds(respBuffer[i].timeSeconds);

            if (cfInfo.getReason() == QCRIL_QMI_VOICE_CCFC_REASON_UNCOND) {
              if (qmiResp->call_fwd_start_time_valid) {
                std::shared_ptr<qcril::interfaces::CallFwdTimerInfo> cftStart =
                    std::make_shared<qcril::interfaces::CallFwdTimerInfo>();
                bool ret = qcril_qmi_voice_translate_voice_time_type_to_ril_callfwdtimerinfo(
                    cftStart, qmiResp->call_fwd_start_time);
                if (ret) {
                  cfInfo.setCallFwdTimerStart(cftStart);
                }
              }
              if (qmiResp->call_fwd_end_time_valid) {
                std::shared_ptr<qcril::interfaces::CallFwdTimerInfo> cftEnd =
                    std::make_shared<qcril::interfaces::CallFwdTimerInfo>();
                bool ret = qcril_qmi_voice_translate_voice_time_type_to_ril_callfwdtimerinfo(
                    cftEnd, qmiResp->call_fwd_end_time);
                if (ret) {
                  cfInfo.setCallFwdTimerEnd(cftEnd);
                }
              }
            }
            cfRespData->addToCallForwardInfo(cfInfo);
          }
        }
      } else {
        if (data->errNo != RIL_E_SUCCESS) {
          if (qmiResp->failure_cause_valid == TRUE) {
            qcril_qmi_send_ss_failure_cause_oem_hook_unsol_resp(
                qmiResp->failure_cause,
                (qmiResp->call_id_valid ? qmiResp->call_id : VOICE_INVALID_CALL_ID));
          }
        }
        if (qmiResp->sip_error_code_valid || qmiResp->failure_cause_description_valid) {
          auto errorDetails = buildSipErrorInfo(
              qmiResp->sip_error_code_valid, qmiResp->sip_error_code,
              qmiResp->failure_cause_description_valid ? qmiResp->failure_cause_description_len : 0,
              (uint16_t *)&(qmiResp->failure_cause_description));
          cfRespData->setErrorDetails(errorDetails);
        }
      }
    }

    if (msg) {
      auto respPayload =
          std::make_shared<QcRilRequestMessageCallbackPayload>(data->errNo, cfRespData);
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

void VoiceModule::processSetLocalCallHoldResponse(CommonVoiceResponseData *data) {
  QCRIL_LOG_FUNC_ENTRY();
  if (data) {
    auto pendingMsg = getPendingMessageList().find(data->req_id);
    if (pendingMsg) {
      auto msg(std::static_pointer_cast<QcRilRequestSetLocalCallHoldMessage>(pendingMsg));
      msg->setPendingMessageState((data->errNo == RIL_E_SUCCESS
                                       ? PendingMessageState::COMPLETED_SUCCESS
                                       : PendingMessageState::COMPLETED_FAILURE));

      // Send response if the request is for LOCAL_UNHOLD
      // otherwise (for LOCAL_HOLD case), send response only after getting the confirmation
      // from audio manager.
      if ((msg->getLocalHoldType() == QcRilRequestSetLocalCallHoldMessage::LocalHoldType::UNHOLD) ||
          msg->isAudioStateUpdateCompleted()) {
        getPendingMessageList().erase(data->req_id);
        auto respPayload =
            std::make_shared<QcRilRequestMessageCallbackPayload>(data->errNo, nullptr);
        msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
      }
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

void VoiceModule::handleQcRilRequestCsDialMessage(std::shared_ptr<QcRilRequestCsDialMessage> msg) {
  QCRIL_LOG_FUNC_ENTRY("msg = %s", (msg ? msg->dump().c_str() : "nullptr"));

  RIL_Errno rilErr = RIL_E_SUCCESS;
  voice_dial_call_req_msg_v02 dial_call_req = {};
  qcril_qmi_voice_voip_call_info_elaboration_type elaboration =
      QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_CS_DOMAIN;
  auto pendingMsgStatus = std::make_pair(0, false);

  if (msg != nullptr) {
    do {
      if (!msg->hasAddress() || msg->getAddress().empty()) {
        QCRIL_LOG_ERROR("Invalid parameters: Address");
        rilErr = RIL_E_INVALID_ARGUMENTS;
        break;
      }

      string subAddress = "";
      string callingNumber = "";
      string sipUriOverflow = "";
      rilErr = convert_dial_call_req_number(msg->getAddress(), callingNumber, sipUriOverflow,
                                            subAddress);
      if (rilErr != RIL_E_SUCCESS || !sipUriOverflow.empty()) {
        rilErr = RIL_E_INVALID_ARGUMENTS;
        QCRIL_LOG_ERROR("Invalid number");
        break;
      }
      if (!subAddress.empty()) {
        dial_call_req.called_party_subaddress_valid = TRUE;
        // Always set to 1 according to spec Table 10.5.119/3GPP TS 24.008
        dial_call_req.called_party_subaddress.extension_bit = 1;
        dial_call_req.called_party_subaddress.subaddress_type = SUBADDRESS_TYPE_NSAP_V02;
        dial_call_req.called_party_subaddress.odd_even_ind = subAddress.size() % 2;
        memcpy(
            dial_call_req.called_party_subaddress.subaddress, subAddress.c_str(),
            std::min(sizeof(dial_call_req.called_party_subaddress.subaddress), subAddress.size()));
        dial_call_req.called_party_subaddress.subaddress_len = subAddress.size();
      }
      if (!callingNumber.empty()) {
        memcpy(dial_call_req.calling_number, callingNumber.c_str(),
               std::min(sizeof(dial_call_req.calling_number), callingNumber.size()));
      }

      if (msg->hasClir()) {
        dial_call_req.clir_type_valid =
            convert_dial_call_req_clir_type(msg->getClir(), dial_call_req.clir_type);
      }
      if (msg->hasUusInfo() && msg->getUusInfo() != nullptr) {
        dial_call_req.uus_valid =
            convert_dial_call_req_uss(*(msg->getUusInfo()), dial_call_req.uus);
      }

      dial_call_req.call_type_valid = TRUE;
      dial_call_req.call_type = CALL_TYPE_VOICE_V02;
      dial_call_req.service_type_valid = TRUE;
      dial_call_req.service_type =
          qcril_qmi_nas_setting_srv_type_based_on_elaboration_and_rat(elaboration);

      pendingMsgStatus = getPendingMessageList().insert(msg);
      if (pendingMsgStatus.second != true) {
        QCRIL_LOG_ERROR("getPendingMessageList().insert failed!!");
        break;
      }
      uint16_t req_id = pendingMsgStatus.first;

      rilErr = qcril_qmi_voice_process_dial_call_req(
          req_id, dial_call_req,
          std::bind(&VoiceModule::processDialCallResponse, this, std::placeholders::_1),
          std::bind(&VoiceModule::processDialCommandOversightCompletion, this,
                    std::placeholders::_1),
          elaboration);
      if (rilErr == RIL_E_SUCCESS) {
        qcril_qmi_voice_enable_voice_indications(TRUE);
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

void VoiceModule::handleQcRilRequestQueryCallForwardMessage(
    std::shared_ptr<QcRilRequestQueryCallForwardMessage> msg) {
  QCRIL_LOG_FUNC_ENTRY("msg = %s", (msg ? msg->dump().c_str() : "nullptr"));

  RIL_Errno rilErr = RIL_E_SUCCESS;
  voice_get_call_forwarding_req_msg_v02 get_cf_req = {};
  auto pendingMsgStatus = std::make_pair(0, false);

  if (msg != nullptr) {
    do {
      bool isValid = msg->hasReason();
      if (msg->hasReason()) {
        isValid =
            qcril_qmi_voice_map_ril_reason_to_qmi_cfw_reason(get_cf_req.reason, msg->getReason());
      }
      if (!isValid) {
        QCRIL_LOG_ERROR("Invalid parameter: reason");
        rilErr = RIL_E_INVALID_ARGUMENTS;
        break;
      }

      if (msg->hasServiceClass() && msg->getServiceClass() > 0 && msg->getServiceClass() <= 0xFF) {
        get_cf_req.service_class_valid = TRUE;
        get_cf_req.service_class = msg->getServiceClass();
      } else {
        QCRIL_LOG_ERROR("Invalid parameter: ServiceClass 0x%X", msg->getServiceClass());
      }

      pendingMsgStatus = getPendingMessageList().insert(msg);
      if (pendingMsgStatus.second != true) {
        QCRIL_LOG_ERROR("getPendingMessageList().insert failed!!");
        rilErr = RIL_E_GENERIC_FAILURE;
        break;
      }
      uint16_t req_id = pendingMsgStatus.first;

      rilErr = qcril_qmi_voice_process_get_call_forwarding_req(
          req_id, get_cf_req,
          std::bind(&VoiceModule::processQueryCallForwardResponse, this, std::placeholders::_1));
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

void VoiceModule::handleQcRilRequestSetCallForwardMessage(
    std::shared_ptr<QcRilRequestSetCallForwardMessage> msg) {
  QCRIL_LOG_FUNC_ENTRY("msg = %s", (msg ? msg->dump().c_str() : "nullptr"));

  RIL_Errno rilErr = RIL_E_SUCCESS;
  auto pendingMsgStatus = std::make_pair(0, false);

  if (msg != nullptr) {
    do {
      bool isValid = msg->hasStatus();
      voice_service_enum_v02 voice_service = VOICE_SERVICE_ACTIVATE_V02; // dummy init
      if (msg->hasStatus()) {
        isValid = qcril_qmi_voice_map_ril_status_to_qmi_cfw_status(voice_service, msg->getStatus());
      }
      if (!isValid) {
        QCRIL_LOG_ERROR("Invalid parameter: status");
        rilErr = RIL_E_INVALID_ARGUMENTS;
        break;
      }
      voice_reason_enum_v02 reason = VOICE_REASON_FWD_UNCONDITIONAL_V02; // dummy init
      isValid = msg->hasReason();
      if (msg->hasReason()) {
        isValid = qcril_qmi_voice_map_ril_reason_to_qmi_cfw_reason(reason, msg->getReason());
      }
      if (!isValid) {
        QCRIL_LOG_ERROR("Invalid parameter: reason");
        rilErr = RIL_E_INVALID_ARGUMENTS;
        break;
      }

      uint8_t number_valid = FALSE;
      std::string number;
      uint8_t timer_value_valid = FALSE;
      uint8_t timer_value = 0;
      if ((msg->getStatus() == QCRIL_QMI_VOICE_MODE_REG)) {
        if (msg->hasNumber() && (msg->getNumber().size() > 0) &&
            msg->getNumber().size() <= QMI_VOICE_NUMBER_MAX_V02) {
          number = msg->getNumber();
          if (msg->hasToa()) {
            if (msg->getToa() == 145 && number[0] != '+') {
              number = "+" + number;
            }
          }
          number_valid = TRUE;
        }
        if (msg->hasReason() &&
            (msg->getReason() == QCRIL_QMI_VOICE_CCFC_REASON_NOREPLY ||
             msg->getReason() == QCRIL_QMI_VOICE_CCFC_REASON_ALLCALL ||
             msg->getReason() == QCRIL_QMI_VOICE_CCFC_REASON_ALLCOND) &&
            msg->hasTimeSeconds() && msg->getTimeSeconds() > 0) {
          timer_value_valid = TRUE;
          timer_value = msg->getTimeSeconds();
        }
      }

      uint8_t call_fwd_start_time_valid = FALSE;
      voice_time_type_v02 call_fwd_start_time = {};

      uint8_t call_fwd_end_time_valid = FALSE;
      voice_time_type_v02 call_fwd_end_time = {};



      if (msg->hasCallFwdTimerStart() && (msg->getCallFwdTimerStart() != nullptr)) {
        QCRIL_LOG_ERROR("msg->getCallFwdTimerStart: %s",
            qcril::interfaces::toString(*msg->getCallFwdTimerStart()).c_str());
        call_fwd_start_time_valid = TRUE;
        qcril_qmi_voice_translate_ril_callfwdtimerinfo_to_voice_time_type(
            call_fwd_start_time, msg->getCallFwdTimerStart());
      }
      if (msg->hasCallFwdTimerEnd() && (msg->getCallFwdTimerEnd() != nullptr)) {
        QCRIL_LOG_ERROR("msg->getCallFwdTimerEnd: %s",
            qcril::interfaces::toString(*msg->getCallFwdTimerEnd()).c_str());
        call_fwd_end_time_valid = TRUE;
        qcril_qmi_voice_translate_ril_callfwdtimerinfo_to_voice_time_type(
            call_fwd_end_time, msg->getCallFwdTimerEnd());
      }

      pendingMsgStatus = getPendingMessageList().insert(msg);
      if (pendingMsgStatus.second != true) {
        QCRIL_LOG_ERROR("getPendingMessageList().insert failed!!");
        rilErr = RIL_E_GENERIC_FAILURE;
        break;
      }
      uint16_t req_id = pendingMsgStatus.first;

      if (mSetAllCallForwardSupported) {
        if (reason == VOICE_REASON_FWD_ALLFORWARDING_V02 ||
            reason == VOICE_REASON_FWD_ALLCONDITIONAL_V02) {
          // Use SET_ALL_CALL_FORWARD
          voice_set_all_call_fwd_sups_req_msg_v02 set_all_call_fwd_req = {
              .voice_service = voice_service,
              .reason_valid = TRUE,
              .reason = reason,
              .timer_value_valid = timer_value_valid,
              .timer_value = timer_value,
              .call_fwd_start_time_valid = call_fwd_start_time_valid,
              .call_fwd_start_time = call_fwd_start_time,
              .call_fwd_end_time_valid = call_fwd_end_time_valid,
              .call_fwd_end_time = call_fwd_end_time};

          if (msg->hasServiceClass() && msg->getServiceClass() > 0) {
            set_all_call_fwd_req.service_class_valid = TRUE;
            set_all_call_fwd_req.service_class = (voice_service_class_enum_v02)(msg->getServiceClass());
          }
          set_all_call_fwd_req.number_valid = number_valid;
          if (number_valid) {
            memcpy(set_all_call_fwd_req.number, number.c_str(), number.size());
          }

          rilErr = qcril_qmi_voice_process_set_all_call_fwd_sups_req(
              req_id, set_all_call_fwd_req,
              std::bind(&VoiceModule::processSetAllCallFwdSupsReqResponse, this,
                        std::placeholders::_1));
          break;
        }
      }

      // Use SET_SUPS_SERVICE_REQ
      voice_set_sups_service_req_msg_v02 set_sups_req = {
              .supplementary_service_info.voice_service = voice_service,
              .supplementary_service_info.reason = reason,
              .timer_value_valid = timer_value_valid,
              .timer_value = timer_value};

      if (msg->hasServiceClass() && msg->getServiceClass() > 0) {
        set_sups_req.service_class_valid = TRUE;
        set_sups_req.service_class = msg->getServiceClass();
      }

      set_sups_req.number_valid = number_valid;
      if (number_valid) {
        memcpy(set_sups_req.number, number.c_str(), number.size());
      }

      if (msg->hasReason() && (msg->getReason() == QCRIL_QMI_VOICE_CCFC_REASON_UNCOND)) {
        set_sups_req.call_fwd_start_time_valid = call_fwd_start_time_valid;
        set_sups_req.call_fwd_start_time = call_fwd_start_time;

        set_sups_req.call_fwd_end_time_valid = call_fwd_end_time_valid;
        set_sups_req.call_fwd_end_time = call_fwd_end_time;
      }

      rilErr = qcril_qmi_voice_process_set_sups_service_req(
          req_id, set_sups_req,
          std::bind(&VoiceModule::processSetCallFwdSupsReqResponse, this, std::placeholders::_1));
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

void VoiceModule::handleQcRilRequestSetSupsServiceMessage(
    std::shared_ptr<QcRilRequestSetSupsServiceMessage> msg) {
  QCRIL_LOG_FUNC_ENTRY("msg = %s", (msg ? msg->dump().c_str() : "nullptr"));

  RIL_Errno rilErr = RIL_E_SUCCESS;
  voice_set_sups_service_req_msg_v02 set_sups_req = {};
  auto pendingMsgStatus = std::make_pair(0, false);

  if (msg != nullptr) {
    do {
      bool isValid = msg->hasOperationType();
      if (msg->hasOperationType()) {
        switch (msg->getOperationType()) {
          case 1:  // ACTIVATE
            set_sups_req.supplementary_service_info.voice_service = VOICE_SERVICE_ACTIVATE_V02;
            break;
          case 2:  // DEACTIVATE
            set_sups_req.supplementary_service_info.voice_service = VOICE_SERVICE_DEACTIVATE_V02;
            break;
          case 4:  // REGISTER
            set_sups_req.supplementary_service_info.voice_service = VOICE_SERVICE_REGISTER_V02;
            break;
          case 5:  // ERASURE
            set_sups_req.supplementary_service_info.voice_service = VOICE_SERVICE_ERASE_V02;
            break;
          default:
            isValid = false;
            rilErr = RIL_E_INVALID_ARGUMENTS;
            break;
        }
      }
      if (!isValid) {
        QCRIL_LOG_ERROR("Invalid parameter: OperationType");
        rilErr = RIL_E_INVALID_ARGUMENTS;
        break;
      }
      isValid = msg->hasFacilityType();
      if (msg->hasFacilityType()) {
        isValid =
            mapFacilityType(set_sups_req.supplementary_service_info.reason, msg->getFacilityType());
      }
      if (!isValid) {
        QCRIL_LOG_ERROR("Invalid parameter: FacilityType");
        rilErr = RIL_E_INVALID_ARGUMENTS;
        break;
      }

      if (msg->hasServiceClass() && msg->getServiceClass() > 0) {
        set_sups_req.service_class_valid = TRUE;
        set_sups_req.service_class = msg->getServiceClass();
      }

      if (msg->hasPassword() && msg->getPassword().size()) {
        set_sups_req.password_valid = TRUE;
        memcpy(set_sups_req.password, msg->getPassword().c_str(),
               std::min(sizeof(set_sups_req.password), msg->getPassword().size()));
      }

      if (msg->hasCallBarringNumberList() && msg->getCallBarringNumberList().size() > 0) {
        set_sups_req.call_barring_numbers_list_valid = TRUE;
        for (uint32_t i = 0; ((i < QMI_VOICE_MAX_BARRED_NUMBERS_LIST_V02) &&
                              (i < msg->getCallBarringNumberList().size()));
             i++) {
          strlcpy(set_sups_req.call_barring_numbers_list[i].barred_number,
                  msg->getCallBarringNumberList()[i].c_str(),
                  sizeof(set_sups_req.call_barring_numbers_list[i].barred_number));
          set_sups_req.call_barring_numbers_list_len++;
        }
      }

      pendingMsgStatus = getPendingMessageList().insert(msg);
      if (pendingMsgStatus.second != true) {
        QCRIL_LOG_ERROR("getPendingMessageList().insert failed!!");
        rilErr = RIL_E_GENERIC_FAILURE;
        break;
      }
      uint16_t req_id = pendingMsgStatus.first;

      rilErr = qcril_qmi_voice_process_set_sups_service_req(
          req_id, set_sups_req,
          std::bind(&VoiceModule::processSetSupsServiceResponse, this, std::placeholders::_1));
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

void VoiceModule::handleQcRilRequestQueryColpMessage(
    std::shared_ptr<QcRilRequestQueryColpMessage> msg) {
  QCRIL_LOG_FUNC_ENTRY("msg = %s", (msg ? msg->dump().c_str() : "nullptr"));

  RIL_Errno rilErr = RIL_E_SUCCESS;
  auto pendingMsgStatus = std::make_pair(0, false);

  if (msg != nullptr) {
    do {
      voice_get_colp_req_msg_v02 colp_req = {};
      pendingMsgStatus = getPendingMessageList().insert(msg);
      if (pendingMsgStatus.second != true) {
        QCRIL_LOG_ERROR("getPendingMessageList().insert failed!!");
        rilErr = RIL_E_GENERIC_FAILURE;
        break;
      }
      uint16_t req_id = pendingMsgStatus.first;

      rilErr = qcril_qmi_voice_process_get_colp_req(
          req_id, colp_req,
          std::bind(&VoiceModule::processGetColpResponse, this, std::placeholders::_1));
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

void VoiceModule::handleQcRilRequestGetCallBarringMessage(
    std::shared_ptr<QcRilRequestGetCallBarringMessage> msg) {
  QCRIL_LOG_FUNC_ENTRY("msg = %s", (msg ? msg->dump().c_str() : "nullptr"));

  RIL_Errno rilErr = RIL_E_SUCCESS;
  voice_get_call_barring_req_msg_v02 get_cb_req = {};
  auto pendingMsgStatus = std::make_pair(0, false);

  if (msg != nullptr) {
    do {
      bool isValid = msg->hasFacilityType();
      if (msg->hasFacilityType()) {
        isValid =
            mapFacilityType(get_cb_req.reason, msg->getFacilityType());
      }
      if (!isValid) {
        QCRIL_LOG_ERROR("Invalid parameter: FacilityType");
        rilErr = RIL_E_INVALID_ARGUMENTS;
        break;
      }
      if (msg->hasServiceClass() && msg->getServiceClass() > 0 && msg->getServiceClass() <= 0xFF) {
        get_cb_req.service_class_valid = TRUE;
        get_cb_req.service_class = msg->getServiceClass();
      } else {
        QCRIL_LOG_ERROR("Invalid parameter: ServiceClass 0x%X", msg->getServiceClass());
      }

      pendingMsgStatus = getPendingMessageList().insert(msg);
      if (pendingMsgStatus.second != true) {
        QCRIL_LOG_ERROR("getPendingMessageList().insert failed!!");
        rilErr = RIL_E_GENERIC_FAILURE;
        break;
      }
      uint16_t req_id = pendingMsgStatus.first;

      rilErr = qcril_qmi_voice_process_get_call_barring_req(
          req_id, get_cb_req,
          std::bind(&VoiceModule::processGetCallBarringResponse, this, std::placeholders::_1));
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

void VoiceModule::handleQcRilRequestSetCallBarringPasswordMessage(
    std::shared_ptr<QcRilRequestSetCallBarringPasswordMessage> msg) {
  QCRIL_LOG_FUNC_ENTRY("msg = %s", (msg ? msg->dump().c_str() : "nullptr"));

  RIL_Errno rilErr = RIL_E_SUCCESS;
  voice_set_call_barring_password_req_msg_v02 set_cb_pwd_req = {};
  auto pendingMsgStatus = std::make_pair(0, false);

  if (msg != nullptr) {
    do {
      bool isValid = msg->hasFacilityType();
      if (msg->hasFacilityType()) {
        isValid = mapFacilityType(set_cb_pwd_req.call_barring_password_info.reason,
                                  msg->getFacilityType());
      }
      if (!isValid) {
        QCRIL_LOG_ERROR("Invalid parameter: FacilityType");
        rilErr = RIL_E_INVALID_ARGUMENTS;
        break;
      }
      if (msg->hasOldPassword() && !msg->getOldPassword().empty()) {
        memcpy(set_cb_pwd_req.call_barring_password_info.old_password,
               msg->getOldPassword().c_str(),
               std::min(sizeof(set_cb_pwd_req.call_barring_password_info.old_password),
                        msg->getOldPassword().size()));
      }

      if (msg->hasNewPassword() && !msg->getNewPassword().empty()) {
        memcpy(set_cb_pwd_req.call_barring_password_info.new_password,
               msg->getNewPassword().c_str(),
               std::min(sizeof(set_cb_pwd_req.call_barring_password_info.new_password),
                        msg->getNewPassword().size()));

        memcpy(set_cb_pwd_req.call_barring_password_info.new_password_again,
               msg->getNewPassword().c_str(),
               std::min(sizeof(set_cb_pwd_req.call_barring_password_info.new_password_again),
                        msg->getNewPassword().size()));
      }

      pendingMsgStatus = getPendingMessageList().insert(msg);
      if (pendingMsgStatus.second != true) {
        QCRIL_LOG_ERROR("getPendingMessageList().insert failed!!");
        rilErr = RIL_E_GENERIC_FAILURE;
        break;
      }
      uint16_t req_id = pendingMsgStatus.first;

      rilErr = qcril_qmi_voice_process_set_call_barring_password_req(
          req_id, set_cb_pwd_req,
          std::bind(&VoiceModule::processSetCallBarringPasswordResponse, this, std::placeholders::_1));
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

void VoiceModule::handleQcRilRequestSetSuppSvcNotificationMessage(
    std::shared_ptr<QcRilRequestSetSuppSvcNotificationMessage> msg) {
  QCRIL_LOG_FUNC_ENTRY("msg = %s", (msg ? msg->dump().c_str() : "nullptr"));

  RIL_Errno rilErr = RIL_E_SUCCESS;
  voice_indication_register_req_msg_v02 indication_req = {};
  auto pendingMsgStatus = std::make_pair(0, false);

  if (msg != nullptr) {
    do {
      if (msg->hasStatus()) {
        indication_req.supps_notification_events_valid = TRUE;
        if (msg->getStatus()) {
          indication_req.supps_notification_events = 0x01;
        } else {
          indication_req.supps_notification_events = 0x00;
        }
      } else {
        QCRIL_LOG_ERROR("Invalid parameter: Status");
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

      rilErr = qcril_qmi_voice_process_indication_register_req(
          req_id, indication_req,
          std::bind(&VoiceModule::processSetSuppSvcNotificationResponse, this, std::placeholders::_1));
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

void VoiceModule::handleQcRilRequestCsAnswerMessage(std::shared_ptr<QcRilRequestCsAnswerMessage> msg) {
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

      pendingMsgStatus = getPendingMessageList().insert(msg);
      if (pendingMsgStatus.second != true) {
        QCRIL_LOG_ERROR("getPendingMessageList().insert failed!!");
        rilErr = RIL_E_GENERIC_FAILURE;
        break;
      }
      uint16_t req_id = pendingMsgStatus.first;

      rilErr = qcril_qmi_voice_process_answer_call_req(
          req_id, ans_call_req,
          std::bind(&VoiceModule::processAnswerResponse, this, std::placeholders::_1),
          std::bind(&VoiceModule::processAnswerCommandOversightCompletion, this,
                    std::placeholders::_1));
      if (rilErr == RIL_E_SUCCESS) {
        qcril_am_handle_event(QCRIL_AM_EVENT_VOICE_ANSWER, NULL);
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

void VoiceModule::handleQcRilRequestCsHangupMessage(std::shared_ptr<QcRilRequestCsHangupMessage> msg) {
  QCRIL_LOG_FUNC_ENTRY("msg = %s", (msg ? msg->dump().c_str() : "nullptr"));

  RIL_Errno rilErr = RIL_E_SUCCESS;
  voice_end_call_req_msg_v02 end_call_req = {};
  qcril_qmi_voice_voip_call_info_entry_type *call_info_entry = nullptr;
  int conn_index = 0;
  auto pendingMsgStatus = std::make_pair(0, false);

  if (msg != nullptr) {
    do {
      pendingMsgStatus = getPendingMessageList().insert(msg);
      if (pendingMsgStatus.second != true) {
        QCRIL_LOG_ERROR("getPendingMessageList().insert failed!!");
        break;
      }
      uint16_t req_id = pendingMsgStatus.first;

      if (!msg->hasCallIndex()) {
        QCRIL_LOG_ERROR("Invalid parameter: status");
        rilErr = RIL_E_INVALID_ARGUMENTS;
        break;
      }

      conn_index = msg->getCallIndex();
      QCRIL_LOG_INFO("conn_index: %d", conn_index);

      if (qcril_qmi_voice_cancel_pending_call(true)) {
        QCRIL_LOG_INFO("Cancelled a pending call request; send failure response");
        rilErr = RIL_E_GENERIC_FAILURE;
        break;
      }

      if (conn_index == -1) {
        if (RIL_E_SUCCESS != qcril_qmi_process_hangup_on_call_being_setup(&conn_index)) {
          /* Could not retrieve call id */
          rilErr = RIL_E_INVALID_ARGUMENTS;
          break;
        } else if (conn_index == -1) {
          /* Have not received call id, hence we will wait to receive call id. */
          // The msg stored in getPendingMessageList() will be poped and re-evalue once the call id
          // is available.
          rilErr = RIL_E_SUCCESS;
          break;
        }
      }

      // find call info
      call_info_entry = qcril_qmi_voice_voip_find_call_info_entry_by_call_android_id(conn_index);
      if (!call_info_entry) {
        QCRIL_LOG_ERROR("Failed to find call entry, aborting!");
        rilErr = RIL_E_INVALID_ARGUMENTS;
        break;
      }

      end_call_req.call_id = call_info_entry->qmi_call_id;

      rilErr = qcril_qmi_voice_process_end_call_req(
          req_id, end_call_req,
          std::bind(&VoiceModule::processHangupResponse, this, std::placeholders::_1),
          std::bind(&VoiceModule::processHangupCommandOversightCompletion, this,
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

void VoiceModule::handleQcRilRequestGetCurrentCallsMessage(
    std::shared_ptr<QcRilRequestGetCurrentCallsMessage> msg) {
  QCRIL_LOG_FUNC_ENTRY("msg = %s", (msg ? msg->dump().c_str() : "nullptr"));

  qcril_qmi_voice_request_get_current_atel_calls(msg);

  QCRIL_LOG_FUNC_RETURN();
}

void VoiceModule::handleQcRilRequestLastCallFailCauseMessage(
    std::shared_ptr<QcRilRequestLastCallFailCauseMessage> msg) {
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
      qcril_qmi_voice_request_last_call_fail_cause();
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

void VoiceModule::handleQcRilRequestSendDtmfMessage(
    std::shared_ptr<QcRilRequestSendDtmfMessage> msg) {
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
          std::bind(&VoiceModule::processStartContDtmfResponse, this, std::placeholders::_1));
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

void VoiceModule::handleQcRilRequestStartDtmfMessage(
    std::shared_ptr<QcRilRequestStartDtmfMessage> msg) {
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
          std::bind(&VoiceModule::processStartContDtmfResponse, this, std::placeholders::_1));
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

void VoiceModule::handleQcRilRequestStopDtmfMessage(
    std::shared_ptr<QcRilRequestStopDtmfMessage> msg) {
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
          std::bind(&VoiceModule::processStopContDtmfResponse, this, std::placeholders::_1));
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

void VoiceModule::handleQcRilRequestCdmaFlashMessage(
    std::shared_ptr<QcRilRequestCdmaFlashMessage> msg) {
  QCRIL_LOG_FUNC_ENTRY("msg = %s", (msg ? msg->dump().c_str() : "nullptr"));

  RIL_Errno rilErr = RIL_E_SUCCESS;
  voice_send_flash_req_msg_v02 send_flash_req = {.call_id = QCRIL_QMI_VOICE_UNKNOWN_ACTIVE_CONN_ID};
  auto pendingMsgStatus = std::make_pair(0, false);

  if (msg != nullptr) {
    do {
      send_flash_req.flash_payload_valid = TRUE;
      if (msg->hasFeatureCode() && !msg->getFeatureCode().empty() &&
          (msg->getFeatureCode().size() < sizeof(send_flash_req.flash_payload))) {
        strlcpy(send_flash_req.flash_payload, msg->getFeatureCode().c_str(),
                sizeof(send_flash_req.flash_payload));
      }

      pendingMsgStatus = getPendingMessageList().insert(msg);
      if (pendingMsgStatus.second != true) {
        QCRIL_LOG_ERROR("getPendingMessageList().insert failed!!");
        break;
      }
      uint16_t req_id = pendingMsgStatus.first;

      rilErr = qcril_qmi_voice_process_send_flash_req(
          req_id, send_flash_req,
          std::bind(&VoiceModule::processSendFlashResponse, this, std::placeholders::_1));
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

void VoiceModule::handleQcRilRequestCdmaBurstDtmfMessage(
    std::shared_ptr<QcRilRequestCdmaBurstDtmfMessage> msg) {
  QCRIL_LOG_FUNC_ENTRY("msg = %s", (msg ? msg->dump().c_str() : "nullptr"));

  RIL_Errno rilErr = RIL_E_SUCCESS;
  voice_burst_dtmf_req_msg_v02 burst_dtmf_req = {};
  auto pendingMsgStatus = std::make_pair(0, false);

  if (msg != nullptr) {
    do {
      if (!msg->hasDigitBuffer() || msg->getDigitBuffer().empty() ||
          (msg->getDigitBuffer().size() > QMI_VOICE_DIGIT_BUFFER_MAX_V02)) {
        QCRIL_LOG_ERROR("Invalid parameter: DigitBuffer");
        rilErr = RIL_E_INVALID_ARGUMENTS;
        break;
      }
      if (!msg->hasDtmfOnLength()) {
        QCRIL_LOG_ERROR("Invalid parameter: DtmfOnLength");
        rilErr = RIL_E_INVALID_ARGUMENTS;
        break;
      }
      if (!msg->hasDtmfOffLength()) {
        QCRIL_LOG_ERROR("Invalid parameter: DtmfOffLength");
        rilErr = RIL_E_INVALID_ARGUMENTS;
        break;
      }

      /* Modem determines the active call */
      burst_dtmf_req.burst_dtmf_info.call_id = QCRIL_QMI_VOICE_UNKNOWN_ACTIVE_CONN_ID;
      burst_dtmf_req.burst_dtmf_info.digit_buffer_len = msg->getDigitBuffer().size();
      strlcpy(burst_dtmf_req.burst_dtmf_info.digit_buffer, msg->getDigitBuffer().c_str(),
             sizeof(burst_dtmf_req.burst_dtmf_info.digit_buffer));

      burst_dtmf_req.dtmf_lengths_valid = TRUE;
      switch (msg->getDtmfOnLength()) {
        case 95:
          burst_dtmf_req.dtmf_lengths.dtmf_onlength = DTMF_ONLENGTH_95MS_V02;
          break;
          /* As per the Ril interface - 0 maps to default value */
        case 0:
        case 150:
          burst_dtmf_req.dtmf_lengths.dtmf_onlength = DTMF_ONLENGTH_150MS_V02;
          break;
        case 200:
          burst_dtmf_req.dtmf_lengths.dtmf_onlength = DTMF_ONLENGTH_200MS_V02;
          break;
        case 250:
          burst_dtmf_req.dtmf_lengths.dtmf_onlength = DTMF_ONLENGTH_250MS_V02;
          break;
        case 300:
          burst_dtmf_req.dtmf_lengths.dtmf_onlength = DTMF_ONLENGTH_300MS_V02;
          break;
        case 350:
          burst_dtmf_req.dtmf_lengths.dtmf_onlength = DTMF_ONLENGTH_350MS_V02;
          break;
        default:
          burst_dtmf_req.dtmf_lengths.dtmf_onlength = DTMF_ONLENGTH_SMS_V02;
          break;
      }
      switch (msg->getDtmfOffLength()) {
        case 60:
          burst_dtmf_req.dtmf_lengths.dtmf_offlength = DTMF_OFFLENGTH_60MS_V02;
          break;
        case 100:
          burst_dtmf_req.dtmf_lengths.dtmf_offlength = DTMF_OFFLENGTH_100MS_V02;
          break;
        case 150:
          burst_dtmf_req.dtmf_lengths.dtmf_offlength = DTMF_OFFLENGTH_150MS_V02;
          break;
        case 200:
          burst_dtmf_req.dtmf_lengths.dtmf_offlength = DTMF_OFFLENGTH_200MS_V02;
          break;
        default:
          burst_dtmf_req.dtmf_lengths.dtmf_offlength = DTMF_OFFLENGTH_150MS_V02;
          break;
      }

      pendingMsgStatus = getPendingMessageList().insert(msg);
      if (pendingMsgStatus.second != true) {
        QCRIL_LOG_ERROR("getPendingMessageList().insert failed!!");
        break;
      }
      uint16_t req_id = pendingMsgStatus.first;

      rilErr = qcril_qmi_voice_process_burst_dtmf_req(
          req_id, burst_dtmf_req,
          std::bind(&VoiceModule::processBurstDtmfResponse, this, std::placeholders::_1));
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

void VoiceModule::handleQcRilRequestSetTtyModeMessage(
    std::shared_ptr<QcRilRequestSetTtyModeMessage> msg) {
  QCRIL_LOG_FUNC_ENTRY("msg = %s", (msg ? msg->dump().c_str() : "nullptr"));

  RIL_Errno rilErr = RIL_E_SUCCESS;
  voice_set_config_req_msg_v02 set_config_req = {};
  auto pendingMsgStatus = std::make_pair(0, false);

  if (msg != nullptr) {
    do {
      if (!msg->hasTtyMode()) {
        QCRIL_LOG_ERROR("Invalid parameter: TtyMode");
        rilErr = RIL_E_INVALID_ARGUMENTS;
        break;
      }

      set_config_req.tty_mode_valid = TRUE;
      switch ( msg->getTtyMode())
      {
        case qcril::interfaces::TtyMode::FULL:
          set_config_req.tty_mode = TTY_MODE_FULL_V02;
          break;

        case qcril::interfaces::TtyMode::HCO:
          set_config_req.tty_mode = TTY_MODE_HCO_V02;
          break;

        case qcril::interfaces::TtyMode::VCO:
          set_config_req.tty_mode = TTY_MODE_VCO_V02;
          break;

        case qcril::interfaces::TtyMode::MODE_OFF: // fallthrough
        default:
          set_config_req.tty_mode = TTY_MODE_OFF_V02;
          break;
      }

      pendingMsgStatus = getPendingMessageList().insert(msg);
      if (pendingMsgStatus.second != true) {
        QCRIL_LOG_ERROR("getPendingMessageList().insert failed!!");
        break;
      }
      uint16_t req_id = pendingMsgStatus.first;

      rilErr = qcril_qmi_voice_process_set_config_req_sync(
          req_id, set_config_req,
          std::bind(&VoiceModule::processSetConfigResponse, this, std::placeholders::_1));
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

void VoiceModule::handleQcRilRequestGetTtyModeMessage(
    std::shared_ptr<QcRilRequestGetTtyModeMessage> msg) {
  QCRIL_LOG_FUNC_ENTRY("msg = %s", (msg ? msg->dump().c_str() : "nullptr"));

  RIL_Errno rilErr = RIL_E_SUCCESS;
  voice_get_config_req_msg_v02 get_config_req = {.tty_mode_valid = TRUE, .tty_mode = 0x01};
  auto pendingMsgStatus = std::make_pair(0, false);

  if (msg != nullptr) {
    do {
      pendingMsgStatus = getPendingMessageList().insert(msg);
      if (pendingMsgStatus.second != true) {
        QCRIL_LOG_ERROR("getPendingMessageList().insert failed!!");
        break;
      }
      uint16_t req_id = pendingMsgStatus.first;

      rilErr = qcril_qmi_voice_process_get_config_req(
          req_id, get_config_req,
          std::bind(&VoiceModule::processGetConfigResponse, this, std::placeholders::_1));
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

void VoiceModule::handleQcRilRequestGetCallWaitingMessage(
    std::shared_ptr<QcRilRequestGetCallWaitingMessage> msg) {
  QCRIL_LOG_FUNC_ENTRY("msg = %s", (msg ? msg->dump().c_str() : "nullptr"));

  voice_get_call_waiting_req_msg_v02 get_cw_req = {};
  RIL_Errno rilErr = RIL_E_SUCCESS;
  auto pendingMsgStatus = std::make_pair(0, false);

  if (msg != nullptr) {
    do {
      if (msg->hasServiceClass() && msg->getServiceClass() > 0) {
        get_cw_req.service_class_valid = TRUE;
        get_cw_req.service_class = msg->getServiceClass();
      }

      pendingMsgStatus = getPendingMessageList().insert(msg);
      if (pendingMsgStatus.second != true) {
        QCRIL_LOG_ERROR("getPendingMessageList().insert failed!!");
        rilErr = RIL_E_GENERIC_FAILURE;
        break;
      }
      uint16_t req_id = pendingMsgStatus.first;

      rilErr = qcril_qmi_voice_process_get_call_waiting_req(
          req_id, get_cw_req,
          std::bind(&VoiceModule::processGetCallWaitingResponse, this, std::placeholders::_1));
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

void VoiceModule::handleQcRilRequestSetCallWaitingMessage(
    std::shared_ptr<QcRilRequestSetCallWaitingMessage> msg) {
  QCRIL_LOG_FUNC_ENTRY("msg = %s", (msg ? msg->dump().c_str() : "nullptr"));

  RIL_Errno rilErr = RIL_E_SUCCESS;
  voice_set_sups_service_req_msg_v02 set_sups_req = {};
  auto pendingMsgStatus = std::make_pair(0, false);

  if (msg != nullptr) {
    do {
      bool isValid = msg->hasServiceStatus();
      if (msg->hasServiceStatus()) {
        switch (msg->getServiceStatus()) {
          case qcril::interfaces::ServiceClassStatus::DISABLED:
            set_sups_req.supplementary_service_info.voice_service = VOICE_SERVICE_DEACTIVATE_V02;
            isValid = true;
            break;
          case qcril::interfaces::ServiceClassStatus::ENABLED:
            set_sups_req.supplementary_service_info.voice_service = VOICE_SERVICE_ACTIVATE_V02;
            isValid = true;
            break;
          default:
            isValid = false;
            break;
        }
      }
      if (!isValid) {
        QCRIL_LOG_ERROR("Invalid parameter: status");
        rilErr = RIL_E_INVALID_ARGUMENTS;
        break;
      }

      set_sups_req.supplementary_service_info.reason = VOICE_REASON_CALLWAITING_V02;

      if (msg->hasServiceClass() && msg->getServiceClass() > 0) {
        set_sups_req.service_class_valid = TRUE;
        set_sups_req.service_class = msg->getServiceClass();
      }

      pendingMsgStatus = getPendingMessageList().insert(msg);
      if (pendingMsgStatus.second != true) {
        QCRIL_LOG_ERROR("getPendingMessageList().insert failed!!");
        rilErr = RIL_E_GENERIC_FAILURE;
        break;
      }
      uint16_t req_id = pendingMsgStatus.first;

      rilErr = qcril_qmi_voice_process_set_sups_service_req(
          req_id, set_sups_req,
          std::bind(&VoiceModule::processSetSupsServiceResponse, this, std::placeholders::_1));
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


void VoiceModule::handleQcRilRequestGetClipMessage(
    std::shared_ptr<QcRilRequestGetClipMessage> msg) {
  QCRIL_LOG_FUNC_ENTRY("msg = %s", (msg ? msg->dump().c_str() : "nullptr"));

  RIL_Errno rilErr = RIL_E_SUCCESS;
  auto pendingMsgStatus = std::make_pair(0, false);

  if (msg != nullptr) {
    do {
      voice_get_clip_req_msg_v02 clip_req = {};
      pendingMsgStatus = getPendingMessageList().insert(msg);
      if (pendingMsgStatus.second != true) {
        QCRIL_LOG_ERROR("getPendingMessageList().insert failed!!");
        rilErr = RIL_E_GENERIC_FAILURE;
        break;
      }
      uint16_t req_id = pendingMsgStatus.first;

      rilErr = qcril_qmi_voice_process_get_clip_req(
          req_id, clip_req,
          std::bind(&VoiceModule::processGetClipResponse, this, std::placeholders::_1));
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


void VoiceModule::handleQcRilRequestGetClirMessage(
    std::shared_ptr<QcRilRequestGetClirMessage> msg) {
  QCRIL_LOG_FUNC_ENTRY("msg = %s", (msg ? msg->dump().c_str() : "nullptr"));

  RIL_Errno rilErr = RIL_E_SUCCESS;
  auto pendingMsgStatus = std::make_pair(0, false);

  if (msg != nullptr) {
    do {
      voice_get_clir_req_msg_v02 clir_req = {};
      pendingMsgStatus = getPendingMessageList().insert(msg);
      if (pendingMsgStatus.second != true) {
        QCRIL_LOG_ERROR("getPendingMessageList().insert failed!!");
        rilErr = RIL_E_GENERIC_FAILURE;
        break;
      }
      uint16_t req_id = pendingMsgStatus.first;

      rilErr = qcril_qmi_voice_process_get_clir_req(
          req_id, clir_req,
          std::bind(&VoiceModule::processGetClirResponse, this, std::placeholders::_1));
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


void VoiceModule::handleQcRilRequestSetClirMessage(
    std::shared_ptr<QcRilRequestSetClirMessage> msg) {
  QCRIL_LOG_FUNC_ENTRY("msg = %s", (msg ? msg->dump().c_str() : "nullptr"));

  RIL_Errno rilErr = RIL_E_INVALID_ARGUMENTS;

  if (msg != nullptr) {
    if (msg->hasParamN()) {
      rilErr = qcril_qmi_voice_process_set_clir_req(msg->getParamN());
    }
    std::shared_ptr<QcRilRequestMessageCallbackPayload> respPayload =
        std::make_shared<QcRilRequestMessageCallbackPayload>(rilErr, nullptr);
    msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
  }
  QCRIL_LOG_FUNC_RETURN();
}


void VoiceModule::handleQcRilRequestGetColrMessage(
    std::shared_ptr<QcRilRequestGetColrMessage> msg) {
  QCRIL_LOG_FUNC_ENTRY("msg = %s", (msg ? msg->dump().c_str() : "nullptr"));

  RIL_Errno rilErr = RIL_E_SUCCESS;
  auto pendingMsgStatus = std::make_pair(0, false);

  if (msg != nullptr) {
    do {
      voice_get_colr_req_msg_v02 colr_req = {};
      pendingMsgStatus = getPendingMessageList().insert(msg);
      if (pendingMsgStatus.second != true) {
        QCRIL_LOG_ERROR("getPendingMessageList().insert failed!!");
        rilErr = RIL_E_GENERIC_FAILURE;
        break;
      }
      uint16_t req_id = pendingMsgStatus.first;

      rilErr = qcril_qmi_voice_process_get_colr_req(
          req_id, colr_req,
          std::bind(&VoiceModule::processGetColrResponse, this, std::placeholders::_1));
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

void VoiceModule::handleQcRilRequestSetColrMessage(
    std::shared_ptr<QcRilRequestSetColrMessage> msg) {
  QCRIL_LOG_FUNC_ENTRY("msg = %s", (msg ? msg->dump().c_str() : "nullptr"));

  RIL_Errno rilErr = RIL_E_SUCCESS;
  voice_set_sups_service_req_msg_v02 set_sups_req = {};
  auto pendingMsgStatus = std::make_pair(0, false);

  if (msg != nullptr) {
    do {
      if (!msg->hasPresentation()) {
        QCRIL_LOG_ERROR("Invalid parameters: Presentation");
        rilErr = RIL_E_INVALID_ARGUMENTS;
        break;
      }

      set_sups_req.supplementary_service_info.reason = VOICE_REASON_COLR_V02;
      set_sups_req.colr_pi_valid = TRUE;
      if (msg->getPresentation() == qcril::interfaces::Presentation::RESTRICTED) {
        set_sups_req.supplementary_service_info.voice_service = VOICE_SERVICE_ACTIVATE_V02;
        set_sups_req.colr_pi = COLR_PRESENTATION_RESTRICTED_V02;
      } else if (msg->getPresentation() == qcril::interfaces::Presentation::ALLOWED) {
        set_sups_req.supplementary_service_info.voice_service = VOICE_SERVICE_DEACTIVATE_V02;
        set_sups_req.colr_pi = COLR_PRESENTATION_NOT_RESTRICTED_V02;
      } else {
        QCRIL_LOG_ERROR("Requested colr is not a valid value");
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

      rilErr = qcril_qmi_voice_process_set_sups_service_req(
          req_id, set_sups_req,
          std::bind(&VoiceModule::processSetSupsServiceResponse, this, std::placeholders::_1));
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

void VoiceModule::handleQcRilRequestSendUssdMessage(
    std::shared_ptr<QcRilRequestSendUssdMessage> msg) {
  QCRIL_LOG_FUNC_ENTRY("msg = %s", (msg ? msg->dump().c_str() : "nullptr"));

  RIL_Errno rilErr = RIL_E_SUCCESS;
  auto pendingMsgStatus = std::make_pair(0, false);

  if (msg != nullptr) {
    do {
      if (!msg->hasUssd() || msg->getUssd().empty()) {
        QCRIL_LOG_ERROR("Invalid parameters: USSD");
        rilErr = RIL_E_INVALID_ARGUMENTS;
        break;
      }
      pendingMsgStatus = getPendingMessageList().insert(msg);
      if (pendingMsgStatus.second != true) {
        QCRIL_LOG_ERROR("getPendingMessageList().insert failed!!");
        break;
      }
      uint16_t req_id = pendingMsgStatus.first;

      int ussd_str_len = 0;
      char ussd_str[QMI_VOICE_USS_DATA_MAX_V02] = {};
      uss_dcs_enum_v02 uss_dcs = USS_DCS_ASCII_V02;

      if (qcril_cm_ss_UssdStringIsAscii(msg->getUssd().c_str())) {
        if (msg->getUssd().size() > QMI_VOICE_USS_DATA_MAX_V02) {
          QCRIL_LOG_DEBUG("Received USSD charecters exceed maximum length");
          rilErr = RIL_E_INVALID_ARGUMENTS;
          break;
        }
        ussd_str_len = msg->getUssd().size();
        memcpy(ussd_str, msg->getUssd().c_str(), msg->getUssd().size());
        uss_dcs = USS_DCS_ASCII_V02;
      } else {
        ussd_str_len = qcril_cm_ss_convert_utf8_to_ucs2(
            msg->getUssd().c_str(), msg->getUssd().size(), ussd_str, sizeof(ussd_str));
        if (ussd_str_len == 0 || ussd_str_len > QMI_VOICE_USS_DATA_MAX_V02) {
          QCRIL_LOG_DEBUG("Illegal UTF8 characters received, length = %d", ussd_str_len);
          rilErr = RIL_E_INVALID_ARGUMENTS;
          break;
        }
        uss_dcs = USS_DCS_UCS2_V02;
      }

      QCRIL_LOG_INFO("USSD User Action Required = %d",
                     qcril_qmi_voice_get_ussd_user_action_required());
      if (qcril_qmi_voice_get_ussd_user_action_required() == FALSE) {
        /* case where RIL is initiating an MO USSD request */
        voice_orig_ussd_req_msg_v02 orig_ussd_req = {};

        orig_ussd_req.uss_info.uss_dcs = uss_dcs;
        orig_ussd_req.uss_info.uss_data_len = ussd_str_len;
        memcpy(orig_ussd_req.uss_info.uss_data, ussd_str, ussd_str_len);

        rilErr = qcril_qmi_voice_process_orig_ussd_req(
            req_id, orig_ussd_req,
            std::bind(&VoiceModule::processOrigUssdResponse, this, std::placeholders::_1));
      } else {
        /* case where RIL is responding for MT USSD request */
        voice_answer_ussd_req_msg_v02 answer_ussd_req = {};

        answer_ussd_req.uss_info.uss_dcs = uss_dcs;
        answer_ussd_req.uss_info.uss_data_len = ussd_str_len;
        memcpy(answer_ussd_req.uss_info.uss_data, ussd_str, ussd_str_len);

        rilErr = qcril_qmi_voice_process_answer_ussd_req(
            req_id, answer_ussd_req,
            std::bind(&VoiceModule::processAnswerUssdResponse, this, std::placeholders::_1));
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

void VoiceModule::handleQcRilRequestCancelUssdMessage(
    std::shared_ptr<QcRilRequestCancelUssdMessage> msg) {
  QCRIL_LOG_FUNC_ENTRY("msg = %s", (msg ? msg->dump().c_str() : "nullptr"));

  RIL_Errno rilErr = RIL_E_SUCCESS;
  auto pendingMsgStatus = std::make_pair(0, false);

  if (msg != nullptr) {
    do {
      voice_cancel_ussd_req_msg_v02 cancel_ussd_req = {};
      pendingMsgStatus = getPendingMessageList().insert(msg);
      if (pendingMsgStatus.second != true) {
        QCRIL_LOG_ERROR("getPendingMessageList().insert failed!!");
        break;
      }
      uint16_t req_id = pendingMsgStatus.first;

        rilErr = qcril_qmi_voice_process_cancel_ussd_req(
            req_id, cancel_ussd_req,
            std::bind(&VoiceModule::processCancelUssdResponse, this, std::placeholders::_1));
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


void VoiceModule::handleQcRilRequestSetPreferredVoicePrivacyMessage(
    std::shared_ptr<QcRilRequestSetPreferredVoicePrivacyMessage> msg) {
  QCRIL_LOG_FUNC_ENTRY("msg = %s", (msg ? msg->dump().c_str() : "nullptr"));

  RIL_Errno rilErr = RIL_E_SUCCESS;
  voice_set_preferred_privacy_req_msg_v02 set_preferred_privacy_req = {};
  auto pendingMsgStatus = std::make_pair(0, false);

  if (msg != nullptr) {
    do {
      if (!msg->hasPrivacyMode()) {
        QCRIL_LOG_ERROR("Invalid parameters: PrivacyMode");
        rilErr = RIL_E_INVALID_ARGUMENTS;
        break;
      }

      switch (msg->getPrivacyMode()) {
        case qcril::interfaces::PrivacyMode::STANDARD:
          set_preferred_privacy_req.privacy_pref = VOICE_PRIVACY_STANDARD_V02;
          break;
        case qcril::interfaces::PrivacyMode::ENHANCED:
          set_preferred_privacy_req.privacy_pref = VOICE_PRIVACY_ENHANCED_V02;
          break;
        default:
          set_preferred_privacy_req.privacy_pref = VOICE_PRIVACY_STANDARD_V02;
      }

      pendingMsgStatus = getPendingMessageList().insert(msg);
      if (pendingMsgStatus.second != true) {
        QCRIL_LOG_ERROR("getPendingMessageList().insert failed!!");
        break;
      }
      uint16_t req_id = pendingMsgStatus.first;

      rilErr = qcril_qmi_voice_process_set_preferred_privacy_req(
          req_id, set_preferred_privacy_req,
          std::bind(&VoiceModule::processSetPreferredVoicePrivacyResponse, this,
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

void VoiceModule::handleQcRilRequestGetPreferredVoicePrivacyMessage(
    std::shared_ptr<QcRilRequestGetPreferredVoicePrivacyMessage> msg) {
  QCRIL_LOG_FUNC_ENTRY("msg = %s", (msg ? msg->dump().c_str() : "nullptr"));

  RIL_Errno rilErr = RIL_E_SUCCESS;
  voice_get_config_req_msg_v02 get_config_req = {.voice_privacy_valid = TRUE,
                                                 .voice_privacy = 0x01};
  auto pendingMsgStatus = std::make_pair(0, false);

  if (msg != nullptr) {
    do {
      pendingMsgStatus = getPendingMessageList().insert(msg);
      if (pendingMsgStatus.second != true) {
        QCRIL_LOG_ERROR("getPendingMessageList().insert failed!!");
        break;
      }
      uint16_t req_id = pendingMsgStatus.first;

      rilErr = qcril_qmi_voice_process_get_config_req(
          req_id, get_config_req,
          std::bind(&VoiceModule::processGetPreferredVoicePrivacyResponse, this, std::placeholders::_1));
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

void VoiceModule::handleQcRilRequestHangupWaitingOrBackgroundMessage(
    std::shared_ptr<QcRilRequestHangupWaitingOrBackgroundMessage> msg) {
  QCRIL_LOG_FUNC_ENTRY("msg = %s", (msg ? msg->dump().c_str() : "nullptr"));

  RIL_Errno rilErr = RIL_E_SUCCESS;
  qcril_qmi_voice_voip_call_info_entry_type *call_info_entry = nullptr;
  qcril_qmi_voice_voip_current_call_summary_type calls_summary = {};
  auto pendingMsgStatus = std::make_pair(0, false);

  if (msg != nullptr) {
    do {
      // find call info
      qcril_qmi_voice_voip_lock_overview();
      qcril_qmi_voice_voip_generate_summary(&calls_summary);
      qcril_qmi_voice_voip_unlock_overview();

      call_info_entry = calls_summary.active_or_single_call;
      if (call_info_entry == nullptr) {
        QCRIL_LOG_ERROR(".. call info not found");
        rilErr = RIL_E_INVALID_STATE;
        break;
      }

      pendingMsgStatus = getPendingMessageList().insert(msg);
      if (pendingMsgStatus.second != true) {
        QCRIL_LOG_ERROR("getPendingMessageList().insert failed!!");
        break;
      }
      uint16_t req_id = pendingMsgStatus.first;

      // DOCOMO
      if (qcril_qmi_voice_reject_cause_21_supported()) {
        qcril_qmi_voice_voip_call_info_entry_type *incoming_call =
            qcril_qmi_voice_voip_find_call_info_entry_by_elaboration(
                QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_PENDING_INCOMING, TRUE);
        if (incoming_call &&
            !(incoming_call->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_ANSWERING_CALL)) {
          voice_answer_call_req_msg_v02 ans_call_req = {};
          ans_call_req.call_id = incoming_call->qmi_call_id;
          ans_call_req.reject_call_valid = TRUE;
          ans_call_req.reject_call = TRUE;
          rilErr = qcril_qmi_voice_process_answer_call_req(
              req_id, ans_call_req,
              std::bind(&VoiceModule::processAnswerResponse, this, std::placeholders::_1),
              std::bind(&VoiceModule::processAnswerCommandOversightCompletion, this,
                        std::placeholders::_1));
          break;
        }
      }
      if (calls_summary.nof_3gpp2_calls) {
        // CDMA forever
        // More than one CDMA calls - possibly Dialing and Incoming states
        // Need to disconnect Incoming call
        if (calls_summary.nof_3gpp2_calls > 1) {
          call_info_entry = qcril_qmi_voice_voip_find_call_info_entry_by_elaboration(
              QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_PENDING_INCOMING, TRUE);
          if (call_info_entry == nullptr) {
            QCRIL_LOG_ERROR(".. 3gpp2 incoming call not found");
            rilErr = RIL_E_INVALID_STATE;
            break;
          }
        }
        voice_end_call_req_msg_v02 end_call_req = {};
        end_call_req.call_id = call_info_entry->qmi_call_id;

        rilErr = qcril_qmi_voice_process_end_call_req(
            req_id, end_call_req,
            std::bind(&VoiceModule::processHangupResponse, this, std::placeholders::_1),
            std::bind(&VoiceModule::processHangupCommandOversightCompletion, this,
                      std::placeholders::_1));
        break;
      }
      if (calls_summary.nof_3gpp_calls) {
        voice_manage_calls_req_msg_v02 manage_calls_req = {
            .sups_type = SUPS_TYPE_RELEASE_HELD_OR_WAITING_V02};

        rilErr = qcril_qmi_voice_process_manage_calls_req(
            req_id, manage_calls_req,
            std::bind(&VoiceModule::processReleaseHeldOrWaitingResponse, this,
                      std::placeholders::_1),
            std::bind(&VoiceModule::processReleaseHeldOrWaitingCommandOversightCompletion, this,
                      std::placeholders::_1));

        if (rilErr == RIL_E_SUCCESS) {
          qcril_qmi_voice_voip_mark_with_specified_call_state(
              QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_CALL_END_BY_USER, CALL_STATE_WAITING_V02);
          qcril_qmi_voice_voip_unmark_with_specified_call_state(
              QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_PENDING_INCOMING, CALL_STATE_WAITING_V02);
          qcril_qmi_voice_voip_mark_with_specified_call_state(
              QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_CALL_END_BY_USER, CALL_STATE_INCOMING_V02);
          qcril_qmi_voice_voip_unmark_with_specified_call_state(
              QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_PENDING_INCOMING, CALL_STATE_INCOMING_V02);
          qcril_qmi_voice_voip_mark_with_specified_call_state(
              QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_CALL_END_BY_USER, CALL_STATE_HOLD_V02);
        }
        break;
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
void VoiceModule::handleQcRilRequestHangupForegroundResumeBackgroundMessage(
    std::shared_ptr<QcRilRequestHangupForegroundResumeBackgroundMessage> msg) {
  QCRIL_LOG_FUNC_ENTRY("msg = %s", (msg ? msg->dump().c_str() : "nullptr"));

  RIL_Errno rilErr = RIL_E_SUCCESS;
  qcril_qmi_voice_voip_call_info_entry_type *call_info_entry = nullptr;
  qcril_qmi_voice_voip_current_call_summary_type calls_summary = {};
  auto pendingMsgStatus = std::make_pair(0, false);

  if (msg != nullptr) {
    do {
      // find call info
      qcril_qmi_voice_voip_lock_overview();
      qcril_qmi_voice_voip_generate_summary(&calls_summary);
      qcril_qmi_voice_voip_unlock_overview();

      call_info_entry = calls_summary.active_or_single_call;
      if (call_info_entry == nullptr) {
        QCRIL_LOG_ERROR(".. call info not found");
        rilErr = RIL_E_INVALID_STATE;
        break;
      }

      pendingMsgStatus = getPendingMessageList().insert(msg);
      if (pendingMsgStatus.second != true) {
        QCRIL_LOG_ERROR("getPendingMessageList().insert failed!!");
        break;
      }
      uint16_t req_id = pendingMsgStatus.first;

      // Use QMI_VOICE_END_CALL_REQ to hangup if there is only one call
      // and is not in held state.
      if ((calls_summary.nof_calls_overall == 1) &&
          (call_info_entry->voice_scv_info.call_state != CALL_STATE_HOLD_V02)) {
        // one call
        voice_end_call_req_msg_v02 end_call_req = {};
        end_call_req.call_id = call_info_entry->qmi_call_id;

        rilErr = qcril_qmi_voice_process_end_call_req(
            req_id, end_call_req,
            std::bind(&VoiceModule::processHangupResponse, this, std::placeholders::_1),
            std::bind(&VoiceModule::processHangupCommandOversightCompletion, this,
                      std::placeholders::_1));
        break;
      }
      if (calls_summary.nof_3gpp_calls) {
        voice_manage_calls_req_msg_v02 manage_calls_req = {
            .sups_type = SUPS_TYPE_RELEASE_ACTIVE_ACCEPT_HELD_OR_WAITING_V02};

        rilErr = qcril_qmi_voice_process_manage_calls_req(
            req_id, manage_calls_req,
            std::bind(&VoiceModule::processReleaseActiveAcceptHeldOrWaitingResponse, this,
                      std::placeholders::_1),
            nullptr);

        if (rilErr == RIL_E_SUCCESS) {
          qcril_qmi_voice_voip_mark_with_specified_call_state(
              QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_CALL_END_BY_USER, CALL_STATE_CONVERSATION_V02);
          qcril_qmi_voice_voip_unmark_with_specified_call_state(
              QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_PENDING_INCOMING, CALL_STATE_WAITING_V02);
        }
        break;
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


void VoiceModule::handleQcRilRequestSwitchWaitingOrHoldingAndActiveMessage(
    std::shared_ptr<QcRilRequestSwitchWaitingOrHoldingAndActiveMessage> msg) {
  QCRIL_LOG_FUNC_ENTRY("msg = %s", (msg ? msg->dump().c_str() : "nullptr"));

  RIL_Errno rilErr = RIL_E_SUCCESS;
  qcril_qmi_voice_voip_current_call_summary_type calls_summary = {};
  auto pendingMsgStatus = std::make_pair(0, false);

  if (msg != nullptr) {
    do {
      // find call info
      qcril_qmi_voice_voip_lock_overview();
      qcril_qmi_voice_voip_generate_summary(&calls_summary);
      qcril_qmi_voice_voip_unlock_overview();

      if (calls_summary.nof_3gpp_calls == 0) {
        QCRIL_LOG_ERROR(".. call info not found");
        rilErr = RIL_E_INVALID_STATE;
        break;
      }

      pendingMsgStatus = getPendingMessageList().insert(msg);
      if (pendingMsgStatus.second != true) {
        QCRIL_LOG_ERROR("getPendingMessageList().insert failed!!");
        break;
      }
      uint16_t req_id = pendingMsgStatus.first;

      if (calls_summary.nof_3gpp_calls) {
        voice_manage_calls_req_msg_v02 manage_calls_req = {
            .sups_type = SUPS_TYPE_HOLD_ACTIVE_ACCEPT_WAITING_OR_HELD_V02};

        rilErr = qcril_qmi_voice_process_manage_calls_req(
            req_id, manage_calls_req,
            std::bind(&VoiceModule::processHoldActiveAcceptWaitingOrHeldResponse, this,
                      std::placeholders::_1),
            nullptr);

        if (rilErr == RIL_E_SUCCESS) {
          // update call obj state
          qcril_qmi_voice_voip_unmark_with_specified_call_state(
              QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_PENDING_INCOMING, CALL_STATE_WAITING_V02);

          qcril_qmi_voice_voip_mark_with_specified_call_state(
              QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_SWITCHING_CALL_TO_ACTIVE, CALL_STATE_WAITING_V02);
          qcril_qmi_voice_voip_mark_with_specified_call_state(
              QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_SWITCHING_CALL_TO_ACTIVE, CALL_STATE_HOLD_V02);
          qcril_am_handle_event(QCRIL_AM_EVENT_SWITCH_CALL, NULL);
        }
        break;
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


void VoiceModule::handleQcRilRequestConferenceMessage(
    std::shared_ptr<QcRilRequestConferenceMessage> msg) {
  QCRIL_LOG_FUNC_ENTRY("msg = %s", (msg ? msg->dump().c_str() : "nullptr"));

  RIL_Errno rilErr = RIL_E_SUCCESS;
  qcril_qmi_voice_voip_current_call_summary_type calls_summary = {};
  auto pendingMsgStatus = std::make_pair(0, false);

  if (msg != nullptr) {
    do {
      qcril_qmi_voice_voip_lock_overview();
      qcril_qmi_voice_voip_generate_summary(&calls_summary);
      qcril_qmi_voice_voip_unlock_overview();

      if (calls_summary.nof_3gpp_calls == 0) {
        QCRIL_LOG_ERROR(".. call info not found");
        rilErr = RIL_E_INVALID_STATE;
        break;
      }

      voice_manage_calls_req_msg_v02 manage_calls_req = {
          .sups_type = SUPS_TYPE_MAKE_CONFERENCE_CALL_V02};

      pendingMsgStatus = getPendingMessageList().insert(msg);
      if (pendingMsgStatus.second != true) {
        QCRIL_LOG_ERROR("getPendingMessageList().insert failed!!");
        rilErr = RIL_E_GENERIC_FAILURE;
        break;
      }
      uint16_t req_id = pendingMsgStatus.first;

      rilErr = qcril_qmi_voice_process_manage_calls_req(
          req_id, manage_calls_req,
          std::bind(&VoiceModule::processCommonManageCallsResponse, this,
                    std::placeholders::_1),
          nullptr);
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

void VoiceModule::handleQcRilRequestExplicitCallTransferMessage(
    std::shared_ptr<QcRilRequestExplicitCallTransferMessage> msg) {
  QCRIL_LOG_FUNC_ENTRY("msg = %s", (msg ? msg->dump().c_str() : "nullptr"));

  RIL_Errno rilErr = RIL_E_SUCCESS;
  qcril_qmi_voice_voip_current_call_summary_type calls_summary = {};
  auto pendingMsgStatus = std::make_pair(0, false);

  if (msg != nullptr) {
    do {
      qcril_qmi_voice_voip_lock_overview();
      qcril_qmi_voice_voip_generate_summary(&calls_summary);
      qcril_qmi_voice_voip_unlock_overview();

      if (calls_summary.nof_3gpp_calls == 0) {
        QCRIL_LOG_ERROR(".. call info not found");
        rilErr = RIL_E_INVALID_STATE;
        break;
      }

      voice_manage_calls_req_msg_v02 manage_calls_req = {
          .sups_type = SUPS_TYPE_EXPLICIT_CALL_TRANSFER_V02};

      pendingMsgStatus = getPendingMessageList().insert(msg);
      if (pendingMsgStatus.second != true) {
        QCRIL_LOG_ERROR("getPendingMessageList().insert failed!!");
        rilErr = RIL_E_GENERIC_FAILURE;
        break;
      }
      uint16_t req_id = pendingMsgStatus.first;

      rilErr = qcril_qmi_voice_process_manage_calls_req(
          req_id, manage_calls_req,
          std::bind(&VoiceModule::processCommonManageCallsResponse, this,
                    std::placeholders::_1),
          nullptr);
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

void VoiceModule::handleQcRilRequestSeparateConnectionMessage(
    std::shared_ptr<QcRilRequestSeparateConnectionMessage> msg) {
  QCRIL_LOG_FUNC_ENTRY("msg = %s", (msg ? msg->dump().c_str() : "nullptr"));

  RIL_Errno rilErr = RIL_E_SUCCESS;
  bool sendResponse = true;
  qcril_qmi_voice_voip_current_call_summary_type call_summary = {};
  auto pendingMsgStatus = std::make_pair(0, false);

  if (msg != nullptr) {
    do {
      if (!msg->hasCallId()) {
        QCRIL_LOG_ERROR("Invalid parameter: callId");
        rilErr = RIL_E_INVALID_ARGUMENTS;
        break;
      }

      qcril_qmi_voice_voip_lock_overview();

      qcril_qmi_voice_voip_call_info_entry_type *call_info_entry =
          qcril_qmi_voice_voip_find_call_info_entry_by_call_android_id(msg->getCallId());
      qcril_qmi_voice_voip_generate_summary(&call_summary);

      qcril_qmi_voice_voip_unlock_overview();

      if (NULL == call_info_entry) {
        QCRIL_LOG_ERROR(".. internal call record not found");
        rilErr = RIL_E_INVALID_ARGUMENTS;
        break;
      }

      QCRIL_LOG_INFO(".. nof voice %d, nof voip %d, nof 3gpp %d, nof 3gpp2 %d, nof active %d",
                     call_summary.nof_voice_calls, call_summary.nof_voip_calls,
                     call_summary.nof_3gpp_calls, call_summary.nof_3gpp2_calls,
                     call_summary.nof_active_calls);

      // No calls present
      if (call_summary.nof_voip_calls + call_summary.nof_voice_calls == QMI_RIL_ZERO) {
        rilErr = RIL_E_INVALID_STATE;
        break;
      }

      // voip / 3gpp2 - not applicable
      if (call_summary.nof_voip_calls > QMI_RIL_ZERO ||
          call_summary.nof_3gpp2_calls > QMI_RIL_ZERO) {
        QCRIL_LOG_ERROR(".. not supported for voip or 3gpp2 call");
        rilErr = RIL_E_INVALID_STATE;
        break;
      }
      // single call, return success
      if (call_summary.nof_calls_overall == 1) {
        rilErr = RIL_E_SUCCESS;
        break;
      }
      // the call_id passed in is the only active call, return success
      if ((call_summary.nof_active_calls == 1) &&
          (call_info_entry->voice_scv_info.call_state == CALL_STATE_CONVERSATION_V02)) {
        rilErr = RIL_E_SUCCESS;
        break;
      }

      voice_manage_calls_req_msg_v02 manage_calls_req = {
          .sups_type = SUPS_TYPE_HOLD_ALL_EXCEPT_SPECIFIED_CALL_V02};
      manage_calls_req.call_id_valid = TRUE;
      manage_calls_req.call_id = call_info_entry->qmi_call_id;

      pendingMsgStatus = getPendingMessageList().insert(msg);
      if (pendingMsgStatus.second != true) {
        QCRIL_LOG_ERROR("getPendingMessageList().insert failed!!");
        rilErr = RIL_E_GENERIC_FAILURE;
        break;
      }
      uint16_t req_id = pendingMsgStatus.first;

      rilErr = qcril_qmi_voice_process_manage_calls_req(
          req_id, manage_calls_req,
          std::bind(&VoiceModule::processCommonManageCallsResponse, this, std::placeholders::_1),
          nullptr);
      if (rilErr == RIL_E_SUCCESS) {
        sendResponse = false;
      }
    } while (0);

    if (sendResponse) {
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

void VoiceModule::handleQcRilRequestUdubMessage(
    std::shared_ptr<QcRilRequestUdubMessage> msg) {
  QCRIL_LOG_FUNC_ENTRY("msg = %s", (msg ? msg->dump().c_str() : "nullptr"));

  RIL_Errno rilErr = RIL_E_SUCCESS;
  qcril_qmi_voice_voip_current_call_summary_type calls_summary = {};
  auto pendingMsgStatus = std::make_pair(0, false);

  if (msg != nullptr) {
    do {
      qcril_qmi_voice_voip_lock_overview();
      qcril_qmi_voice_voip_generate_summary(&calls_summary);
      qcril_qmi_voice_voip_unlock_overview();

      if (calls_summary.nof_3gpp_calls == 0) {
        QCRIL_LOG_ERROR(".. call info not found");
        rilErr = RIL_E_INVALID_STATE;
        break;
      }

      voice_manage_calls_req_msg_v02 manage_calls_req = {
          .sups_type = SUPS_TYPE_RELEASE_HELD_OR_WAITING_V02};

      pendingMsgStatus = getPendingMessageList().insert(msg);
      if (pendingMsgStatus.second != true) {
        QCRIL_LOG_ERROR("getPendingMessageList().insert failed!!");
        rilErr = RIL_E_GENERIC_FAILURE;
        break;
      }
      uint16_t req_id = pendingMsgStatus.first;

      rilErr = qcril_qmi_voice_process_manage_calls_req(
          req_id, manage_calls_req,
          std::bind(&VoiceModule::processCommonManageCallsResponse, this,
                    std::placeholders::_1),
          nullptr);
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

void VoiceModule::handleQcRilRequestRejectIncomingCallMessage(
    std::shared_ptr<QcRilRequestRejectIncomingCallMessage> msg) {
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

      qcril_qmi_voice_voip_call_info_entry_type *incoming_call =
          qcril_qmi_voice_voip_find_call_info_entry_by_elaboration(
              QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_PENDING_INCOMING, TRUE);
      if (!incoming_call) {
        QCRIL_LOG_ERROR("No incoming calls found!!");
        rilErr = RIL_E_INVALID_STATE;
        break;
      }

      voice_answer_call_req_msg_v02 ans_call_req = {
          .call_id = incoming_call->qmi_call_id, .reject_call_valid = TRUE, .reject_call = TRUE};

      rilErr = qcril_qmi_voice_process_answer_call_req(
          req_id, ans_call_req,
          std::bind(&VoiceModule::processAnswerResponse, this, std::placeholders::_1),
          std::bind(&VoiceModule::processAnswerCommandOversightCompletion, this,
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

void VoiceModule::handleQcRilRequestGetCurrentSetupCallsMessage(
    std::shared_ptr<QcRilRequestGetCurrentSetupCallsMessage> msg) {
  QCRIL_LOG_FUNC_ENTRY("msg = %s", (msg ? msg->dump().c_str() : "nullptr"));

  qcril_qmi_voice_get_current_setup_calls(msg);

  QCRIL_LOG_FUNC_RETURN();
}


void VoiceModule::handleQcRilRequestSetupAnswerMessage(
    std::shared_ptr<QcRilRequestSetupAnswerMessage> msg) {
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

      qcril_qmi_voice_voip_call_info_entry_type *setup_call =
          qcril_qmi_voice_voip_find_call_info_entry_by_qmi_call_state(CALL_STATE_SETUP_V02);
      if (!setup_call) {
        QCRIL_LOG_ERROR("No setup calls found!!");
        rilErr = RIL_E_INVALID_STATE;
        break;
      }

      voice_setup_answer_req_msg_v02 setup_answer_req = {.call_id = setup_call->qmi_call_id};
      if (msg->hasRejectSetup()) {
        setup_answer_req.reject_setup_valid = TRUE;
        setup_answer_req.reject_setup = (msg->getRejectSetup() ? TRUE : FALSE);
      }

      rilErr = qcril_qmi_voice_process_setup_answer_req(
          req_id, setup_answer_req,
          std::bind(&VoiceModule::processSetupAnswerResponse, this, std::placeholders::_1),
          std::bind(&VoiceModule::processSetupAnswerCommandOversightCompletion, this,
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

void VoiceModule::handleQcRilRequestSetLocalCallHoldMessage(
    std::shared_ptr<QcRilRequestSetLocalCallHoldMessage> msg) {
  QCRIL_LOG_FUNC_ENTRY("msg = %s", (msg ? msg->dump().c_str() : "nullptr"));

  RIL_Errno rilErr = RIL_E_SUCCESS;
  auto pendingMsgStatus = std::make_pair(0, false);

  if (msg != nullptr) {
    do {
      if (!msg->hasLocalHoldType()) {
        QCRIL_LOG_ERROR("Invalid parameters");
        rilErr = RIL_E_GENERIC_FAILURE;
        break;
      }

      pendingMsgStatus = getPendingMessageList().insert(msg);
      if (pendingMsgStatus.second != true) {
        QCRIL_LOG_ERROR("getPendingMessageList().insert failed!!");
        rilErr = RIL_E_GENERIC_FAILURE;
        break;
      }
      uint16_t req_id = pendingMsgStatus.first;

      voice_manage_calls_req_msg_v02 manage_calls_req = {
          .sups_type =
              (msg->getLocalHoldType() == QcRilRequestSetLocalCallHoldMessage::LocalHoldType::HOLD
                   ? SUPS_TYPE_LOCAL_HOLD_V02
                   : SUPS_TYPE_LOCAL_UNHOLD_V02)};

      msg->setPendingMessageState(PendingMessageState::AWAITING_CALLBACK);
      rilErr = qcril_qmi_voice_process_manage_calls_req(
          req_id, manage_calls_req,
          std::bind(&VoiceModule::processSetLocalCallHoldResponse, this, std::placeholders::_1),
          nullptr);

      if (rilErr == RIL_E_SUCCESS) {
#ifndef QMI_RIL_UTF
        qcril_am_handle_event(
            (msg->getLocalHoldType() == QcRilRequestSetLocalCallHoldMessage::LocalHoldType::HOLD
                 ? QCRIL_AM_EVENT_LCH
                 : QCRIL_AM_EVENT_UNLCH),
            NULL);
#else
      // For UTF
      msg->setAudioStateUpdateCompleted(true);
#endif
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

void VoiceModule::handleQcRilRequestSetMuteMessage(
    std::shared_ptr<QcRilRequestSetMuteMessage> msg) {
  QCRIL_LOG_FUNC_ENTRY("msg = %s", (msg ? msg->dump().c_str() : "nullptr"));

  RIL_Errno rilErr = RIL_E_INVALID_ARGUMENTS;

  if (msg != nullptr) {
    if (msg->hasEnable()) {
      mUplinkMute = msg->getEnable();
      rilErr = RIL_E_SUCCESS;
    }
    std::shared_ptr<QcRilRequestMessageCallbackPayload> respPayload =
        std::make_shared<QcRilRequestMessageCallbackPayload>(rilErr, nullptr);
    msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
  }
  QCRIL_LOG_FUNC_RETURN();
}

void VoiceModule::handleQcRilRequestGetMuteMessage(
    std::shared_ptr<QcRilRequestGetMuteMessage> msg) {
  QCRIL_LOG_FUNC_ENTRY("msg = %s", (msg ? msg->dump().c_str() : "nullptr"));

  RIL_Errno rilErr = RIL_E_GENERIC_FAILURE;

  if (msg != nullptr) {
    auto muteInfo = std::make_shared<qcril::interfaces::MuteInfo>();
    if (muteInfo) {
      muteInfo->setEnable(mUplinkMute);
      rilErr = RIL_E_SUCCESS;
    }
    std::shared_ptr<QcRilRequestMessageCallbackPayload> respPayload =
        std::make_shared<QcRilRequestMessageCallbackPayload>(rilErr, muteInfo);
    msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
  }
  QCRIL_LOG_FUNC_RETURN();
}


void VoiceModule::handleVoiceRequestPendingDialMessage(
    std::shared_ptr<VoiceRequestPendingDialMessage> msg) {
  QCRIL_LOG_FUNC_ENTRY();

  if (msg) {
    qcril_qmi_pending_call_info_type pendingCallInfo = msg->getPendingCallInfo();
    RIL_Errno rilErr = qcril_qmi_voice_process_dial_call_req(
        pendingCallInfo.req_id, pendingCallInfo.dial_call_req, pendingCallInfo.responseCb,
        pendingCallInfo.commandOversightCompletionHandler, pendingCallInfo.elaboration);

    if (rilErr != RIL_E_SUCCESS) {
      if (pendingCallInfo.commandOversightCompletionHandler) {
        CommonVoiceResponseData respData = {pendingCallInfo.req_id, RIL_E_GENERIC_FAILURE, nullptr};
        pendingCallInfo.commandOversightCompletionHandler(&respData);
      }
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

/////////////////// VOICE exported interfaces ////////////////////////////////

void VoiceModule::handleGetAnswerCallModeMessage(std::shared_ptr<VoiceGetAnswerCallModeMessage> msg) {
  call_mode_enum_v02 callMode = qcril_qmi_voice_get_answer_call_mode();

  msg->sendResponse(msg, Message::Callback::Status::SUCCESS,
    std::shared_ptr<call_mode_enum_v02>(new call_mode_enum_v02(callMode)));
}

void VoiceModule::handleCheckSpecificCallMessage(std::shared_ptr<VoiceCheckSpecificCallMessage> msg) {
  qcril_qmi_voice_call_filter filter;
  call_mode_enum_v02 call_mode = CALL_MODE_NO_SRV_V02;

  if ((filter = msg->getVoiceCallFilter()) == nullptr) {
    msg->sendResponse(msg, Message::Callback::Status::FAILURE, nullptr);
  } else {
    boolean ret = qcril_qmi_voice_has_specific_call(filter, &call_mode);
    auto result = std::shared_ptr<SpecificCallResult>(new SpecificCallResult);
    if (result == nullptr) {
      msg->sendResponse(msg, Message::Callback::Status::FAILURE, nullptr);
    } else {
      result->has_call = (ret == TRUE);
      result->call_mode = call_mode;
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, result);
    }
  }
}

void VoiceModule::handleCheckCallPresentMessage(std::shared_ptr<VoiceCheckCallPresentMessage> msg) {
  bool present = false;

  switch (msg->getCallType()) {
  case VoiceCheckCallPresentMessage::ALL_CALL:
    present = qcril_qmi_voice_nas_control_is_any_calls_present();
    break;
  case VoiceCheckCallPresentMessage::VOICE_CALL:
    present = qmi_ril_voice_is_under_any_voice_calls();
    break;
  default:
    present = false;
  }
  msg->sendResponse(msg, Message::Callback::Status::SUCCESS,
      std::shared_ptr<bool>(new bool{present}));
}

void VoiceModule::handleLimitedIndMessage(std::shared_ptr<VoiceLimitIndMessage> msg) {
  qcril_qmi_voice_enable_voice_indications(msg->isIndLimited());
}

void VoiceModule::handleImsConnectedMessage(std::shared_ptr<VoiceImsConnectedMessage> msg) {
  (void)msg;
  qcril_qmi_voice_ims_send_unsol_radio_state_change();
  qcril_qmi_voice_ims_send_unsol_vice_dialog_refresh_info_helper();
}

void VoiceModule::handleRteProcessPendingCall(std::shared_ptr<VoiceRteProcessPendingCallMessage> msg) {
  qcril_qmi_voice_nas_control_process_calls_pending_for_specified_voice_rte(
        msg->getCallRadioTechFamily());
}

#ifdef USING_NAS_PUBLIC_API
// handle NAS indication
void VoiceModule::HandleVoiceRegStatusIndMessage(std::shared_ptr<Message> msg) {
  std::shared_ptr<VoiceRegistrationStatusChangeInd> shared_msg(
      std::static_pointer_cast<VoiceRegistrationStatusChangeInd>(msg));
  voiceRegStatusStruct voiceReg;
  // TODO: waiting for QIPL to provide get method
  voiceReg.stateValid = shared_msg->stateValid;
  voiceReg.state = shared_msg->state;
  voiceReg.radioTechValid = shared_msg->radioTechValid;
  voiceReg.radioTech = shared_msg->radioTech;
  voiceReg.statusOverviewValid = shared_msg->statusOverviewValid;
  voiceReg.statusOverview = shared_msg->statusOverview;

  qcril_qmi_voice_update_voice_registration_status(voiceReg);
}

void VoiceModule::HandleServiceRteChangeIndMessage(std::shared_ptr<Message> msg) {
  std::shared_ptr<NetworkServiceRteChangeInd> shared_msg(
      std::static_pointer_cast<NetworkServiceRteChangeInd>(msg));
  networkRteStruct networkRte;
  // TODO: waiting for QIPL to provide get method
  networkRte.voiceRteValid = shared_msg->voiceRteValid;
  networkRte.voiceRte = shared_msg->voiceRte;
  networkRte.dataRteValid = shared_msg->dataRteValid;
  networkRte.dataRte = shared_msg->dataRte;
  networkRte.imsRteValid = shared_msg->imsRteValid;
  networkRte.imsRte = shared_msg->imsRte;

  qcril_qmi_voice_update_service_rte_info(networkRte);
}
#endif

#ifdef QMI_RIL_UTF
void VoiceModule::cleanup()
{
    std::shared_ptr<VoiceModemEndPoint> mVoiceModemEndPoint =
                    ModemEndPointFactory<VoiceModemEndPoint>::getInstance().buildEndPoint();
    VoiceModemEndPointModule* mVoiceModemEndPointModule =
                   (VoiceModemEndPointModule*)mVoiceModemEndPoint->mModule;
    mVoiceModemEndPointModule->cleanUpQmiSvcClient();
    getPendingMessageList().clear();
    mInitialized = false;
}
#endif
