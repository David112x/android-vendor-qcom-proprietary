/******************************************************************************

                          N E T M G R _ T C . C

******************************************************************************/

/******************************************************************************

  @file    netmgr_tc.c
  @brief   Network Manager traffic control

  DESCRIPTION
  Implementation of NetMgr Linux traffic control module.

******************************************************************************/
/*===========================================================================

  Copyright (c) 2010-2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

/*===========================================================================
                              INCLUDE FILES
===========================================================================*/

#ifdef NETMGR_QOS_ENABLED

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>
#include "ds_list.h"
#include "ds_string.h"
#include "netmgr_defs.h"
#include "netmgr_exec.h"
#include "netmgr_platform.h"
#include "netmgr_util.h"
#include "netmgr_tc_i.h"
#include "netmgr_rmnet.h"
#include "netmgr_cb.h"
#include "netmgr_qmi_cb.h"
#include "netmgr_tc_multiq.h"
#include "netmgr_tc.h"
#include "netmgr_main.h"

#ifdef NETMGR_NL_ENABLED
#include "netmgr_rtnetlink.h"
#endif
#include <arpa/inet.h>

/*===========================================================================
                     LOCAL DEFINITIONS AND DECLARATIONS
===========================================================================*/

/*---------------------------------------------------------------------------
  Constants for queue discipline and class configuration
---------------------------------------------------------------------------*/
#define NETMGR_TC_QDISC_DEFAULT_CLASS                 10
#define NETMGR_TC_QDISC_HTB_RATE2QTM                  60  /* bandwidth borroring chunks */
#define NETMGR_TC_BASE_DATARATE                      (55000000UL)        /* bps units */

#define NETMGR_TC_CHAIN_NAME_PREFIX      "qcom"
#define NETMGR_TC_IPTABLES_TOOL_V4       "iptables"
#define NETMGR_TC_IPTABLES_TOOL_V6       "ip6tables"
#define NETMGR_TC_HANDLE_LEN             5
#define NETMGR_TC_TCP_PROTO_IANA_NUM     6
#define NETMGR_TC_UDP_PROTO_IANA_NUM     17
#define NETMGR_TC_INVALID_PROTO          -1

#define NETMGR_TC_CHAIN_NAME_QOS_RESET_POSTROUTING  "qcom_qos_reset_POSTROUTING"
#define NETMGR_TC_CHAIN_NAME_QOS_FILTER_POSTROUTING "qcom_qos_filter_POSTROUTING"

#define NETMGR_TC_DEFAULT_FLOW_MARKING   0

struct netmgr_tc_cfg_s netmgr_tc_cfg;
struct netmgr_qos_modify_cache_s netmgr_qos_modify_cache_cfg;

/*---------------------------------------------------------------------------
  CDMA QoS specification table    Reference: 3GPP2 C.R101-G (TSB-58)
  Used to map profile ID to traffic control priority and datarate.
---------------------------------------------------------------------------*/
struct netmgr_tc_cdma_qos_spec_s {
  uint16      profile_id;
  uint32      datarate;   /* bps units */
  uint8       priority;
};

LOCAL const struct netmgr_tc_cdma_qos_spec_s
netmgr_tc_cdma_qos_spec_tbl[] = {
  {    0, NETMGR_TC_DEFAULT_DATARATE, NETMGR_TC_CLASS_PRIO_BESTEFFORT },
  {    1, 32000,     NETMGR_TC_CLASS_PRIO_STREAMING       },
  {    2, 64000,     NETMGR_TC_CLASS_PRIO_STREAMING       },
  {    3, 96000,     NETMGR_TC_CLASS_PRIO_STREAMING       },
  {    4, 128000,    NETMGR_TC_CLASS_PRIO_STREAMING       },
  {    5, 32000,     NETMGR_TC_CLASS_PRIO_CONVERSATIONAL  },  /*Note1*/
  {    6, 64000,     NETMGR_TC_CLASS_PRIO_CONVERSATIONAL  },  /*Note1*/
  {    7, 96000,     NETMGR_TC_CLASS_PRIO_CONVERSATIONAL  },  /*Note1*/
  {    8, 144000,    NETMGR_TC_CLASS_PRIO_CONVERSATIONAL  },  /*Note1*/
  {    9, 384000,    NETMGR_TC_CLASS_PRIO_CONVERSATIONAL  },  /*Note1*/
  {   10, 768000,    NETMGR_TC_CLASS_PRIO_CONVERSATIONAL  },  /*Note1*/
  {   11, 1536000,   NETMGR_TC_CLASS_PRIO_CONVERSATIONAL  },  /*Note1*/
  {   12, 32000,     NETMGR_TC_CLASS_PRIO_CONVERSATIONAL  },  /*Note1*/
  {   13, 64000,     NETMGR_TC_CLASS_PRIO_CONVERSATIONAL  },  /*Note1*/
  {   14, 96000,     NETMGR_TC_CLASS_PRIO_CONVERSATIONAL  },  /*Note1*/
  {   15, 144000,    NETMGR_TC_CLASS_PRIO_CONVERSATIONAL  },  /*Note1*/
  {   16, 384000,    NETMGR_TC_CLASS_PRIO_CONVERSATIONAL  },  /*Note1*/
  {   17, 32000,     NETMGR_TC_CLASS_PRIO_CONVERSATIONAL  },  /*Note1*/
  {   18, 64000,     NETMGR_TC_CLASS_PRIO_CONVERSATIONAL  },  /*Note1*/
  {   19, 96000,     NETMGR_TC_CLASS_PRIO_CONVERSATIONAL  },  /*Note1*/
  {   20, 144000,    NETMGR_TC_CLASS_PRIO_CONVERSATIONAL  },  /*Note1*/
  {   21, 32000,     NETMGR_TC_CLASS_PRIO_INTERACTIVE     },  /*Note1*/
  {   22, 64000,     NETMGR_TC_CLASS_PRIO_INTERACTIVE     },  /*Note1*/
  {   23, 96000,     NETMGR_TC_CLASS_PRIO_INTERACTIVE     },  /*Note1*/
  {   24, 144000,    NETMGR_TC_CLASS_PRIO_INTERACTIVE     },  /*Note1*/
  {   25, 384000,    NETMGR_TC_CLASS_PRIO_INTERACTIVE     },  /*Note1*/
  {   26, 768000,    NETMGR_TC_CLASS_PRIO_INTERACTIVE     },  /*Note1*/
  {   27, 1536000,   NETMGR_TC_CLASS_PRIO_INTERACTIVE     },  /*Note1*/
  {   28, 32000,     NETMGR_TC_CLASS_PRIO_INTERACTIVE     },  /*Note1*/
  {   29, 64000,     NETMGR_TC_CLASS_PRIO_INTERACTIVE     },  /*Note1*/
  {   30, 96000,     NETMGR_TC_CLASS_PRIO_INTERACTIVE     },  /*Note1*/
  {   31, 144000,    NETMGR_TC_CLASS_PRIO_INTERACTIVE     },  /*Note1*/
  {   32, 384000,    NETMGR_TC_CLASS_PRIO_INTERACTIVE     },  /*Note1*/
  {   33, 768000,    NETMGR_TC_CLASS_PRIO_INTERACTIVE     },  /*Note1*/
  {   34, 1536000,   NETMGR_TC_CLASS_PRIO_INTERACTIVE     },  /*Note1*/
  {   35, 32000,     NETMGR_TC_CLASS_PRIO_INTERACTIVE     },  /*Note1*/
  {   36, 64000,     NETMGR_TC_CLASS_PRIO_INTERACTIVE     },  /*Note1*/
  {   37, 96000,     NETMGR_TC_CLASS_PRIO_INTERACTIVE     },  /*Note1*/
  {   38, 140004,    NETMGR_TC_CLASS_PRIO_INTERACTIVE     },  /*Note1*/
  {   39, 384000,    NETMGR_TC_CLASS_PRIO_INTERACTIVE     },  /*Note1*/
  {   40, 32000,     NETMGR_TC_CLASS_PRIO_BACKGROUND      },  /*Note1*/
  {   41, 64000,     NETMGR_TC_CLASS_PRIO_BACKGROUND      },  /*Note1*/
  {   42, 96000,     NETMGR_TC_CLASS_PRIO_BACKGROUND      },  /*Note1*/
  {   43, 144000,    NETMGR_TC_CLASS_PRIO_BACKGROUND      },  /*Note1*/
  {   44, 384000,    NETMGR_TC_CLASS_PRIO_BACKGROUND      },  /*Note1*/
  {   45, 768000,    NETMGR_TC_CLASS_PRIO_BACKGROUND      },  /*Note1*/
  {   46, 1536000,   NETMGR_TC_CLASS_PRIO_BACKGROUND      },  /*Note1*/
  {   47, 32000,     NETMGR_TC_CLASS_PRIO_BACKGROUND      },  /*Note1*/
  {   48, 64000,     NETMGR_TC_CLASS_PRIO_BACKGROUND      },  /*Note1*/
  {   49, 96000,     NETMGR_TC_CLASS_PRIO_BACKGROUND      },  /*Note1*/
  {   50, 144000,    NETMGR_TC_CLASS_PRIO_BACKGROUND      },  /*Note1*/
  {   51, 384000,    NETMGR_TC_CLASS_PRIO_BACKGROUND      },  /*Note1*/
  {   52, 768000,    NETMGR_TC_CLASS_PRIO_BACKGROUND      },  /*Note1*/
  {   53, 1536000,   NETMGR_TC_CLASS_PRIO_BACKGROUND      },  /*Note1*/
  {   54, 32000,     NETMGR_TC_CLASS_PRIO_BACKGROUND      },  /*Note1*/
  {   55, 64000,     NETMGR_TC_CLASS_PRIO_BACKGROUND      },  /*Note1*/
  {   56, 96000,     NETMGR_TC_CLASS_PRIO_BACKGROUND      },  /*Note1*/
  {   57, 144000,    NETMGR_TC_CLASS_PRIO_BACKGROUND      },  /*Note1*/
  {   58, 384000,    NETMGR_TC_CLASS_PRIO_BACKGROUND      },  /*Note1*/
  {   59, 768000,    NETMGR_TC_CLASS_PRIO_BACKGROUND      },  /*Note1*/
  {   60, 1536000,   NETMGR_TC_CLASS_PRIO_BACKGROUND      },  /*Note1*/
  /* Reserved range: 61-255 */
  {  256, 12200,     NETMGR_TC_CLASS_PRIO_CONVERSATIONAL  },  /*Note2,3*/
  {  257, 15000,     NETMGR_TC_CLASS_PRIO_CONVERSATIONAL  },  /*Note2*/
  {  258, 12200,     NETMGR_TC_CLASS_PRIO_STREAMING       },  /*Note2,3*/
  {  259, 15000,     NETMGR_TC_CLASS_PRIO_STREAMING       },  /*Note2*/
  {  260, 8000,      NETMGR_TC_CLASS_PRIO_STREAMING       },  /*Note2*/
  {  261, 12200,     NETMGR_TC_CLASS_PRIO_CONVERSATIONAL  },  /*Note2,3*/
  {  262, 15000,     NETMGR_TC_CLASS_PRIO_CONVERSATIONAL  },  /*Note2*/
  {  263, 12200,     NETMGR_TC_CLASS_PRIO_STREAMING       },  /*Note2,3*/
  {  264, 15000,     NETMGR_TC_CLASS_PRIO_STREAMING       },  /*Note2*/
  {  265, 8000,      NETMGR_TC_CLASS_PRIO_STREAMING       },  /*Note2*/
  {  266, 12200,     NETMGR_TC_CLASS_PRIO_CONVERSATIONAL  },  /*Note2,3*/
  {  267, 15000,     NETMGR_TC_CLASS_PRIO_CONVERSATIONAL  },  /*Note2*/
  {  268, 12200,     NETMGR_TC_CLASS_PRIO_STREAMING       },  /*Note2,3*/
  {  269, 15000,     NETMGR_TC_CLASS_PRIO_STREAMING       },  /*Note2*/
  {  270, 8000,      NETMGR_TC_CLASS_PRIO_STREAMING       },  /*Note2*/
  {  271, 12200,     NETMGR_TC_CLASS_PRIO_CONVERSATIONAL  },  /*Note2,3*/
  {  272, 15000,     NETMGR_TC_CLASS_PRIO_CONVERSATIONAL  },  /*Note2*/
  {  273, 12200,     NETMGR_TC_CLASS_PRIO_STREAMING       },  /*Note2,3*/
  {  274, 15000,     NETMGR_TC_CLASS_PRIO_STREAMING       },  /*Note2*/
  {  275, 8000,      NETMGR_TC_CLASS_PRIO_STREAMING       },  /*Note2*/
  {  276, 12200,     NETMGR_TC_CLASS_PRIO_CONVERSATIONAL  },  /*Note2,4*/
  {  277, 5000,      NETMGR_TC_CLASS_PRIO_CONVERSATIONAL  },  /*Note2*/
  {  278, 5000,      NETMGR_TC_CLASS_PRIO_CONVERSATIONAL  },  /*Note2*/
  {  279, 5000,      NETMGR_TC_CLASS_PRIO_CONVERSATIONAL  },  /*Note2*/
  {  280, 5000,      NETMGR_TC_CLASS_PRIO_CONVERSATIONAL  },  /*Note2*/
  {  281, 12200,     NETMGR_TC_CLASS_PRIO_STREAMING       },  /*Note2,3*/
  {  282, 5000,      NETMGR_TC_CLASS_PRIO_STREAMING       },  /*Note2*/
  {  283, 5000,      NETMGR_TC_CLASS_PRIO_STREAMING       },  /*Note2*/
  {  284, 5000,      NETMGR_TC_CLASS_PRIO_STREAMING       },  /*Note2*/
  {  285, 5000,      NETMGR_TC_CLASS_PRIO_STREAMING       },  /*Note2*/
  {  286, 6000,      NETMGR_TC_CLASS_PRIO_CONVERSATIONAL  },  /*Note2*/
  {  287, 6000,      NETMGR_TC_CLASS_PRIO_CONVERSATIONAL  },  /*Note2*/
  {  288, 3000,      NETMGR_TC_CLASS_PRIO_CONVERSATIONAL  },  /*Note2*/
  {  289, 6000,      NETMGR_TC_CLASS_PRIO_CONVERSATIONAL  },  /*Note2*/
  {  290, 6000,      NETMGR_TC_CLASS_PRIO_CONVERSATIONAL  },  /*Note2*/
  {  291, 3000,      NETMGR_TC_CLASS_PRIO_CONVERSATIONAL  },  /*Note2*/
  {  292, 10000,     NETMGR_TC_CLASS_PRIO_CONVERSATIONAL  },  /*Note2*/
  /* Reserved range: 293-511 */
  {  512, 16000,     NETMGR_TC_CLASS_PRIO_STREAMING       },
  {  513, 24000,     NETMGR_TC_CLASS_PRIO_STREAMING       },
  {  514, 32000,     NETMGR_TC_CLASS_PRIO_STREAMING       },
  {  515, 48000,     NETMGR_TC_CLASS_PRIO_STREAMING       },
  {  516, 64000,     NETMGR_TC_CLASS_PRIO_STREAMING       },
  /* Reserved range: 517-767 */
  {  768, 24000,     NETMGR_TC_CLASS_PRIO_CONVERSATIONAL  },
  {  769, 32000,     NETMGR_TC_CLASS_PRIO_CONVERSATIONAL  },
  {  770, 40000,     NETMGR_TC_CLASS_PRIO_CONVERSATIONAL  },
  {  771, 48000,     NETMGR_TC_CLASS_PRIO_CONVERSATIONAL  },
  {  772, 56000,     NETMGR_TC_CLASS_PRIO_CONVERSATIONAL  },
  {  773, 64000,     NETMGR_TC_CLASS_PRIO_CONVERSATIONAL  },
  {  774, 24000,     NETMGR_TC_CLASS_PRIO_CONVERSATIONAL  },
  {  775, 32000,     NETMGR_TC_CLASS_PRIO_CONVERSATIONAL  },
  {  776, 40000,     NETMGR_TC_CLASS_PRIO_CONVERSATIONAL  },
  {  777, 48000,     NETMGR_TC_CLASS_PRIO_CONVERSATIONAL  },
  {  778, 56000,     NETMGR_TC_CLASS_PRIO_CONVERSATIONAL  },
  {  779, 64000,     NETMGR_TC_CLASS_PRIO_CONVERSATIONAL  },
  {  780, 24000,     NETMGR_TC_CLASS_PRIO_STREAMING       },
  {  781, 48000,     NETMGR_TC_CLASS_PRIO_STREAMING       },
  {  782, 64000,     NETMGR_TC_CLASS_PRIO_STREAMING       },
  {  783, 96000,     NETMGR_TC_CLASS_PRIO_STREAMING       },
  {  784, 120000,    NETMGR_TC_CLASS_PRIO_STREAMING       },
  {  785, 128000,    NETMGR_TC_CLASS_PRIO_STREAMING       },
  /* Reserved range: 786-1023 */
  { 1024, NETMGR_TC_DEFAULT_DATARATE, NETMGR_TC_CLASS_PRIO_STREAMING }, /*Note2*/
  /* Reserved range: 1025-1279 */
  { 1280, NETMGR_TC_DEFAULT_DATARATE, NETMGR_TC_CLASS_PRIO_CONVERSATIONAL }, /*Note2*/
  { 1281, NETMGR_TC_DEFAULT_DATARATE, NETMGR_TC_CLASS_PRIO_STREAMING      }, /*Note2*/
  { 1282, NETMGR_TC_DEFAULT_DATARATE, NETMGR_TC_CLASS_PRIO_INTERACTIVE    }, /*Note2*/
  { 1283, NETMGR_TC_DEFAULT_DATARATE, NETMGR_TC_CLASS_PRIO_CONVERSATIONAL }, /*Note1,2*/
  /* Reserved range: 1284-1535 */
  { 1536, NETMGR_TC_DEFAULT_DATARATE, NETMGR_TC_CLASS_PRIO_INTERACTIVE    }, /*Note1,2*/
  /* Reserved range: 1537-1791 */
  { 1792, NETMGR_TC_DEFAULT_DATARATE, NETMGR_TC_CLASS_PRIO_INTERACTIVE    }, /*Note2*/
  { 1793, NETMGR_TC_DEFAULT_DATARATE, NETMGR_TC_CLASS_PRIO_INTERACTIVE    }, /*Note2*/
  { 1794, NETMGR_TC_DEFAULT_DATARATE, NETMGR_TC_CLASS_PRIO_INTERACTIVE    }, /*Note2*/
  { 1795, NETMGR_TC_DEFAULT_DATARATE, NETMGR_TC_CLASS_PRIO_INTERACTIVE    }, /*Note2*/
  { 1796, NETMGR_TC_DEFAULT_DATARATE, NETMGR_TC_CLASS_PRIO_INTERACTIVE    }, /*Note2*/
  /* Reserved range: 1797-2047 */

  /* Proprietary flow profile ID list may be appended to this table. */
};
/* Note1: priority assigned based on spec interpretation */
/* Note2: data rate assigned based on spec interpretation */
/* Note3: assuming 12.2kbps AMR codec */

#define NETMGR_TC_MAX_DEDICATED_BEARERS_PER_IP_FAMILY 8

pthread_mutex_t     tc_lock;
static pthread_mutexattr_t tc_lock_attr;

/*===========================================================================
                            FORWARD DECLARATIONS
===========================================================================*/

LOCAL int netmgr_tc_create_2nd_default_flow
(
  int link,
  netmgr_tc_flow_info_t   *flow_info,
  netmgr_tc_handle_info_t *class_handle
);

LOCAL int
netmgr_tc_create_tcp_ack_class
(
  int                      link,
  netmgr_tc_flow_info_t  * flow_info
);

LOCAL int
netmgr_tc_delete_tcp_ack_class
(
  int                      link,
  netmgr_tc_flow_info_t  * flow_info
);

