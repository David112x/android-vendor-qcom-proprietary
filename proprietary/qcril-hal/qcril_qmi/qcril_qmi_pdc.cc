/******************************************************************************
#  Copyright (c) 2013-2015, 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
/******************************************************************************
  @file    qcril_qmi_pdc.c
  @brief   qcril qmi - PDC

  DESCRIPTION
    Handles RIL requests, Callbacks, indications for QMI PDC.

******************************************************************************/


//===========================================================================
//
//                           INCLUDE FILES
//
//===========================================================================

#define TAG "QCRIL_QMI_PDC"

#include <errno.h>
#include <cutils/memory.h>
#include <cutils/properties.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include "telephony/ril.h"
#include "comdef.h"
#include "qcrili.h"
#include "qmi_errors.h"
#include "qcril_reqlist.h"
#include "qcril_qmi_client.h"
#include "qcril_qmi_pdc.h"
#include "qmi_ril_platform_dep.h"
#include "qcril_file_utils.h"
#include "qcril_mbn_hw_update.h"
#include "qcril_mbn_sw_update.h"
#include "persistent_device_configuration_v01.h"
#include "qmi_ril_file_observer.h"
#include "modules/nas/qcril_qmi_nas.h"
#include "qcril_qmi_oem_events.h"

#include "framework/Log.h"
#include "modules/qmi/PdcModemEndPoint.h"
#include "modules/qmi/ModemEndPointFactory.h"
#include "modules/mbn/MbnFileUpdateIndication.h"

//#ifdef QMI_RIL_UTF
#include "modules/mbn/MbnModule.h"
//#endif

#include <interfaces/mbn/mbn.h>

//===========================================================================
//
//                    INTERNAL DEFINITIONS AND TYPES
//
//===========================================================================

#define QCRIL_MODEM_MBN_FILE_PATH             "persist.vendor.radio.mbn_path"
#ifdef QMI_RIL_UTF
#define QCRIL_MBN_SIM_INFO_FILE               "data/vendor/misc/radio/iccid"
#define QCRIL_MBN_SIM_INFO_DIR                "data/vendor/misc/radio"
#else
#define QCRIL_MBN_SIM_INFO_FILE DATA_PARTITION_ROOT "radio/iccid"
#define QCRIL_MBN_SIM_INFO_DIR  DATA_PARTITION_ROOT "radio"
#endif

#define QCRIL_MODEM_MBN_DEFAULT_PATH        "/data/modem_config/"
#define QCRIL_PDC_FRAME_SIZE                900
#define QCRIL_PDC_MAX_SUBS_SUPPORT          3
#define QCRIL_MBN_ACTIVATION_STATUS_PROP_NAME "persist.vendor.radio.stack_id"

#define PDC_CONFIG_LOCK()    g_pdc_info.pdc_config_mutex.lock()
#define PDC_CONFIG_UNLOCK()  g_pdc_info.pdc_config_mutex.unlock()

// error code
#define QCRIL_PDC_NO_ERROR      0
#define QCRIL_PDC_GENERIC_FAIL  -1
#define QCRIL_PDC_NO_MEMORY     -2
#define QCRIL_PDC_NO_CONFIGS    -3
#define QCRIL_PDC_SELECT_FAIL   -4
#define QCRIL_PDC_COMMERCIAL_MBN_PREFIX "commercial"

// validation result related
#define QCRIL_QMI_PDC_INDEX_END         (0xffffffffL)
#define QCRIL_QMI_PDC_IS_LAST_INDEX(idx)   ((idx) == QCRIL_QMI_PDC_INDEX_END)

// default slot_id for disabling modem mbn update for all slots
#define QCRIL_PDC_DEFAULT_SLOT_ID       (0xFFFFFFFF)

typedef struct
{
  // user set info
  uint8_t config_id[PDC_CONFIG_ID_SIZE_MAX_V01];
  uint32_t config_id_len;
  uint32_t conf_size;
  uint32_t load_size;
  int conf_fd;
  pdc_config_type_enum_v01 config_type;
} pdc_config_info_type;

typedef struct {
  // for loading configurations
  pdc_config_info_type pdc_config_info;
  char mbn_file_dir[ PROPERTY_VALUE_MAX ];
  pdc_config_info_resp_type_v01 cur_config_lists[ PDC_CONFIG_LIST_SIZE_MAX_V01 ];
  // for deleting all configurations
  uint32_t cur_config_len;
  uint8_t cur_delete_idx;
  // for select all SUBs
  uint8_t sub_count;
  uint8_t cur_select_sub;
  uint8_t sub_select_mask;
  // for deactivate all SUBs
  uint8_t cur_deact_sub;
  // is processing
  uint8_t is_under_processing;
  // is under cleanup
  uint8_t is_under_cleanup;

  // dump file description (MBN diff dump file: XML format)
  int dump_fd;
  char dump_file[QCRIL_DUMP_FILE_PATH_LEN];

  /* is retrieving mbn info */
  uint8_t is_retrieving_mbn_info;
  uint8_t  config_id[PDC_CONFIG_ID_SIZE_MAX_V01];
  uint32_t config_id_len;

  /* Current active config_id */
  uint8_t  active_config_id[QCRIL_PDC_MAX_SUBS_SUPPORT][PDC_CONFIG_ID_SIZE_MAX_V01];
  uint32_t active_config_id_len[QCRIL_PDC_MAX_SUBS_SUPPORT];

  // for pretecting
  qtimutex::QtiSharedMutex pdc_config_mutex;
} pdc_global_info_type;

typedef struct {
  pdc_config_type_enum_v01  config_type;
  uint8_t                   multi_support;
  uint8_t                   ind_token_valid;
  uint32_t                  ind_token;
} qcril_pdc_list_config_params;

typedef enum
{
  QCRIL_PDC_MBN_INFO_NONE,
  QCRIL_PDC_MBN_INFO_RETRIEVE_ACTIVE_ID,
  QCRIL_PDC_MBN_INFO_RETRIEVE_CONFIG_INFO
} qcril_pdc_mbn_info_retrieval_state;

static pdc_global_info_type g_pdc_info;

static boolean qcril_qmi_pdc_is_test_mbn
(
    void         *config_desc,
    unsigned int  config_desc_len
);

static void qcril_qmi_pdc_retrieve_mbn_info_for_config_id
(
    void *
);

qmi_client_error_type qmi_client_pdc_send_raw_sync(unsigned int msg_id,
                                                void *req_struct,int req_struct_len,
                                                void *resp_struct,unsigned int resp_struct_len) {
    qmi_client_error_type rc = QMI_INTERNAL_ERR;
    QCRIL_LOG_FUNC_ENTRY();
    QCRIL_LOG_DEBUG("%s", __FUNCTION__);

    std::shared_ptr<PdcModemEndPoint> mPdcEndPoint =
        ModemEndPointFactory<PdcModemEndPoint>::getInstance().buildEndPoint();
    if(mPdcEndPoint) {
        rc = mPdcEndPoint->sendRawSync(
                msg_id,req_struct,req_struct_len,resp_struct,resp_struct_len);
    }

    QCRIL_LOG_FUNC_RETURN_WITH_RET(rc);
    return rc;
}

qmi_client_error_type qmi_client_pdc_send_raw_async(unsigned int msg_id,
                                                void *req_struct,int req_struct_len,
                                                int resp_struct_len,
                                                qmiAsyncCbType resp_cb,
                                                void *resp_cb_data ) {
    qmi_client_error_type rc = QMI_INTERNAL_ERR;
    QCRIL_LOG_FUNC_ENTRY();
    QCRIL_LOG_DEBUG("%s", __FUNCTION__);

    (void)resp_cb;
    (void)resp_cb_data;

    std::shared_ptr<PdcModemEndPoint> mPdcEndPoint =
        ModemEndPointFactory<PdcModemEndPoint>::getInstance().buildEndPoint();
    if(mPdcEndPoint) {
        rc = mPdcEndPoint->sendRawAsync(msg_id,req_struct, req_struct_len, resp_struct_len,
                                    resp_cb, resp_cb_data, nullptr);
    }

    QCRIL_LOG_FUNC_RETURN_WITH_RET(rc);
    return rc;
}

/*=========================================================================
  FUNCTION:  qcril_qmi_pdc_load_configuraiton

===========================================================================*/
/*!
    @brief
    Handle QCRIL_EVT_QMI_RIL_PDC_LOAD_CONFIGURATION

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_pdc_load_configuration
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr // Output parameter
)
{
  pdc_load_config_req_msg_v01 qmi_request;
  pdc_load_config_resp_msg_v01 qmi_response;
  pdc_load_config_info_type_v01 *p_load_info;
  qmi_client_error_type qmi_error;
  RIL_Errno result = RIL_E_GENERIC_FAILURE;
  int rd_len;
  errno_enum_type reqlist_found;
  qcril_reqlist_public_type req_info;

  QCRIL_LOG_FUNC_ENTRY();
  QCRIL_NOTUSED(ret_ptr);
  QCRIL_NOTUSED(params_ptr);

  memset( &qmi_request, 0, sizeof(qmi_request) );
  p_load_info = &qmi_request.load_config_info;
  p_load_info->config_type = g_pdc_info.pdc_config_info.config_type;
  p_load_info->total_config_size = g_pdc_info.pdc_config_info.conf_size;

  strlcpy((char *)p_load_info->config_id, (char *)g_pdc_info.pdc_config_info.config_id, PDC_CONFIG_ID_SIZE_MAX_V01);
  p_load_info->config_id_len = strlen((char *)p_load_info->config_id);


  // read loop in case that it is interrupt by signals
  // for example TASK_FREEZE when sleep */
  if ( g_pdc_info.pdc_config_info.conf_fd == -1)
  {
    rd_len = -1;
    QCRIL_LOG_ERROR("The MBN file descriptor is -1");
  }
  else
  {
    do
    {
      rd_len = read( g_pdc_info.pdc_config_info.conf_fd,
          p_load_info->config_frame, QCRIL_PDC_FRAME_SIZE );
    } while ( ( rd_len == -1 ) && ( errno == EINTR ) );
  }

  if ( rd_len == -1 ) // there is some error when read file
  {
    result = RIL_E_GENERIC_FAILURE;
    QCRIL_LOG_ERROR("failed to read MBN file");
  }
  else if (rd_len == 0) // reach file end
  {
    // should not reach here, since the indication handler
    // will not queue this event if all of this config has
    // been loaded. Treat it as a error (one case: the file
    // length is empty)
    result = RIL_E_GENERIC_FAILURE;
    QCRIL_LOG_ERROR("reach file end, should not happen");
  }
  else
  {
    p_load_info->config_frame_len = rd_len;
    g_pdc_info.pdc_config_info.load_size += rd_len;
    QCRIL_LOG_DEBUG("current_load_size %d", g_pdc_info.pdc_config_info.load_size);
    qmi_error = qmi_client_pdc_send_raw_sync(QMI_PDC_LOAD_CONFIG_REQ_V01,&qmi_request,
                                sizeof( qmi_request ),&qmi_response,sizeof( qmi_response ));

   result = qcril_qmi_util_convert_qmi_response_codes_to_ril_result( qmi_error, &qmi_response.resp );
   if ( g_pdc_info.pdc_config_info.load_size >= g_pdc_info.pdc_config_info.conf_size )
   {
     QCRIL_LOG_INFO("load_size is %d, conf_size is %d",
         g_pdc_info.pdc_config_info.load_size, g_pdc_info.pdc_config_info.conf_size);
     close( g_pdc_info.pdc_config_info.conf_fd );
     g_pdc_info.pdc_config_info.conf_fd = -1;
   }
  }

  // on failure, send an unsol message to indicate error
  // on success, do nothing here, the load indication handler
  // will handle the remaining things
  if ( result != RIL_E_SUCCESS )
  {
    g_pdc_info.is_under_processing = 0;
#ifndef QMI_RIL_UTF
    auto msg = std::make_shared<QcRilUnsolMbnConfigResultMessage>(QCRIL_PDC_GENERIC_FAIL);
    if (msg != nullptr)
    {
      msg->broadcast();
    }
#endif
    reqlist_found = qcril_reqlist_query_by_event(QCRIL_DEFAULT_INSTANCE_ID,
                                     QCRIL_DEFAULT_MODEM_ID,
                                     QCRIL_EVT_REQUEST_MBN_HW_LOAD_CONFIG,
                                     &req_info );
    if (reqlist_found == E_SUCCESS)
    {
      qcril_mbn_hw_send_load_config_resp(result);
    }
    else
    {
      reqlist_found = qcril_reqlist_query_by_event(QCRIL_DEFAULT_INSTANCE_ID,
                                       QCRIL_DEFAULT_MODEM_ID,
                                       QCRIL_EVT_REQUEST_MBN_SW_LOAD_CONFIG,
                                       &req_info );
      if (reqlist_found == E_SUCCESS)
      {
        qcril_mbn_sw_send_load_config_resp(result);
      }
    }
  }

  QCRIL_LOG_FUNC_RETURN();
}


