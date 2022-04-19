/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include <sstream>
#include "framework/Log.h"
#include "BaseProfile.h"
#include "qdp_adapter.h"
#include "qdp.h"

/**
 * @brief      Look up wrapper which will call qdp specfic lookup
               API
 *
 * @return     profie id if found, INVALID_PROFILE_ID otherwise
 */
unsigned int BaseProfile::lookup() const
{
  Log::getInstance().d(mName + "Not yet implemented");
  return INVALID_PROFILE_ID;
}

/**
 * @brief      Look up wrapper which will call qdp specfic lookup
               via APN class
 *
 * @return     QDP_FAILURE if profile was not found/created.
 *             QDP_SUCCESS otherwise
 */
unsigned int BaseProfile::lookupByApnClass(const BaseProfile::params_t in_params) const
{
  Log::getInstance().d("[qdphandler] In BaseProfile::lookupByApnClass()");

  qdp_profile_pdn_type     umts_profile_pdn_type;
  unsigned int             umts_profile_id_list[QDP_NUM_PROFILES_EXPECTED_MAX];
  uint8                    umts_profile_list_len;

  qdp_profile_pdn_type     cdma_profile_pdn_type;
  unsigned int             cdma_profile_id_list[QDP_NUM_PROFILES_EXPECTED_MAX];
  uint8                    cdma_profile_list_len;

  qdp_error_info_t         error_info;
  qdp_profile_info_type    prof_params;
  qdp_tech_t               tech_type;
  uint8                    apn_class;
  unsigned int             j=0;
  const char             * qdp_params[QDP_RIL_PARAM_MAX];
  wds_modify_profile_settings_req_msg_v01  new_p_params;
  boolean updateNeeded = FALSE;

  memset( &error_info, 0x0, sizeof(error_info) );
  memset( &prof_params, 0,  sizeof(prof_params) );
  memset( qdp_params, 0, sizeof(qdp_params));

  dataprofile_get_legacy_params(in_params, (char**) qdp_params);

  /* Convert Profile ID to APN class as we expect the same from Telephony */
  #define IMS_CLASS  1
  #define ADMN_CLASS 2
  #define INET_CLASS 3
  #define APP_CLASS  4

  switch(in_params.profileId) {
    case 0:
      apn_class = INET_CLASS;
      break;
    case 2:
      apn_class = IMS_CLASS;
      break;
    case 3:
      apn_class = ADMN_CLASS;
      break;
    case 4:
      apn_class = APP_CLASS;
      break;
    default:
      Log::getInstance().d("[qdphandler] Invalid profileId");
      dataprofile_release_legacy_params((char**)qdp_params);
      return QDP_FAILURE;
  } /* profileId */

  /* Look up for a profile that matches APN class */
  if( QDP_SUCCESS != qdp_profile_look_up_by_param
                     ( qdp_params,
                       QDP_RIL_CLASS,
                       apn_class,
                       &umts_profile_id_list[0],
                       &umts_profile_list_len,
                       &umts_profile_pdn_type,
                       &cdma_profile_id_list[0],
                       &cdma_profile_list_len,
                       &cdma_profile_pdn_type,
                       &tech_type,
                       &error_info ) ) {
     Log::getInstance().d("[qdphandler] failed qdp_profile_look_up_by_param()");
     dataprofile_release_legacy_params((char**)qdp_params);
     return QDP_FAILURE;
  }

  for(j = 0; j < umts_profile_list_len; j++) {
    int qmi_err_code;
    memset(&new_p_params, 0, sizeof(new_p_params));
    if(umts_profile_id_list[j] == QDP_INVALID_PROFILE) {
      break;
    }
    if(tech_type == QDP_3GPP) {
      qdp_3gpp_profile_update_params_v2(qdp_params,
                                     umts_profile_id_list[j],
                                     &new_p_params,
                                     &updateNeeded);

      new_p_params.apn_class_valid = TRUE;
      new_p_params.apn_class = apn_class;

      /* Compare the 3 parameters retrieved from profile through QMI_WDS query
       * & stored in the  new_p_params fields
       */
      if ( in_params.maxConnsTime > 0 ) {
        if ( new_p_params.max_pdn_conn_timer != in_params.maxConnsTime ) {
          Log::getInstance().d("Update needed due to change in maxpdn_conn_timer");
          updateNeeded = TRUE;
          new_p_params.max_pdn_conn_timer_valid = TRUE;
          new_p_params.max_pdn_conn_timer = in_params.maxConnsTime;
        }
      }
      if ( in_params.maxConns > 0 ) {
        if ( new_p_params.max_pdn_conn_per_block != in_params.maxConns ) {
          Log::getInstance().d("Update needed due to change in maxpdn_conn_block");
          updateNeeded = TRUE;
          new_p_params.max_pdn_conn_per_block_valid = TRUE;
          new_p_params.max_pdn_conn_per_block = in_params.maxConns;
        }
      }
      if ( in_params.waitTime > 0 ) {
        if ( new_p_params.pdn_req_wait_interval != in_params.waitTime ) {
          Log::getInstance().d("Update needed due to change in pdn_req_wait_interval");
          updateNeeded = TRUE;
          new_p_params.pdn_req_wait_interval_valid = TRUE;
          new_p_params.pdn_req_wait_interval = in_params.waitTime;
        }
      }
      if ( new_p_params.apn_disabled_flag == in_params.enableProfile ) {
        Log::getInstance().d("Update needed due to change in apn_disabled_flag");
        updateNeeded = TRUE;
        new_p_params.apn_disabled_flag_valid = TRUE;
        new_p_params.apn_disabled_flag = (in_params.enableProfile == 0)? 1:0;
      }
    }
    if( updateNeeded ) {
      Log::getInstance().d("Updating Profile Params");
      if (QDP_SUCCESS != qdp_profile_update_ex( &new_p_params,
                                                &qmi_err_code )) {
        Log::getInstance().d("Profile update failed for profile = " \
            +std::to_string((int)umts_profile_id_list[j])+"err = " \
            +std::to_string((int)qmi_err_code));
      }
    } else {
      Log::getInstance().d("Update of Profile Params is not Needed. Skipping profile updation");
    }
  }

  for(j = 0; j < cdma_profile_list_len; j++) {
    int qmi_err_code;
    memset(&new_p_params, 0, sizeof(new_p_params));
    if(cdma_profile_id_list[j] == QDP_INVALID_PROFILE) {
      break;
    }

    qdp_3gpp2_profile_update_params(qdp_params,
                                    cdma_profile_id_list[j],
                                    &new_p_params);

    new_p_params.apn_class_3gpp2_valid = TRUE;
    new_p_params.apn_class_3gpp2 = apn_class;

    new_p_params.apn_enabled_3gpp2_valid = TRUE;
    new_p_params.apn_enabled_3gpp2 = in_params.enableProfile;

    if (QDP_SUCCESS != qdp_profile_update_ex( &new_p_params,
                                              &qmi_err_code )) {
       Log::getInstance().d("Profile update failed for profile = " \
           +std::to_string((int)cdma_profile_id_list[j])+"err = " \
           +std::to_string((int)qmi_err_code));
     }
  }

  dataprofile_release_legacy_params((char**)qdp_params);
  return QDP_SUCCESS;
} /* BaseProfile::lookupByApnClass() */

