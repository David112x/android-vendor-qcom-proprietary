/******************************************************************************

                          N E T M G R _ T C _ I . H

******************************************************************************/

/******************************************************************************

  @file    netmgr_tc_i.h
  @brief   Network Manager traffic control internal header file

  DESCRIPTION
  Header file for NetMgr Linux traffic control interface.

******************************************************************************/
/*===========================================================================

  Copyright (c) 2010, 2018, 2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

/******************************************************************************

                      EDIT HISTORY FOR FILE

  $Id:$

when       who        what, where, why
--------   ---        -------------------------------------------------------
02/23/10   ar         Initial version

******************************************************************************/

#ifndef __NETMGR_TC_I_H__
#define __NETMGR_TC_I_H__

/*===========================================================================
                              INCLUDE FILES
===========================================================================*/
#include "netmgr_tc.h"

/*===========================================================================
                        DEFINITIONS AND DECLARATIONS
===========================================================================*/

#define NETMGR_TC_QDISC_ROOT_MAJOR                    1
#define NETMGR_TC_QDISC_ROOT_MINOR                    0
#define NETMGR_TC_FILTER_PRIORITY                     1
#define NETMGR_TC_TCP_ACK_FILTER_PRIORITY             0
#define NETMGR_TC_MAX_COMMAND_LENGTH     512

#define NETMGR_TC_CLASS_MINOR_ID_INIT    10  /* starting sequence value    */
#define NETMGR_TC_CLASS_MINOR_ID_INC     10  /* sequence value increment   */
#define NETMGET_TC_ASSIGN_CLASS_MINOR_ID(link,val)               \
        {                                                        \
          val = netmgr_tc_cfg.links[link].next_class_minor;      \
          netmgr_tc_cfg.links[link].next_class_minor +=          \
            NETMGR_TC_CLASS_MINOR_ID_INC;                        \
        }

#define NETMGR_TC_MAX_MULTIQ_NUM 16

extern struct netmgr_tc_cfg_s netmgr_tc_cfg;
extern pthread_mutex_t tc_lock;

/*---------------------------------------------------------------------------
   Type representing TC handle information
---------------------------------------------------------------------------*/
typedef struct netmgr_tc_handle_info_s {
  int major;
  int minor;
} netmgr_tc_handle_info_t;

/*---------------------------------------------------------------------------
   Type representing collection of a flow's state information
---------------------------------------------------------------------------*/
typedef struct netmgr_tc_flow_info_s {
  const char *phys_net_dev;
  const char *link_name;
  netmgr_qmi_qos_flow_info_t  qos_flow; /* Modem flow attributes */
  netmgr_tc_handle_info_t* class_handle; /* class handle */
  netmgr_tc_handle_info_t* tcp_ack_class;
} netmgr_tc_flow_info_t;

/*---------------------------------------------------------------------------
   multiq bearer map
---------------------------------------------------------------------------*/
typedef struct netmgr_tc_mq_info_s {
  uint8 bearer_id;
  uint8 ref;
} netmgr_tc_mq_info_t;

/*---------------------------------------------------------------------------
   Type representing collection of a flow's state information
---------------------------------------------------------------------------*/
typedef struct netmgr_tc_link_info_s {
  netmgr_tc_handle_info_t *root_qdisc;    /* root qdisc handle             */
  netmgr_tc_handle_info_t *root_class;    /* root class handle             */
  netmgr_tc_handle_info_t *default_class; /* default class handle          */
  netmgr_tc_handle_info_t *default_class2;/* 2nd default class handle      */
  ds_dll_el_t *flow_list;                 /* Flow objects list head        */
  int next_class_minor;                   /* Class minor ID sequence       */
  netmgr_tc_mq_info_t mq_info[NETMGR_TC_MAX_MULTIQ_NUM];
} netmgr_tc_link_info_t;

/*---------------------------------------------------------------------------
   Collection of configuration information for the module
---------------------------------------------------------------------------*/
struct netmgr_tc_cfg_s {
  int                   nlink;                     /* number of qmi links  */
  netmgr_ctl_port_config_type *link_array;         /* link enabled array   */
  netmgr_tc_link_info_t links[NETMGR_MAX_LINK];    /* link state info      */
  boolean               is_initialized;            /* Flag init completed  */
  boolean               postrouting_chain_available;  /* chain to only mark qcom qos*/
  ds_dll_el_t           *filter_list;                /* Filter precedence list head */
};

/*-----------------------------------------------------------------------------
 * Keep latest info for a partiluar flow from QMI for ignorning redundant
 * QoS info
 *---------------------------------------------------------------------------*/
typedef struct netmgr_qos_modify_cache_link_info_s {
  ds_dll_el_t *flow_list;                 /* Flow objects list head        */
} netmgr_tc_link_qos_modify_info_t;

struct netmgr_qos_modify_cache_s {
  netmgr_tc_link_qos_modify_info_t links[NETMGR_MAX_LINK];    /* link state info      */
  pthread_mutex_t modify_mutex;
};

extern struct netmgr_qos_modify_cache_s netmgr_qos_modify_cache;
/*===========================================================================
                            FUNCTION DECLARATIONS
===========================================================================*/

netmgr_tc_handle_info_t * netmgr_tc_handle_info_alloc
(
);

netmgr_tc_flow_info_t * netmgr_tc_flow_alloc
(
);

int netmgr_tc_create_flow_chain
(
  int    link,
  uint32 flow_id,
  uint32 ip_family,
  uint32 mark,
  uint32 tcp_ack_mark
);

int netmgr_tc_flow_enable
(
  int                   link,
  netmgr_tc_flow_info_t *flow_info
);

void netmgr_tc_flow_free
(
  netmgr_tc_flow_info_t **flow_info
);

void netmgr_tc_flow_filter_dump
(
  const netmgr_qmi_qos_flow_info_t  *qos_flow
);

int netmgr_tc_delete_flow_chain_ref
(
  netmgr_tc_flow_info_t *flow_info,
  int       link
);

boolean netmgr_tc_is_default_flow_activated
(
  int                                link,
  const netmgr_qmi_qos_flow_info_t * qos_flow
);

netmgr_tc_flow_info_t* netmgr_tc_find_flow_from_bearer
(
  int link,
  uint8 bearer_id
);

netmgr_tc_flow_info_t* netmgr_tc_find_flow_info
(
  int link,
  uint32 flow_id,
  netmgr_qos_ip_version_type ip_ver
);

long int netmgr_tc_match_flows
(
  const void * first,
  const void * second
);

void netmgr_tc_filter_delete_hdlr
(
  netmgr_tc_flow_info_t *flow_info,
  int       link,
  uint32    flow_id
);

#endif /* __NETMGR_TC_I_H__ */
