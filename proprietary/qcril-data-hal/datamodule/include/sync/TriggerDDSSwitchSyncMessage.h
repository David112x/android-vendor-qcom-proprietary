/**
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **/

#include "framework/GenericCallback.h"
#include "framework/SolicitedSyncMessage.h"
#include "framework/add_message_id.h"
#include "MessageCommon.h"
#include "data_system_determination_v01.h"

namespace rildata {

class TriggerDDSSwitchSyncMessage : public SolicitedSyncMessage<SetPreferredDataModemResult_t>,
                                    public add_message_id<TriggerDDSSwitchSyncMessage> {
  public:
    static constexpr const char *MESSAGE_NAME = "TriggerDDSSwitchSyncMessage";
    TriggerDDSSwitchSyncMessage(int subId, dsd_dds_switch_type_enum_v01 switch_type):
      SolicitedSyncMessage<SetPreferredDataModemResult_t>(get_class_message_id()),
      mSubId(subId),mSwitchType(switch_type) {
      mName = MESSAGE_NAME;
    }
    ~TriggerDDSSwitchSyncMessage() {}

    int getSubId() { return mSubId; }
    dsd_dds_switch_type_enum_v01 getSwitchType() {return mSwitchType;}
    string dump() { return mName; }
  private:
    int mSubId;
    dsd_dds_switch_type_enum_v01 mSwitchType;
};
}