/*===========================================================================
                            LOCAL FUNCTION DEFINITIONS
===========================================================================*/
/*===========================================================================
  FUNCTION  netmgr_tc_init_lock
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
static int netmgr_tc_init_lock(void)
{
  int ret = 0;

  ret = pthread_mutexattr_init(&tc_lock_attr);
  if (0 != ret)
  {
    netmgr_log_err("%s(): failed to initialize mutex attribute! Err: %d", __func__, ret);
    return NETMGR_FAILURE;
  }

  ret = pthread_mutexattr_settype(&tc_lock_attr, PTHREAD_MUTEX_RECURSIVE);
  if (0 != ret)
  {
    netmgr_log_err("%s(): failed to set mutex attribute! Err: %d", __func__, ret);
    return NETMGR_FAILURE;
  }

  ret = pthread_mutex_init(&tc_lock, &tc_lock_attr);
  if (0 != ret)
  {
    netmgr_log_err("%s(): failed to initialize pthread object! Err: %d", __func__, ret);
    return NETMGR_FAILURE;
  }

  ret = pthread_mutexattr_destroy(&tc_lock_attr);
  if (0 != ret)
  {
    netmgr_log_err("%s(): failed to detroy mutex attribute! Err: %d", __func__, ret);
    return NETMGR_FAILURE;
  }

  return NETMGR_SUCCESS;
}

/*===========================================================================
  FUNCTION  netmgr_tc_flow_filter_dump
===========================================================================*/
/*!
@brief
  Writes data in Filters to QXDM logs

@return
  None.

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/

void
netmgr_tc_flow_filter_dump
(
  const netmgr_qmi_qos_flow_info_t  *qos_flow
)
{

  int tmp_cntr =0;
  const netmgr_qos_granted_filter_data_type *fltr_lst;

  if (qos_flow == NULL)
    return;

  netmgr_log_high("flow handle=0x%08x\n",
                 (unsigned int)qos_flow->flow_id );
  netmgr_log_high("flow is_new=%s\n",
                 ((TRUE==qos_flow->is_new)?"TRUE":"FALSE") );
  netmgr_log_high("flow priority=%d\n", qos_flow->priority );
  netmgr_log_high("flow datarate=%lu\n", qos_flow->datarate );
  netmgr_log_high("flow num_filter=%d\n", qos_flow->num_filter );
  netmgr_log_high("flow filter_list=%p\n", qos_flow->filter_list );
  netmgr_log_high("flow bearer_id=%d\n", qos_flow->bearer_id);
  netmgr_log_high("flow ip_ver=%d\n", qos_flow->ip_ver);

  for (tmp_cntr = 0; tmp_cntr < qos_flow->num_filter; tmp_cntr++)
  {
    fltr_lst = &(qos_flow->filter_list[tmp_cntr]);
    netmgr_log_high( "flow filter_list addrs=%p\n", fltr_lst );

    netmgr_log_high("[%d] param mask = %d\n",
                    tmp_cntr, (int)(fltr_lst->qos_filter.filter_desc.param_mask));
    netmgr_log_high("[%d] filter_index = %d\n",
                    tmp_cntr, fltr_lst->filter_index);
    netmgr_log_high("[%d]Filter ID = %d\n",
                    tmp_cntr, fltr_lst->qos_filter.filter_desc.filter_id);
    netmgr_log_high("[%d] precedence = %d\n",
                    tmp_cntr, fltr_lst->qos_filter.filter_desc.precedence);
    netmgr_log_low("[%d] esp security poicy index = %d\n",
                    tmp_cntr,
                    (int)fltr_lst->qos_filter.filter_desc.esp_security_policy_index);
    netmgr_log_high("[%d] IP version = %d\n",
                    tmp_cntr, fltr_lst->qos_filter.ip_version);
    netmgr_log_low("[%d] IPV6 Flow Label = %d\n",
                     tmp_cntr,
                    (int)fltr_lst->qos_filter.filter_desc.ipv6_flow_label);
    netmgr_log_low("[%d] IPV6 Src addr = %s\n",
                    tmp_cntr,
                    fltr_lst->qos_filter.filter_desc.ipv6_src_addr.ipv6_ip_addr);
    netmgr_log_low("[%d] IPV6 Src addr frefix len = %d\n",
                    tmp_cntr,
                    fltr_lst->qos_filter.filter_desc.ipv6_src_addr.ipv6_filter_prefix_len);
    netmgr_log_low("[%d] IPV6 Dest addr = %s\n",
                    tmp_cntr,
                    fltr_lst->qos_filter.filter_desc.ipv6_dest_addr.ipv6_ip_addr);
    netmgr_log_low("[%d] IPV6 Dest addr frefix len = %d\n",
                    tmp_cntr,
                    fltr_lst->qos_filter.filter_desc.ipv6_dest_addr.ipv6_filter_prefix_len);
    netmgr_log_low("[%d] IPV6 Traffic class Value = %d, Mask %d\n",
                    tmp_cntr,
                    fltr_lst->qos_filter.filter_desc.ipv6_traffic_class.traffic_class_value,
                    fltr_lst->qos_filter.filter_desc.ipv6_traffic_class.traffic_class_mask);
    netmgr_log_high("[%d] Protocol = %d\n",
                    tmp_cntr, fltr_lst->qos_filter.filter_desc.protocol);
    netmgr_log_low("[%d] TOS Value = %d TOS Mask %d \n",
                    tmp_cntr,
                    fltr_lst->qos_filter.filter_desc.tos.tos_value,
                    fltr_lst->qos_filter.filter_desc.tos.tos_mask);
    netmgr_log_low("[%d] TCP SRC start port = %d, Range %d\n",
                    tmp_cntr,
                    fltr_lst->qos_filter.filter_desc.tcp_src_ports.start_port,
                    fltr_lst->qos_filter.filter_desc.tcp_src_ports.range);
    netmgr_log_low("[%d] TCP Dest start port = %d, Range %d\n",
                    tmp_cntr,
                    fltr_lst->qos_filter.filter_desc.tcp_dest_ports.start_port,
                    fltr_lst->qos_filter.filter_desc.tcp_dest_ports.range);
    netmgr_log_low("[%d] UDP SRC start port = %d, Range %d\n",
                    tmp_cntr,
                    fltr_lst->qos_filter.filter_desc.udp_src_ports.start_port,
                    fltr_lst->qos_filter.filter_desc.udp_src_ports.range);
    netmgr_log_low("[%d] UDP Dest start port = %d, Range %d\n",
                    tmp_cntr,
                    fltr_lst->qos_filter.filter_desc.udp_dest_ports.start_port,
                    fltr_lst->qos_filter.filter_desc.udp_dest_ports.range);
    netmgr_log_low("[%d] Transport SRC start port = %d, Range %d\n",
                    tmp_cntr,
                    fltr_lst->qos_filter.filter_desc.transport_src_ports.start_port,
                    fltr_lst->qos_filter.filter_desc.transport_src_ports.range);
    netmgr_log_low("[%d] Transport Dest start port = %d, Range %d\n",
                    tmp_cntr,
                    fltr_lst->qos_filter.filter_desc.transport_dest_ports.start_port,
                    fltr_lst->qos_filter.filter_desc.transport_dest_ports.range);
    netmgr_log_low("[%d] IPV4 SRC Addr  = %d, subnet %d\n",
                    tmp_cntr,
                    (int)fltr_lst->qos_filter.filter_desc.src_addr.ipv4_ip_addr,
                    (int)fltr_lst->qos_filter.filter_desc.src_addr.ipv4_subnet_mask);
    netmgr_log_low("[%d] IPV4 DEST Addr  = %d, subnet %d\n",
                    tmp_cntr,
                    (int)fltr_lst->qos_filter.filter_desc.dest_addr.ipv4_ip_addr,
                    (int)fltr_lst->qos_filter.filter_desc.dest_addr.ipv4_subnet_mask);
  }
}

/*===========================================================================
  FUNCTION  netmgr_tc_get_filter_index
===========================================================================*/
/*!
@brief
  Loops ovet the filter_data list and returns the index of the filter with
a precedence value passed.

@return
  int - Index in the list.

@note

  - Dependencies
    - netmgr_tc_cfg.links[ link ].filter_list should be initialized.

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL int
netmgr_tc_get_filter_index
(
  int    link,
  uint32 flow_id,
  uint32 rule_id,
  uint32 precedence,
  uint32 ip_version,
  int  * list_index
)
{
  ds_dll_el_t *iter = NULL;
  netmgr_tc_filter_data *filter_data;
  int index = 0;

  (void) link;
  (void) flow_id;
  (void) rule_id;

  if ( ! netmgr_tc_cfg.filter_list )
  {
    netmgr_log_err("netmgr_tc_get_filter_index: filter_list is un-initialized !");
    return -1;
  }

  *list_index = 0;
  iter = netmgr_tc_cfg.filter_list->next;

  while ( iter )
  {
    filter_data = (netmgr_tc_filter_data *)ds_dll_data(iter);
    if ((filter_data->ip_version == ip_version) && (filter_data->precedence > precedence))
      break;
    iter = iter->next;
    if (filter_data->ip_version == ip_version)
      index++;
    (*list_index)++;
  }

  netmgr_log_high("%s: Found index %d, list index %d", __func__, index, *list_index);

  return index;
}

/*===========================================================================
  FUNCTION  netmgr_tc_flow_alloc
===========================================================================*/
/*!
@brief
  Allocate dynamic memory for flow info buffer

@return
  netmgr_tc_flow_info_t * - pointer to allocated buffer on success,
                            NULL otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
netmgr_tc_flow_info_t *
netmgr_tc_flow_alloc( void )
{
  netmgr_tc_flow_info_t * flow_buf = NULL;

  NETMGR_LOG_FUNC_ENTRY;

  if((flow_buf = netmgr_malloc(sizeof(netmgr_tc_flow_info_t))) == NULL ) {
    netmgr_log_err("netmgr_tc_flow_alloc: netmgr_malloc failed\n");
  }

  NETMGR_LOG_FUNC_EXIT;

  /* Return ptr to buffer, or NULL if none available */
  return flow_buf;
}

/*===========================================================================
  FUNCTION  netmgr_tc_flow_free
===========================================================================*/
/*!
@brief
  Release dynamic memory for flow info buffer

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
void
netmgr_tc_flow_free
(
  netmgr_tc_flow_info_t **flow_info
)
{
  NETMGR_LOG_FUNC_ENTRY;
  NETMGR_ASSERT(flow_info);
  NETMGR_ASSERT(*flow_info);

  netmgr_free((*flow_info)->class_handle);
  netmgr_free((*flow_info)->tcp_ack_class);
  netmgr_free(*flow_info);
  *flow_info = NULL;

  NETMGR_LOG_FUNC_EXIT;
  return;
}

/*===========================================================================
  FUNCTION  netmgr_tc_filter_alloc
===========================================================================*/
/*!
@brief
  Allocate dynamic memory for flow info buffer

@return
  netmgr_tc_filter_data * - pointer to allocated buffer on success,
                            NULL otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL netmgr_tc_filter_data *
netmgr_tc_filter_alloc( void )
{
  netmgr_tc_filter_data * filter_buf = NULL;

  NETMGR_LOG_FUNC_ENTRY;

  if((filter_buf = netmgr_malloc(sizeof(netmgr_tc_filter_data))) == NULL ) {
    netmgr_log_err("netmgr_tc_filter_data: netmgr_malloc failed\n");
  }

  NETMGR_LOG_FUNC_EXIT;

  /* Return ptr to buffer, or NULL if none available */
  return filter_buf;
}

/*===========================================================================
  FUNCTION  netmgr_tc_handle_info_alloc
===========================================================================*/
/*!
@brief
  Allocate dynamic memory for handle info buffer

@return
  netmgr_tc_handle_info_t * - pointer to allocated buffer on success,
                              NULL otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
netmgr_tc_handle_info_t*
netmgr_tc_handle_info_alloc()
{
  netmgr_tc_handle_info_t* handle = NULL;

  NETMGR_LOG_FUNC_ENTRY;

  if ((handle = netmgr_malloc(sizeof(netmgr_tc_handle_info_t))) == NULL)
  {
    netmgr_log_err("netmgr_tc_handle_info_alloc: netmgr_malloc failed\n");
  }
  else
  {
    memset(handle, 0, sizeof(netmgr_tc_handle_info_t));
  }

  NETMGR_LOG_FUNC_EXIT;
  return handle;
}

/*===========================================================================
  FUNCTION  netmgr_tc_match_flows
===========================================================================*/
/*!
@brief
  Compares flow objects to determine if they match.  Used by
  linked-list search route.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
long int
netmgr_tc_match_flows
(
  const void * first,
  const void * second
)
{
  /* Return zero when the flows match, nonzero when they do not*/
  netmgr_tc_flow_info_t * in_data = (netmgr_tc_flow_info_t *)first;
  netmgr_tc_flow_info_t * list_data = (netmgr_tc_flow_info_t *)second;

  if (in_data == NULL || list_data == NULL)
  {
    return -1;
  }

  if ((list_data->qos_flow.flow_id == in_data->qos_flow.flow_id) &&
      (list_data->qos_flow.ip_ver == in_data->qos_flow.ip_ver))
  {
    return 0;
  }

  return 1;
}

/*===========================================================================
  FUNCTION  netmgr_tc_match_filter_data
===========================================================================*/
/*!
@brief
  Compares filter objects to determine if they match.  Used by
  linked-list search route.

@return
  0 if comparision succeeds else -1.

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL long int
netmgr_tc_match_filter_data
(
  const void * first,
  const void * second
)
{
  netmgr_log_high("netmgr_tc_match_filter_data: enter\n");

  netmgr_log_high("first flow handle=0x%08x\n",
                 (unsigned int)((netmgr_tc_filter_data *)first)->flow_id);

  netmgr_log_high("second flow handle=0x%08x\n",
                 (unsigned int)((netmgr_tc_filter_data *)second)->flow_id);

  if( (((netmgr_tc_filter_data *)first)->link == ((netmgr_tc_filter_data *)second)->link) &&
      (((netmgr_tc_filter_data *)first)->flow_id == ((netmgr_tc_filter_data *)second)->flow_id) &&
      (((netmgr_tc_filter_data *)first)->rule_id == ((netmgr_tc_filter_data *)second)->rule_id) &&
      (((netmgr_tc_filter_data *)first)->precedence == ((netmgr_tc_filter_data *)second)->precedence) &&
      (((netmgr_tc_filter_data *)first)->ip_version == ((netmgr_tc_filter_data *)second)->ip_version))
  {
    netmgr_log_high("netmgr_tc_match_filter_data: success\n");
    return 0;
  }
  return -1;
}

/*===========================================================================
  FUNCTION  netmgr_tc_create_root_qdisc
===========================================================================*/
/*!
@brief
  Create root queue discipline for specified link.
  The HTB qdisc is used for shaping and scheduling capabilities.

@return
  int - NETMGR_SUCCESS on successful operation,
        NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL int
netmgr_tc_create_root_qdisc
(
  int                       link,
  netmgr_tc_handle_info_t * class_handle
)
{
  char cmd[NETMGR_TC_MAX_COMMAND_LENGTH];
  int result =  NETMGR_FAILURE;
  netmgr_tc_handle_info_t * root_handle = NULL;
  const char *dev_name = NULL;

  NETMGR_LOG_FUNC_ENTRY;

  /* Configure the object handles */
  root_handle = netmgr_tc_handle_info_alloc();
  if( !root_handle )
  {
    netmgr_log_err("failed to allocate root handle for link %d", link);
    goto error;
  }

  dev_name = netmgr_kif_get_name(link);
  if(NULL == dev_name)
  {
    netmgr_log_err("%s(): unable to determine name for link=%d\n", __func__, link);
    goto error;
  }

  root_handle->major = NETMGR_TC_QDISC_ROOT_MAJOR;
  root_handle->minor = 0;

  class_handle->major = root_handle->major;
  class_handle->minor = 1;

  /* Create root qdisc on network interface */
  std_strlprintf(cmd, sizeof(cmd),
                 "tc qdisc add dev %s root handle %d:0 htb default %d",
                 dev_name,
                 NETMGR_TC_QDISC_ROOT_MAJOR,
                 NETMGR_TC_QDISC_DEFAULT_CLASS);

  result = netmgr_ds_exec(cmd, NULL);

  if (DS_EXEC_OK != result)
  {
    netmgr_log_err("failed to create root qdisc for link %d", link);
    goto error;
  }

  netmgr_tc_cfg.links[link].root_qdisc = root_handle;

  NETMGR_LOG_FUNC_EXIT;
  return NETMGR_SUCCESS;

error:
  netmgr_free( root_handle );
  NETMGR_LOG_FUNC_EXIT;
  return NETMGR_FAILURE;
}

/*===========================================================================
  FUNCTION  netmgr_tc_create_leaf_qdisc
===========================================================================*/
/*!
@brief
  Create leaf queue discipline for specified class link.
  The prio qdisc is used for scheduling capabilities.

@return
  int - NETMGR_SUCCESS on successful operation,
        NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL int
netmgr_tc_create_leaf_qdisc
(
  int       link,
  netmgr_tc_handle_info_t* parent
)
{
  char cmd[NETMGR_TC_MAX_COMMAND_LENGTH];
  int major, minor;
  int result = NETMGR_FAILURE;
  const char *dev_name = NULL;

  NETMGR_LOG_FUNC_ENTRY;
  dev_name = netmgr_kif_get_name(link);
  if (NULL == dev_name)
  {
    netmgr_log_err("%s(): unable to determine name for link=%d\n", __func__, link);
    goto bail;
  }

  snprintf(cmd, sizeof(cmd),"%d",parent->major);
  major = strtol(cmd, NULL, 16);
  snprintf(cmd, sizeof(cmd),"%d",parent->minor);
  minor = strtol(cmd, NULL, 16);
  snprintf(cmd, sizeof(cmd),
          "tc qdisc add dev %s parent %d:%d handle %d:0 prio flow enable",
           dev_name,
           parent->major,  /* parent major */
           parent->minor,  /* parent minor */
           parent->minor); /* qdisc major is minor of parent */
  if ( major < 0 | minor < 0)
  {
    netmgr_log_err("%s(): invalid handle maj:%d min:%d\n",
                   __func__, major, minor);
    goto bail;
  }


  if (netmgr_main_cfg.rtnetlink_tc_enabled)
  {
#ifdef NETMGR_NL_ENABLED
    if (netmgr_rtnetlink_flow_control(dev_name,           /* Device */
                                      major << 16 | minor,/* Parent */
                                      minor << 16,        /* Handle */
                                      NETMGR_FLOW_ENABLE,
                                      NETMGR_ADD_NETLINK)== NETMGR_SUCCESS)
    {
      netmgr_log_med("%s Successful execution cmd: \"%s\"\n",
                     __func__, cmd);
      return NETMGR_SUCCESS;
    }
    netmgr_log_err("%s Error netlink qdisc reverting to legacy tc utility:\n",
                   __func__);
    (void) netmgr_ds_exec(cmd, NULL);
#endif
  }
  else
  {
    (void) netmgr_ds_exec(cmd, NULL);
  }
  /* Ignore this error since it is possible that tc may not have the patch for flow control. */
  result = NETMGR_SUCCESS;

bail:
  NETMGR_LOG_FUNC_EXIT;
  return result;
}

/*===========================================================================
  FUNCTION  netmgr_tc_create_class
===========================================================================*/
/*!
@brief
  Create internal class for specified link root qdisc.
  The HTB classs is used for shaping capabilities.

@return
  int - NETMGR_SUCCESS on successful operation,
        NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL int
netmgr_tc_create_class
(
  int          link,
  netmgr_tc_handle_info_t* parent,
  uint8        priority,
  uint64       datarate,
  netmgr_tc_handle_info_t* handle,
  boolean is_default_flow
)
{
  int result =  NETMGR_FAILURE;
  char cmd[NETMGR_TC_MAX_COMMAND_LENGTH];
  const char *dev_name = NULL;
  uint64 base_rate;
  unsigned long burst;


  NETMGR_LOG_FUNC_ENTRY;
  NETMGR_ASSERT(handle);

  base_rate = netmgr_main_get_tc_ul_baserate();
  burst = netmgr_main_get_tc_ul_burst();
  /* Default to old behavior if the configuration is not set */
  if(base_rate == 0)
  {
    base_rate = netmgr_main_cfg.tc_ul_ceiling_mbps * 1000000;
  }
  if(datarate == 0)
  {
    datarate = NETMGR_TC_DEFAULT_DATARATE;
  }
  if(burst == 0)
  {
    burst = NETMGR_TC_DEFAULT_BURST;
  }

  dev_name = netmgr_kif_get_name(link);
  if(NULL == dev_name)
  {
    netmgr_log_err("%s(): unable to determine name for link=%d\n", __func__, link);
    goto bail;
  }

  if( NULL == parent )
  {
    snprintf(cmd, sizeof(cmd),
             "tc class add dev %s parent root classid %d:%d "
             "htb prio %d rate %" PRIu64 "bit ceil %umbit burst %lu",
             dev_name,
             handle->major,              /* class major id */
             handle->minor,              /* class minor id */
             priority,                   /* priority level */
             base_rate,                  /* rate in bps */
             netmgr_main_cfg.tc_ul_ceiling_mbps, /* ceiling in bps */
             burst);                     /* burst in bits */
  }
  else
  {
    snprintf(cmd, sizeof(cmd),
             "tc class add dev %s parent %d:%d classid %d:%d "
             "htb prio %d rate %" PRIu64 "bit ceil %umbit burst %lu",
             dev_name,
             parent->major,              /* parent major id */
             parent->minor,              /* parent minor id */
             handle->major,              /* class major id */
             handle->minor,              /* class minor id */
             priority,                   /* priority level */
             ((is_default_flow == TRUE)? base_rate : datarate), /* rate in bps */
             netmgr_main_cfg.tc_ul_ceiling_mbps, /* ceiling in bps */
             burst);                     /* burst in bits */
  }

  result = netmgr_ds_exec(cmd, NULL);
  if(result != DS_EXEC_OK)
    result = NETMGR_FAILURE;

bail:
  NETMGR_LOG_FUNC_EXIT;
  return result;
}

/*===========================================================================
  FUNCTION  netmgr_tc_create_filter
===========================================================================*/
/*!
@brief
  Create filter (classifier) on specified link root qdisc to route
  packets MARKed with flow handle to designated class.  MARK is set
  using IPTables mangle rule to assign flowid to skb->mark so VED can
  access value.

@return
  int - NETMGR_SUCCESS on successful operation,
        NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL int
netmgr_tc_create_filter
(
  int                             link,
  netmgr_tc_flow_info_t         * flow_info,
  const netmgr_tc_handle_info_t * parent,
  const netmgr_tc_handle_info_t * class_info
)
{
  int result =  NETMGR_FAILURE;
  char cmd[NETMGR_TC_MAX_COMMAND_LENGTH];
  const char *dev_name = NULL;

  NETMGR_LOG_FUNC_ENTRY;
  dev_name = netmgr_kif_get_name(link);
  if(NULL == dev_name)
  {
    netmgr_log_err("%s(): unable to determine name for link=%d\n", __func__, link);
    goto bail;
  }

  NETMGR_ASSERT( flow_info );
  NETMGR_ASSERT( parent );
  NETMGR_ASSERT( class_info );

  snprintf(cmd, sizeof(cmd),
           "tc filter add dev %s parent %d:%d "
           "prio %d protocol ip handle 0x%x fw classid %d:%d",
           dev_name,
           parent->major,
           parent->minor,
           NETMGR_TC_FILTER_PRIORITY,
           flow_info->qos_flow.flow_id,
           class_info->major,
           class_info->minor);

  result = netmgr_ds_exec(cmd, NULL);
  if (result != DS_EXEC_OK)
    result = NETMGR_FAILURE;

bail:
  NETMGR_LOG_FUNC_EXIT;
  return result;
}



/*===========================================================================
  FUNCTION  netmgr_tc_create_tcp_ack_filter
===========================================================================*/
/*!
@brief
  Create u32 filter on specified link root qdisc to route packets to
  designated class. The filters ensures that only TCP ACKs are filtered
  and sent through the prioritized class.

@return
  int - NETMGR_SUCCESS on successful operation,
        NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL int
netmgr_tc_create_tcp_ack_filter
(
  int                             link,
  netmgr_tc_flow_info_t         * flow_info,
  const netmgr_tc_handle_info_t * parent,
  const netmgr_tc_handle_info_t * class_info
)
{
  int result =  NETMGR_SUCCESS;
  char cmd[NETMGR_TC_MAX_COMMAND_LENGTH];
  const char *dev_name = NULL;

  NETMGR_LOG_FUNC_ENTRY;

  NETMGR_ASSERT( flow_info );
  NETMGR_ASSERT( parent );
  NETMGR_ASSERT( class_info );

  dev_name = netmgr_kif_get_name(link);
  if(NULL == dev_name)
  {
    netmgr_log_err("%s(): unable to determine name for link=%d\n", __func__, link);
    goto bail;
  }

  if (flow_info->qos_flow.ip_ver == NETMGR_QOS_IP_VERSION_4)
  {
    /* The following uses a u32 filter to filter TCP ACKs.
       ip protocol 6 ensures that it is an TCP packet.
       0x05 0x0f at 0 ensures that the header lengh is 20 bytes
       0x0000 0xffc0 at 2 ensures that the IP packet totoal lengh is smaller than 64 bytes.
       0x10 0xff at 33 ensures that the TCP ACK bit is set.
   */
    snprintf(cmd, sizeof(cmd),
             "tc filter add dev %s parent %d:%d "
             "protocol ip prio %d u32 "
             "match ip protocol 6 0xff "
             "match mark 0x%x 0xffffffff "
             "match u8 0x05 0x0f at 0 match u16 0x0000 0xffc0 at 2 "
             "match u8 0x10 0xff at 33 flowid %d:%d",
             dev_name,
             parent->major,
             parent->minor,
             NETMGR_TC_TCP_ACK_FILTER_PRIORITY,
             flow_info->qos_flow.flow_id,
             class_info->major,
             class_info->minor);
  }
  else
  {
    /* The following uses a u32 filter to filter TCP ACKs.
       ip6 protocol 6 ensures that it is an TCP packet.
       0x0020 0xffff at 4 ensures that payload is 32 bytes (20 + 12 options)
       0x8010 0xffff at 32 ensures that TCP header is 32 bytes and TCP ACK is set.
   */
    snprintf(cmd, sizeof(cmd),
             "tc filter add dev %s parent %d:%d "
             "protocol ipv6 prio %d u32 "
             "match ip6 protocol 6 0xff "
             "match mark 0x%x 0xffffffff "
             "match u16 0x0020 0xffff at 4 "
             "match u16 0x8010 0xffff at 52 flowid %d:%d",
             dev_name,
             parent->major,
             parent->minor,
             NETMGR_TC_TCP_ACK_FILTER_PRIORITY,
             flow_info->qos_flow.flow_id,
             class_info->major,
             class_info->minor);
  }

  result = netmgr_ds_exec(cmd, NULL);
  if (result != DS_EXEC_OK)
    result = NETMGR_FAILURE;

bail:
  NETMGR_LOG_FUNC_EXIT;
  return result;
}

