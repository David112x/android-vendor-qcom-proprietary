/*============================================================================

 Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 Qualcomm Technologies Proprietary and Confidential.

 ============================================================================*/
#include <stdio.h>
#include <fcntl.h>
#include <poll.h>
#include <errno.h>
#include <media/msm_vpu.h>
#include "vpu_client.h"
#include "vpu_client_private.h"
#include "vpu_log.h"
#include <string.h>
#include <stdlib.h>

#define VPU_V4L2_DEVICE_NAME      "msm_vpu"
#define VPU_DEV_ID_MAX            255

#define DUMP_BUF_INFO               0

#define DEFAULT_TNR_LEVEL           100

typedef enum _vpu_port_type_t
{
  VPU_PORT_INPUT = 0,
  VPU_PORT_OUTPUT = 1,
} vpu_port_type_t;

static enum v4l2_buf_type vpu_buf_type[] =
{
  V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE, /* for input port */
  V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE, /* for output port */
};

vpu_client_ctrl_parm_t vpu_default_ctrl_parm = {
 .tnr_enable = true,
 .tnr_auto_mode = false,
 .tnr_value = DEFAULT_TNR_LEVEL,
};

/** vpu_client_find_vpu_device:
 *
 * Find the V4L2 devide with matching name by reading
 * /sys/class/video4linux entries
 *
 * Returns:
 *   Video Device number on Success
 *   -1 on failure
 **/
static int32_t vpu_client_find_vpu_device()
{
  FILE *fp;
  int32_t dev_id = 0;
  char dev_name[256];
  char name[256];
  for (dev_id = 0; dev_id <= VPU_DEV_ID_MAX; dev_id++) {
    snprintf(dev_name, 256, "/sys/class/video4linux/video%d/name", dev_id);
    fp = fopen(dev_name, "r");
    if (!fp) {
      continue;
    }
    fscanf(fp, "%s", name);
    VPU_LOW("idx=%d, name=%s", dev_id, name);
    if (!strncmp(name, VPU_V4L2_DEVICE_NAME, sizeof(name))) {
      VPU_HIGH("vpu device found: /dev/video%d", dev_id);
      fclose(fp);
      return dev_id;
    }
    fclose(fp);
  }
  VPU_ERR("VPU device not found");
  return -1;
}

/** vpu_client_query_sessions:
 *
 * Query number of available sessions on the VPU driver
 *
 * Returns:
 *   Number of available sessions on Success (Max 2)
 *   -1 on failure
 **/
static int32_t vpu_client_query_sessions(vpu_client_t client)
{
  int rc;
  int num = -1;
  rc = ioctl(client->fd, VPU_QUERY_SESSIONS, &num);
  if (rc < 0) {
    VPU_ERR("VPU_QUERY_SESSIONS ioctl() failed");
    return -1;
  }
  return num;
}

/** vpu_client_attach_to_session:
 *
 * Attach to a session on VPU driver
 *   @client: vpu client handle
 *   @session: id of the session to be attached
 *
 * Returns:
 *   0: Success
 *   negative: failure
 **/
int32_t vpu_client_attach_to_session(vpu_client_t client, int session)
{
  int32_t rc;
  rc = ioctl(client->fd, VPU_ATTACH_TO_SESSION, &session);
  if (rc < 0) {
    VPU_ERR("VPU_ATTACH_TO_SESSION ioctl() failed: %s", strerror(errno));
    return rc;
  }
  client->session_attached = session;
  VPU_LOW("attached to session %d", session);
  return 0;
}


int32_t vpu_create_and_attach_session(vpu_client_t client)
{
  int32_t rc;
  int32_t session;
  rc = ioctl(client->fd, VPU_CREATE_SESSION, &session);
  if (rc < 0) {
    VPU_ERR("VPU_CREATE_SESSION ioctl() failed: %s", strerror(errno));
    return rc;
  }
  client->session_attached = session;
  VPU_DBG("attached to new session %d", session);
  return 0;
}

