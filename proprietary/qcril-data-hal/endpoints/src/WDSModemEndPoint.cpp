/******************************************************************************
#  Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include "WDSModemEndPoint.h"
#include "sync/GetAttachListMessage.h"
#include "sync/SetAttachListSyncMessage.h"
#include "sync/SetProfileParamsSyncMessage.h"
#include "sync/GetCallBringUpCapabilitySyncMessage.h"
#include "sync/GetApnTypesForName.h"
#include "sync/RegisterForKeepAliveSyncMessage.h"
#include <sstream>

using std::to_string;

constexpr const char *WDSModemEndPoint::NAME;

void WDSModemEndPoint::requestSetup(string clientToken,
                                      GenericCallback<string>* callback)
{
  auto shared_setupMsg = std::make_shared<QmiSetupRequest>
        (clientToken, 0, nullptr, callback);
  mModule->dispatch(shared_setupMsg);
}

Message::Callback::Status WDSModemEndPoint::getAttachListSync
(
  std::shared_ptr<std::list<uint16_t>>& attach_list
)
{
  // Allow any QMI IDL API invocation only if we are in OPERATIONAL state.
  if (getState() == ModemEndPoint::State::OPERATIONAL)
  {
    Log::getInstance().d("[WDSModemEndPoint::getAttachListSync] "
      "dispatching message GetAttachListMessage");
    // Note that callback is not required for sync calls.
    auto msg =
        std::make_shared<GetAttachListMessage>(nullptr);
    Message::Callback::Status apiStatus;
    apiStatus = msg->dispatchSync(attach_list);
    std::ostringstream ss;
    ss << "[WDSModemEndPoint::getAttachListSync] status = " << (int) apiStatus;
    Log::getInstance().d(ss.str());
    return apiStatus;
  } else {
    Log::getInstance().d("[WDSModemEndPoint::getAttachListSync]"
      " Failed to send message GetAttachListMessage");
    return Message::Callback::Status::FAILURE;
  }
}

Message::Callback::Status WDSModemEndPoint::setAttachListSync
(
  const std::shared_ptr<std::list<uint16_t>>& attach_list,
  const SetAttachListSyncMessage::AttachListAction action
)
{
  // Allow any QMI IDL API invocation only if we are in OPERATIONAL state.
  if (getState() == ModemEndPoint::State::OPERATIONAL)
  {
    Log::getInstance().d("[WDSModemEndPoint::setAttachListSync]"
      "dispatching message SetAttachListSyncMessage");
    // Note that callback is not required for 'dispatchSync' calls.
    auto msg =
        std::make_shared<SetAttachListSyncMessage>(nullptr);
    if(attach_list)
    {
      msg->setParams(*attach_list, action);
    } else {
      Log::getInstance().d("[WDSModemEndPoint::setAttachListSync]"
      "attach_list is NULL.Returning FAILURE");
      return Message::Callback::Status::FAILURE;
    }

    Message::Callback::Status apiStatus;
    auto r = std::make_shared<int>();
    apiStatus = msg->dispatchSync(r);
    std::ostringstream ss;
    ss << "[WDSModemEndPoint::setAttachListSync] status = " << (int) apiStatus;
    Log::getInstance().d(ss.str());
    return apiStatus;
  } else
  {
    Log::getInstance().d("[WDSModemEndPoint::setAttachListSync]"
      "Failed to send message SetAttachListSyncMessage");
    return Message::Callback::Status::FAILURE;
  }
}

Message::Callback::Status WDSModemEndPoint::getAttachListCapabilitySync
(
  std::shared_ptr<AttachListCap>& cap
)
{
  // Allow any QMI IDL API invocation only if we are in OPERATIONAL state.
  if (getState() == ModemEndPoint::State::OPERATIONAL)
  {
    Log::getInstance().d("[WDSModemEndPoint::getAttachListCapabilitySync]"
      " dispatching message GetAttachListCapabilitySyncMessage");

    // Note that callback is not required for sync calls.
    auto msg = std::make_shared<GetAttachListCapabilitySyncMessage>(nullptr);
    Message::Callback::Status apiStatus;
    apiStatus = msg->dispatchSync(cap);
    std::ostringstream ss;
    ss << "[WDSModemEndPoint::getAttachListCapabilitySync] status = " << (int) apiStatus;
    Log::getInstance().d(ss.str());
    return apiStatus;

  } else
  {
    Log::getInstance().d("[WDSModemEndPoint::getAttachListCapabilitySync]"
      "Failed to send message GetAttachListCapabilitySyncMessage");
    return Message::Callback::Status::FAILURE;
  }
}

Message::Callback::Status WDSModemEndPoint::getApnTypesForName(
    string apnName,
    std::shared_ptr<std::list<int32_t>>& apnTypes
)
{
  // Allow any QMI IDL API invocation only if we are in OPERATIONAL state.
  if (getState() == ModemEndPoint::State::OPERATIONAL)
  {
    Log::getInstance().d("[WDSModemEndPoint::getApnTypesForName] "
      "dispatching message GetApnTypesForName");
    // Note that callback is not required for sync calls.
    auto msg =
        std::make_shared<GetApnTypesForName>();
    Message::Callback::Status apiStatus;
    msg->setParams(apnName);
    apiStatus = msg->dispatchSync(apnTypes);
    std::ostringstream ss;
    ss << "[WDSModemEndPoint::getApnTypesForName] status = " << (int) apiStatus;
    Log::getInstance().d(ss.str());
    return apiStatus;
  } else {
    Log::getInstance().d("[WDSModemEndPoint::getApnTypesForName]"
      " Failed to send message GetApnTypesForName");
    return Message::Callback::Status::FAILURE;
  }
}

/**
 * @brief Posts SetProfileParamsSyncMessage to handle the passed profile
 * parameters
 *
 * @return Success if message is posted succesfully, Failure otherwise
 **/
