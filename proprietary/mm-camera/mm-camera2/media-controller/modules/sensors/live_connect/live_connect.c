/* live_connect.c
 *
 * Copyright (c) 2015,2016 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include <stdio.h>
#include <dlfcn.h>
#include <poll.h>
#include <asm-generic/errno-base.h>
#include "live_connect.h"
#include <linux/socket.h>
#include <arpa/inet.h>

#define LIVE_CONNECT_PING_CODE 0xDEADBEEF

typedef enum {
  THREAD_FD,
  TCP_IP_FD,
  MAX_FD
} live_connect_fd;

/** live_connect_read_data:
 *
 * @connfd: connection fd
 * @readBuff: pointer to the data buffer
 * @bytes_to_read: number of bytes to read from socket.
 *
 * Return: TRUE for success and FALSE on failure
 **/
static boolean live_connect_read_data(int connfd, uint8_t *readBuff,
  int bytes_to_read)
{
  int i = 0;
  int bytes_read = 0;
  uint8_t *data = NULL;

  RETURN_ON_NULL(readBuff);

  while((bytes_read = read(connfd, readBuff, bytes_to_read)) > 0)
  {
    if (bytes_to_read == bytes_read)
    {
      data = readBuff;
      for (i = 0;i < bytes_to_read; i++)
        SLOW("received %d", data[i]);
      return TRUE;
    }
    bytes_to_read -= bytes_read;
    readBuff += bytes_read;
    continue;
  }
  return FALSE;
}

/** live_connect_write_data:
 *
 * @connfd: connection fd
 * @writeBuff: pointer to the data buffer
 * @bytes_to_write: number of bytes to write to socket.
 *
 * Return: TRUE for success and FALSE on failure
 **/
static boolean live_connect_write_data(int connfd, uint8_t *writeBuff,
  int bytes_to_write)
{
  int i = 0;
  int bytes_written = 0;
  uint8_t *data = NULL;

  RETURN_ON_NULL(writeBuff);
  while((bytes_written = write(connfd, writeBuff, bytes_to_write)) > 0)
  {
    if (bytes_to_write == bytes_written)
    {
      data = writeBuff;
      for (i = 0;i < bytes_to_write; i++)
        SLOW("send %d", data[i]);
      return TRUE;
    }
    bytes_to_write -= bytes_written;
    writeBuff += bytes_written;
    continue;
  }
  return FALSE;
}

/** live_connect_write_data:
 *
 * @connfd: connection fd
 * @item_map: pointer to the data buffer
 * @live_connect_cmd: live connect command type.
 * @is_read: flag to notify if current operation is read or write from client perspective.
 * @value: value to read or write.
 *
 * Return: TRUE for success and FALSE on failure
 **/
static boolean live_connect_sent_receive_data(
  int connfd, live_connect_items_map *item_map, int live_connect_cmd,
  uint8_t is_read, uint32_t value)
{
  if (is_read == 1) {
    switch(item_map[live_connect_cmd].size)
    {
      case 2:
        SLOW("pValue = %p thread id = %u",
          item_map[live_connect_cmd].pValue, (uint32_t)pthread_self());
        value = *((uint16_t *)item_map[live_connect_cmd].pValue);
        break;
      case 4:
        value = *((uint32_t *)item_map[live_connect_cmd].pValue);
        SLOW("value = %d", value);
        break;
      default:
        SHIGH("error wrong value = %d", item_map[live_connect_cmd].size);
        break;
    }
    RETURN_ON_FALSE(live_connect_write_data(connfd, (uint8_t *)&value, 4));
  }
  else if (is_read == 0) {
    SLOW("live_connect_cmd = %d size = %d value = %d",
      live_connect_cmd, item_map[live_connect_cmd].size, value);
    switch(item_map[live_connect_cmd].size)
    {
      case 2:
        *((uint16_t *)item_map[live_connect_cmd].pValue) = (uint16_t)value;
        break;
      case 4:
        *((uint32_t *)item_map[live_connect_cmd].pValue) = value;
        break;
      default:
        SHIGH("error wrong value value = %d", item_map[live_connect_cmd].size);
        break;
    }
  }
  else {
    SERR("Invalid value read");
    return FALSE;
  }
  return TRUE;
}

/** live_connect_process:
 *
 * @connfd: connection fd
 * @s_bundle: pointer to sensor bundle sent from parent thread.
 * @item_map: map to live connect items.
 * @pInfo: pointer to live connect info.
 *
 * Return: TRUE for success and FALSE on failure
 **/
