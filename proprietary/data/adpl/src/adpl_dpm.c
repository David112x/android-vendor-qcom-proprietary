/******************************************************************************

                        ADPL_DPM.C

******************************************************************************/

/******************************************************************************

  @file    adpl_dpm.c
  @brief   Accelerated Data Path Logging module for Data Port Mapper interaction.


  DESCRIPTION
  This file has functions which interact with DPM QMI service for
  ADPL logging.

  ---------------------------------------------------------------------------
  Copyright (c) 2014-2015,2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------


******************************************************************************/


/******************************************************************************

                      EDIT HISTORY FOR FILE

when       who        what, where, why
--------   ---        -------------------------------------------------------
04/25/18   vv         Initial version

******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <asm/types.h>
#include <fcntl.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#include "adpl.h"
#include "data_common_v01.h"
#include "data_port_mapper_v01.h"
#include "qmi_client_instance_defs.h"

static  adpl_param          * adpl_state_config = NULL;
static  odl_param           * odl_state_config = NULL;
qmi_client_os_params           adpl_dpm_os_params;
qmi_idl_service_object_type    adpl_dpm_service_object;
qmi_client_os_params           adpl_dpm_os_params_init;
extern                         boolean is_ssr_in_progress;
extern unsigned int            dpm_adpl_sockfd;

/*===========================================================================
                               FUNCTION DEFINITIONS
=========================================================================*/

/*===========================================================================

FUNCTION DPM_CLIENT_INIT()

DESCRIPTION
  Initializes a DPM QMI client

DEPENDENCIES
  None.

RETURN VALUE
  ADPL_SUCCESS on success
  ADPL_FAILURE on failure


SIDE EFFECTS
  None

=========================================================================*/

