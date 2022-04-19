/**************************************************************************
* Copyright (c) 2012-2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
***************************************************************************/

#include <sys/ioctl.h>
#include <stdint.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include <pthread.h>
#include <math.h>
#include <linux/msm_ion.h>
#include "jpegdma_lib.h"
#include "jpegdma_lib_hw.h"
#include "jpeg_lib_hw_reg.h"

#include "jpeg_hw_dbg.h"
#ifdef _ANDROID_
  #include <cutils/properties.h>
#endif
#define HW_INPUT_SIZE 607;
#define MSM_JPEGDMA_NAME "/dev/jpeg3"

#define JPEGDMA_MAX_NUMBER_PIPES (2)
#define JPEG_CLK_RATE_NORMAL (266670000)
#define JPEG_CLK_RATE_HIGH (460000000)
#define JPEGDMA_SCALE_UNITY (1U << 21)
#define JPEGDMA_DIV_Q521(a,b) (((a)*JPEGDMA_SCALE_UNITY)/(b))
#define JPEGDMA_RATIO(a) ((a) << 21)

#define JPEGDMA_Q521_TO_INT(a) (a >> 21)

volatile uint32_t g_jpeghwloglevel;

static const char* jpeg_devs[] = {MSM_JPEGDMA_NAME, NULL};

typedef struct
{
  int jpegefd;
  int (*jpegdma_hw_lib_event_handler) (void *,
    struct jpegdma_hw_evt *, int);
  int (*jpegdma_hw_lib_input_handler) (void *, struct jpegdma_hw_buf *);
  int (*jpegdma_hw_lib_output_handler) (void *, struct jpegdma_hw_buf *);

  pthread_t event_thread_id;
  unsigned char event_thread_exit;
  pthread_mutex_t event_thread_ready_mutex;
  pthread_cond_t event_thread_ready_cond;
  uint8_t event_thread_is_ready;

  pthread_t input_thread_id;
  unsigned char input_thread_exit;
  pthread_mutex_t input_thread_ready_mutex;
  pthread_cond_t input_thread_ready_cond;
  uint8_t input_thread_is_ready;

  pthread_t output_thread_id;
  unsigned char output_thread_exit;
  pthread_mutex_t output_thread_ready_mutex;
  pthread_cond_t output_thread_ready_cond;
  uint8_t output_thread_is_ready;

  void *p_userdata;
  uint32_t version;
  jpegdma_fe_input_cfg fe_input_cfg;
  jpegdma_we_output_cfg we_output_cfg;
  uint8_t scale_enable;
  uint32_t num_pipes;
} __jpegdma_hw_obj_t;


void jpegdma_hw_lib_wait_thread_ready (__jpegdma_hw_obj_t * jpegdma_hw_obj_p,
  pthread_t* thread_id)
{
  JPEG_HW_DBG("%s:%d], thread_id %d", __func__, __LINE__, (int) *thread_id);
  if (*thread_id == jpegdma_hw_obj_p->event_thread_id) {
    pthread_mutex_lock (&jpegdma_hw_obj_p->event_thread_ready_mutex);
    JPEG_HW_DBG("%s:%d], event thread ready %d", __func__, __LINE__,
      jpegdma_hw_obj_p->event_thread_is_ready);
    if (!jpegdma_hw_obj_p->event_thread_is_ready){
      pthread_cond_wait (&jpegdma_hw_obj_p->event_thread_ready_cond,
       &jpegdma_hw_obj_p->event_thread_ready_mutex);
    }
    jpegdma_hw_obj_p->event_thread_is_ready = 0;
    pthread_mutex_unlock (&jpegdma_hw_obj_p->event_thread_ready_mutex);
  } else if (*thread_id == jpegdma_hw_obj_p->input_thread_id) {
    pthread_mutex_lock (&jpegdma_hw_obj_p->input_thread_ready_mutex);
    JPEG_HW_DBG("%s:%d], ready %d", __func__, __LINE__,
      jpegdma_hw_obj_p->input_thread_is_ready);
    if (!jpegdma_hw_obj_p->input_thread_is_ready)
      pthread_cond_wait (&jpegdma_hw_obj_p->input_thread_ready_cond,
        &jpegdma_hw_obj_p->input_thread_ready_mutex);
    jpegdma_hw_obj_p->input_thread_is_ready = 0;
    pthread_mutex_unlock (&jpegdma_hw_obj_p->input_thread_ready_mutex);
  } else if (*thread_id == jpegdma_hw_obj_p->output_thread_id) {
    pthread_mutex_lock (&jpegdma_hw_obj_p->output_thread_ready_mutex);
    JPEG_HW_DBG("%s:%d], ready %d", __func__, __LINE__,
      jpegdma_hw_obj_p->output_thread_is_ready);
    if (!jpegdma_hw_obj_p->output_thread_is_ready)
      pthread_cond_wait (&jpegdma_hw_obj_p->output_thread_ready_cond,
        &jpegdma_hw_obj_p->output_thread_ready_mutex);
    jpegdma_hw_obj_p->output_thread_is_ready = 0;
    pthread_mutex_unlock (&jpegdma_hw_obj_p->output_thread_ready_mutex);
  }
  JPEG_HW_DBG("%s:%d] thread_id %d done", __func__, __LINE__,
    (int) *thread_id);
}

void jpegdma_hw_lib_send_thread_ready (__jpegdma_hw_obj_t * jpegdma_hw_obj_p,
  pthread_t* thread_id)
{
  JPEG_HW_DBG("%s:%d], thread_id %d", __func__, __LINE__, (int) *thread_id);
  if (*thread_id == jpegdma_hw_obj_p->event_thread_id) {
    pthread_mutex_lock (&jpegdma_hw_obj_p->event_thread_ready_mutex);
    jpegdma_hw_obj_p->event_thread_is_ready = 1;
    pthread_cond_signal (&jpegdma_hw_obj_p->event_thread_ready_cond);
    pthread_mutex_unlock (&jpegdma_hw_obj_p->event_thread_ready_mutex);
  } else if (*thread_id == jpegdma_hw_obj_p->input_thread_id) {
    pthread_mutex_lock (&jpegdma_hw_obj_p->input_thread_ready_mutex);
    jpegdma_hw_obj_p->input_thread_is_ready = 1;
    pthread_cond_signal (&jpegdma_hw_obj_p->input_thread_ready_cond);
    pthread_mutex_unlock (&jpegdma_hw_obj_p->input_thread_ready_mutex);
  } else if (*thread_id == jpegdma_hw_obj_p->output_thread_id) {
    pthread_mutex_lock (&jpegdma_hw_obj_p->output_thread_ready_mutex);
    jpegdma_hw_obj_p->output_thread_is_ready = 1;
    pthread_cond_signal (&jpegdma_hw_obj_p->output_thread_ready_cond);
    pthread_mutex_unlock (&jpegdma_hw_obj_p->output_thread_ready_mutex);
  }
  JPEG_HW_DBG("%s:%d], thread_id %d done", __func__, __LINE__,
    (int) *thread_id);
}

