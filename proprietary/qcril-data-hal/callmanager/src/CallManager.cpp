/******************************************************************************
#  Copyright (c) 2018-2020 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include <sstream>
#include <string.h>
#include "framework/Log.h"
#include "modules/qmi/ModemEndPointFactory.h"
#include "modules/qmi/QmiSetupRequestCallback.h"
#include "UnSolMessages/RadioDataCallListChangeIndMessage.h"
#include "UnSolMessages/IWlanDataCallListChangeIndMessage.h"
#include "UnSolMessages/BearerAllocationUpdateMessage.h"
#include "UnSolMessages/HandoverInformationIndMessage.h"
#include "UnSolMessages/DataCallTimerExpiredMessage.h"
#include "UnSolMessages/DeregisterWdsIndMessage.h"
#include "sync/GetCallBringUpCapabilitySyncMessage.h"
#include "event/CallBringupFallbackMessage.h"

#include "DataModule.h"
#include "CallManager.h"
#include "CallInfo.h"
#include "CallState.h"
#include "qcril_data.h"
#include "DsiWrapper.h"
#include <sstream>

using namespace rildata;
using std::placeholders::_1;
using std::placeholders::_2;

CallManager::CallManager(LocalLogBuffer& setLogBuffer): logBuffer(setLogBuffer) {
  mApAssistMode = false;
  mPartialRetryEnabled = true;
  mSaMode = false;
  mRadioDataAvailable = false;
  mNulBearerReasonAvailable = true;
  mRadioVoiceAvailable = false;
  mMaxPartialRetryTimeout = DEFAULT_MAX_PARTIAL_RETRY_TIMEOUT;
  mReportPhysicalChannelConfig = false;
  mReportLinkActiveStateChange = false;
  toggleBearerUpdateToDataConnectionService = false;
  mPhysicalChannelInfoValid = false;
  availableRadioDataService.clear();
  mPhysicalConfigStructInfo.clear();
}

CallManager::~CallManager() {
  while (!pCallInfoList.empty()) {
    pCallInfoList.erase(pCallInfoList.begin());
  }
  #ifdef QMI_RIL_UTF
  mReportPhysicalChannelConfig = false;
  #endif
  cleanUpAllBearerAllocation();
  availableRadioDataService.clear();
  prevDefaultProfileId = QDP_PROFILE_ID_INVALID;
  currDefaultProfileId = QDP_PROFILE_ID_INVALID;
}

void CallManager::init(bool apAssistMode, bool partialRetry, unsigned long maxPartialRetryTimeout)
{
  Log::getInstance().d("[CallManager]: init");
  mApAssistMode = apAssistMode;
  mPartialRetryEnabled = partialRetry;
  mMaxPartialRetryTimeout = maxPartialRetryTimeout;
  mCallBringupCapability = BRING_UP_LEGACY;
  memset(availableCallIds, true, sizeof(availableCallIds));
  while (!pCallInfoList.empty()) {
    pCallInfoList.erase(pCallInfoList.begin());
  }

  wds_endpoint = ModemEndPointFactory<WDSModemEndPoint>::getInstance().buildEndPoint();
  dsd_endpoint = ModemEndPointFactory<DSDModemEndPoint>::getInstance().buildEndPoint();

  if(wds_endpoint) {
    auto cap = std::make_shared<BringUpCapability>();
    Message::Callback::Status status = wds_endpoint->getCallBringUpCapabilitySync(cap);
    if (status != Message::Callback::Status::SUCCESS) {
      Log::getInstance().d("[CallManager] failed to get call bringup capability, result = "
        + std::to_string((int) status));
    } else if(mApAssistMode){ /* bringup by type requires Ap-assist mode*/
      mCallBringupCapability = *cap;
    }
  }
  Log::getInstance().d("[CallManager]: Using call bringup capability = " + \
      std::to_string((int)mCallBringupCapability));
}
std::string CallManager::convertToString(const RatType_t& rat)
{
  switch (rat) {
    case RatType_t::RAT_UNSPECIFIED:
      return "UNSPECIFIED";
    case RatType_t::RAT_4G:
      return "4G";
    case RatType_t::RAT_5G:
      return "5G";
    case RatType_t::RAT_SPLITED:
      return "SPLIT";
    default:
      return "UNKNOWN";
  }
}

std::string CallManager::convertToString(const BearerInfo_t& bearer)
{
  return "bearer=" + to_string(bearer.bearerId) + ", uplink=" + convertToString(bearer.uplink) +
         ", downlink=" + convertToString(bearer.downlink);
}

dsd_apn_pref_sys_enum_v01 convertToDsdPrefSys(HandoffNetworkType_t network) {
  switch (network) {
    case _eWWAN:
      return DSD_APN_PREF_SYS_WWAN_V01;
    case _eWLAN:
      return DSD_APN_PREF_SYS_WLAN_V01;
    case _eIWLAN:
      return DSD_APN_PREF_SYS_IWLAN_V01;
    default:
      Log::getInstance().d("[CallManager]: invalid preferred sys=" +
                           std::to_string(static_cast<int>(network)));
      return DSD_APN_PREF_SYS_WWAN_V01;
  }
}

HandoffNetworkType_t convertToHandoffNetworkType(dsd_apn_pref_sys_enum_v01 prefSys) {
  switch (prefSys) {
    case DSD_APN_PREF_SYS_WWAN_V01:
      return _eWWAN;
    case DSD_APN_PREF_SYS_WLAN_V01:
      return _eWLAN;
    case DSD_APN_PREF_SYS_IWLAN_V01:
      return _eIWLAN;
    default:
      Log::getInstance().d("[CallManager]: invalid preferred sys=" +
                           std::to_string(static_cast<int>(prefSys)));
      return _eWWAN;
  }
}

void CallManager::dump(string padding, ostream& os)
{
  os << padding << "CallManager:" << endl;
  for (auto it = pCallInfoList.begin(); it != pCallInfoList.end(); it++) {
    CallInfo *callInfo = *it;
    if (callInfo == nullptr) {
      os << padding << "    NULL call:" << endl;
    } else {
      callInfo->dump(padding, os);
    }
  }
  os << padding << "    SAMode:" << std::boolalpha << mSaMode << endl;
  os << padding << "Bearers:" << endl;
  for (auto it = bearerCollection.begin(); it != bearerCollection.end(); it++) {
    os << padding << "    cid=" << it->second.cid << std::boolalpha <<
          ", apn=" << it->second.apn <<
          ", iface=" << it->second.ifaceName << endl;
    for (auto bearer : it->second.bearers) {
      os << padding << "        " << convertToString(bearer) << endl;
    }
  }
}

/**
 * @brief insert a call info into call list
 * @details insert a call info into call list
 *
 * @param pCallinstance - call info object
 * @return true - success for inserting call info into call list
 *         false - failure for inserting call info into call list
 */
bool CallManager::insertCallInfo(CallInfo *pCallinstance) {
  Log::getInstance().d("[CallManager]: insertCallInfo");

  bool ret = false;
  auto result = pCallInfoList.insert(pCallinstance);
  if(result.second) {
    Log::getInstance().d("[CallManager]: call instance insert success");
    ret = true;
  }
  else {
    Log::getInstance().d("[CallManager]: call instance insert failed");
    ret = false;
  }

  return ret;
}


bool CallManager::validateCallInfo(CallInfo *pCallInfoInstance)
{
  std::set<CallInfo *>::iterator it;
  stringstream ss;

  if( pCallInfoInstance == nullptr)
  {
   Log::getInstance().d("[CallManager]: validateCallInfo pCallInfoInstance is null");
   return false;
  }

  ss<< pCallInfoInstance;
  Log::getInstance().d("[CallManager]: Searching for addr = " + ss.str());

  for (it=pCallInfoList.begin(); it!=pCallInfoList.end(); ++it) {
    ss.str( std::string());
    ss.clear();
    ss<< *it;
    Log::getInstance().d("[CallManager]: existing addr = " + ss.str());
    if((*it) == pCallInfoInstance) {
      Log::getInstance().d("[CallManager]: found call info pointer");
      return true;
    }
  }
  Log::getInstance().d("[CallManager]: call info pointer not found");
  return false;
}

/**
 * @brief Find call info instance with apn types
 */
CallInfo * CallManager::findCallInfo(ApnTypes_t apnTypes) {
  Log::getInstance().d("[CallManager]: findCallInfo with apn type = " + std::to_string((int)apnTypes));

  std::set<CallInfo *>::iterator it;
  for (it=pCallInfoList.begin(); it!=pCallInfoList.end(); ++it) {
    if((*it)->getProfileInfo() != nullptr) {
      Log::getInstance().d("[CallManager]: apn types = " + std::to_string((int)(*it)->getProfileInfo()->getApnTypes()));
      if((*it)->getProfileInfo()->getApnTypes() == (uint32_t)apnTypes) {
        Log::getInstance().d("[CallManager]: found call info");
        return *it;
      }
    }
    else
    {
      Log::getInstance().d("[CallManager]: profileInfo is nullptr");
    }
  }
  return nullptr;
}

/**
 * @brief Find call info instance with apn, apn types, and ip type
 */
CallInfo * CallManager::findCallInfo(std::string apn, ApnTypes_t apnTypes, std::string ipType) {
  Log::getInstance().d("[CallManager]: findCallInfo with apn = " + apn + " apn type = " + std::to_string((int)apnTypes) +
                       " ip type = " + ipType);

  std::set<CallInfo *>::iterator it;
  for (it=pCallInfoList.begin(); it!=pCallInfoList.end(); ++it) {
    auto pProfileInfo = (*it)->getProfileInfo();
    if(pProfileInfo != nullptr)
    {
      Log::getInstance().d("[CallManager]:     apn = " + apn + " apn types = " + std::to_string((int)pProfileInfo->getApnTypes()) + " ip type = " + ipType);
    }

    if(pProfileInfo!=nullptr && pProfileInfo->getApn() == apn &&
      pProfileInfo->getApnTypes() == (uint32_t)apnTypes &&
      pProfileInfo->getProtocol() == ipType) {
      Log::getInstance().d("[CallManager]: found call info");

      return *it;
    }
  }

  return nullptr;
}

/**
 * @brief Find call info instance with apn
 * @details Find call info instance with apn
 *
 * @param apn [description]
 * @return Call info instance if found, or nullptr if not found
 */
CallInfo * CallManager::findCallInfo(std::string apn) {
  Log::getInstance().d("[CallManager]: findCallInfo with apn = " + apn);

  std::set<CallInfo *>::iterator it;
  for (it=pCallInfoList.begin(); it!=pCallInfoList.end(); ++it) {
    if((*it)->getProfileInfo() == nullptr)
      continue;
    string callApn = (*it)->getProfileInfo()->getApn();
    Log::getInstance().d("[CallManager]: apn = " + callApn);

    //Do case insensitive matching of apn name
    if ((apn.length() == callApn.length()) &&
        (std::equal(apn.begin(), apn.end(), callApn.begin(), []
           (auto a, auto b){return std::tolower(a)==std::tolower(b);}))) {
      Log::getInstance().d("[CallManager]: found call info");

      return *it;
    }
  }

  return nullptr;
}

/**
 * @brief Find call info instance with deviceName
 * @details Find call info instance with deviceName
 *
 * @param deviceName [description]
 * @return Call info instance if found, or nullptr if not found
 */
CallInfo * CallManager::findCallInfoFromDeviceName(std::string deviceName) {
  Log::getInstance().d("[CallManager]: findCallInfo with deviceName= " + deviceName);

  std::set<CallInfo *>::iterator it;
  for (it=pCallInfoList.begin(); it!=pCallInfoList.end(); ++it) {
    string callDevName = (*it)->getDeviceName();
    Log::getInstance().d("[CallManager]: devName = " + callDevName);

    //Do case insensitive matching of device name
    if ((deviceName.length() == callDevName.length()) &&
        (std::equal(deviceName.begin(), deviceName.end(), callDevName.begin(), []
           (auto a, auto b){return std::tolower(a)==std::tolower(b);}))) {
      Log::getInstance().d("[CallManager]: found call info");

      return *it;
    }
  }

  return nullptr;
}


/**
 * @brief Find call info instance with cid
 * @details Find call info instance with cid
 *
 * @param cid [description]
 * @return Call info instance if found, or nullptr if not found
 */
CallInfo * CallManager::findCallInfo(int cid) {
  Log::getInstance().d("[CallManager]: findCallInfo with cid = " + std::to_string(cid));

  std::set<CallInfo *>::iterator it;
  for (it=pCallInfoList.begin(); it!=pCallInfoList.end(); ++it) {
    Log::getInstance().d("[CallManager]: findCallInfo with cid = " + std::to_string((*it)->getCid()));
    if(cid == (*it)->getCid()) {
      Log::getInstance().d("[CallManager]: found call info");
      return *it;
    }
  }

  return nullptr;
}


string CallManager::getApnByCid(int cid) {
  Log::getInstance().d("[CallManager]: find apn name with cid = " + std::to_string(cid));

  std::set<CallInfo *>::iterator it;
  for (it=pCallInfoList.begin(); it!=pCallInfoList.end(); ++it) {
    if(cid == (*it)->getCid()) {
      Log::getInstance().d("[CallManager]: found cid");
      if((*it)->getProfileInfo() != nullptr)
        return ((*it)->getProfileInfo()->getApn());
      else
      {
        Log::getInstance().d("[CallManager]: profileInfo is nullptr");
        return "";
      }
    }
  }
  return "";
}

