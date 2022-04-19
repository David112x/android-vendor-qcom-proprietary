/******************************************************************************
#  Copyright (c) 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#define TAG "IMSA_Module"

#include <framework/Log.h>
#include <framework/Module.h>
#include <framework/ModuleLooper.h>
#include "modules/ims/ImsModule.h"

#include <framework/QcrilInitMessage.h>

#include "modules/ims/ImsInitMessage.h"
#include "modules/qmi/ImsaModemEndPoint.h"
#include "modules/qmi/ImssModemEndPoint.h"
#include "modules/qmi/ModemEndPointFactory.h"
#include "modules/qmi/QmiSetupRequestCallback.h"
#include "modules/qmi/QmiIndMessage.h"
#include "modules/qmi/EndpointStatusIndMessage.h"

#include "qcril_qmi_imss.h"
#include "qcril_qmi_imss_v02.h"
#include "qcril_qmi_imsa.h"

// Used for mModemEndPointReadyMask
#define IMSA_MODEM_END_POINT 0x01
#define IMSS_MODEM_END_POINT 0x02

ImsModule::ImsModule() : AddPendingMessageList("ImsModule") {
  mName = "ImsModule";
  mLooper = nullptr;

  using std::placeholders::_1;
  mMessageHandler = {
      HANDLER(QcrilInitMessage, ImsModule::handleQcrilInitMessage),
      HANDLER(QcrilImsClientConnected, ImsModule::handleQcrilImsClientConnected),

      {REG_MSG("IMSA_QMI_IND"), std::bind(&ImsModule::handleImsaQmiIndMessage, this, _1)},
      {REG_MSG("IMSS_QMI_IND"), std::bind(&ImsModule::handleImssQmiIndMessage, this, _1)},
      // QMI Async response
      HANDLER(QmiAsyncResponseMessage, ImsModule::handleQmiAsyncRespMessage),
      // End Point Status Indication
      {REG_MSG("IMSA_ENDPOINT_STATUS_IND"),
       std::bind(&ImsModule::handleImsaEndpointStatusIndMessage, this, _1)},
      {REG_MSG("IMSS_ENDPOINT_STATUS_IND"),
       std::bind(&ImsModule::handleImssEndpointStatusIndMessage, this, _1)},

      // SMS handling
      HANDLER(SmsImsServiceStatusInd, ImsModule::handleSmsImsServiceStatusInd),

      // Qcril Interface Requests
      HANDLER(QcRilRequestImsGetRegStateMessage,
              ImsModule::handleQcRilRequestImsGetRegStateMessage),
      HANDLER(QcRilRequestImsRegistrationChangeMessage,
              ImsModule::handleQcRilRequestImsRegistrationChangeMessage),
      HANDLER(QcRilRequestImsQueryServiceStatusMessage,
              ImsModule::handleQcRilRequestImsQueryServiceStatusMessage),

      HANDLER(QcRilRequestImsGetRtpStatsMessage, ImsModule::handleQcRilRequestImsGetRtpStatsMessage),
      HANDLER(QcRilRequestImsGetRtpErrorStatsMessage, ImsModule::handleQcRilRequestImsGetRtpErrorStatsMessage),
      HANDLER(QcRilRequestImsGetSubConfigMessage,
          ImsModule::handleQcRilRequestImsGetSubConfigMessage),
      HANDLER(QcRilRequestImsSetServiceStatusMessage,
          ImsModule::handleQcRilRequestImsSetServiceStatusMessage),
      HANDLER(QcRilRequestImsSendRttMessage,
          ImsModule::handleQcRilRequestImsSendRttMessage),
      HANDLER(QcRilRequestImsGeoLocationInfoMessage,
          ImsModule::handleQcRilRequestImsGeoLocationInfoMessage),
      HANDLER(QcRilRequestImsSetConfigMessage,
          ImsModule::handleQcRilRequestImsSetConfigMessage),
      HANDLER(QcRilRequestImsGetConfigMessage,
          ImsModule::handleQcRilRequestImsGetConfigMessage),
      HANDLER(QcRilRequestImsRegisterMultiIdentityMessage,
          ImsModule::handleQcRilRequestImsRegisterMultiLineMessage),
      HANDLER(QcRilRequestImsQueryVirtualLineInfo,
          ImsModule::handleQcRilRequestImsQueryLineInfoMessage),
  };
  ModemEndPointFactory<ImsaModemEndPoint>::getInstance().buildEndPoint();
  ModemEndPointFactory<ImssModemEndPoint>::getInstance().buildEndPoint();
}

ImsModule::~ImsModule() {
  mReady = false;
}

void ImsModule::init() {
  Module::init();
  qcril_qmi_imss_pre_init();
  qcril_qmi_imsa_pre_init();

  // Initializations complete.
  mReady = false;
}


void ImsModule::initImsa() {
  Log::getInstance().d("[" + mName + "]: initialize IMSA Modem Endpoint module");
  /* Init QMI IMSA services.*/
  QmiSetupRequestCallback qmiImsaSetupCallback("ImsModule-Token");
  ModemEndPointFactory<ImsaModemEndPoint>::getInstance().buildEndPoint()->requestSetup(
      "ImsModule-Token", &qmiImsaSetupCallback);
}

