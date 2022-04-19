/*============================================================================
Copyright (c) 2013, 2016 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#include <pthread.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <poll.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <stdlib.h>

#include "camera_dbg.h"
#include "isp_event.h"
#include "iface_ops.h"

#include "iface.h"


/** iface_thread_proc_cmd
 *
 * DESCRIPTION: thread to proc cmd sent from pipe,
 *              called by mainloop
 *
 **/
static int iface_thread_proc_cmd(iface_axi_hw_t *axi_hw,
  iface_thread_t *thread_data, int *thread_exit, boolean use_pipe)
{
  ssize_t read_len;
  int rc = 0;
  boolean sem_posted = FALSE;

  if (use_pipe) {
    read_len = read(thread_data->pipe_fds[0],
      &thread_data->cmd_id, sizeof(thread_data->cmd_id));

    if (read_len != sizeof(uint32_t)) {
      /* each cmd takes 4 bytes
         kill the thread and recreate the thread.*/
      CDBG_ERROR("%s: read cmd from pipe error, readlen = %d\n",
        __func__, (int)read_len);
      thread_data->return_code = -EPIPE;
      sem_post(&thread_data->sig_sem);
      *thread_exit = 1;
      return -EPIPE;
    }
  }
  switch (thread_data->cmd_id) {
  case IFACE_THREAD_CMD_NOTIFY_OPS_INIT: {
    if(!thread_data->init_cmd) {
      CDBG_ERROR("%s: ERROR init_cmd is NULL", __func__);
      thread_data->return_code = -200;
      goto end;
    }
    thread_data->return_code = iface_axi_init((void *)axi_hw,
       thread_data->init_cmd->init_params, thread_data->init_cmd->notify_ops);
  }
    break;

  case IFACE_THREAD_CMD_SET_PARAMS: {
    if(!thread_data->set_param_cmd) {
      CDBG_ERROR("%s: ERROR set_param_cmd is NULL", __func__);
      thread_data->return_code = -200;
      goto end;
    }
    thread_data->return_code =
      iface_axi_set_params((void *)axi_hw,
        thread_data->set_param_cmd->params_id,
        thread_data->set_param_cmd->in_params,
        thread_data->set_param_cmd->in_params_size);
  }
    break;

  case IFACE_THREAD_CMD_GET_PARAMS: {
    if(!thread_data->get_param_cmd) {
      CDBG_ERROR("%s: ERROR get_param_cmd is NULL", __func__);
      thread_data->return_code = -200;
      goto end;
    }
    thread_data->return_code = iface_axi_get_params(
       (void *)axi_hw, thread_data->get_param_cmd->params_id,
       thread_data->get_param_cmd->in_params,
       thread_data->get_param_cmd->in_params_size,
       thread_data->get_param_cmd->out_params,
       thread_data->get_param_cmd->out_params_size);
  }
    break;

  case IFACE_THREAD_CMD_ACTION: {
    if(!thread_data->action_cmd) {
      CDBG_ERROR("%s: ERROR action_cmd is NULL", __func__);
      thread_data->return_code = -200;
      goto end;
    }

    /* take care 3 special case here:
       1. start/stop
       2. start/stop ack
       3. wake up at sof cmd, no needed anymore
       4. default proc action cmd*/
    switch(thread_data->action_cmd->action_code) {
    case IFACE_AXI_ACTION_CODE_STREAM_DIVERT_ACK:
    case IFACE_AXI_ACTION_CODE_STREAM_START:
    case IFACE_AXI_ACTION_CODE_STREAM_STOP: {
      uint32_t start_stop_action = thread_data->action_cmd->action_code;
      start_stop_stream_t start_stop_param =
        *((start_stop_stream_t *)thread_data->action_cmd->data);
      uint32_t data_size = thread_data->action_cmd->data_size;
      thread_data->return_code = 0;
      thread_data->action_cmd = NULL;

      /* start stream unblock caller first,
         because kernel axi on/off is blocking call up to 2 frame time*/
      sem_post(&thread_data->sig_sem);
      sem_posted = TRUE;

      thread_data->async_ret = iface_axi_action((void *)axi_hw, start_stop_action,
        &start_stop_param, data_size, thread_data->return_code);
    }
      break;

    case IFACE_AXI_ACTION_CODE_STREAM_START_ACK:
    case IFACE_AXI_ACTION_CODE_STREAM_STOP_ACK: {

      thread_data->return_code =
        iface_axi_action((void *)axi_hw, thread_data->action_cmd->action_code,
        thread_data->action_cmd->data, thread_data->action_cmd->data_size,
        thread_data->async_ret);
      thread_data->action_cmd = NULL;

      sem_post(&thread_data->sig_sem);
      sem_posted = TRUE;
    }
      break;

    default:
      thread_data->return_code =
        iface_axi_action((void *)axi_hw,
        thread_data->action_cmd->action_code, thread_data->action_cmd->data,
        thread_data->action_cmd->data_size, thread_data->async_ret);
      thread_data->action_cmd = NULL;
      break;
    }
  }
    break;

  case IFACE_THREAD_CMD_TIMER:
    break;

  case IFACE_THREAD_CMD_DESTROY:
    /* exit the thread */
    CDBG("%s: HW thread exitting now\n", __func__);
    *thread_exit = 1;
    break;
  }

  if (!sem_posted && thread_data->cmd_id != IFACE_THREAD_CMD_DESTROY) {
    /* zero out the pointer. Since it's union we
       just use the action_code pointer.*/
    thread_data->action_cmd = NULL;
  }

end:
  if (!sem_posted) {
    sem_post(&thread_data->sig_sem);
  }

  return 0;
}

