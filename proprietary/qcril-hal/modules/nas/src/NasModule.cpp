/******************************************************************************
#  Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#define TAG "[NasModule]"
#include <cstring>
#include <framework/ModuleLooper.h>
#include <framework/QcrilInitMessage.h>
#include <framework/PolicyManager.h>
#include <framework/SingleDispatchRestriction.h>
#include <framework/SingleDispatchRestrictionHandleDuplicate.h>
#include <framework/SingleDispatchRestrictionHandleDuplicateOnDispatch.h>
#include <framework/RejectSimultaneousDispatchRestriction.h>
#include <framework/ThreadPoolManager.h>

#include <modules/android/ril_message_factory.h>

#include "modules/nas/qcril_qmi_prov.h"
#include "qcril_qmi_client.h"
#include "qcril_am.h"

#include "modules/nas/qcril_qmi_nas.h"
#include "modules/nas/qcril_qmi_nas2.h"
#include "modules/nas/qcril_arb.h"
#include "modules/nas/qcril_nas_legacy.h"
#include "modules/nas/NasUimHeaders.h"

#include "modules/nas/NasModule.h"
#include "modules/android/LegacyHandlerSubModule.h"
#include "modules/android/LegacyHandlerMessage.h"
#include "modules/qmi/QmiIndMessage.h"
#include "modules/qmi/EndpointStatusIndMessage.h"
#include "modules/qmi/ModemEndPointFactory.h"
#include "modules/qmi/NasModemEndPoint.h"
#include "modules/nas/NasImsHelper.h"
#include "modules/qmi/QmiNasSetupRequestCallback.h"

//Data related
#include "modules/nas/NasDataCache.h"
#ifndef QMI_RIL_UTF
#include "SetInitialAttachApnRequestMessage.h"
#endif
#include "DataCommon.h"

using rildata::CallStatusMessage;
using rildata::DataSysStatusMessage;
using rildata::SetPrefDataTechMessage;
using rildata::DsdSysStatusMessage;
#ifndef QMI_RIL_UTF
using rildata::DataRegistrationStateMessage;
#endif
using rildata::NewDDSInfoMessage;
using rildata::DDSStatusFollowupMessage;

#include "telephony/ril.h"

QCRIL_ANDROID_DECLARE_REQUEST(RIL_REQUEST_GET_DC_RT_INFO, void, RIL_DcRtInfo*);
QCRIL_ANDROID_DEFINE_REQUEST(RIL_REQUEST_GET_DC_RT_INFO);

static load_module<NasModule> nas_module;

NasModule &getNasModule() {
    return (nas_module.get_module());
}

//TODO: Remove this while cleaning up
static qcril_arb_pref_data_tech_e_type map_data_pref_data_tech_to_arb_pref_data_tech(qcril_data_pref_data_tech_e_type pref_data_tech);
static qcril_qmi_nas_data_call_event_type map_data_call_event_to_internal(qcril_data_net_evt_t evt);
static dds_status_info map_dds_status_info_from_data(DDSStatusInfo status_info);
/*
 * 1. Indicate your preference for looper.
 * 2. Subscribe to the list of messages via mMessageHandler.
 * 3. Follow RAII practice.
 */
NasModule::NasModule() : AddPendingMessageList("NasModule") {
  mName = "NasModule";
  ModemEndPointFactory<NasModemEndPoint>::getInstance().buildEndPoint();

  using std::placeholders::_1;
  mMessageHandler = {
    HANDLER(QcrilInitMessage, NasModule::handleQcrilInit),

    HANDLER(RilRequestGetVoiceRegistrationMessage, NasModule::handleVoiceRegistrationState),
    HANDLER(RilRequestGetDataRegistrationMessage, NasModule::handleDataRegistrationState),
    HANDLER(RilRequestOperatorMessage, NasModule::handleOperator),
    HANDLER(RilRequestQueryNetworkSelectModeMessage, NasModule::handleQueryNetworkSelectionMode),
    HANDLER(RilRequestAllowDataMessage, NasModule::handleAllowData),
    HANDLER(RilRequestQueryAvailNetworkMessage, NasModule::handleQueryAvailableNetworks),
    HANDLER(RilRequestSetNetworkSelectionManualMessage, NasModule::handleSetNetworkSelectionManual),
    HANDLER(RilRequestSetNetworkSelectionAutoMessage, NasModule::handleSetNetworkSelectionAutomatic),
    HANDLER(RilRequestStopNetworkScanMessage, NasModule::handleStopNetworkScan),
    HANDLER(RilRequestStartNetworkScanMessage, NasModule::handleStartNetworkScan),
    HANDLER(RilRequestGetPrefNetworkTypeMessage, NasModule::handleGetPreferredNetworkType),
    HANDLER(RilRequestSetPrefNetworkTypeMessage, NasModule::handleSetPreferredNetworkType),
    HANDLER(RilRequestGetSignalStrengthMessage, NasModule::handleSignalStrength),
    HANDLER(RilRequestGetVoiceRadioTechMessage, NasModule::handleGetVoiceRadioTech),
    HANDLER(RilRequestSetCdmaSubscriptionSourceMessage, NasModule::handleSetCdmaSubscriptionSource),
    HANDLER(RilRequestExitEmergencyCallbackMessage, NasModule::handleExitEmergencyCallbackMode),
    HANDLER(RilRequestScreenStateMessage, NasModule::handleSetScreenState),
    HANDLER(RilRequestSendDeviceStateMessage, NasModule::handleSetDeviceState),
    HANDLER(RilRequestGetNeighborCellIdsMessage, NasModule::handleGetNeighboringCellIds),
    HANDLER(RilRequestCdmaSubscriptionMessage, NasModule::handleCdmaSubscription),
    HANDLER(RilRequestSetCdmaRoamingPrefMessage, NasModule::handleCdmaSetRoamingPreference),
    HANDLER(RilRequestQueryCdmaRoamingPrefMessage, NasModule::handleCdmaQueryRoamingPreference),
    HANDLER(RilRequestGetCdmaSubscriptionSourceMessage, NasModule::handleCdmaGetSubscriptionSource),
    HANDLER(RilRequestSetLocationUpdateMessage, NasModule::handleSetLocationUpdates),
    HANDLER(RilRequestQueryAvailBandModeMessage, NasModule::handleQueryAvailableBandMode),
    HANDLER(RilRequestSetBandModeMessage, NasModule::handleSetBandMode),
    HANDLER(RilRequestSetUiccSubsMessage, NasModule::handleSetUiccSubscription),
    HANDLER(RilRequestGetRadioCapMessage, NasModule::handleGetRadioCapability),
    HANDLER(RilRequestSetRadioCapMessage, NasModule::handleSetRadioCapability),
    HANDLER(RilRequestShutDownMessage, NasModule::handleShutdown),
    HANDLER(RilRequestSet5GStatusMessage, NasModule::handleSet5GStatus),
    HANDLER(RilRequestQuery5GStatusMessage, NasModule::handleQuery5gStatus),
    HANDLER(RilRequestQueryNrDcParamMessage, NasModule::handleQueryEndcDcnr),
    HANDLER(RilRequestQueryNrBearAllocationMessage, NasModule::handleQueryNrBearerAllocation),
    HANDLER(RilRequestQueryNrSignalStrengthMessage, NasModule::handleQueryNrSignalStrength),
    HANDLER(RilRequestQueryUpperLayerIndInfoMessage, NasModule::handleQueryUpperLayerIndInfo),
    HANDLER(RilRequestQuery5gConfigInfoMessage, NasModule::handleQuery5gConfigInfo),
    HANDLER(RilRequestEnableEndcMessage, NasModule::handleEnableEndc),
    HANDLER(RilRequestQueryEndcStatusMessage, NasModule::handleQueryEndcStatus),
    LEGACY_HANDLER(RIL_REQUEST_GET_DC_RT_INFO, NasModule::handleGetDcRtInfo),
    HANDLER(RilRequestSetUnsolRespFilterMessage, NasModule::handleSetUnsolRespFilter),
    HANDLER(NasSetSignalStrengthCriteriaSupported, NasModule::handleSetSignalStrengthCriteriaSupported),
    HANDLER(NasEnablePhysChanConfigReporting, NasModule::handleEnablePhysChanConfigReporting),
    HANDLER(QmiNasRfBandInfoMessage, NasModule::handleQmiNasRfBandInfoMessage),
    HANDLER(QmiNasLteCphyCaIndMessage, NasModule::handleQmiNasLteCphyCaIndMessage),
    HANDLER(RilRequestSetSysSelChannelsMessage, NasModule::handleSetSystemSelectionChannels),
    HANDLER(RilRequestEnableModemMessage, NasModule::handleEnableModem),
    HANDLER(RilRequestGetModemStackStatusMessage, NasModule::handleGetModemStackStatus),
    HANDLER(RilRequestGetPhoneCapabilityMessage, NasModule::handleGetPhoneCapability),
    HANDLER(RilRequestSetUnsolCellInfoListRateMessage, NasModule::handleSetUnsolCellInfoListRateMessage),


    {REG_MSG("NAS_QMI_IND"),
             std::bind(&NasModule::handleNasQmiIndMessage, this, _1)},
    // End Point Status Indication
    {REG_MSG("NAS_ENDPOINT_STATUS_IND"),
             std::bind(&NasModule::handleNasEndpointStatusIndMessage, this, _1)},
    // Qmi Async Response
    HANDLER(QmiAsyncResponseMessage, NasModule::handleQmiAsyncRespMessage),

    // UIM indications
    HANDLER(UimProvisionStatusIndMsg, NasModule::handleUimProvisioningStatusIndMsg),
    HANDLER(UimCardStatusIndMsg, NasModule::handleUimCardStatusIndMsg),
    HANDLER(UimCardStatusChangeIndMsg, NasModule::handleUimCardStatusChangeIndMsg),
    HANDLER(UimCardAppStatusIndMsg, NasModule::handleUimCardAppStatusIndMsg),

    // IMS handling
    HANDLER(ImsServiceStatusInd, NasModule::handleImsServiceStatusInd),
    HANDLER(ImsWfcSettingsStatusInd, NasModule::handleImsWfcStatusInd),
    HANDLER(ImsPdpStatusInd, NasModule::handleImsPdpStatusInd),
    HANDLER(NasSetVoiceDomainPreferenceRequest, NasModule::handleSetVoiceDomainPreferenceRequest),

    //Messages from Data
    HANDLER(NasEmbmsEnableDataConMessage, NasModule::handleEmbmsEnableDataConMessage),
    HANDLER(NasEmbmsDisableDataMessage, NasModule::handleEmbmsDisableDataMessage),
    HANDLER(NasGetPhyChanConfigMessage, NasModule::handleGetPhyChanConfigMessage),

#ifndef QMI_RIL_UTF
    //Data indications
    HANDLER(CallStatusMessage, NasModule::handleDataCallStatusMessage),
    HANDLER(SetPrefDataTechMessage, NasModule::handleDataSetPrefDataTechMessage),
    HANDLER(DataSysStatusMessage, NasModule::handleDataSysStatusMessage),
    HANDLER(DsdSysStatusMessage, NasModule::handleDataDsdSysStatusMessage),
    HANDLER(DataRegistrationStateMessage, NasModule::handleDataRegistrationStateMessage),
    HANDLER(NewDDSInfoMessage, NasModule::handleDataNewDDSInfoMessage),
    HANDLER(DDSStatusFollowupMessage, NasModule::handleDataDDSStatusFollowupMessage),
#endif

    HANDLER(NasSetPsAttachDetachMessage, NasModule::handlePsAttachDetachMessage),
    HANDLER(NasGetSysInfoMessage, NasModule::handleNasGetSysInfoMessage),
  };
}

