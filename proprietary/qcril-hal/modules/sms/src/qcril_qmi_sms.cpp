/******************************************************************************
#  Copyright (c) 2010, 2013, 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
/******************************************************************************
  @file    qcril_qmi_sms.c
  @brief   qcril qmi - SMS

  DESCRIPTION
    Handles RIL requests, Callbacks, indications for QMI SMS.

******************************************************************************/


/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include <string.h>
#include <limits.h>
#include <cutils/properties.h>
#include <sstream>

#ifdef QMI_RIL_UTF
#include <errno.h>
#endif

//#include "qcril_qmi_ims_sms_util.h"
#include "IxErrno.h"
#include "comdef.h"
#include "qcrili.h"
#include "modules/nas/qcril_arb.h"
#include "modules/sms/qcril_qmi_sms.h"
#include "qcril_reqlist.h"
#ifdef __cplusplus
#include "qcril_sms_legacy.h"
#endif
#include "qcril_qmi_client.h"
#include "modules/nas/qcril_qmi_nas.h"
#include "qcril_qmi_sms_errors.h"
#include "qmi_errors.h"
#include "modules/voice/qcril_qmi_voice.h"
#include "qcril_cm_ss.h"
#include "qmi_ril_platform_dep.h"
#include "modules/nas/qcril_db.h"
#include "utilities/bit_field/utils_pack.h"
#include "modules/sms/RilUnsolIncoming3GppSMSMessage.h"
#include "modules/sms/RilUnsolIncoming3Gpp2SMSMessage.h"
#include "modules/sms/RilUnsolIncomingImsSMSMessage.h"
#include "modules/sms/RilUnsolNewSmsOnSimMessage.h"
#include "modules/sms/RilUnsolNewSmsStatusReportMessage.h"
#include "modules/sms/RilUnsolNewImsSmsStatusReportMessage.h"
#include "modules/sms/RilUnsolNewBroadcastSmsMessage.h"
#include "modules/sms/RilUnsolStkCCAlphaNotifyMessage.h"
#include "modules/sms/RilUnsolCdmaRuimSmsStorageFullMessage.h"
#include "modules/sms/RilUnsolSimSmsStorageFullMessage.h"
#include "modules/sms/RilUnsolImsNetworkStateChangedMessage.h"
#include "qcril_qmi_npb_utils.h"

#include <interfaces/sms/QcRilUnsolMessageWaitingInfoMessage.h>

#define TAG "QCRIL_SMS"

#include <framework/legacy.h>
#include "../modules/sms/src/SmsModule.h"
#include <framework/Log.h>
#include "qcril_qmi_oem_events.h"

#include "multipart_gsm_cmas_cellbroadcast.h"
//using namespace android::hardware::radio::V1_0;

/*===========================================================================

                    INTERNAL DEFINITIONS AND TYPES

===========================================================================*/
#define QCRIL_BLOCK_SMS_ON_1X                      "persist.vendor.radio.block_sms_on_1x"

#define QCRIL_QMI_WMS_LONG_SYNC_REQ_TIMEOUT        (3000)

#define SMS_REQUEST_CONCURRENCY_NUM (1)
#define SMS_REQUEST_PENDING_NUM (50)
#define SMS_TELEPHONY_MEMORY_EXCEEDED_ERROR (211)

#define QCRIL_MT_SMS_ACK_EXPRY_WINDOW       "persist.vendor.radio.mt_sms_ack"
#define QCRIL_SMS_LINK_TIMER                "persist.vendor.radio.sms_link_timer"
#define QCRIL_FORCE_ON_DC                   "persist.vendor.radio.force_on_dc"

const char* QCRIL_ACCEPT_ALL_BC_MSG       = "all_bc_msg";


/*===========================================================================

                         LOCAL VARIABLES

===========================================================================*/

// sms cache
static qcril_sms_struct_type *qcril_sms; ///< sms cache per instance
static qmi_ril_sms_svc_status_type qmi_ril_sms_svc_status;
static qmi_ril_sms_primary_client_status_type qmi_ril_sms_primary_client_status;
static int feature_ims_3gpp_retry = 1; /// accept/reject ims sms retries on 3gpp
static int feature_ims_3gpp2_retry = 1; /// accept/reject ims sms retries on 3gpp2
static int qmi_ril_sms_mt_expiry_window_len;
static int qmi_ril_sms_mt_expiry_window_len_set;
static int qmi_ril_sms_link_timer;
static int feature_all_bc_msg_3gpp = 0; /*accept all BroadCast msg for only 3GPP technology.*/
static int feature_all_bc_msg_3gpp2 = 0; /*accept all BroadCast msg for only 3GPP2 technology.*/

/* "is_force_on_dc" is only applicable for 3GPP2. Force the message to be sent on the CDMA dedicated channel.
Values: 0x00 - Do not care about the channel on which the message is sent
        0x01 - Request to send the message over the dedicated channel.*/
static int is_force_on_dc = 0;
static int is_wms_limited_ready_status = 0;

static qtimutex::QtiSharedMutex cache_mutex; /// Mutex for SMS cache data
static wms_service_ready_status_enum_v01 wms_service_state; ///< WMS service state
static boolean is_sms_memory_full = FALSE; // Is SMS memory full
static qcril_mt_sms_buffer_power_opt_payload qcril_mt_sms_buffer_power_opt;

/* storing whether a MO message specified by ref is on IMS or not
 * It is set in MO async cb if result is success, and query when
 * a event report comes. Bases on its value, RIl decide to send
 * event report through IMS or CS pipe*/
static std::unordered_map<uint16_t, bool> mo_sms_on_ims_map;

//============================================================================
// MACROS
//============================================================================
#define SMS_CACHE_LOCK do { \
        QCRIL_LOG_ADDITIONAL("Before acquiring SMS LOCK"); \
        cache_mutex.lock(); \
        QCRIL_LOG_ADDITIONAL("After acquiring SMS LOCK"); \
    }while(0)

#define SMS_CACHE_UNLOCK do { \
        QCRIL_LOG_ADDITIONAL("Before releasing SMS LOCK"); \
        cache_mutex.unlock(); \
        QCRIL_LOG_ADDITIONAL("After releasing SMS LOCK"); \
    }while(0)

/*===========================================================================

                                FUNCTIONS

===========================================================================*/
RIL_Errno qcril_sms_request_write_sms_to_uim(wms_raw_write_req_msg_v01 * write_request_msg,
                                        wms_message_tag_type_enum_v01 tag,
                                        wms_message_mode_enum_v01 message_mode,
                                        int& index_on_sim);

static void qcril_sms_mt_transaction_expired(void * param);
void qcril_sms_perform_transport_layer_info_initialization();
int qmi_ril_block_mo_sms_on_1x();

/// sets protected cach value wms_ready_status and logs its value
void set_wms_service_state(wms_service_ready_status_enum_v01);

/// check if wms ready indications are supported by the modem
int wms_ready_supported(void);

/// callback handler for async wms get ready status msg
void wms_get_ready_status_cb
(
    unsigned int           msg_id,
    std::shared_ptr<void>  resp_c_struct,
    unsigned int           resp_c_struct_len,
    void                   *resp_cb_data,
    qmi_client_error_type  transp_err
);

/// To get wms limited service support available or not.
/// @return 0 | 1 --> not supported, supported
int get_wms_limited_service_status();

/// Check that wms limited service status support available on modem side.
/// @return 0 | 1 --> not supported, supported
int wms_ready_limited_status_support(wms_service_ready_status_enum_v01 state);

/// Check that wms service is ready for <3GPP(minimum limited)> and return a boolean
/// @return 0 | 1 --> not ready, ready
int wms_ready_atleast_limited_3gpp();

/// Check that wms service is ready for <3GPP2(minimum limited)> and return a boolean
/// @return 0 | 1 --> not ready, ready
int wms_ready_atleast_limited_3gpp2();

/// Check that wms service is ready for FULL <3GPP> and return a boolean
/// @return 0 | 1 --> not ready, ready
int wms_ready_full_3gpp();

/// Check that wms service is ready for FULL <3GPP2> and return a boolean
/// @return 0 | 1 --> not ready, ready
int wms_ready_full_3gpp2();

/// Sends a ril response w/RIL_E_GENERIC_FAILURE
void send_generic_failure(RIL_Token token, qcril_evt_e_type event);

/// Convert sms service state to string for logging
const char* sms_state_str(wms_service_ready_status_enum_v01 service_state);

void qcril_qmi_sms_transfer_sim_ucs2_alpha_to_std_ucs2_alpha
(
    const wms_call_control_modified_info_type_v01 *sim_alpha,
    wms_call_control_modified_info_type_v01       *std_alpha
);

static qmi_client_error_type qcril_sms_send_sms_memory_status
(
    int is_memory_available,
    void  *user_data
);

qcril::interfaces::MessagePriority qcril_qmi_sms_map_qmi_mwi_priority_to_ims_priority
(
 wms_mwi_priority_type_enum_v01 wms_mwi_priority
);

qcril::interfaces::MessageType  qcril_qmi_sms_map_qmi_mwi_msg_type_to_ims_msg_type
(
 transport_mwi_wms_message_type_enum_v01 mwi_wms_msg_type
);


//============================================================================
// FUNCTION: qcril_qmi_sms_pre_init
//
// DESCRIPTION:
// pre initialization, to be called at bootup but not after SSR
//
// RETURN: None
//============================================================================
void qcril_qmi_sms_pre_init(void)
{
   wms_service_state = WMS_SERVICE_READY_STATUS_NONE_V01;
   QCRIL_LOG_INFO("wms service ready state: %s",
                                           sms_state_str(wms_service_state) );
}

/*===========================================================================

  FUNCTION:  qcril_qmi_sms_init

===========================================================================*/
/*!
    @brief
    Initialize the SMS subsystem of the RIL.

    @return
    None.
*/
/*=========================================================================*/
qmi_client_error_type qcril_qmi_sms_init
(
  void
)
{
   uint8 i;
   qcril_sms_struct_type *i_ptr;
   qmi_client_error_type qmi_err = QMI_NO_ERR;

  char property_name[ PROPERTY_NAME_MAX ];
  char args[ PROPERTY_VALUE_MAX ];
  int  temp_len;
  unsigned long ret_val;
  char *end_ptr;

  /*-----------------------------------------------------------------------*/
   QCRIL_LOG_FUNC_ENTRY();

   /* Allow cache */
   qcril_sms = (qcril_sms_struct_type *) qcril_arb_allocate_cache( QCRIL_ARB_CACHE_SMS );

  if( qcril_sms == NULL )
  {
    QCRIL_LOG_ERROR("Failed to allocate cache for SMS componet.");
    qmi_err = QMI_INTERNAL_ERR;
  }
  else
  {
     /* Initialize internal data */
     for ( i = 0; i < QCRIL_MAX_INSTANCE_ID; i++ )
     {
       i_ptr = &qcril_sms[ i ];

       i_ptr->sms_ack_info.gw_ack_pending = FALSE;
       i_ptr->sms_ack_info.gw_ack_needed = FALSE;
       i_ptr->sms_ack_info.gw_send_ack_on_ims = FALSE;
       i_ptr->sms_ack_info.cdma_ack_pending = FALSE;
       i_ptr->sms_ack_info.cdma_ack_needed = FALSE;
       i_ptr->sms_ack_info.cdma_send_ack_on_ims = FALSE;
       i_ptr->sms_ack_info.gw_transaction_id = (uint32_t)(-1);
       i_ptr->sms_ack_info.cdma_transaction_id = (uint32_t)(-1);

       i_ptr->mt_pending_ack_expry_tmr     = QMI_RIL_ZERO;
     }

     qmi_ril_set_sms_svc_status( QMI_RIL_SMS_SVC_INIT_PENDING );

     QCRIL_SNPRINTF( property_name, sizeof( property_name ), "%s", QCRIL_MT_SMS_ACK_EXPRY_WINDOW );
     property_get( property_name, args, "" );

     qmi_ril_sms_mt_expiry_window_len_set = FALSE;
     temp_len = strlen( args );
     if ( temp_len > 0 )
     {
       ret_val = strtoul( args, &end_ptr, 0 );
       if ( ( errno == ERANGE ) && ( ret_val == ULONG_MAX ) )
       {
         QCRIL_LOG_ERROR( "Fail to convert MT_SMS_ACK_EXPRY_WINDOW %s", args );
       }
       else
       {
         qmi_ril_sms_mt_expiry_window_len     = ( uint8 ) ret_val;
         qmi_ril_sms_mt_expiry_window_len_set = TRUE;
       }
     }
     QCRIL_LOG_DEBUG( "MT_SMS_ACK_EXPRY_WINDOW set %d to %d", qmi_ril_sms_mt_expiry_window_len_set, qmi_ril_sms_mt_expiry_window_len );

     QCRIL_SNPRINTF( property_name, sizeof( property_name ), "%s", QCRIL_SMS_LINK_TIMER );
     property_get( property_name, args, "" );

     qmi_ril_sms_link_timer = 4;
     temp_len = strlen( args );
     if ( temp_len > 0 )
     {
       ret_val = strtoul( args, &end_ptr, 0 );
       if ( ( errno == ERANGE ) && ( ret_val == ULONG_MAX ) )
       {
         QCRIL_LOG_ERROR( "Fail to convert SMS_LINK_TIMER %s", args );
       }
       else if( ( ret_val > 0 ) && ( ret_val <= 5 ) )
       {
         qmi_ril_sms_link_timer = ( uint8 ) ret_val;
       }
       else
       {
         qmi_ril_sms_link_timer = 5;
       }
     }
     QCRIL_LOG_DEBUG( "SMS_LINK_TIMER val set to %d", qmi_ril_sms_link_timer );

     // set properties
     feature_ims_3gpp_retry = qmi_ril_is_feature_supported(QMI_RIL_FEATURE_IMS_RETRY_3GPP);
     feature_ims_3gpp2_retry = qmi_ril_is_feature_supported(QMI_RIL_FEATURE_IMS_RETRY_3GPP2);

     memset(args,'\0', PROPERTY_VALUE_MAX);
     qcril_db_query_properties_table(QCRIL_ACCEPT_ALL_BC_MSG, args);

     if( !strcmp(args,"all") )
     {
        feature_all_bc_msg_3gpp = 1;
        feature_all_bc_msg_3gpp2 = 1;
     }
     else if( !strcmp(args,"gsm") )
     {
        feature_all_bc_msg_3gpp = 1;
        feature_all_bc_msg_3gpp2 = 0;
     }
     else if( !strcmp(args,"cdma") )
     {
        feature_all_bc_msg_3gpp = 0;
        feature_all_bc_msg_3gpp2 = 1;
     }
     else
     {
        feature_all_bc_msg_3gpp = 0;
        feature_all_bc_msg_3gpp2 = 0;
     }

     QCRIL_LOG_DEBUG( "All Broadcast Msg in 3GPP = %d, 3GPP2 = %d", feature_all_bc_msg_3gpp, feature_all_bc_msg_3gpp2 );

     memset(args,'\0', PROPERTY_VALUE_MAX);
     QCRIL_SNPRINTF( property_name, sizeof( property_name ), "%s", QCRIL_FORCE_ON_DC );
     property_get( property_name, args, "" );

     if( !strcmp(args,"true") )
     {
        is_force_on_dc = 1;
     }
     else
     {
        is_force_on_dc = 0;
     }

     QCRIL_LOG_DEBUG( "IS FORCE ON DC = %d", is_force_on_dc );
  }

  qcril_qmi_sms_reset_mt_sms_ack_needed_power_opt_buffer();
  qcril_qmi_sms_reset_mt_sms_ack_not_needed_power_opt_buffer();

   QCRIL_LOG_FUNC_RETURN_WITH_RET(qmi_err);
   return (qmi_err);
} /* qcril_qmi_sms_init() */

/*===========================================================================

  FUNCTION:  qcril_sms_perform_transport_layer_info_initialization

===========================================================================*/
/*!
    @brief
    Perform the initial configuration of transport layer info status

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_perform_transport_layer_info_initialization()
{
  int temp_len;
  unsigned long ret_val;
  char property_name[ PROPERTY_NAME_MAX ];
  char args[ PROPERTY_VALUE_MAX ];
  char *end_ptr;
  wms_get_transport_layer_resp_msg_v01 wms_get_transport_layer_resp_msg;
  qmi_client_error_type qmi_err;
  RIL_Errno ril_req_res = RIL_E_SUCCESS;
  qcril_sms_struct_type *i_ptr;

  QCRIL_LOG_FUNC_ENTRY();

  i_ptr = &qcril_sms[ QCRIL_DEFAULT_INSTANCE_ID ];

  if( i_ptr )
  {
    i_ptr->transport_layer_info.registered_ind_valid = FALSE;
    i_ptr->transport_layer_info.registered_ind = FALSE;
    i_ptr->transport_layer_info.block_sms_on_1x = FALSE;
    i_ptr->transport_layer_info.sms_format_valid = FALSE;
    i_ptr->transport_layer_info.sms_format = QMI_RIL_SMS_FORMAT_UNKNOWN;
    QCRIL_SNPRINTF( property_name, sizeof( property_name ), "%s", QCRIL_BLOCK_SMS_ON_1X );
    property_get( property_name, args, "" );
    temp_len = strlen( args );
    if ( temp_len > 0 )
    {
      ret_val = strtoul( args, &end_ptr, 0 );
      if ( ( errno == ERANGE ) && ( ret_val == ULONG_MAX ) )
      {
        QCRIL_LOG_ERROR( "Fail to convert BLOCK_SMS_ON_1X %s", args );
      }
      else if ( ret_val > 1 )
      {
        QCRIL_LOG_ERROR( "Invalid saved BLOCK_SMS_ON_1X %ld, use default", ret_val );
      }
      else
      {
        i_ptr->transport_layer_info.block_sms_on_1x = ( uint8 ) ret_val;
      }
    }
    QCRIL_LOG_DEBUG( "BLOCK_SMS_ON_1X=%d", i_ptr->transport_layer_info.block_sms_on_1x);

    memset(&wms_get_transport_layer_resp_msg, 0, sizeof(wms_get_transport_layer_resp_msg));

    qmi_err = qmi_client_sms_send_sync( QMI_WMS_GET_TRANSPORT_LAYER_INFO_REQ_V01,
                                        NULL,
                                        QMI_RIL_ZERO,
                                        (void*) &wms_get_transport_layer_resp_msg,
                                        sizeof( wms_get_transport_layer_resp_msg ) );

    ril_req_res = qcril_qmi_util_convert_qmi_response_codes_to_ril_result( qmi_err, &wms_get_transport_layer_resp_msg.resp );

    if ( RIL_E_SUCCESS == ril_req_res )
    {
      QCRIL_LOG_INFO( "transport layer reg info valid %d, value %d", wms_get_transport_layer_resp_msg.registered_ind_valid, wms_get_transport_layer_resp_msg.registered_ind);
      QCRIL_MUTEX_LOCK( &i_ptr->transport_layer_info_mutex, NULL );
      i_ptr->transport_layer_info.registered_ind_valid = wms_get_transport_layer_resp_msg.registered_ind_valid;
      i_ptr->transport_layer_info.registered_ind = wms_get_transport_layer_resp_msg.registered_ind;
      if (wms_get_transport_layer_resp_msg.transport_layer_info_valid)
      {
        i_ptr->transport_layer_info.sms_format_valid = TRUE;
        i_ptr->transport_layer_info.sms_format =
                (wms_get_transport_layer_resp_msg.transport_layer_info.transport_cap ==
                WMS_TRANSPORT_CAP_CDMA_V01) ?
                QMI_RIL_SMS_FORMAT_3GPP2 : QMI_RIL_SMS_FORMAT_3GPP;
      }
      QCRIL_MUTEX_UNLOCK( &i_ptr->transport_layer_info_mutex, NULL );
    }
    else
    {
      QCRIL_LOG_ERROR( "error while retrieving transport layer reg info");
    }
  }
  else
  {
    QCRIL_LOG_FATAL("sms_cache_ptr is Null pointer");
  }

  QCRIL_LOG_FUNC_RETURN();
} //qcril_sms_perform_transport_layer_info_initialization

/*===========================================================================

  FUNCTION:  qcril_qmi_sms_set_primary_client

===========================================================================*/
/*!
    @brief
    Request to set primary client of WMS

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_sms_set_primary_client( void * param )
{
    qmi_client_error_type qmi_err;
    wms_set_primary_client_req_msg_v01 primary_request_msg;
    wms_set_primary_client_resp_msg_v01 primary_response_msg;
    RIL_Errno ril_req_res = RIL_E_SUCCESS;
    int is_memory_full_tmp = FALSE;
    QCRIL_NOTUSED(param);

    QCRIL_LOG_FUNC_ENTRY();
    // Set QCRIL as the primary client of QMI WMS
    memset(&primary_request_msg, 0, sizeof(wms_set_primary_client_req_msg_v01));
    memset(&primary_response_msg, 0, sizeof(wms_set_primary_client_resp_msg_v01));

    primary_request_msg.primary_client = TRUE;

    qmi_err = qmi_client_sms_send_sync( QMI_WMS_SET_PRIMARY_CLIENT_REQ_V01,
                                        (void*) &primary_request_msg,
                                        sizeof( primary_request_msg ),
                                        (void*) &primary_response_msg,
                                        sizeof( primary_response_msg ) );

    ril_req_res = qcril_qmi_util_convert_qmi_response_codes_to_ril_result( qmi_err, &primary_response_msg.resp );

    if ( RIL_E_SUCCESS != ril_req_res )
    {
        QCRIL_LOG_ERROR( "Failed to set QCRIL as the primary client, err %d", (int)ril_req_res);
    }
    else
    {
        qmi_ril_set_primary_client_status( QMI_RIL_SMS_PRIMARY_CLIENT_SET );
        SMS_CACHE_LOCK;
        is_memory_full_tmp = is_sms_memory_full;
        SMS_CACHE_UNLOCK;

        if (is_memory_full_tmp)
        {
            qcril_sms_send_sms_memory_status(FALSE, NULL);
        }

        QCRIL_LOG_ERROR( "Successfully set QCRIL as the primary client");
    }
}

/*===========================================================================

  FUNCTION:  qcril_sms_perform_initial_configuration

===========================================================================*/
/*!
    @brief
    Perform the initial configuration of QMI WMS

    @return
    None.
*/
/*=========================================================================*/
qmi_client_error_type qcril_sms_perform_initial_configuration
(
  void
)
{
  wms_set_routes_req_msg_v01 routes_request_msg;
  wms_set_routes_resp_msg_v01 routes_response_msg;
  uint8 i;
  qmi_client_error_type qmi_err = QMI_NO_ERR;
  wms_set_event_report_req_msg_v01 event_report_request_msg;
  wms_set_event_report_resp_msg_v01 event_report_response_msg;
  wms_indication_register_req_msg_v01 indication_register_req_msg;
  wms_indication_register_resp_msg_v01 indication_register_resp_msg;
  RIL_Errno ril_req_res = RIL_E_SUCCESS;

  /*-----------------------------------------------------------------------*/
  QCRIL_LOG_FUNC_ENTRY();

  // -----------------------------------------------------------------------

  // Configure the routes
  if (qmi_err == QMI_NO_ERR)
  {
    memset(&routes_request_msg, 0, sizeof(wms_set_routes_req_msg_v01));
    memset(&routes_response_msg, 0, sizeof(wms_set_routes_resp_msg_v01));

    /* Fill in the QMI request to set the routes */
    routes_request_msg.route_list_tuple_len = 6;

    /* Class 2 messages are:
          -- Store and notify
          -- Mem Store is SIM

       All other messages are:
          -- Transfer Only
          -- Mem Store is None
    */
    for (i = 0; i < 6; i++)
    {
      routes_request_msg.route_list_tuple[i].message_type = WMS_MESSAGE_TYPE_POINT_TO_POINT_V01;
      routes_request_msg.route_list_tuple[i].message_class = (wms_message_class_enum_v01) i;
      routes_request_msg.route_list_tuple[i].route_storage = WMS_STORAGE_TYPE_NONE_V01;
      routes_request_msg.route_list_tuple[i].receipt_action = WMS_TRANSFER_ONLY_V01;
    }

    routes_request_msg.route_list_tuple[2].message_type = WMS_MESSAGE_TYPE_POINT_TO_POINT_V01;
    routes_request_msg.route_list_tuple[2].message_class = WMS_MESSAGE_CLASS_2_V01;
    routes_request_msg.route_list_tuple[2].route_storage = WMS_STORAGE_TYPE_UIM_V01;
    routes_request_msg.route_list_tuple[2].receipt_action = WMS_STORE_AND_NOTIFY_V01;

    // All Status Reports should be transferred to the client
    routes_request_msg.transfer_ind_valid = TRUE;
    routes_request_msg.transfer_ind = WMS_TRANSFER_IND_CLIENT_V01;

    qmi_err = qmi_client_sms_send_sync( QMI_WMS_SET_ROUTES_REQ_V01,
                                        (void*) &routes_request_msg,
                                        sizeof( routes_request_msg ),
                                        (void*) &routes_response_msg,
                                        sizeof( routes_response_msg ) );

    ril_req_res = qcril_qmi_util_convert_qmi_response_codes_to_ril_result( qmi_err, &routes_response_msg.resp );

    if ( RIL_E_SUCCESS != ril_req_res )
    {
      QCRIL_LOG_ERROR( "Failed to configure routes, err %d", (int)ril_req_res);
    }
  }

  /*-----------------------------------------------------------------------*/

  // Enable QMI reporting of MT SMS messages
  if (qmi_err == QMI_NO_ERR)
  {
    memset(&event_report_request_msg, 0, sizeof(wms_set_event_report_req_msg_v01));
    memset(&event_report_response_msg, 0, sizeof(wms_set_event_report_resp_msg_v01));

    event_report_request_msg.report_mt_message_valid = TRUE;
    event_report_request_msg.report_mt_message = 0x01;

    event_report_request_msg.report_mwi_message_valid = TRUE;
    event_report_request_msg.report_mwi_message = 0x01;

    event_report_request_msg.report_call_control_info_valid = TRUE;
    event_report_request_msg.report_call_control_info = 0x01;

    qmi_err = qmi_client_sms_send_sync( QMI_WMS_SET_EVENT_REPORT_REQ_V01,
                                        (void*) &event_report_request_msg,
                                        sizeof( event_report_request_msg ),
                                        (void*) &event_report_response_msg,
                                        sizeof( event_report_response_msg ) );

    ril_req_res = qcril_qmi_util_convert_qmi_response_codes_to_ril_result( qmi_err, &event_report_response_msg.resp );

    if ( RIL_E_SUCCESS != ril_req_res )
    {
      QCRIL_LOG_ERROR( "Failed to enable QMI reporting of MT SMS messages, err %d", (int)ril_req_res);
    }
  }

  if (qmi_err == QMI_NO_ERR)
  {
    // Register for the desired indications
    memset(&indication_register_req_msg, 0, sizeof(wms_indication_register_req_msg_v01));
    memset(&indication_register_resp_msg, 0, sizeof(wms_indication_register_resp_msg_v01));

    indication_register_req_msg.reg_transport_layer_info_events_valid = TRUE;
    indication_register_req_msg.reg_transport_layer_info_events = TRUE;
    indication_register_req_msg.reg_transport_nw_reg_info_events_valid = TRUE;
    indication_register_req_msg.reg_transport_nw_reg_info_events = TRUE;
    indication_register_req_msg.reg_service_ready_events_valid = TRUE;
    indication_register_req_msg.reg_service_ready_events = 0x01; // Enable
    indication_register_req_msg.reg_transport_layer_mwi_info_events_valid = TRUE;
    indication_register_req_msg.reg_transport_layer_mwi_info_events = 0x01; // Enable

    qmi_err = qmi_client_sms_send_sync( QMI_WMS_INDICATION_REGISTER_REQ_V01,
                                        (void*) &indication_register_req_msg,
                                        sizeof( indication_register_req_msg ),
                                        (void*) &indication_register_resp_msg,
                                        sizeof( indication_register_resp_msg ) );

    ril_req_res = qcril_qmi_util_convert_qmi_response_codes_to_ril_result( qmi_err, &indication_register_resp_msg.resp );

    if ( RIL_E_SUCCESS != ril_req_res )
    {
      QCRIL_LOG_ERROR( "Failed to register for QMI WMS indications, err %d", (int)ril_req_res);
    }
  }

  if (qmi_ril_is_feature_supported( QMI_RIL_FEATURE_FUSION_CSFB )
   || qmi_ril_is_feature_supported( QMI_RIL_FEATURE_SVLTE2 )
     ) {
     // wms ready indication is not supported for fusion targets yet...
     set_wms_service_state(WMS_SERVICE_READY_STATUS_3GPP_AND_3GPP2_V01);
  }
  else
  {
     // update wms ready status to capture state, assumes ready when not
     // supported
     // Note: we do this after we register for the indication to get the
     // current state
     if (wms_ready_supported()) {
        query_wms_ready_status();
     }
     else {
        // wms ready indication is not supported, assume always ready
        set_wms_service_state(WMS_SERVICE_READY_STATUS_3GPP_AND_3GPP2_V01);
     }
  }

  qcril_sms_perform_transport_layer_info_initialization(); //SMS transport NW reg status info initialization

  qmi_ril_set_sms_svc_status( QMI_RIL_SMS_SVC_FULLY_OPERATIONAL ); // regardless of qmi_err as we will not retry initialization
  QCRIL_LOG_FUNC_RETURN_WITH_RET(qmi_err);

  return(qmi_err);

} // qcril_sms_perform_initial_configuration


/*===========================================================================

  FUNCTION:  qcril_qmi_sms_retrieve_ims_sms_registration_state

===========================================================================*/
/*!
    @brief
    To get IMS SMS registration state

    @return (follow the orginal code logic)
    - If any QMI transport error, return FAILURE
    - If transport_nw_reg_status_valid is 0, return FAILURE
    - Other cases, return SUCCESS
*/
/*=========================================================================*/
RIL_Errno qcril_qmi_sms_retrieve_ims_sms_registration_state
(
  qmi_ril_sms_ims_reg_state& ims_sms_state,
  SmsImsServiceStatusInd::SmsImsServiceStatusInfo& smsImsServiceStatus
)
{
  /* set defautl value */
  ims_sms_state = { FALSE, FALSE, QMI_RIL_SMS_FORMAT_UNKNOWN};
  smsImsServiceStatus.sms_ims_service_status = SmsImsServiceStatusInd::SmsImsServiceStatus::NO_SERVICE;

  wms_get_transport_nw_reg_resp_msg_v01 nw_reg_response_msg;
  memset( &nw_reg_response_msg, 0, sizeof( nw_reg_response_msg ) );

  qmi_client_error_type qmi_transport_error = qmi_client_sms_send_sync(
                                                 QMI_WMS_GET_TRANSPORT_NW_REG_INFO_REQ_V01,
                                                 NULL,
                                                 0,
                                                 &nw_reg_response_msg,
                                                 sizeof(nw_reg_response_msg));
  RIL_Errno res = qcril_qmi_util_convert_qmi_response_codes_to_ril_result(
            qmi_transport_error, &nw_reg_response_msg.resp);
  if (res != RIL_E_SUCCESS)
  {
    QCRIL_LOG_ERROR("Failed to get the response of IMS NW registration state from qmi.");
    return res;
  }

  if ( FALSE == nw_reg_response_msg.transport_nw_reg_status_valid )
  {
    QCRIL_LOG_ERROR("Transport NW Registration status is not valid");
    return RIL_E_MODEM_ERR;
  }

  QCRIL_LOG_INFO("Transport NW Reg Status: %d", nw_reg_response_msg.transport_nw_reg_status);
  if ( WMS_TRANSPORT_NW_REG_STATUS_FULL_SERVICE_V01 == nw_reg_response_msg.transport_nw_reg_status )
  {
      ims_sms_state.is_wms_full_service = TRUE;
      smsImsServiceStatus.sms_ims_service_status = SmsImsServiceStatusInd::SmsImsServiceStatus::FULL_SERVICE;

      wms_get_transport_layer_resp_msg_v01 response_msg;
      memset( &response_msg, 0, sizeof( response_msg ) );

      qmi_transport_error = qmi_client_sms_send_sync(QMI_WMS_GET_TRANSPORT_LAYER_INFO_REQ_V01,
                                                     NULL,
                                                     0,
                                                     &response_msg,
                                                     sizeof(response_msg));
      res = qcril_qmi_util_convert_qmi_response_codes_to_ril_result(qmi_transport_error, (qmi_response_type_v01*) &response_msg);
      if ( res != RIL_E_SUCCESS )
      {
          QCRIL_LOG_ERROR("Failed to get the response of IMS registration state from qmi.");
          return res;
      }

      QCRIL_LOG_INFO("QMI_WMS_GET_TRANSPORT_LAYER_INFO_RESP received: SUCCESS");

      if ( response_msg.registered_ind_valid &&
           response_msg.registered_ind &&
           response_msg.transport_layer_info_valid &&
           (response_msg.transport_layer_info.transport_type == WMS_TRANSPORT_TYPE_IMS_V01)
          )
      {
          ims_sms_state.is_ims_wms_reg = TRUE;
          if (response_msg.transport_layer_info.transport_cap == WMS_TRANSPORT_CAP_GW_V01)
          {
              ims_sms_state.format = QMI_RIL_SMS_FORMAT_3GPP;
          }
          else if (response_msg.transport_layer_info.transport_cap == WMS_TRANSPORT_CAP_CDMA_V01)
          {
              ims_sms_state.format = QMI_RIL_SMS_FORMAT_3GPP2;
          }
          QCRIL_LOG_INFO("Transport Capability: %d", response_msg.transport_layer_info.transport_cap);
      }
      else
      {
        smsImsServiceStatus.sms_ims_service_status = SmsImsServiceStatusInd::SmsImsServiceStatus::LIMITED_SERVICE;
      }
  }
  else if(WMS_TRANSPORT_NW_REG_STATUS_LIMITED_SERVICE_V01 == nw_reg_response_msg.transport_nw_reg_status)
  {
    smsImsServiceStatus.sms_ims_service_status = SmsImsServiceStatusInd::SmsImsServiceStatus::LIMITED_SERVICE;
  }
  else
  {
    smsImsServiceStatus.sms_ims_service_status = SmsImsServiceStatusInd::SmsImsServiceStatus::NO_SERVICE;
  }

  return RIL_E_SUCCESS;
}

/*===========================================================================

  FUNCTION:  qcril_qmi_sms_get_ims_sms_format

===========================================================================*/
/*!
    @brief
    An exported function to get current IMS SMS format
    If the current format is UNKNOWN, this function will not wait for another
    query. Instead, it will return the current value and trigger another query,
    which will store the format in cache later.

    @return
    Current IMS SMS format
*/
/*=========================================================================*/
qmi_ril_sms_format_type qcril_qmi_sms_get_ims_sms_format()
{
    /* if the format is unknown, trigger the re-query, but do not wait for it */
    qcril_sms_struct_type* i_ptr = &qcril_sms[ QCRIL_DEFAULT_INSTANCE_ID ];
    qmi_ril_sms_format_type sms_format;
    QCRIL_MUTEX_LOCK( &i_ptr->transport_layer_info_mutex, NULL );
    if (i_ptr->transport_layer_info.sms_format_valid &&
            i_ptr->transport_layer_info.sms_format != QMI_RIL_SMS_FORMAT_UNKNOWN)
    {
        sms_format = i_ptr->transport_layer_info.sms_format;
        QCRIL_MUTEX_UNLOCK( &i_ptr->transport_layer_info_mutex, NULL );
        return sms_format;
    }
    QCRIL_MUTEX_UNLOCK( &i_ptr->transport_layer_info_mutex, NULL );
    /* If IMS is not registered, get the format using QMI_WMS_GET_TRANSPORT_LAYER_INFO_REQ*/
    wms_get_transport_layer_resp_msg_v01 response_msg;
    memset( &response_msg, 0, sizeof( response_msg ) );

    qmi_client_error_type qmi_transport_error = qmi_client_sms_send_sync(
                                                   QMI_WMS_GET_TRANSPORT_LAYER_INFO_REQ_V01,
                                                   NULL,
                                                   0,
                                                   &response_msg,
                                                   sizeof(response_msg));
     RIL_Errno res = qcril_qmi_util_convert_qmi_response_codes_to_ril_result(qmi_transport_error,
              (qmi_response_type_v01*) &response_msg);
    if ( res != RIL_E_SUCCESS )
    {
        QCRIL_LOG_ERROR("Failed to get the response of IMS registration state from qmi.");
        return QMI_RIL_SMS_FORMAT_UNKNOWN;
    }

    QCRIL_LOG_INFO("QMI_WMS_GET_TRANSPORT_LAYER_INFO_RESP received: SUCCESS");

    if ( response_msg.transport_layer_info_valid )
    {
        if (response_msg.transport_layer_info.transport_cap == WMS_TRANSPORT_CAP_GW_V01)
        {
            sms_format = QMI_RIL_SMS_FORMAT_3GPP;
        }
        else if (response_msg.transport_layer_info.transport_cap == WMS_TRANSPORT_CAP_CDMA_V01)
        {
            sms_format = QMI_RIL_SMS_FORMAT_3GPP2;
        }
        else
        {
            QCRIL_LOG_ERROR("Unknown format format from transport_cap");
            sms_format = QMI_RIL_SMS_FORMAT_UNKNOWN;
        }
        /* Cache the format so that we will not make sync call evertytime to Modem
         * and this cache can also be updated with QMI_WMS_TRANSPORT_LAYER_INFO_IND*/
        qcril_sms_struct_type* i_ptr = &qcril_sms[ QCRIL_DEFAULT_INSTANCE_ID ];
        QCRIL_MUTEX_LOCK( &i_ptr->transport_layer_info_mutex, NULL );
        if (sms_format != QMI_RIL_SMS_FORMAT_UNKNOWN)
        {
            i_ptr->transport_layer_info.sms_format_valid = TRUE;
            i_ptr->transport_layer_info.sms_format = sms_format;
        }
        QCRIL_MUTEX_UNLOCK( &i_ptr->transport_layer_info_mutex, NULL );
        return sms_format;
    }
    else
    {
        QCRIL_LOG_ERROR("transport_layer_info_valid is not valid ");
        return QMI_RIL_SMS_FORMAT_UNKNOWN;
    }
}

/*===========================================================================

                                RIL REQUEST HANDLERS

===========================================================================*/

