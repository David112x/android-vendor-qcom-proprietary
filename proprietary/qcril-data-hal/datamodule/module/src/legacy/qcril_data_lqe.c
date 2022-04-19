/*!
  @file
  qcril_data_lqe.c

  @brief
  Handles RIL LQE requests for DATA services.

*/

/*===========================================================================

  Copyright (c) 2015 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/
#ifdef FEATURE_DATA_LQE
#ifdef QCRIL_DATA_OFFTARGET
#include <netinet/in.h>
#include <errno.h>
#endif

#include "qcril_data.h"
#include "qcril_data_defs.h"
#include "qcril_data_utils.h"
#include "qcril_reqlist.h"
#include "DataModuleMutex.h"
#ifndef QCRIL_RIL_VERSION
#error "Must include ril.h for QCRIL_RIL_VERSION"
#endif

/*===========================================================================

                   EXTERNAL DEFINITIONS AND TYPES

===========================================================================*/

#include <stdint.h>
#include "lqe.h"

/*===========================================================================

                   INTERNAL DEFINITIONS AND TYPES

===========================================================================*/
#define INVALID_LQE_HANDLE (0)

#define LQE_STATUS_NOT_SUPPORTED  (-1)
#define LQE_STATUS_STOPPED        (0)
#define LQE_STATUS_ACTIVE         (1)

#define LQE_SUSPENDED             (1)

#define CONFIDENCE_IN_PERCENTAGE(confidence) \
            ((confidence)>7 ? 100:((confidence) * 100)/7)

static lqe_client_hdl_t lqe_handle = INVALID_LQE_HANDLE;
static boolean lqe_inited = FALSE;

/**
 * We need to collate the uplink and downlink data from modem
 * and only send one indication.
 */
static uint32_t downlink_kbps = 0;
static uint32_t uplink_kbps = 0;

typedef struct
{
  lqe_ind_events_t      event;
  lqe_ind_event_info_t  info;
  void  *self;
} qcril_data_lqe_event_data_t;

typedef struct
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type    modem_id;
} lqe_cb_tbl_type;

typedef enum lqe_report_mode_e
{
  LQE_PUSH_MODE = 0,
  LQE_PULL_MODE = 1
} lqe_report_mode_t;

typedef struct
{
  lqe_report_mode_t report_mode;
  int report_interval;
  int actual_interval;
} lqe_start_req_t;

static boolean qcril_data_lqe_is_pull_supported = TRUE;
static boolean is_threshold_based_reporting_supported = TRUE;
static lqe_cb_tbl_type lqe_cb_info;
static lqe_start_req_t latest_start_lqe_params;

/*===========================================================================

                    INTERNAL FUNCTION PROTOTYPES

===========================================================================*/

static void qcril_data_lqe_ind_cb
(
  lqe_client_hdl_t        client_hndl,
  lqe_ind_events_t        event,
  lqe_ind_event_info_t   *info,
  void                   *data
)
{
  qcril_data_lqe_event_data_t  *evt;

  (void)data;
  (void)client_hndl;

  QCRIL_LOG_INFO( "%s", "qcril_data_lqe_ind_cb entered" );

  QCRIL_DATA_MUTEX_LOCK(LQE_CB_TBL_MUTEX);

  if(!info)
  {
    QCRIL_LOG_ERROR ( "invalid input. info [%p]", info );
    goto bail;
  }

  if( lqe_cb_info.instance_id == QCRIL_DATA_INVALID_INSTANCE_ID
      || lqe_cb_info.modem_id == QCRIL_DATA_INVALID_MODEM_ID )
  {
    QCRIL_LOG_ERROR ( "invalid arg, instance_id [%#x], modem_id [%#x]",
                      lqe_cb_info.instance_id, lqe_cb_info.modem_id );
    goto bail;
  }

  /* Allocate from heap here and clean-up on call end */
  QCRIL_DATA_ALLOC_STORAGE(evt, sizeof( qcril_data_lqe_event_data_t ), bail);
  QCRIL_DS_LOG_DBG_MEM( "event obj alloc", evt );
  memset( evt, 0, sizeof( qcril_data_lqe_event_data_t ) );

  /* Populate data event obj */
  evt->event = event;
  evt->info = *info;
  evt->self = evt;

  QCRIL_LOG_VERBOSE( "queue QCRIL DATA event with "
                     "instance_id [%d], and modem_id [%d]",
                     lqe_cb_info.instance_id, lqe_cb_info.modem_id );

  if(E_SUCCESS != qcril_event_queue(lqe_cb_info.instance_id,
                                    lqe_cb_info.modem_id,
                                    QCRIL_DATA_NOT_ON_STACK,
                                    QCRIL_EVT_DATA_LQE_EVENT_CALLBACK,
                                    (void *)evt,
                                    sizeof(qcril_data_lqe_event_data_t),
                                    (RIL_Token) QCRIL_TOKEN_ID_INTERNAL))
  {
    QCRIL_LOG_ERROR("%s", "qcril_event_queue failed\n");
    goto bail;
  }

bail:
  QCRIL_DATA_MUTEX_UNLOCK(LQE_CB_TBL_MUTEX);

  QCRIL_LOG_INFO( "%s", "qcril_data_lqe_ind_cb completed" );
}