int dpm_client_init()
{
  qmi_client_error_type   qmi_error, qmi_err_code = QMI_NO_ERR;
  boolean                 dpm_client_inited = false;
  int                     clientRetry = 0;
  int                     ret_val;

  if(adpl_state_config == NULL || odl_state_config == NULL)
  {
    LOG_MSG_ERROR("ADPL/ODL state config not set", 0, 0, 0);
    return ADPL_FAILURE;
  }

/*-----------------------------------------------------------------------------
  Deregister QMUXD ports since we want the client init request to go over IPC router
------------------------------------------------------------------------------*/
  qmi_cci_qmux_xport_unregister(QMI_CLIENT_QMUX_RMNET_INSTANCE_0);
  qmi_cci_qmux_xport_unregister(QMI_CLIENT_QMUX_RMNET_USB_INSTANCE_0);
  qmi_cci_qmux_xport_unregister(QMI_CLIENT_QMUX_RMNET_SMUX_INSTANCE_0);
  qmi_cci_qmux_xport_unregister(QMI_CLIENT_QMUX_RMNET_MHI_INSTANCE_0);

  if (adpl_state_config->adpl_dpm_handle > 0)
  {
    LOG_MSG_INFO1("Already DPM client inited : %d", adpl_state_config->adpl_dpm_handle, 0, 0);
    return ADPL_SUCCESS;
  }

/*-----------------------------------------------------------------------------
  Client init
------------------------------------------------------------------------------*/
  while (clientRetry < ADPL_QMI_CLIENT_MAX_RETRY)
  {
    qmi_error = qmi_client_init_instance(adpl_dpm_service_object,
                                         QMI_CLIENT_INSTANCE_ANY,
                                         NULL,
                                         NULL,
                                         &adpl_dpm_os_params,
                                         ADPL_QMI_MSG_TIMEOUT_VALUE,
                                         &(adpl_state_config->adpl_dpm_handle));

    if (qmi_error != QMI_NO_ERR)
    {
      LOG_MSG_ERROR("Can not init DPM client %d", qmi_error, 0, 0);
      clientRetry++;
      continue;
    }
    LOG_MSG_INFO1(" DPM client inited %d", adpl_state_config->adpl_dpm_handle, 0, 0);
    odl_state_config->odl_dpm_handle = adpl_state_config->adpl_dpm_handle;
    dpm_client_inited = true;
    break;
  }

  if ( clientRetry == ADPL_QMI_CLIENT_MAX_RETRY )
  {
    LOG_MSG_ERROR("Can not init DPM client even after %d  attempts ",
                  ADPL_QMI_CLIENT_MAX_RETRY, 0, 0);
    return ADPL_FAILURE;
  }

  if (is_ssr_in_progress)
  {
    adpl_modem_in_service();
  }

#ifndef FEATURE_MDM_LE
  if( (DS_TARGET_MSM8994 == adpl_state_config->target) ||
      (DS_TARGET_JOLOKIA == adpl_state_config->target) ||
      (DS_TARGET_MSM8992 == adpl_state_config->target) ||
      (DS_TARGET_MSM8996 == adpl_state_config->target) ||
      (DS_TARGET_MSM8998 == adpl_state_config->target) ||
      (DS_TARGET_SDM845 == adpl_state_config->target) ||
      (DS_TARGET_MSMNILE == adpl_state_config->target) ||
      (DS_TARGET_FUSION_SM8150_SDX50 == adpl_state_config->target) ||
      (DS_TARGET_TABASCO == adpl_state_config->target) ||
      (DS_TARGET_ELDARION == adpl_state_config->target)||
      (DS_TARGET_FEERO == adpl_state_config->target) ||
      (DS_TARGET_JACALA == adpl_state_config->target) ||
      (DS_TARGET_FEERO6 == adpl_state_config->target)||
      (DS_TARGET_FEEROLITE6 == adpl_state_config->target) ||
      (DS_TARGET_STARLORD == adpl_state_config->target) ||
      (DS_TARGET_STARHAWK == adpl_state_config->target) ||
      (DS_TARGET_WARLOCK  == adpl_state_config->target) ||
      (DS_TARGET_LITO == adpl_state_config->target) ||
      (DS_TARGET_MSMSTEPPE == adpl_state_config->target))
  {
/*-----------------------------------------------------------------------------
  Register for err callback
------------------------------------------------------------------------------*/
    (void) qmi_client_register_error_cb(adpl_state_config->adpl_dpm_handle,
                                        dpm_erro_cb,
                                        (void *)NULL);
  }
#endif

  qmi_error = qmi_client_release(dpm_notifier);
  if (qmi_error != QMI_NO_ERR)
  {
    LOG_MSG_ERROR("Can not release DPM notifier client %d", qmi_error, 0, 0);
  }

  return ADPL_SUCCESS;
}

/*===========================================================================

FUNCTION ADPL_DPM_INIT()

DESCRIPTION
  Initializes a DPM service object and register
  for service indication callback.

DEPENDENCIES
  None.

RETURN VALUE
  ADPL_SUCCESS on success
  ADPL_FAILURE on failure


SIDE EFFECTS
  None

=========================================================================*/

int adpl_dpm_init
(
   adpl_param         * adpl_state,
   odl_param          * odl_state
)
{
  qmi_client_error_type  qmi_error, qmi_err_code = QMI_NO_ERR;
  int ret;
/*---------------------------------------------------------------------------*/

  if(adpl_state == NULL || odl_state == NULL)
  {
    LOG_MSG_ERROR("ADPL/ODL state not valid", 0, 0, 0);
    return ADPL_FAILURE;
  }

  LOG_MSG_INFO1("adpl_dpm_init()", 0, 0, 0);

/*-----------------------------------------------------------------------------
  Static pointer to ADPL config and ODL config
------------------------------------------------------------------------------*/
  adpl_state_config = adpl_state;
  odl_state_config = odl_state;
/*-----------------------------------------------------------------------------
  Get the service object
------------------------------------------------------------------------------*/
  adpl_dpm_service_object = dpm_get_service_object_v01();
  if (adpl_dpm_service_object == NULL)
  {
    LOG_MSG_ERROR("ADPL DPM service object not available",
                   0, 0, 0);
    return ADPL_FAILURE;
  }

  memset(&adpl_dpm_os_params, 0, sizeof(qmi_client_os_params));

  (void) qmi_client_notifier_init(adpl_dpm_service_object,
                                  &adpl_dpm_os_params_init,
                                  &dpm_notifier);

  (void) qmi_client_register_notify_cb(dpm_notifier,
                                       dpm_service_available_cb,
                                       (void *)NULL);

  LOG_MSG_INFO1("Successful DPM service registration", 0, 0, 0);

  return ADPL_SUCCESS;
}

