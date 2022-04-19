/******************************************************************************
#  Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include <cstring>
#include "modules/qmi/EndpointStatusIndMessage.h"
#include "WDSCallModemEndPointModule.h"
#include "WDSCallModemEndPoint.h"
#include <modules/qmi/QmiIndMessage.h>
#include "UnSolMessages/DataBearerTypeChangedMessage.h"
#include "UnSolMessages/DataAllBearerTypeChangedMessage.h"
#include "UnSolMessages/HandoverInformationIndMessage.h"
#include "UnSolMessages/RilPCODataMessage.h"
#include "UnSolMessages/DeregisterWdsIndMessage.h"
#include "UnSolMessages/DeregisterWdsIndCompletedMessage.h"
#include "event/CallManagerEventMessage.h"

using rildata::HandoffState_t;
using rildata::IpFamilyType_t;
using namespace rildata;

WDSCallModemEndPointModule::WDSCallModemEndPointModule
    (string name, ModemEndPoint &owner, IpFamilyType_t ipType, int cid) : ModemEndPointModule(name, owner)
{
  mServiceObject = nullptr;
  mLooper = std::unique_ptr<ModuleLooper>(new ModuleLooper);
  mIpType = ipType;
  mName = name+"_Module";
  mCid = cid;

  std::string qmiInd = name+"_QMI_IND";
  std::string endpointStatusInd = name+"_ENDPOINT_STATUS_IND";
  using std::placeholders::_1;
  mMessageHandler = {
      {REG_MSG(qmiInd), std::bind(&WDSCallModemEndPointModule::handleWdsQmiIndMessage, this, _1)},
      {REG_MSG(endpointStatusInd), std::bind(&WDSCallModemEndPointModule::handleEndPointStatusIndMessage, this, _1)},
      HANDLER(QmiServiceDownIndMessage, WDSCallModemEndPointModule::handleQmiServiceDown),
      HANDLER(DeregisterWdsIndMessage, WDSCallModemEndPointModule::handleDeregisterWdsIndication),
  };
}

WDSCallModemEndPointModule::~WDSCallModemEndPointModule()
{
  Log::getInstance().d(mName +"~WDSCallModemEndPointModule");
  if (mQmiNotifyHandle != nullptr) {
    (void)qmi_client_release(mQmiNotifyHandle);
    mQmiNotifyHandle = nullptr;
  }
  if (mQmiSvcClient != nullptr) {
    (void)qmi_client_release(mQmiSvcClient);
    mQmiSvcClient = nullptr;
  }
  mServiceObject = nullptr;
  Log::getInstance().d("Deleting Looper Queue");
  mLooper.reset(nullptr);
}

void WDSCallModemEndPointModule::init()
{
  Log::getInstance().d(mName+" init");
  ModemEndPointModule::init();
}

qmi_idl_service_object_type WDSCallModemEndPointModule::getServiceObject()
{
  return wds_get_service_object_v01();
}

bool WDSCallModemEndPointModule::handleQmiBinding
(
  qcril_instance_id_e_type instanceId, int8_t stackId
)
{
  Log::getInstance().d(mName+" handleQmiBinding");

  (void)instanceId;
  if (stackId < 0)
    return false;

  wds_bind_subscription_req_msg_v01 bind_request;
  wds_bind_subscription_resp_msg_v01 bind_resp;
  memset(&bind_request, 0, sizeof(bind_request));
  memset(&bind_resp, 0, sizeof(bind_resp));

  if (stackId == 0) {
    bind_request.subscription = WDS_PRIMARY_SUBS_V01;
  }
  else if (stackId == 1) {
    bind_request.subscription = WDS_SECONDARY_SUBS_V01;
  }
  else if (stackId == 2) {
    bind_request.subscription = WDS_TERTIARY_SUBS_V01;
  }
  else {
    return false;
  }

  int ntries = 0;
  do {
    qmi_client_error_type res = qmi_client_send_msg_sync(
         mQmiSvcClient, QMI_WDS_BIND_SUBSCRIPTION_REQ_V01,
         (void*)&bind_request, sizeof(bind_request),
         (void*)&bind_resp, sizeof(bind_resp),
         ModemEndPointModule::DEFAULT_SYNC_TIMEOUT);

    if (QMI_NO_ERR == res && bind_resp.resp.result == QMI_RESULT_SUCCESS_V01) {
      Log::getInstance().d(mName+": QMI binding succeeds. instanceId: "
        + std::to_string((int)instanceId) + " stackId: " + std::to_string(stackId));
      return true;
    }
    usleep(500*1000);
  } while (++ntries < 10);

  return false;
}

void WDSCallModemEndPointModule::processExtendedIpConfigIndication(wds_req_settings_mask_v01 extMask)
{
  Log::getInstance().d(mName+" processExtendedIpConfigIndication mask "
    + std::to_string((int)extMask));
  if (extMask & QMI_WDS_MASK_REQ_SETTINGS_OP_RES_PCO_V01) {
    rildata::PcoDataInfo_t pco = {};
    // query get_runtime_settings
    qmi_client_error_type res = QMI_NO_ERR;
    wds_get_runtime_settings_req_msg_v01 req;
    wds_get_runtime_settings_resp_msg_v01 resp;
    memset (&req, 0, sizeof(req));
    memset (&resp, 0, sizeof(resp));

    req.requested_settings_valid = true;
    req.requested_settings = extMask;

    res = qmi_client_send_msg_sync(
          mQmiSvcClient, QMI_WDS_GET_RUNTIME_SETTINGS_REQ_V01,
          (void*)&req, sizeof(req),
          (void*)&resp, sizeof(resp),
          ModemEndPointModule::DEFAULT_SYNC_TIMEOUT);

    if (res == QMI_NO_ERR && resp.resp.result == QMI_RESULT_SUCCESS_V01) {
      Log::getInstance().d(mName+": QMI WDS get runtime setting success");

      if (resp.operator_reserved_pco_valid) {
        Log::getInstance().d("Operator reserved PCO info update");
        Log::getInstance().d("Container ID: "+std::to_string((int)resp.operator_reserved_pco.container_id));
        Log::getInstance().d("App info len: "+std::to_string((int)resp.operator_reserved_pco.app_specific_info_len));

        pco.cid = static_cast<WDSCallModemEndPoint *>(&mOwner)->getCid();
        pco.bearerProto = (mIpType==IpFamilyType_t::IPv4)?std::string("IP"):std::string("IPV6");
        pco.pcoId = resp.operator_reserved_pco.container_id;
        for (int i=0 ; i<resp.operator_reserved_pco.app_specific_info_len ; i++) {
          Log::getInstance().d("App info "+std::to_string(resp.operator_reserved_pco.app_specific_info[i]));
          pco.contents.push_back(resp.operator_reserved_pco.app_specific_info[i]);
        }
        auto msg = std::make_shared<rildata::RilPCODataMessage>(pco);
        msg->broadcast();
      }
    }
    else {
      Log::getInstance().d(mName+": QMI WDS get runtime setting failed with res = "+
        std::to_string((int)res)+", qmi_err = "+std::to_string((int)resp.resp.error));
    }
  }

  if (extMask & QMI_WDS_MASK_REQ_SETTINGS_PCSCF_SERVER_ADDR_LIST_V01) {
    rildata::CallManagerEvent_t event;
    event.type = rildata::CallManagerEventType_t::_eCallManagerEventPcscfAddressChanged;
    event.cid = static_cast<WDSCallModemEndPoint *>(&mOwner)->getCid();
    auto msg = std::make_shared<rildata::CallManagerEventMessage>(event);
    msg->broadcast();
  }
}

void WDSCallModemEndPointModule::handleWdsQmiIndMessage(std::shared_ptr<Message> msg)
{
  auto shared_indMsg(
        std::static_pointer_cast<QmiIndMessage>(msg));
  Log::getInstance().d(mName+": Handling msg = " + msg->dump());

  QmiIndMsgDataStruct *indData = shared_indMsg->getData();
  if (indData != nullptr)
  {
    WdsUnsolicitedIndHdlr(indData->msgId, indData->indData, indData->indSize);
  } else
  {
    Log::getInstance().d("Unexpected, null data from message");
  }
}

void WDSCallModemEndPointModule::WdsUnsolicitedIndHdlr(unsigned int msg_id, unsigned char *decoded_payload, uint32_t decoded_payload_len)
{
  (void)decoded_payload_len;
  Log::getInstance().d(mName+": WdsUnsolicitedIndHdlr ENTRY = "+std::to_string((int)msg_id));
  switch(msg_id)
  {
    case QMI_WDS_HANDOFF_INFORMATION_IND_V01:
      if (decoded_payload != NULL)
      {
        wds_handoff_information_ind_msg_v01 *res = (wds_handoff_information_ind_msg_v01*) (decoded_payload);
        if(res != NULL)
        {
          Log::getInstance().d(mName+":"
            "wds_handoff_information_ind: result = " + std::to_string(res->handoff_information));
          HandoffState_t state = HandoffState_t::Failure;
          switch(res->handoff_information) {
            case WDS_HANDOFF_INIT_V01:
              state = HandoffState_t::Init;
              break;
            case WDS_HANDOFF_SUCCESS_V01:
              state = HandoffState_t::Success;
              break;
            case WDS_HANDOFF_FAILURE_V01:
              state = HandoffState_t::Failure;
              break;
            default:
              break;
          }

          if(res->handoff_event_info_valid) {
            Log::getInstance().d(mName+":"
            "Handoff srat = " + std::to_string(res->handoff_event_info.srat)+
            ", trat = " + std::to_string(res->handoff_event_info.trat));
            // TODO: compare current srat and trat, and handle as success/failure
          }
          auto msg = std::make_shared<rildata::HandoverInformationIndMessage>(state,
              mIpType, static_cast<WDSCallModemEndPoint *>(&mOwner)->getCid());
          if(res->handoff_failure_reason_valid) {
            Log::getInstance().d(mName+":"
            "Handoff failure reason type = " + std::to_string(res->handoff_failure_reason.failure_reason_type)+
            ", reason = " + std::to_string(res->handoff_failure_reason.failure_reason));
            if (msg != nullptr) {
              msg->setFailReason(res->handoff_failure_reason);
            }
          }
          msg->broadcast();
        }
      }
      break;

    case QMI_WDS_DATA_BEARER_TYPE_IND_V01:
      if (decoded_payload != nullptr)
      {
        wds_data_bearer_type_ind_msg_v01 *ind_payload = (wds_data_bearer_type_ind_msg_v01*) (decoded_payload);
        if(ind_payload != nullptr && ind_payload->bearer_type_info_valid) {
          wds_data_bearer_type_info_v01 bearer_type_info = ind_payload->bearer_type_info;
          // Adding one bearer with BearerInfo_t
          rildata::BearerInfo_t b;
          b.bearerId = (int32_t)bearer_type_info.bearer_id;
          b.uplink = (rildata::RatType_t)bearer_type_info.ul_direction;
          b.downlink = (rildata::RatType_t)bearer_type_info.dl_direction;
          auto msg = std::make_shared<rildata::DataBearerTypeChangedMessage>(
             static_cast<WDSCallModemEndPoint *>(&mOwner)->getCid(), b);
          msg->broadcast();
        }
      }
      break;

    case QMI_WDS_EXTENDED_IP_CONFIG_IND_V01:
      if (decoded_payload != nullptr)
      {
        wds_extended_ip_config_ind_msg_v01 *ind = (wds_extended_ip_config_ind_msg_v01*) (decoded_payload);
        if(ind && ind->changed_ip_config_valid) {
          processExtendedIpConfigIndication(ind->changed_ip_config);
        }
      }
      break;

    case QMI_WDS_EVENT_REPORT_IND_V01:
      if (decoded_payload != nullptr)
      {
        wds_event_report_ind_msg_v01 *event_report_info = (wds_event_report_ind_msg_v01*) (decoded_payload);
        if(event_report_info != nullptr && event_report_info->dormancy_status_valid) {
          rildata::CallManagerEvent_t event;
          if(event_report_info->dormancy_status == WDS_DORMANCY_STATUS_ACTIVE_V01) {
            event.type = rildata::CallManagerEventType_t::_eCallManagerEventLinkActive;
          }
          else {
            event.type = rildata::CallManagerEventType_t::_eCallManagerEventLinkDormant;
          }
          event.cid = static_cast<WDSCallModemEndPoint *>(&mOwner)->getCid();
          auto msg = std::make_shared<rildata::CallManagerEventMessage>(event);
          msg->broadcast();
        }
      }
      break;

    default:
      Log::getInstance().d("Ignoring wds ind event %d"+ std::to_string(msg_id));
      break;
  }
}

void WDSCallModemEndPointModule::handleEndPointStatusIndMessage(std::shared_ptr<Message> msg)
{
  Log::getInstance().d(mName+": Handling msg = " + msg->dump());

  auto shared_indMsg(std::static_pointer_cast<EndpointStatusIndMessage>(msg));
  if (shared_indMsg->getState() == ModemEndPoint::State::OPERATIONAL) {
    qmi_client_error_type res = QMI_NO_ERR;
    wds_bind_mux_data_port_req_msg_v01 mux_data_port_req;
    wds_bind_mux_data_port_resp_msg_v01 mux_data_port_resp;
    memset(&mux_data_port_req, 0, sizeof(mux_data_port_req));
    memset(&mux_data_port_resp, 0, sizeof(mux_data_port_resp));

    int eptype = - 1;
    int epid = - 1;
    int muxid = - 1;
    static_cast<WDSCallModemEndPoint *>(&mOwner)->getBindMuxDataPortParams(&eptype, &epid, &muxid);

    mux_data_port_req.ep_id_valid = (epid != -1);
    mux_data_port_req.ep_id.ep_type = (data_ep_type_enum_v01)eptype;
    mux_data_port_req.ep_id.iface_id = (uint32_t)epid;

    mux_data_port_req.mux_id_valid = (muxid != -1);
    mux_data_port_req.mux_id = (uint8_t)muxid;

    res = qmi_client_send_msg_sync(
          mQmiSvcClient, QMI_WDS_BIND_MUX_DATA_PORT_REQ_V01,
          (void*)&mux_data_port_req, sizeof(mux_data_port_req),
          (void*)&mux_data_port_resp, sizeof(mux_data_port_resp),
          ModemEndPointModule::DEFAULT_SYNC_TIMEOUT);

    if (res == QMI_NO_ERR && mux_data_port_resp.resp.result == QMI_RESULT_SUCCESS_V01) {
      Log::getInstance().d(mName+": QMI WDS mux data port binding success");
    }
    else {
      Log::getInstance().d(mName+": QMI WDS mux data port binding failed with res = "+
        std::to_string((int)res)+", qmi_err = "+std::to_string((int)mux_data_port_resp.resp.error));
    }

    if(static_cast<WDSCallModemEndPoint *>(&mOwner)->getIpType() == IpFamilyType_t::IPv6) {
      wds_set_client_ip_family_pref_req_msg_v01 ip_family_pref_req;
      wds_set_client_ip_family_pref_resp_msg_v01 ip_family_pref_resp;
      memset(&ip_family_pref_req, 0, sizeof(ip_family_pref_req));
      memset(&ip_family_pref_resp, 0, sizeof(ip_family_pref_resp));

      ip_family_pref_req.ip_preference = WDS_IP_FAMILY_IPV6_V01;
      res = qmi_client_send_msg_sync (
            mQmiSvcClient, QMI_WDS_SET_CLIENT_IP_FAMILY_PREF_REQ_V01,
            (void *)&ip_family_pref_req, sizeof(ip_family_pref_req),
            (void*)&ip_family_pref_resp, sizeof(ip_family_pref_resp),
            ModemEndPointModule::DEFAULT_SYNC_TIMEOUT);

      if (res == QMI_NO_ERR && ip_family_pref_resp.resp.result == QMI_RESULT_SUCCESS_V01) {
        Log::getInstance().d(mName+": QMI WDS IP v6 family binding success");
      }
      else if(res == QMI_SERVICE_ERR && ip_family_pref_resp.resp.error == QMI_ERR_NO_EFFECT_V01) {
          Log::getInstance().d(mName+": QMI WDS IP v6 family already bound");
      }
      else {
        Log::getInstance().d(mName+": QMI WDS IP v6 family binding failed with res = "+
          std::to_string((int)res)+", qmi_err = "+std::to_string((int)ip_family_pref_resp.resp.error));
      }
    }

    wds_indication_register_req_msg_v01 ind_reg_req;
    wds_indication_register_resp_msg_v01 ind_reg_resp;
    memset(&ind_reg_req, 0, sizeof(ind_reg_req));
    memset(&ind_reg_resp, 0, sizeof(ind_reg_resp));

    ind_reg_req.report_handoff_information_valid = true;
    ind_reg_req.report_handoff_information = 1;
    ind_reg_req.report_bearer_type_valid = true;
    ind_reg_req.report_bearer_type = 1;
    ind_reg_req.report_extended_ip_config_change_valid = true;
    ind_reg_req.report_extended_ip_config_change = 1;

    res = qmi_client_send_msg_sync(
          mQmiSvcClient, QMI_WDS_INDICATION_REGISTER_REQ_V01,
          (void*)&ind_reg_req, sizeof(ind_reg_req),
          (void*)&ind_reg_resp, sizeof(ind_reg_resp),
          ModemEndPointModule::DEFAULT_SYNC_TIMEOUT);
    if (res != QMI_NO_ERR) {
      Log::getInstance().d(mName+": wds indication register failed "
                            +std::to_string((int)res));
    }

    queryAllDataBearerType();
    processExtendedIpConfigIndication(QMI_WDS_MASK_REQ_SETTINGS_OP_RES_PCO_V01);
    static_cast<WDSCallModemEndPoint *>(&mOwner)->updateLinkStateChangeReport();
  }
  else {
    Log::getInstance().d(mName+": ModemEndPoint is not operational");
  }
}

Message::Callback::Status WDSCallModemEndPointModule::queryAllDataBearerType(void)
{
  Log::getInstance().d(mName+"::queryAllDataBearerType");

  Message::Callback::Status ret = Message::Callback::Status::SUCCESS;
  qmi_client_error_type res = QMI_NO_ERR;
  wds_get_data_bearer_type_req_msg_v01 req;
  wds_get_data_bearer_type_resp_msg_v01 resp;
  memset(&req, 0, sizeof(req));
  memset(&resp, 0, sizeof(resp));

  res = qmi_client_send_msg_sync(
        mQmiSvcClient, QMI_WDS_GET_DATA_BEARER_TYPE_REQ_V01,
        (void *)&req, sizeof(req),
        (void *)&resp, sizeof(resp),
        ModemEndPointModule::DEFAULT_SYNC_TIMEOUT);
  if (res != QMI_NO_ERR) {
    Log::getInstance().d(mName+" queryAllDataBearerType "
                          "Failed to send QMI_WDS_GET_DATA_BEARER_TYPE_REQ_V01"
                          +std::to_string((int)res));
    ret = Message::Callback::Status::FAILURE;
  }
  else if (resp.resp.result != QMI_RESULT_SUCCESS_V01){
    Log::getInstance().d(mName+" queryAllDataBearerType "
                          "responded result is not success"
                          +std::to_string((int)resp.resp.result));
    ret = Message::Callback::Status::FAILURE;
  }
  else {
    rildata::AllocatedBearer_t bearerInfo;
    bearerInfo.cid = static_cast<WDSCallModemEndPoint *>(&mOwner)->getCid();
    bearerInfo.apn = static_cast<WDSCallModemEndPoint *>(&mOwner)->getApnName();
    bearerInfo.ifaceName = static_cast<WDSCallModemEndPoint *>(&mOwner)->getIfaceName();
    // Adding bearers with BearerInfo_t
    if(resp.bearer_type_info_valid) {
      for(uint32_t i=0;i<resp.bearer_type_info_len;i++) {
        rildata::BearerInfo_t b;
        b.bearerId = (int32_t)resp.bearer_type_info[i].bearer_id;
        b.uplink = (rildata::RatType_t)resp.bearer_type_info[i].ul_direction;
        b.downlink = (rildata::RatType_t)resp.bearer_type_info[i].dl_direction;
        bearerInfo.bearers.push_back(b);
      }
      auto msg = std::make_shared<rildata::DataAllBearerTypeChangedMessage>(bearerInfo);
      msg->broadcast();
    }
  }

  return ret;
}

void WDSCallModemEndPointModule::handleQmiServiceDown(
    std::shared_ptr<QmiServiceDownIndMessage> shared_srvDownMsg) {

  qmi_client_error_type error = shared_srvDownMsg->getErrorCode();

  Log::getInstance().d(mName+": " + mOwner.mName
      + " Service is down. Error code is " + std::to_string(error));

  if (mOwner.getState() == ModemEndPoint::State::OPERATIONAL) {
    mOwner.setState(ModemEndPoint::State::NON_OPERATIONAL);

    // release client
    if (mQmiSvcClient) {
      qmi_client_error_type rc = qmi_client_release(mQmiSvcClient);
      if (rc != QMI_NO_ERR) {
        Log::getInstance().d(mName+": "+ mOwner.mName +" :Failed to release client");
      } else {
        mQmiSvcClient = nullptr;
        Log::getInstance().d(mName+": " + mOwner.mName + " : Release client");
      }
    } else {
      Log::getInstance().d(mName+": " + mOwner.mName + " : unexpected for client");
    }
  }
}

void WDSCallModemEndPointModule::releaseWDSCallEPModuleQMIClient()
{
  if (mQmiNotifyHandle != nullptr) {
    Log::getInstance().d(mName+"::releaseWDSCallEPModuleQMIClient :: release mQmiNotifyHandle");
    (void)qmi_client_release(mQmiNotifyHandle);
    mQmiNotifyHandle = nullptr;
  }
  if (mQmiSvcClient != nullptr) {
    Log::getInstance().d(mName+"::releaseWDSCallEPModuleQMIClient :: release mQmiSvcClient");
    (void)qmi_client_release(mQmiSvcClient);
    mQmiSvcClient = nullptr;
  }
  mServiceObject = nullptr;
}

void WDSCallModemEndPointModule::handleDeregisterWdsIndication(std::shared_ptr<Message> msg)
{
  Log::getInstance().d(mName+"handleDeregisterWdsIndication");
  std::shared_ptr<DeregisterWdsIndMessage> m = std::static_pointer_cast<DeregisterWdsIndMessage>(msg);
  if(m != nullptr)
  {
    if(m->getCid() == this->getCid())
    {
      releaseWDSCallEPModuleQMIClient();
      Log::getInstance().d(mName + "Deregistering messages for "+ mOwner.mName);
      Dispatcher::getInstance().unRegisterHandler(this);
      auto msg = std::make_shared<DeregisterWdsIndCompletedMessage>(this->getCid(), mIpType);
      msg->broadcast();
    }
    else
    {
      Log::getInstance().d("Ignoring DeregisterWdsIndication");
    }
  }
  else
  {
    Log::getInstance().d(mName+"Received Null message");
  }
}