/**
 * @brief Find emergency call info instance
 */
CallInfo * CallManager::findEmergencyCallInfo() {
  Log::getInstance().d("[CallManager]: findEmergencyCallInfo");

  std::set<CallInfo *>::iterator it;
  for (it=pCallInfoList.begin(); it!=pCallInfoList.end(); ++it) {
    if(((*it)->getProfileInfo() != nullptr) && (*it)->getProfileInfo()->getIsEmergencyCall()) {
      Log::getInstance().d("[CallManager]: found emergency call info");

      return *it;
    }
  }

  return nullptr;
}

/**
 * @brief find available cid
 * @details find available cid from availableCallIds array and set to not available
 * @return available cid
 */
int CallManager::getAvailableCid(void) {
  int cid=-1;
  for (int i=0 ; i<MaxConcurrentDataCalls ; i++) {
    if(availableCallIds[i] == true) {
      availableCallIds[i] = false;
      cid = i;
      break;
    }
  }

  Log::getInstance().d("[CallManager]: available cid = " + std::to_string(cid));
  if (cid < 0) {
    Log::getInstance().d("[CallManager]: No more available cids");
    abort();
  }
  return cid;
}

bool CallManager::isApnTypeInvalid(ApnTypes_t apnTypes) {
  int eBit = static_cast<int>(ApnTypes_t::EMERGENCY);
  int apnTypesBitmap = static_cast<int>(apnTypes);
  // if emergency bit is set, apn types should be configured to ONLY emergency type
  // otherwise, qcrildata will treat it as a misconfiguration
  if (((eBit & apnTypesBitmap) == eBit) &&
      (eBit != apnTypesBitmap)) {
    return true;
  }
  return false;
}

bool CallManager::isApnTypeEmergency(ApnTypes_t apnTypes) {
  return apnTypes == ApnTypes_t::EMERGENCY;
}

void CallManager::handleSetupDataCallRequestMessage(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[CallManager]: handleSetupDataCallRequestMessage");
  std::shared_ptr<SetupDataCallRequestMessage> m = std::static_pointer_cast<SetupDataCallRequestMessage>(msg);

  if (m != nullptr) {
    HandoffNetworkType_t NetType = (m->getAccessNetwork() == AccessNetwork_t::IWLAN)?
                        HandoffNetworkType_t::_eIWLAN:HandoffNetworkType_t::_eWWAN;

    // In AP-assist mode, if setupDataCall request with IWLAN on Radio,
    // it is responded with INVALID ARGUMENT
    if(mApAssistMode &&
      (((NetType==HandoffNetworkType_t::_eWWAN)&&(m->getRequestSource()==RequestSource_t::IWLAN))||
      ((NetType==HandoffNetworkType_t::_eIWLAN)&&(m->getRequestSource()==RequestSource_t::RADIO)))) {
      Log::getInstance().d("network type mismatch to request source "+std::to_string((int)m->getRequestSource()));
      SetupDataCallResponse_t result = {};
      result.respErr = ResponseError_t::INVALID_ARGUMENT;
      result.call.cause = DataCallFailCause_t::ERROR_UNSPECIFIED;
      result.call.suggestedRetryTime = -1;
      auto resp = std::make_shared<SetupDataCallResponse_t>(result);
      m->sendResponse(m, Message::Callback::Status::FAILURE, resp);
      return;
    }
    CallInfo *pCallInstance = nullptr;
    if (isApnTypeEmergency(m->getSupportedApnTypesBitmap())) {
      pCallInstance = findEmergencyCallInfo();
    }
    else {
      pCallInstance = findCallInfo(m->getApn(), m->getSupportedApnTypesBitmap(), m->getProtocol());
      if(pCallInstance != nullptr && pCallInstance->getCallStateMachine() != nullptr &&
        (pCallInstance->getCallStateMachine()->getCurrentState() != _eConnected))
      {
        /*Don't Fail setupdatacall if there is any delay from the framework in sending setupDataCall
          for handoff when modem has already started the HO */
        bool isPendingHandoverSetupdataRequest = true;

        if(pCallInstance->getCallStateMachine()->getCurrentState() ==  _eHandover)
        {
          std::shared_ptr<Message> mmm = getDataModule().getPendingMessageList().find(pCallInstance->getMessageToken());
          if(mmm == nullptr)
          {
            isPendingHandoverSetupdataRequest = false;
            Log::getInstance().d("[CallManager] : There is no pending handover setupdatacall request");
          }
          else {
            std::shared_ptr<SetupDataCallRequestMessage> ms = std::static_pointer_cast<SetupDataCallRequestMessage>(mmm);
            if(ms != nullptr)
            {
              Log::getInstance().d("[CallManager] : Setupdatacall is pending with request reason " + std::to_string((int)ms->getDataRequestReason()));
            }
          }
        }

        if(isPendingHandoverSetupdataRequest)
        {
          Log::getInstance().d("[CallManager]: setupdatacall with the same parameters is requested");
          SetupDataCallResponse_t result = {};
          result.respErr = ResponseError_t::NO_ERROR;
          result.call = {.cause = DataCallFailCause_t::OEM_DCFAILCAUSE_1};
          result.call.suggestedRetryTime = -1;
          auto resp = std::make_shared<SetupDataCallResponse_t>(result);
          m->sendResponse(m, Message::Callback::Status::SUCCESS, resp);
          return;
        }
      }
    }

    Log::getInstance().d("[ getPendingMessageList test ]: insert message = " + msg->get_message_name());
    std::pair<uint16_t, bool> result = getDataModule().getPendingMessageList().insert(msg);
    uint16_t token = result.first;
    Log::getInstance().d("[ getPendingMessageList test ]: insert result token = " + std::to_string((int)token));
    getDataModule().getPendingMessageList().print();

    if (pCallInstance != nullptr) {
      if ((m->getDataRequestReason() == DataRequestReason_t::HANDOVER) &&
         (NetType != pCallInstance->getCurrentNetworkType())) {
        Log::getInstance().d("[CallManager]: request reason handover with current and preferred network types different");
        Log::getInstance().d("Handover for Apn = ["+m->getApn()+
                        "] from ["+std::to_string((int)pCallInstance->getCurrentNetworkType())+
                        "] to ["+std::to_string((int)pCallInstance->getPreferredNetworkType())+"]");
        pCallInstance->setPreferredNetworkType(NetType);
        pCallInstance->serialId = m->getSerial();
        pCallInstance->setMessageToken(token);
        if (pCallInstance->handoverSetupDataCall(
          std::make_shared<std::function<void(CallInfo *, rildata::SetupDataCallResponse_t)>>
          (std::bind(&CallManager::setupDataCallCompletedResp, this, _1, _2)))) {
          // Cancel default message timer and handover timer in HandoverState to be used
          Dispatcher::getInstance().clearTimeoutForMessage(m);
          Log::getInstance().d("[CallManager] clearTimeoutForMessage");

          // release wakelock
          if(m->getAcknowlegeRequestCb() != nullptr) {
            auto cb = *(m->getAcknowlegeRequestCb().get());
            cb(m->getSerial());
          }
        }
      }
      // Request reason NORMAL or NetType == pCallInstance->getCurrentNetworkType()
      else {
        SetupDataCallResponse_t setupDataCallResp = {};
        if ((m->getRequestSource()==RequestSource_t::RADIO &&
            pCallInstance->getCurrentNetworkType()==HandoffNetworkType_t::_eIWLAN) ||
            (m->getRequestSource()==RequestSource_t::IWLAN &&
            pCallInstance->getCurrentNetworkType()==HandoffNetworkType_t::_eWWAN)) {
          Log::getInstance().d("[CallManager]: Request APN connection is already exist on another transport");
          setupDataCallResp.respErr = ResponseError_t::NO_ERROR;
          setupDataCallResp.call.cause = DataCallFailCause_t::OEM_DCFAILCAUSE_9;
          setupDataCallResp.call.suggestedRetryTime = -1;
        }
        else {
          //Respond with success either when request reason is NORMAL or
          //request reason is HANDOVER and prefNetwork == currentNetwork
          Log::getInstance().d("[CallManager]: Respond with existing call because the request reason is "+
                                std::to_string((int)m->getDataRequestReason())+
                                ", or the requested preferred network type is "+
                                std::to_string((int)NetType)+
                                " while current network type is "+
                                std::to_string((int)pCallInstance->getCurrentNetworkType()));

          setupDataCallResp.respErr = ResponseError_t::NO_ERROR;
          setupDataCallResp.call.cause = DataCallFailCause_t::NONE;
          setupDataCallResp.call.suggestedRetryTime = -1;
          setupDataCallResp.call.cid = pCallInstance->getCid();
          setupDataCallResp.call.active = pCallInstance->getActiveState();
          setupDataCallResp.call.type = pCallInstance->getIpType();
          setupDataCallResp.call.ifname = pCallInstance->getDeviceName();
          setupDataCallResp.call.addresses = pCallInstance->getIpAddresses();
          setupDataCallResp.call.dnses = pCallInstance->getDnsAddresses();
          setupDataCallResp.call.gateways = pCallInstance->getGatewayAddresses();
          setupDataCallResp.call.pcscf = pCallInstance->getPcscfAddresses();
          setupDataCallResp.call.mtu = pCallInstance->getMtu();
        }
        pCallInstance->setMessageToken(token);
        setupDataCallCompleted(pCallInstance, setupDataCallResp);
      }

      return;
    }

    pCallInstance = new CallInfo(logBuffer);
    insertCallInfo(pCallInstance);

    pCallInstance->setCid(getAvailableCid());
    pCallInstance->setCurrentNetworkType(NetType);
    pCallInstance->setCallBringUpCapability(mCallBringupCapability);
    pCallInstance->setPreferredNetworkType(NetType);
    pCallInstance->serialId = m->getSerial();
    pCallInstance->setMessageToken(token);
    pCallInstance->setPartialRetryEnabled(mPartialRetryEnabled);
    pCallInstance->setMaxPartialRetryTimeout(mMaxPartialRetryTimeout);
    pCallInstance->setLinkStateChangeReport(mReportLinkActiveStateChange);

    Log::getInstance().d("m->mName = "+m->dump());
    Log::getInstance().d("m->mSerial = ["+std::to_string((int)m->getSerial())+"]");
    Log::getInstance().d("m->mAccessNetwork = ["+std::to_string((int)m->getAccessNetwork())+"]");
    Log::getInstance().d("m->mProfileId = ["+std::to_string((int)m->getProfileId())+"]");
    Log::getInstance().d("m->mApn = ["+m->getApn()+"]");
    Log::getInstance().d("m->mProtocol = ["+m->getProtocol()+"]");
    Log::getInstance().d("m->mRoamingProtocol = ["+m->getRoamingProtocol()+"]");
    Log::getInstance().d("m->mAuthType = ["+std::to_string((int)m->getAuthType())+"]");
    Log::getInstance().d("m->mUsername = ["+m->getUsername()+"]");
    Log::getInstance().d("m->mPassword = ["+m->getPassword()+"]");
    Log::getInstance().d("m->mDataProfileInfoType = ["+std::to_string((int)m->getDataProfileInfoType())+"]");
    Log::getInstance().d("m->mMaxConnsTime = ["+std::to_string((int)m->getMaxConnsTime())+"]");
    Log::getInstance().d("m->mMaxConns = ["+std::to_string((int)m->getMaxConns())+"]");
    Log::getInstance().d("m->mWaitTime = ["+std::to_string((int)m->getWaitTime())+"]");
    Log::getInstance().d("m->mEnableProfile = ["+std::to_string((int)m->getEnableProfile())+"]");
    Log::getInstance().d("m->mSupportedApnTypesBitmap = ["+std::to_string((int)m->getSupportedApnTypesBitmap())+"]");
    Log::getInstance().d("m->mBearerBitmap = ["+std::to_string((int)m->getBearerBitmap())+"]");
    Log::getInstance().d("m->mMtu = ["+std::to_string((int)m->getMtu())+"]");
    Log::getInstance().d("m->mPreferred = ["+std::to_string((int)m->getPreferred())+"]");
    Log::getInstance().d("m->mPersistent = ["+std::to_string((int)m->getPersistent())+"]");
    Log::getInstance().d("m->mRoamingAllowed = ["+std::to_string((int)m->getRoamingAllowed())+"]");
    Log::getInstance().d("m->mReason = ["+std::to_string((int)m->getDataRequestReason())+"]");
    std::vector<std::string> addr = m->getAddresses();
    for (unsigned long i=0 ; i<addr.size(); i++) {
      Log::getInstance().d("m->mAddresses["+std::to_string((int)i)+"] = ["+addr[i]+"]");
    }
    std::vector<std::string> dns = m->getDnses();
    for (unsigned long i=0 ; i<dns.size(); i++) {
      Log::getInstance().d("m->mDnses["+std::to_string((int)i)+"] = ["+dns[i]+"]");
    }

    // create CallSetupProfile and configure
    BaseProfile::params_t params;
    params.profileId = (int32_t)m->getProfileId();
    switch(m->getDataProfileInfoType())
    {
      case DataProfileInfoType_t::THREE_GPP2:
        params.radioTech = QDP_RADIOTECH_3GPP2;
        break;
      case DataProfileInfoType_t::THREE_GPP:
        params.radioTech = QDP_RADIOTECH_3GPP;
        break;
      case DataProfileInfoType_t::COMMON:
        params.radioTech = QDP_RADIOTECH_COMMON;
        break;
    }
    RadioAccessFamily_t radioRAT = RadioAccessFamily_t::UNKNOWN;
    if( !( availableRadioDataService.empty() ) )
    {
      params.apn = m->getApn();
      AvailableRadioDataService_t radioData = availableRadioDataService.front();
      radioRAT = radioData.radioFamily;
      Log::getInstance().d("[CallManager]:: availableRadioDataService = "+std::to_string((int)radioRAT));
      if( DATA_IS_RIL_RADIO_TECH_CDMA_1X_EVDO(radioRAT) )
      {
        params.apn = "";
        Log::getInstance().d("Disregarding RIL APN for CDMA related tech");
      }
    }
    else
    {
      Log::getInstance().d("[CallManager] :: availableRadioDataService is empty");
      params.apn = m->getApn();
    }
    params.username = m->getUsername();
    params.password = m->getPassword();
    params.protocol = m->getProtocol();
    params.roamingProtocol = m->getRoamingProtocol();
    params.apnTypeBitmask = (int32_t)m->getSupportedApnTypesBitmap();
    params.bearerBitmask = (int32_t)m->getBearerBitmap();
    params.mtu = m->getMtu();
    params.preferred = m->getPreferred();
    params.persistent = m->getPersistent();
    params.roamingAllowed = m->getRoamingAllowed();
    params.authType = std::to_string((int)m->getAuthType());
    CallSetupProfile *profile = new CallSetupProfile(params);

    if( profile != NULL )
    {
      Log::getInstance().d("[CallManager] ::Setting CallSetupProfile with availableRadioDataService");
      profile->setAvailableRadioRAT(radioRAT);
    }

    if (isApnTypeInvalid(m->getSupportedApnTypesBitmap())) {
      Log::getInstance().d("[CallManager] Invalid apn configuration");
      SetupDataCallResponse_t result;
      result.respErr = ResponseError_t::INVALID_ARGUMENT;
      result.call = {DataCallFailCause_t::OEM_DCFAILCAUSE_4,
                    -1,
                    pCallInstance->getCid(),
                    pCallInstance->getActiveState(),
                    pCallInstance->getIpType(),
                    pCallInstance->getDeviceName().c_str(),
                    pCallInstance->getIpAddresses(),
                    pCallInstance->getDnsAddresses(),
                    pCallInstance->getGatewayAddresses(),
                    pCallInstance->getPcscfAddresses(),
                    pCallInstance->getMtu()};
      this->setupDataCallCompleted(pCallInstance, result);
      return;
    }

    if (isApnTypeEmergency(m->getSupportedApnTypesBitmap())) {
      if (pCallInstance->setEmergencyProfileInfo(profile)) {
        Log::getInstance().d("[CallManager] Handle emergency data call request");
        pCallInstance->setCallInfoChangedCallback(
          std::make_shared<std::function<void(void)>>
          (std::bind(&CallManager::dataCallListChanged, this)));
        if (pCallInstance->setupDataCall(
          std::make_shared<std::function<void(CallInfo *, rildata::SetupDataCallResponse_t)>>
          (std::bind(&CallManager::setupDataCallCompletedResp, this, _1, _2)))) {
          // Cancel default message timer and setupDataCall timer in ConnectingState to be used
          Dispatcher::getInstance().clearTimeoutForMessage(m);
          Log::getInstance().d("[CallManager] clearTimeoutForMessage");
        }
        else { // setupDataCall is not successful with call state changed to error state
          Log::getInstance().d("[CallManager] setupDataCall failed, clean call instance");
          resetCallInfo(pCallInstance);
        }
        // release wakelock
        if(m->getAcknowlegeRequestCb() != nullptr) {
          auto cb = *(m->getAcknowlegeRequestCb().get());
          cb(m->getSerial());
        }
        return;
      }
    }

    if (m->getRequestSource()==RequestSource_t::RADIO && !mRadioDataAvailable && !mRadioVoiceAvailable && mNulBearerReasonAvailable) {
      Log::getInstance().d("[CallManager]: Reject request because both PS/CS not available");
      SetupDataCallResponse_t result;
      result.respErr = ResponseError_t::CALL_NOT_AVAILABLE;
      result.call = {DataCallFailCause_t::DATA_REGISTRATION_FAIL,
                    -1,
                    pCallInstance->getCid(),
                    pCallInstance->getActiveState(),
                    pCallInstance->getIpType(),
                    pCallInstance->getDeviceName().c_str(),
                    pCallInstance->getIpAddresses(),
                    pCallInstance->getDnsAddresses(),
                    pCallInstance->getGatewayAddresses(),
                    pCallInstance->getPcscfAddresses(),
                    pCallInstance->getMtu()};
      this->setupDataCallCompleted(pCallInstance, result);
      return;
    }

    if (true == pCallInstance->setProfileInfo(profile)) {
      //Need to call DSD_SET_APN_INFO to update APN types for call if in
      //"legacy" mode
      if (!mApAssistMode) {
        if(dsd_endpoint) {
          Message::Callback::Status status = Message::Callback::Status::FAILURE;
          status = dsd_endpoint->setApnInfoSync(params.apn,
                                                params.apnTypeBitmask);
          if (status != Message::Callback::Status::SUCCESS) {
            Log::getInstance().d("[CallManager] SET_APN_INFO request failed,"
                            " result = "+ std::to_string((int) status));
          }
          else {
            Log::getInstance().d("[CallManager] SET_APN_INFO request successful"
                                ", result = "+ std::to_string((int) status));
          }
        }
        else {
          Log::getInstance().d("[CallManager] ERROR dsd_endpoint is NULL.");
        }
      } //end if(!mApAssistMode)

      //Add handover IP addresses if call is being setup for 3rd party handover
      if ((m->getDataRequestReason() == DataRequestReason_t::HANDOVER)) {
        Log::getInstance().d("[CallManager] 3rd party handover from AP.");
        pCallInstance->setThirdPartyHOParams(addr);
      }
      pCallInstance->setCallInfoChangedCallback(
        std::make_shared<std::function<void(void)>>
        (std::bind(&CallManager::dataCallListChanged, this)));
      if (pCallInstance->setupDataCall(
        std::make_shared<std::function<void(CallInfo *, rildata::SetupDataCallResponse_t)>>
        (std::bind(&CallManager::setupDataCallCompletedResp, this, _1, _2)))) {
        // Cancel default message timer and setupDataCall timer in ConnectingState to be used
        Dispatcher::getInstance().clearTimeoutForMessage(m);
        Log::getInstance().d("[CallManager] clearTimeoutForMessage");
      }
      else { // setupDataCall is not successful with call state changed to error state
        Log::getInstance().d("[CallManager] setupDataCall failed, clean call instance");
        resetCallInfo(pCallInstance);
      }
      // release wakelock
      if(m->getAcknowlegeRequestCb() != nullptr) {
        auto cb = *(m->getAcknowlegeRequestCb().get());
        cb(m->getSerial());
      }
    }
    else {
      SetupDataCallResponse_t result;
      result.respErr = ResponseError_t::INTERNAL_ERROR;
      result.call = {DataCallFailCause_t::OEM_DCFAILCAUSE_4,
                    -1,
                    pCallInstance->getCid(),
                    pCallInstance->getActiveState(),
                    pCallInstance->getIpType(),
                    pCallInstance->getDeviceName().c_str(),
                    pCallInstance->getIpAddresses(),
                    pCallInstance->getDnsAddresses(),
                    pCallInstance->getGatewayAddresses(),
                    pCallInstance->getPcscfAddresses(),
                    pCallInstance->getMtu()};
      this->setupDataCallCompleted(pCallInstance, result);
    }
  }
  else { // (m == nullptr)
    Log::getInstance().d("[CallManager]: Improper SetupDataCallRequestMessage received");
  }
}

