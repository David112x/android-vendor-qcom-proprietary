/******************************************************************************

                 N M _ M D M _ P R X Y _ M A I N . C

******************************************************************************/

/******************************************************************************

  @file    nm_mdmprxy_main.c
  @brief   Modem proxy plugin

  DESCRIPTION
  Modem proxy plugin module is responsible for managing routing rules
  and packet marking rules for forwarding packets to modem.

******************************************************************************/
/*===========================================================================

  Copyright (c) 2015-2016, 2018-2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

/*===========================================================================
                              INCLUDE FILES
===========================================================================*/
#include <pthread.h>

/* Socket includes */
#include <sys/socket.h>
#include <linux/udp.h>
#ifndef NETMGR_OFFTARGET
#include <netinet/in6.h>
#endif

/* Netmgr core includes */
#include "ds_util.h"
#include "netmgr.h"
#include "netmgr_util.h"

#include "netmgr_config.h"

/* Netmgr callback framework includes */
#include "netmgr_cb.h"
#include "netmgr_main_cb.h"
#include "netmgr_kif_cb.h"
#include "netmgr_qmi_cb.h"

/* Local mdmprxy includes */
#include "nm_mdmprxy.h"
#include "nm_mdmprxy_route_provider_factory.h"
#include "nm_mdmprxy_mark_provider_factory.h"
#include "nm_mdmprxy_iptables_helper.h"
#include "nm_mdmprxy_common.h"

#include <arpa/inet.h>

/*===========================================================================
                     LOCAL DEFINITIONS AND DECLARATIONS
===========================================================================*/

#define NM_MDMPRXY_CB_MODNAME "MODEM_PROXY"

/* Since our vars are global scope, they need to be protected. CB callbacks
 * are not guaranteed to be serialized */
static pthread_mutex_t nm_mdmprxy_lock = PTHREAD_MUTEX_INITIALIZER;

#define nm_mdmprxy_lock() \
  pthread_mutex_lock(&nm_mdmprxy_lock); \
  netmgr_log_low("%s(): locked", __func__);

#define nm_mdmprxy_unlock() \
  pthread_mutex_unlock(&nm_mdmprxy_lock); \
  netmgr_log_low("%s(): unlocked", __func__);

#define do_vtable(X) \
  if (X) X

#define do_vtable_ret(X,Y) \
  if (X) Y = X

/* Global route provider and mark provider instances */
static nm_mdmprxy_route_prov_t *route_prov_inst;
static nm_mdmprxy_mark_prov_t  *mark_prov_inst;

static int nm_mdmprxy_inited = FALSE;

/*===========================================================================
                     CB hooks
===========================================================================*/

/*===========================================================================
  FUNCTION  nm_mdmprxy_init_lock
===========================================================================*/
/*!
@brief
  Initialize mutex lock

@return
  NETMGR_SUCCESS if mutex is initialized correctly
  NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
static int nm_mdmprxy_init_lock(void)
{
  int ret = 0;
  pthread_mutexattr_t nm_mdmprxy_attr;

  ret = pthread_mutexattr_init(&nm_mdmprxy_attr);
  if (0 != ret)
  {
    netmgr_log_err("%s(): failed to initialize mutex attribute! Err: %d", __func__, ret);
    return NETMGR_FAILURE;
  }

  ret = pthread_mutexattr_settype(&nm_mdmprxy_attr, PTHREAD_MUTEX_RECURSIVE);
  if (0 != ret)
  {
    netmgr_log_err("%s(): failed to set mutex attribute! Err: %d", __func__, ret);
    return NETMGR_FAILURE;
  }

  ret = pthread_mutex_init(&nm_mdmprxy_lock, &nm_mdmprxy_attr);
  if (0 != ret)
  {
    netmgr_log_err("%s(): failed to initialize pthread object! Err: %d", __func__, ret);
    return NETMGR_FAILURE;
  }

  ret = pthread_mutexattr_destroy(&nm_mdmprxy_attr);
  if (0 != ret)
  {
    netmgr_log_err("%s(): failed to detroy mutex attribute! Err: %d", __func__, ret);
    return NETMGR_FAILURE;
  }

  return NETMGR_SUCCESS;
}

/*===========================================================================
  FUNCTION  nm_mdmprxy_print_status
===========================================================================*/
/*!
@brief
  Handler for NETMGR_MAIN_CB_SIGUSR1. Prints all open ports

@arg arg - unused

*/
/*=========================================================================*/
static void* nm_mdmprxy_print_status(void *arg)
{
  NM_MDMPRXY_VAR_UNUSED(arg);

  nm_mdmprxy_lock();

  if (mark_prov_inst) {
    do_vtable(mark_prov_inst->sigusr1_hdlr)();
  } else {
    netmgr_log_err("%s(): Route provider not initialized!", __func__);
  }

  if (route_prov_inst) {
    do_vtable(route_prov_inst->sigusr1_hdlr)();
  } else {
    netmgr_log_err("%s(): Route provider not initialized!", __func__);
  }

  nm_mdmprxy_unlock();
  return 0;
}

