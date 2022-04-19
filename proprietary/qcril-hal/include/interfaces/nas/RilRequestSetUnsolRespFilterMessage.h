/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/add_message_id.h"
#include "interfaces/QcRilRequestMessage.h"

/*
 * Request to set the unsolicited response filter.  This is used to prevent unnecessary
 * application processor wake up for power saving purposes by suppressing the unsolicited
 * responses in certain scenarios
 *
 * @Receiver: NasModule
 *
 * Response:
 *   errorCode    : Valid error codes
 *   responseData : nullptr
 */

class RilRequestSetUnsolRespFilterMessage
    : public QcRilRequestMessage,
      public add_message_id<RilRequestSetUnsolRespFilterMessage> {
public:
  static constexpr const char *MESSAGE_NAME = "RilRequestSetUnsolRespFilterMessage";

  RilRequestSetUnsolRespFilterMessage() = delete;
  ~RilRequestSetUnsolRespFilterMessage() {}

  explicit inline RilRequestSetUnsolRespFilterMessage(std::shared_ptr<MessageContext> context,
                                                      int32_t indFilter)
      : QcRilRequestMessage(get_class_message_id(), context), mIndicationFilter(indFilter) {
    mName = MESSAGE_NAME;
  }

  void setIndicationFilter(int32_t val) { mIndicationFilter = val; }
  int32_t getIndicationFilter() { return mIndicationFilter; }

private:
  int32_t mIndicationFilter;
};
