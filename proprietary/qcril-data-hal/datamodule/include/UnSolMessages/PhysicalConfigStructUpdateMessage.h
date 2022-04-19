/**
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**/

#pragma once
#include "framework/Message.h"
#include "framework/UnSolicitedMessage.h"
#include "framework/add_message_id.h"
#include "MessageCommon.h"

namespace rildata {

struct PhysicalConfigStructInfo_t {
  CellConnectionStatus_t status;
  int32_t cellBandwidthDownlink;
  RadioAccessFamily_t rat;
  RadioFrequencyInfo_t rfInfo;
  std::vector<int32_t> contextIds;
  uint32_t physicalCellId;
};

/********************** Class Definitions *************************/
class PhysicalConfigStructUpdateMessage: public UnSolicitedMessage,
                           public add_message_id<PhysicalConfigStructUpdateMessage> {

private:
  std::vector<PhysicalConfigStructInfo_t> mPhysicalConfigStructInfo;
public:
  static constexpr const char *MESSAGE_NAME = "PhysicalConfigStructUpdateMessage";

  PhysicalConfigStructUpdateMessage() = delete;
  ~PhysicalConfigStructUpdateMessage(){};
  PhysicalConfigStructUpdateMessage(std::vector<PhysicalConfigStructInfo_t> const &physicalConfigInfo);

  std::shared_ptr<UnSolicitedMessage> clone();
  const std::vector<PhysicalConfigStructInfo_t> & getPhysicalConfigStructInfo();
  static std::string convertToString(const PhysicalConfigStructInfo_t& config);
  string dump();
};

inline PhysicalConfigStructUpdateMessage::PhysicalConfigStructUpdateMessage
(
  const std::vector<PhysicalConfigStructInfo_t> &physicalConfigStructInfo
) : UnSolicitedMessage(get_class_message_id())
{
  mName = MESSAGE_NAME;
  mPhysicalConfigStructInfo = physicalConfigStructInfo;
}

inline const std::vector<PhysicalConfigStructInfo_t> &
PhysicalConfigStructUpdateMessage::getPhysicalConfigStructInfo()
{
  return mPhysicalConfigStructInfo;
}

inline std::shared_ptr<UnSolicitedMessage> PhysicalConfigStructUpdateMessage::clone()
{
  return std::make_shared<PhysicalConfigStructUpdateMessage>(mPhysicalConfigStructInfo);
}

inline std::string PhysicalConfigStructUpdateMessage::convertToString(const PhysicalConfigStructInfo_t& config) {
  stringstream ss;
  ss << "<Status=" << (int)config.status;
  ss << ", downlink=" << config.cellBandwidthDownlink;
  ss << ", rat=" << (int)config.rat;
  ss << ", rfInfo.range=" << (int)config.rfInfo.range;
  ss << ", rfInfo.channel=" << config.rfInfo.channelNumber;
  ss << ", cids=[";
  std::copy(config.contextIds.begin(), config.contextIds.end(), std::ostream_iterator<int>(ss, ","));
  ss << "], physicalCellId=" << config.physicalCellId << ">";
  return ss.str();
}

inline string PhysicalConfigStructUpdateMessage::dump()
{
  stringstream ss;
  ss << mName << " {";
  for (auto it = mPhysicalConfigStructInfo.begin(); it != mPhysicalConfigStructInfo.end(); it++) {
    ss << PhysicalConfigStructUpdateMessage::convertToString(*it) << ",";
  }
  ss << "}";
  return ss.str();
}

} //namespace