int jpegdma_lib_get_event(jpegdma_hw_obj_t jpegdma_hw_obj,
  struct jpegdma_hw_evt *p_event)
{
  __jpegdma_hw_obj_t *jpegdma_hw_obj_p = (__jpegdma_hw_obj_t *)jpegdma_hw_obj;
  int jpegefd = jpegdma_hw_obj_p->jpegefd;
  struct msm_jpeg_ctrl_cmd jpegeCtrlCmd;
  int result = 0;

  result = ioctl (jpegefd, MSM_JPEG_IOCTL_EVT_GET,
    &jpegeCtrlCmd);
  JPEG_HW_DBG("%s:%d] MSM_JPEG_IOCTL_EVT_GET rc = %d",
    __func__, __LINE__, result);
  if (result) {
    JPEG_HW_DBG("%s:%d] cannot get event", __func__, __LINE__);
  } else {
    p_event->type = jpegeCtrlCmd.type;
    p_event->len  = jpegeCtrlCmd.len;
    p_event->value = jpegeCtrlCmd.value;
  }
  return 0;
}

int jpegdma_lib_get_input(jpegdma_hw_obj_t jpegdma_hw_obj,
    struct jpegdma_hw_buf *p_buf)
{
  __jpegdma_hw_obj_t *jpegdma_hw_obj_p = (__jpegdma_hw_obj_t *)jpegdma_hw_obj;
  int jpegefd = jpegdma_hw_obj_p->jpegefd;

  int result = 0;
  struct msm_jpeg_buf msm_buf;

  result = ioctl (jpegefd, MSM_JPEG_IOCTL_INPUT_GET, &msm_buf);
  JPEG_HW_DBG("%s:%d] MSM_JPEG_IOCTL_INPUT_GET rc = %d", __func__,
    __LINE__, result);
  if (result) {
    JPEG_HW_DBG("%s:%d] cannot get input", __func__, __LINE__);
  } else {
    p_buf->type = msm_buf.type;
    p_buf->fd   = msm_buf.fd;
    p_buf->vaddr = msm_buf.vaddr;
    p_buf->y_off         = msm_buf.y_off;
    p_buf->y_len         = msm_buf.y_len;
    p_buf->framedone_len = msm_buf.framedone_len;
    p_buf->cbcr_off = msm_buf.cbcr_off;
    p_buf->cbcr_len = msm_buf.cbcr_len;
    p_buf->num_of_mcu_rows = msm_buf.num_of_mcu_rows;
  }
  return 0;
}

int jpegdma_lib_get_output(jpegdma_hw_obj_t jpegdma_hw_obj,
    struct jpegdma_hw_buf *p_buf)
{
  __jpegdma_hw_obj_t *jpegdma_hw_obj_p = (__jpegdma_hw_obj_t *)jpegdma_hw_obj;
  int jpegefd = jpegdma_hw_obj_p->jpegefd;

  int result = 0;
  struct msm_jpeg_buf msm_buf;

  result = ioctl (jpegefd, MSM_JPEG_IOCTL_OUTPUT_GET, &msm_buf);
  JPEG_HW_DBG("%s:%d] MSM_JPEG_IOCTL_OUTPUT_GET rc = %d", __func__,
    __LINE__, result);
  if (result) {
    JPEG_HW_DBG("%s:%d] cannot get output", __func__, __LINE__);
  } else {
    p_buf->type = msm_buf.type;
    p_buf->fd   = msm_buf.fd;
    p_buf->vaddr = msm_buf.vaddr;
    p_buf->y_off         = msm_buf.y_off;
    p_buf->y_len         = msm_buf.y_len;
    p_buf->framedone_len = msm_buf.framedone_len;
    p_buf->cbcr_off = msm_buf.cbcr_off;
    p_buf->cbcr_len = msm_buf.cbcr_len;
    p_buf->num_of_mcu_rows = msm_buf.num_of_mcu_rows;
  }
  return 0;
}

void *jpegdma_hw_lib_event_thread (void *context)
{
  __jpegdma_hw_obj_t *jpegdma_hw_obj_p = (__jpegdma_hw_obj_t *) context;
  int jpegefd = jpegdma_hw_obj_p->jpegefd;

  struct msm_jpeg_ctrl_cmd jpegeCtrlCmd;
  struct jpegdma_hw_evt gmnEvt;
  int result = 0;

  JPEG_HW_DBG("%s:%d] Enter threadid %ld", __func__, __LINE__,
    jpegdma_hw_obj_p->event_thread_id);
  jpegdma_hw_lib_send_thread_ready (jpegdma_hw_obj_p,
      &(jpegdma_hw_obj_p->event_thread_id));

  do {
    result = ioctl (jpegefd, MSM_JPEG_IOCTL_EVT_GET,
      &jpegeCtrlCmd);
    JPEG_HW_DBG("%s:%d] MSM_JPEG_IOCTL_EVT_GET rc = %d",
      __func__, __LINE__, result);
    if (result) {
      if (!jpegdma_hw_obj_p->event_thread_exit) {
        JPEG_HW_DBG("%s:%d] fail", __func__, __LINE__);
      }
    } else {
      gmnEvt.type = jpegeCtrlCmd.type;
      gmnEvt.len  = jpegeCtrlCmd.len;
      gmnEvt.value = jpegeCtrlCmd.value;
      jpegdma_hw_obj_p->jpegdma_hw_lib_event_handler(
        jpegdma_hw_obj_p->p_userdata,
        &gmnEvt,
        (int)gmnEvt.type);
    }
    jpegdma_hw_lib_send_thread_ready (jpegdma_hw_obj_p,
      &(jpegdma_hw_obj_p->event_thread_id));
  } while (!jpegdma_hw_obj_p->event_thread_exit);

  JPEG_HW_DBG("%s:%d] Exit", __func__, __LINE__);
  return NULL;
}

