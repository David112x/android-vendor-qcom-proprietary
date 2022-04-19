/******************************************************************************

                        N E T M G R _ S S R . C

******************************************************************************/

/******************************************************************************

  @file    netmgr_ssr.c
  @brief   Network Manager SSR module implementation

  DESCRIPTION
  Implementation of NetMgr's SSR module function.

******************************************************************************/
/*===========================================================================

  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

/******************************************************************************

                      EDIT HISTORY FOR FILE

  $Id:$

when       who        what, where, why
--------   ---        -------------------------------------------------------

******************************************************************************/

/*===========================================================================
                              INCLUDE FILES
===========================================================================*/

#include <pthread.h>
#include "netmgr_util.h"
#include "ds_cmdq.h"
#include "netmgr_cb.h"
#include "netmgr_qmi_cb.h"
#include "netmgr_ssr.h"
#include "wireless_data_service_v01.h"

/*===========================================================================
                     LOCAL DEFINITIONS AND DECLARATIONS
===========================================================================*/

#define NETMGR_SSR_INST_MAX (3)
#define NETMGR_SSR_CMDQ_MAX (10)

static qmi_client_os_params  os_params;
static qmi_client_type       ssr_client;
static ds_cmdq_info_t        ssr_cmdq;

static boolean               ssr_wait;
static qmi_service_instance  ssr_wait_inst;
static boolean               ssr_wait_inst_up;
static pthread_mutex_t       ssr_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t        ssr_cond = PTHREAD_COND_INITIALIZER;

static qmi_service_instance  modem_instances[NETMGR_SSR_INST_MAX];
static boolean               inited = FALSE;

/*===========================================================================
                      INTERNAL FUNCTION DEFINITIONS
===========================================================================*/

/*===========================================================================
  FUNCTION:  netmgr_ssr_is_handler
===========================================================================*/
/*!
@brief
  Handle modem in-service event

@return
  None

@note

*/
/*=========================================================================*/
static void netmgr_ssr_is_handler
(
  qmi_service_instance qmi_inst
)
{
  netmgr_cb_runall(NETMGR_QMI_CB_TABLE,
                   NETMGR_QMI_CB_MODEM_IS_EX,
                   (void *)((intptr_t)qmi_inst));
}

/*===========================================================================
  FUNCTION:  netmgr_ssr_oos_handler
===========================================================================*/
/*!
@brief
  Handle modem out-of-service event

@return
  None

@note

*/
/*=========================================================================*/
static void netmgr_ssr_oos_handler
(
  qmi_service_instance qmi_inst
)
{
  netmgr_cb_runall(NETMGR_QMI_CB_TABLE,
                   NETMGR_QMI_CB_MODEM_OOS_EX,
                   (void *)((intptr_t)qmi_inst));
}

