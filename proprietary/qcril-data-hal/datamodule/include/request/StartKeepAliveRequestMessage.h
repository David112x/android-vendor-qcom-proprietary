/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once

#include "MessageCommon.h"
#include "telephony/ril.h"
#include <string.h>
#include <cstring>

using namespace std;

namespace rildata {

enum class KeepaliveType_t {
  NATT_IPV4 = 0,
  NATT_IPV6 = 1,
};

enum class KeepaliveStatus_t : int {
  RUNNING = 0,
  NONE = 1,
  REQUESTED = 2,
};

struct KeepaliveRequest_t {
  KeepaliveType_t type;
  std::vector<uint8_t> sourceAddress;
  int32_t sourcePort;
  std::vector<uint8_t> destinationAddress;
  int32_t destinationPort;
  int32_t maxKeepaliveIntervalMillis;
  int32_t cid;
};

struct StartKeepAliveResp_t {
  rildata::ResponseError_t error;
  uint32_t handle;
  KeepaliveStatus_t status;
};

class StartKeepAliveRequestMessage : public SolicitedMessage<StartKeepAliveResp_t>,
                          public add_message_id<StartKeepAliveRequestMessage> {
  private:
    int32_t mSerial;
    KeepaliveRequest_t mReq;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.data.StartKeepAliveRequestMessage";
    StartKeepAliveRequestMessage() = delete;
    StartKeepAliveRequestMessage(
      const int32_t serial,
      const KeepaliveRequest_t req
      ):SolicitedMessage<StartKeepAliveResp_t>(get_class_message_id()) {

      mName = MESSAGE_NAME;
      mSerial = serial;
      mReq = req;
    }
    ~StartKeepAliveRequestMessage() = default;

    string dump(){return mName;}
    int32_t getSerial() {return mSerial;}
    KeepaliveRequest_t getKeepaliveRequest() {return mReq;}
};

}