void *jpegdma_hw_lib_output_thread (void *context)
{
  __jpegdma_hw_obj_t *jpegdma_hw_obj_p = (__jpegdma_hw_obj_t *) context;
  int jpegefd = jpegdma_hw_obj_p->jpegefd;

  int result = 0;
  struct msm_jpeg_buf msm_buf;
  struct jpegdma_hw_buf buf;

  JPEG_HW_DBG("%s:%d] Enter threadid %ld", __func__, __LINE__,
    jpegdma_hw_obj_p->output_thread_id);
  jpegdma_hw_lib_send_thread_ready (jpegdma_hw_obj_p,
      &(jpegdma_hw_obj_p->output_thread_id));

  do {
    result = ioctl (jpegefd, MSM_JPEG_IOCTL_OUTPUT_GET, &msm_buf);
    JPEG_HW_DBG("%s:%d] MSM_JPEG_IOCTL_OUTPUT_GET rc = %d", __func__,
      __LINE__, result);
    if (result) {
      if (!jpegdma_hw_obj_p->output_thread_exit) {
        JPEG_HW_DBG("%s:%d] fail", __func__, __LINE__);
      }
    } else {
      buf.type = msm_buf.type;
      buf.fd   = msm_buf.fd;

      buf.vaddr = msm_buf.vaddr;

      buf.y_off         = msm_buf.y_off;
      buf.y_len         = msm_buf.y_len;
      buf.framedone_len = msm_buf.framedone_len;

      buf.cbcr_off = msm_buf.cbcr_off;
      buf.cbcr_len = msm_buf.cbcr_len;

      buf.num_of_mcu_rows = msm_buf.num_of_mcu_rows;
      JPEG_HW_DBG("%s:%d] framedone_len %d", __func__, __LINE__,
        buf.framedone_len);
      jpegdma_hw_obj_p->jpegdma_hw_lib_output_handler(
        jpegdma_hw_obj_p->p_userdata,
        &buf);
    }
    jpegdma_hw_lib_send_thread_ready (jpegdma_hw_obj_p,
      &(jpegdma_hw_obj_p->output_thread_id));
  } while (!jpegdma_hw_obj_p->output_thread_exit);

  JPEG_HW_DBG("%s:%d] Exit", __func__, __LINE__);
  return NULL;
}

void *jpegdma_lib_input_thread (void *context)
{
  __jpegdma_hw_obj_t *jpegdma_hw_obj_p = (__jpegdma_hw_obj_t *) context;
  int jpegefd = jpegdma_hw_obj_p->jpegefd;

  int result = 0;
  struct msm_jpeg_buf msm_buf;
  struct jpegdma_hw_buf buf;

  JPEG_HW_DBG("%s:%d] Enter threadid %ld", __func__, __LINE__,
    jpegdma_hw_obj_p->input_thread_id);
  jpegdma_hw_lib_send_thread_ready (jpegdma_hw_obj_p,
      &(jpegdma_hw_obj_p->input_thread_id));

  do {
    result = ioctl (jpegefd, MSM_JPEG_IOCTL_INPUT_GET, &msm_buf);
    JPEG_HW_DBG("%s:%d] MSM_JPEG_IOCTL_INPUT_GET rc = %d", __func__,
      __LINE__, result);
    if (result) {
      if (!jpegdma_hw_obj_p->input_thread_exit) {
        JPEG_HW_DBG("%s:%d] fail", __func__, __LINE__);
      }
    } else {
      buf.type = msm_buf.type;
      buf.fd   = msm_buf.fd;

      buf.vaddr = msm_buf.vaddr;

      buf.y_off         = msm_buf.y_off;
      buf.y_len         = msm_buf.y_len;
      buf.framedone_len = msm_buf.framedone_len;

      buf.cbcr_off = msm_buf.cbcr_off;
      buf.cbcr_len = msm_buf.cbcr_len;

      buf.num_of_mcu_rows = msm_buf.num_of_mcu_rows;

      jpegdma_hw_obj_p->jpegdma_hw_lib_input_handler (
        jpegdma_hw_obj_p->p_userdata,
        &buf);
    }
    jpegdma_hw_lib_send_thread_ready (jpegdma_hw_obj_p,
      &(jpegdma_hw_obj_p->input_thread_id));
  } while (!jpegdma_hw_obj_p->input_thread_exit);

  JPEG_HW_DBG("%s:%d] Exit", __func__, __LINE__);
  return NULL;
}

