/******************************************************************************
#  Copyright (c) 2009-2010, 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
/******************************************************************************
  @file    qcril_pbm.c
  @brief   qcril qmi - PB

  DESCRIPTION

******************************************************************************/


/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include <string.h>
#include <cutils/properties.h>
#include "qcril_log.h"
#include "qcril_pbm.h"
#include "qcril_cm_ss.h"
#include "modules/nas/qcril_qmi_nas.h"
#include "interfaces/nas/nas_types.h"
#include "interfaces/voice/voice.h"
#include "qcril_other.h"

#include "phonebook_manager_service_v01.h"
#include "qmi_ril_platform_dep.h"
#include "qcril_reqlist.h"

#include "qcril_pbm_legacy.h"
#include "modules/qmi/QmiStruct.h"
#include "modules/qmi/PbmModemEndPoint.h"
#include "modules/qmi/ModemEndPointFactory.h"
#include "qcril_qmi_oem_events.h"
//#ifdef QMI_RIL_UTF
#include "modules/pbm/PbmModule.h"
//#endif

#include <interfaces/nas/RilUnsolEmergencyListIndMessage.h>

using qcril::interfaces::RIL_EccMapType;
using qcril::interfaces::RIL_EccNumberSourceMask;
using qcril::interfaces::RIL_EccCategoryMask;
using qcril::interfaces::RIL_EmergencyNumber_t;

#define TAG                            "qcril_pbm"
/*===========================================================================

                         LOCAL VARIABLES

===========================================================================*/
typedef enum
{
  QCRIL_PBM_STATUS_UNKNOWN                   = 0,
  QCRIL_PBM_STATUS_REGISTER_READY,
  QCRIL_PBM_STATUS_GET_CAPACITY_COMPLETE,
  QCRIL_PBM_STATUS_READING_RECORD,
  QCRIL_PBM_STATUS_READ_RECORD_COMPLETE
} qcril_pbm_status_e_type;

struct qcril_pbm_alt_emer_num_info {
    char number[QMI_VOICE_NUMBER_MAX_V02];
    uint32_t emerg_srv_categ;
};

struct pbm_cache_info_type {
    // store ADN info
    qcril_pbm_status_e_type pbm_status;
    int pbm_num_session;
    int pbm_session_type[QMI_PBM_MAX_NUM_SESSIONS_V01];
    qcril::interfaces::qcril_pbm_adn_count_info pbm_capacity[QMI_PBM_MAX_NUM_SESSIONS_V01];

    //Store reg mask info
    pbm_reg_mask_type_v01 pbm_reg_mask;

    // Store alternate emergency number details
    boolean alt_emer_num_info_valid;
    qcril_pbm_alt_emer_num_info alt_emer_num_info;

    // PBM centralized ecc support
    int pbm_centralized_ecc_supported;

    RIL_EccMapType pbm_ecc_map;
    RIL_EccMapType combined_ecc_map;
};

struct pbm_common_info_type {
    // for protecting
    qtimutex::QtiSharedMutex pbm_info_mutex;
};

static struct pbm_cache_info_type pbm_cache_info;
static struct pbm_common_info_type pbm_common_info;

typedef struct
{
    pbm_session_type_enum_v01 session_type;
    int record_index;
    int token;
} qcril_pbm_cb_struct_type;

#define PBM_CACHE_LOCK()    do {\
        QCRIL_LOG_INFO("LOCK PBM_CACHE_LOCK");\
        pbm_common_info.pbm_info_mutex.lock();\
    } while (0)

#define PBM_CACHE_UNLOCK()  do {\
        pbm_common_info.pbm_info_mutex.unlock();\
        QCRIL_LOG_INFO("UNLOCK PBM_CACHE_LOCK");\
    } while (0)

/*===========================================================================

                                FUNCTIONS

===========================================================================*/
void get_ecc_property_name(char *ecc_prop_name);
void qcril_qmi_pbm_all_phonebook_init_done_ind_hdlr(void *ind_data_ptr, uint32 ind_data_len);
RIL_Errno qcril_qmi_pbm_get_records_count(void);
RIL_Errno qcril_qmi_pbm_get_all_of_records(int adn_count, int session_type);
void qcril_qmi_pbm_record_read_ext_ind_hdlr(void *ind_data_ptr,
                                            uint32 ind_data_len,
                                            qcril_instance_id_e_type instance_id);
RIL_Errno qcril_qmi_pbm_delete_adn_record(int record_id, int session_type, int token);
RIL_Errno qcril_qmi_pbm_add_or_modify_adn_record(
        const qcril::interfaces::AdnRecordInfo* record_info_ptr,
        int record_id,
        int session_type,
        int token);
extern void qcril_qmi_nas_get_ril_ecc_map(qcril::interfaces::RIL_EccMapType& ecc_map);
RIL_Errno qcril_qmi_pbm_query_centralized_ecc_support(void);
void qcril_qmi_pbm_sim_init_done_ind_hdlr(void *ind_data_ptr, uint32 ind_data_len);
void qcril_qmi_pbm_pre_init
(
  void
)
{
  QCRIL_LOG_FUNC_ENTRY();
  memset(&pbm_cache_info, 0, sizeof(pbm_cache_info));
  QCRIL_LOG_FUNC_RETURN();
}

void qcril_qmi_pbm_cleanup
(
  void
)
{
  QCRIL_LOG_FUNC_ENTRY();
  PBM_CACHE_LOCK();
  memset(&pbm_cache_info, 0, sizeof(pbm_cache_info));
  PBM_CACHE_UNLOCK();
  QCRIL_LOG_FUNC_RETURN();
}

void qcril_qmi_pbm_init
(
  void
)
{
  QCRIL_LOG_FUNC_ENTRY();
  qcril_qmi_pbm_query_centralized_ecc_support();
  qcril_qmi_pbm_enable_emergency_number_indications(TRUE);
  qcril_qmi_pbm_enable_phonebook_ready_indications(TRUE);
  qcril_qmi_nas_start_wait_for_pbm_ind_timer();
  QCRIL_LOG_FUNC_RETURN();
}

int qcril_qmi_pbm_centralized_ecc_support_available()
{
    int res = FALSE;
    PBM_CACHE_LOCK();
    res = pbm_cache_info.pbm_centralized_ecc_supported;
    PBM_CACHE_UNLOCK();

    return res;
}

RIL_Errno qcril_qmi_pbm_query_centralized_ecc_support()
{
    RIL_Errno ril_req_res = RIL_E_GENERIC_FAILURE;
    qmi_client_error_type qmi_transport_error;

    pbm_get_configuration_req_msg_v01  qmi_req;
    pbm_get_configuration_resp_msg_v01 qmi_resp;

    memset(&qmi_req, 0, sizeof(qmi_req));
    memset(&qmi_resp, 0, sizeof(qmi_resp));

    qmi_req.pbm_feature_status_mask_valid = TRUE;
    qmi_req.pbm_feature_status_mask = 0x01; // for PBM_CENTRALIZED_ECC

    qmi_transport_error = qmi_client_pbm_send_sync(QMI_PBM_GET_CONFIGURATION_REQ_V01,
                                                   &qmi_req,
                                                   sizeof(qmi_req),
                                                   &qmi_resp,
                                                   sizeof(qmi_resp));
    ril_req_res = qcril_qmi_util_convert_qmi_response_codes_to_ril_result( qmi_transport_error, &qmi_resp.resp );
    if (RIL_E_SUCCESS == ril_req_res)
    {
        QCRIL_LOG_INFO("centralized ecc tlv valid - %d", qmi_resp.centralized_ecc_valid);
        if (qmi_resp.centralized_ecc_valid == TRUE && qmi_resp.centralized_ecc == 1)
        {
            // By default pbm_centralized_ecc_supported is false.
            PBM_CACHE_LOCK();
            pbm_cache_info.pbm_centralized_ecc_supported = TRUE;
            PBM_CACHE_UNLOCK();

            QCRIL_LOG_INFO("centralized ecc supported");
        }
        else
        {
            QCRIL_LOG_INFO("centralized ecc not supported");
        }
    }
    else
    {
        QCRIL_LOG_INFO("centralized ecc query failed, error %d", ril_req_res);
    }

    return ril_req_res;
}


RIL_Errno qcril_qmi_pbm_enable_emergency_number_indications(int enable)
{

  RIL_Errno ril_req_res = RIL_E_GENERIC_FAILURE;
  qmi_client_error_type qmi_transport_error;

  pbm_indication_register_req_msg_v01  indication_req;
  pbm_indication_register_resp_msg_v01 indication_resp;

  QCRIL_LOG_INFO("entered %d",enable);

  memset(&indication_req, 0, sizeof(indication_req));
  memset(&indication_resp, 0, sizeof(indication_resp));

  PBM_CACHE_LOCK();
  if( TRUE == enable )
  {/* Register for Emergency list indications */
    pbm_cache_info.pbm_reg_mask |= PBM_REG_EMERGENCY_NUMBER_LIST_EVENTS_V01;
  }
  else
  {/* Suppress Emergency list indications */
    pbm_cache_info.pbm_reg_mask &= ~PBM_REG_EMERGENCY_NUMBER_LIST_EVENTS_V01;
  }
  indication_req.reg_mask = pbm_cache_info.pbm_reg_mask;
  PBM_CACHE_UNLOCK();

  qmi_transport_error = qmi_client_pbm_send_sync( QMI_PBM_INDICATION_REGISTER_REQ_V01,
                                                   &indication_req,
                                                   sizeof(indication_req),
                                                   &indication_resp,
                                                   sizeof(indication_resp));

  ril_req_res = qcril_qmi_util_convert_qmi_response_codes_to_ril_result( qmi_transport_error, &indication_resp.resp );

  if ( RIL_E_SUCCESS == ril_req_res )
  {
    QCRIL_LOG_INFO("Indication register successful with mask %d", indication_resp.reg_mask);
  }
  else
  {
    QCRIL_LOG_INFO("Indication register failed, error %d", ril_req_res);
  }

  return ril_req_res;
}

