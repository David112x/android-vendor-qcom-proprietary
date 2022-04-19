/******************************************************************************

      N M _ M D M P R X Y _ H A L _ R O U T E _ P R O V I D E R . C

******************************************************************************/

/******************************************************************************

  @file    nm_mdmproxy_hal_route_provider.c
  @brief   Modem proxy plugin

  DESCRIPTION
  Modem proxy module is responsible for managing routing rules and packet
  marking rules for forwarding packets to modem.

******************************************************************************/
/*===========================================================================

  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/
/*===========================================================================
                              INCLUDE FILES
===========================================================================*/

#include <sys/socket.h> /* Needs to be removed */
#include <linux/if.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "netmgr.h"
#include "netmgr_kif.h"

#include "nm_mdmprxy.h"
#include "nm_mdmprxy_route_provider_factory.h"
#include "nm_mdmprxy_route_provider_common.h"
#include "nm_mdmprxy_common.h"
#include "netmgr_netd_client.h"

/*===========================================================================
                             LOCAL DECLARATIONS
===========================================================================*/
#define NM_IWLAN_NW_HANDLE_INVALID  0x0

static int nm_mdmprxy_hal_prov_add_route
(
  int                            link,
  char*                          link_name,
  int                            tech_type,
  sa_family_t                    af,
  ipsec_tunnel_info_t            *tunnel_info,
  struct netmgr_kif_addrstorage  *link_addr
);

static int nm_mdmprxy_hal_prov_remove_route
(
  int                           link,
  char*                         link_name,
  int                           tech_type,
  sa_family_t                   af,
  ipsec_tunnel_info_t           *tunnel_info,
  struct netmgr_kif_addrstorage *link_addr
);

static void nm_mdmprxy_hal_prov_netd_oos_hdlr(void);

static uint32_t nm_mdmprxy_hal_prov_netd_is_hdlr(void);

static int nm_mdmprxy_hal_prov_enable_forwarding(int is_netd_restart);

static int nm_mdmprxy_hal_prov_disable_forwarding(void);

static int nm_mdmprxy_hal_prov_netd_link_is_hdlr(void* arg);

static int nm_mdmprxy_hal_prov_link_oos_hdlr(void* arg);

static uint32_t nm_mdmprxy_hal_prov_init(void);

static int nm_mdmprxy_hal_prov_deinit(int restart);

nm_mdmprxy_route_prov_t nm_mdmprxy_hal_route_provider =
{
  .add_route               = nm_mdmprxy_hal_prov_add_route,
  .remove_route            = nm_mdmprxy_hal_prov_remove_route,
  .install_rule            = nm_mdmprxy_route_prov_common_install_iiface_rules,
  .remove_rule             = nm_mdmprxy_route_prov_common_remove_iiface_rules,
  .enable_forwarding       = nm_mdmprxy_hal_prov_enable_forwarding,
  .disable_forwarding      = nm_mdmprxy_hal_prov_disable_forwarding,
  .init                    = nm_mdmprxy_hal_prov_init,
  .deinit                  = nm_mdmprxy_hal_prov_deinit,
  .is_hdlr                 = nm_mdmprxy_route_prov_common_is_hdlr,
  .oos_hdlr                = nm_mdmprxy_route_prov_common_oos_hdlr,
  .is_hdlr_per_link        = nm_mdmprxy_route_prov_common_link_is_hdlr,
  .oos_hdlr_per_link       = nm_mdmprxy_hal_prov_link_oos_hdlr,
  .netd_oos_hdlr           = nm_mdmprxy_hal_prov_netd_oos_hdlr,
  .netd_is_hdlr            = nm_mdmprxy_hal_prov_netd_is_hdlr,
  .netd_is_hdlr_per_link   = nm_mdmprxy_hal_prov_netd_link_is_hdlr
};

typedef struct networkSettings nw_settings_t;

static boolean hal_network_created = FALSE;

static nw_settings_t nw_info;

#ifdef FEATURE_RECOVERY_ENABLED
static nw_settings_t recovery_nw_info;
#endif /* FEATURE_RECOVERY_ENABLED */