static boolean live_connect_process(int connfd,
  module_sensor_bundle_info_t *s_bundle,
  live_connect_items_map *item_map,
  live_connect_info * pInfo)
{
  uint8_t  readData[20];
  uint8_t  *readBuff = readData;
  uint8_t  writeData[20];
  uint8_t  *writeBuff = writeData;
  int32_t  rc = 0;
  uint32_t live_connect_cmd = 0;
  uint8_t  is_read = 0;
  uint32_t value = 0;
  sensor_submodule_event_type_t event_type;
  void *data_ptr = NULL;
  actuator_live_tune_ctrl_t live_tune_data;
  af_update_t af_update;

  RETURN_ON_NULL(item_map);
  RETURN_ON_NULL(s_bundle);
  RETURN_ON_NULL(pInfo);

  SLOW("item_map = %p thread_id = %u", item_map, (uint32_t)pthread_self());
  RETURN_ON_FALSE(live_connect_read_data(connfd, readBuff, 9));
  live_connect_cmd = *((uint32_t *)readBuff);
  is_read = readBuff[4];
  value = *((uint32_t *)(readBuff + 5));

  SLOW("value = %d\n", value);
  if ((live_connect_cmd >= pInfo->live_connect_cmd_start) &&
    (live_connect_cmd <= pInfo->live_connect_cmd_end)) {
    RETURN_ON_FALSE(live_connect_sent_receive_data(connfd, item_map,
      live_connect_cmd, is_read, value));
    return TRUE;
  }

  SLOW("live_connect_cmd = 0x%X", live_connect_cmd);
  switch (live_connect_cmd) {
    case LIVE_CONNECT_CMD_MOVE_FOCUS_NEAR:
    {
      memset(&af_update, 0, sizeof(af_update));
      af_update.move_lens = TRUE;
      af_update.direction = MOVE_FAR;
      af_update.num_of_steps = value;
      event_type = ACTUATOR_MOVE_FOCUS;
      data_ptr = &af_update;
    }
      break;
    case LIVE_CONNECT_CMD_MOVE_FOCUS_FAR:
    {
      memset(&af_update, 0, sizeof(af_update));
      af_update.move_lens = TRUE;
      af_update.direction = MOVE_NEAR;
      af_update.num_of_steps = value;
      event_type = ACTUATOR_MOVE_FOCUS;
      data_ptr = &af_update;
    }
      break;
    case LIVE_CONNECT_CMD_PING:
      value = LIVE_CONNECT_PING_CODE;
      RETURN_ON_FALSE(live_connect_write_data(connfd, (uint8_t *)&value, 4));
      return TRUE;
    case LIVE_CONNECT_CMD_GET_CUR_STEP_POSITION:
    {
      live_tune_data.tuning_cmd = ACTUATOR_LIVE_TUNE_GET_CUR_STEP_POSITION;
      event_type = ACTUATOR_FOCUS_LIVE_TUNING;
      data_ptr = &live_tune_data;
    }
      break;
    case LIVE_CONNECT_CMD_GET_CUR_LENS_POSITION:
    {
      live_tune_data.tuning_cmd = ACTUATOR_LIVE_TUNE_GET_CUR_LENS_POSITION;
      event_type = ACTUATOR_FOCUS_LIVE_TUNING;
      data_ptr = &live_tune_data;
    }
      break;
    case LIVE_CONNECT_CMD_RUN_RINGING_TEST:
    {
      live_tune_data.tuning_cmd = ACTUATOR_LIVE_TUNE_RINGING_TEST;
      live_tune_data.u.delay = value;
      live_tune_data.step_size = is_read;
      event_type = ACTUATOR_FOCUS_LIVE_TUNING;
      data_ptr = &live_tune_data;
    }
      break;
    case LIVE_CONNECT_CMD_RUN_LINEAR_TEST:
    {
      live_tune_data.tuning_cmd = ACTUATOR_LIVE_TUNE_LINEARITY_TEST;
      live_tune_data.u.delay = value;
      live_tune_data.step_size = is_read;
      event_type = ACTUATOR_FOCUS_LIVE_TUNING;
      data_ptr = &live_tune_data;
    }
      break;
    case LIVE_CONNECT_CMD_START_PLOT:
    {
      live_tune_data.tuning_cmd = ACTUATOR_LIVE_TUNE_START_PLOT;
      event_type = ACTUATOR_FOCUS_LIVE_TUNING;
      data_ptr = &live_tune_data;
    }
      break;
    case LIVE_CONNECT_CMD_STOP_PLOT:
    {
      live_tune_data.tuning_cmd = ACTUATOR_LIVE_TUNE_STOP_PLOT;
      event_type = ACTUATOR_FOCUS_LIVE_TUNING;
      data_ptr = &live_tune_data;
    }
      break;
    case LIVE_CONNECT_CMD_SET_DEFAULT_FOCUS:
    {
      live_tune_data.tuning_cmd = ACTUATOR_LIVE_TUNE_SET_DEFAULT_FOCUS;
      event_type = ACTUATOR_FOCUS_LIVE_TUNING;
      data_ptr = &live_tune_data;
    }
      break;
    default:
      SHIGH("Invalid live connect command = %d", live_connect_cmd);
      return FALSE;
  }

  SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_ACTUATOR,
    event_type, data_ptr, rc);
  if (rc < 0)
    SERR("ACTUATOR_MOVE_FOCUS failed");

  if ((live_connect_cmd == LIVE_CONNECT_CMD_GET_CUR_STEP_POSITION) ||
    (live_connect_cmd == LIVE_CONNECT_CMD_GET_CUR_LENS_POSITION)) {
      value = live_tune_data.u.step_position;
      RETURN_ON_FALSE(live_connect_write_data(connfd, (uint8_t *)&value, 4));
  }
  else if (live_connect_cmd == LIVE_CONNECT_CMD_STOP_PLOT) {
    uint32_t index = 0;
    uint8_t data_buf[512];
    int32_t size = live_tune_data.u.plot_info.size;

    index = 0;
    memcpy(&data_buf[index], &size, sizeof(size));

    index += sizeof(size);
    memcpy(&data_buf[index], &(live_tune_data.u.plot_info.step_pos[0]),
      sizeof(live_tune_data.u.plot_info.step_pos[0]) * size);

    index += sizeof(live_tune_data.u.plot_info.step_pos[0]) * size;
    memcpy(&data_buf[index], &(live_tune_data.u.plot_info.lens_pos[0]),
      sizeof(live_tune_data.u.plot_info.lens_pos[0]) * size);

    index += sizeof(live_tune_data.u.plot_info.lens_pos[0]) * size;
    RETURN_ON_FALSE(live_connect_write_data(connfd, &data_buf[0], 512));
  }
  return TRUE;
}

