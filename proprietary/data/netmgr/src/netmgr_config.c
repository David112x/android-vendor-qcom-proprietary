/*!
  @file
  netmgr_config.c

  @brief
  This file provides implementation of netmgr configration using configdb.
*/

/*===========================================================================
  Copyright (c) 2013-2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

#include <sys/utsname.h>
#include "netmgr_config.h"
#include "netmgr_util.h"
#include "configdb.h"
#include "ds_util.h"

#include "netmgr_cb.h"
#include "netmgr_main_cb.h"

#define NETMGR_CONFIG_CB_MODNAME "CONFIG"

#ifdef FEATURE_DATA_IWLAN
/*===========================================================================
  FUNCTION:  netmgr_process_iwlan_config
===========================================================================*/
/*!
    @brief
    This function processes the iwlan related config items

    @param[in] target: configuration to be used within the XML file.

    @return  0 on error.
    @return -1 on failure.
*/
/*=========================================================================*/
LOCAL
int netmgr_process_iwlan_config
(
  const char* target
)
{
  int result = 0;
  int ret;
  int i,j, rev_min;
  char prop_name[NETMGR_CFGDB_STRVAL_MAX];
  char prop_val_str[NETMGR_CFGDB_STRVAL_MAX];
  int32 prop_val_int;
  int32 rev_control_ports_len = 0;
  int32 rev_data_ports_len = 0;

  rev_min = 0;
  /* iwlan_enable */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".iwlan_enable");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    result = -1;
    goto err;
  }
  switch(prop_val_int)
  {
    case IWLAN_DISABLE:
     netmgr_main_cfg.iwlan_enabled = IWLAN_DISABLE;
     netmgr_log_med("setting netmgr_main_cfg.iwlan_enabled as IWLAN_DISABLE");
     break;

    case IWLAN_ENABLE:
     netmgr_main_cfg.iwlan_enabled = IWLAN_ENABLE;
     netmgr_log_med("setting netmgr_main_cfg.iwlan_enabled as IWLAN_ENABLE");
     break;

    case NSWO_ONLY:
     netmgr_main_cfg.iwlan_enabled = NSWO_ONLY;
     netmgr_log_med("setting netmgr_main_cfg.iwlan_enabled as NSWO_ONLY");
     break;

    default:
     netmgr_main_cfg.iwlan_enabled = IWLAN_DISABLE;
     netmgr_log_err("Error reading property [%s] property value[%d] ", prop_name, prop_val_int);
  }

  /* rev_control_ports_len and rev_control_ports */
  j = 0;
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".rev_control_ports_len");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &rev_control_ports_len, sizeof(rev_control_ports_len));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    result = -1;
    goto err;
  }

  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".rev_data_ports_len");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &rev_data_ports_len, sizeof(rev_data_ports_len));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    result = -1;
    goto err;
  }
  netmgr_main_cfg.num_rev_data_ports = rev_data_ports_len;

  /* The reverse ports starting and ending index depends on the
   * modem being used. Query the correct starting and ending indexes
   */
  if (netmgr_main_cfg.modem_enable[0])
  {
    rev_min = netmgr_main_cfg.rev_link_start;
  }
  else if(netmgr_main_cfg.modem_enable[1])
  {
    rev_min = netmgr_main_cfg.rev_link_start + netmgr_main_cfg.num_rev_data_ports;
  }

  /* static_rev_links */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".static_rev_links");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    prop_val_int = rev_control_ports_len;
    netmgr_log_err("Error reading property [%s] Err[%d]; Setting to rev control ports len", prop_name, ret);
  }
  netmgr_main_cfg.static_rev_links = (int)prop_val_int;

  for (i = rev_min; i < (rev_min + rev_control_ports_len); i++)
  {
    snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s%d", "netmgr_config.", target, ".rev_control_ports.",j);
    memset(prop_val_str, 0, sizeof(prop_val_str));
    ret = configdb_get_parameter(prop_name, CFGDB_TYPE_STRING, &prop_val_str, sizeof(prop_val_str));
    if (ret != CFGDB_SUCCESS)
    {
      netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
      result = -1;
      goto err;
    }
    strlcpy(netmgr_ctl_port_array[i].qmi_conn_id, prop_val_str, sizeof(netmgr_ctl_port_array[i].qmi_conn_id));
    if ((i - rev_min) < netmgr_main_cfg.static_rev_links)
    {
      netmgr_ctl_port_array[i].enabled = TRUE;
    }
    j++;
  }

  /* rev_data_ports */
  j = 0;
  for (i = rev_min; i < (rev_min + rev_data_ports_len); i++)
  {
    snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s%d", "netmgr_config.", target, ".rev_data_ports.",j);
    memset(prop_val_str, 0, sizeof(prop_val_str));
    ret = configdb_get_parameter(prop_name, CFGDB_TYPE_STRING, &prop_val_str, sizeof(prop_val_str));
    if (ret != CFGDB_SUCCESS)
    {
      netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
      result = -1;
      goto err;
    }
    strlcpy(netmgr_ctl_port_array[i].data_ctl_port, prop_val_str, sizeof(netmgr_ctl_port_array[i].data_ctl_port));
    if ((i - rev_min) < netmgr_main_cfg.static_rev_links)
    {
      netmgr_ctl_port_array[i].enabled = TRUE;
    }
    j++;
  }

  /* Remove prefix table as an enhancement later */
  if (netmgr_main_cfg.rmnet_data_enabled)
  {
    if (netmgr_main_cfg.modem_enable[0])
    {
      /* Change prefix for reverse links */
      netmgr_log_high("%s", "Changing MSM rev prefixes");
      strlcpy(NETMGR_MAIN_GET_DEV_REV_PREFIX(NETMGR_MODEM_MSM),
              NETMGR_MAIN_REV_RMNET_DATA_PREFIX,
              NETMGR_IF_NAME_MAX_LEN);
    }
    else if(netmgr_main_cfg.modem_enable[1])
    {
      /* Change prefix for reverse links */
      netmgr_log_high("%s", "Changing MDM rev prefixes");
      strlcpy(NETMGR_MAIN_GET_DEV_REV_PREFIX(NETMGR_MODEM_MDM),
              NETMGR_MAIN_REV_RMNET_DATA_PREFIX,
              NETMGR_IF_NAME_MAX_LEN);
    }
  }

err:
  return result;
}

#endif /* FEATURE_DATA_IWLAN */