/*=========================================================================
  FUNCTION:  qcril_qmi_pdc_load_config_ind_hdlr

===========================================================================*/
/*!
    @brief
    Handle QMI_PDC_LOAD_CONFIG_IND_V01

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_pdc_load_config_ind_hdlr
(
  void *ind_data_ptr,
  uint32 ind_data_len
)
{
  pdc_load_config_ind_msg_v01 *load_ind;
  boolean load_completed = FALSE;
  errno_enum_type           reqlist_found;
  qcril_reqlist_public_type req_info;

  QCRIL_LOG_FUNC_ENTRY();
  QCRIL_NOTUSED(ind_data_len);

  if ( ind_data_ptr != NULL )
  {
    load_ind = (pdc_load_config_ind_msg_v01*)ind_data_ptr;
    if ( load_ind->error == QMI_ERR_NONE_V01 )
    {
      // if modem pass the remaining info, we use it to determine
      // whether the load has completed
      if ( load_ind->remaining_config_size_valid )
      {
        QCRIL_LOG_INFO("The remaininng_config_size is %d", load_ind->remaining_config_size);
        QCRIL_LOG_INFO("The received_config_size is %d", load_ind->received_config_size);
        if ( load_ind->remaining_config_size == 0 )
        {
          load_completed = TRUE;
        }
        else
        {

        //TODO remove event loop
          // continue loading config
          qcril_event_queue( QCRIL_DEFAULT_INSTANCE_ID,
                        QCRIL_DEFAULT_MODEM_ID,
                        QCRIL_DATA_ON_STACK,
                        QCRIL_EVT_QMI_RIL_PDC_LOAD_CONFIGURATION,
                        NULL,
                        QMI_RIL_ZERO,
                        (RIL_Token) QCRIL_MBN_INTERNAL_TOKEN_ID );
        }
      }
      else
      {
        if ( g_pdc_info.pdc_config_info.load_size >= g_pdc_info.pdc_config_info.conf_size )
        {
          load_completed = TRUE;
        }
        else
        {

          //TODO remove event
          // continue loading config
          qcril_event_queue( QCRIL_DEFAULT_INSTANCE_ID,
                        QCRIL_DEFAULT_MODEM_ID,
                        QCRIL_DATA_ON_STACK,
                        QCRIL_EVT_QMI_RIL_PDC_LOAD_CONFIGURATION,
                        NULL,
                        QMI_RIL_ZERO,
                        (RIL_Token) QCRIL_MBN_INTERNAL_TOKEN_ID );
        }
      }
    }
    else // there is some error
    {
      // case 1: the config is already loaded
      if ( load_ind->error == QMI_ERR_INVALID_ID_V01 )
      {
        QCRIL_LOG_INFO("Invalid config id, maybe already exists");
        load_completed = TRUE;
      }
      // case 2: no enough space
      else if ( load_ind->error == QMI_ERR_NO_MEMORY_V01 )
      {
        QCRIL_LOG_INFO("no memory in modem EFS");
        g_pdc_info.is_under_processing = 0;
        auto msg = std::make_shared<QcRilUnsolMbnConfigResultMessage>(QCRIL_PDC_NO_MEMORY);
        if (msg != nullptr)
        {
          msg->broadcast();
        }
      }
      else
      {
        QCRIL_LOG_INFO("Failed to load configuration");
        g_pdc_info.is_under_processing = 0;
        auto msg = std::make_shared<QcRilUnsolMbnConfigResultMessage>(QCRIL_PDC_GENERIC_FAIL);
        if (msg != nullptr)
        {
          msg->broadcast();
        }
      }
    } // there is some error

    QCRIL_LOG_INFO("load completed %d", load_completed);
    if ( load_completed )
    {
      reqlist_found = qcril_reqlist_query_by_event(QCRIL_DEFAULT_INSTANCE_ID,
                                       QCRIL_DEFAULT_MODEM_ID,
                                       QCRIL_EVT_REQUEST_MBN_HW_LOAD_CONFIG,
                                       &req_info );
      if (reqlist_found == E_SUCCESS)
      {
          qcril_mbn_hw_send_load_config_resp(load_ind->error);
          g_pdc_info.is_under_processing = 0;
      }
      else
      {
          reqlist_found = qcril_reqlist_query_by_event(QCRIL_DEFAULT_INSTANCE_ID,
                                           QCRIL_DEFAULT_MODEM_ID,
                                           QCRIL_EVT_REQUEST_MBN_SW_LOAD_CONFIG,
                                           &req_info );
          if (reqlist_found == E_SUCCESS)
          {
              g_pdc_info.is_under_processing = 0;
              // ignore the error where config is already loaded for sw_mbn_update
              if (load_ind->error == QMI_ERR_INVALID_ID_V01 )
              {
                qcril_mbn_sw_send_load_config_resp(QMI_ERR_NONE_V01);
              }
              else
              {
                qcril_mbn_sw_send_load_config_resp(load_ind->error);
              }
          }
          else
          {
              // get first sub that needs to select
              g_pdc_info.cur_select_sub = 0;
              while ( g_pdc_info.cur_select_sub < g_pdc_info.sub_count )
              {
                if ( (1 << g_pdc_info.cur_select_sub) & g_pdc_info.sub_select_mask )
                  break;
                g_pdc_info.cur_select_sub++;
              }
              if ( g_pdc_info.cur_select_sub >= g_pdc_info.sub_count )
              {
                // no sub needs to select
                // all done, sent unsol msg to ATEL
                g_pdc_info.is_under_processing = 0;
                g_pdc_info.cur_select_sub = 0;
                auto msg = std::make_shared<QcRilUnsolMbnConfigResultMessage>(QCRIL_PDC_NO_ERROR);
                if (msg != nullptr)
                {
                  msg->broadcast();
                }
              }
              else
              {

                //TODO remove event thread
                // kick the select config start
                qcril_event_queue( QCRIL_DEFAULT_INSTANCE_ID,
                              QCRIL_DEFAULT_MODEM_ID,
                              QCRIL_DATA_ON_STACK,
                              QCRIL_EVT_QMI_RIL_PDC_SELECT_CONFIGURATION,
                              &g_pdc_info.pdc_config_info.config_type,
                              sizeof(g_pdc_info.pdc_config_info.config_type),
                              (RIL_Token) QCRIL_MBN_INTERNAL_TOKEN_ID );
              }
          }
      }
    }
  } //ind_data_ptr != NULL
}

/*=========================================================================
  FUNCTION:  qcril_qmi_pdc_select_config_ind_hdlr

===========================================================================*/
/*!
    @brief
    Handle QMI_PDC_SET_SELECTED_CONFIG_IND_V01

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_pdc_select_config_ind_hdlr
(
  void *ind_data_ptr,
  uint32 ind_data_len
)
{
  pdc_set_selected_config_ind_msg_v01 *set_ind;
  qcril_reqlist_public_type req_info;
  errno_enum_type           reqlist_found;
  char payload;

  QCRIL_LOG_FUNC_ENTRY();
  QCRIL_NOTUSED(ind_data_len);

  payload = QCRIL_PDC_GENERIC_FAIL;
  if ( ind_data_ptr != NULL )
  {
    set_ind = (pdc_set_selected_config_ind_msg_v01*)ind_data_ptr;
    if ( set_ind->error == QMI_ERR_NONE_V01 )
    {
      QCRIL_LOG_INFO("select successful for SUB:%d", g_pdc_info.cur_select_sub);
      payload = QCRIL_PDC_NO_ERROR;
    }
    else // there is some error
    {
      QCRIL_LOG_ERROR("select indication error for SUB:%d", g_pdc_info.cur_select_sub);
      payload = QCRIL_PDC_SELECT_FAIL;
    }
  }
  else
  {
    QCRIL_LOG_ERROR("select indication param NULL");
    payload = QCRIL_PDC_GENERIC_FAIL;
  }

  reqlist_found = qcril_reqlist_query_by_event(QCRIL_DEFAULT_INSTANCE_ID,
                                   QCRIL_DEFAULT_MODEM_ID,
                                   QCRIL_EVT_REQUEST_MBN_HW_SELECT_CONFIG,
                                   &req_info );
  if (reqlist_found == E_SUCCESS)
  {
      qcril_mbn_hw_send_select_config_resp(static_cast<RIL_Errno>(payload));
      g_pdc_info.is_under_processing = 0;
  }
  else
  {
      reqlist_found = qcril_reqlist_query_by_event(QCRIL_DEFAULT_INSTANCE_ID,
                                       QCRIL_DEFAULT_MODEM_ID,
                                       QCRIL_EVT_REQUEST_MBN_SW_SELECT_CONFIG,
                                       &req_info );
      if (reqlist_found == E_SUCCESS)
      {
          if( (payload == QCRIL_PDC_NO_ERROR) &&
              (g_pdc_info.config_id_len < PDC_CONFIG_ID_SIZE_MAX_V01) )
          {
              if(g_pdc_info.cur_select_sub < QCRIL_PDC_MAX_SUBS_SUPPORT)
              {
                //Store current active config info
                memset(g_pdc_info.active_config_id[g_pdc_info.cur_select_sub], 0,
                     sizeof(g_pdc_info.active_config_id[g_pdc_info.cur_select_sub]));
                memcpy(g_pdc_info.active_config_id[g_pdc_info.cur_select_sub],
                       (unsigned char *)g_pdc_info.config_id,
                       g_pdc_info.config_id_len);
                g_pdc_info.active_config_id_len[g_pdc_info.cur_select_sub]
                                                     = g_pdc_info.config_id_len;

                QCRIL_LOG_INFO("Selected config for SUB%d is %s",
                              g_pdc_info.cur_select_sub,
                              g_pdc_info.active_config_id[g_pdc_info.cur_select_sub]);
              }
              else
              {
                QCRIL_LOG_ERROR("Invalid select SUB ID-%d", g_pdc_info.cur_select_sub);
              }
          }

          qcril_mbn_sw_send_select_config_resp(static_cast<RIL_Errno>(payload));
          g_pdc_info.is_under_processing = 0;
      }
      else
      {
          // get next sub that needs to be selected
          while (g_pdc_info.cur_select_sub < g_pdc_info.sub_count )
          {
            g_pdc_info.cur_select_sub++;
            if ( (1 << g_pdc_info.cur_select_sub) & g_pdc_info.sub_select_mask )
              break;
          }

          if ( g_pdc_info.cur_select_sub >= g_pdc_info.sub_count )
          {
            // all done, sent unsol msg to ATEL
            g_pdc_info.is_under_processing = 0;
            g_pdc_info.cur_select_sub = 0;
            auto msg = std::make_shared<QcRilUnsolMbnConfigResultMessage>(payload);
            if (msg != nullptr)
            {
              msg->broadcast();
            }
          }
          else
          {
            //TODO remove event thread
            // continue to select config
            qcril_event_queue( QCRIL_DEFAULT_INSTANCE_ID,
                          QCRIL_DEFAULT_MODEM_ID,
                          QCRIL_DATA_ON_STACK,
                          QCRIL_EVT_QMI_RIL_PDC_SELECT_CONFIGURATION,
                          &g_pdc_info.pdc_config_info.config_type,
                          sizeof(g_pdc_info.pdc_config_info.config_type),
                          (RIL_Token) QCRIL_MBN_INTERNAL_TOKEN_ID );
          }
      }
  }
}

/*=========================================================================
  FUNCTION:  qcril_qmi_pdc_activate_config_ind_hdlr

===========================================================================*/
/*!
    @brief
    Handle QMI_PDC_ACTIVATE_CONFIG_IND_V01

    @return
    None.

    @note
    This function should not be entered in, since modem will perform SSR
    before activation indication comes.
*/
/*=========================================================================*/
void qcril_qmi_pdc_activate_config_ind_hdlr
(
  void *ind_data_ptr,
  uint32 ind_data_len
)
{
  pdc_activate_config_ind_msg_v01 *act_ind;
  qcril_reqlist_public_type req_info;
  RIL_Errno result = RIL_E_GENERIC_FAILURE;
  boolean is_more_act_ind_expected;
  errno_enum_type reqlist_found;

  QCRIL_LOG_FUNC_ENTRY();
  QCRIL_NOTUSED(ind_data_len);

  if ( ind_data_ptr != NULL )
  {
    act_ind = (pdc_activate_config_ind_msg_v01*)ind_data_ptr;
    if ( act_ind->error == QMI_ERR_NONE_V01 )
    {
      QCRIL_LOG_INFO("activate successful");
      result = RIL_E_SUCCESS;
    }
    else
    {
      QCRIL_LOG_ERROR("activate error, qmi error num: %d", act_ind->error);
    }

    reqlist_found = qcril_reqlist_query_by_event(QCRIL_DEFAULT_INSTANCE_ID,
                                     QCRIL_DEFAULT_MODEM_ID,
                                     QCRIL_EVT_REQUEST_MBN_HW_ACTIVATE_CONFIG,
                                     &req_info );
    if (reqlist_found == E_SUCCESS)
    {
        qcril_mbn_hw_send_activate_config_resp(result);
        g_pdc_info.is_under_processing = 0;
    }
    else
    {
        reqlist_found = qcril_reqlist_query_by_event(QCRIL_DEFAULT_INSTANCE_ID,
                                         QCRIL_DEFAULT_MODEM_ID,
                                         QCRIL_EVT_REQUEST_MBN_SW_ACTIVATE_CONFIG,
                                         &req_info );
        if (reqlist_found == E_SUCCESS)
        {
            is_more_act_ind_expected = qcril_mbn_sw_send_activate_config_resp(result, TRUE);
            if (!is_more_act_ind_expected)
            {
                g_pdc_info.is_under_processing = 0;
                QCRIL_LOG_INFO("No more activation indication expectd");
            }
        }
    }

  }

  QCRIL_LOG_FUNC_RETURN();
}