/*===========================================================================

  FUNCTION:  qcril_mo_sms_error_check

===========================================================================*/
/*!
    @brief
    Does error checking on mobile-originated SMS.  This checking should be
    done before sending an SMS, or writing an SMS to SIM.

    @return
    TRUE if SMS passes error-checking; FALSE otherwise.
*/
/*=========================================================================*/
boolean qcril_mo_sms_error_check
(
  const char *pdu
)
{
  boolean err_check_passed = TRUE;

  /*-----------------------------------------------------------------------*/
  QCRIL_LOG_FUNC_ENTRY();
  /* Check if the PDU is null */
  if ( pdu == NULL )
  {
    QCRIL_LOG_ERROR("PDU in SMS is NULL!");
    err_check_passed = FALSE;
  }

  /* Check if the length exceeds the maximum */
  else if ( ( strlen( pdu ) / 2 ) > WMS_MESSAGE_LENGTH_MAX_V01 )
  {
    QCRIL_LOG_ERROR("PDU in SMS exceeds maximum allowable length!");
    err_check_passed = FALSE;
  }

  QCRIL_LOG_FUNC_RETURN_WITH_RET(err_check_passed);
  return (err_check_passed);

} /* qcril_mo_sms_error_check */


/*===========================================================================

  FUNCTION:  qcril_sms_fill_wms_payload

===========================================================================*/
/*!
    @brief
    Fill SMS payload per QMI requirement.

    @return
    TRUE if successful; FALSE otherwise
*/
/*=========================================================================*/
boolean qcril_sms_fill_wms_payload
(
  boolean payload_in_cdma_format,
  RIL_CDMA_SMS_Message *cdma_sms_msg,
  const char *gw_smsc_address,
  const char *gw_pdu,
  size_t gw_pdu_len, /* only valid when is_gw_pdu_hex_string is FALSE */
  boolean is_gw_pdu_hex_string,
  wms_message_format_enum_v01  * format,
  uint32 * raw_message_len,
  uint8  * raw_message,
  boolean sms_on_ims,
  boolean is_mo_sms
)
{
  uint8 byte_array_pos = 0;
  boolean success = TRUE;

  QCRIL_LOG_INFO( "payload in cdma format valid %d", payload_in_cdma_format);
  /* 3GPP2 format */
  if ( payload_in_cdma_format )
  {
    if(cdma_sms_msg != NULL)
    {
       /* Convert the message to OTA format */
       if (  qcril_sms_convert_sms_ril_to_qmi(cdma_sms_msg,
                                           raw_message,
                                           WMS_MESSAGE_LENGTH_MAX_V01,
                                           (uint16 *) raw_message_len,
                                           sms_on_ims,
                                           is_mo_sms) != TRUE)
       {
          QCRIL_LOG_ERROR( "Conversion from RIL format to QMI format failed" );
          success = FALSE;
       }
       else
       {
          *format = WMS_MESSAGE_FORMAT_CDMA_V01;
       }
    }
    else
    {
      QCRIL_LOG_ERROR("cdma_sms_msg is NULL!");
      success = FALSE;
    }
  }
  /* 3GPP format */
  else
  {
    if(gw_pdu != NULL)
    {
      *format = WMS_MESSAGE_FORMAT_GW_PP_V01;

       if ( gw_smsc_address == NULL )
       {
          /* There is no SMSC address.  The first byte of the raw data is the SMSC length,
           which is 0. */
          raw_message[0] = 0;

          byte_array_pos = 1;
       }
       else
       {
          /* Convert the SMSC address from ASCII Hex to byte*/
          qcril_sms_hex_to_byte( gw_smsc_address,
                             &raw_message[0],
                             strlen(gw_smsc_address) );

          /* The input is in ASCII hex format, and it is being packed into a byte array.
           Two ASCII characters fit in each byte, so divide the length by two */
          byte_array_pos = strlen(gw_smsc_address) / 2;
       }

      if (is_gw_pdu_hex_string)
      {
        /* Translate the PDU from ASCII hex to bytes */
        qcril_sms_hex_to_byte( gw_pdu,
                             &raw_message[byte_array_pos],
                             strlen( gw_pdu ) );

        *raw_message_len = byte_array_pos + strlen(gw_pdu) / 2;
      }
      else
      {
        memcpy(raw_message + byte_array_pos, gw_pdu, gw_pdu_len);
        *raw_message_len = byte_array_pos + gw_pdu_len;
      }
    }
    else
    {
      QCRIL_LOG_ERROR("gw_pdu is NULL!");
      success = FALSE;
    }
  }

  QCRIL_LOG_FUNC_RETURN_WITH_RET(success);
  return (success);

} /* qcril_sms_fill_wms_payload */


/*===========================================================================

  FUNCTION:  qcril_sms_request_send_gw_sms

===========================================================================*/
/*!
    @brief
    Sends a GW SMS message
    Handle RIL_REQUEST_SEND_SMS and RIL_REQUEST_SEND_SMS_EXPECT_MORE

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_send_gw_sms
(
  std::shared_ptr<RilRequestSendSmsMessage> msg
)
{
  uint32 user_data;
  const char *smsc_address;
  const char *gw_pdu;
  wms_raw_send_req_msg_v01 request_msg;
  nas_sms_status_enum_type_v01 lte_sms_status;
  uint8_t lte_sms_status_valid;
  uint8_t is_voice_over_lte = FALSE;
  bool inserted_to_pendinglist = false;
  RIL_Errno ril_err = RIL_E_SUCCESS;

  QCRIL_LOG_FUNC_ENTRY();

  do
  {
    // service status check
    qcril_qmi_nas_fetch_lte_sms_status( &lte_sms_status_valid, &lte_sms_status);
    is_voice_over_lte = qcril_qmi_nas_is_voice_over_lte_registered();
    if (!wms_ready_atleast_limited_3gpp() &&
        !(is_voice_over_lte && lte_sms_status_valid && (NAS_SMS_STATUS_3GPP_V01 == lte_sms_status))
       ) {
       ril_err = RIL_E_INVALID_STATE;
       break;
    }

    // sanity check
    if ( !qcril_mo_sms_error_check(msg->getPdu().c_str()) )
    {
      QCRIL_LOG_ERROR("qcril_mo_sms_error_check failed for MO GW SMS message.");
      ril_err = RIL_E_INVALID_ARGUMENTS;
      break;
    }

    // Add Message to the pending list, and get the token in case of success
    auto res = getSmsModule().getPendingMessageList().insert(msg);
    if (!res.second)
    {
      ril_err = RIL_E_INTERNAL_ERR;
      break;
    }
    inserted_to_pendinglist = true;
    uint16_t token = res.first;

    /* Fill in the message */
    smsc_address = msg->getSmscPdu().empty() ? nullptr : msg->getSmscPdu().c_str();
    gw_pdu = msg->getPdu().empty() ? nullptr : msg->getPdu().c_str();

    /* Fill in the QMI request */
    memset(&request_msg, 0, sizeof(wms_raw_send_req_msg_v01));

    if (!qcril_sms_fill_wms_payload(
                                   FALSE,
                                   NULL,
                                   smsc_address,
                                   gw_pdu,
                                   0,
                                   TRUE,
                                   &request_msg.raw_message_data.format,
                                   (uint32 *)&request_msg.raw_message_data.raw_message_len,
                                   &request_msg.raw_message_data.raw_message[0],
                                   FALSE,
                                   TRUE ))
    {
      QCRIL_LOG_ERROR("Filling SMS payload per QMI requirement is failed for GW SMS message.");
      ril_err = RIL_E_INVALID_SMS_FORMAT;
      break;
    }

    if (msg->getLinkCtlEnable())
    {
      request_msg.link_timer_valid = TRUE;
      request_msg.link_timer = qmi_ril_sms_link_timer;
    }

    request_msg.sms_on_ims_valid = TRUE;
    request_msg.sms_on_ims = FALSE;

    user_data = QCRIL_COMPOSE_USER_DATA( QCRIL_DEFAULT_INSTANCE_ID, QCRIL_DEFAULT_MODEM_ID,
                                         token );

    if (qmi_client_sms_send_async(QMI_WMS_RAW_SEND_REQ_V01,
                                  &request_msg,
                                  sizeof(wms_raw_send_req_msg_v01),
                                  sizeof(wms_raw_send_resp_msg_v01),
                                  qcril_qmi_sms_command_cb,
                                  (void *)(uintptr_t) user_data) != QMI_NO_ERR)
    {
      QCRIL_LOG_ERROR("Failed in getting the response message from qmi.");
      ril_err = RIL_E_SYSTEM_ERR;
      break;
    }

  } while(0);

  if(ril_err != RIL_E_SUCCESS)
  {
    if(inserted_to_pendinglist) {
        getSmsModule().getPendingMessageList().erase(msg);
    }
    auto resp = std::make_shared<RilSendSmsResult_t>(ril_err);
    msg->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
  }

  QCRIL_LOG_FUNC_RETURN();
} /* qcril_sms_request_send_gw_sms */

#if 0
void qcril_sms_convert_to_ims_errors
(
  const RIL_Errno ril_err,
  const wms_raw_send_resp_msg_v01* resp,
  const RIL_RadioTechnologyFamily tech,
  ImsSmsSendStatusResult& sms_status_report,
  ImsSmsSendFailureReason& failure_reason
)
{
  QCRIL_NOTUSED(resp);

  if (ril_err == RIL_E_SUCCESS) {
    sms_status_report = ImsSmsSendStatusResult::SEND_STATUS_OK;
    failure_reason = ImsSmsSendFailureReason::RESULT_ERROR_NONE;
  } else if (ril_err == RIL_E_SMS_SEND_FAIL_RETRY) {
    // if tech is 3GPP and device is registered in LTE only, retry on IMS pipe again
    if (TRUE == qcril_qmi_sms_block_retry_3pgg_sms_on_lte_only(tech, TRUE))
    {
      sms_status_report = ImsSmsSendStatusResult::SEND_STATUS_ERROR;
      failure_reason = ImsSmsSendFailureReason::RESULT_ERROR_GENERIC_FAILURE;
    }
    else
    {
      sms_status_report = ImsSmsSendStatusResult::SEND_STATUS_ERROR_FALLBACK;
      failure_reason = ImsSmsSendFailureReason::RESULT_ERROR_GENERIC_FAILURE;
    }
  } else {
    sms_status_report = ImsSmsSendStatusResult::SEND_STATUS_ERROR;
    failure_reason = ImsSmsSendFailureReason::RESULT_ERROR_GENERIC_FAILURE;
  }
}
#endif


/*===========================================================================

  FUNCTION:  qcril_mo_sms_cb

===========================================================================*/
/*!
    @brief
    Handles response to RIL_REQUEST_SEND_SMS, RIL_REQUEST_SEND_SMS_EXPECT_MORE,
    RIL_REQUEST_CDMA_SEND_SMS, and RIL_REQUEST_IMS_SEND_SMS.

    @return
    None.
*/
/*=========================================================================*/
void qcril_mo_sms_cb
(
  std::shared_ptr<Message> msg,
  qmi_client_error_type transp_err,
  wms_raw_send_resp_msg_v01* raw_send_resp
)
{
  qcril_instance_id_e_type instance_id;
  boolean retry_status = FALSE;
  uint8 coding_scheme;
  wms_call_control_modified_info_type_v01 wms_call_control_alpha_info;
  char buf_str[WMS_ALPHA_ID_LENGTH_MAX_V01 * 2 + 2];
  uint8_t is_device_registered_on_cs = FALSE;

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_FUNC_ENTRY();
  instance_id = QCRIL_DEFAULT_INSTANCE_ID;

  /*-----------------------------------------------------------------------*/

  // 1) generate message_ref, error code and ril error
  int message_ref = -1;
  int error_code = -1;
  RIL_Errno ril_err = RIL_E_GENERIC_FAILURE;

  if (transp_err != QMI_NO_ERR)
  {
      QCRIL_LOG_INFO("Transp error (0x%x) recieved from QMI", transp_err);
  }
  else
  {
    message_ref = raw_send_resp->message_id;
    if (raw_send_resp->resp.result == QMI_RESULT_SUCCESS_V01)
    {
      QCRIL_LOG_INFO("QMI_WMS_RAW_SEND_RESP received: SUCCESS");
      ril_err = RIL_E_SUCCESS;
    }
    else if (raw_send_resp->resp.result == QMI_RESULT_FAILURE_V01)
    {
      QCRIL_LOG_INFO("QMI_WMS_RAW_SEND_RESP received with error %s",
                      qmisvc_errstr(raw_send_resp->resp.error) );

      /* Fill in the error codes*/
      if (raw_send_resp->resp.error == QMI_ERR_CAUSE_CODE_V01)
      {
        if (raw_send_resp->gw_cause_info_valid)
        {
          error_code = raw_send_resp->gw_cause_info.rp_cause;
          QCRIL_LOG_INFO("QMI_WMS_RAW_SEND_RESP cause code %s",
                          gw_cause_code_str(error_code) );
        }
        else if (raw_send_resp->cause_code_valid)
        {
          error_code = raw_send_resp->cause_code;
          QCRIL_LOG_INFO("QMI_WMS_RAW_SEND_RESP cause code %s",
                        cause_code_str(error_code) );
        }
        else
        {
          QCRIL_LOG_INFO("QMI_WMS_RAW_SEND_RESP missing cause code!");
        }
      }

      /* Determine whether the message can be retried or not */
      if (raw_send_resp->resp.error == QMI_ERR_CAUSE_CODE_V01)
      {
        if ((raw_send_resp->gw_cause_info_valid) &&
            (raw_send_resp->gw_cause_info.rp_cause == 41))
        {
          retry_status = TRUE;
        }
        else if ((raw_send_resp->error_class_valid) &&
                 (raw_send_resp->error_class == 0))
        {
          retry_status = TRUE;
        }
        else if ((raw_send_resp->gw_cause_info_valid) &&
            (raw_send_resp->gw_cause_info.tp_cause == WMS_TP_CAUSE_SM_REJECTED_OR_DUPLICATE_V01))
        {
          /* This case will happen when first MO SMS has been delivered to network,
          but WMS didn't got response from network during certain amount of time,
          then WMS will send Temporary Failure, but in actual Network is still trying
          to deliver and it got delivered. During RETRY, if RIL gets tp_cause= -59
          that means SMS Rejected-Duplicate SMS that means same SMS has
          already been received by network. So, RIL should send SUCCESS to Telephony.*/

          ril_err = RIL_E_SUCCESS;
        }
      }
      else if (raw_send_resp->resp.error == QMI_ERR_MESSAGE_DELIVERY_FAILURE_V01)
      {
        is_device_registered_on_cs = qcril_qmi_nas_is_device_registered_for_cs_domain();

        if (raw_send_resp->message_delivery_failure_type_valid &&
            (raw_send_resp->message_delivery_failure_type == WMS_MESSAGE_DELIVERY_FAILURE_TEMPORARY_V01))
        {
          retry_status = TRUE;
        }
        else if (raw_send_resp->sms_ims_rej_cause_info_valid && is_device_registered_on_cs)
        {
          retry_status = TRUE;
        }
      }
      else if ((raw_send_resp->resp.error == QMI_ERR_DEVICE_NOT_READY_V01) ||
               (raw_send_resp->resp.error == QMI_ERR_NETWORK_NOT_READY_V01) ||
               (raw_send_resp->resp.error == QMI_ERR_NO_MEMORY_V01) ||
               (raw_send_resp->resp.error == QMI_ERR_INVALID_OPERATION_V01))
      {
        retry_status = TRUE;
      }

      else if((raw_send_resp->resp.error == QMI_ERR_INVALID_OPERATION_V01) &&
            msg->get_message_id() == RilRequestImsSendSmsMessage::get_class_message_id())
      {
        /* There might be a chance of user sending sms on ims while ims is
           de-registered for WMS. In which case WMS sends INVALID_OPERATION.
           In these scenarios the sms should be re-tried on CS */
        retry_status = TRUE;
      }

      if (retry_status == TRUE)
      {
        ril_err = RIL_E_SMS_SEND_FAIL_RETRY;

        if (raw_send_resp->resp.error == QMI_ERR_INVALID_OPERATION_V01) {
          QCRIL_LOG_INFO("Reset MsgRef for Errorcode invalid operation");
          message_ref = -1;
        }
      }
      else if (raw_send_resp->resp.error == QMI_ERR_MSG_BLOCKED_V01)
      {
        ril_err = RIL_E_FDN_CHECK_FAILURE;
      }
      else if(ril_err != RIL_E_SUCCESS && ril_err != RIL_E_SMS_SEND_FAIL_RETRY && ril_err != RIL_E_FDN_CHECK_FAILURE)
      {
        switch(raw_send_resp->resp.error)
        {
          case QMI_ERR_INVALID_MESSAGE_ID_V01:
              ril_err = RIL_E_INVALID_ARGUMENTS;
              break;
          case QMI_ERR_MESSAGE_NOT_SENT_V01:
              ril_err = RIL_E_MODEM_ERR;
              break;
          case QMI_ERR_CALL_FAILED_V01:
              ril_err = RIL_E_NETWORK_ERR;
              break;
          default:
             ril_err = qcril_qmi_util_convert_qmi_response_codes_to_ril_result(QMI_NO_ERR,
                                                                   &(raw_send_resp->resp));
             break;
        }
      }
    } // raw_send_resp->resp.result == QMI_RESULT_FAILURE_V01
  } // transp_err == QMI_NO_ERR

  // 2) Send the reponse
  if (msg->get_message_id() == RilRequestSendSmsMessage::get_class_message_id())
  {
    if (ril_err == RIL_E_SUCCESS)
        mo_sms_on_ims_map[raw_send_resp->message_id] = false;
    auto resp = std::make_shared<RilSendSmsResult_t>(
            ril_err, message_ref, error_code);
    auto ril_msg = std::static_pointer_cast<RilRequestSendSmsMessage>(msg);
    ril_msg->sendResponse(ril_msg, Message::Callback::Status::SUCCESS, resp);
  }
  else if (msg->get_message_id() == RilRequestCdmaSendSmsMessage::get_class_message_id())
  {
    if (ril_err == RIL_E_SUCCESS)
        mo_sms_on_ims_map[raw_send_resp->message_id] = false;
    auto resp = std::make_shared<RilSendSmsResult_t>(
            ril_err, message_ref, error_code);
    auto ril_msg = std::static_pointer_cast<RilRequestCdmaSendSmsMessage>(msg);
    ril_msg->sendResponse(ril_msg, Message::Callback::Status::SUCCESS, resp);

  }
  else if (msg->get_message_id() == RilRequestImsSendSmsMessage::get_class_message_id())
  {
    if (ril_err == RIL_E_SUCCESS)
        mo_sms_on_ims_map[raw_send_resp->message_id] = true;
    auto resp = std::make_shared<RilSendSmsResult_t>(
            ril_err, message_ref, error_code, !!qcril_qmi_nas_is_lte_only_registered());
    auto ril_msg = std::static_pointer_cast<RilRequestImsSendSmsMessage>(msg);
    ril_msg->sendResponse(ril_msg, Message::Callback::Status::SUCCESS, resp);
  }

  if (transp_err == QMI_NO_ERR &&
        ( TRUE == raw_send_resp->call_control_modified_info_valid ) &&
        ( raw_send_resp->call_control_modified_info.alpha_id_len > 0 ) )
  {
    memset(buf_str, 0 , sizeof(buf_str));
    coding_scheme = raw_send_resp->call_control_modified_info.alpha_id[0];
    if( coding_scheme == 0x80 || coding_scheme == 0x81 || coding_scheme == 0x82 ) //UCS2
    {
        QCRIL_LOG_INFO("Coding scheme is %x, ucs2 data",coding_scheme);
        qcril_qmi_sms_transfer_sim_ucs2_alpha_to_std_ucs2_alpha ( &raw_send_resp->call_control_modified_info, &wms_call_control_alpha_info);
        qcril_cm_ss_convert_ussd_string_to_utf8(  QCRIL_QMI_VOICE_USSD_DCS_UCS2,
                                                    wms_call_control_alpha_info.alpha_id,
                                                    wms_call_control_alpha_info.alpha_id_len,
                                                    buf_str,
                                                    sizeof(buf_str) );
    }
    else //gsm8
    {
        QCRIL_LOG_INFO("Coding scheme is %x, gsm8 data",coding_scheme);
        if( raw_send_resp->call_control_modified_info.alpha_id_len < WMS_ALPHA_ID_LENGTH_MAX_V01 )
        {
            qcril_cm_ss_convert_gsm8bit_alpha_string_to_utf8( (char*) raw_send_resp->call_control_modified_info.alpha_id,
                                                                raw_send_resp->call_control_modified_info.alpha_id_len,
                                                                buf_str,
                                                                sizeof(buf_str) );
        }
    }

    if ( *buf_str )
    {
        auto msg = std::make_shared<RilUnsolStkCCAlphaNotifyMessage>(
                std::string(static_cast<const char*>(buf_str)));
        Dispatcher::getInstance().dispatch(msg);
    }
  }

  QCRIL_LOG_FUNC_RETURN();
} /* qcril_mo_sms_cb */

void qcril_qmi_sms_transfer_sim_ucs2_alpha_to_std_ucs2_alpha(const wms_call_control_modified_info_type_v01 *sim_alpha, wms_call_control_modified_info_type_v01 *std_alpha)
{
  uint8 idx;
  uint8 coding_scheme;
  uint8 num_of_char;
  uint16 base_val;
  uint16 ucs2_val;

  QCRIL_LOG_FUNC_ENTRY();

  memset(std_alpha, 0, sizeof(*std_alpha));

  do
  {
    if ( sim_alpha->alpha_id_len > 0)
    {
      coding_scheme = sim_alpha->alpha_id[0];
    }
    else
    {
      QCRIL_LOG_ERROR("alpha_id_len is 0");
      break;
    }
    QCRIL_LOG_INFO("coding scheme %x", (int)coding_scheme);

    idx = 1;
    switch (coding_scheme)
    {
      case 0x80:
        while ( (uint32)(idx+1) < sim_alpha->alpha_id_len )
        {
          std_alpha->alpha_id[idx-1] = sim_alpha->alpha_id[idx+1];
          std_alpha->alpha_id[idx] = sim_alpha->alpha_id[idx];
          idx += 2;
        }

        if ( (uint32)(idx+1) == sim_alpha->alpha_id_len && 0 != sim_alpha->alpha_id[idx] )
        {
          QCRIL_LOG_ERROR("an unexpected extra non-zero byte in source alpha buffer");
        }

        std_alpha->alpha_id_len = idx-1;
        break;

      case 0x81:
        if ( sim_alpha->alpha_id_len < 3 )
        {
          QCRIL_LOG_ERROR("sim_alpha->alpha_id_len (%d) less than 3", sim_alpha->alpha_id_len);
        }
        else
        {
          num_of_char = sim_alpha->alpha_id[1];
          base_val = sim_alpha->alpha_id[2];
          base_val <<= 7;
          idx = 3;

          if ( idx + num_of_char > sim_alpha->alpha_id_len )
          {
            QCRIL_LOG_DEBUG("num_of_char > sim_alpha->alpha_id_len - 3");
            num_of_char = sim_alpha->alpha_id_len - idx;
          }

          if (num_of_char * 2 > WMS_ALPHA_ID_LENGTH_MAX_V01)
          {
            QCRIL_LOG_DEBUG("num_of_char * 2 > WMS_ALPHA_ID_LENGTH_MAX_V01");
            num_of_char = WMS_ALPHA_ID_LENGTH_MAX_V01 / 2;
          }

          int i;
          for ( i = 0; i< num_of_char; i++, idx++ )
          {
            ucs2_val = sim_alpha->alpha_id[idx];

            if ( ucs2_val >= 0x80 )
            {
              ucs2_val &= 0x7F;
              ucs2_val |= base_val;
            }

            std_alpha->alpha_id[2*i]   = (uint8) (ucs2_val);
            std_alpha->alpha_id[2*i+1] = (uint8) (ucs2_val >> 8);
          }

          std_alpha->alpha_id_len = num_of_char * 2;
        }

        break;

      case 0x82:
        if ( sim_alpha->alpha_id_len < 4 )
        {
          QCRIL_LOG_DEBUG("sim_alpha->alpha_id_len (%d) less than 4", sim_alpha->alpha_id_len);
        }
        else
        {
          num_of_char = sim_alpha->alpha_id[1];
          base_val = sim_alpha->alpha_id[2];
          base_val <<= 8;
          base_val += sim_alpha->alpha_id[3];
          idx = 4;

          if ( idx + num_of_char > sim_alpha->alpha_id_len )
          {
            QCRIL_LOG_DEBUG("num_of_char > sim_alpha->alpha_id_len - 4");
            num_of_char = sim_alpha->alpha_id_len - idx;
          }

          if (num_of_char * 2 > WMS_ALPHA_ID_LENGTH_MAX_V01)
          {
            QCRIL_LOG_DEBUG("num_of_char * 2 > WMS_ALPHA_ID_LENGTH_MAX_V01");
            num_of_char = WMS_ALPHA_ID_LENGTH_MAX_V01 / 2;
          }

          int i;
          for ( i = 0; i< num_of_char; i++, idx++ )
          {
            ucs2_val = sim_alpha->alpha_id[idx];

            if ( ucs2_val >= 0x80 )
            {
              ucs2_val &= 0x7F;
              ucs2_val += base_val;
            }

            std_alpha->alpha_id[2*i]   = (uint8) (ucs2_val);
            std_alpha->alpha_id[2*i+1] = (uint8) (ucs2_val >> 8);
          }

          std_alpha->alpha_id_len = num_of_char * 2;
        }

        break;

      default:
        QCRIL_LOG_ERROR("unknown SIM coding scheme");
    }

  } while ( FALSE );

  QCRIL_LOG_FUNC_RETURN();
}

/*===========================================================================

  FUNCTION:  qcril_sms_request_sms_acknowledge

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_SMS_ACKNOWLEDGE.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_sms_acknowledge
(
  std::shared_ptr<RilRequestAckGsmSmsMessage> msg
)
{
  qcril_instance_id_e_type instance_id;
  qcril_sms_struct_type *i_ptr;
  int tp_cause;
  uint32 user_data;
  char details[ 80 ];
  bool inserted_to_pendinglist = false;
  wms_send_ack_req_msg_v01 request_msg;
  RIL_Errno res = RIL_E_SUCCESS;

  int need_reset_ack_expry_tmr = FALSE;
  int is_wms_limited_status = FALSE;

  /*-----------------------------------------------------------------------*/


  QCRIL_LOG_FUNC_ENTRY();
  instance_id = QCRIL_DEFAULT_INSTANCE_ID;

  i_ptr = &qcril_sms[ instance_id ];

  /*-----------------------------------------------------------------------*/
  QCRIL_SNPRINTF( details, sizeof( details ), "qcril_sms[%d].sms_ack_info_mutex", instance_id );
  QCRIL_MUTEX_LOCK( &i_ptr->sms_ack_info_mutex, details );

  do
  {
    if (!i_ptr->sms_ack_info.gw_ack_pending)
    {
      QCRIL_LOG_ERROR("Unable to send ack; no messages to be acked.");
      res = RIL_E_INVALID_STATE;
      break;
    }

    if (!i_ptr->sms_ack_info.gw_ack_needed)
    {
      QCRIL_LOG_INFO("Dropping ack since no ack is needed.");

      i_ptr->sms_ack_info.gw_ack_pending = FALSE;
      i_ptr->sms_ack_info.gw_send_ack_on_ims = FALSE;

      need_reset_ack_expry_tmr = TRUE;

      msg->sendResponse(msg, Message::Callback::Status::SUCCESS,
            std::make_shared<RIL_Errno>(RIL_E_SUCCESS));
      break;
    }

    is_wms_limited_status = get_wms_limited_service_status();

    if (is_wms_limited_status && !wms_ready_atleast_limited_3gpp() && !i_ptr->sms_ack_info.gw_send_ack_on_ims)
    {
       /* Reset the ack pending flag as Telephony is not going to send another ACK request,
         so that RIL can process further MT SMS*/
       i_ptr->sms_ack_info.gw_ack_pending = FALSE;
       i_ptr->sms_ack_info.gw_ack_needed = FALSE;
       i_ptr->sms_ack_info.gw_send_ack_on_ims = FALSE;
       res = RIL_E_INVALID_STATE;
       break;
    }

    // Add Message to the pending list, and get the token in case of success
    auto ret = getSmsModule().getPendingMessageList().insert(msg);
    if (!ret.second)
    {
        res = RIL_E_INTERNAL_ERR;
        QCRIL_LOG_ERROR("Failed to add an entry for SMS acknowledgement.");
        /* Reset the ack pending flag as Telephony is not going to send another ACK request,
           so that RIL can process further MT SMS*/
        i_ptr->sms_ack_info.gw_ack_pending = FALSE;
        i_ptr->sms_ack_info.gw_ack_needed = FALSE;
        i_ptr->sms_ack_info.gw_send_ack_on_ims = FALSE;
        break;
    }
    inserted_to_pendinglist = true;
    uint16_t token = ret.first;

    /* Fill in the QMI request */
    memset(&request_msg, 0, sizeof(wms_send_ack_req_msg_v01));
    request_msg.ack_information.transaction_id = i_ptr->sms_ack_info.gw_transaction_id;
    request_msg.ack_information.message_protocol = WMS_MESSAGE_PROTOCOL_WCDMA_V01;

    request_msg.ack_information.success = msg->isSuccess();

    if (!request_msg.ack_information.success)
    {
      request_msg.wms_3gpp_failure_information_valid = TRUE;

      tp_cause = msg->getCause();

      request_msg.wms_3gpp_failure_information.tp_cause = (wms_tp_cause_enum_v01) tp_cause;

      /* Initialize the RP cause to unspecified */
      request_msg.wms_3gpp_failure_information.rp_cause = WMS_RP_CAUSE_PROTOCOL_ERROR_ACK_V01;

      /* If the TP cause is memory full, set the RP cause to memory full also */
      if ( tp_cause == SMS_TELEPHONY_MEMORY_EXCEEDED_ERROR )
      {
        request_msg.wms_3gpp_failure_information.rp_cause = WMS_RP_CAUSE_MEMORY_CAP_EXCEEDED_ACK_V01;
      }
    }

    if (i_ptr->sms_ack_info.gw_send_ack_on_ims)
    {
      request_msg.sms_on_ims_valid = TRUE;
      request_msg.sms_on_ims = TRUE;
    }
    else
    {
      request_msg.sms_on_ims_valid = TRUE;
      request_msg.sms_on_ims = FALSE;
    }

    user_data = QCRIL_COMPOSE_USER_DATA( instance_id, QCRIL_DEFAULT_MODEM_ID, token );

    if (qmi_client_sms_send_async(QMI_WMS_SEND_ACK_REQ_V01,
                                  &request_msg,
                                  sizeof(wms_send_ack_req_msg_v01),
                                  sizeof(wms_send_ack_resp_msg_v01),
                                  qcril_qmi_sms_command_cb,
                                  (void *)(uintptr_t) user_data) != QMI_NO_ERR)
    {
      QCRIL_LOG_ERROR("Failed to get the response message from qmi for acknowledgement request.");
      res = RIL_E_SYSTEM_ERR;
    }

    /* Reset the ack pending flag.  If the ack is not received by the network,
       the network will resend the SMS, and RIL will have another chance
       to ack it. */
    i_ptr->sms_ack_info.gw_ack_pending = FALSE;
    i_ptr->sms_ack_info.gw_ack_needed = FALSE;
    i_ptr->sms_ack_info.gw_send_ack_on_ims = FALSE;

    need_reset_ack_expry_tmr = TRUE;

  } while(0);

  if(res != RIL_E_SUCCESS)
  {
    if(inserted_to_pendinglist) {
        getSmsModule().getPendingMessageList().erase(msg);
    }
    auto resp = std::make_shared<RIL_Errno>(res);
    msg->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
  }

  if ( need_reset_ack_expry_tmr )
  {
      if ( QMI_RIL_ZERO != qcril_sms[QCRIL_DEFAULT_INSTANCE_ID].mt_pending_ack_expry_tmr )
      {
         QCRIL_LOG_INFO( "resetting MT SMS ack tmr per ack reception" );
         qcril_cancel_timed_callback( (void*)(uintptr_t)qcril_sms[QCRIL_DEFAULT_INSTANCE_ID].mt_pending_ack_expry_tmr );
         qcril_sms[QCRIL_DEFAULT_INSTANCE_ID].mt_pending_ack_expry_tmr = QMI_RIL_ZERO;
      }
  }

  QCRIL_SNPRINTF( details, sizeof( details ),
                  "qcril_sms[%d].sms_ack_info_mutex", instance_id );
  QCRIL_MUTEX_UNLOCK( &i_ptr->sms_ack_info_mutex, details );

  QCRIL_LOG_FUNC_RETURN();
} /* qcril_sms_request_sms_acknowledge */


/*===========================================================================

  FUNCTION:  qcril_ack_cb

===========================================================================*/
/*!
    @brief
    Handles response to RIL_REQUEST_SMS_ACKNOWLEDGE and
    RIL_REQUEST_CDMA_SMS_ACKNOWLEDGE.

    @return
    None.
*/
/*=========================================================================*/
void qcril_ack_cb
(
  std::shared_ptr<Message> msg,
  qmi_client_error_type transp_err,
  wms_send_ack_resp_msg_v01* ack_resp

)
{
  QCRIL_LOG_INFO("QMI_WMS_SEND_ACK_RESP received transport error %d, qmi error %d",
        transp_err, ack_resp->resp.result);
  RIL_Errno ril_err = qcril_qmi_util_convert_qmi_response_codes_to_ril_result(
        transp_err, &(ack_resp->resp));

  if (msg->get_message_id() == RilRequestAckGsmSmsMessage::get_class_message_id())
  {
    auto resp = std::make_shared<RIL_Errno>(ril_err);
    auto ril_msg = std::static_pointer_cast<RilRequestAckGsmSmsMessage>(msg);
    ril_msg->sendResponse(ril_msg, Message::Callback::Status::SUCCESS, resp);
  }
  else if (msg->get_message_id() == RilRequestAckCdmaSmsMessage::get_class_message_id())
  {
    auto resp = std::make_shared<RIL_Errno>(ril_err);
    auto ril_msg = std::static_pointer_cast<RilRequestAckCdmaSmsMessage>(msg);
    ril_msg->sendResponse(ril_msg, Message::Callback::Status::SUCCESS, resp);
  }
  else if (msg->get_message_id() == RilRequestAckImsSmsMessage::get_class_message_id())
  {
    auto resp = std::make_shared<RIL_Errno>(ril_err);
    auto ril_msg = std::static_pointer_cast<RilRequestAckImsSmsMessage>(msg);
    ril_msg->sendResponse(ril_msg, Message::Callback::Status::SUCCESS, resp);
  }
} /* qcril_ack_cb */


/*===========================================================================

  FUNCTION:  qcril_sms_request_write_sms_to_sim

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_WRITE_SMS_TO_SIM.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_write_sms_to_sim
(
  std::shared_ptr<RilRequestWriteSmsToSimMessage> msg
)
{
  wms_raw_write_req_msg_v01 write_request_msg;
  RIL_Errno ril_err = RIL_E_SUCCESS;
  int index_on_sim = -1;
  const char *smsc_address;
  const char *gw_pdu;


  QCRIL_LOG_FUNC_ENTRY();

  do
  {

    memset(&write_request_msg, 0, sizeof(wms_raw_write_req_msg_v01));

    if (!qcril_mo_sms_error_check(msg->getPdu().c_str()))
    {
      QCRIL_LOG_ERROR("qcril_mo_sms_error_check failed for SMS, which is going to written in SIM.");
      ril_err = RIL_E_INVALID_ARGUMENTS;
      break;
    }

    /* Invalid status */
    if (msg->getStatus() > 3 )
    {
      QCRIL_LOG_ERROR("Invalid SMS status, which is going to written in SIM.");
      ril_err = RIL_E_INVALID_ARGUMENTS;
      break;
    }

    /* Fill in the QMI request to write the message to the SIM */
    write_request_msg.raw_message_write_data.storage_type = WMS_STORAGE_TYPE_UIM_V01;

    /*send nullptr if smsc is empty*/
    smsc_address = msg->getSmscPdu().empty() ? nullptr : msg->getSmscPdu().c_str();
    gw_pdu = msg->getPdu().empty() ? nullptr : msg->getPdu().c_str();

    if (!qcril_sms_fill_wms_payload( FALSE,
                                     NULL,
                                     smsc_address,
                                     gw_pdu,
                                     0,
                                     TRUE,
                                     &write_request_msg.raw_message_write_data.format,
                                     (uint32 *)&write_request_msg.raw_message_write_data.raw_message_len,
                                     &write_request_msg.raw_message_write_data.raw_message[0],
                                     FALSE,
                                     qcril_sms_is_tag_mo(msg->getStatus())))
    {
      QCRIL_LOG_ERROR("Filling SMS payload per QMI requirement is failed for SMS, which is going to written in SIM.");
      ril_err = RIL_E_INVALID_SMS_FORMAT;
      break;
    }

    ril_err = qcril_sms_request_write_sms_to_uim(&write_request_msg,
                                                 qcril_sms_map_ril_tag_to_qmi_tag(msg->getStatus()),
                                                 WMS_MESSAGE_MODE_GW_V01,
                                                 index_on_sim);
  } while(0);

  auto resp = std::make_shared<RilWriteSmsToSimResult_t>(ril_err, index_on_sim);
  msg->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);

  QCRIL_LOG_FUNC_RETURN();
} /* qcril_sms_request_write_sms_to_sim */