/*===========================================================================

                    EXTERNAL FUNCTION PROTOTYPES

===========================================================================*/


/*===========================================================================
  FUNCTION:  qcril_data_lqe_event_hdlr

===========================================================================*/
/*!
    @brief
    Registered with QCRIL to be called by QCRIL on event
    QCRIL_EVT_DATA_LQE_EVENT_CALLBACK

    @return
    None.
*/
/*=========================================================================*/
void qcril_data_lqe_event_hdlr(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr  /*output*/
)
{
  qcril_instance_id_e_type instance_id;
  qcril_data_lqe_event_data_t *evt_info_ptr;

  QCRIL_LOG_DEBUG( "%s", "qcril_data_lqe_event_hdlr: ENTRY" );

  /* Input Validation */
  QCRIL_DS_ASSERT( params_ptr != NULL, "validate input params_ptr" );
  QCRIL_DS_ASSERT( ret_ptr    != NULL, "validate input ret_ptr" );
  if ( ( params_ptr == NULL ) || ( ret_ptr == NULL ) )
  {
    goto bail;
  }

  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  if (instance_id >= QCRIL_MAX_INSTANCE_ID)
  {
    goto bail;
  }

  evt_info_ptr = (qcril_data_lqe_event_data_t *)params_ptr->data;
  QCRIL_DS_LOG_DBG_MEM( "event obj", evt_info_ptr );

  switch(evt_info_ptr->event)
  {
    case LQE_DOWNLINK_THROUGHPUT_IND_EV:
    {
      lqe_downlink_throughput_info_ind_t *dl_info = &evt_info_ptr->info.dl_throughput_info;

      QCRIL_LOG_DEBUG ( "%s", "recvd LQE_DOWNLINK_THROUGHPUT_IND_EV event" );
      if(dl_info->downlink_allowed_rate_valid)
      {
        QCRIL_LOG_INFO( "<<< Rcvd DL throughput: %d kbps", dl_info->downlink_allowed_rate );
        // if there is a change in downlink, query uplink rate as well
        if (downlink_kbps != dl_info->downlink_allowed_rate) {
          int confidence = 0;
          if (LQE_SUCCESS != lqei_get_uplink_throughput_info( lqe_handle,
                                                              (int*) &uplink_kbps,
                                                              &confidence ) ) {
            QCRIL_LOG_ERROR("Failed to get the latest uplink throughput");
          }
          downlink_kbps = dl_info->downlink_allowed_rate;
          QCRIL_LOG_INFO( ">>> Posting UL = %d kbps, DL = %d kbps >>>",
              uplink_kbps, downlink_kbps);
          qcril_data_lce_report_ind(uplink_kbps, downlink_kbps);
        }
      }
      else
      {
        QCRIL_LOG_INFO( "<<< Rcvd Invalid DL bandwidth info! <<<");
      }
    }
    break;

    case LQE_UPLINK_THROUGHPUT_IND_EV:
    {
      lqe_uplink_throughput_info_ind_t *ul_info = &evt_info_ptr->info.ul_throughput_info;

      QCRIL_LOG_DEBUG ( "%s", "recvd LQE_UPLINK_THROUGHPUT_IND_EV event" );
      if(ul_info->uplink_allowed_rate_valid)
      {
        QCRIL_LOG_INFO( "<<< Rcvd UL throughput: %d kbps", ul_info->uplink_allowed_rate );
        // if there is a change in uplink then, query downlink as well
        if (uplink_kbps != ul_info->uplink_allowed_rate) {
          // get the latest downlink kbps
          int confidence = 0;
          if (LQE_SUCCESS != lqei_get_downlink_throughput_info( lqe_handle,
                                                                (int*) &downlink_kbps,
                                                                &confidence)) {
            QCRIL_LOG_ERROR("Failed to get the latest downlink throughput");
          }
          uplink_kbps = ul_info->uplink_allowed_rate;
          QCRIL_LOG_INFO( ">>> Posting UL = %d kbps, DL = %d kbps >>>",
              uplink_kbps, downlink_kbps);
          qcril_data_lce_report_ind(uplink_kbps, downlink_kbps);
        }
      }
      else
      {
        QCRIL_LOG_INFO( "<<< Rcvd Invalid UL bandwidth info! <<<");
      }
    }
    break;
    default:
      QCRIL_LOG_DEBUG ( "<<< Rcvd invalid lqe event [0x%X]", evt_info_ptr->event );
      break;
  }

bail:
  QCRIL_LOG_INFO( "%s", "qcril_data_lqe_event_hdlr completed" );
}

