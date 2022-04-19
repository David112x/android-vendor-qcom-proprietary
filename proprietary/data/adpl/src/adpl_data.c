/******************************************************************************

                        ADPL_DATA.C

******************************************************************************/

/******************************************************************************

  @file    adpl_data.c
  @brief   Accelerated Data Path Logging module for DATA interaction.


  DESCRIPTION
  This file has functions which interact with RmNET DATA for
  setting up ADPL bridge.

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
#include <linux/rmnet_data.h>
#include <linux/msm_rmnet.h>

#include "adpl.h"
#include "librmnetctl.h"


/*===========================================================================
                               FUNCTION DEFINITIONS
=========================================================================*/


/*===========================================================================

FUNCTION ADPL_CALL_IOCTL_ON_DEV()

DESCRIPTION
  - calls the specified IOCTL on the specified interface

DEPENDENCIES
  None.

RETURN VALUE
  ADPL_SUCCESS on success
  ADPL_FAILURE on failure


SIDE EFFECTS
  None

=========================================================================*/
int adpl_call_ioctl_on_dev
(
  const char         *dev,
  unsigned int        req,
  struct ifreq       *ifr
)
{
  int fd;

  /* Open a temporary socket of datagram type to use for issuing the ioctl */
  if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
  {
    LOG_MSG_ERROR("call_ioctl_on_dev: socket open failed", 0, 0, 0);
    return ADPL_FAILURE;
  }

  /* Set device name in the ioctl req struct */
  (void)strlcpy(ifr->ifr_name, dev, sizeof(ifr->ifr_name));

  /* Issue ioctl on the device */
  if (ioctl(fd, req, ifr) < 0)
  {
    LOG_MSG_ERROR("call_ioctl_on_dev: ioctl failed", 0, 0, 0);
    close(fd);
    return ADPL_FAILURE;
  }

  /* Close temporary socket */
  close(fd);
  return ADPL_SUCCESS;
}



/*===========================================================================

FUNCTION ADPL_DATA_INIT_BRIDGE()

DESCRIPTION
  - initializes the RmNet data driver

DEPENDENCIES
  None.

RETURN VALUE
  ADPL_SUCCESS on success
  ADPL_FAILURE on failure


SIDE EFFECTS
  None

=========================================================================*/

