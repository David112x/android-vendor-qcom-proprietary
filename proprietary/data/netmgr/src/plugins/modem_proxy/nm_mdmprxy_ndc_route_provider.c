/******************************************************************************

      N M _ M D M P R X Y _ N D C _ R O U T E _ P R O V I D E R . C

******************************************************************************/

/******************************************************************************

  @file    nm_mdmproxy_ndc_route_provider.c
  @brief   Modem proxy plugin

  DESCRIPTION
  Modem proxy module is responsible for managing routing rules and packet
  marking rules for forwarding packets to modem.

******************************************************************************/
/*===========================================================================

  Copyright (c) 2015-2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

/*===========================================================================
                              INCLUDE FILES
===========================================================================*/

#include <sys/socket.h> /* Needs to be removed */
#include <linux/if.h>
#include <string.h>

#include "netmgr.h"
#include "netmgr_kif.h"

#include "nm_mdmprxy.h"
#include "nm_mdmprxy_route_provider_factory.h"
#include "nm_mdmprxy_route_provider_common.h"
#include "nm_mdmprxy_common.h"

/*===========================================================================
                     LOCAL DEFINITIONS AND DECLARATIONS
===========================================================================*/
#define NM_MDMPRXY_CUSTOM_OEM_NETWORK  "oem9"
static boolean oem_network_created = FALSE;

static uint32_t nm_mdmprxy_ndc_prov_init(void);

static int nm_mdmprxy_ndc_prov_deinit(int restart);

static int nm_mdmprxy_ndc_prov_enable_forwarding(int is_netd_restart);

static int nm_mdmprxy_ndc_prov_disable_forwarding(void);

static int nm_mdmprxy_ndc_prov_link_oos_hdlr(void*);

static void nm_mdmprxy_ndc_prov_netd_oos_hdlr(void);

static uint32_t nm_mdmprxy_ndc_prov_netd_is_hdlr(void);

static int nm_mdmprxy_ndc_prov_netd_link_is_hdlr(void*);

static int nm_mdmprxy_ndc_prov_create_network(void);

#ifdef FEATURE_RECOVERY_ENABLED
static int nm_mdmprxy_ndc_prov_destroy_network(void);
#endif /* FEATURE_RECOVERY_ENABLED */

static int nm_mdmprxy_ndc_prov_add_route
(
  int                            link,
  char*                          link_name,
  int                            tech_type,
  sa_family_t                    af,
  ipsec_tunnel_info_t            *tunnel_info,
  struct netmgr_kif_addrstorage  *link_addr
);

static int nm_mdmprxy_ndc_prov_remove_route
(
  int                            link,
  char*                          link_name,
  int                            tech_type,
  sa_family_t                    af,
  ipsec_tunnel_info_t            *tunnel_info,
  struct netmgr_kif_addrstorage  *link_addr
);

nm_mdmprxy_route_prov_t nm_mdmprxy_ndc_route_provider =
{
  /* Initialize the function table for
   * the ndc route provider */
  .add_route               = nm_mdmprxy_ndc_prov_add_route,
  .remove_route            = nm_mdmprxy_ndc_prov_remove_route,
  .install_rule            = nm_mdmprxy_route_prov_common_install_iiface_rules,
  .remove_rule             = nm_mdmprxy_route_prov_common_remove_iiface_rules,
  .enable_forwarding       = nm_mdmprxy_ndc_prov_enable_forwarding,
  .disable_forwarding      = nm_mdmprxy_ndc_prov_disable_forwarding,
  .init                    = nm_mdmprxy_ndc_prov_init,
  .deinit                  = nm_mdmprxy_ndc_prov_deinit,
  .is_hdlr                 = nm_mdmprxy_route_prov_common_is_hdlr,
  .oos_hdlr                = nm_mdmprxy_route_prov_common_oos_hdlr,
  .is_hdlr_per_link        = nm_mdmprxy_route_prov_common_link_is_hdlr,
  .oos_hdlr_per_link       = nm_mdmprxy_ndc_prov_link_oos_hdlr,
  .netd_oos_hdlr           = nm_mdmprxy_ndc_prov_netd_oos_hdlr,
  .netd_is_hdlr            = nm_mdmprxy_ndc_prov_netd_is_hdlr,
  .netd_is_hdlr_per_link   = nm_mdmprxy_ndc_prov_netd_link_is_hdlr
};