/*===========================================================================
  FUNCTION:  netmgr_main_cfg_update_ike_port
===========================================================================*/
/*!
@brief
  Helper function to update the current IKE port being used

  @param[in] port:  host order port.

  @return  port on success.
  @return  NETMGR_MAIN_IKE_PORT_INVALID on failure.
*

*/
/*=========================================================================*/
int netmgr_main_cfg_update_ike_port(int port, char *local_addr, char *dest_addr)
{
  int i = 0;
  int ret = NETMGR_MAIN_IKE_PORT_INVALID;
  size_t bytes = 0;

  if (!local_addr || !dest_addr)
  {
    return NETMGR_MAIN_IKE_PORT_INVALID;
  }

  pthread_mutex_lock(&netmgr_main_cfg_lock);
  for (i = 0; i < MAX_IKE_PORTS; i++)
  {
    /* Update port with addresses in rev_ind*/
    if (netmgr_main_cfg.ike_ports[i].ike_port == port)
    {
      /*If local and dest addr is already the same don't update*/
      if (!strcmp(netmgr_main_cfg.ike_ports[i].local_addr, local_addr)
          && !strcmp(netmgr_main_cfg.ike_ports[i].dest_addr, dest_addr))
      {
        break;
      }
      bytes = strlcpy(netmgr_main_cfg.ike_ports[i].local_addr, local_addr, NETMGR_MAX_STR_LENGTH);
      if (bytes >= NETMGR_MAX_STR_LENGTH)
      {
        netmgr_log_err("%s(): failed to copy local address to table!", __func__);
        memset(netmgr_main_cfg.ike_ports[i].local_addr, 0, NETMGR_MAX_STR_LENGTH);
        goto bail;
      }

      bytes = strlcpy(netmgr_main_cfg.ike_ports[i].dest_addr, dest_addr, NETMGR_MAX_STR_LENGTH);
      if (bytes >= NETMGR_MAX_STR_LENGTH)
      {
        netmgr_log_err("%s(): failed to copy dest address to table!", __func__);
        memset(netmgr_main_cfg.ike_ports[i].local_addr, 0, NETMGR_MAX_STR_LENGTH);
        memset(netmgr_main_cfg.ike_ports[i].dest_addr, 0, NETMGR_MAX_STR_LENGTH);
        goto bail;
      }
      netmgr_log_med("%s(): update for port %d found at index %d", __func__, port, i);
      break;
    }
  }
  /* Port missing from cfg table*/
  if (i == MAX_IKE_PORTS)
  {
    netmgr_log_err("%s(): port missing from table in update", __func__);
    goto bail;
  }

  ret = port;

bail:
  pthread_mutex_unlock(&netmgr_main_cfg_lock);
  return ret;
}

/*===========================================================================
  FUNCTION:  netmgr_main_cfg_get_ike_port
===========================================================================*/
/*!
@brief
  Helper function to query the current IKE port being used
*/
/*=========================================================================*/
int netmgr_main_cfg_get_ike_port(char *local_addr, char *dest_addr)
{
  int port = NETMGR_MAIN_IKE_PORT_INVALID;
  int i = 0;
  size_t bytes = 0;

  pthread_mutex_lock(&netmgr_main_cfg_lock);

  if (!local_addr || !dest_addr)
  {
    netmgr_log_err("%s(): invalid params!", __func__);
    goto bail;
  }

  for (i = 0; i < MAX_IKE_PORTS; i++)
  {
    /* Search the array for matching local_addr and dest_addr entries */
    if (!strcmp(netmgr_main_cfg.ike_ports[i].local_addr, local_addr)
          && !strcmp(netmgr_main_cfg.ike_ports[i].dest_addr, dest_addr))
    {
      break;
    }
  }

  if (MAX_IKE_PORTS == i)
  {
    /* If there is no matching entry, update one of the existing entries for which
       local_addr and dest_addr are empty and a valid IKE port is set */
    for (i = 0; i < MAX_IKE_PORTS; i++)
    {
      if (NETMGR_MAIN_IKE_PORT_INVALID != netmgr_main_cfg.ike_ports[i].ike_port
            && strlen(netmgr_main_cfg.ike_ports[i].local_addr) == 0
            && strlen(netmgr_main_cfg.ike_ports[i].dest_addr) == 0)
      {
        break;
      }
    }

    if (MAX_IKE_PORTS == i) {
      /* check if we have a static port, whose addresses have not been updated.
       * This can happen if the modem has legacy port support and netmgrd has
       * dynamic port support (Android O and later code).*/
      for (i = 0; i < MAX_IKE_PORTS; i++)
      {
        if (netmgr_main_cfg.ike_ports[i].is_legacy == TRUE) {
          /* break, addresses will be updated below, return legacy port*/
          netmgr_log_err("%s(): found a legacy port %d", __func__,
                         netmgr_main_cfg.ike_ports[i].ike_port);
          break;
        }

      }
    }

    if (MAX_IKE_PORTS == i)
    {
      /* If we are still unable to locate an entry return failure */
      netmgr_log_err("%s(): no valid entry available to save IKE port information!", __func__);
      goto bail;
    }

    bytes = strlcpy(netmgr_main_cfg.ike_ports[i].local_addr, local_addr, NETMGR_MAX_STR_LENGTH);
    if (bytes >= NETMGR_MAX_STR_LENGTH)
    {
      netmgr_log_err("%s(): failed to copy local address to table!", __func__);
      memset(netmgr_main_cfg.ike_ports[i].local_addr, 0, NETMGR_MAX_STR_LENGTH);
      goto bail;
    }

    bytes = strlcpy(netmgr_main_cfg.ike_ports[i].dest_addr, dest_addr, NETMGR_MAX_STR_LENGTH);
    if (bytes >= NETMGR_MAX_STR_LENGTH)
    {
      netmgr_log_err("%s(): failed to copy dest address to table!", __func__);
      memset(netmgr_main_cfg.ike_ports[i].local_addr, 0, NETMGR_MAX_STR_LENGTH);
      memset(netmgr_main_cfg.ike_ports[i].dest_addr, 0, NETMGR_MAX_STR_LENGTH);
      goto bail;
    }
  }

  port = netmgr_main_cfg.ike_ports[i].ike_port;

bail:
  pthread_mutex_unlock(&netmgr_main_cfg_lock);
  return port;
}

/*===========================================================================
  FUNCTION:  netmgr_main_cfg_set_ike_port
===========================================================================*/
/*!
@brief
  Sets the IKE port which will be used for iWLAN call
*/
/*=========================================================================*/
void netmgr_main_cfg_set_ike_port(int port, int is_legacy)
{
  int i = 0;

  netmgr_log_err("%s():  set port %d, is_legacy %d", __func__, port, is_legacy);
  pthread_mutex_lock(&netmgr_main_cfg_lock);

  /* 1. Save the port in an empty slot in the ike_ports array
     2. Set the local and dest addr fields to empty string
     3. When someone calls get_ike_port we will search for a matching address entry
        If found, return the corresponding port
        If not found, update the entry with empty local and dest address fields and return
        the corresponding port */
  for (i = 0; i < MAX_IKE_PORTS; i++)
  {
    if (NETMGR_MAIN_IKE_PORT_INVALID == netmgr_main_cfg.ike_ports[i].ike_port)
    {
      /* Add port info */
      netmgr_log_med("%s(): setting IKE port to %d", __func__, port);
      netmgr_main_cfg.ike_ports[i].ike_port = port;
      netmgr_main_cfg.ike_ports[i].is_legacy = is_legacy;

      /* Zero out the address info */
      memset(netmgr_main_cfg.ike_ports[i].local_addr, 0, NETMGR_MAX_STR_LENGTH);
      memset(netmgr_main_cfg.ike_ports[i].dest_addr, 0, NETMGR_MAX_STR_LENGTH);

      break;
    }
  }

  if (MAX_IKE_PORTS == i)
  {
    netmgr_log_err("%s(): maximum IKE ports reached. Could not save information for port [%d]",
                   __func__, port);
  }

  pthread_mutex_unlock(&netmgr_main_cfg_lock);
}