unsigned int BaseProfile::findMatch(const std::list<uint16_t>& profile_list) const
{
  (void) profile_list;
  Log::getInstance().d(mName + "Not yet implemented");
  return INVALID_PROFILE_ID;
}

void BaseProfile::dump(std::string padding, std::ostream& os)
{
  os << padding << "profileId=" << params.profileId << std::boolalpha <<
        ", tech=" << static_cast<int>(params.radioTech) <<
        ", apn=" << params.apn << std::endl <<
        padding << "username=" << params.username <<
        ", password=" << params.password <<
        ", authType=" << params.authType << std::endl <<
        padding << "protocol=" << params.protocol <<
        ", roamingProtocol=" << params.roamingProtocol << std::endl << std::boolalpha <<
        padding << "apnTypeBitmask=" << params.apnTypeBitmask <<
        ", bearerBitmask=" << params.bearerBitmask <<
        ", mtu=" << static_cast<int>(params.mtu) << std::endl << std::boolalpha <<
        padding << "roamingAllowed=" << params.roamingAllowed <<
        ", preferred=" << params.preferred <<
        ", persistent=" << params.persistent <<
        ", enabled=" << params.enableProfile << std::endl <<
        padding << "maxConnsTime=" << params.maxConnsTime <<
        ", maxConns=" << params.maxConns <<
        ", waitTime=" << params.waitTime << std::endl;
}

//TO BE ENABLED LATER
/*bool BaseProfile::matchHomeProtocolStrict(std::string other) const
{
  (void) other;
  Log::getInstance().d(mName + "Not yet implemented");
  return false;
}

bool BaseProfile::matchHomeProtocolLoose(std::string other) const
{
  (void) other;
  Log::getInstance().d(mName + "Not yet implemented");
  return false;
}

bool BaseProfile::matchRoamProtocolStrict(std::string other) const
{
  (void) other;
  Log::getInstance().d(mName + "Not yet implemented");
  return false;
}

bool BaseProfile::matchRoamProtocolLoose(std::string other) const
{
  (void) other;
  Log::getInstance().d(mName + "Not yet implemented");
  return false;
}*/