/*---------------------------------------------------------------------------
                            ROUTING FUNCTIONS
---------------------------------------------------------------------------*/

/*===========================================================================
  FUNCTION  nm_mdmprxy_ndc_prov_exec_cmd
===========================================================================*/
/*!
@brief
  Helper function to execute ndc commands

@return
  NETMGR_SUCCESS/NETMGR_FAILURE
*/
/*=========================================================================*/
static int nm_mdmprxy_ndc_prov_exec_cmd(char* cmd)
{
  int rc = NETMGR_FAILURE;
  char err_str[NETMGR_MAX_COMMAND_LENGTH] = "";
  char *saveptr = NULL;
  char *ret_str = NULL;

  NETMGR_LOG_FUNC_ENTRY;

  /* ndc commands log everything to stdout even when there is an error
     We need to pass in a valid stdout buffer for the return message
     to be captured so that we can parse the output */
  DS_SYSCALL_DEFAULTS(opts);
  opts.stdout = err_str;
  opts.stdout_length = sizeof(err_str);
  opts.log_options |= DS_EXEC_LOG_EXECTIME;

  if (!cmd)
  {
    netmgr_log_err("%s(): Invalid cmd!\n", __func__);
    goto bail;
  }

  rc = netmgr_ds_exec(cmd, &opts);

  if (DS_EXEC_OK != rc)
  {
    netmgr_log_err("%s(): cmd: %s failed!", __func__, cmd);
    rc = NETMGR_FAILURE;
    goto bail;
  }

  if (strlen(err_str) != 0)
  {
    /* ndc commands print the string 200 0 success on successful
     * execution of the command. We need so search
     * for this keyword to check if the command succeeded */
    ret_str = strtok_r(err_str, " ", &saveptr);

    while (ret_str)
    {
      if (strcmp(ret_str, "200")
            && NULL == strcasestr(ret_str, "200"))
      {
        /* Continue searching till the end of the error string is reached */
        ret_str = strtok_r(NULL, " ", &saveptr);
      }
      else
      {
        rc = NETMGR_SUCCESS;
        break;
      }
    }
  }

  if (NETMGR_SUCCESS != rc)
  {
    netmgr_log_err("%s(): cmd: %s failed! Err: %s", __func__, cmd, err_str);
  }

bail:
  NETMGR_LOG_FUNC_EXIT;
  return rc;
}

/*===========================================================================
  FUNCTION  nm_mdmprxy_ndc_prov_create_network
===========================================================================*/
/*!
@brief
  Creates network to manage custom routing rules

@return
  NETMGR_SUCCESS/NETMGR_FAILURE
*/
/*=========================================================================*/
static int nm_mdmprxy_ndc_prov_create_network(void)
{
  char ndc_create_cmd[NETMGR_MAX_COMMAND_LENGTH] = "";
  int rc = NETMGR_FAILURE;

  NETMGR_LOG_FUNC_ENTRY;

  if (!oem_network_created)
  {
    snprintf(ndc_create_cmd,
             sizeof(ndc_create_cmd),
             "ndc network create %s",
             NM_MDMPRXY_CUSTOM_OEM_NETWORK);

    if (NETMGR_SUCCESS != nm_mdmprxy_ndc_prov_exec_cmd(ndc_create_cmd))
    {
      netmgr_log_err("%s(): Failed to create custom network!\n",
                     __func__);
      goto bail;
    }

    netmgr_log_med("%s(): Successfully created new custom network!\n",
                    __func__);
    oem_network_created = TRUE;
  }

  rc = NETMGR_SUCCESS;

bail:
  NETMGR_LOG_FUNC_EXIT;
  return rc;
}

#ifdef FEATURE_RECOVERY_ENABLED
/*===========================================================================
  FUNCTION  nm_mdmprxy_ndc_prov_destroy_network
===========================================================================*/
/*!
@brief
  Destroys the custom OEM network

@return
  NETMGR_SUCCESS/NETMGR_FAILURE
*/
/*=========================================================================*/
static int nm_mdmprxy_ndc_prov_destroy_network(void)
{
  char ndc_destroy_cmd[NETMGR_MAX_COMMAND_LENGTH] = "";
  int rc = NETMGR_FAILURE;

  NETMGR_LOG_FUNC_ENTRY;

  if (oem_network_created)
  {
    snprintf(ndc_destroy_cmd,
             sizeof(ndc_destroy_cmd),
             "ndc network destroy %s",
             NM_MDMPRXY_CUSTOM_OEM_NETWORK);

    if (NETMGR_SUCCESS != nm_mdmprxy_ndc_prov_exec_cmd(ndc_destroy_cmd))
    {
      netmgr_log_err("%s(): Failed to destroy OEM network!\n",
                     __func__);
      goto bail;
    }

    netmgr_log_med("%s(): Successfully destroyed OEM network!\n",
                    __func__);
    oem_network_created = FALSE;
  }

  rc = NETMGR_SUCCESS;

bail:
  NETMGR_LOG_FUNC_EXIT;
  return rc;
}
#endif /* FEATURE_RECOVERY_ENABLED */