int adpl_data_init_bridge(char* modem_iface, char* peripheral_iface)
{
  char          command[MAX_COMMAND_STR_LEN];
  uint16_t      status_code;
  struct        rmnetctl_hndl_s  *rmnet_cfg_lib_handle;
  int           ret_val;
  struct        ifreq ifr;
  int           mhi_ep_id;
  int           mtu_mru_size = DEFAULT_MTU_MRU_VALUE;
  static char   args[ADPL_PROPERTY_VALUE_MAX];
  char          def[PROPERTY_MTU_SIZE+1];
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
  To handle ADPL restart teardown bridge before setting it up
----------------------------------------------------------------------------*/
  adpl_data_teardown_bridge(modem_iface, peripheral_iface);

/*---------------------------------------------------------------------------
  Get the RmNet data driver handle
----------------------------------------------------------------------------*/

  if ((ret_val = rmnetctl_init(&rmnet_cfg_lib_handle, &status_code)) != RMNETCTL_SUCCESS)
  {
    LOG_MSG_ERROR("Failed to initialize RmNet data driver handle."
                  "Return value = %d. Status code =%d",ret_val, status_code, 0);
    return ADPL_FAILURE;
  }

/*---------------------------------------------------------------------------
  Associate MHI interface to RmNet data driver
----------------------------------------------------------------------------*/
  if ((ret_val = rmnet_associate_network_device(rmnet_cfg_lib_handle,
                                                modem_iface,
                                                &status_code,
                                                RMNETCTL_DEVICE_ASSOCIATE))!= RMNETCTL_SUCCESS)
  {
    LOG_MSG_ERROR("Failed to associate MHI interface with RmNet data driver."
                  "Return value = %d. Status code =%d",ret_val, status_code, 0);
    rmnetctl_cleanup(rmnet_cfg_lib_handle);
    return ADPL_FAILURE;
  }

/*---------------------------------------------------------------------------
  Associate USB interface to RmNet data driver
----------------------------------------------------------------------------*/
  if (( ret_val = rmnet_associate_network_device(rmnet_cfg_lib_handle,
                                              peripheral_iface,
                                              &status_code,
                                              RMNETCTL_DEVICE_ASSOCIATE))!= RMNETCTL_SUCCESS)
  {
    LOG_MSG_ERROR("Failed to associate USB interface with RmNet data driver."
                  "Return value = %d. Status code =%d",ret_val, status_code, 0);
    rmnetctl_cleanup(rmnet_cfg_lib_handle);
    return ADPL_FAILURE;
  }

/*---------------------------------------------------------------------------
  Set logical EP point config for MHI and USB interface
----------------------------------------------------------------------------*/
  if (( ret_val = rmnet_set_logical_ep_config(rmnet_cfg_lib_handle,
                                           -1,
                                           RMNET_EPMODE_BRIDGE,
                                           modem_iface,
                                           peripheral_iface,
                                           &status_code))!= RMNETCTL_SUCCESS)
  {
    LOG_MSG_ERROR("Failed to set logical end point config for MHI and USB interface."
                  "Return value = %d. Status code =%d",ret_val, status_code, 0);
    rmnetctl_cleanup(rmnet_cfg_lib_handle);
    return ADPL_FAILURE;
  }

/*---------------------------------------------------------------------------
  Set logical EP point config for USB and MHI interface
----------------------------------------------------------------------------*/
  if (( ret_val = rmnet_set_logical_ep_config(rmnet_cfg_lib_handle,
                                           -1,
                                           RMNET_EPMODE_BRIDGE,
                                           peripheral_iface,
                                           modem_iface,
                                           &status_code))!= RMNETCTL_SUCCESS)
  {
    LOG_MSG_ERROR("Failed to set logical end point config for USB and MHI interface."
                  "Return value = %d. Status code =%d",ret_val, status_code, 0);
    rmnetctl_cleanup(rmnet_cfg_lib_handle);
    return ADPL_FAILURE;
  }

/*---------------------------------------------------------------------------
  Release RmNet data driver handle
----------------------------------------------------------------------------*/
  rmnetctl_cleanup(rmnet_cfg_lib_handle);

/*---------------------------------------------------------------------------
  Set IP mode on USB interface
----------------------------------------------------------------------------*/
  if (adpl_call_ioctl_on_dev(peripheral_iface,
                                  RMNET_IOCTL_SET_LLP_IP,
                                  &ifr) != ADPL_SUCCESS)
  {
    LOG_MSG_ERROR("Failed to set IP mode on USB interface", 0, 0, 0);
    adpl_data_teardown_bridge(modem_iface, peripheral_iface);
    return ADPL_FAILURE;
  }

/*---------------------------------------------------------------------------
  Set IP mode on MHI interface
----------------------------------------------------------------------------*/
  if (adpl_call_ioctl_on_dev(modem_iface,
                                  RMNET_IOCTL_SET_LLP_IP,
                                  &ifr) != ADPL_SUCCESS)
  {
    LOG_MSG_ERROR("Failed to set IP mode on MHI interface", 0, 0, 0);
    adpl_data_teardown_bridge(modem_iface, peripheral_iface);
    return ADPL_FAILURE;
  }

/*---------------------------------------------------------------------------
  bring down both USB and MHI interfaces
----------------------------------------------------------------------------*/
  snprintf(command, MAX_COMMAND_STR_LEN, "ifconfig %s down", modem_iface);
  ds_system_call(command, strlen(command));

  snprintf(command, MAX_COMMAND_STR_LEN, "ifconfig %s down", peripheral_iface);
  ds_system_call(command, strlen(command));

/*---------------------------------------------------------------------------
  If its ADPL usecase, then obtain the aggr info from Android property if set
----------------------------------------------------------------------------*/

    LOG_MSG_INFO1("ADPL interface", 0, 0, 0);

#ifndef FEATURE_MDM_LE
    memset(args, 0, sizeof(args));
    memset( def, 0x0, sizeof(def) );

    snprintf( def, sizeof(def)-1, "%d", ADPL_PROPERTY_AGGR_DEFAULT );
    ret_val = property_get( ADPL_PROPERTY_AGGR, args, def );

    if (ret_val > (PROPERTY_MTU_SIZE))
    {
      LOG_MSG_ERROR("Android property for ADPL has unexpected size(%d)",
                     ret_val, 0, 0);
    }
    else
    {
      mtu_mru_size = ds_atoi(args);
      LOG_MSG_INFO1("MTU MRU size got for ADPL aggr from Android property is %d",
                    mtu_mru_size, 0, 0);

      if( ADPL_MTU_MAX < mtu_mru_size || mtu_mru_size == ADPL_MTU_INVALID)
      {
        LOG_MSG_ERROR("Android property for ADPL has incorrect value (%d). Setting it to default",
                     mtu_mru_size, 0, 0);
        mtu_mru_size = DEFAULT_MTU_MRU_VALUE;
      }
      else
      {
        /* Update MTU value using property */
        LOG_MSG_INFO1( "MRU MTU overide specified for ADPL, using value %d", mtu_mru_size,0,0 );
      }
    }
#endif

  ret_val = adpl_modem_set_mru(mtu_mru_size, modem_iface);
  if (ret_val != ADPL_SUCCESS)
  {
    LOG_MSG_ERROR("Failed to set default MRU value on modem", 0, 0, 0);
    adpl_data_teardown_bridge(modem_iface, peripheral_iface);
    return ADPL_FAILURE;
  }

  ret_val = adpl_ph_set_mtu(mtu_mru_size, peripheral_iface);
  if (ret_val != ADPL_SUCCESS)
  {
    LOG_MSG_ERROR("Failed to set default MTU value on peripheral", 0, 0, 0);
    adpl_data_teardown_bridge(modem_iface, peripheral_iface);
    return ADPL_FAILURE;
  }

/*---------------------------------------------------------------------------
  Bring up MHI and USB interfaces
----------------------------------------------------------------------------*/

  snprintf(command, MAX_COMMAND_STR_LEN, "ifconfig %s up", modem_iface);
  ds_system_call(command, strlen(command));

  snprintf(command, MAX_COMMAND_STR_LEN, "ifconfig %s up", peripheral_iface);
  ds_system_call(command, strlen(command));

  ret_val = adpl_is_iface_up(modem_iface);
  if(ret_val)
  {
    LOG_MSG_INFO1("Modem iface is up", 0, 0, 0);
  }
  else
  {
    LOG_MSG_ERROR("Modem interface is not up", 0, 0, 0);
    adpl_data_teardown_bridge(modem_iface, peripheral_iface);
    return ADPL_FAILURE;
  }

  ret_val = adpl_is_iface_up(peripheral_iface);
  if(ret_val)
  {
    LOG_MSG_INFO1("Peripheral iface is up", 0, 0, 0);
  }
  else
  {
    LOG_MSG_ERROR("Peripheral_iface interface is not up", 0, 0, 0);
    adpl_data_teardown_bridge(modem_iface, peripheral_iface);
    return ADPL_FAILURE;
  }

  ret_val = adpl_modem_get_ep_id((uint32_t*)&mhi_ep_id, MHI_ADPL_DATA_INTERFACE);
    if (ret_val != ADPL_SUCCESS)
    {
      LOG_MSG_ERROR("Failed to get MHI EP ID", 0, 0, 0);
      adpl_data_teardown_bridge(modem_iface, peripheral_iface);
      return ADPL_FAILURE;
    }

    ret_val = adpl_qmi_wda_set_data_format(mtu_mru_size, DATA_EP_TYPE_PCIE, mhi_ep_id);
    if (ret_val != ADPL_SUCCESS)
    {
      LOG_MSG_ERROR("Failed to send ADPL aggr info %d in set data format", mtu_mru_size, 0, 0);
      adpl_data_teardown_bridge(modem_iface, peripheral_iface);
      return ADPL_FAILURE;
    }
  return ADPL_SUCCESS;
}