/*===========================================================================

FUNCTION ADPL_DPM_RELEASE()

DESCRIPTION
  Releases a DPM QMI client

DEPENDENCIES
  None.

RETURN VALUE
  ADPL_SUCCESS on success
  ADPL_FAILURE on failure


SIDE EFFECTS
  None

=========================================================================*/

int adpl_dpm_release()
{
  qmi_client_error_type                 qmi_error;

/*------------------------------------------------------------------------*/

  if(adpl_state_config == NULL)
  {
    LOG_MSG_ERROR("ADPL state not valid", 0, 0, 0);
    return ADPL_FAILURE;
  }

  LOG_MSG_INFO1("adpl_dpm_release Entry", 0, 0, 0);

  qmi_error = qmi_client_release(adpl_state_config->adpl_dpm_handle);
  if (qmi_error != QMI_NO_ERR)
  {
    LOG_MSG_ERROR("Can not release DPM client %d", qmi_error, 0, 0);
    return ADPL_FAILURE;
  }
  adpl_state_config->adpl_dpm_handle = NULL;
  LOG_MSG_INFO1("Successfully deregistered DPM client", 0, 0, 0);

  return ADPL_SUCCESS;
}

/*===========================================================================
  FUNCTION adpl_dpm_process_msg
=============================================================================
@brief
  This function is to send the message to server socket

@input

@return
  void

@note

- Dependencies
- None

- Side Effects
- None
===========================================================================*/
void adpl_dpm_process_msg(adpl_event_e event)
{
  adpl_event_t event_buf;
  int numBytes=0, len=0;
  struct sockaddr_un cmd_adpl;

  if (event == ADPL_DPM_INIT_EVENT) {
    LOG_MSG_INFO1("Sending ADPL_DPM_INIT_EVENT event to server socket",0,0,0);
  }
  else if (event == ADPL_DPM_MODEM_NOT_IN_SERVICE_EVENT) {
    LOG_MSG_INFO1("Sending ADPL_DPM_MODEM_NOT_IN_SERVICE_EVENT "
                  "event to server socket",0,0,0);
  }
  cmd_adpl.sun_family = AF_UNIX;
  strlcpy(cmd_adpl.sun_path, DEV_SOC_ADPL_CMD_UDS_FILE, sizeof(DEV_SOC_ADPL_CMD_UDS_FILE));
  len = strlen(cmd_adpl.sun_path) + sizeof(cmd_adpl.sun_family);

  event_buf.event = event;
  if ((numBytes = sendto(dpm_adpl_sockfd, (void *)&event_buf, sizeof(adpl_event_t), 0,
                  (struct sockaddr *)&cmd_adpl, len)) == -1)
  {
    LOG_MSG_ERROR("Failed to send the event info. to server socket errno:%d", errno, 0, 0);
    return;
  }
  LOG_MSG_INFO1("Successfully sent the event info. to server socket", 0, 0, 0);

  return;
}

/*===========================================================================

FUNCTION DPM_ERROR_CB()

DESCRIPTION
  This function is invoked by QCCI framework when DPM service is no longer
  available on modem which in turn is used by ADPL to identify that modem is not
  in service

DEPENDENCIES
  None.

RETURN VALUE


SIDE EFFECTS
  None

=========================================================================*/
void dpm_erro_cb
(
  __attribute__((unused)) qmi_client_type user_handle,
  __attribute__((unused)) qmi_client_error_type error,
  __attribute__((unused)) void *err_cb_data
)
{
  LOG_MSG_INFO1("dpm_erro_cb",0,0,0);

  adpl_dpm_process_msg(ADPL_DPM_MODEM_NOT_IN_SERVICE_EVENT);
  return;
}