/*===========================================================================
  FUNCTION:  netmgr_ssr_exec
===========================================================================*/
/*!
@brief
  SSR event exec function

@return
  None

@note

*/
/*=========================================================================*/
static void netmgr_ssr_exec
(
  ds_cmd_t       * cmd,
  void           * data
)
{
  qmi_service_info            service_info_array[NETMGR_SSR_INST_MAX];
  qmi_service_instance        service_inst_array[NETMGR_SSR_INST_MAX];
  qmi_service_instance        instance_id;
  qmi_idl_service_object_type service_obj;
  qmi_client_error_type       rc;
  unsigned int                num_entries = NETMGR_SSR_INST_MAX;
  unsigned int                num_services = 0;
  unsigned int                i, j = 0;

  (void)cmd;
  service_obj = (qmi_idl_service_object_type)data;

  memset(service_info_array, 0, sizeof(service_info_array));
  for (i = 0; i < NETMGR_SSR_INST_MAX; i++)
  {
    service_inst_array[i] = QMI_CLIENT_INSTANCE_ANY;
  }

  // Retrieve available service list
  rc = qmi_client_get_service_list(service_obj,
                                   service_info_array,
                                   &num_entries,
                                   &num_services);
  if (QMI_NO_ERR != rc)
  {
    // This could happen if all services are gone
    netmgr_log_high("Get service list returned %d\n", rc);
  }

  netmgr_log_med("Num services %d\n", num_services);

  // Retrieve current in-service instances
  for (i = 0; i < num_services; i++)
  {
    rc = qmi_client_get_instance_id(&service_info_array[i],
                                    &instance_id);
    if (QMI_NO_ERR != rc)
    {
      netmgr_log_err("Failed to get instance id\n");
    }
    else
    {
      service_inst_array[j++] = instance_id;
      netmgr_log_low("...%d: 0x%x\n", i, instance_id);
    }
  }

  pthread_mutex_lock(&ssr_mutex);

  // Check if an instance became out-of-service
  for (i = 0; i < NETMGR_SSR_INST_MAX; i++)
  {
    if (QMI_CLIENT_INSTANCE_ANY == modem_instances[i])
    {
      continue;
    }
    for (j = 0; j < NETMGR_SSR_INST_MAX; j++)
    {
      if (modem_instances[i] == service_inst_array[j])
      {
        break;
      }
    }
    if (j >= NETMGR_SSR_INST_MAX)
    {
      netmgr_log_med("Modem 0x%x out-of-service\n", modem_instances[i]);
      netmgr_ssr_oos_handler(modem_instances[i]);
    }
  }

  // Check if an instance became in-service
  for (i = 0; i < NETMGR_SSR_INST_MAX; i++)
  {
    if (QMI_CLIENT_INSTANCE_ANY == service_inst_array[i])
    {
      continue;
    }
    for (j = 0; j < NETMGR_SSR_INST_MAX; j++)
    {
      if (service_inst_array[i] == modem_instances[j])
      {
        break;
      }
    }
    if (j >= NETMGR_SSR_INST_MAX)
    {
      netmgr_log_med("Modem 0x%x in-service\n", service_inst_array[i]);

      /* If user is waiting, signal and bypass callback */
      if (ssr_wait && (ssr_wait_inst == service_inst_array[i] ||
                       ssr_wait_inst == QMI_CLIENT_INSTANCE_ANY))
      {
        ssr_wait = FALSE;
        ssr_wait_inst_up = TRUE;
        pthread_cond_signal(&ssr_cond);
      }
      else
      {
        netmgr_ssr_is_handler(service_inst_array[i]);
      }
    }
  }

  // Save current in-service instances
  memcpy(modem_instances,
         service_inst_array, sizeof(service_inst_array));

  pthread_mutex_unlock(&ssr_mutex);

#if defined(FEATURE_DS_LINUX_ANDROID) && !defined(NETMGR_OFFTARGET)
  netmgr_util_wakelock_release();
#endif
}

/*===========================================================================
  FUNCTION:  netmgr_ssr_cmd_free
===========================================================================*/
/*!
@brief
  SSR command free function

@return
  None

@note

*/
/*=========================================================================*/
static void netmgr_ssr_cmd_free
(
  ds_cmd_t       * cmd,
  void           * data
)
{
  (void)data;

  if (cmd)
  {
    ds_cmdq_release_cmd(cmd);
  }
}

/*===========================================================================
  FUNCTION:  netmgr_ssr_notify_cb
===========================================================================*/
/*!
@brief
  SSR event callback function

@return
  None

@note

*/
/*=========================================================================*/
static void netmgr_ssr_notify_cb
(
  qmi_client_type                user_handle,
  qmi_idl_service_object_type    service_obj,
  qmi_client_notify_event_type   service_event,
  void                         * notify_cb_data
)
{
  ds_cmd_t *cmd = NULL;

  (void)user_handle;
  (void)service_event;
  (void)notify_cb_data;

#if defined(FEATURE_DS_LINUX_ANDROID) && !defined(NETMGR_OFFTARGET)
  netmgr_util_wakelock_acquire();
#endif

  netmgr_log_high("Modem event %d\n", service_event);

  cmd = ds_cmdq_alloc_cmd();
  if (NULL == cmd)
  {
    netmgr_log_err("Failed to allocate ds cmd\n");
    return;
  }

  memset(cmd, 0, sizeof(ds_cmd_t));
  cmd->execute_f = netmgr_ssr_exec;
  cmd->free_f = netmgr_ssr_cmd_free;
  cmd->data = service_obj;

  if (ds_cmdq_enq(&ssr_cmdq, cmd) < 0)
  {
    netmgr_log_err("Failed to enq cmd\n");
    ds_cmdq_release_cmd(cmd);
    return;
  }
}