/* Follow RAII.
*/
NasModule::~NasModule() {}

/*
 * Module specific initialization that does not belong to RAII .
 */
void NasModule::init()
{
  Module::init();
  qcril_qmi_nas_pre_init();
  QCRIL_ANDROID_REGISTER_REQUEST_WITH_RESTRICTION(RIL_REQUEST_GET_DC_RT_INFO, callback,
    std::make_shared<SingleDispatchRestriction>());

  PolicyManager::getInstance().setMessageRestriction(std::vector<std::string>{
        RilRequestGetVoiceRegistrationMessage::MESSAGE_NAME},
        std::make_shared<SingleDispatchRestrictionHandleDuplicate>());

  PolicyManager::getInstance().setMessageRestriction(std::vector<std::string>{
        RilRequestGetDataRegistrationMessage::MESSAGE_NAME},
        std::make_shared<SingleDispatchRestrictionHandleDuplicate>());

  PolicyManager::getInstance().setMessageRestriction(std::vector<std::string>{
        RilRequestOperatorMessage::MESSAGE_NAME},
        std::make_shared<SingleDispatchRestrictionHandleDuplicate>());

  PolicyManager::getInstance().setMessageRestriction(std::vector<std::string>{
        RilRequestQueryNetworkSelectModeMessage::MESSAGE_NAME},
        std::make_shared<SingleDispatchRestrictionHandleDuplicate>());

  PolicyManager::getInstance().setMessageRestriction(std::vector<std::string>{
        RilRequestSetNetworkSelectionManualMessage::MESSAGE_NAME},
        std::make_shared<SingleDispatchRestriction>());

  PolicyManager::getInstance().setMessageRestriction(std::vector<std::string>{
        RilRequestSetNetworkSelectionAutoMessage::MESSAGE_NAME},
        std::make_shared<SingleDispatchRestriction>());

  PolicyManager::getInstance().setMessageRestriction(std::vector<std::string>{
        RilRequestStopNetworkScanMessage::MESSAGE_NAME},
        std::make_shared<SingleDispatchRestriction>());

  PolicyManager::getInstance().setMessageRestriction(std::vector<std::string>{
        RilRequestGetSignalStrengthMessage::MESSAGE_NAME},
        std::make_shared<SingleDispatchRestriction>());

  PolicyManager::getInstance().setMessageRestriction(std::vector<std::string>{
        RilRequestGetVoiceRadioTechMessage::MESSAGE_NAME},
        std::make_shared<SingleDispatchRestriction>());

  PolicyManager::getInstance().setMessageRestriction(std::vector<std::string>{
        RilRequestSetCdmaSubscriptionSourceMessage::MESSAGE_NAME},
        std::make_shared<SingleDispatchRestriction>());

  PolicyManager::getInstance().setMessageRestriction(std::vector<std::string>{
        RilRequestExitEmergencyCallbackMessage::MESSAGE_NAME},
        std::make_shared<SingleDispatchRestriction>());

  PolicyManager::getInstance().setMessageRestriction(std::vector<std::string>{
        RilRequestScreenStateMessage::MESSAGE_NAME},
        std::make_shared<SingleDispatchRestriction>());

  PolicyManager::getInstance().setMessageRestriction(std::vector<std::string>{
        RilRequestGetNeighborCellIdsMessage::MESSAGE_NAME},
        std::make_shared<SingleDispatchRestriction>());

  PolicyManager::getInstance().setMessageRestriction(std::vector<std::string>{
        RilRequestCdmaSubscriptionMessage::MESSAGE_NAME},
        std::make_shared<SingleDispatchRestriction>());

  PolicyManager::getInstance().setMessageRestriction(std::vector<std::string>{
        RilRequestSetCdmaRoamingPrefMessage::MESSAGE_NAME},
        std::make_shared<SingleDispatchRestriction>());

  PolicyManager::getInstance().setMessageRestriction(std::vector<std::string>{
        RilRequestSetLocationUpdateMessage::MESSAGE_NAME},
        std::make_shared<SingleDispatchRestriction>());

  PolicyManager::getInstance().setMessageRestriction(std::vector<std::string>{
        RilRequestQueryAvailBandModeMessage::MESSAGE_NAME},
        std::make_shared<SingleDispatchRestriction>());

  PolicyManager::getInstance().setMessageRestriction(std::vector<std::string>{
        RilRequestSetBandModeMessage::MESSAGE_NAME},
        std::make_shared<SingleDispatchRestriction>());

  PolicyManager::getInstance().setMessageRestriction(std::vector<std::string>{
        RilRequestSetUiccSubsMessage::MESSAGE_NAME},
        std::make_shared<SingleDispatchRestriction>());

  PolicyManager::getInstance().setMessageRestriction(std::vector<std::string>{
        RilRequestGetRadioCapMessage::MESSAGE_NAME},
        std::make_shared<SingleDispatchRestriction>());

  PolicyManager::getInstance().setMessageRestriction(std::vector<std::string>{
        RilRequestShutDownMessage::MESSAGE_NAME},
        std::make_shared<SingleDispatchRestriction>());

  std::vector<std::string> ring1{
    RilRequestGetPrefNetworkTypeMessage::MESSAGE_NAME,
    RilRequestSetPrefNetworkTypeMessage::MESSAGE_NAME,
    RilRequestSet5GStatusMessage::MESSAGE_NAME,
    RilRequestQuery5GStatusMessage::MESSAGE_NAME
  };
  PolicyManager::getInstance().setMessageRestriction(ring1,
         std::make_shared<SingleDispatchRestriction>());

#ifndef QMI_RIL_UTF
  std::vector<std::string> ring2{SetInitialAttachApnRequestMessage::MESSAGE_NAME,
    RilRequestAllowDataMessage::MESSAGE_NAME};
  PolicyManager::getInstance().setMessageRestriction(ring2,
         std::make_shared<SingleDispatchRestriction>());
#endif

  std::vector<std::string> rejsimul{ RilRequestQueryAvailNetworkMessage::MESSAGE_NAME,
    RilRequestSetRadioCapMessage::MESSAGE_NAME };
  PolicyManager::getInstance().setMessageRestriction(rejsimul,
         std::make_shared<RejectSimultaneousDispatchRestriction>());

  std::vector<std::string> setsysPref {RilRequestSetSysSelChannelsMessage::MESSAGE_NAME};
  PolicyManager::getInstance().setMessageRestriction(setsysPref,
        std::make_shared<SingleDispatchRestriction>());


  std::vector<std::string> ring3{RilRequestEnableModemMessage::MESSAGE_NAME,
   RilRequestGetModemStackStatusMessage::MESSAGE_NAME};

  PolicyManager::getInstance().setMessageRestriction(ring3,
         std::make_shared<SingleDispatchRestriction>());

  PolicyManager::getInstance().setMessageRestriction(
      std::vector<std::string>{ QmiNasLteCphyCaIndMessage::MESSAGE_NAME },
      std::make_shared<SingleDispatchRestrictionHandleDuplicateOnDispatch>());
  PolicyManager::getInstance().setMessageRestriction(
      std::vector<std::string>{ QmiNasRfBandInfoMessage::MESSAGE_NAME },
      std::make_shared<SingleDispatchRestrictionHandleDuplicateOnDispatch>());
}

