/******************************************************************************

     N M _ M D M P R X Y _ M A R K _ P R O V I D E R _ C O M M O N . H

******************************************************************************/

/******************************************************************************

  @file    nm_mdmprxy_mark_provider_common.h
  @brief   Modem proxy plugin

  DESCRIPTION
  Mark Provider common module responsible for managing common state and
  rules shared between different mark providers

******************************************************************************/
/*===========================================================================

  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/
/*===========================================================================
                              INCLUDE FILES
===========================================================================*/
#include "nm_mdmprxy_iptables_helper.h"

/*===========================================================================
                          FUNCTION DECLERATIONS
===========================================================================*/
/*===========================================================================
  FUNCTION  nm_mdmprxy_common_mark_prov_remove_spi_rule
===========================================================================*/
/*!
@brief
  Remove SPI based packet-marking rule in iptables for given family

@return
  NETMGR_SUCCESS on successful command execution
  NETMGR_FAILURE otherwise

@return
  NETMGR_SUCCESS
  NETMGR_FAILURE
*/
/*=========================================================================*/
int nm_mdmprxy_common_mark_prov_remove_spi_rule
(
  sa_family_t   af,
  unsigned int  spi
);

/*===========================================================================
  FUNCTION  nm_mdmprxy_common_mark_prov_install_spi_rule
===========================================================================*/
/*!
@brief
  Install SPI based packet-marking rule in iptables for given family

@return
  NETMGR_SUCCESS
  NETMGR_FAILURE
*/
/*=========================================================================*/
int nm_mdmprxy_common_mark_prov_install_spi_rule
(
  sa_family_t   af,
  unsigned int  spi
);

/*===========================================================================
  FUNCTION  nm_mdmprxy_common_mark_prov_remove_portfwd_rule
===========================================================================*/
/*!
@brief
  Removes rules to mark packets in IMS audio range with modem mark

@return
  NETMGR_SUCCESS
  NETMGR_FAILURE
*/
/*=========================================================================*/
int nm_mdmprxy_common_mark_prov_remove_portfwd_rule
(
  sa_family_t af,
  boolean     pref
);

/*===========================================================================
  FUNCTION  nm_mdmprxy_common_mark_prov_install_portfwd_rule
===========================================================================*/
/*!
@brief
  Installs rules to mark packets in IMS audio range with modem mark

@return
  NETMGR_SUCCESS
  NETMGR_FAILURE
*/
/*=========================================================================*/
int nm_mdmprxy_common_mark_prov_install_portfwd_rule
(
  sa_family_t af,
  boolean     pref
);

/*===========================================================================
  FUNCTION  nm_mdmprxy_common_mark_prov_query_portfwd_status
===========================================================================*/
/*!
@brief
  Queries port forward status

@return
  Install preference for the given ip family
*/
/*=========================================================================*/
boolean nm_mdmprxy_common_mark_prov_query_portfwd_status(sa_family_t af);

/*===========================================================================
  FUNCTION  nm_mdmprxy_common_mark_prov_install_icmp_rules
===========================================================================*/
/*!
@brief
  Installs iptables connection tracking rules which support ICMP data
  transfer from modem

@return
  NETMGR_SUCCESS
  NETMGR_FAILURE
*/
/*=========================================================================*/
int nm_mdmprxy_common_mark_prov_install_icmp_rules
(
  sa_family_t af,
  char* link_name
);

/*===========================================================================
  FUNCTION  nm_mdmprxy_common_mark_prov_remove_icmp_rules
===========================================================================*/
/*!
@brief
  Removes iptables connection tracking rules which support ICMP data
  transfer from modem

@return
  NETMGR_SUCCESS
  NETMGR_FAILURE
*/
/*=========================================================================*/
int nm_mdmprxy_common_mark_prov_remove_icmp_rules
(
  sa_family_t af,
  char* link_name
);

/*===========================================================================
  FUNCTION  nm_mdmprxy_common_mark_prov_oos_cleanup
===========================================================================*/
/*!
@brief
  Flushes common mark provider chains during OOS

@return
  NETMGR_SUCCESS
  NETMGR_FAILURE
*/
/*=========================================================================*/
int nm_mdmprxy_common_mark_prov_oos_cleanup(void);

/*===========================================================================
  FUNCTION  nm_mdmprxy_common_mark_prov_netd_oos_hdlr
===========================================================================*/
/*!
@brief
  Handle netd OOS event

@return
  NETMGR_SUCCESS / NETMGR_FAILURE
*/
/*=========================================================================*/
int nm_mdmprxy_common_mark_prov_netd_oos_hdlr(void);

/*===========================================================================
  FUNCTION  nm_mdmprxy_common_mark_prov_netd_is_hdlr
===========================================================================*/
/*!
@brief
  Handle netd IS event

@return
  NETMGR_SUCCESS / NETMGR_FAILURE
*/
/*=========================================================================*/
int nm_mdmprxy_common_mark_prov_netd_is_hdlr(uint32_t packet_mark);

/*===========================================================================
  FUNCTION  nm_mdmprxy_common_mark_prov_init
===========================================================================*/
/*!
@brief
  Initialization routine for setting up common mark provider rules and state

@return
  NETMGR_SUCCESS
  NETMGR_FAILURE
*/
/*=========================================================================*/
int nm_mdmprxy_common_mark_prov_init
(
  struct mark_prov_iptables *rulechains,
  uint32_t packet_mark
);
