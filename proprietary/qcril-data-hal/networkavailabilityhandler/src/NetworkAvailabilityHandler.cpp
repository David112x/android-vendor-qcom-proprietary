/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#include "framework/Log.h"

#include "NetworkAvailabilityHandler.h"
#include "QualifiedNetworksChangeIndMessage.h"
#include "GetApnTypesForName.h"
#include "request/SetDataProfileRequestMessage.h"
#include <cctype>
#include <sstream>

using namespace rildata;

void ConsolidatedNetwork_t::dump(string padding, ostream& os) {
  for (const QualifiedNetwork_t& qualNetwork : qualNetworks) {
    os << padding << NetworkAvailabilityHandler::convertToString(qualNetwork) << endl;;
  }
}

NetworkAvailabilityHandler::NetworkAvailabilityHandler(LocalLogBuffer& setLogBuffer): logBuffer(setLogBuffer)
{
  Log::getInstance().d("[NetworkAvailabilityHandler]: constructor");
  logBuffer.addLogWithTimestamp("constructor");
  wds_endpoint = ModemEndPointFactory<WDSModemEndPoint>::getInstance().buildEndPoint();
  dsd_endpoint = ModemEndPointFactory<DSDModemEndPoint>::getInstance().buildEndPoint();
}

NetworkAvailabilityHandler::~NetworkAvailabilityHandler() {
}

void NetworkAvailabilityHandler::processSetDataProfileRequest(std::shared_ptr<Message> msg)
{
  Log::getInstance().d("[NetworkAvailabilityHandler]: process set data profile request");
  logBuffer.addLogWithTimestamp("process set data profile request");
  updateNetworkAvailabilityCache(msg);
  dumpCache();
  Log::getInstance().d("[NetworkAvailabilityHandler]: processSetDataProfileRequest done");
}

void NetworkAvailabilityHandler::processQmiDsdSystemStatusInd(dsd_apn_avail_sys_info_type_v01 * apn_sys, uint32 len)
{
  Log::getInstance().d("[NetworkAvailabilityHandler]: process system status ind");
  updateNetworkAvailabilityCache(apn_sys, len);
  dumpCache();
  Log::getInstance().d("[NetworkAvailabilityHandler]: processQmiDsdSystemStatusInd done");
}

void NetworkAvailabilityHandler::processQmiDsdIntentToChangeApnPrefSysInd(dsd_intent_to_change_apn_pref_sys_ind_msg_v01 *ind_data)
{
  Log::getInstance().d("[NetworkAvailabilityHandler]: process intent to change ind");
  logBuffer.addLogWithTimestamp("process intent to change ind, apn_pref_sys len=" + std::to_string((int)ind_data->apn_pref_sys_len));
  updateNetworkAvailabilityCache(ind_data->apn_pref_sys, ind_data->apn_pref_sys_len);
  dumpCache();
  Log::getInstance().d("[NetworkAvailabilityHandler]: processQmiDsdIntentToChangeApnPrefSysInd done");
}

void NetworkAvailabilityHandler::getQualifiedNetworks(vector<QualifiedNetwork_t> &qualifiedNetworks)
{
  Log::getInstance().d("[NetworkAvailabilityHandler]: process get qualified networks");
  logBuffer.addLogWithTimestamp("process get qualified networks");
  vector<QualifiedNetwork_t> resultList;
  // add qualified networks from the cache that have a pending intent to change
  for (auto it = mNetworkAvailabilityCache.begin(); it != mNetworkAvailabilityCache.end(); it++) {
    ConsolidatedNetwork_t nwInfoList = it->second;
    if (nwInfoList.hasPendingIntentToChange) {
      resultList.insert(resultList.end(), nwInfoList.qualNetworks.begin(), nwInfoList.qualNetworks.end());
    }
  }
  convertResultList(resultList, qualifiedNetworks);
  for (auto qn : qualifiedNetworks) {
    string padded = "[NetworkAvailabilityHandler]: " + convertToString(qn);
    Log::getInstance().d(padded);
  }
  Log::getInstance().d("[NetworkAvailabilityHandler]: Sent qualified networks");
}

