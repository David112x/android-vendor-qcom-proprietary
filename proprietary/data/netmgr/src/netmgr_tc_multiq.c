/******************************************************************************

                      N E T M G R _ T C _ M U L T I Q. C

******************************************************************************/

/******************************************************************************

  @file    netmgr_tc.c
  @brief   Network Manager traffic control multiq

  DESCRIPTION
  Implementation of NetMgr Linux traffic control multiq module.

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
#include <arpa/inet.h>
#include "ds_list.h"
#include "ds_string.h"
#include "netmgr_defs.h"
#include "netmgr_platform.h"
#include "netmgr_util.h"
#include "netmgr_tc_i.h"
#include "netmgr_cb.h"
#include "netmgr_qmi_cb.h"
#include "netmgr_tc_multiq.h"

/*===========================================================================
                     LOCAL DEFINITIONS AND DECLARATIONS
===========================================================================*/

#define NETMGR_TC_DEFAULT_MULTIQ_NUM 0

/*===========================================================================
                            FORWARD DECLARATIONS
===========================================================================*/


/*===========================================================================
                            LOCAL FUNCTION DEFINITIONS
===========================================================================*/

/*===========================================================================
  FUNCTION netmgr_tc_assign_num_multiq
===========================================================================*/
/*!
@brief
  Assigne a multiq number starting from 1

@return
  The assigned multiq number. 0 if not assigned.

@note
*/
/*=========================================================================*/
LOCAL uint32_t
netmgr_tc_assign_num_multiq
(
  int                    link,
  netmgr_tc_flow_info_t *flow_info
)
{
  uint32_t n = NETMGR_TC_DEFAULT_MULTIQ_NUM;
  uint32_t i;

  NETMGR_ASSERT(link >= 0 && link < netmgr_tc_cfg.nlink);
  NETMGR_ASSERT(flow_info);

  for (i = 0; i < NETMGR_TC_MAX_MULTIQ_NUM; i++)
  {
    if (i == NETMGR_TC_DEFAULT_MULTIQ_NUM)
    {
      continue;
    }

    /* Find a match */
    if (netmgr_tc_cfg.links[link].mq_info[i].ref &&
        netmgr_tc_cfg.links[link].mq_info[i].bearer_id ==
                   flow_info->qos_flow.bearer_id)
    {
      netmgr_tc_cfg.links[link].mq_info[i].ref++;
      n = i;
      goto out;
    }

    /* Find an empty slot */
    if (n == NETMGR_TC_DEFAULT_MULTIQ_NUM &&
        !netmgr_tc_cfg.links[link].mq_info[i].ref)
    {
      n = i;
    }
  }

  if (n != NETMGR_TC_DEFAULT_MULTIQ_NUM)
  {
    netmgr_tc_cfg.links[link].mq_info[n].bearer_id =
                        flow_info->qos_flow.bearer_id;
    netmgr_tc_cfg.links[link].mq_info[n].ref = 1;
  }
  else
  {
    netmgr_log_err("Could not find free multiq num, use default");
  }

out:
  netmgr_log_high("Assigned multiq num %u for bearer %u flow %u ref %u",
                  n, flow_info->qos_flow.bearer_id,
                  flow_info->qos_flow.flow_id,
                  netmgr_tc_cfg.links[link].mq_info[n].ref);
  return n;
}

/*===========================================================================
  FUNCTION netmgr_tc_release_num_multiq
===========================================================================*/
/*!
@brief
  Release a multiq number

@return
  None

@note
*/
/*=========================================================================*/
LOCAL void
netmgr_tc_release_num_multiq
(
  int                    link,
  netmgr_tc_flow_info_t *flow_info
)
{
  uint32_t i;

  NETMGR_ASSERT(link >= 0 && link < netmgr_tc_cfg.nlink);
  NETMGR_ASSERT(flow_info);

  if (!flow_info->class_handle ||
      flow_info->class_handle->major >= NETMGR_TC_MAX_MULTIQ_NUM)
  {
    netmgr_log_err("Invalid multiq for flow %d", flow_info->qos_flow.flow_id);
    return;
  }

  i = flow_info->class_handle->major;

  if (i == NETMGR_TC_DEFAULT_MULTIQ_NUM)
  {
    netmgr_log_high("Released multiq num %d", NETMGR_TC_DEFAULT_MULTIQ_NUM);
    return;
  }

  if (netmgr_tc_cfg.links[link].mq_info[i].ref &&
      netmgr_tc_cfg.links[link].mq_info[i].bearer_id ==
                 flow_info->qos_flow.bearer_id)
  {
    netmgr_tc_cfg.links[link].mq_info[i].ref--;
    netmgr_log_high("Released multiq num %u for bearer %u flow %u ref %u",
                    i, flow_info->qos_flow.bearer_id,
                    flow_info->qos_flow.flow_id,
                    netmgr_tc_cfg.links[link].mq_info[i].ref);
  }
  else
  {
    netmgr_log_err("Not release multiq %d with bearer %d, mq ref %d bearer %d",
                   i, flow_info->qos_flow.bearer_id,
                   netmgr_tc_cfg.links[link].mq_info[i].ref,
                   netmgr_tc_cfg.links[link].mq_info[i].bearer_id);
  }
}

