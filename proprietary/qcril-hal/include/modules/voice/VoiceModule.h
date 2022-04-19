/******************************************************************************
#  Copyright (c) 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/Module.h"
#include "framework/legacy.h"
#include "framework/QcrilInitMessage.h"
#include "framework/QcrilImsClientConnected.h"
#include <framework/AddPendingMessageList.h>

#include <interfaces/voice/QcRilRequestSendDtmfMessage.h>
#include <interfaces/voice/QcRilRequestStartDtmfMessage.h>
#include <interfaces/voice/QcRilRequestStopDtmfMessage.h>
#include <interfaces/voice/QcRilRequestCancelUssdMessage.h>
#include <interfaces/voice/QcRilRequestCsDialMessage.h>
#include <interfaces/voice/QcRilRequestSetTtyModeMessage.h>
#include <interfaces/voice/QcRilRequestSetPreferredVoicePrivacyMessage.h>
#include <interfaces/voice/QcRilRequestSendUssdMessage.h>
#include <interfaces/voice/QcRilRequestGetTtyModeMessage.h>
#include <interfaces/voice/QcRilRequestGetPreferredVoicePrivacyMessage.h>
#include <interfaces/voice/QcRilRequestGetCurrentCallsMessage.h>
#include <interfaces/voice/QcRilRequestConferenceMessage.h>
#include <interfaces/voice/QcRilRequestCsAnswerMessage.h>
#include <interfaces/voice/QcRilRequestLastCallFailCauseMessage.h>
#include <interfaces/voice/QcRilRequestCdmaFlashMessage.h>
#include <interfaces/voice/QcRilRequestExplicitCallTransferMessage.h>
#include <interfaces/voice/QcRilRequestCdmaBurstDtmfMessage.h>
#include <interfaces/voice/QcRilRequestGetCallWaitingMessage.h>
#include <interfaces/voice/QcRilRequestGetClipMessage.h>
#include <interfaces/voice/QcRilRequestGetClirMessage.h>
#include <interfaces/voice/QcRilRequestGetColrMessage.h>
#include <interfaces/voice/QcRilRequestHangupForegroundResumeBackgroundMessage.h>
#include <interfaces/voice/QcRilRequestCsHangupMessage.h>
#include <interfaces/voice/QcRilRequestHangupWaitingOrBackgroundMessage.h>
#include <interfaces/voice/QcRilRequestQueryCallForwardMessage.h>
#include <interfaces/voice/QcRilRequestQueryColpMessage.h>
#include <interfaces/voice/QcRilRequestGetCallBarringMessage.h>
#include <interfaces/voice/QcRilRequestSeparateConnectionMessage.h>
#include <interfaces/voice/QcRilRequestSetCallBarringPasswordMessage.h>
#include <interfaces/voice/QcRilRequestSetCallForwardMessage.h>
#include <interfaces/voice/QcRilRequestSetCallWaitingMessage.h>
#include <interfaces/voice/QcRilRequestSetClirMessage.h>
#include <interfaces/voice/QcRilRequestSetColrMessage.h>
#include <interfaces/voice/QcRilRequestSetSuppSvcNotificationMessage.h>
#include <interfaces/voice/QcRilRequestSetSupsServiceMessage.h>
#include <interfaces/voice/QcRilRequestSwitchWaitingOrHoldingAndActiveMessage.h>
#include <interfaces/voice/QcRilRequestUdubMessage.h>
#include <interfaces/voice/QcRilRequestRejectIncomingCallMessage.h>
#include <interfaces/voice/QcRilRequestSetupAnswerMessage.h>
#include <interfaces/voice/QcRilRequestGetCurrentSetupCallsMessage.h>
#include <interfaces/voice/QcRilRequestSetLocalCallHoldMessage.h>
#include <interfaces/voice/QcRilRequestSetMuteMessage.h>
#include <interfaces/voice/QcRilRequestGetMuteMessage.h>

#include "modules/voice/RilRequestDialSyncMessage.h"

#include "modules/voice/VoiceLimitIndMessage.h"
#include "modules/voice/VoiceGetAnswerCallModeMessage.h"
#include "modules/voice/VoiceCheckSpecificCallMessage.h"
#include "modules/voice/VoiceImsConnectedMessage.h"
#include "modules/voice/VoiceRteProcessPendingCallMessage.h"
#include "modules/voice/VoiceCheckCallPresentMessage.h"
#include "modules/voice/VoiceRequestPendingDialMessage.h"
#include "modules/qmi/QmiAsyncResponseMessage.h"

#include "modules/voice/qcril_qmi_voice.h"
#include <string.h>

class VoiceModule : public Module, public AddPendingMessageList {
 private:
  bool mInitialized;
  bool mSetAllCallForwardSupported = false;

  // Uplink (microphone) mute settings
  bool mUplinkMute = false;

 public:
  VoiceModule();
  VoiceModule(string name);
  ~VoiceModule();
  void init();
#ifdef QMI_RIL_UTF
  virtual void cleanup();
#endif

 private:
  // handle QMI IND
  void handleVoiceQmiIndMessage(std::shared_ptr<Message> msg);
  // handle QMI ASYNC RESP
  void handleQmiAsyncRespMessage(std::shared_ptr<QmiAsyncResponseMessage> msg);
  // handle End Point Status Indication
  void handleVoiceEndpointStatusIndMessage(std::shared_ptr<Message> msg);
  // handle Qcril Init Message
  void handleQcrilInit(std::shared_ptr<QcrilInitMessage> msg);
  // handle QcrilImsClientConnected
  void handleQcrilImsClientConnected(std::shared_ptr<QcrilImsClientConnected> msg);

  // RIL requests with translators
  void handleDialSyncMessage(std::shared_ptr<RilRequestDialSyncMessage> shared_dialMsg);

  void handleQcRilRequestCsDialMessage(std::shared_ptr<QcRilRequestCsDialMessage> msg);
  void handleQcRilRequestGetCurrentCallsMessage(
      std::shared_ptr<QcRilRequestGetCurrentCallsMessage> msg);
  void handleQcRilRequestLastCallFailCauseMessage(
      std::shared_ptr<QcRilRequestLastCallFailCauseMessage> msg);
  void handleQcRilRequestCsAnswerMessage(std::shared_ptr<QcRilRequestCsAnswerMessage> msg);
  void handleQcRilRequestCsHangupMessage(std::shared_ptr<QcRilRequestCsHangupMessage> msg);
  void handleQcRilRequestCdmaFlashMessage(std::shared_ptr<QcRilRequestCdmaFlashMessage> msg);
  void handleQcRilRequestGetPreferredVoicePrivacyMessage(
      std::shared_ptr<QcRilRequestGetPreferredVoicePrivacyMessage> msg);
  void handleQcRilRequestSetPreferredVoicePrivacyMessage(
      std::shared_ptr<QcRilRequestSetPreferredVoicePrivacyMessage> msg);
  void handleQcRilRequestSendDtmfMessage(std::shared_ptr<QcRilRequestSendDtmfMessage> msg);
  void handleQcRilRequestStartDtmfMessage(std::shared_ptr<QcRilRequestStartDtmfMessage> msg);
  void handleQcRilRequestStopDtmfMessage(std::shared_ptr<QcRilRequestStopDtmfMessage> msg);
  void handleQcRilRequestCancelUssdMessage(std::shared_ptr<QcRilRequestCancelUssdMessage> msg);
  void handleQcRilRequestSendUssdMessage(std::shared_ptr<QcRilRequestSendUssdMessage> msg);
  void handleQcRilRequestSetTtyModeMessage(std::shared_ptr<QcRilRequestSetTtyModeMessage> msg);
  void handleQcRilRequestGetTtyModeMessage(std::shared_ptr<QcRilRequestGetTtyModeMessage> msg);
  void handleQcRilRequestSetSuppSvcNotificationMessage(
      std::shared_ptr<QcRilRequestSetSuppSvcNotificationMessage> msg);
  void handleQcRilRequestGetClipMessage(std::shared_ptr<QcRilRequestGetClipMessage> msg);
  void handleQcRilRequestGetClirMessage(std::shared_ptr<QcRilRequestGetClirMessage> msg);
  void handleQcRilRequestSetClirMessage(std::shared_ptr<QcRilRequestSetClirMessage> msg);
  void handleQcRilRequestGetColrMessage(std::shared_ptr<QcRilRequestGetColrMessage> msg);
  void handleQcRilRequestSetColrMessage(std::shared_ptr<QcRilRequestSetColrMessage> msg);
  void handleQcRilRequestQueryCallForwardMessage(
      std::shared_ptr<QcRilRequestQueryCallForwardMessage> msg);
  void handleQcRilRequestSetCallForwardMessage(
      std::shared_ptr<QcRilRequestSetCallForwardMessage> msg);
  void handleQcRilRequestGetCallWaitingMessage(
      std::shared_ptr<QcRilRequestGetCallWaitingMessage> msg);
  void handleQcRilRequestSetCallWaitingMessage(
      std::shared_ptr<QcRilRequestSetCallWaitingMessage> msg);
  void handleQcRilRequestSetSupsServiceMessage(
      std::shared_ptr<QcRilRequestSetSupsServiceMessage> msg);
  void handleQcRilRequestQueryColpMessage(
      std::shared_ptr<QcRilRequestQueryColpMessage> msg);
  void handleQcRilRequestGetCallBarringMessage(
      std::shared_ptr<QcRilRequestGetCallBarringMessage> msg);
  void handleQcRilRequestSetCallBarringPasswordMessage(
      std::shared_ptr<QcRilRequestSetCallBarringPasswordMessage> msg);
  void handleQcRilRequestConferenceMessage(
      std::shared_ptr<QcRilRequestConferenceMessage> msg);
  void handleQcRilRequestHangupForegroundResumeBackgroundMessage(
      std::shared_ptr<QcRilRequestHangupForegroundResumeBackgroundMessage> msg);
  void handleQcRilRequestHangupWaitingOrBackgroundMessage(
      std::shared_ptr<QcRilRequestHangupWaitingOrBackgroundMessage> msg);
  void handleQcRilRequestSwitchWaitingOrHoldingAndActiveMessage(
      std::shared_ptr<QcRilRequestSwitchWaitingOrHoldingAndActiveMessage> msg);
  void handleQcRilRequestUdubMessage(
      std::shared_ptr<QcRilRequestUdubMessage> msg);
  void handleQcRilRequestSeparateConnectionMessage(
      std::shared_ptr<QcRilRequestSeparateConnectionMessage> msg);
  void handleQcRilRequestExplicitCallTransferMessage(
      std::shared_ptr<QcRilRequestExplicitCallTransferMessage> msg);
  void handleQcRilRequestCdmaBurstDtmfMessage(
      std::shared_ptr<QcRilRequestCdmaBurstDtmfMessage> msg);
  void handleQcRilRequestRejectIncomingCallMessage(
      std::shared_ptr<QcRilRequestRejectIncomingCallMessage> msg);
  void handleQcRilRequestGetCurrentSetupCallsMessage(
      std::shared_ptr<QcRilRequestGetCurrentSetupCallsMessage> msg);
  void handleQcRilRequestSetupAnswerMessage(
      std::shared_ptr<QcRilRequestSetupAnswerMessage> msg);
  void handleQcRilRequestSetLocalCallHoldMessage(
      std::shared_ptr<QcRilRequestSetLocalCallHoldMessage> msg);
  void handleQcRilRequestSetMuteMessage(
      std::shared_ptr<QcRilRequestSetMuteMessage> msg);
  void handleQcRilRequestGetMuteMessage(
      std::shared_ptr<QcRilRequestGetMuteMessage> msg);

  // public APIs
  void handleLimitedIndMessage(std::shared_ptr<VoiceLimitIndMessage> msg);
  void handleGetAnswerCallModeMessage(std::shared_ptr<VoiceGetAnswerCallModeMessage> msg);
  void handleCheckSpecificCallMessage(std::shared_ptr<VoiceCheckSpecificCallMessage> msg);
  void handleImsConnectedMessage(std::shared_ptr<VoiceImsConnectedMessage> msg);
  void handleRteProcessPendingCall(std::shared_ptr<VoiceRteProcessPendingCallMessage> msg);
  void handleCheckCallPresentMessage(std::shared_ptr<VoiceCheckCallPresentMessage> msg);
  void handleVoiceRequestPendingDialMessage(std::shared_ptr<VoiceRequestPendingDialMessage> msg);

  // NAS Indications
  void HandleVoiceRegStatusIndMessage(std::shared_ptr<Message> msg);
  void HandleServiceRteChangeIndMessage(std::shared_ptr<Message> msg);
  // NAS public API
  void handleGetNasInterface();

  void processDialCallResponse(CommonVoiceResponseData *data);
  void processHangupResponse(CommonVoiceResponseData *data);
  void processStartContDtmfResponse(CommonVoiceResponseData *data);
  void processStopContDtmfResponse(CommonVoiceResponseData *data);
  void processReleaseHeldOrWaitingResponse(CommonVoiceResponseData *data);
  void processReleaseActiveAcceptHeldOrWaitingResponse(CommonVoiceResponseData *data);
  void processHoldActiveAcceptWaitingOrHeldResponse(CommonVoiceResponseData *data);
  void processCommonManageCallsResponse(CommonVoiceResponseData *data);
  void processAnswerResponse(CommonVoiceResponseData *data);
  void processSetupAnswerResponse(CommonVoiceResponseData *data);
  void processQueryCallForwardResponse(CommonVoiceResponseData *data);
  void processGetCallWaitingResponse(CommonVoiceResponseData *data);
  void processGetClipResponse(CommonVoiceResponseData *data);
  void processGetClirResponse(CommonVoiceResponseData *data);
  void processGetColrResponse(CommonVoiceResponseData *data);
  void processGetColpResponse(CommonVoiceResponseData *data);
  void processGetCallBarringResponse(CommonVoiceResponseData *data);
  void processSetSupsServiceResponse(CommonVoiceResponseData *data);
  void processSetCallFwdSupsReqResponse(CommonVoiceResponseData *data);
  void processSetAllCallFwdSupsReqResponse(CommonVoiceResponseData *data);
  void processOrigUssdResponse(CommonVoiceResponseData *data);
  void processAnswerUssdResponse(CommonVoiceResponseData *data);
  void processCancelUssdResponse(CommonVoiceResponseData *data);
  void processSetSuppSvcNotificationResponse(CommonVoiceResponseData *data);
  void processSetCallBarringPasswordResponse(CommonVoiceResponseData *data);
  void processGetConfigResponse(CommonVoiceResponseData *data);
  void processSetConfigResponse(CommonVoiceResponseData *data);
  void processBurstDtmfResponse(CommonVoiceResponseData *data);
  void processSendFlashResponse(CommonVoiceResponseData *data);
  void processGetPreferredVoicePrivacyResponse(CommonVoiceResponseData *data);
  void processSetPreferredVoicePrivacyResponse(CommonVoiceResponseData *data);
  void processSetLocalCallHoldResponse(CommonVoiceResponseData *data);

  void processDialCommandOversightCompletion(CommonVoiceResponseData *data);
  void processHangupCommandOversightCompletion(CommonVoiceResponseData *data);
  void processReleaseHeldOrWaitingCommandOversightCompletion(CommonVoiceResponseData *data);
  void processAnswerCommandOversightCompletion(CommonVoiceResponseData *data);
  void processSetupAnswerCommandOversightCompletion(CommonVoiceResponseData *data);
};

VoiceModule *getVoiceModule();