/*===========================================================================
                           LOCAL DEFINITIONS
===========================================================================*/
/*===========================================================================
  FUNCTION  nm_mdmprxy_hal_prov_register_network
===========================================================================*/
/*!
@brief
  Creates network to manage custom routing rules

@return
  NETMGR_SUCCESS/NETMGR_FAILURE
*/
/*=========================================================================*/
static int nm_mdmprxy_hal_prov_register_network(void)
{
  int rc = NETMGR_FAILURE;
  nw_settings_t ns = {0, 0};

  NETMGR_LOG_FUNC_ENTRY;

  if (FALSE == hal_network_created)
  {
    rc = NetmgrNetdClientRegisterNetwork(&ns, "IWLAN_NETWORK");
    if (NETMGR_SUCCESS != rc)
    {
      netmgr_log_err("%s(): failed to create network!", __func__);
      NETMGR_LOG_FUNC_EXIT;
      return NETMGR_FAILURE;
    }

    hal_network_created = TRUE;

    netmgr_log_med("%s(): Successfully created new network, "
                   "[handle : 0x%" PRIx64 "] [mark : 0x%" PRIx32 "]",
                   __func__, ns.networkHandle, ns.packetMark);

    /* Update global data */
    nw_info.networkHandle = ns.networkHandle;
    nw_info.packetMark = ns.packetMark;
  }

  NETMGR_LOG_FUNC_EXIT;
  return NETMGR_SUCCESS;
}

#ifdef FEATURE_RECOVERY_ENABLED
/*===========================================================================
  FUNCTION  nm_mdmprxy_hal_prov_unregister_network
===========================================================================*/
/*!
@brief
  Helper function to cleanup custom routing rules

@return
  NETMGR_SUCCESS/NETMGR_FAILURE
*/
/*=========================================================================*/
static int nm_mdmprxy_hal_prov_unregister_network(uint64_t nw_handle)
{
  int rc = NETMGR_FAILURE;

  if (NM_IWLAN_NW_HANDLE_INVALID == nw_handle)
  {
    netmgr_log_err("%s(): network handle is invalid!", __func__);
    return NETMGR_FAILURE;
  }

  netmgr_log_low("%s(): unregister network for handle [0x%" PRIx64 "]",
                 __func__, nw_handle);
  rc = NetmgrNetdClientUnregisterNetwork(nw_handle, "IWLAN_NETWORK");
  if (NETMGR_SUCCESS != rc)
  {
    return NETMGR_FAILURE;
  }

  /* Invalidate global network settings */
  nw_info.networkHandle = NM_IWLAN_NW_HANDLE_INVALID;
  nw_info.packetMark = NM_MDMPRXY_INVALID_PACKET_MARK;

  return NETMGR_SUCCESS;
}
#endif /* FEATURE_RECOVERY_ENABLED */

