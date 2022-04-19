#ifndef _TH_CONFIG_H__
#define _TH_CONFIG_H__

/*========================================================================

*//** @file config.h

FILE SERVICES:
     Static configuration for thread services library to use.

     Detailed description

Copyright (c) 2015 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

*//*====================================================================== */

/*========================================================================
                      Edit History

when       who    what, where, why
--------   ---    -------------------------------------------------------
11/25/14   rg     Initial Version.

========================================================================== */


/*========================================================================*/
/* MACRO DEFINITIONS */
/*========================================================================*/
#define MAX_TH_SESSIONS 3
#define MAX_TH_DATAPART 4
#define MAX_TH_QUEUES 3
#define MAX_TH_JOB_REQUESTS 1024 //maximum of 1024 outstanding job requests
#define MAX_TH_WORK_THREADS 3 //maximum of <10 worker threads per session
#define MAX_TH_UNIQUE_JOBS 32 //maximum unique thread functions taken out

#define MAX_TH_QUEUE_ELEMENTS MAX_TH_JOB_REQUESTS

//maximum of 8 instances per job (i.e. 8 parallel threads doing the same work)
#define MAX_REGISTERED_INSTANCES 8

//maximum number of fifo jobs pending at  a time
#define MAX_PENDING_JOBS 10

#define JOB_PRIO_CRITICAL 1
#define JOB_PRIO_HIGH 2
#define JOB_PRIO_NORMAL 3

//#define ENABLE_THREAD_PROFILING 1

#endif /* end of macro _TH_CONFIG_H__ */