/*===========================================================================
  FUNCTION  netmgr_tc_delete_flow_chain_ref
===========================================================================*/
/*!
@brief
 Remove the qos custom flow chain reference(s) from qos postrouting chain

@return
  int

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int
netmgr_tc_delete_flow_chain_ref
(
  netmgr_tc_flow_info_t *flow_info,
  int       link
)
{
  int i, idx = 0;
  char cmd[NETMGR_TC_MAX_COMMAND_LENGTH];
  char *ipt_tool;
  ds_dll_el_t* tail = NULL;
  ds_dll_el_t* node = NULL;
  const void* dummy = NULL;
  netmgr_tc_filter_data filter_data;

  NETMGR_LOG_FUNC_ENTRY;

  for (i = 0; i < flow_info->qos_flow.num_filter; i++)
  {
    filter_data.link = link;
    filter_data.flow_id = flow_info->qos_flow.flow_id;
    filter_data.rule_id = i;
    filter_data.precedence = flow_info->qos_flow.filter_list[i].qos_filter.filter_desc.precedence;
    filter_data.ip_version = flow_info->qos_flow.filter_list[i].qos_filter.ip_version;

    node = ds_dll_next(netmgr_tc_cfg.filter_list, &dummy);
    while (NULL != node)
    {
      tail = node;
      if (filter_data.ip_version == ((netmgr_tc_filter_data *)dummy)->ip_version)
      {
        /* idx is seperate for v4 and v6 */
        idx++;
      }
      if (netmgr_tc_match_filter_data(&filter_data, dummy) == 0) {
        /* Remove this flow chain ref from qos postrouting chain */
        memset( cmd, 0x0, sizeof(cmd) );
        ipt_tool = (NETMGR_QOS_IP_VERSION_6 == filter_data.ip_version)?
                NETMGR_TC_IPTABLES_TOOL_V6 : NETMGR_TC_IPTABLES_TOOL_V4;
        snprintf(cmd, sizeof(cmd), "%s " NETMGR_IPTABLES_FLAGS " -t mangle -D %s %d",
                 ipt_tool,
                 NETMGR_TC_CHAIN_NAME_QOS_FILTER_POSTROUTING,
                 idx);
        if (netmgr_ds_exec(cmd, NULL) != DS_EXEC_OK)
        {
          netmgr_log_high("Failed to delete chain ref with link=%d, flow_id=0x%08x,"
                          " rule_id=%d, precedence=%d, ip_version=%d",
                          filter_data.link,
                          filter_data.flow_id,
                          filter_data.rule_id,
                          filter_data.precedence,
                          filter_data.ip_version);
        }
      }
      node = ds_dll_next(tail, &dummy);
    }

    /* filter_data index for next rule updated after ds_dll_delete. reset idx for next search */
    idx = 0;
    node = ds_dll_delete(netmgr_tc_cfg.filter_list,
                  &tail,
                  (void *) &filter_data,
                  netmgr_tc_match_filter_data);

    if (NULL == node)
    {
      netmgr_log_err("Failed to delete filter with link=%d, flow_id=0x%08x, rule_id=%d,"
                     " precedence=%d, ip_version=%d",
                     filter_data.link,
                     filter_data.flow_id,
                     filter_data.rule_id,
                     filter_data.precedence,
                     filter_data.ip_version);
    }
    else
    {
      netmgr_free((void *)ds_dll_data(node));
      ds_dll_free(node);
    }
  }

  NETMGR_LOG_FUNC_EXIT;
  return NETMGR_SUCCESS;
}

/*===========================================================================
  FUNCTION  netmgr_tc_delete_flow_chain
===========================================================================*/
/*!
@brief
  Delete the qos custom flow chain.

@return

  int - NETMGR_SUCCESS on successful operation,
        NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL int
netmgr_tc_delete_flow_chain
(
  int    link,
  uint32 flow_id,
  uint32 ip_family
)
{
  char * ipt_tool = NULL;
  char cmd[NETMGR_TC_MAX_COMMAND_LENGTH];
  int rc = NETMGR_SUCCESS;
#ifdef FEATURE_RECOVERY_ENABLED
  int file_idx = -1;
  file_info_t file_info_v4;
  file_info_t file_info_v6;
#endif /* FEATURE_RECOVERY_ENABLED */

  NETMGR_LOG_FUNC_ENTRY;

  ipt_tool = (NETMGR_QOS_IP_VERSION_6 == ip_family)?
                NETMGR_TC_IPTABLES_TOOL_V6 : NETMGR_TC_IPTABLES_TOOL_V4;

  memset( cmd, 0x0, sizeof(cmd) );
  snprintf(cmd, sizeof(cmd), "%s " NETMGR_IPTABLES_FLAGS " -t mangle -F %s_0x%08x.%d",
          ipt_tool,
          NETMGR_TC_CHAIN_NAME_PREFIX,
          flow_id,
          link);

  if (netmgr_ds_exec(cmd, NULL) != DS_EXEC_OK)
  {
    netmgr_log_err("failed to flush chain for flow 0x%1x in link %d", flow_id, link);
    goto bail;
  }

  memset( cmd, 0x0, sizeof(cmd) );
  snprintf(cmd, sizeof(cmd), "%s " NETMGR_IPTABLES_FLAGS " -t mangle -X %s_0x%08x.%d",
          ipt_tool,
          NETMGR_TC_CHAIN_NAME_PREFIX,
          flow_id,
          link);

  if (netmgr_ds_exec(cmd, NULL) != DS_EXEC_OK)
  {
    netmgr_log_err("failed to delete chain for flow 0x%1x in link %d", flow_id, link);
    goto bail;
  }

#ifdef FEATURE_RECOVERY_ENABLED
  if (NETMGR_QOS_IP_VERSION_4 == ip_family)
  {
    memset(&file_info_v4, 0, sizeof(file_info_t));
    file_info_v4.is_valid  = TRUE;
    file_info_v4.info_type = TYPE_BEARER_INFO;
    file_info_v4.recovery_info.bearer_info.link = link;
    file_info_v4.recovery_info.bearer_info.ip_family = AF_INET;
    file_info_v4.recovery_info.bearer_info.flow_id = flow_id;
    file_info_v4.recovery_info.bearer_info.filter_mask &= ~(((uint8_t) 1) << link);

    if (NETMGR_SUCCESS != netmgr_recovery_file_write_entry(&file_info_v4))
    {
      netmgr_log_err("%s(): failed to write V4 qos bearer info in"
                     "recovery file -- link = %d , flow_id = 0x%x",
                      __func__, link, flow_id);
    }
  }
  else
  {
    memset(&file_info_v6, 0, sizeof(file_info_t));
    file_info_v6.is_valid  = TRUE;
    file_info_v6.info_type = TYPE_BEARER_INFO;
    file_info_v6.recovery_info.bearer_info.link = link;
    file_info_v6.recovery_info.bearer_info.ip_family = AF_INET6;
    file_info_v6.recovery_info.bearer_info.flow_id = flow_id;
    file_info_v6.recovery_info.bearer_info.filter_mask &= ~(((uint8_t) 1) << link);

    if (NETMGR_SUCCESS != netmgr_recovery_file_write_entry(&file_info_v6))
    {
      netmgr_log_err("%s(): failed to write V6 qos bearer info in"
                     "recovery file -- link = %d , flow_id = 0x%x",
                     __func__, link, flow_id);
    }
  }

  /* Check if the recovery information needs to be invalidated */
  memset(&file_info_v4, 0, sizeof(file_info_t));
  file_info_v4.is_valid  = TRUE;
  file_info_v4.info_type = TYPE_BEARER_INFO;
  file_info_v4.recovery_info.bearer_info.link = link;
  file_info_v4.recovery_info.bearer_info.ip_family = AF_INET;
  file_info_v4.recovery_info.bearer_info.flow_id = flow_id;

  rc = netmgr_recovery_file_find_entry(&file_info_v4, &file_idx);
  if (NETMGR_SUCCESS == rc)
  {
    /* All the dedicated bearers have been cleaned up, we can invalidate the entry */
    if (file_info_v4.is_valid == TRUE && file_info_v4.recovery_info.bearer_info.filter_mask == 0)
    {
      file_info_v4.is_valid  = FALSE;
      if (NETMGR_SUCCESS != netmgr_recovery_file_write_entry(&file_info_v4))
      {
        netmgr_log_err("%s(): failed to invalidate V4 qos bearer info in"
                       "recovery file -- link = %d , flow_id = 0x%x",
                       __func__, link, flow_id);
      }
    }
  }

  memset(&file_info_v6, 0, sizeof(file_info_t));
  file_info_v6.is_valid  = TRUE;
  file_info_v6.info_type = TYPE_BEARER_INFO;
  file_info_v6.recovery_info.bearer_info.link = link;
  file_info_v6.recovery_info.bearer_info.ip_family = AF_INET6;
  file_info_v6.recovery_info.bearer_info.flow_id = flow_id;

  rc = netmgr_recovery_file_find_entry(&file_info_v6, &file_idx);
  if (NETMGR_SUCCESS == rc)
  {
    if (file_info_v6.is_valid == TRUE && file_info_v6.recovery_info.bearer_info.filter_mask == 0)
    {
      /* All the dedicated bearers have been cleaned up, we can invalidate the entry */
      file_info_v6.is_valid  = FALSE;
      if (NETMGR_SUCCESS != netmgr_recovery_file_write_entry(&file_info_v6))
      {
        netmgr_log_err("%s(): failed to invalidate V6 qos bearer info in"
                       "recovery file -- link = %d , flow_id = 0x%x",
                       __func__, link, flow_id);
      }
    }
  }
#endif /* FEATURE_RECOVERY_ENABLED */

  NETMGR_LOG_FUNC_EXIT;
  return NETMGR_SUCCESS;

bail:
  NETMGR_LOG_FUNC_EXIT;
  return NETMGR_FAILURE;
}

/*===========================================================================
  FUNCTION  netmgr_tc_create_flow_chain
===========================================================================*/
/*!
@brief
  Create a qos custom flow chain, append MARK rule.

@return
  int - NETMGR_SUCCESS on successful operation,
        NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int
netmgr_tc_create_flow_chain
(
  int    link,
  uint32 flow_id,
  uint32 ip_family,
  uint32 mark,
  uint32 tcp_ack_mark
)
{
  int result = NETMGR_FAILURE;
  char * ipt_tool = NULL;
  char cmd[NETMGR_TC_MAX_COMMAND_LENGTH];
  int rc = NETMGR_SUCCESS;
  int idx = 0;
#ifdef FEATURE_RECOVERY_ENABLED
  file_info_t file_info;
#endif /* FEATURE_RECOVERY_ENABLED */

  NETMGR_LOG_FUNC_ENTRY;

  ipt_tool = (NETMGR_QOS_IP_VERSION_6 == ip_family)?
                NETMGR_TC_IPTABLES_TOOL_V6 : NETMGR_TC_IPTABLES_TOOL_V4;

  memset( cmd, 0x0, sizeof(cmd) );
  snprintf(cmd, sizeof(cmd), "%s " NETMGR_IPTABLES_FLAGS " -t mangle -N %s_0x%08x.%d",
          ipt_tool,
          NETMGR_TC_CHAIN_NAME_PREFIX,
          flow_id,
          link);

  if (netmgr_ds_exec(cmd, NULL) != DS_EXEC_OK)
  {
    netmgr_log_err("failed to create chain for flow 0x%1x in link %d", flow_id, link);
    goto bail;
  }

  memset( cmd, 0x0, sizeof(cmd) );
  snprintf(cmd, sizeof(cmd), "%s " NETMGR_IPTABLES_FLAGS " -t mangle -A %s_0x%08x.%d -j MARK --set-mark 0x%08x",
          ipt_tool,
          NETMGR_TC_CHAIN_NAME_PREFIX,
          flow_id,
          link,
          mark);

  if (netmgr_ds_exec(cmd, NULL) != DS_EXEC_OK)
  {
    netmgr_log_err("failed to append MARK to chain for flow 0x%1x in link %d", flow_id, link);
    goto bail;
  }

  /* Mark TCP acks */
  if (tcp_ack_mark != mark)
  {
    memset( cmd, 0x0, sizeof(cmd) );

    if (NETMGR_QOS_IP_VERSION_4 == ip_family)
    {
      snprintf(cmd, sizeof(cmd), "%s " NETMGR_IPTABLES_FLAGS " -t mangle -A %s_0x%08x.%d "
                                 "-p tcp --tcp-flags ACK ACK -m length --length 40:64 "
                                 "-j MARK --set-mark 0x%08x",
          ipt_tool,
          NETMGR_TC_CHAIN_NAME_PREFIX,
          flow_id,
          link,
          tcp_ack_mark);
    }
    else
    {
      snprintf(cmd, sizeof(cmd), "%s " NETMGR_IPTABLES_FLAGS " -t mangle -A %s_0x%08x.%d "
                                 "-p tcp --tcp-flags ACK ACK -m length --length 60:72 "
                                 "-j MARK --set-mark 0x%08x",
          ipt_tool,
          NETMGR_TC_CHAIN_NAME_PREFIX,
          flow_id,
          link,
          tcp_ack_mark);
    }

    if (netmgr_ds_exec(cmd, NULL) != DS_EXEC_OK)
    {
      netmgr_log_err("failed to append MARK to chain for flow 0x%1x in link %d", flow_id, link);
      goto bail;
    }
  }

#ifdef FEATURE_RECOVERY_ENABLED
  /* Save flow id and mark this rule index as valid in the recovery file */
  memset(&file_info, 0, sizeof(file_info_t));
  file_info.is_valid  = TRUE;
  file_info.info_type = TYPE_BEARER_INFO;
  file_info.recovery_info.bearer_info.link = link;
  file_info.recovery_info.bearer_info.ip_family 
      = (NETMGR_QOS_IP_VERSION_6 == ip_family) ? AF_INET6 : AF_INET;
  file_info.recovery_info.bearer_info.flow_id = flow_id;

  rc = netmgr_recovery_file_find_entry(&file_info, &idx);
  if (NETMGR_SUCCESS != rc)
  {
    /* If the entry does not exist then we need to set the flow_id and
       set the filter_mask bitmask */
    file_info.recovery_info.bearer_info.filter_mask = ((uint8_t) 1) << link;
  }
  else
  {
    /* Update the filter_mask in the entry */
    file_info.recovery_info.bearer_info.filter_mask |= ((uint8_t) 1) << link;
  }

  if (NETMGR_SUCCESS != netmgr_recovery_file_write_entry(&file_info))
  {
    if (NETMGR_QOS_IP_VERSION_4 == ip_family)
    {
      netmgr_log_err("%s(): failed to save V4 qos bearer info to recovery file "
                     "link = %d , flow_id = 0x%x , link = %d",
                     __func__, link, flow_id, link);
    }
    else
    {
      netmgr_log_err("%s(): failed to save V6 qos bearer info to recovery file "
                     "link = %d , flow_id = 0x%x , link = %d",
                     __func__, link, flow_id, link);
    }
  }
#endif /* FEATURE_RECOVERY_ENABLED */

  result = NETMGR_SUCCESS;

bail:
  NETMGR_LOG_FUNC_EXIT;
  return result;

}

/*===========================================================================
  FUNCTION  netmgr_tc_find_flow_info
===========================================================================*/
/*!
@brief
  Search through flow list to find the specified flow info structure.

@return
  flow info if found, NULL otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
netmgr_tc_flow_info_t*
netmgr_tc_find_flow_info
(
  int link,
  uint32 flow_id,
  netmgr_qos_ip_version_type ip_ver
)
{
  ds_dll_el_t* node = NULL;
  netmgr_tc_flow_info_t flow_buf;
  netmgr_tc_flow_info_t* flow_info = NULL;

  NETMGR_LOG_FUNC_ENTRY;

  flow_buf.qos_flow.flow_id = flow_id;
  flow_buf.qos_flow.ip_ver = ip_ver;
  node = ds_dll_search(netmgr_tc_cfg.links[link].flow_list,
                       &flow_buf,
                       netmgr_tc_match_flows);

  if (node != NULL)
  {
    flow_info = (netmgr_tc_flow_info_t*) ds_dll_data(node);
  }

  if (flow_info == NULL)
  {
    netmgr_log_high("%s: flow 0x%x ip %d not found",
                                __func__, flow_id, ip_ver);
  }

  NETMGR_LOG_FUNC_EXIT;
  return flow_info;
}

/*===========================================================================
  FUNCTION  netmgr_tc_iptables_v4_rule_helper
===========================================================================*/
/*!
@brief
  Helper function to create the iptables V4 rule

@return
  int - NETMGR_SUCCESS on successful operation,
        NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL int
netmgr_tc_iptables_v4_rule_helper
(
  int link,
  uint32 flow_id,
  uint32 rule_id,
  const netmgr_qos_granted_filter_data_type * qos_filter,
  int   protocol
)
{
  char cmd[NETMGR_TC_MAX_COMMAND_LENGTH];
  unsigned int length;
  const unsigned int maxlength = sizeof(cmd)-1;
  int range;
  int start_port;
  const char *dev_name = NULL;

  /* Build the new rule string and add to the parent chain*/
  memset( cmd, 0x0, sizeof(cmd) );
  length = (unsigned int)snprintf(cmd, maxlength, "%s " NETMGR_IPTABLES_FLAGS " -t mangle -I %s %d -g %s_0x%08x.%d",
                                  NETMGR_TC_IPTABLES_TOOL_V4,
                                  NETMGR_TC_CHAIN_NAME_QOS_FILTER_POSTROUTING,
                                  rule_id,
                                  NETMGR_TC_CHAIN_NAME_PREFIX,
                                  flow_id,
                                  link);

  if( length >= maxlength )
    goto error;

  dev_name = netmgr_kif_get_name(netmgr_tc_cfg.link_array[link].link_id);

  if(NULL == dev_name)
  {
    netmgr_log_err("Dev name is NULL -returning NETMGR_FAILURE\n");
    goto error;
  }

  /* Process address and subnet mask in network-byte order */
  if (qos_filter->qos_filter.filter_desc.param_mask & NETMGR_QOS_FILTER_PARAM_SRC_ADDR)
  {
    length += (unsigned int)snprintf(cmd + length, maxlength - length, " --src %d.%d.%d.%d/%d.%d.%d.%d",
                       (unsigned int) (qos_filter->qos_filter.filter_desc.src_addr.ipv4_ip_addr & 0xFF000000) >> 24,
                       (unsigned int) (qos_filter->qos_filter.filter_desc.src_addr.ipv4_ip_addr & 0x00FF0000) >> 16,
                       (unsigned int) (qos_filter->qos_filter.filter_desc.src_addr.ipv4_ip_addr & 0x0000FF00) >> 8,
                       (unsigned int) (qos_filter->qos_filter.filter_desc.src_addr.ipv4_ip_addr & 0x000000FF) >> 0,
                       (unsigned int) (qos_filter->qos_filter.filter_desc.src_addr.ipv4_subnet_mask & 0xFF000000) >> 24,
                       (unsigned int) (qos_filter->qos_filter.filter_desc.src_addr.ipv4_subnet_mask & 0x00FF0000) >> 16,
                       (unsigned int) (qos_filter->qos_filter.filter_desc.src_addr.ipv4_subnet_mask & 0x0000FF00) >> 8,
                       (unsigned int) (qos_filter->qos_filter.filter_desc.src_addr.ipv4_subnet_mask & 0x000000FF) >> 0 );

    if( length >= maxlength )
      goto error;
  }

  if (qos_filter->qos_filter.filter_desc.param_mask & NETMGR_QOS_FILTER_PARAM_DEST_ADDR)
  {
    length += (unsigned int)snprintf(cmd + length, maxlength - length, " --dst %d.%d.%d.%d/%d.%d.%d.%d",
                       (unsigned int) (qos_filter->qos_filter.filter_desc.dest_addr.ipv4_ip_addr & 0xFF000000) >> 24,
                       (unsigned int) (qos_filter->qos_filter.filter_desc.dest_addr.ipv4_ip_addr & 0x00FF0000) >> 16,
                       (unsigned int) (qos_filter->qos_filter.filter_desc.dest_addr.ipv4_ip_addr & 0x0000FF00) >> 8,
                       (unsigned int) (qos_filter->qos_filter.filter_desc.dest_addr.ipv4_ip_addr & 0x000000FF) >> 0,
                       (unsigned int) (qos_filter->qos_filter.filter_desc.dest_addr.ipv4_subnet_mask & 0xFF000000) >> 24,
                       (unsigned int) (qos_filter->qos_filter.filter_desc.dest_addr.ipv4_subnet_mask & 0x00FF0000) >> 16,
                       (unsigned int) (qos_filter->qos_filter.filter_desc.dest_addr.ipv4_subnet_mask & 0x0000FF00) >> 8,
                       (unsigned int) (qos_filter->qos_filter.filter_desc.dest_addr.ipv4_subnet_mask & 0x000000FF) >> 0 );

    if( length >= maxlength )
      goto error;
  }

  if (protocol != NETMGR_TC_INVALID_PROTO)
  {
    length += (unsigned int)snprintf(cmd + length, maxlength - length, " --protocol %d", protocol);

    if( length >= maxlength )
      goto error;
  }

  if (qos_filter->qos_filter.filter_desc.param_mask & NETMGR_QOS_FILTER_PARAM_TOS)
  {
    length += (unsigned int)snprintf(cmd + length, maxlength - length, " -m tos --tos %d",
                       qos_filter->qos_filter.filter_desc.tos.tos_value);

    if( length >= maxlength )
      goto error;
  }

  do
  {
    if (qos_filter->qos_filter.filter_desc.param_mask & NETMGR_QOS_FILTER_PARAM_TCP_SRC_PORTS)
    {
      range = qos_filter->qos_filter.filter_desc.tcp_src_ports.range;
      start_port = qos_filter ->qos_filter.filter_desc.tcp_src_ports.start_port;
    }
    else if (qos_filter->qos_filter.filter_desc.param_mask & NETMGR_QOS_FILTER_PARAM_UDP_SRC_PORTS)
    {
      range = qos_filter->qos_filter.filter_desc.udp_src_ports.range;
      start_port = qos_filter->qos_filter.filter_desc.udp_src_ports.start_port;
    }
    else if (qos_filter->qos_filter.filter_desc.param_mask &
               NETMGR_QOS_FILTER_PARAM_TRANSPORT_SRC_PORTS)
    {
      range = qos_filter->qos_filter.filter_desc.transport_src_ports.range;
      start_port = qos_filter->qos_filter.filter_desc.transport_src_ports.start_port;
    }
    else
    {
      break;
    }

    if (range == 0)
      length += (unsigned int)snprintf(cmd + length, maxlength - length, " --sport %d",
                         start_port);
    else
      length += (unsigned int)snprintf(cmd + length, maxlength - length, " --sport %d:%d",
                         start_port,
                         start_port + range);

    if( length >= maxlength )
      goto error;
  } while (0);

  do
  {
    if (qos_filter->qos_filter.filter_desc.param_mask & NETMGR_QOS_FILTER_PARAM_TCP_DEST_PORTS)
    {
      range = qos_filter->qos_filter.filter_desc.tcp_dest_ports.range;
      start_port = qos_filter->qos_filter.filter_desc.tcp_dest_ports.start_port;
    }
    else if (qos_filter->qos_filter.filter_desc.param_mask & NETMGR_QOS_FILTER_PARAM_UDP_DEST_PORTS)
    {
      range = qos_filter->qos_filter.filter_desc.udp_dest_ports.range;
      start_port = qos_filter->qos_filter.filter_desc.udp_dest_ports.start_port;
    }
    else if (qos_filter->qos_filter.filter_desc.param_mask &
               NETMGR_QOS_FILTER_PARAM_TRANSPORT_DEST_PORTS)
    {
      range = qos_filter->qos_filter.filter_desc.transport_dest_ports.range;
      start_port = qos_filter->qos_filter.filter_desc.transport_dest_ports.start_port;
    }
    else
    {
      break;
    }

    if (range == 0)
      length += (unsigned int)snprintf(cmd + length,
                         maxlength - length,
                         " --dport %d",
                         start_port);
    else
      length += (unsigned int)snprintf(cmd + length,
                         maxlength - length,
                         " --dport %d:%d",
                         start_port,
                         start_port + range);

    if( length >= maxlength )
      goto error;
  } while (0);

  length += (unsigned int)snprintf(cmd + length,
                     maxlength - length,
                     " -o %s",
                     dev_name);

  if (netmgr_ds_exec(cmd, NULL) != DS_EXEC_OK)
  {
    goto error;
  }

  NETMGR_LOG_FUNC_EXIT;
  return NETMGR_SUCCESS;