/*===========================================================================
  FUNCTION  nm_mdmprxy_hal_prov_add_route
===========================================================================*/
/*!
@brief
  Setup routing rules for the provided link

@return
  NETMGR_SUCCESS/NETMGR_FAILURE
*/
/*=========================================================================*/
static int nm_mdmprxy_hal_prov_add_route
(
  int                            link,
  char*                          link_name,
  int                            tech_type,
  sa_family_t                    af,
  ipsec_tunnel_info_t            *tunnel_info,
  struct netmgr_kif_addrstorage  *link_addr
)
{
  int rc = NETMGR_FAILURE;

  NETMGR_LOG_FUNC_ENTRY;

  NM_MDMPRXY_VAR_UNUSED(tech_type);
  NM_MDMPRXY_VAR_UNUSED(tunnel_info);
  NM_MDMPRXY_VAR_UNUSED(link_addr);

  if (!link_name)
  {
    netmgr_log_err("%s(): link_name cannot be NULL!", __func__);
    goto bail;
  }

  /* Currently netd-hal interface does not provide an option to enable routing rules per
     ip family. We will be installing rules for both V4 and V6 at the same time.
     To handle a dual-ip case where only one of the address families goes up and down
     we need to ensure routing rules installation and deletion is handled correctly */
  if ((AF_INET == af && TRUE == NM_MDMPRXY_GET_ROUTING_RULES_INSTALLED_V6(link))
        || (AF_INET6 == af && TRUE == NM_MDMPRXY_GET_ROUTING_RULES_INSTALLED_V4(link)))
  {
    /* Routing rules need to be installed only once */
    netmgr_log_med("%s(): Routing rules already installed for link [%s]", __func__, link_name);
    rc = NETMGR_SUCCESS;
    goto bail;
  }

  /* If netd had started after netmgr in the bootup sequence then the initial
     attempt to create the OEM network might have failed. Attempt to create
     the network again here */
  if (FALSE == hal_network_created)
  {
    if (NETMGR_SUCCESS != nm_mdmprxy_hal_prov_register_network())
    {
      netmgr_log_err("%s(): Failed to create custom network!", __func__);
      goto bail;
    }
  }

  /* Add link to custom network */
  rc = NetmgrNetdClientAddInterfaceToOemNetwork(nw_info.networkHandle, link_name);
  if (NETMGR_SUCCESS != rc)
  {
    netmgr_log_err("%s(): Failed to add interface [%s] to network!",
                   __func__, link_name);
    goto bail;
  }

  /* Add routing rule to send packets from the provided link over wlan0 */
  rc = NetmgrNetdClientSetForwardingBetweenInterfaces(link_name, "wlan0", TRUE);
  if (NETMGR_SUCCESS != rc)
  {
    netmgr_log_err("%s(): Failed to add forwarding rule from link [%s] to wlan0!",
                   __func__, link_name);
    goto bail;
  }

  /* Add a default V4 route link's table via dummy gateway. The cloned packets
   * will be forwarded to modem over this route */
  rc = NetmgrNetdClientAddRouteToOemNetwork(nw_info.networkHandle, link_name,
                                            NM_MDMPRXY_GATEWAY_V4_ADDR_MASK_STR, NULL);

  if (NETMGR_SUCCESS != rc)
  {
    netmgr_log_err("%s(): Failed to add default V4 gateway route for link [%s]!",
                   __func__, link_name);
    goto bail;
  }

  /* Add routing rule to send packets from the provided link over dummy0 */
  rc = NetmgrNetdClientSetForwardingBetweenInterfaces(link_name, "dummy0", TRUE);
  if (NETMGR_SUCCESS != rc)
  {
    netmgr_log_err("%s(): Failed to add forwarding rule from link [%s] to dummy0!",
                   __func__, link_name);
    goto bail;
  }

  /* Add a default V6 route link's table via dummy gateway. The cloned packets
   * will be forwarded to modem over this route */
  rc = NetmgrNetdClientAddRouteToOemNetwork(nw_info.networkHandle, link_name,
                                            NM_MDMPRXY_GATEWAY_V6_ADDR_MASK_STR, NULL);

  if (NETMGR_SUCCESS != rc)
  {
    netmgr_log_err("%s(): Failed to add default V6 gateway route for link [%s]!",
                   __func__, link_name);
    goto bail;
  }

bail:
  if (NETMGR_SUCCESS == rc)
  {
    NM_MDMPRXY_SET_ROUTING_RULES_INSTALLED(link, af, TRUE);
    netmgr_log_low("%s(): Routing rules installed for link [%s], family [%d]",
                   __func__, link_name, af);
  }

  NETMGR_LOG_FUNC_EXIT;
  return rc;
}

