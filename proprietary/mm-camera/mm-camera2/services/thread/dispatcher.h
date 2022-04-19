#ifndef __TH_DISPATCHER_H__
#define __TH_DISPATCHER_H__

/*========================================================================

*//** @file dispatcher.h

FILE SERVICES:
  Internal Dispatcher API for Thread Services Library.

  Dispatcher handles the Manager and Worker thead pool lifecycle.
..Works closely with the Core Services and Desicion Manager

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


void dispatch_job(th_session_t *th_session, th_job_t *job,
                     th_profiler_t *profiler);

th_err_t init_dispatcher(th_session_t *th_session);

th_err_t deinit_dispatcher(th_session_t *th_session);

th_err_t trigger_dispatcher(th_session_t *th_session,
                                unsigned int trigger_mask);


#if defined( __cplusplus )
}
#endif /* end of macro __cplusplus */

#endif /* end of macro __TH_DISPATCHER_H__ */
