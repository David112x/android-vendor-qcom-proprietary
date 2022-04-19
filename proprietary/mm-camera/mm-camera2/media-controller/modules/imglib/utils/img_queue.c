/**********************************************************************
* Copyright (c) 2013,2016,2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#include "img_queue.h"
#include "img_common.h"

/**
 * Function: img_q_init
 *
 * Description: Initializes the queue
 *
 * Input parameters:
 *   p_q - The pointer to the queue
 *   name - name of the queue
 *
 * Return values:
 *     none
 *
 * Notes: none
**/
void img_q_init(img_queue_t *p_q, char *name)
{
  p_q->p_front = NULL;
  p_q->p_rear = NULL;
  p_q->count = 0;
  p_q->name = name;
  pthread_mutex_init(&p_q->mutex, NULL);
  pthread_cond_init(&p_q->cond, NULL);
  p_q->initialized = TRUE;
}

/**
 * Function: img_q_deinit
 *
 * Description: De-Initializes the queue
 *
 * Input parameters:
 *   p_q - The pointer to the queue
 *
 * Return values:
 *     none
 *
 * Notes: none
**/
void img_q_deinit(img_queue_t *p_q)
{
  if (p_q->initialized != TRUE)
    return;

  img_q_flush(p_q);
  pthread_cond_destroy(&p_q->cond);
  pthread_mutex_destroy(&p_q->mutex);
  memset(p_q, 0x0, sizeof(img_queue_t));
}

/**
 * Function: img_q_enqueue
 *
 * Description: Enqueue and object to the queue
 *
 * Input parameters:
 *   p_q - The pointer to the queue
 *   data - the object that needs to be enqueued
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_NO_MEMORY
 *
 * Notes: none
**/
int img_q_enqueue(img_queue_t *p_q, void *data)
{
  img_node_t *temp = (img_node_t *)malloc(sizeof(img_node_t));

  if (NULL == temp) {
    IDBG_ERROR("Enqueue failed");
    return IMG_ERR_NO_MEMORY;
  }
  temp->data = data;
  temp->next = NULL;
  pthread_mutex_lock(&p_q->mutex);
  if ((NULL == p_q->p_front) || (NULL == p_q->p_rear)) {
    p_q->p_front = p_q->p_rear = temp;
  } else {
    p_q->p_rear->next = temp;
    p_q->p_rear = temp;
  }
  p_q->count++;
  pthread_mutex_unlock(&p_q->mutex);
  return IMG_SUCCESS;
}

/**
 * Function: img_q_remove
 *
 * Description: Removes an element from the queue. No mutex protection is done
 *   in this function. It needs to be private for this file. It needs to be
 *   invoked only from functions that are protecting access to teh queue
 *
 * Input parameters:
 *   p_q - The pointer to the queue
 *
 * Return values:
 *     Valid object pointer
 *     NULL if the queue is empty
 *
 * Notes: none
**/
static void *img_q_remove(img_queue_t *p_q)
{
  void* data = NULL;
  img_node_t* node = NULL;
  if ((NULL == p_q->p_front) || (NULL == p_q->p_rear)) {
    IDBG_ERROR("Dequeue failed");
    return NULL;
  }
  data = p_q->p_front->data;
  node = p_q->p_front->next;
  if (p_q->p_front == p_q->p_rear) {
    p_q->p_rear = NULL;
  }
  free(p_q->p_front);
  p_q->p_front = node;
  p_q->count--;
  return data;
}

/**
 * Function: img_q_dequeue
 *
 * Description: Dequeue an element from the queue
 *
 * Input parameters:
 *   p_q - The pointer to the queue
 *
 * Return values:
 *     Valid object pointer
 *     NULL if the queue is empty
 *
 * Notes: none
**/
void *img_q_dequeue(img_queue_t *p_q)
{
  void* data = NULL;

  pthread_mutex_lock(&p_q->mutex);
  if (p_q->count > 0)
    data = img_q_remove(p_q);
  pthread_mutex_unlock(&p_q->mutex);

  return data;
}


/**
 * Function: img_q_peek
 *
 * Description: Peek at the first element from the queue
 *
 * Input parameters:
 *   p_q - The pointer to the queue
 *
 * Return values:
 *     Valid object pointer
 *     NULL if the queue is empty
 *
 * Notes: none
**/
void *img_q_peek(img_queue_t *p_q)
{
  void* data = NULL;

  pthread_mutex_lock(&p_q->mutex);
  if ((NULL == p_q->p_front) || (NULL == p_q->p_rear)) {
    pthread_mutex_unlock(&p_q->mutex);
    IDBG_WARN("Peek failed");
    return NULL;
  }

  data = p_q->p_front->data;
  pthread_mutex_unlock(&p_q->mutex);

  return data;
}

