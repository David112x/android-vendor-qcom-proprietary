/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include "framework/SolicitedSyncMessage.h"
#include "framework/add_message_id.h"
#include "string"


/*===========================================================================*/
/*!
    @brief
    Message used to read the adb property.

    mProperty  - ADB property

    Retruns adb property value
*/
/*==========================================================================*/
class UimSetRilPropertySyncMsg : public SolicitedSyncMessage<void>,
                                 public add_message_id<UimSetRilPropertySyncMsg>
{
  private:
    const std::string    mProperty;
    const std::string    mValue;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.uim.set_ril_property";
    UimSetRilPropertySyncMsg() = delete;
    inline ~UimSetRilPropertySyncMsg() {}

    UimSetRilPropertySyncMsg(const std::string &adb_prop, const std::string &value):
      SolicitedSyncMessage<void>(get_class_message_id()), mProperty(adb_prop), mValue(value)
    {
      mName = MESSAGE_NAME;
    }

    inline const string &get_property(void)
    {
      return mProperty;
    }

    inline const string &get_value(void)
    {
      return mValue;
    }

    inline string dump()
    {
      return mName + "Property: " + mProperty + "Value: " + mValue;
    }
};