/*=========================================================================
  FUNCTION:  qcril_qmi_pdc_delete_config_ind_hdlr

===========================================================================*/
/*!
    @brief
    Handle QMI_PDC_DELETE_CONFIG_IND_V01

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_pdc_delete_config_ind_hdlr
(
  void *ind_data_ptr,
  uint32 ind_data_len
)
{
  pdc_delete_config_ind_msg_v01 *del_ind;
  qcril_reqlist_public_type req_info;
  RIL_Errno result = RIL_E_SUCCESS;
  errno_enum_type reqlist_found;
  QCRIL_NOTUSED(ind_data_len);
  char payload;

  QCRIL_LOG_FUNC_ENTRY();
  if ( ind_data_ptr != NULL )
  {
    del_ind = (pdc_delete_config_ind_msg_v01*)ind_data_ptr;
    if ( del_ind->error == QMI_ERR_NONE_V01 )
    {
      QCRIL_LOG_INFO("delete successfully");
    }
    else
    {
      QCRIL_LOG_ERROR("failed to delete, error id = %d", del_ind->error);
      result = RIL_E_GENERIC_FAILURE;
    }
  }
  else // ind_data_ptr = NULL
  {
    QCRIL_LOG_ERROR("NULL parameter");
    result = RIL_E_GENERIC_FAILURE;
  }

  g_pdc_info.is_under_processing = 0;
  if (g_pdc_info.is_under_cleanup)
  {
    g_pdc_info.is_under_cleanup = 0;
    payload = (result == RIL_E_GENERIC_FAILURE) ? QCRIL_PDC_GENERIC_FAIL : QCRIL_PDC_NO_ERROR;
    auto msg = std::make_shared<QcRilUnsolMbnConfigClearedMessage>(payload);
    if (msg != nullptr)
    {
      msg->broadcast();
    }
  }
  else
  {
    reqlist_found = qcril_reqlist_query_by_event(QCRIL_DEFAULT_INSTANCE_ID,
                                       QCRIL_DEFAULT_MODEM_ID, QCRIL_EVT_REQUEST_MBN_HW_DELETE_CONFIG,
                                       &req_info );
    if (reqlist_found == E_SUCCESS)
    {
        qcril_mbn_hw_send_delete_config_resp(result);
    }
    else
    {
        reqlist_found = qcril_reqlist_query_by_event(QCRIL_DEFAULT_INSTANCE_ID,
                                           QCRIL_DEFAULT_MODEM_ID,
                                           QCRIL_EVT_REQUEST_MBN_SW_DELETE_CONFIG,
                                           &req_info );
        if (reqlist_found == E_SUCCESS)
        {
            qcril_mbn_sw_send_delete_config_resp(result);
        }
        else
        {
            reqlist_found = qcril_reqlist_query_by_event(QCRIL_DEFAULT_INSTANCE_ID,
                                            QCRIL_DEFAULT_MODEM_ID,
                                            QCRIL_EVT_REQUEST_MBN_SW_CLEANUP_CONFIG,
                                            &req_info);
            if (reqlist_found == E_SUCCESS)
            {
                qcril_mbn_sw_send_cleanup_config_resp(result);
            }
        }
    }
  }
}

/*=========================================================================
  FUNCTION:  qcril_qmi_pdc_get_config_info_ind_hdlr

===========================================================================*/
/*!
    @brief
    Handle QMI_PDC_GET_CONFIG_INFO_IND_V01

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_pdc_get_config_info_ind_hdlr
(
  void *ind_data_ptr,
  uint32 ind_data_len
)
{
  pdc_get_config_info_ind_msg_v01 *get_ind = NULL;
  qcril_reqlist_public_type req_info;
  RIL_Errno result = RIL_E_GENERIC_FAILURE;
  errno_enum_type reqlist_found;
  QCRIL_NOTUSED(ind_data_len);

  boolean has_config_desc = FALSE;
  int8_t config_desc[PDC_CONFIG_DESC_SIZE_MAX_V01] = {};
  uint32_t config_desc_len = 0;
  boolean has_base_version = FALSE;
  boolean has_config_version = FALSE;
  uint32_t base_version = 0;
  uint32_t config_version = 0;
  std::shared_ptr<Message> pendingMsg;

  QCRIL_LOG_FUNC_ENTRY();

  memset(config_desc, 0, sizeof(config_desc));
  if ( ind_data_ptr != NULL )
  {
    get_ind = (pdc_get_config_info_ind_msg_v01*)ind_data_ptr;
    // Ignore the qmi error code. Sometimes config_desc is reported although
    // there is qmi error. (Will be fixed by modem MCFG team later)
    if ( ( get_ind->config_desc_valid == 1) && ( get_ind->config_desc_len > 0 )
            && ( get_ind->config_desc_len < PDC_CONFIG_DESC_SIZE_MAX_V01 ) )
    {
      has_config_desc = TRUE;
      config_desc_len = get_ind->config_desc_len;
      memcpy(config_desc, get_ind->config_desc, config_desc_len);

      PDC_CONFIG_LOCK();
      if (QCRIL_PDC_MBN_INFO_RETRIEVE_CONFIG_INFO == g_pdc_info.is_retrieving_mbn_info)
      {
          g_pdc_info.is_retrieving_mbn_info = QCRIL_PDC_MBN_INFO_NONE;

          PDC_CONFIG_UNLOCK();
          if (qcril_qmi_pdc_is_test_mbn(config_desc, config_desc_len))
        {
              if (qmi_ril_is_feature_supported(QMI_RIL_FEATURE_SUPPRESS_REQ))
              {
                qcril_request_suppress_request(RIL_REQUEST_SET_INITIAL_ATTACH_APN,
                                                 NULL);
                qcril_request_suppress_request(RIL_REQUEST_SETUP_DATA_CALL, NULL);
              }
        }
          else
          {
              qcril_request_clean_up_suppress_list();
          }
      }
      else
      {
          PDC_CONFIG_UNLOCK();
      }
    }

    if (get_ind->base_version_valid == 1)
    {
      has_base_version = TRUE;
      base_version = get_ind->base_version;
    }

    if (get_ind->config_version_valid == 1)
    {
      has_config_version = TRUE;
      config_version = get_ind->config_version;
    }
  }
  else // ind_data_ptr = NULL
  {
    QCRIL_LOG_ERROR("NULL parameter");
  }

  // send response for QCRIL_EVT_HOOK_GET_META_INFO if has
  reqlist_found = qcril_reqlist_query_by_event( QCRIL_DEFAULT_INSTANCE_ID,
                                                QCRIL_DEFAULT_MODEM_ID,
                                                QCRIL_EVT_REQUEST_MBN_SW_GET_CONFIG_INFO,
                                                &req_info );
  if ( reqlist_found == E_SUCCESS )
  {
      g_pdc_info.is_under_processing = 0;
      result = RIL_E_SUCCESS;
      qcril_mbn_sw_send_get_config_info_resp(result, has_base_version, base_version,
                                            has_config_version, config_version);
  }
  else
  {
      pendingMsg = getMbnModule().getPendingMessageList().find(
              QcRilRequestGetMetaInfoMessage::get_class_message_id());

      if (pendingMsg != nullptr)
      {
        getMbnModule().getPendingMessageList().erase(pendingMsg);

        result = (has_config_desc) ? RIL_E_SUCCESS : RIL_E_GENERIC_FAILURE;

        auto mbnConfigResp = std::make_shared<qcril::interfaces::MbnConfigResp>();
        if(mbnConfigResp != nullptr)
        {
            mbnConfigResp->setLength(config_desc_len);
            std::vector<uint8_t> configInfo(config_desc, config_desc + config_desc_len);
            configInfo.insert(configInfo.end(), '\0');
            QCRIL_LOG_INFO("config info is %s", configInfo.data());
            mbnConfigResp->setConfig(configInfo);
        }
        else
        {
            QCRIL_LOG_DEBUG("unable to make shared pointer for mbnConfigResp");
            result = RIL_E_NO_MEMORY;
        }
        auto resp = std::make_shared<QcRilRequestMessageCallbackPayload>(result, mbnConfigResp);
        if(resp != nullptr)
        {
            auto ril_msg(std::static_pointer_cast<QcRilRequestGetMetaInfoMessage>(pendingMsg));
            ril_msg->sendResponse(ril_msg, Message::Callback::Status::SUCCESS, resp);
        }
        else
        {
            QCRIL_LOG_DEBUG("unable to make ril_msg for response");
        }
      }
      else
      {
        QCRIL_LOG_DEBUG("unable to find reqlist by message: QcRilRequestGetMetaInfoMessage");
      }
  }

#ifndef QMI_RIL_UTF
  // send response for QCRIL_EVT_HOOK_GET_QC_VERSION_OF_CONFIGID if has
  pendingMsg = getMbnModule().getPendingMessageList().find(
          QcRilRequestGetQcVersionOfConfigMessage::get_class_message_id());

  if (pendingMsg != nullptr)
  {
      result = (has_config_version) ? RIL_E_SUCCESS : RIL_E_GENERIC_FAILURE;

      getMbnModule().getPendingMessageList().erase(pendingMsg);

      auto configVersionResp = std::make_shared<qcril::interfaces::ConfigVersionResp>();
      configVersionResp->setVersion(base_version);
      auto resp = std::make_shared<QcRilRequestMessageCallbackPayload>(result, configVersionResp);
      auto ril_msg(std::static_pointer_cast<QcRilRequestGetQcVersionOfConfigMessage>(pendingMsg));
      ril_msg->sendResponse(ril_msg, Message::Callback::Status::SUCCESS, resp);
  }
  else
  {
    QCRIL_LOG_DEBUG("unable to find reqlist by message: QcRilRequestGetQcVersionOfConfigMessage");
  }

  // send response for QCRIL_EVT_HOOK_GET_OEM_VERSION_OF_CONFIGID if has
  pendingMsg = getMbnModule().getPendingMessageList().find(
          QcRilRequestGetOemVersionOfConfigMessage::get_class_message_id());

  if (pendingMsg != nullptr)
  {
      result = (has_config_version) ? RIL_E_SUCCESS : RIL_E_GENERIC_FAILURE;

      getMbnModule().getPendingMessageList().erase(pendingMsg);

      auto configVersionResp = std::make_shared<qcril::interfaces::ConfigVersionResp>();
      configVersionResp->setVersion(config_version);
      auto resp = std::make_shared<QcRilRequestMessageCallbackPayload>(result, configVersionResp);
      auto ril_msg(std::static_pointer_cast<QcRilRequestGetOemVersionOfConfigMessage>(pendingMsg));
      ril_msg->sendResponse(ril_msg, Message::Callback::Status::SUCCESS, resp);
  }
  else
  {
    QCRIL_LOG_DEBUG("unable to find reqlist by message: QcRilRequestGetOemVersionOfConfigMessage");
  }
#endif

  g_pdc_info.is_under_processing = 0;

  QCRIL_LOG_FUNC_RETURN();
}

/*=========================================================================
  FUNCTION:  qcril_qmi_pdc_list_configs_ind_hdlr

===========================================================================*/
/*!
    @brief
    Handle QMI_PDC_LIST_CONFIGS_IND_V01

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_pdc_list_configs_ind_hdlr
(
  void *ind_data_ptr,
  uint32 ind_data_len
)
{
  pdc_list_configs_ind_msg_v01 *list_ind = NULL;
  qcril_pdc_list_config_params list_params;
  RIL_Errno result = RIL_E_SUCCESS;
  char* result_array = NULL;
  char simple_payload;
  uint32_t i, index;
  QCRIL_NOTUSED(ind_data_len);

  QCRIL_LOG_FUNC_ENTRY();

  if ( ind_data_ptr != NULL )
  {
    list_ind = (pdc_list_configs_ind_msg_v01*)ind_data_ptr;
    if ( list_ind->error == QMI_ERR_NONE_V01 )
    {
      if ( (list_ind->config_list_valid) && (list_ind->config_list_len > 0)
            && (list_ind->config_list_len <= PDC_CONFIG_LIST_SIZE_MAX_V01) )
      {
        QCRIL_LOG_INFO("total configuration count %d", list_ind->config_list_len);
      }
      else
      {
        QCRIL_LOG_ERROR("no valid config lists available");
        result = RIL_E_GENERIC_FAILURE;
      }
    }
    else // QMI ERROR
    {
      QCRIL_LOG_ERROR("QMI error, error code %d", list_ind->error);
      result = RIL_E_GENERIC_FAILURE;
    }
  }
  else // ind_data_ptr = NULL
  {
    QCRIL_LOG_ERROR("NULL parameter");
    result = RIL_E_GENERIC_FAILURE;
  }

  if (result == RIL_E_SUCCESS)
  {
    result_array = (char *)qcril_malloc(QCRIL_PDC_ALL_CONFIGS_LEN);
    if ( !result_array )
    {
        result = RIL_E_GENERIC_FAILURE;
    }
    else
    {
        // The first byte indicates the result;
        result_array[0] = 0;
        // The second byte indicates if there is more config available
        // Set 0 initially
        result_array[1] = 0;
        // The second byte indicates the config list length
        result_array[2] = list_ind->config_list_len;
        // Fill the configuration list
        index = 3;
        for (i = 0; i < list_ind->config_list_len; i++)
        {
            result_array[index++] = list_ind->config_list[i].config_id_len;
            memcpy(result_array+index, list_ind->config_list[i].config_id,
                    list_ind->config_list[i].config_id_len);
            index += list_ind->config_list[i].config_id_len;
        }
    }
  }

  if (result == RIL_E_SUCCESS)
  {
    if (list_ind->more_available_valid && list_ind->ind_token_valid
            && (list_ind->more_available != 0))
    {
        // The second byte indicates if there is more config available
        result_array[1] = 1;
        // start the next list request
        list_params.config_type = PDC_CONFIG_TYPE_MODEM_SW_V01;
        list_params.multi_support = 1;
        list_params.ind_token_valid = 1;
        list_params.ind_token = list_ind->ind_token;
        (void)qcril_event_queue(QCRIL_DEFAULT_INSTANCE_ID,
                            QCRIL_DEFAULT_MODEM_ID,
                            QCRIL_DATA_ON_STACK,
                            QCRIL_EVT_QMI_RIL_PDC_LIST_CONFIGURATION,
                            (void*) &list_params,
                            sizeof(list_params),
                            (RIL_Token) QCRIL_MBN_INTERNAL_TOKEN_ID);

    }
    else
    {
        g_pdc_info.is_under_processing = 0;
    }

    std::vector<uint8_t> config_array(result_array, result_array + index);
    auto msg = std::make_shared<QcRilUnsolMbnConfigListMessage>(config_array);
    if (msg != nullptr)
    {
      msg->broadcast();
    }

    free(result_array);
  }
  else
  {
    g_pdc_info.is_under_processing = 0;
    simple_payload = QCRIL_PDC_GENERIC_FAIL;
    std::vector<uint8_t> payload(&simple_payload, &simple_payload + sizeof(simple_payload));
    auto msg = std::make_shared<QcRilUnsolMbnConfigListMessage>(payload);
    if (msg != nullptr)
    {
      msg->broadcast();
    }
  }

}

#ifndef QMI_RIL_UTF
/*=========================================================================
  FUNCTION:  qcril_qmi_pdc_validate_config_ind_hdlr

===========================================================================*/
/*!
    @brief
    Handle QMI_PDC_VALIDATE_CONFIG_IND_V01

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_pdc_validate_config_ind_hdlr
(
  void *ind_data_ptr,
  uint32 ind_data_len
)
{
  pdc_validate_config_ind_msg_v01 *valid_ind;
  RIL_Errno result = RIL_E_GENERIC_FAILURE;
  QCRIL_NOTUSED(ind_data_len);

  QCRIL_LOG_FUNC_ENTRY();

  do
  {
    if ( !g_pdc_info.is_under_processing )
    {
      QCRIL_LOG_ERROR("ignore this indication, since error happens");
      break;
    }

    if ( ind_data_ptr != NULL )
    {
      valid_ind = (pdc_validate_config_ind_msg_v01*)ind_data_ptr;
      if ( valid_ind->error == QMI_ERR_NONE_V01 )
      {
        QCRIL_LOG_INFO("Validate success");
        result = RIL_E_SUCCESS;
      }
      else // QMI ERROR
      {
        QCRIL_LOG_ERROR("QMI error code %d", valid_ind->error);
        break;
      }
    }
    else // ind_data_ptr = NULL
    {
      QCRIL_LOG_ERROR("NULL parameter");
      break;
    }

    if ( !valid_ind->frame_index_valid || !valid_ind->result_frame_valid
              || valid_ind->result_frame_len > PDC_CONFIG_FRAME_SIZE_MAX_V01)
    {
      result = RIL_E_GENERIC_FAILURE;
      QCRIL_LOG_ERROR("frame_index valid: %d, result_frame valid: %d, frame_len: %d",
            valid_ind->frame_index_valid, valid_ind->result_frame_valid,
            valid_ind->result_frame_len);
      break;
    }

    if ( write(g_pdc_info.dump_fd, valid_ind->result_frame, valid_ind->result_frame_len)
                    != (ssize_t)valid_ind->result_frame_len )
    {
      result = RIL_E_GENERIC_FAILURE;
      QCRIL_LOG_ERROR("failed to write the diff result to dump file: %s", strerror(errno));
      break;
    }

    if (QCRIL_QMI_PDC_IS_LAST_INDEX(valid_ind->frame_index))
    {
      QCRIL_LOG_INFO("validation result is fully dumped to file");
      g_pdc_info.is_under_processing = 0;
      close(g_pdc_info.dump_fd);
      auto msg = std::make_shared<QcRilUnsolMbnValidateDumpedMessage>(
            std::string(static_cast<const char*>((char *)g_pdc_info.dump_file)));
      if (msg != nullptr)
      {
        msg->broadcast();
      }

      //TODO remove event thread
      qcril_event_queue( QCRIL_DEFAULT_INSTANCE_ID,
                         QCRIL_DEFAULT_MODEM_ID,
                         QCRIL_DATA_ON_STACK,
                         QCRIL_EVT_QMI_RIL_PDC_PARSE_DIFF_RESULT,
                         (void*)g_pdc_info.dump_file,
                         strlen(g_pdc_info.dump_file),
                         (RIL_Token) QCRIL_MBN_INTERNAL_TOKEN_ID );
    }
  } while (0);

  if ( (result == RIL_E_GENERIC_FAILURE) && (g_pdc_info.is_under_processing) )
  {
    g_pdc_info.is_under_processing = 0;
    // send a message to ATEL, indicating the failure
    qcril_qmi_mbn_diff_send_unsol_msg(result, -1, NULL, NULL, NULL);
  }

  QCRIL_LOG_FUNC_RETURN();
}
#endif

/*=========================================================================
  FUNCTION:  qcril_qmi_pdc_deactivate_config_ind_hdlr

===========================================================================*/
/*!
    @brief
    Handle QMI_PDC_DEACTIVATE_CONFIG_IND_V01

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_pdc_deactivate_config_ind_hdlr
(
  void *ind_data_ptr,
  uint32 ind_data_len
)
{
  pdc_deactivate_config_ind_msg_v01 *deact_ind;
  qcril_reqlist_public_type req_info;
  RIL_Errno result;
  errno_enum_type reqlist_found;
  QCRIL_NOTUSED(ind_data_len);

  QCRIL_LOG_FUNC_ENTRY();

  if ( ind_data_ptr != NULL )
  {
    deact_ind = (pdc_deactivate_config_ind_msg_v01*)ind_data_ptr;
    if ( deact_ind->error == QMI_ERR_NONE_V01 )
    {
      QCRIL_LOG_INFO("Deactivate success for SUB%d", g_pdc_info.cur_deact_sub);
      result = RIL_E_SUCCESS;
    }
    else // QMI ERROR
    {
      QCRIL_LOG_ERROR("QMI error for SUB%d, error code %d", g_pdc_info.cur_deact_sub, deact_ind->error);
      result = RIL_E_GENERIC_FAILURE;
    }
  }
  else // ind_data_ptr = NULL
  {
    QCRIL_LOG_ERROR("NULL parameter");
    result = RIL_E_GENERIC_FAILURE;
  }

  reqlist_found = qcril_reqlist_query_by_event(QCRIL_DEFAULT_INSTANCE_ID,
                                   QCRIL_DEFAULT_MODEM_ID,
                                   QCRIL_EVT_REQUEST_MBN_HW_DEACTIVATE_CONFIG,
                                   &req_info );
  if (reqlist_found == E_SUCCESS)
  {
      g_pdc_info.is_under_processing = 0;
      qcril_mbn_hw_send_deactivate_config_resp(result);
  }
  else
  {
      reqlist_found = qcril_reqlist_query_by_event(QCRIL_DEFAULT_INSTANCE_ID,
                                       QCRIL_DEFAULT_MODEM_ID,
                                       QCRIL_EVT_REQUEST_MBN_SW_DEACTIVATE_CONFIG,
                                       &req_info );
      if (reqlist_found == E_SUCCESS)
      {
          g_pdc_info.is_under_processing = 0;
          qcril_mbn_sw_send_deactivate_config_resp(result);
      }
      else
      {
          // if the there is the error, we still do the delete work anyway
          // no need to check the result
          g_pdc_info.cur_deact_sub++;
          if ( g_pdc_info.cur_deact_sub >= g_pdc_info.sub_count )
          {
            if (g_pdc_info.is_under_cleanup)
            {
              QCRIL_LOG_DEBUG("is_under_cleanup: %d", g_pdc_info.is_under_cleanup);
              // kick the delete all start
              qcril_event_queue( QCRIL_DEFAULT_INSTANCE_ID,
                             QCRIL_DEFAULT_MODEM_ID,
                             QCRIL_DATA_ON_STACK,
                             QCRIL_EVT_QMI_RIL_PDC_DELETE_ALL,
                             NULL,
                             QMI_RIL_ZERO,
                             (RIL_Token) QCRIL_MBN_INTERNAL_TOKEN_ID );
            }
            else
            {
              // send response for deactivate
              std::shared_ptr<Message> pendingMsg = getMbnModule().getPendingMessageList().find(
              QcRilRequestDeactivateMbnConfigMessage::get_class_message_id());

              if (pendingMsg != nullptr)
              {
                result = RIL_E_SUCCESS;
                getMbnModule().getPendingMessageList().erase(pendingMsg);

                auto resp = std::make_shared<QcRilRequestMessageCallbackPayload>(result, nullptr);
                auto ril_msg(std::static_pointer_cast<QcRilRequestDeactivateMbnConfigMessage>(pendingMsg));
                ril_msg->sendResponse(ril_msg, Message::Callback::Status::SUCCESS, resp);
              }
              else
              {
                QCRIL_LOG_DEBUG("unable to find reqlist by message: QcRilRequestDeactivateMbnConfigMessage");
              }
              g_pdc_info.is_under_processing = 0;
            }
          }
          else
          {
            //TODO remove event thread
            // continue to deactivate the next SUB
            qcril_event_queue( QCRIL_DEFAULT_INSTANCE_ID,
                            QCRIL_DEFAULT_MODEM_ID,
                            QCRIL_DATA_ON_STACK,
                            QCRIL_EVT_QMI_RIL_PDC_DEACTIVATE_CONFIGURATION,
                            &g_pdc_info.pdc_config_info.config_type,
                            sizeof(g_pdc_info.pdc_config_info.config_type),
                            (RIL_Token) QCRIL_MBN_INTERNAL_TOKEN_ID );
          }
      }
  }
  QCRIL_LOG_FUNC_RETURN();
}

/*=========================================================================

  FUNCTION:  qcril_qmi_pdc_respond_to_get_selected_config_req

===========================================================================*/
/*!
    @brief
    Handle QMI_PDC_GET_SELECTED_CONFIG_IND_V01

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_pdc_respond_to_get_selected_config_req
(
  uint8_t    *config_id,
  uint32_t    config_id_len,
  uint8_t     pending_valid,
  RIL_Errno   result
)
{
    qcril_reqlist_public_type req_info;
    errno_enum_type           reqlist_found;

    QCRIL_LOG_FUNC_ENTRY();
    // send response
    std::shared_ptr<Message> pendingMsg = getMbnModule().getPendingMessageList().find(
            QcRilRequestGetMbnConfigMessage::get_class_message_id());

    if (pendingMsg != nullptr)
    {
        getMbnModule().getPendingMessageList().erase(pendingMsg);

        auto mbnConfigResp = std::make_shared<qcril::interfaces::MbnConfigResp>();
        mbnConfigResp->setLength(config_id_len);
        std::vector<uint8_t> configId(config_id, config_id + config_id_len);
        configId.insert(configId.end(), '\0');
        QCRIL_LOG_INFO("config ID is %s", configId.data());
        mbnConfigResp->setConfig(configId);
        auto resp = std::make_shared<QcRilRequestMessageCallbackPayload>(result, mbnConfigResp);
        auto ril_msg(std::static_pointer_cast<QcRilRequestGetMbnConfigMessage>(pendingMsg));
        ril_msg->sendResponse(ril_msg, Message::Callback::Status::SUCCESS, resp);
    }
    else
    {
        QCRIL_LOG_DEBUG("unable to find reqlist by message: QcRilRequestGetMbnConfigMessage");

        reqlist_found = qcril_reqlist_query_by_event(QCRIL_DEFAULT_INSTANCE_ID,
                                 QCRIL_DEFAULT_MODEM_ID,
                                 QCRIL_EVT_REQUEST_MBN_HW_GET_SELECTED_CONFIG,
                                 &req_info );
        QCRIL_LOG_INFO("reqlist_found %d", reqlist_found);
        if (reqlist_found == E_SUCCESS)
        {
            g_pdc_info.is_under_processing = 0;
            qcril_mbn_hw_send_get_selected_resp(result, config_id, config_id_len);
        }
        else
        {
            reqlist_found = qcril_reqlist_query_by_event(QCRIL_DEFAULT_INSTANCE_ID,
                                     QCRIL_DEFAULT_MODEM_ID,
                                     QCRIL_EVT_REQUEST_MBN_SW_GET_SELECTED_CONFIG,
                                     &req_info );
            if (reqlist_found == E_SUCCESS)
            {
                g_pdc_info.is_under_processing = 0;
                qcril_mbn_sw_send_get_selected_resp(result, config_id, config_id_len);
            }
            else
            {
                reqlist_found = qcril_reqlist_query_by_event(QCRIL_DEFAULT_INSTANCE_ID,
                                     QCRIL_DEFAULT_MODEM_ID,
                                     QCRIL_EVT_REQUEST_MBN_SW_COUNT_PENDING_CONFIGS,
                                     &req_info );
                if (reqlist_found == E_SUCCESS)
                {
                    qcril_mbn_sw_send_check_pending_response(pending_valid);

                }
            }
        }
    }
    QCRIL_LOG_FUNC_RETURN();
}

/*=========================================================================

  FUNCTION:  qcril_qmi_pdc_acquire_pdc_ownership_for_oem_hook

===========================================================================*/
/*!
    @brief
    Acquire pdc ownership

    @return
    True if success, else failure
*/
/*=========================================================================*/
boolean qcril_qmi_pdc_acquire_pdc_ownership_for_oem_hook
(
    void
)
{
    boolean result = FALSE;
    //TODO call legacy mbn state api from mbnmodule
    if (qcril_mbn_hw_is_completed() && qcril_mbn_sw_is_completed())
    {
        result = TRUE;
    }

    QCRIL_LOG_FUNC_RETURN_WITH_RET(result);
    return result;
}