/*===========================================================================
  FUNCTION:  netmgr_main_cfg_associate_local_addr_with_ike_ports
===========================================================================*/
/*!
@brief
  Update the local address for all valid IKE ports
*/
/*=========================================================================*/
void netmgr_main_cfg_associate_local_addr_with_ike_ports(char *local_addr)
{
  int i = 0;
  size_t bytes = 0;

  pthread_mutex_lock(&netmgr_main_cfg_lock);

  if (!local_addr)
  {
    netmgr_log_err("%s(): invalid params!", __func__);
    pthread_mutex_unlock(&netmgr_main_cfg_lock);
    return;
  }

  for (i = 0; i < MAX_IKE_PORTS; i++)
  {
    if (NETMGR_MAIN_IKE_PORT_INVALID != netmgr_main_cfg.ike_ports[i].ike_port
          && strlen(netmgr_main_cfg.ike_ports[i].dest_addr) != 0)
    {
      /* Update the local addresses for all valid entries */
      bytes = strlcpy(netmgr_main_cfg.ike_ports[i].local_addr, local_addr,
                      NETMGR_MAX_STR_LENGTH);
      if (bytes >= NETMGR_MAX_STR_LENGTH)
      {
        netmgr_log_err("%s(): failed to copy local address to table!", __func__);
        memset(netmgr_main_cfg.ike_ports[i].local_addr, 0, NETMGR_MAX_STR_LENGTH);
      }
    }
  }

  pthread_mutex_unlock(&netmgr_main_cfg_lock);
}

/*===========================================================================
  FUNCTION:  netmgr_main_cfg_cleanup_local_addr_for_ike_port
===========================================================================*/
/*!
@brief
  Cleanup the local address associated with all valid IKE ports
*/
/*=========================================================================*/
void netmgr_main_cfg_cleanup_local_addr_for_ike_ports(char *local_addr)
{
  int i = 0;

  pthread_mutex_lock(&netmgr_main_cfg_lock);

  if (!local_addr)
  {
    netmgr_log_err("%s(): invalid params!", __func__);
    pthread_mutex_unlock(&netmgr_main_cfg_lock);
    return;
  }

  for (i = 0; i < MAX_IKE_PORTS; i++)
  {
    if (NETMGR_MAIN_IKE_PORT_INVALID != netmgr_main_cfg.ike_ports[i].ike_port
          && strlen(netmgr_main_cfg.ike_ports[i].dest_addr) != 0
          && !strcmp(local_addr, netmgr_main_cfg.ike_ports[i].local_addr))
    {
      /* If the local address changes for some reason we need to ensure that
         the local_addr info for all the matching table entries is cleaned up.
         When the new local_addr info is available, the table entries will be
         repopulated */
      memset(netmgr_main_cfg.ike_ports[i].local_addr, 0, NETMGR_MAX_STR_LENGTH);
    }
  }

  pthread_mutex_unlock(&netmgr_main_cfg_lock);
}

/*===========================================================================
  FUNCTION:  netmgr_main_cfg_cleanup_ike_port
===========================================================================*/
/*!
@brief
  Cleanup entry corresponding to the IKE port
*/
/*=========================================================================*/
void netmgr_main_cfg_cleanup_ike_port(int port)
{
  int i = 0;

  pthread_mutex_lock(&netmgr_main_cfg_lock);

  for (i = 0; i < MAX_IKE_PORTS; i++)
  {
    if (netmgr_main_cfg.ike_ports[i].ike_port == port)
    {
      /* Zero out the address info */
      memset(&netmgr_main_cfg.ike_ports[i], 0, sizeof(netmgr_ike_port_info_t));
      netmgr_main_cfg.ike_ports[i].ike_port = NETMGR_MAIN_IKE_PORT_INVALID;
      break;
    }
  }

  if (MAX_IKE_PORTS == i)
  {
    netmgr_log_err("%s(): no information found for port [%d]", __func__, port);
  }

  pthread_mutex_unlock(&netmgr_main_cfg_lock);
}

/*===========================================================================
  FUNCTION:  netmgr_process_cmd_logging_config
===========================================================================*/
/*!
    @brief
    This function processes the ds_exec option overrides

    @return  nothing
*/
/*=========================================================================*/
LOCAL
void netmgr_process_cmd_logging_config
(
  void
)
{
  int ret;
  char *prop_name;
  char prop_val_str[NETMGR_CFGDB_STRVAL_MAX];
  int32 prop_val_int;

  prop_name = "netmgr_config.cmd_logging_config.cmd_debug";
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_med("Couldn't read property [%s] Err[%d], defaulting to 0", prop_name, ret);
    prop_val_int = 0;
  }
  netmgr_main_cfg.exec_opts.cmd_debug = prop_val_int;

  prop_name = "netmgr_config.cmd_logging_config.log_options";
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_med("Couldn't read property [%s] Err[%d], defaulting to 0", prop_name, ret);
    prop_val_int = 0;
  }
  netmgr_main_cfg.exec_opts.log_options = prop_val_int;

  prop_name = "netmgr_config.cmd_logging_config.outfile";
  memset(prop_val_str, 0, sizeof(prop_val_str));
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_STRING, &prop_val_str, sizeof(prop_val_str));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    memset(prop_val_str, 0, sizeof(prop_val_str));
  }
  strlcpy(netmgr_main_cfg.exec_opts.outfile, prop_val_str, sizeof(netmgr_main_cfg.exec_opts.outfile));
}

/*===========================================================================
  FUNCTION:  netmgr_config_load
===========================================================================*/
/*!
    @brief
    This function loads the configuration based on the config file and
    "target" property specified.

    @param[in] xml_file: XML file location string.
    @param[in] target: configuration to be used within the XML file.

    @return  0 if configuration file is loaded properly.
    @return -1 if configuration file file load/parase fails.
*/
/*=========================================================================*/
int netmgr_config_load
(
  const char *xml_file,
  const char *target
)
{
  int result = 0;
  int32 ret;
  int i;
  char prop_name[NETMGR_CFGDB_STRVAL_MAX];
  int32 num_modems = 0;
  int32 control_ports_len = 0;
  int32 data_ports_len = 0;
  int32 prop_val_int;
  char prop_val_str[NETMGR_CFGDB_STRVAL_MAX];

  netmgr_log_med("netmgr_config: Configuring using file %s, target %s\n", xml_file, target);
  ret = configdb_init(CFGDB_OPMODE_CACHED, xml_file);
  if (CFGDB_SUCCESS != ret)
  {
    netmgr_log_err("Unable to open/parse config file [%s] Err [%d]", xml_file, ret);
    result = -1;
    goto bail;
  }

  /* qmi_dpm_enabled */
#ifdef FEATURE_DS_LINUX_ANDROID
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".qmi_dpm_enabled");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    result = -1;
    goto bail;
  }
  netmgr_main_cfg.dpm_enabled = (prop_val_int ? TRUE : FALSE);
