/******************************************************************************
#  Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "qmi_client.h"
#include "common_v01.h"
#include "wireless_data_service_v01.h"
#include "MessageCommon.h"

#include "modules/qmi/ModemEndPoint.h"
#include "modules/qmi/ModemEndPointModule.h"

class WDSCallModemEndPointModule : public ModemEndPointModule
{
private:
  int mCid;
  rildata::IpFamilyType_t mIpType;
  qmi_idl_service_object_type getServiceObject() override;
  bool handleQmiBinding(qcril_instance_id_e_type instanceId, int8_t stackId) override;
  void processExtendedIpConfigIndication(wds_req_settings_mask_v01 extMask);
  void handleWdsQmiIndMessage(std::shared_ptr<Message> msg);
  void handleEndPointStatusIndMessage(std::shared_ptr<Message> msg);
  void handleQmiServiceDown(std::shared_ptr<QmiServiceDownIndMessage> shared_srvDownMsg);
  void WdsUnsolicitedIndHdlr(unsigned int msg_id, unsigned char *decoded_payload, uint32_t decoded_payload_len);
  Message::Callback::Status queryAllDataBearerType(void);

public:
  WDSCallModemEndPointModule(string name, ModemEndPoint& owner, rildata::IpFamilyType_t iptype, int cid);
  virtual ~WDSCallModemEndPointModule();
  void init();
  void releaseWDSCallEPModuleQMIClient();
  int getCid() {return mCid;}
  void handleDeregisterWdsIndication(std::shared_ptr<Message> msg);
};