/*=========================================================================

  FUNCTION:  qcril_qmi_pdc_set_feature_ind_hndlr

===========================================================================*/
/*!
    @brief
    Handle QMI_PDC_SET_FEATURE_IND_V01

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_pdc_set_feature_ind_hndlr
(
  void *ind_data_ptr,
  uint32 ind_data_len
)
{
  pdc_set_feature_ind_msg_v01 *set_feature_ind;
  RIL_Errno result = RIL_E_GENERIC_FAILURE;
  std::shared_ptr<Message> pendingMsg;

  QCRIL_LOG_FUNC_ENTRY();
  QCRIL_NOTUSED(ind_data_len);

  if ( ind_data_ptr != NULL )
  {
    set_feature_ind = (pdc_set_feature_ind_msg_v01*)ind_data_ptr;
    if ( set_feature_ind->error == QMI_ERR_NONE_V01 ||
         set_feature_ind->error == QMI_ERR_NO_EFFECT_V01)
    {
        result = RIL_E_SUCCESS;
    }
  }
  QCRIL_LOG_DEBUG("result %d", result);

  pendingMsg = getMbnModule().getPendingMessageList().find(
          QcRilRequestEnableModemUpdateMessage::get_class_message_id());

  g_pdc_info.is_under_processing = 0;

  if (pendingMsg != nullptr)
  {
      getMbnModule().getPendingMessageList().erase(pendingMsg);

      auto resp = std::make_shared<QcRilRequestMessageCallbackPayload>(result, nullptr);
      auto ril_msg(std::static_pointer_cast<QcRilRequestEnableModemUpdateMessage>(pendingMsg));
      ril_msg->sendResponse(ril_msg, Message::Callback::Status::SUCCESS, resp);
  }
  else
  {
    QCRIL_LOG_DEBUG("unable to find reqlist by message: QcRilRequestEnableModemUpdateMessage");
    qcril_mbn_sw_send_disable_modem_update_resp(result, true);
  }

  QCRIL_LOG_FUNC_RETURN();
}

/*=========================================================================
  FUNCTION:  qcril_qmi_pdc_get_selected_config_ind_hdlr

===========================================================================*/
/*!
    @brief
    Handle QMI_PDC_GET_SELECTED_CONFIG_IND_V01

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_pdc_get_selected_config_ind_hdlr
(
  void *ind_data_ptr,
  uint32 ind_data_len
)
{
  pdc_get_selected_config_ind_msg_v01 *sel_ind = NULL;
  uint8_t config_id[PDC_CONFIG_ID_SIZE_MAX_V01] = {0};
  uint32_t config_id_len = 0;
  RIL_Errno result = RIL_E_SUCCESS;

  QCRIL_LOG_FUNC_ENTRY();
  QCRIL_NOTUSED(ind_data_len);

  if ( ind_data_ptr != NULL )
  {
    sel_ind = (pdc_get_selected_config_ind_msg_v01*)ind_data_ptr;
    if ( sel_ind->error == QMI_ERR_NONE_V01 )
    {
      if ( sel_ind->active_config_id_valid ) // selected and activated
      {
        config_id_len = sel_ind->active_config_id_len;
        if ( config_id_len >= PDC_CONFIG_ID_SIZE_MAX_V01 )
        {
          result = RIL_E_GENERIC_FAILURE;
        }
        else
        {
          memcpy(config_id, sel_ind->active_config_id, config_id_len);

          if(g_pdc_info.cur_select_sub < QCRIL_PDC_MAX_SUBS_SUPPORT)
          {
            //Store current active config info
            QCRIL_LOG_INFO("Store active config for SUB%d as %s",
                         g_pdc_info.cur_select_sub, sel_ind->active_config_id);

            memset(g_pdc_info.active_config_id[g_pdc_info.cur_select_sub], 0,
                 sizeof(g_pdc_info.active_config_id[g_pdc_info.cur_select_sub]));
            memcpy(g_pdc_info.active_config_id[g_pdc_info.cur_select_sub],
                   (unsigned char *)sel_ind->active_config_id,
                   config_id_len);
            g_pdc_info.active_config_id_len[g_pdc_info.cur_select_sub] =
                                                                config_id_len;

          }
          else
          {
            QCRIL_LOG_ERROR("Invalid select SUB ID-%d", g_pdc_info.cur_select_sub);
          }

          if (qmi_ril_is_feature_supported(QMI_RIL_FEATURE_SUPPRESS_REQ))
          {
              PDC_CONFIG_LOCK();
              if ( g_pdc_info.is_retrieving_mbn_info == QCRIL_PDC_MBN_INFO_RETRIEVE_ACTIVE_ID )
              {
                memcpy(g_pdc_info.config_id, sel_ind->active_config_id, config_id_len);
                g_pdc_info.config_id_len = sel_ind->active_config_id_len;
                PDC_CONFIG_UNLOCK();
                qcril_setup_timed_callback(QCRIL_DEFAULT_INSTANCE_ID,
                                             QCRIL_DEFAULT_MODEM_ID,
                                             qcril_qmi_pdc_retrieve_mbn_info_for_config_id,
                                             NULL, NULL );
              }
              else
              {
                PDC_CONFIG_UNLOCK();
              }
          }

          QCRIL_LOG_INFO("current active config id is %s", config_id);
        }
      }
      else if ( sel_ind->pending_config_id_valid ) // selected but not activated
      {
        config_id_len = sel_ind->pending_config_id_len;
        if ( config_id_len >= PDC_CONFIG_ID_SIZE_MAX_V01 )
        {
          result = RIL_E_GENERIC_FAILURE;
        }
        else
        {
          memcpy(config_id, sel_ind->pending_config_id, config_id_len);

          if(g_pdc_info.cur_select_sub < QCRIL_PDC_MAX_SUBS_SUPPORT)
          {
            //Store current active config info
            QCRIL_LOG_INFO("Store active config for SUB%d as %s",
                         g_pdc_info.cur_select_sub, sel_ind->pending_config_id);

            memset(g_pdc_info.active_config_id[g_pdc_info.cur_select_sub], 0,
                 sizeof(g_pdc_info.active_config_id[g_pdc_info.cur_select_sub]));
            memcpy(g_pdc_info.active_config_id[g_pdc_info.cur_select_sub],
                   (unsigned char *)sel_ind->pending_config_id,
                   config_id_len);
            g_pdc_info.active_config_id_len[g_pdc_info.cur_select_sub] =
                                                                config_id_len;
          }
          else
          {
            QCRIL_LOG_ERROR("Invalid select SUB ID-%d", g_pdc_info.cur_select_sub);
          }
        }
        QCRIL_LOG_INFO("current select config id is %s", config_id);
      }
      else
      {
        QCRIL_LOG_ERROR("invalid config id");
        result = RIL_E_GENERIC_FAILURE;
      }
    }
    else // QMI ERROR
    {
      QCRIL_LOG_ERROR("no selected config id");
      result = RIL_E_GENERIC_FAILURE;
    }
  }
  else // ind_data_ptr = NULL
  {
    QCRIL_LOG_ERROR("NULL parameter");
    result = RIL_E_GENERIC_FAILURE;
  }

  qcril_qmi_pdc_respond_to_get_selected_config_req(config_id, config_id_len,
                        (sel_ind) ? sel_ind->pending_config_id_valid : 0, result);
  g_pdc_info.is_under_processing = 0;

  QCRIL_LOG_FUNC_RETURN();
}


/*=========================================================================
  FUNCTION:  qcril_qmi_pdc_list_configuration_event_hdlr

===========================================================================*/
/*!
    @brief
    Handle QCRIL_EVT_QMI_RIL_PDC_LIST_CONFIGURATION

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_pdc_list_configuration_event_hdlr
(
    const qcril_request_params_type *const params_ptr,
    qcril_request_return_type *const ret_ptr // Output parameter
)
{
    qcril_pdc_list_config_params *params = NULL;
    RIL_Errno result = RIL_E_GENERIC_FAILURE;
    pdc_list_configs_req_msg_v01   qmi_request;
    pdc_list_configs_resp_msg_v01  qmi_response;
    qmi_client_error_type          qmi_error;

    QCRIL_LOG_FUNC_ENTRY();
    QCRIL_NOTUSED(params_ptr);
    QCRIL_NOTUSED(ret_ptr);

    do
    {
        params = (qcril_pdc_list_config_params*)params_ptr->data;
        if (!params)
        {
            break;
        }
        memset(&qmi_request, 0, sizeof(qmi_request));
        qmi_request.config_type = params->config_type;
        qmi_request.config_type_valid = TRUE;
        qmi_request.multi_support = params->multi_support;
        qmi_request.multi_support_valid = TRUE;
        if ((qmi_request.ind_token_valid = params->ind_token_valid) != 0)
        {
            qmi_request.ind_token = params->ind_token;
        }

        g_pdc_info.is_under_processing = 1;

        qmi_error = qmi_client_pdc_send_raw_sync(QMI_PDC_LIST_CONFIGS_REQ_V01,
                               &qmi_request,
                               sizeof(qmi_request),
                               &qmi_response,
                               sizeof(qmi_response));

        result = qcril_qmi_util_convert_qmi_response_codes_to_ril_result(qmi_error,
                                                                  &qmi_response.resp);
        if (result != RIL_E_SUCCESS)
        {
            g_pdc_info.is_under_processing = 0;
        }
    } while (0);

    QCRIL_LOG_FUNC_RETURN();
}

/*=========================================================================
  FUNCTION:  qcril_qmi_pdc_select_configuration

===========================================================================*/
/*!
    @brief
    Handle QCRIL_EVT_QMI_RIL_PDC_SELECT_CONFIGURATION

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_pdc_select_configuration
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr // Output parameter
)
{
  RIL_Errno ril_req_res = RIL_E_GENERIC_FAILURE;
  pdc_config_type_enum_v01 pdc_mbn_type;

  QCRIL_LOG_FUNC_ENTRY();
  QCRIL_NOTUSED(params_ptr);
  QCRIL_NOTUSED(ret_ptr);

  if (params_ptr && (params_ptr->data != NULL) && (params_ptr->datalen > 0))
  {
      pdc_mbn_type = (pdc_config_type_enum_v01) ((uint32_t*)params_ptr->data)[0];
  }
  else
  {
      pdc_mbn_type = PDC_CONFIG_TYPE_MODEM_SW_V01;
  }

  QCRIL_LOG_DEBUG("pdc_mbn_type: %d", pdc_mbn_type);
  if (pdc_mbn_type == PDC_CONFIG_TYPE_ENUM_MIN_ENUM_VAL_V01)
  {
      ril_req_res = RIL_E_GENERIC_FAILURE;
  }
  else
  {
      ril_req_res = qcril_qmi_pdc_set_select_configuration(
                                            g_pdc_info.cur_select_sub,
                                            pdc_mbn_type,
                                            g_pdc_info.pdc_config_info.config_id,
                                            g_pdc_info.pdc_config_info.config_id_len);
  }

  // on failure, send an unsol message to indicate error
  // on success, do nothing here, the select indication handler
  // will handle the remaining things
  if ( ril_req_res != RIL_E_SUCCESS )
  {
    auto msg = std::make_shared<QcRilUnsolMbnConfigResultMessage>(QCRIL_PDC_GENERIC_FAIL);
    if (msg != nullptr)
    {
      msg->broadcast();
    }
  }

  QCRIL_LOG_FUNC_RETURN();
}

/*=========================================================================
  FUNCTION:  qcril_qmi_pdc_convert_to_qmi_pdc_type

===========================================================================*/
/*!
    @brief
    Convert mbn type to qmi pdc interface mbn type

    @return
    qmi pdc type.
*/
/*=========================================================================*/
pdc_config_type_enum_v01 qcril_qmi_pdc_convert_to_qmi_pdc_type
(
    qcril::interfaces::qcril_pdc_mbn_type mbn_type
)
{
    pdc_config_type_enum_v01 pdc_mbn_type = PDC_CONFIG_TYPE_ENUM_MIN_ENUM_VAL_V01;
    switch (mbn_type)
    {
        case qcril::interfaces::QCRIL_PDC_MBN_TYPE_SW:
        {
            pdc_mbn_type = PDC_CONFIG_TYPE_MODEM_SW_V01;
            break;
        }

        case qcril::interfaces::QCRIL_PDC_MBN_TYPE_HW:
        {
            pdc_mbn_type = PDC_CONFIG_TYPE_MODEM_PLATFORM_V01;
            break;
        }

        default:
        {
            break;
        }

    }

    return pdc_mbn_type;

}

/*=========================================================================
  FUNCTION:  qcril_qmi_pdc_activate_configuration

===========================================================================*/
/*!
    @brief
    Handle QCRIL_EVT_QMI_RIL_PDC_ACTIVATE_CONFIGURATION

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_pdc_activate_configuration
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr // Output parameter
)
{
  RIL_Errno ril_req_res;
  qcril::interfaces::qcril_pdc_mbn_type mbn_type;
  pdc_config_type_enum_v01 pdc_mbn_type;

  QCRIL_LOG_FUNC_ENTRY();
  QCRIL_NOTUSED(ret_ptr);

  if (params_ptr && (params_ptr->data != NULL) && (params_ptr->datalen > 0))
  {
      mbn_type = static_cast<qcril::interfaces::qcril_pdc_mbn_type>(((uint32_t*)params_ptr->data)[0]);
  }
  else
  {
      mbn_type = qcril::interfaces::QCRIL_PDC_MBN_TYPE_SW;
  }

  pdc_mbn_type = qcril_qmi_pdc_convert_to_qmi_pdc_type(mbn_type);

  QCRIL_LOG_DEBUG("pdc_mbn_type: %d", pdc_mbn_type);
  if (pdc_mbn_type == PDC_CONFIG_TYPE_ENUM_MIN_ENUM_VAL_V01)
  {
      ril_req_res = RIL_E_GENERIC_FAILURE;
  }
  else
  {
      ril_req_res = qcril_qmi_pdc_activate_configuration_hdlr(0, pdc_mbn_type);
  }

  // on failure, send an unsol message to indicate error
  // on success, do nothing here, the activate indication handler
  // will handle the remaining things
  if ( ril_req_res != RIL_E_SUCCESS )
  {
    g_pdc_info.is_under_processing = 0;
    auto msg = std::make_shared<QcRilUnsolMbnConfigResultMessage>(QCRIL_PDC_GENERIC_FAIL);
    if (msg != nullptr)
    {
      msg->broadcast();
    }
  }

  QCRIL_LOG_FUNC_RETURN();
}

/*=========================================================================
  FUNCTION:  qcril_mbn_pdc_delete_all_sw_configs

===========================================================================*/
/*!
    @brief
    Handle QCRIL_EVT_QMI_RIL_PDC_DELETE_ALL

    @return
    None.
*/
/*=========================================================================*/
void qcril_mbn_pdc_delete_all_sw_configs
(
  QCRIL_UNUSED(const qcril_request_params_type *const params_ptr),
  QCRIL_UNUSED(qcril_request_return_type *const ret_ptr) // Output parameter
)
{
  RIL_Errno result = RIL_E_GENERIC_FAILURE;
  qcril_reqlist_public_type req_info;
  errno_enum_type reqlist_found;

  QCRIL_LOG_FUNC_ENTRY();

  // If no config_id specified, MCFG will delete all inactive MBNs
  result = qcril_qmi_pdc_delete_configuration_hdlr(NULL, 0,
                        PDC_CONFIG_TYPE_MODEM_SW_V01);

  if (result == RIL_E_GENERIC_FAILURE)
  {
      QCRIL_LOG_INFO("delete completed, with error %d", result);
      if (g_pdc_info.is_under_cleanup)
      {
          g_pdc_info.is_under_cleanup = 0;
          g_pdc_info.is_under_processing = 0;
          auto msg = std::make_shared<QcRilUnsolMbnConfigClearedMessage>(QCRIL_PDC_GENERIC_FAIL);
          if (msg != nullptr)
          {
            msg->broadcast();
          }
      }
      else
      {
          reqlist_found = qcril_reqlist_query_by_event(QCRIL_DEFAULT_INSTANCE_ID,
                                        QCRIL_DEFAULT_MODEM_ID,
                                        QCRIL_EVT_REQUEST_MBN_SW_CLEANUP_CONFIG,
                                        &req_info);
          if (reqlist_found == E_SUCCESS)
          {
              qcril_mbn_sw_send_cleanup_config_resp(result);
          }
      }
  }
}

/*=========================================================================
  FUNCTION:  qcril_mbn_pdc_query_pending_sw_config

===========================================================================*/
/*!
    @brief
    Handle QCRIL_EVT_REQUEST_MBN_SW_QUERY_PENDING_CONFIG

    @return
    None.
*/
/*=========================================================================*/
void qcril_mbn_pdc_query_pending_sw_config
(
  const qcril_request_params_type *const params_ptr,
  QCRIL_UNUSED(qcril_request_return_type *const ret_ptr) // Output parameter
)
{
  uint32_t sub_id;
  RIL_Errno result  = RIL_E_GENERIC_FAILURE;

  QCRIL_LOG_FUNC_ENTRY();

  if (params_ptr && (params_ptr->data != NULL) && (params_ptr->datalen > 0))
  {
      sub_id = ((uint32_t*)params_ptr->data)[0];
  }
  else
  {
      sub_id = 0;
  }
  QCRIL_LOG_DEBUG("checking pending config for sub_id: %u", sub_id);

  result = qcril_qmi_pdc_get_selected_mbn_config(sub_id, PDC_CONFIG_TYPE_MODEM_SW_V01);

  if (result == RIL_E_GENERIC_FAILURE)
  {
      // failure will be treated as no pendings on this sub
      qcril_mbn_sw_send_check_pending_response(0);
  }

  QCRIL_LOG_FUNC_RETURN();
}

