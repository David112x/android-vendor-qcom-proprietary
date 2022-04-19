/**********************************************************************
* Copyright (c) 2014,2016-2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#include "img_list.h"
#include "img_common.h"

/**
 * Function: img_list_create
 *
 * Description: Initializes the list
 *
 * Input parameters:
  *
 * Return values:
 *     img_list_t *
 *
 * Notes: none
**/
img_list_t *img_list_create(void)
{
  img_list_t *p_list = (img_list_t *)malloc(sizeof(img_list_t));
  if (NULL == p_list) {
    IDBG_ERROR("%s:%d] Img list create failed", __func__, __LINE__);
    return NULL;
  }
  p_list->p_front = NULL;
  p_list->p_rear = NULL;
  p_list->count = 0;
  pthread_mutexattr_t attr;
  pthread_mutexattr_init(&attr);
  pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
  pthread_mutex_init(&p_list->mutex, &attr);
  pthread_cond_init(&p_list->cond, NULL);

  return p_list;
}

/**
 * Function: img_list_free
 *
 * Description:Free the list
 *
 * Input parameters:
 *   p_list - The pointer to the list
 *
 * Return values:
 *     none
 *
 * Notes: none
**/
void img_list_free(img_list_t *p_list)
{
  img_list_flush(p_list);

  free(p_list);
}

/**
 * Function: img_list_append
 *
 * Description: Append and object to the list
 *
 * Input parameters:
 *   p_list - The pointer to the list
 *   data - the object that needs to be appended
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_NO_MEMORY
 *
 * Notes: none
**/
int img_list_append(img_list_t *p_list, void *data)
{
  img_list_node_t *temp = (img_list_node_t *)malloc(sizeof(img_list_node_t));

  if (NULL == temp) {
    IDBG_ERROR("%s:%d] List append failed", __func__, __LINE__);
    return IMG_ERR_NO_MEMORY;
  }
  temp->data = data;
  temp->next = NULL;
  pthread_mutex_lock(&p_list->mutex);
  if ((NULL == p_list->p_front) || (NULL == p_list->p_rear)) {
    p_list->p_front = p_list->p_rear = temp;
  } else {
    p_list->p_rear->next = temp;
    p_list->p_rear = temp;
  }
  p_list->count++;
  pthread_mutex_unlock(&p_list->mutex);
  return IMG_SUCCESS;
}

/**
 * Function: img_list_remove_head
 *
 * Description: Removes an element from the list. No mutex protection is done
 *   in this function. It needs to be private for this file. It needs to be
 *   invoked only from functions that are protecting access to the list
 *
 * Input parameters:
 *   p_list - The pointer to the list
 *
 * Return values:
 *     Valid object pointer
 *     NULL if the list is empty
 *
 * Notes: none
**/
static void *img_list_remove_head(img_list_t *p_list)
{
  void *data = NULL;
  img_list_node_t *node = NULL;
  if ((NULL == p_list->p_front) || (NULL == p_list->p_rear)) {
    IDBG_ERROR("%s:%d] Img list is empty", __func__, __LINE__);
    return NULL;
  }
  data = p_list->p_front->data;
  node = p_list->p_front->next;
  free(p_list->p_front);
  p_list->p_front = node;
  p_list->count--;
  if (p_list->count==0) {
    p_list->p_rear = p_list->p_front = NULL;
  }
  return data;
}

/**
 * Function: img_list_delist
 *
 * Description: Delist an element from the list head
 *
 * Input parameters:
 *   p_list - The pointer to the list
 *
 * Return values:
 *     Valid object pointer
 *     NULL if the list is empty
 *
 * Notes: none
**/
void *img_list_delist(img_list_t *p_list)
{
  void *data = NULL;

  pthread_mutex_lock(&p_list->mutex);
  data = img_list_remove_head(p_list);
  pthread_mutex_unlock(&p_list->mutex);

  return data;
}

/**
 * Function: img_list_flush
 *
 * Description: Flushes the list
 *
 * Input parameters:
 *   p_list - The pointer to the list
 *
 * Return values:
 *     none
 *
 * Notes: none
**/
void img_list_flush(img_list_t *p_list)
{
  void *temp = NULL;

  pthread_mutex_lock(&p_list->mutex);
  while (p_list->p_front) {
    temp = img_list_remove_head(p_list);
    if (temp) {
      free(temp);
    }
  }
  pthread_mutex_unlock(&p_list->mutex);
}