bool NasModule::isDmsEndpointReady() {
  return qcril_qmi_client_is_client_initialized(QCRIL_QMI_CLIENT_DMS);
}

/*
 * List of individual private handlers for the subscribed messages.
 */

void NasModule::handleUimCardStatusChangeIndMsg(std::shared_ptr<UimCardStatusChangeIndMsg> msg) {
  Log::getInstance().d("[NasModule]: Handling msg = " + msg->dump());

  if (isDmsEndpointReady()) {
    qcril_qmi_nas_trigger_bootup_optimization();
  }
}

void NasModule::handleUimCardAppStatusIndMsg(std::shared_ptr<UimCardAppStatusIndMsg> msg) {
  qcril_request_return_type ret_ptr;

  Log::getInstance().d("[NasModule]: Handling msg = " + msg->dump());
  qcril_request_params_type params_ptr;
  std::memset(&params_ptr, 0, sizeof(params_ptr));
  params_ptr.data = static_cast<void *>(new char[sizeof(RIL_UIM_AppStatus)]);
  if(params_ptr.data)
  {
    std::memcpy(params_ptr.data, &msg->get_app_info(), sizeof(RIL_UIM_AppStatus));
    params_ptr.datalen = sizeof(RIL_UIM_AppStatus);
    params_ptr.modem_id = QCRIL_DEFAULT_MODEM_ID;

    qcril_qmi_nas_event_app_status_update (&params_ptr, &ret_ptr);

    delete [] (char*)params_ptr.data;
  }
  else
  {
    Log::getInstance().d("[NasModule]: Memory allocation failure");
  }
}


void NasModule::handleUimCardStatusIndMsg(std::shared_ptr<UimCardStatusIndMsg> msg) {
  qcril_request_return_type ret_ptr;

  Log::getInstance().d("[NasModule]: Handling msg = " + msg->dump());
  qcril_request_params_type params_ptr;
  std::memset(&params_ptr, 0, sizeof(params_ptr));
  params_ptr.data = static_cast<void *>(new char[sizeof(qcril_uim_card_info_type)]);
  if(params_ptr.data)
  {
    std::memcpy(params_ptr.data, &msg->get_card_status(), sizeof(qcril_uim_card_info_type));
    params_ptr.datalen = sizeof(qcril_uim_card_info_type);
    params_ptr.modem_id = QCRIL_DEFAULT_MODEM_ID;

    qcril_qmi_nas_event_card_status_update (&params_ptr, &ret_ptr);

    delete [] (char*)params_ptr.data;
  }
  else
  {
    Log::getInstance().d("[NasModule]: Memory allocation failure");
  }
}


void NasModule::handleUimProvisioningStatusIndMsg(std::shared_ptr<UimProvisionStatusIndMsg> msg) {
  qcril_request_return_type ret_ptr;

  Log::getInstance().d("[NasModule]: Handling msg = " + msg->dump());
  qcril_request_params_type params_ptr;
  std::memset(&params_ptr, 0, sizeof(params_ptr));
  params_ptr.data = static_cast<void *>(new char[sizeof(qcril_provision_info_type)]);
  if(params_ptr.data)
  {
    std::memcpy(params_ptr.data, &msg->get_provision_status_info(), sizeof(qcril_provision_info_type));
    params_ptr.datalen = sizeof(qcril_provision_info_type);
    params_ptr.modem_id = QCRIL_DEFAULT_MODEM_ID;

    if(msg->get_provision_type() == UIM_ACTIVATE_PROVISION_STATUS) {
        qcril_qmi_prov_subs_activate_followup (&params_ptr, &ret_ptr);
    } else {
        qcril_qmi_prov_subs_deactivate_followup (&params_ptr, &ret_ptr);

        delete [] (char*)params_ptr.data;
    }
  }
  else
  {
    Log::getInstance().d("[NasModule]: Memory allocation failure");
  }
}


