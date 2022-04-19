/******************************************************************************

                        ADPL_PERIPHERAL.C

******************************************************************************/

/******************************************************************************

  @file    adpl_peripheral.c
  @brief   Accelerated Data Path Logging module for ADPL logging. This file contains
           ADPL interaction with peripheral

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
#include <sys/socket.h>
#include <sys/un.h>
#include <linux/msm_rmnet.h>
#include "linux/msm_ipa.h"

#include "adpl.h"

static adpl_param          * adpl_state_config = NULL;
static odl_param           * odl_state_config = NULL;
extern boolean                is_ssr_in_progress;

/*===========================================================================
                          FUNCTION DEFINITIONS
============================================================================*/
/*===========================================================================

FUNCTION PRINT_BUFFER()

DESCRIPTION

  This function
  - prints QMI message.

DEPENDENCIES
  None.

RETURN VALUE

SIDE EFFECTS
  None

=========================================================================*/
void print_buffer
(
  char    *buf,
  int      size
)
{
  int i;

  for(i=0; i < size; i++)
  {
    if(i%8 == 0)
      printf("\n%02X ", buf[i]);
    else
      printf("%02X ", buf[i]);
  }
  printf("\n");
}

/*===========================================================================

FUNCTION ADPL_FILE_OPEN()

DESCRIPTION

  This function
  - opens the device file which is used for interfacing with peripheral USB

DEPENDENCIES
  None.

RETURN VALUE

SIDE EFFECTS
  None

=========================================================================*/


int adpl_file_open
(
   char * adpl_dev_file,
   int  * fd
)
{
  int adpl_fd;
  int retry_count = 0;
/*--------------------------------------------------------------------------*/
  while ( retry_count < ADPL_FILE_NODE_OPEN_MAX_RETRY)
  {
    adpl_fd = open(adpl_dev_file, O_RDWR);
    if(adpl_fd < 0)
    {
      LOG_MSG_ERROR("Could not open device file. Errno %d error msg=%s",
                     errno, strerror (errno), 0);
      sleep(ONE_SEC);
      retry_count++;
      continue;
    }
    else
    {
      LOG_MSG_INFO1("Successfully opened device file. FD is %d",
                     adpl_fd, 0, 0);
      *fd = adpl_fd;
      return ADPL_SUCCESS;
    }
  }

  if ( retry_count == ADPL_FILE_NODE_OPEN_MAX_RETRY)
  {
    LOG_MSG_ERROR("Could not open device file. Abort. Errno %d error msg=%s",
                   errno, strerror (errno), 0);
    return ADPL_FAILURE;
  }

  return ADPL_SUCCESS;
}

/*===========================================================================

FUNCTION ADPL_PH_SET_MTU()

DESCRIPTION
  - sets the MTU on the peripheral interface

DEPENDENCIES
  None.

RETURN VALUE
  ADPL_SUCCESS on success
  ADPL_FAILURE on failure

SIDE EFFECTS
  None

==========================================================================*/
int adpl_ph_set_mtu
(
  uint32_t mtu,
  char* data_iface
)
{
  int fd;
  int rval = ADPL_FAILURE;
  struct ifreq ifr;
/*-----------------------------------------------------------------------*/

  /* Open a datagram socket to use for issuing the ioctl */
  if( (fd = socket( AF_INET, SOCK_DGRAM, 0 )) < 0 )
  {
    LOG_MSG_ERROR("ifioctl_set: socket failed", 0, 0, 0);
    goto error;
  }

  /* Initialize the ioctl req struct to null */
  memset(&ifr, 0, sizeof(ifr));

  /* Set device name in ioctl req struct */
  strlcpy(ifr.ifr_name,
          data_iface,
          strlen(USB_ADPL_DATA_INTERFACE)+1);

  ifr.ifr_data = (void*)(intptr_t)mtu;

  /* Set if MTU for the device */
  if( ioctl( fd, SIOCSIFMTU, &ifr ) < 0 )
  {
    LOG_MSG_ERROR("ifioctl_set: SIOCSIFMTU ioctl failed. errno %d", errno, 0, 0);
    close(fd);
    goto error;
  }

  /* Close temporary socket */
  close(fd);

  rval = ADPL_SUCCESS;

  error:
  return rval;

}

