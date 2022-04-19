/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <string>
#include <vector>
#include <map>

#include "framework/Log.h"
#include "framework/Message.h"

#include "data_system_determination_v01.h"

#include "DataCommon.h"
#include "MessageCommon.h"
#include "GetRadioDataRegistrationStateRequestMessage.h"
#include "GetIWlanDataRegistrationStateRequestMessage.h"

namespace rildata {
class NetworkServiceHandler {
private:
    void broadcastDsdSysStatusMessage(dsd_system_status_ind_msg_v01 *dsd_sys_status);

public:
    NetworkServiceHandler();
    virtual ~NetworkServiceHandler(){}
    void processRadioRegStatusInd(dsd_system_status_ind_msg_v01 *ind);
    virtual void processQmiDsdSystemStatusInd(dsd_system_status_ind_msg_v01 *ind_data);
    virtual void dump(string, ostream&) {}
};

class ApAssistNetworkServiceHandler : public NetworkServiceHandler {
  struct iwlanRegCacheType_t {
    bool valid;
    DataRegState_t regState;
    int32_t reasonForDenial;
  };

  private:
    iwlanRegCacheType_t iwlanRegCache;
  public:
    ApAssistNetworkServiceHandler();
    virtual ~ApAssistNetworkServiceHandler(){}
    DataRegState_t getIWlanDataRegistrationState(void);
    std::string convertToString(const DataRegState_t& regState);
    void processIWlanRegStatusInd(dsd_system_status_ind_msg_v01 *ind);
    void processQmiDsdSystemStatusInd(dsd_system_status_ind_msg_v01 *ind_data) override;
    virtual void dump(string padding, ostream& os) override;
};

}
