/**
* Copyright (c) 2017-2020 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**/

#pragma once
#include "framework/PendingMessageList.h"
#include "module/IDataModule.h"
#include "DataCommon.h"
#include "AuthManager.h"
#include "LceHandler.h"
#include "ProfileHandler.h"
#include "NetworkServiceHandler.h"
#include "NetworkAvailabilityHandler.h"
#include "request/IWLANCapabilityHandshake.h"
#include "InitTracker.h"
#include "CallManager.h"
#include "PreferredDataStateMachine.h"
#include "VoiceCallModemEndPoint.h"
#include "qtibus/IPCMessage.h"
#include "KeepAliveHandler.h"
#include "modules/uim/qcril_uim_types.h"
#ifdef FEATURE_DATA_LQE
  #include "OTTModemEndPoint.h"
#endif /* FEATURE_DATA_LQE */

namespace rildata {

typedef void (*datactlInitFnPtr)(int, void (*logFn)(std::string));
typedef bool (*datactlEnableIWlanFnPtr)();
typedef bool (*datactlDisableIWlanFnPtr)();

struct datactlSymbols {
  datactlInitFnPtr datactlInit;
  datactlEnableIWlanFnPtr datactlEnableIWlan;
  datactlDisableIWlanFnPtr datactlDisableIWlan;
};

class DataModule : public IDataModule {
 public:
  DataModule();
  ~DataModule();
  PendingMessageList& getPendingMessageList();
  void dump(int fd);
  void init();
#ifdef QMI_RIL_UTF
  virtual void cleanup();
#endif

 private:
  static constexpr TimeKeeper::millisec TempDDS_SWITCH_REQUEST_TIMEOUT = 5000;
  dsd_system_status_ind_msg_v01 mCachedSystemStatus = { };
  InitTracker mInitTracker;
  int32_t iwlanHandshakeMsgToken;
  bool mAuthServiceReady = false;
  PendingMessageList mMessageList;
  TimeKeeper::timer_id tempDDSSwitchRequestTimer;
  bool TempddsSwitchRequestPending = false;
  bool mLinkStatusReportEnabled = false;
  bool mLimitedIndReportEnabled = false;
  bool mInitCompleted = false;

  std::unique_ptr<AuthManager> auth_manager;
  std::unique_ptr<CallManager> call_manager;
  std::unique_ptr<ProfileHandler> profile_handler;
  std::unique_ptr<NetworkServiceHandler> network_service_handler;
  std::unique_ptr<NetworkAvailabilityHandler> networkavailability_handler;

  std::shared_ptr<DSDModemEndPoint> dsd_endpoint;
  std::shared_ptr<WDSModemEndPoint> wds_endpoint;
  std::shared_ptr<AuthModemEndPoint> auth_endpoint;
  #ifdef FEATURE_DATA_LQE
    std::shared_ptr<OTTModemEndPoint> ott_endpoint;
  #endif /* FEATURE_DATA_LQE */
  std::shared_ptr<PreferredDataInfo_t> preferred_data_state_info;
  std::shared_ptr<VoiceCallModemEndPoint> voiceCallEndPointSub0;
  std::shared_ptr<VoiceCallModemEndPoint> voiceCallEndPointSub1;

  datactlSymbols dcSymbols;
  bool loadDatactl();
  static void logFn(std::string);

  LceHandler lceHandler;
  LocalLogBuffer logBuffer{string("RIL"), 200};
  LocalLogBuffer networkAvailabilityLogBuffer{string("NAH"), 200};
  std::unique_ptr<PreferredDataStateMachine> preferred_data_sm;