/*===========================================================================

FUNCTION DPM_SERVICE_AVAILABLE_CB()

DESCRIPTION
  This function is invoked by dpm, to notify ADPL when service is available.

DEPENDENCIES
  None.

RETURN VALUE

SIDE EFFECTS
  None

=========================================================================*/
void dpm_service_available_cb
(
   __attribute__((unused)) qmi_client_type user_handle,
   __attribute__((unused)) qmi_idl_service_object_type service_obj,
   __attribute__((unused)) qmi_client_notify_event_type service_event,
   __attribute__((unused)) void *notify_cb_data
)
{
  LOG_MSG_INFO1("dpm_service_available_cb \n",0, 0, 0);

  adpl_dpm_process_msg(ADPL_DPM_INIT_EVENT);
  return;
}

/*===========================================================================

FUNCTION ADPL_DPM_PORT_OPEN()

DESCRIPTION
  Opens the USB tethered ADPL port using DPM port open request

DEPENDENCIES
  None.

RETURN VALUE
  ADPL_SUCCESS on success
  ADPL_FAILURE on failure


SIDE EFFECTS
  None

=========================================================================*/

int adpl_dpm_port_open()
{
  dpm_open_port_req_msg_v01    adpl_dpm_port_open_req_msg;
  dpm_open_port_resp_msg_v01   adpl_dpm_port_open_resp_msg;
  qmi_client_error_type        qmi_error;
  int                          ret_val;
  char                         command[MAX_COMMAND_STR_LEN];

/*--------------------------------------------------------------------------*/

  if(adpl_state_config == NULL)
  {
    LOG_MSG_ERROR("ADPL state not valid", 0, 0, 0);
    return ADPL_FAILURE;
  }

  memset(&adpl_dpm_port_open_req_msg,
         0,
         sizeof(dpm_open_port_req_msg_v01));

  memset(&adpl_dpm_port_open_resp_msg,
         0,
         sizeof(dpm_open_port_resp_msg_v01));
/*-----------------------------------------------------------------------------
  Get the ADPL USB/IPA EP info
------------------------------------------------------------------------------*/
  ret_val = adpl_ph_get_ep_info();
  if (ret_val != ADPL_SUCCESS)
  {
    LOG_MSG_ERROR("Failed to get EP info", 0, 0, 0);
    return ADPL_FAILURE;
  }

/*-----------------------------------------------------------------------------
  Populate the DPM port open message
------------------------------------------------------------------------------*/
  adpl_dpm_port_open_req_msg.hardware_data_port_list_valid = TRUE;
  adpl_dpm_port_open_req_msg.hardware_data_port_list_len = 1;
  adpl_dpm_port_open_req_msg.hardware_data_port_list[0].ep_id.ep_type =
                  (data_ep_type_enum_v01)(adpl_state_config->ep_info.ph_ep_info.ep_type);
  adpl_dpm_port_open_req_msg.hardware_data_port_list[0].ep_id.iface_id =
    adpl_state_config->ep_info.ph_ep_info.peripheral_iface_id;

  adpl_dpm_port_open_req_msg.hardware_data_port_list[0].hardware_ep_pair.consumer_pipe_num =
    adpl_state_config->ep_info.ipa_ep_pair.consumer_pipe_num;

  adpl_dpm_port_open_req_msg.hardware_data_port_list[0].hardware_ep_pair.producer_pipe_num =
    adpl_state_config->ep_info.ipa_ep_pair.producer_pipe_num;

  LOG_MSG_INFO1_6("Sending to modem  Peripheral type %d, peripheral id %d, ipa consumer EP %d, ipa producer EP %d",
                 adpl_state_config->ep_info.ph_ep_info.ep_type,
                 adpl_state_config->ep_info.ph_ep_info.peripheral_iface_id,
                 adpl_state_config->ep_info.ipa_ep_pair.consumer_pipe_num,
                 adpl_state_config->ep_info.ipa_ep_pair.producer_pipe_num, 0, 0);
/*-----------------------------------------------------------------------------
  Send the DPM port open QMI message
------------------------------------------------------------------------------*/
  qmi_error = qmi_client_send_msg_sync(adpl_state_config->adpl_dpm_handle,
                                       QMI_DPM_OPEN_PORT_REQ_V01,
                                       &adpl_dpm_port_open_req_msg,
                                       sizeof(dpm_open_port_req_msg_v01),
                                       &adpl_dpm_port_open_resp_msg,
                                       sizeof(dpm_open_port_resp_msg_v01),
                                       ADPL_QMI_MSG_TIMEOUT_VALUE);

  LOG_MSG_INFO1("qmi_client_send_msg_sync(dpm_port_open): error %d result %d",
                qmi_error,
                adpl_dpm_port_open_resp_msg.resp.result,
                0);

  if ((qmi_error != QMI_NO_ERR) ||
      (adpl_dpm_port_open_resp_msg.resp.result != QMI_NO_ERR))
  {
    LOG_MSG_ERROR( "Cannot open ADPL port %d : %d",
        qmi_error, adpl_dpm_port_open_resp_msg.resp.error,0);
    return ADPL_FAILURE;
  }

  LOG_MSG_INFO1( "ADPL port opened", 0, 0, 0);

  return ADPL_SUCCESS;
}

