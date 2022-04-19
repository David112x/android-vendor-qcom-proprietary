/*
 * Copyright (c) 2019-2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "UnSolMessages/VoiceIndMessage.h"

using namespace rildata;

VoiceIndMessage::VoiceIndMessage(VoiceIndResp voiceResp):UnSolicitedMessage(get_class_message_id())
{
  mVoiceResp.voiceSubId = voiceResp.voiceSubId;
  mVoiceResp.voiceCallState = voiceResp.voiceCallState;
  mVoiceResp.voiceCallRat = voiceResp.voiceCallRat;
  mVoiceResp.voiceCallType = voiceResp.voiceCallType;
}

VoiceIndMessage::~VoiceIndMessage()
{
}

std::shared_ptr<UnSolicitedMessage> VoiceIndMessage::clone()
{
  return std::make_shared<VoiceIndMessage>(mVoiceResp);
}

string VoiceIndMessage::dump()
{
  return MESSAGE_NAME;
}

VoiceIndResp VoiceIndMessage::getParam()
{
  return mVoiceResp;
}

SubId VoiceIndMessage::getSubId()
{
  return mVoiceResp.voiceSubId;
}

VoiceCallStateEnum VoiceIndMessage::getCallState()
{
  return mVoiceResp.voiceCallState;
}

VoiceCallTypeEnum VoiceIndMessage::getCallType()
{
  return mVoiceResp.voiceCallType;
}

VoiceCallRATEnum VoiceIndMessage::getCallRat()
{
	return mVoiceResp.voiceCallRat;
}