void NasModule::handleNasQmiIndMessage(std::shared_ptr<Message> msg) {
  auto shared_indMsg(
        std::static_pointer_cast<QmiIndMessage>(msg));

  Log::getInstance().d("[NasModule]: Handling msg = " + msg->dump());
  QmiIndMsgDataStruct *indData = shared_indMsg->getData();
  if (indData != nullptr) {
    qcril_qmi_nas_unsolicited_indication_cb_helper(indData->msgId, indData->indData,
        indData->indSize);
  } else {
    Log::getInstance().d("Unexpected, null data from message");
  }
}

void NasModule::handleQmiAsyncRespMessage(std::shared_ptr<QmiAsyncResponseMessage> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  QmiAsyncRespData *asyncResp = msg->getData();
  if (asyncResp != nullptr && asyncResp->cb != nullptr) {
    asyncResp->cb(asyncResp->msgId, asyncResp->respData, asyncResp->respDataSize,
        asyncResp->cbData, asyncResp->traspErr);
  } else {
    Log::getInstance().d("Unexpected, null data from message");
  }
}


void NasModule::handleVoiceRegistrationState(std::shared_ptr<RilRequestGetVoiceRegistrationMessage> shared_msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());

    auto cbData = new ScheduleCallbackData(shared_msg);
    if (cbData) {
        ThreadPoolManager::getInstance().scheduleExecution(
            qcril_qmi_nas_request_registration_state_wrapper, cbData);
    } else {
        shared_msg->sendResponse(shared_msg, Message::Callback::Status::SUCCESS,
            std::make_shared<QcRilRequestMessageCallbackPayload>(RIL_E_NO_MEMORY, nullptr));
    }
}


void NasModule::handleDataRegistrationState(std::shared_ptr<RilRequestGetDataRegistrationMessage> shared_msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());

    auto cbData = new ScheduleCallbackData(shared_msg);
    if (cbData) {
        ThreadPoolManager::getInstance().scheduleExecution(
            qcril_qmi_nas_request_data_registration_state_wrapper, cbData);
    } else {
        shared_msg->sendResponse(shared_msg, Message::Callback::Status::SUCCESS,
            std::make_shared<QcRilRequestMessageCallbackPayload>(RIL_E_NO_MEMORY, nullptr));
    }
}


void NasModule::handleOperator(std::shared_ptr<RilRequestOperatorMessage> shared_msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());

    auto cbData = new ScheduleCallbackData(shared_msg);
    if (cbData) {
        ThreadPoolManager::getInstance().scheduleExecution(
            qcril_qmi_nas_request_operator_wrapper, cbData);
    } else {
        shared_msg->sendResponse(shared_msg, Message::Callback::Status::SUCCESS,
            std::make_shared<QcRilRequestMessageCallbackPayload>(RIL_E_NO_MEMORY, nullptr));
    }
}

void NasModule::handleQueryNetworkSelectionMode(
        std::shared_ptr<RilRequestQueryNetworkSelectModeMessage> shared_msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());

    auto cbData = new ScheduleCallbackData(shared_msg);
    if (cbData) {
        ThreadPoolManager::getInstance().scheduleExecution(
            qcril_qmi_nas_query_network_selection_mode_wrapper, cbData);
    } else {
        shared_msg->sendResponse(shared_msg, Message::Callback::Status::SUCCESS,
            std::make_shared<QcRilRequestMessageCallbackPayload>(RIL_E_NO_MEMORY, nullptr));
    }
}


void NasModule::handleAllowData(std::shared_ptr<RilRequestAllowDataMessage> shared_msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());

    qcril_qmi_nas_request_allow_data(shared_msg);
}


void NasModule::handleCdmaSubscription(std::shared_ptr<RilRequestCdmaSubscriptionMessage> shared_msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());

    qcril_qmi_nas_request_cdma_subscription(shared_msg);
}


void NasModule::handleCdmaSetRoamingPreference(std::shared_ptr<RilRequestSetCdmaRoamingPrefMessage> shared_msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());

    qcril_qmi_nas_request_set_roaming_preference(shared_msg);
}

void NasModule::handleCdmaQueryRoamingPreference(
    std::shared_ptr<RilRequestQueryCdmaRoamingPrefMessage> shared_msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());
    qcril_qmi_nas_request_query_roaming_preference(shared_msg);
}

void NasModule::handleCdmaGetSubscriptionSource(
    std::shared_ptr<RilRequestGetCdmaSubscriptionSourceMessage> shared_msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());
    qcril_qmi_nas_get_subscription_source(shared_msg);
}

void NasModule::handleSetLocationUpdates(std::shared_ptr<RilRequestSetLocationUpdateMessage> shared_msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());

    qcril_qmi_nas_set_location_updates(shared_msg);
}


void NasModule::handleQueryAvailableNetworks(std::shared_ptr<RilRequestQueryAvailNetworkMessage> shared_msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());

    qcril_qmi_nas_query_available_networks(shared_msg);
}

void NasModule::handleQueryAvailableBandMode(std::shared_ptr<RilRequestQueryAvailBandModeMessage> shared_msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());

    qcril_qmi_nas_query_available_band_mode(shared_msg);
}


void NasModule::handleSignalStrength(std::shared_ptr<RilRequestGetSignalStrengthMessage> shared_msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());

    qcril_qmi_nas_request_signal_strength(shared_msg);
}


void NasModule::handleSetPreferredNetworkType(std::shared_ptr<RilRequestSetPrefNetworkTypeMessage> shared_msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());

    qcril_qmi_nas_request_set_preferred_network_type(shared_msg);
}


void NasModule::handleGetPreferredNetworkType(std::shared_ptr<RilRequestGetPrefNetworkTypeMessage> shared_msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());

    qcril_qmi_nas_request_get_preferred_network_type(shared_msg);
}

void NasModule::handleSet5GStatus(std::shared_ptr<RilRequestSet5GStatusMessage> msg) {
    Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());

    switch (msg->getStatus()) {
        case FIVE_G_MODE_DISABLED:
          qcril_qmi_nas_request_disable_5g(msg);
          break;
        case FIVE_G_MODE_INCLUSIVE:
          qcril_qmi_nas_request_enable_5g(msg);
          break;
        case FIVE_G_MODE_EXCLUSIVE:
          qcril_qmi_nas_request_enable_5g_only(msg);
          break;
        default:
          Log::getInstance().d("[" + mName + "] Invalid 5G mode requested.");
          break;
    }
}

void NasModule::handleQuery5gStatus(std::shared_ptr<RilRequestQuery5GStatusMessage> msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + msg->dump());
    qcril_qmi_nas_request_query_5g_status(msg);
}

void NasModule::handleQueryEndcDcnr(std::shared_ptr<RilRequestQueryNrDcParamMessage> msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + msg->dump());
    qcril_qmi_nas_request_query_endc_dcnr(msg);
}

void NasModule::handleQueryNrBearerAllocation(std::shared_ptr<RilRequestQueryNrBearAllocationMessage> msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + msg->dump());
    qcril_qmi_nas_request_query_nr_bearer_allocation(msg);
}

void NasModule::handleQueryNrSignalStrength(std::shared_ptr<RilRequestQueryNrSignalStrengthMessage> msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + msg->dump());
    qcril_qmi_nas_request_nr_signal_strength(msg);
}

void NasModule::handleSetCdmaSubscriptionSource(
        std::shared_ptr<RilRequestSetCdmaSubscriptionSourceMessage> shared_msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());

    qcril_qmi_nas_set_subscription_source(shared_msg);
}

