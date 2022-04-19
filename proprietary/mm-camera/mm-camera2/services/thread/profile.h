#ifndef __TH_PROFILE_H__
#define __TH_PROFILE_H__

/*========================================================================

*//** @file profile.h

FILE SERVICES:
  Internal Profiler API for Thread Services Library.

  Detailed description

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


#if defined(ENABLE_THREAD_PROFILING)

th_err_t create_profiler(th_session_manager_t *th_sess_mgr);

th_err_t destroy_profiler(th_session_manager_t *th_sess_mgr);

th_err_t profiler_add_job(th_profiler_t *th_profiler, th_job_t *job);

th_err_t profiler_finish_job(th_profiler_t *th_profiler, th_job_t *job);

th_err_t profiler_end_thread_exec(th_profiler_t *th_profiler,
                                    unsigned int idx, int jobs_taken);

th_err_t profiler_start_thread_exec(th_profiler_t *th_profiler,
                                                    unsigned int idx);

th_err_t profiler_start_thread_wait(th_profiler_t *th_profiler,
                                                    unsigned int idx);

th_err_t profiler_end_thread_wait(th_profiler_t *th_profiler,
                                                    unsigned int idx);

th_err_t init_profiler(th_session_t *th_session);

th_err_t deinit_profiler(th_session_t *th_session);

#else

#define create_profiler(x) TH_FAILURE
#define destroy_profiler(x)TH_FAILURE
#define profiler_add_job(x,y) TH_SUCCESS
#define profiler_finish_job(x,y) TH_SUCCESS
#define profiler_end_thread_exec(x,y,z) TH_SUCCESS
#define profiler_start_thread_exec(x,y) TH_SUCCESS
#define profiler_start_thread_wait(x,y) TH_SUCCESS
#define profiler_end_thread_wait(x,y) TH_SUCCESS
#define init_profiler(x) TH_SUCCESS
#define deinit_profiler(x) TH_SUCCESS

#endif


#if defined( __cplusplus )
}
#endif /* end of macro __cplusplus */

#endif /* end of macro __TH_PROFILE_H__ */
