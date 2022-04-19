/******************************************************************************
#  Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include "framework/Module.h"
#include "modules/qmi/NasModemEndPoint.h"
#include "modules/qmi/QmiAsyncResponseMessage.h"
#include "modules/qmi/EndpointStatusIndMessage.h"

#include "modules/nas/NasUimHeaders.h"
#include "modules/nas/NasRequestDataShutdownMessage.h"

#include <modules/android/RilRequestMessage.h>
#include <modules/ims/ImsServiceStatusInd.h>
#include <modules/ims/ImsWfcSettingsStatusInd.h>
#include <modules/ims/ImsPdpStatusInd.h>
#include "modules/nas/NasSetVoiceDomainPreferenceRequest.h"

// Messages from Data
#include "modules/nas/NasEmbmsEnableDataConMessage.h"
#include "modules/nas/NasEmbmsDisableDataMessage.h"
#include "NasPhysChanConfigMessage.h"

// NAS exported API
#include "modules/nas/NasSetPsAttachDetachMessage.h"
#include "modules/nas/NasGetSysInfoMessage.h"

//interface header files
#include "interfaces/nas/RilRequestGetVoiceRegistrationMessage.h"
#include "interfaces/nas/RilRequestGetDataRegistrationMessage.h"
#include "interfaces/nas/RilRequestOperatorMessage.h"
#include "interfaces/nas/RilRequestQueryNetworkSelectModeMessage.h"
#include "interfaces/nas/RilRequestAllowDataMessage.h"
#include "interfaces/nas/RilRequestQueryAvailNetworkMessage.h"
#include "interfaces/nas/RilRequestSetNetworkSelectionManualMessage.h"
#include "interfaces/nas/RilRequestSetNetworkSelectionAutoMessage.h"
#include "interfaces/nas/RilRequestStopNetworkScanMessage.h"
#include "interfaces/nas/RilRequestStartNetworkScanMessage.h"
#include "interfaces/nas/RilRequestGetPrefNetworkTypeMessage.h"
#include "interfaces/nas/RilRequestSetPrefNetworkTypeMessage.h"
#include "interfaces/nas/RilRequestGetSignalStrengthMessage.h"
#include "interfaces/nas/RilRequestGetVoiceRadioTechMessage.h"
#include "interfaces/nas/RilRequestSetCdmaSubscriptionSourceMessage.h"
#include "interfaces/nas/RilRequestExitEmergencyCallbackMessage.h"
#include "interfaces/nas/RilRequestScreenStateMessage.h"
#include "interfaces/nas/RilRequestSendDeviceStateMessage.h"
#include "interfaces/nas/RilRequestGetNeighborCellIdsMessage.h"
#include "interfaces/nas/RilRequestCdmaSubscriptionMessage.h"
#include "interfaces/nas/RilRequestSetCdmaRoamingPrefMessage.h"
#include "interfaces/nas/RilRequestQueryCdmaRoamingPrefMessage.h"
#include "interfaces/nas/RilRequestGetCdmaSubscriptionSourceMessage.h"
#include "interfaces/nas/RilRequestSetLocationUpdateMessage.h"
#include "interfaces/nas/RilRequestQueryAvailBandModeMessage.h"
#include "interfaces/nas/RilRequestSetBandModeMessage.h"
#include "interfaces/nas/RilRequestSetUiccSubsMessage.h"
#include "interfaces/nas/RilRequestGetRadioCapMessage.h"
#include "interfaces/nas/RilRequestSetRadioCapMessage.h"
#include "interfaces/nas/RilRequestShutDownMessage.h"
#include "interfaces/nas/RilRequestEnableModemMessage.h"
#include "interfaces/nas/RilRequestGetModemStackStatusMessage.h"
#include "interfaces/nas/RilRequestSetSysSelChannelsMessage.h"
#include "interfaces/nas/RilRequestGetPhoneCapabilityMessage.h"
#include "interfaces/nas/RilRequestSetUnsolRespFilterMessage.h"
#include "interfaces/nas/RilRequestSetUnsolCellInfoListRateMessage.h"

// 5G related
#include "interfaces/nas/RilRequestSet5GStatusMessage.h"
#include "interfaces/nas/RilRequestQuery5GStatusMessage.h"
#include "interfaces/nas/RilRequestQueryNrDcParamMessage.h"
#include "interfaces/nas/RilRequestQueryNrBearAllocationMessage.h"
#include "interfaces/nas/RilRequestQueryNrSignalStrengthMessage.h"
#include "interfaces/nas/RilRequestQueryUpperLayerIndInfoMessage.h"
#include "interfaces/nas/RilRequestQuery5gConfigInfoMessage.h"
#include "interfaces/nas/RilRequestEnableEndcMessage.h"
#include "interfaces/nas/RilRequestQueryEndcStatusMessage.h"

//Data Unsol Messages
#include "modules/nas/NasDataCache.h"
#include "CallStatusMessage.h"
#include "DataSysStatusMessage.h"
#include "SetPrefDataTechMessage.h"
#include "DsdSysStatusMessage.h"
#ifndef QMI_RIL_UTF
#include "DataRegistrationStateMessage.h"
#endif
#include "NewDDSInfoMessage.h"
#include "DDSStatusFollowupMessage.h"

#include "framework/GenericCallback.h"
#include "framework/SolicitedMessage.h"
#include "framework/Message.h"
#include "framework/add_message_id.h"
#include <framework/AddPendingMessageList.h>

enum class NasSettingResultCode {
    SUCCESS = RIL_E_SUCCESS,
    FAILURE = RIL_E_INTERNAL_ERR,
};

/** NasSetSignalStrengthCriteriaSupported.
 * This message informs the Nas module that the
 * SetSignalStrengthReportingCriteria request is supported.
 * If the parameter "supported" is true, it will adjust its
 * call to QMI_NAS_CONFIG_SIG_INFO2 to prevent overriding
 * user-provided values.
 */
