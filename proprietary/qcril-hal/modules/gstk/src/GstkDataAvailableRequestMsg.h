/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/SolicitedMessage.h"

/*===========================================================================

  CLASS:  GstkDataAvailableRequestMsg

===========================================================================*/
/*!
    @brief
    Data available request message used internal, not sent to any module

    @return
    None.
*/
/*=========================================================================*/
class GstkDataAvailableRequestMsg : public SolicitedMessage<void>,
                                    public add_message_id<GstkDataAvailableRequestMsg>
{
  private:
    uint32_t mBipId;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.gstk.data_available";
    GstkDataAvailableRequestMsg() = delete;
    ~GstkDataAvailableRequestMsg() = default;

    inline GstkDataAvailableRequestMsg(uint32_t  bip_id): SolicitedMessage<void>(get_class_message_id())
    {
      mBipId = bip_id;
      mName = MESSAGE_NAME;
    }

    inline uint32_t get_bip_id(void)
    {
      return mBipId;
    }

    inline string dump()
    {
      return mName + "{}";
    }
};
