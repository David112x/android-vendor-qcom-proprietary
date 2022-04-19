/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include "framework/GenericCallback.h"
#include "framework/SolicitedMessage.h"
#include "framework/Message.h"
#include "framework/add_message_id.h"
#include "telephony/ril.h"

class NasSetPsAttachDetachMessage : public SolicitedMessage<RIL_Errno>,
                              public add_message_id<NasSetPsAttachDetachMessage>
{
  public:
    enum class PsAttachAction { PS_NONE = 0, PS_ATTACH, PS_DETACH, PS_ATTACH_ON_DEMAND,
            PS_ATTACH_FORCED , PS_ACTION_COUNTS};
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.nas.set_ps_attach_detach_request";
    NasSetPsAttachDetachMessage() = delete;
    ~NasSetPsAttachDetachMessage() = default;

    NasSetPsAttachDetachMessage(PsAttachAction action):
                          SolicitedMessage<RIL_Errno>(get_class_message_id())
    {
      mAction = action;
      mName = MESSAGE_NAME;
    }

    NasSetPsAttachDetachMessage(PsAttachAction action,
                            GenericCallback<RIL_Errno> *callback):
                          SolicitedMessage<RIL_Errno>(get_class_message_id())
    {
      mAction = action;
      mName = MESSAGE_NAME;
      setCallback(callback);
    }

    inline PsAttachAction getAction(void)
    {
      return mAction;
    }

    inline string dump()
    {
      static const char* actionStrings[] = {"PS_NONE", "PS_ATTACH", "PS_DETACH",
          "PS_ATTACH_ON_DEMOND", "PS_ATTACH_FORCED", "OUT_OF_BOUNDARY"};
      auto index = static_cast<int>(mAction);
      return mName + "{ action :" + actionStrings[index] + "}";
    }
  private:
    PsAttachAction mAction;
};