class NasSetSignalStrengthCriteriaSupported : public SolicitedMessage<NasSettingResultCode>,
                              public add_message_id<NasSetSignalStrengthCriteriaSupported>
{
 private:
  bool mSupported;

 public:
 static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.nas.SetSignalStrengthCriteriaSupported";
 NasSetSignalStrengthCriteriaSupported() = delete;
  ~NasSetSignalStrengthCriteriaSupported() {}

  inline NasSetSignalStrengthCriteriaSupported(bool supported):
      SolicitedMessage<NasSettingResultCode>(get_class_message_id()),
      mSupported(supported)
  {
    mName = MESSAGE_NAME;
  }

  inline bool isSupported() {
    return mSupported;
  }

  string dump() {
      return mName + "[ mSupported = " + (mSupported ? "true" : "false") + "]";
  }
};
/** NasEnablePhysChanConfigReporting.
 * This message informs the Nas module whether it should
 * report the physical channel config information.
 * If the parameter "enabled" is true, the indication will be
 * enabled.
 */
class NasEnablePhysChanConfigReporting : public SolicitedMessage<NasSettingResultCode>,
                              public add_message_id<NasEnablePhysChanConfigReporting>
{
 private:
  bool mEnable;

 public:
 using cb_t = GenericCallback<NasSettingResultCode>::cb_t;
 static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.nas.EnablePhysChanConfigReporting";
 NasEnablePhysChanConfigReporting() = delete;
  ~NasEnablePhysChanConfigReporting() {}

  inline NasEnablePhysChanConfigReporting(bool enabled, std::weak_ptr<MessageContext> ctx, cb_t callbackfn):
      SolicitedMessage<NasSettingResultCode>(MESSAGE_NAME, ctx, get_class_message_id()),
      mEnable(enabled)
  {
      GenericCallback<NasSettingResultCode> cb(callbackfn);
      setCallback(&cb);
  }

  inline bool isEnable() {
    return mEnable;
  }

  string dump() {
      return mName + "[ mEnable = " + (mEnable ? "true" : "false") + "]";
  }
};

int32_t toKhz(nas_bandwidth_enum_type_v01 bw);
int32_t toKhz(nas_lte_cphy_ca_bandwidth_enum_v01 bw);

class NasModule : public Module, public AddPendingMessageList {
  public:
    NasModule();
    ~NasModule();
    void init();
#ifdef QMI_RIL_UTF
    void qcrilHalNasModuleCleanup();
#endif

  private:
    bool mReady = false;
    qtimutex::QtiRecursiveMutex mMutex;
    bool mSetSignalStrengthCriteriaSupported = false;
    PhysChanInfo chaninfo;

  public:
    bool getSetSignalStrengthCriteriaSupported() {
        return mSetSignalStrengthCriteriaSupported;
    }
    int32_t getLteBw();
    void resetPhysChanConfig();
    void reportCurrentPhysChanConfig();
    bool isDmsEndpointReady();
  private:
    void handleQcrilInit(std::shared_ptr<Message> msg);
    void handleNasQmiIndMessage(std::shared_ptr<Message> msg);
    void handleQmiAsyncRespMessage(std::shared_ptr<QmiAsyncResponseMessage> msg);
    void handleNasEndpointStatusIndMessage(std::shared_ptr<Message> msg);