uint64_t convertToRilCategories(uint8_t qmi_cat) {
    uint64_t ril_cat = 0;
    if ( qmi_cat & (1 << QMI_PBM_EMER_CAT_POLICE_BIT_V01)) {
        ril_cat |= (uint64_t)RIL_EccCategoryMask::POLICE;
    }
    if ( qmi_cat & (1 << QMI_PBM_EMER_CAT_AMBULANCE_BIT_V01)) {
        ril_cat |= (uint64_t)RIL_EccCategoryMask::AMBULANCE;
    }
    if ( qmi_cat & (1 << QMI_PBM_EMER_CAT_FIRE_BRIGADE_BIT_V01)) {
        ril_cat |= (uint64_t)RIL_EccCategoryMask::FIRE_BRIGADE;
    }
    if ( qmi_cat & (1 << QMI_PBM_EMER_CAT_MARINE_GUARD_BIT_V01)) {
        ril_cat |= (uint64_t)RIL_EccCategoryMask::MARINE_GUARD;
    }
    if ( qmi_cat & (1 << QMI_PBM_EMER_CAT_MOUNTAIN_RESCUE_BIT_V01)) {
        ril_cat |= (uint64_t)RIL_EccCategoryMask::MOUNTAIN_RESCUE;
    }
    if ( qmi_cat & (1 << QMI_PBM_EMER_CAT_MANUAL_ECALL_BIT_V01)) {
        ril_cat |= (uint64_t)RIL_EccCategoryMask::MANUAL_ECALL;
    }
    if ( qmi_cat & (1 << QMI_PBM_EMER_CAT_AUTO_ECALL_BIT_V01)) {
        ril_cat |= (uint64_t)RIL_EccCategoryMask::AUTO_ECALL;
    }

    if ( ril_cat == 0) {
        ril_cat = (uint64_t)RIL_EccCategoryMask::UNKNOWN;
    }
    return (uint64_t)ril_cat;
}


/*=========================================================================
  FUNCTION:  qcril_qmi_pbm_emergency_list_ind_hdlr

===========================================================================*/
/*!
    @brief
    Handle QMI_PBM_EMERGENCY_LIST_IND.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_pbm_emergency_list_ind_hdlr
(
  void *ind_data_ptr,
  uint32 ind_data_len
)
{
  unsigned int i, k;
  RIL_EccMapType ecc_map;
  char mcc[NAS_MCC_MNC_MAX_SIZE] = {0};
  char mnc[NAS_MCC_MNC_MAX_SIZE] = {0};

  QCRIL_LOG_FUNC_ENTRY();

  QCRIL_NOTUSED( ind_data_len );

  if (  ind_data_ptr != NULL )
  {
    auto emerg_ind = (pbm_emergency_list_ind_msg_v01*)ind_data_ptr;

    /* ELENL emergency numbers */
    if(emerg_ind->elenl_emer_nums_valid) {
      for (k=0; k<emerg_ind->elenl_emer_nums_len; k++) {
        for(i=0; i < emerg_ind->elenl_emer_nums[k].emer_nums_len; i++) {
          auto ecc = std::make_shared<RIL_EmergencyNumber_t>();
          if (ecc != nullptr) {
              auto ecc_num = emerg_ind->elenl_emer_nums[k].emer_nums[i].emer_num;
              auto ecc_num_len = emerg_ind->elenl_emer_nums[k].emer_nums[i].emer_num_len;
              ecc->number = std::string(ecc_num, ecc_num_len);
              QCRIL_LOG_INFO("elenl emergency number - %s", ecc->number.c_str());
              auto sub_service = emerg_ind->elenl_emer_nums[k].emer_nums[i].sub_service;
              auto sub_service_len = emerg_ind->elenl_emer_nums[k].emer_nums[i].sub_service_len;
              ecc->urns.push_back(std::string((char*)sub_service, sub_service_len));
              ecc->sources = RIL_EccNumberSourceMask::NETWORK_SIGNALING;
              if (qcril_qmi_nas_fetch_mcc_mnc_of_current_system(mcc, mnc, TRUE, TRUE)) {
                  ecc->mcc = std::string(mcc);
                  ecc->mnc = std::string(mnc);
              }
              ecc_map.insert({ecc->number, ecc});
          }
        }
      }
    }

    /* OTA emergency numbers */
    if(emerg_ind->network_emer_nums_valid) {
      for (k=0; k<emerg_ind->network_emer_nums_len; k++) {
        for(i=0; i < emerg_ind->network_emer_nums[k].emer_nums_len; i++) {
          auto ecc = std::make_shared<RIL_EmergencyNumber_t>();
          if (ecc != nullptr) {
              auto ecc_num = emerg_ind->network_emer_nums[k].emer_nums[i].emer_num;
              auto ecc_num_len = emerg_ind->network_emer_nums[k].emer_nums[i].emer_num_len;
              ecc->number = std::string(ecc_num, ecc_num_len);
              QCRIL_LOG_INFO("network emergency number - %s", ecc->number.c_str());
              ecc->categories = convertToRilCategories(emerg_ind->network_emer_nums[k].emer_nums[i].cat);
              QCRIL_LOG_INFO("network emergency number categories - %d",
                      ecc->categories);
              ecc->sources = RIL_EccNumberSourceMask::NETWORK_SIGNALING;
              if (qcril_qmi_nas_fetch_mcc_mnc_of_current_system(mcc, mnc, TRUE, TRUE)) {
                  ecc->mcc = std::string(mcc);
                  ecc->mnc = std::string(mnc);
              }
              ecc_map.insert({ecc->number, ecc});
          }
        }
      }
    }

    /* Emergency numbers on Card  */
    if ( emerg_ind->card_emer_nums_valid ) {
      for (k=0; k<emerg_ind->card_emer_nums_len; k++) {
        for ( i = 0; i < emerg_ind->card_emer_nums[k].emer_nums_len; i++ ) {
            auto ecc = std::make_shared<RIL_EmergencyNumber_t>();
            if (ecc != nullptr) {
                auto ecc_num = emerg_ind->card_emer_nums[k].emer_nums[i].emer_num;
                auto ecc_num_len = emerg_ind->card_emer_nums[k].emer_nums[i].emer_num_len;
                ecc->number = std::string(ecc_num, ecc_num_len);
                QCRIL_LOG_INFO("card emergency number - %s", ecc->number.c_str());
                ecc->categories = convertToRilCategories(emerg_ind->card_emer_nums[k].emer_nums[i].cat);
                QCRIL_LOG_INFO("card emergency number categories - %d", ecc->categories);
                ecc->sources = RIL_EccNumberSourceMask::SIM;
                ecc_map.insert({ecc->number, ecc});
            }
        }
      }
    }

    /* Extended hardcoded emergency numbers */
    if ( emerg_ind->emer_nums_extended_valid ) {
      for(i = 0; i < emerg_ind->emer_nums_extended_len; i++) {
          auto ecc = std::make_shared<RIL_EmergencyNumber_t>();
          if (ecc != nullptr) {
              auto ecc_num = emerg_ind->emer_nums_extended[i].emer_num;
              auto ecc_num_len = emerg_ind->emer_nums_extended[i].emer_num_len;
              ecc->number = std::string(ecc_num, ecc_num_len);
              QCRIL_LOG_INFO("extended emergency number - %s", ecc->number.c_str());
              if (emerg_ind->emer_nums_extended_cat_valid &&
                  i < emerg_ind->emer_nums_extended_cat_len) {
                ecc->categories = convertToRilCategories(
                    emerg_ind->emer_nums_extended_cat[i]);
                QCRIL_LOG_INFO("extended emergency number categories - %d",
                        ecc->categories);
              }
              else {
                ecc->categories = (uint64_t)RIL_EccCategoryMask::UNKNOWN;
              }
              ecc->sources = RIL_EccNumberSourceMask::DEFAULT;
              ecc_map.insert({ecc->number, ecc});
          }
      }
    }

    /* NV emergency numbers */
    if ( emerg_ind->nv_emer_nums_extended_valid ) {
      for(i = 0; i < emerg_ind->nv_emer_nums_extended_len; i++) {
          auto ecc = std::make_shared<RIL_EmergencyNumber_t>();
          if (ecc != nullptr) {
              auto ecc_num = emerg_ind->nv_emer_nums_extended[i].emer_num;
              auto ecc_num_len = emerg_ind->nv_emer_nums_extended[i].emer_num_len;
              ecc->number = std::string(ecc_num, ecc_num_len);
              QCRIL_LOG_INFO("nv emergency number - %s", ecc->number.c_str());
              if (emerg_ind->nv_emer_nums_extended_cat_valid &&
                  i < emerg_ind->nv_emer_nums_extended_cat_len) {
                ecc->categories = (uint64_t)convertToRilCategories(
                    emerg_ind->nv_emer_nums_extended_cat[i]);
                QCRIL_LOG_INFO("nv emergency number categories - %d",
                        ecc->categories);
              }
              else {
                ecc->categories = (uint64_t)RIL_EccCategoryMask::UNKNOWN;
              }
              ecc->sources = RIL_EccNumberSourceMask::MODEM_CONFIG;
              ecc_map.insert({ecc->number, ecc});
          }
      }
    }

    /* Custom emergency numbers based on mcc */
    if ( emerg_ind->mcc_emer_nums_valid ) {
      /* For mcc numbers, pick numbers based on slot as these are based on specific card.
         For all other emergency numbers, keep legacy behaviour intact */
      uint32_t slot = qmi_ril_get_sim_slot();
      if (slot < QMI_PBM_MAX_NUM_SLOT_V01) {
          for ( i = 0; i < emerg_ind->mcc_emer_nums[slot].emer_nums_len; i++ ) {
              auto ecc = std::make_shared<RIL_EmergencyNumber_t>();
              if (ecc != nullptr) {
                  auto ecc_num = emerg_ind->mcc_emer_nums[slot].emer_nums[i].emer_num;
                  auto ecc_num_len = emerg_ind->mcc_emer_nums[slot].emer_nums[i].emer_num_len;
                  ecc->number = std::string(ecc_num, ecc_num_len);
                  QCRIL_LOG_INFO("mcc emergency number - %s", ecc->number.c_str());
                  ecc->categories = (uint64_t)RIL_EccCategoryMask::UNKNOWN;
                  QCRIL_LOG_INFO("mcc emergency number categories - %d",
                          ecc->categories);
                  ecc->sources = RIL_EccNumberSourceMask::MODEM_CONFIG;
                  ecc_map.insert({ecc->number, ecc});
              }
          }
      }
    }

    PBM_CACHE_LOCK();
    pbm_cache_info.pbm_ecc_map = std::move(ecc_map);
    PBM_CACHE_UNLOCK();

    boolean *ensure_fresh_check = (boolean *)qcril_malloc(sizeof(boolean));
    size_t ensure_fresh_check_len = 0;
    if (ensure_fresh_check)
    {
      *ensure_fresh_check = TRUE;
      ensure_fresh_check_len = sizeof(boolean);
    }
    qcril_event_queue( QCRIL_DEFAULT_INSTANCE_ID,
                   QCRIL_DEFAULT_MODEM_ID,
                   QCRIL_DATA_NOT_ON_STACK,
                   QCRIL_EVT_QMI_RIL_ASSESS_EMRGENCY_NUMBER_LIST_DESIGNATED_COUNTRY,
                   ensure_fresh_check,
                   ensure_fresh_check_len,
                   (RIL_Token) QCRIL_CUSTOM_ECC_INTERNAL_TOKEN );
  }

  QCRIL_LOG_FUNC_RETURN();

}/* qcril_qmi_pbm_emergency_list_ind_hdlr */