/*===========================================================================
  FUNCTION  nm_mdmprxy_ndc_prov_add_route
===========================================================================*/
/*!
@brief
  Setup routing rules for the provided link

@return
  NETMGR_SUCCESS/NETMGR_FAILURE
*/
/*=========================================================================*/
static int nm_mdmprxy_ndc_prov_add_route
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
  char cmd[NETMGR_MAX_COMMAND_LENGTH] = "";

  NETMGR_LOG_FUNC_ENTRY;

  NM_MDMPRXY_VAR_UNUSED(tech_type);
  NM_MDMPRXY_VAR_UNUSED(tunnel_info);
  NM_MDMPRXY_VAR_UNUSED(link_addr);

  if (!link_name)
  {
    netmgr_log_err("%s(): link_name cannot be NULL!", __func__);
    goto bail;
  }

  /* Currently ndc interface does not provide an option to enable routing rules per
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
     attempt to create the oem10 network might have failed. Attempt to create
     the network again here */
  if (NETMGR_SUCCESS != nm_mdmprxy_ndc_prov_create_network())
  {
    netmgr_log_err("%s(): Failed to create custom network!", __func__);
    goto bail;
  }

  /* Add link to custom network */
  snprintf(cmd,
           sizeof(cmd),
           "ndc network interface add %s %s",
           NM_MDMPRXY_CUSTOM_OEM_NETWORK,
           link_name);

  if (NETMGR_SUCCESS !=
        nm_mdmprxy_ndc_prov_exec_cmd(cmd))
  {
    netmgr_log_err("%s(): Failed to add interface [%s] to network!",
                   __func__, link_name);
    goto bail;
  }

  /* Add routing rule to send packets from the provided link over wlan0 */
  snprintf(cmd,
           sizeof(cmd),
           "ndc ipfwd add %s wlan0",
           link_name);

  if (NETMGR_SUCCESS !=
        nm_mdmprxy_ndc_prov_exec_cmd(cmd))
  {
    netmgr_log_err("%s(): Failed to add forwarding rule from link [%s] to wlan0!",
                   __func__, link_name);
    goto bail;
  }

  /* Add a default V4 route link's table via dummy gateway. The cloned packets
   * will be forwarded to modem over this route */
  snprintf(cmd,
           sizeof(cmd),
           "ndc network route add %s %s %s/32",
           NM_MDMPRXY_CUSTOM_OEM_NETWORK,
           link_name,
           NM_MDMPRXY_GATEWAY_V4_ADDR_STR);

  if (NETMGR_SUCCESS !=
        nm_mdmprxy_ndc_prov_exec_cmd(cmd))
  {
    netmgr_log_err("%s(): Failed to add default V4 gateway route for link [%s]!",
                   __func__, link_name);
    goto bail;
  }

  /* Add routing rule to send packets from the provided link over dummy0 */
  snprintf(cmd,
           sizeof(cmd),
           "ndc ipfwd add %s dummy0",
           link_name);

  if (NETMGR_SUCCESS !=
        nm_mdmprxy_ndc_prov_exec_cmd(cmd))
  {
    netmgr_log_err("%s(): Failed to add forwarding rule from link [%s] to dummy0!",
                   __func__, link_name);
    goto bail;
  }

  /* Add a default V6 route link's table via dummy gateway. The cloned packets
   * will be forwarded to modem over this route */
  snprintf(cmd,
           sizeof(cmd),
           "ndc network route add %s %s %s/128",
           NM_MDMPRXY_CUSTOM_OEM_NETWORK,
           link_name,
           NM_MDMPRXY_GATEWAY_V6_ADDR);

  if (NETMGR_SUCCESS !=
        nm_mdmprxy_ndc_prov_exec_cmd(cmd))
  {
    netmgr_log_err("%s(): Failed to add default V6 gateway route for link [%s]!",
                   __func__, link_name);
    goto bail;
  }

  rc = NETMGR_SUCCESS;

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
  FUNCTION  nm_mdmprxy_ndc_prov_remove_route