/*===========================================================================

FUNCTION ADPL_PROCESS_PH_RESET()

DESCRIPTION
  - processes peripheral reset for ADPL logging
  - upon USB cable plug-in : DPM open
  - upon USB cable plug-out : DPM close.

DEPENDENCIES
  None.

RETURN VALUE
  ADPL_SUCCESS on success
  ADPL_FAILURE on failure


SIDE EFFECTS
  None

=========================================================================*/
int adpl_process_ph_reset()
{
  int                             ret_val;
  int                             ret;
  int                             line_state;
  int                             retry_count = 0;
/*-------------------------------------------------------------------------*/

  if (adpl_state_config == NULL)
  {
    LOG_MSG_ERROR("ADPL state config not set", 0, 0, 0);
    return ADPL_FAILURE;
  }

  LOG_MSG_INFO1("Processing ADPL peripheral reset", 0, 0, 0);

  ret = ioctl(adpl_state_config->adpl_iface_fd,
              FRMNET_CTRL_GET_LINE_STATE,
              &line_state);
  if (ret)
  {
    LOG_MSG_INFO1("Couldn't get FRMNET LINE STATE from driver",
                   0, 0, 0);
    line_state = -1;
    return ADPL_FAILURE;
  }
  else
  {
    LOG_MSG_INFO1("ADPL line state %d", line_state, 0, 0);
    if(line_state == 1)
    {
      if(!adpl_state_config->dtr_enabled)
      {
        if(DS_TARGET_FUSION_SDM845_SDX24 == adpl_state_config->target ||
           DS_TARGET_FUSION4_5_PCIE == adpl_state_config->target ||
           DS_TARGET_FUSION_8084_9X45 == adpl_state_config->target ||
           DS_TARGET_FUSION_8096_9X55 == adpl_state_config->target )
        {
          while (retry_count < ADPL_SETUP_RETRY_MAX)
          {
            ret_val = adpl_data_init_bridge(MHI_ADPL_DATA_INTERFACE, USB_ADPL_DATA_INTERFACE);
            if (ret_val != ADPL_SUCCESS)
            {
              LOG_MSG_ERROR("Failed to initialize Bridge. Retrying", 0, 0, 0);
              sleep(2);
              retry_count++;
              continue;
            }
            else
            {
              LOG_MSG_INFO1("Initialized ADPL bridge", 0, 0, 0);
              break;
            }
          }
          if (retry_count == ADPL_SETUP_RETRY_MAX)
          {
            LOG_MSG_ERROR("Failed to initialize Bridge after 3 retries", 0, 0, 0);
            return ADPL_FAILURE;
          }
        }
        else
        {
          ret_val = adpl_dpm_port_open();
          if (ret_val != ADPL_SUCCESS)
          {
            LOG_MSG_ERROR("Failed to open ADPL port through DPM", 0, 0, 0);
            return ADPL_FAILURE;
          }
        }
        adpl_state_config->dtr_enabled = 1;
      }
    }
/*----------------------------------------------------------------------------
  ph cable plug out happened
----------------------------------------------------------------------------*/
    else if(line_state == 0)
    {
      if(adpl_state_config->dtr_enabled)
      {
        if(DS_TARGET_FUSION_SDM845_SDX24 == adpl_state_config->target ||
           DS_TARGET_FUSION4_5_PCIE == adpl_state_config->target ||
           DS_TARGET_FUSION_8084_9X45 == adpl_state_config->target ||
           DS_TARGET_FUSION_8096_9X55 == adpl_state_config->target )
        {
          ret_val = adpl_data_teardown_bridge(MHI_ADPL_DATA_INTERFACE, USB_ADPL_DATA_INTERFACE);
          if (ret_val != ADPL_SUCCESS)
          {
            LOG_MSG_ERROR("Failed to initialize Bridge", 0, 0, 0);
            return ADPL_FAILURE;
          }
        }
        else
        {
          ret_val = adpl_dpm_port_close();
          if (ret_val != ADPL_SUCCESS)
          {
            LOG_MSG_ERROR("Failed to close ADPL port through DPM", 0, 0, 0);
          }
        }
        adpl_state_config->dtr_enabled = 0;
      }
    }
  }

  LOG_MSG_INFO1("Completed ADPL peripheral reset processing", 0, 0, 0);

  return ADPL_SUCCESS;
}