/** live_connect_update_map:
 *
 * @item_map: map to live connect items.
 * @s_bundle: pointer to sensor bundle sent from parent thread.
 * @pInfo: pointer to live connect info.
 *
 * Return: TRUE for success and FALSE on failure
 **/
static boolean live_connect_update_map(live_connect_items_map **item_map,
  module_sensor_bundle_info_t *s_bundle,
  live_connect_info *pInfo)
{
  boolean                  status = TRUE;
  int32_t                  rc = 0;
  int                      num_items = 0;
  actuator_driver_params_t *af_driver_ptr = NULL;

  RETURN_ON_NULL(item_map);
  RETURN_ON_NULL(pInfo);

  num_items = live_connect_get_cmd_num();
  RETURN_ON_FALSE(num_items);
  SLOW("num_items = %d", num_items);

  pInfo->live_connect_cmd_start = live_connect_get_start_cmd();
  pInfo->live_connect_cmd_end = live_connect_get_end_cmd();

  *item_map = malloc(num_items * sizeof(live_connect_items_map));
  RETURN_ON_NULL(*item_map);

  if (s_bundle->sensor_info->subdev_id[SUB_MODULE_ACTUATOR] == -1) {
    SHIGH("no actuator for this sensor. returning from live connect server.");
    return FALSE;
  }

  SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_ACTUATOR,
    ACTUATOR_GET_AF_DRIVER_PARAM_PTR, &af_driver_ptr, rc);
  if (rc < 0 ||!af_driver_ptr) {
    SERR("ACTUATOR_GET_AF_DRIVER_PARAM_PTR failed");
    return FALSE;
  }

  status = live_connect_fill_map(*item_map,
    &af_driver_ptr->actuator_tuned_params);
  RETURN_ON_FALSE(status);
  return TRUE;
}

/** live_connect_process_poll:
 *
 * @pollfd_ptr: pointer to poll file descriptor
 * @num_fd: number of fds to poll on.
 *
 * Return: 0 for success and negative value on failure
 **/
static int32_t live_connect_process_poll(
  struct pollfd * pollfd_ptr, int num_fd)
{
  int32_t rc = 0;
  int32_t i = 0;

  if (!pollfd_ptr) {
    SERR("pollfd pointer NULL");
    return -1;
  }

  while(TRUE)
  {
    rc = poll(pollfd_ptr, num_fd, -1);
    if (rc <= 0)
    {
      if (errno != EINTR) {
        SERR("live_connect poll failed");
        return 0;
      }
    }
    for (i = 0; i < num_fd; i++)
    {
      if (!(pollfd_ptr[i].revents & pollfd_ptr[i].events))
        continue;
      switch (i) {
      case THREAD_FD:
        return -1;
      case TCP_IP_FD:
        if (pollfd_ptr[i].revents & POLLIN)
          return 1;
        else {
          SERR("received error event");
          return 0;
        }
      }
    }
    SERR("false alarm of poll");
  }
}

