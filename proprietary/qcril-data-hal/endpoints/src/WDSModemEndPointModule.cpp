/******************************************************************************
#  Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include <cstring>
#include "WDSModemEndPointModule.h"
#include <modules/qmi/QmiIndMessage.h>
#include "ProfileHandler.h"
#include "sync/GetAttachListMessage.h"
#include "sync/SetProfileParamsSyncMessage.h"
#include "sync/GetCallBringUpCapabilitySyncMessage.h"
#include "UnSolMessages/SetLteAttachPdnListActionResultMessage.h"
#include "MessageCommon.h"
#include "UnSolMessages/KeepAliveIndMessage.h"
#include "sync/RegisterForKeepAliveSyncMessage.h"

using namespace rildata;

WDSModemEndPointModule::WDSModemEndPointModule(string name,
    ModemEndPoint &owner) : ModemEndPointModule(name, owner)
{
  mServiceObject = nullptr;
  mLooper = std::unique_ptr<ModuleLooper>(new ModuleLooper);

  using std::placeholders::_1;
  mMessageHandler = {
      HANDLER(GetAttachListMessage, WDSModemEndPointModule::handleGetAttachList),
      HANDLER(SetAttachListSyncMessage, WDSModemEndPointModule::handleSetAttachList),
      HANDLER(GetAttachListCapabilitySyncMessage,
              WDSModemEndPointModule::handleGetAttachListCap),
      HANDLER(GetApnTypesForName,
              WDSModemEndPointModule::handleGetApnTypesForName),
      HANDLER(SetProfileParamsSyncMessage,
              WDSModemEndPointModule::handleSetProfileParamsSync),
      HANDLER(GetCallBringUpCapabilitySyncMessage,
              WDSModemEndPointModule::handleGetCallBringUpCapabilitySync),
      HANDLER(RegisterForKeepAliveSyncMessage,
              WDSModemEndPointModule::handleRegisterForKeepAliveMessageSync),
      {REG_MSG("WDSModemEndPoint_QMI_IND"),
             std::bind(&WDSModemEndPointModule::handleWdsQmiIndMessage, this, _1)},
  };
}

WDSModemEndPointModule::~WDSModemEndPointModule()
{
  mLooper = nullptr;
}

void WDSModemEndPointModule::init()
{
  ModemEndPointModule::init();
  mStackId[0] = 0;
}

qmi_idl_service_object_type WDSModemEndPointModule::getServiceObject()
{
  return wds_get_service_object_v01();
}

bool WDSModemEndPointModule::handleQmiBinding
(
  qcril_instance_id_e_type instanceId, int8_t stackId
)
{
  Log::getInstance().d("[WDSModemEndPointModule] handleQmiBinding");

  (void)instanceId;
  if (stackId < 0)
    return false;

  wds_bind_subscription_req_msg_v01 bind_request;
  wds_bind_subscription_resp_msg_v01 bind_resp;
  memset(&bind_request, 0, sizeof(bind_request));
  memset(&bind_resp, 0, sizeof(bind_resp));

  if (stackId == 0)
  {
    bind_request.subscription = WDS_PRIMARY_SUBS_V01;
  } else if (stackId == 1)
  {
    bind_request.subscription = WDS_SECONDARY_SUBS_V01;
  } else if (stackId == 2)
  {
    bind_request.subscription = WDS_TERTIARY_SUBS_V01;
  } else
    return false;

  int ntries = 0;
  do
  {
    qmi_client_error_type res = qmi_client_send_msg_sync(
         mQmiSvcClient, QMI_WDS_BIND_SUBSCRIPTION_REQ_V01,
         (void*)&bind_request, sizeof(bind_request),
         (void*)&bind_resp, sizeof(bind_resp),
         ModemEndPointModule::DEFAULT_SYNC_TIMEOUT);

    if (QMI_NO_ERR == res && bind_resp.resp.result ==
            QMI_RESULT_SUCCESS_V01)
    {
      Log::getInstance().d(
        "[WDSModemEndPointModule]: QMI binding succeeds. instanceId: "
        + std::to_string((int)instanceId) + " stackId: " + std::to_string(stackId));
      return true;
    }
    usleep(500*1000);
  } while (++ntries < 10);

  return false;
}

void WDSModemEndPointModule::handleWdsQmiIndMessage
(
  std::shared_ptr<Message> msg
)
{
  auto shared_indMsg(
        std::static_pointer_cast<QmiIndMessage>(msg));
  Log::getInstance().d("[WDSModemEndPointModule]: Handling msg = " + msg->dump());

  QmiIndMsgDataStruct *indData = shared_indMsg->getData();
  if (indData != nullptr)
  {
    WdsUnsolicitedIndHdlr(indData->msgId, indData->indData, indData->indSize);
  } else
  {
    Log::getInstance().d("Unexpected, null data from message");
  }
}

void WDSModemEndPointModule::WdsUnsolicitedIndHdlr
(
  unsigned int   msg_id,
  unsigned char *decoded_payload,
  uint32_t       decoded_payload_len
)
{
  (void)decoded_payload_len;
  Log::getInstance().d("[WDSModemEndPointModule]: WdsUnsolicitedIndHdlr ENTRY");
  switch(msg_id)
  {
    case QMI_WDS_SET_LTE_ATTACH_PDN_LIST_ACTION_RESULT_IND_V01:
      if (decoded_payload != NULL)
      {
        wds_set_lte_attach_pdn_list_action_result_ind_msg_v01 *res
        = (wds_set_lte_attach_pdn_list_action_result_ind_msg_v01*) (decoded_payload);

        if(res != NULL)
        {
          Log::getInstance().d("[WDSModemEndPointModule]:"
            "WdsUnsolicitedIndHdlr: result = " + std::to_string(res->result));
          auto msg = std::make_shared<rildata::SetLteAttachPdnListActionResultMessage>(*res);
          msg->broadcast();
        }
      }
      break;

    case QMI_WDS_MODEM_ASSISTED_KA_STATUS_IND_V01:
       if(decoded_payload != nullptr)
       {
          wds_modem_assisted_ka_status_ind_msg_v01 *ma_ka_info = (wds_modem_assisted_ka_status_ind_msg_v01*) (decoded_payload);
          if(ma_ka_info != nullptr) {
            rildata::KeepAliveInfo_t k;
            k.status = (rildata::KeepAliveResult_t)ma_ka_info->status;
            k.handle = (uint32_t)ma_ka_info->keep_alive_handle;
            auto msg = std::make_shared<rildata::KeepAliveIndMessage>(k);
            msg->broadcast();
          }
       }
       break;

    default:
      Log::getInstance().d("Ignoring wds ind event %d"+ std::to_string(msg_id));
      break;
  }
}

void WDSModemEndPointModule::handleGetAttachList
(
  std::shared_ptr<Message> m
)
{
  Log::getInstance().d("[WDSModemEndPointModule]: Handling msg = " + m->dump());

  auto msg = std::static_pointer_cast<GetAttachListMessage>(m);
  if (msg!=NULL)
  {
    if (mQmiSvcClient != nullptr)
    {
      qmi_client_error_type rc;
      wds_get_lte_attach_pdn_list_req_msg_v01 req;
      wds_get_lte_attach_pdn_list_resp_msg_v01 resp;
      memset(&req, 0, sizeof(req));
      memset(&resp, 0, sizeof(resp));

      rc = qmi_client_send_msg_sync(
          mQmiSvcClient, QMI_WDS_GET_LTE_ATTACH_PDN_LIST_REQ_V01,
          (void *)&req, sizeof(req),
          (void *)&resp, sizeof(resp), QCRIL_DATA_QMI_TIMEOUT);
      if (QMI_NO_ERR != rc)
      {
        Log::getInstance().d("[WDSModemEndPointModule]:"
                             "failed to send get attach list request");
        msg->sendResponse(
            msg, Message::Callback::Status::FAILURE, nullptr);
      } else
      {
        if (QMI_RESULT_SUCCESS_V01 != resp.resp.result)
        {
          Log::getInstance().d("[WDSModemEndPointModule]:"
                               " get attach list response returned failed, err = " +
              std::to_string(resp.resp.result));
          msg->sendResponse(
              msg, Message::Callback::Status::FAILURE, nullptr);
        } else
        {
          Log::getInstance().d("[WDSModemEndPointModule]:"
                               " Get attach list response succeeeded");
          std::shared_ptr<std::list<uint16_t>> temp_list = std::make_shared<std::list<uint16_t>>();
          if (resp.attach_pdn_list_valid)
          {
            if( resp.pending_attach_pdn_list_valid )
            {
              for (uint32_t i = 0; i < resp.pending_attach_pdn_list_len; i++)
              {
                Log::getInstance().d("[WDSModemEndPointModule]:Attach pending pdn list profile = "
                                       +std::to_string(resp.pending_attach_pdn_list[i]));
                temp_list->push_back(resp.pending_attach_pdn_list[i]);
              }
            }
            else
            {
              for (uint32_t i = 0; i < resp.attach_pdn_list_len; i++)
              {
                Log::getInstance().d("[WDSModemEndPointModule]:Attach pdn list profile = "
                                      +std::to_string(resp.attach_pdn_list[i]));
                temp_list->push_back(resp.attach_pdn_list[i]);
              }
            }
          }
          else
          {
            Log::getInstance().d("[WDSModemEndPointModule]::"
                      "Optional parameter, Attach Pdn list in empty ");
          }
          msg->sendResponse(msg, Message::Callback::Status::SUCCESS,
                               temp_list);
        }
      }
    }
  }
}

void WDSModemEndPointModule::handleSetAttachList
(
  std::shared_ptr<Message> m
)
{
  Log::getInstance().d("[WDSModemEndPointModule]: Handling msg = " + m->dump());
  auto msg = std::static_pointer_cast<SetAttachListSyncMessage>(m);
  if (msg!=NULL)
  {
    Log::getInstance().d("[WDSModemEndPointModule]: handleSetAttachList Entry");
    if (mQmiSvcClient != nullptr)
    {
      qmi_client_error_type rc;
      wds_set_lte_attach_pdn_list_req_msg_v01 req;
      wds_set_lte_attach_pdn_list_resp_msg_v01 resp;

      memset(&req, 0, sizeof(req));
      memset(&resp, 0, sizeof(resp));
      auto params = std::make_shared<std::list<uint16_t>>();

      SetAttachListSyncMessage::AttachListAction action = SetAttachListSyncMessage::AttachListAction::NONE;
      msg->getParams(*params, action);

      req.attach_pdn_list_len = params->size();
      std::list<uint16_t>::const_iterator it;
      uint8_t i = 0;
      for(i=0, it = params->begin(); (( it != params->end()) && (i < req.attach_pdn_list_len));++it,++i)
      {
        req.attach_pdn_list[i] = *it;
      }
      // Print all elements in the list
      for(uint8_t i =0;i < params->size(); i++)
      {
        Log::getInstance().d("[WDSModemEndPointModule]:"
          " elements set in attach list ="+ std::to_string(req.attach_pdn_list[i]));
      }
      if( action == SetAttachListSyncMessage::AttachListAction::DISCONNECT_ATTACH_APN_ONLY )
      {
        req.action_valid = TRUE;
        req.action = wds_attach_pdn_list_action_enum_type_v01::WDS_ATTACH_PDN_LIST_ACTION_DETACH_OR_PDN_DISCONNECT_V01;
      } else if (action == SetAttachListSyncMessage::AttachListAction::NONE )
      {
        req.action_valid = FALSE;
        req.action = wds_attach_pdn_list_action_enum_type_v01::WDS_ATTACH_PDN_LIST_ACTION_NONE_V01;
      }
      rc = qmi_client_send_msg_sync(
          mQmiSvcClient, QMI_WDS_SET_LTE_ATTACH_PDN_LIST_REQ_V01,
          (void *)&req, sizeof(req),
          (void *)&resp, sizeof(resp), QCRIL_DATA_QMI_TIMEOUT);

      if (QMI_NO_ERR != rc)
      {
        Log::getInstance().d("[WDSModemEndPointModule]:"
                          " failed to send set attach list request");
        msg->sendResponse(msg, Message::Callback::Status::FAILURE, nullptr);
      } else
      {
        if (QMI_RESULT_SUCCESS_V01 != resp.resp.result)
        {
          Log::getInstance().d("[WDSModemEndPointModule]:"
                               " Set attach list response returned failed, err = " +
                                std::to_string(resp.resp.result));
          msg->sendResponse(msg, Message::Callback::Status::FAILURE, nullptr);
        } else
        {
          Log::getInstance().d("[WDSModemEndPointModule]: Set attach list response succeeeded.");
          msg->sendResponse(msg, Message::Callback::Status::SUCCESS,
                            nullptr);
        }
      }
    }
  }
}

void WDSModemEndPointModule::handleGetAttachListCap
(
  std::shared_ptr<Message> m
)
{
  Log::getInstance().d("[WDSModemEndPointModule]: Handling msg = " + m->dump());
  auto msg = std::static_pointer_cast<GetAttachListCapabilitySyncMessage>(m);
  if (msg!=NULL)
  {
    if (mQmiSvcClient != nullptr)
    {
      qmi_client_error_type rc;
      wds_get_capabilities_req_msg_v01 req;
      wds_get_capabilities_resp_msg_v01 resp;
      memset(&req, 0, sizeof(req));
      memset(&resp, 0, sizeof(resp));

      req.attach_pdn_list_action_capability_valid = TRUE;
      req.attach_pdn_list_action_capability = AttachAction::ATTACH_PDN_ACTION_SUPPORTED;

      rc = qmi_client_send_msg_sync(
           mQmiSvcClient, QMI_WDS_GET_CAPABILITIES_REQ_V01,
           (void *)&req, sizeof(req),
           (void *)&resp, sizeof(resp), QCRIL_DATA_QMI_TIMEOUT);

      if (QMI_NO_ERR != rc)
      {
        Log::getInstance().d("[WDSModemEndPointModule]:"
                             " failed to send 'get capabilities' request");
        msg->sendResponse(msg, Message::Callback::Status::FAILURE, nullptr);
      } else
      {
        if (QMI_RESULT_SUCCESS_V01 != resp.resp.result)
        {
          Log::getInstance().d("[WDSModemEndPointModule]:get attach list response"
                               "returned failed, err = "+ std::to_string(resp.resp.result));
          msg->sendResponse(msg, Message::Callback::Status::FAILURE, nullptr);
        } else
        {
          auto cap = std::make_shared<AttachListCap>();
          Log::getInstance().d("[WDSModemEndPointModule]:"
                               " Get Capabilities response succeeeded");
          if (resp.attach_pdn_list_action_capability_valid)
          {
            if ( resp.attach_pdn_list_action_capability ==
                      AttachAction::ATTACH_PDN_ACTION_SUPPORTED )
            {
              Log::getInstance().d("[WDSModemEndPointModule]:Get Capabilities"
                                   "response returned Action_Supported");
              *cap = ACTION_SUPPORTED;
            } else
            {
              Log::getInstance().d("[WDSModemEndPointModule]:Get Capabilities"
                                   " response returned Action_Not_Supported");
              *cap = ACTION_NOT_SUPPORTED;
            }
          } else
          {
            Log::getInstance().d("[WDSModemEndPointModule]:Get Capabilities"
                            "action invalid returning Action_Not_Supported");
            *cap = ACTION_NOT_SUPPORTED;
          }
          msg->sendResponse(msg, Message::Callback::Status::SUCCESS,cap);
        }
      }
    }
  }
}

void WDSModemEndPointModule::handleGetApnTypesForName
(
  std::shared_ptr<Message> m
)
{
  Log::getInstance().d("[WDSModemEndPointModule]: Handling msg = " + m->dump());

  auto msg = std::static_pointer_cast<GetApnTypesForName>(m);
  if (msg!=NULL)
  {
    if (mQmiSvcClient != nullptr)
    {
      string apnName = msg->getParams();

      std::list<int32_t> apnTypes = getApnTypesByTechType(WDS_PROFILE_TYPE_3GPP_V01, apnName);
      if (apnTypes.size() == 0)
      {
        Log::getInstance().d("[WDSModemEndPointModule]:"
                           "could not find any matches for APN name " + apnName+
                             "in 3GPP profiles, trying 3GPP2");
        apnTypes = getApnTypesByTechType(WDS_PROFILE_TYPE_3GPP2_V01, apnName);
      }
      if (apnTypes.size() == 0)
      {
        Log::getInstance().d("[WDSModemEndPointModule]:"
                           "could not find any matches for APN name " + apnName +
                             "in 3GPP2 profiles either");
        msg->sendResponse(msg, Message::Callback::Status::FAILURE, nullptr);
        return;
      }
      std::shared_ptr<std::list<int32_t>> temp_list = std::make_shared<std::list<int32_t>>(apnTypes);
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS,
                               temp_list);
    }
    else
    {
      Log::getInstance().d("[WDSModemEndPointModule]:"
                          "mQmiSvcClient is null");
      msg->sendResponse(msg, Message::Callback::Status::FAILURE, nullptr);
    }
  }
}

std::list<int32_t> WDSModemEndPointModule::getApnTypesByTechType(wds_profile_type_enum_v01 techType, string apnName)
{
  qmi_client_error_type rc;
  wds_get_profile_list_req_msg_v01   pl_req;
  wds_get_profile_list_resp_msg_v01  pl_resp;
  std::list<int32_t> apnTypes;
  memset(&pl_req, 0, sizeof(pl_req));
  memset(&pl_resp, 0, sizeof(pl_resp));

  do {
    pl_req.profile_type_valid = TRUE;
    pl_req.profile_type = techType;

    rc = qmi_client_send_msg_sync(
        mQmiSvcClient, QMI_WDS_GET_PROFILE_LIST_REQ_V01,
        (void *)&pl_req, sizeof(pl_req),
        (void *)&pl_resp, sizeof(pl_resp), QCRIL_DATA_QMI_TIMEOUT);
    if (QMI_NO_ERR != rc) {
      Log::getInstance().d("[WDSModemEndPointModule]:"
                           "failed to send get profile list request");
      break;
    }
    else {
      if (QMI_RESULT_SUCCESS_V01 != pl_resp.resp.result) {
        Log::getInstance().d("[WDSModemEndPointModule]:"
                             " get profile list response returned failed, err = " +
            std::to_string(pl_resp.resp.result));
        break;
      }
      else {
        Log::getInstance().d("[WDSModemEndPointModule]:"
                             " Get profile list response succeeeded");
        Log::getInstance().d("get_profile_list for profile_tech "  +
                             std::to_string(pl_req.profile_type) +
                             "returned profile ids " +
                             std::to_string(pl_resp.profile_list_len));
        for(uint32_t i=0; i<pl_resp.profile_list_len; i++) {
          wds_get_profile_settings_req_msg_v01   ps_req;
          wds_get_profile_settings_resp_msg_v01  ps_resp;

          memset(&ps_req, 0, sizeof(ps_req));
          memset(&ps_resp, 0, sizeof(ps_resp));

          ps_req.profile.profile_type = techType;
          ps_req.profile.profile_index = pl_resp.profile_list[i].profile_index;

          rc = qmi_client_send_msg_sync(
              mQmiSvcClient, QMI_WDS_GET_PROFILE_SETTINGS_REQ_V01,
              (void *)&ps_req, sizeof(ps_req),
              (void *)&ps_resp, sizeof(ps_resp), QCRIL_DATA_QMI_TIMEOUT);
          if (QMI_NO_ERR != rc) {
            Log::getInstance().d("[WDSModemEndPointModule]:"
                           "failed to send get profile settings request");
            break;
          }
          else {
            if (QMI_RESULT_SUCCESS_V01 != ps_resp.resp.result) {
              Log::getInstance().d("[WDSModemEndPointModule]:"
                             " get profile settings response returned failed, err = " +
                  std::to_string(ps_resp.resp.result));
              break;
            }
            else {
              Log::getInstance().d("successfully queried profile " +
                      std::to_string(ps_req.profile.profile_index));
              string qmiApnName;
              if ((techType == WDS_PROFILE_TYPE_3GPP_V01 ) &&
                   ps_resp.apn_name_valid) {
                qmiApnName = ps_resp.apn_name;
              }
              if ((techType == WDS_PROFILE_TYPE_3GPP2_V01 ) &&
                   ps_resp.apn_string_valid){
                qmiApnName = ps_resp.apn_string;
              }
              //Do case insensitive matching for apn name
              if ((qmiApnName.length() == apnName.length()) &&
                (std::equal(qmiApnName.begin(), qmiApnName.end(), apnName.begin(), []
                    (auto a, auto b){return std::tolower(a)==std::tolower(b);})))
              {
                Log::getInstance().d("[WDSModemEndPointModule]: Match found for apnName " +
                                      apnName);
                if (ps_resp.apn_type_mask_valid) {
                  Log::getInstance().d("[mask " + std::to_string(ps_resp.apn_type_mask));
                  for (int j=(int)WDS_APN_TYPE_MASK_DEFAULT_V01, bitpos = 0; j<= (int)WDS_APN_TYPE_MASK_EMERGENCY_V01; j=1<< ++bitpos)
                  {
                    if (ps_resp.apn_type_mask & j) {
                      apnTypes.push_back(j);
                    }
                  }
                  //We only look for the 1st entry in profile db that matches apn name
                  break;
                }
                else {
                  Log::getInstance().d("[WDSModemEndPointModule]: apn type mask not present ");
                }
              } //apn name match
            } // successful query of profile entry
          }
        }
      }
    }
  }while (0);

  return apnTypes;
}

/**
 * @brief Returns boolean to indicate if passed apn type is Internet
 * capable
 *
 * @return boolean
 *
 **/
