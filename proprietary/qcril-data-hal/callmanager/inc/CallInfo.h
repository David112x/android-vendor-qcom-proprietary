/*===========================================================================

  Copyright (c) 2018-19 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/
#pragma once

#include <stdint.h>
#include <set>
#include <unordered_map>

#include "legacy/qcril_data.h"
#include "WDSCallModemEndPoint.h"
#include "WDSModemEndPoint.h"
#include "event/RilEventDataCallback.h"
#include "event/CallManagerEventMessage.h"

#include "CallStateMachine.h"
#include "CallSetupProfile.h"
#include "LocalLogBuffer.h"
#include "request/SetupDataCallRequestMessage.h"
#include "request/DeactivateDataCallRequestMessage.h"
#include "sync/GetCallBringUpCapabilitySyncMessage.h"
#include "EmbmsCallHandler.h"
#include "UnSolMessages/DeregisterWdsIndCompletedMessage.h"

#include "DsiWrapper.h"

#define SuggestedRetryBackoffTime -1; // no value is suggested

using namespace std;

namespace rildata {

enum HandoffNetworkType_t {
  _eWWAN  = 1 << 0,
  _eWLAN  = 1 << 1,
  _eIWLAN = 1 << 2,
};

enum LinkActiveState {
  _eInactive = 0,
  _eActivePhysicalLinkDown = 1,
  _eActivePhysicalLinkUp = 2
};

struct DsiEventDataType {
  dsi_net_evt_t netEvent;
  void *eventData;
  std::string getEventName() const;
};

class CallEndReason {
public:
  static DataCallFailCause_t getCallEndReason(dsi_ce_reason_t dsiReason);
  static bool isPermanentFailure(DataCallFailCause_t rilReason);
  static constexpr int AP_ASSIST_HANDOVER = -1;

private:
  CallEndReason();
  typedef unordered_map<int, DataCallFailCause_t> CallEndReasonMap_t;
  static unordered_map<dsi_ce_reason_type_t, CallEndReasonMap_t> callEndReasonTypeMap;
  static CallEndReasonMap_t mipReasonMap;
  static CallEndReasonMap_t internalReasonMap;
  static CallEndReasonMap_t cmReasonMap;
  static CallEndReasonMap_t _3gppReasonMap;
  static CallEndReasonMap_t pppReasonMap;
  static CallEndReasonMap_t ehrpdReasonMap;
  static CallEndReasonMap_t ipv6ReasonMap;
  static CallEndReasonMap_t handoffReasonMap;
  static CallEndReasonMap_t apAssistHandoffReasonMap;
};

/**
 * @brief Call information class
 * @details All related information for one data call is maintained
 * This object is created when a data call bring up is requested and deleted
 * when its call is terminated
  */
class CallInfo {
public:
  CallInfo(LocalLogBuffer& logBuffer);
  ~CallInfo();
  void dump(string padding, ostream& os);

  int32_t serialId;
  std::shared_ptr<EmbmsCallHandler> pEmbmsInfo;

  void setCid(int cid) {cId = cid;};
  int getCid(){return cId;};

  void setIntentToChangeInProgress(bool inProgress);
  bool isIntentToChangeInProgress(void){return intentToChangeInProgress;};

  void setThirdPartyHOParams(std::vector<std::string> addr);
  bool isThirdPartyHOParamsSet(){return thirdPartyHOParamsSet;};
  std::vector<std::string> getThirdPartyHOParams() {return thirdPartyHOAddr;};

  void setFallbackInProgress(bool inProgress);
  bool isFallbackInProgress(void){return fallbackInProgress;};

  bool setProfileInfo(CallSetupProfile *profile);
  bool setEmergencyProfileInfo(CallSetupProfile *profile);
  CallSetupProfile *getProfileInfo(void) {return pProfileInfo;};

  void setDsiHandle(dsi_hndl_t handle) {dsiHandle=handle;};
  dsi_hndl_t getDsiHandle(void) {return dsiHandle;};

  void setCurrentNetworkType(HandoffNetworkType_t rat){currentRAT=rat;};
  HandoffNetworkType_t getCurrentNetworkType(void){return currentRAT;};

  void setCallBringUpCapability(BringUpCapability cap){mCallBringupCapability = cap;};
  BringUpCapability getCallBringUpCapability(void){return mCallBringupCapability;};

  void setPreferredNetworkType(HandoffNetworkType_t rat){preferredRAT=rat;};
  HandoffNetworkType_t getPreferredNetworkType(void){return preferredRAT;};

  void setMessageToken(uint16_t tok) {pendingMessageToken=tok;};
  uint16_t getMessageToken(void) {return pendingMessageToken;};

  void setRilEventId(qcril_evt_e_type id) {eventId=id;};
  qcril_evt_e_type getRilEventId(void) {return eventId;};

  void setCallParamsChangedFlag(bool flag) {callParamsChanged=flag;};
  bool getCallParamsChangedFlag(void) {return callParamsChanged;};

  CallStateMachine *getCallStateMachine(void) {return pCallsm;};

  int getActiveState(void) {return (int)active;};

  string getDeviceName(void) {return deviceName;};

  string getIpAddresses(void) {return ipAddresses;};

  string getDnsAddresses(void) {return dnsAddresses;};

