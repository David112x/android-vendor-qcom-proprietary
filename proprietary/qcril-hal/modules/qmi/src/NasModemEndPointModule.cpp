/******************************************************************************
#  Copyright (c) 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#define TAG "NasModemEndPointModule"
#include <framework/Log.h>
#include <cstring>
#include "modules/qmi/NasModemEndPointModule.h"
#include "modules/qmi/NasModemEndPoint.h"

#include "modules/qmi/QmiServiceUpIndMessage.h"
#include "modules/qmi/QmiSendRawSyncMessage.h"
#include "modules/qmi/QmiSendRawAsyncMessage.h"

#include "network_access_service_v01.h"

NasModemEndPointModule::NasModemEndPointModule(string name,
    ModemEndPoint &owner) : ModemEndPointModule(name, owner) {

  mServiceObject = nullptr;
  mLooper = std::unique_ptr<ModuleLooper>(new ModuleLooper);

  mMessageHandler = {
      HANDLER(QmiNasRegisterForIndRequest, NasModemEndPointModule::handleQmiNasRegisterForIndRequest)
  };

}

NasModemEndPointModule::~NasModemEndPointModule() {
  mLooper = nullptr;
}

bool NasModemEndPointModule::handleQmiBinding(
    qcril_instance_id_e_type instanceId, int8_t stackId) {

  (void)instanceId;
  if (stackId < 0)
    return false;

  nas_bind_subscription_req_msg_v01 nas_bind_request;
  nas_bind_subscription_resp_msg_v01 nas_bind_resp;
  memset(&nas_bind_request, 0, sizeof(nas_bind_request));
  memset(&nas_bind_resp, 0, sizeof(nas_bind_resp));

  if (stackId == 0) {
    nas_bind_request.subs_type = NAS_PRIMARY_SUBSCRIPTION_V01;
  } else if (stackId == 1) {
    nas_bind_request.subs_type = NAS_SECONDARY_SUBSCRIPTION_V01;
  } else if (stackId == 2) {
    nas_bind_request.subs_type = NAS_TERTIARY_SUBSCRIPTION_V01;
  } else
    return false;

  int ntries = 0;
  do {
    qmi_client_error_type res = qmi_client_send_msg_sync(
         mQmiSvcClient, QMI_NAS_BIND_SUBSCRIPTION_REQ_MSG_V01,
         (void*)&nas_bind_request, sizeof(nas_bind_request),
         (void*)&nas_bind_resp, sizeof(nas_bind_resp),
         ModemEndPoint::DEFAULT_SYNC_TIMEOUT);
    if (QMI_NO_ERR == res && nas_bind_resp.resp.result ==
            QMI_RESULT_SUCCESS) {
      Log::getInstance().d(
        "[NasModemEndPointModule]: QMI binding succeeds. instanceId: "
        + std::to_string((int)instanceId) + " stackId: " + std::to_string(stackId));
      return true;
    }
    else if(QMI_SERVICE_ERR == res)
    {
        return false;
    }

    usleep(500*1000);
  } while (++ntries < 10);

  return false;
}

qmi_idl_service_object_type NasModemEndPointModule::getServiceObject() {
    return nas_get_service_object_v01();
}

void NasModemEndPointModule::onQmiUnsolIndCb(qmi_client_type user_handle, unsigned int msg_id,
        void * ind_buf, unsigned int ind_buf_len)
{
    QCRIL_NOTUSED(user_handle);
    switch (msg_id) {
        case QMI_NAS_RF_BAND_INFO_IND_V01: {
                nas_rf_band_info_ind_msg_v01 rfBandInfo = {};
                qmi_client_error_type qmi_err = qmi_client_message_decode(mQmiSvcClient, QMI_IDL_INDICATION,
                    QMI_NAS_RF_BAND_INFO_IND_V01, ind_buf, ind_buf_len,
                    &rfBandInfo, sizeof(rfBandInfo));
                if (!qmi_err) {
                    auto msg = std::make_shared<QmiNasRfBandInfoMessage>(rfBandInfo);
                    QCRIL_LOG_DEBUG("broadcasting message for QMI_NAS_RF_BAND_INFO_IND_V01");
                    msg->broadcast();
                }
            }
            break;
        case QMI_NAS_LTE_CPHY_CA_IND_V01: {
                nas_lte_cphy_ca_ind_msg_v01 lteCaInfo = {};
                qmi_client_error_type qmi_err = qmi_client_message_decode(mQmiSvcClient, QMI_IDL_INDICATION,
                    QMI_NAS_LTE_CPHY_CA_IND_V01, ind_buf, ind_buf_len,
                    &lteCaInfo, sizeof(lteCaInfo));
                if (!qmi_err) {
                    auto msg = std::make_shared<QmiNasLteCphyCaIndMessage>(lteCaInfo);
                    QCRIL_LOG_DEBUG("broadcasting message for  QMI_NAS_LTE_CPHY_CA_IND_V01");
                    msg->broadcast();
                }
            }
            break;
        default:
            ModemEndPointModule::onQmiUnsolIndCb(user_handle,
                    msg_id,
                    ind_buf,
                    ind_buf_len);
    }
}

void NasModemEndPointModule::handleQmiNasRegisterForIndRequest(std::shared_ptr<QmiNasRegisterForIndRequest> msg)
{
    QCRIL_LOG_FUNC_ENTRY();
    if (!msg) {
        QCRIL_LOG_FUNC_RETURN("msg is null");
        return;
    }

    nas_indication_register_req_msg_v01 req = {};
    nas_indication_register_resp_msg_v01 rsp = {};

    for (auto pair: msg->getIndicationIds()) {
        unsigned id = pair.first;
        bool enable = pair.second;

        switch(id) {
            case QMI_NAS_RF_BAND_INFO_IND_V01:
                {
                    QCRIL_LOG_ERROR("%segistering for QMI_NAS_RF_BAND_INFO_IND_V01(0x%04x)", enable ? "R":"De-", id);
                    req.reg_rf_band_info_valid = TRUE;
                    req.reg_rf_band_info = enable;
                }
                break;
            case QMI_NAS_LTE_CPHY_CA_IND_V01:
                {
                    QCRIL_LOG_INFO("%segistering for QMI_NAS_LTE_CPHY_CA_IND_V01(0x%04x)", enable ? "R":"De-", id);
                    req.reg_lte_cphy_ca_valid = TRUE;
                    req.reg_lte_cphy_ca = enable;
                }
                break;
            default:
                {
                    QCRIL_LOG_ERROR("Ignoring unknown or unhandled QMI_NAS indication 0x%04x", id);
                }
                break;
        }
    }
    qmi_client_error_type rc = qmi_client_send_msg_sync(
                                             mQmiSvcClient,
                                             QMI_NAS_INDICATION_REGISTER_REQ_MSG_V01,
                                             &req,
                                             sizeof(req),
                                             &rsp,
                                             sizeof(rsp),
                                             ModemEndPointModule::DEFAULT_SYNC_TIMEOUT);
    if(rc) {
        QCRIL_LOG_ERROR("Error registering for indications: %d", rc);
    }

    auto indrsp = std::make_shared<QmiNasRegisterForIndResponse>();
    if (indrsp) {
        indrsp->payload = rsp;
        indrsp->err = rc;
        msg->sendResponse(msg,
                Message::Callback::Status::SUCCESS,
                indrsp);
    }

    QCRIL_LOG_FUNC_RETURN();
}
