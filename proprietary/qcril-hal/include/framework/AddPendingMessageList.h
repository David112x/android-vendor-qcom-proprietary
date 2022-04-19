/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once

#include <framework/PendingMessageList.h>


class AddPendingMessageList {
private:
  PendingMessageList mMessageList;

public:
  AddPendingMessageList(std::string name);
  PendingMessageList &getPendingMessageList();
  void clearPendingMessageList();
};
