#ifndef __TH_DECISION_H__
#define __TH_DECISION_H__

/*========================================================================

*//** @file decision.h

FILE SERVICES:
  Internal Decision Manager API for Thread Services Library.

  Decision Manager handles handles the Master Job List and the Job Registry.
  Interacts with Session Manager on one side, and with Dispatcher and Core
  Core Services on the other

Copyright (c) 2015 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

*//*====================================================================== */

/*========================================================================
                      Edit History

when       who    what, where, why
--------   ---    -------------------------------------------------------
11/10/14   rg     Initial Version.

========================================================================== */

/*========================================================================*/
/* INCLUDE FILES */
/*==  =====================================================================*/
/* std headers */

/* local headers */
#include "common.h"

/*========================================================================*/
/* DEFINITIONS AND DECLARATIONS */
/*========================================================================*/
#if defined( __cplusplus )
extern "C"
{
#endif /* end of macro __cplusplus */


th_err_t decision_manager_add_job(th_session_t *th_session,
                                th_job_t *job);

th_err_t decision_manager_del_job();

th_err_t decision_manager_check_job_eligibility(th_session_t *th_session,
                               th_job_t *job);

th_err_t decision_manager_register_job(th_session_t *th_session,
                   job_func_t job_func, char *job_func_name,
                   job_callback_t flush_done_addr,
                   int is_fifo, int session_key, unsigned int *handle);

th_err_t decision_manager_stop_job(th_session_t *th_session,
                   unsigned int handle, void *user_data, int is_blocking);

th_err_t decision_manager_create_job_entry(th_session_t *th_session,
                   unsigned int handle, job_params_t *job_params,
                   th_job_t **this_job);

th_err_t decision_manager_set_one_empty_slot(th_session_t *th_session,
                                     th_job_t *job, int unblock_caller);

void decision_manager_send_stop_done(th_session_t *th_session,
                                              th_job_t *job);

void decision_manager_set_empty_slot(th_session_t *th_session,
                                   th_job_t *job);

th_err_t init_decision_manager(th_session_t *th_session);

th_err_t deinit_decision_manager(th_session_t *th_session);

th_err_t desicion_manager_send_flush_done(th_session_t *th_session,
                 unsigned int handle);

void decision_manager_update_job_registry(th_session_t *th_session,
                                    th_job_t *job);


#if defined( __cplusplus )
}
#endif /* end of macro __cplusplus */

#endif /* end of macro __TH_DECISION_H__ */
