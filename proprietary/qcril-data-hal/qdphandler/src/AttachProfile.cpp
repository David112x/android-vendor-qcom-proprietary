/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include <sstream>
#include "framework/Log.h"
#include "AttachProfile.h"
#include "qdp_adapter.h"
#include "qdp.h"

unsigned int AttachProfile::lookup() const
{
  Log::getInstance().d("[ProfileHandler] In AttachProfile::lookup()");
  char *qdp_params[QDP_RIL_PARAM_MAX];
  memset(qdp_params, 0, sizeof(qdp_params));
  dataprofile_get_legacy_params(params, (char**) qdp_params);

  unsigned int selected_profile_id = QDP_INVALID_PROFILE;
  qdp_profile_pdn_type incoming_profile_pdn_type = QDP_PROFILE_PDN_TYPE_INVALID;
  unsigned int incoming_profile_id = QDP_INVALID_PROFILE;
  qdp_error_info_t error_info;
  qdp_profile_info_type prof_params;

  memset( &error_info, 0x0, sizeof(error_info) );
  memset( &prof_params, 0,  sizeof(prof_params) );

  int rc = qdp_lte_attach_profile_lookup( (const char**) qdp_params /*in */,
                                         &selected_profile_id /* out */,
                                         &incoming_profile_pdn_type /*in */,
                                         &incoming_profile_id /*in */,
                                         &prof_params,
                                         &error_info);
  if( QDP_SUCCESS != rc )
  {
    std::ostringstream ss;
    ss << "[AttachProfile] Failed with error = " << error_info.error;
    Log::getInstance().d(ss.str());
    selected_profile_id = AttachProfile::INVALID_PROFILE_ID;
  }
  dataprofile_release_legacy_params((char**)qdp_params);
  return selected_profile_id;
}

unsigned int AttachProfile::findMatch(const std::list<uint16_t>& profile_list) const
{
  Log::getInstance().d("In AttachProfile::findMatch ENTRY");
  const char *qdp_params[QDP_RIL_PARAM_MAX];
  memset(qdp_params, 0, sizeof(qdp_params));
  dataprofile_get_legacy_params(params, (char**) qdp_params);

  std::list<uint16_t>::const_iterator it;
  for(it = profile_list.begin(); it != profile_list.end(); ++it)
  {
    if (QDP_SUCCESS == qdp_match_lte_attach_profile_params(*it, qdp_params))
    {
      dataprofile_release_legacy_params((char**)qdp_params);
      return *it;
    }
  }
  dataprofile_release_legacy_params((char**)qdp_params);
  return INVALID_PROFILE_ID;
}

AttachProfile::params_t AttachProfile::getParams()
{
  return params;
}