/*===========================================================================

  FUNCTION:  qcril_data_lqe_init

===========================================================================*/
/*!
    @brief

    Do LQE init and register for a client. This must be called only once

    @pre Before calling, LQE_MUTEX must be locked by the calling thread

    @return

    None on Success, Generic failure response on Failure
*/
/*=========================================================================*/
int qcril_data_lqe_init
(
  int instance_id,
  int modem_id
)
{
  int ret = FAILURE;
  int dl_rate;
  int confidence_level;

  QCRIL_LOG_INFO( "%s", "qcril_data_lqe_init entered" );

  if(lqe_inited)
  {
    QCRIL_LOG_DEBUG ("%s", "LQE already initialized");
    ret = SUCCESS;
    goto bail;
  }

  if(LQE_SUCCESS != lqei_init())
  {
      QCRIL_LOG_DEBUG ("%s", "Failed to initialize LQE");
      goto bail;
  }

  if(LQE_SUCCESS != lqei_client_register(qcril_data_lqe_ind_cb,
                                         NULL,
                                         &lqe_handle))
  {
    QCRIL_LOG_ERROR ("%s", "Failed to register for LQE client");
    lqe_handle = INVALID_LQE_HANDLE;
    lqei_release();
    goto bail;
  }
  QCRIL_LOG_DEBUG ("Success LQE client registration, handle [0x%X]", lqe_handle);

  qcril_data_lqe_is_pull_supported = TRUE;

  /* Check if threshold based reporting is supported and subscribe for
   * the indications
   */
  if (lqei_check_and_subscribe_threshold_reporting(lqe_handle) != 0) {
    QCRIL_LOG_INFO("Interval-based reporting is supported!");
    is_threshold_based_reporting_supported = FALSE;
    if(LQE_NOT_SUPPORTED == lqei_get_downlink_throughput_info(lqe_handle,
                                                              &dl_rate,
                                                              &confidence_level))
    {
      QCRIL_LOG_INFO ("%s", "LQE PULL mode is not supported");
      qcril_data_lqe_is_pull_supported = FALSE;
    }
  }
  else {
    QCRIL_LOG_INFO("Threshold-based reporting is supported!");
    qcril_data_lqe_is_pull_supported = FALSE;

    /* Enable reporting by default. */
    if (lqei_toggle_throughput_reporting( lqe_handle,
                                           LQE_DATA_UPLINK_DIRECTION,
                                           LQE_TURN_ON_INDICATIONS) != LQE_SUCCESS)
    {
      QCRIL_LOG_ERROR ("Failed to start UL reporting, client [0x%x]", lqe_handle);
    }
    if (lqei_toggle_throughput_reporting( lqe_handle,
                                           LQE_DATA_DOWNLINK_DIRECTION,
                                           LQE_TURN_ON_INDICATIONS) != LQE_SUCCESS)
    {
      QCRIL_LOG_ERROR ("Failed to start DL reporting, client [0x%x]", lqe_handle);
    }
  }

  lqe_inited = TRUE;

  lqe_cb_info.instance_id = instance_id;
  lqe_cb_info.modem_id = modem_id;

  ret = SUCCESS;

bail:
  QCRIL_LOG_INFO( "%s", "qcril_data_lqe_init exit" );
  return ret;
}