bool WDSModemEndPointModule::isDefaultProfile
(
  int32_t supportedApnTypesBitmap
)
{
  if ( supportedApnTypesBitmap & (int32_t)ApnTypes_t::DEFAULT) {
    return TRUE;
  }
  return FALSE;
} /* WDSModemEndPointModule::isDefaultProfile() */

/**
 * @brief Converts ApnTypes_t to WDS Apn type mask
 *
 * @return ApnTypes_t
 *
 **/
wds_apn_type_mask_v01 WDSModemEndPointModule::convertToApnTypeMask
(
  int32_t supportedApnTypesBitmap
)
{
  wds_apn_type_mask_v01 apnTypeMask = WDS_APN_TYPE_MASK_UNSPECIFIED_V01;

  if(supportedApnTypesBitmap & (int32_t)ApnTypes_t::DEFAULT)
  {
    apnTypeMask |= WDS_APN_TYPE_MASK_DEFAULT_V01;
  }
  if(supportedApnTypesBitmap & (int32_t)ApnTypes_t::IMS)
  {
    apnTypeMask |= WDS_APN_TYPE_MASK_IMS_V01;
  }
  if(supportedApnTypesBitmap & (int32_t)ApnTypes_t::MMS)
  {
    apnTypeMask |= WDS_APN_TYPE_MASK_MMS_V01;
  }
  if(supportedApnTypesBitmap & (int32_t)ApnTypes_t::DUN)
  {
    apnTypeMask |= WDS_APN_TYPE_MASK_DUN_V01;
  }
  if(supportedApnTypesBitmap & (int32_t)ApnTypes_t::SUPL)
  {
    apnTypeMask |= WDS_APN_TYPE_MASK_SUPL_V01;
  }
  if(supportedApnTypesBitmap & (int32_t)ApnTypes_t::HIPRI)
  {
    apnTypeMask |= WDS_APN_TYPE_MASK_HIPRI_V01;
  }
  if(supportedApnTypesBitmap & (int32_t)ApnTypes_t::FOTA)
  {
    apnTypeMask |= WDS_APN_TYPE_MASK_FOTA_V01;
  }
  if(supportedApnTypesBitmap & (int32_t)ApnTypes_t::CBS)
  {
    apnTypeMask |= WDS_APN_TYPE_MASK_CBS_V01;
  }
  if(supportedApnTypesBitmap & (int32_t)ApnTypes_t::IA)
  {
    apnTypeMask |= WDS_APN_TYPE_MASK_IA_V01;
  }
  if(supportedApnTypesBitmap & (int32_t)ApnTypes_t::EMERGENCY)
  {
    apnTypeMask |= WDS_APN_TYPE_MASK_EMERGENCY_V01;
  }
  return apnTypeMask;
}