void NasModule::handleQueryUpperLayerIndInfo(std::shared_ptr<RilRequestQueryUpperLayerIndInfoMessage> msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + msg->dump());
    qcril_qmi_nas_request_query_upperlayer_ind_info(msg);
}

void NasModule::handleQuery5gConfigInfo(std::shared_ptr<RilRequestQuery5gConfigInfoMessage> msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + msg->dump());
    qcril_qmi_nas_request_query_5g_config_info(msg);
}

void NasModule::handleEnableEndc(std::shared_ptr<RilRequestEnableEndcMessage> msg) {
    Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
    qcril_qmi_nas_request_enable_endc(msg);
}

void NasModule::handleQueryEndcStatus(std::shared_ptr<RilRequestQueryEndcStatusMessage> msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + msg->dump());
    qcril_qmi_nas_request_query_endc_status(msg);
}

void NasModule::handleExitEmergencyCallbackMode(std::shared_ptr<RilRequestExitEmergencyCallbackMessage> shared_msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());

    qcril_qmi_nas_exit_emergency_callback_mode_msg(shared_msg);
}


void NasModule::handleGetVoiceRadioTech(std::shared_ptr<RilRequestGetVoiceRadioTechMessage> shared_msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());

    qcril_qmi_nas_request_radio_tech(shared_msg);
}


void NasModule::handleSetUiccSubscription(std::shared_ptr<RilRequestSetUiccSubsMessage> shared_msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());

    qcril_qmi_nas_set_uicc_subscription(shared_msg);
}


void NasModule::handleGetDcRtInfo(std::shared_ptr<RilRequestMessage> shared_msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());
    qcril_request_return_type ret_ptr;

    qcril_qmi_nas_request_get_dc_rt_info(
      &shared_msg->get_params(), &ret_ptr);
}


void NasModule::handleShutdown(std::shared_ptr<RilRequestShutDownMessage> shared_msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());

    qcril_qmi_nas_request_shutdown(shared_msg);
}


void NasModule::handleGetRadioCapability(std::shared_ptr<RilRequestGetRadioCapMessage> shared_msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());

    qcril_qmi_nas_get_radio_capability(shared_msg);
}


void NasModule::handleSetRadioCapability(std::shared_ptr<RilRequestSetRadioCapMessage> shared_msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());

    qcril_qmi_nas_set_radio_capability(shared_msg);
}


void NasModule::handleSetNetworkSelectionAutomatic(std::shared_ptr<RilRequestSetNetworkSelectionAutoMessage> shared_msg) {
  Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());

  qcril_qmi_nas_set_network_selection_automatic(shared_msg);
}


void NasModule::handleSetNetworkSelectionManual(
    std::shared_ptr<RilRequestSetNetworkSelectionManualMessage> shared_msg) {
  Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());

  qcril_qmi_nas_set_network_selection_manual(shared_msg);
}


void NasModule::handleSetBandMode(std::shared_ptr<RilRequestSetBandModeMessage> shared_msg) {
  Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());

  qcril_qmi_nas_set_band_mode(shared_msg);
}


void NasModule::handleGetNeighboringCellIds(std::shared_ptr<RilRequestGetNeighborCellIdsMessage> shared_msg) {
  Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());

  qcril_qmi_nas_get_neighboring_cell_ids(shared_msg);
}


void NasModule::handleSetScreenState(std::shared_ptr<RilRequestScreenStateMessage> shared_msg) {
  Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());

  qcril_qmi_nas_request_screen_state(shared_msg);
}

void NasModule::handleSetDeviceState(std::shared_ptr<RilRequestSendDeviceStateMessage> msg) {
  Log::getInstance().d("[NasModule]: Handling msg = " + msg->dump());

  qcril_qmi_nas_request_send_device_state(msg);
}

void NasModule::handleSetUnsolRespFilter(std::shared_ptr<RilRequestSetUnsolRespFilterMessage> msg) {
  QCRIL_LOG_FUNC_ENTRY("msg = %s", (msg ? msg->dump().c_str() : "nullptr"));
  qcril_qmi_nas_request_set_unsol_response_filter(msg);
  QCRIL_LOG_FUNC_RETURN();
}

void NasModule::handleSetUnsolCellInfoListRateMessage(
    std::shared_ptr<RilRequestSetUnsolCellInfoListRateMessage> msg) {
  QCRIL_LOG_FUNC_ENTRY("msg = %s", (msg ? msg->dump().c_str() : "nullptr"));
  RIL_Errno rilErr = RIL_E_INVALID_ARGUMENTS;
  if (msg != nullptr) {
    if (msg->hasRate()) {
      qcril_qmi_nas_set_cell_info_list_rate(msg->getRate());
      rilErr = RIL_E_SUCCESS;
    }
    std::shared_ptr<QcRilRequestMessageCallbackPayload> respPayload =
        std::make_shared<QcRilRequestMessageCallbackPayload>(rilErr, nullptr);
    msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
  }
  QCRIL_LOG_FUNC_RETURN();
}

void NasModule::handleStartNetworkScan(std::shared_ptr<RilRequestStartNetworkScanMessage> msg) {
  Log::getInstance().d("[NasModule]: Handling msg = " + msg->dump());
  Log::getInstance().d("[NasModule]: request Source = " +
                        std::to_string(static_cast<int>(msg->getRequestSource())));

  if(msg->getRequestSource() == RilRequestStartNetworkScanMessage::RequestSource::OEM) {
    qcril_qmi_nas_perform_incremental_network_scan(msg);
  } else {
    qcril_qmi_nas_start_advanced_scan(msg);
  }
}

void NasModule::handleStopNetworkScan(std::shared_ptr<RilRequestStopNetworkScanMessage> msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + msg->dump());

    qcril_qmi_nas_cancel_query_available_networks(msg);
}

void NasModule::handleQcrilInit(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  /* Init QMI services.*/
  QmiNasSetupRequestCallback qmiNasSetupCallback("NasModule-Token");
  ModemEndPointFactory<NasModemEndPoint>::getInstance().buildEndPoint()->requestSetup("NasModule-Token", &qmiNasSetupCallback);
}


void NasModule::handleNasEndpointStatusIndMessage(std::shared_ptr<Message> msg) {
  auto shared_indMsg(std::static_pointer_cast<EndpointStatusIndMessage>(msg));

  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());

  if (shared_indMsg->getState() == ModemEndPoint::State::OPERATIONAL) {
     if (!mReady) {
       mReady = true;
       qcril_qmi_nas_init();
     }
  } else {
    // state is not operational
    mReady = false;
    qcril_qmi_nas_cleanup();

    // invalidate physical channel configuration cache
    chaninfo.bandwidth_info_list_valid = false;
    chaninfo.pcell_info_valid = false;
    chaninfo.scell_info_list_valid = false;
  }
}

// IMS handling
void NasModule::handleImsServiceStatusInd(std::shared_ptr<ImsServiceStatusInd> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  qcril_qmi_nas_handle_ims_service_status_change(msg->getServiceStatusInfo());
}

void NasModule::handleImsWfcStatusInd(std::shared_ptr<ImsWfcSettingsStatusInd> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  qcril_qmi_nas_handle_ims_wfc_status_change(msg->getStatusInfo());
}

void NasModule::handleImsPdpStatusInd(std::shared_ptr<ImsPdpStatusInd> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  qcril_qmi_nas_handle_ims_pdp_status_change(msg->isPdpConnected());
}

void NasModule::handleSetVoiceDomainPreferenceRequest(std::shared_ptr<NasSetVoiceDomainPreferenceRequest> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  qcril_qmi_nas_set_voice_domain_preference(msg);
}