void CallManager::handleDeactivateDataCallRequestMessage(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[CallManager]: handleDeactivateDataCallRequestMessage");
  std::shared_ptr<DeactivateDataCallRequestMessage> m = std::static_pointer_cast<DeactivateDataCallRequestMessage>(msg);

  if (m != nullptr) {
    CallInfo *pCallInstance = findCallInfo((int)m->getCid());

    if (pCallInstance != nullptr && pCallInstance->getCallStateMachine() != nullptr) {
      Log::getInstance().d("[ getPendingMessageList test ]: insert message = " + msg->get_message_name());
      std::pair<uint16_t, bool> result = getDataModule().getPendingMessageList().insert(msg);
      uint16_t token = result.first;
      Log::getInstance().d("[ getPendingMessageList test ]: insert result token = " + std::to_string((int)token));
      getDataModule().getPendingMessageList().print();

      Log::getInstance().d("m->mSerial = ["+std::to_string((int)m->getSerial())+"]");
      Log::getInstance().d("m->mCid = ["+std::to_string((int)m->getCid())+"]");
      Log::getInstance().d("m->mReason = ["+std::to_string((int)m->getDataRequestReason())+"]");
      Dispatcher::getInstance().clearTimeoutForMessage(m);

      // DEACTIVATE_DATA_CALL request while the call is in disconnecting state
      // Respond new DEACTIVATE_DATA_CALL request with failure and wait for
      // the previous DEACTIVATE_DATA_CALL to be completed
      if (pCallInstance->getCallStateMachine()->getCurrentState() == _eDisconnecting) {
        ResponseError_t result = ResponseError_t::CALL_NOT_AVAILABLE;
        auto resp = std::make_shared<ResponseError_t>(result);
        m->sendResponse(msg, Message::Callback::Status::FAILURE, resp);
        if(m->getAcknowlegeRequestCb() != nullptr) {
          auto cb = *(m->getAcknowlegeRequestCb().get());
          cb(m->getSerial());
        }
        return;
      }
      // DEACTIVATE_DATA_CALL request while the call is in Connecting or Handover state
      // Respond the pending SETUP_DATA_CALL request and continue DEACTIVATE_DATA_CALL
      if ((pCallInstance->getCallStateMachine()->getCurrentState() == _eConnecting) ||
         (pCallInstance->getCallStateMachine()->getCurrentState() == _eHandover)) {
        pCallInstance->setupDataCallFailResponse(DataCallFailCause_t::OEM_DCFAILCAUSE_4);
      }

      pCallInstance->setMessageToken(token);
      if (m->getDataRequestReason() == DataRequestReason_t::HANDOVER) {
        pCallInstance->handoverDeactivateDataCall(
         std::make_shared<std::function<void(CallInfo *, rildata::ResponseError_t)>>
         (std::bind(&CallManager::deactivateDataCallCompleted, this, _1, _2)));
      }
      else {
        pCallInstance->deactivateDataCall(
         std::make_shared<std::function<void(CallInfo *, rildata::ResponseError_t)>>
         (std::bind(&CallManager::deactivateDataCallCompleted, this, _1, _2)));
      }
    }
    else {
      Log::getInstance().d("No active call with given cid");
      ResponseError_t result = ResponseError_t::CALL_NOT_AVAILABLE;
      auto resp = std::make_shared<ResponseError_t>(result);
      m->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
    }

    // release wakelock
    if(m->getAcknowlegeRequestCb() != nullptr) {
    auto cb = *(m->getAcknowlegeRequestCb().get());
    cb(m->getSerial());
    }
  }
  else { // (m == nullptr)
    Log::getInstance().d("[CallManager]: Improper DeactivateDataCallRequestMessage received");
  }
}

void CallManager::setupDataCallCompleted(CallInfo *pCallInstance, SetupDataCallResponse_t result)
{
  Log::getInstance().d("[CallManager::setupDataCallCompleted] = "+std::to_string((int)result.respErr));

  getDataModule().getPendingMessageList().print();
  std::shared_ptr<Message> mmm = getDataModule().getPendingMessageList().find(pCallInstance->getMessageToken());
  if(mmm!=nullptr) {
    Log::getInstance().d("[ getPendingMessageList test ]: found message = " + mmm->get_message_name());
    std::shared_ptr<SetupDataCallRequestMessage> ms = std::static_pointer_cast<SetupDataCallRequestMessage>(mmm);
    auto resp = std::make_shared<SetupDataCallResponse_t>(result);
    if(ms != nullptr) {
      logBuffer.addLogWithTimestamp("[CallManager]: " + std::to_string(ms->getSerial()) + "< setupDataCallResponse resp=" +
                                    std::to_string((int)result.respErr) + ", cause=" +
                                    std::to_string((int)result.call.cause));
      ms->sendResponse(mmm, Message::Callback::Status::SUCCESS, resp);
    }
    else { // (ms == nullptr)
      Log::getInstance().d("[CallManager]: Corrupted SetupDataCallRequestMessage - not able to send response");
    }
    getDataModule().getPendingMessageList().erase(mmm);
    getDataModule().getPendingMessageList().print();

    if(result.respErr == ResponseError_t::NO_ERROR) {
        pCallInstance->setCallParamsChangedFlag(true);
        dataCallListChanged();
    } else {
        Log::getInstance().d("[CallManager]: setupdatacallResp failed");
        resetCallInfo(pCallInstance);

        if( ms != nullptr)
        {
          Log::getInstance().d("[DataModule]: releasing the Wakelock");
          //release wakelock
          if(ms->getAcknowlegeRequestCb() != nullptr) {
            auto cb = *(ms->getAcknowlegeRequestCb().get());
            cb(ms->getSerial());
          }
        }
    }
  }
  else {
    Log::getInstance().d("[ getPendingMessageList test ]: not found");
  }
}