/*=========================================================================
  FUNCTION:  qcril_qmi_pbm_unsolicited_indication_cb_helper

===========================================================================*/
/*!
    @brief
    helper function for handling pbm indication

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_pbm_unsolicited_indication_cb_helper
(
    unsigned int   msg_id,
    unsigned char *decoded_payload,
    uint32_t       decoded_payload_len
)
{
    do {
       QCRIL_LOG_INFO("invoked msg 0x%x", (int) msg_id);

       if( !decoded_payload_len )
       {
          // ok, this is a null payload - don't need to process it.
       }
       else
       {
          switch ( msg_id )
          {
            case QMI_PBM_EMERGENCY_LIST_IND_V01:
              qcril_qmi_pbm_emergency_list_ind_hdlr(decoded_payload, decoded_payload_len);
              break;
            case QMI_PBM_ALL_PB_INIT_DONE_IND_V01:
              qcril_qmi_pbm_all_phonebook_init_done_ind_hdlr(decoded_payload, decoded_payload_len);
              break;

            case QMI_PBM_RECORD_READ_EXT_IND_V01:
              qcril_qmi_pbm_record_read_ext_ind_hdlr(decoded_payload,
                                                decoded_payload_len,
                                                QCRIL_DEFAULT_INSTANCE_ID);
              break;

            case QMI_PBM_SIM_INIT_DONE_IND_V01:
              qcril_qmi_pbm_sim_init_done_ind_hdlr(decoded_payload, decoded_payload_len);
              break;
            default:
              QCRIL_LOG_INFO("Unsupported QMI PBM indication %x hex", msg_id);
              break;
          }
       }

    } while(0);

    QCRIL_LOG_FUNC_RETURN();
}//qcril_qmi_pbm_unsolicited_indication_cb_helper

//===========================================================================
//qmi_ril_phone_number_is_emergency
//===========================================================================
int qmi_ril_phone_number_is_emergency(char * number_to_check)
{
  int res = FALSE;

  PBM_CACHE_LOCK();
  auto map = pbm_cache_info.combined_ecc_map;
  PBM_CACHE_UNLOCK();

  if (map.find(number_to_check) != map.end()) {
    res = TRUE;
  }

  QCRIL_LOG_FUNC_RETURN_WITH_RET( res );
  return res;
} // qmi_ril_phone_number_is_emergency

//===========================================================================
//qmi_ril_phone_number_is_non_std_otasp
//===========================================================================
int qmi_ril_phone_number_is_non_std_otasp(const char * number_to_check)
{
  return strncmp(number_to_check, NON_STD_OTASP_NUMBER,strlen(NON_STD_OTASP_NUMBER)+1) ? 0 : 1;
} // qmi_ril_phone_number_is_non_std_otasp

//===========================================================================
//qmi_ril_number_received_from_pbm
//===========================================================================
int qmi_ril_number_received_from_pbm(char * number)
{
  int res = FALSE;
  if (number == nullptr) {
    QCRIL_LOG_INFO("Number is null");
    return res;
  }

  PBM_CACHE_LOCK();
  auto map = pbm_cache_info.pbm_ecc_map;
  PBM_CACHE_UNLOCK();

  if (map.find(number) != map.end()) {
    res = TRUE;
  }

  QCRIL_LOG_INFO("Number reiceved from QMI PBM - %d", res);
  return res;
} // qmi_ril_number_received_from_pbm


//===========================================================================
//get_ecc_property_name
//Returns the ecc list property name in ecc_prop_name for this instance.
//For Default instance and in non multisim targets this is ril.ecclist.
//===========================================================================
void get_ecc_property_name(char *ecc_prop_name)
{
   qcril_instance_id_e_type instance_id = qmi_ril_get_process_instance_id();

   if ( ecc_prop_name == NULL)
   {
       QCRIL_LOG_ERROR("get_ecc_property_name: Invalid argument.");
   }
   else
   {
       if ( instance_id == QCRIL_DEFAULT_INSTANCE_ID )
       {
           snprintf( ecc_prop_name, PROPERTY_NAME_MAX, "%s",QCRIL_ECC_LIST);
       }
       else
       {
           snprintf( ecc_prop_name, PROPERTY_NAME_MAX, "%s%d", QCRIL_ECC_LIST, instance_id);
       }
       QCRIL_LOG_ERROR("ecc list name : %s.", ecc_prop_name);
   }
}

/*=========================================================================
  FUNCTION:  qmi_ril_fill_ecc_map

===========================================================================*/
void qmi_ril_fill_ecc_map
(
    char* ecc_numbers,
    RIL_EccMapType& ecc_map,
    const RIL_EccNumberSourceMask sources,
    const char *mcc,
    const char *mnc
)
{
    size_t param_ecc_len;
    RIL_Errno ret = RIL_E_SUCCESS;
    const char delim[2] = {',',0};
    char *num_parsing_holder, *saveptr, *token;

    QCRIL_LOG_FUNC_ENTRY();

    if (ecc_numbers) {
        param_ecc_len = strlen(ecc_numbers);
        QCRIL_LOG_INFO("param_ecc_len: %d, ecc_numbers: %s", param_ecc_len, ecc_numbers);
        num_parsing_holder = (char*)qcril_malloc(param_ecc_len + 1);
        if (num_parsing_holder) {
            /* not checking return values since there could not be truncation */
            strlcpy(num_parsing_holder, ecc_numbers, param_ecc_len + 1);
            token = strtok_r(num_parsing_holder, delim, &saveptr);
            while (token) {
                auto ecc = std::make_shared<RIL_EmergencyNumber_t>();
                if (ecc != nullptr) {
                    ecc->number = token;
                    ecc->sources = sources;
                    if (mcc != nullptr) {
                        ecc->mcc = mcc;
                    }
                    if (mnc != nullptr) {
                        ecc->mnc = mnc;
                    }
                    QCRIL_LOG_INFO("token: %s, mcc: %s, mnc: %s",
                            token, ecc->mcc.empty() ? " " : ecc->mcc.c_str(),
                            ecc->mnc.empty() ? " " : ecc->mnc.c_str());
                    ecc_map.insert({ecc->number, ecc});
                    QCRIL_LOG_INFO("number: %s, shared_ptr: %p", ecc->number.c_str(), (void*) ecc.get());
                }
                token = strtok_r(NULL, delim, &saveptr);
            }
            qcril_free(num_parsing_holder);
        }
    }
    QCRIL_LOG_FUNC_RETURN_WITH_RET(ret);
}


/*=========================================================================
  FUNCTION:  qmi_ril_send_ecc_list_indication

===========================================================================*/
RIL_Errno qmi_ril_send_ecc_list_indication()
{
    RIL_Errno ret = RIL_E_SUCCESS;
    RIL_EccMapType final_ecc_map;
    RIL_EccMapType ril_ecc_map;

    QCRIL_LOG_FUNC_ENTRY();

    PBM_CACHE_LOCK();
    for (auto elem : pbm_cache_info.pbm_ecc_map) {
        QCRIL_LOG_INFO("copy pbm list to final ecc map - number: %s, shared_ptr: %p",
                elem.first.c_str(), (void*) elem.second.get());
        final_ecc_map.insert(elem);
    }
    PBM_CACHE_UNLOCK();

    qcril_qmi_nas_get_ril_ecc_map(ril_ecc_map);

    for (auto elem : ril_ecc_map) {
        QCRIL_LOG_INFO("copy ril list to final ecc map - number: %s, shared_ptr: %p",
                elem.first.c_str(), (void*) elem.second.get());
        final_ecc_map.insert(elem);
    }

    PBM_CACHE_LOCK();
    // Add alt_emer_num
    if (pbm_cache_info.alt_emer_num_info_valid)
    {
        auto alt_ecc = std::make_shared<RIL_EmergencyNumber_t>();
        alt_ecc->number = pbm_cache_info.alt_emer_num_info.number;
        final_ecc_map.insert({alt_ecc->number, alt_ecc});
    }
    pbm_cache_info.combined_ecc_map = final_ecc_map;
    PBM_CACHE_UNLOCK();

    std::vector<std::shared_ptr<RIL_EmergencyNumber_t>> ecc_list;
    for (auto val : final_ecc_map) {
        ecc_list.push_back(val.second);
    }

    if (ecc_list.size() != 0) {
        auto msg = std::make_shared<RilUnsolEmergencyListIndMessage>(std::move(ecc_list));
        if (msg != nullptr) {
            Dispatcher::getInstance().dispatchSync(msg);
        } else {
            ret = RIL_E_NO_MEMORY;
        }
    }

    // For backward compatibility set ril.ecclist as well.
    qmi_ril_set_ecc_property();

    QCRIL_LOG_FUNC_RETURN_WITH_RET(ret);
    return ret;
}


/*=========================================================================
  FUNCTION:  qmi_ril_set_ecc_property

===========================================================================*/
/*!
    @brief
    Combine param "ecc_bumbers" and "numbers in PBM cache" into ECC property

    @params
    - ecc_numbers: ecc numbers delimited with comma, ended with '\0'. It could
    be *NULL*.

    @return
    0 if success. Other value if failure
*/
/*=========================================================================*/

int qmi_ril_set_ecc_property()
{
    char ecc_prop_name[PROPERTY_NAME_MAX];
    char ecc_prop_val[PROPERTY_VALUE_MAX];
    size_t len = 0, val_index = 0;
    int ret, no_space = 0;
    bool first_entry = true;
    RIL_EccMapType map;

    QCRIL_LOG_FUNC_ENTRY();

    /* get the right prop name for current RIL instance */
    get_ecc_property_name(ecc_prop_name);
    memset(ecc_prop_val, 0, PROPERTY_VALUE_MAX);

    /* copy numbers from PBM IND */
    PBM_CACHE_LOCK();
    map = pbm_cache_info.combined_ecc_map;
    PBM_CACHE_UNLOCK();

    for (auto elem : map) {
        len = elem.first.length();
        // We need space for "," and emergency number if its not first entry
        if (!first_entry) {
            len++;
        }

        if ((len) > (PROPERTY_VALUE_MAX - val_index)) {
            no_space = 1;
            break;
        } else {
            // This avoid situations like ending up with "," at the end.
            if (!first_entry) {
                strlcat(ecc_prop_val, ",", PROPERTY_VALUE_MAX);
            }
            strlcat(ecc_prop_val, elem.first.c_str(), PROPERTY_VALUE_MAX);
            first_entry = false;
        }
        val_index += len;
    }

    if (no_space)
    {
        QCRIL_LOG_ERROR("The ecc numbers exceeds the property length, abandon some of them");
    }
    QCRIL_LOG_INFO("new saved %s: %s", ecc_prop_name, ecc_prop_val);

    /* set the property */
    if ((ret = property_set(ecc_prop_name, ecc_prop_val)) != E_SUCCESS)
    {
        QCRIL_LOG_FATAL("Failed to save %s to system property", ecc_prop_name);
    }

    QCRIL_LOG_FUNC_RETURN_WITH_RET(ret);

    return ret;
}