#elif defined(FEATURE_DATA_MDM_SINGLE_QMUX_CHANNEL)
  netmgr_log_med("MDM platform, single QMUX channel enabled, enabling DPM\n");
  netmgr_main_cfg.dpm_enabled = TRUE;
#endif


  /* qos_via_idl */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".qos_via_idl");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    netmgr_main_cfg.qos_via_idl = FALSE;
  }
  else
  {
    netmgr_main_cfg.qos_via_idl = (prop_val_int ? TRUE : FALSE);
  }

  /* use_qmuxd */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".use_qmuxd");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d] Default[%d]", prop_name, ret, TRUE);
    prop_val_int = TRUE;
  }
  netmgr_main_cfg.use_qmuxd = (prop_val_int ? TRUE : FALSE);

  /* dpm_retry_timeout value */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".dpm_retry_timeout");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    prop_val_int = 10000;
    netmgr_log_err("Error reading property [%s] Err[%d] Default [%d]",
                   prop_name, ret,prop_val_int);
  }
  netmgr_main_cfg.dpm_retry_timeout = prop_val_int;

  /* wda_data_format_enabled */
#ifdef FEATURE_DS_LINUX_ANDROID
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".wda_data_format_enabled");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    result = -1;
    goto bail;
  }
  netmgr_main_cfg.wda_data_format = (prop_val_int ? TRUE : FALSE);
#elif defined(FEATURE_DATA_MDM_SINGLE_QMUX_CHANNEL)
  netmgr_log_med("MDM platform, single QMUX channel enabled, enabling WDA data format\n");
  netmgr_main_cfg.wda_data_format = TRUE;
#elif defined(NETMGR_OFFTARGET)
  netmgr_log_med("For offtarget setting wda format enabled to true");
  netmgr_main_cfg.wda_data_format = TRUE;
#endif

  /* ep_type */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".ep_type");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]. Default[%d]",
                   prop_name, ret, DS_EP_TYPE_EMBEDDED);
    prop_val_int = DS_EP_TYPE_EMBEDDED;
  }
  netmgr_main_cfg.ep_type = prop_val_int;

  /* single_qmux_ch_enabled */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".single_qmux_ch_enabled");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    result = -1;
    goto bail;
  }
  netmgr_main_cfg.single_qmux_ch_enabled = (prop_val_int ? TRUE : FALSE);

  /* single_qmux_ch_conn_id_str */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".single_qmux_ch_conn_id_str");
  memset(prop_val_str, 0, sizeof(prop_val_str));
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_STRING, prop_val_str, sizeof(prop_val_str));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    result = -1;
    goto bail;
  }
  strlcpy(netmgr_main_cfg.qmux_ch_name, prop_val_str, sizeof(netmgr_main_cfg.qmux_ch_name));

  /* single_qmux_ch_name */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".single_qmux_ch_name");
  memset(prop_val_str, 0, sizeof(prop_val_str));
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_STRING, prop_val_str, sizeof(prop_val_str));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    result = -1;
    goto bail;
  }
  strlcpy(netmgr_main_cfg.smd_ch_name, prop_val_str, sizeof(netmgr_main_cfg.smd_ch_name));

  /* rmnet_data_enabled */
#ifdef FEATURE_DS_LINUX_ANDROID
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".rmnet_data_enabled");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    result = -1;
    goto bail;
  }
  netmgr_main_cfg.rmnet_data_enabled = (prop_val_int ? TRUE : FALSE);
#elif defined(FEATURE_DATA_MDM_SINGLE_QMUX_CHANNEL)
  netmgr_log_med("MDM platform, single QMUX channel, enable RMNET data\n");
  netmgr_main_cfg.rmnet_data_enabled = TRUE;
#elif defined(NETMGR_OFFTARGET)
  netmgr_log_med("For offtarget setting rmnet data as true");
  netmgr_main_cfg.rmnet_data_enabled = TRUE;