void CallManager::setupDataCallCompletedResp(CallInfo *pCallInstance, SetupDataCallResponse_t result)
{
  Log::getInstance().d("[CallManager::setupDataCallCompletedResp] = "+std::to_string((int)result.respErr));

  getDataModule().getPendingMessageList().print();
  std::shared_ptr<Message> mmm = getDataModule().getPendingMessageList().find(pCallInstance->getMessageToken());
  if(mmm!=nullptr) {
    Log::getInstance().d("[ getPendingMessageList test ]: found message = " + mmm->get_message_name());
    std::shared_ptr<SetupDataCallRequestMessage> ms = std::static_pointer_cast<SetupDataCallRequestMessage>(mmm);
    auto resp = std::make_shared<SetupDataCallResponse_t>(result);
    if(ms != nullptr) {
      logBuffer.addLogWithTimestamp("[CallManager]: " + std::to_string(ms->getSerial()) + "< setupDataCallResponse resp=" +
                                    std::to_string((int)result.respErr) + ", cause=" +
                                    std::to_string((int)result.call.cause));
      ms->sendResponse(mmm, Message::Callback::Status::SUCCESS, resp);
    }
    else { // (ms == nullptr)
      Log::getInstance().d("[CallManager]: Corrupted SetupDataCallRequestMessage - not able to send response");
    }
    getDataModule().getPendingMessageList().erase(mmm);
    getDataModule().getPendingMessageList().print();

    if(result.respErr == ResponseError_t::NO_ERROR) {
        pCallInstance->setCallParamsChangedFlag(true);
        dataCallListChanged();
    } else {
        Log::getInstance().d("[CallManager]: setupdatacallResp failed");
    }
  }
  else {
    Log::getInstance().d("[ getPendingMessageList test ]: not found");
  }
}
void CallManager::deactivateDataCallCompleted(CallInfo *pCallInstance, ResponseError_t result)
{
  Log::getInstance().d("[CallManager::deactivateDataCallCompleted] = "+std::to_string((int)result));

  getDataModule().getPendingMessageList().print();
  std::shared_ptr<Message> mmm = getDataModule().getPendingMessageList().find(pCallInstance->getMessageToken());
  if(mmm!=nullptr) {
    Log::getInstance().d("[ getPendingMessageList test ]: found message = " + mmm->get_message_name());
    std::shared_ptr<DeactivateDataCallRequestMessage> ms = std::static_pointer_cast<DeactivateDataCallRequestMessage>(mmm);
    auto resp = std::make_shared<ResponseError_t>(result);
    if(ms != nullptr) {
      logBuffer.addLogWithTimestamp("[CallManager]: " + std::to_string(ms->getSerial()) + "< deactivateDataCallResponse resp=" +
                                    std::to_string((int)result));
      ms->sendResponse(mmm, Message::Callback::Status::SUCCESS, resp);
    }
    else { // (ms == nullptr)
      Log::getInstance().d("[CallManager]: Corrupted DeactivateDataCallRequestMessage - not able to send response");
    }
    getDataModule().getPendingMessageList().erase(mmm);
    getDataModule().getPendingMessageList().print();

    if(result != ResponseError_t::NO_ERROR)
    {
      Log::getInstance().d("[CallManager]: DeactivatedatacallResp failed");
      resetCallInfo(pCallInstance);

      if( ms != nullptr)
      {
        Log::getInstance().d("[DataModule]: releasing the Wakelock");
        //release wakelock
        if(ms->getAcknowlegeRequestCb() != nullptr) {
          auto cb = *(ms->getAcknowlegeRequestCb().get());
          cb(ms->getSerial());
        }
      }
    } else {
      // In case of handover deactivate, the call instance will not be cleaned, but
      // switching the network type from source to target will still cause DC list change
      if (ms != nullptr && DataRequestReason_t::HANDOVER == ms->getDataRequestReason())
      {
        dataCallListChanged();
      }
    }
  }
  else {
    Log::getInstance().d("[ getPendingMessageList test ]: not found");
  }
}

void CallManager::getRadioDataCallList(vector<DataCallResult_t> &call)
{
  Log::getInstance().d("[CallManager::getRadioDataCallList] from "+ std::to_string((int)pCallInfoList.size()));

  std::set<CallInfo *>::iterator it;
  for (it=pCallInfoList.begin(); it!=pCallInfoList.end(); ++it) {
    if(_eWWAN == ((*it)->getCurrentNetworkType() & _eWWAN)) {
      DataCallResult_t callInst = {
        DataCallFailCause_t::NONE,
        -1,
        (*it)->getCid(),
        (*it)->getActiveState(),
        (*it)->getIpType(),
        (*it)->getDeviceName(),
        (*it)->getIpAddresses(),
        (*it)->getDnsAddresses(),
        (*it)->getGatewayAddresses(),
        (*it)->getPcscfAddresses(),
        (*it)->getMtu()
        };

      Log::getInstance().d("[cid]: = " + std::to_string(callInst.cid));
      if((*it)->getProfileInfo())
      {
        Log::getInstance().d("[apn]: " + (*it)->getProfileInfo()->getApn());
      }
      Log::getInstance().d("[active state]: " + std::to_string(callInst.active));
      Log::getInstance().d("[protocol]: " + callInst.type);
      Log::getInstance().d("[device name]: " + callInst.ifname);
      Log::getInstance().d("[IP addresses]: " + callInst.addresses);
      Log::getInstance().d("[DNS addresses]: " + callInst.dnses);
      Log::getInstance().d("[gateway addresses]: " + callInst.gateways);
      Log::getInstance().d("[pcscf addresses]: " + callInst.pcscf);
      Log::getInstance().d("[mtu]: " + std::to_string(callInst.mtu));

      if((*it)->getCallStateMachine())
      {
        Log::getInstance().d("[state]: " + std::to_string((*it)->getCallStateMachine()->getCurrentState()));
      }

      call.push_back(callInst);
    }
  }

  Log::getInstance().d("[CallManager]: Radio call lists = " + std::to_string((int)call.size()));
}

void CallManager::getIWlanDataCallList(vector<DataCallResult_t> &call)
{
  Log::getInstance().d("[CallManager::getIWlanDataCallList] from "+ std::to_string((int)pCallInfoList.size()));

  std::set<CallInfo *>::iterator it;
  for (it=pCallInfoList.begin(); it!=pCallInfoList.end(); ++it) {
    if(_eIWLAN == ((*it)->getCurrentNetworkType() & _eIWLAN)) {
      DataCallResult_t callInst = {
        DataCallFailCause_t::NONE,
        -1,
        (*it)->getCid(),
        (*it)->getActiveState(),
        (*it)->getIpType(),
        (*it)->getDeviceName(),
        (*it)->getIpAddresses(),
        (*it)->getDnsAddresses(),
        (*it)->getGatewayAddresses(),
        (*it)->getPcscfAddresses(),
        (*it)->getMtu()
        };

      Log::getInstance().d("[cid]: = " + std::to_string(callInst.cid));

      if((*it)->getProfileInfo())
      {
        Log::getInstance().d("[apn]: " + (*it)->getProfileInfo()->getApn());
      }
      Log::getInstance().d("[active state]: " + std::to_string(callInst.active));
      Log::getInstance().d("[protocol]: " + callInst.type);
      Log::getInstance().d("[device name]: " + callInst.ifname);
      Log::getInstance().d("[IP addresses]: " + callInst.addresses);
      Log::getInstance().d("[DNS addresses]: " + callInst.dnses);
      Log::getInstance().d("[gateway addresses]: " + callInst.gateways);
      Log::getInstance().d("[pcscf addresses]: " + callInst.pcscf);
      Log::getInstance().d("[mtu]: " + std::to_string(callInst.mtu));

      if((*it)->getCallStateMachine())
      {
        Log::getInstance().d("[state]: " + std::to_string((*it)->getCallStateMachine()->getCurrentState()));
      }

      call.push_back(callInst);
    }
  }

  Log::getInstance().d("[CallManager]: IWlan call lists = " + std::to_string((int)call.size()));
}

void CallManager::handleRilEventDataCallback(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[CallManager]: handleRilEventDataCallback");

  std::shared_ptr<RilEventDataCallback> m = std::static_pointer_cast<RilEventDataCallback>(msg);
  if( m != nullptr ) {

    CallInfo * pCallInstance = (CallInfo *)m->getEventData()->data;

    if( pCallInstance != nullptr && validateCallInfo(pCallInstance)) {
      Log::getInstance().d("[CallManager]: pCallInstance found");
      int nextState = pCallInstance->handleRilEventDataCallback(msg);
      if(nextState == _eIdle || nextState == _eError) {
      if(pCallInstance->isIntentToChangeInProgress() && pCallInstance->getProfileInfo() != nullptr) {
        Log::getInstance().d("[CallManager]: sending pending ack for intent to change for "+
                              pCallInstance->getProfileInfo()->getApn()+
                              " with pending preferred sys "+
                              std::to_string((int)pCallInstance->getPreferredNetworkType()));
        sendSetApnPreferredSystemRequest( pCallInstance->getProfileInfo()->getApn(),
                                          pCallInstance->getPreferredNetworkType());
      }
      //Defer cleanup of CallInfo if fallback is in progress. This instance will
      //get cleaned up later
      if (!pCallInstance->isFallbackInProgress()) {
        resetCallInfo(pCallInstance);
      }
    }
    }
    else {
      Log::getInstance().d("[CallManager]: pCallInstance  is NULL or does not exist");
    }
  }
  else {
    Log::getInstance().d("[CallManager]: Improper message received = " + msg->dump());
  }
}

void CallManager::cleanCallInfo(CallInfo *pCallInstance) {
  if(pCallInstance == nullptr)
  {
    Log::getInstance().d("[CallManager]: pCallInstance is already NULL");
    return;
  }
  Log::getInstance().d("[CallManager]: cleanCallInfo for cid = " + std::to_string(pCallInstance->getCid()));
  availableCallIds[pCallInstance->getCid()] = true;
  pCallInfoList.erase(pCallInstance);
  delete pCallInstance;
  pCallInstance = nullptr;

  #ifndef QMI_RIL_UTF
  if (mReportPhysicalChannelConfig) {
    updatePhysicalChannelConfigs();
  }
  #endif
  dataCallListChanged();
}

void CallManager::deleteWdsCallEndPoint(int cid, IpFamilyType_t type)
{
  Log::getInstance().d("[CallManager]: deleteWdsCallEndPoint for cid = " + std::to_string(cid));
  CallInfo *pCallInstance = findCallInfo(cid);
  if(pCallInstance != nullptr)
  {
    pCallInstance->deleteWdsCallEndPoint(type);
    if(!pCallInstance->hasWdsCallEndPoints())
    {
      cleanCallInfo(pCallInstance);
    }
  } else {
    Log::getInstance().d("There is no callInfo with cid" + std::to_string(cid));
  }
}

//This ApI will reset all the callInfo params
void CallManager::resetCallInfo(CallInfo *pCallInstance) {
  if(pCallInstance == nullptr)
  {
    Log::getInstance().d("[CallManager]: pCallInstance is already NULL");
    return;
  }
  Log::getInstance().d("[CallManager]: resetCallInfo for cid = " + std::to_string(pCallInstance->getCid()));

  pCallInstance->resetCallInfo();

  if (pCallInstance->hasWdsCallEndPoints()) {
    auto msg = std::make_shared<DeregisterWdsIndMessage>(pCallInstance->getCid());
    msg->broadcast();
  } else {
    cleanCallInfo(pCallInstance);
  }
}

