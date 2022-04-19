/*===========================================================================

  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/
#pragma once

#include "framework/Log.h"
#include "framework/Message.h"

#include "data_system_determination_v01.h"

#include "DataCommon.h"
#include "UnSolMessages/DsdSystemStatusMessage.h"
#include "MessageCommon.h"
#include "WDSModemEndPoint.h"
#include "DSDModemEndPoint.h"
#include "LocalLogBuffer.h"
#include "modules/qmi/ModemEndPointFactory.h"

namespace rildata {

struct ConsolidatedNetwork_t {
  bool hasPendingIntentToChange;
  vector<QualifiedNetwork_t> qualNetworks;
  void dump(string padding, ostream& os);
};

/**
 * @brief NetworkAvailabilityHandler class
 * @details NetworkAvailabilityHandler class
 */
class NetworkAvailabilityHandler {
public:
  NetworkAvailabilityHandler(LocalLogBuffer& logBuffer);
  ~NetworkAvailabilityHandler();

  // QMI indication process functions
  void processSetDataProfileRequest(std::shared_ptr<Message> msg);
  void processQmiDsdSystemStatusInd(dsd_apn_avail_sys_info_type_v01 * apn_sys, uint32 len);
  void processQmiDsdIntentToChangeApnPrefSysInd(dsd_intent_to_change_apn_pref_sys_ind_msg_v01 *ind_data);
  void getQualifiedNetworks(vector<QualifiedNetwork_t> &qualifiedNetworks);
  static std::string convertToString(const AccessNetwork_t &ran);
  static std::string convertToString(const ApnTypes_t &apnType);
  static std::string convertToString(const QualifiedNetwork_t &qualNetwork);
  void dump(string padding, ostream& os);

private:
  LocalLogBuffer& logBuffer;
  std::shared_ptr<WDSModemEndPoint> wds_endpoint;
  std::shared_ptr<DSDModemEndPoint> dsd_endpoint;
  std::unordered_map<std::string, ConsolidatedNetwork_t> mNetworkAvailabilityCache;
  void updateNetworkAvailabilityCache(std::shared_ptr<Message> msg);
  void updateNetworkAvailabilityCache(dsd_apn_avail_sys_info_type_v01 *, uint32);
  void updateNetworkAvailabilityCache(dsd_apn_pref_sys_type_ex_v01 *, uint32);
  AccessNetwork_t convertToRadioAccessNetwork(dsd_system_status_info_type_v01 );
  vector<int32_t> convertToRadioAccessNetworkList(dsd_apn_avail_sys_info_type_v01 );
  int32_t convertToRadioApnType(uint64_t qmiApnType);
  vector<int32_t> convertToRadioApnTypeList(const ApnTypes_t &apnTypes);
  void dumpCache();
  void convertResultList(vector<QualifiedNetwork_t> &resultList,
                         vector<QualifiedNetwork_t> &qualifiedNetworks);
};

} /* namespace rildata */
