/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "QtiBusSocketTransportServer.h"
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <functional>
#include <string>
#include <thread>
#include "QtiBusSocketTransport.h"

#include "framework/Util.h"

#define TAG "QtiBus"

#define MAX_EPOLL_EVENTS 64

QtiBusSocketTransportServer::QtiBusSocketTransportServer(std::string sockname)
    : mSocketName(sockname) {}

void QtiBusSocketTransportServer::start() {
  if (!getStarted()) {
    DEBUG_LOG(<< "Starting serverLoop thread");
    mServerThread = std::thread{&QtiBusSocketTransportServer::serverLoop, this,
                                [this](bool started) { setStarted(started); }};
  } else {
    DEBUG_LOG(<< "serverLoop thread is already started");
  }
}

void QtiBusSocketTransportServer::stop() {
  if (getStarted()) {
    mServerThread.join();
  }
}

static pid_t getSenderPid(int fd) {
  pid_t pid = 0;
  int r;
  int val = 1;
  struct ucred ucred = {};

  do {
    if (fd == -1) {
      break;
    }

    // Use this option to get pid from other end of the socket
    r = setsockopt(fd, SOL_SOCKET, SO_PASSCRED, &val, sizeof(val));
    if (r < 0) {
      DEBUG_LOG(<< "error setting sock opts");
      break;
    }

    socklen_t len = sizeof(struct ucred);
    if (getsockopt(fd, SOL_SOCKET, SO_PEERCRED, &ucred, &len) < 0) {
      DEBUG_LOG(<< "failed to get credentials:" << strerror(errno));
      break;
    }
    pid = ucred.pid;
    // DEBUG_LOG(<< "pid of peer socket fd: " << fd << " is process:" << pid);
  } while (0);

  return pid;
}

void QtiBusSocketTransportServer::addClient(int fd) {
  mClientFdList.push_back(fd);

  // Broadcast NEW_CIENT indication to all the other clients
  broadcastMessage(fd, CommandId::NEW_CLIENT, std::string(""));

  // NEW_CLIENT indication to this FD to make it aware of the already connected
  // clients on the server.
  for (auto const &entry : mClientFdList) {
    if (entry != fd) {
      broadcastMessage(entry, CommandId::NEW_CLIENT, std::string(""), fd);
    }
  }
}

void QtiBusSocketTransportServer::removeClient(int fd) {
  DEBUG_LOG(<< "Client : (fd: " << fd << " pid: " << getSenderPid(fd) << ") closed connection");
  // Clear fd from the list of client fds
  mClientFdList.erase(std::remove(mClientFdList.begin(), mClientFdList.end(), fd),
                      mClientFdList.end());

  // Client closed connection, send CLIENT_DEAD
  broadcastMessage(fd, CommandId::CLIENT_DEAD, std::string(""));

  close(fd);
}

void QtiBusSocketTransportServer::cleanUpClientFds() {
  // Close all fds, clear vector for client fds
  for (auto const &entry : mClientFdList) {
    close(entry);
  }
  mClientFdList.clear();
}


void QtiBusSocketTransportServer::broadcastMessage(int sender, CommandId cmd, std::string payload,
                                                   int receiver) {
  std::stringstream os;
  size_t overall_size = 0;

  // Write the overall size.
  // Come back to it later, when we have completed writing
  // and have the overall size.
  std::stringstream::pos_type pos = os.tellp();
  os.write((char *)&overall_size, sizeof(overall_size));

  // Write the CommandId
  os.write((char *)&cmd, sizeof(cmd));
  DEBUG_LOG(<< "CommandId: " << cmd);

  // Write the sender
  pid_t pid = getSenderPid(sender);
  os.write((char *)&pid, sizeof(pid));
  DEBUG_LOG(<< "Sender: " << pid);

  // Write the payload
  if (payload.size() > 0) {
    size_t payload_size = payload.size();
    os.write((char *)&payload_size, sizeof(payload_size));
    DEBUG_LOG(<< "payload_size: " << payload_size);

    os.width(payload_size);
    os << std::noskipws << payload;
  }

  // Write the overall size
  os.seekp(0, std::ios_base::seekdir::end);
  overall_size = os.tellp() - pos;

  os.seekp(pos);
  os.write((char *)&overall_size, sizeof(overall_size));
  os.seekp(pos);

  DEBUG_LOG(<< "overall_size: " << overall_size);

  std::string buf = os.str();

  DEBUG_LOG(<< "Send Buffer: " << stringToHex(buf));

  int rc = -1;
  if (receiver != -1) {
    DEBUG_LOG(<< "sending to client: (fd: " << receiver << " pid: " << pid << ")");
    rc = send(receiver, buf.c_str(), buf.size(), 0);
  } else {
    // Send this to all other clients
    for (auto const &entry : mClientFdList) {
      if (entry != sender) {
        DEBUG_LOG(<< "sending to client: (fd:" << entry << " pid: " << getSenderPid(entry) << ")");
        rc = send(entry, buf.c_str(), buf.size(), 0);
      }
    }
  }
}