/**
 * Function: img_q_flush
 *
 * Description: Flushes the queue
 *
 * Input parameters:
 *   p_q - The pointer to the queue
 *
 * Return values:
 *     none
 *
 * Notes: none
**/
void img_q_flush(img_queue_t *p_q)
{
  void* temp = NULL;

  pthread_mutex_lock(&p_q->mutex);
  while (p_q->count > 0)
    temp = img_q_remove(p_q);
  pthread_mutex_unlock(&p_q->mutex);
}

/**
 * Function: img_q_flush_and_destroy
 *
 * Description: Flushes and free elements of the queue
 *
 * Input parameters:
 *   p_q - The pointer to the queue
 *
 * Return values:
 *   number of elements in the queue
 *
 * Notes: none
**/
void img_q_flush_and_destroy(img_queue_t *p_q)
{
  void* temp = NULL;

  pthread_mutex_lock(&p_q->mutex);
  while (p_q->count > 0) {
    temp = img_q_remove(p_q);
    if (temp)
      free(temp);
  }
  pthread_mutex_unlock(&p_q->mutex);
}

/**
 * Function: img_q_count_locked
 *
 * Description: Returns the number of elements in the queue
 *   but locks queue mutex before looking at the count
 * Input parameters:
 *   p_q - The pointer to the queue
 *
 * Return values:
 *   number of elements in the queue
 *
 * Notes: none
**/
int img_q_count_locked(img_queue_t *p_q)
{
  int count;
  pthread_mutex_lock(&p_q->mutex);
  count = p_q->count;
  pthread_mutex_unlock(&p_q->mutex);
  return count;
}

/**
 * Function: img_q_count
 *
 * Description: Returns the number of elements in the queue
 *
 * Input parameters:
 *   p_q - The pointer to the queue
 *
 * Return values:
 *     none
 *
 * Notes: none
**/
int img_q_count(img_queue_t *p_q)
{
  return p_q->count;
}

/**
 * Function: img_q_wait
 *
 * Description: waits till the element is available in queue or external
 *              condition is not met
 *
 * Input parameters:
 *   p_q - The pointer to the queue
 *   q_wait_cond - pointer to the external condition
 *   p_userdata - userdata pointer
 *
 * Return values:
 *     valid data pointer
 *     NULL if external condition is met
 *
 * Notes: none
**/
void *img_q_wait(img_queue_t *p_q, q_wait_cond_func wait_cond,
  void *p_userdata)
{
  void *data = NULL;
  if (NULL == wait_cond) {
    return NULL;
  }
  pthread_mutex_lock(&p_q->mutex);
  while (1) {
    if ((p_q->count <= 0) && wait_cond(p_userdata)) {
      IDBG_MED("%s: before wait", __func__);
      pthread_cond_wait(&p_q->cond, &p_q->mutex);
    }
    IDBG_MED("%s:%d] after wait count %d", __func__, __LINE__, p_q->count);
    if (wait_cond(p_userdata)) {
      data = img_q_remove(p_q);

      // wait_cond is true, but we didn't have a message in the Queue, keep waiting.
      // This could be possible in the following scenario:
      //   1. Thread1 : BUF_DIVERT : enqueue message and signal cond_wait
      //   2. Thread2: STREAM_OFF : Flush the Queue
      //   3. Thread3 comes out of cond_wait because of 'signal' from '1' above
      //      Finds no message in the Queue (as msg_q is flushed in STREAM_OFF)
      //      and data could be NULL.
      // We dont need to exit in this case, we can keep waiting.
      if (data)
        break;
    } else {
      // wait_cond is false, need to exit
      break;
    }
  }
  pthread_mutex_unlock(&p_q->mutex);
  IDBG_MED("%s:%d] data %p", __func__, __LINE__, data);
  return data;
}

/**
 * Function: img_q_wait_for_signal
 *
 * Description: waits till the element is available in queue or external
 *              condition is not met. The data wont be dequeued
 *              from the queue
 *
 * Input parameters:
 *   p_q - The pointer to the queue
 *   q_wait_cond - pointer to the external condition
 *   p_userdata - userdata pointer
 *
 * Return values:
 *     valid data pointer
 *     NULL if external condition is met
 *
 * Notes: none
**/
void img_q_wait_for_signal(img_queue_t *p_q, q_wait_cond_func wait_cond,
  void *p_userdata)
{
  if (NULL == wait_cond) {
    return;
  }
  pthread_mutex_lock(&p_q->mutex);
  if ((p_q->count <= 0) && wait_cond(p_userdata)) {
    IDBG_MED("%s: before wait", __func__);
    pthread_cond_wait(&p_q->cond, &p_q->mutex);
  }
  IDBG_MED("%s:%d] after wait count %d", __func__, __LINE__, p_q->count);
  pthread_mutex_unlock(&p_q->mutex);
  return;
}

