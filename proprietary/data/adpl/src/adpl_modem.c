/******************************************************************************

                        ADPL_MODEM.C

******************************************************************************/

/******************************************************************************

  @file    adpl_modem.c
  @brief   Accelerated Data Path Logging module for ADPL logging. This file contains
           ADPL interaction with modem interconnect

  DESCRIPTION
  Implementation file for ADPL interaction with modem interconnect.

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
#include <stdint.h>
#include <linux/msm_rmnet.h>
#include <errno.h>

#include "adpl.h"
#include "data_common_v01.h"
#include "data_port_mapper_v01.h"
#include "qmi_client_instance_defs.h"

extern qmi_client_os_params          adpl_dpm_os_params;
static  adpl_param                * adpl_state_config = NULL;
static  odl_param                 * odl_state_config = NULL;
boolean                              is_ssr_in_progress = true;

/*===========================================================================
                          FUNCTION DEFINITIONS

============================================================================*/
/*===========================================================================

FUNCTION ADPL_MODEM_EVENT_CB()

DESCRIPTION

  This function
  - handles SSR events.

DEPENDENCIES
  None.

RETURN VALUE
  ADPL_SUCCESS on success
  ADPL_FAILURE on failure

SIDE EFFECTS
  None

=========================================================================*/
static void adpl_modem_event_cb
(
  qmi_sys_event_type              event_id,
  const qmi_sys_event_info_type   *event_info,
  void                            *user_data
)
{
#ifndef FEATURE_MDM_LE
  int                             ret_val;
  int                             line_state;
  int                             ph_iface_fd = 0;
/*----------------------------------------------------------------------*/
  (void) user_data;

  if (adpl_state_config == NULL)
  {
    LOG_MSG_ERROR("ADPL state config not set", 0, 0, 0);
    return;
  }

  LOG_MSG_INFO1("Received QMUXD cb event id %d", event_id, 0, 0);
  if( DS_TARGET_FUSION_SDM845_SDX24 == adpl_state_config->target ||
      DS_TARGET_FUSION_8084_9X45 == adpl_state_config->target ||
      DS_TARGET_FUSION4_5_PCIE == adpl_state_config->target ||
      DS_TARGET_FUSION_8096_9X55 == adpl_state_config->target )
  {
    if(event_id == QMI_SYS_EVENT_MODEM_OUT_OF_SERVICE_IND)
    {
      LOG_MSG_INFO1("Received QMUXD cb event on conn id %d",
                    event_info->qmi_modem_service_ind.conn_id, 0, 0);

      if(event_info->qmi_modem_service_ind.conn_id ==
         adpl_state_config->qmux_conn_id)
      {
        if(adpl_state_config->adpl_iface_fd > 0)
        {
          ret_val = ioctl(adpl_state_config->adpl_iface_fd, FRMNET_CTRL_GET_LINE_STATE, &line_state);
          if (ret_val)
          {
            LOG_MSG_INFO1("Couldn't get FRMNET LINE STATE from ADPL driver", 0, 0, 0);
          }
          else if (line_state == 0)
          {
            LOG_MSG_INFO1("USB cable not connected. Dont reset modem rmnet components for ADPL",
                           0, 0, 0);
            return;
          }
          else if (line_state == 1)
          {
            ret_val = adpl_data_teardown_bridge(MHI_ADPL_DATA_INTERFACE, USB_ADPL_DATA_INTERFACE);
            if (ret_val != ADPL_SUCCESS)
            {
              LOG_MSG_ERROR("Failed to teardown data bridge driver for ADPL", 0, 0, 0);
            }
          }
        }

        LOG_MSG_INFO1("Completed processing modem out of service ind", 0, 0, 0);
      }
    }
    else if(event_id == QMI_SYS_EVENT_MODEM_IN_SERVICE_IND)
    {
      if(event_info->qmi_modem_service_ind.conn_id ==
         adpl_state_config->qmux_conn_id)
      {
        if(adpl_state_config->adpl_iface_fd > 0)
        {
          ret_val = ioctl(adpl_state_config->adpl_iface_fd, FRMNET_CTRL_GET_LINE_STATE, &line_state);
          if (ret_val)
          {
            LOG_MSG_INFO1("Couldn't get FRMNET LINE STATE from ADPL driver", 0, 0, 0);
          }
          else if (line_state == 0)
          {
            LOG_MSG_INFO1("USB cable not connected. Dont init modem rmnet components for ADPL",
                           0, 0, 0);
            return;
          }
          else if (line_state == 1)
          {
            ret_val = adpl_data_init_bridge(MHI_ADPL_DATA_INTERFACE, USB_ADPL_DATA_INTERFACE);
            if (ret_val != ADPL_SUCCESS)
            {
              LOG_MSG_ERROR("Failed to init data bridge driver for ADPL", 0, 0, 0);
            }
          }
        }

        LOG_MSG_INFO1("Completed processing modem in service ind", 0, 0, 0);
      }
    }
    else
    {
      LOG_MSG_INFO1("Ignoring event %d", event_id, 0, 0);
    }
  }
#endif //FEATURE_MDM_LE
}

