/*============================================================================
Copyright (c) 2013,2015 Qualcomm Technologies, Inc. All Rights Reserved.
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
#include <time.h>

#include "camera_dbg.h"
#include "isp_event.h"
#include "iface_ops.h"
#include "iface_def.h"
#include "axi/iface_axi.h"
#include "iface.h"
#include "iface_util.h"
#include "mct_profiler.h"

#define UV_SS_WAIT_ISP_TIMEOUT_CNT 3

/** iface_axi_halt_immediately
 *
 * DESCRIPTION:
 *
 **/
static void iface_axi_halt_immediately(iface_axi_hw_t *axi_hw __unused)
{
   /*Todo: hal AXI*/
}

static int iface_session_thread_proc_resource_alloc(
  iface_session_t        *session,
  iface_session_thread_t *thread_data,
  int                    *thread_exit __unused,
  boolean                *sem_posted)
{
  int rc = 0;
  iface_t *iface = NULL;
  uint32_t i=0;
  iface_stream_info_t  *stream_ptr = NULL;
  enum msm_vfe_axi_stream_src axi_stream_src = VFE_AXI_SRC_MAX;

  /*pass thread resource alloc data to local copy*/
  if (thread_data->resource_alloc.iface == NULL) {
    CDBG_ERROR("%s: iface is NULL\n", __func__);
    return -1;
  }

  iface = (iface_t *) thread_data->resource_alloc.iface;

  memcpy(&(session->nativbuf_alloc_info),
    &thread_data->resource_alloc.buf_alloc_info,sizeof(iface_buf_alloc_t));
  /*reset the thread payload*/
  memset(&thread_data->resource_alloc, 0,
    sizeof(thread_data->resource_alloc));
  thread_data->async_cmd_id = IFACE_ASYNC_COMMAND_INVALID;
  sem_post(&thread_data->sig_sem);
  *sem_posted = TRUE;

  for (i = 0; i < session->nativbuf_alloc_info.num_pix_stream; i++) {
    stream_ptr = &session->nativbuf_alloc_info.stream_info[i];
    axi_stream_src = stream_ptr->axi_stream_src;
    if (axi_stream_src != VFE_AXI_SRC_MAX &&
      stream_ptr->use_native_buffer ) {

      if (session->image_bufs[axi_stream_src][0].vaddr==NULL) {
        rc = iface_util_request_image_bufs(iface, session,
          &(stream_ptr->buf_planes.plane_info),
          axi_stream_src, stream_ptr->need_adsp_heap,
          stream_ptr->num_additional_buffers);

        if (rc < 0) {
          CDBG_ERROR("%s: native buf allocation failed rc=%d\n", __func__, rc);
          pthread_mutex_lock(&thread_data->busy_mutex);
          thread_data->thread_busy = FALSE;
          pthread_mutex_unlock(&thread_data->busy_mutex);
          return -1;
        }
      }
    }
  }

  pthread_mutex_lock(&thread_data->busy_mutex);
  thread_data->thread_busy = FALSE;
  pthread_mutex_unlock(&thread_data->busy_mutex);

  return rc;
}


/** iface_thread_proc_cmd
 *
 * DESCRIPTION: thread to proc cmd sent from pipe,
 *              called by mainloop
 *
 **/
