/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once

#include "framework/SolicitedSyncMessage.h"
#include "framework/add_message_id.h"
#include "wireless_data_service_v01.h"

class SetWDSProfileInfoSyncMessage : public SolicitedSyncMessage<int>,
                                    public add_message_id<SetWDSProfileInfoSyncMessage>
{
public:

  static constexpr const char *MESSAGE_NAME =
          "com.qualcomm.qti.qcril.data.SETWDS_PROFILE_INFO_SYNC";

  inline SetWDSProfileInfoSyncMessage(GenericCallback<int> *callback) :
    SolicitedSyncMessage<int>(get_class_message_id())
  {
    std::ignore=callback;
    mName = MESSAGE_NAME;
  }

  ~SetWDSProfileInfoSyncMessage()
  {

  }

  void setParams(int32_t profileType, uint8_t profileIndex)
  {
    prof_id.profile_type = (wds_profile_type_enum_v01)profileType;
    prof_id.profile_index = profileIndex;
  }

  void getParams(wds_get_profile_settings_req_msg_v01 *profile_id)
  {
    profile_id->profile = prof_id;
  }

  string dump()
  {
    return mName;
  }

private:

  wds_profile_identifier_type_v01 prof_id;
};