void NetworkAvailabilityHandler::dump(string padding, ostream& os)
{
  for (auto it = mNetworkAvailabilityCache.begin(); it != mNetworkAvailabilityCache.end(); it++) {
    os << padding << "apn=" << it->first << " hasPendingIntent=" << std::boolalpha << it->second.hasPendingIntentToChange << endl;
    it->second.dump(padding + "    ", os);
  }
}

void NetworkAvailabilityHandler::convertResultList(vector<QualifiedNetwork_t> &resultList,
                                                   vector<QualifiedNetwork_t> &qualifiedNetworks)
{
  for (QualifiedNetwork_t &qualNetwork : resultList) {
    std::ostringstream ss, networks;
    ApnTypes_t qualApnType = static_cast<ApnTypes_t>(qualNetwork.apnType);
    copy( qualNetwork.network.begin(), qualNetwork.network.end(), ostream_iterator<int>(networks, " "));
    ss << "[NetworkAvailabilityHandler::convertResultList] apnType = " <<
      convertToString(qualApnType) << " networks = " << networks.str();
    Log::getInstance().d(ss.str());

    // Remove IWlan from the network list for default or IA apn types
    if (qualApnType == ApnTypes_t::DEFAULT || qualApnType == ApnTypes_t::IA) {
      auto iwlanNw = find(qualNetwork.network.begin(), qualNetwork.network.end(),
                          static_cast<int32_t>(AccessNetwork_t::IWLAN));
      if (iwlanNw != qualNetwork.network.end()) {
        qualNetwork.network.erase(iwlanNw);
        Log::getInstance().d("Removed IWLAN from network list for APN type " + convertToString(qualApnType));
      }
    }

    if (qualNetwork.network.empty() ||
        qualNetwork.network.front() == static_cast<int32_t>(AccessNetwork_t::UNKNOWN)) {
      Log::getInstance().d("[NetworkAvailabilityHandler] Skipping UNKNOWN or empty network for APN type "
                           + convertToString(qualApnType));
    } else {
      // Find a matching QualifiedNetwork by apn type and override it, preventing duplicate entries
      auto priorEntry = find_if(qualifiedNetworks.begin(), qualifiedNetworks.end(),
          [qualNetwork](const QualifiedNetwork_t &existingNetwork) -> bool {
            return existingNetwork.apnType == qualNetwork.apnType;
          });
      if (priorEntry != qualifiedNetworks.end()) {
        qualifiedNetworks.erase(priorEntry);
        Log::getInstance().d("[NetworkAvailabilityHandler] Override existing entry with APN type " + convertToString(qualApnType));
      }
      qualifiedNetworks.push_back(qualNetwork);
    }
  }
}

void NetworkAvailabilityHandler::updateNetworkAvailabilityCache(std::shared_ptr<Message> msg) {
  auto m = std::static_pointer_cast<SetDataProfileRequestMessage>(msg);
  if (m != nullptr) {
    // Cache all apn types from the SetDataProfileMessage with empty network lists
    mNetworkAvailabilityCache.clear();
    vector<DataProfileInfo_t> profiles = m->getProfiles();
    for (auto profile : profiles) {
      std::vector<QualifiedNetwork_t> qualNetworks;
      std::vector<int32_t> apnTypes = convertToRadioApnTypeList(profile.supportedApnTypesBitmap);
      for (int32_t apnType : apnTypes) {
        qualNetworks.push_back({apnType, vector<int32_t>()});
      }
      ConsolidatedNetwork_t cacheEntry = {false, qualNetworks};
      std::string apnLower(profile.apn);
      // convert apn to lower case for case insensitive matching
      std::transform( apnLower.begin(), apnLower.end(), apnLower.begin(),
                     [](char c) -> char { return std::tolower(c); });
      mNetworkAvailabilityCache[apnLower] = cacheEntry;
      std::ostringstream typeSs;
      transform( apnTypes.begin(), apnTypes.end(), ostream_iterator<string>(typeSs, "|"),
                [](int32_t type) -> string { return convertToString(static_cast<ApnTypes_t>(type)); });
      logBuffer.addLogWithTimestamp("Added new apn=" + apnLower + " types=" + typeSs.str());
    }
  }
}