static int iface_session_thread_proc_cmd(iface_session_t *session,
  iface_session_thread_t *thread_data, int *thread_exit)
{

  int rc = 0;
  iface_t *iface = NULL;
  boolean sem_posted = FALSE;
  struct timespec timeout;

  if (thread_data->async_cmd_id == IFACE_ASYNC_COMMAND_INVALID) {
    CDBG_ERROR("%s: invalid asny cmd id = %d, rc = -1\n",
      __func__, thread_data->async_cmd_id);
    rc = -1;
    goto end;
  }

  if (session == NULL) {
    CDBG_ERROR("%s: NULL, session %p\n", __func__, session);
    rc = -1;
    goto end;
  }

  iface = (iface_t *)session->iface;
  if (!iface) {
    CDBG_ERROR("%s: NULL, iface %p\n", __func__, iface);
    rc=  -1;
    goto end;
  }

  switch(thread_data->async_cmd_id) {
  case IFACE_ASYNC_COMMAND_EXIT: {
    *thread_exit = 1;
  }
    break;

  case IFACE_ASYNC_COMMAND_RECOVERY : {
    CDBG_HIGH("%s: SESSION CMD RECOVERY\n", __func__);
   iface_session_cmd_recovery_cfg_t  recovery_param =
       thread_data->recovery_param;

    /*reset data after deep copy to local parm*/
    memset(&thread_data->recovery_param, 0,
      sizeof(thread_data->recovery_param));
    thread_data->async_cmd_id = IFACE_ASYNC_COMMAND_INVALID;

    /*streamon, pass parameter from thread data*/
    thread_data->return_code = iface_halt_recovery(recovery_param.iface,
      &recovery_param.session_id, recovery_param.isp_id, recovery_param.frame_id);

    pthread_mutex_lock(&thread_data->busy_mutex);
    thread_data->thread_busy = FALSE;
    pthread_mutex_unlock(&thread_data->busy_mutex);

    /* block until both axi done streamon*/
    sem_post(&thread_data->sig_sem);
    sem_posted = TRUE;

  }
    break;

  case IFACE_ASYNC_COMMAND_OFFLINE_CONFIG: {
    CDBG_HIGH("%s: SESSION CMD OFFLINE CONFIG\n", __func__);


    /* unblock caller thread */
    sem_post(&thread_data->sig_sem);
    sem_posted = TRUE;

    /*pass thread offline data to local copy*/
    iface_session_cmd_offline_cfg_t offline_cfg_parm =
       *((iface_session_cmd_offline_cfg_t *)&thread_data->offline_cfg_parm);

    /*reset the thread payload*/
    memset(&thread_data->offline_cfg_parm, 0,
      sizeof(thread_data->offline_cfg_parm));
    thread_data->async_cmd_id = IFACE_ASYNC_COMMAND_INVALID;

    /*streamon, pass parameter from thread data*/
    thread_data->return_code = iface_offline_stream_config(offline_cfg_parm.iface,
      offline_cfg_parm.iface_sink_port,
      offline_cfg_parm.stream_id, session,
      offline_cfg_parm.offline_info);

    pthread_mutex_lock(&thread_data->busy_mutex);
    thread_data->thread_busy = FALSE;
    pthread_mutex_unlock(&thread_data->busy_mutex);

  }
    break;

  case IFACE_ASYNC_COMMAND_RESOURCE_ALLOC:
    rc = iface_session_thread_proc_resource_alloc(session, thread_data, thread_exit,
      &sem_posted);
  break;

  case IFACE_ASYNC_COMMAND_STREAMON: {
    CDBG("%s: SESSION CMD STREAMON\n", __func__);
    iface_session_cmd_stream_cfg_t stream_on_off_parm =
       *((iface_session_cmd_stream_cfg_t *)&thread_data->stream_on_off_parm);
    iface_hw_stream_t *hw_stream = NULL;
    boolean wait_for_sof = FALSE;

    iface_session_t *session = iface_util_get_session_by_id(iface, stream_on_off_parm.session_id);

    if (session == NULL) {
      CDBG_ERROR("%s session not found\n", __func__);
      rc = -1;
      goto end;
    }

    hw_stream =
      iface_util_find_hw_stream_by_mct_id(stream_on_off_parm.iface,
        session, stream_on_off_parm.stream_id);
    if (hw_stream == NULL) {
      CDBG_ERROR("%s: can not find hw stream on sess %d, stream %d\n",
      __func__, stream_on_off_parm.session_id, stream_on_off_parm.stream_id);
    } else {
      if ((stream_on_off_parm.is_first_streamon == FALSE) &&
          (hw_stream->state != IFACE_HW_STREAM_STATE_ACTIVE) &&
	  !session->lpm_enable) {
        wait_for_sof = TRUE;
      }
    }

    /*reset data after deep copy to local parm*/
    memset(&thread_data->stream_on_off_parm, 0,
      sizeof(thread_data->stream_on_off_parm));
    thread_data->async_cmd_id = IFACE_ASYNC_COMMAND_INVALID;

    /*only run time stream on need to align with SOF*/
    pthread_mutex_lock(&thread_data->sof_mutex);
    if(wait_for_sof == TRUE) {
      session->session_thread.wait_for_sof = TRUE;
    }
    pthread_mutex_unlock(&thread_data->sof_mutex);

    /*only run time stream on need to align with SOF*/
    if (wait_for_sof == TRUE) {
      memset(&timeout, 0, sizeof(timeout));
      SEM_WAIT_TIME(&thread_data->wait_sof_sem, &timeout, 300000000, rc);
      if (rc < 0) {
        CDBG_ERROR("%s: streamon sem_time timeout  rc = -1, errno= %d \
          (ETIMEOUT -110, INVALID -22)\n",
          __func__, errno);
      }
    }

    /*streamon, pass parameter from thread data*/
    MCT_PROF_LOG_BEG(PROF_IFACE_STREAM_ON);
    if (rc >= 0) {
      thread_data->return_code = iface_streamon(stream_on_off_parm.iface,
        stream_on_off_parm.iface_sink_port, stream_on_off_parm.session_id,
        stream_on_off_parm.stream_id, stream_on_off_parm.event);
    }
    MCT_PROF_LOG_END();

    pthread_mutex_lock(&thread_data->busy_mutex);
    thread_data->thread_busy = FALSE;
    pthread_mutex_unlock(&thread_data->busy_mutex);

    /* block until both axi done streamon*/
    sem_post(&thread_data->sig_sem);
    sem_posted = TRUE;

  }
    break;
 case IFACE_ASYNC_COMMAND_STREAMOFF: {
    CDBG("%s: SESSION CMD STREAMOFF\n", __func__);
    iface_session_cmd_stream_cfg_t stream_on_off_parm =
       *((iface_session_cmd_stream_cfg_t *)&thread_data->stream_on_off_parm);

    /*reset data after deep copy to local parm*/
    memset(&thread_data->stream_on_off_parm, 0,
      sizeof(thread_data->stream_on_off_parm));
    thread_data->async_cmd_id = IFACE_ASYNC_COMMAND_INVALID;

    /*streamon, pass parameter from thread data*/
    MCT_PROF_LOG_BEG(PROF_IFACE_STREAM_OFF);
    thread_data->return_code = iface_streamoff(stream_on_off_parm.iface,
      stream_on_off_parm.iface_sink_port, stream_on_off_parm.session_id,
      stream_on_off_parm.stream_id, stream_on_off_parm.event);
    MCT_PROF_LOG_END();

    pthread_mutex_lock(&thread_data->busy_mutex);
    thread_data->thread_busy = FALSE;
    pthread_mutex_unlock(&thread_data->busy_mutex);

    /* block until both axi done streamon*/
    sem_post(&thread_data->sig_sem);
    sem_posted = TRUE;

  }
      break;
  case IFACE_ASYNC_COMMAND_UV_SUBSAMPLE: {
    mct_bus_msg_iface_metadata_t iface_metadata;
    boolean                      ret = TRUE;
    /*reset data after deep copy to local parm*/
    thread_data->async_cmd_id = IFACE_ASYNC_COMMAND_INVALID;

    thread_data->uv_subsample_parm.enable =
      session->cds_info.need_cds_subsample;

    /*UV subsample non block caller thread*/
    sem_post(&thread_data->sig_sem);
    sem_posted = TRUE;

    /*get isp updating flag after ISP signal, update AXI STREAM
      if not getting isp updating flag means sof timeout, skip this CDS*/
    thread_data->return_code =
      iface_util_uv_subsample(session,
      thread_data->uv_subsample_parm.enable);

    /*after hw update AXI,
      need to wait few frames before kernel done update*/
    pthread_mutex_lock(&thread_data->busy_mutex);
      /*no stream active, drop CDS*/
    if (session->active_count == 0) {
      IFACE_HIGH("<cds_dbg> active acout = 0, CDS update immediately\n");
      thread_data->is_busy_wait = FALSE;
    } else {
       thread_data->is_busy_wait = TRUE;
    }
    pthread_mutex_unlock(&thread_data->busy_mutex);

    /* wait sof to count to unblock session thread,
      UV subsample need to block session thread for 2 more frame
      tunable UV_SS_WAIT_CNT */
    IFACE_HIGH("<cds_dbg> cds wait KERNEL......\n");
    if (thread_data->is_busy_wait) {
      memset(&timeout, 0, sizeof(timeout));
      SEM_WAIT_TIME(&thread_data->busy_sem, &timeout, 300000000, rc);
      if (rc < 0) {
          CDBG_ERROR("%s: CDS sem_time timeout with errno= %d \
            (ETIMEOUT -110, INVALID -22)\n",
           __func__, errno);
      }
    }
    IFACE_HIGH("<cds_dbg> cds wait KERNEL done!\n");

    /*only when we reset busy flag, we can get new cds request*/
    pthread_mutex_lock(&thread_data->busy_mutex);
    /*after wait for kernel, now CDS done update*/
    session->cds_curr_enb = thread_data->uv_subsample_parm.enable;
    session->cds_status = IFACE_CDS_STATUS_IDLE;
    thread_data->thread_busy = FALSE;
    pthread_mutex_unlock(&thread_data->busy_mutex);
  }
      break;

  default:
    CDBG("%s: invalid async cmd id = %d\n", __func__,
        thread_data->async_cmd_id);
    break;
  }


end:
  if (!sem_posted) {
    sem_post(&thread_data->sig_sem);
  }

  return rc;

}
/** iface_sem_thread_main
 *
 * DESCRIPTION: thread to proc cmd
 *
 **/
