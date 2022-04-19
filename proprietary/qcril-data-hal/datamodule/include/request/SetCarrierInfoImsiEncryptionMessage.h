/*===========================================================================

  Copyright (c) 2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

#pragma once
#include "MessageCommon.h"

namespace rildata {

enum class PublicKeyType_t : int32_t {
  UNKNOWN = 0,
  EPDG    = 1,
  WLAN    = 2,
};

struct ImsiEncryptionInfo_t {
  string mcc;
  string mnc;
  vector<uint8_t> carrierKey;
  string keyIdentifier;
  int64_t expiryTime;
  PublicKeyType_t keyType;
};

class SetCarrierInfoImsiEncryptionMessage : public SolicitedMessage<RIL_Errno>,
                                  public add_message_id<SetCarrierInfoImsiEncryptionMessage> {
  private:
    ImsiEncryptionInfo_t mImsiEncryptionInfo;

  public:
    static constexpr const char* MESSAGE_NAME = "SetupDataCallRequestMessage";
    SetCarrierInfoImsiEncryptionMessage() = delete;
    SetCarrierInfoImsiEncryptionMessage(
      const ImsiEncryptionInfo_t imsiEncryptionInfo
      ):SolicitedMessage<RIL_Errno>(get_class_message_id()) {
      mName = MESSAGE_NAME;
      mImsiEncryptionInfo = imsiEncryptionInfo;
    }
    ~SetCarrierInfoImsiEncryptionMessage() = default;

    string dump(){
      std::stringstream ss;
      ss << "[" << mName << "] ";
      ss << "keyIdentifier =" << mImsiEncryptionInfo.keyIdentifier <<",";
      return ss.str();
    }

    const ImsiEncryptionInfo_t* getImsiEncryptionInfo() {return &mImsiEncryptionInfo;}
};

}//namespace
