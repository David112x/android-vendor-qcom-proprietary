/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <framework/UnSolicitedMessage.h>
#include <framework/add_message_id.h>
#include <interfaces/voice/voice.h>
#include <optional>

/*
 * Unsol message to notify Extended Burst Type International Info
 */
class QcRilUnsolExtBurstIntlMessage : public UnSolicitedMessage,
                                      public add_message_id<QcRilUnsolExtBurstIntlMessage> {
 private:
  uint16_t mMcc;             /**<   Mobile country code.  */
  uint8_t mDataBurstSubType; /**<   Data burst subtype.  */
  uint8_t mChargeInd;        /**<   Charge indication.  */
  uint8_t mSubUnit;          /**<   Unit call time in 1/10 second.  */
  uint8_t mUnit;             /**<   Unit call time in seconds.  */

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilUnsolExtBurstIntlMessage";
  ~QcRilUnsolExtBurstIntlMessage() {}

  QcRilUnsolExtBurstIntlMessage(uint16_t mcc, uint8_t dbSubType, uint8_t chgInd, uint8_t subUnit,
                                uint8_t unit)
      : UnSolicitedMessage(get_class_message_id()),
        mMcc(mcc),
        mDataBurstSubType(dbSubType),
        mChargeInd(chgInd),
        mSubUnit(subUnit),
        mUnit(unit) {
    mName = MESSAGE_NAME;
  }

  std::shared_ptr<UnSolicitedMessage> clone() {
    std::shared_ptr<QcRilUnsolExtBurstIntlMessage> msg =
        std::make_shared<QcRilUnsolExtBurstIntlMessage>(mMcc, mDataBurstSubType, mChargeInd,
                                                        mSubUnit, mUnit);
    return msg;
  }

  uint16_t getMcc() { return mMcc; }
  uint8_t getDataBurstSubType() { return mDataBurstSubType; }
  uint8_t getChargeInd() { return mChargeInd; }
  uint8_t getSubUnit() { return mSubUnit; }
  uint8_t getUnit() { return mUnit; }

  std::string dump() {
    return mName + "{" + "mMcc=" + std::to_string(mMcc) +
           " mDataBurstSubType=" + std::to_string(mDataBurstSubType) +
           " mChargeInd=" + std::to_string(mChargeInd) + "mSubUnit=" + std::to_string(mSubUnit) +
           " mUnit=" + std::to_string(mUnit) + "}";
  }
};
