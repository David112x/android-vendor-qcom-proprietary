#ifndef __TH_SESSION_H__
#define __TH_SESSION_H__

/*========================================================================

*//** @file session.h

FILE SERVICES:
  Internal Session API for Thread Services Library

  Session Manager handles session level interactions for Thread Library.
  Maintains a one:one correspondence with actual camera sessions in use.
  Logical separation of thread library entities, and none of the entities look
  beyond session boundaries

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


th_err_t create_session_manager(th_services_t *th_serv);

th_err_t init_thread_session(th_services_t *th_serv, int session_key);

th_err_t destroy_session_manager(th_services_t *th_serv);

th_err_t deinit_thread_session(th_services_t *th_serv, int session_key);

th_err_t submit_new_job(th_services_t *th_serv, unsigned int handle,
             job_params_t *job_params);

th_err_t register_new_job(th_services_t *th_serv, unsigned int session_key,
            job_func_t job_func, char *job_func_name,
            job_callback_t flush_done_addr, int is_fifo, unsigned int *handle);

th_err_t stop_existing_job(th_services_t *th_serv, unsigned int handle,
                       void *user_data, int is_blocking);


#if defined( __cplusplus )
}
#endif /* end of macro __cplusplus */

#endif /* end of macro __TH_SESSION_H__ */