/* Handlers for Messages from Data */
void NasModule::handleEmbmsEnableDataConMessage(std::shared_ptr<NasEmbmsEnableDataConMessage> msg)
{
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  qcril_qmi_nas_embms_enable_data_con(&(msg->get_payload()));
  msg->sendResponse(msg, Message::Callback::Status::SUCCESS, nullptr);
}
void NasModule::handleEmbmsDisableDataMessage(std::shared_ptr<NasEmbmsDisableDataMessage> msg)
{
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  qcril_qmi_nas_embms_disable_data_ind(&(msg->get_payload()));
  msg->sendResponse(msg, Message::Callback::Status::SUCCESS, nullptr);
}

void NasModule::handleGetPhyChanConfigMessage(std::shared_ptr<NasGetPhyChanConfigMessage> msg)
{
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  msg->sendResponse(msg, Message::Callback::Status::SUCCESS, getCurrentPhysChanConfig());
}

#ifndef QMI_RIL_UTF
/* Data Indication Handlers */
void NasModule::handleDataCallStatusMessage(std::shared_ptr<CallStatusMessage> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  const rildata::CallStatusParams data = msg->getCallParams();
  NasDataCache::getInstance().setNumDataCalls(data.num_calls);

  //CALL FUNCTION IN QCRIL to notify this indication.
  qcril_qmi_nas_handle_data_call_status_notification(map_data_call_event_to_internal(data.evt));
}
void NasModule::handleDataSetPrefDataTechMessage(std::shared_ptr<SetPrefDataTechMessage> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  //NasDataCache::getInstance().setsetPreferredDataTech(indMsg->get());

  //Call Function in QCRIL to notify this indication
  qcril_arb_set_pref_data_tech(map_data_pref_data_tech_to_arb_pref_data_tech(msg->get()));
}
void NasModule::handleDataSysStatusMessage(std::shared_ptr<DataSysStatusMessage> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  //TODO: Revisit this
  qcril_arb_set_data_sys_status(&(msg->getParams()));
}
void NasModule::handleDataDsdSysStatusMessage(std::shared_ptr<DsdSysStatusMessage> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  //TODO: Revisit this
  qcril_arb_set_dsd_sys_status(&(msg->getParams()));
}

void NasModule::handleDataRegistrationStateMessage(std::shared_ptr<DataRegistrationStateMessage> msg) {
  // qcril-data will not be sending DSD indication if 5G rat available and when 5G is OOS.
  // This logic will make sure that in same file, we process 5G and legacy rats.
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  if (msg->getRatValue() != rildata::Rat::UNKNOWN) {
    dsd_system_status_ind_msg_v01 dsd_ind_msg;
    memset(&dsd_ind_msg, 0, sizeof(dsd_ind_msg));
    dsd_ind_msg.avail_sys_valid = 1;
    dsd_ind_msg.avail_sys_len = 2;
    dsd_ind_msg.avail_sys[0].technology = DSD_SYS_NETWORK_3GPP_V01;
    dsd_ind_msg.avail_sys[0].rat_value = DSD_SYS_RAT_EX_3GPP_5G_V01;
    if (msg->getRatValue() == rildata::Rat::NR_NSA) {
       dsd_ind_msg.avail_sys[0].so_mask = QMI_DSD_3GPP_SO_MASK_5G_NSA_V01;
    } else if (msg->getRatValue() == rildata::Rat::NR_SA) {
       dsd_ind_msg.avail_sys[0].so_mask = QMI_DSD_3GPP_SO_MASK_5G_SA_V01;
    } else if (msg->getRatValue() == rildata::Rat::NR_NSA_MMWV) {
       dsd_ind_msg.avail_sys[0].so_mask = QMI_DSD_3GPP_SO_MASK_5G_NSA_V01 |
                                          QMI_DSD_3GPP_SO_MASK_5G_MMWAVE_V01;
    } else if (msg->getRatValue() == rildata::Rat::NR_SA_MMWV) {
       dsd_ind_msg.avail_sys[0].so_mask = QMI_DSD_3GPP_SO_MASK_5G_SA_V01 |
                                          QMI_DSD_3GPP_SO_MASK_5G_MMWAVE_V01;
    }

    dsd_ind_msg.avail_sys[1].technology = DSD_SYS_NETWORK_3GPP_V01;
    dsd_ind_msg.avail_sys[1].rat_value = DSD_SYS_RAT_EX_3GPP_LTE_V01;
    qcril_arb_set_dsd_sys_status(&dsd_ind_msg);
  }
}

void NasModule::handleDataNewDDSInfoMessage(std::shared_ptr<rildata::NewDDSInfoMessage> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  qcril_qmi_nas_data_event_new_dds_info(msg->getDDS());
}

void NasModule::handleDataDDSStatusFollowupMessage(std::shared_ptr<rildata::DDSStatusFollowupMessage> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  DDSStatusInfo status_info = msg->getDDSStatusInfo();
  qcril_qmi_nas_data_event_dds_status_follow_up(map_dds_status_info_from_data(status_info));
}

dds_status_info map_dds_status_info_from_data(DDSStatusInfo status_info)
{
  dds_status_info ret;
  memset(&ret, 0, sizeof(ret));

  ret.result = status_info.result;
  ret.error_cause = status_info.error_cause;

  return ret;
}

qcril_arb_pref_data_tech_e_type map_data_pref_data_tech_to_arb_pref_data_tech(qcril_data_pref_data_tech_e_type pref_data_tech) {
  qcril_arb_pref_data_tech_e_type ret = QCRIL_ARB_PREF_DATA_TECH_INVALID;
  switch (pref_data_tech) {
  case QCRIL_DATA_PREF_DATA_TECH_UNKNOWN:
      ret = QCRIL_ARB_PREF_DATA_TECH_UNKNOWN;
      break;
  case QCRIL_DATA_PREF_DATA_TECH_CDMA:
      ret = QCRIL_ARB_PREF_DATA_TECH_CDMA;
      break;
  case QCRIL_DATA_PREF_DATA_TECH_EVDO:
      ret = QCRIL_ARB_PREF_DATA_TECH_EVDO;
      break;
  case QCRIL_DATA_PREF_DATA_TECH_GSM:
      ret = QCRIL_ARB_PREF_DATA_TECH_GSM;
      break;
  case QCRIL_DATA_PREF_DATA_TECH_UMTS:
      ret = QCRIL_ARB_PREF_DATA_TECH_UMTS;
      break;
  case QCRIL_DATA_PREF_DATA_TECH_EHRPD:
      ret = QCRIL_ARB_PREF_DATA_TECH_EHRPD;
      break;
  case QCRIL_DATA_PREF_DATA_TECH_LTE:
      ret = QCRIL_ARB_PREF_DATA_TECH_LTE;
      break;
  case QCRIL_DATA_PREF_DATA_TECH_TDSCDMA:
      ret = QCRIL_ARB_PREF_DATA_TECH_TDSCDMA;
      break;
  default:
      ret = QCRIL_ARB_PREF_DATA_TECH_INVALID;
      break;
  }
  return ret;
}
static qcril_qmi_nas_data_call_event_type map_data_call_event_to_internal(qcril_data_net_evt_t evt) {
  qcril_qmi_nas_data_call_event_type ret = QCRIL_QMI_NAS_DATA_EVT_UNKNOWN;
  switch(evt) {
  case QCRIL_DATA_EVT_CALL_CONNECTED:
     ret = QCRIL_QMI_NAS_DATA_EVT_CALL_CONNECTED;
     break;
  case QCRIL_DATA_EVT_CALL_PHYSLINK_UP:
     ret = QCRIL_QMI_NAS_DATA_EVT_CALL_PHYSLINK_UP;
     break;
  case QCRIL_DATA_EVT_CALL_PHYSLINK_DOWN:
     ret = QCRIL_QMI_NAS_DATA_EVT_CALL_PHYSLINK_DOWN;
     break;
  case QCRIL_DATA_EVT_CALL_RELEASED:
     ret = QCRIL_QMI_NAS_DATA_EVT_CALL_RELEASED;
     break;
  default:
     ret = QCRIL_QMI_NAS_DATA_EVT_UNKNOWN;
     break;
  }
  return ret;
}
#endif

