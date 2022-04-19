/******************************************************************************

                   N M _ M D M P R X Y _ C O M M O N _ . H

******************************************************************************/

/******************************************************************************

  @file    nm_mdmprxy_common.h
  @brief   Modem proxy plugin

  DESCRIPTION
  Modem proxy plugin module is responsible for managing routing rules
  and packet marking rules for forwarding packets to modem.

******************************************************************************/
/*===========================================================================

  Copyright (c) 2015, 2017-2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

#include <sys/socket.h>
#include <inttypes.h>

extern int iiface_refcount_v4;
extern int iiface_refcount_v6;

#ifdef FEATURE_RECOVERY_ENABLED
/*===========================================================================
  FUNCTION  nm_mdmprxy_common_check_error
===========================================================================*/
/*!
@brief
  Check whether the error can be ignored

@return
  NETMGR_SUCCESS if the error can be ignored
  NETMGR_FAILURE if the error cannot be ignored
*/
/*=========================================================================*/
int nm_mdmprxy_common_check_error(char *error);
#endif /* FEATURE_RECOVERY_ENABLED */

/*===========================================================================
  FUNCTION  nm_mdmprxy_common_uninstall_jump_rules
===========================================================================*/
/*!
@brief
  In the main iptables chains, remove jump rule to custom chains

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
int nm_mdmprxy_common_uninstall_jump_rules(sa_family_t af);

/*===========================================================================
  FUNCTION  nm_mdmprxy_common_install_jump_rules
===========================================================================*/
/*!
@brief
  In the main iptables chains, install jump rule to custom chains

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
int nm_mdmprxy_common_install_jump_rules(sa_family_t af);

/*===========================================================================
  FUNCTION  nm_mdmprxy_common_install_jump_rules_netd_restart
===========================================================================*/
/*!
@brief
  In the main iptables chains, install jump rule to custom chains
  on netd restart

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
int nm_mdmprxy_common_install_jump_rules_netd_restart(sa_family_t af);

/*===========================================================================
  FUNCTION  nm_mdmprxy_common_clear_tcp_mss
===========================================================================*/
/*!
@brief
  Remove iptables TCP MSS clamping rule.

@return
  NETMGR_SUCCESS
  NETMGR_FAILURE
*/
/*=========================================================================*/
int nm_mdmprxy_common_clear_tcp_mss
(
  int                           af,
  int                           tech_type,
  const char                    *ifname,
  struct netmgr_kif_addrstorage *addr
);

/*===========================================================================
  FUNCTION  nm_mdmprxy_common_set_tcp_mss
===========================================================================*/
/*!
@brief
  Install iptables rule to set the TCP MSS to NM_MDMPRXY_TCP_MSS to avoid
  fragmentation of incoming ESP tunneled packets as such fragments might
  be discarded on some networks due to security concerns.

@return
  NETMGR_SUCCESS
  NETMGR_FAILURE
*/
/*=========================================================================*/
int nm_mdmprxy_common_set_tcp_mss
(
  int                           af,
  int                           tech_type,
  const char                    *ifname,
  struct netmgr_kif_addrstorage *addr
);

/*===========================================================================
  FUNCTION  nm_mdmprxy_common_oos_hdlr
===========================================================================*/
/*!
@brief
  Global OOS handler for common module

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
int nm_mdmprxy_common_oos_hdlr(boolean force);

/*===========================================================================
  FUNCTION  nm_mdmprxy_common_init
===========================================================================*/
/*!
@brief
  Handler for modem IS event

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
int nm_mdmprxy_common_init(void* arg);

/*===========================================================================
  FUNCTION  nm_mdmprxy_common_deinit
===========================================================================*/
/*!
@brief
  Cleanup function

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
int nm_mdmprxy_common_deinit(int restart);
