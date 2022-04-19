/******************************************************************************
#  Copyright (c) 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include "framework/Module.h"
#include "framework/ModuleLooper.h"
#include "modules/qmi/ModemEndPoint.h"
#include "modules/qmi/ModemEndPointModule.h"

class QmiNasRegisterForIndRequest;

class NasModemEndPointModule : public ModemEndPointModule {
  public:
    NasModemEndPointModule(string name, ModemEndPoint& owner);
    ~NasModemEndPointModule();

  private:
    qmi_idl_service_object_type getServiceObject() override;
    bool handleQmiBinding(qcril_instance_id_e_type instanceId, int8_t stackId) override;
    void onQmiUnsolIndCb(qmi_client_type user_handle, unsigned int msg_id,
            void * ind_buf, unsigned int ind_buf_len);
    void handleQmiNasRegisterForIndRequest(std::shared_ptr<QmiNasRegisterForIndRequest> msg);
};