/*===========================================================================
  FUNCTION  nm_mdmprxy_hal_prov_remove_route
===========================================================================*/
/*!
@brief
  Remove routing rules for the provided link

@return
  NETMGR_SUCCESS/NETMGR_FAILURE
*/
/*=========================================================================*/
static int nm_mdmprxy_hal_prov_remove_route
(
  int                           link,
  char*                         link_name,
  int                           tech_type,
  sa_family_t                   af,
  ipsec_tunnel_info_t           *tunnel_info,
  struct netmgr_kif_addrstorage *link_addr
)
{
  int rc = NETMGR_FAILURE;

  NM_MDMPRXY_VAR_UNUSED(tech_type);
  NM_MDMPRXY_VAR_UNUSED(tunnel_info);
  NM_MDMPRXY_VAR_UNUSED(link_addr);

  NETMGR_LOG_FUNC_ENTRY;

  if (!link_name)
  {
    netmgr_log_err("%s(): link_name cannot be NULL!", __func__);
    goto bail;
  }

  if ( AF_INET != af && AF_INET6 != af)
  {
    netmgr_log_err("%s(): invalid ip family [%d]", __func__, af);
    goto bail;
  }

  if ((AF_INET == af && TRUE == NM_MDMPRXY_GET_ROUTING_RULES_INSTALLED_V6(link))
        || (AF_INET6 == af && TRUE == NM_MDMPRXY_GET_ROUTING_RULES_INSTALLED_V4(link)))
  {
    /* If one of the IP families is still up, we need to skip rule deletion*/
    netmgr_log_med("%s(): Cannot remove routing rules for link [%s] as another IP family"
                   " is still UP on it!", __func__, link_name);
    rc = NETMGR_SUCCESS;
    goto bail;
  }

  if (FALSE == NM_MDMPRXY_GET_ROUTING_RULES_INSTALLED_V4(link)
      && FALSE == NM_MDMPRXY_GET_ROUTING_RULES_INSTALLED_V6(link))
  {
    netmgr_log_med("%s(): Routing rules already removed for link [%s]", __func__, link_name);
    rc = NETMGR_SUCCESS;
    goto bail;
  }

  /* Remove link to custom network */
  rc = NetmgrNetdClientRemoveInterfaceFromOemNetwork(nw_info.networkHandle, link_name);
  if (NETMGR_SUCCESS != rc)
  {
    netmgr_log_err("%s(): failed to remove link [%s] from custom network",
                   __func__, link_name);
    goto bail;
  }

bail:
  if (NETMGR_SUCCESS == rc)
  {
    NM_MDMPRXY_SET_ROUTING_RULES_INSTALLED(link, af, FALSE);
    netmgr_log_med( "%s(): successfully removed link [%s], ip_family [%d]"
                    " from custom network\n",
                    __func__, link_name, af);
  }

  NETMGR_LOG_FUNC_EXIT;
  return rc;
}

/*===========================================================================
  FUNCTION  nm_mdmprxy_hal_prov_netd_oos_hdlr
===========================================================================*/
/*!
@brief
  Global OOS handler for netd
*/
/*=========================================================================*/
static void nm_mdmprxy_hal_prov_netd_oos_hdlr(void)
{
  nm_mdmprxy_route_prov_common_netd_oos_hdlr();
}

/*===========================================================================
  FUNCTION  nm_mdmprxy_hal_prov_netd_is_hdlr
===========================================================================*/
/*!
@brief
  Global IS handler for netd
*/
/*=========================================================================*/
static uint32_t nm_mdmprxy_hal_prov_netd_is_hdlr(void)
{
#ifdef FEATURE_RECOVERY_ENABLED
  int rc = NETMGR_SUCCESS;
  file_info_t recovery_data;

  memset(&recovery_data, 0, sizeof(file_info_t));

  /* When netd restarts all the previously installed routing rules and
     custom OEM networks will be flushed. We will need to invalidate
     cached network settings information for iwlan in the recovery file */
  recovery_data.is_valid = TRUE;
  recovery_data.info_type = TYPE_IWLAN_NW_SETTINGS;

  rc = netmgr_recovery_file_find_and_invalidate_entry(&recovery_data);
  if (NETMGR_SUCCESS != rc)
  {
    /* If there was no cached information available or lookup failed for
       some reason then we will assume that no OEM network was created
       for iwlan calls and proceed accordingly. The error from
       recovery module will be ignored but we will print it for logging
       purpose */
    netmgr_log_err("%s(): failed to find and invalidate iwlan nw settings"
                   " in recovery file", __func__);
    memset(&nw_info, 0, sizeof(struct networkSettings));
  }
#endif /* FEATURE_RECOVERY_ENABLED */

  /* Reset global flag */
  hal_network_created = FALSE;

  /* Get new packet mark from netd */
  if (NETMGR_SUCCESS != nm_mdmprxy_hal_prov_register_network())
  {
    netmgr_log_err("%s(): failed to create network!", __func__);
    return NM_MDMPRXY_INVALID_PACKET_MARK;
  }

#ifdef FEATURE_RECOVERY_ENABLED
  /* Save the network handle and packet mark for link-local module
     in recovery file. */
  memset(&recovery_data, 0, sizeof(recovery_data));
  recovery_data.is_valid = TRUE;
  recovery_data.info_type = TYPE_IWLAN_NW_SETTINGS;
  recovery_data.recovery_info.nw_settings.network_hndl = nw_info.networkHandle;
  recovery_data.recovery_info.nw_settings.packet_mark = nw_info.packetMark;

  rc = netmgr_recovery_file_write_entry(&recovery_data);
  if (NETMGR_SUCCESS != rc)
  {
    netmgr_log_err("%s(): failed to save iwlan network settings "
                   "in recovery file", __func__);
  }
#endif /* FEATURE_RECOVERY_ENABLED */

  if (NETMGR_SUCCESS != nm_mdmprxy_route_prov_common_init(nw_info.packetMark))
  {
    netmgr_log_err("%s(): failed to initialize route provider!", __func__);
    return NM_MDMPRXY_INVALID_PACKET_MARK;
  }

  return nw_info.packetMark;
}

