/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <framework/add_message_id.h>
#include <interfaces/QcRilRequestMessage.h>

/*
 * Request to get the ims rtp stats from lower layers
 *
 * Response:
 *   errorCode    : Valid error codes
 *                   SUCCESS
 *                   GENERIC_FAILURE
 *   responseData : std::shared_ptr<qcril::interfaces::RtpStatisticsData>
 */

class QcRilRequestImsGetRtpStatsMessage : public QcRilRequestMessage,
                                          public add_message_id<QcRilRequestImsGetRtpStatsMessage> {
private:

public:
  static constexpr const char *MESSAGE_NAME = "QcRilRequestImsGetRtpStatsMessage";

  QcRilRequestImsGetRtpStatsMessage() = delete;

  ~QcRilRequestImsGetRtpStatsMessage() {};

  inline QcRilRequestImsGetRtpStatsMessage(std::shared_ptr<MessageContext> context)
      : QcRilRequestMessage(get_class_message_id(), context) {
    mName = MESSAGE_NAME;
  }
};