/*===========================================================================

FUNCTION ADPL_DATA_TEARDOWN_BRIDGE()

DESCRIPTION
  resets RmNet data driver

DEPENDENCIES
  None.

RETURN VALUE
  ADPL_SUCCESS on success
  ADPL_FAILURE on failure


SIDE EFFECTS
  None

=========================================================================*/

int adpl_data_teardown_bridge(char* modem_iface, char* peripheral_iface)
{
  char   command[MAX_COMMAND_STR_LEN];
  uint16_t status_code;
  struct rmnetctl_hndl_s  *rmnet_cfg_lib_handle;
  int    ret_val;
  int    mtu_mru_size;
  int    mhi_ep_id;
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
  Bring down MHI and USB interfaces
----------------------------------------------------------------------------*/
  snprintf(command, MAX_COMMAND_STR_LEN, "ifconfig %s down", modem_iface);
  ds_system_call(command, strlen(command));

  snprintf(command, MAX_COMMAND_STR_LEN, "ifconfig %s down", peripheral_iface);
  ds_system_call(command, strlen(command));

/*---------------------------------------------------------------------------
  Get RmNet data driver handle
----------------------------------------------------------------------------*/
  if ((ret_val = rmnetctl_init(&rmnet_cfg_lib_handle, &status_code)) != RMNETCTL_SUCCESS)
  {
    LOG_MSG_ERROR("Failed to initialize RmNet data driver handle."
                  "Return value = %d. Status code =%d",ret_val, status_code, 0);
    return ADPL_FAILURE;
  }

/*---------------------------------------------------------------------------
  Unset logical ep config for MHI and USB interfaces from RmNet data driver
----------------------------------------------------------------------------*/
  if ((ret_val = rmnet_unset_logical_ep_config(rmnet_cfg_lib_handle,
                                               -1,
                                               modem_iface,
                                               &status_code))!= RMNETCTL_SUCCESS)
  {
    LOG_MSG_ERROR("Failed to unset logical ep config for MHI interface with RmNet data driver."
                  "Return value = %d. Status code =%d",ret_val, status_code, 0);
  }

  if ((ret_val = rmnet_unset_logical_ep_config(rmnet_cfg_lib_handle,
                                               -1,
                                               peripheral_iface,
                                               &status_code))!= RMNETCTL_SUCCESS)
  {
    LOG_MSG_ERROR("Failed to unset logical ep config for USB interface with RmNet data driver."
                  "Return value = %d. Status code =%d",ret_val, status_code, 0);
  }

/*---------------------------------------------------------------------------
  Unassociate MHI and USB interfaces from RmNet data driver
----------------------------------------------------------------------------*/
  if ((ret_val = rmnet_associate_network_device(rmnet_cfg_lib_handle,
                                                modem_iface,
                                                &status_code,
                                                RMNETCTL_DEVICE_UNASSOCIATE))!= RMNETCTL_SUCCESS)
  {
    LOG_MSG_ERROR("Failed to unassociate MHI interface from RmNet data driver."
                  "Return value = %d. Status code =%d",ret_val, status_code, 0);
  }

  if (( ret_val = rmnet_associate_network_device(rmnet_cfg_lib_handle,
                                              peripheral_iface,
                                              &status_code,
                                              RMNETCTL_DEVICE_UNASSOCIATE))!= RMNETCTL_SUCCESS)
  {
    LOG_MSG_ERROR("Failed to unassociate USB interface from RmNet data driver"
                  "Return value = %d. Status code =%d",ret_val, status_code, 0);
  }

/*---------------------------------------------------------------------------
  Release RmNet data driver handle
----------------------------------------------------------------------------*/
  rmnetctl_cleanup(rmnet_cfg_lib_handle);

  ret_val = adpl_modem_get_ep_id((uint32_t*)&mhi_ep_id, MHI_ADPL_DATA_INTERFACE);
    if (ret_val != ADPL_SUCCESS)
    {
      LOG_MSG_ERROR("Failed to get MHI EP ID", 0, 0, 0);
      return ADPL_FAILURE;
    }

    mtu_mru_size = 0;

    ret_val = adpl_qmi_wda_set_data_format(mtu_mru_size, DATA_EP_TYPE_PCIE, mhi_ep_id);
    if (ret_val != ADPL_SUCCESS)
    {
      LOG_MSG_ERROR("Failed to send ADPL aggr info %d in set data format", mtu_mru_size, 0, 0);
    }

  ret_val = adpl_is_iface_up(modem_iface);
  if(!ret_val)
  {
    LOG_MSG_INFO1("Modem iface is down", 0, 0, 0);
  }
  else
  {
    LOG_MSG_ERROR("Modem interface is not down", 0, 0, 0);
  }

  ret_val = adpl_is_iface_up(peripheral_iface);
  if(!ret_val)
  {
    LOG_MSG_INFO1("Peripheral iface is down", 0, 0, 0);
  }
  else
  {
    LOG_MSG_ERROR("Peripheral_iface interface is not down", 0, 0, 0);
  }

  return ADPL_SUCCESS;

}