/**
 * @brief Sets supported APN types for the profile
 *
 * @return None
 **/
void WDSModemEndPointModule::setProfileSupportedAPNTypes
(
  wds_profile_type_enum_v01 profile_type,
  uint8_t profile_index,
  wds_apn_type_mask_v01 apnTypeMask
)
{
  Log::getInstance().d("WDSModemEndPointModule::setProfileSupportedAPNTypes profile_type= "+
      std::to_string((int)profile_type)+" profile_index= " + std::to_string(profile_index) + \
      "apnTypeMask = "+std::to_string((int)apnTypeMask));
  if (mQmiSvcClient != nullptr)
  {
    wds_modify_profile_settings_req_msg_v01 ps_req;
    wds_modify_profile_settings_resp_msg_v01 ps_resp;

    memset(&ps_req, 0, sizeof(wds_modify_profile_settings_req_msg_v01));
    memset(&ps_resp, 0, sizeof(wds_modify_profile_settings_resp_msg_v01));

    ps_req.profile.profile_type = profile_type;
    ps_req.profile.profile_index = profile_index;
    ps_req.apn_type_mask_valid = TRUE;
    ps_req.apn_type_mask = apnTypeMask;

    int rc = qmi_client_send_msg_sync(mQmiSvcClient,
                                      QMI_WDS_MODIFY_PROFILE_SETTINGS_REQ_V01,
                                      (void *)&ps_req,
                                      sizeof(ps_req),
                                      (void *)&ps_resp,
                                      sizeof(ps_resp),
                                      QCRIL_DATA_QMI_TIMEOUT);
    if ( QMI_NO_ERR != rc ||
         QMI_RESULT_SUCCESS_V01 != ps_resp.resp.result) {
      Log::getInstance().d("[WDSModemEndPointModule] Failed to send"
        "QMI_WDS_MODIFY_PROFILE_SETTINGS_REQ_V01 for apn_type:" \
          +std::to_string(apnTypeMask)+"with rc ="+std::to_string(rc));
    } else {
      Log::getInstance().d("[WDSModemEndPointModule] setProfileApnTypeMask::Successfully sent"
        "QMI_WDS_MODIFY_PROFILE_SETTINGS_REQ_V01 for type ="+ \
        std::to_string(apnTypeMask));
    }
  }
} /* WDSModemEndPointModule::setProfileSupportedAPNTypes() */