   void handleQcrilInitMessage(std::shared_ptr<Message> msg);
   void broadcastReady();
   std::string readProperty(std::string name, string defaultValue);
   void handleRilRequestSetupDataCallMessage(std::shared_ptr<Message> msg);
   void handleDataCallListMessage(std::shared_ptr<Message> msg);
   void handleGetDataCallProfileMessage(std::shared_ptr<Message> msg);
   void handleRilRequestDeactivateDataCallMessage(std::shared_ptr<Message> msg);
   void handleEmbmsActivateDeactivateTmgiMessage(std::shared_ptr<Message> msg);
   void handleEmbmsActivateTmgiMessage(std::shared_ptr<Message> msg);
   void handleEmbmsContentDescUpdateMessage(std::shared_ptr<Message> msg);
   void handleEmbmsDeactivateTmgiMessage(std::shared_ptr<Message> msg);
   void handleEmbmsEnableDataReqMessage(std::shared_ptr<Message> msg);
   void handleEmbmsGetActiveTmgiMessage(std::shared_ptr<Message> msg);
   void handleEmbmsGetAvailTmgiMessage(std::shared_ptr<Message> msg);
   void handleEmbmsSendIntTmgiListMessage(std::shared_ptr<Message> msg);
   void handleGoDormantMessage(std::shared_ptr<Message> msg);
   void handleLastDataCallFailCauseMessage(std::shared_ptr<Message> msg);
   void handleProcessScreenStateChangeMessage(std::shared_ptr<Message> msg);
   void handleProcessStackSwitchMessage(std::shared_ptr<Message> msg);
   void handlePullLceDataMessage(std::shared_ptr<Message> msg);
   void handleSetApnInfoMessage(std::shared_ptr<Message> msg);
   void handleRilRequestSetDataProfileMessage(std::shared_ptr<Message> msg);
   void handleSetIsDataEnabledMessage(std::shared_ptr<Message> msg);
   void handleSetIsDataRoamingEnabledMessage(std::shared_ptr<Message> msg);
   void handleSetLteAttachProfileMessage(std::shared_ptr<Message> msg);
   void handleSetQualityMeasurementMessage(std::shared_ptr<Message> msg);
   void handleStartLceMessage(std::shared_ptr<Message> msg);
   void handleStopLceMessage(std::shared_ptr<Message> msg);
   void handleToggleDormancyIndMessage(std::shared_ptr<Message> msg);
   void handleToggleLimitedSysIndMessage(std::shared_ptr<Message> msg);
   void handleUpdateMtuMessage(std::shared_ptr<Message> msg);
   void handleDataEmbmsActiveMessage(std::shared_ptr<Message> msg);
   void handleGetDdsSubIdMessage(std::shared_ptr<Message> msg);
   void handleDataRequestDDSSwitchMessage(std::shared_ptr<Message> msg);
   void handleSetInitialAttachApn(std::shared_ptr<Message> msg);
   void handleSetLteAttachPdnListActionResult(std::shared_ptr<Message> msg);
   void handleVoiceIndMessage(std::shared_ptr<Message> msg);
   void handleCallManagerEventMessage(std::shared_ptr<Message> msg);
   void handleTempDDSSwitchTimerExpired(void *resp);
   void handleDeregisterWdsIndCompletedMessage(std::shared_ptr<Message> msg);
   void handleVoiceCallOrigTimerExpired(void *resp);
   void handleVoiceCallOrigTimeoutMessage(std::shared_ptr<Message> msg);

#if (QCRIL_RIL_VERSION >= 15)

   std::shared_ptr<KeepAliveHandler> keep_alive;

   void handleSetLteAttachProfileMessage_v15(std::shared_ptr<Message> msg);
   void handleSetDataProfileMessage_v15(std::shared_ptr<Message> msg);
   void handleSetDataProfileRequestMessage(std::shared_ptr<Message> msg);

   void handleStartKeepaliveMessage(std::shared_ptr<Message> msg);
   void handleStopKeepaliveMessage(std::shared_ptr<Message> msg);
   void handleKeepAliveIndMessage(std::shared_ptr<Message> msg);
   void handleCleanDataCallStateMessage(std::shared_ptr<Message> msg);

   void handleRilRequestSetCarrierInfoImsiEncryptionMessage(std::shared_ptr<Message> msg);
   void handleQmiAuthServiceIndMessage(std::shared_ptr<Message> msg);
   void handleQmiAuthEndpointStatusIndMessage(std::shared_ptr<Message> msg);
   void handleQmiDsdEndpointStatusIndMessage(std::shared_ptr<Message> msg);

   void handleSetLinkCapFilterMessage(std::shared_ptr<Message> msg);
   void handleSetLinkCapRptCriteriaMessage(std::shared_ptr<Message> msg);
   keep_alive_start_request getApnName(const qcril_request_params_type *const params_ptr, qcril_request_return_type *const ret_ptr);

   void handleStartKeepAliveRequestMessage(std::shared_ptr<Message> msg);
   void handleStopKeepAliveRequestMessage(std::shared_ptr<Message> msg);
   #ifdef FEATURE_DATA_LQE
     void handleQmiOttEndpointStatusIndMessage(std::shared_ptr<Message> msg);
   #endif /* FEATURE_DATA_LQE */
#endif

