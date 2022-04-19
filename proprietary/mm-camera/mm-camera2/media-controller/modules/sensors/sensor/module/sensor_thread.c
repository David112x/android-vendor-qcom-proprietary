
/* sensor.c
 *
 * Copyright (c) 2012-2017 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include <stdio.h>
#include <dlfcn.h>
#include <asm-generic/errno-base.h>
#include <poll.h>
#include "sensor.h"
#include "mct_pipeline.h"
#include "sensor_thread.h"

static long long sensor_current_timestamp(void)
{
  struct    timeval te;
  long long milliseconds;
  gettimeofday(&te, NULL);
  /* Calculate milliseconds */
  milliseconds = te.tv_sec*1000LL + te.tv_usec/1000;

  return milliseconds;
}

boolean cancel_autofocus = FALSE;
/** sensor_cancel_autofocus:
 *
 *  Return:
 *
 *  This function cancels the autofocus polling loop **/
void sensor_cancel_autofocus_loop()
{
   cancel_autofocus = TRUE;
}
/** sensor_process_thread_message:
 *
 *  Return:
 *
 *  This function processes the thread message **/
void sensor_process_thread_message(sensor_thread_msg_t *msg)
{
  mct_bus_msg_t               bus_msg;
  mct_bus_msg_af_status_t     af_msg;
  enum sensor_af_t            status;
  int32_t                     i = 0;
  ssize_t                     ret = 0;
  cam_focus_distances_info_t  fdistance;

  SLOW("Processing Pipe message %d", msg->msgtype);
  long long start __attribute__((unused)) = sensor_current_timestamp();

  switch(msg->msgtype){
  case SET_AUTOFOCUS: {
    status = SENSOR_AF_NOT_FOCUSSED;
    while  (i < 20) {
      ret = ioctl(msg->fd, VIDIOC_MSM_SENSOR_GET_AF_STATUS, &status);
      if (ret < 0) {
        SERR("failed");
      }
      if(status ==  SENSOR_AF_FOCUSSED)
        break;
      if(cancel_autofocus) {
        cancel_autofocus = FALSE;
        break;
      }
      usleep(10000);
      i++;
    }
    /* Send the AF call back */
    switch (status) {
    case SENSOR_AF_FOCUSSED:
      af_msg.focus_state = CAM_AF_FOCUSED;
      break;
    default:
      af_msg.focus_state = CAM_AF_NOT_FOCUSED;
      break;
    }
    memset(&fdistance, 0, sizeof(fdistance));
    af_msg.f_distance = fdistance;
    memset(&bus_msg, 0, sizeof(mct_bus_msg_t));
    bus_msg.type = MCT_BUS_MSG_SENSOR_AF_STATUS;
    bus_msg.msg = (void *)&af_msg;
    bus_msg.size = sizeof(mct_bus_msg_af_status_t);
    bus_msg.sessionid = msg->sessionid;
    mct_module_post_bus_msg(msg->module,&bus_msg);
    cancel_autofocus = FALSE;
    SLOW("Setting Auto Focus message received");
    break;
  }
  case OFFLOAD_FUNC: {
    if (msg->offload_func) {
      msg->offload_func(msg->param1, msg->param2, msg->param3, msg->param4);
    } else {
      SERR("msg->offload_func is null");
    }
  }
  break;

  default:
    break;
  }

  long long end __attribute__((unused)) = sensor_current_timestamp();
  SLOW("task took %llu ms", end - start);
}
/** sensor_thread_func: sensor_thread_func
 *
 *  Return:
 *
 *  This is the main thread function **/

void* sensor_thread_func(void *data)
{
   sensor_thread_t *thread = (sensor_thread_t*)data;
   int32_t readfd, writefd;
   pthread_mutex_lock(&thread->mutex);
   thread->is_thread_started = TRUE;
   readfd = thread->readfd;
   writefd = thread->writefd;
   pthread_cond_signal(&thread->cond);
   pthread_mutex_unlock(&thread->mutex);
   struct pollfd pollfds;
   int32_t num_of_fds = 1;
   boolean thread_exit = FALSE;
   int32_t ready = 0;
   pollfds.fd = readfd;
   pollfds.events = POLLIN | POLLPRI;
   cancel_autofocus = FALSE;

   while(!thread_exit){
     ready = poll(&pollfds, (nfds_t)num_of_fds, -1);
      if(ready > 0)
          {
             SLOW("Got some events");
             if(pollfds.revents & (POLLIN | POLLPRI)){
               ssize_t nread = 0;
               sensor_thread_msg_t msg;
               nread = read(pollfds.fd, &msg, sizeof(sensor_thread_msg_t));
               if(nread < 0) {
                 SERR("Unable to read the message");
               }
               if(msg.stop_thread) {
                 break;
               }
               sensor_process_thread_message(&msg);
             }
          }
      else{
          if (errno != EINTR) {
            SERR("Unable to ple exiting the thread");
            break;
          }
      }

   }
   SHIGH("Sensor thread is exiting");
   close(readfd);
   close(writefd);
   pthread_exit(0);
   return NULL;
}
/** sensor_thread_create: sensor_thread_create
 *
 *  Return:
 *
 *  This function creates sensor thread **/
boolean sensor_thread_create(module_sensor_bundle_info_t *s_bundle)
{
  int32_t ret = 0;
  sensor_thread_t thread;
  pthread_attr_t attr;

  if (!s_bundle) {
    SERR("failed: s_bundle %p", s_bundle);
    return FALSE;
  }

  if(pipe(s_bundle->pfd) < 0) {
     SERR("%s: Error in creating the pipe",__func__);
  }

  if (TRUE == s_bundle->is_stereo_configuration &&
      s_bundle->stereo_peer_s_bundle != NULL) {

    s_bundle->stereo_peer_s_bundle->pfd[0] = s_bundle->pfd[0];
    s_bundle->stereo_peer_s_bundle->pfd[1] = s_bundle->pfd[1];

    SLOW("%s: Stereo pair pipe initialization pfd[0]: %d pfd[1]: %d",
      __func__,
      s_bundle->stereo_peer_s_bundle->pfd[0],
      s_bundle->stereo_peer_s_bundle->pfd[1]);
  }

   pthread_attr_init(&attr);
   pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
   pthread_mutex_init(&thread.mutex, NULL);
   pthread_condattr_init(&thread.condattr);
   pthread_condattr_setclock(&thread.condattr, CLOCK_MONOTONIC);
   pthread_cond_init(&thread.cond, &thread.condattr);
   thread.is_thread_started = FALSE;
   thread.readfd = s_bundle->pfd[0];
   thread.writefd = s_bundle->pfd[1];
   ret = pthread_create(&thread.td, &attr, sensor_thread_func, &thread );
   pthread_setname_np(thread.td, "CAM_sensor");
   if(ret < 0) {
     SERR("Failed to create af_status thread");
     return FALSE;
   }
   pthread_mutex_lock(&thread.mutex);
    while(thread.is_thread_started == FALSE) {
       pthread_cond_wait(&thread.cond, &thread.mutex);
    }
   pthread_mutex_unlock(&thread.mutex);
   return TRUE;
}