RIL_Errno qcril_qmi_pbm_enable_phonebook_ready_indications(int enable)
{
    RIL_Errno ril_req_res = RIL_E_GENERIC_FAILURE;
    qmi_client_error_type qmi_transport_error;

    pbm_indication_register_req_msg_v01  indication_req;
    pbm_indication_register_resp_msg_v01 indication_resp;

    QCRIL_LOG_INFO("entered %d",enable);

    memset(&indication_req, 0, sizeof(indication_req));
    memset(&indication_resp, 0, sizeof(indication_resp));

    PBM_CACHE_LOCK();
    if( TRUE == enable )
    {/* Register for phonebook ready indications */
        pbm_cache_info.pbm_reg_mask |= PBM_REG_RECORD_UPDATE_EVENTS_V01;
        pbm_cache_info.pbm_reg_mask |= PBM_REG_PHONEBOOK_READY_EVENTS_V01;
    }
    else
    {/* Suppress phonebook ready indications */
        pbm_cache_info.pbm_reg_mask &= ~PBM_REG_RECORD_UPDATE_EVENTS_V01;
        pbm_cache_info.pbm_reg_mask &= ~PBM_REG_PHONEBOOK_READY_EVENTS_V01;
    }
    indication_req.reg_mask = pbm_cache_info.pbm_reg_mask;
    PBM_CACHE_UNLOCK();

    qmi_transport_error = qmi_client_pbm_send_sync( QMI_PBM_INDICATION_REGISTER_REQ_V01,
                                        &indication_req,
                                        sizeof(indication_req),
                                        &indication_resp,
                                        sizeof(indication_resp));

    ril_req_res = qcril_qmi_util_convert_qmi_response_codes_to_ril_result( qmi_transport_error,
                                                                        &indication_resp.resp );

    if ( RIL_E_SUCCESS == ril_req_res )
    {
        QCRIL_LOG_INFO("Indication register successful with mask %d", indication_resp.reg_mask);
    }
    else
    {
        QCRIL_LOG_INFO("Indication register failed, error %d", ril_req_res);
    }

    return ril_req_res;
}


/*=========================================================================
  FUNCTION:  qcril_qmi_pbm_all_phonebook_init_done_ind_hdlr

===========================================================================*/
/*!
    @brief
    Handle QMI_PBM_ALL_PB_INIT_DONE_IND_V01.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_pbm_all_phonebook_init_done_ind_hdlr
(
  void *ind_data_ptr,
  uint32 ind_data_len
)
{
    pbm_all_pb_init_done_ind_msg_v01 *all_pb_init_done_ind;

    QCRIL_LOG_FUNC_ENTRY();

    QCRIL_NOTUSED( ind_data_len );

    if ( ind_data_ptr != NULL )
    {
        all_pb_init_done_ind = (pbm_all_pb_init_done_ind_msg_v01*)ind_data_ptr;

        uint16_t pb_type = all_pb_init_done_ind->phonebook_ready_info[0].pb_bit_mask;
        pbm_session_type_enum_v01 session_type =
                            all_pb_init_done_ind->phonebook_ready_info[0].session_type;

        if( all_pb_init_done_ind->phonebook_ready_info_len > 0 && (pb_type & PBM_PB_TYPE_ADN_V01) ) {
            QCRIL_LOG_INFO("Pb type is ADN, session type is %d", session_type);
        }
    }

    QCRIL_LOG_FUNC_RETURN();
}

/*=========================================================================
  FUNCTION:  qcril_qmi_pbm_sim_init_done_ind_hdlr

===========================================================================*/
/*!
    @brief
    Handle QMI_PBM_SIM_INIT_DONE_IND_V01.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_pbm_sim_init_done_ind_hdlr
(
  void *ind_data_ptr,
  uint32 ind_data_len
)
{
    pbm_sim_init_done_ind_msg_v01 *sim_init_done_ind;

    QCRIL_LOG_FUNC_ENTRY();

    QCRIL_NOTUSED( ind_data_len );

    if ( ind_data_ptr != NULL )
    {
        sim_init_done_ind = (pbm_sim_init_done_ind_msg_v01*)ind_data_ptr;

        uint8 slot_id = (uint8)sim_init_done_ind->slot_id;

        if( slot_id == qmi_ril_get_sim_slot() + 1 ) {

            QCRIL_LOG_INFO("All Pb is init completed");

            (void)qcril_event_queue(QCRIL_DEFAULT_INSTANCE_ID,
                                QCRIL_DEFAULT_MODEM_ID,
                                QCRIL_DATA_ON_STACK,
                                QCRIL_EVT_PBM_GET_PB_CAPACITY,
                                NULL,
                                0,
                                (RIL_Token) QCRIL_TOKEN_ID_INTERNAL);
        }
    }

    QCRIL_LOG_FUNC_RETURN();
}

/*=========================================================================
  FUNCTION:  qcril_qmi_pbm_get_pb_capacity_hndlr

===========================================================================*/
/*!
    @brief
    Handle QCRIL_EVT_PBM_GET_PB_CAPACITY.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_pbm_get_pb_capacity_hndlr
(
    const qcril_request_params_type *const params_ptr,
    qcril_request_return_type *const ret_ptr
)
{
    RIL_Errno result = RIL_E_GENERIC_FAILURE;
    int session_idx = 0;

    QCRIL_NOTUSED(ret_ptr);
    QCRIL_NOTUSED(params_ptr);

    QCRIL_LOG_FUNC_ENTRY();

    PBM_CACHE_LOCK();
    for(session_idx=0; session_idx<(int)pbm_cache_info.pbm_num_session; session_idx++) {
        pbm_cache_info.pbm_session_type[session_idx] = 0;
        memset(&pbm_cache_info.pbm_capacity[session_idx], 0,
                                    sizeof(qcril::interfaces::qcril_pbm_adn_count_info));
    }
    pbm_cache_info.pbm_num_session = 0;
    pbm_cache_info.pbm_status = QCRIL_PBM_STATUS_REGISTER_READY;
    PBM_CACHE_UNLOCK();

    result = qcril_qmi_pbm_get_records_count();

    QCRIL_LOG_DEBUG("result: %d", result);

    if(result == RIL_E_SUCCESS
        && pbm_cache_info.pbm_status == QCRIL_PBM_STATUS_GET_CAPACITY_COMPLETE)
    {
        /* Send unsolicited response to framework */
        auto msg = std::make_shared<QcRilUnsolAdnInitDoneMessage>();
        if (msg != nullptr)
        {
          msg->broadcast();
        }
    }
    QCRIL_LOG_FUNC_RETURN();
}

/* This function is to get the records count of the PB via QMI PBM */
RIL_Errno qcril_qmi_pbm_get_records_count(void)
{
    pbm_get_all_pb_capabilities_resp_msg_v01 pbm_info_resp;
    int session_idx, pbset_idx, caps_idx;
    qmi_client_error_type            qmi_error;
    RIL_Errno                        result = RIL_E_GENERIC_FAILURE;
    int max_records = 0, used_records = 0, max_email = 0,
        used_email = 0, max_ad_num = 0, used_ad_num = 0,
        max_name_len = 0, max_number_len = 0, max_email_len = 0,
        max_anr_len = 0;

    QCRIL_LOG_FUNC_ENTRY();

    memset(&pbm_info_resp,0,sizeof(pbm_info_resp));

    qmi_error = qmi_client_pbm_send_sync( QMI_PBM_GET_ALL_PB_CAPABILITIES_REQ_V01,
                        NULL,
                        NAS_NIL,
                        &pbm_info_resp,
                        sizeof(pbm_info_resp));

    result = qcril_qmi_util_convert_qmi_response_codes_to_ril_result(qmi_error,
                                                            &pbm_info_resp.resp);

    if ( RIL_E_SUCCESS == result )
    {
        QCRIL_LOG_INFO("Get pb capabilities successfully");
        if (pbm_info_resp.pb_set_info_array_valid)
        {
          for(session_idx= 0; session_idx < (int)pbm_info_resp.pb_set_info_array_len; session_idx++)
          {
            int session_type = (int)pbm_info_resp.pb_set_info_array[session_idx].session_type;

            pbm_pbr_cap_info_type_v01 *pbset_info_ptr =
                                & pbm_info_resp.pb_set_info_array[session_idx].pb_set_info;
            max_records = used_records =  max_email = used_email = max_ad_num = used_ad_num = 0;
            max_name_len = max_number_len = max_email_len = max_anr_len = 0;
            for (pbset_idx= 0; pbset_idx < (int)pbset_info_ptr->num_of_pb_sets; pbset_idx++)
            {
                pbm_pb_set_cap_info_type_v01 * pbset_caps_ptr =
                                    & pbset_info_ptr->pbset_caps[pbset_idx];
                max_records += pbset_caps_ptr->adn_caps.max_adn_records;
                used_records += pbset_caps_ptr->adn_caps.used_adn_records;
                max_name_len = MAX(max_name_len, pbset_caps_ptr->adn_caps.max_adn_name_len);
                max_number_len = MAX(max_number_len, pbset_caps_ptr->adn_caps.max_adn_num_len);
                for (caps_idx= 0; caps_idx < (int)pbset_caps_ptr->email_caps_len; caps_idx++)
                {
                     max_email += pbset_caps_ptr->email_caps[caps_idx].record_caps.max_records;
                     used_email += pbset_caps_ptr->email_caps[caps_idx].record_caps.used_records;
                     max_email_len = MAX(max_email_len, pbset_caps_ptr->email_caps[caps_idx].record_caps.max_record_len);
                }
                for (caps_idx= 0; caps_idx < (int)pbset_caps_ptr->ad_num_caps_len; caps_idx++)
                {
                     max_ad_num += pbset_caps_ptr->ad_num_caps[caps_idx].record_caps.max_records;
                     used_ad_num += pbset_caps_ptr->ad_num_caps[caps_idx].record_caps.used_records;
                     max_anr_len = MAX(max_anr_len,  pbset_caps_ptr->ad_num_caps[caps_idx].record_caps.max_record_len);
                }
          }

          //int pb_type = (int) pbm_info_resp.capability_basic_info.pb_type;
          QCRIL_LOG_INFO("Session type: %d", session_type);

          QCRIL_LOG_INFO("max records: %d, valid records: %d", max_records, used_records);
          QCRIL_LOG_INFO("max email: %d, valid email: %d, max ad number: %d, valid ad number: %d",
                        max_email, used_email, max_ad_num, used_ad_num);
          QCRIL_LOG_INFO("max name len: %d, max number len: %d, max email len: %d, max anr len: %d",
                        max_name_len, max_number_len, max_email_len, max_anr_len);

          if(max_records > 0)
          {
            PBM_CACHE_LOCK();
            pbm_cache_info.pbm_session_type[pbm_cache_info.pbm_num_session] = session_type;
            pbm_cache_info.pbm_capacity[pbm_cache_info.pbm_num_session].max_adn_num= max_records;
            pbm_cache_info.pbm_capacity[pbm_cache_info.pbm_num_session].valid_adn_num= used_records;
            pbm_cache_info.pbm_capacity[pbm_cache_info.pbm_num_session].max_email_num= max_email;
            pbm_cache_info.pbm_capacity[pbm_cache_info.pbm_num_session].valid_email_num= used_email;
            pbm_cache_info.pbm_capacity[pbm_cache_info.pbm_num_session].max_ad_num= max_ad_num;
            pbm_cache_info.pbm_capacity[pbm_cache_info.pbm_num_session].valid_ad_num= used_ad_num;
            pbm_cache_info.pbm_capacity[pbm_cache_info.pbm_num_session].max_name_len= max_name_len;
            pbm_cache_info.pbm_capacity[pbm_cache_info.pbm_num_session].max_number_len= max_number_len;
            pbm_cache_info.pbm_capacity[pbm_cache_info.pbm_num_session].max_email_len= max_email_len;
            pbm_cache_info.pbm_capacity[pbm_cache_info.pbm_num_session].max_anr_len= max_anr_len;
            pbm_cache_info.pbm_num_session ++;
            pbm_cache_info.pbm_status = QCRIL_PBM_STATUS_GET_CAPACITY_COMPLETE;
            PBM_CACHE_UNLOCK();
          }
        }
      }
    }
    else
    {
        QCRIL_LOG_INFO("Get pb capabilities failed, error %d", result);
    }
    return result;

    QCRIL_LOG_FUNC_RETURN();
}