/** iface_axi_halt_immediately
 *
 * DESCRIPTION:
 *
 **/
static void iface_axi_halt_immediately(iface_axi_hw_t *axi_hw __unused)
{
   /*Todo: hal AXI*/
}

/** iface_thread_main_loop
 *
 * DESCRIPTION: main loop to proc cmd or poll v4l2 event
 *
 **/
static void *iface_thread_main_loop(void *data)
{
  int rc = 0, i;
  int timeout;
  int thread_exit = 0;
  iface_thread_t *thread_data = (iface_thread_t *)data;
  iface_axi_hw_t *axi_hw = (iface_axi_hw_t *)thread_data->hw_ptr;

  timeout = thread_data->poll_timeoutms;
  /* wake up the creater first */
  sem_post(&thread_data->sig_sem);
  while(!thread_exit) {
    for(i = 0; i < thread_data->num_fds; i++)
      thread_data->poll_fds[i].events = POLLIN|POLLRDNORM|POLLPRI;

    rc = poll(thread_data->poll_fds, thread_data->num_fds, timeout);
    if(rc > 0) {
      if ((thread_data->poll_fds[0].revents & POLLIN) &&
        (thread_data->poll_fds[0].revents & POLLRDNORM)) {
        /* if we have data on pipe, we only process pipe in this iteration */
        rc = iface_thread_proc_cmd(axi_hw, thread_data, &thread_exit, TRUE);
        if (rc !=0 ) {
          CDBG_WARN("iface_thread_proc_cmd failed !!!\n");
        }
      } else {
        if ((thread_data->poll_fds[1].revents & POLLPRI) ||
          (thread_data->poll_fds[1].revents & POLLIN) ||
          (thread_data->poll_fds[1].revents & POLLRDNORM)) {
          /* if we have data on subdev */
          iface_axi_proc_subdev_event(axi_hw, thread_data);
        }
      }
    } else {
      CDBG_WARN("poll() failed - %s %d",
        strerror(errno), errno);
    }
  }

  if (thread_data->pipe_fds[0] > 0) {
    close(thread_data->pipe_fds[0]);
    thread_data->pipe_fds[0] = 0;
  }
  if (thread_data->pipe_fds[1] > 0) {
    close(thread_data->pipe_fds[1]);
    thread_data->pipe_fds[1] = 0;
  }

  return NULL;
}

/** iface_sem_thread_main
 *
 * DESCRIPTION: thread to proc cmd
 *
 **/
static void *iface_sem_thread_main(void *data)
{
  int rc = 0;
  int thread_exit = 0;
  iface_thread_t *thread_data = (iface_thread_t *)data;
  iface_axi_hw_t *axi_hw = (iface_axi_hw_t *)thread_data->hw_ptr;

  /* wake up the creater first */
  sem_post(&thread_data->sig_sem);

  while(!thread_exit) {
    sem_wait(&thread_data->thread_wait_sem);
    /* use pipe = false*/
    rc = iface_thread_proc_cmd(axi_hw, thread_data, &thread_exit, FALSE);
    if (rc !=0 ) {
      CDBG_WARN("iface_thread_proc_cmd failed !!!\n");
    }
  }

  return NULL;
}
void iface_unmap_kptr(void *ptr)
{
  int rc;
  isp_kstate_ptr_t *p = (isp_kstate_ptr_t *)ptr;

  rc = munmap(p->isp_kstate_ptr, PAGE_SIZE);
  if (rc) {
    CDBG_ERROR("munmap failed %s", strerror(errno));
  }
}
/** iface_thread_start
 *
 * DESCRIPTION:
 *
 **/