error:
  NETMGR_LOG_FUNC_EXIT;
  return NETMGR_FAILURE;
}


/*===========================================================================
  FUNCTION  netmgr_tc_create_iptables_rule_v4
===========================================================================*/
/*!
@brief
  Create netfilter mangle table POSTROUTING rule for QoS IPv4 filter. This will
  assign skb->mark field to teh specified flow ID for packet
  forwarding via traffic control classifier(filter).

@return
  int - NETMGR_SUCCESS on successful operation,
        NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL int
netmgr_tc_create_iptables_rule_v4
(
  int link,
  uint32 flow_id,
  uint32 rule_id,
  const netmgr_qos_granted_filter_data_type * qos_filter
)
{
  NETMGR_LOG_FUNC_ENTRY;
  NETMGR_ASSERT( qos_filter );

  if (!(qos_filter->qos_filter.filter_desc.param_mask &
        NETMGR_QOS_FILTER_PARAM_TRANS_PROTOCOL))
  {
    if ((qos_filter->qos_filter.filter_desc.param_mask &
         (NETMGR_QOS_FILTER_PARAM_TRANSPORT_DEST_PORTS |
           NETMGR_QOS_FILTER_PARAM_TRANSPORT_SRC_PORTS)))
    {
      if (netmgr_tc_iptables_v4_rule_helper( link,
                                             flow_id,
                                             rule_id,
                                             qos_filter,
                                             NETMGR_TC_TCP_PROTO_IANA_NUM) != NETMGR_SUCCESS)
      {
        netmgr_log_err("create_iptables_rule_v4: Error creating a V4 TCP filter");
        goto error;
      }

      if (netmgr_tc_iptables_v4_rule_helper( link,
                                             flow_id,
                                             rule_id,
                                             qos_filter,
                                             NETMGR_TC_UDP_PROTO_IANA_NUM) != NETMGR_SUCCESS)
      {
        netmgr_log_err("create_iptables_rule_v4: Error creating a V4 TCP filter");
        goto error;
      }
    }
    else
    {
      if (netmgr_tc_iptables_v4_rule_helper( link,
                                             flow_id,
                                             rule_id,
                                             qos_filter,
                                             NETMGR_TC_INVALID_PROTO) != NETMGR_SUCCESS)
      {
        netmgr_log_err("create_iptables_rule_v4: Error creating a V4 TCP filter");
        goto error;
      }
    }
  }
  else
  {
    if (netmgr_tc_iptables_v4_rule_helper( link,
                                           flow_id,
                                           rule_id,
                                           qos_filter,
                                           qos_filter->qos_filter.filter_desc.protocol) != NETMGR_SUCCESS)
    {
      netmgr_log_err("create_iptables_rule_v4: Error creating a V4 filter");
      goto error;
    }
  }

  NETMGR_LOG_FUNC_EXIT;
  return NETMGR_SUCCESS;

error:
  NETMGR_LOG_FUNC_EXIT;
  return NETMGR_FAILURE;
}

/*===========================================================================
  FUNCTION  netmgr_tc_iptables_v6_rule_helper
===========================================================================*/
/*!
@brief
  Helper function to create the iptables V6 rule

@return
  int - NETMGR_SUCCESS on successful operation,
        NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL int
netmgr_tc_iptables_v6_rule_helper
(
  int link,
  uint32 flow_id,
  uint32 rule_id,
  const netmgr_qos_granted_filter_data_type * qos_filter,
  int protocol
)
{

  char cmd[NETMGR_TC_MAX_COMMAND_LENGTH];
  unsigned int length;
  const unsigned int maxlength = sizeof(cmd)-1;
  int range;
  int start_port;
  const char *dev_name = NULL;

  NETMGR_LOG_FUNC_ENTRY;
  NETMGR_ASSERT( qos_filter );

  /* Build the new rule string */
  memset( cmd, 0x0, sizeof(cmd) );
  length = (unsigned int)snprintf(cmd, maxlength, "%s " NETMGR_IPTABLES_FLAGS " -t mangle -I %s %d -g %s_0x%08x.%d",
                                  NETMGR_TC_IPTABLES_TOOL_V6,
                                  NETMGR_TC_CHAIN_NAME_QOS_FILTER_POSTROUTING,
                                  rule_id,
                                  NETMGR_TC_CHAIN_NAME_PREFIX,
                                  flow_id,
                                  link);

  if( length >= maxlength )
    goto error;

  dev_name = netmgr_kif_get_name(netmgr_tc_cfg.link_array[link].link_id);
  if(NULL == dev_name)
  {
    netmgr_log_err("Dev name is NULL -returning NETMGR_FAILURE\n");
    goto error;
  }

  /* Process address and subnet mask in network-byte order */
  if (qos_filter->qos_filter.filter_desc.param_mask & NETMGR_QOS_FILTER_PARAM_IPV6_SRC_ADDR)
  {
    length += (unsigned int)snprintf(cmd + length, maxlength - length,
                       " --src %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x/%d",
                       qos_filter->qos_filter.filter_desc.ipv6_src_addr.ipv6_ip_addr[0],
                       qos_filter->qos_filter.filter_desc.ipv6_src_addr.ipv6_ip_addr[1],
                       qos_filter->qos_filter.filter_desc.ipv6_src_addr.ipv6_ip_addr[2],
                       qos_filter->qos_filter.filter_desc.ipv6_src_addr.ipv6_ip_addr[3],
                       qos_filter->qos_filter.filter_desc.ipv6_src_addr.ipv6_ip_addr[4],
                       qos_filter->qos_filter.filter_desc.ipv6_src_addr.ipv6_ip_addr[5],
                       qos_filter->qos_filter.filter_desc.ipv6_src_addr.ipv6_ip_addr[6],
                       qos_filter->qos_filter.filter_desc.ipv6_src_addr.ipv6_ip_addr[7],
                       qos_filter->qos_filter.filter_desc.ipv6_src_addr.ipv6_ip_addr[8],
                       qos_filter->qos_filter.filter_desc.ipv6_src_addr.ipv6_ip_addr[9],
                       qos_filter->qos_filter.filter_desc.ipv6_src_addr.ipv6_ip_addr[10],
                       qos_filter->qos_filter.filter_desc.ipv6_src_addr.ipv6_ip_addr[11],
                       qos_filter->qos_filter.filter_desc.ipv6_src_addr.ipv6_ip_addr[12],
                       qos_filter->qos_filter.filter_desc.ipv6_src_addr.ipv6_ip_addr[13],
                       qos_filter->qos_filter.filter_desc.ipv6_src_addr.ipv6_ip_addr[14],
                       qos_filter->qos_filter.filter_desc.ipv6_src_addr.ipv6_ip_addr[15],
                       qos_filter->qos_filter.filter_desc.ipv6_src_addr.ipv6_filter_prefix_len);

    if( length >= maxlength )
      goto error;
  }

  if (qos_filter->qos_filter.filter_desc.param_mask & NETMGR_QOS_FILTER_PARAM_IPV6_DEST_ADDR)
  {
    length += (unsigned int)snprintf(cmd + length, maxlength - length,
                       " --dst %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x/%d",
                       qos_filter->qos_filter.filter_desc.ipv6_dest_addr.ipv6_ip_addr[0],
                       qos_filter->qos_filter.filter_desc.ipv6_dest_addr.ipv6_ip_addr[1],
                       qos_filter->qos_filter.filter_desc.ipv6_dest_addr.ipv6_ip_addr[2],
                       qos_filter->qos_filter.filter_desc.ipv6_dest_addr.ipv6_ip_addr[3],
                       qos_filter->qos_filter.filter_desc.ipv6_dest_addr.ipv6_ip_addr[4],
                       qos_filter->qos_filter.filter_desc.ipv6_dest_addr.ipv6_ip_addr[5],
                       qos_filter->qos_filter.filter_desc.ipv6_dest_addr.ipv6_ip_addr[6],
                       qos_filter->qos_filter.filter_desc.ipv6_dest_addr.ipv6_ip_addr[7],
                       qos_filter->qos_filter.filter_desc.ipv6_dest_addr.ipv6_ip_addr[8],
                       qos_filter->qos_filter.filter_desc.ipv6_dest_addr.ipv6_ip_addr[9],
                       qos_filter->qos_filter.filter_desc.ipv6_dest_addr.ipv6_ip_addr[10],
                       qos_filter->qos_filter.filter_desc.ipv6_dest_addr.ipv6_ip_addr[11],
                       qos_filter->qos_filter.filter_desc.ipv6_dest_addr.ipv6_ip_addr[12],
                       qos_filter->qos_filter.filter_desc.ipv6_dest_addr.ipv6_ip_addr[13],
                       qos_filter->qos_filter.filter_desc.ipv6_dest_addr.ipv6_ip_addr[14],
                       qos_filter->qos_filter.filter_desc.ipv6_dest_addr.ipv6_ip_addr[15],
                       qos_filter->qos_filter.filter_desc.ipv6_dest_addr.ipv6_filter_prefix_len);

    if( length >= maxlength )
      goto error;
  }

  if (protocol != NETMGR_TC_INVALID_PROTO)
  {
    length += (unsigned int)snprintf(cmd + length, maxlength - length, " --protocol %d", protocol);

    if( length >= maxlength )
      goto error;
  }

  do
  {
    if (qos_filter->qos_filter.filter_desc.param_mask & NETMGR_QOS_FILTER_PARAM_TCP_SRC_PORTS)
    {
      range = qos_filter->qos_filter.filter_desc.tcp_src_ports.range;
      start_port = qos_filter->qos_filter.filter_desc.tcp_src_ports.start_port;
    }
    else if (qos_filter->qos_filter.filter_desc.param_mask & NETMGR_QOS_FILTER_PARAM_UDP_SRC_PORTS)
    {
      range = qos_filter->qos_filter.filter_desc.udp_src_ports.range;
      start_port = qos_filter->qos_filter.filter_desc.udp_src_ports.start_port;
    }
    else if (qos_filter->qos_filter.filter_desc.param_mask &
               NETMGR_QOS_FILTER_PARAM_TRANSPORT_SRC_PORTS)
    {
      range = qos_filter->qos_filter.filter_desc.transport_src_ports.range;
      start_port = qos_filter->qos_filter.filter_desc.transport_src_ports.start_port;
    }
    else
    {
      break;
    }

    if (range == 0)
      length += (unsigned int)snprintf(cmd + length, maxlength - length, " --sport %d",
                         start_port);
    else
      length += (unsigned int)snprintf(cmd + length, maxlength - length, " --sport %d:%d",
                         start_port,
                         start_port + range);

    if( length >= maxlength )
      goto error;
  } while (0);

  do
  {
    if (qos_filter->qos_filter.filter_desc.param_mask & NETMGR_QOS_FILTER_PARAM_TCP_DEST_PORTS)
    {
      range = qos_filter->qos_filter.filter_desc.tcp_dest_ports.range;
      start_port = qos_filter->qos_filter.filter_desc.tcp_dest_ports.start_port;
    }
    else if (qos_filter->qos_filter.filter_desc.param_mask & NETMGR_QOS_FILTER_PARAM_UDP_DEST_PORTS)
    {
      range = qos_filter->qos_filter.filter_desc.udp_dest_ports.range;
      start_port = qos_filter->qos_filter.filter_desc.udp_dest_ports.start_port;
    }
    else if (qos_filter->qos_filter.filter_desc.param_mask &
                NETMGR_QOS_FILTER_PARAM_TRANSPORT_DEST_PORTS)
    {
      range = qos_filter->qos_filter.filter_desc.transport_dest_ports.range;
      start_port = qos_filter->qos_filter.filter_desc.transport_dest_ports.start_port;
    }
    else
    {
      break;
    }

    if (range == 0)
      length += (unsigned int)snprintf(cmd + length, maxlength - length, " --dport %d",
                         start_port);
    else
      length += (unsigned int)snprintf(cmd + length, maxlength - length, " --dport %d:%d",
                         start_port,
                         start_port + range);

    if( length >= maxlength )
      goto error;
  } while (0);

  length += (unsigned int)snprintf(cmd + length, maxlength - length, " -o %s",dev_name);

  if (netmgr_ds_exec(cmd, NULL) != DS_EXEC_OK)
  {
    goto error;
  }

  NETMGR_LOG_FUNC_EXIT;
  return NETMGR_SUCCESS;

error:
  NETMGR_LOG_FUNC_EXIT;
  return NETMGR_FAILURE;
}

/*===========================================================================
  FUNCTION  netmgr_tc_create_iptables_rule_v6
===========================================================================*/
/*!
@brief
  Create netfilter mangle table POSTROUTING rule for QoS IPv6 filter. This will
  assign skb->mark field to teh specified flow ID for packet
  forwarding via traffic control classifier(filter).

@return
  int - NETMGR_SUCCESS on successful operation,
        NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL int
netmgr_tc_create_iptables_rule_v6
(
  int link,
  uint32 flow_id,
  uint32 rule_id,
  const netmgr_qos_granted_filter_data_type * qos_filter
)
{
  NETMGR_LOG_FUNC_ENTRY;
  NETMGR_ASSERT( qos_filter );

  if (!(qos_filter->qos_filter.filter_desc.param_mask &
        NETMGR_QOS_FILTER_PARAM_TRANS_PROTOCOL))
  {
    if ((qos_filter->qos_filter.filter_desc.param_mask &
         (NETMGR_QOS_FILTER_PARAM_TRANSPORT_DEST_PORTS |
           NETMGR_QOS_FILTER_PARAM_TRANSPORT_SRC_PORTS)))
    {
      if (netmgr_tc_iptables_v6_rule_helper( link,
                                             flow_id,
                                             rule_id,
                                             qos_filter,
                                             NETMGR_TC_TCP_PROTO_IANA_NUM) != NETMGR_SUCCESS)
      {
        netmgr_log_err("create_iptables_rule_v6: Error creating a V6 TCP filter");
        goto error;
      }

      if (netmgr_tc_iptables_v6_rule_helper( link,
                                             flow_id,
                                             rule_id,
                                             qos_filter,
                                             NETMGR_TC_UDP_PROTO_IANA_NUM) != NETMGR_SUCCESS)
      {
        netmgr_log_err("create_iptables_rule_v6: Error creating a V6 TCP filter");
        goto error;
      }
    }
    else
    {
      if (netmgr_tc_iptables_v6_rule_helper( link,
                                             flow_id,
                                             rule_id,
                                             qos_filter,
                                             NETMGR_TC_INVALID_PROTO) != NETMGR_SUCCESS)
      {
        netmgr_log_err("create_iptables_rule_v6: Error creating a V6 TCP filter");
        goto error;
      }
    }
  }
  else
  {
    if (netmgr_tc_iptables_v6_rule_helper( link,
                                           flow_id,
                                           rule_id,
                                           qos_filter,
                                           qos_filter->qos_filter.filter_desc.protocol) != NETMGR_SUCCESS)
    {
      netmgr_log_err("create_iptables_rule_v6: Error creating a V6 filter");
      goto error;
    }
  }

  NETMGR_LOG_FUNC_EXIT;
  return NETMGR_SUCCESS;

error:
  NETMGR_LOG_FUNC_EXIT;
  return NETMGR_FAILURE;
}

/*===========================================================================
  FUNCTION  netmgr_tc_flow_enable
===========================================================================*/
/*!
@brief
  Install the flow traffic control elements.

@return
  int - NETMGR_SUCCESS on successful operation, NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int
netmgr_tc_flow_enable
(
  int                   link,
  netmgr_tc_flow_info_t *flow_info
)
{
  int i;
  int result = NETMGR_FAILURE;
  char * ipt_tool = NULL;
  int filter_index, list_index;
  netmgr_tc_filter_data *filter_data;
  ds_dll_el_t* tail = NULL;
  ds_dll_el_t* node = NULL;
  const void* dummy = NULL;
  NETMGR_LOG_FUNC_ENTRY;

  if (flow_info == NULL)
  {
    netmgr_log_err( "NULL flow info pointer, ignoring\n" );
    return result;
  }

  /* Add references of flow chain per filter rule into the qos postrouting chain in mangle table. */
  for (i = 0; i < flow_info->qos_flow.num_filter; i++)
  {
    if(flow_info->qos_flow.ip_ver != flow_info->qos_flow.filter_list[i].qos_filter.ip_version)
    {
      netmgr_log_low( "ip type mismatch during qos handling, ignoring\n" );
      continue;
    }

    ipt_tool = (NETMGR_QOS_IP_VERSION_6 == flow_info->qos_flow.filter_list[i].qos_filter.ip_version)?
                NETMGR_TC_IPTABLES_TOOL_V6 : NETMGR_TC_IPTABLES_TOOL_V4;

    filter_data = netmgr_tc_filter_alloc();

    if (filter_data == NULL)
    {
      netmgr_log_err( "No Memory\n" );
      goto error;
    }

    filter_data->link = link;
    filter_data->flow_id = flow_info->qos_flow.flow_id;
    filter_data->rule_id = i;
    filter_data->precedence = flow_info->qos_flow.filter_list[i].qos_filter.filter_desc.precedence;
    filter_data->ip_version = flow_info->qos_flow.filter_list[i].qos_filter.ip_version;

    /* Lower the precedence value of a filter, higher is its evaluation order.
       Find an index of the node in the filter_list which has lesser precedence value.
    */
    filter_index = netmgr_tc_get_filter_index(
                                  link,
                                  filter_data->flow_id,
                                  filter_data->rule_id,
                                  filter_data->precedence,
                                  filter_data->ip_version,
                                  &list_index);
    if ( filter_index < 0 )
    {
      netmgr_free(filter_data);
      goto error;
    }

    /* Insert the currnet filter object in the list */
    if ( ( ds_dll_insert(netmgr_tc_cfg.filter_list,
                         NULL,
                         (void *)filter_data,
                         list_index) ) == NULL )
    {
      netmgr_free(filter_data);
      goto error;
    }

    /*Insert the rules in qcom_qos_POSTROUTING chain in the order of increasing
      precedence, so that lower precedence filter match happends first.
      Use (index + 1) as iptables does not allow inserting at index 0 */
    switch( filter_data->ip_version )
    {
      case NETMGR_QOS_IP_VERSION_4:
        result = netmgr_tc_create_iptables_rule_v4( link,flow_info->qos_flow.flow_id,
                                           filter_index + 1,
                                           &flow_info->qos_flow.filter_list[i] );
        break;
      case NETMGR_QOS_IP_VERSION_6:
        result = netmgr_tc_create_iptables_rule_v6( link,flow_info->qos_flow.flow_id,
                                           filter_index + 1,
                                           &flow_info->qos_flow.filter_list[i] );
        break;
      default:
        netmgr_log_err( "unsupported filter IP version[%d]",
                        flow_info->qos_flow.filter_list[i].qos_filter.ip_version );
        break;
    }
    if (result == NETMGR_FAILURE)
    {
      node = ds_dll_next(netmgr_tc_cfg.filter_list, &dummy);
      while (NULL != node)
      {
        tail = node;
        node = ds_dll_next(tail, &dummy);
      }

      node = ds_dll_delete(netmgr_tc_cfg.filter_list,
                      &tail,
                      (void *) filter_data,
                      netmgr_tc_match_filter_data);

      netmgr_free(filter_data);

      goto error;
    }
  }

  flow_info->qos_flow.state = NETMGR_TC_FLOW_ACTIVE;
  result = NETMGR_SUCCESS;

error:
  NETMGR_LOG_FUNC_EXIT;
  return result;
}