/*===========================================================================

  FUNCTION:  qcril_sms_request_write_sms_to_uim

===========================================================================*/
/*!
    @brief
    Writes an SMS message to UIM.

    @return
    None.
*/
/*=========================================================================*/
RIL_Errno qcril_sms_request_write_sms_to_uim
(
  wms_raw_write_req_msg_v01 * write_request_msg,
  wms_message_tag_type_enum_v01 tag,
  wms_message_mode_enum_v01 message_mode,
  int& index_on_sim
)
{
  wms_raw_write_resp_msg_v01 write_response_msg;
  RIL_Errno ril_req_res = RIL_E_SUCCESS;
  qmi_client_error_type qmi_err;

  QCRIL_LOG_FUNC_ENTRY();

  QCRIL_NOTUSED(message_mode);
  do
  {
    if ( write_request_msg != NULL )
    {
      // Check WMS_SERVICE_READY
      //========================================
      // For all storage types (NV and UIM), so just check
      //   WMS_MESSAGE_FORMAT_CDMA_V01 (3GPP2)
      //   WMS_MESSAGE_FORMAT_GW_PP_V01 (3GPP)
      // Note: WMS_MESSAGE_FORMAT_GW_BC_V01 and WMS_MESSAGE_FORMAT_MWI_V01
      // return QMI_ERR_INVALID_ARG and so are not checked
      if ((write_request_msg->raw_message_write_data.format
                                                 != WMS_MESSAGE_FORMAT_GW_PP_V01) &&
          ( write_request_msg->raw_message_write_data.format
                                                 != WMS_MESSAGE_FORMAT_CDMA_V01))
      {
         QCRIL_LOG_ERROR("Unsupported format %d for write, dropping",
                         (int)write_request_msg->raw_message_write_data.format);
         ril_req_res = RIL_E_INVALID_SMS_FORMAT;
         break;
      }

      memset(&write_response_msg, 0, sizeof(wms_raw_write_resp_msg_v01));
      write_request_msg->tag_type_valid = TRUE;
      write_request_msg->tag_type = tag;

      /* Writing the SMS means invoking two QMI requests: to write the message and modify the
         tag.  Make the requests synchronous, because we cannot invoke the second QMI request from
         a callback function.  Writing a message to UIM should be fast, so it should be safe
         to use synchronous calls in this case. */
      qmi_err = qmi_client_sms_send_sync(QMI_WMS_RAW_WRITE_REQ_V01,
                                         write_request_msg,
                                         sizeof(wms_raw_write_req_msg_v01),
                                         &write_response_msg,
                                         sizeof(wms_raw_write_resp_msg_v01),
                                         ModemEndPoint::SYNC_REQ_UNRESTRICTED_TIMEOUT);

      ril_req_res = qcril_qmi_util_convert_qmi_response_codes_to_ril_result( qmi_err, &write_response_msg.resp );

      if ( RIL_E_SUCCESS == ril_req_res )
      {
        /* The message was successfully written.*/
        QCRIL_LOG_INFO("QMI_WMS_RAW_WRITE_RESP received: SUCCESS");

        /* Android uses the GSDI index, since it calls RIL_REQUEST_SIM_IO.
           The GSDI index is the WMS index plus 1. */
        index_on_sim = write_response_msg.storage_index + 1;
      }
    }
    else
    {
      QCRIL_LOG_ERROR("write_request_msg is NULL!");
      ril_req_res = RIL_E_INTERNAL_ERR;
    }
  } while(0);

  QCRIL_LOG_FUNC_RETURN();
  return ril_req_res;
} /* qcril_sms_request_write_sms_to_uim */


/*===========================================================================

  FUNCTION:  qcril_sms_request_delete_sms_on_sim

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_DELETE_SMS_ON_SIM

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_delete_sms_on_sim
(
  std::shared_ptr<RilRequestDeleteSmsOnSimMessage> msg
)
{
  uint32 user_data;
  bool inserted_to_pendinglist = false;
  wms_delete_req_msg_v01 request_msg;
  RIL_Errno ril_err = RIL_E_SUCCESS;


  QCRIL_LOG_FUNC_ENTRY();

  do
  {
    // Add Message to the pending list, and get the token in case of success
    auto ret = getSmsModule().getPendingMessageList().insert(msg);
    if (!ret.second)
    {
        QCRIL_LOG_ERROR("Failed to add an entry for delete SMS on SIM.");
        ril_err = RIL_E_INTERNAL_ERR;
        break;
    }
    inserted_to_pendinglist = true;
    uint16_t token = ret.first;

    /* Fill in the QMI request */
    memset(&request_msg, 0, sizeof(wms_delete_req_msg_v01));
    request_msg.storage_type = WMS_STORAGE_TYPE_UIM_V01;

    request_msg.index_valid = TRUE;
    request_msg.index = static_cast<int>(msg->getIndex()) - 1;

    request_msg.message_mode_valid = TRUE;
    request_msg.message_mode = WMS_MESSAGE_MODE_GW_V01;

    user_data = QCRIL_COMPOSE_USER_DATA(QCRIL_DEFAULT_INSTANCE_ID,
            QCRIL_DEFAULT_MODEM_ID, token);

    if (qmi_client_sms_send_async(QMI_WMS_DELETE_REQ_V01,
                                  &request_msg,
                                  sizeof(wms_delete_req_msg_v01),
                                  sizeof(wms_delete_resp_msg_v01),
                                  qcril_qmi_sms_command_cb,
                                  (void *)(uintptr_t) user_data) != QMI_NO_ERR)
    {
      QCRIL_LOG_ERROR("Failed to receive response message for RIL_REQUEST_DELETE_SMS_ON_SIM.");
      ril_err = RIL_E_SYSTEM_ERR;
      break;
    }

  } while(0);

  if(ril_err != RIL_E_SUCCESS)
  {
    if(inserted_to_pendinglist) {
        getSmsModule().getPendingMessageList().erase(msg);
    }
    auto resp = std::make_shared<RIL_Errno>(ril_err);
    msg->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
  }
  QCRIL_LOG_FUNC_RETURN();
} /* qcril_sms_request_delete_sms_on_sim */


/*===========================================================================

  FUNCTION:  qcril_delete_cb

===========================================================================*/
/*!
    @brief
    Handles response to RIL_REQUEST_DELETE_SMS_ON_SIM and
    RIL_REQUEST_CDMA_DELETE_SMS_ON_RUIM.

    @return
    None.
*/
/*=========================================================================*/
void qcril_delete_cb
(
  std::shared_ptr<Message> msg,
  qmi_client_error_type transp_err,
  wms_delete_resp_msg_v01* delete_resp
)
{
  QCRIL_LOG_INFO("QMI_WMS_DELETE_RESP received transport error %d, qmi error %d",
        transp_err, delete_resp->resp.result);
  RIL_Errno ril_err = qcril_qmi_util_convert_qmi_response_codes_to_ril_result(
        transp_err, &(delete_resp->resp));

  if (msg->get_message_id() == RilRequestDeleteSmsOnSimMessage::get_class_message_id())
  {
    auto resp = std::make_shared<RIL_Errno>(ril_err);
    auto ril_msg = std::static_pointer_cast<RilRequestDeleteSmsOnSimMessage>(msg);
    ril_msg->sendResponse(ril_msg, Message::Callback::Status::SUCCESS, resp);
  }
  else if (msg->get_message_id() == RilRequestCdmaDeleteSmsOnRuimMessage::get_class_message_id())
  {
    auto resp = std::make_shared<RIL_Errno>(ril_err);
    auto ril_msg = std::static_pointer_cast<RilRequestCdmaDeleteSmsOnRuimMessage>(msg);
    ril_msg->sendResponse(ril_msg, Message::Callback::Status::SUCCESS, resp);
  }
} /* qcril_delete_cb */


/*===========================================================================

  FUNCTION:  qcril_sms_request_get_smsc_address

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_GET_SMSC_ADDRESS.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_get_smsc_address
(
  std::shared_ptr<RilRequestGetSmscAddressMessage> msg
)
{
   uint32 user_data;
   bool inserted_to_pendinglist = false;
   qmi_client_error_type result;
   RIL_Errno ril_err = RIL_E_SUCCESS;


   QCRIL_LOG_FUNC_ENTRY();

   do
   {
      // Add Message to the pending list, and get the token in case of success
      auto ret = getSmsModule().getPendingMessageList().insert(msg);
      if (!ret.second)
      {
          QCRIL_LOG_ERROR("Failed to add an entry for delete SMS on SIM.");
          ril_err = RIL_E_INTERNAL_ERR;
          break;
      }
      inserted_to_pendinglist = true;
      uint16_t token = ret.first;

      user_data = QCRIL_COMPOSE_USER_DATA(QCRIL_DEFAULT_INSTANCE_ID,
            QCRIL_DEFAULT_MODEM_ID, token);

      result = qmi_client_sms_send_async(QMI_WMS_GET_SMSC_ADDRESS_REQ_V01,
                                         NULL,
                                         0,
                                         sizeof(wms_get_smsc_address_resp_msg_v01),
                                         qcril_qmi_sms_command_cb,
                                         (void *)(uintptr_t) user_data);
      if ( result != QMI_NO_ERR )
      {
         QCRIL_LOG_ERROR("Failed to get response from qmi for getting SMSC Address");
         ril_err = RIL_E_SYSTEM_ERR;
         break;
      }
   } while(0);

   if(ril_err != RIL_E_SUCCESS)
   {
     if(inserted_to_pendinglist) {
         getSmsModule().getPendingMessageList().erase(msg);
     }
     auto resp = std::make_shared<RilGetSmscAddrResult_t>(ril_err);
     msg->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
   }
   QCRIL_LOG_FUNC_RETURN();

} /* qcril_sms_request_get_smsc_address() */

/*===========================================================================

  FUNCTION:  qcril_get_smsc_addr_cb

===========================================================================*/
/*!
    @brief
    Handles response to RIL_REQUEST_GET_SMSC_ADDRESS.

    @return
    None.
*/
/*=========================================================================*/
void qcril_get_smsc_addr_cb
(
  std::shared_ptr<Message> msg,
  qmi_client_error_type transp_err,
  wms_get_smsc_address_resp_msg_v01* get_smsc_addr_resp
)
{
  std::string smsc_address;
  RIL_Errno ril_err;

  if (transp_err == QMI_NO_ERR && get_smsc_addr_resp->resp.result == QMI_RESULT_SUCCESS_V01)
  {
    QCRIL_LOG_INFO("QMI_WMS_GET_SMSC_ADDRESS_RESP received: SUCCESS");
    ril_err = qcril_sms_convert_smsc_address_to_ril_format(get_smsc_addr_resp, smsc_address);
  }
  else
  {
    ril_err = qcril_qmi_util_convert_qmi_response_codes_to_ril_result(transp_err, &(get_smsc_addr_resp->resp));
    QCRIL_LOG_INFO("QMI_WMS_GET_SMSC_ADDRESS_RESP received with transport error %d, qmi error %d",
            transp_err, get_smsc_addr_resp->resp.error);
  }

  if (msg->get_message_id() == RilRequestGetSmscAddressMessage::get_class_message_id())
  {
    auto ril_msg = std::static_pointer_cast<RilRequestGetSmscAddressMessage>(msg);
    auto resp = std::make_shared<RilGetSmscAddrResult_t>(ril_err, std::move(smsc_address));
    ril_msg->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
  }
} /* qcril_get_smsc_addr_cb */


/*===========================================================================

  FUNCTION:  qcril_sms_request_set_smsc_address

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_SET_SMSC_ADDRESS.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_set_smsc_address
(
  std::shared_ptr<RilRequestSetSmscAddressMessage> msg
)
{
  uint32 user_data;
  bool inserted_to_pendinglist = false;
  wms_set_smsc_address_req_msg_v01 request_msg;
  RIL_Errno ril_err = RIL_E_SUCCESS;

  QCRIL_LOG_FUNC_ENTRY();

  do
  {
    /* Translate the SMSC address from ATCOP format to QMI format */
    memset(&request_msg, 0, sizeof(wms_set_smsc_address_req_msg_v01));
    if ( !qcril_sms_convert_smsc_address_to_qmi_format(msg->getSmscAddr().c_str(), &request_msg ) )
    {
      QCRIL_LOG_ERROR("Unable to convert SMSC address to QMI format");
      ril_err = RIL_E_INVALID_SMS_FORMAT;
      break;
    }

    // Add Message to the pending list, and get the token in case of success
    auto res = getSmsModule().getPendingMessageList().insert(msg);
    if (!res.second)
    {
        ril_err = RIL_E_INTERNAL_ERR;
        break;
    }
    inserted_to_pendinglist = true;
    uint16_t token = res.first;

    user_data = QCRIL_COMPOSE_USER_DATA(QCRIL_DEFAULT_INSTANCE_ID,
            QCRIL_DEFAULT_MODEM_ID, token);

    if (qmi_client_sms_send_async(QMI_WMS_SET_SMSC_ADDRESS_REQ_V01,
                                  &request_msg,
                                  sizeof(wms_set_smsc_address_req_msg_v01),
                                  sizeof(wms_set_smsc_address_resp_msg_v01),
                                  qcril_qmi_sms_command_cb,
                                  (void *)(uintptr_t) user_data) != QMI_NO_ERR)
    {
      QCRIL_LOG_ERROR("Failed to get the response of setting the smsc address from qmi.");
      ril_err = RIL_E_SYSTEM_ERR;
    }
  } while(0);

  if(ril_err != RIL_E_SUCCESS)
  {
    if(inserted_to_pendinglist) {
        getSmsModule().getPendingMessageList().erase(msg);
    }
    auto resp = std::make_shared<RIL_Errno>(ril_err);
    msg->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
  }
  QCRIL_LOG_FUNC_RETURN();
} /* qcril_sms_request_set_smsc_address() */


/*===========================================================================

  FUNCTION:  qcril_set_smsc_addr_cb

===========================================================================*/
/*!
    @brief
    Handles response to RIL_REQUEST_SET_SMSC_ADDRESS.

    @return
    None.
*/
/*=========================================================================*/
void qcril_set_smsc_addr_cb
(
  std::shared_ptr<Message> msg,
  qmi_client_error_type transp_err,
  wms_set_smsc_address_resp_msg_v01* set_smsc_addr_resp
)
{

  QCRIL_LOG_INFO("QMI_WMS_SET_SMSC_ADDRESS_RESP transport error %d, qmi error %d",
        transp_err, set_smsc_addr_resp->resp.result);
  RIL_Errno ril_err = qcril_qmi_util_convert_qmi_response_codes_to_ril_result(
        transp_err, &(set_smsc_addr_resp->resp));

  if (msg->get_message_id() == RilRequestSetSmscAddressMessage::get_class_message_id())
  {
    auto resp = std::make_shared<RIL_Errno>(ril_err);
    auto ril_msg = std::static_pointer_cast<RilRequestSetSmscAddressMessage>(msg);
    ril_msg->sendResponse(ril_msg, Message::Callback::Status::SUCCESS, resp);
  }
} /* qcril_set_smsc_addr_cb */

/*===========================================================================

  FUNCTION:  qcril_sms_send_sms_memory_status

===========================================================================*/
/*!
    @brief
    sends sms memory status to modem

    @return
    errno_enum_type.
*/
/*=========================================================================*/
static qmi_client_error_type qcril_sms_send_sms_memory_status
(
    int    is_memory_available,
    void  *user_data
)
{
  wms_set_memory_status_req_msg_v01 request_msg;
  qmi_client_error_type ret = E_FAILURE;

  QCRIL_LOG_FUNC_ENTRY();

  do
  {

    /* Fill in the QMI request */
    memset(&request_msg, 0, sizeof(wms_set_memory_status_req_msg_v01));
    request_msg.memory_available = is_memory_available;

    if ((ret = qmi_client_sms_send_async(
                                    QMI_WMS_SET_MEMORY_STATUS_REQ_V01,
                                    &request_msg,
                                    sizeof(wms_set_memory_status_req_msg_v01),
                                    sizeof(wms_set_memory_status_resp_msg_v01),
                                    qcril_qmi_sms_command_cb,
                                    user_data)) != QMI_NO_ERR)
    {
      QCRIL_LOG_ERROR("Failed to get the response of sms memory status from qmi.");
    }

  } while(0);

  QCRIL_LOG_FUNC_RETURN();

  return ret;
} /* qcril_sms_send_sms_memory_status() */

/*===========================================================================

  FUNCTION:  qcril_sms_request_report_sms_memory_status

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_REPORT_SMS_MEMORY_STATUS

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_report_sms_memory_status
(
  std::shared_ptr<RilRequestReportSmsMemoryStatusMessage> msg
)
{
  uint32 user_data;
  bool inserted_to_pendinglist = false;
  const char *mem_full_name = "";
  wms_set_memory_status_req_msg_v01 request_msg;
  int is_wms_limited_status = FALSE;
  RIL_Errno ril_err = RIL_E_SUCCESS;

  QCRIL_LOG_FUNC_ENTRY();

  do
  {
    is_wms_limited_status = get_wms_limited_service_status();


    // Add Message to the pending list, and get the token in case of success
    auto ret = getSmsModule().getPendingMessageList().insert(msg);
    if (!ret.second)
    {
        QCRIL_LOG_ERROR("Failed to add an entry for delete SMS on SIM.");
        ril_err = RIL_E_INTERNAL_ERR;
        break;
    }
    inserted_to_pendinglist = true;
    uint16_t token = ret.first;

    SMS_CACHE_LOCK;
    if (!msg->isAvailable())
    {
      mem_full_name = "Memory is full";
      is_sms_memory_full = TRUE;
    }
    else
    {
      mem_full_name = "Memory is available";
      is_sms_memory_full = FALSE;
    }
    SMS_CACHE_UNLOCK;

    if (is_wms_limited_status && !wms_ready_atleast_limited_3gpp())
    {
       ril_err = RIL_E_INVALID_STATE;
       break;
    }

    QCRIL_LOG_DEBUG( "Report Memory Status - %s", mem_full_name );

    /* Fill in the QMI request */
    memset(&request_msg, 0, sizeof(wms_set_memory_status_req_msg_v01));
    request_msg.memory_available = static_cast<int>(msg->isAvailable());

    user_data = QCRIL_COMPOSE_USER_DATA( QCRIL_DEFAULT_INSTANCE_ID,
            QCRIL_DEFAULT_MODEM_ID, token);

    if (qmi_client_sms_send_async(QMI_WMS_SET_MEMORY_STATUS_REQ_V01,
                                  &request_msg,
                                  sizeof(wms_set_memory_status_req_msg_v01),
                                  sizeof(wms_set_memory_status_resp_msg_v01),
                                  qcril_qmi_sms_command_cb,
                                  (void *)(uintptr_t) user_data) != QMI_NO_ERR)
    {
      QCRIL_LOG_ERROR("Failed to get the response of sms memory status from qmi.");
      ril_err = RIL_E_SYSTEM_ERR;
    }

  } while(0);
  if(ril_err != RIL_E_SUCCESS)
  {
    if(inserted_to_pendinglist) {
        getSmsModule().getPendingMessageList().erase(msg);
    }
    auto resp = std::make_shared<RIL_Errno>(ril_err);
    msg->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
  }
  QCRIL_LOG_FUNC_RETURN();
} /* qcril_sms_request_report_sms_memory_status() */


/*===========================================================================

  FUNCTION:  qcril_report_mem_status_cb

===========================================================================*/
/*!
    @brief
    Handles response to RIL_REQUEST_REPORT_SMS_MEMORY_STATUS

    @return
    None.
*/
/*=========================================================================*/
void qcril_report_mem_status_cb
(
  std::shared_ptr<Message> msg,
  qmi_client_error_type transp_err,
  wms_set_memory_status_resp_msg_v01* mem_status_resp
)
{
  QCRIL_LOG_INFO("QMI_WMS_SET_MEMORY_STATUS_RESP received transport err %d, qmi error %d",
        transp_err, mem_status_resp->resp.result);
  RIL_Errno ril_err = qcril_qmi_util_convert_qmi_response_codes_to_ril_result(
        transp_err, &(mem_status_resp->resp));

  if (msg->get_message_id() == RilRequestReportSmsMemoryStatusMessage::get_class_message_id())
  {
    auto resp = std::make_shared<RIL_Errno>(ril_err);
    auto ril_msg = std::static_pointer_cast<RilRequestReportSmsMemoryStatusMessage>(msg);
    ril_msg->sendResponse(ril_msg, Message::Callback::Status::SUCCESS, resp);
  }
} /* qcril_report_mem_status_cb */


/*===========================================================================

  FUNCTION:  qcril_sms_request_gsm_get_broadcast_sms_config

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_GSM_GET_BROADCAST_SMS_CONFIG.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_gsm_get_broadcast_sms_config
(
  std::shared_ptr<RilRequestGetGsmBroadcastConfigMessage> msg
)
{
  uint32 user_data;
  wms_get_broadcast_config_req_msg_v01 request_msg;
  int is_wms_limited_status = FALSE;
  bool inserted_to_pendinglist = false;
  RIL_Errno ril_err = RIL_E_SUCCESS;


  QCRIL_LOG_FUNC_ENTRY();

  do
  {
    is_wms_limited_status = get_wms_limited_service_status();

    if (is_wms_limited_status && !wms_ready_full_3gpp())
    {
       ril_err = RIL_E_INVALID_STATE;
       break;
    }

    // Add Message to the pending list, and get the token in case of success
    auto res = getSmsModule().getPendingMessageList().insert(msg);
    if (!res.second)
    {
        ril_err = RIL_E_INTERNAL_ERR;
        break;
    }
    inserted_to_pendinglist = true;
    uint16_t token = res.first;

    /* Fill in the QMI request */
    memset(&request_msg, 0, sizeof(wms_get_broadcast_config_req_msg_v01));
    request_msg.message_mode = WMS_MESSAGE_MODE_GW_V01;

    user_data = QCRIL_COMPOSE_USER_DATA(QCRIL_DEFAULT_INSTANCE_ID,
            QCRIL_DEFAULT_MODEM_ID, token);

    if (qmi_client_sms_send_async(QMI_WMS_GET_BROADCAST_CONFIG_REQ_V01,
                                 &request_msg,
                                 sizeof(wms_get_broadcast_config_req_msg_v01),
                                 sizeof(wms_get_broadcast_config_resp_msg_v01),
                                 qcril_qmi_sms_command_cb,
                                 (void *)(uintptr_t) user_data) != QMI_NO_ERR)
    {
      QCRIL_LOG_ERROR("Failed to get the response of broadcast sms config from qmi.");
      ril_err = RIL_E_SYSTEM_ERR;
      break;
    }
  } while(0);

  if(ril_err != RIL_E_SUCCESS)
  {
    if(inserted_to_pendinglist) {
        getSmsModule().getPendingMessageList().erase(msg);
    }
    auto resp = std::make_shared<RilGetGsmBroadcastConfigResult_t>(ril_err);
    msg->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
  }
  QCRIL_LOG_FUNC_RETURN();

} /* qcril_sms_request_gsm_get_broadcast_sms_config() */


/*===========================================================================

  FUNCTION:  qcril_get_bc_config_cb

===========================================================================*/
/*!
    @brief
    Handles response to RIL_REQUEST_GSM_GET_BROADCAST_SMS_CONFIG and
    RIL_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG.

    @return
    None.
*/
/*=========================================================================*/
void qcril_get_bc_config_cb
(
  std::shared_ptr<Message> msg,
  qmi_client_error_type transp_err,
  wms_get_broadcast_config_resp_msg_v01* bc_config_resp
)
{
  RIL_Errno ril_err = RIL_E_SUCCESS;

  do
  {
    if (transp_err == QMI_NO_ERR && bc_config_resp->resp.result == QMI_RESULT_SUCCESS_V01)
    {
      QCRIL_LOG_INFO("QMI_WMS_GET_BROADCAST_CONFIG_RESP received: SUCCESS");

      /* 3GPP */
      if (bc_config_resp->wms_3gpp_broadcast_info_valid)
      {
        auto table_size = bc_config_resp->wms_3gpp_broadcast_info.wms_3gpp_broadcast_config_info_len;
        std::vector<RIL_GSM_BroadcastSmsConfigInfo> config_list(table_size);

        for (unsigned i = 0; i < table_size; i++)
        {
            config_list[i].fromServiceId =
                bc_config_resp->wms_3gpp_broadcast_info.wms_3gpp_broadcast_config_info[i].from_service_id;
            config_list[i].toServiceId =
                bc_config_resp->wms_3gpp_broadcast_info.wms_3gpp_broadcast_config_info[i].to_service_id;
            /* The modem does not support getting/setting the data coding
               scheme.  The interface specifies all values outside of
               0x00 to 0xFF will be treated as empty data coding scheme. */
            config_list[i].fromCodeScheme = 0xFFF;
            config_list[i].toCodeScheme = 0xFFF;
            config_list[i].selected =
                bc_config_resp->wms_3gpp_broadcast_info.wms_3gpp_broadcast_config_info[i].selected;
        }
        if (msg->get_message_id() == RilRequestGetGsmBroadcastConfigMessage::get_class_message_id())
        {
            auto resp = std::make_shared<RilGetGsmBroadcastConfigResult_t>(RIL_E_SUCCESS,
                    std::move(config_list));
            auto ril_msg = std::static_pointer_cast<RilRequestGetGsmBroadcastConfigMessage>(msg);
            ril_msg->sendResponse(ril_msg, Message::Callback::Status::SUCCESS, resp);
        }
        else
        {
            QCRIL_LOG_ERROR("unexpected: the pending message id is not get gsm broadcast config");
        }
      }
      /* 3GPP2 */
      else if (bc_config_resp->wms_3gpp2_broadcast_info_valid)
      {
        auto table_size = bc_config_resp->wms_3gpp2_broadcast_info.wms_3gpp2_broadcast_config_info_len;
        std::vector<RIL_CDMA_BroadcastSmsConfigInfo> config_list(table_size);

        for (unsigned i = 0; i < table_size; i++)
        {
            config_list[i].language =
                bc_config_resp->wms_3gpp2_broadcast_info.wms_3gpp2_broadcast_config_info[i].language;
            config_list[i].service_category =
                bc_config_resp->wms_3gpp2_broadcast_info.wms_3gpp2_broadcast_config_info[i].service_category;
            config_list[i].selected =
                bc_config_resp->wms_3gpp2_broadcast_info.wms_3gpp2_broadcast_config_info[i].selected;
        }
        if (msg->get_message_id() == RilRequestGetCdmaBroadcastConfigMessage::get_class_message_id())
        {
            auto resp = std::make_shared<RilGetCdmaBroadcastConfigResult_t>(RIL_E_SUCCESS,
                    std::move(config_list));
            auto ril_msg = std::static_pointer_cast<RilRequestGetCdmaBroadcastConfigMessage>(msg);
            ril_msg->sendResponse(ril_msg, Message::Callback::Status::SUCCESS, resp);
        }
        else
        {
            QCRIL_LOG_ERROR("unexpected: the pending message id is not get cdma broadcast config");
        }
      }
      else
      {
        QCRIL_LOG_INFO("QMI_WMS_GET_BROADCAST_CONFIG_RESP did not contain either 3GPP or 3GPP2 info");
        ril_err = RIL_E_NO_RESOURCES;
        break;
      }
    }
    else
    {
      QCRIL_LOG_INFO("QMI_WMS_GET_BROADCAST_CONFIG_RESP received transport error %d, qmi error %d",
            transp_err, bc_config_resp->resp.error);
      ril_err = qcril_qmi_util_convert_qmi_response_codes_to_ril_result(transp_err, &(bc_config_resp->resp));
      break;
    }
  } while(0);

  if(ril_err != RIL_E_SUCCESS)
  {
    if (msg->get_message_id() == RilRequestGetGsmBroadcastConfigMessage::get_class_message_id())
    {
        auto resp = std::make_shared<RilGetGsmBroadcastConfigResult_t>(ril_err);
        auto ril_msg = std::static_pointer_cast<RilRequestGetGsmBroadcastConfigMessage>(msg);
        ril_msg->sendResponse(ril_msg, Message::Callback::Status::SUCCESS, resp);
    }
    else if (msg->get_message_id() == RilRequestGetCdmaBroadcastConfigMessage::get_class_message_id())
    {
        auto resp = std::make_shared<RilGetCdmaBroadcastConfigResult_t>(ril_err);
        auto ril_msg = std::static_pointer_cast<RilRequestGetCdmaBroadcastConfigMessage>(msg);
        ril_msg->sendResponse(ril_msg, Message::Callback::Status::SUCCESS, resp);
    }
  }
} /* qcril_get_bc_config_cb */


/*===========================================================================

  FUNCTION:  qcril_sms_request_gsm_set_broadcast_sms_config

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_GSM_SET_BROADCAST_SMS_CONFIG

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_gsm_set_broadcast_sms_config
(
  std::shared_ptr<RilRequestGsmSetBroadcastSmsConfigMessage> msg
)
{
  uint32 user_data;
  uint32 i;
  uint8 bc_table_size;
  bool inserted_to_pendinglist = false;
  wms_set_broadcast_config_req_msg_v01 request_msg;
  int is_wms_limited_status = FALSE;
  RIL_Errno ril_err = RIL_E_SUCCESS;


  QCRIL_LOG_FUNC_ENTRY();

  do
  {
    is_wms_limited_status = get_wms_limited_service_status();

    if (is_wms_limited_status && !wms_ready_full_3gpp())
    {
       ril_err = RIL_E_INVALID_STATE;
       break;
    }

    /* Check if the broadcast SMS table is too big */
    bc_table_size = msg->getConfigList().size();
    if ( bc_table_size > WMS_3GPP_BROADCAST_CONFIG_MAX_V01 ) // use QMI size
    {
      QCRIL_LOG_ERROR( "Size of GW Broadcast SMS Config table is too big : %d", bc_table_size );
      ril_err = RIL_E_INVALID_ARGUMENTS;
      break;
    }

    // Add Message to the pending list, and get the token in case of success
    auto res = getSmsModule().getPendingMessageList().insert(msg);
    if (!res.second)
    {
        ril_err = RIL_E_INTERNAL_ERR;
        break;
    }
    inserted_to_pendinglist = true;
    uint16_t token = res.first;

    /* Fill in the QMI request */
    memset(&request_msg, 0, sizeof(wms_set_broadcast_config_req_msg_v01));
    request_msg.message_mode = WMS_MESSAGE_MODE_GW_V01;
    request_msg.wms_3gpp_broadcast_config_info_valid = TRUE;
    request_msg.wms_3gpp_broadcast_config_info_len = bc_table_size;

    auto& inputData = msg->getConfigList();
    for ( i = 0; i < bc_table_size; i++ )
    {
      request_msg.wms_3gpp_broadcast_config_info[i].from_service_id =
            inputData[i].fromServiceId;
      request_msg.wms_3gpp_broadcast_config_info[i].to_service_id =
            inputData[i].toServiceId;
      request_msg.wms_3gpp_broadcast_config_info[i].selected =
            inputData[i].selected;
    }

    user_data = QCRIL_COMPOSE_USER_DATA(QCRIL_DEFAULT_INSTANCE_ID,
            QCRIL_DEFAULT_MODEM_ID, token);

    if (qmi_client_sms_send_async(QMI_WMS_SET_BROADCAST_CONFIG_REQ_V01,
                                 &request_msg,
                                 sizeof(wms_set_broadcast_config_req_msg_v01),
                                 sizeof(wms_set_broadcast_config_resp_msg_v01),
                                 qcril_qmi_sms_command_cb,
                                 (void *)(uintptr_t) user_data) != QMI_NO_ERR)
    {
      QCRIL_LOG_ERROR("Failed to get the response of setting the broadcast sms config from qmi.");
      ril_err = RIL_E_SYSTEM_ERR;
      break;
    }
  } while(0);

  if(ril_err != RIL_E_SUCCESS)
  {
    if(inserted_to_pendinglist) {
        getSmsModule().getPendingMessageList().erase(msg);
    }
    auto resp = std::make_shared<RIL_Errno>(ril_err);
    msg->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
  }

  QCRIL_LOG_FUNC_RETURN();
} /* qcril_sms_request_gsm_set_broadcast_sms_config() */


/*===========================================================================

  FUNCTION:  qcril_set_bc_config_cb

===========================================================================*/
/*!
    @brief
    Handles response to RIL_REQUEST_GSM_SET_BROADCAST_SMS_CONFIG and
    RIL_REQUEST_CDMA_SET_BROADCAST_SMS_CONFIG.

    @return
    None.
*/
/*=========================================================================*/
void qcril_set_bc_config_cb
(
  std::shared_ptr<Message> msg,
  qmi_client_error_type transp_err,
  wms_set_broadcast_config_resp_msg_v01 *bc_config_resp
)
{
  QCRIL_LOG_INFO("QMI_WMS_SET_BROADCAST_CONFIG_RESP received transport error %d, qmi error %d",
        transp_err, bc_config_resp->resp.result);
  RIL_Errno ril_err = qcril_qmi_util_convert_qmi_response_codes_to_ril_result(
        transp_err, &(bc_config_resp->resp));

  if (msg->get_message_id() == RilRequestGsmSetBroadcastSmsConfigMessage::get_class_message_id())
  {
    auto resp = std::make_shared<RIL_Errno>(ril_err);
    auto ril_msg = std::static_pointer_cast<RilRequestGsmSetBroadcastSmsConfigMessage>(msg);
    ril_msg->sendResponse(ril_msg, Message::Callback::Status::SUCCESS, resp);
  }
  else if (msg->get_message_id() == RilRequestCdmaSetBroadcastSmsConfigMessage::get_class_message_id())
  {
    auto resp = std::make_shared<RIL_Errno>(ril_err);
    auto ril_msg = std::static_pointer_cast<RilRequestCdmaSetBroadcastSmsConfigMessage>(msg);
    ril_msg->sendResponse(ril_msg, Message::Callback::Status::SUCCESS, resp);
  }
} /* qcril_set_bc_config_cb */


/*===========================================================================

  FUNCTION:  qcril_sms_request_gsm_sms_broadcast_activation

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_GSM_SMS_BROADCAST_ACTIVATION

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_gsm_sms_broadcast_activation
(
  std::shared_ptr<RilRequestGsmSmsBroadcastActivateMessage> msg
)
{
  uint32 user_data;
  bool inserted_to_pendinglist = false;
  wms_set_broadcast_activation_req_msg_v01 request_msg;
  int is_wms_limited_status = FALSE;
  RIL_Errno ril_err = RIL_E_SUCCESS;

  QCRIL_LOG_FUNC_ENTRY();

  do
  {
    is_wms_limited_status = get_wms_limited_service_status();

    if (is_wms_limited_status && !wms_ready_full_3gpp())
    {
       ril_err = RIL_E_INVALID_STATE;
       break;
    }

    // Add Message to the pending list, and get the token in case of success
    auto ret = getSmsModule().getPendingMessageList().insert(msg);
    if (!ret.second)
    {
        QCRIL_LOG_ERROR("Failed to add an entry for delete SMS on SIM.");
        ril_err = RIL_E_INTERNAL_ERR;
        break;
    }
    inserted_to_pendinglist = true;
    uint16_t token = ret.first;

    /* Fill in QMI message */
    memset(&request_msg, 0, sizeof(wms_set_broadcast_activation_req_msg_v01));
    request_msg.broadcast_activation_info.message_mode = WMS_MESSAGE_MODE_GW_V01;
    request_msg.broadcast_activation_info.bc_activate = static_cast<int>(msg->getActivateStatus());

    if( feature_all_bc_msg_3gpp )
    {
        request_msg.activate_all_valid = TRUE;
        request_msg.activate_all = 0x01;
    }

    user_data = QCRIL_COMPOSE_USER_DATA(QCRIL_DEFAULT_INSTANCE_ID,
            QCRIL_DEFAULT_MODEM_ID, token);

    if (qmi_client_sms_send_async(QMI_WMS_SET_BROADCAST_ACTIVATION_REQ_V01,
                                  &request_msg,
                                  sizeof(wms_set_broadcast_activation_req_msg_v01),
                                  sizeof(wms_set_broadcast_activation_resp_msg_v01),
                                  qcril_qmi_sms_command_cb,
                                  (void *)(uintptr_t) user_data) != QMI_NO_ERR)
    {
      QCRIL_LOG_ERROR("Failed to get the response of sms broadcast activation.");
      ril_err = RIL_E_SYSTEM_ERR;
      break;
    }

  } while(0);

  if(ril_err != RIL_E_SUCCESS)
  {
    if(inserted_to_pendinglist) {
        getSmsModule().getPendingMessageList().erase(msg);
    }
    auto resp = std::make_shared<RIL_Errno>(ril_err);
    msg->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
  }
  QCRIL_LOG_FUNC_RETURN();
} /* qcril_sms_request_gsm_sms_broadcast_activation() */


/*===========================================================================

  FUNCTION:  qcril_bc_activate_cb

===========================================================================*/
/*!
    @brief
    Handles response to RIL_REQUEST_GSM_SMS_BROADCAST_ACTIVATION and
    RIL_REQUEST_CDMA_SMS_BROADCAST_ACTIVATION.

    @return
    None.
*/
/*=========================================================================*/
void qcril_bc_activate_cb
(
  std::shared_ptr<Message> msg,
  qmi_client_error_type transp_err,
  wms_set_broadcast_activation_resp_msg_v01* bc_activate_resp
)
{
  QCRIL_LOG_INFO("QMI_WMS_SET_BROADCAST_ACTIVATION_RESP received tranport err %d, qmi error %d",
        transp_err, bc_activate_resp->resp.result);
  RIL_Errno ril_err = qcril_qmi_util_convert_qmi_response_codes_to_ril_result(
        transp_err, &(bc_activate_resp->resp));

  if (msg->get_message_id() == RilRequestGsmSmsBroadcastActivateMessage::get_class_message_id())
  {
    auto resp = std::make_shared<RIL_Errno>(ril_err);
    auto ril_msg = std::static_pointer_cast<RilRequestGsmSmsBroadcastActivateMessage>(msg);
    ril_msg->sendResponse(ril_msg, Message::Callback::Status::SUCCESS, resp);
  }
  else if (msg->get_message_id() == RilRequestCdmaSmsBroadcastActivateMessage::get_class_message_id())
  {
    auto resp = std::make_shared<RIL_Errno>(ril_err);
    auto ril_msg = std::static_pointer_cast<RilRequestCdmaSmsBroadcastActivateMessage>(msg);
    ril_msg->sendResponse(ril_msg, Message::Callback::Status::SUCCESS, resp);
  }
} /* qcril_bc_activate_cb */

/*=========================================================================
  FUNCTION: qcril_sms_force_on_dc
===========================================================================*/
boolean qcril_sms_force_on_dc()
{
  char mcc[NAS_MCC_MNC_MAX_SIZE] = {0};
  char mnc[NAS_MCC_MNC_MAX_SIZE] = {0};

  if (qcril_qmi_nas_fetch_mcc_mnc_of_current_system(mcc, mnc, FALSE, TRUE))
  {
    QCRIL_LOG_INFO("qcril_sms_force_on_dc: mcc %s, mnc %s",mcc,mnc);
    return qcril_db_query_force_on_dc(mcc, mnc);
  }
  return false;
} // qcril_sms_force_on_dc