    void handleGetPreferredNetworkType(std::shared_ptr<RilRequestGetPrefNetworkTypeMessage> msg);
    void handleSetPreferredNetworkType(std::shared_ptr<RilRequestSetPrefNetworkTypeMessage> msg);
    void handleQuery5gStatus(std::shared_ptr<RilRequestQuery5GStatusMessage> msg);
    void handleQueryEndcDcnr(std::shared_ptr<RilRequestQueryNrDcParamMessage> msg);
    void handleQueryNrBearerAllocation(std::shared_ptr<RilRequestQueryNrBearAllocationMessage> msg);
    void handleQueryNrSignalStrength(std::shared_ptr<RilRequestQueryNrSignalStrengthMessage> shared_msg);
    void handleSetCdmaSubscriptionSource(std::shared_ptr<RilRequestSetCdmaSubscriptionSourceMessage> msg);
    void handleExitEmergencyCallbackMode(std::shared_ptr<RilRequestExitEmergencyCallbackMessage> msg);
    void handleGetVoiceRadioTech(std::shared_ptr<RilRequestGetVoiceRadioTechMessage> msg);
    void handleSetUiccSubscription(std::shared_ptr<RilRequestSetUiccSubsMessage> msg);
    void handleQueryUpperLayerIndInfo(std::shared_ptr<RilRequestQueryUpperLayerIndInfoMessage> msg);
    void handleQuery5gConfigInfo(std::shared_ptr<RilRequestQuery5gConfigInfoMessage> msg);
    void handleEnableEndc(std::shared_ptr<RilRequestEnableEndcMessage> msg);
    void handleQueryEndcStatus(std::shared_ptr<RilRequestQueryEndcStatusMessage> msg);

    void handleGetDcRtInfo(std::shared_ptr<RilRequestMessage> msg);
    void handleShutdown(std::shared_ptr<RilRequestShutDownMessage> msg);
    void handleGetRadioCapability(std::shared_ptr<RilRequestGetRadioCapMessage> msg);
    void handleSetRadioCapability(std::shared_ptr<RilRequestSetRadioCapMessage> msg);
    void handleQueryNetworkSelectionMode(std::shared_ptr<RilRequestQueryNetworkSelectModeMessage> msg);
    void handleSetBandMode(std::shared_ptr<RilRequestSetBandModeMessage> msg);
    void handleGetNeighboringCellIds(std::shared_ptr<RilRequestGetNeighborCellIdsMessage> msg);
    void handleSetNetworkSelectionAutomatic(
        std::shared_ptr<RilRequestSetNetworkSelectionAutoMessage> msg);
    void handleSetNetworkSelectionManual(
        std::shared_ptr<RilRequestSetNetworkSelectionManualMessage> msg);
    void handleVoiceRegistrationState(std::shared_ptr<RilRequestGetVoiceRegistrationMessage> msg);
    void handleDataRegistrationState(std::shared_ptr<RilRequestGetDataRegistrationMessage> msg);
    void handleOperator(std::shared_ptr<RilRequestOperatorMessage> msg);
    void handleAllowData(std::shared_ptr<RilRequestAllowDataMessage> msg);
    void handleCdmaSubscription(std::shared_ptr<RilRequestCdmaSubscriptionMessage> msg);
    void handleCdmaSetRoamingPreference(std::shared_ptr<RilRequestSetCdmaRoamingPrefMessage> msg);
    void handleSetLocationUpdates(std::shared_ptr<RilRequestSetLocationUpdateMessage> msg);
    void handleQueryAvailableNetworks(std::shared_ptr<RilRequestQueryAvailNetworkMessage> msg);
    void handleQueryAvailableBandMode(std::shared_ptr<RilRequestQueryAvailBandModeMessage> msg);
    void handleSignalStrength(std::shared_ptr<RilRequestGetSignalStrengthMessage> msg);
    void handleSetScreenState(std::shared_ptr<RilRequestScreenStateMessage> msg);
    void handleSetDeviceState(std::shared_ptr<RilRequestSendDeviceStateMessage> msg);
    void handleSetUnsolRespFilter(std::shared_ptr<RilRequestSetUnsolRespFilterMessage> msg);
    void handleStartNetworkScan(std::shared_ptr<RilRequestStartNetworkScanMessage> msg);
    void handleStopNetworkScan(std::shared_ptr<RilRequestStopNetworkScanMessage> msg);
    void handleGetModemStackStatus(
            std::shared_ptr<RilRequestGetModemStackStatusMessage> msg);
    void handleEnableModem(
            std::shared_ptr<RilRequestEnableModemMessage> msg);
    void handleSetSystemSelectionChannels(
            std::shared_ptr<RilRequestSetSysSelChannelsMessage> msg);
    void handleGetPhoneCapability(
            std::shared_ptr<RilRequestGetPhoneCapabilityMessage> msg);
    void handleSetUnsolCellInfoListRateMessage(
            std::shared_ptr<RilRequestSetUnsolCellInfoListRateMessage> msg);
    void handleCdmaQueryRoamingPreference(
            std::shared_ptr<RilRequestQueryCdmaRoamingPrefMessage> msg);
    void handleCdmaGetSubscriptionSource(
            std::shared_ptr<RilRequestGetCdmaSubscriptionSourceMessage> msg);