static void *iface_session_sem_thread_main(void *data)
{
  int rc = 0;
  int thread_exit = 0;
  iface_session_thread_t *thread_data = (iface_session_thread_t *)data;
  iface_session_t *iface_session = (iface_session_t *)thread_data->hw_ptr;

  /* wake up the creater first */
  sem_post(&thread_data->sig_sem);

  while(!thread_exit) {
    sem_wait(&thread_data->thread_wait_sem);
    rc = iface_session_thread_proc_cmd(iface_session, thread_data, &thread_exit);
    if (rc != 0) {
      CDBG_ERROR("iface_session_thread_proc_cmd failed !!!\n");
    }
  }

  return NULL;
}

/** iface_sem_thread_start
 *
 * DESCRIPTION:
 *
 **/
int iface_session_sem_thread_start(iface_session_thread_t *thread_data, void *hw_ptr)
{
    int rc = 0;
    thread_data->hw_ptr = hw_ptr;

    /*mutex*/
    pthread_mutex_init(&thread_data->cmd_mutex, NULL);
    pthread_mutex_init(&thread_data->busy_mutex, NULL);
    pthread_mutex_init(&thread_data->sof_mutex, NULL);
    /*semophore
      1. sig sem: to signal caller thread
      2. thread wait, to wait in main loop
      3. wait for sof: wake up when sof
      4. busy sem: to block thread till busy cnt = 0*/
    sem_init(&thread_data->sig_sem, 0, 0);
    sem_init(&thread_data->thread_wait_sem, 0, 0);
    sem_init(&thread_data->wait_sof_sem, 0, 0);
    sem_init(&thread_data->busy_sem, 0, 0);
    sem_init(&thread_data->ispif_start_sem, 0, 0);

    rc = pthread_create(&thread_data->pid, NULL,
      iface_session_sem_thread_main, (void *)thread_data);
    if(!rc) {
      pthread_setname_np(thread_data->pid, "CAM_iface_ses");
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
int iface_session_sem_thread_stop(iface_session_thread_t *thread_data)
{

  pthread_mutex_lock(&thread_data->cmd_mutex);
  //thread_data->cmd_id = IFACE_THREAD_CMD_DESTROY;
  sem_post(&thread_data->thread_wait_sem);
  pthread_join(thread_data->pid, NULL);
  pthread_mutex_unlock(&thread_data->cmd_mutex);
  sem_destroy(&thread_data->sig_sem);
  sem_destroy(&thread_data->busy_sem);
  sem_destroy(&thread_data->thread_wait_sem);
  sem_destroy(&thread_data->wait_sof_sem);
  sem_destroy(&thread_data->ispif_start_sem);
  pthread_mutex_destroy(&thread_data->cmd_mutex);
  pthread_mutex_destroy(&thread_data->busy_mutex);
  return 0;
}