/*=========================================================================
  FUNCTION:  qcril_qmi_pdc_deactivate_configuration

===========================================================================*/
/*!
    @brief
    Handle QCRIL_EVT_QMI_RIL_PDC_DEACTIVATE_CONFIGURATION

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_pdc_deactivate_configuration
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr // Output parameter
)
{
  RIL_Errno result  = RIL_E_GENERIC_FAILURE;
  pdc_config_type_enum_v01 pdc_mbn_type;
  QCRIL_NOTUSED(ret_ptr);

  QCRIL_LOG_FUNC_ENTRY();

  if (params_ptr && (params_ptr->data != NULL) && (params_ptr->datalen > 0))
  {
      pdc_mbn_type = (pdc_config_type_enum_v01) ((uint32_t*)params_ptr->data)[0];
  }
  else
  {
      pdc_mbn_type = PDC_CONFIG_TYPE_MODEM_SW_V01;
  }

  QCRIL_LOG_DEBUG("pdc_mbn_type: %d", pdc_mbn_type);
  if (pdc_mbn_type == PDC_CONFIG_TYPE_ENUM_MIN_ENUM_VAL_V01)
  {
      result = RIL_E_GENERIC_FAILURE;
  }
  else
  {
      result = qcril_qmi_pdc_deactivate_configuration_hdlr(g_pdc_info.cur_deact_sub,
                                                           pdc_mbn_type);
  }

  QCRIL_LOG_FUNC_RETURN();
}


//===========================================================================
// QCRIL_EVT_HOOK_GET_AVAILABLE_CONFIGS
//===========================================================================
void qcril_qmi_pdc_get_available_configs
(
  std::shared_ptr<QcRilRequestGetAvlMbnConfigMessage> msg
)
{
  RIL_Errno result = RIL_E_GENERIC_FAILURE;
  qcril_pdc_list_config_params list_params;

  QCRIL_LOG_FUNC_ENTRY();

  do
  {
    if ( qmi_ril_get_process_instance_id() != QCRIL_DEFAULT_INSTANCE_ID )
    {
      QCRIL_LOG_ERROR("QMI PDC client is only available on primary subscrition");
      result = RIL_E_GENERIC_FAILURE;
      break;
    }

    if ( g_pdc_info.is_under_processing )
    {
      QCRIL_LOG_ERROR("QMI PDC is busy");
      result = RIL_E_GENERIC_FAILURE;
      break;
    }
    g_pdc_info.is_under_processing = 1;

    list_params.config_type = PDC_CONFIG_TYPE_MODEM_SW_V01;
    list_params.multi_support = 1;
    list_params.ind_token_valid = 0;

    (void)qcril_event_queue(QCRIL_DEFAULT_INSTANCE_ID,
                        QCRIL_DEFAULT_MODEM_ID,
                        QCRIL_DATA_ON_STACK,
                        QCRIL_EVT_QMI_RIL_PDC_LIST_CONFIGURATION,
                        (void*) &list_params,
                        sizeof(list_params),
                        (RIL_Token) QCRIL_MBN_INTERNAL_TOKEN_ID);
    result = RIL_E_SUCCESS;
  } while (0);

  auto resp = std::make_shared<QcRilRequestMessageCallbackPayload>(result, nullptr);
  msg->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);

  QCRIL_LOG_FUNC_RETURN();
}


//===========================================================================
//
// param: config_id
// TODO: need to block the call if the previous set is not complete
//===========================================================================
RIL_Errno qcril_qmi_pdc_load_config_file
(
    uint32_t sub_id,
    const char *file_path,
    const char *config_id,
    pdc_config_type_enum_v01 config_type
)
{
  RIL_Errno result = RIL_E_SUCCESS;
  struct stat f_stat;
  size_t str_size;
  char mbn_file_path[QCRIL_MBN_FILE_PATH_LEN];

  QCRIL_LOG_FUNC_ENTRY();

  do
  {
    if ( g_pdc_info.is_under_processing )
    {
      QCRIL_LOG_ERROR("QMI PDC is busy");
      result = RIL_E_GENERIC_FAILURE;
      break;
    }
    g_pdc_info.is_under_processing = 1;

    memset(&g_pdc_info.pdc_config_info, 0, sizeof(pdc_config_info_type));
    g_pdc_info.pdc_config_info.conf_fd = -1;
    if (file_path)
    {
        g_pdc_info.sub_select_mask              = sub_id;
        g_pdc_info.pdc_config_info.config_type  = config_type;
        QCRIL_LOG_INFO("sub_select_mask is %d", g_pdc_info.sub_select_mask);

        // fetch file name
        str_size = strlcpy( mbn_file_path, file_path, QCRIL_MBN_FILE_PATH_LEN);
        QCRIL_LOG_INFO("mbn file path is %s", mbn_file_path);
        if ( str_size > QCRIL_MBN_FILE_PATH_LEN )
        {
            QCRIL_LOG_ERROR("invalid parameter: file name length too long");
            result = RIL_E_GENERIC_FAILURE;
            break;
        }

        str_size = strlcpy((char *)g_pdc_info.pdc_config_info.config_id,
                            config_id, PDC_CONFIG_ID_SIZE_MAX_V01);
        QCRIL_LOG_INFO("config id is %s", g_pdc_info.pdc_config_info.config_id);
        if ( str_size > PDC_CONFIG_ID_SIZE_MAX_V01 )
        {
            QCRIL_LOG_ERROR("valid paramter: config_id length too long");
            result = RIL_E_GENERIC_FAILURE;
            break;
        }
        g_pdc_info.pdc_config_info.config_id_len = str_size;
    }
    else
    {
        QCRIL_LOG_ERROR("invalid parameter");
        result = RIL_E_GENERIC_FAILURE;
        break;
    }

    g_pdc_info.sub_count = QCRIL_PDC_MAX_SUBS_SUPPORT;
    // fill the file descripter and the config size
    g_pdc_info.pdc_config_info.conf_fd = open( mbn_file_path, O_RDONLY );
    if ( g_pdc_info.pdc_config_info.conf_fd == -1 )
    {
      QCRIL_LOG_ERROR("Failed to open file %s: %s", mbn_file_path, strerror(errno));
      result = RIL_E_GENERIC_FAILURE;
      break;
    }
    if ( fstat( g_pdc_info.pdc_config_info.conf_fd, &f_stat ) == -1 )
    {
      QCRIL_LOG_ERROR("Failed to fstat file: %s", strerror(errno));
      close(g_pdc_info.pdc_config_info.conf_fd);
      result = RIL_E_GENERIC_FAILURE;
      break;
    }
    g_pdc_info.pdc_config_info.conf_size = f_stat.st_size;
    QCRIL_LOG_DEBUG("total_conf_size %d", g_pdc_info.pdc_config_info.conf_size);


    //TODO remove event thread
    // kick load config start
    qcril_event_queue( QCRIL_DEFAULT_INSTANCE_ID,
                    QCRIL_DEFAULT_MODEM_ID,
                    QCRIL_DATA_ON_STACK,
                    QCRIL_EVT_QMI_RIL_PDC_LOAD_CONFIGURATION,
                    NULL,
                    QMI_RIL_ZERO,
                    (RIL_Token) QCRIL_MBN_INTERNAL_TOKEN_ID );

  } while (0);

  if ( result == RIL_E_GENERIC_FAILURE )
  {
    g_pdc_info.is_under_processing = 0;
  }

  return result;
}

//===========================================================================
//QCRIL_EVT_HOOK_SET_MODEM_TEST_MODE
// param: config_id
// TODO: need to block the call if the previous set is not complete
//===========================================================================
void qcril_qmi_pdc_set_modem_test_mode
(
  std::shared_ptr<QcRilRequestSetMbnConfigMessage> msg
)
{
  RIL_Errno result = RIL_E_SUCCESS;
  pdc_config_type_enum_v01 pdc_mbn_type;

  QCRIL_LOG_FUNC_ENTRY();

  do
  {
    if (!qcril_qmi_pdc_acquire_pdc_ownership_for_oem_hook())
    {
        QCRIL_LOG_ERROR("no ownership");
        result = RIL_E_GENERIC_FAILURE;
        break;
    }

    if ( qmi_ril_get_process_instance_id() != QCRIL_DEFAULT_INSTANCE_ID )
    {
      QCRIL_LOG_ERROR("QMI PDC client is only available on primary subscrition");
      result = RIL_E_GENERIC_FAILURE;
      break;
    }

    pdc_mbn_type = qcril_qmi_pdc_convert_to_qmi_pdc_type(msg->getMbnType());

    if (pdc_mbn_type == PDC_CONFIG_TYPE_ENUM_MIN_ENUM_VAL_V01)
    {
        result = RIL_E_INVALID_ARGUMENTS;
        break;
    }

    result = qcril_qmi_pdc_load_config_file(msg->getSubId(),
                                   msg->getFilePath().c_str(),
                                   (const char *)msg->getConfigId().data(),
                                   pdc_mbn_type);
  } while (0);

  auto resp = std::make_shared<QcRilRequestMessageCallbackPayload>(result, nullptr);
  msg->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);

  QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
//QCRIL_EVT_HOOK_QUERY_MODEM_TEST_MODE
//===========================================================================
void qcril_qmi_pdc_query_modem_test_mode
(
  std::shared_ptr<QcRilRequestGetMbnConfigMessage> msg
)
{
  RIL_Errno result = RIL_E_GENERIC_FAILURE;
  pdc_config_type_enum_v01 pdc_mbn_type;
  int token = 0;
  bool sendResponse = false;

  QCRIL_LOG_FUNC_ENTRY();

  do
  {
    if (msg == nullptr) {
      break;
    }

    if (!qcril_qmi_pdc_acquire_pdc_ownership_for_oem_hook())
    {
        sendResponse = true;
        QCRIL_LOG_ERROR("no ownership");
        break;
    }

    pdc_mbn_type = qcril_qmi_pdc_convert_to_qmi_pdc_type(msg->getMbnType());

    if (pdc_mbn_type == PDC_CONFIG_TYPE_ENUM_MIN_ENUM_VAL_V01)
    {
        result = RIL_E_GENERIC_FAILURE;
        sendResponse = true;
        break;
    }

    auto ret = getMbnModule().getPendingMessageList().insert(msg);
    if(ret.second)
    {
        token = ret.first;
        result = qcril_qmi_pdc_get_selected_mbn_config(msg->getSubId(),
                                                     pdc_mbn_type);
        if (result != RIL_E_SUCCESS) {
            sendResponse = true;
        }
    }
    else
    {
        QCRIL_LOG_ERROR("Failed to insert to PendingList for query mbn config");
        sendResponse = true;
    }
  } while (0);

  if (sendResponse == true)
  {
    QCRIL_LOG_ERROR("Result of query mbn config is %d", result);

    getMbnModule().getPendingMessageList().erase(token);

    auto resp = std::make_shared<QcRilRequestMessageCallbackPayload>(result, nullptr);
    msg->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
  }

  QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
// qcril_qmi_pdc_disable_modem_update
//===========================================================================
RIL_Errno qcril_qmi_pdc_disable_modem_update
(
   void
)
{
    pdc_set_feature_req_msg_v01 qmi_request;
    pdc_set_feature_resp_msg_v01  qmi_response;
    qmi_client_error_type qmi_error;
    RIL_Errno result = RIL_E_GENERIC_FAILURE;

    QCRIL_LOG_FUNC_ENTRY();

    do
    {
      if ( qmi_ril_get_process_instance_id() != QCRIL_DEFAULT_INSTANCE_ID )
      {
        QCRIL_LOG_ERROR("QMI PDC client is only available on primary subscrition");
        break;
      }

      if ( g_pdc_info.is_under_processing )
      {
        QCRIL_LOG_ERROR("QMI PDC is busy");
        break;
      }
      g_pdc_info.is_under_processing = 1;

      memset( &qmi_request, 0, sizeof(qmi_request) );
      qmi_request.slot_id = QCRIL_PDC_DEFAULT_SLOT_ID;
      qmi_request.selection_mode_valid = true;
      qmi_request.selection_mode = PDC_SELECTION_MODE_DISABLED_V01;
      qmi_request.flex_mapping_valid = true;
      qmi_request.flex_mapping = false;
      qmi_request.carrier_valid = true;
      qmi_request.carrier = PDC_SELECTION_OPEN_MARKET_V01;
      qmi_request.refresh_mode_valid = true;
      qmi_request.refresh_mode = PDC_REFRESH_MODE_ENABLED_V01;

      qmi_error = qmi_client_pdc_send_raw_sync(QMI_PDC_SET_FEATURE_REQ_V01,
                                           &qmi_request, sizeof( qmi_request ),
                                           &qmi_response, sizeof( qmi_response ));

      result = qcril_qmi_util_convert_qmi_response_codes_to_ril_result(
                                          qmi_error,
                                          &qmi_response.resp );
    } while (0);

    QCRIL_LOG_INFO("result %d", result);
    if ( result == RIL_E_GENERIC_FAILURE )
    {
      g_pdc_info.is_under_processing = 0;
    }

    QCRIL_LOG_FUNC_RETURN();

    return result;
}

//===========================================================================
// qcril_qmi_pdc_get_selected_mbn_config
//===========================================================================
RIL_Errno qcril_qmi_pdc_get_selected_mbn_config
(
    int                      sub_id,
    pdc_config_type_enum_v01 config_type
)
{
  pdc_get_selected_config_req_msg_v01 qmi_request;
  pdc_get_selected_config_resp_msg_v01 qmi_response;
  qmi_client_error_type qmi_error;
  RIL_Errno result = RIL_E_GENERIC_FAILURE;
  uint32_t sub_info;

  QCRIL_LOG_FUNC_ENTRY();

  do
  {
    if ( qmi_ril_get_process_instance_id() != QCRIL_DEFAULT_INSTANCE_ID )
    {
      QCRIL_LOG_ERROR("QMI PDC client is only available on primary subscrition");
      break;
    }

    if ( g_pdc_info.is_under_processing )
    {
      QCRIL_LOG_ERROR("QMI PDC is busy");
      break;
    }
    g_pdc_info.is_under_processing = 1;

    sub_info = sub_id;
    g_pdc_info.cur_select_sub = sub_id;
    if ( sub_info > QCRIL_PDC_MAX_SUBS_SUPPORT )
    {
      QCRIL_LOG_ERROR("sub index too large");
      break;
    }

    memset( &qmi_request, 0, sizeof(qmi_request) );
    qmi_request.config_type = config_type;

    if (config_type != PDC_CONFIG_TYPE_MODEM_PLATFORM_V01)
    {
        qmi_request.subscription_id_valid = 1;
        qmi_request.subscription_id = sub_info;
    }

    qmi_error = qmi_client_pdc_send_raw_sync(QMI_PDC_GET_SELECTED_CONFIG_REQ_V01,
                                        &qmi_request, sizeof( qmi_request ),
                                        &qmi_response, sizeof( qmi_response ));

    result = qcril_qmi_util_convert_qmi_response_codes_to_ril_result( qmi_error, &qmi_response.resp );
  } while (0);

  QCRIL_LOG_INFO("result %d", result);
  if ( result != RIL_E_SUCCESS )
  {
    g_pdc_info.is_under_processing = 0;
  }

  QCRIL_LOG_FUNC_RETURN();

  return result;
}

//===========================================================================
// QCRIL_EVT_HOOK_ACTIVATE_CONFIGS
//===========================================================================
void qcril_qmi_pdc_activate_configs
(
  std::shared_ptr<QcRilRequestActivateMbnConfigMessage> msg
)
{
  RIL_Errno result = RIL_E_GENERIC_FAILURE;
  pdc_config_type_enum_v01 pdc_mbn_type;

  QCRIL_LOG_FUNC_ENTRY();

  do
  {
    if (!qcril_qmi_pdc_acquire_pdc_ownership_for_oem_hook())
    {
        QCRIL_LOG_ERROR("no ownership");
        break;
    }

    // base check
    if ( qmi_ril_get_process_instance_id() != QCRIL_DEFAULT_INSTANCE_ID )
    {
      QCRIL_LOG_ERROR("QMI PDC client is only available on primary subscrition");
      result = RIL_E_GENERIC_FAILURE;
      break;
    }
    if ( g_pdc_info.is_under_processing )
    {
      QCRIL_LOG_ERROR("QMI PDC is busy");
      result = RIL_E_GENERIC_FAILURE;
      break;
    }
    g_pdc_info.is_under_processing = 1;

    pdc_mbn_type = qcril_qmi_pdc_convert_to_qmi_pdc_type(msg->getMbnType());

    if (pdc_mbn_type == PDC_CONFIG_TYPE_ENUM_MIN_ENUM_VAL_V01)
    {
        result = RIL_E_GENERIC_FAILURE;
    }
    else
    {
        result = qcril_qmi_pdc_activate_configuration_hdlr(0, pdc_mbn_type);
    }

  } while (0);

  g_pdc_info.is_under_processing = 0;

  auto resp = std::make_shared<QcRilRequestMessageCallbackPayload>(result, nullptr);
  msg->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);

  QCRIL_LOG_FUNC_RETURN_WITH_RET(result);
}

//===========================================================================
// QCRIL_EVT_HOOK_DEACTIVATE_CONFIGS
//===========================================================================
void qcril_qmi_pdc_deactivate_configs
(
  std::shared_ptr<QcRilRequestDeactivateMbnConfigMessage> msg
)
{
  RIL_Errno result;
  pdc_config_type_enum_v01 pdc_mbn_type;
  int token = 0;

  QCRIL_LOG_FUNC_ENTRY();

  if (!qcril_qmi_pdc_acquire_pdc_ownership_for_oem_hook())
  {
      QCRIL_LOG_ERROR("no ownership");
      result = RIL_E_GENERIC_FAILURE;
  }
  else if ( qmi_ril_get_process_instance_id() != QCRIL_DEFAULT_INSTANCE_ID )
  {
    QCRIL_LOG_ERROR("QMI PDC client is only available on primary subscrition");
    result = RIL_E_GENERIC_FAILURE;
  }
  else if ( g_pdc_info.is_under_processing )
  {
    QCRIL_LOG_ERROR("QMI PDC is busy");
    result = RIL_E_GENERIC_FAILURE;
  }
  else
  {
    g_pdc_info.is_under_processing = 1;
    // FIXME: hard code sub_count as 3 when deactivate
    // 1. when deactivate all subs, by default we deactivate for SUB0, SUB1, SUB2,
    // in case we miss any. Because if one SUB still has active configuration, we
    // will fail to delete all configurations.
    // 2. And If any SUB is not supported during deactivate or there is no active
    // config on it, MCFG will report error in QMI deactivate indication. And RIL
    // will ignore this error.
    g_pdc_info.sub_count = QCRIL_PDC_MAX_SUBS_SUPPORT;
    g_pdc_info.cur_deact_sub = 0;
    QCRIL_LOG_INFO("The current maximum subscriptions is %d", g_pdc_info.sub_count);

    pdc_mbn_type = qcril_qmi_pdc_convert_to_qmi_pdc_type(msg->getMbnType());
    if (pdc_mbn_type == PDC_CONFIG_TYPE_ENUM_MIN_ENUM_VAL_V01)
    {
        QCRIL_LOG_ERROR("Invalid mbn type");
        result = RIL_E_GENERIC_FAILURE;
    }
    else
    {
        auto ret = getMbnModule().getPendingMessageList().insert(msg);
        if(ret.second)
        {
            token = ret.first;
            qcril_event_queue( QCRIL_DEFAULT_INSTANCE_ID,
                            QCRIL_DEFAULT_MODEM_ID,
                            QCRIL_DATA_ON_STACK,
                            QCRIL_EVT_QMI_RIL_PDC_DEACTIVATE_CONFIGURATION,
                            &pdc_mbn_type,
                            sizeof(pdc_mbn_type),
                            (RIL_Token) QCRIL_MBN_INTERNAL_TOKEN_ID );
            result = RIL_E_SUCCESS;
        }
        else
        {
            QCRIL_LOG_ERROR("Failed to insert to PendingList for query mbn config");
            result = RIL_E_GENERIC_FAILURE;
        }
    } // if (pdc_mbn_type != PDC_CONFIG_TYPE_ENUM_MIN_ENUM_VAL_V01)
  }

  if ( result == RIL_E_GENERIC_FAILURE )
  {
    QCRIL_LOG_ERROR("Failed to qcril_qmi_pdc_deactivate_configs");

    getMbnModule().getPendingMessageList().erase(token);

    g_pdc_info.is_under_processing = 0;
    auto resp = std::make_shared<QcRilRequestMessageCallbackPayload>(result, nullptr);
    msg->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
  }

  QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
// QCRIL_EVT_HOOK_CLEANUP_LOADED_CONFIGS
//===========================================================================
void qcril_qmi_pdc_cleanup_loaded_configs
(
  std::shared_ptr<QcRilRequestCleanupMbnConfigMessage> msg
)
{
  RIL_Errno result = RIL_E_GENERIC_FAILURE;

  QCRIL_LOG_FUNC_ENTRY();

  if (!qcril_qmi_pdc_acquire_pdc_ownership_for_oem_hook())
  {
    QCRIL_LOG_ERROR("no ownership");
  }
  else if ( qmi_ril_get_process_instance_id() != QCRIL_DEFAULT_INSTANCE_ID )
  {
    QCRIL_LOG_ERROR("QMI PDC client is only available on primary subscrition");
  }
  else if ( g_pdc_info.is_under_processing )
  {
    QCRIL_LOG_ERROR("QMI PDC is busy");
  }
  else
  {
    g_pdc_info.is_under_processing = 1;
    g_pdc_info.is_under_cleanup = 1;
    // FIXME: hard code sub_count as 3 when deactivate
    // 1. when deactivate all subs, by default we deactivate for SUB0, SUB1, SUB2,
    // in case we miss any. Because if one SUB still has active configuration, we
    // will fail to delete all configurations.
    // 2. And If any SUB is not supported during deactivate or there is no active
    // config on it, MCFG will report error in QMI deactivate indication. And RIL
    // will ignore this error.
    g_pdc_info.sub_count = QCRIL_PDC_MAX_SUBS_SUPPORT;
    g_pdc_info.cur_deact_sub = 0;
    QCRIL_LOG_INFO("The current maximum subscriptions is %d", g_pdc_info.sub_count);

    //TODO remove event thread
    qcril_event_queue( QCRIL_DEFAULT_INSTANCE_ID,
                    QCRIL_DEFAULT_MODEM_ID,
                    QCRIL_DATA_ON_STACK,
                    QCRIL_EVT_QMI_RIL_PDC_DEACTIVATE_CONFIGURATION,
                    NULL,
                    QMI_RIL_ZERO,
                    (RIL_Token) QCRIL_MBN_INTERNAL_TOKEN_ID );
    result = RIL_E_SUCCESS;
  }

  auto resp = std::make_shared<QcRilRequestMessageCallbackPayload>(result, nullptr);
  msg->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);

  QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
// QCRIL_EVT_HOOK_SEL_CONFIG
//===========================================================================
void qcril_qmi_pdc_select_configs
(
  std::shared_ptr<QcRilRequestSelectMbnConfigMessage> msg
)
{
  RIL_Errno result = RIL_E_SUCCESS;
  pdc_config_type_enum_v01 pdc_mbn_type = PDC_CONFIG_TYPE_MODEM_SW_V01;

  QCRIL_LOG_FUNC_ENTRY();
  do
  {
    if (!qcril_qmi_pdc_acquire_pdc_ownership_for_oem_hook())
    {
        QCRIL_LOG_ERROR("no ownership");
        break;
    }

    if ( qmi_ril_get_process_instance_id() != QCRIL_DEFAULT_INSTANCE_ID )
    {
      QCRIL_LOG_ERROR("QMI PDC client is only available on primary subscription");
      result = RIL_E_GENERIC_FAILURE;
      break;
    }

    if(!msg || !msg->getConfigId().size()) {
        QCRIL_LOG_ERROR("Invalid Size");
        result = RIL_E_GENERIC_FAILURE;
        break;
    }

    if ( g_pdc_info.is_under_processing )
    {
      QCRIL_LOG_ERROR("QMI PDC is busy");
      result = RIL_E_GENERIC_FAILURE;
      break;
    }
    g_pdc_info.is_under_processing = 1;

    g_pdc_info.sub_select_mask = msg->getSubId();
    QCRIL_LOG_INFO("sub_select_mask is %d", g_pdc_info.sub_select_mask);

    g_pdc_info.pdc_config_info.config_id_len =
            MIN(PDC_CONFIG_ID_SIZE_MAX_V01, msg->getConfigId().size() - 1);
    memcpy(g_pdc_info.pdc_config_info.config_id,
         msg->getConfigId().data(),
         g_pdc_info.pdc_config_info.config_id_len);
    QCRIL_LOG_INFO("config_id is %s", g_pdc_info.pdc_config_info.config_id);

    pdc_mbn_type = qcril_qmi_pdc_convert_to_qmi_pdc_type(msg->getMbnType());

    // submask: each bit represents each SUB and the LSB is for SUB0
    //          value 1 means enable while value 0 means disable
    g_pdc_info.cur_select_sub = 0;
    g_pdc_info.sub_count = QCRIL_PDC_MAX_SUBS_SUPPORT;
    // if no SUB is selected, return directly
    if (!( g_pdc_info.sub_select_mask & ( (1 << g_pdc_info.sub_count) - 1) ))
    {
      QCRIL_LOG_ERROR("no selected subscription");
      result = RIL_E_GENERIC_FAILURE;
      break;
    }
    // get first sub that needs to be selected
    while ( g_pdc_info.cur_select_sub < g_pdc_info.sub_count )
    {
      // found the first enabled SUB
      if ( (1 << g_pdc_info.cur_select_sub) & g_pdc_info.sub_select_mask )
        break;
      g_pdc_info.cur_select_sub++;
    }

    //TODO remove event thread
    // kick the select config start
    qcril_event_queue( QCRIL_DEFAULT_INSTANCE_ID,
                  QCRIL_DEFAULT_MODEM_ID,
                  QCRIL_DATA_ON_STACK,
                  QCRIL_EVT_QMI_RIL_PDC_SELECT_CONFIGURATION,
                  &pdc_mbn_type,
                  sizeof(pdc_mbn_type),
                 (RIL_Token) QCRIL_MBN_INTERNAL_TOKEN_ID );
  } while (0);

  if ( result == RIL_E_GENERIC_FAILURE )
  {
    g_pdc_info.is_under_processing = 0;
  }

  auto resp = std::make_shared<QcRilRequestMessageCallbackPayload>(result, nullptr);
  if(msg)
  {
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
  }

  QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
// qcril_qmi_pdc_get_config_info
//===========================================================================
RIL_Errno qcril_qmi_pdc_get_config_info
(
    const char    *config_id,
    int      config_id_len,
    pdc_config_type_enum_v01 config_type
)
{
  pdc_get_config_info_req_msg_v01 qmi_request;
  pdc_get_config_info_resp_msg_v01 qmi_response;
  RIL_Errno result = RIL_E_GENERIC_FAILURE;
  qmi_client_error_type qmi_error;

  QCRIL_LOG_FUNC_ENTRY();

  do
  {
    if ( g_pdc_info.is_under_processing )
    {
      QCRIL_LOG_ERROR("QMI PDC is busy");
      result = RIL_E_GENERIC_FAILURE;
      break;
    }
    g_pdc_info.is_under_processing = 1;

    memset( &qmi_request, 0, sizeof(qmi_request) );
    qmi_request.new_config_info.config_type = config_type;
    if ((NULL != config_id) && (0 < config_id_len)
                  && (PDC_CONFIG_ID_SIZE_MAX_V01 > config_id_len))
    {
      qmi_request.new_config_info.config_id_len = config_id_len;
      memcpy( qmi_request.new_config_info.config_id,
                  config_id, config_id_len);
    }
    else
    {
      QCRIL_LOG_ERROR("invalid parameter for config_id");
      result = RIL_E_GENERIC_FAILURE;
      break;
    }

    qmi_error = qmi_client_pdc_send_raw_sync(QMI_PDC_GET_CONFIG_INFO_REQ_V01,
                                        &qmi_request, sizeof( qmi_request ),
                                        &qmi_response, sizeof( qmi_response ));

    result = qcril_qmi_util_convert_qmi_response_codes_to_ril_result(
                                            qmi_error, &qmi_response.resp );
  } while (0);

  if ( result != RIL_E_SUCCESS )
  {
    g_pdc_info.is_under_processing = 0;
  }

  QCRIL_LOG_FUNC_RETURN();

  return result;

}

//===========================================================================
// QCRIL_EVT_HOOK_GET_META_INFO
//===========================================================================
void qcril_qmi_pdc_get_meta_info_of_config
(
  std::shared_ptr<QcRilRequestGetMetaInfoMessage> msg
)
{
  RIL_Errno result = RIL_E_GENERIC_FAILURE;
  pdc_config_type_enum_v01 pdc_mbn_type;
  int token = 0;
  bool sendResponse = false;

  QCRIL_LOG_FUNC_ENTRY();

  do
  {
    if (msg == nullptr) {
        break;
    }

    if (!qcril_qmi_pdc_acquire_pdc_ownership_for_oem_hook())
    {
        QCRIL_LOG_ERROR("no ownership");
        result = RIL_E_GENERIC_FAILURE;
        sendResponse = true;
        break;
    }

    if ( qmi_ril_get_process_instance_id() != QCRIL_DEFAULT_INSTANCE_ID )
    {
      QCRIL_LOG_ERROR("QMI PDC client is only available on primary subscrition");
      result = RIL_E_GENERIC_FAILURE;
      sendResponse = true;
      break;
    }

    auto ret = getMbnModule().getPendingMessageList().insert(msg);
    if(ret.second)
    {
        token = ret.first;
        pdc_mbn_type = qcril_qmi_pdc_convert_to_qmi_pdc_type(msg->getMbnType());
        QCRIL_LOG_DEBUG("pdc mbn type is %d", pdc_mbn_type);

        if (pdc_mbn_type == PDC_CONFIG_TYPE_ENUM_MIN_ENUM_VAL_V01)
        {
          result = RIL_E_GENERIC_FAILURE;
          sendResponse = true;
          break;
        }

        result = qcril_qmi_pdc_get_config_info((const char *)msg->getConfigId().data(),
                    msg->getConfigId().size() - 1, pdc_mbn_type);
        if (result != RIL_E_SUCCESS) {
            sendResponse = true;
            break;
        }
    }
    else
    {
        QCRIL_LOG_ERROR("Failed to insert to PendingList for get meta info ");
        sendResponse = true;
    }
  } while (0);

  if ( sendResponse == true )
  {
    QCRIL_LOG_ERROR("Result of get meta info is %d", result);

    getMbnModule().getPendingMessageList().erase(token);

    auto resp = std::make_shared<QcRilRequestMessageCallbackPayload>(result, nullptr);
    msg->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
  }

  QCRIL_LOG_FUNC_RETURN();
}

#ifndef QMI_RIL_UTF
//===========================================================================
// QCRIL_EVT_HOOK_GET_QC_VERSION_OF_CONFIGID
//===========================================================================
void qcril_qmi_pdc_get_qc_version_of_configid
(
  std::shared_ptr<QcRilRequestGetQcVersionOfConfigMessage> msg
)
{
  pdc_get_config_info_req_msg_v01 qmi_request;
  pdc_get_config_info_resp_msg_v01 qmi_response;
  RIL_Errno result = RIL_E_GENERIC_FAILURE;
  qmi_client_error_type qmi_error;
  int token = 0;

  QCRIL_LOG_FUNC_ENTRY();

  do
  {
    if (!qcril_qmi_pdc_acquire_pdc_ownership_for_oem_hook())
    {
        QCRIL_LOG_ERROR("no ownership");
        result = RIL_E_GENERIC_FAILURE;
        break;
    }

    if ( qmi_ril_get_process_instance_id() != QCRIL_DEFAULT_INSTANCE_ID )
    {
      QCRIL_LOG_ERROR("QMI PDC client is only available on primary subscrition");
      result = RIL_E_GENERIC_FAILURE;
      break;
    }

    if ( g_pdc_info.is_under_processing )
    {
      QCRIL_LOG_ERROR("QMI PDC is busy");
      result = RIL_E_GENERIC_FAILURE;
      break;
    }
    g_pdc_info.is_under_processing = 1;

    memset( &qmi_request, 0, sizeof(qmi_request) );
    qmi_request.new_config_info.config_type = PDC_CONFIG_TYPE_MODEM_SW_V01;
    qmi_request.new_config_info.config_id_len = msg->getConfigId().size() - 1;
    memcpy( qmi_request.new_config_info.config_id,
              msg->getConfigId().data(), qmi_request.new_config_info.config_id_len + 1 );

    auto ret = getMbnModule().getPendingMessageList().insert(msg);
    if(ret.second)
    {
        token = ret.first;
        qmi_error = qmi_client_pdc_send_raw_sync(QMI_PDC_GET_CONFIG_INFO_REQ_V01,
                                              &qmi_request, sizeof( qmi_request ),
                                              &qmi_response, sizeof( qmi_response ));

        result = qcril_qmi_util_convert_qmi_response_codes_to_ril_result( qmi_error, &qmi_response.resp );
    }
    else
    {
        QCRIL_LOG_ERROR("Failed to insert to PendingList for get config info");
        result = RIL_E_GENERIC_FAILURE;
    }
  } while (0);

  if ( result != RIL_E_SUCCESS )
  {
    QCRIL_LOG_ERROR("Failed to qcril_qmi_pdc_get_qc_version_of_configid");

    getMbnModule().getPendingMessageList().erase(token);

    g_pdc_info.is_under_processing = 0;
    auto resp = std::make_shared<QcRilRequestMessageCallbackPayload>(result, nullptr);
    msg->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
  }

  QCRIL_LOG_FUNC_RETURN();
}
#endif

//===========================================================================
// QCRIL_EVT_HOOK_GET_OEM_VERSION_OF_CONFIGID
//===========================================================================
void qcril_qmi_pdc_get_oem_version_of_configid
(
  std::shared_ptr<QcRilRequestGetOemVersionOfConfigMessage> msg
)
{
  pdc_get_config_info_req_msg_v01 qmi_request;
  pdc_get_config_info_resp_msg_v01 qmi_response;
  RIL_Errno result = RIL_E_GENERIC_FAILURE;
  qmi_client_error_type qmi_error;
  int token = 0;

  QCRIL_LOG_FUNC_ENTRY();

  do
  {
    if (!qcril_qmi_pdc_acquire_pdc_ownership_for_oem_hook())
    {
        QCRIL_LOG_ERROR("no ownership");
        result = RIL_E_GENERIC_FAILURE;
        break;
    }

    if ( qmi_ril_get_process_instance_id() != QCRIL_DEFAULT_INSTANCE_ID )
    {
      QCRIL_LOG_ERROR("QMI PDC client is only available on primary subscrition");
      result = RIL_E_GENERIC_FAILURE;
      break;
    }

    if ( g_pdc_info.is_under_processing )
    {
      QCRIL_LOG_ERROR("QMI PDC is busy");
      result = RIL_E_GENERIC_FAILURE;
      break;
    }
    g_pdc_info.is_under_processing = 1;

    memset( &qmi_request, 0, sizeof(qmi_request) );
    qmi_request.new_config_info.config_type = PDC_CONFIG_TYPE_MODEM_SW_V01;
    qmi_request.new_config_info.config_id_len = msg->getConfigId().size() - 1;
    memcpy( qmi_request.new_config_info.config_id,
              msg->getConfigId().data(), qmi_request.new_config_info.config_id_len + 1 );

    auto ret = getMbnModule().getPendingMessageList().insert(msg);
    if(ret.second)
    {
       token = ret.first;
       qmi_error = qmi_client_pdc_send_raw_sync(QMI_PDC_GET_CONFIG_INFO_REQ_V01,
                                              &qmi_request, sizeof( qmi_request ),
                                              &qmi_response, sizeof( qmi_response ));

      result = qcril_qmi_util_convert_qmi_response_codes_to_ril_result( qmi_error, &qmi_response.resp );
    }
    else
    {
        QCRIL_LOG_ERROR("Failed to insert to PendingList for get config info");
        result = RIL_E_GENERIC_FAILURE;
    }
  } while (0);

  if ( result != RIL_E_SUCCESS )
  {
    QCRIL_LOG_ERROR("Failed to qcril_qmi_pdc_get_oem_version_of_configid");

    getMbnModule().getPendingMessageList().erase(token);

    g_pdc_info.is_under_processing = 0;
    auto resp = std::make_shared<QcRilRequestMessageCallbackPayload>(result, nullptr);
    msg->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
  }

  QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
// QCRIL_EVT_HOOK_VALIDATE_CONFIG
//===========================================================================
void qcril_qmi_pdc_validate_config
(
  std::shared_ptr<QcRilRequestValidateMbnConfigMessage> msg
)
{
  pdc_validate_config_req_msg_v01 qmi_request;
  pdc_validate_config_resp_msg_v01 qmi_response;
  qmi_client_error_type qmi_error;
  RIL_Errno result = RIL_E_GENERIC_FAILURE;
  int len;

  QCRIL_LOG_FUNC_ENTRY();
  do
  {
    g_pdc_info.dump_fd = -1;

    if (!qcril_qmi_pdc_acquire_pdc_ownership_for_oem_hook())
    {
        QCRIL_LOG_ERROR("no ownership");
        result = RIL_E_GENERIC_FAILURE;
        break;
    }

    if ( qmi_ril_get_process_instance_id() != QCRIL_DEFAULT_INSTANCE_ID )
    {
      QCRIL_LOG_ERROR("QMI PDC client is only available on primary subscrition");
      result = RIL_E_GENERIC_FAILURE;
      break;
    }
    if ( g_pdc_info.is_under_processing )
    {
      QCRIL_LOG_ERROR("QMI PDC is busy");
      result = RIL_E_GENERIC_FAILURE;
      break;
    }
    g_pdc_info.is_under_processing = 1;

    // fill qmi request structure & check parameter
    memset( &qmi_request, 0, sizeof(qmi_request) );
    qmi_request.config_type = PDC_CONFIG_TYPE_MODEM_SW_V01;

    qmi_request.subscription_id = msg->getSubId();
    qmi_request.subscription_id_valid = 1;
    QCRIL_LOG_INFO("sub_id is %d", qmi_request.subscription_id);

    qmi_request.config_id_len = msg->getConfigId().size() - 1;
    memcpy((char *)qmi_request.config_id,
        msg->getConfigId().data(), qmi_request.config_id_len + 1 );
    qmi_request.config_id_valid = 1;
    QCRIL_LOG_INFO("config id is %s, config_id_len is %d",
                      qmi_request.config_id, qmi_request.config_id_len);

    // create DIFF dump file
    len = snprintf(g_pdc_info.dump_file, QCRIL_DUMP_FILE_PATH_LEN,
            QCRIL_DUMP_FILE_PREFIX"%d.xml", qmi_request.subscription_id);
    if ( len == QCRIL_DUMP_FILE_PATH_LEN )
    {
      QCRIL_LOG_ERROR("unexpected: dump length exceed");
      break;
    }
    // permission should be "radio:radio -rw-r--r--"
    g_pdc_info.dump_fd = open(g_pdc_info.dump_file, O_WRONLY | O_CREAT | O_TRUNC,
                                    S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if ( g_pdc_info.dump_fd == -1 )
    {
      QCRIL_LOG_ERROR("failed to create dump file: %s, %s",
                                    g_pdc_info.dump_file, strerror(errno));
      break;
    }

    // send validate request to MCFG
    qmi_error = qmi_client_pdc_send_raw_sync(QMI_PDC_VALIDATE_CONFIG_REQ_V01,
                                            &qmi_request, sizeof( qmi_request ),
                                            &qmi_response, sizeof( qmi_response ));

    result = qcril_qmi_util_convert_qmi_response_codes_to_ril_result( qmi_error, &qmi_response.resp);
  } while (0);

  if ( result != RIL_E_SUCCESS )
  {
    g_pdc_info.is_under_processing = 0;
    if ( g_pdc_info.dump_fd != -1 )
      close(g_pdc_info.dump_fd);
  }

  auto resp = std::make_shared<QcRilRequestMessageCallbackPayload>(result, nullptr);
  msg->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);

  QCRIL_LOG_FUNC_RETURN();

}

/*=========================================================================

  FUNCTION: qcril_qmi_pdc_unsol_ind_cb_hndlr

===========================================================================*/
/*!
    @brief
    Callback for QMI PDC indications

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_pdc_unsol_ind_cb_hndlr
(
    unsigned int   msg_id,
    unsigned char * decoded_payload,
    uint32_t decoded_payload_len
)
{
  QCRIL_LOG_FUNC_ENTRY();
  do {
      switch(msg_id)
      {
        case QMI_PDC_LOAD_CONFIG_IND_V01:
            // TODO: add load configure indication process
            qcril_qmi_pdc_load_config_ind_hdlr(decoded_payload, decoded_payload_len);
        break;

        case QMI_PDC_SET_SELECTED_CONFIG_IND_V01:
            qcril_qmi_pdc_select_config_ind_hdlr(decoded_payload, decoded_payload_len);
        break;

        case QMI_PDC_ACTIVATE_CONFIG_IND_V01:
            qcril_qmi_pdc_activate_config_ind_hdlr(decoded_payload, decoded_payload_len);
        break;

        case QMI_PDC_SET_FEATURE_IND_V01:
            qcril_qmi_pdc_set_feature_ind_hndlr(decoded_payload, decoded_payload_len);
        break;

        case QMI_PDC_GET_SELECTED_CONFIG_IND_V01:
            qcril_qmi_pdc_get_selected_config_ind_hdlr(decoded_payload, decoded_payload_len);
        break;

        case QMI_PDC_DEACTIVATE_CONFIG_IND_V01:
            qcril_qmi_pdc_deactivate_config_ind_hdlr(decoded_payload, decoded_payload_len);
        break;

        case QMI_PDC_LIST_CONFIGS_IND_V01:
            qcril_qmi_pdc_list_configs_ind_hdlr(decoded_payload, decoded_payload_len);
        break;

        case QMI_PDC_DELETE_CONFIG_IND_V01:
            qcril_qmi_pdc_delete_config_ind_hdlr(decoded_payload, decoded_payload_len);
        break;

        case QMI_PDC_GET_CONFIG_INFO_IND_V01:
            qcril_qmi_pdc_get_config_info_ind_hdlr(decoded_payload, decoded_payload_len);
        break;

#ifndef QMI_RIL_UTF
        case QMI_PDC_VALIDATE_CONFIG_IND_V01:
            qcril_qmi_pdc_validate_config_ind_hdlr(decoded_payload, decoded_payload_len);
        break;
#endif

        default:
            QCRIL_LOG_INFO("Unsupported QMI PDC indication %x hex", msg_id);
        break;
    }
  } while (0);

  QCRIL_LOG_FUNC_RETURN();
}

/*===========================================================================

  FUNCTION:  qcril_qmi_pdc_init

===========================================================================*/
/*!
    @brief
    Initialize the PDC subsystem of the RIL.

    @return
    None.
*/
 /*=========================================================================*/