/*===========================================================================
  FUNCTION  netmgr_tc_find_mdm_tc_mapping
===========================================================================*/
/*!
@brief
 Finds the mapping between the mdm and tc flow handles.
 tcm_str and tc_flow_hndl must be valid and non NULL.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL void
netmgr_tc_find_mdm_tc_mapping
(
  char      *tcm_str,
  int       flow_info_minor,
  uint32_t  *tc_flow_hndl
)
{
  memset(tcm_str, 0, NETMGR_TC_HANDLE_LEN);
  snprintf(tcm_str,NETMGR_TC_HANDLE_LEN,"%d",flow_info_minor);
  *tc_flow_hndl = (uint32_t)strtoul(tcm_str, NULL, 16);
  *tc_flow_hndl <<= 16;
}

/*===========================================================================
  FUNCTION  netmgr_tc_add_kernel_flow
===========================================================================*/
/*!
@brief
 Adds the flow control elemnets created in the kernel.

@return
  int - NETMGR_SUCCESS on successful operation,
        NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL int
netmgr_tc_add_kernel_flow
(
  int                                link,
  const netmgr_qmi_qos_flow_info_t  *qos_flow
)
{
  int result = NETMGR_FAILURE;
  netmgr_tc_flow_info_t *flow_info = NULL;
  uint32_t mdm_flow_hndl, tc_flow_hndl;
  char tcm_str[NETMGR_TC_HANDLE_LEN];
  uint32_t link_offset;

  NETMGR_LOG_FUNC_ENTRY;

  NETMGR_ASSERT( qos_flow );
  NETMGR_ASSERT(link >= 0 && link < netmgr_tc_cfg.nlink);

  if (netmgr_rmnet_get_device_qmi_offset(&link_offset) != NETMGR_SUCCESS)
  {
    goto error;
  }
  flow_info = netmgr_tc_find_flow_info(link, qos_flow->flow_id, qos_flow->ip_ver);
  if (flow_info)
  {
    mdm_flow_hndl = (uint32_t)qos_flow->flow_id;
    netmgr_tc_find_mdm_tc_mapping(tcm_str,
                                  flow_info->class_handle->minor,
                                  &tc_flow_hndl);
    result = netmgr_rmnet_add_del_flow(link - (int)link_offset,
                                       mdm_flow_hndl,
                                       tc_flow_hndl,
                                       NETMGR_RMNET_ADD_FLOW );
    if (result != NETMGR_SUCCESS)
    {
      netmgr_log_high("%s for link:%u mdm hndl:%u tc hndl:%u",
                      "Unable to add flow info in kernel",
                      link,
                      mdm_flow_hndl,
                      tc_flow_hndl);
    }
    if ((NETMGR_IS_DEFAULT_FLOW(qos_flow->flow_id )) &&
        (netmgr_main_get_tcpackprio_enabled()) &&
        (flow_info->tcp_ack_class != NULL) &&
        (result == NETMGR_SUCCESS))
    {
      netmgr_tc_find_mdm_tc_mapping(tcm_str,
                                    flow_info->tcp_ack_class->minor,
                                    &tc_flow_hndl);
      result = netmgr_rmnet_add_del_flow(link - (int)link_offset,
                                         mdm_flow_hndl,
                                         tc_flow_hndl,
                                         NETMGR_RMNET_ADD_FLOW);
      if (result != NETMGR_SUCCESS)
      {
        netmgr_log_high("%s for link:%u mdm hndl:%u tc hndl:%u",
                        "Unable to add tcp ack prio flow info in kernel",
                        link,
                        mdm_flow_hndl,
                        tc_flow_hndl);
      }
    }
  }

error:
  NETMGR_LOG_FUNC_EXIT;
  return result;
}

/*===========================================================================
  FUNCTION  netmgr_tc_del_kernel_flow
===========================================================================*/
/*!
@brief
 Deletes the flow control elemnets created in the kernel.

@return
  int - NETMGR_SUCCESS on successful operation,
        NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL void
netmgr_tc_del_kernel_flow
(
  int       link,
  const netmgr_qmi_qos_flow_info_t  *qos_flow
)
{
  netmgr_tc_flow_info_t *flow_info = NULL;
  uint32_t mdm_flow_hndl, tc_flow_hndl;
  char tcm_str[NETMGR_TC_HANDLE_LEN];
  uint32_t link_offset;

  NETMGR_LOG_FUNC_ENTRY;

  NETMGR_ASSERT(qos_flow);
  NETMGR_ASSERT(link >= 0 && link < netmgr_tc_cfg.nlink);

  flow_info = netmgr_tc_find_flow_info(link, qos_flow->flow_id, qos_flow->ip_ver);
  if (flow_info == NULL)
  {
    netmgr_log_high("Unable to find flow info to delete in kernel.");
    goto error;
  }
  if (netmgr_rmnet_get_device_qmi_offset(&link_offset) != NETMGR_SUCCESS)
  {
    goto error;
  }
  mdm_flow_hndl = (uint32_t)qos_flow->flow_id;
  netmgr_tc_find_mdm_tc_mapping(tcm_str,
                                flow_info->class_handle->minor,
                                &tc_flow_hndl);
  if (netmgr_rmnet_add_del_flow(link - (int)link_offset,
                                mdm_flow_hndl,
                                tc_flow_hndl,
                                NETMGR_RMNET_DEL_FLOW) != NETMGR_SUCCESS)
  {
    netmgr_log_high("%s for link:%u mdm hndl:%u tc hndl:%u",
                    "Unable to delete flow info in kernel",
                    link,
                    mdm_flow_hndl,
                    tc_flow_hndl);
  }
  if ((NETMGR_IS_DEFAULT_FLOW(qos_flow->flow_id)) &&
       (netmgr_main_get_tcpackprio_enabled()) &&
       (flow_info->tcp_ack_class != NULL))
  {
    netmgr_tc_find_mdm_tc_mapping(tcm_str,
                                  flow_info->tcp_ack_class->minor,
                                  &tc_flow_hndl);
    if (netmgr_rmnet_add_del_flow(link - (int)link_offset,
                                  mdm_flow_hndl,
                                  tc_flow_hndl,
                                  NETMGR_RMNET_DEL_FLOW ) != NETMGR_SUCCESS)
    {
      netmgr_log_high("%s for link:%u mdm hndl:%u tc hndl:%u",
                      "Unable to delete tcp ack prio flow info in kernel",
                      link,
                      mdm_flow_hndl,
                      tc_flow_hndl);
    }
  }

error:
  NETMGR_LOG_FUNC_EXIT;
}

/*===========================================================================
  FUNCTION  netmgr_tc_flow_create
===========================================================================*/
/*!
@brief
 Create flow object and install the traffic control elements.

@return
  int - NETMGR_SUCCESS on successful operation,
        NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL int
netmgr_tc_flow_create
(
  int                                link,
  const netmgr_qmi_qos_flow_info_t  *qos_flow
)
{
  int result;
  netmgr_tc_flow_info_t * flow_info = NULL;
  netmgr_tc_handle_info_t * class_handle = NULL;
  netmgr_tc_handle_info_t * rclass_handle = NULL;

  NETMGR_LOG_FUNC_ENTRY;

  NETMGR_ASSERT( qos_flow );

  /* Allocate a flow info buffer */
  flow_info = netmgr_tc_flow_alloc();
  NETMGR_ASSERT( flow_info );
  memset(flow_info, 0x0, sizeof(netmgr_tc_flow_info_t));

  /* Assign flow attributes */
  flow_info->phys_net_dev = netmgr_main_cfg.phys_net_dev;
  flow_info->link_name = netmgr_kif_get_name(link);
  flow_info->qos_flow = *qos_flow;

  /* Allocate handle for classes */
  class_handle = netmgr_tc_handle_info_alloc();
  if( !class_handle )
  {
    netmgr_log_err("failed to allocate class handle for flow 0x%lx", (unsigned long)qos_flow->flow_id);
    goto error;
  }

  netmgr_tc_flow_filter_dump(qos_flow);

  /* Check for primary flow */
  if( NETMGR_IS_DEFAULT_FLOW( qos_flow->flow_id ) )
  {
    if (netmgr_tc_cfg.links[link].root_class != NULL)
    {
      result = netmgr_tc_create_2nd_default_flow(link, flow_info, class_handle);
      if (result == NETMGR_FAILURE)
      {
        netmgr_log_err("failed to create 2nd tc default flow");
        goto error;
      }
      else
      {
        netmgr_log_high("created 2nd tc default flow");
        goto out;
      }
    }

    rclass_handle = netmgr_tc_handle_info_alloc();
    if( !rclass_handle )
    {
      netmgr_log_err("failed to allocate root class handle for flow 0x%lx", (unsigned long)qos_flow->flow_id);
      goto error;
    }

    /* Install kernel traffic control elements for primary flow */
    /* Create root HTB qdisc */
    result = netmgr_tc_create_root_qdisc(link, rclass_handle);

    if (result == NETMGR_FAILURE)
    {
      netmgr_log_err("error creating root qdisc for flow 0x%lx", (unsigned long)qos_flow->flow_id);
      goto error;
    }

    /* Create root class, using maximum datarate */
    result = netmgr_tc_create_class(link,
                                    NULL,  /* indicates root */
                                    0,     /* priority irrelevant on root */
                                    netmgr_main_cfg.tc_ul_ceiling_mbps,
                                    rclass_handle,
                                    TRUE);

    if (result == NETMGR_FAILURE)
    {
      netmgr_log_err("error creating default class for link %d", link);
      goto error;
    }

    /* Create child class for default flow */
    class_handle->major = rclass_handle->minor;
    NETMGET_TC_ASSIGN_CLASS_MINOR_ID(link, class_handle->minor);

    result = netmgr_tc_create_class(link,
                                    rclass_handle,
                                    qos_flow->priority,
                                    netmgr_main_cfg.tc_ul_ceiling_mbps,
                                    class_handle,
                                    TRUE);
    if (result == NETMGR_FAILURE)
    {
      netmgr_log_err("error creating class for flow 0x%lx", (unsigned long)qos_flow->flow_id);
      goto error;
    }

    netmgr_tc_cfg.links[link].root_class = rclass_handle;
    netmgr_tc_cfg.links[link].default_class = class_handle;
    flow_info->class_handle = class_handle;

    /* Create leaf qdisc */
    result = netmgr_tc_create_leaf_qdisc(link, flow_info->class_handle);

    if (result == NETMGR_FAILURE)
    {
      netmgr_log_err("error creating leaf qdisc for flow 0x%lx", (unsigned long)qos_flow->flow_id);
      netmgr_tc_cfg.links[link].root_class = NULL;
      netmgr_tc_cfg.links[link].default_class = NULL;
      flow_info->class_handle = NULL;
      goto error;
    }

    /* Add flow object to list for this link */
    flow_info->qos_flow.state = NETMGR_TC_FLOW_ACTIVE;
    ds_dll_enq( netmgr_tc_cfg.links[link].flow_list, NULL, (void*)flow_info );

    /* Primary flow does not have classifier/filter, relies only on
       root qdisc default class assignment. */
  }
  else
  {
    /* Check whether root class is non-null */
    if( NULL == netmgr_tc_cfg.links[link].root_class )
    {
      netmgr_log_err("Root class is NULL for flow 0x%lx", (unsigned long)qos_flow->flow_id);
      goto error;
    }

    /* Install kernel traffic control elements for secondary flow */
    class_handle->major = netmgr_tc_cfg.links[link].root_class->minor;
    NETMGET_TC_ASSIGN_CLASS_MINOR_ID(link, class_handle->minor);

    /* Create child class */
    result = netmgr_tc_create_class(link,
                                    netmgr_tc_cfg.links[link].root_class,
                                    qos_flow->priority,
                                    qos_flow->datarate,
                                    class_handle,
                                    FALSE);

    if (result == NETMGR_FAILURE)
    {
      netmgr_log_err("error creating class for flow 0x%lx", (unsigned long)qos_flow->flow_id);
      goto error;
    }

    flow_info->class_handle = class_handle;

    /* Create leaf qdisc */
    result = netmgr_tc_create_leaf_qdisc(link, flow_info->class_handle);

    if (result == NETMGR_FAILURE)
    {
      netmgr_log_err("error creating leaf qdisc for flow 0x%lx", (unsigned long)qos_flow->flow_id);
      flow_info->class_handle = NULL;
      goto error;
    }

    /* direct matching egress traffic to the new class */
    netmgr_tc_create_filter(link,
                            flow_info,
                            netmgr_tc_cfg.links[link].root_qdisc,
                            flow_info->class_handle);

    netmgr_log_high("num_filters: %d",qos_flow->num_filter);

    /* Mark all matching packets with the value of the link */
    if (NETMGR_SUCCESS != netmgr_tc_create_flow_chain(link, qos_flow->flow_id,
                                    qos_flow->filter_list[0].qos_filter.ip_version,
                                    qos_flow->flow_id, qos_flow->flow_id))
    {
      flow_info->class_handle = NULL;
      goto error;
    }
    /* Link the chain into the mangle table */
    if (netmgr_tc_flow_enable( link,flow_info ) != NETMGR_SUCCESS)
    {
      flow_info->class_handle = NULL;
      goto error;
    }

    /* Add flow object to list for this link only if the flow is enabled successfully*/
    ds_dll_enq( netmgr_tc_cfg.links[link].flow_list, NULL, (void*)flow_info );
  }

out:
  if (netmgr_main_get_tcpackprio_enabled() &&
      NETMGR_IS_DEFAULT_FLOW(flow_info->qos_flow.flow_id))
  {
    /* TODO Due to issues in filter delete, create ack class only for default flows */
    netmgr_tc_create_tcp_ack_class(link, flow_info);
  }
  netmgr_cb_runall(NETMGR_QMI_CB_TABLE, NETMGR_QMI_CB_FLOW_ACTIVATE, flow_info);

  NETMGR_LOG_FUNC_EXIT;
  return NETMGR_SUCCESS;

error:
  netmgr_tc_flow_free( &flow_info );
  netmgr_free( class_handle );
  netmgr_free( rclass_handle );
  NETMGR_LOG_FUNC_EXIT;
  return NETMGR_FAILURE;
}

/*===========================================================================
  FUNCTION  netmgr_tc_flow_control_hdlr
===========================================================================*/
/*!
@brief
  Enable/disable packet scheduling from specific leaf qdisc.

@return
  int - NETMGR_SUCCESS on successful operation,
        NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL int
netmgr_tc_flow_control_hdlr
(
  int link,
  const netmgr_qmi_qos_flow_info_t  *qos_flow,
  netmgr_tc_flow_state_t state
)
{
  netmgr_tc_flow_info_t* flow_info;
  int result = NETMGR_SUCCESS;
  char tcm_str[NETMGR_TC_HANDLE_LEN];
  uint32 tcm_handle;
  const char *dev_name = NULL;

  NETMGR_LOG_FUNC_ENTRY;

  NETMGR_ASSERT(qos_flow);
  NETMGR_ASSERT(link >= 0 && link < netmgr_tc_cfg.nlink);

  flow_info = netmgr_tc_find_flow_info(link, qos_flow->flow_id, qos_flow->ip_ver);
  if (flow_info == NULL || flow_info->class_handle == NULL)
  {
    result = NETMGR_FAILURE;
    goto error;
  }

  /* tcm handle is a string representation of a hex (base 16)
   * number that needs to be left shifted by 4 hex digits to
   * derive the handle that is used by kernel */
  memset(tcm_str, 0, NETMGR_TC_HANDLE_LEN);
  snprintf(tcm_str,sizeof(tcm_str),"%d",flow_info->class_handle->minor);
  tcm_handle = (uint32)strtoul(tcm_str, NULL, 16);

  dev_name = netmgr_kif_get_name(link);
  if (NULL == dev_name)
  {
    netmgr_log_err("%s(): unable to determine name for link=%d\n", __func__, link);
    result = NETMGR_FAILURE;
    goto error;
  }

  if (netmgr_main_cfg.rtnetlink_tc_enabled)
  {
#ifdef NETMGR_NL_ENABLED
     if (netmgr_rtnetlink_flow_control(dev_name,
                                      NETMGR_TC_QDISC_ROOT_MAJOR << 16
                                      | tcm_handle,
                                      tcm_handle << 16,
                                      ((NETMGR_TC_FLOW_DISABLED == state)?0:1),
                                      NETMGR_CHANGE_NETLINK) == NETMGR_SUCCESS)
    {
      netmgr_log_med("%s(): tc qdisc message hndl 0x%x success",
                     __func__, tcm_handle);
    }
    else
    {
      netmgr_log_err("%s(): tc qdisc message hndl 0x%x failed %s",
                     __func__, tcm_handle, "falling back to ioctls!");
      netmgr_kif_ifioctl_flow_control(dev_name,
                                      (int)tcm_handle << 16,
                                      (NETMGR_TC_FLOW_DISABLED == state)?0:1);
    }
#endif
  }
  else
  {
       netmgr_kif_ifioctl_flow_control(dev_name,
                                       (int)tcm_handle << 16,
                                       (NETMGR_TC_FLOW_DISABLED == state)?0:1);
  }

  if ((NETMGR_IS_DEFAULT_FLOW( qos_flow->flow_id )) &&
      (netmgr_main_get_tcpackprio_enabled()) &&
      (netmgr_tc_cfg.links[link].default_class == flow_info->class_handle) &&
      (flow_info->tcp_ack_class != NULL))
  {
    memset(tcm_str, 0, NETMGR_TC_HANDLE_LEN);
    snprintf(tcm_str,sizeof(tcm_str),"%d",
             flow_info->tcp_ack_class->minor);
    tcm_handle = (uint32)strtoul(tcm_str, NULL, 16);

    if (netmgr_main_cfg.rtnetlink_tc_enabled)
    {
#ifdef NETMGR_NL_ENABLED

      if (netmgr_rtnetlink_flow_control(dev_name,
                                      NETMGR_TC_QDISC_ROOT_MAJOR << 16
                                      | tcm_handle,
                                      tcm_handle << 16,
                                      ((NETMGR_TC_FLOW_DISABLED == state)?0:1),
                                      NETMGR_CHANGE_NETLINK) == NETMGR_SUCCESS)
      {
        netmgr_log_med("%s(): tc qdisc message hndl 0x%x success",
                       __func__, tcm_handle);
      }
      else
      {
        netmgr_log_med("%s(): tc qdisc message hndl 0x%x failed  %s",
                        __func__, tcm_handle, "falling back to ioctls!");

        netmgr_kif_ifioctl_flow_control(dev_name,
                                        (int)tcm_handle << 16,
                                        (NETMGR_TC_FLOW_DISABLED == state)?0:1);
      }
#endif
    }
    else
    {
      netmgr_kif_ifioctl_flow_control(dev_name,
                                      (int)tcm_handle << 16,
                                      (NETMGR_TC_FLOW_DISABLED == state)?0:1);
    }
  }
error:
  NETMGR_LOG_FUNC_EXIT;
  return result;
}

/*===========================================================================
  FUNCTION  netmgr_tc_filter_delete_hdlr
===========================================================================*/
/*!
@brief
 Remove the iptable filter rules associated with a leaf class of HTB

@return
  int - NETMGR_SUCCESS on successful operation,
        NETMGR_FAILURE otherwise

@note
  The caller is assumed to delete the flow object from  flow_list dll.


  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
void
netmgr_tc_filter_delete_hdlr
(
  netmgr_tc_flow_info_t *flow_info,
  int       link,
  uint32    flow_id
)
{
  NETMGR_LOG_FUNC_ENTRY;

  if (flow_info == NULL)
  {
    netmgr_log_err("filter_delete_hdlr: invalid INPUT parameter received");
    return;
  }

  netmgr_tc_delete_flow_chain_ref(flow_info, link);
  netmgr_tc_delete_flow_chain(link, flow_id,
                              flow_info->qos_flow.filter_list[0].qos_filter.ip_version);

  NETMGR_LOG_FUNC_EXIT;
}

/*===========================================================================
  FUNCTION  netmgr_tc_flow_modify_hdlr
===========================================================================*/
/*!
@brief
 Change the traffic control elements on Modem QoS flow modify.

@return
  int - NETMGR_SUCCESS on successful operation,
        NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL int
netmgr_tc_flow_modify_hdlr
(
  int       link,
  const netmgr_qmi_qos_flow_info_t  *qos_flow
)
{
  char cmd[NETMGR_TC_MAX_COMMAND_LENGTH];
  unsigned int length = 0;
  netmgr_tc_flow_info_t *flow_info = NULL;
  const char *dev_name = NULL;
  int flow_modify_count = 0;
  int result = NETMGR_FAILURE;
  uint8 bearer_id;

  NETMGR_LOG_FUNC_ENTRY;

  NETMGR_ASSERT(link >= 0 && link < netmgr_tc_cfg.nlink);
  NETMGR_ASSERT(qos_flow != NULL);

  NETMGR_LOCK_MUTEX(tc_lock);

  flow_info = netmgr_tc_find_flow_info(link, qos_flow->flow_id, qos_flow->ip_ver);
  netmgr_log_low("%s(): modifying flow for id [%d]", __func__, qos_flow->flow_id);

  if ( netmgr_main_get_skip_buffered_qos_modify() != TRUE)
  {
      netmgr_log_high("Feature to skip buffered QoS modify is not enabled for this target");
  }
  else
  {
      /*Check in cache for how many modify for particular link-flow_id combination
       * have been recieved. If it is more than 1 then netmgr wont process this one
       * and will process only the last one. This is done because modem provides all
       * the filters in all the modify request and netmgr previously blindly applied
       * those filters adding to extra processing delays for other commands.
       */
      pthread_mutex_lock(&(netmgr_qos_modify_cache.modify_mutex));
      flow_modify_count = netmgr_tc_get_rcvd_modify_flow_counter(link, qos_flow->flow_id);
      pthread_mutex_unlock(&(netmgr_qos_modify_cache.modify_mutex));

      if ( flow_modify_count != 1 )
      {
          netmgr_log_med("%s(): not processing this QoS modify request as other requests "
                  "are already [%d] items pending in queue for flow_id [%d]",
                  __func__,
                  flow_modify_count,
                  qos_flow->flow_id);

          result = NETMGR_SUCCESS;
          goto bail;
      }
  }

  netmgr_tc_flow_filter_dump(qos_flow);

  if (flow_info == NULL)
  {
    netmgr_log_err("flow_modify_hdlr: the flow object does not exist");
    goto bail;
  }

  dev_name = netmgr_kif_get_name(link);
  if(NULL == dev_name)
  {
    netmgr_log_err("%s(): unable to determine name for link=%d\n", __func__, link);
    goto bail;
  }

  /*disble flow on the leaf qdisc before modifying parent class*/
  if (NETMGR_SUCCESS != ( netmgr_tc_flow_control_hdlr(link,
                                                      qos_flow,
                                                      NETMGR_TC_FLOW_DISABLED)))
  {
    netmgr_log_err("could not disable exisiting flow");
    goto bail;
  }

  length = (unsigned int)snprintf(cmd,
                    sizeof(cmd),
                    "tc qdisc replace dev %s parent %d:%d classid %d:%d "
                    "htb prio %d rate %lubit ceil %lubit",
                    dev_name,
                    netmgr_tc_cfg.links[link].root_class->major,
                    netmgr_tc_cfg.links[link].root_class->minor,
                    flow_info->class_handle->major,
                    flow_info->class_handle->minor,
                    qos_flow->priority,
                    ((qos_flow->datarate == 0)? NETMGR_TC_DEFAULT_DATARATE :
                       qos_flow->datarate),
                    netmgr_main_cfg.tc_ul_ceiling_mbps);

  if (length >= sizeof(cmd))
  {
    netmgr_log_err("command message truncated;");
    goto bail;
  }

  if (DS_EXEC_OK != (netmgr_ds_exec(cmd, NULL)))
  {
    netmgr_log_err("could not modify the leaf qdisc class");
  }

  netmgr_tc_delete_flow_chain_ref(flow_info, link);

  /* Since the flowID does not change, there is no need to
   * change the tc filter installed at the root
   */
  /*replace old qos spec with the new one*/
  bearer_id = flow_info->qos_flow.bearer_id;
  flow_info->qos_flow = *qos_flow;
  flow_info->qos_flow.bearer_id = bearer_id;

  /* Link the chain into the mangle table */
  if (netmgr_tc_flow_enable(link, flow_info ) != NETMGR_SUCCESS)
  {
    netmgr_log_err("error in linking iptables chain to mangle table");
    goto bail;
  }

  /* If we reach here QOS modification is successul,
   * re-enable the leaf qdisc at this point
   */
  if (NETMGR_SUCCESS != ( netmgr_tc_flow_control_hdlr(link,
                                                      qos_flow,
                                                      NETMGR_TC_FLOW_ACTIVE)))
  {
    netmgr_log_err("could not re-enable flow");
    goto bail;
  }

  NETMGR_UNLOCK_MUTEX(tc_lock);

  result = NETMGR_SUCCESS;

bail:
  if ( netmgr_main_get_skip_buffered_qos_modify() == TRUE)
  {
      pthread_mutex_lock(&(netmgr_qos_modify_cache.modify_mutex));
      if ( netmgr_tc_decr_rcvd_modify_flow_counter(link, qos_flow->flow_id) != NETMGR_SUCCESS)
      {
          netmgr_log_err("%s(): failed at decrementing the modifying flow counter", __func__ );
      }
      pthread_mutex_unlock(&(netmgr_qos_modify_cache.modify_mutex));
  }

  NETMGR_UNLOCK_MUTEX(tc_lock);
  NETMGR_LOG_FUNC_EXIT;
  return result;
}

/*===========================================================================
  FUNCTION  netmgr_tc_flow_activate_hdlr
===========================================================================*/
/*!
@brief
  Install the traffic control elements on Modem QoS flow create/resume.

@return
  int - NETMGR_SUCCESS on successful operation,
        NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL int
netmgr_tc_flow_activate_hdlr
(
  int                                link,
  const netmgr_qmi_qos_flow_info_t  *qos_flow
)
{
  int result = NETMGR_FAILURE;
  netmgr_tc_flow_info_t *flow_info = NULL;

  NETMGR_LOG_FUNC_ENTRY;

  NETMGR_ASSERT( qos_flow );
  NETMGR_ASSERT(link >= 0 && link < netmgr_tc_cfg.nlink);

  NETMGR_LOCK_MUTEX(tc_lock);

  netmgr_log_low( "flow handle=0x%08x\n", (unsigned int)qos_flow->flow_id );
  netmgr_log_low( "flow is_new=%s\n", ((TRUE==qos_flow->is_new)?"TRUE":"FALSE") );
  netmgr_log_low( "flow priority=%d\n", qos_flow->priority );
  netmgr_log_low( "flow datarate=%lu\n", qos_flow->datarate );
  netmgr_log_low( "flow num_filter=%d\n", qos_flow->num_filter );
  netmgr_log_low( "flow filter_list=%p\n", qos_flow->filter_list );
  netmgr_log_low( "flow bearer_id=%d\n", qos_flow->bearer_id );
  netmgr_log_low( "flow ip_ver=%d\n", qos_flow->ip_ver );

  /* Check if default flow is already activated -
   * Could be one of the dual-IP sub call */
  if (NETMGR_IS_DEFAULT_FLOW(qos_flow->flow_id))
  {
    if (netmgr_tc_is_default_flow_activated(link, qos_flow))
    {
      netmgr_log_high("Default flow activated\n");
      result = NETMGR_SUCCESS;
      goto out;
    }
  }
  else
  {
    flow_info = netmgr_tc_find_flow_info(
                  link, qos_flow->flow_id, qos_flow->ip_ver);
  }

  /* Check for newly created flow */
  if( qos_flow->is_new ) {
    /* Check for pre-existing flow ID */
    if( !flow_info )
    {
      /* Create flow object and install QoS measures */
      if ((result = netmgr_tc_flow_create( link, qos_flow )) == NETMGR_SUCCESS)
      {
        if (netmgr_main_get_ibfc_enabled())
        {
          result = netmgr_tc_add_kernel_flow(link, qos_flow);
        }
      }
    }
    else
    {
      /* This may happen if previously a suspend or activated flow event
       * happened. In this case network may be updating either flow or filter
       * or both. Hence we need to modify the flow and filter spec installed
       * in the kernel
       */
      netmgr_log_med( "flow is already active, modifying flow.\n" );
      result = netmgr_tc_flow_modify_hdlr(link,qos_flow);
    }
  }
  else
  {
    if( flow_info )
    {
      /* This may happen if previously a suspend or activated flow event
       * happened. In this case network may be updating either flow or filter
       * or both. Hence we need to modify the flow and filter spec installed
       * in the kernel
      */
      netmgr_log_med( "flow is already active, modifying flow.\n" );
      netmgr_tc_flow_modify_hdlr(link,qos_flow);
    }
    else
    {
      netmgr_log_err( "Cannot find flow object[0x%x], ignoring\n", qos_flow->flow_id );
    }
  }