/**
 * @brief Sets provided profile as default on the modem
 *
 * @return None
 **/
void WDSModemEndPointModule::setProfileDefault
(
  wds_profile_type_enum_v01 profile_type,
  uint8_t profile_index
)
{
  Log::getInstance().d("WDSModemEndPointModule::setProfileDefault profile_type= " +\
      std::to_string((int)profile_type)+" index= "+std::to_string(profile_index));
  if (mQmiSvcClient != nullptr)
  {
    /* set supplied profile as default */
    wds_set_default_profile_num_req_msg_v01 ps_req;
    wds_set_default_profile_num_resp_msg_v01 ps_resp;
    int rc = 0;

    memset(&ps_req, 0, sizeof(wds_set_default_profile_num_req_msg_v01));
    memset(&ps_resp, 0, sizeof(wds_set_default_profile_num_resp_msg_v01));

    ps_req.profile_identifier.profile_family = WDS_PROFILE_FAMILY_EMBEDDED_V01;
    ps_req.profile_identifier.profile_type = profile_type;
    ps_req.profile_identifier.profile_index = profile_index;

    rc = qmi_client_send_msg_sync(mQmiSvcClient,
                                  QMI_WDS_SET_DEFAULT_PROFILE_NUM_REQ_V01,
                                  (void *)&ps_req,
                                  sizeof(ps_req),
                                  (void *)&ps_resp,
                                  sizeof(ps_resp),
                                  QCRIL_DATA_QMI_TIMEOUT);
    if ( QMI_NO_ERR != rc ||
         QMI_RESULT_SUCCESS_V01 != ps_resp.resp.result) {
      Log::getInstance().d("[WDSModemEndPointModule] Failed to send"
        "QMI_WDS_SET_DEFAULT_PROFILE_NUM_V01 for profile_index:" \
          +std::to_string(ps_req.profile_identifier.profile_index)+"with rc ="+std::to_string(rc));
    } else {
      Log::getInstance().d("[WDSModemEndPointModule] setProfileDefault::Successfully sent"
        "QMI_WDS_SET_DEFAULT_PROFILE_NUM_V01 for profile index =" \
        + std::to_string(ps_req.profile_identifier.profile_index));
    }
  }
} /* WDSModemEndPointModule::setProfileDefault() */