int jpegdma_lib_init (jpegdma_hw_obj_t *jpegdma_hw_obj,
  void *p_userdata,
  int (*event_handler) (jpegdma_hw_obj_t,
  struct jpegdma_hw_evt *,
  int event),
  int (*output_handler) (jpegdma_hw_obj_t, struct jpegdma_hw_buf *),
  int (*input_handler) (jpegdma_hw_obj_t, struct jpegdma_hw_buf *))
{

#ifdef _ANDROID_
  char prop[PROPERTY_VALUE_MAX];
  property_get("persist.vendor.camera.mmstill.logs", prop, "0");
  g_jpeghwloglevel = (uint32_t)atoi(prop);
  JPEG_HW_DBG("%s:%d ###jpeghw_Loglevel %d",
    __func__, __LINE__, g_jpeghwloglevel);
#endif

  __jpegdma_hw_obj_t *jpegdma_hw_obj_p;
  int jpegefd = -1;

  int result;
  struct msm_jpeg_ctrl_cmd jpegeCtrlCmd;
  const char **p;

  jpegdma_hw_obj_p = malloc (sizeof (__jpegdma_hw_obj_t));
  if (!jpegdma_hw_obj_p) {
    JPEG_HW_PR_ERR ("%s:%d] no mem", __func__, __LINE__);
    return -1;
  }
  memset (jpegdma_hw_obj_p, 0, sizeof (__jpegdma_hw_obj_t));

  for (p = jpeg_devs; *p; p++ ) {
    jpegefd = open (*p, O_RDWR);
    JPEG_HW_DBG("open %s: fd = %d", MSM_JPEGDMA_NAME, jpegefd);
    if (jpegefd < 0) {
      JPEG_HW_PR_ERR ("Cannot open %s", MSM_JPEGDMA_NAME);
    } else {
      break;
    }
  }

  if (!*p) {
    goto jpegdma_init_err;
  }

  jpegdma_hw_obj_p->jpegdma_hw_lib_event_handler = event_handler;
  jpegdma_hw_obj_p->jpegdma_hw_lib_input_handler = input_handler;
  jpegdma_hw_obj_p->jpegdma_hw_lib_output_handler = output_handler;
  jpegdma_hw_obj_p->jpegefd = jpegefd;
  jpegdma_hw_obj_p->p_userdata = p_userdata;

  pthread_mutex_init (&jpegdma_hw_obj_p->event_thread_ready_mutex, NULL);
  pthread_cond_init (&jpegdma_hw_obj_p->event_thread_ready_cond, NULL);
  jpegdma_hw_obj_p->event_thread_is_ready = 0;

  pthread_mutex_init (&jpegdma_hw_obj_p->input_thread_ready_mutex, NULL);
  pthread_cond_init (&jpegdma_hw_obj_p->input_thread_ready_cond, NULL);
  jpegdma_hw_obj_p->input_thread_is_ready = 0;

  pthread_mutex_init (&jpegdma_hw_obj_p->output_thread_ready_mutex, NULL);
  pthread_cond_init (&jpegdma_hw_obj_p->output_thread_ready_cond, NULL);
  jpegdma_hw_obj_p->output_thread_is_ready = 0;

  JPEG_HW_DBG("%s:%d] handler %p %p %p", __func__, __LINE__,
    event_handler, input_handler, output_handler);
  if (event_handler) {
    pthread_mutex_lock(&jpegdma_hw_obj_p->event_thread_ready_mutex);
      result = pthread_create (&jpegdma_hw_obj_p->event_thread_id, NULL,
        jpegdma_hw_lib_event_thread, jpegdma_hw_obj_p);
    if (result < 0) {
      JPEG_HW_PR_ERR ("%s event thread creation failed", __func__);
      pthread_mutex_unlock(&jpegdma_hw_obj_p->event_thread_ready_mutex);
      goto jpegdma_init_err;
    }
    pthread_mutex_unlock(&jpegdma_hw_obj_p->event_thread_ready_mutex);
  }
  if (input_handler) {
    pthread_mutex_lock(&jpegdma_hw_obj_p->input_thread_ready_mutex);
    result = pthread_create (&jpegdma_hw_obj_p->input_thread_id, NULL,
      jpegdma_lib_input_thread, jpegdma_hw_obj_p);
    if (result < 0) {
      JPEG_HW_PR_ERR ("%s input thread creation failed", __func__);
      pthread_mutex_unlock(&jpegdma_hw_obj_p->input_thread_ready_mutex);
      goto jpegdma_init_err;
    }
    pthread_mutex_unlock(&jpegdma_hw_obj_p->input_thread_ready_mutex);
  }
  if (output_handler) {
    pthread_mutex_lock(&jpegdma_hw_obj_p->output_thread_ready_mutex);
    result = pthread_create (&jpegdma_hw_obj_p->output_thread_id, NULL,
    jpegdma_hw_lib_output_thread, jpegdma_hw_obj_p);
    if (result < 0) {
      JPEG_HW_PR_ERR ("%s output thread creation failed", __func__);
      pthread_mutex_unlock(&jpegdma_hw_obj_p->output_thread_ready_mutex);
      goto jpegdma_init_err;
    }
    pthread_mutex_unlock(&jpegdma_hw_obj_p->output_thread_ready_mutex);
  }

  if (event_handler || output_handler || input_handler) {
    JPEG_HW_DBG("jpeg create all threads success");
    jpegdma_lib_wait_done(jpegdma_hw_obj_p);
    JPEG_HW_DBG("jpeg after starting all threads");
  } else {
    JPEG_HW_DBG("%s:%d] Successful", __func__, __LINE__);
  }
  *jpegdma_hw_obj = jpegdma_hw_obj_p;
  return jpegefd;
jpegdma_init_err:
  if (jpegdma_hw_obj_p) {
    free (jpegdma_hw_obj_p);
  }
  return -1;
}