/*===========================================================================

FUNCTION ADPL_MODEM_INIT_THRU_QMUXD()

DESCRIPTION

  This function
  - initializes RMNET modem state parameters.

DEPENDENCIES
  None.

RETURN VALUE
  ADPL_SUCCESS on success
  ADPL_FAILURE on failure

SIDE EFFECTS
  None

=========================================================================*/

int adpl_modem_init_thru_qmuxd
(
  adpl_param          * adpl_state,
  qcmap_sock_thrd_fd_read_f read_f
)
{
  int qmi_err = QMI_NO_ERR;
/*-------------------------------------------------------------------------*/

  if (adpl_state == NULL)
  {
    LOG_MSG_ERROR("Invalid ADPL state data received", 0, 0, 0);
    return ADPL_FAILURE;
  }

  adpl_state_config = adpl_state;

/*------------------------------------------------------------------------
  Initialize a QMUX client and obtain a handle
-------------------------------------------------------------------------*/
  qmi_err = qmi_qmux_if_pwr_up_init_ex((qmi_qmux_if_rx_msg_hdlr_type)read_f,
                                       adpl_modem_event_cb,
                                       NULL,
                                       &adpl_state_config->adpl_qmux_qmi_handle,
                                       QMI_QMUX_IF_CLNT_MODE_RAW);

  if (qmi_err != QMI_NO_ERR)
  {
    LOG_MSG_ERROR("Failed to get a QMUX handle for ADPL", 0, 0, 0);
    return ADPL_FAILURE;
  }
  else
  {
    LOG_MSG_INFO1("Succeeded to get a QMUX handle for ADPL", 0, 0, 0);
  }

  return ADPL_SUCCESS;
}


/*===========================================================================

FUNCTION ADPL_MODEM_RECV_MSG_THRU_QMUXD()

DESCRIPTION

  This function
  - receives QMI messages from modem

DEPENDENCIES
  None.

RETURN VALUE
  ADPL_SUCCESS on success
  ADPL_FAILURE on failure

SIDE EFFECTS
  None

=========================================================================*/


void adpl_modem_recv_msg_thru_qmuxd
(
  qmi_connection_id_type conn_id,
  qmi_service_id_type    service_id,
  qmi_client_id_type     client_id,
  unsigned char          control_flags,
  unsigned char         *rx_msg,
  int                    rx_msg_len
)
{
  return;
}

/*===========================================================================

FUNCTION ADPL_MODEM_INIT()

DESCRIPTION

  This function
  - initializes modem state parameters.

DEPENDENCIES
  None.

RETURN VALUE
  ADPL_SUCCESS on success
  ADPL_FAILURE on failure

SIDE EFFECTS
  None

=========================================================================*/

int adpl_modem_init
(
  adpl_param  * adpl_state,
  odl_param   * odl_state
)
{
  int ret_val;
/*-------------------------------------------------------------------------*/

  if (adpl_state == NULL || odl_state == NULL)
  {
    LOG_MSG_ERROR("Invalid adpl/odl state data received", 0, 0, 0);
    return ADPL_FAILURE;
  }

  adpl_state_config = adpl_state;
  odl_state_config = odl_state;

  return ADPL_SUCCESS;
}
/*===========================================================================

FUNCTION ADPL_MODEM_GET_EP_ID()

DESCRIPTION
  - obtains PCIe EP ID

DEPENDENCIES
  None.

RETURN VALUE
  ADPL_SUCCESS on success
  ADPL_FAILURE on failure

SIDE EFFECTS
  None

==========================================================================*/

int adpl_modem_get_ep_id
(
  uint32_t *ep_id,
  char* data_iface
)
{
  int rval = ADPL_FAILURE;
  int ep_type;
/*-----------------------------------------------------------------------*/
  if(ep_id == NULL)
  {
    LOG_MSG_ERROR("Got invalid EP ID", 0, 0, 0);
    return ADPL_FAILURE;
  }
  if (adpl_state_config == NULL)
  {
    LOG_MSG_ERROR("ADPL state config not set", 0, 0, 0);
    return rval;
  }
  *ep_id = DS_EP_ID_INVALID;
  ds_get_epid(data_iface, &ep_type, ep_id);

  if (ep_id == DS_EP_ID_INVALID)
  {
    LOG_MSG_ERROR("get_epid: ds RMNET_IOCTL_GET_EPID failed err %d", errno, 0, 0);
    goto error;
  }
  LOG_MSG_INFO1("get_epid: ds_get_epid succesful  ep_id[%d] ",*ep_id, 0, 0);
  rval = ADPL_SUCCESS;

error:
  return rval;
}