    // 5G related:
    void handleSet5GStatus(std::shared_ptr<RilRequestSet5GStatusMessage> msg);

    // UIM handling
    void handleUimProvisioningStatusIndMsg(std::shared_ptr<UimProvisionStatusIndMsg> msg);
    void handleUimCardStatusIndMsg(std::shared_ptr<UimCardStatusIndMsg> msg);
    void handleUimCardStatusChangeIndMsg(std::shared_ptr<UimCardStatusChangeIndMsg> msg);
    void handleUimCardAppStatusIndMsg(std::shared_ptr<UimCardAppStatusIndMsg> msg);

    // IMS handling
    void handleImsServiceStatusInd(std::shared_ptr<ImsServiceStatusInd> msg);
    void handleImsWfcStatusInd(std::shared_ptr<ImsWfcSettingsStatusInd> msg);
    void handleImsPdpStatusInd(std::shared_ptr<ImsPdpStatusInd> msg);
    void handleSetVoiceDomainPreferenceRequest(std::shared_ptr<NasSetVoiceDomainPreferenceRequest> msg);

    //Handlers for Messages from Data
    void handleEmbmsEnableDataConMessage(std::shared_ptr<NasEmbmsEnableDataConMessage> msg);
    void handleEmbmsDisableDataMessage(std::shared_ptr<NasEmbmsDisableDataMessage> msg);
    void handleGetPhyChanConfigMessage(std::shared_ptr<NasGetPhyChanConfigMessage> msg);

    //Data Indication Handlers
    void handleDataCallStatusMessage(std::shared_ptr<rildata::CallStatusMessage> msg);
    void handleDataSetPrefDataTechMessage(std::shared_ptr<rildata::SetPrefDataTechMessage> msg);
    void handleDataSysStatusMessage(std::shared_ptr<rildata::DataSysStatusMessage> msg);
    void handleDataDsdSysStatusMessage(std::shared_ptr<rildata::DsdSysStatusMessage> msg);
#ifndef QMI_RIL_UTF
    void handleDataRegistrationStateMessage(std::shared_ptr<rildata::DataRegistrationStateMessage> msg);
#endif
    void handleDataNewDDSInfoMessage(std::shared_ptr<rildata::NewDDSInfoMessage> msg);
    void handleDataDDSStatusFollowupMessage(std::shared_ptr<rildata::DDSStatusFollowupMessage> msg);

    // Nas API
    void handlePsAttachDetachMessage(std::shared_ptr<NasSetPsAttachDetachMessage> msg);
    void handleNasGetSysInfoMessage(std::shared_ptr<NasGetSysInfoMessage> msg);

    // Handler for DMS Endpoint Status Indications
    void handleDmsEndpointStatusIndMsg(std::shared_ptr<Message> msg);

    // Settings
    void handleSetSignalStrengthCriteriaSupported(
            std::shared_ptr<NasSetSignalStrengthCriteriaSupported> msg);
    void handleEnablePhysChanConfigReporting(
            std::shared_ptr<NasEnablePhysChanConfigReporting> msg);
    std::shared_ptr<std::vector<NasPhysChanInfo>> getCurrentPhysChanConfig();

    // Physical channel configuration
    inline bool shouldReportScell(const nas_lte_cphy_scell_info_type_v01 &scell_info);
    inline int32_t getBandwidth(nas_radio_if_enum_v01 radio_if);
    inline int32_t getBandwidth();
    void handleQmiNasRfBandInfoMessage(
            std::shared_ptr<QmiNasRfBandInfoMessage> bandInfoMsg);
    void handleQmiNasLteCphyCaIndMessage(
            std::shared_ptr<QmiNasLteCphyCaIndMessage> lteCphyMsg);
    bool getPhysChanConfigFromModem();
};

NasModule &getNasModule();

