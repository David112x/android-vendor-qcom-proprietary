/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once

#include "framework/Message.h"
#include "framework/UnSolicitedMessage.h"
#include "framework/add_message_id.h"

class NasUpperLayerIndInfoIndMessage : public UnSolicitedMessage,
                              public add_message_id<NasUpperLayerIndInfoIndMessage>
{
  private:
    bool mPlmnsListInfoStatus;
    bool mUpperLayerIndInfoStatus;

  public:
    static constexpr const char *MESSAGE_NAME =
        "com.qualcomm.qti.qcril.nas.upper_layer_ind_info_indication";
    NasUpperLayerIndInfoIndMessage() = delete;
    ~NasUpperLayerIndInfoIndMessage() = default;

    inline NasUpperLayerIndInfoIndMessage(bool plmnListStatus, bool upliStatus):
                       UnSolicitedMessage(get_class_message_id())
    {
      mPlmnsListInfoStatus = plmnListStatus;
      mUpperLayerIndInfoStatus = upliStatus;
      mName = MESSAGE_NAME;
    }

    inline std::shared_ptr<UnSolicitedMessage> clone()
    {
      return std::static_pointer_cast<UnSolicitedMessage>(
              std::make_shared<NasUpperLayerIndInfoIndMessage>(mPlmnsListInfoStatus, mUpperLayerIndInfoStatus));
    }

    inline string dump()
    {
      return mName + "{}" + "PLMNS INFO LIST Available: " + std::to_string(mPlmnsListInfoStatus) +
          " UPPER LAYER IND INFO Available: " + std::to_string(mUpperLayerIndInfoStatus);
    }

    inline bool isPlmnsListInfoAvailable(void)
    {
      return mPlmnsListInfoStatus;
    }

    inline bool isUpperLayerIndInfoAvailable(void)
    {
      return mUpperLayerIndInfoStatus;
    }
};
