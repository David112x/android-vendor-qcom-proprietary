/**********************************************************************
* Copyright (c) 2015 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/
#ifndef __QIMG_RB_H__
#define __QIMG_RB_H__

#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <stdbool.h>

/****************** Ring Buffer *********************/

/**
 *   @count: Count of elements in the ring buffer
 *   @front: Front index of the ring buffer
 *   @rear: Rear index of the ring buffer
 *   @p_data: pointer to the data stream
 *   @data_size: size of each data
 *   @max_count: maximum number of elements in the ring buffer
 *
 *   Ring buffer structure
 */
typedef struct {
  uint32_t count;
  uint32_t front;
  uint32_t rear;
  uint8_t *p_data;
  uint32_t data_size;
  uint32_t max_count;
} img_rb_t;

/** img_rb_reset:
 *
 *  Arguments:
 *    @p_rb: Pointer to ring buffer
 *    @p_data: pointer to the array head
 *    @data_size: Size of the data
 *    @array_size: Size of the total array
 *
 *  Return:
 *       IMG_ERR_INVALID_INPUT/IMG_SUCCESS
 *
 *  Description:
 *       Resets the ring buffer
 *
 **/
int32_t img_rb_reset(img_rb_t *p_rb,
  uint8_t *p_data,
  uint32_t data_size,
  uint32_t array_size);

/** img_rb_empty:
 *
 *  Arguments:
 *    @p_rb: Pointer to ring buffer
 *
 *  Return:
 *       count of elements in ring buffer
 *
 *  Description:
 *       Returns the number of elements in the ring buffer
 *
 **/
static inline bool img_rb_count(img_rb_t *p_rb)
{
  return p_rb->count;
}

/** img_rb_empty:
 *
 *  Arguments:
 *    @p_rb: Pointer to ring buffer
 *
 *  Return:
 *       none
 *
 *  Description:
 *       check if the ring buffer is empty
 *
 **/
static inline bool img_rb_empty(img_rb_t *p_rb)
{
  return (p_rb->count == 0);
}

/** img_rb_full:
 *
 *  Arguments:
 *    @p_rb: Pointer to ring buffer
 *
 *  Return:
 *       none
 *
 *  Description:
 *       check if the ring buffer is full
 *
 **/
static inline bool img_rb_full(img_rb_t *p_rb)
{
  return (p_rb->count == p_rb->max_count);
}

/** img_rb_enqueue:
 *
 *  Arguments:
 *    @p_rb: Pointer to ring buffer
 *    @p_data: data to be inserted
 *
 *  Return:
 *       true/false
 *
 *  Description:
 *       enqueue an element into ring buffer
 *
 **/
int32_t img_rb_enqueue(img_rb_t *p_rb, uint8_t *p_data);

/** img_rb_dequeue:
 *
 *  Arguments:
 *    @p_rb: Pointer to ring buffer
 *
 *  Return:
 *       valid data pointer or NULL incase of error
 *
 *  Description:
 *       dequeue an element from the ring buffer
 *
 **/
void *img_rb_deque(img_rb_t *p_rb);


/** img_rb_enqueue_idx:
 *
 *  Arguments:
 *    @p_rb: Pointer to ring buffer
 *
 *  Return:
 *       Valid index, -ve img_error values incase of error
 *
 *  Description:
 *       Returns the index of ring buffer in which the data
 *       needs to be inserted
 *
 **/
int32_t img_rb_enqueue_idx(img_rb_t *p_rb);

/** img_rb_deque_idx:
 *
 *  Arguments:
 *    @p_rb: Pointer to ring buffer
 *
 *  Return:
 *       Valid index, -ve img_error values incase of error
 *
 *  Description:
 *       Returns the index from which data can be dequeued from
 *       the ring buffer
 *
 **/
int32_t img_rb_deque_idx(img_rb_t *p_rb);


/** img_rb_peek_idx:
 *
 *  Arguments:
 *    @p_rb: Pointer to ring buffer
 *
 *  Return:
 *       Valid index, -1 incase of error
 *
 *  Description:
 *       Returns the peek index
 *
 **/
int32_t img_rb_peek_idx(img_rb_t *p_rb);

#endif //__QIMG_RB_H__
