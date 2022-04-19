/******************************************************************************
#  Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include <modules/uim/UimGetMccMncRequestMsg.h>
#include <framework/GenericCallback.h>
#include "local/DataModule.h"

class DataGetMccMncCallback : public GenericCallback<qcril_uim_mcc_mnc_info_type> {
  public:
    inline DataGetMccMncCallback(string clientToken)
      : GenericCallback(clientToken){};

    Message::Callback *clone() override;

    void onResponse(std::shared_ptr<Message> solicitedMsg, Status status,
        std::shared_ptr<qcril_uim_mcc_mnc_info_type> responseDataPtr) override;

    Status getStatus();

    Status mStatus;
    std::shared_ptr<void> mResponseData;
};
