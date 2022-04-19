/******************************************************************************
#  Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include "framework/GenericCallback.h"
#include "framework/SolicitedMessage.h"
#include "framework/add_message_id.h"
#include "modules/uim/qcril_uim_types.h"

/*===========================================================================*/
/*!
    @brief
    Provides GID of requested aid of a slot.

    mSlot     - In Slot on which GID needed
    mAid      - In AID string on which GID is needed
    mGidType  - GID1 or GID2

    Retruns unit8_t shared pointer
*/
/*==========================================================================*/
class UimGetGIDRequestMsg : public SolicitedMessage<RIL_UIM_GIDResponse>,
                            public add_message_id<UimGetGIDRequestMsg>
{
  public:
    enum gid_type
    {
      GID1,
      GID2
    };

  private:
    uint8_t            mSlot;
    string             mAid;
    RIL_UIM_AppType    mAppType;
    gid_type           mGidType;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.uim.get_gid_request";

    UimGetGIDRequestMsg() = delete;
    ~UimGetGIDRequestMsg() {}

    UimGetGIDRequestMsg(uint8_t slot, const char *aid, RIL_UIM_AppType app_type, gid_type gid,
                        GenericCallback<RIL_UIM_GIDResponse> *callback) :
                        SolicitedMessage<RIL_UIM_GIDResponse>(get_class_message_id())
    {
      mSlot = slot;
      if (aid != NULL)
      {
        mAid = aid;
      }
      else
      {
        mAid.clear();
      }
      mAppType = app_type;
      mName = MESSAGE_NAME;
      mGidType = gid;
      setCallback(callback);
    }

    inline uint8_t get_slot(void)
    {
      return mSlot;
    }

    inline const string &get_aid(void)
    {
      return mAid;
    }

    inline gid_type get_gid_type()
    {
      return mGidType;
    }

    inline RIL_UIM_AppType get_app_type()
    {
      return mAppType;
    }

    inline string dump()
    {
      return mName + "{ slot :" + std::to_string(mSlot) + "} AidPtr: " + mAid;
    }
};
