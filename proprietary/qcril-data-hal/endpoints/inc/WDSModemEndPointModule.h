/******************************************************************************
#  Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "wireless_data_service_v01.h"
#include "qmi_client.h"
#include "common_v01.h"
#include <list>
#include "modules/qmi/ModemEndPoint.h"
#include "modules/qmi/ModemEndPointModule.h"
#include "modules/qmi/QmiServiceUpIndMessage.h"
#include "modules/qmi/QmiSetupRequest.h"
#include "sync/SetAttachListSyncMessage.h"
#include "sync/SetWDSProfileInfoSyncMessage.h"
#include "sync/GetAttachListCapabilitySyncMessage.h"
#include "sync/GetCallBringUpCapabilitySyncMessage.h"
#include "sync/GetApnTypesForName.h"
#include "MessageCommon.h"

#define QCRIL_DATA_QMI_TIMEOUT 10000

using namespace rildata;

struct profileSettings_t {
  wds_profile_type_enum_v01 profile_type;
  uint8_t profile_index;
  char apn_name_or_string[QMI_WDS_APN_NAME_MAX_V01 + 1];
};

class WDSModemEndPointModule : public ModemEndPointModule
{
private:
  enum AttachAction
  {
    ATTACH_PDN_ACTION_NOT_SUPPORTED =0,
    ATTACH_PDN_ACTION_SUPPORTED
  };
  qmi_idl_service_object_type getServiceObject() override;
  bool handleQmiBinding(qcril_instance_id_e_type instanceId,
                        int8_t stackId) override;
  void handleGetAttachList(std::shared_ptr<Message> msg);
  void handleSetAttachList(std::shared_ptr<Message> msg);
  void handleGetAttachListCap(std::shared_ptr<Message> msg);
  void handleGetApnTypesForName(std::shared_ptr<Message> msg);
  void handleSetProfileParamsSync(std::shared_ptr<Message> msg);
  void handleGetCallBringUpCapabilitySync(std::shared_ptr<Message> msg);
  void handleRegisterForKeepAliveMessageSync(std::shared_ptr<Message> msg);
  void handleWdsQmiIndMessage(std::shared_ptr<Message> msg);
  void WdsUnsolicitedIndHdlr(unsigned int   msg_id,
  unsigned char *decoded_payload,
  uint32_t       decoded_payload_len);
  std::list<int32_t> getApnTypesByTechType(wds_profile_type_enum_v01 techType,
                                           string apnName);
  bool isDefaultProfile
  (
    int32_t supportedApnTypesBitmap
  );
  int getProfilesByTechType
  (
    wds_profile_type_enum_v01 techType,
    vector<profileSettings_t>& queriedProfileList
  );
  bool findMatchApnName
  (
    string apnName,
    vector<profileSettings_t>& searchList,
    profileSettings_t& matchingProfile
  );

  void setProfileSupportedAPNTypes
  (
    wds_profile_type_enum_v01 profile_type,
    uint8_t profile_index,
    wds_apn_type_mask_v01 apnTypeMask
  );
  void setProfileDefault
  (
    wds_profile_type_enum_v01 profile_type,
    uint8_t profile_index
  );
public:
  WDSModemEndPointModule(string name, ModemEndPoint& owner);
  virtual ~WDSModemEndPointModule();
  void init();
  static wds_apn_type_mask_v01 convertToApnTypeMask
  (
    int32_t supportedApnTypesBitmap
  );
};