/*===========================================================================

FUNCTION ADPL_MODEM_SET_MRU()

DESCRIPTION
  - sets MRU on modem interface

DEPENDENCIES
  None.

RETURN VALUE
  ADPL_SUCCESS on success
  ADPL_FAILURE on failure

SIDE EFFECTS
  None

==========================================================================*/
int adpl_modem_set_mru
(
  uint32_t mru,
  char* data_iface
)
{
  int fd;
  int rval = ADPL_FAILURE;
  struct ifreq ifr;
  struct rmnet_ioctl_extended_s *ext_ioctl_arg;
/*-----------------------------------------------------------------------*/

  if (adpl_state_config == NULL)
  {
    LOG_MSG_ERROR("ADPL state config not set", 0, 0, 0);
    return rval;
  }

  /* Open a datagram socket to use for issuing the ioctl */
  if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
  {
    LOG_MSG_ERROR("set_mtu: socket failed", 0, 0, 0);
    goto error;
  }

  /* Allocate the extended IOCTL argument */
  ext_ioctl_arg = (void *) malloc (sizeof(struct rmnet_ioctl_extended_s));
  if (NULL == ext_ioctl_arg)
  {
    LOG_MSG_ERROR("set_mru: malloc for ifru_data failed", 0, 0, 0);
    close(fd);
    goto error;
  }


  /* Initialize the ioctl req struct */
  memset(&ifr, 0, sizeof(ifr));
  strlcpy(ifr.ifr_name,
          data_iface,
          strlen(MHI_ADPL_DATA_INTERFACE)+1);
  ifr.ifr_ifru.ifru_data = ext_ioctl_arg;
  memset(ext_ioctl_arg, 0, sizeof(struct rmnet_ioctl_extended_s));
  ext_ioctl_arg->extended_ioctl = RMNET_IOCTL_SET_MRU;
  ext_ioctl_arg->u.data = mru;

  /* Get current if flags for the device */
  if (ioctl(fd, RMNET_IOCTL_EXTENDED, &ifr) < 0)
  {
    LOG_MSG_ERROR("set_mru: ioctl RMNET_IOCTL_SET_MRU failed err %d", errno, 0, 0);
    close(fd);
    free(ext_ioctl_arg);
    goto error;
  }

  LOG_MSG_INFO1("set_mru: ioctl RMNET_IOCTL_SET_MRU as %d",
                ext_ioctl_arg->u.data, 0, 0);

  /* close temporary socket */
  close(fd);
  free(ext_ioctl_arg);
  rval = ADPL_SUCCESS;

error:
  return rval;
}

/*===========================================================================

FUNCTION ADPL_MODEM_NOT_IN_SERVICE()

DESCRIPTION
  Function invoked during SSR functionality, when modem is not in service.

DEPENDENCIES
  None.

RETURN VALUE

SIDE EFFECTS
  None

==========================================================================*/
void adpl_modem_not_in_service()
{
  int        ret_val;
  qmi_idl_service_object_type dpm_svc_obj;
  int rc = QMI_INTERNAL_ERR;

#ifndef FEATURE_MDM_LE

  if (adpl_state_config == NULL || odl_state_config == NULL)
  {
    LOG_MSG_ERROR("ADPL/ODL state config not set", 0, 0, 0);
    return;
  }

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
      (DS_TARGET_FEEROLITE6 == adpl_state_config->target)||
      (DS_TARGET_STARLORD == adpl_state_config->target) ||
      (DS_TARGET_STARHAWK == adpl_state_config->target) ||
      (DS_TARGET_WARLOCK  == adpl_state_config->target) ||
      (DS_TARGET_LITO == adpl_state_config->target) ||
      (DS_TARGET_MSMSTEPPE == adpl_state_config->target))
  {
    dpm_svc_obj = dpm_get_service_object_v01();
    if( dpm_svc_obj == NULL )
    {
      LOG_MSG_ERROR("Failed to get dpm service object", 0, 0, 0);
      return;
    }

    ret_val = adpl_dpm_release();
    if (ret_val != ADPL_SUCCESS)
    {
      LOG_MSG_ERROR("Failed to release DPM client", 0, 0, 0);
    }

    is_ssr_in_progress = true;
    odl_state_config->odl_enabled = FALSE;
    odl_state_config->is_odl_ipa_enabled = ODL_INVALID_VALUE;
    /*Reset this variable to call adpl_process_ph_reset in adpl_modem_in_service for SSR*/
    adpl_state_config->dtr_enabled = 0;
    LOG_MSG_INFO1("adpl modem not in service"
                  " going to register DPM call back\n",0, 0,0);
/*----------------------------------------------------------------------------
    Register icallback to get notified when modem is in service
---------------------------------------------------------------------------- */
    (void) qmi_client_notifier_init(dpm_svc_obj, &adpl_dpm_os_params, &dpm_notifier);
    rc = qmi_client_register_notify_cb(dpm_notifier,dpm_service_available_cb , (void *)NULL);
    if (QMI_NO_ERR != rc)
    {
      LOG_MSG_ERROR("failed to register dpm notifier. clnt_hndl=%p rc=%d!\n",
                    dpm_notifier, rc,0);
    }

    LOG_MSG_INFO1("Completed processing modem out of service ind", 0, 0, 0);
  }