/*=========================================================================
  FUNCTION:  qcril_qmi_pbm_get_adn_record_hndlr

===========================================================================*/
/*!
    @brief
    Handle QCRIL_EVT_HOOK_GET_ADN_RECORD.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_pbm_get_adn_record_hndlr
(
    std::shared_ptr<QcRilRequestGetAdnRecordMessage> msg
)
{
    RIL_Errno                             ril_req_res = RIL_E_SUCCESS;
    qcril::interfaces::qcril_pbm_adn_count_info      adn_count_info;
    int i;

    QCRIL_LOG_FUNC_ENTRY();

    memset(&adn_count_info, 0, sizeof(qcril::interfaces::qcril_pbm_adn_count_info));

    for ( i = 0; i < pbm_cache_info.pbm_num_session; i++ )
    {
        int adn_count = pbm_cache_info.pbm_capacity[i].max_adn_num;
        if(adn_count > 0) {
            ril_req_res = qcril_qmi_pbm_get_all_of_records(adn_count,
                                                pbm_cache_info.pbm_session_type[i]);
        } else {
            QCRIL_LOG_INFO("The ADN file did not exist on this session: ",
                                                pbm_cache_info.pbm_session_type[i]);
        }
        adn_count_info.max_adn_num += pbm_cache_info.pbm_capacity[i].max_adn_num;
        adn_count_info.valid_adn_num += pbm_cache_info.pbm_capacity[i].valid_adn_num;
        adn_count_info.max_email_num += pbm_cache_info.pbm_capacity[i].max_email_num;
        adn_count_info.valid_email_num += pbm_cache_info.pbm_capacity[i].valid_email_num;
        adn_count_info.max_ad_num += pbm_cache_info.pbm_capacity[i].max_ad_num;
        adn_count_info.valid_ad_num += pbm_cache_info.pbm_capacity[i].valid_ad_num;
        adn_count_info.max_name_len = MAX(adn_count_info.max_name_len, pbm_cache_info.pbm_capacity[i].max_name_len);
        adn_count_info.max_number_len = MAX(adn_count_info.max_number_len, pbm_cache_info.pbm_capacity[i].max_number_len);
        adn_count_info.max_email_len = MAX(adn_count_info.max_email_len, pbm_cache_info.pbm_capacity[i].max_email_len);
        adn_count_info.max_anr_len = MAX(adn_count_info.max_anr_len, pbm_cache_info.pbm_capacity[i].max_anr_len);
    }

    QCRIL_LOG_INFO("For all active session, the max records is %d, the valid records is %d",
                                        adn_count_info.max_adn_num, adn_count_info.valid_adn_num);
    QCRIL_LOG_INFO("For all active session, the max email is %d, the max ad num is %d",
                                        adn_count_info.max_email_num, adn_count_info.max_ad_num);
    QCRIL_LOG_INFO("For all active session, the max name length is %d, the max number length is %d",
                                        adn_count_info.max_name_len, adn_count_info.max_number_len);
    QCRIL_LOG_INFO("For all active session, the max email length is %d, the max anr length is %d",
                                        adn_count_info.max_email_len, adn_count_info.max_anr_len);

    auto countInfoResp = std::make_shared<qcril::interfaces::AdnCountInfoResp>();
    countInfoResp->setAdnCountInfo(adn_count_info);
    std::shared_ptr<QcRilRequestMessageCallbackPayload> resp =
          std::make_shared<QcRilRequestMessageCallbackPayload>(ril_req_res, countInfoResp);
    msg->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);

    QCRIL_LOG_FUNC_RETURN();
}

RIL_Errno qcril_qmi_pbm_get_all_of_records(int adn_count, int session_type)
{
    RIL_Errno ril_req_res = RIL_E_GENERIC_FAILURE;
    qmi_client_error_type qmi_transport_error;

    pbm_read_records_ext_req_msg_v01  pbm_read_req;
    pbm_read_records_ext_resp_msg_v01 pbm_read_resp;
    int index;

    QCRIL_LOG_FUNC_ENTRY();

    memset(&pbm_read_req, 0, sizeof(pbm_read_req));
    memset(&pbm_read_resp, 0, sizeof(pbm_read_resp));

    pbm_read_req.record_info.pb_type = PBM_PB_TYPE_ADN_V01;
    pbm_read_req.record_info.session_type = (pbm_session_type_enum_v01)session_type;
    pbm_read_req.record_info.record_start_id = 1;
    pbm_read_req.record_info.record_end_id = adn_count;

    QCRIL_LOG_INFO("Start ID is %d, end ID is %d",
                pbm_read_req.record_info.record_start_id,  pbm_read_req.record_info.record_end_id);
    QCRIL_LOG_INFO("Pb type is %d, session type is %d",
                pbm_read_req.record_info.pb_type,  pbm_read_req.record_info.session_type);

    qmi_transport_error = qmi_client_pbm_send_sync( QMI_PBM_READ_RECORDS_EXT_REQ_V01,
                                        &pbm_read_req,
                                        sizeof(pbm_read_req),
                                        &pbm_read_resp,
                                        sizeof(pbm_read_resp));

    ril_req_res = qcril_qmi_util_convert_qmi_response_codes_to_ril_result( qmi_transport_error,
                                                                        &pbm_read_resp.resp );

    if ( RIL_E_SUCCESS == ril_req_res && pbm_read_resp.num_of_recs_valid )
    {
        int used_records = pbm_read_resp.num_of_recs;
        if (used_records == 0)
        {
            QCRIL_LOG_INFO("There are no entries filled in ADN, nothing to read");
        }
        else
        {
            QCRIL_LOG_INFO("There are %d filled entries in ADN", used_records);
            for ( index = 0; index < pbm_cache_info.pbm_num_session; index++ )
            {
                if(session_type == pbm_cache_info.pbm_session_type[index]) {
                    PBM_CACHE_LOCK();
                    pbm_cache_info.pbm_status = QCRIL_PBM_STATUS_READING_RECORD;
                    pbm_cache_info.pbm_capacity[index].valid_adn_num = used_records;
                    PBM_CACHE_UNLOCK();
                    break;
                 }
            }
        }
    }
    else
    {
        QCRIL_LOG_INFO("Read request for ADN failed with error code = %d",
                                            (int)pbm_read_resp.resp.error);
    }

    return ril_req_res;
    QCRIL_LOG_FUNC_RETURN();
}

char* qcril_qmi_pbm_alloc_to_utf8
(
  const uint8*  buffer_ptr,
  uint16        buffer_size,
  boolean       is_ucs2
)
{
  char*  out_ptr    = NULL;
  uint16 string_len = 0;

  if(buffer_ptr == NULL || buffer_size == 0)
  {
    QCRIL_LOG_INFO("Original buffer is NULL pointer");
    return NULL;
  }

  if(is_ucs2)
  {
      buffer_size <<= 1;
  }

  string_len = (buffer_size << 1) + sizeof(char);

  out_ptr = (char*)qcril_malloc(string_len);
  if (out_ptr != NULL)
  {
    memset(out_ptr, 0, string_len);
    if(is_ucs2)
    {
        qcril_cm_ss_convert_ucs2_to_utf8((char*) buffer_ptr,
                                            buffer_size,
                                            out_ptr,
                                            string_len);
    }
    else
    {
        qcril_cm_ss_ascii_to_utf8((unsigned char*) buffer_ptr, buffer_size, out_ptr, string_len);
    }
  }
  else
  {
      QCRIL_LOG_INFO("Failed to allocate buffer for UTF-8 output");
  }

  return out_ptr;
}

#if 0
void qcril_qmi_pbm_free_adn_record_info
(
  AdnRecords*  adn_record_ptr
)
{
  int index, j = 0;

  if (adn_record_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "Invalid input, cannot proceed");
    QCRIL_ASSERT(0);
    return;
  }

  QCRIL_LOG_INFO( "%s\n", "Freeing ADN record info buffers" );

  /* Loop through all the apps and free buffers allocated */
  for (index = 0; index < adn_record_ptr->record_elements; index++)
  {
    if(adn_record_ptr->adn_record_info[index].name != NULL)
    {
      qcril_free(adn_record_ptr->adn_record_info[index].name);
      adn_record_ptr->adn_record_info[index].name = NULL;
      QCRIL_LOG_DEBUG("Freed Name pointer, record[%d]", index);
    }

    if(adn_record_ptr->adn_record_info[index].number != NULL)
    {
      qcril_free(adn_record_ptr->adn_record_info[index].number);
      adn_record_ptr->adn_record_info[index].number = NULL;
      QCRIL_LOG_DEBUG("Freed Number pointer, record[%d]", index);
    }

    if(adn_record_ptr->adn_record_info[index].email_elements > 0)
    {
      for(j = 0; j < adn_record_ptr->adn_record_info[index].email_elements; j++)
      {
          qcril_free(adn_record_ptr->adn_record_info[index].email[j]);
          adn_record_ptr->adn_record_info[index].email[j] = NULL;
          QCRIL_LOG_DEBUG("Freed Email pointer, index[%d], record[%d]", j, index);
      }
    }

    if(adn_record_ptr->adn_record_info[index].anr_elements > 0)
    {
      for(j = 0; j < adn_record_ptr->adn_record_info[index].anr_elements; j++)
      {
          qcril_free(adn_record_ptr->adn_record_info[index].ad_number[j]);
          adn_record_ptr->adn_record_info[index].ad_number[j] = NULL;
          QCRIL_LOG_DEBUG("Freed AdNumber pointer, index[%d], record[%d]", j, index);
      }
    }
  }
}
#endif