/*===========================================================================

  FUNCTION:  qcril_sms_request_cdma_send_sms

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_CDMA_SEND_SMS.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_cdma_send_sms
(
  std::shared_ptr<RilRequestCdmaSendSmsMessage> msg
)
{
  uint32 user_data;
  bool inserted_to_pendinglist = false;
  wms_raw_send_req_msg_v01 request_msg;
  nas_sms_status_enum_type_v01 lte_sms_status;
  uint8_t lte_sms_status_valid;
  uint8_t is_voice_over_lte = FALSE;
  RIL_Errno ril_err = RIL_E_SUCCESS;

  QCRIL_LOG_FUNC_ENTRY();

  do
  {
    qcril_qmi_nas_fetch_lte_sms_status( &lte_sms_status_valid, &lte_sms_status);
    is_voice_over_lte = qcril_qmi_nas_is_voice_over_lte_registered();
    if (!wms_ready_atleast_limited_3gpp2() &&
        !(is_voice_over_lte && lte_sms_status_valid && (NAS_SMS_STATUS_1X_V01 == lte_sms_status))
       ) {
       ril_err = RIL_E_INVALID_STATE;
       break;
    }

    if( TRUE == qmi_ril_block_mo_sms_on_1x() ) //check If user wants the MO SMS to be blocked when IMS is not yet registered
    {
      QCRIL_LOG_ERROR("Sending cdma MO sms failed because IMS is not registered.");
      ril_err = RIL_E_INVALID_STATE;
      break;
    }

    // Add Message to the pending list, and get the token in case of success
    auto res = getSmsModule().getPendingMessageList().insert(msg);
    if (!res.second)
    {
        ril_err = RIL_E_INTERNAL_ERR;
        break;
    }
    inserted_to_pendinglist = true;
    uint16_t token = res.first;

    auto cdma_sms_msg = msg->getCdmaSms();
    /* Fill in the QMI request */
    memset(&request_msg, 0, sizeof(wms_raw_send_req_msg_v01));
    if (!qcril_sms_fill_wms_payload(
                                   TRUE,
                                   &cdma_sms_msg,
                                   NULL,
                                   NULL,
                                   0,
                                   TRUE,
                                   &request_msg.raw_message_data.format,
                                   (uint32 *)&request_msg.raw_message_data.raw_message_len,
                                   &request_msg.raw_message_data.raw_message[0],
                                   FALSE,
                                   TRUE ))
    {
      QCRIL_LOG_ERROR("Filling SMS payload per QMI requirement is failed for cdma SMS message.");
      ril_err = RIL_E_INVALID_SMS_FORMAT;
      break;
    }
    QCRIL_LOG_DEBUG("follow_on_dc_valid: %d", cdma_sms_msg.expectMore);
    if( cdma_sms_msg.expectMore) {
        request_msg.follow_on_dc_valid = TRUE;
        request_msg.follow_on_dc = WMS_FOLLOW_ON_DC_ON_V01;
    }

    request_msg.sms_on_ims_valid = TRUE;
    request_msg.sms_on_ims = FALSE;

    QCRIL_LOG_DEBUG( "IS FORCE ON DC = %d", is_force_on_dc );
    if(is_force_on_dc || qcril_sms_force_on_dc())
    {
        request_msg.force_on_dc_valid = TRUE;
        request_msg.force_on_dc.force_on_dc = 0x01;
    }

    /* Compose the user data */
    user_data = QCRIL_COMPOSE_USER_DATA( QCRIL_DEFAULT_INSTANCE_ID, QCRIL_DEFAULT_MODEM_ID,
                                         token );

    if (qmi_client_sms_send_async(QMI_WMS_RAW_SEND_REQ_V01,
                                  &request_msg,
                                  sizeof(wms_raw_send_req_msg_v01),
                                  sizeof(wms_raw_send_resp_msg_v01),
                                  qcril_qmi_sms_command_cb,
                                  (void *)(uintptr_t) user_data) != QMI_NO_ERR)
    {
      QCRIL_LOG_ERROR("Failed to get the response of cdma sms from qmi.");
      ril_err = RIL_E_SYSTEM_ERR;
      break;
    }

  } while(0);

  if(ril_err != RIL_E_SUCCESS)
  {
    if(inserted_to_pendinglist) {
        getSmsModule().getPendingMessageList().erase(msg);
    }
    auto resp = std::make_shared<RilSendSmsResult_t>(ril_err);
    msg->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
  }

  QCRIL_LOG_FUNC_RETURN();
} /* qcril_sms_request_cdma_send_sms() */


/*===========================================================================

  FUNCTION:  qcril_sms_request_cdma_sms_acknowledge

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_CDMA_SMS_ACKNOWLEDGE.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_cdma_sms_acknowledge
(
  std::shared_ptr<RilRequestAckCdmaSmsMessage> msg
)
{
  qcril_instance_id_e_type instance_id;
  qcril_sms_struct_type *i_ptr;
  uint32 user_data;
  char details[ 80 ];
  bool inserted_to_pendinglist = false;
  wms_send_ack_req_msg_v01 request_msg;
  RIL_Errno ril_err = RIL_E_SUCCESS;

  int                        need_reset_ack_expry_tmr;
  int is_wms_limited_status = FALSE;

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_FUNC_ENTRY();

  instance_id = QCRIL_DEFAULT_INSTANCE_ID;

  i_ptr = &qcril_sms[ instance_id ];

  need_reset_ack_expry_tmr = FALSE;

  /*-----------------------------------------------------------------------*/

  QCRIL_SNPRINTF( details, sizeof( details ),
                  "qcril_sms[%d].sms_ack_info_mutex", instance_id );
  QCRIL_MUTEX_LOCK( &i_ptr->sms_ack_info_mutex, details );

  do
  {
    if (!i_ptr->sms_ack_info.cdma_ack_pending)
    {
      QCRIL_LOG_ERROR("Unable to send ack; no messages to be acked." );
      ril_err = RIL_E_NO_SMS_TO_ACK;
      break;
    }

    if (!i_ptr->sms_ack_info.cdma_ack_needed)
    {
      QCRIL_LOG_INFO("Dropping ack since no ack is needed.");

      i_ptr->sms_ack_info.cdma_ack_pending = FALSE;
      i_ptr->sms_ack_info.cdma_send_ack_on_ims = FALSE;

      need_reset_ack_expry_tmr = TRUE;

      msg->sendResponse(msg, Message::Callback::Status::SUCCESS,
            std::make_shared<RIL_Errno>(RIL_E_SUCCESS));
      break;
    }

    is_wms_limited_status = get_wms_limited_service_status();

    if (is_wms_limited_status && !wms_ready_atleast_limited_3gpp2() && !i_ptr->sms_ack_info.cdma_send_ack_on_ims)
    {
       /* Reset the ack pending flag as Telephony will not send another ACK request, so that RIL can process
       further MT SMS. */
       i_ptr->sms_ack_info.cdma_ack_pending = FALSE;
       i_ptr->sms_ack_info.cdma_ack_needed = FALSE;
       i_ptr->sms_ack_info.cdma_send_ack_on_ims = FALSE;
       ril_err = RIL_E_INVALID_STATE;
       break;
    }


    // Add Message to the pending list, and get the token in case of success
    auto ret = getSmsModule().getPendingMessageList().insert(msg);
    if (!ret.second)
    {
      ril_err = RIL_E_INTERNAL_ERR;
      QCRIL_LOG_ERROR("Failed to add an entry for acknowledgement of cdma sms.");
      /* Reset the ack pending flag as Telephony will not send another ACK request, so that RIL can process
      further MT SMS. */
      i_ptr->sms_ack_info.cdma_ack_pending = FALSE;
      i_ptr->sms_ack_info.cdma_ack_needed = FALSE;
      i_ptr->sms_ack_info.cdma_send_ack_on_ims = FALSE;
      break;

    }
    inserted_to_pendinglist = true;
    uint16_t token = ret.first;

    /* Fill in the QMI request */
    memset(&request_msg, 0, sizeof(wms_send_ack_req_msg_v01));
    request_msg.ack_information.transaction_id = i_ptr->sms_ack_info.cdma_transaction_id;
    request_msg.ack_information.message_protocol = WMS_MESSAGE_PROTOCOL_CDMA_V01;

    if ( msg->isSuccess())
    {
      request_msg.ack_information.success = TRUE;
    }
    else
    {
      request_msg.ack_information.success = FALSE;

      request_msg.wms_3gpp2_failure_information_valid = TRUE;
      request_msg.wms_3gpp2_failure_information.error_class = WMS_ERROR_CLASS_3GPP2_FAILURE_TEMPORARY_V01;
      request_msg.wms_3gpp2_failure_information.tl_status =
        (wms_tl_cause_code_send_ack_enum_v01) msg->getCause();
    }

    if (i_ptr->sms_ack_info.cdma_send_ack_on_ims)
    {
      request_msg.sms_on_ims_valid = TRUE;
      request_msg.sms_on_ims = TRUE;
    }
    else
    {
      request_msg.sms_on_ims_valid = TRUE;
      request_msg.sms_on_ims = FALSE;
    }

    user_data = QCRIL_COMPOSE_USER_DATA( instance_id, QCRIL_DEFAULT_MODEM_ID, token);

    if (qmi_client_sms_send_async(QMI_WMS_SEND_ACK_REQ_V01,
                                  &request_msg,
                                  sizeof(wms_send_ack_req_msg_v01),
                                  sizeof(wms_send_ack_resp_msg_v01),
                                  qcril_qmi_sms_command_cb,
                                  (void *)(uintptr_t) user_data) != QMI_NO_ERR)
    {
      QCRIL_LOG_ERROR("Failed to get the response for acknowledgement of cdma sms.");
      ril_err = RIL_E_SYSTEM_ERR;
      break;
    }

    /* Reset the ack pending flag. If the ack is not received by the network,
       the network will resend the SMS, and RIL will have another chance
       to ack it. */
    i_ptr->sms_ack_info.cdma_ack_pending = FALSE;
    i_ptr->sms_ack_info.cdma_ack_needed = FALSE;
    i_ptr->sms_ack_info.cdma_send_ack_on_ims = FALSE;

    need_reset_ack_expry_tmr = TRUE;
  } while(0);

  if(ril_err != RIL_E_SUCCESS)
  {
    if(inserted_to_pendinglist) {
        getSmsModule().getPendingMessageList().erase(msg);
    }
    auto resp = std::make_shared<RIL_Errno>(ril_err);
    msg->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
  }

  if ( need_reset_ack_expry_tmr )
  {
      if ( QMI_RIL_ZERO != qcril_sms[QCRIL_DEFAULT_INSTANCE_ID].mt_pending_ack_expry_tmr )
      {
        QCRIL_LOG_INFO( "resetting MT SMS ack tmr per ack reception" );
        qcril_cancel_timed_callback( (void*)(uintptr_t)qcril_sms[QCRIL_DEFAULT_INSTANCE_ID].mt_pending_ack_expry_tmr );
        qcril_sms[QCRIL_DEFAULT_INSTANCE_ID].mt_pending_ack_expry_tmr = QMI_RIL_ZERO;
      }
  }

  QCRIL_SNPRINTF( details, sizeof( details ),
                 "qcril_sms[%d].sms_ack_info_mutex", instance_id );
  QCRIL_MUTEX_UNLOCK( &i_ptr->sms_ack_info_mutex, details );

  QCRIL_LOG_FUNC_RETURN();

} /* qcril_sms_request_cdma_sms_acknowledge */


/*===========================================================================

  FUNCTION:  qcril_sms_request_cdma_write_sms_to_ruim

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_CDMA_WRITE_SMS_TO_RUIM.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_cdma_write_sms_to_ruim
(
  std::shared_ptr<RilRequestCdmaWriteSmsToRuimMessage> msg
)
{
  wms_raw_write_req_msg_v01 request_msg;
  RIL_Errno ril_err = RIL_E_SUCCESS;
  int index_on_sim = -1;

  QCRIL_LOG_FUNC_ENTRY();

  do
  {
    memset(&request_msg, 0, sizeof(wms_raw_write_req_msg_v01));

    /* Invalid status */
    if (msg->getStatus() > 3)
    {
      QCRIL_LOG_ERROR("Invalid status of cdma write sms to RUIM.");
      ril_err = RIL_E_INVALID_ARGUMENTS;
      break;
    }

    /* Fill in the QMI request to write the message to the RUIM */
    request_msg.raw_message_write_data.storage_type = WMS_STORAGE_TYPE_UIM_V01;

    if (!qcril_sms_fill_wms_payload( TRUE,
                                     &msg->getCdmaSms(),
                                     NULL,
                                     NULL,
                                     0,
                                     TRUE,
                                     &request_msg.raw_message_write_data.format,
                                     (uint32 *)&request_msg.raw_message_write_data.raw_message_len,
                                     &request_msg.raw_message_write_data.raw_message[0],
                                     FALSE,
                                     qcril_sms_is_tag_mo(msg->getStatus())))
    {
      QCRIL_LOG_ERROR("Filling SMS payload per QMI requirement is failed for cdma write sms to RUIM.");
      ril_err = RIL_E_INVALID_SMS_FORMAT;
      break;
    }

    ril_err = qcril_sms_request_write_sms_to_uim(&request_msg,
                                                 qcril_sms_map_ril_tag_to_qmi_tag(msg->getStatus()),
                                                 WMS_MESSAGE_MODE_CDMA_V01,
                                                 index_on_sim);
  } while (0);

  auto resp = std::make_shared<RilWriteSmsToSimResult_t>(ril_err, index_on_sim);
  msg->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);

  QCRIL_LOG_FUNC_RETURN();
} /* qcril_sms_request_cdma_write_sms_to_ruim() */


/*===========================================================================

  FUNCTION:  qcril_sms_request_cdma_delete_sms_on_ruim

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_CDMA_DELETE_SMS_ON_RUIM.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_cdma_delete_sms_on_ruim
(
  std::shared_ptr<RilRequestCdmaDeleteSmsOnRuimMessage> msg
)
{
  uint32 user_data;
  bool inserted_to_pendinglist = false;
  wms_delete_req_msg_v01 request_msg;
  RIL_Errno ril_err = RIL_E_SUCCESS;

  QCRIL_LOG_FUNC_ENTRY();

  do
  {
    // Add Message to the pending list, and get the token in case of success
    auto ret = getSmsModule().getPendingMessageList().insert(msg);
    if (!ret.second)
    {
        QCRIL_LOG_ERROR("Failed to add an entry for delete SMS on SIM.");
        ril_err = RIL_E_INTERNAL_ERR;
        break;
    }
    inserted_to_pendinglist = true;
    uint16_t token = ret.first;

    /* Fill in the QMI request */
    memset(&request_msg, 0, sizeof(wms_delete_req_msg_v01));
    request_msg.storage_type = WMS_STORAGE_TYPE_UIM_V01;

    request_msg.index_valid = TRUE;
    request_msg.index = static_cast<int>(msg->getIndex()) - 1;

    request_msg.message_mode_valid = TRUE;
    request_msg.message_mode = WMS_MESSAGE_MODE_CDMA_V01;

    user_data = QCRIL_COMPOSE_USER_DATA( QCRIL_DEFAULT_INSTANCE_ID,
            QCRIL_DEFAULT_MODEM_ID, token );

    if (qmi_client_sms_send_async(QMI_WMS_DELETE_REQ_V01,
                                  &request_msg,
                                  sizeof(wms_delete_req_msg_v01),
                                  sizeof(wms_delete_resp_msg_v01),
                                  qcril_qmi_sms_command_cb,
                                  (void *)(uintptr_t) user_data) != QMI_NO_ERR)
    {
      QCRIL_LOG_ERROR("Failed to get the response of deleting cdma sms on RUIM from qmi.");
      ril_err = RIL_E_SYSTEM_ERR;
    }

  } while(0);

  if(ril_err != RIL_E_SUCCESS)
  {
    if(inserted_to_pendinglist) {
        getSmsModule().getPendingMessageList().erase(msg);
    }
    auto resp = std::make_shared<RIL_Errno>(ril_err);
    msg->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
  }
  QCRIL_LOG_FUNC_RETURN();
} /* qcril_sms_request_cdma_delete_sms_on_ruim */


/*===========================================================================

  FUNCTION:  qcril_sms_request_cdma_get_broadcast_sms_config

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_cdma_get_broadcast_sms_config
(
  std::shared_ptr<RilRequestGetCdmaBroadcastConfigMessage> msg
)
{
  uint32 user_data;
  bool inserted_to_pendinglist = false;
  wms_get_broadcast_config_req_msg_v01 request_msg;
  int is_wms_limited_status = FALSE;
  RIL_Errno ril_err = RIL_E_SUCCESS;

  QCRIL_LOG_FUNC_ENTRY();

  do
  {
    is_wms_limited_status = get_wms_limited_service_status();

    if (is_wms_limited_status && !wms_ready_full_3gpp2())
    {
       ril_err = RIL_E_INVALID_STATE;
       break;
    }

    // Add Message to the pending list, and get the token in case of success
    auto res = getSmsModule().getPendingMessageList().insert(msg);
    if (!res.second)
    {
        ril_err = RIL_E_INTERNAL_ERR;
        break;
    }
    inserted_to_pendinglist = true;
    uint16_t token = res.first;

    /* Fill in the QMI request */
    memset(&request_msg, 0, sizeof(wms_get_broadcast_config_req_msg_v01));
    request_msg.message_mode = WMS_MESSAGE_MODE_CDMA_V01;

    user_data = QCRIL_COMPOSE_USER_DATA(QCRIL_DEFAULT_INSTANCE_ID,
            QCRIL_DEFAULT_MODEM_ID, token);

    if (qmi_client_sms_send_async(QMI_WMS_GET_BROADCAST_CONFIG_REQ_V01,
                                  &request_msg,
                                  sizeof(wms_get_broadcast_config_req_msg_v01),
                                  sizeof(wms_get_broadcast_config_resp_msg_v01),
                                  qcril_qmi_sms_command_cb,
                                  (void *)(uintptr_t) user_data) != QMI_NO_ERR)
    {
      QCRIL_LOG_ERROR("Failed to get the response of cdma broadcast sms config from qmi.");
      ril_err = RIL_E_SYSTEM_ERR;
      break;
    }

  } while(0);

  if(ril_err != RIL_E_SUCCESS)
  {
    if(inserted_to_pendinglist) {
        getSmsModule().getPendingMessageList().erase(msg);
    }
    auto resp = std::make_shared<RilGetCdmaBroadcastConfigResult_t>(ril_err);
    msg->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
  }
  QCRIL_LOG_FUNC_RETURN();
} /* qcril_sms_request_cdma_get_broadcast_sms_config() */


/*===========================================================================

  FUNCTION:  qcril_sms_request_cdma_set_broadcast_sms_config

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_CDMA_SET_BROADCAST_SMS_CONFIG.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_cdma_set_broadcast_sms_config
(
  std::shared_ptr<RilRequestCdmaSetBroadcastSmsConfigMessage> msg
)
{
  uint32 user_data;
  uint8 bc_table_size;
  wms_set_broadcast_config_req_msg_v01 request_msg;
  uint32 i;
  int is_wms_limited_status = FALSE;
  bool inserted_to_pendinglist = false;
  RIL_Errno ril_err = RIL_E_SUCCESS;

  QCRIL_LOG_FUNC_ENTRY();

  do
  {
    is_wms_limited_status = get_wms_limited_service_status();

    if (is_wms_limited_status && !wms_ready_full_3gpp2())
    {
       ril_err = RIL_E_INVALID_STATE;
       break;
    }

    /* Check if the broadcast SMS table is too big */
    bc_table_size = msg->getConfigList().size();
    if ( bc_table_size > WMS_3GPP2_BROADCAST_CONFIG_MAX_V01 ) // use QMI size
    {
      QCRIL_LOG_ERROR( "Size of CDMA Broadcast SMS Config table is too big : %d", bc_table_size );
      ril_err = RIL_E_INVALID_ARGUMENTS;
      break;
    }

    // Add Message to the pending list, and get the token in case of success
    auto res = getSmsModule().getPendingMessageList().insert(msg);
    if (!res.second)
    {
        ril_err = RIL_E_INTERNAL_ERR;
        break;
    }
    inserted_to_pendinglist = true;
    uint16_t token = res.first;

    /* Fill in the QMI request */
    memset(&request_msg, 0, sizeof(wms_set_broadcast_config_req_msg_v01));
    request_msg.message_mode = WMS_MESSAGE_MODE_CDMA_V01;
    request_msg.wms_3gpp2_broadcast_config_info_valid = TRUE;
    request_msg.wms_3gpp2_broadcast_config_info_len = bc_table_size;

    auto& inputData = msg->getConfigList();
    for ( i = 0; i < bc_table_size; i++ )
    {
      request_msg.wms_3gpp2_broadcast_config_info[i].service_category =
            static_cast<wms_service_category_enum_v01>(inputData[i].service_category);
      request_msg.wms_3gpp2_broadcast_config_info[i].language =
            static_cast<wms_language_enum_v01>(inputData[i].language);
      request_msg.wms_3gpp2_broadcast_config_info[i].selected =
            inputData[i].selected;
    }

    user_data = QCRIL_COMPOSE_USER_DATA(QCRIL_DEFAULT_INSTANCE_ID,
            QCRIL_DEFAULT_MODEM_ID, token);

    if (qmi_client_sms_send_async(QMI_WMS_SET_BROADCAST_CONFIG_REQ_V01,
                                  &request_msg,
                                  sizeof(wms_set_broadcast_config_req_msg_v01),
                                  sizeof(wms_set_broadcast_config_resp_msg_v01),
                                  qcril_qmi_sms_command_cb,
                                  (void *)(uintptr_t) user_data) != QMI_NO_ERR)
    {
      QCRIL_LOG_ERROR("Failed to get the response of setting cdma broadcast sms config from qmi.");
      ril_err = RIL_E_SYSTEM_ERR;
      break;
    }

  } while(0);

  if(ril_err != RIL_E_SUCCESS)
  {
    if(inserted_to_pendinglist) {
        getSmsModule().getPendingMessageList().erase(msg);
    }
    auto resp = std::make_shared<RIL_Errno>(ril_err);
    msg->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
  }

  QCRIL_LOG_FUNC_RETURN();
} /* qcril_sms_request_cdma_set_broadcast_sms_config */


/*===========================================================================

  FUNCTION:  qcril_sms_request_cdma_sms_broadcast_activation

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_CDMA_SMS_BROADCAST_ACTIVATION.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_cdma_sms_broadcast_activation
(
  std::shared_ptr<RilRequestCdmaSmsBroadcastActivateMessage> msg
)
{
  uint32 user_data;
  bool inserted_to_pendinglist = false;
  wms_set_broadcast_activation_req_msg_v01 request_msg;
  int is_wms_limited_status = FALSE;
  RIL_Errno ril_err = RIL_E_SUCCESS;

  QCRIL_LOG_FUNC_ENTRY();

  do
  {
    is_wms_limited_status = get_wms_limited_service_status();

    if (is_wms_limited_status && !wms_ready_full_3gpp2())
    {
       ril_err = RIL_E_INVALID_STATE;
       break;
    }

    // Add Message to the pending list, and get the token in case of success
    auto ret = getSmsModule().getPendingMessageList().insert(msg);
    if (!ret.second)
    {
        QCRIL_LOG_ERROR("Failed to add an entry for delete SMS on SIM.");
        ril_err = RIL_E_INTERNAL_ERR;
        break;
    }
    inserted_to_pendinglist = true;
    uint16_t token = ret.first;

    /* Fill in QMI message */
    memset(&request_msg, 0, sizeof(wms_set_broadcast_activation_req_msg_v01));
    request_msg.broadcast_activation_info.message_mode = WMS_MESSAGE_MODE_CDMA_V01;
    request_msg.broadcast_activation_info.bc_activate = static_cast<int>(msg->getActivateStatus());

    if( feature_all_bc_msg_3gpp2 )
    {
        request_msg.activate_all_valid = TRUE;
        request_msg.activate_all = 0x01;
    }

    user_data = QCRIL_COMPOSE_USER_DATA(QCRIL_DEFAULT_INSTANCE_ID,
            QCRIL_DEFAULT_MODEM_ID, token);

    if (qmi_client_sms_send_async(QMI_WMS_SET_BROADCAST_ACTIVATION_REQ_V01,
                                  &request_msg,
                                  sizeof(wms_set_broadcast_activation_req_msg_v01),
                                  sizeof(wms_set_broadcast_activation_resp_msg_v01),
                                  qcril_qmi_sms_command_cb,
                                  (void *)(uintptr_t) user_data) != QMI_NO_ERR)
    {
      QCRIL_LOG_ERROR("Failed to get the response of cdma sms broadcast activation from qmi.");
      ril_err = RIL_E_SYSTEM_ERR;
    }

  } while(0);

  if(ril_err != RIL_E_SUCCESS)
  {
    if(inserted_to_pendinglist) {
        getSmsModule().getPendingMessageList().erase(msg);
    }
    auto resp = std::make_shared<RIL_Errno>(ril_err);
    msg->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
  }
  QCRIL_LOG_FUNC_RETURN();
} /* qcril_sms_request_cdma_sms_broadcast_activation() */


/*===========================================================================

  FUNCTION:  qcril_sms_request_ims_registration_state

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_IMS_REGISTRATION_STATE.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_ims_registration_state
(
  std::shared_ptr<RilRequestGetImsRegistrationMessage> msg
)
{

  QCRIL_LOG_FUNC_ENTRY();

  qmi_ril_sms_ims_reg_state ims_sms_state;
  SmsImsServiceStatusInd::SmsImsServiceStatusInfo smsImsServiceStatus;
  RIL_Errno res = qcril_qmi_sms_retrieve_ims_sms_registration_state(ims_sms_state, smsImsServiceStatus);

  auto resp = std::make_shared<RilGetImsRegistrationResult_t>(res);
  if (resp && (res == RIL_E_SUCCESS))
  {
      resp->isRegistered = !!ims_sms_state.is_ims_wms_reg;
      resp->ratFamily = (ims_sms_state.format == QMI_RIL_SMS_FORMAT_3GPP2) ?
            RADIO_TECH_3GPP2 : RADIO_TECH_3GPP;
  }
  msg->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);

  QCRIL_LOG_FUNC_RETURN();
} /* qcril_sms_request_ims_registration_state() */

/*===========================================================================

  FUNCTION:  qcril_sms_request_ims_send_sms

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_IMS_SEND_SMS.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_ims_send_sms
(
   std::shared_ptr<RilRequestImsSendSmsMessage> msg
)
{
   uint32 user_data;
   wms_raw_send_req_msg_v01 request_msg;
   boolean sms_payload_in_cdma_format;
   const char *smsc_address = NULL;
   const char *gw_pdu = NULL;
   nas_sms_status_enum_type_v01 lte_sms_status;
   uint8_t lte_sms_status_valid;
   uint8_t is_voice_over_lte = FALSE;
   bool inserted_to_pendinglist = false;
   RIL_Errno ril_err = RIL_E_SUCCESS;

   QCRIL_LOG_FUNC_ENTRY();

   do
   {
      qcril_qmi_nas_fetch_lte_sms_status( &lte_sms_status_valid, &lte_sms_status);
      is_voice_over_lte = qcril_qmi_nas_is_voice_over_lte_registered();
      auto tech = msg->getRadioTechFamily();
      bool retry = msg->isRetry();
#if 0
      // XXX: This is only needed for Legacy IMS SMS: do we still need to support it
      // Check if retry needs to be blocked
      if (TRUE == qcril_qmi_sms_block_retry_3pgg_sms_on_lte_only(tech, retry))
      {
          QCRIL_LOG_ERROR("Retry not possible");
          ril_err = RIL_E_INVALID_STATE;
          break;
      }
#endif
      // Check for WMS SERVICE READY
      if (tech == RADIO_TECH_3GPP)
      {
         if ( retry && (!wms_ready_atleast_limited_3gpp() &&
              !(is_voice_over_lte && lte_sms_status_valid && (NAS_SMS_STATUS_3GPP_V01 == lte_sms_status) ) )
            )
         {
            ril_err = RIL_E_INVALID_STATE;
            break;
         }
      }
      else if (tech == RADIO_TECH_3GPP2)
      {
         // tech is 3GPP2
         if ( retry && ( !wms_ready_atleast_limited_3gpp2() &&
              !(is_voice_over_lte && lte_sms_status_valid && (NAS_SMS_STATUS_1X_V01 == lte_sms_status)) )
            )
         {
            ril_err = RIL_E_INVALID_STATE;
            break;
         }
      }
      else
      {
         QCRIL_LOG_ERROR("unsupported radio tech %d", tech);
         ril_err = RIL_E_INVALID_STATE;
         break;
      }

      QCRIL_LOG_INFO("ims retry 3gpp %d, 3gpp2 %d, is_rety %d, tech %d",
                      feature_ims_3gpp_retry, feature_ims_3gpp2_retry, retry, tech);

      // reject retries on 3gpp if retry feature is false
      if (!feature_ims_3gpp_retry && retry
          && (tech == RADIO_TECH_3GPP) ) {
         QCRIL_LOG_INFO("Feature ims_retry_on_3gpp disabled - SMS REJECTED");
         ril_err = RIL_E_INVALID_STATE;
         break;
      }

      // reject retries on 3gpp2 if feature is false
      if (!feature_ims_3gpp2_retry && retry
          && (tech == RADIO_TECH_3GPP2) ) {
         QCRIL_LOG_INFO("Feature ims_retry_on_3gpp2 disabled - SMS REJECTED");
         ril_err = RIL_E_INVALID_STATE;
         break;
      }

      if (tech == RADIO_TECH_3GPP2)
      {
         QCRIL_LOG_INFO("sms tech is 3GPP2");
         sms_payload_in_cdma_format = TRUE;
      }
      else
      {
         // tech is 3GPP
         QCRIL_LOG_INFO("sms tech is 3GPP");

         sms_payload_in_cdma_format = FALSE;
         smsc_address = msg->getGsmSmscPdu().empty() ? nullptr : msg->getGsmSmscPdu().c_str();
         gw_pdu = msg->getGsmPdu().empty() ? nullptr : msg->getGsmPdu().c_str();
         QCRIL_LOG_INFO("smsc address %s, pdu %s", msg->getGsmSmscPdu().c_str(),
                msg->getGsmPdu().c_str());

         if ( !qcril_mo_sms_error_check( gw_pdu ) )
         {
            QCRIL_LOG_ERROR("qcril_mo_sms_error_check failed");
            ril_err = RIL_E_INVALID_ARGUMENTS;
            break;
         }
      }

      // Add Message to the pending list, and get the token in case of success
      auto res = getSmsModule().getPendingMessageList().insert(msg);
      if (!res.second)
      {
        ril_err = RIL_E_INTERNAL_ERR;
        break;
      }
      inserted_to_pendinglist = true;
      uint16_t token = res.first;

      /* Fill in the QMI request */
      memset(&request_msg, 0, sizeof(wms_raw_send_req_msg_v01));
      if (!qcril_sms_fill_wms_payload(
                       sms_payload_in_cdma_format,
                       (tech == RADIO_TECH_3GPP2) ? &msg->getCdmaPayload() : nullptr,
                       smsc_address,
                       gw_pdu,
                       0,
                       TRUE, /* hex string for gw_pdu */
                       &request_msg.raw_message_data.format,
                       (uint32 *)&request_msg.raw_message_data.raw_message_len,
                       &request_msg.raw_message_data.raw_message[0],
                       TRUE, //!retry,
                       TRUE ))
      {
         QCRIL_LOG_ERROR("Failed qcril_sms_fill_wms_payload");
         ril_err = RIL_E_INVALID_SMS_FORMAT;
         break;
      }

      if (retry)
      {
        int32_t messageRef = msg->getMessageReference();
        if(messageRef > 0)
        {
          QCRIL_LOG_INFO("sms msg is a retry...");
          request_msg.retry_message_valid = TRUE;
          request_msg.retry_message = WMS_MESSAGE_IS_A_RETRY_V01;

          request_msg.retry_message_id_valid = TRUE;
          request_msg.retry_message_id = messageRef;
        }
#if 0
        // XXX: Do we need to suppport Legacy IMS SMS behavior???
        request_msg.sms_on_ims_valid = TRUE;
        if (params_ptr->event_id == QCRIL_EVT_IMS_SOCKET_REQ_SEND_SMS)
        {
          /* For the new IMS SMS refactor, RETRY will still go through IMS,
           * while FALLBACK will go through CS using IRADIO interface */
          request_msg.sms_on_ims = TRUE;
        }
        else
        {
          /* Indicate the message must NOT go on IMS */
          request_msg.sms_on_ims = FALSE;
        }
#else
        request_msg.sms_on_ims_valid = TRUE;
        request_msg.sms_on_ims = TRUE;
#endif

        QCRIL_LOG_DEBUG( "IS FORCE ON DC = %d", is_force_on_dc );
        if((tech == RADIO_TECH_3GPP2)
            && (is_force_on_dc || qcril_sms_force_on_dc()))
        {
           request_msg.force_on_dc_valid = TRUE;
           request_msg.force_on_dc.force_on_dc = 0x01;
        }
      }
      else
      {
         /* Indicate the message MUST go on IMS */
         QCRIL_LOG_INFO("sending sms on ims");
         request_msg.sms_on_ims_valid = TRUE;
         request_msg.sms_on_ims = TRUE;
      }

      /* Compose the user data */
      user_data = QCRIL_COMPOSE_USER_DATA(QCRIL_DEFAULT_INSTANCE_ID,
            QCRIL_DEFAULT_MODEM_ID, token);

      int result = qmi_client_sms_send_async(QMI_WMS_RAW_SEND_REQ_V01,
                                             &request_msg,
                                             sizeof(wms_raw_send_req_msg_v01),
                                             sizeof(wms_raw_send_resp_msg_v01),
                                             qcril_qmi_sms_command_cb,
                                             (void *)(uintptr_t) user_data);
      if (result == QMI_NO_ERR)
      {
         QCRIL_LOG_INFO("Sent QMI_WMS_RAW_SEND_REQ_V01");
      }
      else
      {
         QCRIL_LOG_ERROR("Failed qcril_qmi_client_send_msg_async w/%d", result);
         ril_err = RIL_E_SYSTEM_ERR;
      }

   } while(0);

  if(ril_err != RIL_E_SUCCESS)
  {
    if(inserted_to_pendinglist) {
        getSmsModule().getPendingMessageList().erase(msg);
    }
    auto resp = std::make_shared<RilSendSmsResult_t>(ril_err);
    msg->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
  }

  QCRIL_LOG_FUNC_RETURN();
} /* qcril_sms_request_ims_send_sms() */