===========================================================================*/
/*!
@brief
  Remove routing rules for the provided link

@return
  NETMGR_SUCCESS/NETMGR_FAILURE
*/
/*=========================================================================*/
static int nm_mdmprxy_ndc_prov_remove_route
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
  char ndc_remove_cmd[NETMGR_MAX_COMMAND_LENGTH] = "";

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
    rc = NETMGR_FAILURE;
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

  snprintf(ndc_remove_cmd,
           sizeof(ndc_remove_cmd),
           "ndc network interface remove %s %s",
           NM_MDMPRXY_CUSTOM_OEM_NETWORK,
           link_name);

  if (NETMGR_SUCCESS != nm_mdmprxy_ndc_prov_exec_cmd(ndc_remove_cmd))
  {
    netmgr_log_err("%s(): failed to remove link [%s] from custom network",
                   __func__, link_name);
    goto bail;
  }

  rc = NETMGR_SUCCESS;

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
  FUNCTION  nm_mdmprxy_ndc_prov_netd_oos_hdlr
===========================================================================*/
/*!
@brief
  Global OOS handler for netd
*/
/*=========================================================================*/
static void nm_mdmprxy_ndc_prov_netd_oos_hdlr(void)
{
  nm_mdmprxy_route_prov_common_netd_oos_hdlr();
}

/*===========================================================================
  FUNCTION  nm_mdmprxy_ndc_prov_netd_is_hdlr
===========================================================================*/
/*!
@brief
  Global IS handler for netd
*/
/*=========================================================================*/
static uint32_t nm_mdmprxy_ndc_prov_netd_is_hdlr(void)
{
  int rc = NETMGR_SUCCESS;

  /* Reset global flag */
  oem_network_created = FALSE;

  rc = nm_mdmprxy_ndc_prov_create_network();
  if (NETMGR_SUCCESS != rc)
  {
    return (uint32_t) NM_MDMPRXY_INVALID_PACKET_MARK;
  }

  return (uint32_t) NM_MDMPRXY_MARK;
}

/*===========================================================================
  FUNCTION  nm_mdmprxy_ndc_prov_netd_link_is_hdlr
===========================================================================*/
/*!
@brief
  Per link IS handler for netd restart
*/
/*=========================================================================*/
static int nm_mdmprxy_ndc_prov_netd_link_is_hdlr(void* arg)
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
      rc = nm_mdmprxy_ndc_prov_add_route(ifinfo->link,
                                         ifinfo->name,
                                         ifinfo->tech_type,
                                         AF_INET,
                                         NULL,
                                         NULL);
    }

    if (TRUE == NM_MDMPRXY_GET_ROUTING_RULES_INSTALLED_V6(ifinfo->link))
    {
      NM_MDMPRXY_SET_ROUTING_RULES_INSTALLED(ifinfo->link, AF_INET6, FALSE);
      rc = nm_mdmprxy_ndc_prov_add_route(ifinfo->link,
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
}

/*===========================================================================
  FUNCTION  nm_mdmprxy_ndc_prov_link_oos_hdlr
===========================================================================*/
/*!
@brief
  Per link OOS handler
*/
/*=========================================================================*/
static int nm_mdmprxy_ndc_prov_link_oos_hdlr(void* arg)
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
     when modem goes down, we need to ensure that the corresponding route is cleaned
     up as well */
  if ( NETMGR_KIF_IPF_MASK_IPV4 & ifinfo->addr_mask )
  {
    rc = nm_mdmprxy_ndc_prov_remove_route(ifinfo->link,
                                          ifinfo->name,
                                          ifinfo->tech_type,
                                          AF_INET,
                                          NULL,
                                          NULL);
    if ( NETMGR_FAILURE == rc )
    {
      netmgr_log_err("%s(): failed remove ndc route for AF_INET",__func__);
    }
  }

  if ( NETMGR_KIF_IPF_MASK_IPV6 & ifinfo->addr_mask )
  {
    rc = nm_mdmprxy_ndc_prov_remove_route(ifinfo->link,
                                          ifinfo->name,
                                          ifinfo->tech_type,
                                          AF_INET6,
                                          NULL,
                                          NULL);
    if ( NETMGR_FAILURE == rc)
    {
      netmgr_log_err("%s(): failed remove ndc route for AF_INET6",__func__);
    }
  }

bail:
  netmgr_log_low("%s(): complete", __func__);
  return rc;
}


