/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <framework/UnSolicitedMessage.h>
#include <framework/add_message_id.h>
#include <interfaces/voice/voice.h>
#include <optional>

/**
 * Unsol message to notify supplementary service indications
 */
class QcRilUnsolCdmaOtaProvisionStatusMessage : public UnSolicitedMessage,
                                         public add_message_id<QcRilUnsolCdmaOtaProvisionStatusMessage> {
 private:
  std::optional<RIL_CDMA_OTA_ProvisionStatus> mStatus;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilUnsolCdmaOtaProvisionStatusMessage";
  ~QcRilUnsolCdmaOtaProvisionStatusMessage() {}

  QcRilUnsolCdmaOtaProvisionStatusMessage() : UnSolicitedMessage(get_class_message_id()) {
    mName = MESSAGE_NAME;
  }

  std::shared_ptr<UnSolicitedMessage> clone() {
    std::shared_ptr<QcRilUnsolCdmaOtaProvisionStatusMessage> msg =
        std::make_shared<QcRilUnsolCdmaOtaProvisionStatusMessage>();
    return msg;
  }

  bool hasStatus() { return mStatus ? true : false; }
  RIL_CDMA_OTA_ProvisionStatus getStatus() { return *mStatus; }
  void setStatus(RIL_CDMA_OTA_ProvisionStatus val) { mStatus = val; }

  std::string dump() { return mName + "{}"; }
};