/*===========================================================================
                      EXTERNAL FUNCTION DEFINITIONS
===========================================================================*/

/*===========================================================================
  FUNCTION:  netmgr_ssr_init
===========================================================================*/
/*!
@brief
  Initialize SSR module

@return
  int - NETMGR_SUCCESS
        NETMGR_FAILURE

@note

*/
/*=========================================================================*/
int netmgr_ssr_init
(
  qmi_idl_service_object_type  svc_obj,
  boolean                      blocking,
  qmi_service_instance         blocking_inst,
  unsigned int                 timeout
)
{
  qmi_client_error_type       rc;
  int                         i;
  struct timespec             ts;

  if (inited)
  {
    netmgr_log_err("SSR module already initialized\n");
    return NETMGR_FAILURE;
  }

  for (i = 0; i < NETMGR_SSR_INST_MAX; i++)
  {
    modem_instances[i] = QMI_CLIENT_INSTANCE_ANY;
  }

  ssr_wait = blocking;
  ssr_wait_inst = blocking_inst;
  ssr_wait_inst_up = FALSE;

  /* Notifier client */
  if (NULL == svc_obj)
  {
    svc_obj = wds_get_service_object_v01();
  }

  if (NULL == svc_obj)
  {
    netmgr_log_err("Failed to get service object\n");
    return NETMGR_FAILURE;
  }

  rc = qmi_client_notifier_init(svc_obj, &os_params, &ssr_client);
  if (QMI_NO_ERR != rc)
  {
    netmgr_log_err("Failed to init notifier client rc %d\n", rc);
    return NETMGR_FAILURE;
  }

  /* SSR cmdq */
  if (ds_cmdq_init(&ssr_cmdq, NETMGR_SSR_CMDQ_MAX) < 0)
  {
    netmgr_log_err("Failed to init ds cmdq\n");
    qmi_client_release(ssr_client);
    return NETMGR_FAILURE;
  }

  rc = qmi_client_register_notify_cb(
              ssr_client, netmgr_ssr_notify_cb, NULL);
  if (QMI_NO_ERR != rc)
  {
    netmgr_log_err("Failed to register notify cb rc %d\n", rc);
    qmi_client_release(ssr_client);
    ds_cmdq_deinit(&ssr_cmdq);
    return NETMGR_FAILURE;
  }

  inited = TRUE;
  netmgr_log_high("SSR module initialized\n");

  /* Wait for the modem instance up */
  if (blocking)
  {
    netmgr_log_high("Waiting for modem instance 0x%x...\n", blocking_inst);

    clock_gettime(CLOCK_MONOTONIC, &ts);
    ts.tv_sec += timeout;

    pthread_mutex_lock(&ssr_mutex);
    while (!ssr_wait_inst_up)
    {
      if (timeout)
      {
        if (ETIMEDOUT == pthread_cond_timedwait(
                             &ssr_cond, &ssr_mutex, &ts))
        {
          netmgr_log_high("Timed out waiting modem!\n");
          pthread_mutex_unlock(&ssr_mutex);
          netmgr_ssr_deinit();
          return NETMGR_FAILURE;
        }
      }
      else
      {
        pthread_cond_wait(&ssr_cond, &ssr_mutex);
      }
    }
    ssr_wait = FALSE;
    ssr_wait_inst = QMI_CLIENT_INSTANCE_ANY;
    ssr_wait_inst_up = FALSE;
    pthread_mutex_unlock(&ssr_mutex);

    netmgr_log_high("Modem instance 0x%x up!\n", blocking_inst);
  }

  return NETMGR_SUCCESS;
}

/*===========================================================================
  FUNCTION:  netmgr_ssr_deinit
===========================================================================*/
/*!
@brief
  De-Initialize SSR module

@return
  None

@note

*/
/*=========================================================================*/
void netmgr_ssr_deinit()
{
  if (!inited)
  {
    return;
  }

  qmi_client_release(ssr_client);
  ds_cmdq_deinit(&ssr_cmdq);

  inited = FALSE;

  netmgr_log_high("SSR module de-inited\n");
}
