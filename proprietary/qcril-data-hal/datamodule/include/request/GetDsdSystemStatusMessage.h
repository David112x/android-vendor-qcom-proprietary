/******************************************************************************
#  Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once

#include "framework/SolicitedSyncMessage.h"
#include "framework/add_message_id.h"

#include "MessageCommon.h"
#include "data_system_determination_v01.h"

using namespace std;

namespace rildata {

struct DsdSystemStatusResult_t {
  dsd_system_status_ind_msg_v01 resp_ind;
  std::vector<dsd_apn_avail_sys_info_type_v01> apn_sys;
};

class GetDsdSystemStatusMessage : public SolicitedMessage<DsdSystemStatusResult_t>,
                                    public add_message_id<GetDsdSystemStatusMessage>
{
 public:
  static constexpr const char *MESSAGE_NAME = "GetDsdSystemStatusMessage";
  inline GetDsdSystemStatusMessage() :
    SolicitedMessage<DsdSystemStatusResult_t>(get_class_message_id())
  {
    mName = MESSAGE_NAME;
  }
  ~GetDsdSystemStatusMessage() {}

  string dump() { return mName; }
};

}