/*===========================================================================
  FUNCTION  nm_mdmprxy_enable_audio_port_fwd
===========================================================================*/
/*!
@brief
  Handler for NETMGR_KIF_USER_CMD_ENABLE_PORTFWD. Used for installing
  audio-port forwarding rules

@arg arg - struct netmgr_kif_cb_ifinfo

*/
/*=========================================================================*/
static void* nm_mdmprxy_enable_audio_port_fwd(void *arg)
{
  struct netmgr_kif_cb_usercmd_info *usercmd_info = (struct netmgr_kif_cb_usercmd_info *) arg;
  int rc = NETMGR_CB_RESULT_SUCCESS;

  if (!usercmd_info)
    return 0;

  nm_mdmprxy_lock();

  if (!mark_prov_inst)
  {
    netmgr_log_err("%s(): Mark provider not initialized!", __func__);
    nm_mdmprxy_unlock();
    return 0;
  }

  /* Port forwarding rules are not link specific */
  do_vtable_ret(mark_prov_inst->install_portfwd_rule, rc)(usercmd_info->ip_family,
                                                          usercmd_info->preference);

  nm_mdmprxy_unlock();

  /* Populate the return values */
  usercmd_info->ip_family  = usercmd_info->ip_family;
  usercmd_info->preference = usercmd_info->preference;
  usercmd_info->result     = rc;

  return 0;
}

/*===========================================================================
  FUNCTION  nm_mdmprxy_disable_audio_port_fwd
===========================================================================*/
/*!
@brief
  Handler for NETMGR_KIF_USER_CMD_DISABLE_PORTFWD. Used for removing
  audio-port forwarding rules

@arg arg - struct netmgr_kif_cb_ifinfo

*/
/*=========================================================================*/
static void* nm_mdmprxy_disable_audio_port_fwd(void *arg)
{
  struct netmgr_kif_cb_usercmd_info *usercmd_info = (struct netmgr_kif_cb_usercmd_info *) arg;
  int rc = NETMGR_CB_RESULT_SUCCESS;

  if (!usercmd_info)
    return 0;

  nm_mdmprxy_lock();

  if (!mark_prov_inst)
  {
    netmgr_log_err("%s(): Mark provider not initialized!", __func__);
    nm_mdmprxy_unlock();
    return 0;
  }

  do_vtable_ret(mark_prov_inst->remove_portfwd_rule, rc)(usercmd_info->ip_family,
                                                         usercmd_info->preference);

  nm_mdmprxy_unlock();

  /* Populate the return values */
  usercmd_info->ip_family  = usercmd_info->ip_family;
  usercmd_info->preference = usercmd_info->preference;
  usercmd_info->result     = rc;

  return 0;
}

/*===========================================================================
  FUNCTION  nm_mdmprxy_query_audio_port_fwd_status
===========================================================================*/
/*!
@brief
  Handler for NETMGR_KIF_USER_CMD_QUERY_PORTFWD. Used for removing
  audio-port forwarding rules

@arg arg - struct netmgr_kif_cb_ifinfo

*/
/*=========================================================================*/
static void* nm_mdmprxy_query_audio_port_fwd_status(void *arg)
{
  struct netmgr_kif_cb_usercmd_info *usercmd_info = (struct netmgr_kif_cb_usercmd_info *) arg;
  int rc = NETMGR_CB_RESULT_SUCCESS;
  boolean pref = FALSE;

  if (!usercmd_info)
    return 0;

  nm_mdmprxy_lock();

  if (!mark_prov_inst)
  {
    netmgr_log_err("%s(): Mark provider not initialized!", __func__);
    nm_mdmprxy_unlock();
    return 0;
  }

  do_vtable_ret(mark_prov_inst->query_portfwd_status, pref)(usercmd_info->ip_family);

  nm_mdmprxy_unlock();

  /* Populate the return values */
  usercmd_info->ip_family  = usercmd_info->ip_family;
  usercmd_info->preference = pref;
  usercmd_info->result     = rc;

  return 0;
}