/**
 * @brief findMatchApnName
 * Populates matchingProfile with first matching profile for apnName in
 * searchList
 *
 * @return true on success, false otherwise
 **/
bool WDSModemEndPointModule::findMatchApnName
(
  string apnName,
  vector<profileSettings_t>& searchList,
  profileSettings_t& matchingProfile
)
{
  vector<profileSettings_t>::iterator it;
  for(it = searchList.begin(); it != searchList.end(); ++it) {
    Log::getInstance().d("[Debug]: Looking for apn= " + apnName + \
        " comparing with= " + it->apn_name_or_string);
    string qmiApnName = it->apn_name_or_string;
    //Do case insensitive matching of apn name
    if ((apnName.length() == qmiApnName.length()) &&
        (std::equal(apnName.begin(), apnName.end(), qmiApnName.begin(), []
           (auto a, auto b){return std::tolower(a)==std::tolower(b);}))) {
      matchingProfile = *it;
      return TRUE;
    }
  }
  return FALSE;
} /* WDSModemEndPointModule::findMatchApnName() */

/**
 * @brief getProfilesByTechType
 * Populates queriedProfileList for techType supplied
 *
 * @return 0 on success, -1 otherwise
 **/
int WDSModemEndPointModule::getProfilesByTechType
(
  wds_profile_type_enum_v01 techType,
  vector<profileSettings_t>& queriedProfileList
)
{
  int result = 0;
  if (mQmiSvcClient != nullptr)
  {
    qmi_client_error_type rc;
    wds_get_profile_list_req_msg_v01 pl_req;
    wds_get_profile_list_resp_msg_v01  pl_resp;
    memset(&pl_req, 0, sizeof(pl_req));
    memset(&pl_resp, 0, sizeof(pl_resp));
    do {
      pl_req.profile_type_valid = TRUE;
      pl_req.profile_type = techType;

      rc = qmi_client_send_msg_sync( mQmiSvcClient,
                                     QMI_WDS_GET_PROFILE_LIST_REQ_V01,
                                     (void *)&pl_req,
                                     sizeof(pl_req),
                                     (void *)&pl_resp,
                                     sizeof(pl_resp),
                                     QCRIL_DATA_QMI_TIMEOUT);
      if (QMI_NO_ERR != rc) {
        Log::getInstance().d("[WDSModemEndPointModule]:"
                             "failed to send get profile list request");
        result = -1;
        break;
      } else {
        Log::getInstance().d("[WDSModemEndPointModule]:"
                             " Get profile list response succeeeded");
        Log::getInstance().d("get_profile_list for profile_tech "  +
                             std::to_string(pl_req.profile_type) +
                             "returned profile ids " +
                             std::to_string(pl_resp.profile_list_len));
        for(uint32_t i = 0; i < pl_resp.profile_list_len; i++) {
          wds_get_profile_settings_req_msg_v01 ps_req;
          wds_get_profile_settings_resp_msg_v01 ps_resp;

          memset(&ps_req, 0, sizeof(ps_req));
          memset(&ps_resp, 0, sizeof(ps_req));

          ps_req.profile.profile_type = techType;
          ps_req.profile.profile_index =
            pl_resp.profile_list[i].profile_index;

          rc = qmi_client_send_msg_sync( mQmiSvcClient,
                                         QMI_WDS_GET_PROFILE_SETTINGS_REQ_V01,
                                         (void *)&ps_req,
                                         sizeof(ps_req),
                                         (void *)&ps_resp,
                                         sizeof(ps_resp),
                                         QCRIL_DATA_QMI_TIMEOUT );
          if (QMI_NO_ERR != rc ||
              QMI_RESULT_SUCCESS_V01 != ps_resp.resp.result) {
            Log::getInstance().d("[WDSModemEndPointModule]:"
                           "failed to send get profile settings request");
            result = -1;
            break;
          } else  {
            profileSettings_t pSettings;
            memset(&pSettings, 0, sizeof(pSettings));
            pSettings.profile_type = techType;
            pSettings.profile_index = ps_req.profile.profile_index;
            switch(techType) {
              case WDS_PROFILE_TYPE_3GPP_V01:
                if( ps_resp.apn_name_valid ) {
                  strlcpy(pSettings.apn_name_or_string,
                          ps_resp.apn_name,
                          QMI_WDS_APN_NAME_MAX_V01 + 1);
                }
                queriedProfileList.push_back(pSettings);
                break;
              case WDS_PROFILE_TYPE_3GPP2_V01:
                if( ps_resp.apn_string_valid ) {
                  strlcpy(pSettings.apn_name_or_string,
                          ps_resp.apn_string,
                          QMI_WDS_APN_NAME_MAX_V01 + 1);
                }
                queriedProfileList.push_back(pSettings);
                break;
               default:
                Log::getInstance().d("[WDSModemEndPointModule]:" \
                               "invalid techType in get profile settings response");
                result = -1;
                break;
            }
          } /* queried profile settings correctly */
        } /* examine all profiles */
      } /* list of profiles available */
    } while (0);
  }
  return result;
} /* WDSModemEndPointModule::getProfilesByTechType() */