Message::Callback::Status WDSModemEndPoint::setProfileParamsSync
(
  vector<rildata::DataProfileInfo_t>& pInfo
)
{
  // Allow any QMI IDL API invocation only if we are in OPERATIONAL state.
  if (getState() == ModemEndPoint::State::OPERATIONAL)
  {
    Log::getInstance().d("[WDSModemEndPoint::setProfileParamsSync]"
                         "dispatching message SetProfileParamsSyncMessage");
    // Note that callback is not required for sync calls.
    auto msg = std::make_shared<rildata::SetProfileParamsSyncMessage>(nullptr);
    msg->setParams(pInfo);
    Message::Callback::Status apiStatus;
    auto r = std::make_shared<int>();
    apiStatus = msg->dispatchSync(r);
    std::ostringstream ss;
    ss << "[WDSModemEndPoint::handleSetProfileParamsSync] status = " << (int) apiStatus;
    Log::getInstance().d(ss.str());
    return apiStatus;
  } else {
    Log::getInstance().d("[WDSModemEndPoint::handleSetProfileParamsSync]"
                         "Failed to send message SetProfileParamsSyncMessage");
    return Message::Callback::Status::FAILURE;
  }
} /* WDSModemEndPoint::SetProfileParamsSync */

/**
 * @brief Handler for getWDSProfileClatCapability
 *
 * @details This processes the provided profile parameters to check
 * if profiles supports XLAT or not
 *
 * @return bool
 **/
bool WDSModemEndPoint::getWDSProfileClatCapability
(
  int32_t profileType,
  uint8_t profileIndex
)
{
  // Allow any QMI IDL API invocation only if we are in OPERATIONAL state.
  if (getState() == ModemEndPoint::State::OPERATIONAL)
  {
    Log::getInstance().d("[WDSModemEndPoint::getWDSProfileClatCapability]"
            "dispatching message SetWDSProfileInfoSyncMessage");

    auto msg =
          std::make_shared<SetWDSProfileInfoSyncMessage>(nullptr);

    msg->setParams(profileType, profileIndex);

    auto r = std::make_shared<int>();
    Message::Callback::Status apiStatus = msg->dispatchSync(r);
    std::ostringstream ss;
    ss << "[WDSModemEndPoint::getWDSProfileClatCapability] status = " << (int) apiStatus;
    Log::getInstance().d(ss.str());

    if(apiStatus != Message::Callback::Status::SUCCESS)
    {
      return false;
    }
    return true;
  }
  else
  {
    Log::getInstance().d("[WDSModemEndPoint::getWDSProfileClatCapability]"
            "Failed to send message SetWDSProfileInfoSyncMessage");
    return false;
  }
} /* WDSModemEndPoint::getWDSProfileClatCapability */

/**
 * @brief Posts GetCallBringUpCapabilitySyncMessage to query the modem
 *
 * @return Success if message is posted succesfully, Failure otherwise
 **/