/*===========================================================================
  FUNCTION  nm_mdmprxy_netd_is_handler
===========================================================================*/
/*!
@brief
  Handler for NETMGR_KIF_CB_NETD_IS. This handles global netd restart
  event

@arg arg - unused

*/
/*=========================================================================*/
static void* nm_mdmprxy_netd_is_handler(void *arg)
{
  NM_MDMPRXY_VAR_UNUSED(arg);

  uint32_t packet_mark = 0;

  nm_mdmprxy_lock();

  netmgr_log_low("%s(): entry", __func__);

  /* Cleanup rules installed by route provider related to the old mark */
  do_vtable(route_prov_inst->netd_oos_hdlr)();

  /* Un-install mark provider rules associated with old oem mark */
  do_vtable(mark_prov_inst->netd_oos_hdlr)();

  /* Re-init routing state which will query netd for new oem mark */
  do_vtable_ret(route_prov_inst->netd_is_hdlr, packet_mark)();

  if (NM_MDMPRXY_INVALID_PACKET_MARK == packet_mark)
  {
    /* If we get invalid packet mark again during IS handling it probably means
       that netd crashed again or the netmgr could not re-register with netd
       properly. In this scenario there is no point to continue with re-init.
       Any dangling rules from previous init should be flushed with the above
       steps */
    netmgr_log_err("%s(): invalid packet mark received! Route provider was not"
                   " initialized correctly!", __func__);
    nm_mdmprxy_unlock();
    return 0;
  }
  else
  {
    /* Pass the new oem mark to mark provider to re-install rules with new mark */
    do_vtable(mark_prov_inst->netd_is_hdlr)(packet_mark);

    nm_mdmprxy_common_install_jump_rules_netd_restart(AF_INET);
    nm_mdmprxy_common_install_jump_rules_netd_restart(AF_INET6);

    /* Enable forwarding with is_netd_restart param set to TRUE */
    do_vtable(route_prov_inst->enable_forwarding)(TRUE);
  }

  /* If we were unable to initialize modem proxy module at powerup the
     'nm_mdmprxy_inited' flag will not be set. As part of netd restart
     handling we are almost re-initing the route provider and mark
     provider sub-modules again so we can set the flag to TRUE here if
     not set already */
  if (!nm_mdmprxy_inited)
  {
    nm_mdmprxy_inited = TRUE;

    /* Let listeners know about successful initialization of modem proxy module
       This will be sent only when modem proxy did not initialize properly
       during powerup */
    netmgr_cb_runall(NETMGR_QMI_CB_TABLE, NETMGR_QMI_CB_MDMPRXY_INITED, NULL);
  }

  nm_mdmprxy_unlock();

  netmgr_log_low("%s(): complete", __func__);
  return 0;
}

/*===========================================================================
  FUNCTION  nm_mdmprxy_netd_per_link_is_handler
===========================================================================*/
/*!
@brief
  Handler for NETMGR_KIF_CB_LINK_NETD_IS

@arg arg - unused

*/
/*=========================================================================*/
static void* nm_mdmprxy_netd_per_link_is_handler(void *arg)
{
  nm_mdmprxy_lock();

  if (route_prov_inst)
  {
    do_vtable(route_prov_inst->netd_is_hdlr_per_link)(arg);
  }
  else
  {
    netmgr_log_err("%s(): Route provider not initialized!", __func__);
  }

  if (mark_prov_inst)
  {
    do_vtable(mark_prov_inst->netd_is_hdlr_per_link)(arg);
  }
  else
  {
    netmgr_log_err("%s(): Mark provider not initialized!", __func__);
  }

  nm_mdmprxy_unlock();

  return 0;
}

/*===========================================================================
  FUNCTION  nm_mdmprxy_ssr_per_link_oos_handler
===========================================================================*/
/*!
@brief
  Handler for NETMGR_QMI_CB_LINK_OOS.

@arg arg - unused

*/
/*=========================================================================*/
static void* nm_mdmprxy_ssr_per_link_oos_handler(void *arg)
{
  netmgr_kif_cb_link_ssr_info_t *ifinfo =
         (netmgr_kif_cb_link_ssr_info_t *) arg;

  NM_MDMPRXY_NULL_CHECK(ifinfo);

  if (ifinfo->type != NETMGR_KIF_CB_IFTYPE_REVERSE)
  {
    return 0;
  }

  nm_mdmprxy_lock();

  if (route_prov_inst) {
    do_vtable(route_prov_inst->oos_hdlr_per_link)(arg);
  } else {
    netmgr_log_err("%s(): Route provider not initialized!", __func__);
  }

  if (NETMGR_KIF_IPF_MASK_IPV4 & ifinfo->addr_mask)
  {
    /* Cleanup per link tcp v4 mss rule */
    nm_mdmprxy_common_clear_tcp_mss(AF_INET,
                                    ifinfo->tech_type,
                                    ifinfo->name,
                                    &ifinfo->ipv4);
  }

  if ( NETMGR_KIF_IPF_MASK_IPV6 & ifinfo->addr_mask)
  {
    /* Cleanup per link tcp v6 mss rule */
    nm_mdmprxy_common_clear_tcp_mss(AF_INET6,
                                    ifinfo->tech_type,
                                    ifinfo->name,
                                    &ifinfo->ipv6);
  }

  nm_mdmprxy_unlock();

  return 0;
}

/*===========================================================================
  FUNCTION  nm_mdmprxy_ssr_per_link_is_handler
===========================================================================*/
/*!
@brief
  Handler for NETMGR_QMI_CB_LINK_IS.

@arg arg - unused

*/
/*=========================================================================*/
static void* nm_mdmprxy_ssr_per_link_is_handler(void *arg)
{
  nm_mdmprxy_lock();

  if (route_prov_inst) {
    do_vtable(route_prov_inst->is_hdlr_per_link)(arg);
  } else {
    netmgr_log_err("%s(): Route provider not initialized!", __func__);
  }

  nm_mdmprxy_unlock();

  return 0;
}