/** vpu_client_dump_buffer_info:
 *
 * Log v4l2 buffer information used by the driver
 *   @buffer: pointer to a populated v4l2 buffer
 **/
void vpu_client_dump_buffer_info(struct v4l2_buffer *buffer)
{
  uint32_t i;
  VPU_HIGH("v4l2 buffer info");
  VPU_HIGH("buffer.index = %u", buffer->index);
  VPU_HIGH("buffer.type = %u", buffer->type);
  VPU_HIGH("buffer.flags = 0x%08x", buffer->flags);
  VPU_HIGH("buffer.field = 0x%08x", buffer->field);
  VPU_HIGH("buffer.length = %u", buffer->length);
  VPU_HIGH("buffer.memory = %d", buffer->memory);

  for (i = 0; i < buffer->length && i < VIDEO_MAX_PLANES; i++) {
    VPU_HIGH("buffer.m.planes[%d].bytesused = %u", i,
        (uint32_t )buffer->m.planes[i].bytesused);
    VPU_HIGH("buffer.m.planes[%d].length = %u", i,
        (uint32_t )buffer->m.planes[i].length);
    VPU_HIGH("buffer.m.planes[%d].m.fd = %d", i,
        (int32_t )buffer->m.planes[i].m.fd);
    VPU_HIGH("buffer.m.planes[%d].reserved[0] = %u", i,
        (uint32_t )buffer->m.planes[i].reserved[0]);
  }
}

/** vpu_request_bufs:
 *
 * Register a number of buffers on a VPU port
 *   @client: vpu client handle
 *   @buf_count: number of buffers
 *   @port: vpu port type
 *
 * Returns:
 *   0: Success
 *   negative: failure
 **/
int32_t vpu_request_bufs(vpu_client_t client, int32_t buf_count,
    vpu_port_type_t port)
{
  struct v4l2_requestbuffers reqbuf;
  int rc;
  enum v4l2_buf_type buf_type;
  buf_type = vpu_buf_type[port];

  memset(&reqbuf, 0, sizeof(struct v4l2_requestbuffers));
  reqbuf.type = buf_type;
  reqbuf.memory = V4L2_MEMORY_USERPTR;
  reqbuf.count = buf_count;

  rc = ioctl(client->fd, VIDIOC_REQBUFS, &reqbuf);
  if (rc < 0) {
    VPU_ERR("VIDIOC_REQBUFS ioctl() failed: %s", strerror(errno));
    return rc;
  }

  VPU_LOW("VIDIOC_REQBUFS success, requested=%d, accepted=%d", buf_count,
      reqbuf.count);

  return 0;
}

/** vpu_port_queue_buf:
 *
 * Enqueue a buffer on a vpu port
 *   @client: vpu client handle
 *   @buffer: pointer to a valid v4l2 buffer
 *   @port: vpu port type
 *
 * Returns:
 *   0: Success
 *   negative: failure
 **/
int32_t vpu_port_queue_buf(vpu_client_t client, struct v4l2_buffer *buffer,
    vpu_port_type_t port)
{
  int32_t rc;
  VPU_LOW("VIDIOC_QBUF: idx=%d, port=%d, fd=%d", buffer->index, port,
      client->fd);
  buffer->type = vpu_buf_type[port];
  rc = ioctl(client->fd, VIDIOC_QBUF, buffer);
  if (rc < 0) {
    VPU_ERR("VIDIOC_QBUF: ioctl() failed: %s %d", strerror(errno), errno);
    return rc;
  }
  return 0;
}

/** vpu_port_dequeue_buf:
 *
 * Dequeue a buffer on a vpu port
 *   @client: vpu client handle
 *   @buffer: pointer to a empty v4l2 buffer
 *   @port: vpu port type
 *
 * Returns:
 *   0: Success
 *   negative: failure
 **/