void NasModule::handlePsAttachDetachMessage(std::shared_ptr<NasSetPsAttachDetachMessage> msg) {
  nas_srv_domain_pref_enum_type_v01 pref = NAS_SRV_DOMAIN_PREF_ENUM_TYPE_MIN_ENUM_VAL_V01;
  RIL_Errno res = RIL_E_INVALID_ARGUMENTS;

  switch (msg->getAction()) {
  case NasSetPsAttachDetachMessage::PsAttachAction::PS_ATTACH:
    pref = QMI_SRV_DOMAIN_PREF_PS_ATTACH_V01;
    break;
  case NasSetPsAttachDetachMessage::PsAttachAction::PS_DETACH:
    pref = QMI_SRV_DOMAIN_PREF_PS_DETACH_V01;
    break;
  case NasSetPsAttachDetachMessage::PsAttachAction::PS_ATTACH_ON_DEMAND:
    pref = QMI_SRV_DOMAIN_PREF_ON_DEMAND_PS_ATTACH_V01;
    break;
  case NasSetPsAttachDetachMessage::PsAttachAction::PS_ATTACH_FORCED:
    pref = QMI_SRV_DOMAIN_PREF_FORCE_PS_DETACH_V01;
    break;
  default:
    break;
  }
  if (pref != NAS_SRV_DOMAIN_PREF_ENUM_TYPE_MIN_ENUM_VAL_V01) {
    res = qcril_qmi_nas_send_attach_detach_request(pref);
  }

  msg->sendResponse(msg, Message::Callback::Status::SUCCESS,
        std::make_shared<RIL_Errno>(res));
}

void NasModule::handleNasGetSysInfoMessage(std::shared_ptr<NasGetSysInfoMessage> msg) {

  QCRIL_LOG_FUNC_ENTRY();

  nas_get_sys_info_resp_msg_v01     sys_info_list;
  memset(&sys_info_list, 0, sizeof(nas_get_sys_info_resp_msg_v01));
  qcril_qmi_nas_get_sys_info(&sys_info_list);

  QCRIL_LOG_DEBUG("Received NasGetSysInfoMessage Msg. Executing handleNasGetSysInfoMessage  ");

  QCRIL_LOG_DEBUG("sys info list - service valid=%d & service domain=%d", sys_info_list.lte_sys_info.common_sys_info.srv_domain_valid,sys_info_list.lte_sys_info.common_sys_info.srv_domain);
  msg->sendResponse(msg, Message::Callback::Status::SUCCESS,
                               std::make_shared<nas_get_sys_info_resp_msg_v01>(sys_info_list));
  QCRIL_LOG_FUNC_RETURN();
}

#ifdef QMI_RIL_UTF
void NasModule::qcrilHalNasModuleCleanup()
{
    std::shared_ptr<NasModemEndPoint> mNasModemEndPoint =
                ModemEndPointFactory<NasModemEndPoint>::getInstance().buildEndPoint();
    NasModemEndPointModule* mNasModemEndPointModule =
               (NasModemEndPointModule*)mNasModemEndPoint->mModule;
    mNasModemEndPointModule->cleanUpQmiSvcClient();
    mReady = false;
}
#endif

int32_t toKhz(nas_bandwidth_enum_type_v01 bw) {
    int32_t ret = INT_MAX;
    switch(bw) {
        case NAS_LTE_BW_NRB_6_V01:
            ret = 1400;
            break;
        case NAS_LTE_BW_NRB_15_V01:
            ret = 3000;
            break;
        case NAS_LTE_BW_NRB_25_V01:
            ret = 5000;
            break;
        case NAS_LTE_BW_NRB_50_V01:
            ret = 10000;
            break;
        case NAS_LTE_BW_NRB_75_V01:
            ret = 15000;
            break;
        case NAS_LTE_BW_NRB_100_V01:
            ret = 20000;
            break;
        case NAS_GSM_BW_NRB_2_V01:
            ret = 200;
            break;
        case NAS_WCDMA_BW_NRB_5_V01:
            ret = 5000;
            break;
        case NAS_WCDMA_BW_NRB_10_V01:
            ret = 10000;
            break;
        default:
            QCRIL_LOG_ERROR("Unknown bw value");
    }
    return ret;
}

int32_t toKhz(nas_lte_cphy_ca_bandwidth_enum_v01 bw) {

    switch(bw) {
        case NAS_LTE_CPHY_CA_BW_NRB_6_V01  : return 1400;  // 1.4 MHz bandwidth
        case NAS_LTE_CPHY_CA_BW_NRB_15_V01 : return 3000;  // 3 MHz bandwidth
        case NAS_LTE_CPHY_CA_BW_NRB_25_V01 : return 5000;  // 5 MHz bandwidth
        case NAS_LTE_CPHY_CA_BW_NRB_50_V01 : return 10000; // 10 MHz bandwidth
        case NAS_LTE_CPHY_CA_BW_NRB_75_V01 : return 15000; // 15 MHz bandwidth
        case NAS_LTE_CPHY_CA_BW_NRB_100_V01: return 20000; // 20 MHz bandwidth
        default:
            QCRIL_LOG_ERROR("Unknown bw value");
            return INT_MAX;
    }
}

void NasModule::handleSetSignalStrengthCriteriaSupported(
        std::shared_ptr<NasSetSignalStrengthCriteriaSupported> msg)
{
    mSetSignalStrengthCriteriaSupported = msg->isSupported();
}

inline bool NasModule::shouldReportScell(const nas_lte_cphy_scell_info_type_v01 &scell_info) {
    return (scell_info.scell_state == NAS_LTE_CPHY_SCELL_STATE_CONFIGURED_ACTIVATED_V01 ||
            scell_info.scell_state == NAS_LTE_CPHY_SCELL_STATE_CONFIGURED_DEACTIVATED_V01);
}

inline int32_t NasModule::getBandwidth(nas_radio_if_enum_v01 radio_if) {
    if (radio_if == NAS_RADIO_IF_LTE_V01 && chaninfo.pcell_info_valid) { // LTE CA
        return toKhz(chaninfo.pcell_info.cphy_ca_dl_bandwidth);
    } else if (chaninfo.bandwidth_info_list_valid) { // LTE non-CA and all other RATs
        unsigned int i = 0;
        for (; i < chaninfo.bandwidth_info_list_len; i++) {
            if (chaninfo.bandwidth_info_list[i].radio_if == radio_if) {
                return toKhz(chaninfo.bandwidth_info_list[i].bandwidth);
            }
        }

        if (i == chaninfo.bandwidth_info_list_len) {
            QCRIL_LOG_DEBUG("Couldn't find LTE bandwidth in bandwidth info list.");
        }
    }

    return std::numeric_limits<int32_t>::max();
}

inline int32_t NasModule::getBandwidth() {
    if (chaninfo.radio_if_valid) {
        return getBandwidth(chaninfo.radio_if);
    }
    return std::numeric_limits<int32_t>::max();
}