void CallManager::dataCallListChanged() {
  Log::getInstance().d("[CallManager]: dataCallListChanged for the number of calls = " +
                      std::to_string(pCallInfoList.size()));
  bool isRadioDcListChanged = false;
  bool isIWlanDcListChanged = false;
  uint16_t sizeOfradioDcList = radioDcList.size();
  uint16_t sizeOfiwlanDcList = iwlanDcList.size();
  Log::getInstance().d("current size of radioDcList = "+ std::to_string(sizeOfradioDcList));
  Log::getInstance().d("current size of iwlanDcList = "+ std::to_string(sizeOfiwlanDcList));

  radioDcList.clear();
  iwlanDcList.clear();
  std::set<CallInfo *>::iterator it;
  for (it=pCallInfoList.begin(); it!=pCallInfoList.end(); ++it) {

    //Not sending UNSOL for Calls which are not in active state yet
    if(!(*it)->getActiveState())
      continue;

    DataCallResult_t callInst = {
      DataCallFailCause_t::NONE,
      -1,
      (*it)->getCid(),
      (*it)->getActiveState(),
      (*it)->getIpType(),
      (*it)->getDeviceName(),
      (*it)->getIpAddresses(),
      (*it)->getDnsAddresses(),
      (*it)->getGatewayAddresses(),
      (*it)->getPcscfAddresses(),
      (*it)->getMtu()
      };

    Log::getInstance().d("[cid]: = " + std::to_string(callInst.cid));
    if((*it)->getProfileInfo())
    {
      Log::getInstance().d("[apn]: " + (*it)->getProfileInfo()->getApn());
    }

    Log::getInstance().d("[active state]: " + std::to_string(callInst.active));
    Log::getInstance().d("[protocol]: " + callInst.type);
    Log::getInstance().d("[device name]: " + callInst.ifname);
    Log::getInstance().d("[IP addresses]: " + callInst.addresses);
    Log::getInstance().d("[DNS addresses]: " + callInst.dnses);
    Log::getInstance().d("[gateway addresses]: " + callInst.gateways);
    Log::getInstance().d("[pcscf addresses]: " + callInst.pcscf);
    Log::getInstance().d("[mtu]: " + std::to_string(callInst.mtu));

    if((*it)->getCallStateMachine())
    {
      Log::getInstance().d("[state]: " + std::to_string((*it)->getCallStateMachine()->getCurrentState()));
    }

    if(mApAssistMode && (HandoffNetworkType_t::_eIWLAN == ((*it)->getCurrentNetworkType() &
                                                         HandoffNetworkType_t::_eIWLAN))) {
      Log::getInstance().d("cid " + std::to_string(callInst.cid) + " into IWlan call list");
      iwlanDcList.push_back(callInst);
      if((*it)->getCallParamsChangedFlag()) {
        (*it)->setCallParamsChangedFlag(false);
        isIWlanDcListChanged = true;
      }
    }
    if(HandoffNetworkType_t::_eWWAN == ((*it)->getCurrentNetworkType() &
                                       HandoffNetworkType_t::_eWWAN)) {
      Log::getInstance().d("cid " + std::to_string(callInst.cid) + " into Radio call list");
      radioDcList.push_back(callInst);
      if((*it)->getCallParamsChangedFlag()) {
        (*it)->setCallParamsChangedFlag(false);
        isRadioDcListChanged = true;
      }
    }
  }

  if (sizeOfradioDcList != radioDcList.size()) {
    isRadioDcListChanged = true;
  }
  if (sizeOfiwlanDcList != iwlanDcList.size()) {
    isIWlanDcListChanged = true;
  }

  Log::getInstance().d("Radio Call list number " + std::to_string((int)radioDcList.size()));
  if(isRadioDcListChanged) {
    logBuffer.addLogWithTimestamp("[CallManager]: Radio DataCallListChanged for the number of calls = " +
                                  std::to_string((int)radioDcList.size()));
    auto radioMsg = std::make_shared<rildata::RadioDataCallListChangeIndMessage>(radioDcList);
    radioMsg->broadcast();
  }

  if(mApAssistMode && isIWlanDcListChanged) {
    Log::getInstance().d("IWlan Call list number " + std::to_string((int)iwlanDcList.size()));
    logBuffer.addLogWithTimestamp("[CallManager]: IWlan DataCallListChanged for the number of calls = " +
                                  std::to_string((int)iwlanDcList.size()));
    auto iwlanMsg = std::make_shared<rildata::IWlanDataCallListChangeIndMessage>(iwlanDcList);
    iwlanMsg->broadcast();
  }
}

void CallManager::processQmiDsdApnPreferredSystemResultInd(const dsd_ap_asst_apn_pref_sys_result_ind_msg_v01 *ind) {
  if(ind != nullptr && mApAssistMode) {
    Log::getInstance().d("CallManager::processQmiDsdApnPreferredSystemResultInd ENTRY" + std::to_string((int)ind->result_info.result));
    std::string apn(ind->result_info.apn_pref_sys.apn_name);
    CallInfo * pCallInstance = findCallInfo(apn);
    if (pCallInstance != nullptr) {
      HandoffState_t state = HandoffState_t::Failure;
      if(ind->result_info.result == DSD_AP_ASST_APN_PREF_SYS_RESULT_SUCCESS_V01) {
        state = HandoffState_t::PrefSysChangedSuccess;
      } else if (ind->result_info.result == DSD_AP_ASST_APN_PREF_SYS_RESULT_FAILURE_V01) {
        state = HandoffState_t::PrefSysChangedFailure;
      }
      auto placeholder = std::make_shared<HandoverInformationIndMessage>(
                          state, IpFamilyType_t::NotAvailable, pCallInstance->getCid());
      pCallInstance->handoffInformationUpdate(state, placeholder);
    }
  }
}

bool CallManager::sendSetApnPreferredSystemRequest(string apnName, HandoffNetworkType_t prefNetwork) {
  int rc;
  bool ret = true;
  dsd_set_apn_preferred_system_req_msg_v01 req;
  dsd_set_apn_preferred_system_resp_msg_v01 resp;
  memset(&req, 0, sizeof(req));
  memset(&resp, 0, sizeof(resp));
  dsd_apn_pref_sys_enum_v01 prefSys = convertToDsdPrefSys(prefNetwork);
  Log::getInstance().d("[CallManager]: sendSetApnPreferredSystemRequest for apn ="+apnName+
                       ", prefSys ="+std::to_string((int)prefSys));

  strlcpy(req.apn_pref_sys.apn_name, apnName.c_str(), QMI_DSD_MAX_APN_LEN_V01+1);
  req.apn_pref_sys.pref_sys = prefSys;
  req.is_ap_asst_valid = true;
  req.is_ap_asst = true;

  rc = ModemEndPointFactory<DSDModemEndPoint>::getInstance().buildEndPoint()->sendRawSync(
    QMI_DSD_SET_APN_PREFERRED_SYSTEM_REQ_V01,
    &req, sizeof(req),
    &resp, sizeof(resp));

  if (rc!=QMI_NO_ERR) {
    Log::getInstance().d("[CallManager]: Failed to set APN preferred system change = " + std::to_string(rc)
    + " resp.result = " + std::to_string(resp.resp.result)
    + " resp.error = " + std::to_string(resp.resp.error));
    ret = false;
  }

  return ret;
}

void CallManager::processQmiDsdIntentToChangeApnPrefSysInd(const dsd_intent_to_change_apn_pref_sys_ind_msg_v01 *ind)
{
  Log::getInstance().d("[CallManager]: process intent to change ind");
  if(ind != nullptr && mApAssistMode) {
    Log::getInstance().d("[CallManager]: number of pref systems = "+std::to_string((int)ind->apn_pref_sys_len));
    for (uint32 i=0 ; i<ind->apn_pref_sys_len; i++) {
      string apnName(ind->apn_pref_sys[i].apn_pref_info.apn_name);
      HandoffNetworkType_t prefNetwork = convertToHandoffNetworkType(ind->apn_pref_sys[i].apn_pref_info.pref_sys);
      CallInfo *callInst = findCallInfo(apnName);
      if(callInst == nullptr) {
        // call is not found for apn, send ack to modem
        Log::getInstance().d("[CallManager]: call is not found for apn = "+apnName+
                             ", ack to modem with pref sys = "+std::to_string((int)prefNetwork));
        sendSetApnPreferredSystemRequest(apnName, prefNetwork);
      } else {
        // call is found, and preferred system is same to current system, send ack to modem
        // respond to framework with handover failure
        if (prefNetwork == callInst->getCurrentNetworkType()) {
          Log::getInstance().d("[CallManager]: call is found for apn = "+apnName+
                             ", no pref sys change, just ack to modem with pref sys = "+std::to_string((int)prefNetwork));
          callInst->handleHandoverPreferenceUnchanged();
          sendSetApnPreferredSystemRequest(apnName, prefNetwork);
        }
        else {
          // call is found, and preferred system is different to current system
           //
          callInst->setPreferredNetworkType(prefNetwork);
          // if rat is null bearer and attach in progress is true, QNP will not send
          // pref RAT change to the framework since there is no valid RAT. Hence
          // ack the intent immediately since framework will not do any handoff
          if ((ind->apn_pref_sys[i].apn_pref_info.pref_sys == DSD_APN_PREF_SYS_WWAN_V01) &&
              (ind->apn_pref_sys[i].rat == DSD_SYS_RAT_EX_NULL_BEARER_V01) &&
              (ind->apn_status_valid) &&
              (ind->apn_status[i] & DSD_APN_PREF_SYS_APN_STATUS_ATTACH_IN_PROGRESS_V01)){
            Log::getInstance().d("[CallManager]: rat is null bearer due to attach in progress for apn = "
                                 +apnName + " ack the intent");
            sendSetApnPreferredSystemRequest(apnName, prefNetwork);
          }
          else {
          // Handover is expected from Telephony framework, so mark intent_to_change is in progress
          // the pending ack to be sent when Handover is triggered or call is disconnected
          Log::getInstance().d("[CallManager]: pending ack for intent to change for apn = "+apnName);
          callInst->setIntentToChangeInProgress(true);
        }
      }
    }
  }
  }
}

/**
 * Toggle whether to send bearer allocation updates via HAL
 */
ResponseError_t CallManager::handleToggleBearerAllocationUpdate(bool enable)
{
  Log::getInstance().d("[CallManager]: handleToggleBearerAllocationUpdate = "+std::to_string((int)enable));
  toggleBearerUpdateToDataConnectionService = enable;
  updateAllBearerAllocations();

  return ResponseError_t::NO_ERROR;
}

/**
 * handle modem bearer type update indication
 */
void CallManager::handleDataBearerTypeUpdate(int32_t cid, rildata::BearerInfo_t bearer)
{
  Log::getInstance().d("[CallManager]: handleDataBearerTypeChanged for cid = "
      +std::to_string((int)cid)
      +", bid = "
      +std::to_string((int)bearer.bearerId));

  std::map<int32_t, rildata::AllocatedBearer_t>::iterator it;
  it = bearerCollection.find(cid);
  if (it != bearerCollection.end()) {
    std::vector<rildata::BearerInfo_t>::iterator i;
    for (i=(it->second).bearers.begin() ; i!=(it->second).bearers.end() ; ++i)
    {
      if(i->bearerId == bearer.bearerId) {
          if ((i->uplink != bearer.uplink) || (i->downlink != bearer.downlink)) {
              i->uplink = bearer.uplink;
              i->downlink = bearer.downlink;
          }
          break;
      }
    }
    if (i==(it->second).bearers.end()) {
      // new bearer
      (it->second).bearers.push_back(bearer);
    }
    updateAllBearerAllocations();
  }
  else {
      Log::getInstance().d("[CallManager]: no call for cid = "+std::to_string((int)cid));
  }
}

/**
 * handle modem bearer type update indication
 */
void CallManager::handleDataAllBearerTypeUpdate(rildata::AllocatedBearer_t bearer)
{
    // update all bearer with AllocatedBearer_t bearer
    std::map<int32_t, rildata::AllocatedBearer_t>::iterator it;
    it = bearerCollection.find(bearer.cid);
    if (it != bearerCollection.end()) {
        // existing call id, update bearers
        (it->second).bearers.clear();
        (it->second).bearers.insert((it->second).bearers.end(), bearer.bearers.begin(), bearer.bearers.end());
    }
    else {
        // new call id
        bearerCollection.insert(std::pair<int32_t, AllocatedBearer_t>(bearer.cid, bearer));
    }
    updateAllBearerAllocations();
}

/**
 * handle get bearers for one data connection with cid
 */
AllocatedBearerResult_t CallManager::handleGetBearerAllocation(int32_t cid)
{
    Log::getInstance().d("[CallManager]: handleGetBearerAllocation = "+std::to_string((int)cid));

    AllocatedBearerResult_t result;
    std::map<int32_t, rildata::AllocatedBearer_t>::iterator it;
    it = bearerCollection.find(cid);
    if (it != bearerCollection.end()) {
        result.error = ResponseError_t::NO_ERROR;
        result.connections.push_back(it->second);
    }
    else {
        result.error = ResponseError_t::CALL_NOT_AVAILABLE;
    }

    return result;
}

/**
 * handle get all bearer information request
 */
AllocatedBearerResult_t CallManager::handleGetAllBearerAllocations()
{
    Log::getInstance().d("[CallManager]: handleGetAllBearerAllocations");

    AllocatedBearerResult_t result;
    for (std::map<int32_t, rildata::AllocatedBearer_t>::iterator it=bearerCollection.begin(); it!=bearerCollection.end(); ++it) {
        result.connections.push_back(it->second);
    }
    result.error = ResponseError_t::NO_ERROR;

    return result;
}

/**
 * Updates all bearer type info.
 *
 * If there is at least one client registered for bearer allocation updates
 * through IDataConnection HAL, the bearer list will be sent to the HAL client.
 */
