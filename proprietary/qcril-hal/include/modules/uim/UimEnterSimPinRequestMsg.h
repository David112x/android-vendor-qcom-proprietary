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
     Sends SIM PIN to the modem when CardStatus has AppState as PIN required

    @params
     pin   - SIM PIN value
     aid   - AID value
*/
/*==========================================================================*/
class UimEnterSimPinRequestMsg : public SolicitedMessage<RIL_UIM_SIM_PIN_Response>,
                                 public add_message_id<UimEnterSimPinRequestMsg>
{
  private:
    RIL_UIM_PIN_Type mPinType;
    string           mPin;
    string           mAid;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.uim.enter_sim_pin_request";
    UimEnterSimPinRequestMsg() = delete;
    ~UimEnterSimPinRequestMsg() {}

    UimEnterSimPinRequestMsg(RIL_UIM_PIN_Type pinType,
                             const string     &pin,
                             const string     &aid) :
                             SolicitedMessage<RIL_UIM_SIM_PIN_Response>(get_class_message_id())
    {
      mPin = pin;
      mAid = aid;
      mPinType = pinType;
      mName = MESSAGE_NAME;
    }

    inline const string &get_pin(void)
    {
      return mPin;
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
}; /* UimEnterSimPinRequestMsg */