/*===========================================================================
  FUNCTION  netmgr_tc_flow_delete_multiq
===========================================================================*/
/*!
@brief
 Delete a flow object and the corresponding traffic control elements.

@return
 None

*/
/*=========================================================================*/
LOCAL void
netmgr_tc_flow_delete_multiq
(
  int link,
  netmgr_tc_flow_info_t *flow_info
)
{
  NETMGR_ASSERT(link >= 0 && link < netmgr_tc_cfg.nlink);
  if (flow_info == NULL)
  {
    netmgr_log_err("flow info is NULL");
    return;
  }

  netmgr_cb_runall(NETMGR_QMI_CB_TABLE, NETMGR_QMI_CB_FLOW_DELETE, flow_info);
  netmgr_tc_release_num_multiq(link, flow_info);

  netmgr_log_med("Freeing flow_id=%u ip=%d",
              flow_info->qos_flow.flow_id, flow_info->qos_flow.ip_ver);

  if (!NETMGR_IS_DEFAULT_FLOW(flow_info->qos_flow.flow_id))
  {
    netmgr_tc_filter_delete_hdlr(flow_info, link, flow_info->qos_flow.flow_id);
  }

  netmgr_tc_flow_free(&flow_info);
}

/*===========================================================================
  FUNCTION  netmgr_tc_flow_rebind_multiq
===========================================================================*/
/*!
@brief
 Rebind QoS flow to a different bearer id.

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
netmgr_tc_flow_rebind_multiq
(
  int                     link,
  netmgr_tc_flow_info_t * flow_info,
  uint8                   bearer_id
)
{
  NETMGR_ASSERT(flow_info);

  if (flow_info->qos_flow.bearer_id == bearer_id)
  {
    return NETMGR_SUCCESS;
  }

  netmgr_log_high("Rebinding flow %d from bearer %d to bearer %d",
                  flow_info->qos_flow.flow_id, flow_info->qos_flow.bearer_id,
                  bearer_id);

  if (!flow_info->class_handle)
  {
    netmgr_log_err("No class handle for flow %d", flow_info->qos_flow.flow_id);
    return NETMGR_FAILURE;
  }

  netmgr_tc_release_num_multiq(link, flow_info);

  flow_info->qos_flow.bearer_id = bearer_id;
  flow_info->class_handle->major = netmgr_tc_assign_num_multiq(link, flow_info);
  flow_info->class_handle->minor = flow_info->class_handle->major;

  /* Kernel will treat the flow activate with a different bearer as a re-bind */
  netmgr_cb_runall(NETMGR_QMI_CB_TABLE, NETMGR_QMI_CB_FLOW_ACTIVATE, flow_info);

  return NETMGR_SUCCESS;
}

