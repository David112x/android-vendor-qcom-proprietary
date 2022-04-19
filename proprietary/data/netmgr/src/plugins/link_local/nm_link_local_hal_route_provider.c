
/******************************************************************************

     N M _ L I N K _ L O C A L _ H A L _ R O U T E _ P R O V I D E R. C

******************************************************************************/

/******************************************************************************

  @file    nm_link_local_hal_route_provider.c
  @brief   Link-local routing plugin

  DESCRIPTION
  Manage the routing rules required for link-local data calls

******************************************************************************/
/*===========================================================================

  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

/*===========================================================================
                              INCLUDE FILES
===========================================================================*/


#include <string.h>
#include "netmgr.h"
#include "netmgr_kif.h"
#include "netmgr_netd_client.h"
#include "nm_link_local_route_provider_factory.h"

#define NM_LINK_LOCAL_NW_HANDLE_INVALID  0x0

static boolean bHalNetCreated = FALSE;

static int nm_link_local_hal_prov_init(boolean force);
static int nm_link_local_hal_prov_deinit(int restart);
static int nm_link_local_hal_prov_add_interface(int link, char* link_name);
static int nm_link_local_hal_prov_remove_interface(int link, char* link_name);

nm_ll_route_provider_t ll_nm_link_local_hal_prov =
{
  /* Initialize the function table for
   * the hal route provider */
  .init         = nm_link_local_hal_prov_init,
  .deinit       = nm_link_local_hal_prov_deinit,
  .add_route    = nm_link_local_hal_prov_add_interface,
  .remove_route = nm_link_local_hal_prov_remove_interface
};

typedef struct networkSettings nwSettings_t;
static nwSettings_t nwSet;

#ifdef FEATURE_RECOVERY_ENABLED
static nwSettings_t recoveryNwSet;
#endif /* FEATURE_RECOVERY_ENABLED */

/*===========================================================================
  FUNCTION  nm_link_local_hal_prov_register_network
===========================================================================*/
/*!
@brief
  Helper function to cleanup custom routing rules

@return
  NETMGR_SUCCESS
  NETMGR_FAILURE
*/
/*=========================================================================*/
static int nm_link_local_hal_prov_register_network(void)
{
  int rc = NETMGR_FAILURE;

  rc = NetmgrNetdClientRegisterNetwork(&nwSet, "LINK_LOCAL");
  if (NETMGR_SUCCESS != rc)
  {
    netmgr_log_err("%s(): Failed to create custom network !\n", __func__);
    return NETMGR_FAILURE;
  }

  netmgr_log_med("%s(): Successfully created network with Handle: 0x%" PRIx64 ""
                 " and Packet Mark: 0x%" PRIx32 "", __func__, nwSet.networkHandle,
                 nwSet.packetMark);

  return NETMGR_SUCCESS;
}

#ifdef FEATURE_RECOVERY_ENABLED
/*===========================================================================
  FUNCTION  nm_link_local_hal_prov_unregister_network
===========================================================================*/
/*!
@brief
  Helper function to cleanup custom routing rules

@return
  NETMGR_SUCCESS
  NETMGR_FAILURE
*/
/*=========================================================================*/
static int nm_link_local_hal_prov_unregister_network(uint64_t nw_handle)
{
  int rc = NETMGR_FAILURE;

  if (NM_LINK_LOCAL_NW_HANDLE_INVALID == nw_handle)
  {
    netmgr_log_err("%s(): network handle is invalid!", __func__);
    return NETMGR_FAILURE;
  }

  netmgr_log_low("%s(): unregister network for handle [0x%" PRIx64 "]",
                 __func__, nw_handle);
  rc = NetmgrNetdClientUnregisterNetwork(nw_handle, "LINK_LOCAL");
  if (NETMGR_SUCCESS != rc)
  {
    return NETMGR_FAILURE;
  }

  return NETMGR_SUCCESS;
}
#endif /* FEATURE_RECOVERY_ENABLED */