#endif

  /* dataformat_agg_dl_pkt */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".dataformat_agg_dl_pkt");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    result = -1;
    goto bail;
  }
  netmgr_main_cfg.data_format.dl_agg_cnt = (int)prop_val_int;

  /* dataformat_agg_dl_size */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".dataformat_agg_dl_size");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    result = -1;
    goto bail;
  }
  netmgr_main_cfg.data_format.dl_agg_size = (int)prop_val_int;

  /* dataformat_agg_ul_pkt */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".dataformat_agg_ul_pkt");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    prop_val_int = 0;
    netmgr_log_med("Warning: Error reading property [%s] Err[%d]; setting to 0", prop_name, ret);
  }
  netmgr_main_cfg.data_format.ul_agg_cnt = prop_val_int;

  /* dataformat_agg_ul_size */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".dataformat_agg_ul_size");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    prop_val_int = 0;
    netmgr_log_med("Warning: reading property [%s] Err[%d]; setting to 0", prop_name, ret);
  }
  netmgr_main_cfg.data_format.ul_agg_size = prop_val_int;

  /* dataformat_agg_ul_time */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".dataformat_agg_ul_time");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    prop_val_int = 3000000;
    netmgr_log_med("Warning: reading property [%s] Err[%d]; setting to 3000000", prop_name, ret);
  }
  netmgr_main_cfg.data_format.ul_agg_time = prop_val_int;

  /* dataformat_agg_ul_features */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".dataformat_agg_ul_features");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    prop_val_int = 0;
    netmgr_log_med("Warning: reading property [%s] Err[%d]; setting to 0", prop_name, ret);
  }
  netmgr_main_cfg.data_format.ul_agg_features = prop_val_int;

  /* dataformat_num_iwlan_channels*/
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".dataformat_num_iwlan_channels");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    prop_val_int = 9;
    netmgr_log_med("Warning: reading property [%s] Err[%d]; setting to 0", prop_name, ret);
  }
  netmgr_main_cfg.data_format.num_iwlan_channels = prop_val_int;

  /* dataformat_num_mux_channels */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".dataformat_num_mux_channels");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    prop_val_int = 8;
    netmgr_log_med("Warning: reading property [%s] Err[%d]; setting to 0", prop_name, ret);
  }
  netmgr_main_cfg.data_format.num_mux_channels= prop_val_int;

  /* dataformat_agg_dl_pad */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".dataformat_agg_dl_pad");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    prop_val_int = 0;
    netmgr_log_med("Warning: Error reading property [%s] Err[%d]; setting to 0", prop_name, ret);
  }
  netmgr_main_cfg.data_format.dl_agg_pad = prop_val_int;

  /* dataformat_dl_data_aggregation_protocol */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".dataformat_dl_data_aggregation_protocol");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    result = -1;
    goto bail;
  }
  netmgr_main_cfg.data_format.dl_data_aggregation_protocol = (int)prop_val_int;

  /* dataformat_ul_data_aggregation_protocol */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".dataformat_ul_data_aggregation_protocol");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    result = -1;
    goto bail;
  }
  netmgr_main_cfg.data_format.ul_data_aggregation_protocol = (int)prop_val_int;

  /* dataformat_dl_gro_enabled */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".dataformat_dl_gro_enabled");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    prop_val_int = 0;
  }
  netmgr_main_cfg.data_format.dl_gro_enabled = (int)prop_val_int;

  /* Max configurable MTU */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".max_mtu");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    prop_val_int = 0;
    netmgr_log_med("Error reading property [%s] Err[%d]; Setting to 0", prop_name, ret);
  }
  netmgr_main_cfg.max_mtu = (int)prop_val_int;

  /* rmnet_perf */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.",
           target, ".rmnet_perf");

  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int,
                               sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    prop_val_int = 0;
  }

  netmgr_main_cfg.rmnet_perf = (int)prop_val_int;

  /* rmnet shs*/
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.",
           target, ".rmnet_shs");

  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int,
                               sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    prop_val_int = 0;
  }

  netmgr_main_cfg.rmnet_shs = (int)prop_val_int;

  /* Disable tx csum offload for xlat */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.",
           target, ".xlat_txcsum_disabled");

  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int,
                               sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    prop_val_int = 0;
  }

  netmgr_main_cfg.xlat_txcsum_disabled = (boolean)prop_val_int;

  /* new_rmnet_data_enabled */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.",
           target, ".rtm_rmnet_data_enabled");

  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int,
                               sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    prop_val_int = 0;
  }

  netmgr_main_cfg.rtm_rmnet_enabled = (int)prop_val_int;

  /* new_tc_enabled */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.",
           target, ".rtnetlink_tc_enabled");

  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int,
                               sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    prop_val_int = 0;
  }

  netmgr_main_cfg.rtnetlink_tc_enabled = (int)prop_val_int;

  /* parse dl marker packets */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.",
           target, ".dl_marker_enabled");

  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int,
                               sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    prop_val_int = 0;
  }
  netmgr_main_cfg.dl_marker = (int)prop_val_int;

  /* dataformat_ul_gso_enabled */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".dataformat_ul_gso_enabled");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    result = -1;
    goto bail;
  }
  netmgr_main_cfg.data_format.ul_gso_enabled = (int)prop_val_int;

  /* phys_net_dev */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".phys_net_dev");
  memset(prop_val_str, 0, sizeof(prop_val_str));
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_STRING, prop_val_str, sizeof(prop_val_str));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    result = -1;
    goto bail;
  }

  strlcpy(netmgr_main_cfg.phys_net_dev, prop_val_str, sizeof(netmgr_main_cfg.phys_net_dev));

  /* tcp_mtu_probing */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".tcp_mtu_probing");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    prop_val_int = 0;
    netmgr_log_err("Error reading property [%s] Err[%d]; Setting to 0", prop_name, ret);
  }
  netmgr_main_cfg.tcp_mtu_probing = (int)prop_val_int;

  /* pnd_rps_mask */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".pnd_rps_mask");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    prop_val_int = 0;
    netmgr_log_err("Error reading property [%s] Err[%d]; Setting to 0", prop_name, ret);
  }
  netmgr_main_cfg.pnd_rps_mask = (int)prop_val_int;

  /* vnd_rps_mask */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".vnd_rps_mask");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    prop_val_int = 0;
    netmgr_log_err("Error reading property [%s] Err[%d]; Setting to 0", prop_name, ret);
  }
  netmgr_main_cfg.vnd_rps_mask = (int)prop_val_int;

  /* clat_rps_mask */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".clat_rps_mask");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    prop_val_int = 0;
    netmgr_log_err("Error reading property [%s] Err[%d]; Setting to 0", prop_name, ret);
  }
  netmgr_main_cfg.clat_rps_mask = (int)prop_val_int;
  /* overwrite rps mask config, This is for targets whose core numbers vary
   * from previous variants
   */
  netmgr_config_overwrite();

  /* netdev_budget */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".netdev_budget");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    prop_val_int = 0;
    netmgr_log_err("Error reading property [%s] Err[%d]; Setting to 0", prop_name, ret);
  }
  netmgr_main_cfg.netdev_budget = (int)prop_val_int;

#ifdef NETMGR_OFFTARGET
  netmgr_main_cfg.low_latency_filters = FALSE;
#else
  /* low_latency_filters */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".low_latency_filters");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    result = -1;
    goto bail;
  }
  netmgr_main_cfg.low_latency_filters = (prop_val_int ? TRUE : FALSE);
#endif

  /* ibfc */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".ibfc");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    prop_val_int = 0;
    netmgr_log_med("Error reading property [%s] Err[%d]; Setting to 0", prop_name, ret);
  }
  netmgr_main_cfg.ibfc = (int)prop_val_int;

  /* dropssdp */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".dropssdp");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    prop_val_int = 1;
    netmgr_log_med("Error reading property [%s] Err[%d]; Setting to 0", prop_name, ret);
  }
  netmgr_main_cfg.dropssdp = (int)prop_val_int;

  /* MTU */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".mtu");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    prop_val_int = 0;
    netmgr_log_med("Error reading property [%s] Err[%d]; Setting to 0", prop_name, ret);
  }
  netmgr_main_cfg.mtu = (int)prop_val_int;

  /* wakelock_timer */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".wakelock_timeout");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    prop_val_int = 2; //default = 2
    netmgr_log_med("Error reading property [%s] Err[%d]; Setting to 0", prop_name, ret);
  }
  netmgr_main_cfg.wakelock_timeout = (int)prop_val_int;

  /* iwlan_rekey */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".iwlan_rekey");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    prop_val_int = 0;
    netmgr_log_med("Error reading property [%s] Err[%d]; Setting to 0", prop_name, ret);
  }
  netmgr_main_cfg.iwlan_rekey = (int)prop_val_int;

  /* qos_disabled; overrides the android property */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".qos_disabled");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
#ifdef NETMGR_QOS_ENABLED
    prop_val_int = 0;
#else
    prop_val_int = 1;
