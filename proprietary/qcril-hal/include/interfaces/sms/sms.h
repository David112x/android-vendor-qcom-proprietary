/******************************************************************************
#  Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once

#include <string>
#include <vector>
#include "interfaces/common.h"

namespace qcril {
namespace interfaces {

enum class MessageType {
  UNKNOWN,
  NONE,
  VOICE,
  VIDEO,
  FAX,
  PAGER,
  MULTIMEDIA,
  TEXT,
};
inline std::string toString(const MessageType &o) {
  if (o == MessageType::UNKNOWN) {
    return "UNKNOWN";
  }
  if (o == MessageType::NONE) {
    return "NONE";
  }
  if (o == MessageType::VOICE) {
    return "VOICE";
  }
  if (o == MessageType::FAX) {
    return "FAX";
  }
  if (o == MessageType::PAGER) {
    return "PAGER";
  }
  if (o == MessageType::MULTIMEDIA) {
    return "MULTIMEDIA";
  }
  if (o == MessageType::TEXT) {
    return "TEXT";
  }
  return "<invalid>";
}

enum class MessagePriority {
  UNKNOWN,
  LOW,
  NORMAL,
  URGENT,
};

inline std::string toString(const MessagePriority &o) {
  if (o == MessagePriority::UNKNOWN) {
    return "UNKNOWN";
  }
  if (o == MessagePriority::LOW) {
    return "LOW";
  }
  if (o == MessagePriority::NORMAL) {
    return "NORMAL";
  }
  if (o == MessagePriority::URGENT) {
    return "URGENT";
  }
  return "<invalid>";
}

class MessageSummary {
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(MessageType, MessageType);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(uint32_t, NewMessageCount);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(uint32_t, OldMessageCount);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(uint32_t, NewUrgentMessageCount);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(uint32_t, OldUrgentMessageCount);

 public:
  MessageSummary() {
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(MessageType);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(NewMessageCount);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(OldMessageCount);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(NewUrgentMessageCount);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(OldUrgentMessageCount);
  }
  ~MessageSummary() {}
  MessageSummary(const MessageSummary &from) {
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, MessageType);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, NewMessageCount);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, OldMessageCount);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, NewUrgentMessageCount);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, OldUrgentMessageCount);
  }
};

class MessageDetails {
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(std::string, ToAddress);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(std::string, FromAddress);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(std::string, Subject);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(std::string, Date);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(MessagePriority, Priority);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(std::string, MessageId);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(MessageType, MessageType);

 public:
  MessageDetails() {
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(ToAddress);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(FromAddress);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(Subject);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(Date);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(Priority);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(MessageId);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(MessageType);
  }
  ~MessageDetails() {}
  MessageDetails(const MessageDetails &from) {
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, ToAddress);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, FromAddress);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, Subject);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, Date);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, Priority);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, MessageId);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, MessageType);
  }
};

enum class DeliverStatus {
    STATUS_OK = 0,
    STATUS_ERROR,
    STATUS_NO_MEMORY,
    STATUS_REQUEST_NOT_SUPPORTED
};

}  // namespace interfaces
}  // namespace qcril