int jpegdma_lib_release(jpegdma_hw_obj_t jpegdma_hw_obj)
{
  int result;
  if (!jpegdma_hw_obj) {
    JPEG_HW_PR_ERR("%s:%d]jpege object is NULL in release",
      __func__, __LINE__);
    return -EINVALID;
  }
  __jpegdma_hw_obj_t *jpegdma_hw_obj_p = (__jpegdma_hw_obj_t *) jpegdma_hw_obj;
  int jpegefd = jpegdma_hw_obj_p->jpegefd;

  jpegdma_hw_obj_p->event_thread_exit = 1;
  jpegdma_hw_obj_p->input_thread_exit = 1;
  jpegdma_hw_obj_p->output_thread_exit = 1;

  if (jpegdma_hw_obj_p->jpegdma_hw_lib_event_handler) {
    ioctl (jpegefd, MSM_JPEG_IOCTL_EVT_GET_UNBLOCK);
    JPEG_HW_DBG("%s:%d] pthread_join: event_thread", __func__,
      __LINE__);
    if (pthread_join (jpegdma_hw_obj_p->event_thread_id, NULL) != 0) {
      JPEG_HW_DBG("%s: failed %d", __func__, __LINE__);
    }
  }

  if (jpegdma_hw_obj_p->jpegdma_hw_lib_input_handler) {
    ioctl (jpegefd, MSM_JPEG_IOCTL_INPUT_GET_UNBLOCK);
    JPEG_HW_DBG("%s:%d] pthread_join: input_thread", __func__,
      __LINE__);

    if (pthread_join (jpegdma_hw_obj_p->input_thread_id, NULL) != 0) {
      JPEG_HW_DBG("%s: failed %d", __func__, __LINE__);
    }

  }

  if (jpegdma_hw_obj_p->jpegdma_hw_lib_output_handler) {
    ioctl (jpegefd, MSM_JPEG_IOCTL_OUTPUT_GET_UNBLOCK);
    JPEG_HW_DBG("%s:%d] pthread_join: output_thread", __func__,
      __LINE__);
    if (pthread_join (jpegdma_hw_obj_p->output_thread_id, NULL) != 0) {
      JPEG_HW_DBG("%s: failed %d", __func__, __LINE__);
    }
  }

  result = close (jpegefd);

  pthread_mutex_destroy (&jpegdma_hw_obj_p->event_thread_ready_mutex);
  pthread_cond_destroy (&jpegdma_hw_obj_p->event_thread_ready_cond);

  pthread_mutex_destroy (&jpegdma_hw_obj_p->input_thread_ready_mutex);
  pthread_cond_destroy (&jpegdma_hw_obj_p->input_thread_ready_cond);

  pthread_mutex_destroy (&jpegdma_hw_obj_p->output_thread_ready_mutex);
  pthread_cond_destroy (&jpegdma_hw_obj_p->output_thread_ready_cond);

  JPEG_HW_DBG("%s:%d] closed %s", __func__, __LINE__, MSM_JPEGDMA_NAME);

  if (jpegdma_hw_obj) {
    free(jpegdma_hw_obj);
  }
  return result;
}

typedef struct {
  float range_end;
  uint32_t block_width;
  uint32_t block_width_sel;
} jpegdma_blk_cfg;

static const jpegdma_blk_cfg jpegdma_step_list[] = {
    {1.875,  256, 4},
    {3.875,  128, 3},
    {7.875,  64,  2},
    {15.875, 32,  1},
    {32.0, 16,  0},
};

static const jpegdma_blk_cfg *jpegdma_find_blk(float ratio)
{
  const jpegdma_blk_cfg *p_blk = jpegdma_step_list;

  for (; ratio >= p_blk->range_end; p_blk++)
    ;
  return p_blk;
}