/*===========================================================================

FUNCTION ADPL_DPM_PORT_CLOSE()

DESCRIPTION
  Closes the USB tethered SMD port using DPM port close request

DEPENDENCIES
  None.

RETURN VALUE
  ADPL_SUCCESS on success
  ADPL_FAILURE on failure


SIDE EFFECTS
  None

=========================================================================*/


int adpl_dpm_port_close()
{
  dpm_close_port_req_msg_v01    adpl_dpm_port_close_req_msg;
  dpm_close_port_resp_msg_v01   adpl_dpm_port_close_resp_msg;
  qmi_client_error_type         qmi_error;

/*--------------------------------------------------------------------------*/

  if(adpl_state_config == NULL)
  {
    LOG_MSG_ERROR("RmNet state not valid", 0, 0, 0);
    return ADPL_FAILURE;
  }

  memset(&adpl_dpm_port_close_req_msg,
         0,
         sizeof(dpm_close_port_req_msg_v01));

  memset(&adpl_dpm_port_close_resp_msg,
         0,
         sizeof(dpm_close_port_resp_msg_v01));

/*-----------------------------------------------------------------------------
  Populate the DPM port close QMI message
------------------------------------------------------------------------------*/
  adpl_dpm_port_close_req_msg.data_port_list_valid = TRUE;
  adpl_dpm_port_close_req_msg.data_port_list_len = 1;
  adpl_dpm_port_close_req_msg.data_port_list[0].ep_type =
      (data_ep_type_enum_v01)(adpl_state_config->ep_info.ph_ep_info.ep_type);
  adpl_dpm_port_close_req_msg.data_port_list[0].iface_id =
    adpl_state_config->ep_info.ph_ep_info.peripheral_iface_id;
/*-----------------------------------------------------------------------------
  Send the QMI message
------------------------------------------------------------------------------*/
  qmi_error = qmi_client_send_msg_sync(adpl_state_config->adpl_dpm_handle,
                                       QMI_DPM_CLOSE_PORT_REQ_V01,
                                       &adpl_dpm_port_close_req_msg,
                                       sizeof(dpm_close_port_req_msg_v01),
                                       &adpl_dpm_port_close_resp_msg,
                                       sizeof(dpm_close_port_resp_msg_v01),
                                       ADPL_QMI_MSG_TIMEOUT_VALUE);

  LOG_MSG_INFO1("qmi_client_send_msg_sync(dpm_port_close): error %d result %d",
                qmi_error,
                adpl_dpm_port_close_resp_msg.resp.result,
                0);

  if ((qmi_error != QMI_NO_ERR) ||
      (adpl_dpm_port_close_resp_msg.resp.result != QMI_NO_ERR))
  {
    LOG_MSG_ERROR( "Cannot close port %d : %d",
        qmi_error, adpl_dpm_port_close_resp_msg.resp.error,0);
    return ADPL_FAILURE;
  }

  LOG_MSG_INFO1( "ADPL port closed", 0, 0, 0);

  return ADPL_SUCCESS;
}