out:
  NETMGR_UNLOCK_MUTEX(tc_lock);
  NETMGR_LOG_FUNC_EXIT;
  return result;
}

/*===========================================================================
  FUNCTION  netmgr_tc_flow_delete_hdlr
===========================================================================*/
/*!
@brief
 Remove the traffic control elements on Modem QoS flow release.

@return
  int - NETMGR_SUCCESS on successful operation,
        NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL int
netmgr_tc_flow_delete_hdlr
(
  int       link,
  const netmgr_qmi_qos_flow_info_t  *qos_flow
)
{
  char cmd[NETMGR_TC_MAX_COMMAND_LENGTH];
  ds_dll_el_t* node = NULL;
  ds_dll_el_t* tail = NULL;
  netmgr_tc_flow_info_t flow_buf;
  netmgr_tc_flow_info_t* flow_info = NULL;
  const void* dummy = NULL;
  const void* data = NULL;
  int major, minor;
  int result = NETMGR_FAILURE;
  const char * dev_name = NULL;
  ds_dll_el_t* qos_modify_cache_node = NULL;
  ds_dll_el_t* qos_modify_cache_tail = NULL;
  netmgr_qmi_qos_modify_info_t qos_modify_cache_flow_buf;
  netmgr_qmi_qos_modify_info_t* qos_modify_cache_flow_info = NULL;
  const void* qos_modify_cache_data;
  const void* dummy_qos_modify_cache_node;

  NETMGR_LOG_FUNC_ENTRY;

  NETMGR_ASSERT(qos_flow);
  NETMGR_ASSERT(link >= 0 && link < netmgr_tc_cfg.nlink);

  dev_name = netmgr_kif_get_name(link);
  if(NULL == dev_name)
  {
    netmgr_log_err("%s(): unable to determine name for link=%d\n", __func__, link);
    goto done;
  }

  NETMGR_LOCK_MUTEX(tc_lock);

  if (netmgr_main_get_ibfc_enabled())
  {
    netmgr_tc_del_kernel_flow(link, qos_flow);
  }

  if (NETMGR_IS_DEFAULT_FLOW(qos_flow->flow_id))
  {
    /* Empty the flow list */
    netmgr_log_med("Empty the flow list");
    while (NULL != (node = ds_dll_deq(netmgr_tc_cfg.links[link].flow_list, NULL, &data)))
    {
      flow_info = (netmgr_tc_flow_info_t*) (data);
      if (flow_info)
      {
        netmgr_cb_runall(NETMGR_QMI_CB_TABLE, NETMGR_QMI_CB_FLOW_DELETE, flow_info);
        netmgr_log_med("Freeing flow_id=%u ip=%d",
                    flow_info->qos_flow.flow_id, flow_info->qos_flow.ip_ver);
        netmgr_tc_flow_free(&flow_info);
      }

      ds_dll_free(node);
    }

    /* Delete root qdisc removes all classes/filters/qdiscs */
    netmgr_log_med("Delete root qdisc on %s", dev_name);
    snprintf(cmd, sizeof(cmd), "tc qdisc delete dev %s root", dev_name);

    if (netmgr_ds_exec(cmd, NULL) != DS_EXEC_OK)
    {
      netmgr_log_high("Error deleting root qdisc. Ignoring");
    }

    if (netmgr_tc_cfg.links[link].default_class)
    {
      /* Default class handle already freed when freeing flow_info */
      netmgr_tc_cfg.links[link].default_class = NULL;
    }

    if (netmgr_tc_cfg.links[link].default_class2)
    {
      netmgr_tc_cfg.links[link].default_class2 = NULL;
    }

    if (netmgr_tc_cfg.links[link].root_class)
    {
      netmgr_free(netmgr_tc_cfg.links[link].root_class);
      netmgr_tc_cfg.links[link].root_class = NULL;
    }

    if (netmgr_tc_cfg.links[link].root_qdisc)
    {
      netmgr_free(netmgr_tc_cfg.links[link].root_qdisc);
      netmgr_tc_cfg.links[link].root_qdisc = NULL;
    }

    /* Start with the initial MINOR_ID value */
    netmgr_tc_cfg.links[ link ].next_class_minor = NETMGR_TC_CLASS_MINOR_ID_INIT;

    if ( netmgr_main_get_skip_buffered_qos_modify() != TRUE)
    {
        netmgr_log_med("%s(): feature skip_buffered_qos_modify is disabled for this target", __func__);
    }
    else
    {
        /*Empty the QoS Modify Cache*/
        netmgr_log_med("Empty the QoS Modify Cache");
        pthread_mutex_lock(&(netmgr_qos_modify_cache.modify_mutex));
        while(NULL != (qos_modify_cache_node = ds_dll_deq(netmgr_qos_modify_cache.links[link].flow_list, NULL, &qos_modify_cache_data)))
        {
            qos_modify_cache_flow_info = (netmgr_qmi_qos_modify_info_t*) qos_modify_cache_data;

            if(qos_modify_cache_flow_info)
            {
                netmgr_log_med("Freeing QoS modify cache entry for flow_id = 0x%lx", (unsigned long) qos_modify_cache_flow_info->flow_id);

                qos_modify_cache_flow_info->rcvd_qos_modify_counter = 0;
                netmgr_free(qos_modify_cache_flow_info);

                qos_modify_cache_flow_info = NULL;
            }

            ds_dll_free(qos_modify_cache_node);
        }
        pthread_mutex_unlock(&(netmgr_qos_modify_cache.modify_mutex));
    }
    result = NETMGR_SUCCESS;
  }
  else
  {
    /* find the tail of the flow list */
    node = ds_dll_next(netmgr_tc_cfg.links[link].flow_list, &dummy);
    while (NULL != node)
    {
      tail = node;
      node = ds_dll_next(tail, &dummy);
    }

    /* search for the flow object and remove it from the dll */
    flow_buf.qos_flow.flow_id = qos_flow->flow_id;
    flow_buf.qos_flow.ip_ver = qos_flow->ip_ver;
    node = ds_dll_delete(netmgr_tc_cfg.links[link].flow_list,
                         &tail,
                         &flow_buf,
                         netmgr_tc_match_flows);


    if (node == NULL)
    {
      netmgr_log_err("flow %lx not found",(unsigned long) qos_flow->flow_id);
      goto done;
    }

    flow_info = (netmgr_tc_flow_info_t*) ds_dll_data(node);

    if (flow_info == NULL)
    {
      netmgr_log_err("flow %lx data not found",(unsigned long) qos_flow->flow_id);
      goto done;
    }

    netmgr_cb_runall(NETMGR_QMI_CB_TABLE, NETMGR_QMI_CB_FLOW_DELETE, flow_info);

    netmgr_tc_filter_delete_hdlr(flow_info, link, qos_flow->flow_id);

    if(netmgr_tc_cfg.links[link].root_class)
    {
      /* delete the filter */
      snprintf(cmd, sizeof(cmd), "tc filter delete dev %s parent %d:%d "
               "prio %d protocol ip handle 0x%x fw classid %d:%d",
               dev_name,
               NETMGR_TC_QDISC_ROOT_MAJOR,
               NETMGR_TC_QDISC_ROOT_MINOR,
               NETMGR_TC_FILTER_PRIORITY,
               qos_flow->flow_id,
               flow_info->class_handle->major,
               flow_info->class_handle->minor);

      if (netmgr_ds_exec(cmd, NULL) != DS_EXEC_OK)
      {
        netmgr_log_high("Error deleting filter. Ignoring");
      }

      snprintf(cmd, sizeof(cmd),"%d", flow_info->class_handle->major);
      major = strtol(cmd, NULL, 16);
      snprintf(cmd, sizeof(cmd),"%d", flow_info->class_handle->minor);
      minor = strtol(cmd, NULL, 16);
#ifdef NETMGR_NL_ENABLED
      if (netmgr_rtnetlink_flow_control(dev_name, (major << 16) | minor,
                                     minor << 16,
                                     NETMGR_FLOW_DISABLE,
                                     NETMGR_DEL_NETLINK) != NETMGR_SUCCESS)
      {

        netmgr_log_high("tc qdisc msg failed, falling back to legacy ds_exec");
        /* delete the leaf qdisc */


#endif
        snprintf(cmd, sizeof(cmd), "tc qdisc delete dev %s parent %d:%d "
                 "handle %d:0 prio flow enable",
                 dev_name,
                 flow_info->class_handle->major,
                 flow_info->class_handle->minor,
                 flow_info->class_handle->minor);

        if (netmgr_ds_exec(cmd, NULL) != DS_EXEC_OK)
        {
          netmgr_log_high("Error deleting leaf qdisc. Ignoring");
        }
#ifdef NETMGR_NL_ENABLED
      }
      else
      {
        netmgr_log_high("Netlink Qdisc tc msg succesful");
      }
#endif

      /* delete the class */
      snprintf(cmd, sizeof(cmd), "tc class delete dev %s parent %d:%d "
               "classid %d:%d",
               dev_name,
               netmgr_tc_cfg.links[link].root_class->major,
               netmgr_tc_cfg.links[link].root_class->minor,
               flow_info->class_handle->major,
               flow_info->class_handle->minor);

      if (netmgr_ds_exec(cmd, NULL) != DS_EXEC_OK)
      {
        netmgr_log_high("Error deleting class. Ignoring");
      }

      if (netmgr_main_get_tcpackprio_enabled())
      {
        netmgr_tc_delete_tcp_ack_class(link, flow_info);
      }
    }
    else
    {
      netmgr_log_med("root class deleted, skipping heirarchy update");
    }

    netmgr_tc_flow_free(&flow_info);
    ds_dll_free(node);

    if ( netmgr_main_get_skip_buffered_qos_modify() != TRUE)
    {
        netmgr_log_med("%s(): feature skip_buffered_qos_modify is disabled for this target", __func__);
    }
    else
    {
        /*find the tail of the QoS Modify cache*/
        qos_modify_cache_node = ds_dll_next(netmgr_qos_modify_cache.links[link].flow_list, &dummy_qos_modify_cache_node);
        while(NULL != qos_modify_cache_node)
        {
            qos_modify_cache_tail = qos_modify_cache_node;
            qos_modify_cache_node = ds_dll_next(qos_modify_cache_tail,
                                                &dummy_qos_modify_cache_node);
        }

        /*search for the flow object in QoS modify cache and remove it from dll */
        qos_modify_cache_flow_buf.flow_id = qos_flow->flow_id;
        pthread_mutex_lock(&(netmgr_qos_modify_cache.modify_mutex));
        qos_modify_cache_node = ds_dll_delete(netmgr_qos_modify_cache.links[link].flow_list,
                                              &qos_modify_cache_tail,
                                              &qos_modify_cache_flow_buf,
                                              netmgr_tc_match_qos_modify_cache_flows);

        if ( qos_modify_cache_node )
        {
            ds_dll_free(qos_modify_cache_node);
        }
        else
        {
            netmgr_log_err("flow %lx not found in QoS modify cache", qos_flow->flow_id);
        }

        pthread_mutex_unlock(&(netmgr_qos_modify_cache.modify_mutex));
    }
    result = NETMGR_SUCCESS;
  }

done:
  NETMGR_UNLOCK_MUTEX(tc_lock);

  NETMGR_LOG_FUNC_EXIT;
  return result;
}

/*===========================================================================
  FUNCTION  netmgr_tc_flow_update_data_rate_hdlr
===========================================================================*/
/*!
@brief
 Update the traffic control elements based on a configured throughput
 indication.

@return
  int - NETMGR_SUCCESS on successful operation,
        NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int
netmgr_tc_flow_update_data_rate_hdlr
(
  int       link,
  uint64_t  ul_cfg_tput
)
{
  char cmd[NETMGR_TC_MAX_COMMAND_LENGTH];
  ds_dll_el_t* node = NULL;
  netmgr_tc_flow_info_t* flow_info = NULL;
  const void* data = NULL;
  int result = NETMGR_FAILURE;
  const char * dev_name = NULL;
  unsigned long burst;

  NETMGR_LOG_FUNC_ENTRY;
  NETMGR_ASSERT(link >= 0 && link < netmgr_tc_cfg.nlink);

  burst = netmgr_main_get_tc_ul_burst();
  if(burst == 0)
  {
    burst = NETMGR_TC_DEFAULT_BURST;
  }

  dev_name = netmgr_kif_get_name(link);
  if(NULL == dev_name)
  {
    netmgr_log_err("%s(): unable to determine name for link=%d\n", __func__, link);
    goto done;
  }

    NETMGR_LOCK_MUTEX(tc_lock);

    memset(cmd, 0, sizeof(cmd));

    if (netmgr_tc_cfg.links[link].root_class
          && netmgr_tc_cfg.links[link].default_class)
    {
      snprintf(cmd, sizeof(cmd), "tc class change dev %s parent %d:%d classid %d:%d htb prio 7 rate %" PRIu64 "bit ceil %umbit burst %lu",
               dev_name,
               netmgr_tc_cfg.links[link].root_class->major,
               netmgr_tc_cfg.links[link].root_class->minor,
               netmgr_tc_cfg.links[link].default_class->major,
               netmgr_tc_cfg.links[link].default_class->minor,
               ul_cfg_tput,
               netmgr_main_cfg.tc_ul_ceiling_mbps,
               burst);
      if (netmgr_ds_exec(cmd, NULL) != DS_EXEC_OK)
      {
        netmgr_log_high("Error changing default class. Ignoring");
      }
    }

    memset(cmd, 0, sizeof(cmd));
    if (netmgr_tc_cfg.links[link].root_class &&
            netmgr_tc_cfg.links[link].default_class2)
    {
      snprintf(cmd, sizeof(cmd), "tc class change dev %s parent %d:%d classid %d:%d htb prio 7 rate %" PRIu64 "bit ceil %umbit burst %lu",
               dev_name,
               netmgr_tc_cfg.links[link].root_class->major,
               netmgr_tc_cfg.links[link].root_class->minor,
               netmgr_tc_cfg.links[link].default_class2->major,
               netmgr_tc_cfg.links[link].default_class2->minor,
               ul_cfg_tput,
               netmgr_main_cfg.tc_ul_ceiling_mbps,
               burst);
      if (netmgr_ds_exec(cmd, NULL) != DS_EXEC_OK)
      {
        netmgr_log_high("Error changing default class 2. Ignoring");
      }
    }

    if (netmgr_main_get_tcpackprio_enabled() &&
        netmgr_tc_cfg.links[link].root_class)
    {
      node = netmgr_tc_cfg.links[link].flow_list;
      while (node != NULL)
      {
        flow_info = (netmgr_tc_flow_info_t *)ds_dll_data(node);
        if (flow_info && flow_info->tcp_ack_class &&
            NETMGR_IS_DEFAULT_FLOW(flow_info->qos_flow.flow_id))
        {
          memset(cmd, 0, sizeof(cmd));
          snprintf(cmd, sizeof(cmd), "tc class change dev %s parent %d:%d classid %d:%d htb prio 7 rate %" PRIu64 "bit ceil %umbit burst %lu",
                   dev_name,
                   netmgr_tc_cfg.links[link].root_class->major,
                   netmgr_tc_cfg.links[link].root_class->minor,
                   flow_info->tcp_ack_class->major,
                   flow_info->tcp_ack_class->minor,
                   ul_cfg_tput,
                   netmgr_main_cfg.tc_ul_ceiling_mbps,
                   burst);
          if (netmgr_ds_exec(cmd, NULL) != DS_EXEC_OK)
          {
            netmgr_log_high("Error change ack class. Ignoring");
          }
        }
        node = ds_dll_next(node, &data);
      }
    }

    memset(cmd, 0, sizeof(cmd));
    if (netmgr_tc_cfg.links[link].root_class)
    {
      snprintf(cmd, sizeof(cmd), "tc class change dev %s parent root classid %d:%d htb prio 0 rate %" PRIu64 "bit ceil %umbit burst %lu",
               dev_name,
               netmgr_tc_cfg.links[link].root_class->major,
               netmgr_tc_cfg.links[link].root_class->minor,
               ul_cfg_tput,
               netmgr_main_cfg.tc_ul_ceiling_mbps,
               burst);
      if (netmgr_ds_exec(cmd, NULL) != DS_EXEC_OK)
      {
        netmgr_log_high("Error changing root class. Ignoring");
      }
    }

    result = NETMGR_SUCCESS;

done:
  NETMGR_UNLOCK_MUTEX(tc_lock);

  NETMGR_LOG_FUNC_EXIT;
  return result;
}

/*===========================================================================
  FUNCTION  netmgr_tc_link_cleanup
===========================================================================*/
/*!
@brief
  Performs cleanup of flow objects.  Invoked at process termination.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL void
netmgr_tc_link_cleanup
(
  int link
)
{
  char cmd[NETMGR_TC_MAX_COMMAND_LENGTH];
  const char *dev_name = NULL;

  NETMGR_LOG_FUNC_ENTRY;

  dev_name = netmgr_kif_get_name(link);
  if(NULL == dev_name)
  {
    netmgr_log_err("%s(): unable to determine name for link=%d\n", __func__, link);
    goto bail;
  }

  snprintf(cmd, sizeof(cmd), "tc qdisc del dev %s root",
           dev_name);
  (void) netmgr_ds_exec(cmd, NULL);

bail:
  NETMGR_LOG_FUNC_EXIT;
  return;
}

/*===========================================================================
  FUNCTION  netmgr_tc_filter_list_cleanup
===========================================================================*/
/*!
@brief
  Performs cleanup of filter list objects.  Invoked at process termination.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL void
netmgr_tc_filter_list_cleanup( link )
{
  ds_dll_el_t *iter = NULL;
  netmgr_tc_filter_data *filter_data;
  netmgr_tc_flow_info_t* flow_info;

  NETMGR_LOG_FUNC_ENTRY;

  iter = netmgr_tc_cfg.filter_list->next;

  while ( iter )
  {
    filter_data = (netmgr_tc_filter_data *)ds_dll_data(iter);
    if (filter_data->link == link) {
      flow_info = netmgr_tc_find_flow_info(
             link, filter_data->flow_id, filter_data->ip_version);
      netmgr_tc_filter_delete_hdlr(flow_info, link, filter_data->flow_id);
    }
    iter = netmgr_tc_cfg.filter_list->next;
  }
  NETMGR_LOG_FUNC_EXIT;
}

/*===========================================================================
  FUNCTION  netmgr_tc_link_init
===========================================================================*/
/*!
@brief
 Initializes link state information.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL void
netmgr_tc_link_init( int link )
{
  NETMGR_LOG_FUNC_ENTRY;

  NETMGR_LOCK_MUTEX(tc_lock);

  /* Initializing specified interface */
  if( netmgr_tc_cfg.link_array[ link ].enabled )
  {
    memset((void*) &netmgr_tc_cfg.links[ link ], 0x0, sizeof(netmgr_tc_cfg.links[ link ]));

    netmgr_tc_cfg.links[ link ].root_qdisc = NULL;
    netmgr_tc_cfg.links[ link ].next_class_minor = NETMGR_TC_CLASS_MINOR_ID_INIT;
    memset(&netmgr_tc_cfg.links[ link ].mq_info, 0,
           sizeof(netmgr_tc_cfg.links[ link ].mq_info));
    netmgr_tc_cfg.links[ link ].flow_list = ds_dll_init( netmgr_tc_cfg.links[ link ].flow_list );

    netmgr_qos_modify_cache.links[ link ].flow_list = ds_dll_init( netmgr_qos_modify_cache.links[ link ].flow_list );

    if (!netmgr_tc_cfg.filter_list)
    {
      /* filter_list is not per link, only init it if header is not existing */
      netmgr_tc_cfg.filter_list = ds_dll_init( netmgr_tc_cfg.filter_list );
    }
  }

  NETMGR_UNLOCK_MUTEX(tc_lock);

  NETMGR_LOG_FUNC_EXIT;
  return;
}

/*===========================================================================
  FUNCTION  netmgr_tc_reset_link
===========================================================================*/
/*!
@brief
  Reinitialize link data structures on reset command.

@return
  int - NETMGR_SUCCESS on successful operation, NETMGR_FAILURE otherwise.

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL int
netmgr_tc_reset_link( int link )
{
  NETMGR_LOG_FUNC_ENTRY;

  if (!netmgr_main_cfg.kfc_mode)
  {
    /* Cleanup dynamic memory */
    netmgr_tc_link_cleanup( link );
  }

  /* if filter_list is initialized (all links done),
    clean up any link related filter_data in list */
  if (netmgr_tc_cfg.filter_list && netmgr_tc_cfg.is_initialized)
  {
    netmgr_tc_filter_list_cleanup( link );
  }
  /* Reinitialize link data structures */
  netmgr_tc_link_init( link );

  NETMGR_LOG_FUNC_EXIT;
  return NETMGR_SUCCESS;
}


/*===========================================================================
  FUNCTION  netmgr_tc_create_qos_filter_postrouting_mangle_chain
===========================================================================*/
/*!
@brief
 In this new postrouting mangle chain to inserting filter rule chains in
 the order of precedence values of the filter specification.

@return
  NETMGR_SUCCESS, NETMGR_FAILURE

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
static int
netmgr_tc_create_qos_filter_postrouting_mangle_chain
(
  uint32 ip_family
)
{
  char cmd[NETMGR_TC_MAX_COMMAND_LENGTH];
  int length;
  const unsigned int maxlength = sizeof(cmd)-1;
  char * ipt_tool = NULL;

  NETMGR_LOG_FUNC_ENTRY;

  if (ip_family != NETMGR_IPV6_ADDR &&
        ip_family != NETMGR_IPV4_ADDR )
  {
    netmgr_log_err("invalid IP version revceived");
    goto error;
  }

  ipt_tool = (NETMGR_IPV6_ADDR == ip_family)?
                 NETMGR_TC_IPTABLES_TOOL_V6 : NETMGR_TC_IPTABLES_TOOL_V4;

  memset( cmd, 0x0, sizeof(cmd) );
  length = snprintf(cmd,
                    maxlength,
                    "%s " NETMGR_IPTABLES_FLAGS " -t mangle -N %s",
                    ipt_tool,
                    NETMGR_TC_CHAIN_NAME_QOS_FILTER_POSTROUTING);

  if( (unsigned int)length >= maxlength )
    goto error;

  if (netmgr_ds_exec(cmd, NULL) != DS_EXEC_OK)
  {
    netmgr_log_err("failed to create the postrouing chain");
    goto error;
  }

  memset( cmd, 0x0, sizeof(cmd) );
  /*Link the chain to POSTROUTING chain of mangle table*/
  length = snprintf(cmd,
                    maxlength,
                    "%s " NETMGR_IPTABLES_FLAGS " -t mangle -A POSTROUTING -j %s",
                    ipt_tool,
                    NETMGR_TC_CHAIN_NAME_QOS_FILTER_POSTROUTING);

  if( (unsigned int)length >= maxlength )
    goto error;

  if (netmgr_ds_exec(cmd, NULL) != DS_EXEC_OK)
  {
    goto error;
  }

  return NETMGR_SUCCESS;

error:
  return NETMGR_FAILURE;
}