/*===========================================================================
  FUNCTION  nm_mdmprxy_ssr_oos_handler
===========================================================================*/
/*!
@brief
  Handler for NETMGR_QMI_CB_MODEM_OOS. De-initializes all the
  components of the module

@arg arg - unused

*/
/*=========================================================================*/
static void* nm_mdmprxy_ssr_oos_handler(void *arg)
{
  nm_mdmprxy_lock();

  NM_MDMPRXY_VAR_UNUSED(arg);

  if (!route_prov_inst || !mark_prov_inst)
  {
    netmgr_log_err("%s(): providers are not initialized", __func__);
    nm_mdmprxy_unlock();
    return 0;
  }

  /* Uninitialize components in the order opposite to global IS
     1. Mark provider
     2. Route provider
     3. Common module */

  if (mark_prov_inst) {
    do_vtable(mark_prov_inst->oos_hdlr)(FALSE);
  } else {
    netmgr_log_err("%s(): Mark provider not initialized!", __func__);
  }

  if (route_prov_inst) {
    do_vtable(route_prov_inst->oos_hdlr)();
  } else {
    netmgr_log_err("%s(): Route provider not initialized!", __func__);
  }

  nm_mdmprxy_unlock();

  netmgr_log_med("%s(): OOS handler complete", __func__);
  return 0;
}

/*===========================================================================
  FUNCTION  nm_mdmprxy_ssr_oos_end_handler
===========================================================================*/
/*!
@brief
  Handler for NETMGR_QMI_CB_MODEM_OOS_END. Do clean up that needs to be done
  after all the link clean up is complete.

@arg arg - unused

*/
/*=========================================================================*/
static void* nm_mdmprxy_ssr_oos_complete_handler(void *arg)
{

  netmgr_log_med("%s(): ENTRY", __func__);
  nm_mdmprxy_lock();

  NM_MDMPRXY_VAR_UNUSED(arg);

  nm_mdmprxy_common_oos_hdlr(TRUE);

  nm_mdmprxy_unlock();

  netmgr_log_med("%s(): EXIT", __func__);
  return NULL;
}

/*===========================================================================
  FUNCTION  nm_mdmprxy_ssr_is_handler
===========================================================================*/
/*!
@brief
  Handler for NETMGR_QMI_CB_MODEM_IS. Initializes all the different
  components of the module

@arg arg - unused

*/
/*=========================================================================*/
static void* nm_mdmprxy_ssr_is_handler(void *arg)
{
  nm_mdmprxy_lock();

  NM_MDMPRXY_VAR_UNUSED(arg);

  do_vtable(route_prov_inst->is_hdlr)();
  do_vtable(mark_prov_inst->is_hdlr)();

  nm_mdmprxy_unlock();
  return NULL;
}

/*===========================================================================
  FUNCTION  nm_mdmprxy_ifcfg_pre_handler
===========================================================================*/
/*!
@brief
  Handler for NETMGR_KIF_CB_IFCFG_PRE. Installs call-specific iptables and
  routing rules

@arg arg - struct netmgr_kif_cb_ifinfo

*/
/*=========================================================================*/
static void* nm_mdmprxy_ifcfg_pre_handler(void *arg)
{
  struct netmgr_kif_cb_ifinfo *ifinfo = (struct netmgr_kif_cb_ifinfo *) arg;

  if (!ifinfo)
    return 0;

  if (ifinfo->type != NETMGR_KIF_CB_IFTYPE_REVERSE)
    return 0;

  netmgr_log_low("%s(): Found a reverse iface to do stuff on: %d:%s",
                 __func__, ifinfo->link, ifinfo->name);

  nm_mdmprxy_lock();

  if (!route_prov_inst)
  {
    netmgr_log_err("%s(): route provider is not initialized!", __func__);
    nm_mdmprxy_unlock();
    return 0;
  }

  /* TODO: We MUST do proper error handling for these functions and revert all partially installed
     rules in case of any failure */

  if (ifinfo->addr.type == NETMGR_KIF_CB_ADDR_IPV4)
  {
    /* Install jump rules to common cusom chains from POSTROUTING and RAW
       The component will ensure that the jump rules is installed only once
       per family */
    nm_mdmprxy_common_install_jump_rules(AF_INET);

    /* Install TCP MSS clamping rule */
    nm_mdmprxy_common_set_tcp_mss(AF_INET,
                                  ifinfo->tech_type,
                                  ifinfo->name,
                                  &ifinfo->addr);

    /* Enable ip_forward proc entry so that packets between interfaces */
    do_vtable(route_prov_inst->enable_forwarding)(FALSE);

    /* Install packet forwarding iptables rules */
    do_vtable(route_prov_inst->install_rule)(ifinfo->link,
                                             ifinfo->name,
                                             ifinfo->tech_type,
                                             TRUE,
                                             ifinfo->addr._in4.addr,
                                             FALSE,
                                             NULL,
                                             0,
                                             &ifinfo->tunnel_info);

    /* Install routing rules */
    do_vtable(route_prov_inst->add_route)(ifinfo->link,
                                          ifinfo->name,
                                          ifinfo->tech_type,
                                          AF_INET,
                                          &ifinfo->tunnel_info,
                                          &ifinfo->addr);

    /* Install icmp rule for the given link */
    mark_prov_inst->install_icmp_rule(AF_INET, ifinfo->name);
  }
  else if (ifinfo->addr.type == NETMGR_KIF_CB_ADDR_IPV6)
  {
    /* Install jump rules to common cusom chains from POSTROUTING and RAW
       The component will ensure that the jump rules is installed only once
       per family */
    nm_mdmprxy_common_install_jump_rules(AF_INET6);

    /* Install TCP MSS clamping rule */
    nm_mdmprxy_common_set_tcp_mss(AF_INET6,
                                  ifinfo->tech_type,
                                  ifinfo->name,
                                  &ifinfo->addr);

    /* Enable ip_forward proc entry so that packets between interfaces */
    do_vtable(route_prov_inst->enable_forwarding)(FALSE);

    /* Install packet forwarding iptables rules */
    do_vtable(route_prov_inst->install_rule)(ifinfo->link,
                                             ifinfo->name,
                                             ifinfo->tech_type,
                                             FALSE,
                                             NM_MDMPRXY_V4ADDR_INVALID,
                                             TRUE,
                                             &ifinfo->addr._in6.addr,
                                             ifinfo->addr._in6.mask,
                                             &ifinfo->tunnel_info);

    /* Install routing rules */
    do_vtable(route_prov_inst->add_route)(ifinfo->link,
                                          ifinfo->name,
                                          ifinfo->tech_type,
                                          AF_INET6,
                                          &ifinfo->tunnel_info,
                                          &ifinfo->addr);

    /* Install icmp rule */
    do_vtable(mark_prov_inst->install_icmp_rule)(AF_INET6, ifinfo->name);
  }
  else
  {
    netmgr_log_err("%s(): Unhandled AF; skipping config", __func__);
  }

  nm_mdmprxy_unlock();
  return 0;
}