int jpegdma_lib_hw_config (jpegdma_hw_obj_t jpegdma_hw_obj,
  jpegdma_cmd_input_cfg *p_input_cfg,
  jpegdma_cmd_jpeg_transfer_cfg *p_transfer_cfg,
  jpegdma_cmd_scale_cfg *p_scale_cfg)
{
  int result, i=0,j=0;
  struct msm_jpeg_ctrl_cmd jpegeCtrlCmd;
  struct msm_jpeg_hw_cmd hw_cmd;
  struct msm_jpeg_hw_cmd input_cmd;
  struct msm_jpeg_hw_cmds *p_hw_cmds = NULL;
  unsigned int jpeg_clk;
  jpegdma_fe_input_cfg *fe_input_cfg;
  jpegdma_we_output_cfg *we_output_cfg;
  jpegdma_we_cfg we_cfg;
  jpegdma_scale_cfg pp_scale_cfg;
  float we_height, fe_height;
  uint32_t v_phase_1;
  uint32_t data;
  uint32_t image_width, image_height;
  uint32_t num_pipes;

  __jpegdma_hw_obj_t *jpegdma_hw_obj_p = (__jpegdma_hw_obj_t *) jpegdma_hw_obj;
  int jpegefd = jpegdma_hw_obj_p->jpegefd;
  const jpegdma_blk_cfg *p_blk_cfg;
  float scale_ratio_w, scale_ratio_h;
  fe_input_cfg = &jpegdma_hw_obj_p->fe_input_cfg;
  we_output_cfg = &jpegdma_hw_obj_p->we_output_cfg;

  JPEG_HW_DBG("input w:%d h:%d str:%d ", p_input_cfg->image_width,
    p_input_cfg->image_height, p_input_cfg->stride);

  JPEG_HW_DBG("output scale:%d w:%d h:%d", p_scale_cfg->scale_enable,
    p_scale_cfg->output_width, p_scale_cfg->output_height);


  jpegeCtrlCmd.type = MSM_JPEG_MODE_OFFLINE_ENCODE;
  result = ioctl (jpegefd, MSM_JPEG_IOCTL_RESET, &jpegeCtrlCmd);
  JPEG_HW_DBG("ioctl MSM_JPEG_IOCTL_RESET: rc = %d", result);
  if (result) {
    JPEG_HW_PR_ERR("%s:%d] MSM_JPEG_IOCTL_RESET: failed rc = %d",
      __func__, __LINE__, result);
    goto fail;
  }

  jpegdma_lib_hw_get_version (&hw_cmd);
  result = ioctl (jpegefd, MSM_JPEG_IOCTL_GET_HW_VERSION, &hw_cmd);
  JPEG_HW_DBG("%s:%d] result %d", __func__, __LINE__, result);
  if (result) {
    JPEG_HW_PR_ERR("%s:%d] MSM_JPEG_IOCTL_GET_HW_VERSION:"
      " failed rc = %d", __func__, __LINE__, result);
    goto fail;
  }
  jpegdma_hw_obj_p->version = hw_cmd.data;
  JPEG_HW_DBG("%s:%d] Version %x", __func__, __LINE__,
    jpegdma_hw_obj_p->version);

  jpegdma_lib_hw_get_num_pipes(&hw_cmd);
  result = ioctl (jpegefd, MSM_JPEG_IOCTL_HW_CMD, &hw_cmd);
  JPEG_HW_DBG("%s:%d] result %d", __func__, __LINE__, result);
  if (result) {
    JPEG_HW_PR_ERR("%s:%d] Get num pipes HW command"
      " failed rc = %d", __func__, __LINE__, result);
    goto fail;
  }

  num_pipes = hw_cmd.data >> JPEGDMA_HW_CAPABILITY__NUM_PROCESSING_PIPES_SHFT;
  JPEG_HW_DBG("%s:%d] Processing pipes number %d",
    __func__, __LINE__, num_pipes);
  if ((num_pipes < 1) || (num_pipes > JPEGDMA_MAX_NUMBER_PIPES)) {
    JPEG_HW_PR_ERR("%s:%d] Error Number of pipes %d not supported max is %d",
      __func__, __LINE__, num_pipes, JPEGDMA_MAX_NUMBER_PIPES);
    result = -1;
    goto fail;
  }
  jpegdma_hw_obj_p->num_pipes = num_pipes;

  p_hw_cmds = jpegdma_cmd_core_cfg(p_scale_cfg, num_pipes);
  if (!p_hw_cmds)
    goto fail;
  result = ioctl (jpegefd, MSM_JPEG_IOCTL_HW_CMDS, p_hw_cmds);
  free (p_hw_cmds);
  JPEG_HW_DBG("ioctl jpegdma_cmd_core_cfg: rc = %d", result);
  if (result) {
    JPEG_HW_PR_ERR("%s:%d] Core configuration  failed rc = %d",
    __func__, __LINE__, result);
    goto fail;
  }

  /* Set input input order only for cbcr plane type */
  if (p_input_cfg->plane_type == JPEGDMA_PLANE_TYPE_CBCR) {
    fe_input_cfg->input_cbcr_order =
      (jpegdma_hw_obj_p->num_pipes >= 2) ?
      p_input_cfg->input_cbcr_order : 0;
  } else {
    fe_input_cfg->input_cbcr_order = 0;
  }

  if (p_scale_cfg->scale_enable || p_scale_cfg->crop_enable) {
    image_width = p_scale_cfg->scale_input_width;
    image_height = p_scale_cfg->scale_input_height;
  } else {
    image_width = p_input_cfg->image_width;
    image_height = p_input_cfg->image_height;
  }

  fe_input_cfg->image_width =      image_width;
  fe_input_cfg->plane_type =       p_input_cfg->plane_type;
  fe_input_cfg->stride =           p_input_cfg->stride;
  fe_input_cfg->scanline =         p_input_cfg->scanline;
  fe_input_cfg->hw_buf_size =      p_input_cfg->hw_buf_size;

  pp_scale_cfg.scale_enable = p_scale_cfg->scale_enable;
  jpegdma_hw_obj_p->scale_enable = pp_scale_cfg.scale_enable;
  scale_ratio_h = (float) image_height / p_scale_cfg->output_height;
  scale_ratio_w = (float) image_width / p_scale_cfg->output_width;
  pp_scale_cfg.phase_h_step_0 = (uint32_t)(scale_ratio_w * JPEGDMA_SCALE_UNITY);
  pp_scale_cfg.phase_v_step_0 = (uint32_t)(scale_ratio_h * JPEGDMA_SCALE_UNITY);

  we_height = (float)p_scale_cfg->output_height / (float)num_pipes;

  fe_height = we_height * scale_ratio_h;
  p_blk_cfg = jpegdma_find_blk(scale_ratio_w);

  JPEG_HW_DBG("block: width %d sel %d ", p_blk_cfg->block_width,
    p_blk_cfg->block_width_sel);

  fe_input_cfg->block_width = p_blk_cfg->block_width_sel;
  we_output_cfg->output_height_0 = (uint32_t) ceil(we_height);

  fe_input_cfg->image_height_0 = (uint32_t) fe_height;

  v_phase_1 = ((uint32_t)((fe_height - floor(fe_height)) *
    (float)JPEGDMA_SCALE_UNITY)) & 0x1FFFFF;

  fe_input_cfg->h_phase_0_frac = 0;
  fe_input_cfg->h_phase_0_int = 0;
  fe_input_cfg->v_phase_0_int = 0;
  fe_input_cfg->v_phase_0_frac = v_phase_1;

  we_output_cfg->blocks_per_row_0 =  (uint16_t)
    ceil(((double)p_scale_cfg->output_width / p_blk_cfg->block_width));

  we_output_cfg->blocks_per_col_0 = (uint16_t)we_output_cfg->output_height_0;

  we_output_cfg->h_step = p_blk_cfg->block_width;

  we_output_cfg->last_h_step = p_scale_cfg->output_width %
      p_blk_cfg->block_width;

  if (!we_output_cfg->last_h_step) {
    we_output_cfg->last_h_step = we_output_cfg->h_step;
  }

  we_output_cfg->last_v_step_0 = 1;

  //Configure second pipeline
  if (num_pipes > 1) {
    pp_scale_cfg.phase_h_step_1 = pp_scale_cfg.phase_h_step_0;
    pp_scale_cfg.phase_v_step_1 = pp_scale_cfg.phase_v_step_0;

    fe_input_cfg->image_height_1 = image_height - fe_input_cfg->image_height_0;

    we_output_cfg->output_height_1 = p_scale_cfg->output_height -
        we_output_cfg->output_height_0;

    fe_input_cfg->h_phase_1_frac = 0;
    fe_input_cfg->h_phase_1_int = 0;
    fe_input_cfg->v_phase_1_int = 0;
    fe_input_cfg->v_phase_1_frac = v_phase_1;

    we_output_cfg->blocks_per_row_1 = we_output_cfg->blocks_per_row_0;
    we_output_cfg->blocks_per_col_1 = (uint16_t)we_output_cfg->output_height_1;
    we_output_cfg->last_v_step_1 = 1;
  }

  JPEG_HW_DBG("fe_cfg: w:%d h0:%d h1:%d str:%d scan:%d blk_w_sel:%d v_phase:%d"
    " plntype:%d cbcr:%d",
    fe_input_cfg->image_width,
    fe_input_cfg->image_height_0,
    fe_input_cfg->image_height_1,
    fe_input_cfg->stride,
    fe_input_cfg->scanline,
    fe_input_cfg->block_width,
    fe_input_cfg->v_phase_1_frac,
    fe_input_cfg->plane_type,
    fe_input_cfg->input_cbcr_order);

  //Configure Fetch Engine
  p_hw_cmds = jpegdma_lib_hw_fe_cfg(fe_input_cfg, 0, num_pipes);
  if (!p_hw_cmds) {
    JPEG_HW_PR_ERR("%s:%d] FE Configuration alloc failed rc = %d",
      __func__, __LINE__, result);
    goto fail;
  }
  result = ioctl (jpegefd, MSM_JPEG_IOCTL_HW_CMDS, p_hw_cmds);
  free (p_hw_cmds);
  JPEG_HW_DBG("ioctl jpegdma_lib_hw_fe_cfg: rc = %d", result);
  if (result) {
    JPEG_HW_PR_ERR("%s:%d] FE configuration failed rc = %d",
     __func__, __LINE__, result);
    goto fail;
  }

  we_output_cfg->output_width = p_scale_cfg->output_width;
  we_output_cfg->stride = p_scale_cfg->output_stride;
  we_output_cfg->scanline = p_scale_cfg->output_scanline;

  //Configure Fetch Engine Buffers
  p_hw_cmds = jpegdma_lib_hw_fe_buffer_cfg(fe_input_cfg, p_scale_cfg,
    num_pipes);
  if (!p_hw_cmds) {
    JPEG_HW_PR_ERR("%s:%d] FE buffer configuration alloc failed rc = %d",
     __func__, __LINE__, result);
    goto fail;
  }
  result = ioctl (jpegefd, MSM_JPEG_IOCTL_HW_CMDS, p_hw_cmds);
  free (p_hw_cmds);
  JPEG_HW_DBG("ioctl jpegdma_lib_hw_fe_buffer_cfg: rc = %d", result);
  if (result) {
    JPEG_HW_PR_ERR("%s:%d] FE buffer configuration failed rc = %d",
     __func__, __LINE__, result);
    goto fail;
  }

  we_cfg.cbcr_order = fe_input_cfg->input_cbcr_order;
  JPEG_HW_DBG("%s:%d] WE CBCR order %d", __func__, __LINE__,
    we_cfg.cbcr_order);

  //Configure Write Engine
  p_hw_cmds = jpegdma_lib_hw_we_cfg (&we_cfg);
  if (!p_hw_cmds) {
    JPEG_HW_PR_ERR("%s:%d] WE configuration alloc failed rc = %d",
     __func__, __LINE__, result);
    goto fail;
  }
  result = ioctl (jpegefd, MSM_JPEG_IOCTL_HW_CMDS, p_hw_cmds);
  free (p_hw_cmds);
  JPEG_HW_DBG("ioctl jpegdma_lib_hw_we_cfg: rc = %d", result);
  if (result) {
    JPEG_HW_PR_ERR("%s:%d] WE configuration failed rc = %d",
     __func__, __LINE__, result);
    goto fail;
  }

  //Configure Write Engine Buffers
  p_hw_cmds = jpegdma_lib_hw_we_bffr_cfg(we_output_cfg, p_scale_cfg, num_pipes);
  if (!p_hw_cmds) {
    JPEG_HW_PR_ERR("%s:%d] WE buffer configuration alloc failed rc = %d",
     __func__, __LINE__, result);
    goto fail;
  }
  result = ioctl (jpegefd, MSM_JPEG_IOCTL_HW_CMDS, p_hw_cmds);
  free (p_hw_cmds);
  JPEG_HW_DBG("ioctl jpegdma_lib_hw_we_buffer_cfg: rc = %d", result);
  if (result) {
    JPEG_HW_PR_ERR("%s:%d] WE buffer configuration failed rc = %d",
     __func__, __LINE__, result);
    goto fail;
  }

  JPEG_HW_DBG("Scaling enabled.. Setting scaling params");

  p_hw_cmds = jpegdma_lib_hw_scale_cfg(&pp_scale_cfg, p_input_cfg, num_pipes);
  if (!p_hw_cmds) {
    JPEG_HW_PR_ERR("%s:%d] Scale configuration alloc failed rc = %d",
     __func__, __LINE__, result);
    goto fail;
  }
  result = ioctl (jpegefd, MSM_JPEG_IOCTL_HW_CMDS, p_hw_cmds);
  free (p_hw_cmds);
  JPEG_HW_DBG("ioctl jpegdma_lib_hw_scale_cfg: rc = %d", result);
  if (result) {
    JPEG_HW_PR_ERR("%s:%d] Scale configuration failed rc = %d",
     __func__, __LINE__, result);
    goto fail;
  }

  jpeg_clk = JPEG_CLK_RATE_NORMAL;
  if (p_transfer_cfg->speed_mode == JPEGDMA_SPEED_HIGH) {
    jpeg_clk = JPEG_CLK_RATE_HIGH;

    JPEG_HW_DBG("%s:%d] set clk %d", __func__, __LINE__, jpeg_clk);
    result = ioctl (jpegefd, MSM_JPEG_IOCTL_SET_CLK_RATE, &jpeg_clk);
    JPEG_HW_DBG("%s:%d] after set clk", __func__, __LINE__);
    if (result) {
      JPEG_HW_PR_ERR("%s:%d] Set clock rate failed rc = %d",
       __func__, __LINE__, result);
      goto fail;
    }
  }

  JPEG_HW_DBG("%s:%d] success", __func__, __LINE__);
  return result;

fail:
  return result;
}

