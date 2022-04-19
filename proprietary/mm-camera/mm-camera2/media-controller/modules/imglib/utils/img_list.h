/**********************************************************************
* Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                 *
**********************************************************************/

#ifndef __QIMG_LIST_H__
#define __QIMG_LIST_H__

#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>

typedef struct _img_list_node {
  void *data;
  struct _img_list_node *next;
} img_list_node_t;

typedef struct {
  img_list_node_t *p_front;
  img_list_node_t *p_rear;
  int count;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
} img_list_t;

typedef int (*list_wait_cond_func)(void *p_userdata);
typedef int (*list_node_process_func)(void *p_nodedata, void *p_userdata);

/* Functions */
img_list_t *img_list_create(void);
void img_list_free(img_list_t *p_list);
int img_list_append(img_list_t *p_list, void *data);
void *img_list_delist(img_list_t *p_list);
void img_list_flush(img_list_t *p_list);
int img_list_count(img_list_t *p_list);

void *img_list_wait(img_list_t *p_list, list_wait_cond_func wait_cond,
  void *p_userdata);
void img_list_signal(img_list_t *p_list);
int img_list_traverse(img_list_t *p_list, list_node_process_func func,
  void *p_userdata);
void img_list_wait_for_signal(img_list_t *p_list, list_wait_cond_func wait_cond,
  void *p_userdata);


void *img_list_get_last(img_list_t *p_list);
void *img_list_get_first(img_list_t *p_list);
void *img_list_find(img_list_t *p_list, list_node_process_func func,
  void *p_userdata);
int img_list_find_all(img_list_t *p_list, list_node_process_func func,
  void *p_userdata, void *p_data []);
int img_list_remove(img_list_t *p_list, list_node_process_func func,
  void *p_userdata);

#endif //__QIMG_LIST_H__