/*===========================================================================

FUNCTION ODL_DPM_PORT_OPEN()

DESCRIPTION
  Opens DPM port open request with ODL endpoint

DEPENDENCIES
  None.

RETURN VALUE
  ADPL_SUCCESS on success
  ADPL_FAILURE on failure


SIDE EFFECTS
  None

=========================================================================*/

int odl_dpm_port_open()
{
  dpm_open_port_req_msg_v01    odl_dpm_port_open_req_msg;
  dpm_open_port_resp_msg_v01   odl_dpm_port_open_resp_msg;
  qmi_client_error_type        qmi_error;
  int                          ret_val;
  char                         command[MAX_COMMAND_STR_LEN];

/*--------------------------------------------------------------------------*/

  if(odl_state_config == NULL)
  {
    LOG_MSG_ERROR("ODL state not valid", 0, 0, 0);
    return ADPL_FAILURE;
  }

  memset(&odl_dpm_port_open_req_msg,
         0,
         sizeof(dpm_open_port_req_msg_v01));

  memset(&odl_dpm_port_open_resp_msg,
         0,
         sizeof(dpm_open_port_resp_msg_v01));
/*-----------------------------------------------------------------------------
  Get the ODL IPA EP info
------------------------------------------------------------------------------*/
  ret_val = odl_get_ep_info();
  if (ret_val != ADPL_SUCCESS)
  {
    LOG_MSG_ERROR("Failed to get EP info for ODL", 0, 0, 0);
    return ADPL_FAILURE;
  }

/*-----------------------------------------------------------------------------
  Populate the DPM port open message
------------------------------------------------------------------------------*/
  odl_dpm_port_open_req_msg.hardware_data_port_list_valid = TRUE;
  odl_dpm_port_open_req_msg.hardware_data_port_list_len = 1;
  odl_dpm_port_open_req_msg.hardware_data_port_list[0].ep_id.ep_type =
                  (data_ep_type_enum_v01)(odl_state_config->ep_info.ph_ep_info.ep_type);
  odl_dpm_port_open_req_msg.hardware_data_port_list[0].ep_id.iface_id =
    odl_state_config->ep_info.ph_ep_info.peripheral_iface_id;

  odl_dpm_port_open_req_msg.hardware_data_port_list[0].hardware_ep_pair.consumer_pipe_num =
    odl_state_config->ep_info.ipa_ep_pair.consumer_pipe_num;

  odl_dpm_port_open_req_msg.hardware_data_port_list[0].hardware_ep_pair.producer_pipe_num =
    odl_state_config->ep_info.ipa_ep_pair.producer_pipe_num;

  LOG_MSG_INFO1_6("Sending to modem  Peripheral type %d, peripheral id %d, ipa consumer EP %d, ipa producer EP %d",
                 odl_state_config->ep_info.ph_ep_info.ep_type,
                 odl_state_config->ep_info.ph_ep_info.peripheral_iface_id,
                 odl_state_config->ep_info.ipa_ep_pair.consumer_pipe_num,
                 odl_state_config->ep_info.ipa_ep_pair.producer_pipe_num, 0, 0);
/*-----------------------------------------------------------------------------
  Send the DPM port open QMI message
------------------------------------------------------------------------------*/
  qmi_error = qmi_client_send_msg_sync(odl_state_config->odl_dpm_handle,
                                       QMI_DPM_OPEN_PORT_REQ_V01,
                                       &odl_dpm_port_open_req_msg,
                                       sizeof(dpm_open_port_req_msg_v01),
                                       &odl_dpm_port_open_resp_msg,
                                       sizeof(dpm_open_port_resp_msg_v01),
                                       ADPL_QMI_MSG_TIMEOUT_VALUE);

  LOG_MSG_INFO1("qmi_client_send_msg_sync(dpm_port_open): error %d result %d",
                qmi_error,
                odl_dpm_port_open_resp_msg.resp.result,
                0);

  if ((qmi_error != QMI_NO_ERR) ||
      (odl_dpm_port_open_resp_msg.resp.result != QMI_NO_ERR))
  {
    LOG_MSG_ERROR( "Cannot open ODL port %d : %d",
        qmi_error, odl_dpm_port_open_resp_msg.resp.error,0);
    return ADPL_FAILURE;
  }

  LOG_MSG_INFO1( "ODL port opened", 0, 0, 0);
  return ADPL_SUCCESS;
}

