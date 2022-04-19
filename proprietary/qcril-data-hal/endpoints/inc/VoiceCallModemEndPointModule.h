/******************************************************************************
#  Copyright (c) 2019-2020 Qualcomm Technologies, Inc.
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
#include "voice_service_v02.h"
#include "UnSolMessages/VoiceIndMessage.h"


class VoiceCallModemEndPointModule : public ModemEndPointModule
{
private:
  rildata::SubId mSub;
  qmi_idl_service_object_type getServiceObject() override;
  bool handleQmiBinding(qcril_instance_id_e_type instanceId, int8_t stackId) override;
  void handleVoiceIndMessage(std::shared_ptr<Message> msg);
  void VoiceIndHdlr(unsigned int msg_id, unsigned char *decoded_payload, uint32_t decoded_payload_len);

public:
  VoiceCallModemEndPointModule(string name, ModemEndPoint& owner, rildata::SubId sub);
  virtual ~VoiceCallModemEndPointModule();
  void init();
};