/*===========================================================================
  FUNCTION  nm_mdmprxy_ndc_prov_init
===========================================================================*/
/*!
@brief
  Init function

@note
  - Dependencies
    - Global nm_mdmprxy_common_init
*/
/*=========================================================================*/
static uint32_t nm_mdmprxy_ndc_prov_init(void)
{
  nm_mdmprxy_route_prov_common_init((uint32_t) NM_MDMPRXY_MARK);

  /* Create custom oem network */
  if (NETMGR_SUCCESS
       != nm_mdmprxy_ndc_prov_create_network())
  {
    netmgr_log_err("%s(): failed!", __func__);
  }

  return (uint32_t) NM_MDMPRXY_MARK;
}

/*===========================================================================
  FUNCTION  nm_mdmprxy_ndc_prov_deinit
===========================================================================*/
/*!
@brief
  De-init function
*/
/*=========================================================================*/
static int nm_mdmprxy_ndc_prov_deinit(int restart)
{
#if defined(FEATURE_RECOVERY_ENABLED)
  if (restart)
  {
    netmgr_log_high("%s(): netmgr has been restarted, force cleanup", __func__);
    oem_network_created = TRUE;
  }

  /* Destroy custom oem network */
  if (NETMGR_SUCCESS != nm_mdmprxy_ndc_prov_destroy_network())
  {
    netmgr_log_err("%s(): failed to destroy OEM network!", __func__);
  }

  /* Cleanup iptables rules for route provider */
  if (NETMGR_SUCCESS != nm_mdmprxy_route_prov_common_deinit())
  {
    netmgr_log_err("%s(): failed to cleanup iptables rules for route provider!",
                   __func__);
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
  FUNCTION  nm_mdmprxy_ndc_prov_enable_forwarding
===========================================================================*/
/*!
@brief
  Enable IP forwarding by calling 'ndc ipfwd enable iwlan'. Netd will always
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
int nm_mdmprxy_ndc_prov_enable_forwarding(int is_netd_restart)
{
  unsigned int active_calls = 0;
  int rc = NETMGR_SUCCESS;
  const char *cmd = "ndc ipfwd enable iwlan";

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

  DS_SYSCALL_DEFAULTS(opts);
  opts.log_options |= DS_EXEC_LOG_CMDSTR;
  opts.log_options |= DS_EXEC_LOG_EXECTIME;

  if (DS_EXEC_OK != netmgr_ds_exec(cmd, &opts))
  {
    netmgr_log_err("%s(): cmd: %s failed!", __func__, cmd);
    rc = NETMGR_FAILURE;
  }

ret:
  if (NETMGR_SUCCESS == rc)
  {
    /* The 'ndc' command will be setting both forwarding flags at the same time */
    netmgr_log_med("%s(): complete - is_netd_restart = %d\n",
                   __func__, is_netd_restart);
  }

  NETMGR_LOG_FUNC_EXIT;
  return rc;
}

/*===========================================================================
  FUNCTION  nm_mdmprxy_ndc_prov_disable_forwarding
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
int nm_mdmprxy_ndc_prov_disable_forwarding(void)
{
  unsigned int active_calls = 0;
  int rc = NETMGR_SUCCESS;
  const char *cmd = "ndc ipfwd disable iwlan";

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

  DS_SYSCALL_DEFAULTS(opts);
  opts.log_options |= DS_EXEC_LOG_CMDSTR;
  opts.log_options |= DS_EXEC_LOG_EXECTIME;

  if (DS_EXEC_OK != netmgr_ds_exec(cmd, &opts))
  {
    netmgr_log_err("%s(): cmd: %s failed!", __func__, cmd);
    rc = NETMGR_FAILURE;
  }

  if (NETMGR_SUCCESS == rc)
  {
    /* The 'ndc' command will be clearing both forwarding flags at the same time */
    netmgr_log_med("%s(): complete\n", __func__);
  }

ret:
  NETMGR_LOG_FUNC_EXIT;
  return rc;
}