/*===========================================================================

  FUNCTION:  qcril_sms_request_ims_ack_sms

===========================================================================*/
/*!
    @brief
    Handles ims_MsgId_REQUEST_ACK_SMS
    No resp sent out to ATEL

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_ims_ack_sms
(
   std::shared_ptr<RilRequestAckImsSmsMessage> msg
)
{

   bool need_reset_ack_expry_tmr = false;
   bool inserted_to_pendinglist = false;
   qcril_instance_id_e_type instance_id = QCRIL_DEFAULT_INSTANCE_ID;
   qcril_sms_struct_type *i_ptr = &qcril_sms[ instance_id ];
   RIL_Errno ril_err = RIL_E_SUCCESS;

   char details[ 80 ];
   QCRIL_SNPRINTF( details, sizeof( details ), "qcril_sms[%d].sms_ack_info_mutex", instance_id );
   QCRIL_MUTEX_LOCK( &i_ptr->sms_ack_info_mutex, details );

   do {
    QCRIL_LOG_DEBUG("ack ref id : %d, gw pending %d, cdma pending %d \
                    gw ack needed: %d, cdma ack needed %d \
                    gw transcation id: %d, cdma transaction id: %d \
                    gw send ack on ims %d, cdma send ack on ims: %d",
                    msg->getMessageRef(),
                    i_ptr->sms_ack_info.gw_ack_pending,
                    i_ptr->sms_ack_info.cdma_ack_pending,
                    i_ptr->sms_ack_info.gw_ack_needed,
                    i_ptr->sms_ack_info.cdma_ack_needed,
                    i_ptr->sms_ack_info.gw_transaction_id,
                    i_ptr->sms_ack_info.cdma_transaction_id,
                    i_ptr->sms_ack_info.gw_send_ack_on_ims,
                    i_ptr->sms_ack_info.cdma_send_ack_on_ims);

    if (i_ptr->sms_ack_info.gw_send_ack_on_ims)
    {
        if (!i_ptr->sms_ack_info.gw_ack_pending)
        {
            QCRIL_LOG_ERROR("Unable to send ack; no messages to be acked.");
            ril_err = RIL_E_INVALID_STATE;
            break;
        }

        if (!i_ptr->sms_ack_info.gw_ack_needed)
        {
            QCRIL_LOG_INFO("Dropping ack since no ack is needed.");

            i_ptr->sms_ack_info.gw_ack_pending = FALSE;
            i_ptr->sms_ack_info.gw_send_ack_on_ims = FALSE;

            need_reset_ack_expry_tmr = true;
            ril_err = RIL_E_INVALID_STATE;
            break;
        }
    }
    else if (i_ptr->sms_ack_info.cdma_send_ack_on_ims)
    {
        if (!i_ptr->sms_ack_info.cdma_ack_pending)
        {
            QCRIL_LOG_ERROR("Unable to send ack; no messages to be acked." );
            ril_err = RIL_E_INVALID_STATE;
            break;
        }

        if (!i_ptr->sms_ack_info.cdma_ack_needed)
        {
            QCRIL_LOG_INFO("Dropping ack since no ack is needed.");

            i_ptr->sms_ack_info.cdma_ack_pending = FALSE;
            i_ptr->sms_ack_info.cdma_send_ack_on_ims = FALSE;

            need_reset_ack_expry_tmr = true;
            ril_err = RIL_E_INVALID_STATE;
            break;
        }
    }
    else
    {
        QCRIL_LOG_ERROR("There is no pending IMS SMS ack");
        ril_err = RIL_E_INVALID_STATE;
        break;
    }

    // Add Message to the pending list, and get the token in case of success
    auto res = getSmsModule().getPendingMessageList().insert(msg);
    if (!res.second)
    {
      ril_err = RIL_E_INTERNAL_ERR;
      break;
    }
    inserted_to_pendinglist = true;
    uint16_t token = res.first;

    /* Fill in the QMI request */
    wms_send_ack_req_msg_v01 request_msg;
    memset(&request_msg, 0, sizeof(wms_send_ack_req_msg_v01));
    request_msg.sms_on_ims_valid = TRUE;
    request_msg.sms_on_ims = TRUE;
    if (i_ptr->sms_ack_info.gw_send_ack_on_ims)
    {
        request_msg.ack_information.transaction_id = i_ptr->sms_ack_info.gw_transaction_id;
        request_msg.ack_information.message_protocol = WMS_MESSAGE_PROTOCOL_WCDMA_V01;
    }
    else
    {
        request_msg.ack_information.transaction_id = i_ptr->sms_ack_info.cdma_transaction_id;
        request_msg.ack_information.message_protocol = WMS_MESSAGE_PROTOCOL_CDMA_V01;
    }

    auto delivery_status = msg->getDeliverStatus();
    request_msg.ack_information.success =
        (delivery_status == qcril::interfaces::DeliverStatus::STATUS_OK) ?
        0x01 : 0x00;
    if (request_msg.ack_information.success != 0x01)
    {
        if (i_ptr->sms_ack_info.gw_send_ack_on_ims)
        {
            request_msg.wms_3gpp_failure_information_valid = TRUE;
            switch (delivery_status) {
            case qcril::interfaces::DeliverStatus::STATUS_NO_MEMORY:
                request_msg.wms_3gpp_failure_information.rp_cause =
                        WMS_RP_CAUSE_MEMORY_CAP_EXCEEDED_ACK_V01;
                request_msg.wms_3gpp_failure_information.tp_cause =
                        WMS_TP_CAUSE_MEMORY_CAP_EXCEEDED_V01;
                break;
            default:
                request_msg.wms_3gpp_failure_information.rp_cause =
                        WMS_RP_CAUSE_PROTOCOL_ERROR_ACK_V01;
                request_msg.wms_3gpp_failure_information.tp_cause =
                        WMS_TP_CAUSE_UNSPECIFIED_ERROR_V01;
            }
        }
        else
        {
            request_msg.wms_3gpp2_failure_information_valid = TRUE;
            request_msg.wms_3gpp2_failure_information.error_class =
                    WMS_ERROR_CLASS_3GPP2_FAILURE_TEMPORARY_V01;
            switch (delivery_status) {
            case qcril::interfaces::DeliverStatus::STATUS_NO_MEMORY:
                request_msg.wms_3gpp2_failure_information.tl_status =
                        WMS_TL_CAUSE_CODE_DEST_RESOURCE_SHORTAGE_ACK_V01;
                break;
            case qcril::interfaces::DeliverStatus::STATUS_REQUEST_NOT_SUPPORTED:
                request_msg.wms_3gpp2_failure_information.tl_status =
                        WMS_TL_CAUSE_CODE_INVALID_TELESERVICE_ID_ACK_V01;
                break;
            default:
                request_msg.wms_3gpp2_failure_information.tl_status =
                        WMS_TL_CAUSE_CODE_DEST_OTHER_ACK_V01;
            }
        }
    }

    uint32_t user_data = QCRIL_COMPOSE_USER_DATA(instance_id, QCRIL_DEFAULT_MODEM_ID, token);
    if (qmi_client_sms_send_async(QMI_WMS_SEND_ACK_REQ_V01,
                                  &request_msg,
                                  sizeof(wms_send_ack_req_msg_v01),
                                  sizeof(wms_send_ack_resp_msg_v01),
                                  qcril_qmi_sms_command_cb,
                                  (void *)(uintptr_t) user_data) != QMI_NO_ERR)
    {
      QCRIL_LOG_ERROR("Failed to get the response message from qmi for acknowledgement request.");
      break;
    }

    need_reset_ack_expry_tmr = true;

  } while(0);

  /* Reset the ack pending flag ALWAYS if this handler is ever called, since ATEL
     has no retry logic for acknowledgement. If the ack is not received by the
     network, the network will resend the SMS, and RIL will have another chance
     to ack it. */
  if (i_ptr->sms_ack_info.gw_send_ack_on_ims)
  {
      i_ptr->sms_ack_info.gw_ack_pending = FALSE;
      i_ptr->sms_ack_info.gw_ack_needed = FALSE;
      i_ptr->sms_ack_info.gw_send_ack_on_ims = FALSE;
      i_ptr->sms_ack_info.gw_transaction_id = (uint32_t)(-1);
  }
  else
  {
      i_ptr->sms_ack_info.cdma_ack_pending = FALSE;
      i_ptr->sms_ack_info.cdma_ack_needed = FALSE;
      i_ptr->sms_ack_info.cdma_send_ack_on_ims = FALSE;
      i_ptr->sms_ack_info.cdma_transaction_id = (uint32_t)(-1);
  }

  if ( need_reset_ack_expry_tmr )
  {
      if ( QMI_RIL_ZERO != qcril_sms[QCRIL_DEFAULT_INSTANCE_ID].mt_pending_ack_expry_tmr )
      {
         QCRIL_LOG_INFO( "resetting MT SMS ack tmr per ack reception" );
         qcril_cancel_timed_callback( (void*)(uintptr_t)qcril_sms[QCRIL_DEFAULT_INSTANCE_ID].mt_pending_ack_expry_tmr );
         qcril_sms[QCRIL_DEFAULT_INSTANCE_ID].mt_pending_ack_expry_tmr = QMI_RIL_ZERO;
      }
  }

  QCRIL_SNPRINTF( details, sizeof( details ),
                  "qcril_sms[%d].sms_ack_info_mutex", instance_id );
  QCRIL_MUTEX_UNLOCK( &i_ptr->sms_ack_info_mutex, details );

  if (ril_err != RIL_E_SUCCESS)
  {
    if(inserted_to_pendinglist) {
        getSmsModule().getPendingMessageList().erase(msg);
    }
    auto resp = std::make_shared<RIL_Errno>(ril_err);
    msg->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
  }

  QCRIL_LOG_FUNC_RETURN();
}

/*===========================================================================

                   QMI COMMAND CALLBACK HANDLER

===========================================================================*/

/*=========================================================================
  FUNCTION:  qcril_qmi_sms_command_cb

===========================================================================*/
/*!
    @brief
    Common Callback for all the QMI voice commands.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_sms_command_cb
(
   unsigned int                 msg_id,
   std::shared_ptr<void>        resp_c_struct,
   unsigned int                 resp_c_struct_len,
   void                        *resp_cb_data,
   qmi_client_error_type        transp_err
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  uint16 req_id;
  /*-----------------------------------------------------------------------*/

  // resp_cb_data is not a pointer, do not check for NULL
  QCRIL_ASSERT( resp_c_struct != NULL );
  instance_id = (qcril_instance_id_e_type) QCRIL_EXTRACT_INSTANCE_ID_FROM_USER_DATA( (uint32)(uintptr_t) resp_cb_data );
  modem_id = (qcril_modem_id_e_type) QCRIL_EXTRACT_MODEM_ID_FROM_USER_DATA( (uint32)(uintptr_t) resp_cb_data );
  req_id = QCRIL_EXTRACT_USER_ID_FROM_USER_DATA( (uint32)(uintptr_t) resp_cb_data );
  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_INFO("msg_id (0x%x) %s", msg_id, qcril_sms_lookup_cmd_name(msg_id));
  auto msg = getSmsModule().getPendingMessageList().extract(req_id);
  if (msg == nullptr)
  {
    QCRIL_LOG_ERROR("token: %d not found in SMS pending list", req_id);
  }
  else
  {
    switch (msg_id) {
    case QMI_WMS_RAW_SEND_REQ_V01:
        QCRIL_ASSERT(resp_c_struct_len == sizeof(wms_raw_send_resp_msg_v01));
        qcril_mo_sms_cb(msg, transp_err,
                static_cast<wms_raw_send_resp_msg_v01*>(resp_c_struct.get()));
        break;
    case QMI_WMS_SEND_ACK_REQ_V01:
        QCRIL_ASSERT(resp_c_struct_len == sizeof(wms_send_ack_resp_msg_v01));
        qcril_ack_cb(msg, transp_err,
                static_cast<wms_send_ack_resp_msg_v01*>(resp_c_struct.get()));
        break;
    case QMI_WMS_DELETE_REQ_V01:
        QCRIL_ASSERT(resp_c_struct_len == sizeof(wms_delete_resp_msg_v01));
        qcril_delete_cb(msg, transp_err,
                static_cast<wms_delete_resp_msg_v01*>(resp_c_struct.get()));
        break;
    case QMI_WMS_GET_SMSC_ADDRESS_REQ_V01:
        QCRIL_ASSERT(resp_c_struct_len == sizeof(wms_get_smsc_address_resp_msg_v01));
        qcril_get_smsc_addr_cb(msg, transp_err,
                static_cast<wms_get_smsc_address_resp_msg_v01*>(resp_c_struct.get()));
        break;
    case QMI_WMS_SET_SMSC_ADDRESS_REQ_V01:
        QCRIL_ASSERT(resp_c_struct_len == sizeof(wms_set_smsc_address_resp_msg_v01));
        qcril_set_smsc_addr_cb(msg, transp_err,
                static_cast<wms_set_smsc_address_resp_msg_v01*>(resp_c_struct.get()));
        break;
    case QMI_WMS_GET_BROADCAST_CONFIG_REQ_V01:
        QCRIL_ASSERT(resp_c_struct_len == sizeof(wms_get_broadcast_config_resp_msg_v01));
        qcril_get_bc_config_cb(msg, transp_err,
                static_cast<wms_get_broadcast_config_resp_msg_v01*>(resp_c_struct.get()));
        break;
    case QMI_WMS_SET_BROADCAST_CONFIG_REQ_V01:
        QCRIL_ASSERT(resp_c_struct_len == sizeof(wms_set_broadcast_config_resp_msg_v01));
        qcril_set_bc_config_cb(msg, transp_err,
                static_cast<wms_set_broadcast_config_resp_msg_v01*>(resp_c_struct.get()));
        break;
    case QMI_WMS_SET_BROADCAST_ACTIVATION_REQ_V01:
        QCRIL_ASSERT(resp_c_struct_len == sizeof(wms_set_broadcast_activation_resp_msg_v01));
        qcril_bc_activate_cb(msg, transp_err,
                static_cast<wms_set_broadcast_activation_resp_msg_v01*>(resp_c_struct.get()));
        break;
    case QMI_WMS_SET_MEMORY_STATUS_REQ_V01:
        QCRIL_ASSERT(resp_c_struct_len == sizeof(wms_set_memory_status_resp_msg_v01));
        qcril_report_mem_status_cb(msg, transp_err,
                static_cast<wms_set_memory_status_resp_msg_v01*>(resp_c_struct.get()));
        break;
    default:
        QCRIL_LOG_INFO("Unsupported QMI WMS message 0x%x", msg_id);
        break;
    }
  }

  QCRIL_LOG_FUNC_RETURN();
}/* qcril_qmi_sms_command_cb */


/*===========================================================================

                   INDICATION PROCESSING FUNCTIONS

===========================================================================*/
/*=========================================================================
  FUNCTION:  qcril_sms_decode_ind

===========================================================================*/
/*!
    @brief
    Decodes a QMI indication

    @return
    If successful, payload of QMI indication
    NULL otherwise.
*/
/*=========================================================================*/
void * qcril_sms_decode_ind
(
  unsigned long      msg_id,
  unsigned char      *ind_buf,
  int                ind_buf_len
)
{
  uint32 decoded_payload_len = 0;
  boolean success = TRUE;
  qmi_client_error_type qmi_err;
  void * decoded_payload = NULL;

  QCRIL_LOG_FUNC_ENTRY();
  qmi_err = qmi_idl_get_message_c_struct_len(qcril_qmi_client_get_service_object(QCRIL_QMI_CLIENT_WMS),
                                             QMI_IDL_INDICATION,
                                             msg_id,
                                             (uint32_t *) &decoded_payload_len);

  if (qmi_err != QMI_NO_ERR)
  {
    QCRIL_LOG_ERROR("Error %d getting message struct len for QMI WMS Indication: %d", qmi_err, msg_id);
    success = FALSE;
  }

  if (success)
  {
    decoded_payload = qcril_malloc(decoded_payload_len);

    if (decoded_payload == NULL)
    {
      QCRIL_LOG_ERROR("Failed to allocate buffer for QMI WMS Indication: %d", msg_id);
      success = FALSE;
    }
  }

  if (success)
  {
    memset(decoded_payload, 0, decoded_payload_len);

    qmi_err = qmi_client_message_decode(qcril_qmi_client_get_user_handle(QCRIL_QMI_CLIENT_WMS),
                                        QMI_IDL_INDICATION,
                                        msg_id,
                                        ind_buf,
                                        ind_buf_len,
                                        decoded_payload,
                                        (int)decoded_payload_len);

    if (qmi_err != QMI_NO_ERR)
    {
      QCRIL_LOG_ERROR("Error %d decoding QMI WMS Indication: %d", qmi_err, msg_id);
      success = FALSE;
    }
  }

  if(!success && decoded_payload != NULL)
    {
      qcril_free(decoded_payload);
    decoded_payload = NULL;
    }

  QCRIL_LOG_FUNC_RETURN();
  return decoded_payload;

} /* qcril_sms_decode_ind */


/*=========================================================================
  FUNCTION:  qcril_sms_check_if_gw_msg_is_status_report

===========================================================================*/
/*!
    @brief
    Determine if the GW SMS message is a Status Report.

    @return
    TRUE if the GW SMS message is a Status Report; FALSE otherwise
*/
/*=========================================================================*/
boolean qcril_sms_check_if_gw_msg_is_status_report
(
  wms_transfer_route_mt_message_type_v01 * mt_sms,
  uint8_t& message_ref
)
{
  boolean is_status_report = FALSE;

  QCRIL_LOG_FUNC_ENTRY();
  /* The first two bits of the first byte of the payload indicates the
     TP-MTI (Message Type Indicator).  A value of 2 means it is a Status
     Report. */
  if ((mt_sms->data[0] & 0x03) ==  2)
  {
    is_status_report = TRUE;
    message_ref = mt_sms->data[1];
  }

  QCRIL_LOG_FUNC_RETURN_WITH_RET(is_status_report);
  return (is_status_report);
} /* qcril_sms_check_if_gw_msg_is_status_report */

 /*=========================================================================
   FUNCTION:  qcril_sms_get_cdma_message_type_and_ref

 ===========================================================================*/
 /*!
     @brief
     get the message type and message ref

     @reference
     3GPP2 C.S0015-B - 4.5 Bearer Data Subparameters and 4.5.1 Message
     Identifier

     @return
     TRUE if it is sucessful to get the message_type and message_reference
     FALSE otherwise
 */
 /*=========================================================================*/
boolean qcril_sms_get_cdma_message_type_and_ref
(
  unsigned char* bearer_data,
  uint32_t data_len,
  uint8_t &message_type,
  uint32_t &message_ref
)
{
  // find where message identifier is, and store the position into index
  // if it is not found, the index will be equal or greater than data_len
  uint32_t index = 0;
  while (index < data_len)
  {
    uint8_t id = bearer_data[index];
    if (id == 0) break;
    if (++index >= data_len) return FALSE;
    uint8_t len = bearer_data[index];
    // skip the remaining data
    index += (len + 1);
  }

  // exact the message type and message reference
  if (++index >= data_len) return FALSE;
  uint8_t len = bearer_data[index++];
  if (len >= 3 && index + len <= data_len)
  {
    message_type = b_unpackb(bearer_data, index*8, 4);
    message_ref = b_unpackw(bearer_data, index*8+4, 16);
    return TRUE;
  }

  return FALSE;
}

boolean qcril_sms_check_if_cdma_msg_is_status_report
(
  int teleservice_id,
  unsigned char *bearer_data,
  uint32_t data_len,
  uint32_t &message_ref
)
{
  if (teleservice_id != QCRIL_SMS_TELESERVICE_WMT &&
        teleservice_id != QCRIL_SMS_TELESERVICE_WEMT)
    return FALSE;

  uint8_t message_type = 0;
  (void)qcril_sms_get_cdma_message_type_and_ref(bearer_data, data_len,
    message_type, message_ref);

  return (message_type == QCRIL_3GPP2_SMS_DELIVERY_ACK);
}

/*=========================================================================
  FUNCTION:  qcril_sms_convert_mt_cdma_sms_to_RIL_format

===========================================================================*/
/*!
    @brief
    Translates a CDMA SMS message from QMI format to RIL format.

    @return
    TRUE if the MT CDMA SMS was successfully converted to RIL format
    FALSE otherwise.
*/
/*=========================================================================*/
boolean qcril_sms_convert_mt_cdma_sms_to_RIL_format
(
  wms_transfer_route_mt_message_type_v01 * mt_sms,
  RIL_CDMA_SMS_Message *RIL_sms_message
)
{
  boolean success = FALSE;

  QCRIL_LOG_FUNC_ENTRY();
  if(mt_sms != NULL && RIL_sms_message != NULL)
  {

  success = qcril_sms_convert_mt_sms_qmi_to_ril(mt_sms->data,
                                                mt_sms->data_len,
                                                RIL_sms_message);
  }

  if (success == FALSE)
  {
    QCRIL_LOG_ERROR("Failed to convert MT CDMA SMS to RIL format");
  }

  QCRIL_LOG_FUNC_RETURN_WITH_RET(success);
  return(success);
} /* qcril_sms_convert_mt_cdma_sms_to_RIL_format */


/*=========================================================================
  FUNCTION:  qcril_sms_process_mt_cdma_sms

===========================================================================*/
/*!
    @brief
    Performs processing necessary when an MT CDMA SMS message is received.

    @return
    None
*/
/*=========================================================================*/
void qcril_sms_process_mt_cdma_sms
(
  qcril_instance_id_e_type instance_id,
  wms_event_report_ind_msg_v01 * event_report_ind,
  wms_message_format_enum_v01 format
)
{
  qcril_sms_struct_type *i_ptr;
  boolean is_processing_ok = TRUE;
  char *buf_ptr;
  char details[ 80 ];
  RIL_CDMA_SMS_Message * cdma_sms_msg;

  struct timeval ack_expry_tmr_window = { 30 , 0 }; // 30 seconds

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_FUNC_ENTRY();
  i_ptr = &qcril_sms[ instance_id ];


  /*-----------------------------------------------------------------------*/

  QCRIL_SNPRINTF( details, sizeof( details ), "qcril_sms[%d].sms_ack_info_mutex", instance_id );
  QCRIL_MUTEX_LOCK( &i_ptr->sms_ack_info_mutex, details );

  /*-----------------------------------------------------------------------*/

  /* Allocate buffer to decode new CDMA SMS */
  buf_ptr = (char*) qcril_malloc( sizeof( RIL_CDMA_SMS_Message ) );
  if ( buf_ptr == NULL )
  {
    QCRIL_LOG_ERROR("Failed to allocate buffer to decode new CDMA SMS.");
  }
  else
  {
    /* According to ril.h, we are to drop new SMS Messages until the previous
       message has been acked.  Note that the network will not send any new
       messages until the previous one has been acked; so the messages we are
       dropping here should all be duplicates.

       We only drop new SMS messages if an ack is needed.  There are some
       messages in the category of "no ack needed" which need to be sent to
       Android immediately.  For example, the segments of a multi-page BC SMS
       message are received too quickly for Android to ack them; waiting for
       an ack from Android will lead to segments being dropped.
    */
    if (i_ptr->sms_ack_info.cdma_ack_pending &&
                    i_ptr->sms_ack_info.cdma_ack_needed)
    {
      QCRIL_LOG_DEBUG("Ignoring CDMA SMS Message. Waiting for RIL to ack a previous SMS.");
      is_processing_ok = FALSE;
    }

    if (is_processing_ok)
    {
      if (format == WMS_MESSAGE_FORMAT_CDMA_V01)
      {
        /* Translate the CDMA SMS message from QMI format to RIL format. */
        if (qcril_sms_convert_mt_cdma_sms_to_RIL_format( &event_report_ind->transfer_route_mt_message,
                                                         (RIL_CDMA_SMS_Message *) buf_ptr ) == FALSE)
        {
          QCRIL_LOG_DEBUG("Failed to convert mt cdma sms to RIL format.");
          is_processing_ok = FALSE;
        }
      }
      else if (format == WMS_MESSAGE_FORMAT_MWI_V01)
      {
        cdma_sms_msg = (RIL_CDMA_SMS_Message *) buf_ptr;
        cdma_sms_msg->uTeleserviceID = 0x00040000;
        cdma_sms_msg->bIsServicePresent = FALSE;
        cdma_sms_msg->sAddress.number_of_digits = 0;
        cdma_sms_msg->sSubAddress.number_of_digits = 0;
        cdma_sms_msg->uBearerDataLen = 1;
        cdma_sms_msg->aBearerData[0] = event_report_ind->transfer_route_mt_message.data[0];
      }
    }

    if (is_processing_ok)
    {
      /* An MT SMS has been received.  An ack is pending from Android. */
      i_ptr->sms_ack_info.cdma_ack_pending = TRUE;

      /* Ack_indicator of 0x01 means no ack is needed. If no ack is needed, QCRIL will
         drop the ack when it is received from the Android SMS application. */
      i_ptr->sms_ack_info.cdma_ack_needed = !event_report_ind->transfer_route_mt_message.ack_indicator;

      i_ptr->sms_ack_info.cdma_transaction_id = event_report_ind->transfer_route_mt_message.transaction_id;

      if (event_report_ind->sms_on_ims_valid &&
          event_report_ind->sms_on_ims)
      {
        i_ptr->sms_ack_info.cdma_send_ack_on_ims = TRUE;
      }
      else
      {
        i_ptr->sms_ack_info.cdma_send_ack_on_ims = FALSE;
      }

      // ack expiry tmr
      if ( QMI_RIL_ZERO != qcril_sms[QCRIL_DEFAULT_INSTANCE_ID].mt_pending_ack_expry_tmr )
      {
        qcril_cancel_timed_callback( (void*)(uintptr_t)qcril_sms[QCRIL_DEFAULT_INSTANCE_ID].mt_pending_ack_expry_tmr );
        qcril_sms[QCRIL_DEFAULT_INSTANCE_ID].mt_pending_ack_expry_tmr = QMI_RIL_ZERO;
      }

      QCRIL_LOG_DEBUG("Considering timeout for ack/nack for MT SMS, enabled %d, timer window %d",
                      qmi_ril_sms_mt_expiry_window_len_set,
                      qmi_ril_sms_mt_expiry_window_len);
      if ( qmi_ril_sms_mt_expiry_window_len_set )
      {
        ack_expry_tmr_window.tv_sec  = qmi_ril_sms_mt_expiry_window_len;
        ack_expry_tmr_window.tv_usec = QMI_RIL_ZERO;

        qcril_setup_timed_callback(   QCRIL_DEFAULT_INSTANCE_ID,
                                      QCRIL_DEFAULT_MODEM_ID,
                                      qcril_sms_mt_transaction_expired,
                                      &ack_expry_tmr_window,
                                      &qcril_sms[QCRIL_DEFAULT_INSTANCE_ID].mt_pending_ack_expry_tmr );
      }

      uint32_t message_ref;
      RIL_CDMA_SMS_Message& cdma_msg = *(reinterpret_cast<RIL_CDMA_SMS_Message*>(buf_ptr));
      if (event_report_ind->sms_on_ims_valid && event_report_ind->sms_on_ims &&
              qcril_sms_check_if_cdma_msg_is_status_report(cdma_msg.uTeleserviceID, cdma_msg.aBearerData,
              RIL_CDMA_SMS_BEARER_DATA_MAX, message_ref))
      {
        auto msg = std::make_shared<RilUnsolNewImsSmsStatusReportMessage>(
              message_ref, RADIO_TECH_3GPP2);
        if (msg)
        {
            msg->setCdmaPayload(cdma_msg);
            Dispatcher::getInstance().dispatch(msg);
        }
      }
      else
      {
        if (event_report_ind->sms_on_ims_valid && event_report_ind->sms_on_ims)
        {
          auto mt_ims_msg = std::make_shared<RilUnsolIncomingImsSMSMessage>(RADIO_TECH_3GPP2);
          if (mt_ims_msg)
          {
              mt_ims_msg->setCdmaPayload(cdma_msg);
              Dispatcher::getInstance().dispatch(mt_ims_msg);
          }
        }
        else
        {
          auto mt_sms_msg = std::make_shared<RilUnsolIncoming3Gpp2SMSMessage>(cdma_msg);
          Dispatcher::getInstance().dispatch(mt_sms_msg);
        }
      }
    }
    qcril_free( buf_ptr );
  }

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_UNLOCK( &i_ptr->sms_ack_info_mutex, details );
  QCRIL_LOG_FUNC_RETURN();

} /* qcril_sms_process_mt_cdma_sms */

/*=========================================================================
  FUNCTION:  qcril_sms_mt_transaction_expired

===========================================================================*/
/*!
    @brief
    Performs MT SMS transaction cleanup when ATEL posts no
    ack or nack within prespecified window after CDMA MT SMS

    @return
    None
*/
/*=========================================================================*/

void qcril_sms_mt_transaction_expired(void * param)
{
    QCRIL_LOG_FUNC_ENTRY();
    QCRIL_NOTUSED( param );

    QCRIL_MUTEX_LOCK( &qcril_sms[QCRIL_DEFAULT_INSTANCE_ID].sms_ack_info_mutex, "sms_ack_info_mutex" );

    QCRIL_LOG_INFO( "current CDMA ack pending %d, needed %d",
                    qcril_sms[QCRIL_DEFAULT_INSTANCE_ID].sms_ack_info.cdma_ack_pending,
                    qcril_sms[QCRIL_DEFAULT_INSTANCE_ID].sms_ack_info.cdma_ack_needed  );

    QCRIL_LOG_INFO( "current GW ack pending %d, needed %d",
                    qcril_sms[QCRIL_DEFAULT_INSTANCE_ID].sms_ack_info.gw_ack_pending,
                    qcril_sms[QCRIL_DEFAULT_INSTANCE_ID].sms_ack_info.gw_ack_needed  );

    qcril_sms[QCRIL_DEFAULT_INSTANCE_ID].sms_ack_info.cdma_ack_pending     = FALSE;
    qcril_sms[QCRIL_DEFAULT_INSTANCE_ID].sms_ack_info.cdma_ack_needed      = FALSE;
    qcril_sms[QCRIL_DEFAULT_INSTANCE_ID].sms_ack_info.cdma_send_ack_on_ims = FALSE;

    qcril_sms[QCRIL_DEFAULT_INSTANCE_ID].sms_ack_info.gw_ack_pending     = FALSE;
    qcril_sms[QCRIL_DEFAULT_INSTANCE_ID].sms_ack_info.gw_ack_needed      = FALSE;
    qcril_sms[QCRIL_DEFAULT_INSTANCE_ID].sms_ack_info.gw_send_ack_on_ims = FALSE;

    qcril_sms[QCRIL_DEFAULT_INSTANCE_ID].mt_pending_ack_expry_tmr     = QMI_RIL_ZERO;

    QCRIL_MUTEX_UNLOCK( &qcril_sms[QCRIL_DEFAULT_INSTANCE_ID].sms_ack_info_mutex, "sms_ack_info_mutex" );

    QCRIL_LOG_FUNC_RETURN();
} /* qcril_sms_mt_transaction_expired */

/*=========================================================================
  FUNCTION:  qcril_sms_convert_mt_gw_sms_to_RIL_format

===========================================================================*/
/*!
    @brief
    Translates a GW SMS message from QMI format to RIL format.

    @return
    TRUE if the MT GW SMS was successfully converted to RIL format
    FALSE otherwise.
*/
/*=========================================================================*/
boolean qcril_sms_convert_mt_gw_sms_to_RIL_format
(
  wms_event_report_ind_msg_v01 * event_report_ind,
  std::vector<uint8_t>& buf_data
)
{
  QCRIL_LOG_FUNC_ENTRY();

  if (event_report_ind->mt_message_smsc_address_valid)
  {
    /* The SMSC Address was included. */
    buf_data.push_back(static_cast<uint8_t>(
          event_report_ind->mt_message_smsc_address.data_len));
    buf_data.insert(buf_data.end(), event_report_ind->mt_message_smsc_address.data,
          event_report_ind->mt_message_smsc_address.data +
          event_report_ind->mt_message_smsc_address.data_len);
  }
  else
  {
    /* no SMSC Address */
    buf_data.push_back(0);
  }

  buf_data.insert(buf_data.end(), event_report_ind->transfer_route_mt_message.data,
          event_report_ind->transfer_route_mt_message.data +
          event_report_ind->transfer_route_mt_message.data_len);

  return TRUE;
} /* qcril_sms_convert_mt_gw_sms_to_RIL_format */

qcril::interfaces::VerificationStatus
qcril_sms_convert_qmi_verstat_to_internal_type
(
  mt_ims_verstat_enum_v02 verstat_status_info
)
{
    switch(verstat_status_info) {
    case MT_IMS_TN_VALIDATION_NONE_V02:
        return qcril::interfaces::VerificationStatus::NONE;
    case MT_IMS_TN_VALIDATION_PASS_V02:
        return qcril::interfaces::VerificationStatus::PASS;
    case MT_IMS_TN_VALIDATION_FAIL_V02:
        return qcril::interfaces::VerificationStatus::FAIL;
    default:
        QCRIL_LOG_ERROR("Unknown verstat info: %d", verstat_status_info);
        return qcril::interfaces::VerificationStatus::UNKNOWN;
    }
}

/*=========================================================================
  FUNCTION:  qcril_sms_process_mt_gw_sms

===========================================================================*/
/*!
    @brief
    Performs processing necessary when a GW SMS message or Status Report
    is received.

    @return
    None
*/
/*=========================================================================*/
void qcril_sms_process_mt_gw_sms
(
  qcril_instance_id_e_type instance_id,
  wms_event_report_ind_msg_v01 * event_report_ind
)
{
  qcril_sms_struct_type *i_ptr;
  boolean is_processing_ok = TRUE;
  char details[ 80 ];

  struct timeval ack_expry_tmr_window = { 30 , 0 }; // 30 seconds

  /*-----------------------------------------------------------------------*/
  QCRIL_LOG_FUNC_ENTRY();
  i_ptr = &qcril_sms[ instance_id ];

  /*-----------------------------------------------------------------------*/

  QCRIL_SNPRINTF( details, sizeof( details ), "qcril_sms[%d].sms_ack_info_mutex", instance_id );
  QCRIL_MUTEX_LOCK( &i_ptr->sms_ack_info_mutex, details );

  /*-----------------------------------------------------------------------*/

  std::vector<uint8_t> buf_data;

  /* According to ril.h, we are to drop new SMS Messages until the previous
     message has been acked.  Note that the network will not send any new
     messages until the previous one has been acked; so the messages we are
     dropping here should all be duplicates.

     We only drop new SMS messages if an ack is needed.  There are some
     messages in the category of "no ack needed" which need to be sent to
     Android immediately.  For example, the segments of a multi-page BC SMS
     message are received too quickly for Android to ack them; waiting for
     an ack from Android will lead to segments being dropped.
  */
  if ( i_ptr->sms_ack_info.gw_ack_pending &&
      i_ptr->sms_ack_info.gw_ack_needed )
  {
    QCRIL_LOG_DEBUG("Ignoring GW SMS Message or Status Report. Waiting for RIL to ack a previous SMS.");
    is_processing_ok = FALSE;
  }

  if (is_processing_ok)
  {
    /* Convert the MT GW SMS message to RIL format. */
    if ( qcril_sms_convert_mt_gw_sms_to_RIL_format( event_report_ind,
                                                    buf_data ) == FALSE )
    {
      is_processing_ok = FALSE;
    }
  }

  if (is_processing_ok)
  {
    /* An MT SMS has been received.  An ack is pending from Android. */
    i_ptr->sms_ack_info.gw_ack_pending = TRUE;

    /* Ack_indicator of 0x01 means no ack is needed. If no ack is needed, QCRIL will
       drop the ack when it is received from the Android SMS application. */
    i_ptr->sms_ack_info.gw_ack_needed = !event_report_ind->transfer_route_mt_message.ack_indicator;

    i_ptr->sms_ack_info.gw_transaction_id = event_report_ind->transfer_route_mt_message.transaction_id;

    if (event_report_ind->sms_on_ims_valid &&
        event_report_ind->sms_on_ims)
    {
      i_ptr->sms_ack_info.gw_send_ack_on_ims = TRUE;
    }
    else
    {
      i_ptr->sms_ack_info.gw_send_ack_on_ims = FALSE;
    }

    // ack expiry tmr
    if ( QMI_RIL_ZERO != qcril_sms[QCRIL_DEFAULT_INSTANCE_ID].mt_pending_ack_expry_tmr )
    {
      qcril_cancel_timed_callback( (void*)(uintptr_t)qcril_sms[QCRIL_DEFAULT_INSTANCE_ID].mt_pending_ack_expry_tmr );
      qcril_sms[QCRIL_DEFAULT_INSTANCE_ID].mt_pending_ack_expry_tmr = QMI_RIL_ZERO;
    }

    QCRIL_LOG_DEBUG("Considering timeout for ack/nack for MT SMS, enabled %d, timer window %d",
                    qmi_ril_sms_mt_expiry_window_len_set,
                    qmi_ril_sms_mt_expiry_window_len);
    if ( qmi_ril_sms_mt_expiry_window_len_set )
    {
      ack_expry_tmr_window.tv_sec  = qmi_ril_sms_mt_expiry_window_len;
      ack_expry_tmr_window.tv_usec = QMI_RIL_ZERO;

      qcril_setup_timed_callback(   QCRIL_DEFAULT_INSTANCE_ID,
                                    QCRIL_DEFAULT_MODEM_ID,
                                    qcril_sms_mt_transaction_expired,
                                    &ack_expry_tmr_window,
                                    &qcril_sms[QCRIL_DEFAULT_INSTANCE_ID].mt_pending_ack_expry_tmr );
    }

    uint8_t message_ref;
    /* Determine whether to send the indication for Status Report or New SMS */
    if (qcril_sms_check_if_gw_msg_is_status_report(&event_report_ind->transfer_route_mt_message, message_ref))
    {
      bool is_on_ims = false; // better to query if IMS is registered
      if (mo_sms_on_ims_map.find(message_ref) != mo_sms_on_ims_map.end())
      {
        is_on_ims = mo_sms_on_ims_map[message_ref];
        /*Do not erase the ref id from map. Multiple reports may arrive for same msg*/
      }

      if (is_on_ims)
      {
        auto msg = std::make_shared<RilUnsolNewImsSmsStatusReportMessage>(
                message_ref, RADIO_TECH_3GPP);
        if (msg)
        {
            msg->setGsmPayload(std::move(buf_data));
            Dispatcher::getInstance().dispatch(msg);
        }
      }
      else
      {
        auto msg = std::make_shared<RilUnsolNewSmsStatusReportMessage>(
                std::move(buf_data));
        Dispatcher::getInstance().dispatch(msg);
      }
    }
    else
    {
      if (event_report_ind->sms_on_ims_valid && event_report_ind->sms_on_ims)
      {
        auto mt_ims_msg = std::make_shared<RilUnsolIncomingImsSMSMessage>(
                RADIO_TECH_3GPP);
        if (mt_ims_msg)
        {
            mt_ims_msg->setGsmPayload(std::move(buf_data));
            if (event_report_ind->verstat_status_info_valid)
            {
                mt_ims_msg->setVerificationStatus(
                    qcril_sms_convert_qmi_verstat_to_internal_type(
                    event_report_ind->verstat_status_info));
            }
            Dispatcher::getInstance().dispatch(mt_ims_msg);
        }
      }
      else
      {
        auto mt_sms_msg = std::make_shared<RilUnsolIncoming3GppSMSMessage>(
            std::move(buf_data));
        Dispatcher::getInstance().dispatch(mt_sms_msg);
      }
    }
  }

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_UNLOCK( &i_ptr->sms_ack_info_mutex, details );
  QCRIL_LOG_FUNC_RETURN();
} /* qcril_sms_process_mt_gw_sms */

/*=========================================================================
  FUNCTION:  qcril_sms_request_raw_read

===========================================================================*/
/*!
    @brief
    Function for sending a request for reading message stored in NV

    @return
    None
*/
/*=========================================================================*/

