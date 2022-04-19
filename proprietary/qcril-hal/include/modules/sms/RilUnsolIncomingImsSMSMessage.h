/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include "telephony/ril.h"
#include "framework/UnSolicitedMessage.h"
#include "framework/Message.h"
#include "framework/add_message_id.h"
#include "interfaces/voice/voice.h"

class RilUnsolIncomingImsSMSMessage : public UnSolicitedMessage,
                public add_message_id<RilUnsolIncomingImsSMSMessage> {
 public:
  static constexpr const char *MESSAGE_NAME =
        "com.qualcomm.qti.qcril.sms.incoming_ims_sms_message";

  RilUnsolIncomingImsSMSMessage() = delete;
  ~RilUnsolIncomingImsSMSMessage();

  explicit inline RilUnsolIncomingImsSMSMessage(RIL_RadioTechnologyFamily tech)
      : UnSolicitedMessage(get_class_message_id()), mTech(tech),
      mVerStatus(qcril::interfaces::VerificationStatus::NONE) {
    mName = MESSAGE_NAME;
    memset(&mCdmaPayload, 0, sizeof(mCdmaPayload));
  }

  template<typename T>
  inline void setGsmPayload(T&& pdu) {
    if (mTech == RADIO_TECH_3GPP) {
        mGsmPayload = std::forward<T>(pdu);
    }
  }
  void setCdmaPayload(const RIL_CDMA_SMS_Message& payload);
  void setVerificationStatus(qcril::interfaces::VerificationStatus status);

  std::shared_ptr<UnSolicitedMessage> clone() override;
  string dump() override;

  RIL_RadioTechnologyFamily getTech();
  const std::vector<uint8_t>& getGsmPayload();
  RIL_CDMA_SMS_Message& getCdmaPayload();
  qcril::interfaces::VerificationStatus getVerificationStatus();

 private:
  const RIL_RadioTechnologyFamily mTech;
  qcril::interfaces::VerificationStatus mVerStatus;
  std::vector<uint8_t> mGsmPayload;
  RIL_CDMA_SMS_Message mCdmaPayload;

};
