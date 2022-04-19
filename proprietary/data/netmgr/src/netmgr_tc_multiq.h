/******************************************************************************

                   N E T M G R _ T C _ M U L T I Q. H

******************************************************************************/

/******************************************************************************

  @file    netmgr_tc_multiq.h
  @brief   Network Manager traffic control multiq header file

  DESCRIPTION
  Header file for NetMgr Linux traffic control interface.

******************************************************************************/
/*===========================================================================

  Copyright (c) 2010-2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

#ifndef __NETMGR_TC_MULTIQ_H__
#define __NETMGR_TC_MULTIQ_H__

/*===========================================================================
                              INCLUDE FILES
===========================================================================*/
#include "netmgr_qmi.h"

/*===========================================================================
                     GLOBAL DEFINITIONS AND DECLARATIONS
===========================================================================*/


/*===========================================================================
                            GLOBAL FUNCTION DECLARATIONS
===========================================================================*/

int netmgr_tc_flow_activate_hdlr_multiq
(
  int link,
  const netmgr_qmi_qos_flow_info_t  *qos_flow
);

int netmgr_tc_flow_delete_hdlr_multiq
(
  int link,
  const netmgr_qmi_qos_flow_info_t  *qos_flow
);

int netmgr_tc_flow_control_hdlr_multiq
(
  int link,
  const netmgr_qmi_qos_flow_info_t  *qos_flow,
  netmgr_tc_flow_state_t state
);

int netmgr_tc_flow_modify_hdlr_multiq
(
  int link,
  const netmgr_qmi_qos_flow_info_t  *qos_flow
);

int netmgr_tc_flow_rebind_hdlr_multiq
(
  int link,
  const netmgr_qmi_qos_flow_info_t  *qos_flow
);

#endif /* __NETMGR_TC_MULTIQ_H__ */