/**
 * Function: img_list_count
 *
 * Description: Returns the number of elements in the list
 *
 * Input parameters:
 *   p_list - The pointer to the list
 *
 * Return values:
 *    Number of elements in the list
 *
 * Notes: none
**/
int img_list_count(img_list_t *p_list)
{
  int count = 0;
  pthread_mutex_lock(&p_list->mutex);
  count = p_list->count;
  pthread_mutex_unlock(&p_list->mutex);
  return count;
}

/**
 * Function: img_list_wait
 *
 * Description: waits till the element is available in list or external
 *              condition is not met
 *
 * Input parameters:
 *   p_list - The pointer to the list
 *   list_wait_cond - pointer to the external condition
 *   p_userdata - userdata pointer
 *
 * Return values:
 *     valid data pointer
 *     NULL if external condition is met
 *
 * Notes: none
**/
void *img_list_wait(img_list_t *p_list, list_wait_cond_func wait_cond,
  void *p_userdata)
{
  void *data = NULL;
  if (NULL == wait_cond) {
    return NULL;
  }
  pthread_mutex_lock(&p_list->mutex);
  if ((p_list->count <= 0) && wait_cond(p_userdata)) {
    IDBG_MED("%s: before wait", __func__);
    pthread_cond_wait(&p_list->cond, &p_list->mutex);
  }
  IDBG_MED("%s:%d] after wait count %d", __func__, __LINE__, p_list->count);
  if (wait_cond(p_userdata))
    data = img_list_remove_head(p_list);
  pthread_mutex_unlock(&p_list->mutex);
  IDBG_MED("%s:%d] data %p", __func__, __LINE__, data);
  return data;
}

/**
 * Function: img_list_wait_for_signal
 *
 * Description: waits till the element is available in list or external
 *              condition is not met. The data wont be removed
 *              from the list
 *
 * Input parameters:
 *   p_list - The pointer to the list
 *   list_wait_cond - pointer to the external condition
 *   p_userdata - userdata pointer
 *
 * Return values:
 *     valid data pointer
 *     NULL if external condition is met
 *
 * Notes: none
**/
void img_list_wait_for_signal(img_list_t *p_list, list_wait_cond_func wait_cond,
  void *p_userdata)
{
  if (NULL == wait_cond) {
    return;
  }
  pthread_mutex_lock(&p_list->mutex);
  if ((p_list->count <= 0) && wait_cond(p_userdata)) {
    IDBG_MED("%s: before wait", __func__);
    pthread_cond_wait(&p_list->cond, &p_list->mutex);
  }
  IDBG_MED("%s:%d] after wait count %d", __func__, __LINE__, p_list->count);
  pthread_mutex_unlock(&p_list->mutex);
  return;
}

/**
 * Function: img_list_signal
 *
 * Description: signals the list
 *
 * Input parameters:
 *   p_list - The pointer to the list
 *
 * Return values:
 *     valid data pointer
 *     NULL if external condition is met
 *
 * Notes: none
**/
void img_list_signal(img_list_t *p_list)
{
  pthread_mutex_lock(&p_list->mutex);
  pthread_cond_signal(&p_list->cond);
  pthread_mutex_unlock(&p_list->mutex);
}