#endif
}

/*===========================================================================

FUNCTION ADPL_MODEM_IN_SERVICE()

DESCRIPTION
  Function is used during SSR functionality, when modem is back in service.

DEPENDENCIES
  None.

RETURN VALUE

SIDE EFFECTS
  None

==========================================================================*/

void adpl_modem_in_service()
{

#ifndef FEATURE_MDM_LE

  if (adpl_state_config == NULL || odl_state_config == NULL)
  {
    LOG_MSG_ERROR("ADPL/ODL state config not set", 0, 0, 0);
    goto bail ;
  }

  if( (DS_TARGET_MSM8994 == adpl_state_config->target) ||
      (DS_TARGET_JOLOKIA == adpl_state_config->target) ||
      (DS_TARGET_MSM8992 == adpl_state_config->target) ||
      (DS_TARGET_MSM8996 == adpl_state_config->target) ||
      (DS_TARGET_MSM8998 == adpl_state_config->target) ||
      (DS_TARGET_MSMNILE == adpl_state_config->target) ||
      (DS_TARGET_FUSION_SM8150_SDX50 == adpl_state_config->target) ||
      (DS_TARGET_SDM845 == adpl_state_config->target) ||
      (DS_TARGET_TABASCO == adpl_state_config->target) ||
      (DS_TARGET_ELDARION == adpl_state_config->target)||
      (DS_TARGET_FEERO == adpl_state_config->target) ||
      (DS_TARGET_JACALA == adpl_state_config->target) ||
      (DS_TARGET_FEERO6 == adpl_state_config->target)||
      (DS_TARGET_FEEROLITE6 == adpl_state_config->target)||
      (DS_TARGET_STARLORD == adpl_state_config->target) ||
      (DS_TARGET_STARHAWK == adpl_state_config->target) ||
      (DS_TARGET_WARLOCK  == adpl_state_config->target) ||
      (DS_TARGET_LITO == adpl_state_config->target) ||
      (DS_TARGET_MSMSTEPPE == adpl_state_config->target))
  {
    adpl_process_ph_reset();
    if (odl_state_config->is_odl_ipa_enabled > 0)
    {
      if (odl_process_ph_reset())
      {
        LOG_MSG_INFO1("Successfully reset the odl process", 0, 0, 0);
      }
    }
    LOG_MSG_INFO1("Completed processing modem in service ind", 0, 0, 0);
    bail:
      is_ssr_in_progress = false;
      return ;
  }
  else
  {
    LOG_MSG_INFO1("Invalid Target Name:%s",
                  ds_get_target_str(adpl_state_config->target), 0, 0);
  }

#endif
}

/*===========================================================================

FUNCTION ADPL_IS_IFACE_UP()

DESCRIPTION
- checks if the interface is up

DEPENDENCIES
  None.

RETURN VALUE
  TRUE on success
  FALSE on failure

SIDE EFFECTS
  None

==========================================================================*/
int adpl_is_iface_up
(
  char* data_iface
)
{
  int fd;
  int rval = FALSE;
  struct ifreq ifr;
/*-----------------------------------------------------------------------*/
  /* Open a datagram socket to use for issuing the ioctl */
  if( (fd = socket( AF_INET, SOCK_DGRAM, 0 )) < 0 )
  {
    LOG_MSG_ERROR("ifioctl: socket failed", 0, 0, 0);
    goto error;
  }

  /* Initialize the ioctl req struct to null */
  memset(&ifr, 0, sizeof(ifr));

  /* Set device name in ioctl req struct */
  strlcpy(ifr.ifr_name,
          data_iface,
          strlen(MHI_ADPL_DATA_INTERFACE)+1);


  /* Check if iface is up*/
  if( ioctl( fd, SIOCGIFFLAGS, &ifr ) < 0 )
  {
    LOG_MSG_ERROR("ifioctl_set: SIOCGIFFLAGS ioctl failed err %d", errno, 0, 0);
    close(fd);
    goto error;
  }

  if(ifr.ifr_flags & IFF_UP)
  {
    rval = TRUE;
  }

  /* Close temporary socket */
  close(fd);

  error:
  return rval;

}