/*===========================================================================

FUNCTION ADPL_PH_INIT()

DESCRIPTION

  This function
  - opens the ph device file for ADPL logging
  - adds the ph fd to wait on select call

DEPENDENCIES
  None.

RETURN VALUE
  ADPL_SUCCESS on success
  ADPL_FAILURE on failure

SIDE EFFECTS
  None

=========================================================================*/

int adpl_ph_init
(
  adpl_param           * adpl_state,
  qcmap_sk_fd_set_info_t    * fd_set,
  qcmap_sock_thrd_fd_read_f read_f
)
{
  int ret_val;
/*-------------------------------------------------------------------------*/
  if (adpl_state == NULL)
  {
    LOG_MSG_ERROR("ADPL state config not set", 0, 0, 0);
    return ADPL_FAILURE;
  }

  LOG_MSG_INFO1("Open ADPL file to receive QMI messages", 0, 0, 0);
  adpl_state_config = adpl_state;

  ret_val = adpl_file_open(adpl_state_config->adpl_iface_device_file,
                             &(adpl_state_config->adpl_iface_fd));

  if(ret_val == ADPL_FAILURE)
  {
    LOG_MSG_ERROR("Failed to open ADPL device file. Error %d",
                   errno, 0, 0);
    return ADPL_FAILURE;
  }
  else
  {
    LOG_MSG_INFO1("Opened ADPL file's fd is %d",
                  adpl_state_config->adpl_iface_fd, 0, 0);

    ret_val = qcmap_nl_addfd_map(fd_set, adpl_state_config->adpl_iface_fd,
                                 read_f, MAX_NUM_OF_FD);
    if(ret_val == QCMAP_UTIL_FAILURE)
    {
      LOG_MSG_ERROR("Failed to map ADPL fd with the read function",
                     0, 0, 0);
      close(adpl_state_config->adpl_iface_fd);
      return ADPL_FAILURE;
    }

/*-------------------------------------------------------------------------
  On boot up since ADPL comes up a bit late when compared to USB driver,
  we may miss the 0 byte data from USB. So in order to handle those cases
  query the ADPL state in peripheral driver by processing it like a reset.
--------------------------------------------------------------------------*/
    /* If target is not listed process ph reset will be ran after DPM client init */
    if(DS_TARGET_FUSION_SDM845_SDX24 == adpl_state_config->target ||
       DS_TARGET_FUSION4_5_PCIE == adpl_state_config->target ||
       DS_TARGET_FUSION_8084_9X45 == adpl_state_config->target ||
       DS_TARGET_FUSION_8096_9X55 == adpl_state_config->target )
    {
      adpl_process_ph_reset();
    }
  }

  return ADPL_SUCCESS;
}

/*===========================================================================

FUNCTION ADPL_PH_RECV_MSG()

DESCRIPTION

  This function
  - receives data from peripheral interface for ADPL logging.

DEPENDENCIES
  None.

RETURN VALUE

SIDE EFFECTS
  None

=========================================================================*/


int adpl_ph_recv_msg
(
   int adpl_ph_fd
)
{
  int        ret;
  int        ret_val;
  char       ph_rx_buf[ADPL_MAX_TRANSFER_SIZE];

 /*-------------------------------------------------------------------------*/

  if (adpl_state_config == NULL || odl_state_config == NULL)
  {
    LOG_MSG_ERROR("ADPL/ODL state config not set", 0, 0, 0);
    return ADPL_FAILURE;
  }

  memset(ph_rx_buf, 0, sizeof(ph_rx_buf));

  ret = read(adpl_ph_fd, ph_rx_buf, ADPL_MAX_TRANSFER_SIZE);

  LOG_MSG_INFO1("Read %d data from peripheral device file", ret, 0, 0);

  if (ret < 0)
  {
    LOG_MSG_ERROR("Failed to read from the dev file. Errno %d",
                   errno, 0, 0);
    return ADPL_FAILURE;
  }
  else if (ret > ADPL_MAX_TRANSFER_SIZE)
  {
    LOG_MSG_ERROR("Received data larger than dest buff %d",
                   errno, 0, 0);
    return ADPL_FAILURE;
  }
/*----------------------------------------------------------------------------
  Peripheral cable plug out/ plug in happened
----------------------------------------------------------------------------*/
  else if (ret == 0)
  {
    /*Ignore the DTR high/low event if odl is up*/
    if (odl_state_config->odl_enabled)
    {
      adpl_state_config->adpl_composition_status = TRUE;
     /*Reset DTR status so that it will be queried for status after ODL is down*/
      adpl_state_config->dtr_enabled = 0;
      LOG_MSG_INFO1("In ODL mode, ignore the DTR high/low event", 0, 0, 0);
    }
    /*it will be handled when Modem in service comes*/
    else if (!is_ssr_in_progress)
    {
      adpl_process_ph_reset();
    }
    else
    {
      LOG_MSG_INFO1("Will be handled later, is_ssr_in_progress:%d odl status:%d",
                   is_ssr_in_progress, odl_state_config->odl_enabled, 0);
    }
  }
  else
  {
    LOG_MSG_INFO1("Ignoring %d data read from ADPL peripheral device file", ret, 0, 0);
  }

  return ADPL_SUCCESS;
}