#endif
    netmgr_log_med("Error reading property [%s] Err[%d]; Setting to 0", prop_name, ret);
  }
  netmgr_main_cfg.qos_disabled = (int)prop_val_int;
  if (netmgr_main_cfg.qos_disabled)
  {
    /* Clear QOS enabled flag */
    netmgr_main_cfg.runmode &= ~NETMGR_MAIN_RUNMODE_QOSHDR;
    netmgr_log_med("Disabling QOS");
  }
  else
  {
    /* By default, we need to enable QOS if the property is not set
     * to false explicitly */
    netmgr_main_cfg.runmode |= NETMGR_MAIN_RUNMODE_QOSHDR;
    netmgr_log_med("Setting QOS enabled");
  }

  /* kernel flow control */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".kfc_mode");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret == CFGDB_SUCCESS)
  {
    if (prop_val_int < 0 ||
        netmgr_main_cfg.qos_disabled ||
        !netmgr_main_cfg.rtm_rmnet_enabled)
    {
      netmgr_main_cfg.kfc_mode = 0;
    }
    else
    {
      netmgr_main_cfg.kfc_mode = prop_val_int;
    }
  }

  /* Kernel QMAP flow control */
  if (netmgr_main_cfg.kfc_mode)
  {
    snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".kfc_qmap");
    ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
    if (ret != CFGDB_SUCCESS)
    {
      prop_val_int = 0;
    }
    netmgr_main_cfg.kfc_qmap = (boolean)prop_val_int;
  }

  /* Power collapse */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".qmi_pc");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS || !netmgr_main_cfg.rtm_rmnet_enabled)
  {
    prop_val_int = 0;
  }
  netmgr_main_cfg.qmi_pc = (boolean)prop_val_int;

  /* tc_ul_baserate */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".tc_ul_baserate");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_med("Error reading property [%s] Err[%d]; setting to 0", prop_name, ret);
    netmgr_main_cfg.tc_ul_baserate = 0;
  }
  else
  {
    /* TODO: configdb does not have a UL type; we are limited to ~2Gbps */
    netmgr_main_cfg.tc_ul_baserate = (uint64_t)prop_val_int;
  }

  /* tc_ul_ceiling_mbps */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".tc_ul_ceiling_mbps");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret == CFGDB_SUCCESS)
  {
    netmgr_main_cfg.tc_ul_ceiling_mbps = (uint32)prop_val_int;
  }

  /* dynamic_tc_ul_baserate */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".dynamic_tc_ul_baserate");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    prop_val_int = 0;
    netmgr_log_med("Error reading property [%s] Err[%d]; Setting to 0", prop_name, ret);
  }
  netmgr_main_cfg.dynamic_tc_ul_baserate = (boolean)prop_val_int;

  /* tc_ul_burst */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".tc_ul_burst");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_med("Error reading property [%s] Err[%d]; setting to 0", prop_name, ret);
    netmgr_main_cfg.tc_ul_burst = 0;
  }
  else
  {
    netmgr_main_cfg.tc_ul_burst = (unsigned long)prop_val_int;
  }

  /* netdev_max_backlog */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".netdev_max_backlog");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    prop_val_int = 0;
    netmgr_log_err("Error reading property [%s] Err[%d]; Setting to 0", prop_name, ret);
  }
  netmgr_main_cfg.netdev_max_backlog = (int)prop_val_int;

  /* disable_tcp_hystart_detect */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".disable_tcp_hystart_detect");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    prop_val_int = 0;
    netmgr_log_err("Error reading property [%s] Err[%d]; Setting to 0", prop_name, ret);
  }
  netmgr_main_cfg.disable_tcp_hystart_detect = (int)prop_val_int;

  /* disable_hystart */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".disable_hystart");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    prop_val_int = 0;
    netmgr_log_err("Error reading property [%s] Err[%d]; Setting to 0", prop_name, ret);
  }
  netmgr_main_cfg.disable_hystart = (int)prop_val_int;

  /* initial_ssthresh */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".initial_ssthresh");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    prop_val_int = -1;
    netmgr_log_err("Error reading property [%s] Err[%d]; Setting to -1", prop_name, ret);
  }
  netmgr_main_cfg.initial_ssthresh = (int)prop_val_int;

  /* frag_low_mem */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".frag_low_mem");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    prop_val_int = 0;
    netmgr_log_err("Error reading property [%s] Err[%d]; Setting to 0", prop_name, ret);
  }
  netmgr_main_cfg.frag_low_mem = (int)prop_val_int;

  /* frag_high_mem */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".frag_high_mem");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    prop_val_int = 0;
    netmgr_log_err("Error reading property [%s] Err[%d]; Setting to 0", prop_name, ret);
  }
  netmgr_main_cfg.frag_high_mem = (int)prop_val_int;

  /* tcp_ack_prio */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".tcp_ack_prio");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    prop_val_int = 0;
    netmgr_log_err("Error reading property [%s] Err[%d]; Setting to 0", prop_name, ret);
  }
  netmgr_main_cfg.tcp_ack_prio = (int)prop_val_int;

  /* qos_hybrid */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".hybrid_qos");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    prop_val_int = 0;
    netmgr_log_err("Error reading property [%s] Err[%d]; Setting to 0", prop_name, ret);
  }
  netmgr_main_cfg.hybrid_qos = (int)prop_val_int;

  /* rsc */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".rsc");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    prop_val_int = 0;
    netmgr_log_err("Error reading property [%s] Err[%d]; Setting to 0", prop_name, ret);
  }
  netmgr_main_cfg.rsc = (int)prop_val_int;

  /* rsb */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".rsb");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    prop_val_int = 0;
    netmgr_log_err("Error reading property [%s] Err[%d]; Setting to 0", prop_name, ret);
  }
  netmgr_main_cfg.rsb = (int)prop_val_int;

  /* mhip */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".mhip");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    prop_val_int = 0;
    netmgr_log_err("Error reading property [%s] Err[%d]; Setting to 0", prop_name, ret);
  }
  netmgr_main_cfg.mhip = (int)prop_val_int;

  /* skip_buffered_qos_modify */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".skip_buffered_qos_modify");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]; Setting to 0", prop_name, ret);
    prop_val_int = 0;
  }
  netmgr_main_cfg.skip_buffered_qos_modify = (int)prop_val_int;

  /* uplink_priority_port */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".uplink_priority_port");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]; Setting to 0", prop_name, ret);
    prop_val_int = 0;
  }
  netmgr_main_cfg.uplink_priority_port = (uint16_t)prop_val_int;

  /* num_modems and enable status */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".num_modems");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &num_modems, sizeof(num_modems));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    result = -1;
    goto bail;
  }
  for (i = 0; i < num_modems; i++)
  {
    snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s%d", "netmgr_config.", target, ".modems_enabled.",i);
    ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
    if (ret != CFGDB_SUCCESS)
    {
      netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
      result = -1;
      goto bail;
    }
    netmgr_main_cfg.modem_enable[i] = (prop_val_int ? TRUE : FALSE);
  }

  /* control_ports_len and control_ports */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".control_ports_len");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &control_ports_len, sizeof(control_ports_len));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    result = -1;
    goto bail;
  }

  /* static_fwd_links */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".static_fwd_links");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    prop_val_int = control_ports_len;
    netmgr_log_err("Error reading property [%s] Err[%d]; Setting to num control ports", prop_name, ret);
    /* Use this absence of this config to indicate that dynamic link creation is not supported for this target.
       Set the link ranges to the default, un-extended values */
    netmgr_main_cfg.dynamic_link_creation = 0;
    netmgr_main_cfg.rev_mux_offset = NETMGR_REV_MUX_ID_OFFSET; /* 11, or 17 for new targets */
    netmgr_main_cfg.fwd_link_start = NETMGR_RMNET_START; /* 0 */
    netmgr_main_cfg.rev_link_start = NETMGR_REV_RMNET_START; /* 22, or 34 for new targets */
    netmgr_main_cfg.max_links_per_modem = NETMGR_MAX_LINKS_PER_MODEM; /* 11+9, or 17+16 on newer targets */
    netmgr_main_cfg.max_links_in_total = NETMGR_RMNET_LAST_LINK; /* 66, only used on newer targets */
  }
  else
  {
    /* Use this config to indicate that dynamic link creation is supported for this target.
       Update the link ranges to the extended values */
    netmgr_main_cfg.dynamic_link_creation = 1;
    netmgr_main_cfg.rev_mux_offset = NETMGR_REV_MUX_ID_OFFSET_EXT; /* 11, or 17 for new targets */
    netmgr_main_cfg.fwd_link_start = NETMGR_RMNET_START; /* 0 */
    netmgr_main_cfg.rev_link_start = NETMGR_REV_RMNET_START_EXT; /* 22, or 34 for new targets */
    netmgr_main_cfg.max_links_per_modem = NETMGR_MAX_LINKS_PER_MODEM_EXT; /* 11+9, or 17+16 on newer targets */
    netmgr_main_cfg.max_links_in_total = NETMGR_RMNET_LAST_LINK_EXT; /* 66, only used on newer targets */
  }
  netmgr_main_cfg.static_fwd_links = (int)prop_val_int;

  netmgr_main_cfg.nint = netmgr_main_cfg.max_links_in_total;

  /* Disable all the links first, only enable ones present in config file */
  for (i = 0; i < netmgr_main_cfg.max_links_in_total; i++)
  {
    netmgr_ctl_port_array[i].enabled = FALSE;
  }

  for (i = 0; i < control_ports_len; i++)
  {
    snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s%d", "netmgr_config.", target, ".control_ports.",i);
    memset(prop_val_str, 0, sizeof(prop_val_str));
    ret = configdb_get_parameter(prop_name, CFGDB_TYPE_STRING, &prop_val_str, sizeof(prop_val_str));
    if (ret != CFGDB_SUCCESS)
    {
      netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
      result = -1;
      goto bail;
    }
    strlcpy(netmgr_ctl_port_array[i].qmi_conn_id, prop_val_str, sizeof(netmgr_ctl_port_array[i].qmi_conn_id));
    if (i < netmgr_main_cfg.static_fwd_links)
    {
      netmgr_ctl_port_array[i].enabled = TRUE;
    }
  }

  /* data_ports_len and data_ports */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".data_ports_len");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &data_ports_len, sizeof(data_ports_len));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    result = -1;
    goto bail;
  }
  netmgr_main_cfg.num_data_ports = data_ports_len;

  for (i = 0; i < data_ports_len; i++)
  {
    snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s%d", "netmgr_config.", target, ".data_ports.",i);
    memset(prop_val_str, 0, sizeof(prop_val_str));
    ret = configdb_get_parameter(prop_name, CFGDB_TYPE_STRING, &prop_val_str, sizeof(prop_val_str));
    if (ret != CFGDB_SUCCESS)
    {
      netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
      result = -1;
      goto bail;
    }
    strlcpy(netmgr_ctl_port_array[i].data_ctl_port, prop_val_str, sizeof(netmgr_ctl_port_array[i].data_ctl_port));
    if (i < netmgr_main_cfg.static_fwd_links)
    {
      netmgr_ctl_port_array[i].enabled = TRUE;
    }
  }

