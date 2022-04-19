/* supermct.h
 *
 * This file contains the media controller implementation. All commands coming
 * from the server arrive here first. There is one media controller per
 * session.
 *
 *Copyright (c) 2016 Qualcomm Technologies, Inc.
  * All Rights Reserved.
  * Confidential and Proprietary - Qualcomm Technologies, Inc.
  */

#ifndef __SUPERMCT_H__
#define __SUPERMCT_H__

#include "mct_object.h"

#define SUPERMCT_CAST(supermct)        ((supermct_t *)(supermct))
#define SUPERMCT_PARENT(supermct)      (MCT_OBJECT_PARENT(supermct))
#define SUPERMCT_NUM_CHILDREN(supermct)(MCT_OBJECT_NUM_CHILDREN(supermct))
#define SUPERMCT_CHILDREN(supermct)    (MCT_OBJECT_CHILDREN(supermct))
#define SUPERMCT_LOCK(super_mct)        (MCT_OBJECT_LOCK(super_mct))
#define SUPERMCT_UNLOCK(supermct)      (MCT_OBJECT_UNLOCK(supermct))

/** supermct_zone
*    Dual Camera Zones:
*           Wide:   Primary: Awake,  Auxiliary: Asleep
*           Dual:   Primary: Awake,  Auxiliary: Awake
*           Tele:   Primary: Asleep, Auxiliary: Awake
**/
typedef enum {
  SUPERMCT_WIDE_ZONE, /* Wide:   Primary: Awake,  Auxiliary: Asleep */
  SUPERMCT_DUAL_ZONE, /* Dual:   Primary: Awake,  Auxiliary: Awake */
  SUPERMCT_TELE_ZONE, /* Tele:   Primary: Asleep, Auxiliary: Awake */
} supermct_zone_t;


/** supermct_t
 *
 *    Supermct is parent of related MCTs. and holds related MCT.
 *    Supermct parent-child hierarchy is only created when there is
 *
 *    @object: keeps parent child hierarchy
 *    related_sensors_sync_buf: related camera information
 *    related_sensors_sync_buf_size: related camera buffer size
 *    related_sensors_sync_buf_fd: related camera FD
 *    sof_q_mutex: Mutex for SOF quque
 *    sof_queue: SOF queue to hold SOF messages
 *    supermct_cond: conditional variable to wake up supermct thread
 *    supermct_mutex: mutex for conditional variable
 *    supermct_sof_id: Holds sof id
 *    sof_bus_msg: hold sof bus message
 *
 **/
typedef struct {
  /* Related MCTs can extend for more than two MCTs */
  /* Supermct's parent is NULL */
  /* Supermct's child is pipeline */
  mct_object_t object;

  /* Payload for related session information */
  void       *related_sensors_sync_buf;
  size_t     related_sensors_sync_buf_size;
  int        related_sensors_sync_buf_fd;

  /* Mutex for sof_queue */
  pthread_mutex_t sof_q_mutex;
  mct_queue_t     *sof_queue;

  pthread_cond_t supermct_cond;
  pthread_mutex_t supermct_mutex;

  pthread_t   supermct_tid;
  supermct_zone_t supermct_zone;
  unsigned int    request_number;
  unsigned int    next_skipped_sof;
  unsigned int    last_valid_sof_id;
  pthread_mutex_t update_sof_id_mutex;
  unsigned int    pipeline1_frame_id;
  unsigned int    pipeline2_frame_id;
}supermct_t;

/* Supermct's external function */

/** is_supermct_exist:
 *    It takes pipeline as input and find out parent MCT,
 *    and parent MCT has super MCT or not.
 *
 *    @pipeline: Pipeline as Input to findout supermct exist or not
 *
 *  Return: TRUE, if supermct exist
           FALSE, if supermct do not exist
 **/
boolean is_supermct_exist(mct_pipeline_t *pipeline);

/** is_dual_zone:
 *    It takes pipeline as input and find out parent MCT,
 *    and parent pipelines are awake or not.
 *
 *    @pipeline: Pipeline as Input to findout Dual camera is in dual zone or not
 *
 *  Return: TRUE, if dual zone
           FALSE, if wide/tele zone
 **/
boolean is_dual_zone(mct_pipeline_t *pipeline);

/** supermct_notify:
 *    This function notify/wakeup supermct thread about SOF message
 *
 *    @bus_msg: bus message is input. It is receives from ISP.
 *
 *  Return: TRUE, sucessful
           FALSE, Failure
 **/
boolean supermct_notify(mct_bus_msg_t *bus_msg);

/** supermct_new:
 *    This function create new supermct object.
 *    It takes pipeline as input and adds pipeline parent - child relationship
 *    Plus add respective MCT to supermct parent child list.
 *
 *    @pipeline: it takes pipeline as input.
 *
 *  Return: TRUE, sucessful
 *          FALSE, Failure
 **/
boolean supermct_new(mct_pipeline_t *pipeline);

/** supermct_destroy:
 *    This function destroy supermct object.
 *    It takes pipeline as input and deletes pipeline parent - child relationship
 *
 *    @pipeline: it takes pipeline as input.
 *
 *  Return: VOID
 **/
void supermct_destroy(mct_pipeline_t *pipeline);

/** supermct_update_ack_id:
 *    This function keep track of last sof request has been made.
 *    It takes sof id as input.
 *
 *    @pipeline: it takes pipeline as input.
 *
 *  Return: VOID
 **/
void supermct_update_ack_id(unsigned int sof_id);

/** supermct_lock_sof_update:
 *    This function is to block sof update in supermct.
 *
 *  Return: VOID
 **/
void supermct_lock_sof_update();

/** supermct_unlock_sof_update:
 *    This function is to unblock sof update in supermct.
 *
 *  Return: VOID
 **/
void supermct_unlock_sof_update();
#endif /* __SUPERMCT_H__ */