/*===========================================================================

FUNCTION ADPL_PH_GET_EP_INFO()

DESCRIPTION
  Gets the ph, IPA EP information from ph

DEPENDENCIES
  None.

RETURN VALUE
  ADPL_SUCCESS on success
  ADPL_FAILURE on failure

SIDE EFFECTS
  None

=========================================================================*/

int adpl_ph_get_ep_info()
{

  int ret_val= ADPL_SUCCESS ;
  int retry_count = 0;
  char command[MAX_COMMAND_STR_LEN];

/*------------------------------------------------------------------------*/
  if (adpl_state_config == NULL)
  {
    LOG_MSG_ERROR("ADPL state config not set", 0, 0, 0);
    return ADPL_FAILURE;
  }

/*-------------------------------------------------------------------------
  peripheral(USB) needs to wait till pipes(IPA-USB) get connected to get the required
  IPA pipe info. This can take sometime and if we call IOCTL before peripheral
  gets the IPA pipe info, the IOCTL can fail. So we retry for 5 sec to allow
  peripheral to get the IPA pipe info.
--------------------------------------------------------------------------*/
  while (retry_count < EP_IOCTL_RETRY_COUNT)
  {
    ret_val = ioctl(adpl_state_config->adpl_iface_fd,
                    FRMNET_CTRL_EP_LOOKUP,
                    &(adpl_state_config->ep_info));
    if(ret_val == ADPL_SUCCESS)
      break;

    sleep(ONE_SEC);
    retry_count++;
    LOG_MSG_INFO1("Retrying to get EP info", 0, 0, 0);
  }

  if(retry_count == EP_IOCTL_RETRY_COUNT)
  {
    LOG_MSG_ERROR("Unable to get EP info", 0, 0, 0);
    return ADPL_FAILURE;
  }

  LOG_MSG_INFO1_6("Peripheral type %d, peripheral id %d, ipa consumer EP %d, ipa producer EP %d \n",
                  adpl_state_config->ep_info.ph_ep_info.ep_type,
                  adpl_state_config->ep_info.ph_ep_info.peripheral_iface_id,
                  adpl_state_config->ep_info.ipa_ep_pair.consumer_pipe_num,
                  adpl_state_config->ep_info.ipa_ep_pair.producer_pipe_num, 0, 0);

  return ADPL_SUCCESS;
}
/*===========================================================================

FUNCTION ODL_PH_INIT()

DESCRIPTION

  This function
  - opens the ph device file for ODL logging
  - adds the ph fd to wait on select call

DEPENDENCIES
  None.

RETURN VALUE
  ADPL_SUCCESS on success
  ADPL_FAILURE on failure

SIDE EFFECTS
  None

=========================================================================*/