#ifdef FEATURE_DATA_IWLAN
  /* Process iWLAN related configuration items */
  if (0 != netmgr_process_iwlan_config(target))
  {
    netmgr_log_err("%s", "Error processing iWLAN config items");
    result = -1;
    goto bail;
  }
#endif /* FEATURE_DATA_IWLAN */

  netmgr_process_cmd_logging_config();

  /* Remove prefix table as an enhancement later */
  if (netmgr_main_cfg.rmnet_data_enabled)
  {
    if (netmgr_main_cfg.modem_enable[0])
    {
      /* If rmnet_data is enabled, then update the dev-prefix for approriate modem */
      strlcpy(NETMGR_MAIN_GET_DEV_PREFIX(NETMGR_MODEM_MSM),
              NETMGR_MAIN_RMNET_DATA_PREFIX,
              NETMGR_IF_NAME_MAX_LEN);
    }
    else if(netmgr_main_cfg.modem_enable[1])
    {
      /* If rmnet_data is enabled, then update the dev-prefix for approriate modem */
      strlcpy(NETMGR_MAIN_GET_DEV_PREFIX(NETMGR_MODEM_MDM),
              NETMGR_MAIN_RMNET_DATA_PREFIX,
              NETMGR_IF_NAME_MAX_LEN);
    }
  }

bail:
  return result;
}

/*===========================================================================
  FUNCTION:  netmgr_config_overwrite
===========================================================================*/
/*!
    @brief
    This function overwrites the configuration for targets whose cpu
    core numbers/clusters are different from previous targets

    @param None.
    @return  None.
*/
/*=========================================================================*/
void netmgr_config_overwrite()
{
  if (DS_TARGET_SOC_ID_SDM632 == ds_get_soc_id())
  {
    /* Cores 0-3 are Silver and 4-7 are Gold.
     * avoiding Core 0, as it is boot core
     */
    netmgr_main_cfg.vnd_rps_mask = 4;
    netmgr_main_cfg.pnd_rps_mask = 8;
    netmgr_main_cfg.clat_rps_mask = 2;
  }
  else if ( DS_TARGET_SOC_ID_SDM439 == ds_get_soc_id())
  {
    /* Cores 0-3 are Gold and cores 4-7 are Silver.
     * Avoiding CPU core 4,
     * Boot core functionality will be offloaded to this core after boot-up
     */
    netmgr_main_cfg.vnd_rps_mask = 192;
    netmgr_main_cfg.pnd_rps_mask = 32;
    netmgr_main_cfg.clat_rps_mask = 0;
  }
#ifdef NETMGR_AUTO
  else if (!strncmp(target, "msm8996", strlen("msm8996")))
  {
    netmgr_main_cfg.vnd_rps_mask = 12;
    netmgr_main_cfg.pnd_rps_mask = 2;
    netmgr_main_cfg.clat_rps_mask = 0;
  }
#endif

}