void qcril_sms_request_raw_read
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  wms_raw_read_req_msg_v01 *wms_raw_read_req_msg_ptr;
  wms_raw_read_resp_msg_v01 wms_raw_read_resp_msg;
  qmi_client_error_type qmi_client_error;
  qcril_instance_id_e_type instance_id;
  RIL_Errno ril_req_res = RIL_E_SUCCESS;
  RIL_CDMA_SMS_Message * cdma_sms_msg = nullptr;

  QCRIL_LOG_FUNC_ENTRY();

  instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  wms_raw_read_req_msg_ptr = ( wms_raw_read_req_msg_v01 * ) params_ptr->data;
  QCRIL_NOTUSED( ret_ptr );

  if ( wms_raw_read_req_msg_ptr != NULL )
  {
    // Check for WMS SERVICE READY
    if ((wms_raw_read_req_msg_ptr->message_mode != WMS_MESSAGE_MODE_GW_V01) &&
        (wms_raw_read_req_msg_ptr->message_mode != WMS_MESSAGE_MODE_CDMA_V01))
    {
       // Inavlid message mode - while its an optional TLV this function
       // is only called internaly w/a valid message mode
       QCRIL_LOG_ERROR("Raw Read Req Failed w/invalid message mode %d",
                        wms_raw_read_req_msg_ptr->message_mode);
       QCRIL_LOG_FUNC_RETURN();
       return;
    }

    memset(&wms_raw_read_resp_msg,0,sizeof(wms_raw_read_resp_msg));

    QCRIL_LOG_INFO("MSG PARAMS %d %d %d",
                   wms_raw_read_req_msg_ptr->message_memory_storage_identification.storage_index,
                   wms_raw_read_req_msg_ptr->message_memory_storage_identification.storage_type,
                   wms_raw_read_req_msg_ptr->message_mode);

    qmi_client_error = qmi_client_sms_send_sync(QMI_WMS_RAW_READ_REQ_V01,
                                                wms_raw_read_req_msg_ptr,
                                                sizeof(*wms_raw_read_req_msg_ptr),
                                                &wms_raw_read_resp_msg,
                                                sizeof(wms_raw_read_resp_msg),
                                                ModemEndPoint::SYNC_REQ_UNRESTRICTED_TIMEOUT);

    ril_req_res = qcril_qmi_util_convert_qmi_response_codes_to_ril_result( qmi_client_error, &wms_raw_read_resp_msg.resp );

    if ( RIL_E_SUCCESS != ril_req_res )
    {
      QCRIL_LOG_ERROR("Raw Read Req Msg Send Failed, err %d", (int)ril_req_res);
    }
    else
    {
      QCRIL_LOG_INFO("Raw Read Request succeeded");

      /* Allocate buffer to decode new CDMA SMS */
      cdma_sms_msg = static_cast<RIL_CDMA_SMS_Message*>(qcril_malloc(sizeof(RIL_CDMA_SMS_Message)));
      if (cdma_sms_msg == nullptr)
      {
        QCRIL_LOG_ERROR("Failed to allocate buffer to decode new CDMA SMS.");
      }
      else
      {
        if (wms_raw_read_resp_msg.raw_message_data.format == WMS_MESSAGE_FORMAT_MWI_V01)
        {
          cdma_sms_msg->uTeleserviceID = 0x00040000;
          cdma_sms_msg->bIsServicePresent = FALSE;
          cdma_sms_msg->sAddress.number_of_digits = 0;
          cdma_sms_msg->sSubAddress.number_of_digits = 0;
          cdma_sms_msg->uBearerDataLen = 1;
          cdma_sms_msg->aBearerData[0] = wms_raw_read_resp_msg.raw_message_data.data[0];

          auto mt_sms_msg = std::make_shared<RilUnsolIncoming3Gpp2SMSMessage>(*cdma_sms_msg);
          Dispatcher::getInstance().dispatch(mt_sms_msg);
        }
        else if (wms_raw_read_resp_msg.raw_message_data.format == WMS_MESSAGE_FORMAT_CDMA_V01)
        {
          if (qcril_sms_convert_mt_sms_qmi_to_ril(wms_raw_read_resp_msg.raw_message_data.data,
                                                  wms_raw_read_resp_msg.raw_message_data.data_len,
                                                  cdma_sms_msg) == TRUE)
          {
            auto mt_sms_msg = std::make_shared<RilUnsolIncoming3Gpp2SMSMessage>(*cdma_sms_msg);
            Dispatcher::getInstance().dispatch(mt_sms_msg);
          }
          else
          {
            QCRIL_LOG_ERROR("failed to convert SMS to RIL format.");
          }
        }
        else
        {
          QCRIL_LOG_ERROR("unexpected format.");
        }
      qcril_free(cdma_sms_msg);
      }
    }
  }

  QCRIL_LOG_FUNC_RETURN();

} /*qcril_sms_request_raw_read*/

MultipartGsmCmasCellbroadcast::RadioTech convert_sms_rat_info(
    const wms_event_report_ind_msg_v01 &event_report_ind) {
    MultipartGsmCmasCellbroadcast::RadioTech rat{MultipartGsmCmasCellbroadcast::RadioTech::GSM};
    if (event_report_ind.bc_sms_rat_info_valid) {
        switch (event_report_ind.bc_sms_rat_info) {
            case WMS_BC_RAT_GSM_RADIO_ACCESS_V01:
                rat = MultipartGsmCmasCellbroadcast::RadioTech::GSM;
                break;
            case WMS_BC_RAT_WCDMA_RADIO_ACCESS_V01:
                rat = MultipartGsmCmasCellbroadcast::RadioTech::WCDMA;
                break;
            case WMS_BC_RAT_LTE_RADIO_ACCESS_V01:
                rat = MultipartGsmCmasCellbroadcast::RadioTech::LTE;
                break;
            case WMS_BC_RAT_CDMA_RADIO_ACCESS_V01:
                rat = MultipartGsmCmasCellbroadcast::RadioTech::CDMA;
                break;
            case WMS_BC_RAT_NR5G_RADIO_ACCESS_V01:
                rat = MultipartGsmCmasCellbroadcast::RadioTech::NR5G;
                break;
            default:
                rat = MultipartGsmCmasCellbroadcast::RadioTech::GSM;
                break;
        }
    }
    return rat;
}

std::vector<uint8_t> get_transfer_route_mt_message(
    const wms_event_report_ind_msg_v01 &event_report_ind) {
    uint32_t data_len = event_report_ind.transfer_route_mt_message.data_len;
    auto array_begin = event_report_ind.transfer_route_mt_message.data;
    auto array_end = array_begin + data_len;
    std::vector<uint8_t> qmi_buffer(array_begin, array_end);
    return qmi_buffer;
}

std::vector<uint8_t> get_warning_area_buffer(
    const wms_event_report_ind_msg_v01 &event_report_ind) {
    if (!event_report_ind.warning_area_valid) {
        return {};
    }
    uint32_t wac_len = event_report_ind.warning_area_len;
    auto wac_begin = event_report_ind.warning_area;
    auto wac_end = wac_begin + wac_len;
    auto wac_buffer = std::vector<uint8_t>(wac_begin, wac_end);
    return wac_buffer;
}
std::string byte_buffer_to_hex_string(const std::vector<uint8_t>& buffer) {
    static constexpr const char *hex_upper = "0123456789ABCDEF";
    std::ostringstream os;

    for (const auto &val : buffer) {
        os << hex_upper[val >> 4];
        os << hex_upper[val & 0x0f];
    }
    return  os.str();
}
/*=========================================================================
  FUNCTION:  qcril_sms_process_event_report_ind

===========================================================================*/
/*!
    @brief
    Performs processing necessary when a QMI_WMS_EVENT_REPORT_IND indication
    is received.

    @return
    None
*/
/*=========================================================================*/
void qcril_sms_process_event_report_ind
(
  wms_event_report_ind_msg_v01 * event_report_ind,
  qcril_instance_id_e_type instance_id
)
{
  int index_on_sim;
  uint8 coding_scheme;
  wms_call_control_modified_info_type_v01 src_wms_call_control_alpha_info;
  wms_call_control_modified_info_type_v01 wms_call_control_alpha_info;
  char buf_str[WMS_ALPHA_ID_LENGTH_MAX_V01*2 + 2];

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_FUNC_ENTRY();
  memset(&src_wms_call_control_alpha_info, 0, sizeof(src_wms_call_control_alpha_info));
  memset(&wms_call_control_alpha_info, 0, sizeof(wms_call_control_alpha_info));

  if( event_report_ind != NULL)
  {
  QCRIL_LOG_INFO( ".. transfer_route_mt_message_valid %d", (int) event_report_ind->transfer_route_mt_message_valid );
  if (event_report_ind->transfer_route_mt_message_valid)
  {
    QCRIL_LOG_INFO( ".. transfer_route_mt_message.format %d", (int) event_report_ind->transfer_route_mt_message.format );
    /* MT CDMA SMS */
    if ((event_report_ind->transfer_route_mt_message.format == WMS_MESSAGE_FORMAT_CDMA_V01) ||
        (event_report_ind->transfer_route_mt_message.format == WMS_MESSAGE_FORMAT_MWI_V01))
    {
      qcril_sms_process_mt_cdma_sms( instance_id,
                                     event_report_ind,
                                     event_report_ind->transfer_route_mt_message.format);
    }
    /* MT GW SMS */
    else if (event_report_ind->transfer_route_mt_message.format == WMS_MESSAGE_FORMAT_GW_PP_V01)
    {
      qcril_sms_process_mt_gw_sms( instance_id, event_report_ind );
    }
    /* GW SMS Broadcast */
    else if (event_report_ind->transfer_route_mt_message.format == WMS_MESSAGE_FORMAT_GW_BC_V01)
    {
      /* It is a broadcast SMS.  Maximum size is 88 bytes long. */
      QCRIL_LOG_VERBOSE("GSM Broadcast SMS Message");

      /* Convert QMI enum to internal enum */
      auto rat = convert_sms_rat_info(*event_report_ind);

      /* Get the cmas broadcast message buffer from event_report_ind. */
      std::vector<uint8_t> cmas_qmi_buffer = get_transfer_route_mt_message(*event_report_ind);
      QCRIL_LOG_VERBOSE("Qmi-wms cmas buffer {%s}",
                        byte_buffer_to_hex_string(cmas_qmi_buffer).c_str());

      /* Get WAC buffer from the event_report_ind. If WAC is not present then
       * initialize to empty vector. */
      std::vector<uint8_t> wac_buffer = get_warning_area_buffer(*event_report_ind);
      QCRIL_LOG_VERBOSE("Qmi-wms wac buffer {%s}", byte_buffer_to_hex_string(wac_buffer).c_str());

      /* Lambda to send RilUnsolNewBroadcastSmsMessage to upper layer */
      MultipartGsmCmasCellbroadcast::CallbackType callback = [](const std::vector<uint8_t> buffer) {
          QCRIL_LOG_VERBOSE("Android cmas buffer {%s}", byte_buffer_to_hex_string(buffer).c_str());
          auto msg = std::make_shared<RilUnsolNewBroadcastSmsMessage>(std::move(buffer));
          Dispatcher::getInstance().dispatch(msg);
          return true;
      };

      /* Main handler to translate the qmi-format of cmas broadcast to Android format.*/
      handle_event_report_ind_cmas_buffer(rat, callback, cmas_qmi_buffer, wac_buffer);
    }
  }
  /* SMS on SIM */
  else if ((event_report_ind->mt_message_valid) &&
           (event_report_ind->message_mode_valid))
  {
    if (event_report_ind->mt_message.storage_type == WMS_STORAGE_TYPE_UIM_V01)
    {
       if (event_report_ind->message_mode == WMS_MESSAGE_MODE_GW_V01)
       {
         /* Android uses the GSDI index, since it calls RIL_REQUEST_SIM_IO.
            The GSDI index is the QMI index plus 1. */
         index_on_sim = event_report_ind->mt_message.storage_index + 1;

         /* Notify client that we received a class 2 message (new SMS on SIM) */
         auto msg = std::make_shared<RilUnsolNewSmsOnSimMessage>(index_on_sim);
         Dispatcher::getInstance().dispatch(msg);
       }
       else if (event_report_ind->message_mode == WMS_MESSAGE_MODE_CDMA_V01)
       {
          wms_raw_read_req_msg_v01 read_req;

          read_req.message_mode_valid = TRUE;
          read_req.message_mode = event_report_ind->message_mode;
          read_req.message_memory_storage_identification.storage_type =
          event_report_ind->mt_message.storage_type;
          read_req.message_memory_storage_identification.storage_index =
          event_report_ind->mt_message.storage_index;
          read_req.sms_on_ims_valid = event_report_ind->sms_on_ims_valid;
          read_req.sms_on_ims = event_report_ind->sms_on_ims;
          QCRIL_LOG_INFO("Fetching MT CDMA SMS on SIM at storage index %d",
                          event_report_ind->mt_message.storage_index);
          qcril_event_queue( QCRIL_DEFAULT_INSTANCE_ID, QCRIL_DEFAULT_MODEM_ID,
                             QCRIL_DATA_ON_STACK, QCRIL_EVT_SMS_RAW_READ,
                             (void *) &read_req, sizeof( read_req ),
                             (RIL_Token) QCRIL_TOKEN_ID_INTERNAL );
       }
       else {
          QCRIL_LOG_ERROR("Invalid WMS message mode %d, ignoring",
                           event_report_ind->message_mode);
       }
    }
    else if ((event_report_ind->mt_message.storage_type == WMS_STORAGE_TYPE_NV_V01) &&
             (event_report_ind->message_mode == WMS_MESSAGE_MODE_CDMA_V01))
    {
      wms_raw_read_req_msg_v01 read_req;

      read_req.message_mode_valid = TRUE;
      read_req.message_mode = event_report_ind->message_mode;
      read_req.message_memory_storage_identification.storage_type =
      event_report_ind->mt_message.storage_type;
      read_req.message_memory_storage_identification.storage_index =
      event_report_ind->mt_message.storage_index;
      read_req.sms_on_ims_valid = event_report_ind->sms_on_ims_valid;
      read_req.sms_on_ims = event_report_ind->sms_on_ims;
      QCRIL_LOG_INFO("storage index %d", event_report_ind->mt_message.storage_index);
      qcril_event_queue( QCRIL_DEFAULT_INSTANCE_ID, QCRIL_DEFAULT_MODEM_ID,
                         QCRIL_DATA_ON_STACK, QCRIL_EVT_SMS_RAW_READ, (void *) &read_req,
                         sizeof( read_req ), (RIL_Token) QCRIL_TOKEN_ID_INTERNAL );
    }
  }
  /* ETWS message */
  else if (event_report_ind->etws_message_valid)
  {
    std::vector<uint8_t> buf_data(event_report_ind->etws_message.data,
            event_report_ind->etws_message.data + event_report_ind->etws_message.data_len);
    auto msg = std::make_shared<RilUnsolNewBroadcastSmsMessage>(
            std::move(buf_data));
    Dispatcher::getInstance().dispatch(msg);
  }
  /* Alpha Info */
  else if( ( TRUE == event_report_ind->call_control_info_valid ) &&
    ( event_report_ind->call_control_info.alpha_id_len > 0 ) )
  {
    memset(buf_str, 0 , sizeof(buf_str));
    coding_scheme = event_report_ind->call_control_info.alpha_id[0];
    if( coding_scheme == 0x80 || coding_scheme == 0x81 || coding_scheme == 0x82 ) //UCS2
    {
        src_wms_call_control_alpha_info.alpha_id_len = event_report_ind->call_control_info.alpha_id_len;
        memcpy(src_wms_call_control_alpha_info.alpha_id,event_report_ind->call_control_info.alpha_id,WMS_ALPHA_ID_LENGTH_MAX_V01);
        QCRIL_LOG_INFO("Coding scheme is %x, ucs2 data",coding_scheme);
        qcril_qmi_sms_transfer_sim_ucs2_alpha_to_std_ucs2_alpha ( &src_wms_call_control_alpha_info, &wms_call_control_alpha_info);
        qcril_cm_ss_convert_ussd_string_to_utf8(  QCRIL_QMI_VOICE_USSD_DCS_UCS2,
                                                wms_call_control_alpha_info.alpha_id,
                                                wms_call_control_alpha_info.alpha_id_len,
                                                buf_str,
                                                sizeof(buf_str));
    }
    else //gsm8
    {
        QCRIL_LOG_INFO("Coding scheme is %x, gsm8 data",coding_scheme);
        if( event_report_ind->call_control_info.alpha_id_len < WMS_ALPHA_ID_LENGTH_MAX_V01 )
        {
            qcril_cm_ss_convert_gsm8bit_alpha_string_to_utf8( (char*) event_report_ind->call_control_info.alpha_id,
                                                            event_report_ind->call_control_info.alpha_id_len,
                                                            buf_str,
                                                            sizeof(buf_str));
        }
    }

    if ( *buf_str )
    {
        auto msg = std::make_shared<RilUnsolStkCCAlphaNotifyMessage>(
                std::string(static_cast<const char*>(buf_str)));
        Dispatcher::getInstance().dispatch(msg);
    }
  }

  }

  QCRIL_LOG_FUNC_RETURN();

} /* qcril_sms_process_event_report_ind */


/*=========================================================================
  FUNCTION:  qcril_sms_process_memory_full_ind

===========================================================================*/
/*!
    @brief
    Performs processing necessary when a QMI_WMS_MEMORY_FULL_IND indication
    is received.

    @return
    None
*/
/*=========================================================================*/
void qcril_sms_process_memory_full_ind
(
  wms_memory_full_ind_msg_v01 * memory_full_ind,
  qcril_instance_id_e_type instance_id
)
{
  (void)instance_id;
  if (memory_full_ind != NULL && memory_full_ind->memory_full_info.storage_type == WMS_STORAGE_TYPE_UIM_V01)
  {
    if (memory_full_ind->memory_full_info.message_mode == WMS_MESSAGE_MODE_CDMA_V01)
    {
      QCRIL_LOG_DEBUG("EVENT_MEMORY_FULL: mem store RUIM");
      auto msg = std::make_shared<RilUnsolCdmaRuimSmsStorageFullMessage>();
      Dispatcher::getInstance().dispatch(msg);
    }
    else if (memory_full_ind->memory_full_info.message_mode == WMS_MESSAGE_MODE_GW_V01)
    {
      QCRIL_LOG_DEBUG("EVENT_MEMORY_FULL: mem store SIM");
      auto msg = std::make_shared<RilUnsolSimSmsStorageFullMessage>();
      Dispatcher::getInstance().dispatch(msg);
    }
  }

} /* qcril_sms_process_memory_full_ind */


/*=========================================================================
  FUNCTION:  qcril_sms_process_transport_nw_reg_info_ind

===========================================================================*/
/*!
    @brief
    Performs processing necessary when a QMI_WMS_TRANSPORT_NW_REG_INFO_IND
    indication is received.

    @return
    None
*/
/*=========================================================================*/
void qcril_sms_process_transport_nw_reg_info_ind
(
  wms_transport_nw_reg_info_ind_msg_v01 * transport_nw_reg_info_ind,
  qcril_instance_id_e_type instance_id
)
{
  qmi_ril_sms_ims_reg_state ims_sms_state;
  RIL_Errno res = RIL_E_SUCCESS;
  SmsImsServiceStatusInd::SmsImsServiceStatusInfo smsImsServiceStatus;

  QCRIL_NOTUSED(instance_id);
  QCRIL_LOG_FUNC_ENTRY();

  qcril_qmi_sms_report_unsol_ims_state_change();

  qcril_qmi_nas_set_registered_on_ims( (WMS_TRANSPORT_NW_REG_STATUS_FULL_SERVICE_V01 == transport_nw_reg_info_ind->transport_nw_reg_status) ? TRUE : FALSE );

  res = qcril_qmi_sms_retrieve_ims_sms_registration_state(ims_sms_state, smsImsServiceStatus);

  if(RIL_E_SUCCESS == res)
  {
    std::shared_ptr<SmsImsServiceStatusInd> msg =
      std::make_shared<SmsImsServiceStatusInd>(smsImsServiceStatus);
    Dispatcher::getInstance().broadcast(msg);
  }

  QCRIL_LOG_FUNC_RETURN();
} /* qcril_sms_process_transport_nw_reg_info_ind */

/*=========================================================================
  FUNCTION:  qcril_sms_process_transport_layer_info_ind

===========================================================================*/
/*!
    @brief
    Performs processing necessary when a QMI_WMS_TRANSPORT_LAYER_INFO_IND
    indication is received.

    @return
    None
*/
/*=========================================================================*/
void qcril_sms_process_transport_layer_info_ind
(
  wms_transport_layer_info_ind_msg_v01 * transport_layer_info_ind,
  qcril_instance_id_e_type instance_id
)
{
  qcril_sms_struct_type *i_ptr;
  qmi_ril_sms_ims_reg_state ims_sms_state;
  RIL_Errno res = RIL_E_SUCCESS;
  SmsImsServiceStatusInd::SmsImsServiceStatusInfo smsImsServiceStatus;
  QCRIL_NOTUSED(instance_id);

  QCRIL_LOG_FUNC_ENTRY();

  i_ptr = &qcril_sms[ QCRIL_DEFAULT_INSTANCE_ID ];
  QCRIL_MUTEX_LOCK( &i_ptr->transport_layer_info_mutex, NULL );
  QCRIL_LOG_INFO( "transport layer reg info value %d", transport_layer_info_ind->registered_ind);
  i_ptr->transport_layer_info.registered_ind_valid = TRUE;
  i_ptr->transport_layer_info.registered_ind = transport_layer_info_ind->registered_ind;
  if (transport_layer_info_ind->transport_layer_info_valid)
  {
     i_ptr->transport_layer_info.sms_format_valid = TRUE;
     i_ptr->transport_layer_info.sms_format =
       (transport_layer_info_ind->transport_layer_info.transport_cap == WMS_TRANSPORT_CAP_CDMA_V01) ?
       QMI_RIL_SMS_FORMAT_3GPP2 : QMI_RIL_SMS_FORMAT_3GPP;
  }
  QCRIL_MUTEX_UNLOCK( &i_ptr->transport_layer_info_mutex, NULL );

  qcril_qmi_sms_report_unsol_ims_state_change();

  res = qcril_qmi_sms_retrieve_ims_sms_registration_state(ims_sms_state, smsImsServiceStatus);

  if(RIL_E_SUCCESS == res)
  {
    std::shared_ptr<SmsImsServiceStatusInd> msg =
      std::make_shared<SmsImsServiceStatusInd>(smsImsServiceStatus);
    Dispatcher::getInstance().broadcast(msg);
  }

  QCRIL_LOG_FUNC_RETURN();
} /* qcril_sms_process_transport_layer_info_ind */

//=========================================================================
// FUNCTION: qcril_sms_process_service_ready_ind
//
// DESCRIPTION:
// set wms ready state and relay state change to telephony
//
// RETURN: None
//=========================================================================
void qcril_sms_process_service_ready_ind
(
  wms_service_ready_ind_msg_v01* service_msg
)
{
  QCRIL_LOG_FUNC_ENTRY();

  if (!service_msg) {
     QCRIL_LOG_FATAL("service_msg was NULL!");
     QCRIL_ASSERT(0);
     return;
  }

  // set global service state and log new state
  set_wms_service_state(service_msg->ready_status);

  // send indication to telephony via oem hook, rilwms
  unsigned char data = (unsigned char)(service_msg->ready_status);
  qcril_hook_unsol_response(QCRIL_DEFAULT_INSTANCE_ID,
                            QCRIL_EVT_HOOK_UNSOL_WMS_READY,
                            (char*)&data, 1);


  QCRIL_LOG_FUNC_RETURN();
}

//=========================================================================
// FUNCTION: qcril_sms_process_transport_layer_mwi_ind
//
// DESCRIPTION:
// Message waiting indication handler
//
// RETURN: None
//=========================================================================
void qcril_sms_process_transport_layer_mwi_ind
(
 wms_transport_layer_mwi_ind_msg_v01 *qmi_mwi_msg
)
{
  QCRIL_LOG_FUNC_ENTRY();

  if (!qmi_mwi_msg) {
    QCRIL_LOG_FATAL("mwi_msg was NULL!");
    QCRIL_ASSERT(0);
    return;
  }

  auto msg = std::make_shared<QcRilUnsolMessageWaitingInfoMessage>();
  if (msg != nullptr) {
    uint32_t i = 0;
    uint32_t sum_len = qmi_mwi_msg->message_waiting_summary_info_len;
    QCRIL_LOG_INFO( "num of mwi_msg_summary = %d", sum_len);
    if (sum_len > 0)
    {
      std::vector<qcril::interfaces::MessageSummary> messageSummary(sum_len);

      for ( i = 0; i < sum_len; i++)
      {
        messageSummary[i].setMessageType(qcril_qmi_sms_map_qmi_mwi_msg_type_to_ims_msg_type(
              qmi_mwi_msg->message_waiting_summary_info[i].message_type));

        messageSummary[i].setNewMessageCount( qmi_mwi_msg->message_waiting_summary_info[i].new_msg);

        messageSummary[i].setOldMessageCount( qmi_mwi_msg->message_waiting_summary_info[i].old_msg);

        messageSummary[i].setNewUrgentMessageCount(
            qmi_mwi_msg->message_waiting_summary_info[i].new_urgent);

        messageSummary[i].setOldUrgentMessageCount(
            qmi_mwi_msg->message_waiting_summary_info[i].old_urgent);

      }
      msg->setMessageSummary(messageSummary);
    }
    msg->setUeAddress(qmi_mwi_msg->UE_address);

    if (qmi_mwi_msg->message_waiting_detail_info_valid &&
        qmi_mwi_msg->message_waiting_detail_info_len > 0)
    {
      uint32_t detail_len = qmi_mwi_msg->message_waiting_detail_info_len;
      QCRIL_LOG_INFO( "num of mwi_msg_details = %d", detail_len);

      std::vector<qcril::interfaces::MessageDetails> messageDetails(detail_len);

      for ( i = 0; i < detail_len; i++)
      {
        messageDetails[i].setToAddress(
            qmi_mwi_msg->message_waiting_detail_info[i].to_address);
        messageDetails[i].setFromAddress(
            qmi_mwi_msg->message_waiting_detail_info[i].from_address);
        messageDetails[i].setSubject(
            qmi_mwi_msg->message_waiting_detail_info[i].subject);
        messageDetails[i].setDate(
            qmi_mwi_msg->message_waiting_detail_info[i].date_time);
        messageDetails[i].setPriority(
            qcril_qmi_sms_map_qmi_mwi_priority_to_ims_priority(
              qmi_mwi_msg->message_waiting_detail_info[i].priority));
        messageDetails[i].setMessageId(
            qmi_mwi_msg->message_waiting_detail_info[i].message_id);
        messageDetails[i].setMessageType(
            qcril_qmi_sms_map_qmi_mwi_msg_type_to_ims_msg_type(
              qmi_mwi_msg->message_waiting_detail_info[i].message_type));
      }
      msg->setMessageDetails(messageDetails);
    }

    msg->broadcast();
  }

  QCRIL_LOG_FUNC_RETURN();
}

//=========================================================================
// FUNCTION: qcril_sms_post_ready_status_update
//
// DESCRIPTION:
// post current wms state to telephony
//
// RETURN: None
//=========================================================================
void qcril_sms_post_ready_status_update(void)
{
   wms_service_ready_status_enum_v01 state;

   QCRIL_LOG_FUNC_ENTRY();

   if ( qcril_qmi_client_is_available() && QMI_RIL_SMS_SVC_FULLY_OPERATIONAL == qmi_ril_get_sms_svc_status() ) // see if SMS client initialization is in reasonable state
   {
      SMS_CACHE_LOCK;
      state = wms_service_state;
      SMS_CACHE_UNLOCK;

      QCRIL_LOG_INFO("..posting wms_ready status %d (%s)", (int)state, sms_state_str(state) );

      qcril_hook_unsol_response( QCRIL_DEFAULT_INSTANCE_ID,
                                 QCRIL_EVT_HOOK_UNSOL_WMS_READY,
                                 (char*)&state, sizeof(state) );
   }

   QCRIL_LOG_FUNC_RETURN();
}

/*===========================================================================

                   QMI UNSOL INDICATION HANDLER

===========================================================================*/

/*=========================================================================
  FUNCTION:  qcril_qmi_sms_unsolicited_indication_cb_helper

===========================================================================*/
/*!
    @brief
    helper function for handling sms indication

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_sms_unsolicited_indication_cb_helper
(
  unsigned int   msg_id,
  unsigned char *decoded_payload,
  uint32_t       decoded_payload_len
)
{
   qcril_instance_id_e_type instance_id;

   QCRIL_NOTUSED(decoded_payload_len);
   QCRIL_LOG_FUNC_ENTRY();

   do
   {
      QCRIL_LOG_INFO("msg_id (0x%04x) %s", msg_id,
                qcril_sms_lookup_ind_name(msg_id));

      qmi_ril_gen_operational_status_type op_status
                                            = qmi_ril_get_operational_status();

      QCRIL_LOG_INFO("operation state - %d", (int) op_status );

      if (!decoded_payload)
      {
        QCRIL_LOG_ERROR("Decoded payload is null.");
        break;
      }

      instance_id = QCRIL_DEFAULT_INSTANCE_ID;

      switch (msg_id)
      {
        case QMI_WMS_EVENT_REPORT_IND_V01: {
           qcril_qmi_nas_notify_mt_sms_ind_hdlr(
                 (wms_event_report_ind_msg_v01*) decoded_payload, instance_id);
           break;
           }
        case QMI_WMS_MEMORY_FULL_IND_V01: {
           qcril_sms_process_memory_full_ind(
                  (wms_memory_full_ind_msg_v01*) decoded_payload, instance_id);
           break;
           }
        case QMI_WMS_TRANSPORT_LAYER_INFO_IND_V01: {
           qcril_sms_process_transport_layer_info_ind(
              (wms_transport_layer_info_ind_msg_v01*) decoded_payload, instance_id);
           break;
           }
        case QMI_WMS_TRANSPORT_NW_REG_INFO_IND_V01: {
           qcril_sms_process_transport_nw_reg_info_ind(
              (wms_transport_nw_reg_info_ind_msg_v01*) decoded_payload, instance_id);
           break;
           }
        case QMI_WMS_SERVICE_READY_IND_V01: {
           qcril_sms_process_service_ready_ind(
                (wms_service_ready_ind_msg_v01*) decoded_payload);
           break;
           }
        case QMI_WMS_TRANSPORT_LAYER_MWI_IND_V01: {
           qcril_sms_process_transport_layer_mwi_ind(
                (wms_transport_layer_mwi_ind_msg_v01*) decoded_payload);
           break;
           }
        default: {
           QCRIL_LOG_INFO("Unexpected, ignoring QMI WMS indication 0x%04x",
                         msg_id);
           break;
           }
      }
   }while(FALSE);

   QCRIL_LOG_FUNC_RETURN();
}

/*===========================================================================

                                UTILITY FUNCTIONS

===========================================================================*/
/*===========================================================================

FUNCTION    qcril_sms_strip_quotes

DESCRIPTION
  Strips out quotes from the string that is wrapped in quotes.
  Resultant string will be placed in the out pointer.
  Gives an error if the string does not begin or end with a quote

DEPENDENCIES
  None

RETURN VALUE
  Boolean :
    TRUE : if successful in stripping the quotes out
    FALSE : if there is any error

SIDE EFFECTS
  None

===========================================================================*/
boolean qcril_sms_strip_quotes
(
  const char * in_ptr,   /* Pointer to the string to be processed  */
  char * out_ptr         /* Pointer to the resultant string buffer */
)
{
  int ret_value = FALSE;
  do
  {
  if (*in_ptr != '\0' )
  {
    if ( *in_ptr++ == '"' )
    {
      while ( *in_ptr != '"' && *in_ptr != '\0' )
      {
        *out_ptr++ = *in_ptr++;
      }
      /* Check to see if the string ends with a null */
      if ( *in_ptr == '\0' )
      {
        /* We got a string without ending quotes */
          ret_value = FALSE;
          break;
      }
      else
      {
        /* Everything is happy */
        *out_ptr = '\0';
          ret_value = TRUE;
          break;
      }
    }
    else
    {
      /* We got a string with out quotes */
        ret_value = FALSE;
        break;
    }
  }
  else
  {
      ret_value = FALSE;
      break;
  }
  }while(0);

  return ret_value;
} /* qcril_sms_strip_quotes */


/*=========================================================================
  FUNCTION:  qcril_sms_convert_smsc_address_to_qmi_format

===========================================================================*/
/*!
    @brief
    Converts an SMSC address from RIL format to QMI format.
    Note that the RIL format comes from the CSCA ATCOP command:
    "+358501234567",145

    @return
    TRUE if the SMSC address was successfully translated to QMI format.
    FALSE otherwise.
*/
/*=========================================================================*/
boolean qcril_sms_convert_smsc_address_to_qmi_format
(
  const char * input_smsc_address_ptr,
  wms_set_smsc_address_req_msg_v01 * qmi_request
)
{
  char * asciiDigits = NULL;
  char * toa = NULL;
  boolean status = TRUE;
  char * ascii_digits_ptr = NULL;
  char * toa_ptr = NULL;

  if(input_smsc_address_ptr != NULL && qmi_request != NULL && strlen(input_smsc_address_ptr) != 0)
  {
  /* Allocate space for the SMSC address and SMSC type of address.  Leave
     a space for null terminator. */
  asciiDigits = (char *) qcril_malloc(strlen(input_smsc_address_ptr) + 1);
  toa = (char *) qcril_malloc(strlen(input_smsc_address_ptr) + 1);
  if (asciiDigits == NULL || toa == NULL)
  {
    QCRIL_LOG_ERROR("Unable to allocate buffer for SMSC address");
    status = FALSE;
  }
  else
  {
    ascii_digits_ptr = asciiDigits;
    toa_ptr = toa;

    /* Copy everything up to the first comma into the SMSC address */
    while ((*input_smsc_address_ptr != '\0') &&
           (*input_smsc_address_ptr != ','))
    {
      *ascii_digits_ptr++ = *input_smsc_address_ptr++;
    }
    *ascii_digits_ptr = '\0';

    if (*input_smsc_address_ptr == ',')
    {
      /* Increment past the comma */
      input_smsc_address_ptr++;
      while (*input_smsc_address_ptr != '\0')
      {
        /* Copy the rest of the string into the type of address */
        *toa_ptr++ = *input_smsc_address_ptr++;
      }
    }
    *toa_ptr = '\0';

    /* Strip the quotes from the SMSC address */
    if (!qcril_sms_strip_quotes(asciiDigits, asciiDigits))
    {
      QCRIL_LOG_ERROR("SMSC address either did not begin or end with quotes!");
      status = FALSE;
    }
    else if (strlen(asciiDigits) > WMS_ADDRESS_DIGIT_MAX_V01)
    {
      QCRIL_LOG_ERROR("Too many digits in SMSC address");
      status = FALSE;
    }
    else if (strlen(toa) > WMS_ADDRESS_TYPE_MAX_V01)
    {
      QCRIL_LOG_ERROR("Too many digits in SMSC address type");
      status = FALSE;
    }
    else if (strlen(asciiDigits) == 0)
    {
      QCRIL_LOG_ERROR("SMSC address is empty!");
      status = FALSE;
    }

    if (status)
    {
      /* Fill in the SMSC Address.  Note that the input needs to be NULL-terminated,
         so copy the NULL byte as well. */
      strlcpy(qmi_request->smsc_address_digits,
             asciiDigits,
             WMS_ADDRESS_DIGIT_MAX_V01 + 1);

      if (strlen(toa) > 0)
      {
        /* Include the SMSC Address Type TLV */
        qmi_request->smsc_address_type_valid = TRUE;

        strlcpy(qmi_request->smsc_address_type,
               toa,
               WMS_ADDRESS_TYPE_MAX_V01 + 1);
      }
    }
  }

  if (asciiDigits != NULL)
  {
    qcril_free( asciiDigits );
  }
  if (toa != NULL)
  {
    qcril_free( toa );
  }
  }
  else
  {
    status = FALSE;
  }

  return status;

} /* qcril_sms_convert_smsc_address_to_qmi_format */


/*=========================================================================
  FUNCTION:  qcril_sms_convert_smsc_address_to_ril_format

===========================================================================*/
/*!
    @brief
    Converts an SMSC address from QMI format to RIL format.
    Note that the RIL format comes from the CSCA ATCOP command:
    "+358501234567",145

    @return
    RIL_E_SUCCESS if the SMSC address was successfully translated to RIL format.
    RIL_E_INTERNAL_ERR otherwise.
*/
/*=========================================================================*/
RIL_Errno qcril_sms_convert_smsc_address_to_ril_format
(
  const wms_get_smsc_address_resp_msg_v01 * qmi_response,
  std::string& output_smsc_address
)
{
  if (qmi_response == NULL || qmi_response->smsc_address.smsc_address_digits_len == 0)
  {
    return RIL_E_INTERNAL_ERR;
  }

  output_smsc_address.reserve(qmi_response->smsc_address.smsc_address_digits_len + 7);
  /* Add quote to the beginning of the SMSC address */
  output_smsc_address.push_back('"');

  /* QMI already gives ASCII digits */
  for (unsigned idx = 0; idx < qmi_response->smsc_address.smsc_address_digits_len; idx++)
    output_smsc_address.push_back(qmi_response->smsc_address.smsc_address_digits[idx]);

  /* Add a quote to the end of the SMSC address, and a comma */
  output_smsc_address.push_back('"');
  output_smsc_address.push_back(',');

  /* Copy the SMSC address type (3 digits). QMI already gives ASCII digits */
  for (unsigned idx = 0; idx < 3; idx++)
    output_smsc_address.push_back(qmi_response->smsc_address.smsc_address_type[idx]);

  return RIL_E_SUCCESS;
} /* qcril_sms_convert_smsc_address_to_ril_format */


/*===========================================================================

  FUNCTION:  qcril_sms_map_ril_tag_to_qmi_tag

===========================================================================*/
/*!
    @brief
    Convert the tag from RIL format to QMI WMS format

    @return
    None.
*/
/*=========================================================================*/
wms_message_tag_type_enum_v01 qcril_sms_map_ril_tag_to_qmi_tag
(
  int ril_tag
)
{
  wms_message_tag_type_enum_v01 qmi_tag;

  /*-----------------------------------------------------------------------*/

  switch ( ril_tag )
  {
    case 0:
      qmi_tag = WMS_TAG_TYPE_MT_NOT_READ_V01;
      break;

    case 1:
      qmi_tag = WMS_TAG_TYPE_MT_READ_V01;
      break;

    case 2:
      qmi_tag = WMS_TAG_TYPE_MO_NOT_SENT_V01;
      break;

    case 3:
      qmi_tag = WMS_TAG_TYPE_MO_SENT_V01;
      break;

    default:
      QCRIL_LOG_ERROR("Attempting to write an SMS with an unrecognized tag: %d", ril_tag);
      qmi_tag = (wms_message_tag_type_enum_v01) 0xFF;
      break;
  }

  return qmi_tag;

} /* qcril_sms_map_ril_tag_to_qmi_tag */


/*===========================================================================

  FUNCTION:  qcril_sms_is_tag_mo

===========================================================================*/
/*!
    @brief
    Determine if the tag indicates the message is MO or MT.

    @return
    None.
*/
/*=========================================================================*/
boolean qcril_sms_is_tag_mo
(
  int ril_tag
)
{
  boolean is_tag_mo;

  /*-----------------------------------------------------------------------*/

  switch ( ril_tag )
  {
    case 0:
      is_tag_mo = FALSE;
      break;

    case 1:
      is_tag_mo = FALSE;
      break;

    case 2:
      is_tag_mo = TRUE;
      break;

    case 3:
      is_tag_mo = TRUE;
      break;

    default:
      QCRIL_LOG_ERROR("Attempting to write an SMS with an unrecognized tag: %d", ril_tag);
      is_tag_mo = TRUE;
      break;
  }

  return is_tag_mo;

} /* qcril_sms_is_tag_mo */