int odl_ph_init
(
  odl_param           * odl_state,
  qcmap_sk_fd_set_info_t    * fd_set,
  qcmap_sock_thrd_fd_read_f read_f
)
{
  int ret_val;
/*-------------------------------------------------------------------------*/
  if (odl_state == NULL)
  {
    LOG_MSG_ERROR("ODL state config not set", 0, 0, 0);
    return ADPL_FAILURE;
  }

  odl_state_config = odl_state;

  if (!((DS_TARGET_MSMNILE == odl_state_config->target) ||
     (DS_TARGET_LITO == odl_state_config->target) ||
     (DS_TARGET_MSMSTEPPE == odl_state_config->target)))
  {
    LOG_MSG_INFO1("ADPL over ODL not supported in NON MSMNILE/Saipan targets", 0, 0, 0);
    return ADPL_SUCCESS;
  }

  LOG_MSG_INFO1("Open ODL file to receive QMI messages", 0, 0, 0);

  ret_val = adpl_file_open(odl_state_config->odl_iface_device_file,
                             &(odl_state_config->odl_iface_fd));

  if(ret_val == ADPL_FAILURE)
  {
    LOG_MSG_ERROR("Failed to open ODL device file. Error %d",
                   errno, 0, 0);
    return ADPL_FAILURE;
  }
  else
  {
    LOG_MSG_INFO1("Opened ODL file's fd is %d",
                  odl_state_config->odl_iface_fd, 0, 0);

    ret_val = qcmap_nl_addfd_map(fd_set, odl_state_config->odl_iface_fd,
                                 read_f, MAX_NUM_OF_FD);
    if(ret_val == QCMAP_UTIL_FAILURE)
    {
      LOG_MSG_ERROR("Failed to map ODL fd with the read function",
                     0, 0, 0);
      close(odl_state_config->odl_iface_fd);
      return ADPL_FAILURE;
    }
  }
  return ADPL_SUCCESS;
}


/*===========================================================================

FUNCTION ODL_PROCESS_PH_RESET()

DESCRIPTION
  - processes peripheral reset for ODL logging
  - upon 1 in ipa device file : DPM open
  - upon 0 in ipa device file : DPM close.

DEPENDENCIES
  None.

RETURN VALUE
  ADPL_SUCCESS on success
  ADPL_FAILURE on failure


SIDE EFFECTS
  None

=========================================================================*/
int odl_process_ph_reset()
{
  int ret_val = 0;

  if (odl_state_config == NULL || adpl_state_config == NULL)
  {
    LOG_MSG_ERROR("ODL/ADPL state config not set", 0, 0, 0);
    return ADPL_FAILURE;
  }

  LOG_MSG_INFO1("Processing ODL reset for odl_status:%d",
                 odl_state_config->is_odl_ipa_enabled, 0, 0);

  if(odl_state_config->is_odl_ipa_enabled == ODL_UP)
  {
    if (odl_state_config->odl_enabled)
    {
      LOG_MSG_INFO1("Already ODL is up", 0, 0, 0);
    }
    else
    {
      if (adpl_state_config->dtr_enabled)
      {
        ret_val = adpl_dpm_port_close();
        if (ret_val != ADPL_SUCCESS)
        {
          LOG_MSG_ERROR("Failed to close ADPL port through DPM", 0, 0, 0);
          return ADPL_FAILURE;
        }
      }
      ret_val = odl_dpm_port_open();
      if (ret_val != ADPL_SUCCESS)
      {
        odl_write_dpm_status_to_ipa(ODL_DPM_PORT_OPEN_FAILURE);
        LOG_MSG_ERROR("Failed to open ODL port through DPM", 0, 0, 0);
        if (adpl_state_config->dtr_enabled)
        {
          ret_val = adpl_dpm_port_open();
          if (ret_val != ADPL_SUCCESS)
          {
            LOG_MSG_ERROR("Failed to open ADPL port through DPM", 0, 0, 0);
            return ADPL_FAILURE;
          }
        }
        return ADPL_FAILURE;
      }
      odl_write_dpm_status_to_ipa(ODL_DPM_PORT_OPEN_SUCCESS);
      odl_state_config->odl_enabled = TRUE;
    }
    /*Resetting var after reading the value*/
    odl_state_config->is_odl_ipa_enabled = ODL_INVALID_VALUE;
  }
  else if(odl_state_config->is_odl_ipa_enabled == ODL_DOWN)
  {
    if (odl_state_config->odl_enabled)
    {
      ret_val = odl_dpm_port_close();
      if (ret_val != ADPL_SUCCESS)
      {
        LOG_MSG_ERROR("Failed to open ODL port through DPM", 0, 0, 0);
        return ADPL_FAILURE;
      }
      odl_state_config->odl_enabled = FALSE;
      /*Handling events which are ingnored on ODL UP*/
      if (adpl_state_config->adpl_composition_status)
      {
        adpl_process_ph_reset();
        adpl_state_config->adpl_composition_status = FALSE;
      }
      else if (adpl_state_config->dtr_enabled)
      {
        ret_val = adpl_dpm_port_open();
        if (ret_val != ADPL_SUCCESS)
        {
          LOG_MSG_ERROR("Failed to open ADPL port through DPM", 0, 0, 0);
          return ADPL_FAILURE;
        }
      }
      else
      {
        LOG_MSG_INFO1("Not in ADPL composition", 0, 0, 0);
      }
    }
    else
    {
      LOG_MSG_INFO1("Already ODL is down", 0, 0, 0);
    }
    odl_state_config->is_odl_ipa_enabled = ODL_INVALID_VALUE;
  }
  else
  {
    LOG_MSG_ERROR("Invalid odl status : %d", odl_state_config->is_odl_ipa_enabled, 0, 0);
    return ADPL_FAILURE;
  }

  LOG_MSG_INFO1("Completed ODL reset processing", 0, 0, 0);

  return ADPL_SUCCESS;

}