/*===========================================================================
  FUNCTION  nm_link_local_hal_prov_init
===========================================================================*/
/*!
@brief
  Creates network to manage custom routing rules

@return
  NETMGR_SUCCESS on successful command execution
  NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
static int nm_link_local_hal_prov_init(boolean force)
{
#ifdef FEATURE_RECOVERY_ENABLED
  int rc = NETMGR_SUCCESS;
  file_info_t recovery_data;

  memset(&recovery_data, 0, sizeof(file_info_t));
#endif /* FEATURE_RECOVERY_ENABLED */

  if (force)
  {
    netmgr_log_med("%s(): forcing network registration for link-local",
                   __func__);

#ifdef FEATURE_RECOVERY_ENABLED
    /* When netd restarts all the previously installed routing rules and
       custom OEM networks will be flushed. We will need to invalidate
       cached network settings information for link-local in the recovery
       file */
    recovery_data.is_valid = TRUE;
    recovery_data.info_type = TYPE_LINK_LOCAL_NW_SETTINGS;

    rc = netmgr_recovery_file_find_and_invalidate_entry(&recovery_data);
    if (NETMGR_SUCCESS != rc)
    {
      /* If there was no cached information available or lookup failed for
         some reason then we will assume that no OEM network was created
         for link-local calls and proceed accordingly. The error from
         recovery module will be ignored but we will print it for logging
         purpose */
      netmgr_log_err("%s(): failed to find and invalidate link-local nw settings"
                     " in recovery file", __func__);
      memset(&nwSet, 0, sizeof(struct networkSettings));
    }
#endif /* FEATURE_RECOVERY_ENABLED */

    bHalNetCreated = FALSE;
  }

  if (!bHalNetCreated)
  {
    if(NETMGR_SUCCESS != nm_link_local_hal_prov_register_network())
    {
      return NETMGR_FAILURE;
    }

#ifdef FEATURE_RECOVERY_ENABLED
    /* Save the network handle and packet mark for link-local module
       in recovery file */
    recovery_data.is_valid = TRUE;
    recovery_data.info_type = TYPE_LINK_LOCAL_NW_SETTINGS;
    recovery_data.recovery_info.nw_settings.network_hndl = nwSet.networkHandle;
    recovery_data.recovery_info.nw_settings.packet_mark = nwSet.packetMark;

    rc = netmgr_recovery_file_write_entry(&recovery_data);
    if (NETMGR_SUCCESS != rc)
    {
      netmgr_log_err("%s(): failed to save link-local network settings "
                     "in recovery file!", __func__);

      /* TODO: How to handle failure to write to recovery file? */
    }
#endif /* FEATURE_RECOVERY_ENABLED */

    bHalNetCreated = TRUE;
  }

  return NETMGR_SUCCESS;
}

/*===========================================================================
  FUNCTION  nm_link_local_hal_prov_deinit
===========================================================================*/
/*!
@brief
  Destroys the custom oem network

@return
  NETMGR_SUCCESS on successful command execution
  NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
static int nm_link_local_hal_prov_deinit(int restart)
{
#if defined(FEATURE_RECOVERY_ENABLED)

  int rc = NETMGR_FAILURE;
  int file_idx = -1;
  uint64_t nw_handle = NM_LINK_LOCAL_NW_HANDLE_INVALID;
  file_info_t recovery_data;

  if (restart)
  {
    /* Check if recovery file has information on link-local network
       handle. If yes we need to use that handle to unregister with
       netd */
    memset(&recovery_data, 0, sizeof(file_info_t));
    recovery_data.is_valid = TRUE;
    recovery_data.info_type = TYPE_LINK_LOCAL_NW_SETTINGS;

    rc = netmgr_recovery_file_find_entry(&recovery_data, &file_idx);
    if (NETMGR_SUCCESS != rc)
    {
      /* If there was no cached information available then we can proceed
         without any side-effects. However, if there was some issue in
         reading the information from the file then we will be unable
         to cleanup the old routing rules which could cause side-effects.
         Additionally the entry in the file may not be invalidated which
         may result in the file slowly running out of space.

         TODO: need to determine the correct way of handling errors */
      netmgr_log_err("%s(): failed to find link-local settings in recovery file", __func__);
      memset(&nwSet, 0, sizeof(struct networkSettings));
    }
    else
    {
      memset(&recoveryNwSet, 0, sizeof(struct networkSettings));
      recoveryNwSet.networkHandle = recovery_data.recovery_info.nw_settings.network_hndl;
      recoveryNwSet.packetMark = recovery_data.recovery_info.nw_settings.packet_mark;

      if (NM_LINK_LOCAL_NW_HANDLE_INVALID != recoveryNwSet.networkHandle)
      {
        netmgr_log_low("%s(): attempting to cleanup old link-local "
                       "routing rules", __func__);
        nw_handle = recoveryNwSet.networkHandle;
      }
    }
  }
  else
  {
    if (bHalNetCreated)
    {
      nw_handle = nwSet.networkHandle;
    }
  }

  rc = nm_link_local_hal_prov_unregister_network(nw_handle);
  if (NETMGR_SUCCESS != rc)
  {
    netmgr_log_err("%s(): failed to destroy OEM network "
                   "for link-local calls using handle [0x%" PRIx64 "]!",
                   __func__, nw_handle);
    return NETMGR_FAILURE;
  }

  if (restart)
  {
    /* After we have destroyed the OEM network we need to invalidate
       the entry in the recovery file */
    if (NM_LINK_LOCAL_NW_HANDLE_INVALID != recoveryNwSet.networkHandle)
    {
      recovery_data.is_valid = FALSE;
      rc = netmgr_recovery_file_write_entry(&recovery_data);
      if (NETMGR_SUCCESS != rc)
      {
        netmgr_log_err("%s(): failed to invalidate old link-local network settings!",
                       __func__);
      }

      memset(&nwSet, 0, sizeof(struct networkSettings));
      memset(&recoveryNwSet, 0, sizeof(struct networkSettings));
    }
  }

  bHalNetCreated = FALSE;

  return NETMGR_SUCCESS;