/*===========================================================================
  FUNCTION  nm_mdmprxy_ifdn_pre_handler
===========================================================================*/
/*!
@brief
  Handler for NETMGR_KIF_CB_IFDN_PRE. Removes call-specific iptables and
  routing rules

@arg arg - struct netmgr_kif_cb_ifinfo

*/
/*=========================================================================*/
static void* nm_mdmprxy_ifdn_pre_handler(void *arg)
{
  struct netmgr_kif_cb_ifinfo *ifinfo = (struct netmgr_kif_cb_ifinfo *) arg;

  if (!ifinfo)
    return 0;

  if (ifinfo->type != NETMGR_KIF_CB_IFTYPE_REVERSE)
    return 0;

  netmgr_log_low("%s(): Found a reverse iface to do stuff on: %d:%s",
                 __func__, ifinfo->link, ifinfo->name);

  nm_mdmprxy_lock();

  if (!route_prov_inst)
  {
    netmgr_log_err("%s(): route provider is not initialized!", __func__);
    nm_mdmprxy_unlock();
    return 0;
  }

  if (ifinfo->addr.type == NETMGR_KIF_CB_ADDR_IPV4)
  {
    /* Remove jump rules to common cusom chains from POSTROUTING and RAW
       The component will ensure that the jump rules is installed only once
       per family */
    nm_mdmprxy_common_uninstall_jump_rules(AF_INET);

    /* Remove TCP MSS clamping rule */
    nm_mdmprxy_common_clear_tcp_mss(AF_INET,
                                    ifinfo->tech_type,
                                    ifinfo->name,
                                    &ifinfo->addr);

    /* Disable ip_forward proc entry so that packets between interfaces */
    do_vtable(route_prov_inst->disable_forwarding)();

    /* Remove routing rules */
    do_vtable(route_prov_inst->remove_route)(ifinfo->link,
                                             ifinfo->name,
                                             ifinfo->tech_type,
                                             AF_INET,
                                             &ifinfo->tunnel_info,
                                             &ifinfo->addr);

    /* Remove packet forwarding iptables rules */
    do_vtable(route_prov_inst->remove_rule)(ifinfo->link,
                                            ifinfo->name,
                                            ifinfo->tech_type,
                                            AF_INET,
                                            &ifinfo->tunnel_info,
                                            &ifinfo->addr);

    /* Remove icmp rule for the given link */
    do_vtable(mark_prov_inst->remove_icmp_rule)(AF_INET, ifinfo->name);
  }
  else if (ifinfo->addr.type == NETMGR_KIF_CB_ADDR_IPV6)
  {
    /* Remove jump rules to common cusom chains from POSTROUTING and RAW
       The component will ensure that the jump rules is installed only once
       per family */
    nm_mdmprxy_common_uninstall_jump_rules(AF_INET6);

    /* Remove TCP MSS clamping rule */
    nm_mdmprxy_common_clear_tcp_mss(AF_INET6,
                                    ifinfo->tech_type,
                                    ifinfo->name,
                                    &ifinfo->addr);

    /* Disable ip_forward proc entry so that packets between interfaces */
    do_vtable(route_prov_inst->disable_forwarding)();

    /* Remove routing rules */
    do_vtable(route_prov_inst->remove_route)(ifinfo->link,
                                             ifinfo->name,
                                             ifinfo->tech_type,
                                             AF_INET6,
                                             &ifinfo->tunnel_info,
                                             &ifinfo->addr);

    /* Remove packet forwarding iptables rules */
    do_vtable(route_prov_inst->remove_rule)(ifinfo->link,
                                            ifinfo->name,
                                            ifinfo->tech_type,
                                            AF_INET6,
                                            &ifinfo->tunnel_info,
                                            &ifinfo->addr);

    /* Remove icmp rule */
    do_vtable(mark_prov_inst->remove_icmp_rule)(AF_INET6, ifinfo->name);
  }
  else
  {
    netmgr_log_err("%s(): Unhandled AF; skipping config", __func__);
  }

  nm_mdmprxy_unlock();
  return 0;
}