/*===========================================================================

FUNCTION ODL_PH_RECV_MSG()

DESCRIPTION

  This function
  - receives data from ipa dev file.

DEPENDENCIES
  None.

RETURN VALUE
  ADPL_SUCCESS on success
  ADPL_FAILURE on failure


SIDE EFFECTS
  None

=========================================================================*/
int odl_ph_recv_msg
(
   int odl_ph_fd
)
{
  char      ph_rx_buf = '0';
  int      ret = 0;
  int      received_odl_data = ODL_INVALID_VALUE;
/*-------------------------------------------------------------------------*/

  if (odl_state_config == NULL)
  {
    LOG_MSG_ERROR("ODL state config not set", 0, 0, 0);
    return ADPL_FAILURE;
  }

  ret = read(odl_ph_fd, &ph_rx_buf, ODL_TRANSFER_SIZE);

  LOG_MSG_INFO1("Read %d data from ipa device file", ret, 0, 0);
  if (ret < 0)
  {
    LOG_MSG_ERROR("Failed to read from the dev file. Errno %d",
                   errno, 0, 0);
    return ADPL_FAILURE;
  }
  else if (ret > ODL_TRANSFER_SIZE)
  {
    LOG_MSG_ERROR("Received data larger than dest buff %d",
                   errno, 0, 0);
    return ADPL_FAILURE;
  }
/*----------------------------------------------------------------------------
  Odl is up/down
----------------------------------------------------------------------------*/
  else if (ret >= 0 && ret <= ODL_TRANSFER_SIZE)
  {
    received_odl_data = (int)(ph_rx_buf);
    if (received_odl_data == ODL_DOWN || received_odl_data == ODL_UP)
    {
      odl_state_config->is_odl_ipa_enabled = received_odl_data;
    }
    else
    {
      LOG_MSG_INFO1("Received Invalid data from IPA:%d", received_odl_data, 0, 0);
      odl_state_config->is_odl_ipa_enabled = ODL_INVALID_VALUE;
      return ADPL_FAILURE;
    }
    if (!is_ssr_in_progress)
    {
      odl_process_ph_reset();
    }
    else
    {
      LOG_MSG_INFO1("Will be handled later, is_ssr_in_progress:%d odl status:%d is_odl_ipa_enabled %d",
                   is_ssr_in_progress, odl_state_config->odl_enabled, odl_state_config->is_odl_ipa_enabled);
    }
  }
  else
  {
    LOG_MSG_INFO1("Ignoring data read from ODL ipa device file", 0, 0, 0);
  }

  return ADPL_SUCCESS;
}

/*===========================================================================

FUNCTION ODL_GET_EP_INFO()

DESCRIPTION
  Gets the ODL, IPA EP information from ODL dev

DEPENDENCIES
  None.

RETURN VALUE
  ADPL_SUCCESS on success
  ADPL_FAILURE on failure

SIDE EFFECTS
  None

=========================================================================*/

