#ifndef __TH_CORE_H__
#define __TH_CORE_H__

/*========================================================================

*//** @file core.h

FILE SERVICES:
  Internal Core Services API for Thread Services Library.

  Core Services maintains the Priority Qs and the Job lifecycle through
  those Priority Q-s. It interacts closely with the Decision Manager and
  the Dispatcher

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


th_err_t add_job_to_queue(th_session_t *th_session, th_job_t *job);

th_err_t remove_job_from_queue(th_session_t *th_session, th_job_t *job);

th_err_t init_queues_lists(th_session_t *th_session);

th_err_t deinit_queues_lists(th_session_t *th_session);

void *get_from_q(th_queue_t *q);

th_err_t add_to_q(th_queue_t *q, void *data);

void *peek_from_q(th_queue_t *q);

th_err_t delete_from_q(th_queue_t *q, void *data, int pull_tail);

th_err_t compact_q(th_queue_t *q);

th_err_t set_all_jobs(void *q, unsigned int handle, th_job_state to_state);

th_err_t set_first_job(th_session_t *th_session, th_job_t *job,
                     th_job_state from_state, th_job_state to_state);

th_job_t *get_first_job(th_session_t *th_session, th_queue_t *q, int idx);


#if defined( __cplusplus )
}
#endif /* end of macro __cplusplus */

#endif /* end of macro __TH_CORE_H__ */