int32_t vpu_port_dequeue_buf(vpu_client_t client, struct v4l2_buffer *buffer,
    vpu_port_type_t port)
{
  int32_t rc;
  struct v4l2_plane planes[VIDEO_MAX_PLANES];
  VPU_LOW("VIDIOC_DQBUF: fd=%d port=%d", client->fd, port);
  memset(buffer, 0x00, sizeof(struct v4l2_buffer));
  buffer->type = vpu_buf_type[port];
  buffer->memory = V4L2_MEMORY_USERPTR;
  buffer->m.planes = planes;
  buffer->length = VIDEO_MAX_PLANES;
  rc = ioctl(client->fd, VIDIOC_DQBUF, buffer);
  if (rc < 0) {
    VPU_ERR("VIDIOC_DQBUF ioctl() failed: %s %d", strerror(errno), errno);
    return rc;
  }
  return 0;
}

/** vpu_port_stream_on:
 *
 * Start streaming on a vpu port
 *   @client: vpu client handle
 *   @port: vpu port type
 *
 * Returns:
 *   0: Success
 *   negative: failure
 **/
int32_t vpu_port_stream_on(vpu_client_t client, vpu_port_type_t port)
{
  int32_t rc = 0;
  enum v4l2_buf_type buf_type = vpu_buf_type[port];
  rc = ioctl(client->fd, VIDIOC_STREAMON, &buf_type);
  if (rc < 0) {
    VPU_ERR("VIDIOC_STREAMON ioctl() failed");
    return rc;
  }
  VPU_LOW("VIDIOC_STREAMON: success");
  return 0;
}

/** vpu_port_stream_off:
 *
 * Stop streaming on a vpu port
 *   @client: vpu client handle
 *   @port: vpu port type
 *
 * Returns:
 *   0: Success
 *   negative: failure
 **/
int32_t vpu_port_stream_off(vpu_client_t client, vpu_port_type_t port)
{
  int32_t rc = 0;
  enum v4l2_buf_type buf_type = vpu_buf_type[port];
  rc = ioctl(client->fd, VIDIOC_STREAMOFF, &buf_type);
  if (rc < 0) {
    VPU_ERR("VIDIOC_STREAMOFF ioctl() failed");
    return rc;
  }
  VPU_LOW("VIDIOC_STREAMOFF: success");
  return 0;
}

/** vpu_port_init:
 *
 * Initialize a vpu port
 *   @client: vpu client handle
 *   @fmt: image format info
 *   @port: vpu port type
 *
 * Returns:
 *   0: Success
 *   negative: failure
 **/
int32_t vpu_port_init(vpu_client_t client, struct v4l2_format format,
  vpu_port_type_t port)
{
  int32_t rc;
  enum v4l2_buf_type buf_type = vpu_buf_type[port];
  format.type = buf_type;

  rc = ioctl(client->fd, VIDIOC_S_FMT, &format);
  if (rc < 0) {
    VPU_ERR("VIDIOC_S_FMT ioctl() failed");
    return rc;
  }
  return 0;
}

bool vpu_client_compare_ctrl_parms(vpu_client_ctrl_parm_t p1,
  vpu_client_ctrl_parm_t p2)
{
  if (p1.tnr_enable == p2.tnr_enable &&
      p1.tnr_value == p2.tnr_value &&
      p1.tnr_auto_mode == p2.tnr_auto_mode) {
    return true;
  }
  return false;
}

