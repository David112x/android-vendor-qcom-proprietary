/**************************************************************************************

       N M _ M D M P R X Y _ M A R K _ P R O V I D E R _ F A C T O R Y . H

***************************************************************************************/

/**************************************************************************************

  @file    nm_mdmproxy_mark_provider_factory.h
  @brief   Modem proxy plugin

  DESCRIPTION
  Header file for mark provider factory

**************************************************************************************/
/*====================================================================================

  Copyright (c) 2015-2016, 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

======================================================================================*/

#ifndef __MDM_PROXY_MARK_FACTORY_H__
#define __MDM_PROXY_MARK_FACTORY_H__
#endif /* __MDM_PROXY_ROUTE_FACTORY_H__ */

/*=====================================================================================
                                  INCLUDE FILES
=====================================================================================*/
#include <sys/socket.h>
#include "data_filter_service_v01.h"

/*=====================================================================================
                      LOCAL DEFINITIONS AND DECLARATIONS
=====================================================================================*/

typedef enum
{
  NM_MDMPRXY_SOCKET_STATE_SHUTDOWN,
  NM_MDMPRXY_SOCKET_STATE_LISTENING
} socket_state_t;

typedef struct mark_provider_s
{
  /* Install and remove iptables marking rules */
  int (*install_rules) (sa_family_t);
  int (*remove_rules)  (sa_family_t);

  /* Specific use-cases */
  int (*install_spi_rule) (sa_family_t, unsigned int);
  int (*remove_spi_rule)  (sa_family_t, unsigned int);

  int     (*install_portfwd_rule) (sa_family_t, boolean);
  int     (*remove_portfwd_rule)  (sa_family_t, boolean);
  boolean (*query_portfwd_status) (sa_family_t);

  int (*install_icmp_rule)    (sa_family_t, char*);
  int (*remove_icmp_rule)     (sa_family_t, char*);

  int (*init)          (uint32_t);
  int (*deinit)        (int);

  /* Global OOS and IS handlers */
  int (*is_hdlr)       (void);
  int (*oos_hdlr)      (int);

  /* Netd OOS and IS handlers */
  void (*netd_oos_hdlr)         (void);
  void (*netd_is_hdlr)          (uint32_t);
  int  (*netd_is_hdlr_per_link) (void*);

  /* Change sate of sockets */
  void (*change_socket_state)  (socket_state_t);

  void (*sigusr1_hdlr) (void);
} nm_mdmprxy_mark_prov_t;

/*=====================================================================================
                                PUBLIC FUNCTIONS
=====================================================================================*/
/*====================================================================================
  FUNCTION  nm_mdmprxy_get_mark_provider
====================================================================================*/
/*!
@brief
  Returns the mark provider instance based on modem query support

@return
  struct route_provider instance

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*==================================================================================*/
nm_mdmprxy_mark_prov_t *nm_mdmprxy_get_mark_provider(void);

/*====================================================================================
  FUNCTION  nm_mdmprxy_get_dfs_service_version
====================================================================================*/
/*!
@brief
  Returns the version number for DFS service

@return
  nm_mdmprxy_mark_prov_t instance

@note

  - Dependencies
    - We have requested for capability information from modem via
      QMI_DFS_GET_CAPABILITY_REQ_V01

  - Side Effects
    - None
*/
/*==================================================================================*/
dfs_remote_socket_handling_ver_enum_v01 nm_mdmprxy_get_dfs_service_version(void);
