/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once

#include "framework/add_message_id.h"
#include "interfaces/QcRilRequestMessage.h"
#include <string>


/* Request to select network manually
   @Receiver: NasModule
   */

class RilRequestSetNetworkSelectionManualMessage : public QcRilRequestMessage,
                              public add_message_id<RilRequestSetNetworkSelectionManualMessage>
{

public:
  static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.nas.set_network_selection_manual";
  RilRequestSetNetworkSelectionManualMessage() = delete;
  ~RilRequestSetNetworkSelectionManualMessage() {}

  template<typename T>
  explicit inline RilRequestSetNetworkSelectionManualMessage(std::shared_ptr<MessageContext> context,
        T operatorNumeric) : QcRilRequestMessage(get_class_message_id(), context),
        mOperatorNumeric(std::forward<T>(operatorNumeric)) {
    mName = MESSAGE_NAME;
  }

  const std::string& getOperatorNumeric() { return mOperatorNumeric; }
private:
  std::string mOperatorNumeric;
};