void NetworkAvailabilityHandler::updateNetworkAvailabilityCache(dsd_apn_avail_sys_info_type_v01 * apn_sys, uint32 len)
{
  std::vector<QualifiedNetwork_t> resultList;
  for (uint32 i = 0; i < len; i++) {
    std::string apnLower(apn_sys[i].apn_name);
    // convert apn to lower case for case insensitive matching
    std::transform( apnLower.begin(), apnLower.end(), apnLower.begin(),
                    [](char c) -> char { return std::tolower(c); });
    Log::getInstance().d("[NetworkAvailabilityHandler]: processing apn name " +
                          apnLower);
    if (apnLower.length() == 0) {
      Log::getInstance().d("[NetworkAvailabilityHandler]: apn name is NULL, skipping the entry ");
      continue;
    }
    if (mNetworkAvailabilityCache.find(apnLower) != mNetworkAvailabilityCache.end()) {
      ConsolidatedNetwork_t &cacheEntry = mNetworkAvailabilityCache[apnLower];
      // If there is an intent to change pending, check that the cached preferred RAT matches with the indication's
      // and consider the handover completed by setting hasPendingIntentToChange, otherwise ignore it
      if (cacheEntry.hasPendingIntentToChange) {
        Log::getInstance().d("[NetworkAvailabilityHandler]: apn already present with pending intent to change");
        if (apn_sys[i].apn_avail_sys_len > 0) {
          auto prefRan = convertToRadioAccessNetwork(apn_sys[i].apn_avail_sys[0]);
          if (!cacheEntry.qualNetworks.empty()) {
            auto qualNetwork = cacheEntry.qualNetworks.front();
            if (!qualNetwork.network.empty()) {
              AccessNetwork_t ran = static_cast<AccessNetwork_t>(qualNetwork.network.front());
              if (ran == prefRan) {
                string padded = "Completed intent to change for apn="
                                              + apnLower + " preferredRat=" + convertToString(prefRan);
                logBuffer.addLogWithTimestamp(padded);
                Log::getInstance().d("NetworkAvailabilityHandler]: " + padded);
                cacheEntry.hasPendingIntentToChange = false;
              }
            }
          }
        } else {
          Log::getInstance().d("[NetworkAvailabilityHandler]: no available system for apn");
          continue;
        }
      } else {
        Log::getInstance().d("[NetworkAvailabilityHandler]: apn already present without pending intent to change");
        std::vector<int32_t> networksList = convertToRadioAccessNetworkList(apn_sys[i]);

        // If network list changed for a qualified network, add it to the result list to be sent
        for (QualifiedNetwork_t &qualNetwork : cacheEntry.qualNetworks) {
          if (qualNetwork.network != networksList) {
            qualNetwork.network = networksList;
            resultList.push_back({qualNetwork.apnType, networksList});
            string padded = "Updated network list for apn=" + apnLower + " " + convertToString(qualNetwork);
            Log::getInstance().d("[NetworkAvailabilityHandler]: " + padded);
            logBuffer.addLogWithTimestamp(padded);
          }
        }
      }
    } else {
      Log::getInstance().d("[NetworkAvailabilityHandler]: apn not found in cache");
      // Query WDS for APN types, and store them in the cache with the network list from the indication
      std::vector<int32_t> networksList = convertToRadioAccessNetworkList(apn_sys[i]);
      auto apnTypes = std::make_shared<std::list<int32_t>>();
      // temporarily store the newly added qualified networks for this APN
      vector<QualifiedNetwork_t> partialResultList;
      Message::Callback::Status status = wds_endpoint->getApnTypesForName(apnLower,
                                                                          apnTypes);
      if (apnTypes != nullptr && status == Message::Callback::Status::SUCCESS) {
        for (auto it = apnTypes->begin(); it != apnTypes->end(); it++) {
          partialResultList.push_back({convertToRadioApnType(*it), networksList});
        }

        std::ostringstream typeSs, nwSs;
        transform( apnTypes->begin(), apnTypes->end(), ostream_iterator<string>(typeSs, "|"),
                  [](int32_t type) -> string { return convertToString(static_cast<ApnTypes_t>(type)); });
        transform( networksList.begin(), networksList.end(), ostream_iterator<string>(nwSs, ","),
                  [](int32_t nw) -> string { return convertToString(static_cast<AccessNetwork_t>(nw)); });
        string padded = "Added new apn=" + apnLower + " types=" + typeSs.str() + " networks=" + nwSs.str();
        Log::getInstance().d("[NetworkAvailabilityHandler]: " + padded);
        logBuffer.addLogWithTimestamp(padded);
        mNetworkAvailabilityCache[apnLower] = {false, partialResultList};
        // add all the newly added qualified networks to the result list to be sent
        resultList.insert(resultList.end(), partialResultList.begin(), partialResultList.end());
      }
      else {
        Log::getInstance().d("[NetworkAvailabilityHandler]: getApnTypesForName is failed");
      }
    }
  }
  std::vector<QualifiedNetwork_t> qualifiedNetworks;
  convertResultList(resultList, qualifiedNetworks);
  for (auto qn : qualifiedNetworks) {
    string padded = convertToString(qn);
    Log::getInstance().d("[NetworkAvailabilityHandler]: " + padded);
    logBuffer.addLogWithTimestamp(padded);
  }
  if (!qualifiedNetworks.empty()) {
    auto indMsg = std::make_shared<rildata::QualifiedNetworksChangeIndMessage>(qualifiedNetworks);
    indMsg->broadcast();
    Log::getInstance().d("[NetworkAvailabilityHandler]: Sent qualified networks");
  }
}