qmi_client_error_type qcril_qmi_pdc_init
(
  void
)
{
  qmi_client_error_type qmi_err = QMI_NO_ERR;
  char property_name[ PROPERTY_NAME_MAX ];
  int sub_id = 0;

  QCRIL_LOG_FUNC_ENTRY();

  QCRIL_SNPRINTF( property_name, sizeof( property_name ), "%s", QCRIL_MODEM_MBN_FILE_PATH );
  property_get( property_name, g_pdc_info.mbn_file_dir, QCRIL_MODEM_MBN_DEFAULT_PATH );
  /* TODO: check if the data path is valid */

  for(sub_id = 0; sub_id < QCRIL_PDC_MAX_SUBS_SUPPORT; sub_id++)
  {
    memset(g_pdc_info.active_config_id[sub_id], 0,
           sizeof(g_pdc_info.active_config_id[sub_id]));
    g_pdc_info.active_config_id_len[sub_id] = 0;
  }

  QCRIL_LOG_FUNC_RETURN_WITH_RET(qmi_err);

  return (qmi_err);
}

/*===========================================================================

  FUNCTION:  qcril_qmi_pdc_set_sim_info

===========================================================================*/
/*!
    @brief
    Set pdc iccid, mcc, mnc

    @return
    None.
*/
 /*=========================================================================*/

