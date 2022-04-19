/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include <framework/GenericCallback.h>

class DataNasPsAttachDetachCallback : public GenericCallback<RIL_Errno> {
  public:
  inline DataNasPsAttachDetachCallback(string clientToken)
    : GenericCallback(clientToken){};
  Message::Callback *clone() override;

  void onResponse(std::shared_ptr<Message> solicitedMsg, Status status,
    std::shared_ptr<RIL_Errno> responseDataPtr) override;
  Status getStatus();

  Status mStatus;
  std::shared_ptr<void> mResponseData;
};