void ImsModule::initImss() {
  Log::getInstance().d("[" + mName + "]: initialize IMSS Modem Endpoint module");
  QmiSetupRequestCallback qmiImssSetupCallback("ImssModule-Token");
  ModemEndPointFactory<ImssModemEndPoint>::getInstance().buildEndPoint()->requestSetup(
      "ImssModule-Token", &qmiImssSetupCallback);
}

bool ImsModule::isReady() {
  return mReady;
}

void ImsModule::broadcastReady() {
  Log::getInstance().d("[" + mName + "]: broadcastReady");
  std::shared_ptr<ImsInitMessage> imsInitMsg = std::make_shared<ImsInitMessage>();
  if(imsInitMsg) {
    imsInitMsg->broadcast();
  }
}

bool ImsModule::isImsaModemEndpointReady() {
  bool isReady = false;
  if (mModemEndPointReadyMask & IMSA_MODEM_END_POINT) {
    isReady = true;
  }
  return isReady;
}

void ImsModule::setImsaModemEndpointReady() {
  mModemEndPointReadyMask |= IMSA_MODEM_END_POINT;
}

void ImsModule::setImsaModemEndpointNotReady() {
  mModemEndPointReadyMask &= ~IMSA_MODEM_END_POINT;
}

bool ImsModule::isImssModemEndpointReady() {
  bool isReady = false;
  if (mModemEndPointReadyMask & IMSS_MODEM_END_POINT) {
    isReady = true;
  }
  return isReady;
}

void ImsModule::setImssModemEndpointReady() {
  mModemEndPointReadyMask |= IMSS_MODEM_END_POINT;
}

void ImsModule::setImssModemEndpointNotReady() {
  mModemEndPointReadyMask &= ~IMSS_MODEM_END_POINT;
}

bool ImsModule::isAllModemEndpointsReady() {
  bool isReady = false;
  uint8_t servicesNeeded = (IMSA_MODEM_END_POINT|IMSS_MODEM_END_POINT);

  Log::getInstance().d("[" + mName + "]: mModemEndPointReadyMask = " +
      std::to_string(mModemEndPointReadyMask));

  if ((mModemEndPointReadyMask & servicesNeeded) == servicesNeeded) {
    isReady = true;
  }

  return isReady;
}

void ImsModule::handleQcrilInitMessage(std::shared_ptr<QcrilInitMessage> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  initImsa();
  initImss();
}

void ImsModule::handleQcrilImsClientConnected(std::shared_ptr<QcrilImsClientConnected> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  if (isImssModemEndpointReady()) {
    qcril_qmi_ims_send_unsol_wfc_roaming_config();
  }
}

void ImsModule::handleImsaEndpointStatusIndMessage(std::shared_ptr<Message> msg) {
  auto shared_indMsg(std::static_pointer_cast<EndpointStatusIndMessage>(msg));

  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());

  if (shared_indMsg->getState() == ModemEndPoint::State::OPERATIONAL) {
    if (!isImsaModemEndpointReady()) {
      qcril_qmi_imsa_init();
      setImsaModemEndpointReady();
    }
    if (isAllModemEndpointsReady()) {
      mReady = true;
      broadcastReady();
    }
  } else {
    // state is not operational
    setImsaModemEndpointNotReady();
    mReady = false;
    if (!isAllModemEndpointsReady()) {
      clearPendingMessageList();
    }
    // clean up
    qcril_qmi_imsa_cleanup();
  }
}

