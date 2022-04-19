/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
//#include <cstring>
#include <string>
#include "AuthModemEndPointModule.h"

AuthModemEndPointModule::AuthModemEndPointModule(string name,
    ModemEndPoint &owner) : ModemEndPointModule(name, owner) {

  mServiceObject = nullptr;
  mLooper = std::unique_ptr<ModuleLooper>(new ModuleLooper);

  using std::placeholders::_1;
  mMessageHandler = {
  };
}

AuthModemEndPointModule::~AuthModemEndPointModule() { mLooper = nullptr; }

void AuthModemEndPointModule::init() {
  ModemEndPointModule::init();
}

qmi_idl_service_object_type AuthModemEndPointModule::getServiceObject() {
  return auth_get_service_object_v01();
}

bool AuthModemEndPointModule::handleQmiBinding(
    qcril_instance_id_e_type instanceId, int8_t stackId) {
  Log::getInstance().d(
        "[AuthModemEndPointModule] handleQmiBinding");

  (void)instanceId;
  if (stackId < 0)
    return false;

  auth_set_subscription_binding_req_msg_v01 bind_request;
  auth_set_subscription_binding_resp_msg_v01 bind_resp;
  memset(&bind_request, 0, sizeof(bind_request));
  memset(&bind_resp, 0, sizeof(bind_resp));

  if (stackId == 0) {
    bind_request.bind_subs = AUTH_PRIMARY_SUBS_V01;
  } else if (stackId == 1) {
    bind_request.bind_subs = AUTH_SECONDARY_SUBS_V01;
  } else if (stackId == 2) {
    bind_request.bind_subs = AUTH_TERTIARY_SUBS_V01;
  } else
    return false;

  int ntries = 0;
  do {
    qmi_client_error_type res = qmi_client_send_msg_sync(
         mQmiSvcClient, QMI_AUTH_SET_SUBSCRIPTION_BINDING_REQ_V01,
         (void*)&bind_request, sizeof(bind_request),
         (void*)&bind_resp, sizeof(bind_resp),
         ModemEndPoint::DEFAULT_SYNC_TIMEOUT);
    if (QMI_NO_ERR == res && bind_resp.resp.result ==
            QMI_RESULT_SUCCESS_V01) {
      Log::getInstance().d(
        "[AuthModemEndPointModule]: QMI binding succeeds. instanceId: "
        + std::to_string((int)instanceId) + " stackId: " + std::to_string(stackId));
      return true;
    }
    usleep(500*1000);// TODO check if this can be reduced/removed
  } while (++ntries < 10);

  return false;
}