/*===========================================================================
  FUNCTION  nm_mdmprxy_rev_ip_dfs_ind_hdlr
===========================================================================*/
/*!
@brief
  Handler for NETMGR_QMI_CB_DFS_REVIP_FILTER_IND. Used for managing
  SPI-based iptables rules

@arg arg - struct netmgr_qmi_cb_ifinfo

*/
/*=========================================================================*/
static void* nm_mdmprxy_rev_ip_dfs_ind_hdlr(void *arg)
{
  struct netmgr_qmi_cb_ifinfo *ifinfo = (struct netmgr_qmi_cb_ifinfo *) arg;

  if (!ifinfo)
  {
    return 0;
  }

  if (ifinfo->type != NETMGR_QMI_CB_IFTYPE_REVERSE)
  {
    return 0;
  }

  netmgr_log_low("%s(): Found a reverse iface to do stuff on: %d",
                 __func__, ifinfo->link);

  nm_mdmprxy_lock();

  if (!mark_prov_inst)
  {
    netmgr_log_err("%s(): Mark provider not initialized!", __func__);
    nm_mdmprxy_unlock();
    return 0;
  }

  switch (ifinfo->filter_op)
  {
    case REV_IP_FILTER_OP_ADD:
      do_vtable(mark_prov_inst->install_spi_rule)(ifinfo->ip_family, ifinfo->spi);
      break;
    case REV_IP_FILTER_OP_REMOVE:
      do_vtable(mark_prov_inst->remove_spi_rule)(ifinfo->ip_family, ifinfo->spi);
      break;
    default:
      netmgr_log_err("%s(): Undefined operation!", __func__);
      break;
  }

  nm_mdmprxy_unlock();
  return 0;
}

/*===========================================================================
  FUNCTION  nm_mdmprxy_install_wlan_rules
===========================================================================*/
/*!
@brief
  Handler for NETMGR_KIF_CB_WLAN_UP. We need to have a separate handler
  for wlan up event for supporting modem only iwlan data calls

@arg arg - struct netmgr_kif_cb_ifinfo

*/
/*=========================================================================*/
static void* nm_mdmprxy_install_wlan_rules(void *arg)
{
  struct netmgr_kif_cb_ifinfo *ifinfo = (struct netmgr_kif_cb_ifinfo *) arg;

  if (!ifinfo)
    return 0;

  if (!route_prov_inst)
    return 0;

  nm_mdmprxy_lock();

  netmgr_log_low("%s(): WLAN interface is UP, installing routing rules", __func__);

  if (ifinfo->addr.type == NETMGR_KIF_CB_ADDR_IPV4
      || ifinfo->addr.type == NETMGR_KIF_CB_ADDR_IPV4V6)
  {
    do_vtable(route_prov_inst->add_route)(ifinfo->link,
                                          ifinfo->name,
                                          ifinfo->tech_type,
                                          AF_INET,
                                          &ifinfo->tunnel_info,
                                          &ifinfo->addr);
  }

  if (ifinfo->addr.type == NETMGR_KIF_CB_ADDR_IPV6
      || ifinfo->addr.type == NETMGR_KIF_CB_ADDR_IPV4V6)
  {
    do_vtable(route_prov_inst->add_route)(ifinfo->link,
                                          ifinfo->name,
                                          ifinfo->tech_type,
                                          AF_INET6,
                                          &ifinfo->tunnel_info,
                                          &ifinfo->addr);
  }

  nm_mdmprxy_unlock();
  return 0;
}

/*===========================================================================
  FUNCTION  nm_mdmprxy_remove_wlan_rules
===========================================================================*/
/*!
@brief
  Handler for NETMGR_KIF_CB_WLAN_DOWN. We need to have a separate handler
  for wlan down event for supporting modem only iwlan data calls

@arg arg - struct netmgr_kif_cb_ifinfo

*/
/*=========================================================================*/
static void* nm_mdmprxy_remove_wlan_rules(void *arg)
{
  struct netmgr_kif_cb_ifinfo *ifinfo = (struct netmgr_kif_cb_ifinfo *) arg;

  if (!ifinfo)
    return 0;

  if (!route_prov_inst)
    return 0;

  nm_mdmprxy_lock();

  netmgr_log_low("%s(): WLAN interface is DOWN, removing routing rules", __func__);

  if (ifinfo->addr.type == NETMGR_KIF_CB_ADDR_IPV4
      || ifinfo->addr.type == NETMGR_KIF_CB_ADDR_IPV4V6)
  {
    do_vtable(route_prov_inst->remove_route)(ifinfo->link,
                                             ifinfo->name,
                                             ifinfo->tech_type,
                                             AF_INET,
                                             &ifinfo->tunnel_info,
                                             &ifinfo->addr);
  }

  if (ifinfo->addr.type == NETMGR_KIF_CB_ADDR_IPV6
      || ifinfo->addr.type == NETMGR_KIF_CB_ADDR_IPV4V6)
  {
    do_vtable(route_prov_inst->remove_route)(ifinfo->link,
                                             ifinfo->name,
                                             ifinfo->tech_type,
                                             AF_INET6,
                                             &ifinfo->tunnel_info,
                                             &ifinfo->addr);
  }

  nm_mdmprxy_unlock();
  return 0;
}

