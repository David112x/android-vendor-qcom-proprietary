/******************************************************************************
#  Copyright (c) 2018-2020 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "data_system_determination_v01.h"
#include "qmi_client.h"
#include "common_v01.h"
#include "modules/qmi/ModemEndPoint.h"
#include "modules/qmi/ModemEndPointModule.h"
#include "modules/qmi/QmiServiceUpIndMessage.h"
#include "modules/qmi/QmiSetupRequest.h"
#include "SegmentTracker.h"

#define QCRIL_DATA_QMI_TIMEOUT 10000

class DSDModemEndPointModule : public ModemEndPointModule
{
private:
  static constexpr TimeKeeper::millisec DDS_SWITCH_REQUEST_TIMEOUT = 17000;
  qmi_idl_service_object_type getServiceObject() override;
  bool handleQmiBinding(qcril_instance_id_e_type instanceId, int8_t stackId) override;
  void handleQmiDsdIndMessage(std::shared_ptr<Message> msg);
  void handleEndPointStatusIndMessage(std::shared_ptr<Message> msg);
  void processDsdSystemStatusIndComplete(rildata::SegmentationStatus_t, uint16_t, std::vector<std::shared_ptr<Message>>);
  void processDsdSystemStatusResultComplete(rildata::SegmentationStatus_t, uint16_t, std::vector<std::shared_ptr<Message>>);
  void processDsdUiInfoIndComplete(rildata::SegmentationStatus_t, uint16_t, std::vector<std::shared_ptr<Message>>);
  void processDsdUiInfoResultComplete(rildata::SegmentationStatus_t, uint16_t, std::vector<std::shared_ptr<Message>>);
  void handleSetCapabilities(std::shared_ptr<Message> msg);
  void handleGetDataNrIconType(std::shared_ptr<Message> msg);
  void registerForUiChangeInd();
  void getUiInfo(std::shared_ptr<Message> msg);
  void handleGetDsdSystemStatus(std::shared_ptr<Message> msg);
  void handleGetDsdSystemStatusV1(std::shared_ptr<Message> msg);
  void handleGetDsdSystemStatusV2(std::shared_ptr<Message> msg);
  void handleRegisterForSystemStatusSync(std::shared_ptr<Message> msg);
  void handleRegisterForAPAsstIWlanIndsSync(std::shared_ptr<Message> msg);
  void handleTurnOnAPAssistIWLANSync(std::shared_ptr<Message> msg);
  void handleSetRatPref(std::shared_ptr<Message> msg);
  void indicationHandler(unsigned int msg_id, unsigned char *decoded_payload, uint32_t decoded_payload_len);
  void processSystemStatusInd(dsd_system_status_ind_msg_v01 *ind_data);
  void processSystemStatusInd(dsd_system_status_v2_ind_msg_v01 *ind_data);
  void processSystemStatusInd(dsd_get_system_status_v2_result_ind_msg_v01 *ind_data);
  void processIntentToChangeApnPrefSysInd(dsd_intent_to_change_apn_pref_sys_ind_msg_v01 *ind_data);
  void processApAsstApnPrefSysResultInd(dsd_ap_asst_apn_pref_sys_result_ind_msg_v01 *ind_data);
  void processUiInfoInd(dsd_ui_info_ind_msg_v01 *ind_data);
  void processUiInfoInd(dsd_get_ui_info_v2_result_ind_msg_v01 *ind_data);
  void handleSetApnPreferredSystemChangeSync(std::shared_ptr<Message> msg);
  void processQmiDdsSwitchInd(dsd_switch_dds_ind_msg_v01 *ind);
  void processQmiCurrentDdsInd(dsd_current_dds_ind_msg_v01 *ind);
  void handleTriggerDDSSwitchSyncMessage(std::shared_ptr<Message> msg);
  dsd_switch_dds_resp_msg_v01 qmiDDSSwitchRequest(int subId,dsd_dds_switch_type_enum_v01 switch_type);
  void onDDSSwitchRequestExpired(void *);
  TimeKeeper::timer_id ddsSwitchRequestTimer;
  bool ddsSwitchRequestPending;
  bool mNrIconReportDisabled;
  rildata::SegmentTracker segmentTracker;
  bool usingSystemStatusV2;
  bool usingUiInfoV2;
  std::unordered_map<rildata::SegmentTracker::KeyType_t, std::shared_ptr<Message>> pendingSegmentRequests;
  void handleSegmentTimeout(std::shared_ptr<Message> m);

public:
  DSDModemEndPointModule(string name, ModemEndPoint& owner);
  virtual ~DSDModemEndPointModule();

  void init();
#ifdef QMI_RIL_UTF
  void cleanUpQmiSvcClient();
#endif
};