void CallManager::updateAllBearerAllocations()
{
    Log::getInstance().d("[CallManager]: updateAllBearerAllocations");
    if(toggleBearerUpdateToDataConnectionService) {
        // Update to IDataConnnection HAL
        AllocatedBearerResult_t localBearers;
        localBearers.error = ResponseError_t::NO_ERROR;
        for (auto it = bearerCollection.begin(); it != bearerCollection.end(); ++it) {
            localBearers.connections.push_back(it->second);
        }
        std::shared_ptr<AllocatedBearerResult_t> bearers = std::make_shared<AllocatedBearerResult_t>(localBearers);
        auto msg = std::make_shared<BearerAllocationUpdateMessage>(bearers);
        msg->broadcast();
    }

    if (mReportPhysicalChannelConfig) {
      updatePhysicalChannelConfigs();
    }
}

/**
 * clean bearer type in the call
 */
void CallManager::cleanUpBearerAllocation(int32_t cid)
{
    Log::getInstance().d("[CallManager]: cleanUpBearerAllocation for cid = "+std::to_string((int)cid));
    std::map<int32_t, rildata::AllocatedBearer_t>::iterator it;
    it = bearerCollection.find(cid);
    if (it != bearerCollection.end()) {
        bearerCollection.erase(it);
    }
    if(mReportPhysicalChannelConfig) {
      updatePhysicalChannelConfigs();
    }
}

/**
 * clean all bearer type info and update
 */
void CallManager::cleanUpAllBearerAllocation()
{
    Log::getInstance().d("[CallManager]: cleanUpAllBearerAllocation");
    bearerCollection.clear();
    if(mReportPhysicalChannelConfig) {
      updatePhysicalChannelConfigs();
    }
}

bool CallManager::convertQmiDsdToAvailableRadioDataService(dsd_system_status_info_type_v01 sys,
                                                    AvailableRadioDataService_t &radioDataService)
{
  Log::getInstance().d("[CallManager]: convertQmiDsdToAvailableRadioDataService , rat = "+std::to_string((int)sys.rat_value)+
                        ", mask = "+std::to_string((uint64_t)sys.so_mask));
  bool addToAvailableService = true;
  radioDataService.radioFamily = RadioAccessFamily_t::UNKNOWN;
  radioDataService.radioFrequency.range = FrequencyRange_t::LOW;
  radioDataService.radioFrequency.channelNumber = 0;

  switch (sys.rat_value) {
    case DSD_SYS_RAT_EX_3GPP_WCDMA_V01:
    case DSD_SYS_RAT_EX_3GPP_TDSCDMA_V01:
      if((sys.so_mask & QMI_DSD_3GPP_SO_MASK_HSDPAPLUS_V01) ||
        (sys.so_mask & QMI_DSD_3GPP_SO_MASK_DC_HSDPAPLUS_V01) ||
        (sys.so_mask & QMI_DSD_3GPP_SO_MASK_64_QAM_V01)) {
        radioDataService.radioFamily = RadioAccessFamily_t::HSPAP;
      }
      else if(((sys.so_mask & QMI_DSD_3GPP_SO_MASK_HSDPA_V01) && (sys.so_mask & QMI_DSD_3GPP_SO_MASK_HSUPA_V01)) ||
             (sys.so_mask & QMI_DSD_3GPP_SO_MASK_HSPA_V01)) {
        radioDataService.radioFamily = RadioAccessFamily_t::HSPA;
      }
      else if((sys.so_mask & QMI_DSD_3GPP_SO_MASK_HSDPA_V01)) {
        radioDataService.radioFamily = RadioAccessFamily_t::HSDPA;
      }
      else if((sys.so_mask & QMI_DSD_3GPP_SO_MASK_HSUPA_V01)) {
        radioDataService.radioFamily = RadioAccessFamily_t::HSUPA;
      }
      else if((sys.so_mask & QMI_DSD_3GPP_SO_MASK_WCDMA_V01)) {
        radioDataService.radioFamily = RadioAccessFamily_t::UMTS;
      }
      else if(sys.rat_value == DSD_SYS_RAT_EX_3GPP_TDSCDMA_V01) {
        radioDataService.radioFamily = RadioAccessFamily_t::TD_SCDMA;
      }
      break;

    case DSD_SYS_RAT_EX_3GPP_GERAN_V01:
      if((sys.so_mask & QMI_DSD_3GPP_SO_MASK_EDGE_V01)) {
        radioDataService.radioFamily = RadioAccessFamily_t::EDGE;
      }
      else if((sys.so_mask & QMI_DSD_3GPP_SO_MASK_GPRS_V01) ||
              (sys.so_mask & QMI_DSD_3GPP_SO_MASK_GSM_V01)) {
        radioDataService.radioFamily = RadioAccessFamily_t::GPRS;
      }
      break;

    case DSD_SYS_RAT_EX_3GPP_LTE_V01:
      if((sys.so_mask & QMI_DSD_3GPP_SO_MASK_LTE_CA_DL_V01) ||
        (sys.so_mask & QMI_DSD_3GPP_SO_MASK_LTE_CA_UL_V01)) {
        radioDataService.radioFamily = RadioAccessFamily_t::LTE_CA;
      }
      else {
        radioDataService.radioFamily = RadioAccessFamily_t::LTE;
      }
      radioDataService.radioFrequency.range = FrequencyRange_t::MID;
      break;

    case DSD_SYS_RAT_EX_3GPP2_1X_V01:
      if((sys.so_mask & QMI_DSD_3GPP2_SO_MASK_1X_IS2000_REL_A_V01) ||
        (sys.so_mask & QMI_DSD_3GPP2_SO_MASK_1X_IS2000_V01)) {
        radioDataService.radioFamily = RadioAccessFamily_t::ONE_X_RTT;
      }
      else if((sys.so_mask & QMI_DSD_3GPP2_SO_MASK_1X_IS95_V01)) {
        radioDataService.radioFamily = RadioAccessFamily_t::IS95A;
      }
      break;

    case DSD_SYS_RAT_EX_3GPP2_HRPD_V01:
      if((sys.so_mask & QMI_DSD_3GPP2_SO_MASK_HDR_REVB_DPA_V01) ||
        (sys.so_mask & QMI_DSD_3GPP2_SO_MASK_HDR_REVB_MPA_V01) ||
        (sys.so_mask & QMI_DSD_3GPP2_SO_MASK_HDR_REVB_MMPA_V01) ||
        (sys.so_mask & QMI_DSD_3GPP2_SO_MASK_HDR_REVB_EMPA_V01)) {
        radioDataService.radioFamily = RadioAccessFamily_t::EVDO_B;
      }
      else if((sys.so_mask & QMI_DSD_3GPP2_SO_MASK_HDR_REVA_DPA_V01) ||
              (sys.so_mask & QMI_DSD_3GPP2_SO_MASK_HDR_REVA_MPA_V01) ||
              (sys.so_mask & QMI_DSD_3GPP2_SO_MASK_HDR_REVA_EMPA_V01)) {
        radioDataService.radioFamily = RadioAccessFamily_t::EVDO_A;
      }
      else if((sys.so_mask & QMI_DSD_3GPP2_SO_MASK_HDR_REV0_DPA_V01)) {
        radioDataService.radioFamily = RadioAccessFamily_t::EVDO_0;
      }
      break;

    case DSD_SYS_RAT_EX_3GPP2_EHRPD_V01:
      radioDataService.radioFamily = RadioAccessFamily_t::EHRPD;
      break;

    case DSD_SYS_RAT_EX_3GPP_5G_V01:
      radioDataService.radioFamily = RadioAccessFamily_t::NR;
      if(sys.so_mask & QMI_DSD_3GPP_SO_MASK_5G_MMWAVE_V01) {
        radioDataService.radioFrequency.range = FrequencyRange_t::MMWAVE;
      }
      else {
        radioDataService.radioFrequency.range = FrequencyRange_t::HIGH;
      }
      break;

    case DSD_SYS_RAT_EX_3GPP_WLAN_V01:
      //If ApAssistMode is legacy then IWLAN is a valid Data Radio
      if (!mApAssistMode &&
          (sys.so_mask & QMI_DSD_3GPP_SO_MASK_S2B_V01)) {
        radioDataService.radioFamily = RadioAccessFamily_t::WLAN;
      } else {
        addToAvailableService = false;
      }
      break;

    case DSD_SYS_RAT_EX_NULL_BEARER_V01:
    default:
      addToAvailableService = false;
      break;
  }

  return addToAvailableService;
}

void CallManager::processQmiDsdSystemStatusInd(const dsd_system_status_ind_msg_v01 * ind)
{
  Log::getInstance().d("[CallManager]: processQmiDsdSystemStatusInd");

  mRadioDataAvailable = false;
  mRadioVoiceAvailable = false;
  if(ind != nullptr && ind->avail_sys_valid) {
    availableRadioDataService.clear();
    AvailableRadioDataService_t radioService;
    for(uint32_t i=0 ; i<ind->avail_sys_len ; i++) {
      if (convertQmiDsdToAvailableRadioDataService(ind->avail_sys[i], radioService)) {
        Log::getInstance().d("[CallManager]: availableRadioDataService , radioFamily = "+std::to_string((int)radioService.radioFamily)+
                            ", radioFrequency.range = "+std::to_string((int)radioService.radioFrequency.range));
        availableRadioDataService.push_back(radioService);
      }
    }
    if(!availableRadioDataService.empty()) {
      Log::getInstance().d("[CallManager]: Radio Data Service is available");
      mRadioDataAvailable = true;
    }

    if(ind->null_bearer_reason_valid &&
      (ind->null_bearer_reason & DSD_NULL_BEARER_REASON_CS_ONLY_V01)) {
      Log::getInstance().d("[CallManager]: Radio Voice Service is available");
      mRadioVoiceAvailable = true;
    }
    if (ind->avail_sys_valid && ind->avail_sys_len >= 1) {
      mSaMode = ((ind->avail_sys[0].so_mask & QMI_DSD_3GPP_SO_MASK_5G_SA_V01) == QMI_DSD_3GPP_SO_MASK_5G_SA_V01);
    }

    // Older modem doesn't support NULL bearer So allowing Call if NULL bearer reason is unspecified
    if(!ind->null_bearer_reason_valid)
    {
      Log::getInstance().d("Null Bearer reason is not set");
      mNulBearerReasonAvailable = false;
    }
  }

  if (mReportPhysicalChannelConfig) {
    updatePhysicalChannelConfigs();
  }
}

void CallManager::generateCurrentPhysicalChannelConfigs()
{
  Log::getInstance().d("[CallManager]: generateCurrentPhysicalChannelConfigs size = " +
                        std::to_string((int)mPhysicalChannelInfo.size()));

  bool fiveGAvailable = false;
  bool primaryServingValid = false;
  bool secondaryServingValid = false;
  RadioAccessFamily_t ratType = RadioAccessFamily_t::UNKNOWN;
  FrequencyRange_t frequencyRange = FrequencyRange_t::LOW;
  PhysicalConfigStructInfo_t fiveGNsaPrimaryServingInfo;
  PhysicalConfigStructInfo_t fiveGNsaSecondaryServingInfo;
  std::vector<PhysicalConfigStructInfo_t> physicalConfigStructInfo;

  for (auto radioService : availableRadioDataService) {
    if(radioService.radioFamily == RadioAccessFamily_t::NR) {
      fiveGAvailable = true;
      break;
    }
    ratType = radioService.radioFamily;
    frequencyRange = radioService.radioFrequency.range;
  }

  if(fiveGAvailable) {
    for (auto radioService : availableRadioDataService) {
      if(radioService.radioFamily == RadioAccessFamily_t::NR) {
        secondaryServingValid = true;
        fiveGNsaSecondaryServingInfo.status = CellConnectionStatus_t::SECONDARY_SERVING;
        fiveGNsaSecondaryServingInfo.rat = radioService.radioFamily;
        fiveGNsaSecondaryServingInfo.rfInfo.range = radioService.radioFrequency.range;
        fiveGNsaSecondaryServingInfo.rfInfo.channelNumber = 0;
      }
      else {
        primaryServingValid = true;
        fiveGNsaPrimaryServingInfo.status = CellConnectionStatus_t::PRIMARY_SERVING;
        fiveGNsaPrimaryServingInfo.rat = radioService.radioFamily;
        fiveGNsaPrimaryServingInfo.rfInfo.range = radioService.radioFrequency.range;
        fiveGNsaPrimaryServingInfo.rfInfo.channelNumber = 0;
      }
    }

    int cid;
    std::set<CallInfo *>::iterator cIterator;
    std::map<int32_t, rildata::AllocatedBearer_t>::iterator bIterator;
    for (cIterator=pCallInfoList.begin(); cIterator!=pCallInfoList.end(); ++cIterator) {
      cid = (*cIterator)->getCid();
      // find cid from call list
      bIterator = bearerCollection.find(cid);
      if (bIterator != bearerCollection.end()) {
        // check all allocated bearers to cid
        std::vector<rildata::BearerInfo_t>::iterator i;
        for (i=(bIterator->second).bearers.begin() ; i!=(bIterator->second).bearers.end() ; ++i) {
          if (i->downlink == RatType_t::RAT_4G) {
            fiveGNsaPrimaryServingInfo.contextIds.push_back(cid);
          }
          else if (i->downlink == RatType_t::RAT_5G) {
            fiveGNsaSecondaryServingInfo.contextIds.push_back(cid);
          }
          else if (i->downlink == RatType_t::RAT_SPLITED) {
            fiveGNsaPrimaryServingInfo.contextIds.push_back(cid);
            fiveGNsaSecondaryServingInfo.contextIds.push_back(cid);
          }
        }
      }
      // in case QMI WDS BEARER TECH is not supported
      else {
        fiveGNsaPrimaryServingInfo.contextIds.push_back(cid);
      }
    }

    if (primaryServingValid) {
      for (NasPhysChanInfo nasinfo : mPhysicalChannelInfo) {
        if(nasinfo.getStatus() == NasPhysChanInfo::Status::PRIMARY) {
          fiveGNsaPrimaryServingInfo.cellBandwidthDownlink = nasinfo.getBandwidth();
          fiveGNsaPrimaryServingInfo.physicalCellId = nasinfo.getPhysicalCellId();
          physicalConfigStructInfo.push_back(fiveGNsaPrimaryServingInfo);
        }
      }
    }

    if (secondaryServingValid) {
      // cellBandwidthDownlink and physicalCellId are not available for 5G NSA
      fiveGNsaSecondaryServingInfo.cellBandwidthDownlink = 0;
      fiveGNsaSecondaryServingInfo.physicalCellId = 0;
      physicalConfigStructInfo.push_back(fiveGNsaSecondaryServingInfo);
    }
  }
  // No 5G case
  else {
    for (NasPhysChanInfo nasinfo : mPhysicalChannelInfo) {
      PhysicalConfigStructInfo_t servingInfo;
      if(nasinfo.getStatus() == NasPhysChanInfo::Status::PRIMARY) {
        servingInfo.status = CellConnectionStatus_t::PRIMARY_SERVING;
      }
      else {
        servingInfo.status = CellConnectionStatus_t::SECONDARY_SERVING;
      }
      servingInfo.cellBandwidthDownlink = nasinfo.getBandwidth();
      servingInfo.rat = ratType;
      servingInfo.rfInfo.range = frequencyRange;
      servingInfo.rfInfo.channelNumber = 0;
      int cid;
      std::set<CallInfo *>::iterator cIterator;
      for (cIterator=pCallInfoList.begin(); cIterator!=pCallInfoList.end(); ++cIterator) {
        cid = (*cIterator)->getCid();
        servingInfo.contextIds.push_back(cid);
      }
      servingInfo.physicalCellId = nasinfo.getPhysicalCellId();
      physicalConfigStructInfo.push_back(servingInfo);
    }
  }

  if (physicalConfigStructInfoChanged(physicalConfigStructInfo)) {
    mPhysicalConfigStructInfo.swap(physicalConfigStructInfo);
    auto physConfmsg = std::make_shared<rildata::PhysicalConfigStructUpdateMessage>(mPhysicalConfigStructInfo);
    physConfmsg->broadcast();
  }
}