/**
 * Function: img_list_traverse
 *
 * Description: Executes specified function with the data from each node
 *
 * Input parameters:
 *   p_list - The pointer to the list
 *   func - The pointer function to be executed with the data from each node
 *   p_userdata - User data assiciated with this function
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_INPUT
 *
 * Notes: none
**/
int img_list_traverse(img_list_t *p_list, list_node_process_func func,
  void *p_userdata)
{
  img_list_node_t *node;

  if (!func) {
    IDBG_ERROR("%s:%d] Invalid input", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  pthread_mutex_lock(&p_list->mutex);

  for (node = p_list->p_front; node; node = node->next) {
    func(node->data, p_userdata);
  }

  pthread_mutex_unlock(&p_list->mutex);

  return IMG_SUCCESS;
}


/**
 * Function: img_list_get_last
 *
 * Description: Gets the last entry from the list
 *
 * Input parameters:
 *   p_list - The pointer to the list
 *
 * Return values:
 *     data pointer stored in the last node
 *     NULL if list is empty
 *
 * Notes: none
**/
void *img_list_get_last(img_list_t *p_list)
{
  void *p_data = NULL;

  pthread_mutex_lock(&p_list->mutex);
  if (p_list->p_rear) {
    p_data = p_list->p_rear->data;
  }
  pthread_mutex_unlock(&p_list->mutex);
  return p_data;
}

/**
 * Function: img_list_get_first
 *
 * Description: Gets the first entry from the list
 *
 * Input parameters:
 *   p_list - The pointer to the list
 *
 * Return values:
 *     data pointer stored in the first node
 *     NULL if list is empty
 *
 * Notes: none
**/
void *img_list_get_first(img_list_t *p_list)
{
  void *p_data = NULL;

  pthread_mutex_lock(&p_list->mutex);
  if (p_list->p_front) {
    p_data = p_list->p_front->data;
  }
  pthread_mutex_unlock(&p_list->mutex);
  return p_data;
}

/**
 * Function: img_list_find
 *
 * Description: Executes specified function with the data to find the specific node
 *                   and return the data of it
 *
 * Input parameters:
 *   p_list - The pointer to the list
 *   func - The pointer function to be executed with the data
 *   p_userdata - User data assiciated with this function
 *
 * Return values:
 *     data pointer stored in the found node
 *     NULL if the node cannot be found
 *
 * Notes: none
**/
void *img_list_find(img_list_t *p_list, list_node_process_func func,
  void *p_userdata)
{
  img_list_node_t *node;
  void *p_data = NULL;

  if (!func) {
    IDBG_ERROR("%s:%d] Invalid input", __func__, __LINE__);
    return NULL;
  }

  pthread_mutex_lock(&p_list->mutex);

  for (node = p_list->p_front; node; node = node->next) {
    if (func(node->data, p_userdata)) {
      //assume if the user function returns TRUE means found the node
      p_data = node->data;
    }
  }

  pthread_mutex_unlock(&p_list->mutex);

  return p_data;
}

/**
 * Function: img_list_find_all
 *
 * Description: Executes specified function with the data the to
 * find all the nodes and return the data and count of it
 *
 * Input parameters:
 *   p_list - The pointer to the list
 *   func - The pointer function to be executed with the data
 *   p_userdata - User data assiciated with this function
 *   p_data: Array of pointers to hold the result
 *
 * Return values:
 *     data pointer stored in the found node
 *     NULL if the node cannot be found
 *
 * Notes: none
**/
int img_list_find_all(img_list_t *p_list, list_node_process_func func,
  void *p_userdata, void *p_data [])
{
  img_list_node_t *node;
  int count = 0;

  if ((!func) || (!p_data)) {
    IDBG_ERROR("%s:%d] Invalid input", __func__, __LINE__);
    return 0;
  }

  pthread_mutex_lock(&p_list->mutex);

  for (node = p_list->p_front; node; node = node->next) {
    if (func(node->data, p_userdata)) {
      //assume if the user function returns TRUE means found the node
      p_data[count] = node->data;
      count ++;
    }
  }
  pthread_mutex_unlock(&p_list->mutex);
  return count;

}

/**
 * Function: img_list_remove
 *
 * Description: Executes specified function with the data to find the specific node
 *                   and remove the node from the list
 *
 * Input parameters:
 *   p_list - The pointer to the list
 *   func - The pointer function to be executed with the data
 *   p_userdata - User data assiciated with this function
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_INPUT
 *
 * Notes: none
**/
int img_list_remove(img_list_t *p_list, list_node_process_func func,
  void *p_userdata)
{
  img_list_node_t *node;
  img_list_node_t *tmp;
  int rc = IMG_SUCCESS;;

  if (!func) {
    IDBG_ERROR("%s:%d] Invalid input", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  pthread_mutex_lock(&p_list->mutex);

  tmp = p_list->p_front;
  for (node = p_list->p_front; node; node = node->next) {
    if (func(node->data, p_userdata)) {
      //assume if the user function returns TRUE means found the node
      if (node == p_list->p_front) {
        p_list->p_front = node->next;
      }
      else {
        tmp->next = node->next;
      }

      if (node == p_list->p_rear) {
        p_list->p_rear = tmp;
      }

      if (node->data) {
        free(node->data);
      }
      p_list->count--;
      if (p_list->count==0) {
        p_list->p_rear = p_list->p_front = NULL;
      }
      free(node);
      rc = IMG_SUCCESS;
      break;
    }
    tmp = node;
  }

  pthread_mutex_unlock(&p_list->mutex);

  return rc;
}