/**
 * @brief Handler for SetProfileParamsSyncMessage
 *
 * @details This processes the provided profile parameters to ensure
 * they are correctly reflected on the modem. Modem is queried to
 * identify a matching profile (APN name match).
 *
 * @return None
 **/
void WDSModemEndPointModule::handleSetProfileParamsSync(std::shared_ptr<Message> m)
{
  int result = 0;
  bool matchFound3gpp = FALSE;
  bool matchFound3gpp2 = FALSE;
  bool defaultSet3gpp = FALSE;
  Log::getInstance().d("[WDSModemEndPointModule]: handleSetProfileParamsSync : Handling msg = " + m->dump());
  auto msg= std::static_pointer_cast<SetProfileParamsSyncMessage>(m);
  if (msg != NULL)
  {
    do {
      vector<rildata::DataProfileInfo_t> pInfo;
      pInfo = msg->getParams();

      vector<profileSettings_t> profileInfo3gpp;
      vector<profileSettings_t> profileInfo3gpp2;

      if ( 0 != getProfilesByTechType( WDS_PROFILE_TYPE_3GPP_V01,
                                       profileInfo3gpp) ) {
        result = -1;
        Log::getInstance().d("[WDSModemEndPointModule]: failed to get 3gpp profiles");
        break;
      }
      if ( 0 != getProfilesByTechType( WDS_PROFILE_TYPE_3GPP2_V01,
                                       profileInfo3gpp2) ) {
        result = -1;
        Log::getInstance().d("[WDSModemEndPointModule]: failed to get 3gpp2 profiles");
        break;
      }

      vector<rildata::DataProfileInfo_t>::iterator it;
      profileSettings_t matchingProfile3gpp;
      profileSettings_t matchingProfile3gpp2;
      for(it = pInfo.begin(); it != pInfo.end(); ++it)
      {
        memset(&matchingProfile3gpp, 0, sizeof(matchingProfile3gpp));
        memset(&matchingProfile3gpp2, 0, sizeof(matchingProfile3gpp2));
        matchFound3gpp = FALSE;
        matchFound3gpp2 = FALSE;

        switch(it->dataProfileInfoType) {
          case DataProfileInfoType_t::THREE_GPP:
            matchFound3gpp = findMatchApnName(it->apn,
                                              profileInfo3gpp,
                                              matchingProfile3gpp);
            break;
          case DataProfileInfoType_t::THREE_GPP2:
            matchFound3gpp2 = findMatchApnName(it->apn,
                                               profileInfo3gpp2,
                                               matchingProfile3gpp2);
            break;
          case DataProfileInfoType_t::COMMON:
            matchFound3gpp = findMatchApnName(it->apn,
                                              profileInfo3gpp,
                                              matchingProfile3gpp);
            matchFound3gpp2 = findMatchApnName(it->apn,
                                               profileInfo3gpp2,
                                               matchingProfile3gpp2);
            break;
          default:
            break;
        } /* switch */
        Log::getInstance().d("Debug: apn = "+ it->apn + "matchFound3gpp = " \
            +std::to_string(matchFound3gpp) + "matchFound3gpp2 = " \
            +std::to_string(matchFound3gpp2));
        if(matchFound3gpp) {
          if(it->preferred) {
            setProfileDefault
            ( WDS_PROFILE_TYPE_3GPP_V01,
              matchingProfile3gpp.profile_index
            );
            defaultSet3gpp = TRUE;
          } else if (!defaultSet3gpp &&
                     isDefaultProfile((int32_t)it->supportedApnTypesBitmap) ) {
            setProfileDefault
            ( WDS_PROFILE_TYPE_3GPP_V01,
              matchingProfile3gpp.profile_index
            );
            defaultSet3gpp = TRUE;
          }
          setProfileSupportedAPNTypes
          ( WDS_PROFILE_TYPE_3GPP_V01,
            matchingProfile3gpp.profile_index,
            convertToApnTypeMask((int32_t)it->supportedApnTypesBitmap)
          );
        }
        if(matchFound3gpp2) {
          /* no default profiles to be set for 3gpp2 */
          setProfileSupportedAPNTypes
          ( WDS_PROFILE_TYPE_3GPP2_V01,
            matchingProfile3gpp2.profile_index,
            convertToApnTypeMask((int32_t)it->supportedApnTypesBitmap)
          );
        }
      } /* process all profiles */
    } while (0);

    if( result != 0 ) {
      Log::getInstance().d("WDSModemEndPointModule:: profile update failed");
      msg->sendResponse(msg, Message::Callback::Status::FAILURE, nullptr);
    } else {
      Log::getInstance().d("WDSModemEndPointModule:: profile update success");
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS,nullptr);
    }
  } /* msg */
} /* WDSModemEndPointModule::handleSetProfileParamsSync() */