void qcril_qmi_pdc_set_sim_info
(
    char *iccid,
    char *mcc,
    char *mnc
)
{
  char file_name[100];
  char final_string[100];
  char curr_string[101];

#ifndef QMI_RIL_UTF
  QCRIL_LOG_FUNC_ENTRY();

  int  instance_id = qmi_ril_get_process_instance_id();
  int  ret = 0;

  QCRIL_LOG_FUNC_ENTRY();

  QCRIL_SNPRINTF(file_name, sizeof(file_name),
                 "%s_%d", QCRIL_MBN_SIM_INFO_FILE, instance_id);

  ret = qcril_file_read_string_from_file(file_name, curr_string,
                                         sizeof(curr_string),
                                         sizeof(curr_string) - 1);
  QCRIL_LOG_DEBUG("file : %s", file_name);
  QCRIL_LOG_DEBUG("curr_string : %s", curr_string);

  QCRIL_SNPRINTF(final_string, sizeof(final_string),
                 "%s,%s,%s",
                 iccid? iccid: "null",
                 mcc? mcc: "null",
                 mnc? mnc: "null");
  QCRIL_LOG_DEBUG("final_string : %s", final_string);

  //Write to file only if there is a change in SIM info
  //or if the file is not created. Otherwise it will
  //unnecessarily trigger MBN loading
  if ((ret != 0) ||
      strcmp(curr_string, final_string))
  {
    qcril_file_write_a_string_to_file(file_name, final_string);
  }
  else
  {
    QCRIL_LOG_DEBUG("No change in sim info");
  }
#endif
}

//===========================================================================
// qcril_qmi_pdc_sim_info_update_handler
//===========================================================================
void qcril_qmi_pdc_sim_info_update_handler
(
    const qmi_ril_file_observer_change_event event
)
{
    if (QMI_RIL_FILE_OBSERVER_CHANGE_CLOSE_WRITE == event)
    {
        if (qcril_mbn_hw_is_completed())
        {
            //qcril_mbn_sw_update_init(FALSE);
            std::shared_ptr<MbnFileUpdateIndication> ind_msg_ptr =
                            std::make_shared<MbnFileUpdateIndication>();
            if (ind_msg_ptr != nullptr) {
                ind_msg_ptr->broadcast();
            }
        }
    }
}

/*===========================================================================

  FUNCTION:  qcril_qmi_pdc_pre_init

===========================================================================*/
/*!
    @brief
    Pre init pdc

    @return
    None.
*/
 /*=========================================================================*/
void qcril_qmi_pdc_pre_init
(
  void
)
{
    char file_name[100];
    int  i;
    int  max_apss_sub;
    struct stat st;

    memset(&st, 0x0, sizeof(struct stat));
    qcril_qmi_pdc_set_sim_info(NULL, NULL, NULL);

    qcril_qmi_mbn_sw_init();
    max_apss_sub = qcril_qmi_dms_get_max_apss_subscritions();

    //Register for file change events only on primary RIL
    //as MBN loading will happen only from primary rild.
    if ( qmi_ril_get_process_instance_id() == QCRIL_DEFAULT_INSTANCE_ID )
    {
        QCRIL_LOG_DEBUG("Register sim info file change events");
        for (i = 0; i < max_apss_sub; i++)
        {
            QCRIL_SNPRINTF(file_name,
                           sizeof(file_name),
                           "%s_%d", QCRIL_MBN_SIM_INFO_FILE,
                           i);

            if (stat(QCRIL_MBN_SIM_INFO_DIR, &st) == -1)
            {
                QCRIL_LOG_ERROR("Directory %s doesn't exist", QCRIL_MBN_SIM_INFO_DIR);
            }
            else
            {
                if (stat(file_name, &st) == -1)
                {
                    //Create and close the file if it doesnt exist. Then
                    //register for the file change events for this file.

                    //this function will create a file if not exists and write string to it
                    char final_string[100];
                    QCRIL_SNPRINTF(final_string, sizeof(final_string),
                                   "%s,%s,%s", "null", "null", "null");
                    QCRIL_LOG_DEBUG("final_string : %s", final_string);
                    qcril_file_write_a_string_to_file(file_name, final_string);
                }

                qmi_ril_file_observer_register_for_change_event(
                                                file_name,
                                                (qmi_ril_file_observer_state_change_handler)qcril_qmi_pdc_sim_info_update_handler);
            }
        }
    }
}

/*===========================================================================

  FUNCTION: qcril_qmi_pdc_get_sim_info

===========================================================================*/
/*!
    @brief
    Get pdc iccid, mcc, mnc

    @return
    None.
*/
 /*=========================================================================*/

void qcril_qmi_pdc_get_sim_info
(
    int   instance_id,
    char *iccid,
    int   icc_id_len,
    char  *mcc,
    char  mcc_len,
    char  *mnc,
    char  mnc_len
)
{
  char file_name[100];
  char string[101];
  char *str = NULL;
  char *lastPtr = NULL;
  RIL_Errno ret = RIL_E_SUCCESS;

  QCRIL_LOG_FUNC_ENTRY();

  if (iccid && icc_id_len && mcc && mcc_len && mnc && mnc_len)
  {
    QCRIL_SNPRINTF(file_name,
                   sizeof(file_name),
                   "%s_%d", QCRIL_MBN_SIM_INFO_FILE,
                   instance_id);
    memset(string, 0, sizeof(string));
    memset(iccid, 0, icc_id_len);
    memset(mcc, 0, mcc_len);
    memset(mnc, 0, mnc_len);

    ret = (RIL_Errno) qcril_file_read_string_from_file(file_name, string, sizeof(string),
                                           sizeof(string) - 1);
    QCRIL_LOG_DEBUG("property : %s", file_name);
    QCRIL_LOG_DEBUG("string : %s", string);

    if (!ret)
    {
        str = strtok_r (string, ",", &lastPtr);

        if (str)
        {
            strlcpy(iccid, str, icc_id_len);
        }

        str = strtok_r (NULL, ",", &lastPtr);
        if (str)
        {
            strlcpy(mcc, str, mcc_len);
        }

        str = strtok_r (NULL, ",", &lastPtr);
        if (str)
        {
            strlcpy(mnc, str, mnc_len);
        }

        QCRIL_LOG_DEBUG("iccid: %s", iccid);
        QCRIL_LOG_DEBUG("mcc: %s", mcc);
        QCRIL_LOG_DEBUG("mnc: %s", mnc);
    }
    else
    {
        QCRIL_LOG_DEBUG("cannot open file %s", file_name);
    }
  }

  QCRIL_LOG_FUNC_RETURN_WITH_RET(ret);
}


/*===========================================================================

  FUNCTION  qcril_qmi_pdc_retrieve_current_mbn_info

===========================================================================*/
/*!
    @brief
    retrieve active mbn info

    @return
    0 on success
*/
/*=========================================================================*/
void qcril_qmi_pdc_retrieve_current_mbn_info
(
    void
)
{
    pdc_get_selected_config_req_msg_v01  qmi_request;
    pdc_get_selected_config_resp_msg_v01 qmi_response;
    qmi_client_error_type                qmi_error;
    RIL_Errno          result = RIL_E_GENERIC_FAILURE;

    QCRIL_LOG_FUNC_ENTRY();
    memset(&qmi_request, 0, sizeof(qmi_request));

    qmi_request.config_type           = PDC_CONFIG_TYPE_MODEM_SW_V01;
    qmi_request.subscription_id_valid = 1;
    qmi_request.subscription_id       = 0;
    PDC_CONFIG_LOCK();
    /* update state that retrieveing mbn info */
    g_pdc_info.is_retrieving_mbn_info = QCRIL_PDC_MBN_INFO_RETRIEVE_ACTIVE_ID;
    PDC_CONFIG_UNLOCK();
    qmi_error = qmi_client_pdc_send_raw_sync(QMI_PDC_GET_SELECTED_CONFIG_REQ_V01,
                                            &qmi_request, sizeof( qmi_request ),
                                            &qmi_response, sizeof( qmi_response ));

    result = qcril_qmi_util_convert_qmi_response_codes_to_ril_result( qmi_error, &qmi_response.resp );
    if (result != RIL_E_SUCCESS)
    {
        PDC_CONFIG_LOCK();

        /* update state that retrieveing active mbn info */
        g_pdc_info.is_retrieving_mbn_info = QCRIL_PDC_MBN_INFO_NONE;
        qcril_request_clean_up_suppress_list();
        PDC_CONFIG_UNLOCK();
    }
    QCRIL_LOG_FUNC_RETURN_WITH_RET(result);
}

/*===========================================================================

  FUNCTION:  qcril_qmi_pdc_get_sub_id

===========================================================================*/
/*!
    @brief
    Get pdc sub_id

    @return
    None.
*/
 /*=========================================================================*/
int qcril_qmi_pdc_get_sub_id
(
    int instance_id
)
{
  char property_name[ PROPERTY_NAME_MAX ];
  int sub_id;

  QCRIL_LOG_FUNC_ENTRY();

  QCRIL_LOG_DEBUG("instance id %d", instance_id);
  QCRIL_SNPRINTF(property_name,
                 sizeof( property_name ),
                 "%s_%d", QCRIL_MBN_ACTIVATION_STATUS_PROP_NAME,
                 instance_id);

  qmi_ril_get_property_value_from_integer(property_name, &sub_id, 0);

  QCRIL_LOG_FUNC_RETURN_WITH_RET(sub_id);
  return sub_id;
}
/*===========================================================================

  FUNCTION:  qcril_qmi_pdc_set_sub_id

===========================================================================*/
/*!
    @brief
    Initialize the PDC subsystem of the RIL.

    @return
    None.
*/
 /*=========================================================================*/
void qcril_qmi_pdc_set_sub_id
(
  int sub_id
)
{
  char property_name[ PROPERTY_NAME_MAX ];

  QCRIL_LOG_FUNC_ENTRY();

  QCRIL_LOG_DEBUG("sub_id %d", sub_id);
  QCRIL_SNPRINTF(property_name,
                 sizeof( property_name ),
                 "%s_%d", QCRIL_MBN_ACTIVATION_STATUS_PROP_NAME,
                 qmi_ril_get_process_instance_id());
  qmi_ril_set_property_value_to_integer(property_name, sub_id);

}