  string getGatewayAddresses(void) {return gatewayAddresses;};

  string getPcscfAddresses(void) {return pcscfAddresses;};

  string getIpType(void) {return ipType;};

  int32_t getMtu(void) {return mtu;};

  timer_t getHandoverTimerId(void) {return handoverTimerId;};

  void setMaxPartialRetryTimeout(unsigned long maxTimeout){maxPartialRetryTimeout=maxTimeout;};

  unsigned long getMaxPartialRetryTimeout(void) {return maxPartialRetryTimeout;};

  void setPartialRetryEnabled(bool enabled){partialRetryEnabled = enabled;};

  bool IsPartialRetryEnabled(void) {return partialRetryEnabled;};

  bool IsV4V6(void) {return "IPV4V6" == ipType;};

  bool IsV4Connected(void) {return v4Connected;};

  bool IsV6Connected(void) {return v6Connected;};

  bool IsV4HandedOver(void) {return v4HandedOver;};

  bool IsV6HandedOver(void) {return v6HandedOver;};

  bool setupDataCall(std::shared_ptr<std::function<void(CallInfo *, rildata::SetupDataCallResponse_t)>> completeCb);
  bool deactivateDataCall(std::shared_ptr<std::function<void(CallInfo *, rildata::ResponseError_t)>> completeCb);
  bool handoverSetupDataCall(std::shared_ptr<std::function<void(CallInfo *, rildata::SetupDataCallResponse_t)>> completeCb);
  bool handoverDeactivateDataCall(std::shared_ptr<std::function<void(CallInfo *, rildata::ResponseError_t)>> completeCb);
  void handleHandoverTimeout();
  void handleSetupDataCallTimeout();
  void handleDeactivateDataCallTimeout();
  void handlePartialRetryTimeout();
  void handlePartialRetryResponseTimeout();
  bool setupEmbmsDataCall(qcril_instance_id_e_type instance_id);

  bool configureParams(bool updateDataCallListChangeInd);

  bool compareCallParams(CallInfo *callInst);

  void setLinkActiveState(LinkActiveState state);

  void setupDataCallResponse(void);

  void setupDataCallFailResponse(DataCallFailCause_t reason);

  void deactivateDataCallResponse(void);

  void deactivateDataCallFailResponse(void);

  int handleRilEventDataCallback(std::shared_ptr<Message> msg);

  void getCallEndReason(int &reason);

  void handoffInformationUpdate(HandoffState_t state, std::shared_ptr<Message> msg);

  void createQmiWdsClients(void);

  void setCallInfoChangedCallback(std::shared_ptr<std::function<void(void)>> callInfoChangedCb);

  void handleHandoverPreferenceUnchanged();

  void processPcscfAddressChangedEvent();

  void setLinkStateChangeReport(bool report);

  static void dsiNetCallback(dsi_hndl_t dsiHandle, void *userData,
    dsi_net_evt_t netEvent, dsi_evt_payload_t *payload);

  bool sendDormancyRequest(void);
  bool isEmbmsCall();

  void resetCallInfo(void);
  void deleteWdsCallEndPoint(IpFamilyType_t type);
  bool hasWdsCallEndPoints();

private:
  LocalLogBuffer& logBuffer;
  int cId;
  LinkActiveState active;
  bool dormantState;
  bool linkStateChangeReport;
  bool partialRetryEnabled;
  unsigned long maxPartialRetryTimeout;
  bool v4Connected;
  bool v6Connected;
  bool v4HandedOver;
  bool v6HandedOver;
  bool intentToChangeInProgress;
  bool thirdPartyHOParamsSet;
  bool fallbackInProgress;
  HandoffNetworkType_t currentRAT;
  HandoffNetworkType_t preferredRAT;
  BringUpCapability mCallBringupCapability;
  int transportType;
  timer_t handoverTimerId;
  string deviceName;
  string ipAddresses;
  string gatewayAddresses;
  string dnsAddresses;
  string pcscfAddresses;
  string ipType;
  int32_t mtu;
  bool callParamsChanged;

  dsi_hndl_t dsiHandle;
  uint16_t pendingMessageToken;
  qcril_evt_e_type eventId;

  CallStateMachine *pCallsm;
  CallSetupProfile *pProfileInfo;

  std::shared_ptr<std::function<void(CallInfo *, rildata::SetupDataCallResponse_t)>> setupDataCallCompleteCallback;
  std::shared_ptr<std::function<void(CallInfo *, rildata::ResponseError_t)>> deactivateDataCallCompleteCallback;
  std::shared_ptr<std::function<void(void)>> callInfoChangedCallback;
  std::shared_ptr<std::function<void(CallInfo *)>> embmsCallCompleteCallback;

  std::shared_ptr<WDSCallModemEndPoint> wds_v4_call_endpoint;
  std::shared_ptr<WDSCallModemEndPoint> wds_v6_call_endpoint;
  std::shared_ptr<WDSModemEndPoint> wds_endpoint;

  std::vector<std::string> thirdPartyHOAddr;

  void releaseWDSCallEPModuleQMIClient();
  bool setRilProfileId( const dsi_hndl_t hndl, const int32_t rilProfile, const uint8_t rilTech);
};

} /* namespace rildata */
