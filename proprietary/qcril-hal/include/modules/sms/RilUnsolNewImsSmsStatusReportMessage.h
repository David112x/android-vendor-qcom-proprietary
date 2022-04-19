/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include "framework/UnSolicitedMessage.h"
#include "framework/Message.h"
#include "framework/add_message_id.h"

class RilUnsolNewImsSmsStatusReportMessage : public UnSolicitedMessage,
                public add_message_id<RilUnsolNewImsSmsStatusReportMessage> {
 private:
  uint32_t mMsgRef;
  RIL_RadioTechnologyFamily mRadioTech;
  std::vector<uint8_t> mGsmPayload;
  RIL_CDMA_SMS_Message mCdmaPayload;

 public:
  static constexpr const char *MESSAGE_NAME =
        "com.qualcomm.qti.qcril.sms.unsol_new_ims_sms_status_report";
  RilUnsolNewImsSmsStatusReportMessage() = delete;
  ~RilUnsolNewImsSmsStatusReportMessage();

  explicit inline RilUnsolNewImsSmsStatusReportMessage(uint32_t msgRef,
        RIL_RadioTechnologyFamily tech)
        : UnSolicitedMessage(get_class_message_id()),
        mMsgRef(msgRef), mRadioTech(tech) {
    mName = MESSAGE_NAME;
    memset(&mCdmaPayload, 0, sizeof(mCdmaPayload));
  }

  template<typename T>
  inline void setGsmPayload(T&& pdu) {
    if (mRadioTech == RADIO_TECH_3GPP) {
        mGsmPayload = std::forward<T>(pdu);
    }
  }

  void setCdmaPayload(const RIL_CDMA_SMS_Message& payload);

  std::shared_ptr<UnSolicitedMessage> clone() override;
  string dump() override;

  uint32_t getMessageRef();
  RIL_RadioTechnologyFamily getRadioTech();
  std::vector<uint8_t>& getGsmPayload();
  RIL_CDMA_SMS_Message& getCdmaPayload();
};