std::shared_ptr<std::vector<NasPhysChanInfo>> NasModule::getCurrentPhysChanConfig() {
    std::shared_ptr<std::vector<NasPhysChanInfo>> physChanInfo = std::make_shared<std::vector<NasPhysChanInfo>>();

    if(physChanInfo == nullptr) {
        Log::getInstance().d("[NasModule]: getCurrentPhysChanConfig: Failed to create vector.");
        return physChanInfo;
    }

    if (chaninfo.physical_cell_id_valid) {
        physChanInfo->push_back(
            NasPhysChanInfo {
                NasPhysChanInfo::Status::PRIMARY,
                getBandwidth(),
                chaninfo.physical_cell_id
            }
        );
    }

    if (chaninfo.scell_info_list_valid) { // Only applicable to LTE CA
        for (size_t i = 0; i < chaninfo.scell_info_list_len; i++) {
            if (shouldReportScell(chaninfo.scell_info_list[i].scell_info)) {
                physChanInfo->push_back(
                    NasPhysChanInfo {
                        NasPhysChanInfo::Status::SECONDARY,
                        toKhz(chaninfo.scell_info_list[i].scell_info.cphy_ca_dl_bandwidth),
                        chaninfo.scell_info_list[i].scell_info.pci
                    }
                );
            }
        }
    }

    return physChanInfo;
}

void NasModule::reportCurrentPhysChanConfig() {
    QCRIL_LOG_FUNC_ENTRY();

    /* If the physical channel configuration obtained from the modem
       is the same as the physical channel configuration in the cache,
       do not send an update. */
    if (!getPhysChanConfigFromModem()) {
        return;
    }

    std::shared_ptr<std::vector<NasPhysChanInfo>> physChanInfo = getCurrentPhysChanConfig();

    if(physChanInfo == nullptr) {
        Log::getInstance().d("[NasModule]: reportCurrentPhysChanConfig: physChanInfo is null");
    }

    if (physChanInfo != nullptr && !physChanInfo->empty()) {
        auto msg = std::make_shared<NasPhysChanConfigMessage>(physChanInfo);
        if (msg) {
            msg->broadcast();
        } else {
            QCRIL_LOG_DEBUG("Unable to allocate msg");
        }
    }
    QCRIL_LOG_FUNC_RETURN();
}

int32_t NasModule::getLteBw() {
    int32_t bandwidth = INT_MAX;

    // if the bandwidth information in the cache isn't valid query the modem
    // for the bandwidth information
    if (!chaninfo.pcell_info_valid && !chaninfo.bandwidth_info_list_valid) {
        getPhysChanConfigFromModem();
    }

    bandwidth = getBandwidth(NAS_RADIO_IF_LTE_V01);
    QCRIL_LOG_ESSENTIAL("returning bandwidth: %d", bandwidth);
    return bandwidth;
}

void NasModule::resetPhysChanConfig()
{
    QCRIL_LOG_FUNC_ENTRY();
    if (chaninfo.compareAndAssign({})) {
        reportCurrentPhysChanConfig();
    }
    QCRIL_LOG_FUNC_RETURN();
}

void NasModule::handleQmiNasRfBandInfoMessage(
        QCRIL_UNUSED(std::shared_ptr<QmiNasRfBandInfoMessage> bandInfoMsg))
{
    QCRIL_LOG_FUNC_ENTRY();
    reportCurrentPhysChanConfig();
    QCRIL_LOG_FUNC_RETURN();
}

void NasModule::handleQmiNasLteCphyCaIndMessage(
        QCRIL_UNUSED(std::shared_ptr<QmiNasLteCphyCaIndMessage> lteCphyMsg))
{
    QCRIL_LOG_FUNC_ENTRY();
    reportCurrentPhysChanConfig();
    QCRIL_LOG_FUNC_RETURN();
}

bool NasModule::getPhysChanConfigFromModem() {
    std::shared_ptr<NasModemEndPoint> nasModemEndpoint = ModemEndPointFactory<NasModemEndPoint>::getInstance().buildEndPoint();
    if (nasModemEndpoint) {
        PhysChanInfo phyChanInfo = {};
        nasModemEndpoint->getPhysChanConfig(phyChanInfo);
        return chaninfo.compareAndAssign(phyChanInfo);
    }
    return false;
}

void NasModule::handleEnablePhysChanConfigReporting(
        std::shared_ptr<NasEnablePhysChanConfigReporting> enableCfgMsg)
{
    if (!enableCfgMsg) {
        QCRIL_LOG_ERROR("No message");
        return;
    }
    auto ctx = enableCfgMsg->getContext();
    bool enable = enableCfgMsg->isEnable();
    auto nasModemEndPoint = ModemEndPointFactory<NasModemEndPoint>::getInstance().buildEndPoint();

    if (nasModemEndPoint) {
        nasModemEndPoint->registerForIndications(
                { { QMI_NAS_RF_BAND_INFO_IND_V01, enable },
                  { QMI_NAS_LTE_CPHY_CA_IND_V01,  enable },
                },
                ctx,
                [enableCfgMsg, ctx]
                (std::shared_ptr<Message> msg,
                    Message::Callback::Status status,
                    std::shared_ptr<QmiNasRegisterForIndResponse> rsp)
                {
                        QCRIL_NOTUSED(rsp);
                        auto lmsg = std::static_pointer_cast<QmiNasRegisterForIndRequest>(msg);
                        if (lmsg) {
                            QCRIL_LOG_INFO("Status: %d", static_cast<int>(status));
                                        NasSettingResultCode rc = NasSettingResultCode::FAILURE;
                                        RIL_Errno e = RIL_E_INTERNAL_ERR;
                                        if (status == Message::Callback::Status::SUCCESS &&
                                                rsp) {
                                            e =
                                                qcril_qmi_util_convert_qmi_response_codes_to_ril_result(
                                                        rsp->err,
                                                        &rsp->payload.resp
                                                        );
                                            if (e == RIL_E_SUCCESS) {
                                                rc = NasSettingResultCode::SUCCESS;
                                            }
                                        }
                                        QCRIL_LOG_INFO("Status: %d RIL_Errno: %d",
                                                static_cast<int>(status),
                                                e);
                                        enableCfgMsg->sendResponse(enableCfgMsg,
                                                status,
                                                std::make_shared<NasSettingResultCode>(rc));
                        }
                });
    }
}


void NasModule::handleGetModemStackStatus(
    std::shared_ptr<RilRequestGetModemStackStatusMessage> msg)
{
    QCRIL_LOG_FUNC_ENTRY();
    qcril_qmi_nas_get_modem_stack_status(msg);
    QCRIL_LOG_FUNC_RETURN();
}

void NasModule::handleEnableModem(
    std::shared_ptr<RilRequestEnableModemMessage> msg)
{
    QCRIL_LOG_FUNC_ENTRY();
    qcril_qmi_nas_request_enable_modem_stack(msg);
    QCRIL_LOG_FUNC_RETURN();
}

void NasModule::handleSetSystemSelectionChannels(
    std::shared_ptr<RilRequestSetSysSelChannelsMessage> msg)
{
    QCRIL_LOG_FUNC_ENTRY();
    qcril_qmi_nas_request_set_system_selection_channels(msg);
    QCRIL_LOG_FUNC_RETURN();
}

void NasModule::handleGetPhoneCapability(
    std::shared_ptr<RilRequestGetPhoneCapabilityMessage> msg)
{
    QCRIL_LOG_FUNC_ENTRY();
    qcril_qmi_nas_request_get_phone_capability(msg);
    QCRIL_LOG_FUNC_RETURN();
}
