/* afd_thread.h
 *
 * Copyright (c) 2013-2016 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __AFD_THREAD_H__
#define __AFD_THREAD_H__

#include <pthread.h>
#include <semaphore.h>
#include "afd_module.h"

typedef struct {
  pthread_mutex_t msg_q_lock;
  mct_queue_t     *msg_q;
  boolean          active;

  pthread_condattr_t thread_condattr;
  pthread_cond_t  thread_cond;
  pthread_mutex_t thread_mutex;
  pthread_t       thread_id;
  sem_t           sem_launch;
  afd_process_data_t process_data;
  boolean         no_stats_mode; /* true, don't take/process any stats, mainly when all stream are off */
} afd_thread_data_t;

/** afd_msg_sync_t
 *    @msg_sem: semaphore to synchronize the "Sync" messages
 *
 *  This structure is used for creating synchronized messages in the queue.
 *  When such message arrives, the enqueue function will block until the
 *  message is processed by the thread handler and the library returns with
 *  the result.
 **/
typedef struct {
  sem_t  msg_sem;
} afd_msg_sync_t;

#if 0
typedef struct {
  afd_thread_data_t *thread_data;

  mct_port_t        *af_port;
  af_callback_func  af_cb;
  af_object_t       *af_obj;
} afd_thread_af_data_t;
#endif
typedef enum {
  MSG_AFD_SET,
  MSG_AFD_GET,
  MSG_AFD_STATS,
  MSG_STOP_AFD_THREAD,
  MSG_AFD_STATS_MODE,
} afd_thread_msg_type_t;

typedef struct {
  afd_thread_msg_type_t type;
  boolean         sync_flag;
  afd_msg_sync_t  *sync_obj;

  union {
    stats_t *stats;
    afd_set_parameter_t afd_set_parm;
    afd_get_parameter_t afd_get_parm;
  } u;
} afd_thread_msg_t;

afd_thread_data_t* afd_thread_init(void);
void afd_thread_deinit(void *p);
boolean afd_thread_start(void *p);

boolean afd_thread_stop(afd_thread_data_t *aecawb_data);
boolean afd_thread_en_q_msg(void *aecawb_data,
  afd_thread_msg_t  *msg);
boolean afd_stats_map(q3a_core_row_sum_stats_type *row_sum_stats,
                      q3a_core_bg_stats_type *bg_stats,
                      const stats_t *stats);

#endif /* __AFD_THREAD_H__ */
