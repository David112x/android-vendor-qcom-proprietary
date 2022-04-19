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
class UimGetRilPropertySyncMsg : public SolicitedSyncMessage<std::string>,
                                 public add_message_id<UimGetRilPropertySyncMsg>
{
  private:
    const std::string    mProperty;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.uim.get_ril_property";
    UimGetRilPropertySyncMsg() = delete;
    inline ~UimGetRilPropertySyncMsg() {}

    UimGetRilPropertySyncMsg(const std::string &adb_prop):
      SolicitedSyncMessage<std::string>(get_class_message_id()), mProperty(adb_prop)
    {
      mName = MESSAGE_NAME;
    }

    inline const string &get_property(void)
    {
      return mProperty;
    }

    inline string dump()
    {
      return mName + "Property: " + mProperty;
    }
};