void CallManager::enablePhysChanConfigReporting(bool enable)
{
  Log::getInstance().d("[CallManager]: enabledPhysChanConfigReporting = "+std::to_string(enable));
  mReportPhysicalChannelConfig = enable;
  if(mReportPhysicalChannelConfig) {
    updatePhysicalChannelConfigs();
  }
}

void CallManager::handleNasPhysChanConfigMessage(std::shared_ptr<Message> msg)
{
  std::shared_ptr<NasPhysChanConfigMessage> m = std::static_pointer_cast<NasPhysChanConfigMessage>(msg);
  if(m != nullptr) {
    std::shared_ptr<const std::vector<NasPhysChanInfo>> info = m->getInfo();
    if (info != nullptr) {
      mPhysicalChannelInfoValid = true;
      mPhysicalChannelInfo = *info;
    }
    if (mReportPhysicalChannelConfig) {
      updatePhysicalChannelConfigs();
    }
    else {
      Log::getInstance().d("[CallManager]: PhysicalChannelConfig reporting is nullptr or disabled "+
                            std::to_string((int)mReportPhysicalChannelConfig));
    }
  }
}

void CallManager::handleHandoverInformationIndMessage(std::shared_ptr<Message> msg)
{
  std::shared_ptr<HandoverInformationIndMessage> m = std::static_pointer_cast<HandoverInformationIndMessage>(msg);
  if(m != nullptr) {
    CallInfo *pCallInstance = findCallInfo((int)m->getCid());
    if (pCallInstance != nullptr) {
      pCallInstance->handoffInformationUpdate(m->getState(), m);
    }
  }
}

void CallManager::handleCallBringupFallbackMessage(std::shared_ptr<Message> msg)
{
  std::shared_ptr<CallBringupFallbackMessage> m = std::static_pointer_cast<CallBringupFallbackMessage>(msg);
  if(m != nullptr) {
    CallInfo *pCallInstanceOld = findCallInfo((int)m->getCid());
    if (pCallInstanceOld != nullptr) {
      CallInfo *pCallInstanceNew = new CallInfo(logBuffer);

      pCallInstanceNew->setCid(getAvailableCid());
      pCallInstanceNew->setCurrentNetworkType(pCallInstanceOld->getCurrentNetworkType());
      pCallInstanceNew->setCallBringUpCapability(BRING_UP_LEGACY);
      pCallInstanceNew->setPreferredNetworkType(pCallInstanceOld->getPreferredNetworkType());
      pCallInstanceNew->serialId = pCallInstanceOld->serialId;
      pCallInstanceNew->setMessageToken(pCallInstanceOld->getMessageToken());
      pCallInstanceNew->setPartialRetryEnabled(mPartialRetryEnabled);
      pCallInstanceNew->setMaxPartialRetryTimeout(mMaxPartialRetryTimeout);
      pCallInstanceNew->setLinkStateChangeReport(mReportLinkActiveStateChange);

      //Create a new profile info, since old one will be destroyed with old CallInfo
      CallSetupProfile * pProfileInfoOld = pCallInstanceOld->getProfileInfo();
      if(pProfileInfoOld == nullptr)
      {
        Log::getInstance().d("[CallManager]: Old Profile infor is null therefore returning");
        return;
      }
      BaseProfile::params_t callParams = pProfileInfoOld->getParams();
      CallSetupProfile * pProfileInfoNew = new CallSetupProfile(callParams);
      pProfileInfoNew->setUmtsProfileId(pProfileInfoOld->getUmtsProfileId());
      pProfileInfoNew->setCdmaProfileId(pProfileInfoOld->getCdmaProfileId());

      pCallInstanceNew->setProfileInfo(pProfileInfoNew);

      if (pCallInstanceOld->isThirdPartyHOParamsSet()) {
        pCallInstanceNew->setThirdPartyHOParams(pCallInstanceOld->getThirdPartyHOParams());
      }

      resetCallInfo(pCallInstanceOld);
      insertCallInfo(pCallInstanceNew);

      pCallInstanceNew->setCallInfoChangedCallback(
        std::make_shared<std::function<void(void)>>
        (std::bind(&CallManager::dataCallListChanged, this)));
      if (pCallInstanceNew->setupDataCall(
        std::make_shared<std::function<void(CallInfo *, rildata::SetupDataCallResponse_t)>>
        (std::bind(&CallManager::setupDataCallCompletedResp, this, _1, _2)))) {
        Log::getInstance().d("[CallManager]: Fallback data call started successfully");
      }
    }
  }
}

void CallManager::handleCallManagerEventMessage(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[CallManager]: Handling msg = " + msg->dump());
  std::shared_ptr<CallManagerEventMessage> m = std::static_pointer_cast<CallManagerEventMessage>(msg);
  if(m != nullptr) {
    CallManagerEvent_t evt = m->getCallManagerEvent();
    Log::getInstance().d("[CallManager]: cid = " + std::to_string((int)evt.cid));
    Log::getInstance().d("[CallManager]: type = " + std::to_string((int)evt.type));
    CallInfo *pCallInstance = findCallInfo(evt.cid);
    if (pCallInstance != nullptr) {
      switch(evt.type) {
        case _eCallManagerEventPcscfAddressChanged:
        pCallInstance->processPcscfAddressChangedEvent();
        dataCallListChanged();
        break;

        case _eCallManagerEventLinkActive:
        if(pCallInstance->getActiveState()!=_eActivePhysicalLinkUp) {
          Log::getInstance().d("link active state changed to up");
          pCallInstance->setLinkActiveState(_eActivePhysicalLinkUp);
          if (mReportLinkActiveStateChange) {
            pCallInstance->setCallParamsChangedFlag(true);
            dataCallListChanged();
          }
          else {
            Log::getInstance().d("link active state report disabled");
          }
        }
        break;

        case _eCallManagerEventLinkDormant:
        if(pCallInstance->getActiveState()!=_eActivePhysicalLinkDown) {
          Log::getInstance().d("link active state changed to down");
          pCallInstance->setLinkActiveState(_eActivePhysicalLinkDown);
          if (mReportLinkActiveStateChange) {
            pCallInstance->setCallParamsChangedFlag(true);
            dataCallListChanged();
          }
          else {
            Log::getInstance().d("link active state report disabled");
          }
        }
        break;

        default:
        break;
      }
    }
  }
}

void CallManager::handleHandoverTimeout(int cid)
{
  CallInfo *pCallInstance = findCallInfo(cid);
  if (pCallInstance != nullptr) {
    pCallInstance->handleHandoverTimeout();
  } else {
    Log::getInstance().d("[CallManager]: handleHandoverTimeout call is null");
  }
}

void CallManager::handleDeactivateDataCallTimeout(int cid)
{
  CallInfo *pCallInstance = findCallInfo(cid);
  if(pCallInstance != nullptr) {
    if(pCallInstance->getDsiHandle() != nullptr)
    {
      //releasing dsi_hndl to avoid further dsi_event
      Log::getInstance().d("[CallManager]: releasing dsi handle");
      dsi_rel_data_srvc_hndl(pCallInstance->getDsiHandle());
      pCallInstance->setDsiHandle(nullptr);
    }
    pCallInstance->handleDeactivateDataCallTimeout();
    resetCallInfo(pCallInstance);
  } else {
    Log::getInstance().d("[CallManager]: handleDeactivateDataCallTimeout call is null");
  }
}

void CallManager::handleSetupDataCallTimeout(int cid)
{
  CallInfo *pCallInstance = findCallInfo(cid);
  if (pCallInstance != nullptr) {
    if(pCallInstance->getDsiHandle() != nullptr)
    {
      //releasing dsi_hndl to avoid further dsi_event
      Log::getInstance().d("[CallManager]: releasing dsi handle");
      dsi_rel_data_srvc_hndl(pCallInstance->getDsiHandle());
      pCallInstance->setDsiHandle(nullptr);
    }
    pCallInstance->handleSetupDataCallTimeout();
    resetCallInfo(pCallInstance);
  } else {
    Log::getInstance().d("[CallManager]: handleSetupDataCallTimeout call is null");
  }
}

void CallManager::handlePartialRetryTimeout(int cid)
{
  CallInfo *pCallInstance = findCallInfo(cid);
  if (pCallInstance != nullptr) {
    pCallInstance->handlePartialRetryTimeout();
  } else {
    Log::getInstance().d("[CallManager]: handlePartialRetryTimeout call is null");
  }
}

void CallManager::handlePartialRetryResponseTimeout(int cid)
{
  CallInfo *pCallInstance = findCallInfo(cid);
  if (pCallInstance != nullptr) {
    pCallInstance->handlePartialRetryResponseTimeout();
  } else {
    Log::getInstance().d("[CallManager]: handlePartialRetryResponseTimeout call is null");
  }
}

void CallManager::generateCurrentPhysicalChannelConfigsFromBearers()
{
    Log::getInstance().d("[CallManager]: generateCurrentPhysicalChannelConfigsFromBearers size = " +
                          std::to_string((int)mPhysicalChannelInfo.size()));
    bool fiveGAvailable = false;
    RadioAccessFamily_t ratType = RadioAccessFamily_t::UNKNOWN;
    FrequencyRange_t frequencyRange = FrequencyRange_t::LOW;
    std::vector<PhysicalConfigStructInfo_t> physicalConfigStructInfo;

    for (auto radioService : availableRadioDataService) {
      if(radioService.radioFamily == RadioAccessFamily_t::NR) {
        fiveGAvailable = true;
        break;
      }
      ratType = radioService.radioFamily;
      frequencyRange = radioService.radioFrequency.range;
    }

    if(fiveGAvailable) {
      PhysicalConfigStructInfo_t fiveGNsaPrimaryServingInfo = {};
      bool primaryServingValid = false;
      for (auto radioService : availableRadioDataService) {
        if(radioService.radioFamily == RadioAccessFamily_t::NR) {
          primaryServingValid = true;
          fiveGNsaPrimaryServingInfo.status = CellConnectionStatus_t::PRIMARY_SERVING;
          fiveGNsaPrimaryServingInfo.rat = radioService.radioFamily;
          fiveGNsaPrimaryServingInfo.rfInfo.range = radioService.radioFrequency.range;
          fiveGNsaPrimaryServingInfo.rfInfo.channelNumber = 0;
        }
      }

      for (auto cIterator=pCallInfoList.begin(); cIterator!=pCallInfoList.end(); ++cIterator) {
        int cid = (*cIterator)->getCid();
        // find cid from call list
        auto bIterator = bearerCollection.find(cid);
        if (bIterator != bearerCollection.end()) {
          // check all allocated bearers to cid
          std::vector<rildata::BearerInfo_t>::iterator i;
          for (i=(bIterator->second).bearers.begin() ; i!=(bIterator->second).bearers.end() ; ++i) {
            if (i->downlink == RatType_t::RAT_5G) {
              fiveGNsaPrimaryServingInfo.contextIds.push_back(cid);
            }
          }
        }
      }
      physicalConfigStructInfo.push_back(fiveGNsaPrimaryServingInfo);
    }
    // No 5G case
    else {
      PhysicalConfigStructInfo_t primaryServingInfo = {.status = CellConnectionStatus_t::PRIMARY_SERVING};
      PhysicalConfigStructInfo_t secondaryServingInfo = {.status = CellConnectionStatus_t::SECONDARY_SERVING};
      for (auto cIterator=pCallInfoList.begin(); cIterator!=pCallInfoList.end(); ++cIterator) {
        int cid = (*cIterator)->getCid();
        primaryServingInfo.contextIds.push_back(cid);
        secondaryServingInfo.contextIds.push_back(cid);
      }
      physicalConfigStructInfo.push_back(primaryServingInfo);
      physicalConfigStructInfo.push_back(secondaryServingInfo);
    }

    if (physicalConfigStructInfoChanged(physicalConfigStructInfo)) {
      mPhysicalConfigStructInfo.swap(physicalConfigStructInfo);
      auto physConfmsg = std::make_shared<rildata::PhysicalConfigStructUpdateMessage>(mPhysicalConfigStructInfo);
      physConfmsg->broadcast();
    }
}

