/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "qmi_client.h"
#include "common_v01.h"
#include "authentication_service_v01.h"

#include "modules/qmi/ModemEndPoint.h"
#include "modules/qmi/ModemEndPointModule.h"

class AuthModemEndPointModule : public ModemEndPointModule {
 private:
  qmi_idl_service_object_type getServiceObject() override;
  bool handleQmiBinding(qcril_instance_id_e_type instanceId, int8_t stackId) override;

 public:
  AuthModemEndPointModule(string name, ModemEndPoint& owner);
  virtual ~AuthModemEndPointModule();

  void init();
};