/*=========================================================================
  FUNCTION:  qcril_qmi_pbm_record_read_ext_ind_hdlr

===========================================================================*/
/*!
    @brief
    Handle QMI_PBM_RECORD_READ_EXT_IND_V01.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_pbm_record_read_ext_ind_hdlr
(
  void *ind_data_ptr,
  uint32 ind_data_len,
  qcril_instance_id_e_type   instance_id
)
{
    unsigned int array_index = 0;
    int m_element_index, element_index, session_start_offset = 0;

    pbm_record_read_ext_ind_msg_v01 *pbm_read_ind;
    qcril::interfaces::AdnRecords*    adn_records_ptr = new qcril::interfaces::AdnRecords();

    QCRIL_LOG_FUNC_ENTRY();

    QCRIL_NOTUSED( ind_data_len );
    QCRIL_NOTUSED( instance_id );

    if ( ind_data_ptr != NULL )
    {
        pbm_read_ind = (pbm_record_read_ext_ind_msg_v01*)ind_data_ptr;

        //session type
        QCRIL_LOG_INFO("Session type of record in ADN is: %d",
                            (int)pbm_read_ind->basic_record_data.session_type);

        //record count
        adn_records_ptr->record_elements = pbm_read_ind->basic_record_data.record_instances_len;
        QCRIL_LOG_INFO("Number of ADN record in the read indication: %d",
                                                adn_records_ptr->record_elements);

        //support multi-session
        for ( element_index = 0; element_index < pbm_cache_info.pbm_num_session; element_index++ )
        {
            if(pbm_read_ind->basic_record_data.session_type == pbm_cache_info.pbm_session_type[element_index])  {
                for ( m_element_index = 0; element_index>0 && m_element_index < element_index; m_element_index++ ) {
                  session_start_offset += pbm_cache_info.pbm_capacity[m_element_index].max_adn_num;
                }
                break;
             }
        }

        QCRIL_LOG_INFO("The offset of ADN record for this session is: %d", session_start_offset);

        //basic adn record info
        for (element_index = 0; element_index < adn_records_ptr->record_elements; element_index++)
        {
            qcril::interfaces::AdnRecordInfo* adn_resp_ptr = & adn_records_ptr->adn_record_info[element_index];
            pbm_record_instance_ext_type_v01 basic_record_info =
                                pbm_read_ind->basic_record_data.record_instances[element_index];

            //record id
            adn_resp_ptr->record_id = basic_record_info.record_id + session_start_offset;
            QCRIL_LOG_INFO("Index of record in ADN: %d", adn_resp_ptr->record_id);

            //name
            char* name = qcril_qmi_pbm_alloc_to_utf8(
                    (const uint8*) basic_record_info.name,
                    basic_record_info.name_len,
                    TRUE);
            if(name != NULL)
            {
              adn_resp_ptr->name = std::string(static_cast<const char*>(name));
              QCRIL_LOG_INFO("Name in the record: %s", adn_resp_ptr->name.c_str());
            }
            else
            {
              QCRIL_LOG_INFO("Name in the record is NULL");
            }

            //number
            char* number = qcril_qmi_pbm_alloc_to_utf8(
                      (const uint8*) basic_record_info.number,
                      basic_record_info.number_len,
                      FALSE);
            if(number != NULL)
            {
              adn_resp_ptr->number = std::string(static_cast<const char*>(number));
              QCRIL_LOG_INFO("Number in the record: %s", adn_resp_ptr->number.c_str());
              QCRIL_LOG_INFO("Number type in the record: %d", (int)basic_record_info.num_type);
              QCRIL_LOG_INFO("Number plan in the record: %d", (int)basic_record_info.num_plan);
            }
            else
            {
              QCRIL_LOG_INFO("Number in the record is NULL");
            }
        }

        //email info
        if(pbm_read_ind->email_info_array_valid && pbm_read_ind->email_info_array_len > 0)
        {
            for (array_index = 0; array_index < pbm_read_ind->email_info_array_len; array_index++)
            {
                pbm_email_record_info_ext_type_v01* email_info_ptr =
                                                        & pbm_read_ind->email_info_array[array_index];
                QCRIL_LOG_INFO("email_info_array_len is : %d", pbm_read_ind->email_info_array_len);

                //match the record id
                for(element_index = 0; element_index < adn_records_ptr->record_elements; element_index++)
                {
                    if(adn_records_ptr->adn_record_info[element_index].record_id == email_info_ptr->record_id + session_start_offset)
                    {
                         //support multi emails
                        adn_records_ptr->adn_record_info[element_index].email_elements =
                                                            email_info_ptr->email_len;
                        QCRIL_LOG_INFO("email record id is : %d", email_info_ptr->record_id);
                        QCRIL_LOG_INFO("email_elements is : %d",
                                                adn_records_ptr->adn_record_info[element_index].email_elements);
                        for(m_element_index = 0; m_element_index < adn_records_ptr->adn_record_info[element_index].email_elements; m_element_index++)
                        {
                            char* email = qcril_qmi_pbm_alloc_to_utf8(
                                      (const uint8*) email_info_ptr->email[m_element_index].email_address,
                                      email_info_ptr->email[m_element_index].email_address_len,
                                      TRUE);
                            if(email != NULL)
                            {
                              adn_records_ptr->adn_record_info[element_index].email[m_element_index] =
                                            std::string(static_cast<const char*>(email));
                              QCRIL_LOG_INFO("Email in the record : %s",
                                                    email_info_ptr->email[m_element_index].email_address);
                            }
                            else
                            {
                              QCRIL_LOG_INFO("Email in the record is NULL");
                            }
                        }
                        break;
                    }
                }
            }
        } else {
            QCRIL_LOG_INFO("This group of ADN records unsupport or exclude the email");
        }

        //additional number info
        if(pbm_read_ind->ad_num_info_array_valid && pbm_read_ind->ad_num_info_array_len > 0)
        {
            for (array_index = 0; array_index < pbm_read_ind->ad_num_info_array_len; array_index++)
            {
                pbm_ad_num_record_info_type_v01* ad_num_info_ptr = & pbm_read_ind->ad_num_info_array[array_index];
                QCRIL_LOG_INFO("ad_num_info_array_len is : %d", pbm_read_ind->ad_num_info_array_len);

                //match the record id
                for(element_index = 0; element_index < adn_records_ptr->record_elements; element_index++)
                {
                    if(adn_records_ptr->adn_record_info[element_index].record_id == ad_num_info_ptr->record_id + session_start_offset)
                    {
                         //support multi additional numbers
                        adn_records_ptr->adn_record_info[element_index].anr_elements = ad_num_info_ptr->ad_nums_len;
                        QCRIL_LOG_INFO("anr record id is : %d", ad_num_info_ptr->record_id);
                        QCRIL_LOG_INFO("anr_elements is : %d",
                                                adn_records_ptr->adn_record_info[element_index].anr_elements);
                        for(m_element_index = 0; m_element_index < adn_records_ptr->adn_record_info[element_index].anr_elements; m_element_index++)
                        {

                            char* ad_number = qcril_qmi_pbm_alloc_to_utf8(
                                          (const uint8*) ad_num_info_ptr->ad_nums[m_element_index].ad_number,
                                          ad_num_info_ptr->ad_nums[m_element_index].ad_number_len,
                                          FALSE);
                            if(ad_number != NULL)
                            {
                              adn_records_ptr->adn_record_info[element_index].ad_number[m_element_index] =
                                            std::string(static_cast<const char*>(ad_number));
                              QCRIL_LOG_INFO("AdNumber in the record : %s",
                                                    ad_num_info_ptr->ad_nums[m_element_index].ad_number);
                              QCRIL_LOG_INFO("Number type of AdNumber in the record: %d", (int)ad_num_info_ptr->ad_nums[m_element_index].num_type);
                              QCRIL_LOG_INFO("Number plan of AdNumber in the record: %d", (int)ad_num_info_ptr->ad_nums[m_element_index].num_plan);
                            }
                            else
                            {
                              QCRIL_LOG_INFO("AdNumber in the record is NULL");
                            }
                        }
                        break;
                    }
                }
            }
        } else {
            QCRIL_LOG_INFO("This group of ADN records unsupport or exclude the additional number");
        }

        /* Send unsolicited response to framework */
        auto msg = std::make_shared<QcRilUnsolAdnRecordsOnSimMessage>(*adn_records_ptr);
        if (msg != nullptr)
        {
          msg->broadcast();
        }

        delete adn_records_ptr;
        adn_records_ptr = nullptr;
        //qcril_qmi_pbm_free_adn_record_info(adn_records_ptr);
    }

    QCRIL_LOG_FUNC_RETURN();
}