/*===========================================================================
  FUNCTION  netmgr_tc_flow_create_multiq
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
netmgr_tc_flow_create_multiq
(
  int                                link,
  const netmgr_qmi_qos_flow_info_t  *qos_flow
)
{
  netmgr_tc_flow_info_t * flow_info = NULL;
  netmgr_tc_handle_info_t * class_handle = NULL;

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

  /* Assign mq for the flow */
  class_handle->major = (int)netmgr_tc_assign_num_multiq(link, flow_info);
  class_handle->minor = class_handle->major;

  flow_info->class_handle = class_handle;

  netmgr_log_high("num_filters: %d",qos_flow->num_filter);

  if (!NETMGR_IS_DEFAULT_FLOW(qos_flow->flow_id))
  {
    if (NETMGR_SUCCESS != netmgr_tc_create_flow_chain(link, qos_flow->flow_id,
                                    qos_flow->filter_list[0].qos_filter.ip_version,
                                    qos_flow->flow_id, qos_flow->flow_id))
    {
      netmgr_tc_release_num_multiq(link, flow_info);
      flow_info->class_handle = NULL;
      goto error;
    }

    /* Link the chain into the mangle table */
    if (netmgr_tc_flow_enable( link,flow_info ) != NETMGR_SUCCESS)
    {
      netmgr_tc_release_num_multiq(link, flow_info);
      flow_info->class_handle = NULL;
      goto error;
    }
  }

  /* Add flow object to list for this link only if the flow is enabled successfully*/
  ds_dll_enq( netmgr_tc_cfg.links[link].flow_list, NULL, (void*)flow_info );

  netmgr_cb_runall(NETMGR_QMI_CB_TABLE, NETMGR_QMI_CB_FLOW_ACTIVATE, flow_info);

  NETMGR_LOG_FUNC_EXIT;
  return NETMGR_SUCCESS;

error:
  netmgr_tc_flow_free( &flow_info );
  netmgr_free( class_handle );
  NETMGR_LOG_FUNC_EXIT;
  return NETMGR_FAILURE;
}

/*===========================================================================
  FUNCTION  netmgr_tc_flow_activate_hdlr_multiq
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
int netmgr_tc_flow_activate_hdlr_multiq
(
  int link,
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
  netmgr_log_low( "flow datarate=%" PRIu64 "\n", qos_flow->datarate );
  netmgr_log_low( "flow num_filter=%d\n", qos_flow->num_filter );
  netmgr_log_low( "flow filter_list=%p\n", qos_flow->filter_list );
  netmgr_log_low( "flow bearer_id=%d\n", qos_flow->bearer_id );
  netmgr_log_low( "flow ip_ver=%d\n", qos_flow->ip_ver );

  flow_info = netmgr_tc_find_flow_info(
                  link, qos_flow->flow_id, qos_flow->ip_ver);

  /* Check for newly created flow */
  if( qos_flow->is_new ) {
    /* Check for pre-existing flow ID */
    if( !flow_info )
    {
      /* Create flow object and install QoS measures */
      result = netmgr_tc_flow_create_multiq( link, qos_flow );
    }
    else if (NETMGR_IS_DEFAULT_FLOW(qos_flow->flow_id))
    {
      /* Default flow bearer id changed - can this happen ? */
      if (qos_flow->bearer_id != flow_info->qos_flow.bearer_id)
      {
        netmgr_tc_flow_rebind_multiq(link, flow_info, qos_flow->bearer_id);
      }
      result = NETMGR_SUCCESS;
    }
    else
    {
      /* This may happen if previously a suspend or activated flow event
       * happened. In this case network may be updating either flow or filter
       * or both. Hence we need to modify the flow and filter spec installed
       * in the kernel
       */
      netmgr_log_med( "flow is already active, modifying flow.\n" );
      result = netmgr_tc_flow_modify_hdlr_multiq(link,qos_flow);
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
      netmgr_tc_flow_modify_hdlr_multiq(link,qos_flow);
    }
    else
    {
      netmgr_log_err( "Cannot find flow object[0x%x], ignoring\n", qos_flow->flow_id );
    }
  }

  NETMGR_UNLOCK_MUTEX(tc_lock);
  NETMGR_LOG_FUNC_EXIT;
  return result;
}

/*===========================================================================
  FUNCTION  netmgr_tc_flow_delete_hdlr_multiq
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
int netmgr_tc_flow_delete_hdlr_multiq
(
  int link,
  const netmgr_qmi_qos_flow_info_t  *qos_flow
)
{
  ds_dll_el_t* node = NULL;
  ds_dll_el_t* tail = NULL;
  netmgr_tc_flow_info_t flow_buf;
  netmgr_tc_flow_info_t* flow_info = NULL;
  const void* dummy = NULL;
  const void* data = NULL;
  int result = NETMGR_FAILURE;

  NETMGR_LOG_FUNC_ENTRY;

  NETMGR_ASSERT(qos_flow);
  NETMGR_ASSERT(link >= 0 && link < netmgr_tc_cfg.nlink);

  NETMGR_LOCK_MUTEX(tc_lock);

  if (NETMGR_IS_DEFAULT_FLOW(qos_flow->flow_id))
  {
    /* Empty the flow list */
    netmgr_log_med("Empty the flow list");
    while (NULL != (node = ds_dll_deq(netmgr_tc_cfg.links[link].flow_list, NULL, &data)))
    {
      flow_info = (netmgr_tc_flow_info_t*) (data);
      netmgr_tc_flow_delete_multiq(link, flow_info);

      ds_dll_free(node);
    }

    memset(&netmgr_tc_cfg.links[ link ].mq_info, 0,
           sizeof(netmgr_tc_cfg.links[ link ].mq_info));

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
    netmgr_tc_flow_delete_multiq(link, flow_info);

    ds_dll_free(node);
    result = NETMGR_SUCCESS;
  }

