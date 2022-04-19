/*===========================================================================

  Copyright (c) 2019-2020 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/
#pragma once
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "framework/Message.h"
#include "framework/TimeKeeper.h"

namespace rildata {

enum class SegmentationStatus_t : int {
  Error = -2,
  TimedOut = -1,
  Success = 0,
};

enum class EndPointName_t : int {
  UNSPECIFIED = 0,
  WDS = 1,
  DSD = 2,
  WDSCall_0 = 3,
  WDSCall_1 = 4,
};

struct PendingTransaction {
  std::vector<std::shared_ptr<Message>> segments;
  std::unordered_set<uint8_t> remainingSegments;
  PendingTransaction(uint8_t totalSegments) {
    segments.resize(totalSegments, nullptr);
    for (uint8_t i = 0; i < totalSegments; i++) {
      remainingSegments.insert(i);
    }
  }
  PendingTransaction() : PendingTransaction(0) {}
};

struct PendingTimer {
  TimeKeeper::timer_id timerId;
  unsigned int msgId;
};

class SegmentTracker {
public:
  using MessageId_t = unsigned int;
  using KeyType_t = std::string;
  using transactionCompleteCb_t = std::function<void(SegmentationStatus_t, uint16_t, std::vector<std::shared_ptr<Message>>)>;
  static KeyType_t serialize(MessageId_t msgId, uint16_t txId);
  void setHandlers(const std::unordered_map<MessageId_t, transactionCompleteCb_t>& handlers);
  void startTransactionTimer(MessageId_t msgId, uint16_t txId);
  void addSegment(MessageId_t msgId, uint16_t txId, uint8_t segmentIndex,
                  uint8_t totalSegments, std::shared_ptr<Message> msg);
  void sendErrorResponse(SegmentationStatus_t status, MessageId_t msgId, uint16_t txId);
  void setEndPointName(EndPointName_t name) {mEndPoint = name;}

  SegmentTracker(){}
  SegmentTracker(const SegmentTracker&) = delete;
  SegmentTracker& operator=(const SegmentTracker&) = delete;
  SegmentTracker(SegmentTracker&&) = delete;
  SegmentTracker& operator=(SegmentTracker&&) = delete;

private:
  EndPointName_t mEndPoint = EndPointName_t::UNSPECIFIED;
  static constexpr TimeKeeper::millisec SEGMENT_TIMEOUT = 20;
  std::unordered_map<MessageId_t, transactionCompleteCb_t> mHandlers;
  std::unordered_map<KeyType_t, PendingTransaction> transactions;
  std::unordered_map<KeyType_t, PendingTimer> timers;
  qtimutex::QtiSharedMutex mTransactionsMutex;

  void sendResponse(SegmentationStatus_t status, MessageId_t msgId, uint16_t txId, std::vector<std::shared_ptr<Message>>& segments);
  void timeoutHandler(MessageId_t msgId, uint16_t txId, void *);
};

} // namespace rildata