/*=========================================================================
  FUNCTION:  qcril_qmi_pbm_update_adn_record_hndlr

===========================================================================*/
/*!
    @brief
    Handle QCRIL_EVT_HOOK_UPDATE_ADN_RECORD.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_pbm_update_adn_record_hndlr
(
    std::shared_ptr<QcRilRequestUpdateAdnRecordMessage> msg
)
{
    RIL_Errno                             ril_req_res = RIL_E_GENERIC_FAILURE;
    const            qcril::interfaces::AdnRecordInfo* in_ptr = &msg->getRecordData();
    int              record_id = 0, index = 0, sum = 0;

    QCRIL_LOG_FUNC_ENTRY();

    auto ret = getPbmModule().getPendingMessageList().insert(msg);
    if(ret.second)
    {
        record_id = in_ptr->record_id;
        //support multi-session
        if(record_id == 0) {
            //select the correct session type to add ADN
            for ( index = 0; index < pbm_cache_info.pbm_num_session; index++ )
            {
                if(pbm_cache_info.pbm_capacity[index].valid_adn_num
                        < pbm_cache_info.pbm_capacity[index].max_adn_num)
                    break;
            }
        }
        else if (record_id > 0)
        {
            //select the correct session type to delete/modify ADN
            for ( index = 0; index < pbm_cache_info.pbm_num_session; index++ )
            {
                sum += pbm_cache_info.pbm_capacity[index].max_adn_num;

                QCRIL_LOG_INFO("Sum is %d, index is %d", sum, index);
                if(record_id <= sum){
                    record_id -=  sum - pbm_cache_info.pbm_capacity[index].max_adn_num;
                    break;
                }
            }
        }

        if(index < pbm_cache_info.pbm_num_session) {
            if((in_ptr->name.empty() || strlen(in_ptr->name.c_str()) == 0)
            && (in_ptr->number.empty() || strlen(in_ptr->number.c_str()) == 0)
            && in_ptr->email_elements == 0 && in_ptr->anr_elements == 0)
            {
                ril_req_res = qcril_qmi_pbm_delete_adn_record(record_id,
                                        pbm_cache_info.pbm_session_type[index],
                                        ret.first);
            }
            else
            {
                ril_req_res = qcril_qmi_pbm_add_or_modify_adn_record(in_ptr,
                                        record_id,
                                        pbm_cache_info.pbm_session_type[index],
                                        ret.first);
            }
        }
        else
        {
            QCRIL_LOG_INFO("Could not find the correct session type");
        }
    }
    else
    {
        QCRIL_LOG_ERROR("Failed to insert to PendingList for update ADN record.");
        ril_req_res = RIL_E_INTERNAL_ERR;
    }

    if(RIL_E_SUCCESS != ril_req_res)
    {
      QCRIL_LOG_INFO("Update request for ADN failed");

      getPbmModule().getPendingMessageList().erase(ret.first);

      auto updatedResp = std::make_shared<qcril::interfaces::AdnRecordUpdatedResp>();
      updatedResp->setRecordIndex(record_id);
      std::shared_ptr<QcRilRequestMessageCallbackPayload> resp =
            std::make_shared<QcRilRequestMessageCallbackPayload>(ril_req_res, updatedResp);
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
    }
    QCRIL_LOG_FUNC_RETURN();
}

/*=========================================================================
  FUNCTION:  qcril_qmi_pbm_async_cb_helper_hndlr

===========================================================================*/
void qcril_qmi_pbm_async_cb_helper_hndlr
(
  unsigned int                 msg_id,
  std::shared_ptr<void>        resp_c_struct,
  unsigned int                 resp_c_struct_len,
  void                         *resp_cb_data,
  qmi_client_error_type        transp_err
)
{
  RIL_Errno ril_req_res = RIL_E_GENERIC_FAILURE;
  qmi_resp_callback_type qmi_resp_callback;
  int session_type, token_id, record_id = 0;
  int j, k, session_start_offset = 0, index = 0;

  QCRIL_LOG_FUNC_ENTRY();

  qmi_resp_callback.msg_id = msg_id;
  qmi_resp_callback.data_buf = (void*) resp_c_struct.get();
  qmi_resp_callback.data_buf_len = resp_c_struct_len;
  qmi_resp_callback.cb_data = resp_cb_data;
  qmi_resp_callback.transp_err = transp_err;

  if(qmi_resp_callback.data_buf == NULL)
  {
    QCRIL_LOG_ERROR("%s", "NULL pointer");
    QCRIL_ASSERT(0);
    return;
  }

  if(qmi_resp_callback.cb_data == NULL)
  {
    QCRIL_LOG_ERROR("%s: resp_cb_data was not valid", __FUNCTION__);
    return;
  }

  session_type = ((qcril_pbm_cb_struct_type*) qmi_resp_callback.cb_data)->session_type;
  record_id = ((qcril_pbm_cb_struct_type*) qmi_resp_callback.cb_data)->record_index;
  token_id = ((qcril_pbm_cb_struct_type*) qmi_resp_callback.cb_data)->token;

  getPbmModule().getPendingMessageList().print();
  auto msg = getPbmModule().getPendingMessageList().extract(token_id);
  getPbmModule().getPendingMessageList().print();
  if (msg == nullptr)
  {
    QCRIL_LOG_ERROR("token: %d not found in PBM pending list", token_id);
  }
  else
  {
    /* call the response conversion function based on message id */
    switch (msg_id)
    {
    case QMI_PBM_DELETE_RECORD_RESP_V01:
      {
        pbm_delete_record_resp_msg_v01* pbm_delete_resp_ptr =
                      (pbm_delete_record_resp_msg_v01 *) qmi_resp_callback.data_buf;

        ril_req_res = qcril_qmi_util_convert_qmi_response_codes_to_ril_result(
                                                        qmi_resp_callback.transp_err,
                                                        &pbm_delete_resp_ptr->resp );
        QCRIL_LOG_INFO("session_type is %d", (int)session_type);
        QCRIL_LOG_INFO("pbm_delete_resp.record_id_valid is %d",
                                          (int)pbm_delete_resp_ptr->record_id_valid);

        if ( RIL_E_SUCCESS == ril_req_res && pbm_delete_resp_ptr->record_id_valid
           && record_id == pbm_delete_resp_ptr->record_id)
        {
          QCRIL_LOG_INFO("The ADN record in %d has already been deleted", record_id);
          for ( index = 0; index < pbm_cache_info.pbm_num_session; index++ )
          {
            if(session_type == pbm_cache_info.pbm_session_type[index])
            {
               PBM_CACHE_LOCK();
               pbm_cache_info.pbm_capacity[index].valid_adn_num --;
               PBM_CACHE_UNLOCK();
               break;
            }
          }
        }
      }
      break;
    case QMI_PBM_WRITE_RECORD_EXT_RESP_V01:
      {
        pbm_write_record_ext_resp_msg_v01* pbm_write_resp_ptr =
                    (pbm_write_record_ext_resp_msg_v01 *) qmi_resp_callback.data_buf;

        ril_req_res = qcril_qmi_util_convert_qmi_response_codes_to_ril_result(
                                                      qmi_resp_callback.transp_err,
                                                      &pbm_write_resp_ptr->resp );

        QCRIL_LOG_INFO("session_type is %d", (int)session_type);
        QCRIL_LOG_INFO("pbm_write_ext_resp.record_id_valid is %d",
                                        (int)pbm_write_resp_ptr->record_id_valid);

        if ( RIL_E_SUCCESS == ril_req_res && pbm_write_resp_ptr->record_id_valid )
        {
            if(record_id == 0) {
              record_id = pbm_write_resp_ptr->record_id;
              QCRIL_LOG_INFO("The ADN record in %d has already been added", record_id);
              for ( index = 0; index < pbm_cache_info.pbm_num_session; index++ )
              {
                if(session_type == pbm_cache_info.pbm_session_type[index])
                {
                  PBM_CACHE_LOCK();
                  pbm_cache_info.pbm_capacity[index].valid_adn_num ++;
                  PBM_CACHE_UNLOCK();
                  break;
                }
              }
            }
            else if(record_id == pbm_write_resp_ptr->record_id)
            {
                QCRIL_LOG_INFO("The ADN record in %d has already been modified", record_id);
            }
        }
      }
      break;
    default:
      QCRIL_LOG_ERROR("qmi_pbm_client_async_cb: unknown rsp ID = %x", (unsigned int)msg_id);
      break;
    }

    //support multi-session
    for ( j = 0; j < pbm_cache_info.pbm_num_session; j++ )
    {
        if(session_type == pbm_cache_info.pbm_session_type[j])  {
            for ( k = 0; j>0 && k < j; k++ ) {
              session_start_offset += pbm_cache_info.pbm_capacity[k].max_adn_num;
            }
            break;
         }
    }
    QCRIL_LOG_INFO("The offset of ADN record for this session is: %d", session_start_offset);

    record_id += session_start_offset;

    auto updatedResp = std::make_shared<qcril::interfaces::AdnRecordUpdatedResp>();
    updatedResp->setRecordIndex(record_id);
    auto ril_msg(std::static_pointer_cast<QcRilRequestMessage>(msg));
    auto resp = std::make_shared<QcRilRequestMessageCallbackPayload>(ril_req_res, updatedResp);
    ril_msg->sendResponse(ril_msg, Message::Callback::Status::SUCCESS, resp);
  }

  qcril_free(qmi_resp_callback.cb_data);

  QCRIL_LOG_FUNC_RETURN();
}

RIL_Errno qcril_qmi_pbm_delete_adn_record(int record_id, int session_type, int token)
{
    RIL_Errno ril_req_res = RIL_E_GENERIC_FAILURE;
    qmi_client_error_type qmi_transport_error;

    pbm_delete_record_req_msg_v01 pbm_delete_req;
    qcril_pbm_cb_struct_type *pbm_rep_helper = NULL;

    QCRIL_LOG_FUNC_ENTRY();

    memset(&pbm_delete_req, 0, sizeof(pbm_delete_req));

    pbm_rep_helper = (qcril_pbm_cb_struct_type*)qcril_malloc( sizeof( *pbm_rep_helper ) );
    if ( pbm_rep_helper )
    {
        pbm_delete_req.record_info.pb_type = PBM_PB_TYPE_ADN_V01;
        pbm_delete_req.record_info.session_type = (pbm_session_type_enum_v01)session_type;
        pbm_delete_req.record_info.record_id = record_id;

        pbm_rep_helper->session_type = (pbm_session_type_enum_v01)session_type;
        pbm_rep_helper->record_index = record_id;
        pbm_rep_helper->token = token;

        QCRIL_LOG_INFO("Record id to delete is %d", pbm_delete_req.record_info.record_id);
        QCRIL_LOG_INFO("Pb type is %d, session type is %d", pbm_delete_req.record_info.pb_type,
                                                        pbm_delete_req.record_info.session_type);

        qmi_transport_error =  qmi_client_pbm_send_async( QMI_PBM_DELETE_RECORD_REQ_V01,
                                        &pbm_delete_req,
                                        sizeof(pbm_delete_req),
                                        sizeof(pbm_delete_record_resp_msg_v01),
                                        qcril_qmi_pbm_async_cb_helper_hndlr,
                                        (void *)pbm_rep_helper);


        ril_req_res = qcril_qmi_util_convert_qmi_response_codes_to_ril_result(qmi_transport_error, NULL);
    }

    if ( RIL_E_SUCCESS != ril_req_res )
    {
        if(pbm_rep_helper != NULL)
        {
            qcril_free( pbm_rep_helper );
        }
    }

    QCRIL_LOG_FUNC_RETURN();
    return ril_req_res;
}