void ImsModule::handleImssEndpointStatusIndMessage(std::shared_ptr<Message> msg) {
  auto shared_indMsg(std::static_pointer_cast<EndpointStatusIndMessage>(msg));

  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());

  if (shared_indMsg->getState() == ModemEndPoint::State::OPERATIONAL) {
    if (!isImssModemEndpointReady()) {
      qcril_qmi_imss_init();
      setImssModemEndpointReady();
    }
    if (isAllModemEndpointsReady()) {
      mReady = true;
      broadcastReady();
    }
  } else {
    // state is not operational
    setImssModemEndpointNotReady();
    mReady = false;
    if (!isAllModemEndpointsReady()) {
      clearPendingMessageList();
    }
    // clean up
    qcril_qmi_imss_cleanup();
  }
}

void ImsModule::handleImsaQmiIndMessage(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  auto shared_indMsg(std::static_pointer_cast<QmiIndMessage>(msg));
  QmiIndMsgDataStruct *indData = shared_indMsg->getData();
  if (indData != nullptr) {
    qcril_qmi_imsa_unsol_ind_cb_helper(indData->msgId, indData->indData, indData->indSize);
  } else {
    Log::getInstance().d("Unexpected, null data from message");
  }
}

void ImsModule::handleImssQmiIndMessage(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());

  auto shared_indMsg(std::static_pointer_cast<QmiIndMessage>(msg));
  QmiIndMsgDataStruct *indData = shared_indMsg->getData();
  if (indData != nullptr) {
    if(qcril_qmi_imss_get_modem_version() == 1)
    {
      qcril_qmi_imss_unsol_ind_cb_helper_v02(indData->msgId, indData->indData, indData->indSize);
    } else {
      qcril_qmi_imss_unsol_ind_cb_helper(indData->msgId, indData->indData, indData->indSize);
    }
  } else {
    Log::getInstance().d("Unexpected, null data from message");
  }
}

void ImsModule::handleQmiAsyncRespMessage(std::shared_ptr<QmiAsyncResponseMessage> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  QmiAsyncRespData *asyncResp = msg->getData();
  if (asyncResp != nullptr && asyncResp->cb != nullptr) {
    asyncResp->cb(asyncResp->msgId, asyncResp->respData, asyncResp->respDataSize,
        asyncResp->cbData, asyncResp->traspErr);
  } else {
    Log::getInstance().d("Unexpected, null data from message");
  }
}

void ImsModule::handleQcRilRequestImsGetRegStateMessage(
    std::shared_ptr<QcRilRequestImsGetRegStateMessage> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());

  qcril_qmi_imsa_request_ims_registration_state(msg);
}

void ImsModule::handleQcRilRequestImsQueryServiceStatusMessage(
    std::shared_ptr<QcRilRequestImsQueryServiceStatusMessage> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  qcril_qmi_imsa_request_query_ims_srv_status(msg);
}

void ImsModule::handleQcRilRequestImsRegistrationChangeMessage(
    std::shared_ptr<QcRilRequestImsRegistrationChangeMessage> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());

  if (qcril_qmi_imss_get_modem_version() == 1) {
    qcril_qmi_imss_request_set_ims_registration_v02(msg);
  } else {
    qcril_qmi_imss_request_set_ims_registration(msg);
  }
}

void ImsModule::handleQcRilRequestImsGetRtpStatsMessage(
    std::shared_ptr<QcRilRequestImsGetRtpStatsMessage> msg) {
    Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());

    qcril_qmi_imsa_request_get_rtp_statistics(msg);
}

void ImsModule::handleQcRilRequestImsGetRtpErrorStatsMessage (
    std::shared_ptr<QcRilRequestImsGetRtpErrorStatsMessage> msg) {
    Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());

    qcril_qmi_imsa_request_get_rtp_error_statistics(msg);
}

void ImsModule::handleQcRilRequestImsGetSubConfigMessage(
    std::shared_ptr<QcRilRequestImsGetSubConfigMessage> msg) {

    Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
    qcril_qmi_imsa_request_get_ims_sub_config(msg);
}