void NetworkAvailabilityHandler::updateNetworkAvailabilityCache(dsd_apn_pref_sys_type_ex_v01 * apn_pref_sys, uint32 len)
{
  vector<QualifiedNetwork_t> resultList;
  for (uint32 i=0; i < len; i++)
  {
    AccessNetwork_t prefNw;
    uint32_t dsdPrefNw;
    // get the preferred network from the indication
    if (apn_pref_sys[i].apn_pref_info.pref_sys == DSD_APN_PREF_SYS_IWLAN_V01)
    {
      prefNw = AccessNetwork_t::IWLAN;
      dsdPrefNw = DSD_APN_PREF_SYS_IWLAN_V01;
    }
    else
    {
      dsd_system_status_info_type_v01 system_status_info;
      system_status_info.rat_value = apn_pref_sys[i].rat;
      system_status_info.so_mask = apn_pref_sys[i].so_mask;
      prefNw = convertToRadioAccessNetwork(system_status_info);
      dsdPrefNw = DSD_APN_PREF_SYS_WWAN_V01;
    }
    std::string apnLower(apn_pref_sys[i].apn_pref_info.apn_name);
    // convert apn to lower case for case insensitive matching
    std::transform( apnLower.begin(), apnLower.end(), apnLower.begin(),
                    [](char c) -> char { return std::tolower(c); });
    // Respond to dsd with ack if the apn is not found in the cache
    if (mNetworkAvailabilityCache.find(apnLower) == mNetworkAvailabilityCache.end()) {
      Log::getInstance().d("[NetworkAvailabilityHandler]: cannot find entry with apn name " + apnLower);
      Message::Callback::Status status = dsd_endpoint->setApnPreferredSystemChangeSync(apnLower,
                                                                                       static_cast<int32_t>(dsdPrefNw));
      if (status != Message::Callback::Status::SUCCESS) {
        Log::getInstance().d("[NetworkAvailabilityHandler]: failed to acknowledge apn");
      }
      continue;
    }
    Log::getInstance().d("[NetworkAvailabilityHandler]: preferred network for apn " + apnLower
       + " is " + std::to_string((int32_t)prefNw) + " apn types mask is " + std::to_string(apn_pref_sys[i].apn_type_mask));
    ConsolidatedNetwork_t &cacheEntry = mNetworkAvailabilityCache[apnLower];
    for (QualifiedNetwork_t &qualNetwork : cacheEntry.qualNetworks) {
      // Remove the preferred network from networks list, if it exists
      // then insert the preferred network to the front of the list
      auto prefNwIt = find(qualNetwork.network.begin(), qualNetwork.network.end(), static_cast<int32_t>(prefNw));
      if (prefNwIt != qualNetwork.network.end()) {
        qualNetwork.network.erase(prefNwIt);
      }
      qualNetwork.network.insert(qualNetwork.network.begin(), static_cast<int32_t>(prefNw));
      string padded = "Updated network list for apn=" + apnLower + " " + convertToString(qualNetwork);
      Log::getInstance().d("[NetworkAvailabilityHandler]: " + padded);
      logBuffer.addLogWithTimestamp(padded);
    }
    cacheEntry.hasPendingIntentToChange = true;
    logBuffer.addLogWithTimestamp("Initiated intent to change for apn=" + apnLower);
    resultList.insert(resultList.end(), cacheEntry.qualNetworks.begin(), cacheEntry.qualNetworks.end());
  }
  std::vector<QualifiedNetwork_t> qualifiedNetworks;
  convertResultList(resultList, qualifiedNetworks);
  for (auto qn : qualifiedNetworks) {
    string padded = convertToString(qn);
    Log::getInstance().d("[NetworkAvailabilityHandler]: " + padded);
    logBuffer.addLogWithTimestamp(padded);
  }
  if (!qualifiedNetworks.empty()) {
    auto indMsg = std::make_shared<rildata::QualifiedNetworksChangeIndMessage>(qualifiedNetworks);
    indMsg->broadcast();
    Log::getInstance().d("[NetworkAvailabilityHandler]: Sent qualified networks");
  }
}

