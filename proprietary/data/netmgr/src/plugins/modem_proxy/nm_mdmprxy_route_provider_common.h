/******************************************************************************

    N M _ M D M P R X Y _ R O U T E _ P R O V I D E R _ C O M M O N . H

******************************************************************************/

/******************************************************************************

  @file    nm_mdmproxy_route_provider_common.h
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
#include "nm_mdmprxy_iptables_helper.h"

#define MAX_IPSEC_TUNNELS 5 /* For now cap it at 5 */

/* Variables shared by ndc and hal route provider versions */
extern int iiface_refcount;
extern int iiface_epdg_refcount;
extern int iiface_handle;

extern boolean iptables_inited;

typedef struct nm_mdmprxy_global_cfg
{
  /* Currently ndc interface does not offer a mechanism to install routing rules per
     IP family however the callbacks from netmgr are per IP family ex. for dual-IP calls
     we will receive one callback for V4 and one for V6. We need to install the routing
     rules only once but there is a possibility of one of the IP families going down
     in the middle of a call. We need to ensure that the rules are removed only after
     both address families of the link are down. For this purpose we need to maintain
     two flags per link, one for V4 and one for V6 */
  boolean is_v4_rule_installed;
  boolean is_v6_rule_installed;
} nm_mdmprxy_cfg_t;

extern nm_mdmprxy_cfg_t nm_mdmprxy_global_link_status[NM_MDMPRXY_MAXDEVSTR];

#define NM_MDMPRXY_GET_ROUTING_RULES_INSTALLED_V4(link)                      \
  nm_mdmprxy_global_link_status[link].is_v4_rule_installed

#define NM_MDMPRXY_GET_ROUTING_RULES_INSTALLED_V6(link)                      \
  nm_mdmprxy_global_link_status[link].is_v6_rule_installed

#define NM_MDMPRXY_SET_ROUTING_RULES_INSTALLED(link, af, status)             \
do                                                                           \
{                                                                            \
  if (AF_INET == af)                                                         \
  {                                                                          \
    *(&nm_mdmprxy_global_link_status[link].is_v4_rule_installed) = status;   \
  }                                                                          \
  else if (AF_INET6 == af)                                                   \
  {                                                                          \
    *(&nm_mdmprxy_global_link_status[link].is_v6_rule_installed) = status;   \
  }                                                                          \
} while(0)

typedef struct
{
  int ref_count;
  ipsec_tunnel_info_t tunnel_info;
} nm_mdmprxy_tunnel_rule_t;

nm_mdmprxy_tunnel_rule_t* tunnel_rules[MAX_IPSEC_TUNNELS];

/*===========================================================================
  FUNCTION  nm_mdmprxy_route_prov_common_install_iiface_rules
===========================================================================*/
/*!
@brief
  Sets up rules to forward packets to the modem by using TEE target
  to clone packets

@return
  NETMGR_SUCCESS/NETMGR_FAILURE
*/
/*=========================================================================*/
int nm_mdmprxy_route_prov_common_install_iiface_rules
(
  int                  link,
  char*                link_name,
  int                  tech_type,
  int                  valid4,
  uint32_t             daddr4,
  int                  valid6,
  struct in6_addr      *daddr6,
  uint32_t             mask6,
  ipsec_tunnel_info_t  *tunnel
);

/*===========================================================================
  FUNCTION  nm_mdmprxy_route_prov_common_remove_iiface_rules
===========================================================================*/
/*!
@brief
  Cleans up iptables rules for the given link

@return
  NETMGR_SUCCESS/NETMGR_FAILURE
*/
/*=========================================================================*/
int nm_mdmprxy_route_prov_common_remove_iiface_rules
(
  int                            link,
  char                           *link_name,
  int                            tech_type,
  int                            ip_family,
  ipsec_tunnel_info_t            *tunnel,
  struct netmgr_kif_addrstorage  *link_addr
);

/*===========================================================================
  FUNCTION  nm_mdmprxy_route_prov_common_flush_doze_mode_rules
===========================================================================*/
/*!
@brief
  Global OOS doze rule clean-up
*/
/*=========================================================================*/
void nm_mdmprxy_route_prov_common_flush_doze_mode_rules(sa_family_t af);

/*===========================================================================
  FUNCTION  nm_mdmprxy_route_prov_common_link_is_hdlr
===========================================================================*/
/*!
@brief
  Per link IS handler
*/
/*=========================================================================*/
int nm_mdmprxy_route_prov_common_link_is_hdlr(void* arg);

/*===========================================================================
  FUNCTION  nm_mdmprxy_route_prov_common_oos_hdlr
===========================================================================*/
/*!
@brief
  Global OOS handler
*/
/*=========================================================================*/
void nm_mdmprxy_route_prov_common_oos_hdlr(void);

/*===========================================================================
  FUNCTION  nm_mdmprxy_route_prov_common_is_hdlr
===========================================================================*/
/*!
@brief
  Global IS handler

@note
  - Dependencies
    - Global IS handler for the common module has been called
*/
/*=========================================================================*/
void nm_mdmprxy_route_prov_common_is_hdlr(void);

/*===========================================================================
  FUNCTION  nm_mdmprxy_route_prov_common_link_oos_hdlr
===========================================================================*/
/*!
@brief
  Per link OOS handler
*/
/*=========================================================================*/
int nm_mdmprxy_route_prov_common_link_oos_hdlr(void* arg);

/*===========================================================================
  FUNCTION  nm_mdmprxy_route_prov_common_netd_oos_hdlr
===========================================================================*/
/*!
@brief
  Netd OOS handler

@return
  NETMGR_SUCCESS / NETMGR_FAILURE
*/
/*=========================================================================*/
int nm_mdmprxy_route_prov_common_netd_oos_hdlr(void);

/*===========================================================================
  FUNCTION  nm_mdmprxy_route_prov_common_init
===========================================================================*/
/*!
@brief
  Init function

@note
  - Dependencies
    - Global nm_mdmprxy_common_init
*/
/*=========================================================================*/
int nm_mdmprxy_route_prov_common_init(uint32_t packetMark);

#ifdef FEATURE_RECOVERY_ENABLED
/*===========================================================================
  FUNCTION  nm_mdmprxy_route_prov_common_deinit
===========================================================================*/
/*!
@brief
  Function to cleanup the installed rules

@note
  - Dependencies
    - Global nm_mdmprxy_common_init
*/
/*=========================================================================*/
int nm_mdmprxy_route_prov_common_deinit(void);
#endif /* FEATURE_RECOVERY_ENABLED */
