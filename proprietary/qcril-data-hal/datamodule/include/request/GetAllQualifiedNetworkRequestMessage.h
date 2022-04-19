/*===========================================================================

  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

#pragma once

#include "MessageCommon.h"

using namespace std;

namespace rildata {

struct QualifiedNetworkResult_t {
  ResponseError_t respErr;
  vector<QualifiedNetwork_t> qualifiedNetwork;
};

class GetAllQualifiedNetworkRequestMessage : public SolicitedMessage<QualifiedNetworkResult_t>,
                          public add_message_id<GetAllQualifiedNetworkRequestMessage> {
  private:

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.data.GetAllQualifiedNetworkRequestMessage";
    inline GetAllQualifiedNetworkRequestMessage() :
      SolicitedMessage<QualifiedNetworkResult_t>(get_class_message_id())
    {
      mName = MESSAGE_NAME;
    }
    ~GetAllQualifiedNetworkRequestMessage() = default;

    string dump(){return mName;}
};

}