/** live_connect_server_thread:
 *
 * @data: pointer to sensor bundle sent from parent thread.
 *
 * Return: NULL.
 **/
void* live_connect_server_thread(void *data)
{
  int listenfd = 0, connfd = -1, i = 0, on = 1;
  int32_t status = 0;
  struct sockaddr_in serv_addr;
  live_connect_items_map *item_map = NULL;
  module_sensor_bundle_info_t *s_bundle = (module_sensor_bundle_info_t *)data;
  struct pollfd fds[MAX_FD];
  int32_t nread = 0;
  uint8_t value = 0;
  int32_t read_fd = 0;
  int32_t write_fd = 0;
  live_connect_info info;

  if (!s_bundle) {
    SERR("s_bindle NULL");
    return NULL;
  }

  read_fd = s_bundle->live_connect_fd[0];
  write_fd = s_bundle->live_connect_fd[1];

  /* Update entry information into map */
  if(!live_connect_update_map(&item_map, s_bundle, &info))
  {
    SERR("live_connect_update_map failed. could be non-fatal");
    goto live_connect_fd_close;
  }

  SLOW("item_map = %p  thread_id = %u", item_map, (uint32_t)pthread_self());

  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  SLOW("listen fd = %d", listenfd);
  status = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,
    (const char *)&on, sizeof (on));
  if (status < 0)
  {
    SERR("setsockopt failed");
    goto live_connect_fd_close;
  }
  memset(&serv_addr, 0, sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(5123);
  bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
  listen(listenfd, 10);

  fds[THREAD_FD].fd = s_bundle->live_connect_fd[0];
  fds[THREAD_FD].events = POLLIN;
  fds[TCP_IP_FD].events = POLLIN;

  while(TRUE)
  {
    fds[TCP_IP_FD].fd = listenfd;
    fds[THREAD_FD].revents = 0;
    fds[TCP_IP_FD].revents = 0;

    SLOW("waiting to connect to tool..");

    status = live_connect_process_poll(&fds[0], MAX_FD);
    if (status <= 0) {
      SERR("live_connect_process_poll failed");
      break;
    }

    connfd = accept(listenfd, (struct sockaddr *)NULL, NULL);
    SHIGH("connected to tool...");
    fds[TCP_IP_FD].fd = connfd;

    while(TRUE)
    {
      SLOW("waiting to read data");
      fds[THREAD_FD].revents = 0;
      fds[TCP_IP_FD].revents = 0;
      status = live_connect_process_poll(&fds[0], MAX_FD);
      if (status > 0) {
        BREAK_ON_FALSE(live_connect_process(connfd, s_bundle, item_map, &info));
      }
      else if(status == 0) {
        SERR("ERROR revent");
        break;
      }
      else {
        SHIGH("received pipe message");
        goto live_connect_socket_close;
      }
    }
    SHIGH("Closing client socket fd");
    close(connfd);
    sleep(1);
  }

live_connect_socket_close:
  close(connfd);
  close(listenfd);
live_connect_fd_close:
  free(item_map);
  close(read_fd);
  close(write_fd);
  SHIGH("Closing live conect socket and exiting thread");
  pthread_exit(0);
  return NULL;
}

/** sensor_live_connect_thread_create
 *
 * @s_bundle: pointer to sensor bundle sent from parent thread.
 *
 * return TRUE if success and FALSE if failure.
 *
 **/
boolean sensor_live_connect_thread_create(
  module_sensor_bundle_info_t *s_bundle)
{
  int ret = 0;
  pthread_t td;
  pthread_attr_t attr;

  /* Validate parameters */
  RETURN_ON_NULL(s_bundle);

  if(pipe(s_bundle->live_connect_fd) < 0) {
     SERR("%s: Error in creating the pipe in live connect", __func__);
     return TRUE;
  }

  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

  ret = pthread_create(&td, &attr, live_connect_server_thread, s_bundle);
  if(ret < 0) {
    SERR("Failed to create af_status thread");
    return FALSE;
  }
  return TRUE;
}

/** sensor_live_connect_thread_destroy
 *
 * @s_bundle: pointer to sensor bundle sent from parent thread.
 *
 **/
void sensor_live_connect_thread_destroy(
  module_sensor_bundle_info_t *s_bundle)
{
  int32_t nwrite = 0;
  uint8_t value = 123;

  nwrite = write(s_bundle->live_connect_fd[1], &value, sizeof(value));
  if(nwrite < 0)
  {
    SERR("%s: Writing into fd failed",__func__);
  }
}