/*===========================================================================
  FUNCTION  netmgr_tc_delete_qos_filter_postrouting_mangle_chain
===========================================================================*/
/*!
@brief
  Deletes the postrouting chain. This should only be called during process
  exit as part of cleanup procedure.

@return
  NETMGR_SUCCESS, NETMGR_FAILURE

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
static void
netmgr_tc_delete_qos_filter_postrouting_mangle_chain
(
  uint32 ip_family
)
{
  char cmd[NETMGR_TC_MAX_COMMAND_LENGTH];
  int length;
  char * ipt_tool = NULL;
  const unsigned int maxlength = sizeof(cmd)-1;

  NETMGR_LOG_FUNC_ENTRY;

  if (ip_family != NETMGR_IPV6_ADDR &&
        ip_family != NETMGR_IPV4_ADDR )
  {
    netmgr_log_err("invalid IP version revceived");
    return;
  }

  ipt_tool = (NETMGR_IPV6_ADDR == ip_family)?
                 NETMGR_TC_IPTABLES_TOOL_V6 : NETMGR_TC_IPTABLES_TOOL_V4;

  memset( cmd, 0x0, sizeof(cmd) );

  /*remove the reference of the chain from the POSTROUTING chain*/
  length = snprintf(cmd,
                    sizeof(cmd),
                    "%s " NETMGR_IPTABLES_FLAGS " -t mangle -D POSTROUTING -j %s",
                    ipt_tool,
                    NETMGR_TC_CHAIN_NAME_QOS_FILTER_POSTROUTING);

  if( (unsigned int)length >= maxlength)
  {
    netmgr_log_err("snprintf buffer overflow");
    return;
  }

  (void) netmgr_ds_exec(cmd, NULL);

  memset( cmd, 0x0, sizeof(cmd) );
  /* Flushing the chain to remove any stale references */
  length = snprintf(cmd,
                    sizeof(cmd),
                    "%s " NETMGR_IPTABLES_FLAGS " -t mangle -F %s",
                    ipt_tool,
                    NETMGR_TC_CHAIN_NAME_QOS_FILTER_POSTROUTING);

  if( (unsigned int)length >= maxlength)
  {
    netmgr_log_err("snprintf buffer overflow");
    return;
  }

  (void) netmgr_ds_exec(cmd, NULL);

  memset( cmd, 0x0, sizeof(cmd) );
  /* Deleting the chain deletes all the rules.
   This chain must not be referenced by any other rules. */
  length = snprintf(cmd,
                    sizeof(cmd),
                    "%s " NETMGR_IPTABLES_FLAGS " -t mangle -X %s",
                    ipt_tool,
                    NETMGR_TC_CHAIN_NAME_QOS_FILTER_POSTROUTING);

  if( (unsigned int)length >= maxlength)
  {
    netmgr_log_err("snprintf buffer overflow");
    return;
  }

  (void) netmgr_ds_exec(cmd, NULL);
}


/*===========================================================================
  FUNCTION  netmgr_tc_create_qos_reset_postrouting_mangle_chain
===========================================================================*/
/*!
@brief
 In this new postrouting mangle chain, we make sure only packets
 marked by this module in the POSTROUTING chain, will be accepted and
 rest of the packets will be explicitly marked with "0".

@return
  NETMGR_SUCCESS, NETMGR_FAILURE

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
static int
netmgr_tc_create_qos_reset_postrouting_mangle_chain
(
  uint32 ip_family
)
{
  char cmd[NETMGR_TC_MAX_COMMAND_LENGTH];
  int length;
  const unsigned int maxlength = sizeof(cmd)-1;
  char * ipt_tool = NULL;

  NETMGR_LOG_FUNC_ENTRY;

  if (ip_family != NETMGR_IPV6_ADDR &&
        ip_family != NETMGR_IPV4_ADDR )
  {
    netmgr_log_err("invalid IP version revceived");
    goto error;
  }

  ipt_tool = (NETMGR_IPV6_ADDR == ip_family)?
                 NETMGR_TC_IPTABLES_TOOL_V6 : NETMGR_TC_IPTABLES_TOOL_V4;

  memset( cmd, 0x0, sizeof(cmd) );
  length = snprintf(cmd,
                    maxlength,
                    "%s " NETMGR_IPTABLES_FLAGS " -t mangle -N %s",
                    ipt_tool,
                    NETMGR_TC_CHAIN_NAME_QOS_RESET_POSTROUTING);

  if( (unsigned int)length >= maxlength )
    goto error;

  if (netmgr_ds_exec(cmd, NULL) != DS_EXEC_OK)
  {
    netmgr_log_err("failed to create the postrouing chain");
    goto error;
  }

  memset( cmd, 0x0, sizeof(cmd) );
  /*Link the chain to postrouting chain of mangle table*/
  length = snprintf(cmd,
                    maxlength,
                    "%s " NETMGR_IPTABLES_FLAGS " -t mangle -A POSTROUTING -j %s",
                    ipt_tool,
                    NETMGR_TC_CHAIN_NAME_QOS_RESET_POSTROUTING);

  if( (unsigned int)length >= maxlength )
    goto error;

  if (netmgr_ds_exec(cmd, NULL) != DS_EXEC_OK)
  {
    goto error;
  }

  return NETMGR_SUCCESS;

error:
  return NETMGR_FAILURE;
}

/*===========================================================================
  FUNCTION  netmgr_tc_delete_qos_reset_postrouting_mangle_chain
===========================================================================*/
/*!
@brief
  Deletes the postrouting chain. This should only be called during process
  exit as part of cleanup procedure.

@return
  NETMGR_SUCCESS, NETMGR_FAILURE

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
static void
netmgr_tc_delete_qos_reset_postrouting_mangle_chain
(
  uint32 ip_family
)
{
  char cmd[NETMGR_TC_MAX_COMMAND_LENGTH];
  int length;
  char * ipt_tool = NULL;
  const unsigned int maxlength = sizeof(cmd)-1;

  NETMGR_LOG_FUNC_ENTRY;

  if (ip_family != NETMGR_IPV6_ADDR &&
        ip_family != NETMGR_IPV4_ADDR )
  {
    netmgr_log_err("invalid IP version revceived");
    return;
  }

  ipt_tool = (NETMGR_IPV6_ADDR == ip_family)?
                 NETMGR_TC_IPTABLES_TOOL_V6 : NETMGR_TC_IPTABLES_TOOL_V4;

  memset( cmd, 0x0, sizeof(cmd) );

  /*remove the reference of the chain from the POSTROUTING chain*/
  length = snprintf(cmd,
                    sizeof(cmd),
                    "%s " NETMGR_IPTABLES_FLAGS " -t mangle -D POSTROUTING -j %s",
                    ipt_tool,
                    NETMGR_TC_CHAIN_NAME_QOS_RESET_POSTROUTING);

  if( (unsigned int)length >= maxlength)
  {
    netmgr_log_err("snprintf buffer overflow");
    return;
  }

  (void) netmgr_ds_exec(cmd, NULL);

  memset( cmd, 0x0, sizeof(cmd) );
  /* Deleting the chain deletes all the rules.
   This chain must not be referenced by any other rules. */
  length = snprintf(cmd,
                    sizeof(cmd),
                    "%s " NETMGR_IPTABLES_FLAGS " -t mangle -F %s",
                    ipt_tool,
                    NETMGR_TC_CHAIN_NAME_QOS_RESET_POSTROUTING);

  if( (unsigned int)length >= maxlength)
  {
    netmgr_log_err("snprintf buffer overflow");
  }

  (void) netmgr_ds_exec(cmd, NULL);

  memset( cmd, 0x0, sizeof(cmd) );
  /* Deleting the chain deletes all the rules.
   This chain must not be referenced by any other rules. */
  length = snprintf(cmd,
                    sizeof(cmd),
                    "%s " NETMGR_IPTABLES_FLAGS " -t mangle -X %s",
                    ipt_tool,
                    NETMGR_TC_CHAIN_NAME_QOS_RESET_POSTROUTING);

  if( (unsigned int)length >= maxlength)
  {
    netmgr_log_err("snprintf buffer overflow");
    return;
  }

  (void) netmgr_ds_exec(cmd, NULL);
}

/*===========================================================================
  FUNCTION  netmgr_tc_reset_link_all
===========================================================================*/
/*!
@brief
  Reinitialize link data structures for all links.

@return
  int - NETMGR_SUCCESS on successful operation, NETMGR_FAILURE otherwise.

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL void
netmgr_tc_reset_link_all
(
  void
)
{
  int i;

  NETMGR_LOG_FUNC_ENTRY;

  for( i = 0; i < netmgr_main_cfg.max_links_in_total; ++i )
  {
    /* skip if this interface is not used */
    if (netmgr_tc_cfg.link_array[i].enabled == FALSE)
    {
      netmgr_log_low( "ignoring link[%d]\n", i );
      continue;
    }

    (void)netmgr_tc_reset_link( i );
  }

  NETMGR_LOG_FUNC_EXIT;
  return;
}

#ifdef FEATURE_RECOVERY_ENABLED
/*===========================================================================
  FUNCTION  netmgr_tc_cleanup_flow_id_based_chains
===========================================================================*/
/*!
@brief
  Read recovery file and cleanup iptables chains created for dedicated
  bearers

@return
  int - NETMGR_SUCCESS on successful operation, NETMGR_FAILURE otherwise
*/
/*=========================================================================*/
static int netmgr_tc_cleanup_flow_id_based_chains(void)
{
  int         cur_index = 0;
  int         rule_id = 0;
  int         rc = NETMGR_SUCCESS;
  int         reti = NETMGR_SUCCESS;
  char        cmd[NETMGR_TC_MAX_COMMAND_LENGTH];
  char        err_str[NETMGR_TC_MAX_COMMAND_LENGTH];
  char        *ipt_tool = NULL;
  const char  *dev_name = NULL;
  uint8_t     filter_mask;
  uint64_t    bitmask;
  file_info_t recovery_data;
  file_info_t meta_info;

  memset(&meta_info, 0, sizeof(file_info_t));

  recovery_data.is_valid = TRUE;
  recovery_data.info_type = TYPE_BEARER_INFO;

  /* Read meta data to get information on the number of valid entries in the
     recovery file */
  rc = netmgr_recovery_file_read_entry(NETMGR_RECOVERY_META_INFO_INDEX, &meta_info);
  if (NETMGR_SUCCESS != rc)
  {
    netmgr_log_err("%s(): failed to read meta information!", __func__);
    return NETMGR_FAILURE;
  }

  /* Read all valid entries from file and look for bearer information */
  bitmask = meta_info.recovery_info.meta_info.entry_bitmask;
  while (bitmask != 0)
  {
    if (bitmask & 0x1)
    {
      if (cur_index >= NETMGR_RECOVERY_MAX_ENTRY_INDEX)
      {
        break;
      }

      memset(&recovery_data, 0, sizeof(file_info_t));
      rc = netmgr_recovery_file_read_entry(cur_index, &recovery_data);
      if (NETMGR_SUCCESS != rc)
      {
        netmgr_log_err("%s(): failed to read entry at index [%d]!",
                       __func__, cur_index);
        reti = NETMGR_FAILURE;
        bitmask >>= 1;
        cur_index++;
        continue;
      }

      if (TRUE == recovery_data.is_valid && TYPE_BEARER_INFO == recovery_data.info_type)
      {
        /* Found bearer information in the file, use it to remove iptables
           chains and any qdisc rules */
        dev_name = netmgr_kif_get_name(recovery_data.recovery_info.bearer_info.link);
        if (!dev_name)
        {
          netmgr_log_err("%s(): failed to obtain link name for link id [%d]!",
                         __func__, recovery_data.recovery_info.bearer_info.link);
        }
        else
        {
          snprintf(cmd, sizeof(cmd), "tc qdisc del dev %s root", dev_name);
          (void) netmgr_ds_exec(cmd, NULL);
        }

        ipt_tool = (AF_INET == recovery_data.recovery_info.bearer_info.ip_family) ?
                      NETMGR_TC_IPTABLES_TOOL_V4 : NETMGR_TC_IPTABLES_TOOL_V6;

        DS_SYSCALL_DEFAULTS(opts);
        opts.stderr = err_str;
        opts.stderr_length = sizeof(err_str);
        opts.log_options |= DS_EXEC_LOG_EXECTIME;

        filter_mask = recovery_data.recovery_info.bearer_info.filter_mask;

        while (filter_mask != 0)
        {
          /* Check each bit of the filter mask to see how many flow based chains
             were created. All those chains need to be flushed and deleted */
          if (filter_mask & 0x1)
          {
            if (rule_id >= NETMGR_TC_MAX_DEDICATED_BEARERS_PER_IP_FAMILY)
            {
              break;
            }

            snprintf(cmd, sizeof(cmd), "%s " NETMGR_IPTABLES_FLAGS " -t mangle -F %s_0x%08x.%d",
                     ipt_tool, NETMGR_TC_CHAIN_NAME_PREFIX,
                     recovery_data.recovery_info.bearer_info.flow_id,
                     rule_id);

            if (DS_EXEC_OK != netmgr_ds_exec(cmd, &opts))
            {
              netmgr_log_err("%s(): cmd: %s failed! Err: %s", __func__, cmd, err_str);
            }

            snprintf(cmd, sizeof(cmd), "%s " NETMGR_IPTABLES_FLAGS " -t mangle -X %s_0x%08x.%d",
                     ipt_tool, NETMGR_TC_CHAIN_NAME_PREFIX,
                     recovery_data.recovery_info.bearer_info.flow_id,
                     rule_id);

            if (DS_EXEC_OK != netmgr_ds_exec(cmd, &opts))
            {
              netmgr_log_err("%s(): cmd: %s failed! Err: %s", __func__, cmd, err_str);
            }
          } /* If we have valid filters */

          filter_mask >>= 1;
          ++rule_id;
        } /* while filter_mask is not zero */

        recovery_data.is_valid = FALSE;
        if (NETMGR_SUCCESS != netmgr_recovery_file_write_entry(&recovery_data))
        {
          netmgr_log_err("%s(): failed to invalidate bearer info in recovery file", __func__);
          reti = NETMGR_FAILURE;
        }
      } /* if recovery entry is valid */
    }

    bitmask >>= 1;
    cur_index += 1;
  } /* Main loop */

  if (NETMGR_SUCCESS != reti)
  {
    return NETMGR_FAILURE;
  }

  return NETMGR_SUCCESS;
}
#endif /* FEATURE_RECOVERY_ENABLED */

/*===========================================================================
  FUNCTION  netmgr_tc_delete_append_qos_postrouting_mangle_chains_ref
===========================================================================*/
/*!
@brief
 Removes/Append qcom_qos chain reference in post routing chain of
 mangle table to make sure it is last after netd startup/restart.

@return
  NETMGR_SUCCESS, NETMGR_FAILURE

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL int
netmgr_tc_delete_append_qos_postrouting_mangle_chains_ref
(
  uint32 ip_family
)
{
  char cmd[NETMGR_TC_MAX_COMMAND_LENGTH];
  int result = 0;
  char * ipt_tool = NULL;

  NETMGR_LOG_FUNC_ENTRY;

  ipt_tool = (NETMGR_IPV6_ADDR == ip_family)?
              NETMGR_TC_IPTABLES_TOOL_V6 : NETMGR_TC_IPTABLES_TOOL_V4;

  memset( cmd, 0x0, sizeof(cmd) );

  /*remove the reference of the reset chain from the POSTROUTING chain*/
  snprintf(cmd,
          sizeof(cmd),
          "%s " NETMGR_IPTABLES_FLAGS " -t mangle -D POSTROUTING -j %s",
          ipt_tool,
          NETMGR_TC_CHAIN_NAME_QOS_RESET_POSTROUTING);

  if (netmgr_ds_exec(cmd, NULL) == DS_EXEC_OK) {
    memset( cmd, 0x0, sizeof(cmd) );
    /*Link the reset chain to postrouting chain of mangle table*/
    snprintf(cmd,
            sizeof(cmd),
            "%s " NETMGR_IPTABLES_FLAGS " -t mangle -A POSTROUTING -j %s",
            ipt_tool,
            NETMGR_TC_CHAIN_NAME_QOS_RESET_POSTROUTING);

    if (netmgr_ds_exec(cmd, NULL) != DS_EXEC_OK) {
      netmgr_log_err("add qos reset chain failed");
      result++;
    }
  } else {
    netmgr_log_err("delete qos reset chain failed");
    result++;
  }

  memset( cmd, 0x0, sizeof(cmd) );

  /*remove the reference of the filter chain from the POSTROUTING chain*/
  snprintf(cmd,
          sizeof(cmd),
          "%s " NETMGR_IPTABLES_FLAGS " -t mangle -D POSTROUTING -j %s",
          ipt_tool,
          NETMGR_TC_CHAIN_NAME_QOS_FILTER_POSTROUTING);

  if (netmgr_ds_exec(cmd, NULL) == DS_EXEC_OK)
  {
    memset( cmd, 0x0, sizeof(cmd) );
    /*Link the filter chain to postrouting chain of mangle table*/
    snprintf(cmd,
            sizeof(cmd),
            "%s " NETMGR_IPTABLES_FLAGS " -t mangle -A POSTROUTING -j %s",
            ipt_tool,
            NETMGR_TC_CHAIN_NAME_QOS_FILTER_POSTROUTING);

    if (netmgr_ds_exec(cmd, NULL) != DS_EXEC_OK) {
        netmgr_log_err("add qos filter chain failed");
        result++;
    }
  } else {
    netmgr_log_err("delete qos filter chain failed");
    result++;
  }

  return (result > 0) ? NETMGR_FAILURE : NETMGR_SUCCESS;
}

/*===========================================================================
                            GLOBAL FUNCTION DEFINITIONS
===========================================================================*/

/*===========================================================================
  FUNCTION  netmgr_tc_get_qos_params_by_profile_id
===========================================================================*/
/*!
@brief
  Lookup the datarate and priority QoS parameters based on CDMA profile ID.

@return
  int - NETMGR_SUCCESS on successful operations, NETMGR_FAILURE otherwise.

@note

  - Dependencies
    - None

  - Side Effects
    - Initializes the QMI Driver
*/
/*=========================================================================*/
int netmgr_tc_get_qos_params_by_profile_id
(
  uint16      profile_id,
  uint64    * datarate,
  uint8     * priority
)
{
  uint16 i;

  NETMGR_ASSERT( datarate );
  NETMGR_ASSERT( priority );

  /* Loop over table records */
  for( i=0; i<ds_arrsize( netmgr_tc_cdma_qos_spec_tbl ); i++ ) {
    /* Check for profile ID match */
    if( profile_id == netmgr_tc_cdma_qos_spec_tbl[i].profile_id ) {
      /* Return QoS parameters to caller */
      *datarate = netmgr_tc_cdma_qos_spec_tbl[i].datarate;
      *priority = netmgr_tc_cdma_qos_spec_tbl[i].priority;
      netmgr_log_low( "netmgr_tc_get_qos_params_by_profile_id: "
                      "datarate=%" PRIu64 " priority=%d\n",
                      *datarate, *priority );
      return NETMGR_SUCCESS;
    }
  }

  netmgr_log_err( "netmgr_tc_get_qos_params_by_profile_id: "
                  "cannot find mapping for profile==%d\n",
                  profile_id );

  return NETMGR_FAILURE;
}

/*===========================================================================
  FUNCTION  netmgr_tc_init
===========================================================================*/
/*!
@brief
 Main initialization routine of the traffic control module.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
void
netmgr_tc_init( int nlink, netmgr_ctl_port_config_type links[], int netmgr_restart )
{
  struct tc_vtbl vtable;

  NETMGR_LOG_FUNC_ENTRY;

  NETMGR_ASSERT(nlink > 0);

  /* Set number of links in the configuration blob */
  netmgr_tc_cfg.nlink = nlink;
  netmgr_tc_cfg.link_array = links;   /* Needed for reset */

  /* Register with Platform layer */
  if (netmgr_main_cfg.kfc_mode)
  {
    vtable.reset             = netmgr_tc_reset_link;
    vtable.flow_activate     = netmgr_tc_flow_activate_hdlr_multiq;
    vtable.flow_modify       = netmgr_tc_flow_modify_hdlr_multiq;
    vtable.flow_delete       = netmgr_tc_flow_delete_hdlr_multiq;
    vtable.flow_control      = netmgr_tc_flow_control_hdlr_multiq;
    vtable.flow_suspend      = netmgr_tc_flow_activate_hdlr_multiq;
    vtable.flow_rebind       = netmgr_tc_flow_rebind_hdlr_multiq;
  }
  else
  {
    vtable.reset             = netmgr_tc_reset_link;
    vtable.flow_activate     = netmgr_tc_flow_activate_hdlr;
    vtable.flow_modify       = netmgr_tc_flow_modify_hdlr;
    vtable.flow_delete       = netmgr_tc_flow_delete_hdlr;
    vtable.flow_control      = netmgr_tc_flow_control_hdlr;
    vtable.flow_suspend      = netmgr_tc_flow_activate_hdlr;
    vtable.flow_rebind       = NULL;
  }

  if( NETMGR_SUCCESS !=
      netmgr_platform_register_vtbl( NETMGR_PLATFORM_VTBL_TC,
                                     (void*)&vtable ) )
  {
    NETMGR_ABORT("cannot register vtable with platform layer");
  }

  if (NETMGR_SUCCESS != netmgr_tc_init_lock())
  {
    netmgr_log_err("%s(): failed to initialize mutex lock!", __func__);
    NETMGR_ABORT("failed to initialize mutex lock!");
  }

  /* Initialize link data structures */
  netmgr_tc_reset_link_all();

  if (netmgr_restart)
  {
    netmgr_tc_delete_qos_reset_postrouting_mangle_chain(NETMGR_IPV4_ADDR);
    netmgr_tc_delete_qos_reset_postrouting_mangle_chain(NETMGR_IPV6_ADDR);

    netmgr_tc_delete_qos_filter_postrouting_mangle_chain(NETMGR_IPV4_ADDR);
    netmgr_tc_delete_qos_filter_postrouting_mangle_chain(NETMGR_IPV6_ADDR);

#ifdef FEATURE_RECOVERY_ENABLED
    netmgr_tc_cleanup_flow_id_based_chains();
#endif /* FEATURE_RECOVERY_ENABLED */
  }

  netmgr_tc_cfg.postrouting_chain_available =
    (netmgr_tc_create_qos_reset_postrouting_mangle_chain(NETMGR_IPV4_ADDR) ==
   NETMGR_SUCCESS) ? TRUE : FALSE;
  netmgr_tc_cfg.postrouting_chain_available =
    (netmgr_tc_create_qos_reset_postrouting_mangle_chain(NETMGR_IPV6_ADDR) ==
  NETMGR_SUCCESS) ? TRUE : FALSE;

  netmgr_tc_create_qos_filter_postrouting_mangle_chain(NETMGR_IPV4_ADDR);
  netmgr_tc_create_qos_filter_postrouting_mangle_chain(NETMGR_IPV6_ADDR);

  netmgr_tc_create_delete_dynamic_post_routing_rule(AF_INET);
  netmgr_tc_create_delete_dynamic_post_routing_rule(AF_INET6);

  /* Register process exit cleanup handler */
  atexit( netmgr_tc_reset_link_all );

  netmgr_tc_cfg.is_initialized = TRUE;

  if(pthread_mutex_init(&netmgr_qos_modify_cache.modify_mutex, NULL) != 0)
  {
      netmgr_log_err("QoS modify cache  mutex init failed, disabling skip_buffered_qos_modify feature");
      netmgr_main_set_skip_buffered_qos_modify(0);
  }

  NETMGR_LOG_FUNC_EXIT;
  return;
}

