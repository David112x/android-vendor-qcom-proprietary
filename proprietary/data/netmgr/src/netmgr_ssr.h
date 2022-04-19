/******************************************************************************

                          N E T M G R _ S S R . H

******************************************************************************/

/******************************************************************************

  @file    netmgr_ssr.h
  @brief   Network Manager SSR module headers

  DESCRIPTION
  Defintions of NetMgr's SSR module function.

******************************************************************************/
/*===========================================================================

  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

#ifndef __NETMGR_SSR_H__
#define __NETMGR_SSR_H__

#include "netmgr_config.h"
#include "qmi_client.h"

int netmgr_ssr_init
(
  qmi_idl_service_object_type  svc_obj,
  boolean                      blocking,
  qmi_service_instance         blocking_inst,
  unsigned int                 timeout
);

void netmgr_ssr_deinit();

#endif /* __NETMGR_SSR_H__ */