/*=========================================================================
  FUNCTION:  qcril_sms_lookup_cmd_name

===========================================================================*/
/*!
    @brief
    Look up the name of a QMI WMS command.

    @return
    The string representing the name of a QMI WMS command.
*/
/*=========================================================================*/
const char *qcril_sms_lookup_cmd_name
(
  unsigned long qmi_cmd
)
{
  switch (qmi_cmd)
  {
    case QMI_WMS_SET_PRIMARY_CLIENT_REQ_V01:
      return "QMI_WMS_SET_PRIMARY_CLIENT_REQ";

    case QMI_WMS_SET_ROUTES_REQ_V01:
      return "QMI_WMS_SET_ROUTES_REQ";

    case QMI_WMS_SET_EVENT_REPORT_REQ_V01:
      return "QMI_WMS_SET_EVENT_REPORT_REQ";

    case QMI_WMS_RAW_SEND_REQ_V01:
      return "QMI_WMS_RAW_SEND_REQ";

    case QMI_WMS_SEND_ACK_REQ_V01:
      return "QMI_WMS_SEND_ACK_REQ";

    case QMI_WMS_RAW_WRITE_REQ_V01:
      return "QMI_WMS_RAW_WRITE_REQ";

    case QMI_WMS_MODIFY_TAG_REQ_V01:
      return "QMI_WMS_MODIFY_TAG_REQ";

    case QMI_WMS_DELETE_REQ_V01:
      return "QMI_WMS_DELETE_REQ";

    case QMI_WMS_GET_SMSC_ADDRESS_REQ_V01:
      return "QMI_WMS_GET_SMSC_ADDRESS_REQ";

    case QMI_WMS_SET_SMSC_ADDRESS_REQ_V01:
      return "QMI_WMS_SET_SMSC_ADDRESS_REQ";

    case QMI_WMS_SET_MEMORY_STATUS_REQ_V01:
      return "QMI_WMS_SET_MEMORY_STATUS_REQ";

    case QMI_WMS_SET_BROADCAST_ACTIVATION_REQ_V01:
      return "QMI_WMS_SET_BROADCAST_ACTIVATION_REQ";

    case QMI_WMS_GET_BROADCAST_CONFIG_REQ_V01:
      return "QMI_WMS_GET_BROADCAST_CONFIG_REQ";

    case QMI_WMS_SET_BROADCAST_CONFIG_REQ_V01:
      return "QMI_WMS_SET_BROADCAST_CONFIG_REQ";

    case QMI_WMS_GET_TRANSPORT_LAYER_INFO_REQ_V01:
      return "QMI_WMS_GET_TRANSPORT_LAYER_INFO_REQ";

    case QMI_WMS_GET_SERVICE_READY_STATUS_REQ_V01:
      return "QMI_WMS_GET_SERVICE_READY_STATUS_REQ";

    case QMI_WMS_GET_INDICATION_REGISTER_REQ_V01:
      return "QMI_WMS_GET_INDICATION_REGISTER_REQ";

    default:
      return "Unknown QMI WMS command";
  }

} /* qcril_sms_lookup_cmd_name */


/*=========================================================================
  FUNCTION:  qcril_sms_lookup_ind_name

===========================================================================*/
/*!
    @brief
    Look up the name of a QMI WMS indication.

    @return
    The string representing the name of a QMI WMS indication.
*/
/*=========================================================================*/
const char *qcril_sms_lookup_ind_name
(
  unsigned long qmi_ind
)
{
  switch (qmi_ind)
  {
    case QMI_WMS_EVENT_REPORT_IND_V01:
      return "QMI_WMS_EVENT_REPORT_IND";

    case QMI_WMS_MEMORY_FULL_IND_V01:
      return "QMI_WMS_MEMORY_FULL_IND";

    case QMI_WMS_TRANSPORT_LAYER_INFO_IND_V01:
      return "QMI_WMS_TRANSPORT_LAYER_INFO_IND";

    case QMI_WMS_SERVICE_READY_IND_V01:
      return "QMI_WMS_SERVICE_READY_IND";

    case QMI_WMS_TRANSPORT_LAYER_MWI_IND_V01:
      return "QMI_WMS_TRANSPORT_LAYER_MWI_IND";

    default:
      return "Unknown QMI WMS indication";
  }
} /* qcril_sms_lookup_ind_name */

/*=========================================================================
  FUNCTION:  qcril_sms_hex_char_to_byte

===========================================================================*/
/*!
    @brief
    Convert a hex character to a byte

    @return
    Byte
*/
/*=========================================================================*/
byte qcril_sms_hex_char_to_byte
(
  char hex_char
)
{
  byte byte_value;

  if (hex_char >= 'A' && hex_char <= 'Z')
  {
    hex_char = hex_char + 'a' - 'A';
  }

  if (hex_char >= 'a' && hex_char <= 'f')
  {
    byte_value = (byte)(hex_char - 'a' + 10);
  }
  else if (hex_char >= 'A' && hex_char <= 'F')
  {
    byte_value = (byte)(hex_char - 'A' + 10);
  }
  else if (hex_char >= '0' && hex_char <= '9')
  {
    byte_value = (byte)(hex_char-'0');
  }
  else
  {
    byte_value = 0;
  }

  return (byte_value);
} /* qcril_sms_hex_char_to_byte */

/*=========================================================================
  FUNCTION:  qcril_sms_hex_to_byte

===========================================================================*/
/*!
    @brief
    Convert an SMS PDU from ASCII hex format to a byte array.

    @return
    None
*/
/*=========================================================================*/
void qcril_sms_hex_to_byte
(
  const char * hex_pdu,   // INPUT
  byte * byte_pdu,        // OUTPUT
  uint32 num_hex_chars
)
{
  uint16 buf_pos = 0;
  uint32 i;

  for (i=0; i < num_hex_chars; i++)
  {
    b_packb(qcril_sms_hex_char_to_byte(hex_pdu[i]), byte_pdu, buf_pos, (word)4);
    buf_pos += 4;
  }

} /* qcril_sms_hex_to_byte */


/*=========================================================================
  FUNCTION:  qcril_sms_byte_to_hex_char

===========================================================================*/
/*!
    @brief
    Convert a byte to a hex character.

    @return
    Hex char
*/
/*=========================================================================*/
char qcril_sms_byte_to_hex_char (byte val)
{
  char hex_char;

  if (val <= 9)
  {
    hex_char = (char)(val+'0');
  }
  else if (val >= 10 && val <= 15)
  {
    hex_char = (char)(val-10+'A');
  }
  else
  {
    hex_char = '0';
  }

  return (hex_char);
} /* qcril_sms_byte_to_hex_char */

/*=========================================================================
  FUNCTION:  qcril_sms_byte_to_hex

===========================================================================*/
/*!
    @brief
    Convert a byte array to an SMS PDU in ASCII hex format.

    @return
    None
*/
/*=========================================================================*/
void qcril_sms_byte_to_hex
(
  byte * byte_pdu,   // INPUT
  char * hex_pdu,    // OUTPUT
  uint32 num_bytes
)
{
  uint32 i;
  uint8 nibble;
  uint16 buf_pos = 0;

  for (i=0;i<num_bytes*2;i++)
  {
    nibble = b_unpackb(byte_pdu,buf_pos,4);
    buf_pos += 4;
    hex_pdu[i] = qcril_sms_byte_to_hex_char(nibble);
  }

} /* qcril_sms_byte_to_hex */

/*=========================================================================
FUNCTION
  qcril_sms_convert_ril_to_tl

DESCRIPTION
  This function converts the SMS TL data from RIL to the internal structure.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

=========================================================================*/
void qcril_sms_convert_ril_to_tl
(
  RIL_CDMA_SMS_Message *cdma_sms_msg,
  qcril_sms_tl_message_type * tl_ptr,
  boolean sms_on_ims,
  boolean is_mo_sms
)
{

  if(cdma_sms_msg != NULL && tl_ptr != NULL)
  {
  tl_ptr->mask = 0;

  tl_ptr->is_mo   = is_mo_sms;

  if ( cdma_sms_msg->uTeleserviceID == QCRIL_SMS_TELESERVICE_BROADCAST )
  {
    tl_ptr->tl_message_type = QCRIL_SMS_TL_TYPE_BROADCAST;
  }
  else
  {
    tl_ptr->tl_message_type = QCRIL_SMS_TL_TYPE_POINT_TO_POINT;
  }

  tl_ptr->mask |= QCRIL_SMS_MASK_TL_TELESERVICE_ID;
  tl_ptr->teleservice = cdma_sms_msg->uTeleserviceID;

  if ( cdma_sms_msg->bIsServicePresent == TRUE )
  {
    tl_ptr->mask |= QCRIL_SMS_MASK_TL_BC_SRV_CATEGORY;
    tl_ptr->service = cdma_sms_msg->uServicecategory;
  }

  // address should always be present
  tl_ptr->mask |= QCRIL_SMS_MASK_TL_ADDRESS;
  tl_ptr->address = cdma_sms_msg->sAddress;

  if ( cdma_sms_msg->sSubAddress.number_of_digits != 0 )
  {
    tl_ptr->mask |= QCRIL_SMS_MASK_TL_SUBADDRESS;
    tl_ptr->subaddress = cdma_sms_msg->sSubAddress;
  }

  if (!sms_on_ims)
  {
    /* TL Ack is always requested for CDMA SMS; it should not be requested for IMS */
    tl_ptr->mask |= QCRIL_SMS_MASK_TL_BEARER_REPLY_OPTION;
    tl_ptr->bearer_reply_seq_num = 255;   /* dummy */
  }

  if (cdma_sms_msg->uBearerDataLen != 0)
  {
    tl_ptr->mask |= QCRIL_SMS_MASK_TL_BEARER_DATA;
  }
  }
  else
  {
    QCRIL_LOG_INFO("Invalid Arguments");
  }

} /* qcril_sms_convert_ril_to_tl */


/*=========================================================================
FUNCTION
  qcril_sms_cdma_encode_address

DESCRIPTION
  This function encodes the address

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

=========================================================================*/
void qcril_sms_cdma_encode_address
(
  const RIL_CDMA_SMS_Address  * address_ptr,
  uint8                       * parm_len_ptr,
  uint8                       * data
)
{
  uint32       bit_pos = 0;
  uint8        digit_size;
  uint32       i;

  if( address_ptr != NULL && parm_len_ptr != NULL && data != NULL)
  {
  /* Digit Mode */
  b_packb( (uint8) address_ptr->digit_mode,
           data,
           (uint16) (bit_pos),
           1 );
  bit_pos++;

  /* Number mode */
  b_packb( (uint8) address_ptr->number_mode,
           data,
           (uint16) (bit_pos),
           1 );
  bit_pos++;

  /* Number type */
  if ( address_ptr->digit_mode == RIL_CDMA_SMS_DIGIT_MODE_8_BIT )
  {
    digit_size = 8;

    b_packb( (uint8) address_ptr->number_type,
             data,
             (uint16) (bit_pos),
             3 );
    bit_pos += 3;

    if ( address_ptr->number_mode ==
         RIL_CDMA_SMS_NUMBER_MODE_NOT_DATA_NETWORK )
    {
      b_packb( (uint8) address_ptr->number_plan,
               data,
               (uint16) (bit_pos),
               4 );
      bit_pos += 4;
    }
  }
  else
  {
    digit_size = 4;
  }

  /* Address size */
  b_packb( address_ptr->number_of_digits,
           data,
           (uint16) (bit_pos),
           8 );
  bit_pos += 8;

  /* pack the digits */
  for ( i= 0; i < address_ptr->number_of_digits; i ++ )
  {
    b_packb( address_ptr->digits[i],
             data,
             (uint16) (bit_pos),
             digit_size );
    bit_pos += digit_size;
  }

  if ( bit_pos % 8 != 0 )
  {
    /* pad 0 to remaining bits
    */
    digit_size = (uint8)(8 - bit_pos % 8);
    b_packb( 0, data, (uint16) (bit_pos),  digit_size );
    bit_pos += digit_size;
  }

  *parm_len_ptr = (uint8) (bit_pos/8);
  }
  else
  {
    QCRIL_LOG_INFO("Invalid Arguments");
  }

} /* qcril_sms_cdma_encode_address */


/*=========================================================================
FUNCTION
  qcril_sms_cdma_encode_subaddress

DESCRIPTION
  This function encodes the subaddress

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

=========================================================================*/
void qcril_sms_cdma_encode_subaddress
(
  const RIL_CDMA_SMS_Subaddress  * address_ptr,
  uint8                          * parm_len_ptr,
  uint8                          * data
)
{
  uint32             bit_pos = 0;
  uint8              digit_size;
  uint32       i;

  if(address_ptr != NULL && parm_len_ptr != NULL && data != NULL)
  {
  /* subaddress type */
  b_packb( (uint8) address_ptr->subaddressType,
           data,
           (uint16) (bit_pos),
           3 );
  bit_pos += 3;

  /* Odd flag */
  b_packb( address_ptr->odd,
           data,
           (uint16) (bit_pos),
           1 );
  bit_pos += 1;

  /* Address size */
  b_packb( address_ptr->number_of_digits,
           data,
           (uint16) (bit_pos),
           8 );
  bit_pos += 8;

  /* Digit size */
  digit_size = 8;  /* always 8 */

  /* Pack the digits */
  for ( i=0; i < address_ptr->number_of_digits; i++ )
  {
    b_packb( address_ptr->digits[i],
             data,
             (uint16) (bit_pos),
             digit_size );
    bit_pos += digit_size;
  }

  /* pad last 4 bits with 0 */
  b_packb( 0,
           data,
           (uint16) (bit_pos),
           4 );
  bit_pos += 4;

  /* return len */
  * parm_len_ptr = (uint8) (bit_pos/8);
  }
  else
  {
    QCRIL_LOG_INFO("Invalid Arguments");
  }

} /* qcril_sms_cdma_encode_subaddress */


/*=========================================================================
FUNCTION
  qcril_sms_convert_tl_to_qmi

DESCRIPTION
  This function converts from TL format to QMI format (OTA format).

DEPENDENCIES
  None

RETURN VALUE
  TRUE if successful
  FALSE otherwise

SIDE EFFECTS
  None

=========================================================================*/
boolean qcril_sms_convert_tl_to_qmi
(
  qcril_sms_tl_message_type        * tl_msg_ptr,   /* IN */
  uint32                             raw_bd_len,   /* IN */
  uint8                            * raw_bd_ptr,   /* IN */
  qcril_sms_OTA_message_type       * OTA_msg_ptr   /* OUT */
)
{
  uint16                       pos = 0;
  uint8                        parm_len;
  uint8                        * parm_len_ptr;
  uint8                        * data;

  boolean status = TRUE;

  do
  {
    if(tl_msg_ptr != NULL && raw_bd_ptr != NULL && OTA_msg_ptr != NULL)
    {
  /* Check whether mandatory srv_category is present */
  if ( QCRIL_SMS_TL_TYPE_BROADCAST == tl_msg_ptr->tl_message_type )
  {
    if ( FALSE == (tl_msg_ptr->mask & QCRIL_SMS_MASK_TL_BC_SRV_CATEGORY) )
    {
          status = FALSE;
          break;
    }
  }

  data = (uint8*) OTA_msg_ptr->data;

  /* For the following special cases, there is TL information and raw_bd_ptr
  ** has the OTA data ready
  */
  if ( tl_msg_ptr->mask & QCRIL_SMS_MASK_TL_TELESERVICE_ID )
  {
    if ( tl_msg_ptr->teleservice == QCRIL_SMS_TELESERVICE_MWI ||
         tl_msg_ptr->teleservice == QCRIL_SMS_TELESERVICE_IS91_PAGE ||
         tl_msg_ptr->teleservice == QCRIL_SMS_TELESERVICE_IS91_VOICE_MAIL ||
         tl_msg_ptr->teleservice == QCRIL_SMS_TELESERVICE_IS91_SHORT_MESSAGE )
    {
      OTA_msg_ptr->data_len = (uint16) MIN (raw_bd_len, RIL_CDMA_SMS_BEARER_DATA_MAX);
      memcpy( (uint8 *)OTA_msg_ptr->data, raw_bd_ptr, OTA_msg_ptr->data_len );
          status = TRUE;
          break;
    }
  }

  /* msg type
  */
  data[0] = (uint8) tl_msg_ptr->tl_message_type;
  pos ++;

  /* the remaining parameters have one or more of the following:
     - PARAMETER_ID    8 bits
     - PARAMETER_LEN   8 bits
     - Parameter Data  8 x PARAMETER_LEN
  */

  if ( tl_msg_ptr->mask & QCRIL_SMS_MASK_TL_TELESERVICE_ID )
  {
    data[pos] = (uint8) QCRIL_SMS_TL_TELESERVICE_ID;
    pos++; /* skip parm id */

    data[pos] = parm_len = 2;
    pos++; /* skip len */

    b_packw( (uint16) tl_msg_ptr->teleservice,
             data,
             (uint16) (pos * 8),
             parm_len * 8 );

    pos += parm_len;  /* skip parm data */
  }

  if ( tl_msg_ptr->mask & QCRIL_SMS_MASK_TL_BC_SRV_CATEGORY )
  {
    data[pos] = (uint8) QCRIL_SMS_TL_BC_SRV_CATEGORY;
    pos++; /* skip parm id */

    data[pos] = parm_len = 2;
    pos++; /* skip len */

    b_packw( (uint16) (tl_msg_ptr->service),
             data,
             (uint16) (pos * 8),
             parm_len * 8 );

    pos += parm_len;  /* skip parm data */
  }

  if ( tl_msg_ptr->mask & QCRIL_SMS_MASK_TL_ADDRESS )
  {
    data[pos] = (uint8) (tl_msg_ptr->is_mo ? QCRIL_SMS_TL_DEST_ADDRESS : QCRIL_SMS_TL_ORIG_ADDRESS);
    pos++; /* skip parm id */

    parm_len_ptr = data + pos;   /* will set value later */
    pos ++; /* skip parm len */

    if(tl_msg_ptr->address.number_of_digits > RIL_CDMA_SMS_ADDRESS_MAX)
    {
        status = FALSE;
        break;
    }

    qcril_sms_cdma_encode_address( & tl_msg_ptr->address,
                                   parm_len_ptr,
                                   data+pos );
    pos += * parm_len_ptr; /* skip parm data */
  }

  if ( tl_msg_ptr->mask & QCRIL_SMS_MASK_TL_SUBADDRESS )
  {
    data[pos] = (uint8) (tl_msg_ptr->is_mo ? QCRIL_SMS_TL_DEST_SUBADDRESS : QCRIL_SMS_TL_ORIG_SUBADDRESS);
    pos++; /* skip parm id */

    parm_len_ptr = data + pos;   /* will set value later */
    pos ++; /* skip parm len */

    if(tl_msg_ptr->subaddress.number_of_digits > RIL_CDMA_SMS_SUBADDRESS_MAX)
    {
        status = FALSE;
        break;
    }

    qcril_sms_cdma_encode_subaddress( & tl_msg_ptr->subaddress,
                                      parm_len_ptr,
                                      data+pos );
    pos += * parm_len_ptr;
  }

  if ( tl_msg_ptr->mask & QCRIL_SMS_MASK_TL_BEARER_REPLY_OPTION )
  {
    data[pos] = (uint8) QCRIL_SMS_TL_BEARER_REPLY_OPTION;
    pos++; /* skip parm id */

    data[pos] = parm_len = 1;
    pos++;

    b_packb( tl_msg_ptr->bearer_reply_seq_num,
             data,
             (uint16) (pos * 8),
             6 );  /* high 6 bits */

    b_packb( 0,
             data,
             (uint16) (pos * 8 + 6),
             2 );  /* low 2 bits: reserved, set to 0 */

    pos += parm_len;  /* skip parm data */
  }

  if ( tl_msg_ptr->mask & QCRIL_SMS_MASK_TL_BEARER_DATA )
  {
    if ( raw_bd_len == 0 || raw_bd_len > RIL_CDMA_SMS_BEARER_DATA_MAX )
    {
      QCRIL_LOG_ERROR( "Invalid bd len: %d", (int)raw_bd_len);
      status = FALSE;
    }
    else
    {
      data[pos] = (uint8) QCRIL_SMS_TL_BEARER_DATA;
      pos++; /* skip parm id */

      data[pos] = (uint8) MIN (raw_bd_len, RIL_CDMA_SMS_BEARER_DATA_MAX);
      pos++; /* skip parm len */

      memcpy( data+pos, raw_bd_ptr, MIN((uint8)raw_bd_len, (uint8)RIL_CDMA_SMS_BEARER_DATA_MAX-pos) );
      pos += (uint16) raw_bd_len; /* skip parm data */
    }
  }

  OTA_msg_ptr->data_len = pos;
  /* data was filled in the above */

  if ( status == TRUE && pos > QCRIL_SMS_TL_MAX_LEN )
  {
    // encoded data has too many bytes
    QCRIL_LOG_ERROR( "Invalid parm size: %d", pos);
    status = FALSE;
  }
    }
    else
    {
      status = FALSE;
    }
  }while(0);

  return status;

} /* qcril_sms_convert_tl_to_qmi */


/*===========================================================================

FUNCTION    qcril_sms_convert_sms_ril_to_qmi

DESCRIPTION
  Convert the SMS from RIL format to QMI format

DEPENDENCIES
  None

RETURN VALUE
  TRUE if successful
  FALSE otherwise

SIDE EFFECTS


===========================================================================*/
boolean qcril_sms_convert_sms_ril_to_qmi
(
  RIL_CDMA_SMS_Message        *cdma_sms_msg,
  uint8                       *data_buf,
  uint16                       data_buf_len,
  uint16                      *ota_data_len,
  boolean                     sms_on_ims,
  boolean                     is_mo_sms
)
{
  qcril_sms_OTA_message_type   ota_data;
  qcril_sms_tl_message_type    tl_msg;
  boolean status = FALSE;

  if(cdma_sms_msg != NULL && data_buf != NULL && ota_data_len != NULL)
  {
  (void) memset((uint8 *)ota_data.data, 0, sizeof(ota_data.data));
  (void) memset(&tl_msg, 0, sizeof(tl_msg));

  qcril_sms_convert_ril_to_tl(cdma_sms_msg, &tl_msg, sms_on_ims, is_mo_sms);
  status = qcril_sms_convert_tl_to_qmi(&tl_msg,
                                       cdma_sms_msg->uBearerDataLen,
                                       cdma_sms_msg->aBearerData,
                                       &ota_data);
  if (status)
  {
    if (NULL != ota_data_len)
    {
      *ota_data_len = ota_data.data_len;
    }
    if (NULL != data_buf && data_buf_len > 0)
    {
      (void) memcpy((uint8 *)data_buf, (uint8 *)ota_data.data, MIN(data_buf_len, ota_data.data_len));
    }
  }
  }

  return status;
} /* qcril_sms_convert_sms_ril_to_qmi */


/*=========================================================================
FUNCTION
  qcril_sms_decode_address

DESCRIPTION
  This function decodes the address

DEPENDENCIES
  None

RETURN VALUE
  TRUE if successful
  FALSE otherwise

SIDE EFFECTS
  None

=========================================================================*/
boolean qcril_sms_decode_address
(
  const uint8             * data,
  uint8                     parm_len,
  RIL_CDMA_SMS_Address    * address_ptr
)
{
  uint32     bit_pos = 0;
  uint8      digit_size;
  uint32     i;
  boolean    status = TRUE;

  if(address_ptr != NULL && data != NULL)
  {
  /* Digit mode */
  address_ptr->digit_mode = (RIL_CDMA_SMS_DigitMode)
                            b_unpackb( (uint8*) data, (uint16) (bit_pos), 1 );
  bit_pos ++;

  /* Number mode */
  address_ptr->number_mode = (RIL_CDMA_SMS_NumberMode)
                             b_unpackb( (uint8*) data, (uint16) (bit_pos), 1 );
  bit_pos ++;


  /* Number type, and determine digit size */
  if ( address_ptr->digit_mode == RIL_CDMA_SMS_DIGIT_MODE_8_BIT )
  {
    address_ptr->number_type = (RIL_CDMA_SMS_NumberType)
                               b_unpackb( (uint8*) data, (uint16) (bit_pos), 3 );
    bit_pos += 3;

    digit_size = 8;
  }
  else
  {
    address_ptr->number_type = RIL_CDMA_SMS_NUMBER_TYPE_UNKNOWN;
    digit_size = 4;
  }


  /* Number plan */
  if ( address_ptr->digit_mode  == RIL_CDMA_SMS_DIGIT_MODE_8_BIT &&
       address_ptr->number_mode == RIL_CDMA_SMS_NUMBER_MODE_NOT_DATA_NETWORK )
  {
    address_ptr->number_plan = (RIL_CDMA_SMS_NumberPlan)
                               b_unpackb( (uint8*) data, (uint16) (bit_pos), 4);
    bit_pos += 4;
  }
  else
  {
    /* number plan is not used */
    address_ptr->number_plan = RIL_CDMA_SMS_NUMBER_PLAN_UNKNOWN;
  }

  /* Address size */
  address_ptr->number_of_digits=
  b_unpackb( (uint8*) data, (uint16) (bit_pos), 8 );
  bit_pos += 8;


  /* Extract all digits:
  */
  if ( address_ptr->number_of_digits > RIL_CDMA_SMS_ADDRESS_MAX )
  {
    QCRIL_LOG_ERROR( "decoding: address digits number too big: %d",
                     address_ptr->number_of_digits);
    status = FALSE;
  }
  else
  {
    for ( i=0; i < address_ptr->number_of_digits; i++ )
    {
      address_ptr->digits[i] = b_unpackb( (uint8*) data,
                                          (uint16) (bit_pos),
                                          digit_size );
      bit_pos += digit_size;
    }
  }

  /* Parm len checking */
  if ( bit_pos > (uint32)(parm_len*8) )
  {
    QCRIL_LOG_ERROR( "decoding: address too long: %d>%d",
                     bit_pos, parm_len*8);
    status = FALSE;
  }
  }
  else
  {
    status = FALSE;
  }

  return status;

} /* qcril_sms_decode_address */


/*=========================================================================
FUNCTION
  qcril_sms_decode_subaddress

DESCRIPTION
  This function decodes the subaddress

DEPENDENCIES
  None

RETURN VALUE
  TRUE if successful
  FALSE otherwise

SIDE EFFECTS
  None

=========================================================================*/
boolean qcril_sms_decode_subaddress
(
  const uint8                * data,
  uint8                        parm_len,
  RIL_CDMA_SMS_Subaddress    * address_ptr
)
{
  uint32     bit_pos = 0;
  uint8      digit_size;
  uint32     i;
  boolean    status = TRUE;

  if(address_ptr != NULL && data != NULL)
  {
  /* address type */
  address_ptr->subaddressType = (RIL_CDMA_SMS_SubaddressType)
                                b_unpackb( (uint8*) data, (uint16) (bit_pos), 3 );
  bit_pos += 3;

  /* Odd flag */
  address_ptr->odd = b_unpackb( (uint8*) data, (uint16) (bit_pos), 1 );
  bit_pos += 1;

  /* Address size */
  address_ptr->number_of_digits = b_unpackb( (uint8*) data, (uint16) (bit_pos), 8 );
  bit_pos += 8;

  /* Extract all digits:
  */
  if ( address_ptr->number_of_digits > RIL_CDMA_SMS_SUBADDRESS_MAX )
  {
    QCRIL_LOG_ERROR( "decoding: subaddress digits number too big: %d",
                     address_ptr->number_of_digits);
    status = FALSE;
  }
  else
  {
    digit_size = 8;  /* always 8 bits so far */

    for ( i=0; i < address_ptr->number_of_digits; i++ )
    {
      address_ptr->digits[i] = b_unpackb( (uint8*) data,
                                          (uint16) (bit_pos),
                                          digit_size );
      bit_pos += digit_size;
    }
  }

  /* parm len check */
  if ( bit_pos > (uint32)(parm_len*8) )
  {
    QCRIL_LOG_ERROR( "decoding: subaddress too long: %d>%d",
                     bit_pos, parm_len*8);
    status = FALSE;
  }
  }
  else
  {
    status = FALSE;
  }

  return status;

} /* qcril_sms_decode_subaddress */


/*=========================================================================
FUNCTION
  qcril_sms_convert_qmi_to_tl

DESCRIPTION
  This function converts from QMI format (OTA format) to TL format.

DEPENDENCIES
  None

RETURN VALUE
  TRUE if successful
  FALSE otherwise

SIDE EFFECTS
  None

=========================================================================*/
boolean qcril_sms_convert_qmi_to_tl
(
  qcril_sms_OTA_message_type     * OTA_msg_ptr,   /* IN */
  qcril_sms_tl_message_type      * tl_ptr,        /* OUT */
  uint32                         * raw_bd_len,    /* OUT */
  uint8                          * raw_bd_ptr     /* OUT */
)
{
  uint16                    pos;      /* running position in bytes */
  qcril_sms_tl_parm_id_e_type parm_id = QCRIL_SMS_TL_DUMMY;
  uint8                    parm_len;
  uint16                    tl_len;  /* len of TL data */
  boolean status = TRUE,return_back = FALSE;

  do
  {
    if(tl_ptr != NULL && raw_bd_ptr != NULL && OTA_msg_ptr != NULL && OTA_msg_ptr->data_len > TL_HEADER_SIZE)
    {
  if ( OTA_msg_ptr->data_len > RIL_CDMA_SMS_BEARER_DATA_MAX )
  {
    QCRIL_LOG_ERROR("** TL Msg len too long: %d", OTA_msg_ptr->data_len);
    status = FALSE;
  }
  else
  {
    tl_len = OTA_msg_ptr->data_len;
    pos = 0;

    /* ---- start decoding ---- */

    tl_ptr->mask = 0;

    /* the first byte is msg type */

    if (OTA_msg_ptr->data[pos] > (uint8) QCRIL_SMS_TL_TYPE_MAX )
    {
      QCRIL_LOG_ERROR("WARNING: Invalid TL msg type: %d", OTA_msg_ptr->data[pos]);
          status = FALSE;
          break;
    }
    else
    {
      tl_ptr->tl_message_type = (qcril_sms_tl_message_type_e_type)(OTA_msg_ptr->data[pos]);
      pos ++;
    }

    /* the remain data has one or more of the following:
       - PARAMETER_ID    8 bits
       - PARAMETER_LEN   8 bits
       - Parameter Data  8 x PARAMETER_LEN
    */

    while ( status == TRUE  )
    {

      if ( pos == tl_len )
      {
        /* Good. Done with parameter processing successfully */
        break; /* out of while loop */
      }
      else if ( pos + TL_PARM_SIZE > tl_len )
      {
        /* Current position goes beyond the msg size. */
        QCRIL_LOG_ERROR( "decoding: msg size %d incorrect", tl_len);
        status = FALSE;
        break; /* out of while loop */
      }

      parm_id = (qcril_sms_tl_parm_id_e_type) ( OTA_msg_ptr->data[ pos ] );
      pos ++;  /* skip parm id */

      parm_len = OTA_msg_ptr->data[ pos ];
      pos ++;  /* skip parm len */

      if ( pos + parm_len > tl_len )
      {
        /* parm data passes the max length of bearer data
        */
        QCRIL_LOG_ERROR( "decoding: parameter (id=%d) extends beyond msg size %d",
                         parm_id, tl_len);
        status = FALSE;
        break; /* out of while loop */
      }
      else if(pos >= RIL_CDMA_SMS_BEARER_DATA_MAX)
      {
        /* current position exceeds maximum msg size
        */
        QCRIL_LOG_ERROR( "decoding: current position %u exceeds maximum msg size %u",
                         pos,RIL_CDMA_SMS_BEARER_DATA_MAX);
        status = FALSE;
        break; /* out of while loop */
      }



      /* Now pos should point to the parm data */
      /* After each case below, pos should point to the next parm Id */

      switch ( parm_id )
      {
        case QCRIL_SMS_TL_TELESERVICE_ID:

          if ( tl_ptr->mask & QCRIL_SMS_MASK_TL_TELESERVICE_ID )
          {
            MSG_DUP_PARM;
            pos += parm_len;    /* skip parm data */
            break;              /* continue with next parm */
          }

          tl_ptr->mask |= QCRIL_SMS_MASK_TL_TELESERVICE_ID;

          if ( pos  + 2 > tl_len ||
               parm_len != 2
             )
          {
            /* tl data too short , or bad len field */
            status = FALSE;
          }
          else
          {
            tl_ptr->teleservice = (qcril_sms_teleservice_e_type)
                                  b_unpackw( (uint8*) OTA_msg_ptr->data,
                                             (uint16) (pos*8),
                                             parm_len*8 );
            pos += parm_len; /* skip parm data */
          }

          break;


        case QCRIL_SMS_TL_BC_SRV_CATEGORY:

          if ( tl_ptr->mask & QCRIL_SMS_MASK_TL_BC_SRV_CATEGORY )
          {
            MSG_DUP_PARM;
            pos += parm_len;    /* skip parm data */
            break;              /* continue with next parm */
          }


          tl_ptr->mask |= QCRIL_SMS_MASK_TL_BC_SRV_CATEGORY;

          if ( pos  + 2 > tl_len ||
               parm_len != 2
             )
          {
            /* tl data too short , or bad len field */
            status = FALSE;
          }
          else
          {
            tl_ptr->service = b_unpackw( (uint8*) OTA_msg_ptr->data,
                                         (uint16) (pos*8),
                                         parm_len*8 );
            pos += parm_len; /* skip parm data */
          }

          break;

        case QCRIL_SMS_TL_ORIG_ADDRESS:
        case QCRIL_SMS_TL_DEST_ADDRESS:

          if ( tl_ptr->mask & QCRIL_SMS_MASK_TL_ADDRESS )
          {
            MSG_DUP_PARM;
            pos += parm_len;    /* skip parm data */
            break;              /* continue with next parm */
          }


          tl_ptr->mask |= QCRIL_SMS_MASK_TL_ADDRESS;

          /* Set the MO/MT tag. */
          tl_ptr->is_mo = ( parm_id == QCRIL_SMS_TL_DEST_ADDRESS ) ?
                          TRUE : FALSE;

          status = qcril_sms_decode_address( (uint8*) OTA_msg_ptr->data+pos,
                                             parm_len,
                                             & tl_ptr->address );
          pos += parm_len;
          break;

        case QCRIL_SMS_TL_ORIG_SUBADDRESS:
        case QCRIL_SMS_TL_DEST_SUBADDRESS:

          if ( tl_ptr->mask & QCRIL_SMS_MASK_TL_SUBADDRESS )
          {
            MSG_DUP_PARM;
            pos += parm_len;    /* skip parm data */
            break;              /* continue with next parm */
          }


          tl_ptr->mask |= QCRIL_SMS_MASK_TL_SUBADDRESS;

          /* Set the MO/MT tag. */
          tl_ptr->is_mo = ( parm_id == QCRIL_SMS_TL_DEST_SUBADDRESS ) ?
                          TRUE : FALSE;

          status = qcril_sms_decode_subaddress( (uint8*) OTA_msg_ptr->data+pos,
                                                parm_len,
                                                & tl_ptr->subaddress );

          pos += parm_len;  /* skip parm data */
          break;

        case QCRIL_SMS_TL_BEARER_REPLY_OPTION:

          if ( tl_ptr->mask & QCRIL_SMS_MASK_TL_BEARER_REPLY_OPTION )
          {
            MSG_DUP_PARM;
            pos += parm_len;    /* skip parm data */
            break;              /* continue with next parm */
          }


          tl_ptr->mask |= QCRIL_SMS_MASK_TL_BEARER_REPLY_OPTION;

              return_back = FALSE;
          if ( parm_len != 1 )
          {
            QCRIL_LOG_ERROR("decoding: bearer reply option len is not 1: %d. Reset to 1.", parm_len);
                status = FALSE;
                return_back = TRUE;
          }
              else
              {
          tl_ptr->bearer_reply_seq_num = OTA_msg_ptr->data[pos] >> 2;
          /* get high 6 bits */

          pos += parm_len; /* skip parm data */
              }

          break;


        case QCRIL_SMS_TL_BEARER_DATA:

          if ( tl_ptr->mask & QCRIL_SMS_MASK_TL_BEARER_DATA )
          {
            MSG_DUP_PARM;
            pos += parm_len;    /* skip parm data */
            break;              /* continue with next parm */
          }


          /* bearer data is not decoded here */
          /* it will be copied to the output */

          tl_ptr->mask |= QCRIL_SMS_MASK_TL_BEARER_DATA;

          if ( parm_len == 0 )
          {
            QCRIL_LOG_ERROR("decoding: bearer data len is 0");
            status = FALSE;
          }
          /* parm_len is uint8, so it won't be greater than WMS_MAX_LEN */
          else if ( pos + parm_len > tl_len )
          {
            QCRIL_LOG_ERROR("decoding: bearer data len %d extends beyond msg size %d",
                            parm_len, tl_len);
            status = FALSE;
          }
          else
          {
            /* copy bearer data to output
            */
            *raw_bd_len = MIN (parm_len, RIL_CDMA_SMS_BEARER_DATA_MAX);
            if((*raw_bd_len + pos) <= RIL_CDMA_SMS_BEARER_DATA_MAX)
            {
                memcpy( raw_bd_ptr, (uint8*)OTA_msg_ptr->data + pos, *raw_bd_len );
            }
          }

          pos += parm_len; /* skip parm data */

          break;

        default:
          QCRIL_LOG_ERROR( "decoding: Invalid parm id: %d", parm_id);
          pos += parm_len; /* skip parm data */
          break;           /* continue with next parm */

      } /* switch */
          if(return_back == TRUE)
          {
            break;
          }
    } /* while */
        if(return_back == TRUE)
        {
          break;
        }


    /* Null the raw bd */
    if ( ! ( tl_ptr->mask & QCRIL_SMS_MASK_TL_BEARER_DATA ) )
    {
      *raw_bd_len = 0;
    }


    /* -------- check CDMA TL mandatory fields -------- */
    if ( status == TRUE )
    {
      switch ( tl_ptr->tl_message_type )
      {
        case QCRIL_SMS_TL_TYPE_POINT_TO_POINT:

          if ( ! ( tl_ptr->mask & QCRIL_SMS_MASK_TL_ADDRESS ) )
          {
            QCRIL_LOG_ERROR("No address present in msg!");
            status = FALSE;
          }

          break;

        case QCRIL_SMS_TL_TYPE_BROADCAST:

          if ( ! ( tl_ptr->mask & QCRIL_SMS_MASK_TL_BC_SRV_CATEGORY ) )
          {
            QCRIL_LOG_ERROR("decoding: broadcast category is not present!");
            status = FALSE;
          }

          break;

        case QCRIL_SMS_TL_TYPE_ACK:

          if ( ! ( tl_ptr->mask & QCRIL_SMS_MASK_TL_CAUSE_CODES ) )
          {
            QCRIL_LOG_ERROR("decoding: cause code is not present in Ack!");
            status = FALSE;
          }

          break;

        default:

          QCRIL_LOG_ERROR( "decoding: invalid TL msg type: %d",
                           tl_ptr->tl_message_type);
          status = FALSE;
          break;

      } /* switch */

    } /* end of CDMA TL fields checking */
  }
    }
    else
    {
      status = FALSE;
    }
  }while(0);


  return status;

} /* qcril_sms_convert_qmi_to_tl */