/*===========================================================================

  FUNCTION:  qcril_data_lqe_release

===========================================================================*/
/*!
    @brief

    Do LQE release and unregister the client. This must be called after lqe_init()

    @pre Before calling, LQE_MUTEX must be locked by the calling thread

    @return

    None on Success, Generic failure response on Failure
*/
/*=========================================================================*/
int qcril_data_lqe_release(void)
{
  QCRIL_LOG_INFO( "%s", "qcril_data_lqe_release entered" );

  if(!lqe_inited)
  {
    QCRIL_LOG_DEBUG ("%s", "LQE is NOT initialized");
    goto bail;
  }

  if(lqe_handle != INVALID_LQE_HANDLE)
  {
    if(LQE_SUCCESS != lqei_client_release(lqe_handle))
    {
      QCRIL_LOG_ERROR ("Failed to release LQE client [0x%X]", lqe_handle);
    }
  }

  lqei_release();

  memset(&lqe_cb_info, 0, sizeof(lqe_cb_info));

  lqe_inited = FALSE;

bail:
  QCRIL_LOG_INFO( "%s", "qcril_data_lqe_release completed" );
  return SUCCESS;
}

/*===========================================================================

  FUNCTION:  qcril_data_lqe_start

===========================================================================*/
/*!
    @brief

    Handles RIL_REQUEST_START_LCE.

    @pre Before calling, LQE_MUTEX must not be locked by the calling thread

    @return

    None on Success, Generic failure response on Failure
*/
/*=========================================================================*/
void qcril_data_lqe_start
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr
)
{
  qcril_instance_id_e_type        instance_id;
  qcril_modem_id_e_type           modem_id;
  qcril_request_resp_params_type  resp;
  int                             ret;
  int                             ril_report_interval;
  int                             ril_report_mode;
  lqe_report_mode_t               report_mode;
  RIL_LceStatusInfo               lce_status_resp;
  RIL_Errno                       ril_req_res = RIL_E_INTERNAL_ERR;
  lqe_downlink_throughput_report_status lqe_dl_throughput_status;

  QCRIL_LOG_INFO( "%s", "qcril_data_lqe_start entered" );

  /* validate input parameter */
  if ( ( NULL == params_ptr ) || ( NULL == ret_ptr ) || ( ( params_ptr->datalen % 4 ) != 0) )
  {
    QCRIL_LOG_ERROR( "BAD input, params_ptr [%p], ret_ptr [%p], datalen [%d]",
                     (unsigned int *)params_ptr, (unsigned int *)ret_ptr,
                     params_ptr ? params_ptr->datalen : 0);
    goto bail;
  }

  instance_id = params_ptr->instance_id;
  modem_id = params_ptr->modem_id;

  if (instance_id >= QCRIL_MAX_INSTANCE_ID ||
      modem_id >= QCRIL_MAX_MODEM_ID)
  {
    QCRIL_LOG_ERROR("BAD iput, instance_id [%d], modem_id [%d]", instance_id, modem_id);
    goto bail;
  }

  /* input parameter is good, proceed... */
  QCRIL_DATA_MUTEX_LOCK(LQE_MUTEX);

  ril_report_interval = ((const int *)params_ptr->data)[0];
  ril_report_mode = ((const int *)params_ptr->data)[1];

  do
  {
    if(!lqe_inited)
    {
      QCRIL_LOG_ERROR ("%s", "LQE not inited");
      break;
    }

    report_mode = (ril_report_mode == 0) ? LQE_PUSH_MODE : LQE_PULL_MODE;

    QCRIL_LOG_DEBUG ("RIL provided report_interval [%dms]", ril_report_interval);
    QCRIL_LOG_DEBUG ("RIL provided report_mode [%s]", (report_mode == LQE_PUSH_MODE)? "PUSH":"PULL");

    if(report_mode == LQE_PULL_MODE && !qcril_data_lqe_is_pull_supported)
    {
      QCRIL_LOG_ERROR ("%s", "Modem does not support PULL mode");
      ril_req_res = RIL_E_LCE_NOT_SUPPORTED;
      break;
    }

    if(ril_report_interval <= 0)
    {
      QCRIL_LOG_ERROR ("%s", "Invalid report_interval");
      break;
    }

    ret = lqei_set_downlink_throughput_indication_frequency(lqe_handle,
                                                            ril_report_interval,
                                                            report_mode==LQE_PULL_MODE ?
                                                                 LQE_TURN_OFF_INDICATIONS
                                                                 : LQE_TURN_ON_INDICATIONS);
    if(LQE_NOT_SUPPORTED == ret)
    {
      QCRIL_LOG_ERROR( "%s", "LCE is not supported by modem" );
      ril_req_res = RIL_E_LCE_NOT_SUPPORTED;
      break;
    }
    else if(LQE_SUCCESS != ret)
    {
      QCRIL_LOG_ERROR( "unable to start LQE, handle [%d]", lqe_handle );
      break;
    }

    memset(&lqe_dl_throughput_status, 0, sizeof(lqe_dl_throughput_status));
    if(LQE_SUCCESS != lqei_get_min_downlink_throughput_frequency(lqe_handle,
                                                                 &lqe_dl_throughput_status))
    {
      QCRIL_LOG_ERROR( "%s", "failed to get min downlink throughput freq" );

      QCRIL_LOG_ERROR( "%s", "attempting to stop LQE" );
      lqei_set_downlink_throughput_indication_frequency(lqe_handle, 0, LQE_TURN_OFF_INDICATIONS);
      break;
    }

    if(!lqe_dl_throughput_status.reporting_status_valid)
    {
        QCRIL_LOG_ERROR( "%s", "Received invalid LCE status from modem" );
        break;
    }

    if(lqe_dl_throughput_status.reporting_status == LQE_THRPT_REPORT_ENABLED ||
       lqe_dl_throughput_status.reporting_status == LQE_THRPT_REPORT_DISABLED_NO_DATA_CALL ||
       lqe_dl_throughput_status.reporting_status == LQE_THRPT_REPORT_DISABLED_ALL_CALLS_DORMANT)
    {
      QCRIL_LOG_INFO( "LCE started successfully with status [%d]",
                      lqe_dl_throughput_status.reporting_status );
    }
    else if(lqe_dl_throughput_status.reporting_status == LQE_THRPT_REPORT_DISABLED_UNSUPPORTED_RAT)
    {
      QCRIL_LOG_ERROR( "%s", "LCE Disabled. RAT not supported" );
      ril_req_res = RIL_E_RADIO_NOT_AVAILABLE;
      break;
    }
    else
    {
      QCRIL_LOG_ERROR( "Received unknown LCE status from modem [%d]",
                       lqe_dl_throughput_status.reporting_status );
      break;
    }

    ril_req_res = RIL_E_SUCCESS;
  }while(FALSE);

  memset ( &lce_status_resp, 0, sizeof( lce_status_resp ) );
  if(ril_req_res == RIL_E_SUCCESS)
  {
    latest_start_lqe_params.report_mode = report_mode;
    latest_start_lqe_params.report_interval = ril_report_interval;
    latest_start_lqe_params.actual_interval = lqe_dl_throughput_status.actual_interval;

    lce_status_resp.lce_status = LQE_STATUS_ACTIVE;
    lce_status_resp.actual_interval_ms = lqe_dl_throughput_status.actual_interval;
  }
  else if(ril_req_res == RIL_E_LCE_NOT_SUPPORTED)
  {
    QCRIL_LOG_ERROR( "%s", "respond to QCRIL as LQE not supported" );
    lce_status_resp.lce_status = LQE_STATUS_NOT_SUPPORTED;
  }
  else if(ril_req_res == RIL_E_RADIO_NOT_AVAILABLE)
  {
    QCRIL_LOG_ERROR( "%s", "respond to QCRIL radio not available" );
    lce_status_resp.lce_status = LQE_STATUS_STOPPED;
  }
  else
  {
    QCRIL_LOG_ERROR( "%s", "respond to QCRIL as generic failure" );
    lce_status_resp.lce_status = LQE_STATUS_STOPPED;
  }

  qcril_default_request_resp_params( instance_id,
                                     params_ptr->t,
                                     params_ptr->event_id,
                                     ril_req_res,/* revisit on error code */
                                     &resp );

  resp.resp_pkt = &lce_status_resp;
  resp.resp_len = sizeof( lce_status_resp );
  qcril_send_request_response( &resp );

  QCRIL_DATA_MUTEX_UNLOCK(LQE_MUTEX);

bail:
  QCRIL_LOG_INFO( "qcril_data_lqe_start completed %d", (int) ril_req_res );
}