/*===========================================================================
  FUNCTION:  netmgr_config_print
===========================================================================*/
/*!
    @brief
    This function prints the configuration currently loaded for netmgr.

    @param None.
    @return  None.
*/
/*=========================================================================*/
void netmgr_config_print()
{
  int i;
  netmgr_log_med("==== Netmanager Configuration ====");
  netmgr_log_med("..qmi_dpm_enabled        : [%d]",     netmgr_main_cfg.dpm_enabled);
  netmgr_log_med("..dpm_retry_timeout      : [%d]",     netmgr_main_cfg.dpm_retry_timeout);
  netmgr_log_med("..wda_data_format_enabled: [%d]",     netmgr_main_cfg.wda_data_format);
  netmgr_log_med("..ep_type                : [%d]",     netmgr_main_cfg.ep_type);
  netmgr_log_med("..epid                   : [%d]",     netmgr_main_cfg.epid);
  netmgr_log_med("..single_qmux_ch_enabled : [%d]",     netmgr_main_cfg.single_qmux_ch_enabled);
  netmgr_log_med("..single_qmux_ch_name    : [%s]",     netmgr_main_cfg.qmux_ch_name);
  netmgr_log_med("..single_qmux_ch_id      : [%s]",     netmgr_main_cfg.smd_ch_name);
  netmgr_log_med("..rmnet_data_enabled     : [%d]",     netmgr_main_cfg.rmnet_data_enabled);
  netmgr_log_med("..phys_net_dev           : [%s]",     netmgr_main_cfg.phys_net_dev);
  netmgr_log_med("..msm_modem_enabled      : [%d]",     netmgr_main_cfg.modem_enable[NETMGR_MODEM_MSM]);
  netmgr_log_med("..mdm_modem_enabled      : [%d]",     netmgr_main_cfg.modem_enable[NETMGR_MODEM_MDM]);
  netmgr_log_med("..modem_ssr_state        : [%d]",     netmgr_main_cfg.modem_ssr_state);
  netmgr_log_med("..df.dl_agg_size         : [%d]",     netmgr_main_cfg.data_format.dl_agg_size);
  netmgr_log_med("..df.dl_agg_cnt          : [%d]",     netmgr_main_cfg.data_format.dl_agg_cnt);
  netmgr_log_med("..df.dl_agg_mode         : [%d]",     netmgr_main_cfg.data_format.dl_data_aggregation_protocol);
  netmgr_log_med("..df.ul_agg_mode         : [%d]",     netmgr_main_cfg.data_format.ul_data_aggregation_protocol);
  netmgr_log_med("..df.dl_gro_enabled      : [%d]",     netmgr_main_cfg.data_format.dl_gro_enabled);
  netmgr_log_med("..exec_opts.log_options  : [0x%08X]", netmgr_main_cfg.exec_opts.log_options);
  netmgr_log_med("..exec_opts.outfile      : [%s]",     netmgr_main_cfg.exec_opts.outfile);
  netmgr_log_med("..df.ul_gso_enabled      : [%d]", netmgr_main_cfg.data_format.ul_gso_enabled);
  netmgr_log_med("..df.ul_agg_size         : [%d]",     netmgr_main_cfg.data_format.ul_agg_size);
  netmgr_log_med("..df.ul_agg_cnt          : [%d]",     netmgr_main_cfg.data_format.ul_agg_cnt);
  netmgr_log_med("..kfc_mode               : [%d]",     netmgr_main_cfg.kfc_mode);
  netmgr_log_med("..kfc_qmap               : [%d]",     netmgr_main_cfg.kfc_qmap);
  netmgr_log_med("..qmi_pc                 : [%d]",     netmgr_main_cfg.qmi_pc);
  netmgr_log_med("..rsc                    : [%d]",     netmgr_main_cfg.rsc);
  netmgr_log_med("..rsb                    : [%d]",     netmgr_main_cfg.rsb);
  netmgr_log_med("..mhip                   : [%d]",     netmgr_main_cfg.mhip);
  netmgr_log_med("..num_data_ports         : [%u]",     netmgr_main_cfg.num_data_ports);
  netmgr_log_med("..num_rev_data_ports     : [%u]",     netmgr_main_cfg.num_rev_data_ports);
  netmgr_log_med("..static_fwd_links       : [%u]",     netmgr_main_cfg.static_fwd_links);
  netmgr_log_med("..static_rev_links       : [%u]",     netmgr_main_cfg.static_rev_links);
  netmgr_log_med("..dynamic_link_creation  : [%d]",     netmgr_main_cfg.dynamic_link_creation);
  netmgr_log_med("..rev_mux_offset         : [%d]",     netmgr_main_cfg.rev_mux_offset);
  netmgr_log_med("..fwd_link_start         : [%d]",     netmgr_main_cfg.fwd_link_start);
  netmgr_log_med("..rev_link_start         : [%d]",     netmgr_main_cfg.rev_link_start);
  netmgr_log_med("..max_links_per_modem    : [%d]",     netmgr_main_cfg.max_links_per_modem);
  netmgr_log_med("..max_links_in_total     : [%d]",     netmgr_main_cfg.max_links_in_total);
  netmgr_log_med("..nint                   : [%d]",     netmgr_main_cfg.nint);
  netmgr_log_med("..uplink_priority_port   : [%u]",     netmgr_main_cfg.uplink_priority_port);

  for (i = 0; i < netmgr_main_cfg.max_links_in_total; i++)
  {
    netmgr_log_med("Link[%d] port[%s] qmi[%s] modem_wait[%d] enabled[%d] on_demand[%d]",
                    netmgr_ctl_port_array[i].link_id,
                    netmgr_ctl_port_array[i].data_ctl_port,
                    netmgr_ctl_port_array[i].qmi_conn_id,
                    netmgr_ctl_port_array[i].modem_wait,
                    netmgr_ctl_port_array[i].enabled,
                    netmgr_ctl_port_array[i].on_demand);
  }
  netmgr_log_med("========");


  return;
}

/*===========================================================================
  FUNCTION:  netmgr_config_print_cb
===========================================================================*/
/*!
    @brief
    Wrapper around netmgr_config_print() to make it work with CB framework
*/
/*=========================================================================*/
LOCAL void *
netmgr_config_print_cb(__attribute__((unused)) void *arg)
{
  netmgr_config_print();
  return 0;
}

/*===========================================================================
  FUNCTION:  netmgr_config_check_output_mark_availability
===========================================================================*/
/*!
    @brief
    Check whether output-mark feature is present or not. Decision is based
    on kernel version as from kernel 4.9 it is present in kernels.
*/
/*=========================================================================*/
void
netmgr_config_check_output_mark_availability()
{
  struct utsname system_info;
  int ret=0;
  if(0==(ret=uname(&system_info)))
  {
    netmgr_log_med("system_info : sysname [%s] nodename [%s] release [%s] version [%s]  machine [%s]",\
          system_info.sysname,system_info.nodename,system_info.release,system_info.version,system_info.machine);
  }
  else
  {
    netmgr_log_err("Failed to get system_info ret [%d] !!", ret);
  }

  if((system_info.release[0] && system_info.release[0]>='4')
	&& (system_info.version[0] && system_info.version[0]>='9'))
  {
    netmgr_main_cfg.output_mark_enable = (int)NETMGR_TRUE;
  }
  else
  {
    netmgr_main_cfg.output_mark_enable = (int)NETMGR_FALSE;
  }
}

/*===========================================================================
  FUNCTION:  netmgr_config_init
===========================================================================*/
/*!
    @brief
    Initialize the configration framework. Things that don't belong in actual
    configuration loading.
*/
/*=========================================================================*/
void
netmgr_config_init()
{
  /* Register callbacks with the MAIN table */
  netmgr_cb_insert_cb(NETMGR_MAIN_CB_TABLE,
                      NETMGR_MAIN_CB_SIGUSR1,
                      NETMGR_CONFIG_CB_MODNAME,
                      &netmgr_config_print_cb);
}