int32_t vpu_client_set_ctrl_parm(vpu_client_t client,
    vpu_client_ctrl_parm_t ctrl_parm)
{
  int32_t rc;
  struct vpu_control ctrl;

  VPU_DBG("enable=%d, auto_mode=%d, value=%d", ctrl_parm.tnr_enable,
    ctrl_parm.tnr_auto_mode, ctrl_parm.tnr_value);

  if (vpu_client_compare_ctrl_parms(client->cur_ctrl_parm, ctrl_parm)) {
    return 0;
  }

  ctrl.control_id = VPU_CTRL_NOISE_REDUCTION;
  ctrl.data.auto_manual.enable = ctrl_parm.tnr_enable;
  ctrl.data.auto_manual.auto_mode = ctrl_parm.tnr_auto_mode;
  ctrl.data.auto_manual.value = ctrl_parm.tnr_value;

  rc = ioctl(client->fd, VPU_S_CONTROL, &ctrl);
  if (rc < 0) {
    VPU_ERR("VPU_S_CONTROL ioctl() failed, %d, %s", errno, strerror(errno));
    return -1;
  }
  client->cur_ctrl_parm = ctrl_parm;
  return 0;
}

/** vpu_client_set_tnr_enable:
 *
 * API for on-the-fly enable/disable control for the TNR in VPU
 **/
int32_t vpu_client_set_tnr_enable(vpu_client_t client, bool enable)
{
  int32_t rc;
  vpu_client_ctrl_parm_t ctrl_parm;

  if (client->cur_ctrl_parm.tnr_enable == enable) {
    VPU_LOW("tnr_enable is already %d", enable);
    return 0;
  }

  ctrl_parm = client->cur_ctrl_parm;
  ctrl_parm.tnr_enable = enable;
  ctrl_parm.tnr_value = enable ? DEFAULT_TNR_LEVEL : 0;
  rc = vpu_client_set_ctrl_parm(client, ctrl_parm);
  if (rc < 0) {
    VPU_ERR("failed");
    return -1;
  }
  VPU_DBG("tnr_enable = %d", enable);
  return 0;
}

int32_t vpu_client_init_streaming(vpu_client_t client, int32_t buf_count)
{
  int rc;
  rc = vpu_port_init(client, client->format, VPU_PORT_INPUT);
  if (rc < 0) {
    VPU_ERR("vpu_client_port_init() failed");
    return rc;
  }
  rc = vpu_port_init(client, client->format, VPU_PORT_OUTPUT);
  if (rc < 0) {
    VPU_ERR("vpu_client_port_init() failed");
    return rc;
  }
  rc = vpu_request_bufs(client, buf_count, VPU_PORT_INPUT);
  if (rc < 0) {
    VPU_ERR("vpu_client_request_bufs() failed");
    return rc;
  }
  rc = vpu_request_bufs(client, buf_count, VPU_PORT_OUTPUT);
  if (rc < 0) {
    VPU_ERR("vpu_client_request_bufs() failed");
    return rc;
  }
  return 0;
}

int32_t vpu_client_set_format(vpu_client_t client,
  struct v4l2_format format)
{
  client->format = format;
  return 0;
}

#if (DUMP_BUF_INFO || VPU_LOG_LEVEL >= 3)
static void vpu_client_dump_frame_bufs(struct v4l2_buffer *in_buf,
  struct v4l2_buffer *out_buf)
{
  VPU_HIGH("input buffer info");
  vpu_client_dump_buffer_info(in_buf);
  VPU_HIGH("output buffer info");
  vpu_client_dump_buffer_info(out_buf);
}
#else
static void vpu_client_dump_frame_bufs(struct v4l2_buffer *in_buf __unused,
  struct v4l2_buffer *out_buf __unused)
{
  return;
}
#endif

/** vpu_client_sched_frame_for_processing:
 *
 *  Schedules a frame for processing on VPU, input and output
 *  buffers are enqueued on respective ports
 *   @client: vpu client handle
 *   @in_buf: input buffer
 *   @out_buf: output buffer
 *
 * Returns:
 *   0: Success
 *   negative: failure
 **/
int32_t vpu_client_sched_frame_for_processing(vpu_client_t client,
    struct v4l2_buffer *in_buf, struct v4l2_buffer *out_buf)
{
  int32_t rc;
  if (!in_buf || !out_buf) {
    VPU_ERR("failed");
    return -1;
  }
  out_buf->sequence = in_buf->sequence;
  vpu_client_dump_frame_bufs(in_buf, out_buf);