/**
 * Function: img_q_signal
 *
 * Description: signals the queue
 *
 * Input parameters:
 *   p_q - The pointer to the queue
 *
 * Return values:
 *     valid data pointer
 *     NULL if external condition is met
 *
 * Notes: none
**/
void img_q_signal(img_queue_t *p_q)
{
  pthread_mutex_lock(&p_q->mutex);
  pthread_cond_signal(&p_q->cond);
  pthread_mutex_unlock(&p_q->mutex);
}

/**
 * Function: img_q_traverse
 *
 * Description: Executes specified function with the data from each node
 *
 * Input parameters:
 *   p_q - The pointer to the queue
 *   func - The pointer function to be executed with the data from each node
 *   p_userdata - User data assiciated with this function
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_INPUT
 *
 * Notes: none
**/
int img_q_traverse(img_queue_t *p_q, q_node_process_func func,
  void *p_userdata)
{
  img_node_t* node;

  if (!func) {
    IDBG_ERROR("Invalid input");
    return IMG_ERR_INVALID_INPUT;
  }

  pthread_mutex_lock(&p_q->mutex);

  for (node = p_q->p_front; node; node = node->next) {
    func(node->data, p_userdata);
  }

  pthread_mutex_unlock(&p_q->mutex);

  return IMG_SUCCESS;
}

/**
 * Function: img_q_traverse_dequeue
 *
 * Description: Executes specified condition function with the
 * data from each node and dequeue node when condition met.
 *
 * Input parameters:
 *   p_q - The pointer to the queue.
 *   func - The pointer condition function to be executed with
 *     the data from each node.
 *   p_userdata - User data associated with this function.
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_INPUT
 *
 * Notes: none
**/
void *img_q_traverse_dequeue(img_queue_t *p_q, q_node_process_func func,
  void *p_userdata)
{
  img_node_t* node_prev;
  img_node_t* node_curr;
  void* data = NULL;

  if (!p_q || !func || !p_userdata) {
    IDBG_ERROR("Invalid input %p %p %p", p_q, func, p_userdata);
    return NULL;
  }

  pthread_mutex_lock(&p_q->mutex);
  node_prev = NULL;
  node_curr = p_q->p_front;

  while (node_curr) {
    if (func(node_curr->data, p_userdata)) {
      data = node_curr->data;
      if (node_prev == NULL) {
        if (p_q->p_front == p_q->p_rear) {
          p_q->p_rear = NULL;
        }
        p_q->p_front = node_curr->next;
      } else {
        if (node_curr->next == NULL) {
          p_q->p_rear = node_prev;
        }
        node_prev->next = node_curr->next;
      }
      free(node_curr);
      p_q->count--;
      pthread_mutex_unlock(&p_q->mutex);
      return data;
    }
    node_prev = node_curr;
    node_curr = node_curr->next;
  }
  pthread_mutex_unlock(&p_q->mutex);

  return NULL;
}

/**
 * Function: img_q_get_last_entry
 *
 * Description: Gets the last entry from the queue
 *         Iterates though the queue elements
 *
 * Input parameters:
 *   p_q - The pointer to the queue
 *   func - function pointer to be invoked
 *   p_userdata - userdata pointer
 *
 * Return values:
 *     none
 *
 * Notes: none
**/
void *img_q_get_last_entry(img_queue_t *p_q,
  q_node_process_func func,
  void *p_userdata)
{
  void* p_lastentry = NULL;

  pthread_mutex_lock(&p_q->mutex);
  do {
    if (func && p_lastentry)
      func(p_lastentry, p_userdata);
    p_lastentry = img_q_remove(p_q);
  } while (p_q->count > 0);
  pthread_mutex_unlock(&p_q->mutex);
  return p_lastentry;
}

/**
 * Function: img_q_flush_traverse
 *
 * Description: Flushes the queue and traverse
 *
 * Input parameters:
 *   @p_q - The pointer to the queue
 *   @func: function pointer to execute
 *   @p_userdata: userdata
 *
 * Return values:
 *     none
 *
 * Notes: none
**/
void img_q_flush_traverse(img_queue_t *p_q,
  q_node_process_func func,
  void *p_userdata)
{
  void* temp = NULL;

  pthread_mutex_lock(&p_q->mutex);
  while (p_q->count > 0) {
    temp = img_q_remove(p_q);
    if (func && temp)
      func(temp, p_userdata);
  }
  pthread_mutex_unlock(&p_q->mutex);
}