/*===========================================================================

  FUNCTION:  qcril_data_lqe_stop

===========================================================================*/
/*!
    @brief

    Handles RIL_REQUEST_STOP_LCE.

    @pre Before calling, info_tbl_mutex must not be locked by the calling thread

    @return

    None on Success, Generic failure response on Failure
*/
/*=========================================================================*/
void qcril_data_lqe_stop
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr
)
{
  qcril_instance_id_e_type        instance_id;
  qcril_modem_id_e_type           modem_id;
  qcril_request_resp_params_type  resp;
  RIL_LceStatusInfo               lce_status_resp;
  int                             ret;

  RIL_Errno ril_req_res = RIL_E_INTERNAL_ERR;

  QCRIL_LOG_INFO( "%s", "qcril_data_lqe_stop entered" );

  /* validate input parameter */
  if ( ( NULL == params_ptr ) || ( NULL == ret_ptr ) )
  {
    QCRIL_LOG_ERROR( "BAD input, params_ptr [%p], ret_ptr [%p]",
                     (unsigned int *)params_ptr, (unsigned int *)ret_ptr);
    goto bail;
  }

  instance_id = params_ptr->instance_id;
  modem_id = params_ptr->modem_id;

  if (instance_id >= QCRIL_MAX_INSTANCE_ID ||
      modem_id >= QCRIL_MAX_MODEM_ID)
  {
    QCRIL_LOG_ERROR("BAD iput, instance_id [%d], modem_id [%d]", instance_id, modem_id);
    goto bail;
  }

  /* input parameter is good, proceed... */
  QCRIL_DATA_MUTEX_LOCK(LQE_MUTEX);

  do
  {
    QCRIL_LOG_ERROR( "%s", "attempting to stop LQE" );
    ret = lqei_set_downlink_throughput_indication_frequency(lqe_handle, 0, 0);
    if(LQE_NOT_SUPPORTED == ret)
    {
      QCRIL_LOG_ERROR( "%s", "LCE is not supported by modem" );
      ril_req_res = RIL_E_LCE_NOT_SUPPORTED;
      break;
    }
    else if(LQE_SUCCESS != ret)
    {
      QCRIL_LOG_ERROR( "unable to stop LQE, handle [%d]", lqe_handle );
      ril_req_res = RIL_E_OEM_ERROR_3;
      break;
    }

    ril_req_res = RIL_E_SUCCESS;
  }while(FALSE);

  memset ( &lce_status_resp, 0, sizeof( lce_status_resp ) );
  if(ril_req_res == RIL_E_SUCCESS)
  {
    lce_status_resp.lce_status = LQE_STATUS_STOPPED;
  }
  else if(ril_req_res == RIL_E_LCE_NOT_SUPPORTED)
  {
    QCRIL_LOG_ERROR( "%s", "respond to QCRIL as LQE not supported" );
    lce_status_resp.lce_status = LQE_STATUS_NOT_SUPPORTED;
  }
  else
  {
    QCRIL_LOG_ERROR( "%s", "respond to QCRIL as command execution failure" );
  }

  qcril_default_request_resp_params( instance_id,
                                     params_ptr->t,
                                     params_ptr->event_id,
                                     ril_req_res,/* revisit on error code */
                                     &resp );

  resp.resp_pkt = &lce_status_resp;
  resp.resp_len = sizeof( lce_status_resp );
  qcril_send_request_response( &resp );

  QCRIL_DATA_MUTEX_UNLOCK(LQE_MUTEX);

bail:
  QCRIL_LOG_INFO( "qcril_data_lqe_stop completed %d", (int) ril_req_res );
}

