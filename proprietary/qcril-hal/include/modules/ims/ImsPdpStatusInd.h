/******************************************************************************
#  Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once

#include <framework/UnSolicitedMessage.h>
#include <framework/add_message_id.h>
#include <framework/legacy.h>

class ImsPdpStatusInd : public UnSolicitedMessage,
                        public add_message_id<ImsPdpStatusInd> {
public:
  static constexpr const char *MESSAGE_NAME =
      "com.qualcomm.qti.qcril.ims.pdp_status_ind";

  ImsPdpStatusInd(bool isPdpConnected)
      : UnSolicitedMessage(get_class_message_id()) {
    mIsPdpConnected = isPdpConnected;
  }

  inline ~ImsPdpStatusInd() {}

  std::shared_ptr<UnSolicitedMessage> clone() {
    return (std::make_shared<ImsPdpStatusInd>(mIsPdpConnected));
  }

  string dump() { return ImsPdpStatusInd::MESSAGE_NAME; }

  inline bool isPdpConnected() { return mIsPdpConnected; }

private:
  bool mIsPdpConnected;
};