void NetworkAvailabilityHandler::dumpCache() {
  for (auto it = mNetworkAvailabilityCache.begin(); it != mNetworkAvailabilityCache.end(); it++) {
    Log::getInstance().d("dumpCache apn=" + it->first + " hasPendingIntent=" +
                         to_string(static_cast<int>(it->second.hasPendingIntentToChange)));
    for (auto qn : it->second.qualNetworks) {
      string padded = "dumpCache     " + convertToString(qn);
      Log::getInstance().d(padded);
    }
  }
}

AccessNetwork_t NetworkAvailabilityHandler::convertToRadioAccessNetwork(dsd_system_status_info_type_v01 system_status_info )
{
  switch (system_status_info.rat_value)
  {
     case DSD_SYS_RAT_EX_3GPP_GERAN_V01:
       return AccessNetwork_t::GERAN;
     case DSD_SYS_RAT_EX_3GPP_WCDMA_V01:
     case DSD_SYS_RAT_EX_3GPP_TDSCDMA_V01:
       return AccessNetwork_t::UTRAN;
     case DSD_SYS_RAT_EX_3GPP_LTE_V01:
     case DSD_SYS_RAT_EX_3GPP_5G_V01:
       return AccessNetwork_t::EUTRAN;
     case DSD_SYS_RAT_EX_3GPP2_1X_V01:
     case DSD_SYS_RAT_EX_3GPP2_HRPD_V01:
     case DSD_SYS_RAT_EX_3GPP2_EHRPD_V01:
       return AccessNetwork_t::CDMA2000;
     case DSD_SYS_RAT_EX_3GPP_WLAN_V01:
       return AccessNetwork_t::IWLAN;
     default:
       return AccessNetwork_t::UNKNOWN;
  }
}

vector<int32_t> NetworkAvailabilityHandler::convertToRadioAccessNetworkList(dsd_apn_avail_sys_info_type_v01 apn_sys) {
  vector<int32_t> networks;
  for (uint32 j=0; j < apn_sys.apn_avail_sys_len; j++)
  {
    int32_t ran = static_cast<int32_t>(convertToRadioAccessNetwork(apn_sys.apn_avail_sys[j]));
    auto it = find(networks.begin(), networks.end(), ran);
    //add AccessNetwork if not already existing in vector
    if (it == networks.end())
    {
      networks.push_back(ran);
      Log::getInstance().d("[NetworkAvailabilityHandler]: added network "
                            + std::to_string(ran));
    }
  }
  return networks;
}