/*===========================================================================

  FUNCTION:  qcril_data_lqe_get_info

===========================================================================*/
/*!
    @brief

    Handles RIL_REQUEST_PULL_LCEDATA.

    @pre Before calling, info_tbl_mutex must not be locked by the calling thread

    @return

    None on Success, Generic failure response on Failure
*/
/*=========================================================================*/
void qcril_data_lqe_get_info
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr
)
{
  qcril_instance_id_e_type        instance_id;
  qcril_modem_id_e_type           modem_id;
  qcril_request_resp_params_type  resp;
  int                             dl_rate = 0;
  int                             confidence_level = 0;
  RIL_LceDataInfo                 lqe_data_resp;
  lqe_downlink_throughput_report_status lqe_dl_throughput_status;

  RIL_Errno ril_req_res = RIL_E_INTERNAL_ERR;

  QCRIL_LOG_INFO( "%s", "qcril_data_lqe_get_info entered" );

  /* validate input parameter */
  if ( ( NULL == params_ptr ) || ( NULL == ret_ptr ) )
  {
    QCRIL_LOG_ERROR( "BAD input, params_ptr [%p], ret_ptr [%p]",
                     (unsigned int *)params_ptr, (unsigned int *)ret_ptr);
    goto bail;
  }

  instance_id = params_ptr->instance_id;
  modem_id = params_ptr->modem_id;

  if (instance_id >= QCRIL_MAX_INSTANCE_ID ||
      modem_id >= QCRIL_MAX_MODEM_ID)
  {
    QCRIL_LOG_ERROR("BAD iput, instance_id [%d], modem_id [%d]", instance_id, modem_id);
    goto bail;
  }

  /* input parameter is good, proceed... */
  QCRIL_DATA_MUTEX_LOCK(LQE_MUTEX);

  do
  {
    memset(&lqe_dl_throughput_status, 0, sizeof(lqe_dl_throughput_status));
    if(LQE_SUCCESS != lqei_get_min_downlink_throughput_frequency(lqe_handle,
                                                 &lqe_dl_throughput_status))
    {
      QCRIL_LOG_ERROR( "%s", "failed to get min downlink throughput freq" );
      break;
    }

    if(!lqe_dl_throughput_status.reporting_status_valid)
    {
      QCRIL_LOG_ERROR( "%s", "Received invalid LCE status from modem" );
      break;
    }

    if(lqe_dl_throughput_status.reporting_status == LQE_THRPT_REPORT_DISABLED_UNSUPPORTED_RAT)
    {
      ril_req_res = RIL_E_RADIO_NOT_AVAILABLE;
      QCRIL_LOG_ERROR( "%s", "respond to QCRIL radio not available" );
      break;
    }

    if ( LQE_SUCCESS != lqei_get_downlink_throughput_info(lqe_handle,
                                                          &dl_rate,
                                                          &confidence_level) )
    {
      QCRIL_LOG_ERROR( "unable to get LQE info, handle [%d]", lqe_handle );
      ril_req_res = RIL_E_OEM_ERROR_3;
      break;
    }
    /* 0xFFFFFFFF is a special value that indicates no available data.
     * Handle this explicitly and return a rate of 0.
     */
    if (dl_rate == (int) 0xffffffff)
    {
        QCRIL_LOG_INFO( "No data to report");
        dl_rate = 0;
    }

    ril_req_res = RIL_E_SUCCESS;
  }while(FALSE);

  memset ( &lqe_data_resp, 0, sizeof( lqe_data_resp ) );
  if(ril_req_res == RIL_E_SUCCESS)
  {
    lqe_data_resp.last_hop_capacity_kbps = dl_rate;
    lqe_data_resp.confidence_level = CONFIDENCE_IN_PERCENTAGE(confidence_level);

    QCRIL_LOG_INFO( "qcril_data_lqe_get_info last_hop_capacity_kbps [%d] confidence_level [%d]",
                    lqe_data_resp.last_hop_capacity_kbps, lqe_data_resp.confidence_level);

  }
  else
  {
    QCRIL_LOG_ERROR( "%s", "respond to QCRIL as generic failure" );
  }

  qcril_default_request_resp_params( instance_id,
                                     params_ptr->t,
                                     params_ptr->event_id,
                                     ril_req_res,/* revisit on error code */
                                     &resp );

  resp.resp_pkt = &lqe_data_resp;
  resp.resp_len = sizeof( lqe_data_resp );
  qcril_send_request_response( &resp );

  QCRIL_DATA_MUTEX_UNLOCK(LQE_MUTEX);

bail:
  QCRIL_LOG_INFO( "qcril_data_lqe_get_info completed %d", (int) ril_req_res );
}

