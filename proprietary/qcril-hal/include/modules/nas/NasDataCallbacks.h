/******************************************************************************
#  Copyright (c) 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include <framework/GenericCallback.h>
#include <modules/android/RilRequestMessage.h>
#include "SetLinkCapRptCriteriaMessage.h"
#include <DataCommon.h>

class NasDataGetDdsSubIdCallback : public GenericCallback<DDSSubIdInfo> {
private:
  uint32_t mMsgToken;  // use uint32_t to store uint16 token. Use uint32_t (-1) to indicate an invalid token
  bool from_allow_data;
public:
  static constexpr uint32_t INVALID_MSG_TOKEN = (uint32_t)(-1);
  inline NasDataGetDdsSubIdCallback(string clientToken,
        uint32_t msgToken,  boolean from_allow_data):
        GenericCallback(clientToken), mMsgToken(msgToken), from_allow_data(from_allow_data) {};

  inline ~NasDataGetDdsSubIdCallback() {};

  Message::Callback *clone() override;

  void onResponse(std::shared_ptr<Message> solicitedMsg, Status status,
      std::shared_ptr<DDSSubIdInfo> responseDataPtr) override;
};

class NasDataRequestDdsSwitchCallback : public GenericCallback<RIL_Errno> {
private:
    uint32_t mMsgToken;  // use uint32_t to store uint16 token. Use uint32_t (-1) to indicate an invalid token
public:
    inline NasDataRequestDdsSwitchCallback(string clientToken,
        uint32_t msgToken) :
        GenericCallback(clientToken), mMsgToken(msgToken) {};

    inline ~NasDataRequestDdsSwitchCallback() {};

    Message::Callback *clone() override;

    void onResponse(std::shared_ptr<Message> solicitedMsg, Status status,
        std::shared_ptr<RIL_Errno> responseDataPtr) override;
};
