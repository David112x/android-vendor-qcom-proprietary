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
#include "vector"

/*===========================================================================*/
/*!
    @brief
    Remote simlock request message

    Retruns RIL_UIM_PhySlotMap shared pointer
*/
/*==========================================================================*/
class UimRemoteSimlockRequestMsg : public SolicitedMessage<RIL_UIM_RemoteSimlockResponse>,
                                   public add_message_id<UimRemoteSimlockRequestMsg>
{
  private:
    RIL_UIM_RemoteSimlockOperationType  mOpType;
    const std::vector<uint8_t>          mSimlockData;
  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.uim.remote_simlock_request";
    UimRemoteSimlockRequestMsg() = delete;
    ~UimRemoteSimlockRequestMsg() = default;

    UimRemoteSimlockRequestMsg(RIL_UIM_RemoteSimlockOperationType op_type,
                               const std::vector<uint8_t> &simlock_data):
                               SolicitedMessage<RIL_UIM_RemoteSimlockResponse>(get_class_message_id()),
                               mSimlockData(simlock_data)
    {
      mOpType = op_type;
      mName = MESSAGE_NAME;
    }

    inline const std::vector<uint8_t> &get_simlock_data(void)
    {
      return mSimlockData;
    }

    inline RIL_UIM_RemoteSimlockOperationType get_op_type(void)
    {
      return mOpType;
    }

    inline string dump()
    {
      return mName;
    }
};