/*===========================================================================
  FUNCTION  nm_mdmprxy_deinit
===========================================================================*/
/*!
@brief
  Cleanup function
*/
/*=========================================================================*/
static void nm_mdmprxy_deinit(int restart)
{
  nm_mdmprxy_lock();

  do_vtable(mark_prov_inst->deinit)(restart);
  do_vtable(route_prov_inst->deinit)(restart);
  nm_mdmprxy_common_deinit(restart);

  /* Disable ip_forward */
  do_vtable(route_prov_inst->disable_forwarding)();

  nm_mdmprxy_inited = FALSE;

  nm_mdmprxy_unlock();

  netmgr_l2s_msg(netmgr_l2s_invalid_link,"de-init completed");
}

/*===========================================================================
  FUNCTION  nm_mdmprxy_is_initialized
===========================================================================*/
int nm_mdmprxy_is_initialized(void)
{
  int init_status = 0;

  nm_mdmprxy_lock();

  init_status = nm_mdmprxy_inited;

  nm_mdmprxy_unlock();

  return init_status;
}

/*===========================================================================
  FUNCTION  nm_mdmprxy_init
===========================================================================*/
/*!
@brief
  Primary module initialization / entry point for modem proxy. Bulk of the
  work is done by the SSR IS handler. Registers with the netmgr CB framework
  for system events we so dearly care about.

*/
/*=========================================================================*/
void nm_mdmprxy_init(int restart)
{
  uint32_t packet_mark = 0;

  if (NETMGR_SUCCESS != nm_mdmprxy_init_lock())
  {
    netmgr_log_err("%s(): failed to initialize mutex lock!", __func__);
    NETMGR_ABORT("failed to initialize mutex lock!");
  }

#if defined(FEATURE_RECOVERY_ENABLED)
  if (restart)
  {
    /* If netmgr was restarted we need to cleanup the routing rules
       destroy the custom OEM network and flush all the iptables rules */
    netmgr_log_err("%s(): netmgr restarted, de-init module and re-int", __func__);
    nm_mdmprxy_deinit(restart);
  }
#else
  (void) restart;
#endif /* FEATURE_RECOVERY_ENABLED */

  nm_mdmprxy_lock();

  /* Initialize the different components in this order
     1. Common module
          - Responsible for creating all the iptables chains used by the module
          - Installs common static rules
          - For now, installs the modem ping related rules as well
     2. Route provider
          - Installs iptables rules to forward packets to the modem (ex. TEE)
          - Installs routing rules to route marked packets to the modem
     3. Mark provider
          - Installs iptables rules to mark packets destined for the modem

     The order of the initialization is critical so that all the rules are setup correctly */

  /* TODO: We MUST do proper error handling for these functions and revert all partially installed
     rules in case of any failure */
  nm_mdmprxy_common_init(NULL);

  do_vtable_ret(route_prov_inst->init, packet_mark)();

  do_vtable(mark_prov_inst->init)(packet_mark);

  if (NM_MDMPRXY_INVALID_PACKET_MARK == packet_mark)
  {
    /* An invalid packet mark can be received if netd was not available at the time of init
       In this scenario there is no point in continuing with initialization since valid mark
       is not available. We will bail oit of the init function.

       The route provider and mark provider initializations can be re-attempted once we
       receive indication about netd start which will happen via the NETMGR_KIF_CB_NETD_IS
       callback

       The common module does not use packet mark for initialization so we need not roll
       back any of the init done via nm_mdmprxy_common. */
    netmgr_log_err("%s(): invalid packet mark received!"
                   " Cannot continue initialization!", __func__);
    nm_mdmprxy_unlock();
    netmgr_l2s_msg(netmgr_l2s_invalid_link, "init failed");
  }
  else
  {
    nm_mdmprxy_unlock();

    nm_mdmprxy_ssr_is_handler(NULL);

    nm_mdmprxy_lock();
    nm_mdmprxy_inited = TRUE;
    nm_mdmprxy_unlock();

    netmgr_l2s_msg(netmgr_l2s_invalid_link,"init completed");
  }
}