int odl_get_ep_info()
{
#ifdef IPA_IOC_ODL_QUERY_ADAPL_EP_INFO
  int ret_val = ADPL_SUCCESS;
  int retry_count = 0;
  struct ipa_odl_ep_info ipa_ep_info;
/*------------------------------------------------------------------------*/
  if (odl_state_config == NULL)
  {
    LOG_MSG_ERROR("ODL state config not set", 0, 0, 0);
    return ADPL_FAILURE;
  }
  memset(&ipa_ep_info, 0, sizeof(ipa_ep_info));

/*-------------------------------------------------------------------------
  Call IOCTL to get the EP info for ODL endpoint
--------------------------------------------------------------------------*/
  while (retry_count < EP_IOCTL_RETRY_COUNT)
  {
    ret_val = ioctl(odl_state_config->odl_iface_fd,
                    IPA_IOC_ODL_QUERY_ADAPL_EP_INFO,
                    &(ipa_ep_info));
    if(ret_val == ADPL_SUCCESS)
      break;

    sleep(ONE_SEC);
    retry_count++;
    LOG_MSG_INFO1("Retrying to get EP info for ODL endpoint", 0, 0, 0);
  }

  if(retry_count == EP_IOCTL_RETRY_COUNT)
  {
    LOG_MSG_ERROR("Unable to get EP info for ODL endpoint", 0, 0, 0);
    return ADPL_FAILURE;
  }
  odl_state_config->ep_info.ph_ep_info.ep_type = ipa_ep_info.ep_type;
  odl_state_config->ep_info.ph_ep_info.peripheral_iface_id = ipa_ep_info.peripheral_iface_id;
  odl_state_config->ep_info.ipa_ep_pair.consumer_pipe_num = ipa_ep_info.cons_pipe_num;
  odl_state_config->ep_info.ipa_ep_pair.producer_pipe_num = ipa_ep_info.prod_pipe_num;

  LOG_MSG_INFO1_6("Peripheral type %d, peripheral id %d, ipa consumer EP %d, ipa producer EP %d \n",
                  odl_state_config->ep_info.ph_ep_info.ep_type,
                  odl_state_config->ep_info.ph_ep_info.peripheral_iface_id,
                  odl_state_config->ep_info.ipa_ep_pair.consumer_pipe_num,
                  odl_state_config->ep_info.ipa_ep_pair.producer_pipe_num, 0, 0);

  return ADPL_SUCCESS;
#else
  return ADPL_FAILURE;
#endif
}

/*===========================================================================

FUNCTION ODL_WRITE_DPM_STATUS_TO_IPA()

DESCRIPTION
  Write the DPM port open status to IPA for ODL

DEPENDENCIES
  None.

RETURN VALUE
  ADPL_SUCCESS on success
  ADPL_FAILURE on failure

SIDE EFFECTS
  None

=========================================================================*/

int odl_write_dpm_status_to_ipa
(
  boolean status
)
{
  if (odl_state_config == NULL)
  {
    LOG_MSG_ERROR("ODL state config not set", 0, 0, 0);
    return ADPL_FAILURE;
  }

#ifdef IPA_IOC_ODL_QUERY_MODEM_CONFIG
  struct ipa_odl_modem_config dpm_port_status_to_ipa;
  memset(&dpm_port_status_to_ipa, 0, sizeof(dpm_port_status_to_ipa));
  LOG_MSG_INFO1("Sending DPM port status :%s to IPA", (status)?"Success":"Failure", 0, 0);

  dpm_port_status_to_ipa.config_status = status;

  /*-------------------------------------------------------------------------
  Call IOCTL to write the DPM port status to IPA for ODL endpoint
  --------------------------------------------------------------------------*/

  if (ioctl(odl_state_config->odl_iface_fd,
            IPA_IOC_ODL_QUERY_MODEM_CONFIG,
            &dpm_port_status_to_ipa) < 0)
  {
    LOG_MSG_ERROR("IOCTL failed to write dpm port status to IPA:%d", errno, 0, 0);
    return ADPL_FAILURE;
  }

  LOG_MSG_INFO1("Successfully write the DPM port status to IPA", 0, 0, 0);
  return ADPL_SUCCESS;
#else
  return ADPL_FAILURE;
#endif
}