/*===========================================================================

  FUNCTION  qcril_qmi_pdc_retrieve_mbn_info_for_config_id

===========================================================================*/
/*!
    @brief
    retrieve mbn info for config id

    @return
    0 on success
*/
/*=========================================================================*/
void qcril_qmi_pdc_retrieve_mbn_info_for_config_id
(
    void *arg
)
{
    pdc_get_config_info_req_msg_v01      qmi_request;
    pdc_get_config_info_resp_msg_v01     qmi_response;
    qmi_client_error_type                qmi_error;
    RIL_Errno                      result = RIL_E_GENERIC_FAILURE;
    uint8_t                              config_id[PDC_CONFIG_ID_SIZE_MAX_V01];
    uint32_t                             config_id_len;
    QCRIL_NOTUSED(arg);

    QCRIL_LOG_FUNC_ENTRY();
    memset(&qmi_request, 0, sizeof(qmi_request));
    qmi_request.new_config_info.config_type = PDC_CONFIG_TYPE_MODEM_SW_V01;

    PDC_CONFIG_LOCK();
    g_pdc_info.is_under_processing = 1;
    memcpy(config_id, g_pdc_info.config_id, g_pdc_info.config_id_len);
    config_id_len = g_pdc_info.config_id_len;
    PDC_CONFIG_UNLOCK();

    if ((0 < config_id_len) &&
         (config_id_len <= PDC_CONFIG_ID_SIZE_MAX_V01))
    {
      qmi_request.new_config_info.config_id_len = config_id_len;
      memcpy(qmi_request.new_config_info.config_id,
                  config_id, config_id_len);
      PDC_CONFIG_LOCK();
      g_pdc_info.is_retrieving_mbn_info = QCRIL_PDC_MBN_INFO_RETRIEVE_CONFIG_INFO;
      PDC_CONFIG_UNLOCK();
      qmi_error = qmi_client_pdc_send_raw_sync(QMI_PDC_GET_CONFIG_INFO_REQ_V01,
                                              &qmi_request, sizeof( qmi_request ),
                                              &qmi_response, sizeof( qmi_response ));

      result = qcril_qmi_util_convert_qmi_response_codes_to_ril_result( qmi_error, &qmi_response.resp );
    }

    if (result != RIL_E_SUCCESS)
    {
        PDC_CONFIG_LOCK();

        g_pdc_info.is_under_processing = 0;
        /* update state that retrieveing active mbn info */
        g_pdc_info.is_retrieving_mbn_info = QCRIL_PDC_MBN_INFO_NONE;
        qcril_request_clean_up_suppress_list();
        PDC_CONFIG_UNLOCK();
    }
    QCRIL_LOG_FUNC_RETURN_WITH_RET(result);
}

/*===========================================================================

  FUNCTION  qcril_qmi_pdc_is_test_mbn

===========================================================================*/
/*!
    @brief
    check if the mbn is test mbn

    @return
    0 on success
*/
/*=========================================================================*/
boolean qcril_qmi_pdc_is_test_mbn(void *data, unsigned int len)
{
    boolean ret = FALSE;
    char text[PDC_CONFIG_DESC_SIZE_MAX_V01 + 1];
    do
    {
        if ((data == NULL) || (len == 0) || (len > PDC_CONFIG_DESC_SIZE_MAX_V01))
        {
            break;
        }

        memset (text, 0, PDC_CONFIG_DESC_SIZE_MAX_V01 + 1);
        memcpy (text, data, len);

        if (!strcasestr(text, QCRIL_PDC_COMMERCIAL_MBN_PREFIX))
        {
            /* This is test mbn */
            ret = TRUE;
        }

    } while (FALSE);
    QCRIL_LOG_FUNC_RETURN_WITH_RET(ret);
    return ret;
}

/*=========================================================================

  FUNCTION:  qcril_qmi_pdc_deactivate_configuration_hdlr

===========================================================================*/
/*!
    @brief
    Deactivate configuration

    @return
    None.
*/
/*=========================================================================*/
RIL_Errno qcril_qmi_pdc_deactivate_configuration_hdlr
(
    uint32_t                 sub_id,
    pdc_config_type_enum_v01 config_type
)
{
    pdc_deactivate_config_req_msg_v01  qmi_request;
    pdc_deactivate_config_resp_msg_v01 qmi_response;
    qmi_client_error_type              qmi_error;
    RIL_Errno                          result = RIL_E_GENERIC_FAILURE;
    errno_enum_type                    reqlist_found = E_FAILURE;
    qcril_reqlist_public_type          req_info;

    QCRIL_LOG_FUNC_ENTRY();

    memset(&qmi_request, 0, sizeof(qmi_request));
    qmi_request.config_type       = config_type;

    if (config_type != PDC_CONFIG_TYPE_MODEM_PLATFORM_V01)
    {
        qmi_request.subscription_id_valid = 1;
        qmi_request.subscription_id = sub_id;
    }

    g_pdc_info.is_under_processing = 1;
    g_pdc_info.cur_deact_sub = sub_id;
    qmi_error = qmi_client_pdc_send_raw_sync(QMI_PDC_DEACTIVATE_CONFIG_REQ_V01,
                                        &qmi_request, sizeof( qmi_request ),
                                        &qmi_response, sizeof( qmi_response ));

    result = qcril_qmi_util_convert_qmi_response_codes_to_ril_result(qmi_error,
                                                              &qmi_response.resp);

    g_pdc_info.pdc_config_info.config_type = config_type;
    if (result != RIL_E_SUCCESS)
    {
        QCRIL_LOG_DEBUG("Error in deactivating config. Type: %d, sub_id: %d",
                config_type, sub_id);

        reqlist_found = qcril_reqlist_query_by_event(QCRIL_DEFAULT_INSTANCE_ID,
                                        QCRIL_DEFAULT_MODEM_ID,
                                        QCRIL_EVT_REQUEST_MBN_HW_DEACTIVATE_CONFIG,
                                        &req_info );
        if (reqlist_found == E_SUCCESS)
        {
            g_pdc_info.is_under_processing = 0;
            qcril_mbn_hw_send_deactivate_config_resp(result);
        }
        else
        {
            reqlist_found = qcril_reqlist_query_by_event(QCRIL_DEFAULT_INSTANCE_ID,
                                            QCRIL_DEFAULT_MODEM_ID,
                                            QCRIL_EVT_REQUEST_MBN_SW_DEACTIVATE_CONFIG,
                                            &req_info );
            if (reqlist_found == E_SUCCESS)
            {
                g_pdc_info.is_under_processing = 0;
                qcril_mbn_sw_send_deactivate_config_resp(result);
            }
            else
            {
                // if the there is the error, we still do the delete work anyway
                // no need to check the result
                g_pdc_info.cur_deact_sub++;
                if ( g_pdc_info.cur_deact_sub >= g_pdc_info.sub_count )
                {
                  if (g_pdc_info.is_under_cleanup)
                  {
                    QCRIL_LOG_DEBUG("is_under_cleanup: %d", g_pdc_info.is_under_cleanup);
                    // kick the delete all start
                    qcril_event_queue( QCRIL_DEFAULT_INSTANCE_ID,
                                    QCRIL_DEFAULT_MODEM_ID,
                                    QCRIL_DATA_ON_STACK,
                                    QCRIL_EVT_QMI_RIL_PDC_DELETE_ALL,
                                    NULL,
                                    QMI_RIL_ZERO,
                                    (RIL_Token) QCRIL_MBN_INTERNAL_TOKEN_ID );
                  }
                  else
                  {
                    // send response for deactivate
                    std::shared_ptr<Message> pendingMsg = getMbnModule().getPendingMessageList().find(
                    QcRilRequestDeactivateMbnConfigMessage::get_class_message_id());

                    if (pendingMsg != nullptr)
                    {
                      result = RIL_E_SUCCESS;
                      getMbnModule().getPendingMessageList().erase(pendingMsg);

                      auto resp = std::make_shared<QcRilRequestMessageCallbackPayload>(result, nullptr);
                      auto ril_msg(std::static_pointer_cast<QcRilRequestDeactivateMbnConfigMessage>(pendingMsg));
                      ril_msg->sendResponse(ril_msg, Message::Callback::Status::SUCCESS, resp);
                    }
                    else
                    {
                      QCRIL_LOG_DEBUG("unable to find reqlist by message: QcRilRequestDeactivateMbnConfigMessage");
                    }
                    g_pdc_info.is_under_processing = 0;
                  }
                }
                else
                {
                  // continue to deactivate the next SUB
                  qcril_event_queue( QCRIL_DEFAULT_INSTANCE_ID,
                                  QCRIL_DEFAULT_MODEM_ID,
                                  QCRIL_DATA_ON_STACK,
                                  QCRIL_EVT_QMI_RIL_PDC_DEACTIVATE_CONFIGURATION,
                                  &g_pdc_info.pdc_config_info.config_type,
                                  sizeof(g_pdc_info.pdc_config_info.config_type),
                                  (RIL_Token) QCRIL_MBN_INTERNAL_TOKEN_ID );
                }
            }
        }
    }

    QCRIL_LOG_FUNC_RETURN();
    return result;
}

/*=========================================================================

  FUNCTION:  qcril_qmi_pdc_activate_configuration_hdlr

===========================================================================*/
/*!
    @brief
    Activate configuration

    @return
    None.
*/
/*=========================================================================*/
RIL_Errno qcril_qmi_pdc_activate_configuration_hdlr
(
    uint32_t                 sub_id,
    pdc_config_type_enum_v01 config_type
)
{
    pdc_activate_config_req_msg_v01  qmi_request;
    pdc_activate_config_resp_msg_v01 qmi_response;
    qmi_client_error_type            qmi_error;
    RIL_Errno                        result = RIL_E_GENERIC_FAILURE;

    QCRIL_LOG_FUNC_ENTRY();

    memset(&qmi_request, 0, sizeof(qmi_request));
    qmi_request.config_type       = config_type;

    if (config_type != PDC_CONFIG_TYPE_MODEM_PLATFORM_V01)
    {
        qmi_request.subscription_id_valid = 1;
        qmi_request.subscription_id = sub_id;
    }

    g_pdc_info.is_under_processing = 1;
    qmi_error = qmi_client_pdc_send_raw_sync(QMI_PDC_ACTIVATE_CONFIG_REQ_V01,
                                        &qmi_request, sizeof( qmi_request ),
                                        &qmi_response, sizeof( qmi_response ));

    result = qcril_qmi_util_convert_qmi_response_codes_to_ril_result(qmi_error,
                                                              &qmi_response.resp);

    if (result != RIL_E_SUCCESS)
    {
        g_pdc_info.is_under_processing = 0;
    }

    QCRIL_LOG_FUNC_RETURN();
    return result;
}


/*=========================================================================

  FUNCTION:  qcril_qmi_pdc_delete_configuration_hdlr

===========================================================================*/
/*!
    @brief
    Delete configuration

    @return
    None.
*/
/*=========================================================================*/
RIL_Errno qcril_qmi_pdc_delete_configuration_hdlr
(
    uint8_t                 *config_id,
    uint32_t                 config_id_len,
    pdc_config_type_enum_v01 config_type
)
{
    pdc_delete_config_req_msg_v01  qmi_request;
    pdc_delete_config_resp_msg_v01 qmi_response;
    qmi_client_error_type          qmi_error;
    RIL_Errno                      result = RIL_E_GENERIC_FAILURE;

    QCRIL_LOG_FUNC_ENTRY();

    do {
        // treat it as error if config_id_len is too large
        if (config_id_len >= PDC_CONFIG_ID_SIZE_MAX_V01)
        {
            break;
        }

        memset(&qmi_request, 0, sizeof(qmi_request));
        if (config_id && (config_id_len > 0))
        {
            qmi_request.config_id_len = config_id_len;
            memcpy(qmi_request.config_id, config_id, qmi_request.config_id_len);
            QCRIL_LOG_INFO("request to delete config id: %s",
                            qmi_request.config_id);
            qmi_request.config_id_valid = TRUE;
        }
        else
        {
            // config_id is null or config_id_len equals 0
            qmi_request.config_id_valid = FALSE;
            QCRIL_LOG_INFO("request to delete all MBNs, due to no config_id specified");
        }
        qmi_request.config_type       = config_type;

        qmi_error = qmi_client_pdc_send_raw_sync(QMI_PDC_DELETE_CONFIG_REQ_V01,
                         &qmi_request,
                         sizeof(qmi_request),
                         &qmi_response,
                         sizeof(qmi_response));

        result = qcril_qmi_util_convert_qmi_response_codes_to_ril_result(qmi_error,
                                                      &qmi_response.resp);
    } while (0);

    QCRIL_LOG_FUNC_RETURN();
    return result;
}

/*=========================================================================

  FUNCTION:  qcril_qmi_pdc_set_select_configuration

===========================================================================*/
/*!
    @brief
    Select a configuration

    @return
    None.
*/
/*=========================================================================*/
RIL_Errno qcril_qmi_pdc_set_select_configuration
(
    uint32_t                 sub_id,
    pdc_config_type_enum_v01 config_type,
    uint8_t                 *config_id,
    uint32_t                 config_id_len
)
{
    pdc_set_selected_config_req_msg_v01     qmi_request;
    pdc_set_selected_config_resp_msg_v01    qmi_response;
    pdc_config_info_req_type_v01           *config_info;
    qmi_client_error_type                   qmi_error;
    RIL_Errno                               result = RIL_E_GENERIC_FAILURE;

    QCRIL_LOG_FUNC_ENTRY();

    do {
        if ((!config_id) || (config_id_len >= PDC_CONFIG_ID_SIZE_MAX_V01))
        {
            break;
        }

        g_pdc_info.is_under_processing = 1;
        g_pdc_info.cur_select_sub = sub_id;
        memset(&qmi_request, 0, sizeof(qmi_request));

        config_info = &qmi_request.new_config_info;
        config_info->config_type   = config_type;
        config_info->config_id_len = config_id_len;
        memcpy(config_info->config_id, config_id, config_info->config_id_len);

        g_pdc_info.config_id_len = config_id_len;
        memcpy(g_pdc_info.config_id, config_id, g_pdc_info.config_id_len);

        if (config_type != PDC_CONFIG_TYPE_MODEM_PLATFORM_V01)
        {
            qmi_request.subscription_id_valid = 1;
            qmi_request.subscription_id = sub_id;
        }

        qmi_error = qmi_client_pdc_send_raw_sync(QMI_PDC_SET_SELECTED_CONFIG_REQ_V01,
                                            &qmi_request, sizeof( qmi_request ),
                                            &qmi_response, sizeof( qmi_response ));

        result = qcril_qmi_util_convert_qmi_response_codes_to_ril_result(
                                                qmi_error, &qmi_response.resp);
    } while (0);

    if (result != RIL_E_SUCCESS)
    {
        g_pdc_info.is_under_processing = 0;
    }

    QCRIL_LOG_FUNC_RETURN();

    return result;
}

//===========================================================================
// qcril_qmi_pdc_enable_modem_update
//===========================================================================
void qcril_qmi_pdc_enable_modem_update
(
  std::shared_ptr<QcRilRequestEnableModemUpdateMessage> msg
)
{
    pdc_set_feature_req_msg_v01 qmi_request;
    pdc_set_feature_resp_msg_v01  qmi_response;
    qmi_client_error_type qmi_error;
    RIL_Errno result = RIL_E_GENERIC_FAILURE;
    uint16_t token = 0;

    QCRIL_LOG_FUNC_ENTRY();

    do
    {
      if ( qmi_ril_get_process_instance_id() != QCRIL_DEFAULT_INSTANCE_ID )
      {
        QCRIL_LOG_ERROR("QMI PDC client is only available on primary subscrition");
        break;
      }

      if ( g_pdc_info.is_under_processing )
      {
        QCRIL_LOG_ERROR("QMI PDC is busy");
        break;
      }
      g_pdc_info.is_under_processing = 1;

      memset( &qmi_request, 0, sizeof(qmi_request) );
      qmi_request.slot_id = msg->getSubId();
      qmi_request.selection_mode_valid = true;
      qmi_request.selection_mode = PDC_SELECTION_MODE_HYBRID_V01;
      qmi_request.flex_mapping_valid = true;
      qmi_request.flex_mapping = true;
      qmi_request.carrier_valid = true;
      qmi_request.carrier = PDC_SELECTION_OPEN_MARKET_V01;
      qmi_request.refresh_mode_valid = true;
      qmi_request.refresh_mode = PDC_REFRESH_MODE_ENABLED_V01;

      auto ret = getMbnModule().getPendingMessageList().insert(msg);
      if(ret.second)
      {
        token = ret.first;
        qmi_error = qmi_client_pdc_send_raw_sync(QMI_PDC_SET_FEATURE_REQ_V01,
                                           &qmi_request, sizeof( qmi_request ),
                                           &qmi_response, sizeof( qmi_response ));
        result = qcril_qmi_util_convert_qmi_response_codes_to_ril_result(
                                  qmi_error,
                                  &qmi_response.resp );

      }
      else
      {
            QCRIL_LOG_ERROR("Failed to insert to PendingList for set mbn feature");
      }

    } while (0);

    QCRIL_LOG_INFO("result %d", result);
    if ( result != RIL_E_SUCCESS )
    {
      g_pdc_info.is_under_processing = 0;

      getMbnModule().getPendingMessageList().erase(token);
      auto resp = std::make_shared<QcRilRequestMessageCallbackPayload>(result, nullptr);
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
    }

    QCRIL_LOG_FUNC_RETURN();
}

void qcril_qmi_pdc_reset_mbn_state()
{
  QCRIL_LOG_INFO("Reset PDC MBN state");
  g_pdc_info.is_under_processing = 0;
}

#ifdef QMI_RIL_UTF
void qcril_qmi_hal_pdc_module_cleanup()
{
    getMbnModule().qcrilHalPdcModuleCleanup();
}
#endif