/*===========================================================================
  FUNCTION  nm_mdmprxy_register_plugin
===========================================================================*/
/*!
@brief
  Registers callbacks with callback framework
*/
/*=========================================================================*/
void nm_mdmprxy_register_plugin(void)
{
  /* Get the route provider instance */
  route_prov_inst = nm_mdmprxy_get_route_provider();
  if (!route_prov_inst)
  {
    /* TODO: Return proper error code */
    netmgr_log_err("%s(): Failed to initialize route provider", __func__);
    netmgr_l2s_msg(netmgr_l2s_invalid_link,"Failed to initialize route provider for "
                                           "modem proxy module");
    return;
  }

  /* Get mark provider instance */
  mark_prov_inst = nm_mdmprxy_get_mark_provider();
  if (!mark_prov_inst)
  {
    /* TODO: Return proper error code */
    netmgr_log_err("%s(): Failed to initialize marking provider", __func__);
    netmgr_l2s_msg(netmgr_l2s_invalid_link,"Failed to initialize marking provider for "
                                           "modem proxy module");
    return;
  }

  /* Register callbacks with the MAIN table */
  netmgr_cb_insert_cb(NETMGR_MAIN_CB_TABLE,
                      NETMGR_MAIN_CB_SIGUSR1,
                      NM_MDMPRXY_CB_MODNAME,
                      &nm_mdmprxy_print_status);

  /* Register callbacks with the QMI table */
  netmgr_cb_insert_cb(NETMGR_QMI_CB_TABLE,
                      NETMGR_QMI_CB_MODEM_OOS,
                      NM_MDMPRXY_CB_MODNAME,
                      &nm_mdmprxy_ssr_oos_handler);

  netmgr_cb_insert_cb(NETMGR_QMI_CB_TABLE,
                      NETMGR_QMI_CB_MODEM_OOS_COMPLETE,
                      NM_MDMPRXY_CB_MODNAME,
                      &nm_mdmprxy_ssr_oos_complete_handler);

  netmgr_cb_insert_cb(NETMGR_QMI_CB_TABLE,
                      NETMGR_QMI_CB_MODEM_IS,
                      NM_MDMPRXY_CB_MODNAME,
                      &nm_mdmprxy_ssr_is_handler);

  netmgr_cb_insert_cb(NETMGR_KIF_CB_TABLE,
                      NETMGR_KIF_CB_LINK_OOS,
                      NM_MDMPRXY_CB_MODNAME,
                      &nm_mdmprxy_ssr_per_link_oos_handler);

  netmgr_cb_insert_cb(NETMGR_KIF_CB_TABLE,
                      NETMGR_KIF_CB_LINK_IS,
                      NM_MDMPRXY_CB_MODNAME,
                      &nm_mdmprxy_ssr_per_link_is_handler);

  netmgr_cb_insert_cb(NETMGR_QMI_CB_TABLE,
                      NETMGR_QMI_CB_DFS_REVIP_FILTER_IND,
                      NM_MDMPRXY_CB_MODNAME,
                      &nm_mdmprxy_rev_ip_dfs_ind_hdlr);

  /* Register callbacks with the KIF table */
  netmgr_cb_insert_cb(NETMGR_KIF_CB_TABLE,
                      NETMGR_KIF_CB_IFCFG_PRE,
                      NM_MDMPRXY_CB_MODNAME,
                      &nm_mdmprxy_ifcfg_pre_handler);

  netmgr_cb_insert_cb(NETMGR_KIF_CB_TABLE,
                      NETMGR_KIF_CB_IFDN_NOSTATE_PRE,
                      NM_MDMPRXY_CB_MODNAME,
                      &nm_mdmprxy_ifdn_pre_handler);

  netmgr_cb_insert_cb(NETMGR_KIF_CB_TABLE,
                      NETMGR_KIF_USER_CMD_ENABLE_PORTFWD,
                      NM_MDMPRXY_CB_MODNAME,
                      &nm_mdmprxy_enable_audio_port_fwd);

  netmgr_cb_insert_cb(NETMGR_KIF_CB_TABLE,
                      NETMGR_KIF_USER_CMD_DISABLE_PORTFWD,
                      NM_MDMPRXY_CB_MODNAME,
                      &nm_mdmprxy_disable_audio_port_fwd);

  netmgr_cb_insert_cb(NETMGR_KIF_CB_TABLE,
                      NETMGR_KIF_USER_CMD_QUERY_PORTFWD,
                      NM_MDMPRXY_CB_MODNAME,
                      &nm_mdmprxy_query_audio_port_fwd_status);

  netmgr_cb_insert_cb(NETMGR_KIF_CB_TABLE,
                      NETMGR_KIF_CB_WLAN_UP,
                      NM_MDMPRXY_CB_MODNAME,
                      &nm_mdmprxy_install_wlan_rules);

  netmgr_cb_insert_cb(NETMGR_KIF_CB_TABLE,
                      NETMGR_KIF_CB_WLAN_DOWN,
                      NM_MDMPRXY_CB_MODNAME,
                      &nm_mdmprxy_remove_wlan_rules);

  /* Register for netd restart handlers */
  netmgr_cb_insert_cb(NETMGR_KIF_CB_TABLE,
                      NETMGR_KIF_CB_NETD_IS,
                      NM_MDMPRXY_CB_MODNAME,
                      &nm_mdmprxy_netd_is_handler);

  netmgr_cb_insert_cb(NETMGR_KIF_CB_TABLE,
                      NETMGR_KIF_CB_LINK_NETD_IS,
                      NM_MDMPRXY_CB_MODNAME,
                      &nm_mdmprxy_netd_per_link_is_handler);

  netmgr_log_low("%s(): Finished", __func__);
}
