/*
 * Copyright (c) 2019-2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#pragma once
#include "framework/Message.h"
#include "framework/UnSolicitedMessage.h"
#include "framework/add_message_id.h"
#include "MessageCommon.h"
#include "voice_service_v02.h"

namespace rildata {

enum class VoiceCallTypeEnum : int {
   CALL_TYPE_ENUM_MIN_ENUM_VAL = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
   CALL_TYPE_VOICE = 0x00, /**<  Voice \n  */
   CALL_TYPE_VOICE_FORCED = 0x01, /**<  Avoid modem call classification \n  */
   CALL_TYPE_VOICE_IP = 0x02, /**<  Voice over IP  \n  */
   CALL_TYPE_VT = 0x03, /**<  Videotelephony call over IP \n  */
   CALL_TYPE_VIDEOSHARE = 0x04, /**<  Videoshare \n  */
   CALL_TYPE_TEST = 0x05, /**<  Test call type \n  */
   CALL_TYPE_OTAPA = 0x06, /**<  OTAPA \n  */
   CALL_TYPE_STD_OTASP = 0x07, /**<  Standard OTASP \n  */
   CALL_TYPE_NON_STD_OTASP = 0x08, /**<  Nonstandard OTASP \n  */
   CALL_TYPE_EMERGENCY = 0x09, /**<  Emergency \n  */
   CALL_TYPE_SUPS = 0x0A, /**<  Supplementary service \n  */
   CALL_TYPE_EMERGENCY_IP = 0x0B, /**<  Emergency VoIP \n  */
   CALL_TYPE_ECALL = 0x0C, /**<  eCall \n  */
   CALL_TYPE_ENUM_MAX_ENUM_VAL = 2147483647

};

enum class VoiceCallStateEnum : int {
   CALL_STATE_ENUM_MIN_ENUM_VAL = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
   CALL_STATE_ORIGINATING = 0x01, /**<  Origination \n  */
   CALL_STATE_INCOMING = 0x02, /**<  Incoming \n  */
   CALL_STATE_CONVERSATION = 0x03, /**<  Conversation \n  */
   CALL_STATE_CC_IN_PROGRESS = 0x04, /**<  Call is originating but waiting for call control to complete  \n  */
   CALL_STATE_ALERTING = 0x05, /**<  Alerting \n  */
   CALL_STATE_HOLD = 0x06, /**<  Hold \n  */
   CALL_STATE_WAITING = 0x07, /**<  Waiting \n  */
   CALL_STATE_DISCONNECTING = 0x08, /**<  Disconnecting \n  */
   CALL_STATE_END = 0x09, /**<  End \n  */
   CALL_STATE_SETUP = 0x0A, /**<  MT call is in Setup state in 3GPP \n  */
   CALL_STATE_ENUM_MAX_ENUM_VAL = 2147483647
};

enum class VoiceCallRATEnum : int {
   CALL_MODE_ENUM_MIN_ENUM_VAL = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
   CALL_MODE_NO_SRV = 0x00, /**<  No service \n  */
   CALL_MODE_CDMA = 0x01, /**<  CDMA \n  */
   CALL_MODE_GSM = 0x02, /**<  GSM \n  */
   CALL_MODE_UMTS = 0x03, /**<  UMTS \n  */
   CALL_MODE_LTE = 0x04, /**<  LTE \n  */
   CALL_MODE_TDS = 0x05, /**<  TD-SCDMA \n  */
   CALL_MODE_UNKNOWN = 0x06, /**<  Unknown \n  */
   CALL_MODE_WLAN = 0x07, /**<  WLAN  */
   CALL_MODE_ENUM_MAX_ENUM_VAL = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/

};

enum class SubId : int {
  UNSPECIFIED = -1,
  PRIMARY = 0,
  SECONDARY = 1,
  TERTIARY = 2,
};

struct VoiceIndResp {
  SubId voiceSubId;
  VoiceCallStateEnum voiceCallState;
  VoiceCallRATEnum voiceCallRat;
  VoiceCallTypeEnum voiceCallType;
};

/********************** Class Definitions *************************/
class VoiceIndMessage: public UnSolicitedMessage,
                         public add_message_id<VoiceIndMessage> {
private:
  VoiceIndResp mVoiceResp;

public:
  static constexpr const char *MESSAGE_NAME = "VoiceIndMessage";
  VoiceIndMessage() = delete;
  VoiceIndMessage(VoiceIndResp voiceResp);
  ~VoiceIndMessage();

  std::shared_ptr<UnSolicitedMessage> clone();
  string dump();
  VoiceIndResp getParam();
  SubId getSubId();
  VoiceCallStateEnum getCallState();
  VoiceCallTypeEnum getCallType();
  VoiceCallRATEnum getCallRat();
};

} //namespace