int jpegdma_lib_input_buf_enq(jpegdma_hw_obj_t jpegdma_hw_obj,
  struct jpegdma_hw_buf *buf)
{
  __jpegdma_hw_obj_t *jpegdma_hw_obj_p = (__jpegdma_hw_obj_t *) jpegdma_hw_obj;
  int jpegefd = jpegdma_hw_obj_p->jpegefd;

  int result;
  struct msm_jpeg_buf msm_buf;
  jpegdma_fe_input_cfg *fe_cfg = &jpegdma_hw_obj_p->fe_input_cfg;

  memset(&msm_buf,0,sizeof( struct msm_jpeg_buf));
  msm_buf.type = buf->type;
  msm_buf.fd   = buf->fd;

  msm_buf.vaddr = buf->vaddr;

  msm_buf.y_off         = buf->y_off;
  msm_buf.y_len         = fe_cfg->image_height_0 * fe_cfg->stride;
  msm_buf.framedone_len = buf->framedone_len;

  msm_buf.cbcr_off = 0;

  msm_buf.cbcr_len = fe_cfg->image_height_1 * fe_cfg->stride;

  JPEG_HW_DBG("%s:%d] input_buf: 0x%p enqueue %d, offset %d,"
    "fd %d ", __func__,__LINE__, buf->vaddr, buf->y_len, buf->offset,
    msm_buf.fd);