   void handleNasSrvDomainPrefInd(std::shared_ptr<Message> msg);
   void handleNasRequestDataShutdown(std::shared_ptr<Message> msg);
   void handleDataAllBearerTypeUpdate(std::shared_ptr<Message> msg);
   void handleDataBearerTypeUpdate(std::shared_ptr<Message> msg);
   void handleToggleBearerAllocationUpdate(std::shared_ptr<Message> msg);
   void handleGetBearerAllocation(std::shared_ptr<Message> msg);
   void handleGetAllBearerAllocations(std::shared_ptr<Message> msg);
   void handleQmiWdsEndpointStatusIndMessage(std::shared_ptr<Message> msg);
   void handleDataConnectionStateChangedMessage(std::shared_ptr<Message> msg);
   void handleIWLANCapabilityHandshake(std::shared_ptr<Message> msg);
   void handleGetAllQualifiedNetworksMessage(std::shared_ptr<Message> msg);
   void handleDsdSystemStatusInd(std::shared_ptr<Message> msg);
   void handleDsdSystemStatusPerApn(std::shared_ptr<Message> msg);
   void handleIntentToChangeInd(std::shared_ptr<Message> msg);
   void handleSetPreferredDataModem(std::shared_ptr<Message> msg);
   void handleCurrentDDSIndMessage(std::shared_ptr<Message> msg);
   void handleDDSSwitchResultIndMessage(std::shared_ptr<Message> msg);
   void handleRadioConfigClientConnectedMessage(std::shared_ptr<Message> msg);
   void handleDDSSwitchTimeoutMessage(std::shared_ptr<Message> msg);
   void handleDDSSwitchIPCMessage(std::shared_ptr<Message> msg);
   std::shared_ptr<IPCMessage> constructDDSSwitchIPCMessage(IPCIStream &is);
   RIL_Errno map_internalerr_from_reqlist_new_to_ril_err(IxErrnoType error);
   void performDataModuleInitialization();
   void initializeIWLAN();
   void deinitializeIWLAN();

   void handleRilEventDataCallback(std::shared_ptr<Message> msg);
   void handleSetupDataCallRequestMessage(std::shared_ptr<Message> msg);
   void handleDeactivateDataCallRequestMessage(std::shared_ptr<Message> msg);
   void handleGetRadioDataCallListRequestMessage(std::shared_ptr<Message> msg);
   void handleGetIWlanDataCallListRequestMessage(std::shared_ptr<Message> msg);
   void handleGetIWlanDataRegistrationStateRequestMessage(std::shared_ptr<Message> msg);
   void handleSetApnPreferredSystemResult(std::shared_ptr<Message> msg);
   void handleSetRadioClientCapMessage(std::shared_ptr<Message> msg);
   void handleSetRadioClientCapUnSolMessage(std::shared_ptr<Message> msg);
   void handleNasPhysChanConfigReportingStatusMessage(std::shared_ptr<Message> msg);
   void handleNasPhysChanConfigMessage(std::shared_ptr<Message> msg);
   void handleHandoverInformationIndMessage(std::shared_ptr<Message> msg);
   void handleDataCallTimerExpiredMessage(std::shared_ptr<Message> msg);
   void handleCallBringupFallbackMessage(std::shared_ptr<Message> msg);
   void handleSetCarrierInfoImsiEncryptionMessage(std::shared_ptr<Message> msg);

   TimeKeeper::timer_id setTimeoutForMsg(std::shared_ptr<Message> msg, TimeKeeper::millisec maxTimeout);
   void deleteEntryInReqlist( std::shared_ptr<Message> msg );
   void handleUimCardAppStatusIndMsg(std::shared_ptr<Message> msg);
   int retrieveUIMCardStatus( std::shared_ptr<RIL_UIM_CardStatus> ril_card_status, string &aid_buffer, RIL_UIM_AppType *app_type);
   int retrieveUIMAppStatusFromAppInfo(RIL_UIM_AppStatus *application, string &aid_buffer, RIL_UIM_AppType *app_type);
   void qcrilDataUimEventAppStatusUpdate ( const qcril_request_params_type *const params_ptr, qcril_request_return_type *const ret_ptr);
   void handleQcrilRadioClientConnectedMessage(std::shared_ptr<Message> msg);
};
void qcrilDataprocessMccMncInfo
(
    const qcril_request_params_type *const params_ptr,
    qcril_request_return_type       *const ret_ptr
);

DataModule& getDataModule();

} //namespace