/*===========================================================================
  FUNCTION  nm_mdmprxy_hal_prov_netd_link_is_hdlr
===========================================================================*/
/*!
@brief
  Per link IS handler for netd restart
*/
/*=========================================================================*/
static int nm_mdmprxy_hal_prov_netd_link_is_hdlr(void* arg)
{
  struct netmgr_kif_cb_ifinfo *ifinfo = (struct netmgr_kif_cb_ifinfo *) arg;
  int rc = NETMGR_FAILURE;

  if (!ifinfo)
  {
    netmgr_log_err("%s(): Invalid params", __func__);
    goto bail;
  }

  if (ifinfo->flags & IFF_UP)
  {
    /* Reinstall routing rules based on ip family */
    if (TRUE == NM_MDMPRXY_GET_ROUTING_RULES_INSTALLED_V4(ifinfo->link))
    {
      NM_MDMPRXY_SET_ROUTING_RULES_INSTALLED(ifinfo->link, AF_INET, FALSE);
      rc = nm_mdmprxy_hal_prov_add_route(ifinfo->link,
                                         ifinfo->name,
                                         ifinfo->tech_type,
                                         AF_INET,
                                         NULL,
                                         NULL);
    }

    if (TRUE == NM_MDMPRXY_GET_ROUTING_RULES_INSTALLED_V6(ifinfo->link))
    {
      NM_MDMPRXY_SET_ROUTING_RULES_INSTALLED(ifinfo->link, AF_INET6, FALSE);
      rc = nm_mdmprxy_hal_prov_add_route(ifinfo->link,
                                         ifinfo->name,
                                         ifinfo->tech_type,
                                         AF_INET6,
                                         NULL,
                                         NULL);
    }
  }

  netmgr_log_low("%s(): complete!", __func__);

bail:
  return rc;

  return NETMGR_SUCCESS;
}

/*===========================================================================
  FUNCTION  nm_mdmprxy_hal_prov_link_oos_hdlr
===========================================================================*/
/*!
@brief
  Per link OOS handler
*/
/*=========================================================================*/
static int nm_mdmprxy_hal_prov_link_oos_hdlr(void* arg)
{
  netmgr_kif_cb_link_ssr_info_t *ifinfo =
         (netmgr_kif_cb_link_ssr_info_t *) arg;
  int rc = NETMGR_SUCCESS;

  rc = nm_mdmprxy_route_prov_common_link_oos_hdlr(arg);
  if (NETMGR_SUCCESS != rc)
  {
    goto bail;
  }

  /* In modem SSR scenario where the kernel interface would have gone down
     when modem goes down, we need to ensure that the corresponding route is
     cleaned up as well */
  if ( NETMGR_KIF_IPF_MASK_IPV4 & ifinfo->addr_mask )
  {
    rc = nm_mdmprxy_hal_prov_remove_route(ifinfo->link,
                                          ifinfo->name,
                                          ifinfo->tech_type,
                                          AF_INET,
                                          NULL,
                                          NULL);
    if ( NETMGR_FAILURE == rc )
    {
      netmgr_log_err("%s(): failed remove route for AF_INET",__func__);
    }
  }

  if ( NETMGR_KIF_IPF_MASK_IPV6 & ifinfo->addr_mask )
  {
    rc = nm_mdmprxy_hal_prov_remove_route(ifinfo->link,
                                          ifinfo->name,
                                          ifinfo->tech_type,
                                          AF_INET6,
                                          NULL,
                                          NULL);
    if ( NETMGR_FAILURE == rc)
    {
      netmgr_log_err("%s(): failed remove route for AF_INET6",__func__);
    }
  }

bail:
  netmgr_log_low("%s(): complete", __func__);
  return rc;
}