  PTHREAD_MUTEX_LOCK(&client->mutex);
  if (client->is_streaming == false) {
    VPU_ERR("cannot process frame when not streaming");
    PTHREAD_MUTEX_UNLOCK(&client->mutex);
    return -1;
  }

  rc = vpu_port_queue_buf(client, out_buf, VPU_PORT_OUTPUT);
  if (rc < 0) {
    VPU_ERR("vpu_port_queue_buf() failed for output port");
    vpu_client_dump_buffer_info(out_buf);
    PTHREAD_MUTEX_UNLOCK(&client->mutex);
    return rc;
  }
  client->output_bufs_pending++;
  rc = vpu_port_queue_buf(client, in_buf, VPU_PORT_INPUT);
  if (rc < 0) {
    VPU_ERR("vpu_port_queue_buf() failed for input port");
    vpu_client_dump_buffer_info(in_buf);
    PTHREAD_MUTEX_UNLOCK(&client->mutex);
    return rc;
  }
  client->input_bufs_pending++;
  PTHREAD_MUTEX_UNLOCK(&client->mutex);
  return 0;
}

/** vpu_subscribe_for_events:
 *
 *  Subscribe for v4l2 events from vpu driver
 *   @client: vpu client handle
 *
 * Returns:
 *   0: Success
 *   negative: failure
 **/
