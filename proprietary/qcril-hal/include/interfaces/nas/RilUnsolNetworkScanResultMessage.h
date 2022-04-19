/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <framework/UnSolicitedMessage.h>
#include <framework/add_message_id.h>

/*
 * Unsol message to report network scan result
 *
 */
class RilUnsolNetworkScanResultMessage : public UnSolicitedMessage,
            public add_message_id<RilUnsolNetworkScanResultMessage> {
 private:
  RIL_ScanStatus mStatus;
  RIL_Errno mError;
  std::vector<RIL_CellInfo_v12> mNetworkInfo;

 public:
  static constexpr const char *MESSAGE_NAME = "RIL_UNSOL_NETWORK_SCAN_RESULT";
  ~RilUnsolNetworkScanResultMessage() { }


  template<typename T>
  RilUnsolNetworkScanResultMessage(RIL_ScanStatus status, RIL_Errno error, T info)
      : UnSolicitedMessage(get_class_message_id()), mStatus(status), mError(error),
        mNetworkInfo(std::forward<T>(info)) { }

  std::shared_ptr<UnSolicitedMessage> clone() {
    return std::make_shared<RilUnsolNetworkScanResultMessage>(
        mStatus, mError, mNetworkInfo);
  }

  const std::vector<RIL_CellInfo_v12>& getNetworkInfo() { return mNetworkInfo; }
  RIL_ScanStatus getStatus() { return mStatus; }
  RIL_Errno getError() { return mError; }

  string dump() {
    return RilUnsolNetworkScanResultMessage::MESSAGE_NAME;
  }
};