bool CallManager::physicalConfigStructInfoChanged(const std::vector<PhysicalConfigStructInfo_t>& configs) {
  if (configs.size() != mPhysicalConfigStructInfo.size()) {
    return true;
  }
  return !std::equal(mPhysicalConfigStructInfo.begin(), mPhysicalConfigStructInfo.end(), configs.begin(),
    [](const PhysicalConfigStructInfo_t& oldConfigs, const PhysicalConfigStructInfo_t& newConfigs) {
      if (oldConfigs.status != newConfigs.status ||
          oldConfigs.cellBandwidthDownlink != newConfigs.cellBandwidthDownlink ||
          oldConfigs.rat != newConfigs.rat ||
          oldConfigs.rfInfo.range != newConfigs.rfInfo.range ||
          oldConfigs.physicalCellId != newConfigs.physicalCellId ||
          oldConfigs.contextIds.size() != newConfigs.contextIds.size()) {
        return false;
      }

      return std::equal(oldConfigs.contextIds.begin(), oldConfigs.contextIds.end(), newConfigs.contextIds.begin());
  });
}

void CallManager::updatePhysicalChannelConfigs()
{
  Log::getInstance().d("[CallManager]: updatePhysicalChannelConfigs");
  if (mSaMode) {
    generateCurrentPhysicalChannelConfigsFromBearers();
  } else {
    if (mPhysicalChannelInfoValid) {
      generateCurrentPhysicalChannelConfigs();
    } else {
      auto nasGetPhyChanConfigMsg = std::make_shared<NasGetPhyChanConfigMessage>();
      if (nasGetPhyChanConfigMsg != nullptr) {
        GenericCallback<std::vector<NasPhysChanInfo>> cb([](std::shared_ptr<Message> nasGetPhyChanConfigMsg,
                                                      Message::Callback::Status status,
                                                      std::shared_ptr<std::vector<NasPhysChanInfo>> rsp) -> void {
          if (nasGetPhyChanConfigMsg && rsp) {
            if(status == Message::Callback::Status::SUCCESS) {
              Log::getInstance().d("NasPhysChanInfo size = " + std::to_string((int)rsp->size()));
              auto indMsg = std::make_shared<NasPhysChanConfigMessage>(rsp);
              indMsg->broadcast();
            }
            else {
              Log::getInstance().d("[CallManager]: NasGetPhyChanConfigMessage response error");
            }
          }
          if(rsp == nullptr) {
            Log::getInstance().d("[CallManager]: PhysicalChannelConfig reporting is nullptr");
          }
        });
        nasGetPhyChanConfigMsg->setCallback(&cb);
        nasGetPhyChanConfigMsg->dispatch();
      }
      else {
        Log::getInstance().d("[CallManager]: NasGetPhyChanConfigMessage error");
      }
    }
  }
}

int CallManager::toggleLinkActiveStateChangeReport(bool report)
{
  Log::getInstance().d("[CallManager]: toggleLinkActiveStateChangeReport "+std::to_string((int)report));
  if (mReportLinkActiveStateChange != report) {
    mReportLinkActiveStateChange = report;
    std::set<CallInfo *>::iterator it;
    for (it=pCallInfoList.begin(); it!=pCallInfoList.end(); ++it) {
      (*it)->setLinkStateChangeReport(mReportLinkActiveStateChange);
    }
  }

  return 0;
}

bool CallManager::handleGoDormantRequest(void *deviceName, size_t len)
{
  bool status = false;
  if(deviceName != nullptr)
  {
    std::string *devName = static_cast<std::string *>(deviceName);

    if (len > 0) {
      CallInfo *pCallInstance = findCallInfoFromDeviceName(*devName);

      if (pCallInstance) {
        status = pCallInstance->sendDormancyRequest();
      }
    } else {
       std::set<CallInfo *>::iterator it;
       for (it=pCallInfoList.begin(); it!=pCallInfoList.end(); ++it) {
         status = (*it)->sendDormancyRequest();
       }
    }
  } else {
      Log::getInstance().d("[CallManager]: DeviceName received is nullptr");
  }
  return status;
}

void CallManager::handleEmbmsEnableDataRequestMessage(qcril_instance_id_e_type instance_id)
{
  CallInfo *pCallInstance = nullptr;

  pCallInstance = new CallInfo(logBuffer);

  if(pCallInstance == nullptr)
  {
    Log::getInstance().d("embms object received is null");
    return;
  }

  insertCallInfo(pCallInstance);
  pCallInstance->setCid(getAvailableCid());
  pCallInstance->setRilEventId(QCRIL_EVT_QMI_RIL_EMBMS_ENABLE_DATA_REQ);

  BaseProfile::params_t params;
  params.profileId = QDP_PROFILE_ID_INVALID;
  params.apn = "";
  params.protocol = "IP";

  CallSetupProfile *profile = new CallSetupProfile(params);

  if (!pCallInstance->setProfileInfo(profile))
  {
    Log::getInstance().d("[CallManager]: setupEmbmsDataCall failed, clean call instance");
    resetCallInfo(pCallInstance);
    return;
  }

  pCallInstance->setCallInfoChangedCallback(
  std::make_shared<std::function<void(void)>>
  (std::bind(&CallManager::dataCallListChanged, this)));

  if(!pCallInstance->setupEmbmsDataCall(instance_id))
  {
    Log::getInstance().d("[CallManager]: setupEmbmsDataCall failed, clean call instance");
    resetCallInfo(pCallInstance);
  }
}


void CallManager::handleEmbmsActivateTmgi(qcril_request_params_type *req)
{
  Log::getInstance().d("[CallManager]: handleEmbmsActivateTmgi");
  if(req != nullptr)
  {
    embms_activate_tmgi_req_msg_v01 *activate_req = (embms_activate_tmgi_req_msg_v01 *)(req->data);
    int cid = activate_req->call_id;
    CallInfo *pCallInstance = findCallInfo(cid);
    if(pCallInstance != nullptr && pCallInstance->pEmbmsInfo != nullptr)
    {
      dsi_hndl_t hndl = pCallInstance->getDsiHandle();
      pCallInstance->pEmbmsInfo->setDsiHandle(hndl);
      pCallInstance->pEmbmsInfo->setInstanceId(req->instance_id);
      pCallInstance->pEmbmsInfo->activateTmgi(req);
    }
  }
}

void CallManager::handleEmbmsDeactivateTmgi(qcril_request_params_type *req)
{
  Log::getInstance().d("[CallManager]: handleEmbmsDeactivateTmgi");
  if(req != nullptr)
  {
    embms_deactivate_tmgi_req_msg_v01 *deactivate_req = (embms_deactivate_tmgi_req_msg_v01 *)(req->data);
    CallInfo *pCallInstance = findCallInfo(deactivate_req->call_id);
    if(pCallInstance != nullptr && pCallInstance->pEmbmsInfo != nullptr)
    {
      dsi_hndl_t hndl = pCallInstance->getDsiHandle();
      pCallInstance->pEmbmsInfo->setDsiHandle(hndl);
      pCallInstance->pEmbmsInfo->setInstanceId(req->instance_id);
      pCallInstance->pEmbmsInfo->deactivateTmgi(req);
    }
  }
}

void CallManager::handleEmbmsActivateDeactivateTmgi(qcril_request_params_type *req)
{
  Log::getInstance().d("[CallManager]: handleEmbmsActivateDeactivateTmgi");
  if(req != nullptr)
  {
    embms_activate_deactivate_tmgi_req_msg_v01 *act_deactivate_req = (embms_activate_deactivate_tmgi_req_msg_v01 *)(req->data);
    CallInfo *pCallInstance = findCallInfo(act_deactivate_req->call_id);
    if(pCallInstance != nullptr && pCallInstance->pEmbmsInfo != nullptr)
    {
      dsi_hndl_t hndl = pCallInstance->getDsiHandle();
      pCallInstance->pEmbmsInfo->setDsiHandle(hndl);
      pCallInstance->pEmbmsInfo->setInstanceId(req->instance_id);
      pCallInstance->pEmbmsInfo->activateDeactivateTmgi(req);
    }
  }
}

void CallManager::handleEmbmsContentDescUpdate(qcril_request_params_type *req)
{
  Log::getInstance().d("[CallManager]: handleEmbmsContentDescUpdate");
  if(req != nullptr)
  {
    embms_update_content_desc_req_msg_v01 *get_update_req = (embms_update_content_desc_req_msg_v01 *)(req->data);
    CallInfo *pCallInstance = findCallInfo(get_update_req->call_id);
    if(pCallInstance != nullptr && pCallInstance->pEmbmsInfo != nullptr)
    {
      dsi_hndl_t hndl = pCallInstance->getDsiHandle();
      pCallInstance->pEmbmsInfo->setDsiHandle(hndl);
      pCallInstance->pEmbmsInfo->setInstanceId(req->instance_id);
      pCallInstance->pEmbmsInfo->contentDescUpdate(req);
    }
  }
}

void CallManager::handleEmbmsGetActiveTmgi(qcril_request_params_type *req)
{
  Log::getInstance().d("[CallManager]: handleEmbmsGetActiveTmgi");
  if(req != nullptr)
  {
    embms_get_active_tmgi_req_msg_v01 *get_active_req = (embms_get_active_tmgi_req_msg_v01 *)(req->data);
    CallInfo *pCallInstance = findCallInfo(get_active_req->call_id);
    if(pCallInstance != nullptr && pCallInstance->pEmbmsInfo != nullptr)
    {
      dsi_hndl_t hndl = pCallInstance->getDsiHandle();
      pCallInstance->pEmbmsInfo->setDsiHandle(hndl);
      pCallInstance->pEmbmsInfo->setInstanceId(req->instance_id);
      pCallInstance->pEmbmsInfo->getActiveTmgi(req);
    }
  }
}

void CallManager::handleEmbmsGetAvailableTmgi(qcril_request_params_type *req)
{
  Log::getInstance().d("[CallManager]: handleEmbmsGetAvailableTmgi");
  if(req != nullptr)
  {
    embms_get_available_tmgi_req_msg_v01 *get_available_req = (embms_get_available_tmgi_req_msg_v01 *)(req->data);
    CallInfo *pCallInstance = findCallInfo(get_available_req->call_id);
    if(pCallInstance != nullptr && pCallInstance->pEmbmsInfo != nullptr)
    {
      dsi_hndl_t hndl = pCallInstance->getDsiHandle();
      pCallInstance->pEmbmsInfo->setDsiHandle(hndl);
      pCallInstance->pEmbmsInfo->setInstanceId(req->instance_id);
      pCallInstance->pEmbmsInfo->getAvailableTmgi(req);
    }
  }
}

void CallManager::handleEmbmsSendInterestedList(qcril_request_params_type *req)
{
  Log::getInstance().d("[CallManager]: handleEmbmsSendInterestedList");
  if(req != nullptr)
  {
    embms_get_interested_tmgi_list_resp_msg_v01 *interest_list_resp = (embms_get_interested_tmgi_list_resp_msg_v01 *)(req->data);
    CallInfo *pCallInstance = findCallInfo(interest_list_resp->call_id);
    if(pCallInstance != nullptr && pCallInstance->pEmbmsInfo != nullptr)
    {
      dsi_hndl_t hndl = pCallInstance->getDsiHandle();
      pCallInstance->pEmbmsInfo->setDsiHandle(hndl);
      pCallInstance->pEmbmsInfo->setInstanceId(req->instance_id);
      pCallInstance->pEmbmsInfo->sendInterestedList(req);
    }
  }
}

void CallManager::cleanAllCalls()
{
  std::set<CallInfo *>::iterator it;
  for (it=pCallInfoList.begin(); it!=pCallInfoList.end(); ++it) {
    Log::getInstance().d("[CallManager]: cleanAllCalls, reset call instance");
    resetCallInfo(*it);
  }
}
