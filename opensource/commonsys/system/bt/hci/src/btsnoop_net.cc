/******************************************************************************
 *
 *  Copyright (C) 2013 Google, Inc.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/

#define LOG_TAG "bt_snoop_net"

#include <base/logging.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <unistd.h>

#include <fcntl.h>
#include <mutex>

#include "osi/include/log.h"
#include "osi/include/osi.h"

static void safe_close_(int* fd);
static void* listen_fn_(void* context);

extern int local_snoop_socket_create();
extern void update_snoop_fd(int snoop_fd);

extern bool is_vndbtsnoop_enabled;

static const char* LISTEN_THREAD_NAME_ = "btsnoop_net_listen";
static const int LOCALHOST_ = 0x7F000001;
static const int LISTEN_PORT_ = 8872;

static pthread_t listen_thread_;
static bool listen_thread_valid_ = false;
static std::mutex client_socket_mutex_;
static int listen_socket_ = -1;
static int client_socket_ = -1;
static int listen_socket_local_ = -1;
// A pair of FD to send information to the listen thread
static int notification_listen_fd = -1;
static int notification_write_fd = -1;

void btsnoop_net_open() {
#if (BT_NET_DEBUG != TRUE)
  return;  // Disable using network sockets for security reasons
#endif

  listen_thread_valid_ =
      (pthread_create(&listen_thread_, NULL, listen_fn_, NULL) == 0);
  if (!listen_thread_valid_)
    LOG_ERROR(LOG_TAG, "%s pthread_create failed: %s", __func__,
              strerror(errno));
}

static int notify_listen_thread() {
  char buffer = '0';
  int ret = -1;

  OSI_NO_INTR(ret = write(notification_write_fd, &buffer, 1));
  if ( ret < 0){
    LOG_ERROR(LOG_TAG,
        "%s: Error in notifying the listen thread to exit",__func__);
    return -1;
  }

  return 0;
}

void btsnoop_net_close() {
#if (BT_NET_DEBUG != TRUE)
  return;  // Disable using network sockets for security reasons
#endif

  if (listen_thread_valid_) {
    notify_listen_thread();
    pthread_join(listen_thread_, NULL);
    shutdown(listen_socket_, SHUT_RDWR);
    shutdown(listen_socket_local_, SHUT_RDWR);
    safe_close_(&listen_socket_);
    safe_close_(&listen_socket_local_);
    safe_close_(&client_socket_);
    safe_close_(&notification_listen_fd);
    safe_close_(&notification_write_fd);
    LOG_WARN(LOG_TAG, "%s stopped the btsnoop listen thread", __func__);
    listen_thread_valid_ = false;
  }
}

void btsnoop_net_write(const void* data, size_t length) {
#if (BT_NET_DEBUG != TRUE)
  return;  // Disable using network sockets for security reasons
#endif

  std::lock_guard<std::mutex> lock(client_socket_mutex_);
  if (client_socket_ != -1) {
    ssize_t ret;
    OSI_NO_INTR(ret = send(client_socket_, data, length, 0));

    if (ret == -1 && errno == ECONNRESET) {
      safe_close_(&client_socket_);
    }
    else if (ret == -1 && errno == EAGAIN) {
      LOG_ERROR(LOG_TAG, "%s Dropping snoop pkts because of congestion", __func__);
    }
  }
}

static void* listen_fn_(UNUSED_ATTR void* context) {
  fd_set sock_fds;
  fd_set save_sock_fds;
  int enable = 1;
  int fd_max = -1;
  struct timeval socket_timeout;
  int self_pipe_fds[2];

  FD_ZERO(&sock_fds);
  FD_ZERO(&save_sock_fds);

  // Set up the communication channel
  if (pipe2(self_pipe_fds, O_NONBLOCK)){
    LOG_ERROR(LOG_TAG,
        "%s:Unable to establish a communication channel to the listen thread ",
        __func__);
    return NULL;
  }

  notification_listen_fd = self_pipe_fds[0];
  notification_write_fd = self_pipe_fds[1];

  FD_SET(notification_listen_fd, &save_sock_fds);
  fd_max = notification_listen_fd;

  prctl(PR_SET_NAME, (unsigned long)LISTEN_THREAD_NAME_, 0, 0, 0);

  listen_socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (listen_socket_ == -1) {
    LOG_ERROR(LOG_TAG, "%s socket creation failed: %s", __func__,
              strerror(errno));
    goto cleanup;
  }

  FD_SET(listen_socket_, &save_sock_fds);
  if(listen_socket_ > fd_max) {
    fd_max = listen_socket_;
  }

  if (setsockopt(listen_socket_, SOL_SOCKET, SO_REUSEADDR, &enable,
                 sizeof(enable)) == -1) {
    LOG_ERROR(LOG_TAG, "%s unable to set SO_REUSEADDR: %s", __func__,
              strerror(errno));
    goto cleanup;
  }

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(LOCALHOST_);
  addr.sin_port = htons(LISTEN_PORT_);
  if (bind(listen_socket_, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
    LOG_ERROR(LOG_TAG, "%s unable to bind listen socket: %s", __func__,
              strerror(errno));
    goto cleanup;
  }

  if (listen(listen_socket_, 10) == -1) {
    LOG_ERROR(LOG_TAG, "%s unable to listen: %s", __func__, strerror(errno));
    goto cleanup;
  }

  if (is_vndbtsnoop_enabled) {
    listen_socket_local_ = local_snoop_socket_create();
    if (listen_socket_local_ != -1) {
      if(listen_socket_local_ > fd_max) {
        fd_max = listen_socket_local_;
      }
      FD_SET(listen_socket_local_, &save_sock_fds);
    }
  }
  for (;;) {
  int client_socket = -1;

    sock_fds = save_sock_fds;
    if ((select(fd_max + 1, &sock_fds, NULL, NULL, NULL)) == -1) {
      LOG_ERROR(LOG_TAG, "%s select failed %s", __func__, strerror(errno));
      if(errno == EINTR)
        continue;
      goto cleanup;
    }

    if ((listen_socket_local_ != -1) && FD_ISSET(listen_socket_local_, &sock_fds)) {
      struct sockaddr_un cliaddr;
      int length;

      OSI_NO_INTR(client_socket = accept(listen_socket_local_, (struct sockaddr *)&cliaddr,
                  (socklen_t *)&length));
      if (client_socket == -1) {
        if (errno == EINVAL || errno == EBADF) {
          LOG_WARN(LOG_TAG, "%s error accepting LOCAL socket: %s", __func__, strerror(errno));
          break;
        }
        LOG_WARN(LOG_TAG, "%s error accepting LOCAL socket: %s", __func__, strerror(errno));
        continue;
      }
      OSI_NO_INTR(write(client_socket, "btsnoop\0\0\0\0\1\0\0\x3\xea", 16));
      update_snoop_fd(client_socket);
      continue;
    } else if((listen_socket_ != -1) && FD_ISSET(listen_socket_, &sock_fds)) {
      OSI_NO_INTR(client_socket = accept(listen_socket_, NULL, NULL));
      if (client_socket == -1) {
        if (errno == EINVAL || errno == EBADF) {
          break;
        }
        LOG_WARN(LOG_TAG, "%s error accepting socket: %s", __func__,
                 strerror(errno));
        continue;
      }
    } else if((notification_listen_fd != -1) && FD_ISSET(notification_listen_fd, &sock_fds)) {
      LOG_WARN(LOG_TAG, "%s exting from listen_fn_ thread ", __func__);
      return NULL;
    }

    socket_timeout.tv_sec = 0;
    socket_timeout.tv_usec = 5000;
    if(setsockopt(client_socket, SOL_SOCKET, SO_SNDTIMEO, &socket_timeout, sizeof(socket_timeout)) < 0) {
      LOG_WARN(LOG_TAG, "%s fail to set socket option %s", __func__,
               strerror(errno));
      close(client_socket);
      continue;
    }

    /* When a new client connects, we have to send the btsnoop file header. This
     * allows a decoder to treat the session as a new, valid btsnoop file. */
    std::lock_guard<std::mutex> lock(client_socket_mutex_);
    safe_close_(&client_socket_);
    client_socket_ = client_socket;

    OSI_NO_INTR(send(client_socket_, "btsnoop\0\0\0\0\1\0\0\x3\xea", 16, 0));
  }

cleanup:
  safe_close_(&listen_socket_);
  return NULL;
}

static void safe_close_(int* fd) {
  CHECK(fd != NULL);
  if (fd != NULL && *fd != -1) {
    close(*fd);
    *fd = -1;
  }
}
