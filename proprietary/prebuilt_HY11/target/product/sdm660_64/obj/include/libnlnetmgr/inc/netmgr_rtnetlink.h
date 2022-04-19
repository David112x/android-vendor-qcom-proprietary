/******************************************************************************

                        N E T M G R _ K I F . H

******************************************************************************/

/******************************************************************************

  @file    netmgr_kif.h
  @brief   Network Manager Kernel Interface Module header file

  DESCRIPTION
  Header file for Kernel Interface module.

******************************************************************************/
/*===========================================================================

  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/
#define NETMGR_FLOW_ENABLE 1
#define NETMGR_FLOW_DISABLE 0

typedef enum netmgr_nl_requests_e
{
  NETMGR_ADD_NETLINK,
  NETMGR_DEL_NETLINK,
  NETMGR_CHANGE_NETLINK,
  NETMGR_REPLACE_NETLINK
} netmgr_nl_requests_t;

/*===========================================================================
  FUNCTION  netmgr_rtnetlink_flow_control
===========================================================================*/
/*!
@brief
  Sends a netlink route message to kernel to setup a tc qdisc prio flow

@return
  NETMGR_SUCCESS on success
  NETMGR_FAILURE on failure

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int
netmgr_rtnetlink_flow_control
(
  const char* devname,
  uint32_t parent,
  uint32_t handle,
  uint8_t enable,
  uint8_t flags
);