/*=========================================================================
FUNCTION
  qcril_sms_convert_tl_to_ril

DESCRIPTION
  This function converts the SMS data from TL format to RIL format

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

=========================================================================*/
void qcril_sms_convert_tl_to_ril
(
  const qcril_sms_tl_message_type * tl_ptr,        /* IN */
  RIL_CDMA_SMS_Message            * cdma_sms_msg   /* OUT */
)
{

  do
  {
    if(tl_ptr == NULL || cdma_sms_msg == NULL)
    {
      break;
    }
  /* msg type
  */
  if ( tl_ptr->tl_message_type == QCRIL_SMS_TL_TYPE_BROADCAST )
  {
    cdma_sms_msg->uTeleserviceID  = QCRIL_SMS_TELESERVICE_BROADCAST;
  }
  else if ( tl_ptr->tl_message_type == QCRIL_SMS_TL_TYPE_POINT_TO_POINT )
  {
    /* teleservice id
    */
    if ( tl_ptr->mask & QCRIL_SMS_MASK_TL_TELESERVICE_ID )
    {
      cdma_sms_msg->uTeleserviceID = tl_ptr->teleservice;
    }
    else
    {
      cdma_sms_msg->uTeleserviceID = QCRIL_SMS_TELESERVICE_UNKNOWN;
    }
  }
  else
  {
    /* it is an ACK msg; not applicable for client msg */
      break;  /* SHORT-RETURN */
  }


  /* address
  */
  if ( tl_ptr->mask & QCRIL_SMS_MASK_TL_ADDRESS )
  {
    cdma_sms_msg->sAddress = tl_ptr->address;
  }
  else
  {
    cdma_sms_msg->sAddress.number_of_digits = 0;
  }

  /* sub-address
  */
  if ( tl_ptr->mask & QCRIL_SMS_MASK_TL_SUBADDRESS )
  {
    cdma_sms_msg->sSubAddress = tl_ptr->subaddress;
  }
  else
  {
    cdma_sms_msg->sSubAddress.number_of_digits = 0;
  }

  /* broadcast category
  */
  if ( tl_ptr->mask & QCRIL_SMS_MASK_TL_BC_SRV_CATEGORY )
  {
    cdma_sms_msg->bIsServicePresent = TRUE;
    cdma_sms_msg->uServicecategory = tl_ptr->service;
  }
  else
  {
    cdma_sms_msg->bIsServicePresent = FALSE;
    cdma_sms_msg->uServicecategory = 0;  /* 0 means unspecified */
  }
  }while(0);

  return;

} /* qcril_sms_convert_tl_to_ril */


/*===========================================================================

FUNCTION    qcril_sms_convert_mt_sms_qmi_to_ril

DESCRIPTION
  Convert the MT SMS from QMI format to RIL format

DEPENDENCIES
  None

RETURN VALUE
  TRUE if successful
  FALSE otherwise

SIDE EFFECTS


===========================================================================*/
boolean qcril_sms_convert_mt_sms_qmi_to_ril
(
  const uint8                 *data,
  uint16                      length,
  RIL_CDMA_SMS_Message        *cdma_sms_msg
)
{
  qcril_sms_OTA_message_type   ota_data;
  qcril_sms_tl_message_type    tl_msg;
  boolean status = TRUE;

  if(cdma_sms_msg != NULL && data != NULL)
  {
  /* reset static structures */
  (void) memset((uint8 *)ota_data.data, 0, sizeof(ota_data.data));
  (void) memset(&tl_msg, 0, sizeof(tl_msg));

  ota_data.data_len  = MIN(length, RIL_CDMA_SMS_BEARER_DATA_MAX);
  (void) memcpy((uint8 *)ota_data.data, (uint8 *)data, ota_data.data_len);

  status = qcril_sms_convert_qmi_to_tl(&ota_data,
                                       &tl_msg,
                                       (uint32 *) &cdma_sms_msg->uBearerDataLen,
                                       cdma_sms_msg->aBearerData);
  if (status == TRUE)
  {
    qcril_sms_convert_tl_to_ril(&tl_msg, cdma_sms_msg);
    }
  }
  else
  {
    status = FALSE;
  }

  return status;
} /* qcril_sms_convert_mt_sms_qmi_to_ril */
//===========================================================================
//qmi_ril_set_sms_svc_status
//===========================================================================
void qmi_ril_set_sms_svc_status(qmi_ril_sms_svc_status_type new_status)
{
  QCRIL_LOG_INFO( "new_status %d", new_status );
  qmi_ril_sms_svc_status = new_status;
} // qmi_ril_set_sms_svc_status
//===========================================================================
//qmi_ril_get_sms_svc_status
//===========================================================================
qmi_ril_sms_svc_status_type qmi_ril_get_sms_svc_status(void)
{
  return qmi_ril_sms_svc_status;
} // qmi_ril_get_sms_svc_status

//===========================================================================
//qmi_ril_set_primary_client_status
//===========================================================================
void qmi_ril_set_primary_client_status(qmi_ril_sms_primary_client_status_type new_status)
{
  QCRIL_LOG_INFO( "new_status %d", new_status );
  qmi_ril_sms_primary_client_status = new_status;
} // qmi_ril_set_primary_client_status
//===========================================================================
//qmi_ril_get_primary_client_status
//===========================================================================
qmi_ril_sms_primary_client_status_type qmi_ril_get_primary_client_status(void)
{
  return qmi_ril_sms_primary_client_status;
} // qmi_ril_get_primary_client_status

//=========================================================================================
//qmi_ril_block_mo_sms_on_1x
//
//returns TRUE if 3gpp2 MO SMS needs to be blocked
//conditions to be met:
//User sets persist.vendor.radio.block_sms_on_1x to "1", Target is NOT SVLTE capable and IMS is not registered but LTE service is available
//=========================================================================================
int qmi_ril_block_mo_sms_on_1x()
{
  qcril_sms_struct_type *i_ptr;
  int ret = FALSE;

  QCRIL_LOG_FUNC_ENTRY();

  i_ptr = &qcril_sms[ QCRIL_DEFAULT_INSTANCE_ID ];
  QCRIL_MUTEX_LOCK( &i_ptr->transport_layer_info_mutex, NULL );
  QCRIL_LOG_INFO( "transport layer info status block %d, valid %d value %d", i_ptr->transport_layer_info.block_sms_on_1x,
                                                               i_ptr->transport_layer_info.registered_ind_valid,
                                                               i_ptr->transport_layer_info.registered_ind );
  if( TRUE == i_ptr->transport_layer_info.block_sms_on_1x )
  {
    if( !(QMI_DMS_MASK_SVLTE_CAPABLE_V01 & qcril_qmi_nas_retrieve_sv_capability()) &&
        (FALSE == i_ptr->transport_layer_info.registered_ind_valid ||
        FALSE == i_ptr->transport_layer_info.registered_ind ) &&
        (RADIO_TECH_LTE == qcril_qmi_nas_retrieve_reported_data_technology() ||
         RADIO_TECH_LTE_CA == qcril_qmi_nas_retrieve_reported_data_technology() )
      )
    {
      ret = TRUE;
    }
  }
  QCRIL_MUTEX_UNLOCK( &i_ptr->transport_layer_info_mutex, NULL );

  if( FALSE == ret )
  {
    ret = qcril_qmi_nas_is_1x_sms_blocked();
  }

  QCRIL_LOG_FUNC_RETURN_WITH_RET(ret);
  return ret;
} // qmi_ril_block_mo_sms_on_1x

//============================================================================
// FUNCTION: set_wms_ready_state <state>
//
// DESCRIPTION:
// Sets protected cache value wms_ready_state and logs its new value
//
// RETURN: None
//============================================================================
void set_wms_service_state(wms_service_ready_status_enum_v01 state) {

   SMS_CACHE_LOCK;
   wms_service_state = state;
   if(!is_wms_limited_ready_status)
   {
       is_wms_limited_ready_status = wms_ready_limited_status_support(state);
   }

   QCRIL_LOG_INFO("WMS limited service support available %d", is_wms_limited_ready_status);
   SMS_CACHE_UNLOCK;

    if( ( QMI_RIL_SMS_PRIMARY_CLIENT_NOT_SET == qmi_ril_get_primary_client_status() ) &&
        wms_ready_atleast_limited_3gpp()
      )
    {
        qcril_setup_timed_callback(QCRIL_DEFAULT_INSTANCE_ID,
                                  QCRIL_DEFAULT_MODEM_ID,
                                  qcril_qmi_sms_set_primary_client,
                                  NULL,
                                  NULL);
    }

   /* if if wms status is becoming none, then clear sms ack cache so that
      even if there are any un acked sms's qcril won't cause it to block further sms
      when wms reports status as fine */
   if (WMS_SERVICE_READY_STATUS_NONE_V01 == state)
   {
       qcril_setup_timed_callback(QCRIL_DEFAULT_INSTANCE_ID,
                                  QCRIL_DEFAULT_MODEM_ID,
                                  qcril_sms_mt_transaction_expired,
                                  NULL,
                                  NULL);
   }

   QCRIL_LOG_INFO("service ready state updated: %s", sms_state_str(state) );

}

//============================================================================
// FUNCTION: wms_ready_supported
//
// DESCRIPTION: Queries modem to determine whether wms ready indications
// are supported or not
// Note: we use the indication registration as verification of support
//
// RETURN: 0 | 1 --> not supported, supported
//============================================================================
int wms_ready_supported(void) {

   int result = 0;
   qmi_client_error_type qmi_err;

   QCRIL_LOG_FUNC_ENTRY();

   wms_get_indication_register_resp_msg_v01 wms_resp;
   bzero(&wms_resp, sizeof(wms_resp));

   qmi_err = qmi_client_sms_send_sync(
                      QMI_WMS_GET_INDICATION_REGISTER_REQ_V01,
                      NULL,
                      0,
                      (void*) &wms_resp,
                      sizeof(wms_resp) );

   if ( qmi_err != QMI_NO_ERR)
   {
      QCRIL_LOG_ERROR("Failed wms get indication register request w/%s",
                                                          qmi_errstr(qmi_err));
      result = 0;
   }
   else {
      if ( ! (wms_resp.reg_service_ready_events_valid
              && (wms_resp.reg_service_ready_events == 0x01) ) ) {
         QCRIL_LOG_WARN( "wms service ready not supported, assuming ready.");
         result = 0;
      }
      else {
         QCRIL_LOG_WARN( "wms service ready is supported.");
         result = 1;
      }
   }

   QCRIL_LOG_FUNC_RETURN_WITH_RET(result);

   return result;
}

//============================================================================
// FUNCTION: query_wms_ready_status
//
// DESCRIPTION: Queries modem for wms ready status
// We use this to capture the status in case we missed an indication set
// while starting up before we've had a chance to register for it
//
// RETURN: None
//============================================================================
void query_wms_ready_status(void)
{
   qmi_client_error_type qmi_err;

   QCRIL_LOG_FUNC_ENTRY();

   qmi_err = qmi_client_sms_send_async(
                     QMI_WMS_GET_SERVICE_READY_STATUS_REQ_V01,
                     NULL,
                     0,
                     sizeof(wms_get_service_ready_status_resp_msg_v01),
                     wms_get_ready_status_cb,
                     NULL);

   if ( qmi_err != QMI_NO_ERR )
   {
      QCRIL_LOG_ERROR("Failed wms service ready status request w/%s",
                                                      qmi_errstr(qmi_err));
   }
   else
   {
      QCRIL_LOG_INFO("QMI_WMS_GET_SERVICE_READY_STATUS_REQ_V01 sent to modem");
   }

   QCRIL_LOG_FUNC_RETURN();
}

//============================================================================
// FUNCTION: wms_get_ready_status_cb <parms type> <parms ptr>
//
// DESCRIPTION:
// Callback handler for async wms_get_ready_status msg
// sets internal cache wms_service_state
//
// RETURN: None
//============================================================================
void wms_get_ready_status_cb
(
    unsigned int           msg_id,
    std::shared_ptr<void>  resp_c_struct,
    unsigned int           resp_c_struct_len,
    void                   *resp_cb_data,
    qmi_client_error_type  transp_err
)
{
   QCRIL_LOG_FUNC_ENTRY();

   QCRIL_ASSERT( resp_c_struct != NULL );

   QCRIL_NOTUSED(transp_err);
   QCRIL_NOTUSED(resp_cb_data);
   QCRIL_NOTUSED(resp_c_struct_len);
   QCRIL_NOTUSED(msg_id);

   wms_get_service_ready_status_resp_msg_v01* wms_resp
              = (wms_get_service_ready_status_resp_msg_v01*) resp_c_struct.get();

   if (!wms_resp) {
      QCRIL_LOG_ERROR("Unexpected Null pointer in response message");
      QCRIL_LOG_FUNC_RETURN();
      return;
   }

   if (wms_resp->resp.result != QMI_RESULT_SUCCESS_V01) {
      QCRIL_LOG_ERROR("wms_get_service_ready_status returned w/%s",
                                          qmisvc_errstr(wms_resp->resp.error));
      QCRIL_LOG_FUNC_RETURN();
      return;
   }

   if (!wms_resp->ready_status_valid) {
      QCRIL_LOG_WARN( "wms service ready tlv missing, assuming ready");
      set_wms_service_state(WMS_SERVICE_READY_STATUS_3GPP_AND_3GPP2_V01);
   }
   else {
      set_wms_service_state(wms_resp->ready_status);
   }

   QCRIL_LOG_FUNC_RETURN();
}

//============================================================================
// FUNCTION: get_wms_limited_service_status <state>
//
// DESCRIPTION:
// To get wms limited service support available or not
//
// RETURN: 0 | 1 --> not supported, supported
//============================================================================
int get_wms_limited_service_status()
{
   int result = 0;
   QCRIL_LOG_FUNC_ENTRY();

   SMS_CACHE_LOCK;
   result = is_wms_limited_ready_status;
   QCRIL_LOG_INFO("WMS limited service support available %d", is_wms_limited_ready_status);
   SMS_CACHE_UNLOCK;

   QCRIL_LOG_FUNC_RETURN_WITH_RET(result);

   return result;
}

//============================================================================
// FUNCTION: wms_ready_limited_status_support
//
// DESCRIPTION:
// Check that wms limited service support available or not
//
// RETURN: 0 | 1 --> not supported, supported
//============================================================================
int wms_ready_limited_status_support(wms_service_ready_status_enum_v01 state)
{
    int result = 0;
    const char* state_str = sms_state_str(state);

    QCRIL_LOG_FUNC_ENTRY();

    switch (state)
    {
        case WMS_SERVICE_READY_STATUS_3GPP_LIMITED_V01:
        case WMS_SERVICE_READY_STATUS_3GPP2_LIMITED_V01:
        case WMS_SERVICE_READY_STATUS_3GPP_LIMITED_AND_3GPP2_LIMITED_V01:
        case WMS_SERVICE_READY_STATUS_3GPP_LIMITED_AND_3GPP2_V01:
        case WMS_SERVICE_READY_STATUS_3GPP_AND_3GPP2_LIMITED_V01:
            result = 1;
            QCRIL_LOG_WARN("WMS limited service support available, %s", state_str);
            break;

        default:
            break;
    }

   QCRIL_LOG_FUNC_RETURN_WITH_RET(result);

   return result;
}

//============================================================================
// FUNCTION: wms_ready_atleast_limited_3gpp
//
// DESCRIPTION:
// Check that wms service is ready for <3GPP(minimum limited)> and return a boolean
//
// RETURN: 0 | 1 --> not ready, ready
//============================================================================
int wms_ready_atleast_limited_3gpp()
{
    int result = 0;
    QCRIL_LOG_FUNC_ENTRY();

    SMS_CACHE_LOCK;
    wms_service_ready_status_enum_v01 state = wms_service_state;
    SMS_CACHE_UNLOCK;

    const char* state_str = sms_state_str(state);

    switch (state)
    {
        case WMS_SERVICE_READY_STATUS_3GPP_LIMITED_V01:
        case WMS_SERVICE_READY_STATUS_3GPP_V01:
        case WMS_SERVICE_READY_STATUS_3GPP_LIMITED_AND_3GPP2_LIMITED_V01:
        case WMS_SERVICE_READY_STATUS_3GPP_LIMITED_AND_3GPP2_V01:
        case WMS_SERVICE_READY_STATUS_3GPP_AND_3GPP2_LIMITED_V01:
        case WMS_SERVICE_READY_STATUS_3GPP_AND_3GPP2_V01:
            result = 1;
            QCRIL_LOG_WARN("WMS service is ready for atleast limited/full 3GPP, %s", state_str);
            break;

        default:
            result = 0;
            QCRIL_LOG_WARN("WMS service not ready for atleast limited 3GPP, %s", state_str);
            break;
    }

   QCRIL_LOG_FUNC_RETURN_WITH_RET(result);

   return result;
}

//============================================================================
// FUNCTION: wms_ready_atleast_limited_3gpp2
//
// DESCRIPTION:
// Check that wms service is ready for <3GPP2(minimum limited)> and return a boolean
//
// RETURN: 0 | 1 --> not ready, ready
//============================================================================
int wms_ready_atleast_limited_3gpp2()
{
    int result = 0;
    QCRIL_LOG_FUNC_ENTRY();

    SMS_CACHE_LOCK;
    wms_service_ready_status_enum_v01 state = wms_service_state;
    SMS_CACHE_UNLOCK;

    const char* state_str = sms_state_str(state);

    switch (state)
    {
        case WMS_SERVICE_READY_STATUS_3GPP2_LIMITED_V01:
        case WMS_SERVICE_READY_STATUS_3GPP2_V01:
        case WMS_SERVICE_READY_STATUS_3GPP_LIMITED_AND_3GPP2_LIMITED_V01:
        case WMS_SERVICE_READY_STATUS_3GPP_LIMITED_AND_3GPP2_V01:
        case WMS_SERVICE_READY_STATUS_3GPP_AND_3GPP2_LIMITED_V01:
        case WMS_SERVICE_READY_STATUS_3GPP_AND_3GPP2_V01:
            result = 1;
            QCRIL_LOG_WARN("WMS service is ready for atleast limited/full 3GPP2, %s", state_str);
            break;

        default:
            result = 0;
            QCRIL_LOG_WARN("WMS service not ready for atleast limited 3GPP2, %s", state_str);
            break;
    }

   QCRIL_LOG_FUNC_RETURN_WITH_RET(result);

   return result;
}

//============================================================================
// FUNCTION: wms_ready_full_3gpp
//
// DESCRIPTION:
// Check that wms service is ready for FULL <3GPP> and return a boolean
//
// RETURN: 0 | 1 --> not ready, ready
//============================================================================
int wms_ready_full_3gpp()
{
    int result = 0;
    QCRIL_LOG_FUNC_ENTRY();

    SMS_CACHE_LOCK;
    wms_service_ready_status_enum_v01 state = wms_service_state;
    SMS_CACHE_UNLOCK;

    const char* state_str = sms_state_str(state);

    switch (state)
    {
        case WMS_SERVICE_READY_STATUS_3GPP_V01:
        case WMS_SERVICE_READY_STATUS_3GPP_AND_3GPP2_LIMITED_V01:
        case WMS_SERVICE_READY_STATUS_3GPP_AND_3GPP2_V01:
            result = 1;
            QCRIL_LOG_WARN("WMS service is ready for FULL 3GPP, %s", state_str);
            break;

        default:
            result = 0;
            QCRIL_LOG_WARN("WMS service not ready for FULL 3GPP, %s", state_str);
            break;
    }

   QCRIL_LOG_FUNC_RETURN_WITH_RET(result);

   return result;
}

//============================================================================
// FUNCTION: wms_ready_full_3gpp2
//
// DESCRIPTION:
// Check that wms service is ready for FULL <3GPP2> and return a boolean
//
// RETURN: 0 | 1 --> not ready, ready
//============================================================================
int wms_ready_full_3gpp2()
{
    int result = 0;
    QCRIL_LOG_FUNC_ENTRY();

    SMS_CACHE_LOCK;
    wms_service_ready_status_enum_v01 state = wms_service_state;
    SMS_CACHE_UNLOCK;

    const char* state_str = sms_state_str(state);

    switch (state)
    {
        case WMS_SERVICE_READY_STATUS_3GPP2_V01:
        case WMS_SERVICE_READY_STATUS_3GPP_LIMITED_AND_3GPP2_V01:
        case WMS_SERVICE_READY_STATUS_3GPP_AND_3GPP2_V01:
            result = 1;
            QCRIL_LOG_WARN("WMS service is ready for FULL 3GPP2, %s", state_str);
            break;

        default:
            result = 0;
            QCRIL_LOG_WARN("WMS service not ready for FULL 3GPP2, %s", state_str);
            break;
    }

   QCRIL_LOG_FUNC_RETURN_WITH_RET(result);

   return result;
}

//============================================================================
// FUNCTION: qcril_send_generic_failure <token> <event>
//
// DESCRIPTION:
// Shortcut function to send a GENERIC_FAILURE result
//
// RETURN: service state string
//============================================================================
void send_generic_failure(RIL_Token token, qcril_evt_e_type event)
{
   qcril_request_resp_params_type resp;
   qcril_default_request_resp_params(QCRIL_DEFAULT_INSTANCE_ID, token,
                                     event, RIL_E_GENERIC_FAILURE, &resp);
   qcril_send_request_response(&resp);
}

//============================================================================
// FUNCTION: sms_state_str <service state>
//
// DESCRIPTION:
// Convert sms service state (wms_service_ready_status_enum_v01) to string
// for logging
//
// RETURN: service state string
//============================================================================
const char* sms_state_str(wms_service_ready_status_enum_v01 service_state)
{
   switch (service_state) {

      case WMS_SERVICE_READY_STATUS_NONE_V01:
         return "WMS_SERVICE_READY_STATUS_NONE";
         break;
      case WMS_SERVICE_READY_STATUS_3GPP_V01:
         return "WMS_SERVICE_READY_STATUS_3GPP";
         break;
      case WMS_SERVICE_READY_STATUS_3GPP2_V01:
         return "WMS_SERVICE_READY_STATUS_3GPP2";
         break;
      case WMS_SERVICE_READY_STATUS_3GPP_AND_3GPP2_V01:
         return "WMS_SERVICE_READY_STATUS_3GPP_AND_3GPP2";
         break;
      case WMS_SERVICE_READY_STATUS_3GPP_LIMITED_V01:
         return "WMS_SERVICE_READY_STATUS_3GPP_LIMITED";
         break;
      case WMS_SERVICE_READY_STATUS_3GPP2_LIMITED_V01:
         return "WMS_SERVICE_READY_STATUS_3GPP2_LIMITED";
         break;
      case WMS_SERVICE_READY_STATUS_3GPP_LIMITED_AND_3GPP2_LIMITED_V01:
         return "WMS_SERVICE_READY_STATUS_3GPP_LIMITED_AND_3GPP2_LIMITED";
         break;
      case WMS_SERVICE_READY_STATUS_3GPP_LIMITED_AND_3GPP2_V01:
         return "WMS_SERVICE_READY_STATUS_3GPP_LIMITED_AND_3GPP2";
         break;
      case WMS_SERVICE_READY_STATUS_3GPP_AND_3GPP2_LIMITED_V01:
         return "WMS_SERVICE_READY_STATUS_3GPP_AND_3GPP2_LIMITED";
         break;
      default:
         return "unmapped";
         break;
   }
}

//============================================================================
// FUNCTION: qcril_qmi_sms_map_qmi_mwi_msg_type_to_ims_msg_type
//
// DESCRIPTION:
// Convert qmi wms mwi message type to ims mwi message type
//============================================================================
qcril::interfaces::MessageType  qcril_qmi_sms_map_qmi_mwi_msg_type_to_ims_msg_type
(
 transport_mwi_wms_message_type_enum_v01 mwi_wms_msg_type
)
{
  qcril::interfaces::MessageType ims_mwi_msg_type;
  switch(mwi_wms_msg_type)
  {
    case TRANSPORT_MWI_MESSAGE_TYPE_VOICEMAIL_V01:
      ims_mwi_msg_type = qcril::interfaces::MessageType::VOICE;
      break;

    case TRANSPORT_MWI_MESSAGE_TYPE_VIDEOMAIL_V01:
      ims_mwi_msg_type = qcril::interfaces::MessageType::VIDEO;
      break;

    case TRANSPORT_MWI_MESSAGE_TYPE_FAX_V01:
      ims_mwi_msg_type = qcril::interfaces::MessageType::FAX;
      break;

    case TRANSPORT_MWI_MESSAGE_TYPE_PAGER_V01:
      ims_mwi_msg_type = qcril::interfaces::MessageType::PAGER;
      break;

    case TRANSPORT_MWI_MESSAGE_TYPE_MULTIMEDIA_V01:
      ims_mwi_msg_type = qcril::interfaces::MessageType::MULTIMEDIA;
      break;

    case TRANSPORT_MWI_MESSAGE_TYPE_TEXT_V01:
      ims_mwi_msg_type = qcril::interfaces::MessageType::TEXT;
      break;

    default:
      ims_mwi_msg_type = qcril::interfaces::MessageType::NONE;
      break;
  }
  QCRIL_LOG_INFO("qmi mwi_wms_msg_type %d mapped to ims mwi_msg_type %d",
          mwi_wms_msg_type, ims_mwi_msg_type);
  return ims_mwi_msg_type;
}

//============================================================================
// FUNCTION: qcril_qmi_sms_map_qmi_mwi_priority_to_ims_priority
//
// DESCRIPTION:
// Convert qmi wms mwi priority to ims mwi priority
//============================================================================
qcril::interfaces::MessagePriority qcril_qmi_sms_map_qmi_mwi_priority_to_ims_priority
(
 wms_mwi_priority_type_enum_v01 wms_mwi_priority
)
{
  qcril::interfaces::MessagePriority mwi_priority;
  switch(wms_mwi_priority)
  {
    case WMS_LOW_PRIORITY_V01:
      mwi_priority = qcril::interfaces::MessagePriority::LOW;
      break;

    case WMS_NORMAL_PRIORITY_V01:
      mwi_priority = qcril::interfaces::MessagePriority::NORMAL;
      break;

    case WMS_URGENT_PRIORITY_V01:
      mwi_priority = qcril::interfaces::MessagePriority::URGENT;
      break;

    case WMS_UNKNOWN_PRIORITY_V01:
    default:
      mwi_priority = qcril::interfaces::MessagePriority::UNKNOWN;
      break;
  }
  QCRIL_LOG_INFO("qmi wms_mwi_priority %d mapped to ims mwi_priority %d",
          wms_mwi_priority, mwi_priority);
  return mwi_priority;
}



//=========================================================================================
//qcril_qmi_sms_block_retry_3pgg_sms_on_lte_only
//returns TRUE if 3gpp SMS is retried and when LTE only
//=========================================================================================
int qcril_qmi_sms_block_retry_3pgg_sms_on_lte_only(RIL_RadioTechnologyFamily tech, unsigned char retry)
{
    int ret = FALSE;

    if (RADIO_TECH_3GPP == tech &&
        TRUE == retry &&
        qcril_qmi_nas_is_lte_only_registered())
    {
        ret = TRUE;
    }
    QCRIL_LOG_FUNC_RETURN_WITH_RET(ret);
    return ret;
} // qcril_qmi_sms_block_retry_3pgg_sms_on_lte_only

//=========================================================================================
//qcril_qmi_sms_report_unsol_ims_state_change
//=========================================================================================
void qcril_qmi_sms_report_unsol_ims_state_change()
{
    qcril_unsol_resp_params_type unsol_resp;

    QCRIL_LOG_FUNC_ENTRY();

    memset(&unsol_resp, 0, sizeof(unsol_resp));
    if ( qmi_ril_is_feature_supported(QMI_RIL_FEATURE_IMS) )
    {
        auto msg = std::make_shared<RilUnsolImsNetworkStateChangedMessage>();
        Dispatcher::getInstance().dispatch(msg);
    }

    QCRIL_LOG_FUNC_RETURN();
}

void qcril_qmi_sms_reset_mt_sms_ack_needed_power_opt_buffer()
{
    QCRIL_LOG_INFO("Reset MT SMS ACK NEEDED Power Opt buffer");
    qcril_mt_sms_buffer_power_opt.mt_sms_with_ack_needed_valid = FALSE;
    qcril_mt_sms_buffer_power_opt.mt_sms_with_ack_needed_length = 0;
    memset(&qcril_mt_sms_buffer_power_opt.mt_sms_with_ack_needed,0,sizeof(qcril_mt_sms_buffer_power_opt.mt_sms_with_ack_needed));
    if ( QMI_RIL_ZERO != qcril_mt_sms_buffer_power_opt.mt_sms_ack_needed_expry_tmr )
    {
        qcril_cancel_timed_callback( (void*)(uintptr_t)qcril_mt_sms_buffer_power_opt.mt_sms_ack_needed_expry_tmr );
        qcril_mt_sms_buffer_power_opt.mt_sms_ack_needed_expry_tmr = 0;
    }
}

void qcril_qmi_sms_reset_mt_sms_ack_not_needed_power_opt_buffer()
{
    QCRIL_LOG_INFO("Reset MT SMS ACK NOT NEEDED Power Opt buffer");
    qcril_mt_sms_buffer_power_opt.mt_sms_with_ack_not_needed_valid = FALSE;
    qcril_mt_sms_buffer_power_opt.mt_sms_with_ack_not_needed_length = 0;
    memset(&qcril_mt_sms_buffer_power_opt.mt_sms_with_ack_not_needed,0,sizeof(qcril_mt_sms_buffer_power_opt.mt_sms_with_ack_not_needed));
}

void qcril_qmi_sms_update_mt_sms_with_ack_needed_power_opt_buffer(wms_event_report_ind_msg_v01 *ind_data_ptr)
{
    int length = 0;

    QCRIL_LOG_FUNC_ENTRY();

    do
    {
        if(NULL == ind_data_ptr)
            break;

        if(!qcril_mt_sms_buffer_power_opt.mt_sms_with_ack_needed_valid)
        {
            qcril_mt_sms_buffer_power_opt.mt_sms_with_ack_needed_valid = TRUE;
            qcril_mt_sms_buffer_power_opt.mt_sms_with_ack_needed_length = 0;
        }

        length = qcril_mt_sms_buffer_power_opt.mt_sms_with_ack_needed_length;
        if(length == MAX_MT_SMS_POWER_OPT_BUFFER)
            break;

        memcpy(&qcril_mt_sms_buffer_power_opt.mt_sms_with_ack_needed[length], ind_data_ptr, sizeof(wms_event_report_ind_msg_v01));
        qcril_mt_sms_buffer_power_opt.mt_sms_with_ack_needed_length++;
        QCRIL_LOG_INFO("MT SMS ACK NEEDED Power Opt buffer length %d", qcril_mt_sms_buffer_power_opt.mt_sms_with_ack_needed_length);
    }while(FALSE);

    QCRIL_LOG_FUNC_RETURN();
}

void qcril_qmi_sms_notify_mt_sms_with_ack_needed_power_opt_to_atel()
{
    int i = 0;
    int length = 0;

    QCRIL_LOG_INFO("MT SMS ACK NEEDED Power Opt buffer valid %d", qcril_mt_sms_buffer_power_opt.mt_sms_with_ack_needed_valid);
    if(qcril_mt_sms_buffer_power_opt.mt_sms_with_ack_needed_valid)
    {
        length = qcril_mt_sms_buffer_power_opt.mt_sms_with_ack_needed_length;
        QCRIL_LOG_INFO("MT SMS ACK NEEDED Power Opt buffer length %d", qcril_mt_sms_buffer_power_opt.mt_sms_with_ack_needed_length);
        for(i=0; i<length; i++)
        {
            qcril_sms_process_event_report_ind(&qcril_mt_sms_buffer_power_opt.mt_sms_with_ack_needed[i], QCRIL_DEFAULT_INSTANCE_ID);
        }

        qcril_qmi_sms_reset_mt_sms_ack_needed_power_opt_buffer();
    }
}

void qcril_qmi_sms_update_mt_sms_with_ack_not_needed_power_opt_buffer(wms_event_report_ind_msg_v01 *ind_data_ptr)
{
    int length = 0;

    QCRIL_LOG_FUNC_ENTRY();

    do
    {
        if(NULL == ind_data_ptr)
            break;

        if(!qcril_mt_sms_buffer_power_opt.mt_sms_with_ack_not_needed_valid)
        {
            qcril_mt_sms_buffer_power_opt.mt_sms_with_ack_not_needed_valid = TRUE;
            qcril_mt_sms_buffer_power_opt.mt_sms_with_ack_not_needed_length = 0;
        }

        length = qcril_mt_sms_buffer_power_opt.mt_sms_with_ack_not_needed_length;
        if(length == MAX_MT_SMS_POWER_OPT_BUFFER)
            break;

        memcpy(&qcril_mt_sms_buffer_power_opt.mt_sms_with_ack_not_needed[length], ind_data_ptr, sizeof(wms_event_report_ind_msg_v01));
        qcril_mt_sms_buffer_power_opt.mt_sms_with_ack_not_needed_length++;
        QCRIL_LOG_INFO("MT SMS ACK NOT NEEDED Power Opt buffer length %d", qcril_mt_sms_buffer_power_opt.mt_sms_with_ack_not_needed_length);
    }while(FALSE);

    QCRIL_LOG_FUNC_RETURN();
}

void qcril_qmi_sms_notify_mt_sms_with_ack_not_needed_power_opt_to_atel()
{
    int i = 0;
    int length = 0;

    QCRIL_LOG_INFO("MT SMS ACK NOT NEEDED Power Opt buffer valid %d", qcril_mt_sms_buffer_power_opt.mt_sms_with_ack_not_needed_valid);
    if(qcril_mt_sms_buffer_power_opt.mt_sms_with_ack_not_needed_valid)
    {
        length = qcril_mt_sms_buffer_power_opt.mt_sms_with_ack_not_needed_length;
        QCRIL_LOG_INFO("MT SMS ACK NOT NEEDED Power Opt buffer length %d", qcril_mt_sms_buffer_power_opt.mt_sms_with_ack_not_needed_length);
        for(i=0; i<length; i++)
        {
            qcril_sms_process_event_report_ind(&qcril_mt_sms_buffer_power_opt.mt_sms_with_ack_not_needed[i], QCRIL_DEFAULT_INSTANCE_ID);
        }

        qcril_qmi_sms_reset_mt_sms_ack_not_needed_power_opt_buffer();
    }
}

//=========================================================================================
//qcril_sms_update_mt_sms_power_opt_buffer
//=========================================================================================
void qcril_sms_update_mt_sms_power_opt_buffer(wms_event_report_ind_msg_v01 *event_report_ind)
{
    uint8_t update_mt_sms_ack_needed_buffer = FALSE;
    uint8_t update_mt_sms_ack_not_needed_buffer = FALSE;
    struct timeval ack_expry_tmr_window = { 0 , 0 };

    do
    {
        if( event_report_ind == NULL)
            break;

        if (event_report_ind->transfer_route_mt_message_valid)
        {
            if ((event_report_ind->transfer_route_mt_message.format == WMS_MESSAGE_FORMAT_CDMA_V01) ||
                (event_report_ind->transfer_route_mt_message.format == WMS_MESSAGE_FORMAT_MWI_V01))
            {
                if(WMS_ACK_INDICATOR_SEND_ACK_V01 == event_report_ind->transfer_route_mt_message.ack_indicator)
                {
                    update_mt_sms_ack_needed_buffer = TRUE;
                    ack_expry_tmr_window.tv_sec  = 2; //2sec for CDMA
                    ack_expry_tmr_window.tv_usec = QMI_RIL_ZERO;
                }
                else
                {
                    update_mt_sms_ack_not_needed_buffer = TRUE;
                }
            }
            else if (event_report_ind->transfer_route_mt_message.format == WMS_MESSAGE_FORMAT_GW_PP_V01)
            {
                if(WMS_ACK_INDICATOR_SEND_ACK_V01 == event_report_ind->transfer_route_mt_message.ack_indicator)
                {
                    update_mt_sms_ack_needed_buffer = TRUE;
                    ack_expry_tmr_window.tv_sec  = 20; //20sec for GSM
                    ack_expry_tmr_window.tv_usec = QMI_RIL_ZERO;
                }
                else
                {
                    update_mt_sms_ack_not_needed_buffer = TRUE;
                }
            }
        }
        else if ((event_report_ind->mt_message_valid) &&
           (event_report_ind->message_mode_valid))
        {
            update_mt_sms_ack_not_needed_buffer = TRUE;
        }

        if(update_mt_sms_ack_not_needed_buffer)
        {
            qcril_qmi_sms_update_mt_sms_with_ack_not_needed_power_opt_buffer(event_report_ind);
        }
        else if(update_mt_sms_ack_needed_buffer)
        {
            /*Currently MAX 1 MT SMS with ACK needed to be buffered and processed is supported for Power Opt*/
            qcril_qmi_sms_reset_mt_sms_ack_needed_power_opt_buffer();
            qcril_qmi_sms_update_mt_sms_with_ack_needed_power_opt_buffer(event_report_ind);
            qcril_setup_timed_callback( QCRIL_DEFAULT_INSTANCE_ID,
                                QCRIL_DEFAULT_MODEM_ID,
                                qcril_qmi_nas_mt_sms_power_opt_transaction_expired,
                                &ack_expry_tmr_window,
                                &qcril_mt_sms_buffer_power_opt.mt_sms_ack_needed_expry_tmr );
        }
    }while(FALSE);
}

#ifdef QMI_RIL_UTF
void qcril_qmi_hal_sms_module_cleanup()
{
    is_sms_memory_full = FALSE;
    getSmsModule().qcrilHalSmsModuleCleanup();
    qmi_ril_set_sms_svc_status( QMI_RIL_SMS_SVC_NOT_INITIALZIED );
    qmi_ril_set_primary_client_status( QMI_RIL_SMS_PRIMARY_CLIENT_NOT_SET );
}
#endif