void QtiBusSocketTransportServer::serverLoop(SetStartCb setStarted) {
  setThreadName("QtiBus-SRV");
  setStarted(true);

  int sock_fd = -1, client_fd = -1, epfd = -1, ready_cnt = 0;
  struct sockaddr_un addr;
  int ret = -1;
  char recv_buf[MAX_BUFFER_SIZE] = {};
  ssize_t recv_buf_len = 0;
  struct epoll_event event;
  struct epoll_event events[MAX_EPOLL_EVENTS] = {};

  DEBUG_LOG(<< "socket name: " << mSocketName);
  if (mSocketName.empty()) {
    DEBUG_LOG(<< "Invalid socket name");
    return;
  }

  sock_fd = socket(PF_UNIX, SOCK_STREAM, 0);
  if (sock_fd < 0) {
    DEBUG_LOG(<< "Failed to open socket" << mSocketName << "error = " << strerror(errno));
    return;
  }

  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", mSocketName.c_str());

  ret = unlink(addr.sun_path);
  DEBUG_LOG(<< "unlink ret: " << ret << " errno: " << strerror(errno));
  if (ret != 0 && errno != ENOENT) {
    DEBUG_LOG(<< "Failed to unlink " << mSocketName);
  }

  ret = ::bind(sock_fd, (struct sockaddr *)&addr, sizeof(addr));
  if (ret < 0) {
    DEBUG_LOG(<< "Failed to bind socket" << mSocketName << " error: " << strerror(errno));
    close(sock_fd);
    sock_fd = -1;
    return;
  }

  // To avoid others accidentally deleting this socket file.
  chmod(addr.sun_path, (mode_t)0660);
  DEBUG_LOG(<< "Created socket success with " << addr.sun_path << " sockfd = " << sock_fd);

  listen(sock_fd, 8);

  epfd = epoll_create(1);
  event.events = EPOLLIN;
  event.data.fd = sock_fd;
  epoll_ctl(epfd, EPOLL_CTL_ADD, sock_fd, &event);

  while (1) {
    DEBUG_LOG(<< "polling..");
    ready_cnt = epoll_wait(epfd, events, MAX_EPOLL_EVENTS, -1 /*forever*/);
    DEBUG_LOG(<< "epoll_wait ready_cnt = " << ready_cnt);

    // Other than EINTR, other failures are FATAL
    if (ready_cnt < 0 && (errno != EINTR)) {
      DEBUG_LOG(<< "epoll_wait failed");
      break;
    }

    // Log and ignore select exit due to time out.
    if (ready_cnt == 0) {
      DEBUG_LOG(<< "epoll_wait timed out, unexpected");
    }

    for (int i = 0; i < ready_cnt; i++) {
      if (events[i].events & EPOLLIN) {
        DEBUG_LOG(<< "Received input event on fd: " << events[i].data.fd);

        // New connection
        if (events[i].data.fd == sock_fd) {
          client_fd = accept(sock_fd, NULL, NULL);
          if (client_fd < 0) {
            // If error is EWOULDBLOCK, EAGAIN ignore.
            if (errno != EWOULDBLOCK || errno != EAGAIN) {
              DEBUG_LOG(<< "accept() failed");
            }
          } else {
            // New Client connected.
            // 1) Add new client fd to vector and send new client command
            DEBUG_LOG(<< "New Client Connected: " << client_fd);
            addClient(client_fd);

            // 2) Add new client fd to epoll list for polling.
            event.events = EPOLLIN;
            event.data.fd = client_fd;
            DEBUG_LOG(<< "adding to the epoll list: " << client_fd);
            if (epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &event) < 0) {
              DEBUG_LOG(<< "epoll_ctl for add failure: " << strerror(errno));
            }
          }
        } else {
          recv_buf_len = recv(events[i].data.fd, recv_buf, sizeof(recv_buf), 0);
          if (recv_buf_len < 0) {
            if (errno != EWOULDBLOCK || errno != EAGAIN) {
              DEBUG_LOG(<< "recv() failed");
            }
          } else if (recv_buf_len == 0) {
            removeClient(events[i].data.fd);
            // Remove fd from the list of polling fd list
            if (epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, NULL) < 0) {
              DEBUG_LOG(<< "epoll_ctl for delete failure: " << strerror(errno));
            }
          } else {
            DEBUG_LOG(<< "received bytes: " << recv_buf_len
                      << " from client fd: " << events[i].data.fd);
            broadcastMessage(events[i].data.fd, CommandId::NEW_MESSAGE,
                             std::string(recv_buf, recv_buf_len));
          }
        }
      }
    }
  }

  setStarted(false);
  cleanUpClientFds();
  close(sock_fd);
}