/*===========================================================================
  FUNCTION  netmgr_tc_create_delete_dynamic_post_routing_rule
===========================================================================*/
/*!
@brief
 Adds/removes source and interface iptable rules in post routing chain of
mangle table to reset skb->mark to zero if there are no matching rules.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_tc_create_delete_dynamic_post_routing_rule(int ip_family)
{
  char cmd[NETMGR_TC_MAX_COMMAND_LENGTH];
  int length;
  const unsigned int maxlength = sizeof(cmd)-1;
  char * ipt_tool = NULL;

  NETMGR_LOG_FUNC_ENTRY;

  if (AF_INET != ip_family && AF_INET6 != ip_family)
  {
    netmgr_log_err("invalid family=%d",ip_family);
    return NETMGR_FAILURE;
  }

  ipt_tool = (AF_INET6 == ip_family)? NETMGR_TC_IPTABLES_TOOL_V6 : NETMGR_TC_IPTABLES_TOOL_V4;

  memset( cmd, 0x0, sizeof(cmd) );
  length = snprintf(cmd,
                    maxlength,
                    "%s " NETMGR_IPTABLES_FLAGS " -t mangle -A %s -o rmnet_data+ -j MARK --set-mark 0x%x",
                    ipt_tool,
                    NETMGR_TC_CHAIN_NAME_QOS_RESET_POSTROUTING,
                    NETMGR_TC_DEFAULT_FLOW_MARKING);

  if( (unsigned int)length >= maxlength )
    goto error;

  if (netmgr_ds_exec(cmd, NULL) != DS_EXEC_OK)
  {
    goto error;
  }

  return NETMGR_SUCCESS;

error:
  return NETMGR_FAILURE;
}

/*===========================================================================
  FUNCTION netmgr_tc_reorder_qos_post_routing_chain_ref
===========================================================================*/
/*!
@brief
  Reorder qos chain ref in post routing chain of mangle table

@return
  int - NETMGR_SUCCESS/NETMGR_FAILURE
*/
/*=========================================================================*/
int
netmgr_tc_reorder_qos_post_routing_chain_ref(void)
{
  int result = 0;
  /* qos_postrouting chain mush be in the last of mangle postrouting table
     so it can exit once rule matched without affecting the other operation in the table */
  if (netmgr_tc_delete_append_qos_postrouting_mangle_chains_ref(NETMGR_IPV4_ADDR)
      != NETMGR_SUCCESS) {
    netmgr_log_err("Failed reorder qos postrouting chain for IPV4");
    result++;
  }
  if (netmgr_tc_delete_append_qos_postrouting_mangle_chains_ref(NETMGR_IPV6_ADDR)
      != NETMGR_SUCCESS) {
    netmgr_log_err("Failed reorder qos postrouting chain for IPV6");
    result++;
  }

  return (result == 0) ? NETMGR_SUCCESS : NETMGR_FAILURE;
}

/*===========================================================================
  FUNCTION  netmgr_tc_match_bearer_id
===========================================================================*/
/*!
@brief
  Compares flow objects to determine if they match.  Used by
  linked-list search route.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL long int
netmgr_tc_match_bearer_id
(
  const void * first,
  const void * second
)
{
  netmgr_tc_flow_info_t * in_data = (netmgr_tc_flow_info_t *)first;
  netmgr_tc_flow_info_t * list_data = (netmgr_tc_flow_info_t *)second;

  if (in_data == NULL || list_data == NULL)
  {
    return -1;
  }

  if (list_data->qos_flow.bearer_id == in_data->qos_flow.bearer_id)
  {
    return 0;
  }

  return 1;
}

/*===========================================================================
  FUNCTION  netmgr_tc_find_flow_from_bearer
===========================================================================*/
/*!
@brief
  Search through flow list to find the flow with the specified beaere_id.

@return
  flow info if found, NULL otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
netmgr_tc_flow_info_t*
netmgr_tc_find_flow_from_bearer
(
  int link,
  uint8 bearer_id
)
{
  ds_dll_el_t* node = NULL;
  netmgr_tc_flow_info_t flow_buf;
  netmgr_tc_flow_info_t* flow_info = NULL;

  NETMGR_LOG_FUNC_ENTRY;

  if (link < 0 || link >= netmgr_main_cfg.max_links_in_total)
  {
    netmgr_log_err("invalid params\n");
  }
  else
  {
    flow_buf.qos_flow.bearer_id = bearer_id;

    node = ds_dll_search(netmgr_tc_cfg.links[link].flow_list,
                         &flow_buf,
                         netmgr_tc_match_bearer_id);

    if (node != NULL)
    {
      flow_info = (netmgr_tc_flow_info_t*) ds_dll_data(node);
    }
  }

  if (flow_info == NULL)
  {
    netmgr_log_high("flow for bearer_id %d not found\n", bearer_id);
  }

  NETMGR_LOG_FUNC_EXIT;
  return flow_info;
}

/*===========================================================================
  FUNCTION  netmgr_tc_is_default_flow_activated
===========================================================================*/
/*!
@brief
  Check if default flow is already activated.

@return
  TRUE/FALSE

@note
*/
/*=========================================================================*/
boolean
netmgr_tc_is_default_flow_activated
(
  int                                link,
  const netmgr_qmi_qos_flow_info_t * qos_flow
)
{
  netmgr_tc_flow_info_t * flow_info = NULL;
  netmgr_tc_flow_info_t * new_flow_info = NULL;
  netmgr_tc_handle_info_t * new_class_handle = NULL;

  NETMGR_ASSERT(qos_flow &&
                NETMGR_IS_DEFAULT_FLOW(qos_flow->flow_id));

  flow_info = netmgr_tc_find_flow_info(
                 link, qos_flow->flow_id, qos_flow->ip_ver);

  if (flow_info != NULL)
  {
    /* Bearer id could have been changed if one of dual-ip sub-call
     * is disconnect and reconnected */
    netmgr_log_high("Default flow (bearer_id %d) ip %d found\n",
                         flow_info->qos_flow.bearer_id,
                         flow_info->qos_flow.ip_ver);

    if (flow_info->qos_flow.bearer_id != qos_flow->bearer_id)
    {
      netmgr_cb_runall(NETMGR_QMI_CB_TABLE, NETMGR_QMI_CB_FLOW_DELETE, flow_info);
      flow_info->qos_flow.bearer_id = qos_flow->bearer_id;
      netmgr_cb_runall(NETMGR_QMI_CB_TABLE, NETMGR_QMI_CB_FLOW_ACTIVATE, flow_info);
    }

    return TRUE;
  }
  else
  {
    flow_info = netmgr_tc_find_flow_from_bearer(link, qos_flow->bearer_id);
    if (flow_info)
    {
      /* Dual-IP calls have the same bearer -- use same TC class */
      netmgr_log_high("Flow with the same bearer_id (flow 0x%d) ip %d found\n",
                         flow_info->qos_flow.flow_id,
                         flow_info->qos_flow.ip_ver);

      new_flow_info = netmgr_tc_flow_alloc();
      new_class_handle = netmgr_tc_handle_info_alloc();
      NETMGR_ASSERT(new_flow_info && new_class_handle);

      memset(new_flow_info, 0, sizeof(netmgr_tc_flow_info_t));
      memcpy(new_class_handle, flow_info->class_handle,
                           sizeof(netmgr_tc_handle_info_t));

      new_flow_info->phys_net_dev = flow_info->phys_net_dev;
      new_flow_info->link_name = flow_info->link_name;
      new_flow_info->qos_flow = *qos_flow;
      new_flow_info->class_handle = new_class_handle;
      new_flow_info->tcp_ack_class = NULL;

      if (netmgr_main_get_tcpackprio_enabled())
      {
        netmgr_tc_create_tcp_ack_class(link, new_flow_info);
      }

      ds_dll_enq(netmgr_tc_cfg.links[link].flow_list, NULL, new_flow_info);

      netmgr_cb_runall(NETMGR_QMI_CB_TABLE, NETMGR_QMI_CB_FLOW_ACTIVATE, new_flow_info);

      return TRUE;
    }
  }

  return FALSE;
}

/*===========================================================================
  FUNCTION  netmgr_tc_create_proto_filter
===========================================================================*/
/*!
@brief
  Create filter (classifier) on specified link root qdisc to route
  all packets with speificied protocol to designated class.

@return
  int - NETMGR_SUCCESS on successful operation,
        NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL int
netmgr_tc_create_proto_filter
(
  int                             link,
  netmgr_tc_flow_info_t         * flow_info,
  const netmgr_tc_handle_info_t * parent,
  const netmgr_tc_handle_info_t * class_info
)
{
  int result =  NETMGR_FAILURE;
  char cmd[NETMGR_TC_MAX_COMMAND_LENGTH];
  const char *dev_name = NULL;

  NETMGR_LOG_FUNC_ENTRY;
  dev_name = netmgr_kif_get_name(link);
  if(NULL == dev_name)
  {
    netmgr_log_err("%s(): unable to determine name for link=%d\n", __func__, link);
    goto bail;
  }

  NETMGR_ASSERT( flow_info );
  NETMGR_ASSERT( parent );
  NETMGR_ASSERT( class_info );

  snprintf(cmd, sizeof(cmd),
           "tc filter add dev %s parent %d:%d "
           "prio %d protocol %s u32 match u32 0 0 classid %d:%d",
           dev_name,
           parent->major,
           parent->minor,
           NETMGR_TC_DEFAULT_PRIORITY,
           (flow_info->qos_flow.ip_ver == NETMGR_QOS_IP_VERSION_6) ? "ipv6" : "ip",
           class_info->major,
           class_info->minor);

  result = netmgr_ds_exec(cmd, NULL);
  if (result != DS_EXEC_OK)
    result = NETMGR_FAILURE;

bail:
  NETMGR_LOG_FUNC_EXIT;
  return result;
}

/*===========================================================================
  FUNCTION  netmgr_tc_create_2nd_default_flow
===========================================================================*/
/*!
@brief
  Create second default flow in case V4V6 have different bearers

@return
  int - NETMGR_SUCCESS on successful operation,
        NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL int netmgr_tc_create_2nd_default_flow
(
  int link,
  netmgr_tc_flow_info_t   *flow_info,
  netmgr_tc_handle_info_t *class_handle
)
{
  int result;

  /* Sanity checks */
  NETMGR_ASSERT(flow_info && class_handle);
  NETMGR_ASSERT(NETMGR_IS_DEFAULT_FLOW(flow_info->qos_flow.flow_id));

  /* Create 2nd child class */
  class_handle->major = netmgr_tc_cfg.links[link].root_class->minor;
  NETMGET_TC_ASSIGN_CLASS_MINOR_ID(link, class_handle->minor);

  result = netmgr_tc_create_class(link,
                                  netmgr_tc_cfg.links[link].root_class,
                                  flow_info->qos_flow.priority,
                                  netmgr_main_cfg.tc_ul_ceiling_mbps,
                                  class_handle,
                                  TRUE);
  if (result != NETMGR_SUCCESS)
  {
    return NETMGR_FAILURE;
  }

  /* Create leaf qdisc */
  result = netmgr_tc_create_leaf_qdisc(link, class_handle);
  if (result != NETMGR_SUCCESS)
  {
    return NETMGR_FAILURE;
  }

  /* Create filter */
  result = netmgr_tc_create_proto_filter(link,
                                         flow_info,
                                         netmgr_tc_cfg.links[link].root_qdisc,
                                         class_handle);
  if (result != NETMGR_SUCCESS)
  {
    return NETMGR_FAILURE;
  }

  /* Hook up the flow */
  ds_dll_enq(netmgr_tc_cfg.links[link].flow_list, NULL, (void*)flow_info);

  netmgr_tc_cfg.links[link].default_class2 = class_handle;

  return NETMGR_SUCCESS;
}

/*===========================================================================
  FUNCTION  netmgr_tc_create_tcp_ack_class
===========================================================================*/
/*!
@brief
 Create TCP ack class for a flow.

@return
  int - NETMGR_SUCCESS on successful operation,
        NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL int
netmgr_tc_create_tcp_ack_class
(
  int                      link,
  netmgr_tc_flow_info_t  * flow_info
)
{
  netmgr_tc_handle_info_t * rclass_handle = NULL;
  netmgr_tc_handle_info_t * tcp_ack_class_handle = NULL;
  int result = NETMGR_FAILURE;

  NETMGR_LOG_FUNC_ENTRY;

  NETMGR_ASSERT(link >= 0 && link < netmgr_main_cfg.max_links_in_total);
  NETMGR_ASSERT(flow_info);

  rclass_handle = netmgr_tc_cfg.links[link].root_class;
  if (!rclass_handle)
  {
    netmgr_log_err("root class is null");
    goto bail;
  }

  /* Create child class for TCP ack flow */
  tcp_ack_class_handle = netmgr_tc_handle_info_alloc();
  if (!tcp_ack_class_handle)
  {
    netmgr_log_err("failed to allocate tcp ack class handle for flow %u",
                        flow_info->qos_flow.flow_id);
    goto bail;
  }

  tcp_ack_class_handle->major = rclass_handle->minor;
  NETMGET_TC_ASSIGN_CLASS_MINOR_ID(link, tcp_ack_class_handle->minor);

  result = netmgr_tc_create_class(link,
                                  rclass_handle,
                                  0, /* Highest priority for TCP Ack's*/
                                  netmgr_main_cfg.tc_ul_ceiling_mbps,
                                  tcp_ack_class_handle,
                                  NETMGR_IS_DEFAULT_FLOW(flow_info->qos_flow.flow_id));

  if (result == NETMGR_FAILURE)
  {
    netmgr_log_err("error creating tcp ack class for flow %u",
                        flow_info->qos_flow.flow_id);
    netmgr_free(tcp_ack_class_handle);
    goto bail;
  }

  /* Create leaf qdisc */
  result = netmgr_tc_create_leaf_qdisc(link, tcp_ack_class_handle);

  if (result == NETMGR_FAILURE)
  {
    netmgr_log_err("error creating leaf qdisc for flow %u", flow_info->qos_flow.flow_id);
    netmgr_free(tcp_ack_class_handle);
    goto bail;
  }

  /* Create a filter for TCP Ack */
  result = netmgr_tc_create_tcp_ack_filter(link,
                                           flow_info,
                                           netmgr_tc_cfg.links[link].root_qdisc,
                                           tcp_ack_class_handle);

  if (result == NETMGR_FAILURE)
  {
    netmgr_log_err("error creating leaf qdisc for flow %u", flow_info->qos_flow.flow_id);
    netmgr_free(tcp_ack_class_handle);
    goto bail;
  }

  flow_info->tcp_ack_class = tcp_ack_class_handle;
  result = NETMGR_SUCCESS;

bail:
  NETMGR_LOG_FUNC_EXIT;
  return result;
}

/*===========================================================================
  FUNCTION  netmgr_tc_delete_tcp_ack_class
===========================================================================*/
/*!
@brief
 Delete TCP ack class for a flow.

@return
  int - NETMGR_SUCCESS on successful operation,
        NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL int
netmgr_tc_delete_tcp_ack_class
(
  int                      link,
  netmgr_tc_flow_info_t  * flow_info
)
{
  char cmd[NETMGR_TC_MAX_COMMAND_LENGTH];
  netmgr_tc_handle_info_t * rclass_handle = NULL;
  int major, minor;
  int result = NETMGR_FAILURE;

  NETMGR_LOG_FUNC_ENTRY;

  NETMGR_ASSERT(link >= 0 && link < netmgr_main_cfg.max_links_in_total);
  NETMGR_ASSERT(flow_info);

  if (!flow_info->tcp_ack_class)
  {
    return NETMGR_SUCCESS;
  }

  rclass_handle = netmgr_tc_cfg.links[link].root_class;
  if (!rclass_handle)
  {
    netmgr_log_err("root class is null");
    goto bail;
  }

  /* delete the filter */
  snprintf(cmd, sizeof(cmd), "tc filter delete dev %s parent %d:%d "
           "protocol %s flowid %d:%d",
           flow_info->link_name,
           NETMGR_TC_QDISC_ROOT_MAJOR,
           NETMGR_TC_QDISC_ROOT_MINOR,
           (flow_info->qos_flow.ip_ver == NETMGR_QOS_IP_VERSION_6) ? "ipv6" : "ip",
           flow_info->tcp_ack_class->major,
           flow_info->tcp_ack_class->minor);

  if (netmgr_ds_exec(cmd, NULL) != DS_EXEC_OK)
  {
    netmgr_log_high("Error deleting filter. Ignoring");
  }

  /* delete leaf qdisc */
  snprintf(cmd, sizeof(cmd),"%d", flow_info->tcp_ack_class->major);
  major = strtol(cmd, NULL, 16);
  snprintf(cmd, sizeof(cmd),"%d", flow_info->tcp_ack_class->minor);
  minor = strtol(cmd, NULL, 16);
#ifdef NETMGR_NL_ENABLED
  if (netmgr_rtnetlink_flow_control(flow_info->link_name, (major << 16) | minor,
                                    minor << 16,
                                    NETMGR_FLOW_DISABLE,
                                    NETMGR_DEL_NETLINK) != NETMGR_SUCCESS)
  {

    netmgr_log_high("tc qdisc msg failed, falling back to legacy ds_exec");
#endif
    snprintf(cmd, sizeof(cmd), "tc qdisc delete dev %s parent %d:%d "
             "handle %d:0 prio flow enable",
             flow_info->link_name,
             flow_info->tcp_ack_class->major,
             flow_info->tcp_ack_class->minor,
             flow_info->tcp_ack_class->minor);

    if (netmgr_ds_exec(cmd, NULL) != DS_EXEC_OK)
    {
      netmgr_log_high("Error deleting leaf qdisc. Ignoring");
    }
#ifdef NETMGR_NL_ENABLED
  }
  else
  {
    netmgr_log_high("Netlink Qdisc tc msg succesful");
  }
#endif

  /* delete the class */
  snprintf(cmd, sizeof(cmd), "tc class delete dev %s parent %d:%d "
           "classid %d:%d",
           flow_info->link_name,
           rclass_handle->major,
           rclass_handle->minor,
           flow_info->tcp_ack_class->major,
           flow_info->tcp_ack_class->minor);

  if (netmgr_ds_exec(cmd, NULL) != DS_EXEC_OK)
  {
    netmgr_log_high("Error deleting ack class. Ignoring");
  }

  netmgr_free(flow_info->tcp_ack_class);
  flow_info->tcp_ack_class = NULL;
  result = NETMGR_SUCCESS;

bail:
  NETMGR_LOG_FUNC_EXIT;
  return result;
}

/*===========================================================================
  FUNCTION netmgr_tc_match_qos_modify_cache_flows
===========================================================================*/
/*!
@brief
  Compare 2 QoS modify cache flows and returns comparison result

@return
  0 - flows match
  non-zero - flows mismatch
*/
/*=========================================================================*/
long int
netmgr_tc_match_qos_modify_cache_flows
(
  const void * first,
  const void * second
)
{
  /* Return zero when the flows match, nonzero when they do not*/
  if ( first != NULL && second != NULL)
  {
    return (((netmgr_qmi_qos_modify_info_t*)first)->flow_id == ((netmgr_qmi_qos_modify_info_t*)second)->flow_id )? NETMGR_SUCCESS : NETMGR_FAILURE;
  }
  else
  {
      netmgr_log_err("%s(): input params first or second is NULL", __func__);
      return NETMGR_FAILURE;
  }
}

/*===========================================================================
  FUNCTION netmgr_tc_find_qos_modify_cache_flow_info
===========================================================================*/
/*!
@brief
  Find flow info from QoS Modify cache

@return
  Pointer to flow info object
*/
/*=========================================================================*/
void*
netmgr_tc_find_qos_modify_cache_flow_info
(
 int link,
 uint32 flow_id
)
{
    ds_dll_el_t* node = NULL;
    netmgr_qmi_qos_modify_info_t flow_buf;
    netmgr_qmi_qos_modify_info_t* flow_info = NULL;

    NETMGR_LOG_FUNC_ENTRY;

    flow_buf.flow_id = flow_id;
    node = ds_dll_search(netmgr_qos_modify_cache.links[link].flow_list,
                         &flow_buf,
                         netmgr_tc_match_qos_modify_cache_flows);

    if (node == NULL)
    {
        netmgr_log_high("%s(): flow 0x%lx not found", __func__, (unsigned long) flow_id);
        NETMGR_LOG_FUNC_EXIT;
        return NULL;
    }

    flow_info = (netmgr_qmi_qos_modify_info_t*) ds_dll_data(node);

    if (flow_info == NULL)
    {
        netmgr_log_err("%s(): flow 0x%lx data not found", __func__, (unsigned long) flow_id);
    }

    NETMGR_LOG_FUNC_EXIT;
    return flow_info;
}

/*===========================================================================
  FUNCTION netmgr_tc_incr_rcvd_qos_modify_counter
===========================================================================*/
/*!
@brief
  Increment the number of QoS Modify requests pending to be processed in cmdq

@return
  NETMGR_SUCCESS - on sucessful increment
  NETMGR_FAILURE - on unsucessful attempt to increment
*/
/*=========================================================================*/
int
netmgr_tc_incr_rcvd_qos_modify_counter
(
  int    link,
  uint32 flow_id
)
{
  netmgr_qmi_qos_modify_info_t* flow_info = NULL;

  flow_info = (netmgr_qmi_qos_modify_info_t*)netmgr_tc_find_qos_modify_cache_flow_info(link, flow_id);

  if ( flow_info == NULL)
  {
    netmgr_log_med("%s(): no matching flow found in modify cache for link=%d, "
                    "flow_id=%d, not incrmenting modify flow counter",
                    __func__,
                    link,
                    flow_id);
    return NETMGR_FAILURE;
  }
  else
  {
    flow_info->rcvd_qos_modify_counter++;
    return NETMGR_SUCCESS;
  }
}

/*===========================================================================
  FUNCTION netmgr_tc_get_rcvd_modify_flow_counter
===========================================================================*/
/*!
@brief
  Get the number of QoS Modify requests pending to be processed in cmdq

@return
  <num> - counter value
*/
/*=========================================================================*/
int
netmgr_tc_get_rcvd_modify_flow_counter
(
  int    link,
  uint32 flow_id
)
{
  netmgr_qmi_qos_modify_info_t* flow_info = NULL;

  flow_info = (netmgr_qmi_qos_modify_info_t*)netmgr_tc_find_qos_modify_cache_flow_info(link, flow_id);

  if ( flow_info == NULL )
  {
    netmgr_log_med("%s(): no matching flow found in modify cache for link=%d, flow_id=%d",
                    __func__,
                    link,
                    flow_id);
    return 0;
  }
  else
  {
    return flow_info->rcvd_qos_modify_counter;
  }
}

/*===========================================================================
  FUNCTION netmgr_tc_decr_rcvd_modify_flow_counter
===========================================================================*/
/*!
@brief
  Decrement the number of QoS Modify requests pending to be processed in cmdq

@return
  NETMGR_SUCCESS - on sucessful decrement
  NETMGR_FAILURE - on unsucessful attempt to decrement
*/
/*=========================================================================*/
int
netmgr_tc_decr_rcvd_modify_flow_counter
(
  int    link,
  uint32 flow_id
)
{
  netmgr_qmi_qos_modify_info_t* flow_info = NULL;
  int result = NETMGR_FAILURE;

  flow_info = (netmgr_qmi_qos_modify_info_t*)netmgr_tc_find_qos_modify_cache_flow_info(link, flow_id);

  if ( flow_info == NULL)
  {
    netmgr_log_med("%s(): no matching flow found in modify cache for link=%d, "
                    "flow_id=%d, not decrementing modify flow counter",
                    __func__,
                    link,
                    flow_id);
  }
  else
  {
    if ( flow_info->rcvd_qos_modify_counter > 0 )
    {
        flow_info->rcvd_qos_modify_counter--;
        result = NETMGR_SUCCESS;
    }
    else
    {
        netmgr_log_err("%s(): rcvd_qos_modify_counter is either already 0 or negative", __func__);
    }
  }

  return result;
}

#endif /* NETMGR_QOS_ENABLED */
