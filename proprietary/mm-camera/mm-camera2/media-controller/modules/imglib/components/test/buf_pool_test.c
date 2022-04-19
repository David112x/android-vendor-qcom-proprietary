/**********************************************************************
*  Copyright (c) 2015-2016 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/
#include "img_test.h"
#include "img_buffer.h"
#include "img_thread.h"

#define NUM_THREAD 4

/**
 * buf pool tasks
 */
typedef struct {
  img_frame_t frame;
  char *filename;
  void *p_buftest;
  int idx;
  uint32_t job_id;
} test_task_t;

/**
 * Buffer pool test structure
 */
typedef struct {
  int32_t client_id;
  imglib_test_t *p_test;
  img_core_type_t thread_affinity[NUM_THREAD];
  img_thread_job_params_t params[NUM_THREAD];
  test_task_t task[NUM_THREAD];
  uint32_t dep_job_ids[1];
  void *buf_handle;
} bufpool_test_t;


static bufpool_test_t g_test_obj;

/**
 * Function: bufpool_process_frame
 *
 * Description: execute bufpool test
 *
 * Input parameters:
 *   p_test - test object
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int bufpool_process_frame(void *param)
{
  test_task_t *p_task = (test_task_t *)param;
  img_frame_t *p_s_frame;
  int32_t rc, i;
  char name[100];

  snprintf(name, sizeof(name), "buf_pool%d", p_task->idx);
  for (i = 0; i < (int32_t)g_test_obj.p_test->in_count; i++) {
    p_task->frame = g_test_obj.p_test->frame[i];
    p_task->frame.frame_id = i + 1;
    IDBG_ERROR("%s:%d] thread %d frame %d dim %dx%d", __func__, __LINE__,
      p_task->idx, p_task->frame.frame_id,
      g_test_obj.p_test->frame[0].info.width,
      g_test_obj.p_test->frame[0].info.height);

    rc = img_buf_pool_get_frame(g_test_obj.buf_handle, -1,
      &p_task->frame,
      &p_s_frame,
      name);
    IMG_TEST_ERR(rc);

    img_dump_frame(p_s_frame, p_task->filename, p_task->idx, NULL);

    IDBG_ERROR("%s:%d] Release frame thread %d frame %d", __func__, __LINE__,
      p_task->idx, p_task->frame.frame_id);

    img_buf_pool_release_frame(g_test_obj.buf_handle, -1,
      p_s_frame->frame_id, name);
  }
  return IMG_SUCCESS;

error:
  return rc;
} /*bufpool_process_frame*/

/**
 * Function: bufpool_test_execute
 *
 * Description: execute bufpool test case
 *
 * Input parameters:
 *   p_test - test object
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int bufpool_test_execute(imglib_test_t *p_test)
{
  int i;
  img_buf_params_t buf_param;
  int rc = img_thread_mgr_create_pool();
  IMG_TEST_ERR(rc);

  IDBG_HERE;
  g_test_obj.p_test = p_test;
  for (i = 0; i < NUM_THREAD; i++)
    g_test_obj.thread_affinity[i] = IMG_CORE_ANY;

  IDBG_HERE;
  g_test_obj.client_id = img_thread_mgr_reserve_threads(NUM_THREAD,
    g_test_obj.thread_affinity);
  if (!g_test_obj.client_id) {
    rc = IMG_ERR_GENERAL;
    IDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    goto error;
  }

  IDBG_HERE;
  buf_param.min_dim = 480;
  g_test_obj.buf_handle = img_buf_pool_create(&buf_param);
  if (!g_test_obj.buf_handle)
    rc = IMG_ERR_NO_MEMORY;
  IDBG_HERE;

  for (i = 0; i < NUM_THREAD; i++) {
    g_test_obj.params[i].client_id = g_test_obj.client_id;
    g_test_obj.params[i].core_affinity = IMG_CORE_ANY;
    g_test_obj.params[i].dep_job_count = 0;
    g_test_obj.params[i].dep_job_ids = g_test_obj.dep_job_ids;
    g_test_obj.params[i].args = &g_test_obj.task[i];
    g_test_obj.task[i].p_buftest = &g_test_obj;
    g_test_obj.task[i].idx = i;
    g_test_obj.task[i].filename = "img_pool";
    g_test_obj.params[i].execute = bufpool_process_frame;
    g_test_obj.task[i].job_id =
      img_thread_mgr_schedule_job(&g_test_obj.params[i]);
    IMG_TEST_ERR(rc);
    IDBG_HERE;
  }

  IDBG_HERE;
  rc = img_thread_mgr_wait_for_completion_by_clientid(g_test_obj.client_id,
    5000);
  IMG_TEST_ERR(rc);
  IDBG_HERE;

error:

  img_buf_pool_destroy(g_test_obj.buf_handle);
  img_thread_mgr_unreserve_threads(g_test_obj.client_id);
  img_thread_mgr_destroy_pool();
  return rc;
} /*bufpool_test_execute*/