/**
 * @brief Handler for GetCallBringUpCapabilitySyncMessage
 *
 * @details This queries the modem for the call bring up capability
 *
 * @return None
 **/
void WDSModemEndPointModule::handleGetCallBringUpCapabilitySync(std::shared_ptr<Message> m)
{
  Log::getInstance().d("WDSModemEndPointModule::handleGetCallBringUpCapabilitySync()");
  auto msg = std::static_pointer_cast<GetCallBringUpCapabilitySyncMessage>(m);
  if (msg != NULL && mQmiSvcClient != nullptr)
  {
    wds_get_capabilities_req_msg_v01 ps_req;
    wds_get_capabilities_resp_msg_v01 ps_resp;
    auto capabilityMask = std::make_shared<BringUpCapability>();
    *capabilityMask = BRING_UP_LEGACY;

    memset(&ps_req, 0, sizeof(wds_get_capabilities_req_msg_v01));
    memset(&ps_resp, 0, sizeof(wds_get_capabilities_resp_msg_v01));

    ps_req.bring_up_capability_valid = TRUE;
    ps_req.bring_up_capability = TRUE;

    int rc = qmi_client_send_msg_sync(mQmiSvcClient,
                                      QMI_WDS_GET_CAPABILITIES_REQ_V01,
                                      (void *)&ps_req,
                                      sizeof(ps_req),
                                      (void *)&ps_resp,
                                      sizeof(ps_resp),
                                      QCRIL_DATA_QMI_TIMEOUT);
    if ( QMI_NO_ERR != rc ||
         QMI_RESULT_SUCCESS_V01 != ps_resp.resp.result) {
      Log::getInstance().d("[WDSModemEndPointModule] Failure querying bring up"
          " capabilityMask, assume legacy");
      msg->sendResponse(msg, Message::Callback::Status::FAILURE, nullptr);
      return;
    } else if (TRUE == ps_resp.bring_up_capability_valid) {
      if(ps_resp.bring_up_capability & WDS_BRING_UP_BY_APN_TYPE_V01) {
        *capabilityMask |= BRING_UP_APN_TYPE;
      }
      if(ps_resp.bring_up_capability & WDS_BRING_UP_BY_APN_NAME_V01) {
        *capabilityMask |= BRING_UP_APN_NAME;
      }
    }
    msg->sendResponse(msg, Message::Callback::Status::SUCCESS, capabilityMask);
  }
} /* WDSModemEndPointModule::handleSetWDSProfileInfoSync() */

