/******************************************************************************
#  Copyright (c) 2020 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <framework/add_message_id.h>
#include <interfaces/QcRilRequestMessage.h>
#include <optional>
#include <string>

/*
 * Sets the minimum time between when RIL_UNSOL_CELL_INFO_LIST should be invoked.
 * A value of 0, means invoke RIL_UNSOL_CELL_INFO_LIST when any of the reported
 * information changes. Setting the value to INT_MAX(0x7fffffff) means never issue
 * a RIL_UNSOL_CELL_INFO_LIST.
 *
 * Response:
 *   errorCode    : Valid error codes
 *   responseData : nullptr
 */
class RilRequestSetUnsolCellInfoListRateMessage
    : public QcRilRequestMessage,
      public add_message_id<RilRequestSetUnsolCellInfoListRateMessage> {
 private:
  std::optional<int32_t> mRate;  // minimum time in milliseconds

 public:
  static constexpr const char* MESSAGE_NAME = "RilRequestSetUnsolCellInfoListRateMessage";

  RilRequestSetUnsolCellInfoListRateMessage() = delete;

  ~RilRequestSetUnsolCellInfoListRateMessage() {
  }

  inline RilRequestSetUnsolCellInfoListRateMessage(std::shared_ptr<MessageContext> context)
      : QcRilRequestMessage(get_class_message_id(), context) {
    mName = MESSAGE_NAME;
  }

  bool hasRate() {
    return mRate ? true : false;
  }
  int32_t getRate() {
    return *mRate;
  }
  void setRate(int32_t val) {
    mRate = val;
  }

  virtual string dump() {
    std::string os;
    os += QcRilRequestMessage::dump();
    os += "{";
    os += ".mRate = ";
    os += mRate ? std::to_string(*mRate) : "<invalid";
    os += "}";
    return os;
  }
};