Message::Callback::Status WDSModemEndPoint::getCallBringUpCapabilitySync
(
  std::shared_ptr<BringUpCapability>& callBringUpCapability
)
{
  // Allow any QMI IDL API invocation only if we are in OPERATIONAL state.
  if (getState() == ModemEndPoint::State::OPERATIONAL)
  {
    Log::getInstance().d("[WDSModemEndPoint::getCallBringUpCapabilitySync] "
      "dispatching message GetCallBringUpCapabilitySyncMessage");
    // Note that callback is not required for sync calls.
    auto msg =
        std::make_shared<GetCallBringUpCapabilitySyncMessage>(nullptr);
    Message::Callback::Status apiStatus;
    apiStatus = msg->dispatchSync(callBringUpCapability);
    std::ostringstream ss;
    ss << "[WDSModemEndPoint::getCallBringUpCapabilitySync] status = " << (int) apiStatus;
    Log::getInstance().d(ss.str());
    return apiStatus;
  } else {
    Log::getInstance().d("[WDSModemEndPoint::getCallBringUpCapabilitySync]"
     " Failed to send message GetCallBringUpCapabilitySyncMessage");
    return Message::Callback::Status::FAILURE;
  }
} /* WDSModemEndPoint::getCallBringUpCapabilitySync */

Message::Callback::Status WDSModemEndPoint::registerForKeepAliveInd
(
  bool toRegister
)
{
  if (getState() == ModemEndPoint::State::OPERATIONAL)
  {
    Log::getInstance().d("[WDSModemEndPoint::registerForKeepAliveInd]"
                         "dispatching message RegisterForKeepAliveSyncMessage register" + std::to_string(toRegister));
    // Note that callback is not required for sync calls.
    auto msg = std::make_shared<RegisterForKeepAliveSyncMessage>(nullptr);
    Message::Callback::Status apiStatus;
    auto r = std::make_shared<int>();
    msg->setParams(toRegister);
    apiStatus = msg->dispatchSync(r);
    std::ostringstream ss;
    ss << "[WDSModemEndPoint::RegisterForKeepAliveIndMessage] status = " << (int) apiStatus;
    Log::getInstance().d(ss.str());
    return apiStatus;
  } else {
     Log::getInstance().d("[WDSModemEndPoint::RegisterForKeepAliveIndMessage]"
                           "Failed to send message RegisterForKeepAliveSyncMessage");
     return Message::Callback::Status::FAILURE;
  }
}

Message::Callback::Status WDSModemEndPoint::setDefaultProfileNum(int index)
{
  Log::getInstance().d("[WDSModemEndPoint::setDefaultProfileNum] = "+std::to_string(index));
  /* set supplied profile as default */
  wds_set_default_profile_num_req_msg_v01 ps_req;
  wds_set_default_profile_num_resp_msg_v01 ps_resp;

  memset(&ps_req, 0, sizeof(wds_set_default_profile_num_req_msg_v01));
  memset(&ps_resp, 0, sizeof(wds_set_default_profile_num_resp_msg_v01));
  ps_req.profile_identifier.profile_family = WDS_PROFILE_FAMILY_EMBEDDED_V01;
  ps_req.profile_identifier.profile_type = WDS_PROFILE_TYPE_3GPP_V01;
  ps_req.profile_identifier.profile_index = index;
  auto rc = sendRawSync(QMI_WDS_SET_DEFAULT_PROFILE_NUM_REQ_V01,
                  (void*)&ps_req, sizeof(ps_req),
                  (void*)&ps_resp, sizeof(ps_resp),
                  DEFAULT_SYNC_TIMEOUT);

  if ( QMI_NO_ERR != rc ||
       QMI_RESULT_SUCCESS_V01 != ps_resp.resp.result) {
    Log::getInstance().d("[WDSModemEndPoint] Failed to send"
      "QMI_WDS_SET_DEFAULT_PROFILE_NUM_V01 for profile_index:" \
        +std::to_string(ps_req.profile_identifier.profile_index)+"with rc ="+std::to_string(rc));
    return Message::Callback::Status::FAILURE;
  } else {
    Log::getInstance().d("[WDSModemEndPoint] setProfileDefault::Successfully sent"
      "QMI_WDS_SET_DEFAULT_PROFILE_NUM_V01 for profile index =" \
      + std::to_string(ps_req.profile_identifier.profile_index));
  }

  return Message::Callback::Status::SUCCESS;
}