void WDSModemEndPointModule::handleRegisterForKeepAliveMessageSync(std::shared_ptr<Message> m)
{
  Log::getInstance().d("[WDSModemEndPointModule]: handleSetProfileParamsSync : Handling msg = " + m->dump());
  auto msg= std::static_pointer_cast<RegisterForKeepAliveSyncMessage>(m);

  if (mQmiSvcClient != nullptr)
  {
    do {
      wds_indication_register_req_msg_v01 req;
      wds_indication_register_resp_msg_v01 resp;
      int rc;

      memset(&req, 0, sizeof(req));
      memset(&resp, 0, sizeof(resp));

      req.report_ma_keep_alive_status_valid = TRUE;
      req.report_ma_keep_alive_status = msg->getParams();

      rc = qmi_client_send_msg_sync (mQmiSvcClient,
                                     QMI_WDS_INDICATION_REGISTER_REQ_V01,
                                     &req,
                                     sizeof(req),
                                     (void*)&resp,
                                     sizeof(resp),
                                     QCRIL_DATA_QMI_TIMEOUT);

      if (rc != QMI_NO_ERR || resp.resp.result != QMI_RESULT_SUCCESS_V01)
      {
        Log::getInstance().d("qcril_data_qmi_wds_registr_keep_alive_ind(): failed with rc="+ std::to_string(rc) +", qmi_err=" + std::to_string(resp.resp.error));
        break;
      }
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, nullptr);
      return;
    } while(0);
  }
  msg->sendResponse(msg, Message::Callback::Status::FAILURE, nullptr);
}
