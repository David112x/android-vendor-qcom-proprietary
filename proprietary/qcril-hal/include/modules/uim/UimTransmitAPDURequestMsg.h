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
    Provides Card status of requested slot.

    @params
    mSlot            - slot on which request need to send
    mIsBasicChannel  - Request is for basic channel
    mAPDUData        - Streaming apdu data

*/
/*==========================================================================*/
class UimTransmitAPDURequestMsg : public SolicitedMessage<RIL_UIM_SIM_IO_Response>,
                                  public add_message_id<UimTransmitAPDURequestMsg>
{
  private:
    uint8_t                       mSlot;
    RIL_UIM_SIM_APDU              mAPDUData;
    bool                          mIsBasicChannel;
    bool                          mIsTelcomCard;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.uim.transmit_apdu_request";
    UimTransmitAPDURequestMsg() = delete;
    ~UimTransmitAPDURequestMsg() {}

    UimTransmitAPDURequestMsg(uint8_t slot,
                              bool is_basic_channel,
                              bool is_telecom_card,
                              const RIL_UIM_SIM_APDU &apdu_data) :
                              SolicitedMessage<RIL_UIM_SIM_IO_Response>(get_class_message_id())
    {
      mAPDUData = apdu_data;
      mSlot = slot;
      mIsBasicChannel = is_basic_channel;
      mIsTelcomCard = is_telecom_card;
      mName = MESSAGE_NAME;
    }

    inline const RIL_UIM_SIM_APDU *get_sim_apdu_data(void)
    {
      return &mAPDUData;
    }

    inline uint8_t get_slot(void)
    {
      return mSlot;
    }

    inline bool is_basic_channel(void)
    {
      return mIsBasicChannel;
    }

    inline bool is_telecom_card(void)
    {
      return mIsTelcomCard;
    }

    inline string dump()
    {
      return mName;
    }
}; /* UimTransmitAPDURequestMsg */