done:
  NETMGR_UNLOCK_MUTEX(tc_lock);

  NETMGR_LOG_FUNC_EXIT;
  return result;
}

/*===========================================================================
  FUNCTION  netmgr_tc_flow_control_hdlr_multiq
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
int netmgr_tc_flow_control_hdlr_multiq
(
  int link,
  const netmgr_qmi_qos_flow_info_t  *qos_flow,
  netmgr_tc_flow_state_t state
)
{
  (void)link;
  (void)qos_flow;
  (void)state;

  return NETMGR_SUCCESS;
}

/*===========================================================================
  FUNCTION  netmgr_tc_flow_modify_hdlr_multiq
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
int netmgr_tc_flow_modify_hdlr_multiq
(
  int link,
  const netmgr_qmi_qos_flow_info_t  *qos_flow
)
{
  netmgr_tc_flow_info_t *flow_info = NULL;
  uint8 bearer_id;

  NETMGR_LOG_FUNC_ENTRY;

  NETMGR_ASSERT(link >= 0 && link < netmgr_tc_cfg.nlink);
  NETMGR_ASSERT(qos_flow != NULL);

  NETMGR_LOCK_MUTEX(tc_lock);

  flow_info = netmgr_tc_find_flow_info(link, qos_flow->flow_id, qos_flow->ip_ver);

  netmgr_tc_flow_filter_dump(qos_flow);

  if (flow_info == NULL)
  {
    netmgr_log_err("flow_modify_hdlr: the flow object does not exist");
    goto error;
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
    goto error;
  }

  NETMGR_UNLOCK_MUTEX(tc_lock);

  NETMGR_LOG_FUNC_EXIT;
  return NETMGR_SUCCESS;

error:
  NETMGR_UNLOCK_MUTEX(tc_lock);
  NETMGR_LOG_FUNC_EXIT;
  return NETMGR_FAILURE;
}

/*===========================================================================
  FUNCTION  netmgr_tc_flow_rebind_hdlr_multiq
===========================================================================*/
/*!
@brief
 Change the traffic control elements on Modem QoS flow rebind.

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
int netmgr_tc_flow_rebind_hdlr_multiq
(
  int link,
  const netmgr_qmi_qos_flow_info_t  *qos_flow
)
{
  netmgr_tc_flow_info_t *flow_info = NULL;

  NETMGR_LOG_FUNC_ENTRY;

  NETMGR_ASSERT(link >= 0 && link < netmgr_tc_cfg.nlink);
  NETMGR_ASSERT(qos_flow != NULL);

  netmgr_tc_flow_filter_dump(qos_flow);

  NETMGR_LOCK_MUTEX(tc_lock);

  flow_info = netmgr_tc_find_flow_info(link, qos_flow->flow_id, qos_flow->ip_ver);
  if (flow_info == NULL)
  {
    netmgr_log_err("%s: the flow object does not exist", __func__);
    goto error;
  }

  if (flow_info->qos_flow.bearer_id == qos_flow->bearer_id)
  {
    netmgr_log_high("%s: no change in bearer_id", __func__);
  }
  else
  {
    netmgr_tc_flow_rebind_multiq(link, flow_info, qos_flow->bearer_id);
  }

  NETMGR_UNLOCK_MUTEX(tc_lock);

  NETMGR_LOG_FUNC_EXIT;
  return NETMGR_SUCCESS;

error:
  NETMGR_UNLOCK_MUTEX(tc_lock);
  NETMGR_LOG_FUNC_EXIT;
  return NETMGR_FAILURE;
}

#endif /* NETMGR_QOS_ENABLED */
