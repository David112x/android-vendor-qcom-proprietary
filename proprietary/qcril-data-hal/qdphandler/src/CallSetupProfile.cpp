/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include <sstream>
#include "framework/Log.h"
#include "CallSetupProfile.h"
#include "qdp_adapter.h"
#include "qdp.h"

bool CallSetupProfile::lookup()
{
  Log::getInstance().d("[ProfileHandler] In CallSetupProfile::lookup()");
  char *qdp_params[QDP_RIL_PARAM_MAX];
  memset(qdp_params, 0, sizeof(qdp_params));

  // look up profile using qdp
  qdp_profile_pdn_type umtsProfilePdnType;
  qdp_profile_pdn_type cdmaProfilePdnType;
  qdp_error_info_t errorInfo;
  unsigned int umtsProfileIdRet = 0;
  unsigned int cdmaProfileIdRet = 0;

  dataprofile_get_legacy_params(params, (char**) qdp_params);

  if( ignoreRilTech )
  {
    Log::getInstance().d("ignore_ril_tech is set to TRUE");
    Log::getInstance().d("qdp param TECH not set on purpose in order to" \
                         " enforce 3gpp and 3gpp2 profile look up");

    free( qdp_params[QDP_RIL_TECH] );
    qdp_params[QDP_RIL_TECH] = NULL;
  }
  qdp_profile_look_up((const char **)qdp_params,
                      &umtsProfileIdRet,
                      &umtsProfilePdnType,
                      &cdmaProfileIdRet,
                      &cdmaProfilePdnType,
                      &errorInfo);
  umtsProfileId = (int)umtsProfileIdRet;
  cdmaProfileId = (int)cdmaProfileIdRet;

  dataprofile_release_legacy_params((char**)qdp_params);
  return true;
}

bool CallSetupProfile::emergencyLookup() {
  Log::getInstance().d("[ProfileHandler] In CallSetupProfile::emergencyLookup()");
  char *qdp_params[QDP_RIL_PARAM_MAX];
  memset(qdp_params, 0, sizeof(qdp_params));
  unsigned int profileId = QDP_INVALID_PROFILE;
  char apnName[QMI_WDS_APN_NAME_MAX_V01+1];
  qdp_profile_pdn_type pdnType;
  qdp_tech_t techType;

  dataprofile_get_legacy_params(params, (char**) qdp_params);
  emergencyProfileLookup2((const char **)qdp_params, &profileId, apnName, &pdnType, &techType);
  switch (techType) {
      case QDP_3GPP:
        setUmtsProfileId((int)profileId);
        break;
      case QDP_3GPP2:
        setCdmaProfileId((int)profileId);
        break;
      default:
        Log::getInstance().d("[ProfileHandler] Invalid tech type");
        return false;
  }
  switch (pdnType) {
      case QDP_PROFILE_PDN_TYPE_IPV4:
        setProtocol("IP");
        break;
      case QDP_PROFILE_PDN_TYPE_IPV6:
        setProtocol("IPV6");
        break;
      case QDP_PROFILE_PDN_TYPE_IPV4V6:
        setProtocol("IPV4V6");
        break;
      default:
        Log::getInstance().d("[ProfileHandler] Invalid pdn type");
        return false;
  }
  setEmergencyCallApn(string(apnName));
  dataprofile_release_legacy_params((char**)qdp_params);
  return true;
}

void CallSetupProfile::dump(std::string padding, std::ostream& os) {
  BaseProfile::dump(padding, os);
  os << padding << "umtsProfileId=" << umtsProfileId <<
        ", cdmaProfileId=" << cdmaProfileId << std::endl;;
}
