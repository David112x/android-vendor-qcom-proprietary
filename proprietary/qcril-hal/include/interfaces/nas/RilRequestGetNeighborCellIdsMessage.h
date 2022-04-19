/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/add_message_id.h"
#include "interfaces/QcRilRequestMessage.h"
#include "telephony/ril.h"

/* Request to get neighboring cell ids Message
   @Receiver: NasModule
   */

class RilRequestGetNeighborCellIdsMessage : public QcRilRequestMessage,
    public add_message_id<RilRequestGetNeighborCellIdsMessage> {
public:
    static constexpr const char *MESSAGE_NAME = "RIL_REQUEST_GET_NEIGHBORING_CELL_IDS";

    RilRequestGetNeighborCellIdsMessage() = delete;
    ~RilRequestGetNeighborCellIdsMessage() {}

    inline RilRequestGetNeighborCellIdsMessage(std::shared_ptr<MessageContext> context)
        : QcRilRequestMessage(get_class_message_id(), context) {
      mName = MESSAGE_NAME;
    }
};
