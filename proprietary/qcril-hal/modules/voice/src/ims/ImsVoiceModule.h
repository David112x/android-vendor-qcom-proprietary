/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once

#include <framework/Module.h>
#include <framework/PendingMessageList.h>
#include <framework/QcrilInitMessage.h>
#include <interfaces/voice/QcRilRequestImsAddParticipantMessage.h>
#include <interfaces/voice/QcRilRequestImsAnswerMessage.h>
#include <interfaces/voice/QcRilRequestImsCancelModifyCallMessage.h>
#include <interfaces/voice/QcRilRequestImsConferenceMessage.h>
#include <interfaces/voice/QcRilRequestImsDeflectCallMessage.h>
#include <interfaces/voice/QcRilRequestImsDialMessage.h>
#include <interfaces/voice/QcRilRequestImsExplicitCallTransferMessage.h>
#include <interfaces/voice/QcRilRequestImsHangupMessage.h>
#include <interfaces/voice/QcRilRequestImsHoldMessage.h>
#include <interfaces/voice/QcRilRequestImsModifyCallConfirmMessage.h>
#include <interfaces/voice/QcRilRequestImsModifyCallInitiateMessage.h>
#include <interfaces/voice/QcRilRequestImsResumeMessage.h>
#include <interfaces/voice/QcRilRequestImsSendDtmfMessage.h>
#include <interfaces/voice/QcRilRequestImsSetUiTtyModeMessage.h>
#include <interfaces/voice/QcRilRequestImsStartDtmfMessage.h>
#include <interfaces/voice/QcRilRequestImsStopDtmfMessage.h>
#include <interfaces/voice/QcRilRequestQueryCallForwardMessage.h>
#include <interfaces/voice/QcRilRequestSetCallForwardMessage.h>
#include <modules/voice/VoiceModule.h>
#include <modules/voice/qcril_qmi_voice.h>

class ImsVoiceModule : public VoiceModule {
 private:
 public:
  ImsVoiceModule();
  ~ImsVoiceModule();
  void init();

 private:
  void handleQcRilRequestImsDialMessage(std::shared_ptr<QcRilRequestImsDialMessage> msg);
  void handleQcRilRequestImsHangupMessage(std::shared_ptr<QcRilRequestImsHangupMessage> msg);
  void handleQcRilRequestImsAnswerMessage(std::shared_ptr<QcRilRequestImsAnswerMessage> msg);
  void handleQcRilRequestImsHoldMessage(std::shared_ptr<QcRilRequestImsHoldMessage> msg);
  void handleQcRilRequestImsResumeMessage(std::shared_ptr<QcRilRequestImsResumeMessage> msg);
  void handleQcRilRequestImsAddParticipantMessage(
      std::shared_ptr<QcRilRequestImsAddParticipantMessage> msg);
  void handleQcRilRequestImsConferenceMessage(
      std::shared_ptr<QcRilRequestImsConferenceMessage> msg);
  void handleQcRilRequestImsModifyCallInitiateMessage(
      std::shared_ptr<QcRilRequestImsModifyCallInitiateMessage> msg);
  void handleQcRilRequestImsCancelModifyCallMessage(
      std::shared_ptr<QcRilRequestImsCancelModifyCallMessage> msg);
  void handleQcRilRequestImsModifyCallConfirmMessage(
      std::shared_ptr<QcRilRequestImsModifyCallConfirmMessage> msg);
  void handleQcRilRequestImsExplicitCallTransferMessage(
      std::shared_ptr<QcRilRequestImsExplicitCallTransferMessage> msg);
  void handleQcRilRequestImsDeflectCallMessage(
      std::shared_ptr<QcRilRequestImsDeflectCallMessage> msg);
  void handleQcRilRequestImsSetUiTtyModeMessage(
      std::shared_ptr<QcRilRequestImsSetUiTtyModeMessage> msg);
  void handleQcRilRequestImsSendDtmfMessage(std::shared_ptr<QcRilRequestImsSendDtmfMessage> msg);
  void handleQcRilRequestImsStartDtmfMessage(std::shared_ptr<QcRilRequestImsStartDtmfMessage> msg);
  void handleQcRilRequestImsStopDtmfMessage(std::shared_ptr<QcRilRequestImsStopDtmfMessage> msg);

  void processDialCallResponse(CommonVoiceResponseData *data);
  void processEndCallResponse(CommonVoiceResponseData *data);
  void processHoldResponse(CommonVoiceResponseData *data);
  void processResumeResponse(CommonVoiceResponseData *data);
  void processReleaseParticipantResponse(CommonVoiceResponseData *data);
  void processAnswerResponse(CommonVoiceResponseData *data);
  void processAddParticipantResponse(CommonVoiceResponseData *data);
  void processConferenceResponse(CommonVoiceResponseData *data);
  void processModifyCallInitiateResponse(CommonVoiceResponseData *data);
  void processCancelModifyCallResponse(CommonVoiceResponseData *data);
  void processModifyCallConfirmResponse(CommonVoiceResponseData *data);
  void processExplicitCallTransferResponse(CommonVoiceResponseData *data);
  void processDeflectCallResponse(CommonVoiceResponseData *data);
  void processSetUiTtyModeResponse(CommonVoiceResponseData *data);
  void processStartContDtmfResponse(CommonVoiceResponseData *data);
  void processStopContDtmfResponse(CommonVoiceResponseData *data);

  void processDialCommandOversightCompletion(CommonVoiceResponseData *data);
  void processAnswerCommandOversightCompletion(CommonVoiceResponseData *data);
  void processHangupCommandOversightCompletion(CommonVoiceResponseData *data);
};