/*===========================================================================
  FUNCTION  nm_mdmprxy_hal_prov_init
===========================================================================*/
/*!
@brief
  Init function

@note
  - Dependencies
    - Global nm_mdmprxy_common_init
*/
/*=========================================================================*/
static uint32_t nm_mdmprxy_hal_prov_init(void)
{
  uint32_t packet_mark = 0;
#ifdef FEATURE_RECOVERY_ENABLED
  int rc = NETMGR_SUCCESS;
  file_info_t recovery_data;
#endif /* FEATURE_RECOVERY_ENABLED */

  /* Create custom OEM network via HAL */
  if (NETMGR_SUCCESS != nm_mdmprxy_hal_prov_register_network())
  {
    netmgr_log_err("%s(): failed to create network!", __func__);
    return NM_MDMPRXY_INVALID_PACKET_MARK;
  }

  packet_mark = nw_info.packetMark;

#ifdef FEATURE_RECOVERY_ENABLED
  /* Save the network handle and packet mark for link-local module
     in recovery file. */
  memset(&recovery_data, 0, sizeof(recovery_data));
  recovery_data.is_valid = TRUE;
  recovery_data.info_type = TYPE_IWLAN_NW_SETTINGS;
  recovery_data.recovery_info.nw_settings.network_hndl = nw_info.networkHandle;
  recovery_data.recovery_info.nw_settings.packet_mark = packet_mark;

  rc = netmgr_recovery_file_write_entry(&recovery_data);
  if (NETMGR_SUCCESS != rc)
  {
    netmgr_log_err("%s(): failed to save iwlan network settings "
                   "in recovery file", __func__);
  }
#endif /* FEATURE_RECOVERY_ENABLED */

  /* NOTE - if nm_mdmprxy_hal_prov_create_network fails we should not try to continue */
  if (NETMGR_SUCCESS != nm_mdmprxy_route_prov_common_init(packet_mark))
  {
    netmgr_log_err("%s(): failed to initialize route provider!", __func__);
    return NM_MDMPRXY_INVALID_PACKET_MARK;
  }

  return packet_mark;
}

/*===========================================================================
  FUNCTION  nm_mdmprxy_hal_prov_deinit
===========================================================================*/
/*!
@brief
  De-init function
*/
/*=========================================================================*/
static int nm_mdmprxy_hal_prov_deinit(int restart)
{
#if defined(FEATURE_RECOVERY_ENABLED)
  int rc = NETMGR_FAILURE;
  int file_idx = -1;
  uint64_t nw_handle = NM_IWLAN_NW_HANDLE_INVALID;
  file_info_t recovery_data;

  if (restart)
  {
    /* Check if recovery file has information on iwlan network handle.
       If yes we need to use that handle to unregister with netd */
    memset(&recovery_data, 0, sizeof(file_info_t));
    recovery_data.is_valid = TRUE;
    recovery_data.info_type = TYPE_IWLAN_NW_SETTINGS;

    rc = netmgr_recovery_file_find_entry(&recovery_data, &file_idx);
    if (NETMGR_SUCCESS != rc)
    {
      /* If there was no cached information available then we can proceed
         without any side-effects. However, if there was some issue in
         reading the information from the file then we will be unable
         to cleanup the old routing rules which could cause side-effects.
         Additionally the entry in the file may not be invalidated which
         may result in the file slowly running out of space.

         TODO: determine the correct way of handling errors */
      netmgr_log_err("%s(): failed to find iwlan settings in recovery file", __func__);
      memset(&nw_info, 0, sizeof(struct networkSettings));
    }
    else
    {
      memset(&recovery_nw_info, 0, sizeof(struct networkSettings));
      recovery_nw_info.networkHandle = recovery_data.recovery_info.nw_settings.network_hndl;
      recovery_nw_info.packetMark = recovery_data.recovery_info.nw_settings.packet_mark;

      if (NM_IWLAN_NW_HANDLE_INVALID != recovery_nw_info.networkHandle)
      {
        netmgr_log_low("%s(): attempting to cleanup old iwlan "
                       "routing rules", __func__);
        nw_handle = recovery_nw_info.networkHandle;
      }
    }
  }
  else
  {
    if (hal_network_created)
    {
      nw_handle = nw_info.networkHandle;
    }
  }

  /* Destroying the OEM network will cleanup all the routing rules added
     for that network. This is an easy way to flush all the iwlan
     routing rules */
  rc = nm_mdmprxy_hal_prov_unregister_network(nw_handle);
  if (NETMGR_SUCCESS != rc)
  {
    netmgr_log_err("%s(): failed to destroy OEM network "
                   "for iwlan using handle [0x%" PRIx64 "]!",
                   __func__, nw_handle);
  }

  if (restart)
  {
    /* After we have destroyed the OEM network we need to invalidate
       the entry in the recovery file */
    if (NM_IWLAN_NW_HANDLE_INVALID != recovery_nw_info.networkHandle)
    {
      recovery_data.is_valid = FALSE;
      rc = netmgr_recovery_file_write_entry(&recovery_data);
      if (NETMGR_SUCCESS != rc)
      {
        netmgr_log_err("%s(): failed to invalidate old iwlan network settings!", __func__);
      }

      memset(&nw_info, 0, sizeof(struct networkSettings));
      memset(&recovery_nw_info, 0, sizeof(struct networkSettings));
    }
  }

  hal_network_created = FALSE;

  /* Cleanup iptables rules for route provider */
  if (NETMGR_SUCCESS != nm_mdmprxy_route_prov_common_deinit())
  {
    netmgr_log_err("%s(): failed to cleanup iptables rules for route provider!", __func__);
  }

  return NETMGR_SUCCESS;
#else
  (void) restart;
  return NETMGR_SUCCESS;
#endif /* FEATURE_RECOVERY_ENABLED */
}