RIL_Errno qcril_qmi_pbm_add_or_modify_adn_record
(
    const qcril::interfaces::AdnRecordInfo* record_info_ptr,
    int record_id,
    int session_type,
    int token
)
{
    RIL_Errno ril_req_res = RIL_E_GENERIC_FAILURE;
    qmi_client_error_type qmi_transport_error;
    pbm_write_record_ext_req_msg_v01  pbm_write_req;
    qcril_pbm_cb_struct_type * pbm_rep_helper = NULL;
    unsigned int i;

    QCRIL_LOG_FUNC_ENTRY();

    memset(&pbm_write_req, 0, sizeof(pbm_write_req));

    pbm_rep_helper = (qcril_pbm_cb_struct_type*)qcril_malloc( sizeof( *pbm_rep_helper ) );
    if ( pbm_rep_helper )
    {
        pbm_write_req.record_info.pb_type = PBM_PB_TYPE_ADN_V01;
        pbm_write_req.record_info.session_type = (pbm_session_type_enum_v01)session_type;
        pbm_write_req.record_info.record_id = record_id;

        pbm_rep_helper->session_type = (pbm_session_type_enum_v01)session_type;
        pbm_rep_helper->record_index = record_id;
        pbm_rep_helper->token = token;

        QCRIL_LOG_INFO("Record id to write is %d", pbm_write_req.record_info.record_id);
        QCRIL_LOG_INFO("Pb type is %d, session type is %d", pbm_write_req.record_info.pb_type,
                                                        pbm_write_req.record_info.session_type);

        if(!record_info_ptr->name.empty())
        {
            pbm_write_req.name_valid = TRUE;
            pbm_write_req.name_len = qcril_cm_ss_convert_utf8_to_ucs2(
                                        (char*) record_info_ptr->name.c_str(),
                                        strlen(record_info_ptr->name.c_str()),
                                        (char*) pbm_write_req.name,
                                        sizeof(pbm_write_req.name));
        }
        QCRIL_LOG_INFO("Name in the record: %s", pbm_write_req.name);

        if(!record_info_ptr->number.empty())
        {
            pbm_write_req.num_valid = TRUE;
            pbm_write_req.num.num_plan= PBM_NUM_PLAN_ISDN_V01;
            pbm_write_req.num.number_len = strlen(record_info_ptr->number.c_str());
            memcpy(pbm_write_req.num.number, record_info_ptr->number.c_str(),
                                        pbm_write_req.num.number_len);
        }
        QCRIL_LOG_INFO("Number in the record: %s", pbm_write_req.num.number);

        if(record_info_ptr->email_elements > 0)
        {
            pbm_write_req.email_info_valid = TRUE;
            pbm_write_req.email_info_len = MIN((unsigned int) record_info_ptr->email_elements,
                                                sizeof(pbm_write_req.email_info));
            for (i = 0; (i < pbm_write_req.email_info_len &&
                  i < QMI_PBM_EMAIL_INFO_MAX_V01 && i < RIL_MAX_NUM_EMAIL_COUNT); i++)
            {
                pbm_write_req.email_info[i].email_address_len =
                                            qcril_cm_ss_convert_utf8_to_ucs2(
                                                record_info_ptr->email[i].c_str(),
                                                strlen(record_info_ptr->email[i].c_str()),
                                                (char*) pbm_write_req.email_info[i].email_address,
                                                sizeof(pbm_write_req.email_info[i].email_address));

                QCRIL_LOG_INFO("Email in the record of index %d: %s", i, pbm_write_req.email_info[i].email_address);
            }
        }
        QCRIL_LOG_INFO("Email elements in the record: %d", pbm_write_req.email_info_len);

        if(record_info_ptr->anr_elements > 0)
        {
            pbm_write_req.ad_num_info_valid = TRUE;
            pbm_write_req.ad_num_info_len = MIN((unsigned int) record_info_ptr->anr_elements,
                                                sizeof(pbm_write_req.ad_num_info));
            for (i = 0; (i < pbm_write_req.ad_num_info_len && i < QMI_PBM_AD_NUM_MAX_V01 && i< RIL_MAX_NUM_AD_COUNT); i++)
            {
                pbm_write_req.ad_num_info[i].num_plan= PBM_NUM_PLAN_ISDN_V01;
                pbm_write_req.ad_num_info[i].ad_number_len =
                                                strlen(record_info_ptr->ad_number[i].c_str());
                memcpy(pbm_write_req.ad_num_info[i].ad_number,
                        record_info_ptr->ad_number[i].c_str(),
                        pbm_write_req.ad_num_info[i].ad_number_len);

                QCRIL_LOG_INFO("Anr length in the record of index %d: %d", i, pbm_write_req.ad_num_info[i].ad_number_len);
                QCRIL_LOG_INFO("Anr in the record of index %d: %s", i, pbm_write_req.ad_num_info[i].ad_number);
            }
        }
        QCRIL_LOG_INFO("Anr elements in the record: %d", pbm_write_req.ad_num_info_len);

        qmi_transport_error =  qmi_client_pbm_send_async(QMI_PBM_WRITE_RECORD_EXT_REQ_V01,
                                        &pbm_write_req,
                                        sizeof(pbm_write_req),
                                        sizeof(pbm_write_record_ext_resp_msg_v01),
                                        qcril_qmi_pbm_async_cb_helper_hndlr,
                                        (void *)pbm_rep_helper);


        ril_req_res = qcril_qmi_util_convert_qmi_response_codes_to_ril_result(qmi_transport_error, NULL);
    }

    if ( RIL_E_SUCCESS != ril_req_res )
    {
        if(pbm_rep_helper != NULL)
        {
            qcril_free( pbm_rep_helper );
        }
    }

    QCRIL_LOG_FUNC_RETURN();
    return ril_req_res;
}

void qmi_ril_set_alternate_emergency_call_info(char *number, uint32_t emer_cat)
{
  QCRIL_LOG_FUNC_ENTRY();
  if (number && strlen(number) > 0)
  {
    PBM_CACHE_LOCK();
    pbm_cache_info.alt_emer_num_info_valid = TRUE;
    strlcpy(pbm_cache_info.alt_emer_num_info.number, number, strlen(number)+1);
    pbm_cache_info.alt_emer_num_info.emerg_srv_categ = emer_cat;
    PBM_CACHE_UNLOCK();
    // Telephony expects the number to be included in the ril.ecclist property
    // when it gets the call END indication with failcause as aternate emergency
    // service so that it can retry the call as emergency call immediately.
    // Update the ril.ecclist property directly.
#ifndef QMI_RIL_UTF
    qmi_ril_send_ecc_list_indication();

    // Post the event to reevaluate the ecc list to include the customized
    // emergency numbers if any.
    qcril_event_queue(QCRIL_DEFAULT_INSTANCE_ID,
                      QCRIL_DEFAULT_MODEM_ID,
                      QCRIL_DATA_ON_STACK,
                      QCRIL_EVT_QMI_RIL_ASSESS_EMRGENCY_NUMBER_LIST_DESIGNATED_COUNTRY,
                      NULL,
                      QMI_RIL_ZERO,
                      (RIL_Token) QCRIL_CUSTOM_ECC_INTERNAL_TOKEN);
#endif
  }
  QCRIL_LOG_FUNC_RETURN();
}

void qmi_ril_reset_alternate_emergency_call_info()
{
  QCRIL_LOG_FUNC_ENTRY();
  if (pbm_cache_info.alt_emer_num_info_valid)
  {
    PBM_CACHE_LOCK();
    pbm_cache_info.alt_emer_num_info_valid = FALSE;
    PBM_CACHE_UNLOCK();
#ifndef QMI_RIL_UTF
    qcril_event_queue(QCRIL_DEFAULT_INSTANCE_ID,
                      QCRIL_DEFAULT_MODEM_ID,
                      QCRIL_DATA_ON_STACK,
                      QCRIL_EVT_QMI_RIL_ASSESS_EMRGENCY_NUMBER_LIST_DESIGNATED_COUNTRY,
                      NULL,
                      QMI_RIL_ZERO,
                      (RIL_Token) QCRIL_CUSTOM_ECC_INTERNAL_TOKEN);
#endif
  }
  QCRIL_LOG_FUNC_RETURN();
}

boolean qmi_ril_is_alternate_emergency_call(char *number)
{
  boolean result = FALSE;
  QCRIL_LOG_FUNC_ENTRY();
  PBM_CACHE_LOCK();
  if (pbm_cache_info.alt_emer_num_info_valid)
  {
    if (strncmp(number, pbm_cache_info.alt_emer_num_info.number,
          strlen(pbm_cache_info.alt_emer_num_info.number)) == 0)
    {
      result = TRUE;
    }
  }
  PBM_CACHE_UNLOCK();

  if (!result)
  {
    qmi_ril_reset_alternate_emergency_call_info();
  }

  QCRIL_LOG_FUNC_RETURN_WITH_RET(result);
  return result;
}

uint32_t qmi_ril_get_alternate_emergency_call_category()
{
  uint32_t emer_cat = 0;
  QCRIL_LOG_FUNC_ENTRY();
  PBM_CACHE_LOCK();
  if (pbm_cache_info.alt_emer_num_info_valid)
  {
    emer_cat = pbm_cache_info.alt_emer_num_info.emerg_srv_categ;
  }
  PBM_CACHE_UNLOCK();
  QCRIL_LOG_FUNC_RETURN_WITH_RET(emer_cat);
  return emer_cat;
}


#ifdef QMI_RIL_UTF
void qcril_qmi_hal_pbm_module_cleanup()
{
    getPbmModule().qcrilHalPbmModuleCleanup();
    qcril_qmi_pbm_cleanup();
}
#endif
