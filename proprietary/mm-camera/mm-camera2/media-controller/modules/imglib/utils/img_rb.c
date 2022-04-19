/**********************************************************************
* Copyright (c) 2015 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#include "img_rb.h"
#include "img_common.h"

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
  uint32_t array_size)
{
  if (!p_data || !data_size || !array_size) {
    IDBG_ERROR("%s:%d] Failed %p %d %d", __func__, __LINE__,
      p_data, data_size, array_size);
    return IMG_ERR_INVALID_INPUT;
  }
  p_rb->count = p_rb->front = p_rb->rear = 0;
  p_rb->p_data = p_data;
  p_rb->data_size = data_size;
  p_rb->max_count = array_size/data_size;
  return IMG_SUCCESS;
}

/** img_rb_enqueue:
 *
 *  Arguments:
 *    @p_rb: Pointer to ring buffer
 *    @p_data: data to be inserted
 *
 *  Return:
 *       IMG_SUCCESS or IMG_ERR_OUT_OF_BOUNDS incase of error
 *
 *  Description:
 *       enqueue an element into ring buffer
 *
 **/
int32_t img_rb_enqueue(img_rb_t *p_rb, uint8_t *p_data)
{
  int32_t rc = IMG_SUCCESS;
  if (img_rb_full(p_rb)) {
    rc = IMG_ERR_OUT_OF_BOUNDS;
  } else {
    memcpy(&p_rb->p_data[p_rb->rear * p_rb->data_size], p_data,
      p_rb->data_size);
    p_rb->rear = (p_rb->rear + 1) % p_rb->max_count;
    p_rb->count++;
  }
  return rc;
}

/** img_rb_dequeue:
 *
 *  Arguments:
 *    @p_rb: Pointer to ring buffer
 *    @p_data: data to be inserted
 *
 *  Return:
 *       valid data, -1 incase of error
 *
 *  Description:
 *       dequeue an element from the ring buffer
 *
 **/
void *img_rb_deque(img_rb_t *p_rb)
{
  uint8_t *p_data;
  if (img_rb_empty(p_rb)) {
    p_data = NULL;
  } else {
    p_data = &p_rb->p_data[p_rb->front * p_rb->data_size];
    p_rb->front = (p_rb->front + 1) % p_rb->max_count;
    p_rb->count--;
  }
  return p_data;
}

/** img_rb_enqueue_idx:
 *
 *  Arguments:
 *    @p_rb: Pointer to ring buffer
 *
 *  Return:
 *       Valid index, -1 incase of error
 *
 *  Description:
 *       Returns the index of ring buffer in which the data
 *       needs to be inserted
 *
 **/
int32_t img_rb_enqueue_idx(img_rb_t *p_rb)
{
  int32_t index;
  if (img_rb_full(p_rb)) {
    index = -1;
  } else {
    index = p_rb->rear;
    p_rb->rear = (p_rb->rear + 1) % p_rb->max_count;
    p_rb->count++;
  }
  return index;
}

/** img_rb_deque_idx:
 *
 *  Arguments:
 *    @p_rb: Pointer to ring buffer
 *
 *  Return:
 *       Valid index, -1 incase of error
 *
 *  Description:
 *       Returns the index from which data can be dequeued from
 *       the ring buffer
 *
 **/
int32_t img_rb_deque_idx(img_rb_t *p_rb)
{
  int32_t index;
  if (img_rb_empty(p_rb)) {
    index = -1;
  } else {
    index = p_rb->front;
    p_rb->front = (p_rb->front + 1) % p_rb->max_count;
    p_rb->count--;
  }
  return index;
}

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
int32_t img_rb_peek_idx(img_rb_t *p_rb)
{
  int32_t index;
  if (img_rb_empty(p_rb)) {
    index = -1;
  } else {
    index = p_rb->front;
  }
  return index;
}

