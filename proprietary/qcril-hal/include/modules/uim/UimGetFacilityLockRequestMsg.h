/******************************************************************************
#  Copyright (c) 2017, 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include "framework/GenericCallback.h"
#include "framework/SolicitedMessage.h"
#include "framework/add_message_id.h"
#include "modules/uim/qcril_uim_types.h"

typedef struct
{
  RIL_UIM_Errno  ril_err;
  /* Lock_status is valid only if ril_err is RIL_UIM_E_SUCCESS */
  int            lock_status;
} RIL_UIM_GetFacilityLockResponseMsg;


class UimGetFacilityLockRequestMsg : public SolicitedMessage<RIL_UIM_GetFacilityLockResponseMsg>,
                                     public add_message_id<UimGetFacilityLockRequestMsg>
                                     
{
  private:
    string                    mPassword;
    string                    mAid;
    RIL_UIM_FacilityLockType  mFLockType;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.uim.get_facility_lock";

    UimGetFacilityLockRequestMsg() = default;
    inline ~UimGetFacilityLockRequestMsg() {}

    inline UimGetFacilityLockRequestMsg(const string &password, const string &aid,
                                        RIL_UIM_FacilityLockType flock_type) :
                                        SolicitedMessage<RIL_UIM_GetFacilityLockResponseMsg>(get_class_message_id())
    {
      mName = MESSAGE_NAME;
      mPassword = password;
      mAid = aid;
      mFLockType = flock_type;
    }

    inline const string &get_password()
    {
      return mPassword;
    }

    inline const string &get_aid()
    {
      return mAid;
    }

    inline RIL_UIM_FacilityLockType get_facility_lock_type()
    {
      return mFLockType;
    }

    inline string dump()
    {
      return mName + "{}";
    }
};
