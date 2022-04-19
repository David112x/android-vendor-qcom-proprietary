/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include "framework/GenericCallback.h"
#include "framework/SolicitedMessage.h"
#include "framework/add_message_id.h"
#include "qcril_uim_types.h"

class UimSetFacilityLockRequestMsg : public SolicitedMessage<RIL_UIM_SIM_PIN_Response>,
                                     public add_message_id<UimSetFacilityLockRequestMsg>
{
  private:
    string                      mPassword;
    string                      mAid;
    RIL_UIM_FacilityLockType    mFLockType;
    bool                        mStatus;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.uim.set_facility_lock";

    UimSetFacilityLockRequestMsg() = default;
    inline ~UimSetFacilityLockRequestMsg() {}

    inline UimSetFacilityLockRequestMsg(const string &password,
                                        const string &aid,
                                        bool status,
                                        RIL_UIM_FacilityLockType flock_type) :
                                        SolicitedMessage<RIL_UIM_SIM_PIN_Response>(get_class_message_id())
    {
      mName = MESSAGE_NAME;
      mPassword = password;
      mAid = aid;
      mFLockType = flock_type;
      mStatus = status;
    }

    inline const string &get_password(void)
    {
      return mPassword;
    }

    inline const string &get_aid(void)
    {
      return mAid;
    }

    inline RIL_UIM_FacilityLockType get_facility_lock_type(void)
    {
      return mFLockType;
    }

    inline bool get_status(void)
    {
      return mStatus;
    }

    inline string dump()
    {
      return mName + "{}";
    }
};
