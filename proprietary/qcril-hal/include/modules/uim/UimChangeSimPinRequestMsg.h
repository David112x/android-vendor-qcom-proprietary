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

/*===========================================================================*/
/*!
    @brief
     Sends requests to change the SIM PIN value

    @params
     pinType  - SIM PIN type
     newPin   - New SIM PIN value
     oldPin   - Old SIM PIN value
     aid      - AID value
*/
/*==========================================================================*/
class UimChangeSimPinRequestMsg : public SolicitedMessage<RIL_UIM_SIM_PIN_Response>,
                                  public add_message_id<UimChangeSimPinRequestMsg>
{
  private:
    string           mNewPin;
    string           mOldPin;
    string           mAid;
    RIL_UIM_PIN_Type mPinType;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.uim.change_sim_pin_request";
    UimChangeSimPinRequestMsg() = delete;
    ~UimChangeSimPinRequestMsg() {}

    UimChangeSimPinRequestMsg(RIL_UIM_PIN_Type pinType,
                              const string &newPin,
                              const string &oldPin,
                              const string &aid) :
                              SolicitedMessage<RIL_UIM_SIM_PIN_Response>(get_class_message_id())
    {
      mNewPin = newPin;
      mOldPin = oldPin;
      mAid = aid;
      mPinType = pinType;
      mName = MESSAGE_NAME;
    }

    inline const string &get_new_pin(void)
    {
      return mNewPin;
    }

    inline const string &get_old_pin(void)
    {
      return mOldPin;
    }

    inline const string &get_aid(void)
    {
      return mAid;
    }

    inline RIL_UIM_PIN_Type get_pin_type(void)
    {
      return mPinType;
    }

    inline string dump()
    {
      return mName;
    }
}; /* UimChangeSimPinRequestMsg */