void ImsModule::handleQcRilRequestImsSendRttMessage(
    std::shared_ptr<QcRilRequestImsSendRttMessage> msg) {
    Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
    qcril_qmi_imss_request_send_rtt_msg(msg);
}

void ImsModule::handleQcRilRequestImsGeoLocationInfoMessage(
    std::shared_ptr<QcRilRequestImsGeoLocationInfoMessage> msg) {
    Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
    qcril_qmi_imsa_set_geo_loc(msg);
}

void ImsModule::handleQcRilRequestImsSetServiceStatusMessage(
    std::shared_ptr<QcRilRequestImsSetServiceStatusMessage> msg) {
    QCRIL_LOG_FUNC_ENTRY("msg = %s", (msg ? msg->dump().c_str() : "nullptr"));
    if (qcril_qmi_imss_get_modem_version() == 1) {
        //TODO v_02 message
        qcril_qmi_imss_request_set_ims_srv_status_v02(msg);
    } else {
        qcril_qmi_imss_request_set_ims_srv_status(msg);
    }

}

void ImsModule::handleQcRilRequestImsSetConfigMessage(
    std::shared_ptr<QcRilRequestImsSetConfigMessage> msg) {
    Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
    if (qcril_qmi_imss_get_modem_version() == 1) {
        //TODO v_02 message
        qcril_qmi_imss_request_set_ims_config_v02(msg);
    } else {
        qcril_qmi_imss_request_set_ims_config(msg);
    }
}

void ImsModule::handleQcRilRequestImsGetConfigMessage(
    std::shared_ptr<QcRilRequestImsGetConfigMessage> msg) {
    Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
    if (qcril_qmi_imss_get_modem_version() == 1) {
        //TODO v_02 message
        qcril_qmi_imss_request_get_ims_config_v02(msg);
    } else {
        qcril_qmi_imss_request_get_ims_config(msg);
    }
}

void ImsModule::handleQcRilRequestImsRegisterMultiLineMessage(
    std::shared_ptr<QcRilRequestImsRegisterMultiIdentityMessage> msg) {
    Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
    qcril_qmi_imss_request_register_multi_identity_lines(msg);
}

void ImsModule::handleQcRilRequestImsQueryLineInfoMessage(
    std::shared_ptr<QcRilRequestImsQueryVirtualLineInfo> msg) {
    Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
    qcril_qmi_imsa_request_query_virtual_line_info(msg);
}

void ImsModule::handleSmsImsServiceStatusInd(std::shared_ptr<SmsImsServiceStatusInd> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  sSmsImsServiceStatus = msg->getSmsImsServiceStatusInfo();
  qcril_qmi_imsa_sms_ims_service_status_hdlr(sSmsImsServiceStatus.sms_ims_service_status);
}

static load_module<ImsModule> sImsModule;

ImsModule *getImsModule() {
    return &(sImsModule.get_module());
}

#ifdef QMI_RIL_UTF
void ImsModule::qcrilHalImsModuleCleanup()
{
    Log::getInstance().d("[" + mName + "]: qcrilHalImsModuleCleanup");
    std::shared_ptr<ImsaModemEndPoint> mImsaModemEndPoint =
                        ModemEndPointFactory<ImsaModemEndPoint>::getInstance().buildEndPoint();
    ImsaModemEndPointModule* mImsaModemEndPointModule =
                        (ImsaModemEndPointModule*)mImsaModemEndPoint->mModule;
    std::shared_ptr<ImssModemEndPoint> mImssModemEndPoint =
                        ModemEndPointFactory<ImssModemEndPoint>::getInstance().buildEndPoint();
    ImssModemEndPointModule* mImssModemEndPointModule =
                        (ImssModemEndPointModule*)mImssModemEndPoint->mModule;

    mImsaModemEndPointModule->cleanUpQmiSvcClient();
    mImssModemEndPointModule->cleanUpQmiSvcClient();
    mReady = false;
    mModemEndPointReadyMask = 0x00;
    qcril_qmi_imsa_cleanup();
    qcril_qmi_imss_pre_init();
    qcril_qmi_imsa_pre_init();
}
#endif