#else

  (void) restart;

  return NETMGR_SUCCESS;
#endif /* FEATURE_RECOVERY_ENABLED */
}

/*===========================================================================
  FUNCTION  nm_link_local_hal_prov_add_route
===========================================================================*/
/*!
@brief
  Setup routing rule for the provided link

@return
  NETMGR_SUCCESS on successful command execution
  NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
static int nm_link_local_hal_prov_add_interface(int link, char* link_name)
{
  int rc = NETMGR_FAILURE;

  NETMGR_LOG_FUNC_ENTRY;

  if (!link_name)
  {
    netmgr_log_err("%s(): link_name cannot be NULL!", __func__);
    goto bail;
  }

  /* If netd had started after netmgr in the bootup sequence then the initial
     attempt to create the network might have failed. Attempt to create
     the network again here */
  if (!bHalNetCreated)
  {
    if (NETMGR_SUCCESS != nm_link_local_hal_prov_register_network())
    {
      netmgr_log_err("%s(): Failed to create custom network!", __func__);
      goto bail;
    }
  }

  if (NETMGR_SUCCESS
        != NetmgrNetdClientAddInterfaceToOemNetwork(nwSet.networkHandle, link_name))
  {
    netmgr_log_err("%s(): failed to add link-local iface [%d] to custom network",
                   __func__, link);
    goto bail;
  }

  netmgr_log_med( "%s(): successfully added link-local iface [%d]"
                  " to custom network\n", __func__, link);

  rc = NETMGR_SUCCESS;

bail:
  NETMGR_LOG_FUNC_EXIT;
  return rc;
}


/*===========================================================================
  FUNCTION  nm_link_local_hal_prov_remove_route
===========================================================================*/
/*!
@brief
  Remove routing rule for the provided link

@return
  NETMGR_SUCCESS on successful command execution
  NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
static int nm_link_local_hal_prov_remove_interface(int link, char* link_name)
{
  int rc = NETMGR_FAILURE;

  NETMGR_LOG_FUNC_ENTRY;

  if (!link_name)
  {
    netmgr_log_err("%s(): link_name cannot be NULL!", __func__);
    goto bail;
  }

  if (NETMGR_SUCCESS
        != NetmgrNetdClientRemoveInterfaceFromOemNetwork(nwSet.networkHandle,
                                                         link_name))
  {
    netmgr_log_err("%s(): failed to remove link-local iface [%d] "
                   "to custom network", __func__, link);
    goto bail;
  }

   netmgr_log_med( "%s(): successfully removed link-local iface [%d]"
                    " from custom network\n",
                    __func__, link);

   rc = NETMGR_SUCCESS;

bail:
  NETMGR_LOG_FUNC_EXIT;
  return rc;
}
