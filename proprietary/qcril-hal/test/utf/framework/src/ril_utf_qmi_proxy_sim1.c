/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_rild_sim.c
  @brief   RIL UTF RIL daemon simulation

  DESCRIPTION
  ---------------------------------------------------------------------------
******************************************************************************/
#include "ril_utf_core.h"

/*========================================================
 * Global Variables
========================================================*/

extern pthread_t qmi_proxy_id;

/*========================================================
  FUNCTION: rild_start
========================================================*/
void *qmi_proxy_start_entry_f(void *param)
{
  RIL_UTF_DEBUG("\n QMI_RIL_UTL: Entering QMI Proxy");

  enter_qmi_proxy();

  RIL_UTF_DEBUG("\n QMI_RIL_UTL: QMI Proxy \"entry thread\" exiting...");

}

/*========================================================
  FUNCTION: start_rild_thread
  - Starts the rild thread.
========================================================*/
void start_qmi_proxy()
{
  pthread_attr_t qmi_proxy_thrd_attr;
  int ret;

  pthread_attr_init(&qmi_proxy_thrd_attr);
  ret = pthread_create(&qmi_proxy_id, &qmi_proxy_thrd_attr, qmi_proxy_start_entry_f, NULL);

  if ( ret < 0 )
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Failed to start QMI PROXY. Exiting...");
    return;
  }
}