bool qcril_data_is_threshold_based_reporting_supported()
{
  return is_threshold_based_reporting_supported;
}

#if (QCRIL_RIL_VERSION >= 15)

int qcril_data_lqe_toggle_reporting(qcril_data_lce_rpt_state state,
  lqe_data_direction_e direction)
{
  if(!lqe_inited) {
    QCRIL_LOG_ERROR("LCE is not initialized!");

    return -1;
  }

  if(!is_threshold_based_reporting_supported) {
    QCRIL_LOG_INFO("Threshold based reporting not supported!");
    return RIL_E_LCE_NOT_SUPPORTED;
  }

  int ret = LQE_FAILURE;

  ret = lqei_toggle_throughput_reporting(lqe_handle,
                                           direction,
                                           state==lce_report_off?
                                           LQE_TURN_OFF_INDICATIONS
                                           : LQE_TURN_ON_INDICATIONS);

  if (ret != LQE_SUCCESS) {
    QCRIL_LOG_ERROR("Failed to change reporting state");
    return -1;
  }
  return 0;
}

int qcril_data_lqe_set_rpt_criteria(qcril_data_lce_rpt_criteria* criteria)
{
  int ret = LQE_FAILURE;

  /* LQE must be initialized */
  if(!lqe_inited) {
    QCRIL_LOG_ERROR("LCE is not initialized!");
    return ret;
  }

  if(!is_threshold_based_reporting_supported) {
    QCRIL_LOG_INFO("Threshold based reporting not supported!");
    return RIL_E_LCE_NOT_SUPPORTED;
  }

  /* criteria must be available */
  if (criteria == NULL) {
    QCRIL_LOG_ERROR("criteria cannot be NULL!");
    return ret;
  }

  /* log the reporting criteria */
  QCRIL_LOG_INFO("LQE: setting reporting criteria: RAN 0x%x, " \
      "hyst = %d ms, hystDL = %d kbps, hystUL = %d kbps " \
      "with %d DL levels, and %d UL levels",
      criteria->ran,
      criteria->hysteresisMs,
      criteria->hysteresisDlKbps,
      criteria->hysteresisUlKbps,
      criteria->thresholdsDownlinkKbps_size,
      criteria->thresholdsUplinkKbps_size);

  /* hysteresis cannot be negative */
  if (criteria->hysteresisMs < 0 ||
      criteria->hysteresisDlKbps < 0 ||
      criteria->hysteresisUlKbps < 0) {
    QCRIL_LOG_ERROR("hysteresis criteria cannot be negative!");
    return LQE_INVALID_ARGUMENTS;
  }

  /* UL threshold levels */
  int i = 0;
  for(i = 0;
      i < criteria->thresholdsUplinkKbps_size &&
      i <= QMI_OTT_UL_THRESHOLD_LIST_LEN_V01; i++)
  {
    QCRIL_LOG_INFO("LQE: setting threshUL[%d] = %d kbps",
        i,
        criteria->thresholdsUplinkKbps[i]);
  }

  /* DL threshold levels */
  for(i = 0;
      i < criteria->thresholdsDownlinkKbps_size &&
      i <= QMI_OTT_DL_THRESHOLD_LIST_LEN_V01; i++)
  {
    QCRIL_LOG_INFO("LQE: setting threshDL[%d] = %d kbps",
        i,
        criteria->thresholdsDownlinkKbps[i]);
  }

  if (criteria->thresholdsDownlinkKbps_size >= 2) {
    int32_t delta = criteria->thresholdsDownlinkKbps[1]-criteria->thresholdsDownlinkKbps[0];
    for (int i=2 ; i<criteria->thresholdsDownlinkKbps_size ; i++) {
      delta = MIN(delta, criteria->thresholdsDownlinkKbps[i]-criteria->thresholdsDownlinkKbps[i-1]);
    }

    if (criteria->hysteresisDlKbps >= delta) {
      QCRIL_LOG_ERROR("hysteresisDlKbps must be smaller than the smallest DL threshold delta!");
      return LQE_INVALID_ARGUMENTS;
    }
  }

  if (criteria->thresholdsUplinkKbps_size >= 2) {
    int32_t delta = criteria->thresholdsUplinkKbps[1]-criteria->thresholdsUplinkKbps[0];
    for (int i=2 ; i<criteria->thresholdsUplinkKbps_size ; i++) {
      delta = MIN(delta, criteria->thresholdsUplinkKbps[i]-criteria->thresholdsUplinkKbps[i-1]);
    }

    if (criteria->hysteresisUlKbps >= delta) {
      QCRIL_LOG_ERROR("hysteresisUlKbps must be smaller than the smallest UL threshold delta!");
      return LQE_INVALID_ARGUMENTS;
    }
  }

  ret = lqei_set_throughput_indication_criteria
    ( lqe_handle,
      LQE_DATA_UPLINK_DIRECTION,
      criteria );
  if (ret != LQE_SUCCESS) {
    QCRIL_LOG_ERROR("Failed to set uplink criteria");
    return ret;
  }

  ret = lqei_set_throughput_indication_criteria
        ( lqe_handle,
          LQE_DATA_DOWNLINK_DIRECTION,
          criteria );
  if (ret != LQE_SUCCESS) {
    QCRIL_LOG_ERROR("Failed to set downlink criteria");
    return ret;
  }
  return ret;
}

#endif /* QCRIL_RIL_VERSION >= 15) */

#endif /* FEATURE_DATA_LQE */