int iface_thread_start(iface_thread_t *thread_data, void *hw_ptr, int poll_fd)
{
  int rc = 0;
  iface_axi_hw_t *axi_hw;

  thread_data->hw_ptr = hw_ptr;
  axi_hw = (iface_axi_hw_t *)thread_data->hw_ptr;

  /* initial pipe*/
  rc = pipe(thread_data->pipe_fds);
  if(rc < 0) {
    CDBG_ERROR("%s: pipe open error = %d\n", __func__, rc);
    return -1;
  }

  /*initial poll*/
  thread_data->poll_timeoutms = -1;
  thread_data->poll_fds[0].fd = thread_data->pipe_fds[0];
  thread_data->num_fds = 1;
  thread_data->poll_fd = poll_fd;
  if (poll_fd)
    thread_data->poll_fds[thread_data->num_fds++].fd = poll_fd;

  pthread_mutex_init(&thread_data->cmd_mutex, NULL);
  pthread_mutex_init(&thread_data->busy_mutex, NULL);
  sem_init(&thread_data->sig_sem, 0, 0);
  axi_hw->kstate_ptr.isp_kstate_ptr =
    mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE,
      MAP_SHARED, poll_fd, 0);
  if (axi_hw->kstate_ptr.isp_kstate_ptr == MAP_FAILED) {
    CDBG_ERROR("%s: Mapping Failed\n", __func__);
    axi_hw->kstate_ptr.isp_kstate_ptr = NULL;
  } else {
    MCT_REFCOUNT_INIT_VAL(&(axi_hw->kstate_ptr), refc, 1, iface_unmap_kptr);
  }

  rc = pthread_create(&thread_data->pid, NULL,
    iface_thread_main_loop, (void *)thread_data);
  pthread_setname_np(thread_data->pid, "CAM_iface_poll");
  if(!rc) {
    sem_wait(&thread_data->sig_sem);
  } else {
    CDBG_ERROR("%s: pthread_creat error = %d\n",
      __func__, rc);
    /* setting EPIPE error code triggers exit hw without join thread. */
    thread_data->return_code = -EPIPE;
    rc = thread_data->return_code;
  }

  return rc;
}

/** iface_sem_thread_start
 *
 * DESCRIPTION:
 *
 **/
int iface_sem_thread_start(iface_thread_t *thread_data, void *hw_ptr)
{
    int rc = 0;

    thread_data->hw_ptr = hw_ptr;
    pthread_mutex_init(&thread_data->cmd_mutex, NULL);
    pthread_mutex_init(&thread_data->busy_mutex, NULL);
    sem_init(&thread_data->sig_sem, 0, 0);
    sem_init(&thread_data->thread_wait_sem, 0, 0);

    rc = pthread_create(&thread_data->pid, NULL,
      iface_sem_thread_main, (void *)thread_data);
    pthread_setname_np(thread_data->pid, "CAM_iface_hw");
    if(!rc) {
      sem_wait(&thread_data->sig_sem);
    } else {
      /* setting EPIPE error code triggers exit hw without join thread. */
      CDBG_ERROR("%s: pthread_creat error = %d, thread_data = %p\n",
        __func__, rc, thread_data);
      thread_data->return_code = -EPIPE;
      rc = thread_data->return_code;
    }

    return rc;
}

/** iface_sem_thread_stop
 *
 * DESCRIPTION:
 *
 **/
int iface_sem_thread_stop(iface_thread_t *thread_data)
{

  pthread_mutex_lock(&thread_data->cmd_mutex);
  thread_data->cmd_id = IFACE_THREAD_CMD_DESTROY;
  sem_post(&thread_data->thread_wait_sem);
  pthread_join(thread_data->pid, NULL);
  pthread_mutex_unlock(&thread_data->cmd_mutex);
  sem_destroy(&thread_data->sig_sem);
  sem_destroy(&thread_data->thread_wait_sem);
  pthread_mutex_destroy(&thread_data->cmd_mutex);
  pthread_mutex_destroy(&thread_data->busy_mutex);

  return 0;
}