  JPEG_HW_DBG("%s:%d] y_off=0x%x cbcr_off=0x%x,"
    "cr_len=%d",__func__,__LINE__, buf->y_off,
    buf->cbcr_off,
    buf->cbcr_len);

  result = ioctl (jpegefd, MSM_JPEG_IOCTL_INPUT_BUF_ENQUEUE, &msm_buf);

  return result;
}

int jpegdma_lib_output_buf_enq(jpegdma_hw_obj_t jpegdma_hw_obj,
  struct jpegdma_hw_buf *buf)
{
  __jpegdma_hw_obj_t *jpegdma_hw_obj_p = (__jpegdma_hw_obj_t *) jpegdma_hw_obj;
  int jpegefd = jpegdma_hw_obj_p->jpegefd;

  int result;
  jpegdma_we_output_cfg *we_cfg = &jpegdma_hw_obj_p->we_output_cfg;
  struct msm_jpeg_buf msm_buf;

  memset(&msm_buf,0,sizeof(struct msm_jpeg_buf));
  msm_buf.type = buf->type;
  msm_buf.fd   = buf->fd;

  msm_buf.vaddr = buf->vaddr;

  msm_buf.y_off         = buf->y_off;
  msm_buf.y_len         = we_cfg->output_height_0 * we_cfg->stride;
  msm_buf.framedone_len = buf->framedone_len;

  msm_buf.cbcr_off = 0;

  //msm_buf.y_off + msm_buf.y_len;

  msm_buf.cbcr_len = we_cfg->output_height_1 * we_cfg->stride;

  result = ioctl (jpegefd, MSM_JPEG_IOCTL_OUTPUT_BUF_ENQUEUE, &msm_buf);
  JPEG_HW_DBG("%s:%d] output_buf: 0x%p enqueue %d, fd %d, result %d", __func__,
    __LINE__, buf->vaddr, buf->y_len, msm_buf.fd, result);
  return result;
}

int jpegdma_lib_transfer (jpegdma_hw_obj_t jpegdma_hw_obj)
{
  __jpegdma_hw_obj_t *jpegdma_hw_obj_p = (__jpegdma_hw_obj_t *) jpegdma_hw_obj;
  int jpegefd = jpegdma_hw_obj_p->jpegefd;

  int result = -1;
  struct msm_jpeg_ctrl_cmd jpegeCtrlCmd;

  struct msm_jpeg_hw_cmd hw_cmd;
  struct msm_jpeg_hw_cmds *p_hw_cmds = NULL;

  p_hw_cmds = jpegdma_lib_hw_start ();
  if (p_hw_cmds) {
    result = ioctl (jpegefd, MSM_JPEG_IOCTL_START, p_hw_cmds);
    JPEG_HW_DBG("ioctl %s: rc = %d", MSM_JPEGDMA_NAME, result);
    free (p_hw_cmds);
  }

  return result;
}

int jpegdma_lib_wait_done (jpegdma_hw_obj_t jpegdma_hw_obj)
{
  __jpegdma_hw_obj_t *jpegdma_hw_obj_p = (__jpegdma_hw_obj_t *) jpegdma_hw_obj;
  int jpegefd = jpegdma_hw_obj_p->jpegefd;

  struct msm_jpeg_ctrl_cmd jpegeCtrlCmd;
  int result = 0;


  JPEG_HW_DBG("%s:%d] jpegdma_hw_lib_wait_thread_ready; event_handler %d",
    __func__, __LINE__, (int)jpegdma_hw_obj_p->event_thread_id);

  if (jpegdma_hw_obj_p->jpegdma_hw_lib_event_handler) {
    jpegdma_hw_lib_wait_thread_ready (jpegdma_hw_obj_p,
      &(jpegdma_hw_obj_p->event_thread_id));
  }

  JPEG_HW_DBG("%s:%d] jpegdma_hw_lib_wait_thread_ready: input_handler %d",
    __func__, __LINE__,(int)jpegdma_hw_obj_p->input_thread_id);

  if (jpegdma_hw_obj_p->jpegdma_hw_lib_input_handler) {
    jpegdma_hw_lib_wait_thread_ready (jpegdma_hw_obj_p,
      &(jpegdma_hw_obj_p->input_thread_id));
  }

  JPEG_HW_DBG("%s:%d] jpegdma_hw_lib_wait_thread_ready: output_handler",
    __func__, __LINE__);

  if (jpegdma_hw_obj_p->jpegdma_hw_lib_output_handler) {
    jpegdma_hw_lib_wait_thread_ready (jpegdma_hw_obj_p,
      &(jpegdma_hw_obj_p->output_thread_id));
  }

  JPEG_HW_DBG("%s:%d] jpegdma_lib_wait_done", __func__, __LINE__);
  return result;
}

int jpegdma_lib_stop (jpegdma_hw_obj_t jpegdma_hw_obj)
{
  int result = 0;
  __jpegdma_hw_obj_t *jpegdma_hw_obj_p = (__jpegdma_hw_obj_t *) jpegdma_hw_obj;
  int jpegefd = jpegdma_hw_obj_p->jpegefd;

  struct msm_jpeg_hw_cmd hw_cmd;
  struct msm_jpeg_hw_cmds *p_hw_cmds = NULL;

  p_hw_cmds = jpegdma_lib_hw_stop ();
  if (p_hw_cmds) {
    JPEG_HW_DBG("%s:%d] ioctl MSM_JPEG_IOCTL_STOP", __func__,
      __LINE__);
    result = ioctl (jpegefd, MSM_JPEG_IOCTL_STOP, p_hw_cmds);
    JPEG_HW_DBG("ioctl %s: rc = %d", MSM_JPEGDMA_NAME, result);

      ioctl (jpegefd, MSM_JPEG_IOCTL_EVT_GET_UNBLOCK);
      ioctl (jpegefd, MSM_JPEG_IOCTL_INPUT_GET_UNBLOCK);
      ioctl (jpegefd, MSM_JPEG_IOCTL_OUTPUT_GET_UNBLOCK);

    free(p_hw_cmds);
  }

  return result;
}
