#ifndef __TH_SERVICES_API_H__
#define __TH_SERVICES_API_H__

/*========================================================================

*//** @file thread_services.h

FILE SERVICES:
      Public API declaration for Thread Services Library (TSL).

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
/*========================================================================*/
/* std headers */

/* local headers */
#include "thread_error.h"

/*========================================================================*/
/* MACRO DEFINITIONS */
/*========================================================================*/
#define JOB_PRIO_CRITICAL 1
#define JOB_PRIO_HIGH 2
#define JOB_PRIO_NORMAL 3

/*========================================================================*/
/* DEFINITIONS AND DECLARATIONS */
/*========================================================================*/
#if defined( __cplusplus )
extern "C"
{
#endif /* end of macro __cplusplus */

//------------------------------------------------------------------------------
/// @description
///   One time initialization of the Thread Services Library, per lifetime of the
///   Camera Daemon Process
///
/// @param none
///
/// @return
///   Standard error from Thread Services Library.
//------------------------------------------------------------------------------
th_err_t init_thread_services();

//------------------------------------------------------------------------------
/// @description
///   One time de-initialization of the Thread Services Library, per lifetime of the
///   Camera Daemon Process
///
/// @param none
///
/// @return
///   Standard error from Thread Services Library.
//------------------------------------------------------------------------------
th_err_t deinit_thread_services();

//------------------------------------------------------------------------------
/// @description
///   Create a session in the Thread Services Library
///
/// @param session_key
///   A unique key to identify the Camera session.
///   NOTE: This cannot be 0
///
/// @return
///   Standard error from Thread Services Library.
//------------------------------------------------------------------------------
th_err_t create_session(
  unsigned int session_key
);

//------------------------------------------------------------------------------
/// @description
///   Destroy a session in the Thread Services Library
///
/// @param session_key
///   A unique key to identify the Camera session, passed previously in create_thread_session
///   NOTE: This cannot be 0
///
/// @return
///   Standard error from Thread Services Library.
//------------------------------------------------------------------------------
th_err_t destroy_session(
  unsigned int session_key
);

//------------------------------------------------------------------------------
/// @description
///   Post a job request to the Thread Services Library.
///
/// @param handle
///   handle returned from register_job_request, job function corresponding to the handle
///   will be dispatched from TSL
///   NOTE: handle cannot be 0
///
/// @param stopped_cb
///   When the module posts a stop request to the TSL, all subsequent job postings for the
///   same job instance results in stopped_cb to be called. The module can use the stopped_cb
///...to decide whether they want to free any data, unblock any caller thread etc.
///   NOTE: TSL uses the user_data as the same data pointer as passed into this finction.
///
/// @param data_ptr
///   Single or an array of data payloads for the job. If the job is NOT splitable into multiple
///...threads, its a single pointer. Else it's an array of pointers, terminated by a NULL pointer. If
///...an array or pointers, cast it to a void * and pass, and TSL will dereference each element.
///...NOTE: TSL determines how many data pointers to look for based on is_splitable param.
///...NOTE: At least ONE non-NULL data ptr shoud be supplied.
///   NOTE: MAXIMUM number of splits allowed is 4
///
/// @param priority
///   Logical priority of the job. Following are the suggested values --->
///   JOB_PRIO_CRITICAL = Reserved for AF critical events, MCT pipeline module start/stop events
///   JOB_PRIO_HIGH = Reserved for other Stats events
///   JOB_PRIO_NORMAL = Reserved for all other events
///...NOTE: If you are NOT sure about the priority, just leave it 0 (ZERO).
///
/// @param relative_deadline
///   Relative deadline of the job. If the priority param is specified, this field is ignored
///...NOTE: If you are NOT sure, just leave it 0 (ZERO).
///
/// @param is_splitable
///   Is the job logically splitable into multiple threads of execution. If yes, supply the
///   appropriate number of data pointers, as specified in the data_ptr param.
///   NOTE: If you indicate that the job is splitable and donot supply the necessary data pointers,
///   the job will be rejected
///
/// @param is_blocking
///   Specify if the job is blocking on the caller or not
///   NOTE: You will still get a job_stopped callback
///
/// @return
///   Standard error from Thread Services Library.
//------------------------------------------------------------------------------
th_err_t post_job_request(
  unsigned int handle,
  void (*stopped_cb)(void *user_data),
  void *data_ptr,
  int priority,
  int relative_deadline,
  int is_splitable,
  int is_blocking
);

//------------------------------------------------------------------------------
/// @description
///   One-time register a job request to the Thread Services Library.
///
/// @param session_key
///   Session id of the camera session that this job belongs to.
///
/// @param job_func_addr
///   Function to be called asynchronously from TSL to get the job done.
///   NOTE: This cannot be NULL
///
/// @param job_name
///   The printable and user-friendly text name of the job function
///
/// @param flush_done_addr
///   Function to be called asynchronously from TSL post a stop request
///   and after all pending jobs (pre stop request) have been dispatched
///
/// @param is_fifo
///   Does the job dispatch need to be serialized from TSL. This is particularly needed
///...if we want to achieve queue add-remove (FIFO) behavior of posting
///   commands to a thread. Most of our backend jobs fall in "is_fifo = 1" category
///
/// @return
///   Non-zero if success, zero if failed.
//------------------------------------------------------------------------------
unsigned int register_job_request(
  unsigned int session_key,
  void *(*job_func_addr) (void *arg),
  char *job_func_name,
  void (*flush_done_addr) (void *arg),
  int is_fifo
);

//------------------------------------------------------------------------------
/// @description
///   Stop a job to be posted from Thread Services Library.
///
/// @param handle
///   handle returned from register_job_request
///   NOTE: handle cannot be 0
///
/// @param user_data
///   user data to be passed along with the flush done callback
///
/// @param is_blocking
///   Specify if the stop request is blocking on the caller or not
///   NOTE: You will still get a flush_done callback
///
/// @return
///   Standard error from Thread Services Library.
//------------------------------------------------------------------------------
th_err_t stop_job_request(
  unsigned int handle,
  void *user_data,
  int is_blocking
);


#if defined( __cplusplus )
}
#endif /* end of macro __cplusplus */

#endif /* end of macro __TH_SERVICES_API_H__ */