int32_t vpu_subscribe_for_events(vpu_client_t client)
{
  int32_t rc;
  struct v4l2_event_subscription sub;
  memset(&sub, 0x00, sizeof(struct v4l2_event_subscription));
  sub.type = V4L2_EVENT_ALL;
  rc = ioctl(client->fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
  if (rc < 0) {
    VPU_ERR("VIDIOC_SUBSCRIBE_EVENT ioctl() failed");
    return rc;
  }
  return 0;
}

/** vpu_dequeue_event:
 *
 *  Dequeue an available event from vpu driver
 *   @client: vpu client handle
 *   @event: pointer to a v4l2_event structure
 *
 * Returns:
 *   0: Success
 *   negative: failure
 **/
int32_t vpu_dequeue_event(vpu_client_t client, struct v4l2_event *event)
{
  int32_t rc;
  memset(event, 0x00, sizeof(struct v4l2_event));
  rc = ioctl(client->fd, VIDIOC_DQEVENT, event);
  if (rc < 0) {
    VPU_ERR("VIDIOC_DQEVENT ioctl() failed");
    return rc;
  }
  return 0;
}

/** vpu_handle_input_buf_event:
 *
 *  Dequeues a buffer from vpu input port and call the
 *  registered callback
 *
 *   @client: vpu client handle
 *
 * Returns:
 *   0: Success
 *   negative: failure
 **/
int32_t vpu_handle_input_buf_event(vpu_client_t client)
{
  int rc;
  struct v4l2_buffer buffer;
  if (client->input_bufs_pending == 0) {
    VPU_ERR("no input buf pending");
    return 0;
  }
  PTHREAD_MUTEX_LOCK(&client->mutex);
  rc = vpu_port_dequeue_buf(client, &buffer, VPU_PORT_INPUT);
  if (rc < 0) {
    VPU_ERR("vpu_dequeue_buf() failed");
    PTHREAD_MUTEX_UNLOCK(&client->mutex);
    return rc;
  }
  client->input_bufs_pending--;
  PTHREAD_MUTEX_UNLOCK(&client->mutex);
  client->event_cb(VPU_CLIENT_EVENT_INPUT_BUF_READY, &buffer, client->userdata);
  return 0;
}

/** vpu_handle_output_buf_event:
 *
 *  Dequeues a buffer from vpu output port and call the
 *  registered callback
 *
 *   @client: vpu client handle
 *
 * Returns:
 *   0: Success
 *   negative: failure
 **/
int32_t vpu_handle_output_buf_event(vpu_client_t client)
{
  int rc;
  struct v4l2_buffer buffer;
  if (client->output_bufs_pending == 0) {
    VPU_ERR("no output buf pending");
    return 0;
  }
  PTHREAD_MUTEX_LOCK(&client->mutex);
  rc = vpu_port_dequeue_buf(client, &buffer, VPU_PORT_OUTPUT);
  if (rc < 0) {
    VPU_ERR("vpu_dequeue_buf() failed");
    PTHREAD_MUTEX_UNLOCK(&client->mutex);
    return rc;
  }
  client->output_bufs_pending--;
  PTHREAD_MUTEX_UNLOCK(&client->mutex);
  client->event_cb(VPU_CLIENT_EVENT_OUTPUT_BUF_READY, &buffer,
      client->userdata);
  return 0;
}

/** vpu_handle_event:
 *
 *  Handle various events from the vpu driver
 *
 *   @client: vpu client handle
 *
 * Returns:
 *   0: Success
 *   negative: failure
 **/
int32_t vpu_handle_event(vpu_client_t client)
{
  int32_t rc;
  struct v4l2_event event;
  rc = vpu_dequeue_event(client, &event);
  if (rc < 0) {
    VPU_ERR("vpu_dequeue_event() failed");
    return rc;
  }
  switch (event.type) {
  case VPU_EVENT_FLUSH_DONE:
    VPU_DBG("VPU_EVENT_FLUSH_DONE");
    break;
  case VPU_EVENT_ACTIVE_REGION_CHANGED:
    VPU_DBG("VPU_EVENT_ACTIVE_REGION_CHANGED");
    break;
  case VPU_EVENT_SESSION_TIMESTAMP:
    VPU_DBG("VPU_EVENT_SESSION_TIMESTAMP");
    break;
  case VPU_EVENT_HW_ERROR:
    VPU_DBG("VPU_EVENT_HW_ERROR");
    break;
  case VPU_EVENT_INVALID_CONFIG:
    VPU_DBG("VPU_EVENT_INVALID_CONFIG");
    break;
  case VPU_EVENT_FAILED_SESSION_STREAMING:
    VPU_DBG("VPU_EVENT_FAILED_SESSION_STREAMING");
    break;
  default:
    VPU_ERR("invalid vpu event type = %d", event.type);
    return -1;
  }
  return 0;
}

int32_t vpu_client_thread_handle_pipe_event(vpu_client_t client)
{
  int32_t num_read;
  vpu_client_pipe_msg_t pipe_msg;
  num_read = read(client->pfd[READ_FD], &(pipe_msg), sizeof(vpu_client_pipe_msg_t));
  if(num_read < 0) {
    VPU_ERR("read() failed");
    pthread_exit(NULL);
  } else if (num_read != sizeof(vpu_client_pipe_msg_t)) {
    VPU_ERR("failed, in read(), num_read=%d, msg_size=%d",
        num_read, sizeof(vpu_client_pipe_msg_t));
    pthread_exit(NULL);
  }
  switch(pipe_msg.type) {
    case PIPE_MSG_EXIT:
      VPU_DBG("client thread exit");
      pthread_exit(NULL);
      break;
    default:
      VPU_ERR("invalid pipe msg");
  }
  return 0;
}

/** vpu_client_thread_func:
 *
 *  Entry function for vpu client thread. Polls the vpu device
 *  for available events/buffers
 *
 *   @arg: void* to vpu client handle
 *
 * Returns:
 *   NULL
 **/
static void* vpu_client_thread_func(void *arg)
{
  int32_t ready;
  struct pollfd poll_fds[2];
  vpu_client_t client = (vpu_client_t) arg;

  /* subscribe to v4l2 events for VPU */
  vpu_subscribe_for_events(client);

  poll_fds[0].fd = client->fd;
  poll_fds[0].events = POLLIN|POLLOUT|POLLPRI|POLLWRNORM|POLLRDNORM;
  poll_fds[1].fd = client->pfd[READ_FD];
  poll_fds[1].events = POLLIN|POLLPRI;

  VPU_DBG("vpu_poll_thread entering polling loop");
  while (1) {
    ready = poll(poll_fds, 2, -1);
    if (ready < 0) {
      if (errno != EINTR) {
        VPU_ERR("poll() failed");
        return NULL;
      }
    } else if (ready == 0) {
      VPU_ERR("poll() timed out");
    } else {
      /* handle events on device fd */
      if (poll_fds[0].revents & (POLLOUT|POLLWRNORM)) {
        /* empty buffer available on input port */
        vpu_handle_input_buf_event(client);
      }
      if (poll_fds[0].revents & (POLLIN|POLLRDNORM)) {
        /* filled buffer available on output port */
        vpu_handle_output_buf_event(client);
      }
      if (poll_fds[0].revents & POLLPRI) {
        vpu_handle_event(client);
      }
      /* handle events on pipe fd */
      if (poll_fds[1].revents & (POLLIN|POLLPRI)) {
        vpu_client_thread_handle_pipe_event(client);
      }
    }
    /* signal any waiting threads about no pending buffers */
    if (client->input_bufs_pending == 0 && client->output_bufs_pending == 0) {
      pthread_cond_broadcast(&client->cond_no_pending_buf);
    }
  }
}

int32_t vpu_client_start_polling(vpu_client_t client)
{
  int32_t rc;
  rc = pthread_create(&(client->vpu_poll_thread),
                      NULL, vpu_client_thread_func, client);
  if(rc < 0) {
    VPU_ERR("pthread_create() failed");
    return rc;
  }
  pthread_setname_np(client->vpu_poll_thread,"CAM_vpu");
  client->is_polling = true;
  return 0;
}

int32_t vpu_client_stop_polling(vpu_client_t client)
{
  vpu_client_pipe_msg_t pipe_msg;
  /* signal the thread to exit and wait for thread to finish */
  pipe_msg.type = PIPE_MSG_EXIT;
  write(client->pfd[WRITE_FD], &pipe_msg, sizeof(vpu_client_pipe_msg_t));
  pthread_join(client->vpu_poll_thread, NULL);
  client->is_polling = false;
  return 0;
}

/** vpu_client_stream_on:
 *
 *  Starts streaming on both vpu ports
 *   @client: vpu client handle
 *
 * Returns:
 *   0: Success
 *   negative: failure
 **/
int32_t vpu_client_stream_on(vpu_client_t client)
{
  /* start the client thread */
  PTHREAD_MUTEX_LOCK(&client->mutex);
  if (client->is_streaming == false) {
    vpu_client_start_polling(client);
    vpu_port_stream_on(client, VPU_PORT_INPUT);
    vpu_port_stream_on(client, VPU_PORT_OUTPUT);
    client->is_streaming = true;
  } else {
    VPU_ERR("stream on called when already streaming");
  }
  PTHREAD_MUTEX_UNLOCK(&client->mutex);
  return 0;
}

/** vpu_client_stream_off:
 *
 *  Waits for any pending buffers to be processed. Stops
 *  streaming on both vpu ports.
 *   @client: vpu client handle
 *
 * Returns:
 *   0: Success
 *   negative: failure
 **/
int32_t vpu_client_stream_off(vpu_client_t client)
{
  PTHREAD_MUTEX_LOCK(&client->mutex);
  if (client->is_streaming == true) {
    VPU_DBG("waiting for pending bufs....");
    if (client->input_bufs_pending != 0 && client->output_bufs_pending != 0) {
      pthread_cond_wait(&client->cond_no_pending_buf, &client->mutex);
    }
    vpu_port_stream_off(client, VPU_PORT_INPUT);
    vpu_port_stream_off(client, VPU_PORT_OUTPUT);
    client->is_streaming = false;

    vpu_client_init_streaming(client, 0); //todo
    vpu_client_stop_polling(client);
    VPU_DBG("stream off done.");
  } else {
    VPU_ERR("stream off called when not streaming");
  }
  PTHREAD_MUTEX_UNLOCK(&client->mutex);
  return 0;
}

int32_t vpu_client_init_session(vpu_client_t client, vpu_client_cb_t cb,
    void *userdata)
{
  char dev_path[256];
  int32_t rc;

  if (!client || !cb) {
    VPU_ERR("failed, client=%p, cb=%p", client, cb);
    return -1;
  }
  snprintf(dev_path, sizeof(dev_path), "/dev/video%d", client->dev_id);
  client->fd = open(dev_path, O_RDWR | O_NONBLOCK);
  if (client->fd < 0) {
    VPU_ERR("device open failed for %s", dev_path);
    return -1;
  }
  int num;
  if ((num = vpu_client_query_sessions(client)) <= 0) {
    VPU_ERR("VPU session unavailabe, num=%d", num);
    goto err0;
  }
  if (vpu_client_attach_to_session(client, 1) < 0) { // todo
    VPU_ERR("cannot attach to session 1");
    goto err0;
  }
  /* register callback function */
  client->event_cb = cb;
  client->userdata = userdata;
  client->is_session_open = true;

  /* set default controls */
  rc = vpu_client_set_ctrl_parm(client, vpu_default_ctrl_parm);
  if (rc < 0) {
    VPU_ERR("failed");
    return -1;
  }
  VPU_LOW("success, fd=%d", client->fd);
  return 0;

  err0: close(client->fd);
  return -1;
}

int32_t vpu_client_deinit_session(vpu_client_t client)
{
  if (client->is_session_open == false) {
    VPU_ERR("session not initialized");
    return -1;
  }
  close(client->fd);
  client->event_cb = NULL;
  client->userdata = NULL;
  return 0;
}

int32_t vpu_client_get_buf_requirements(vpu_client_t client __unused,
  vpu_client_buf_req_t* buf_req)
{
  buf_req->stride_padding = 128;
  buf_req->scanline_padding = 2;
  return 0;
}

/** vpu_client_create:
 *
 *  Create a new vpu client instance.
 *
 * Returns:
 *   vpu_client handle on success
 *   NULL on failure
 **/
vpu_client_t vpu_client_create()
{
  vpu_client_t client;
  int32_t dev_id;
  int32_t rc;

  client = (vpu_client_t) CAM_MALLOC(sizeof(struct _vpu_client_t));
  if (!client) {
    VPU_ERR("CAM_MALLOC() failed");
    return NULL;
  }
  memset(client, 0x00, sizeof(struct _vpu_client_t));
  dev_id = vpu_client_find_vpu_device();
  if (dev_id < 0) {
    VPU_ERR("failed");
    goto err0;
  }
  client->dev_id = dev_id;
  pthread_cond_init(&client->cond_no_pending_buf, NULL);
  pthread_mutex_init(&(client->mutex), NULL);
  pthread_mutex_init(&(client->th_data.mutex), NULL);

  /* Create PIPE for communication with worker thread */
  rc = pipe(client->pfd);
  if (rc < 0) {
    VPU_ERR("pipe() failed");
    goto err0;
  }
  return client;

  err0: free(client);
  return NULL;
}

/** vpu_client_destroy:
 *
 *  Destry a vpu client instance, stop streaming and release all
 *  resources.
 *  @client: vpu client handle
 *
 */
void vpu_client_destroy(vpu_client_t client)
{
  /* stream_off if already streaming */
  if (client->is_streaming) {
    vpu_client_stream_off(client);
  }
  if (client->is_session_open) {
    vpu_client_deinit_session(client);
  }
  /* destroy data structures */
  pthread_cond_destroy(&client->cond_no_pending_buf);
  pthread_mutex_destroy(&(client->mutex));
  close(client->pfd[READ_FD]);
  close(client->pfd[WRITE_FD]);
  free(client);
  VPU_DBG("vpu client destroyed");
}