int32_t NetworkAvailabilityHandler::convertToRadioApnType(uint64_t qmiApnType)
{
  int32_t result;
  switch (qmiApnType)
  {
     case QMI_DSD_APN_TYPE_MASK_DEFAULT_V01:
       result = (int32_t)ApnTypes_t::DEFAULT;
       break;
     case QMI_DSD_APN_TYPE_MASK_IMS_V01:
       result = (int32_t)ApnTypes_t::IMS;
       break;
     case QMI_DSD_APN_TYPE_MASK_MMS_V01:
       result = (int32_t)ApnTypes_t::MMS;
       break;
     case QMI_DSD_APN_TYPE_MASK_DUN_V01:
       result = (int32_t)ApnTypes_t::DUN;
       break;
     case QMI_DSD_APN_TYPE_MASK_SUPL_V01:
       result = (int32_t)ApnTypes_t::SUPL;
       break;
     case QMI_DSD_APN_TYPE_MASK_HIPRI_V01:
       result = (int32_t)ApnTypes_t::HIPRI;
       break;
     case QMI_DSD_APN_TYPE_MASK_FOTA_V01:
       result = (int32_t)ApnTypes_t::FOTA;
       break;
     case QMI_DSD_APN_TYPE_MASK_CBS_V01:
       result = (int32_t)ApnTypes_t::CBS;
       break;
     case QMI_DSD_APN_TYPE_MASK_IA_V01:
       result = (int32_t)ApnTypes_t::IA;
       break;
     case QMI_DSD_APN_TYPE_MASK_EMERGENCY_V01:
       result = (int32_t)ApnTypes_t::EMERGENCY;
       break;
     default:
       result = (int32_t)ApnTypes_t::NONE;
  }
  /*Log::getInstance().d("[NetworkAvailabilityHandler]: qmi apnType " +
                       std::to_string(qmiApnType) + "HAL apnType " +  std::to_string(result));*/
  return result;

}
vector<int32_t> NetworkAvailabilityHandler::convertToRadioApnTypeList(const ApnTypes_t &apnTypes) {
  int32_t apnBitfield = static_cast<int32_t>(apnTypes);
  vector<int32_t> result;
  for (int32_t mask = 1;
               mask <= static_cast<int32_t>(ApnTypes_t::EMERGENCY); mask <<= 1) {
    if ((apnBitfield & mask) == mask) {
      result.push_back(mask);
    }
  }
  return result;
}

string NetworkAvailabilityHandler::convertToString(const AccessNetwork_t &ran) {
  switch (ran) {
    case AccessNetwork_t::UNKNOWN:
      return "UNKNOWN";
    case AccessNetwork_t::GERAN:
      return "GERAN";
    case AccessNetwork_t::UTRAN:
      return "UTRAN";
    case AccessNetwork_t::EUTRAN:
      return "EUTRAN";
    case AccessNetwork_t::CDMA2000:
      return "CDMA2000";
    case AccessNetwork_t::IWLAN:
      return "IWLAN";
    default:
      return to_string(static_cast<int32_t>(ran));
  }
}

string NetworkAvailabilityHandler::convertToString(const ApnTypes_t &apnType) {
  switch (apnType)
  {
    case ApnTypes_t::NONE:
      return "NONE";
    case ApnTypes_t::DEFAULT:
      return "DEFAULT";
    case ApnTypes_t::MMS:
      return "MMS";
    case ApnTypes_t::SUPL:
      return "SUPL";
    case ApnTypes_t::DUN:
      return "DUN";
    case ApnTypes_t::HIPRI:
      return "HIPRI";
    case ApnTypes_t::FOTA:
      return "FOTA";
    case ApnTypes_t::IMS:
      return "IMS";
    case ApnTypes_t::CBS:
      return "CBS";
    case ApnTypes_t::IA:
      return "IA";
    case ApnTypes_t::EMERGENCY:
      return "EMERGENCY";
    default:
      return to_string(static_cast<int32_t>(apnType));
  }
}

std::string NetworkAvailabilityHandler::convertToString(const QualifiedNetwork_t &qn) {
  std::ostringstream ss, networks;
  ApnTypes_t qualApnType = static_cast<ApnTypes_t>(qn.apnType);
  transform( qn.network.begin(), qn.network.end(), ostream_iterator<string>(networks, ","),
            [](int32_t type) -> string { return convertToString(static_cast<AccessNetwork_t>(type)); });
  ss << "type=" << convertToString(qualApnType) << " networks=[" << networks.str() << "]";
  return ss.str();
}