/*===========================================================================

FUNCTION ODL_DPM_PORT_CLOSE()

DESCRIPTION
  DPM port close request for ODL endpoint

DEPENDENCIES
  None.

RETURN VALUE
  ADPL_SUCCESS on success
  ADPL_FAILURE on failure


SIDE EFFECTS
  None

=========================================================================*/


int odl_dpm_port_close()
{
  dpm_close_port_req_msg_v01    odl_dpm_port_close_req_msg;
  dpm_close_port_resp_msg_v01   odl_dpm_port_close_resp_msg;
  qmi_client_error_type         qmi_error;

/*--------------------------------------------------------------------------*/

  if(odl_state_config == NULL)
  {
    LOG_MSG_ERROR("ODL state not valid", 0, 0, 0);
    return ADPL_FAILURE;
  }

  memset(&odl_dpm_port_close_req_msg,
         0,
         sizeof(dpm_close_port_req_msg_v01));

  memset(&odl_dpm_port_close_resp_msg,
         0,
         sizeof(dpm_close_port_resp_msg_v01));

/*-----------------------------------------------------------------------------
  Populate the DPM port close QMI message
------------------------------------------------------------------------------*/
  odl_dpm_port_close_req_msg.data_port_list_valid = TRUE;
  odl_dpm_port_close_req_msg.data_port_list_len = 1;
  odl_dpm_port_close_req_msg.data_port_list[0].ep_type =
      (data_ep_type_enum_v01)(odl_state_config->ep_info.ph_ep_info.ep_type);
  odl_dpm_port_close_req_msg.data_port_list[0].iface_id =
    odl_state_config->ep_info.ph_ep_info.peripheral_iface_id;
/*-----------------------------------------------------------------------------
  Send the QMI message
------------------------------------------------------------------------------*/
  qmi_error = qmi_client_send_msg_sync(odl_state_config->odl_dpm_handle,
                                       QMI_DPM_CLOSE_PORT_REQ_V01,
                                       &odl_dpm_port_close_req_msg,
                                       sizeof(dpm_close_port_req_msg_v01),
                                       &odl_dpm_port_close_resp_msg,
                                       sizeof(dpm_close_port_resp_msg_v01),
                                       ADPL_QMI_MSG_TIMEOUT_VALUE);

  LOG_MSG_INFO1("qmi_client_send_msg_sync(dpm_port_close): error %d result %d",
                qmi_error,
                odl_dpm_port_close_resp_msg.resp.result,
                0);

  if ((qmi_error != QMI_NO_ERR) ||
      (odl_dpm_port_close_resp_msg.resp.result != QMI_NO_ERR))
  {
    LOG_MSG_ERROR( "Cannot close port %d : %d",
        qmi_error, odl_dpm_port_close_resp_msg.resp.error,0);
    return ADPL_FAILURE;
  }

  LOG_MSG_INFO1( "ODL port closed", 0, 0, 0);

  return ADPL_SUCCESS;
}