/*===========================================================================
                           GLOBAL DEFINITIONS
===========================================================================*/
/*===========================================================================
  FUNCTION  nm_mdmprxy_hal_prov_enable_forwarding
===========================================================================*/
/*!
@brief
  Enable IP forwarding by calling hal api. Netd will always
  enable both IPv6 and IPv4 forwarding at the same time.

@return
  int - NETMGR_SUCCESS on successful operation, NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int nm_mdmprxy_hal_prov_enable_forwarding(int is_netd_restart)
{
  unsigned int active_calls = 0;
  int rc = NETMGR_SUCCESS;

  NETMGR_LOG_FUNC_ENTRY;

  /* Count the total active iwlan calls, enable forwarding
   * only if the count is one. We will enable
   * forwarding when the first iwlan call comes up */
  active_calls = iiface_refcount_v4 + iiface_refcount_v6;

  if (!is_netd_restart && active_calls != 1)
  {
    netmgr_log_med("%s(): forwarding already enabled!", __func__);
    goto ret;
  }

  rc = NetmgrNetdClientSetIpForwardEnable(TRUE);

ret:
  if (NETMGR_SUCCESS == rc)
  {
    netmgr_log_med("%s(): complete - is_netd_restart = %d\n",
                   __func__, is_netd_restart);
  }

  NETMGR_LOG_FUNC_EXIT;
  return rc;
}

/*===========================================================================
  FUNCTION  nm_mdmprxy_hal_prov_disable_forwarding
===========================================================================*/
/*!
@brief
  Disable IP forwarding

@return
  int - NETMGR_SUCCESS on successful operation, NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int nm_mdmprxy_hal_prov_disable_forwarding(void)
{
  unsigned int active_calls = 0;
  int rc = NETMGR_SUCCESS;

  NETMGR_LOG_FUNC_ENTRY;

  /* Count the total active iwlan calls, disable forwarding
   * only if the count is zero. We will disable
   * forwarding when the last iwlan call goes down */
  active_calls = iiface_refcount_v4 + iiface_refcount_v6;

  if (active_calls != 0)
  {
    netmgr_log_med("%s(): iwlan calls still active,"
                   " cannot disable forwarding!", __func__);
    goto ret;
  }

  rc = NetmgrNetdClientSetIpForwardEnable(FALSE);

  if (NETMGR_SUCCESS == rc)
  {
    netmgr_log_med("%s(): complete\n", __func__);
  }

ret:
  NETMGR_LOG_FUNC_EXIT;
  return rc;
}
