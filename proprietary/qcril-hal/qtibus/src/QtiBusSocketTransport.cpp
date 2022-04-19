/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "QtiBusSocketTransport.h"
#include <QtiMutex.h>
#include <errno.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <queue>
#include <vector>
#include "framework/Util.h"

#define TAG "QtiBus"

class CommandProcessor {
 private:
  QtiBusSocketTransport::OnClientConnectedCb onClientConnected;
  QtiBusSocketTransport::OnClientDeadCb onClientDead;
  QtiBusSocketTransport::OnNewMessageCb onNewMessage;
  std::queue<Command> incoming;
  qtimutex::QtiSharedMutex mutex;
  std::condition_variable_any cv;
  std::thread th;
  bool finish = false;
  bool started = false;

  void processor() {
    setThreadName("QtiBus-PROC");
    std::unique_lock<qtimutex::QtiSharedMutex> lock(mutex);
    started = true;
    cv.notify_all();
    while (!finish) {
      std::string buffer;
      while (!incoming.empty()) {
        Command command = incoming.front();
        incoming.pop();
        DEBUG_LOG(<< "Unlocking before processing");
        lock.unlock();
        DEBUG_LOG(<< "Unlocked before processing");
        CommandId cmd = command.id;
        DEBUG_LOG(<< "Processing command " << cmd << " sender: " << command.sender
                  << " buffer.size: " << command.buffer.size());
        switch (cmd) {
          case CommandId::NEW_CLIENT:
            if (onClientConnected) {
              DEBUG_LOG(<< "Calling onClientConnected");
              onClientConnected(command.sender);
            } else {
              DEBUG_LOG(<< "onClientConnected is null");
            }
            break;
          case CommandId::NEW_MESSAGE: {
            buffer = command.buffer;
            DEBUG_LOG(<< "onNewMessage: " << (onNewMessage ? "true" : "false"));
            if (onNewMessage) {
              DEBUG_LOG(<< "Calling callback");
              onNewMessage(buffer);
            }
          } break;
          case CommandId::CLIENT_DEAD: {
            onClientDead(command.sender);
          } break;
          case CommandId::UNKNOWN:
          default:
            DEBUG_LOG(<< "Unknown CommandId");
        }
        DEBUG_LOG(<< "Locking after processing");
        lock.lock();
        DEBUG_LOG(<< "Locked after processing");
      }

      DEBUG_LOG(<< "Waiting on condvar");
      cv.wait(lock, [this]() { return finish || !incoming.empty(); });
      DEBUG_LOG(<< "Woke up");
    }
    DEBUG_LOG(<< "Finishing");
  }

 public:
  CommandProcessor(QtiBusSocketTransport::OnClientConnectedCb onClientConnected,
                   QtiBusSocketTransport::OnClientDeadCb onClientDead,
                   QtiBusSocketTransport::OnNewMessageCb onNewMessage)
      : onClientConnected(onClientConnected),
        onClientDead(onClientDead),
        onNewMessage(onNewMessage) {}
  void processCommand(CommandId cmd, int sender, std::string buffer) {
    std::unique_lock<qtimutex::QtiSharedMutex> lock(mutex);
    DEBUG_LOG(<< "command: " << cmd << " buffer.size: " << buffer.size());
    incoming.emplace(sender, cmd, buffer);
    cv.notify_all();
  }
  void stop() {
    {
      std::unique_lock<qtimutex::QtiSharedMutex> lock(mutex);
      finish = true;
      cv.notify_all();
    }
    th.join();
  }
  void start() {
    std::unique_lock<qtimutex::QtiSharedMutex> lock(mutex);
    th = std::thread(&CommandProcessor::processor, this);
    cv.wait(lock, [this]() { return started; });
    DEBUG_LOG(<< "Processor Thread started");
  }
};

int QtiBusSocketTransport::connectToServerSocket() {
  int local_fd = -1;
  int ret = 0;
  int retry_cnt = 50;
  struct sockaddr_un server;

  DEBUG_LOG(<< "socket name: " << mSockname);

  if (mSockname.empty()) {
    DEBUG_LOG(<< "Invalid socket name" << std::endl);
    return -1;
  }

  local_fd = socket(PF_UNIX, SOCK_STREAM, 0);
  if (local_fd < 0) {
    DEBUG_LOG(<< "Failed to open socket: " << mSockname << ", error: " << strerror(errno));
    return -1;
  }

  memset(&server, 0, sizeof(server));
  server.sun_family = AF_UNIX;
  snprintf(server.sun_path, sizeof(server.sun_path), "%s", mSockname.c_str());

  int i = 0;
  while (1) {
    ret = connect(local_fd, (struct sockaddr *)&server, sizeof(server));
    if (ret == 0) {
      DEBUG_LOG(<< "Connected to server: " << mSockname << " socket id: " << local_fd);
      break;
    }

    if (i >= retry_cnt) {
      close(local_fd);
      local_fd = -1;
      break;
    }
    i++;
    DEBUG_LOG(<< "retrying connection to socket ipc - " << i);
    sleep(2);
  }
  return local_fd;
}

void QtiBusSocketTransport::serverDied() {
  DEBUG_LOG(<< "server socket closed");

  // close socket and retry to connect
  close(mClientFd);
  mClientFd = -1;

  // This sleep is required to make sure that clean up is done.
  // If we try to connect without this sleep, might get stale server connection.
  sleep(1);
  // reconnect to server socket
  mClientFd = connectToServerSocket();
  DEBUG_LOG(<< "mClientFd: " << mClientFd);
  if (mClientFd == -1) {
    abort();
  }
}

ssize_t QtiBusSocketTransport::receiveMessage(char *recv_buf,
                                              size_t recv_buf_size) {
  memset(recv_buf, 0, recv_buf_size);
  ssize_t message_size = -1;
  ssize_t recv_len = 0;
  unsigned int bytes_remaining = sizeof(size_t);
  char message_size_buf[sizeof(size_t)] = {};
  char *message_size_buf_ptr = message_size_buf;

  // Data will be received in this format
  // Field |   Parameter    |             Size             |
  // ------+----------------+------------------------------|
  //   1   | overall_size   | sizeof(size_t)               |
  //   2   | actual_payload | overall_size-sizeof(size_t)  |

  // Read the overall_size
  do {
    recv_len = recv(mClientFd, message_size_buf_ptr, bytes_remaining, 0);
    if (recv_len < 1) {
      break;
    }
    message_size_buf_ptr += recv_len;
    bytes_remaining -= recv_len;
  } while (bytes_remaining > 0);
  DEBUG_LOG(<< "recv_len = " << recv_len);

  // Read the actual_payload
  if (recv_len > 0) {
    // message_size = *(static_cast<size_t *>(message_size_buf));
    message_size = *(size_t *)(message_size_buf);
    DEBUG_LOG(<< "message_size = " << message_size);
    // Reduce the 'overall_size' length
    message_size = message_size - sizeof(size_t);
    if (message_size > recv_buf_size) {
      // the message received exceeds the maximum allowed
      // message size, which is a serious issue, so abort.
      DEBUG_LOG(<< "message size exceeds " << recv_buf_size << " bytes. ABORTING...");
      abort();
    }

    bytes_remaining = message_size;
    do {
      recv_len = recv(mClientFd, recv_buf, bytes_remaining, 0);
      if (recv_len < 1) {
        // Receive failure; return msg size as 0
        message_size = -1;
        break;
      }
      recv_buf += recv_len;
      bytes_remaining -= recv_len;
    } while (bytes_remaining > 0);
  }
  return message_size;
}

void QtiBusSocketTransport::processMessage(char *message, ssize_t message_len) {
  std::stringstream is(std::string(message, message_len));

  CommandId cmd = CommandId::UNKNOWN;
  pid_t sender = 0;
  size_t payloadSize = 0;
  size_t header_size = sizeof(cmd) + sizeof(sender);

  if (message_len < header_size) {
    // message size is less than the required header size
    DEBUG_LOG(<< "1. Message is of size " << message_len
              << " bytes. Should be at least " << header_size << " bytes.");
    return;
  }

  // Read Command Id
  is.read((char *)&cmd, sizeof(cmd));
  if (!is.good()) {
    DEBUG_LOG(<< "Error reading message stream. eof: " << is.eof()
              << ", fail: " << is.fail() << ", bad: " << is.bad() << ".");
    return;
  }
  DEBUG_LOG(<< "cmd: " << cmd);

  if ((cmd == CommandId::NEW_CLIENT) || (cmd == CommandId::NEW_MESSAGE) ||
      (cmd == CommandId::CLIENT_DEAD)) {
    // Read sender
    is.read((char *)&sender, sizeof(sender));
    if (!is.good()) {
      DEBUG_LOG(<< "Error reading message stream. eof: " << is.eof()
                << ", fail: " << is.fail() << ", bad: " << is.bad() << ".");
      return;
    }
    DEBUG_LOG(<< "sender: " << sender);

    // Read payload
    std::string payload;
    if (cmd == CommandId::NEW_MESSAGE) {

      header_size += sizeof(payloadSize);
      if (message_len < header_size) {
        // message size is less than the required header size
        DEBUG_LOG(<< "2. Message is of size " << message_len
            << " bytes. Should be at least " << header_size << " bytes.");
        return;
      }
      is.read((char *)&payloadSize, sizeof(payloadSize));
      if (!is.good()) {
        DEBUG_LOG(<< "Error reading message stream. eof: " << is.eof()
                  << ", fail: " << is.fail() << ", bad: " << is.bad() << ".");
        return;
      }
      DEBUG_LOG(<< "payloadSize: " << payloadSize);
      if ((message_len - header_size) != payloadSize) {
        DEBUG_LOG(<< "3. Payload size is " << payloadSize
                  << " bytes. Expected payload size is "
                  << (message_len - header_size) << " bytes.");
        return;
      }
      is.width(payloadSize);
      is >> payload;
      if (!is.good()) {
        DEBUG_LOG(<< "Error reading message stream. eof: " << is.eof()
                  << ", fail: " << is.fail() << ", bad: " << is.bad() << ".");
        return;
      }
      DEBUG_LOG(<< "payload: " << stringToHex(payload));
    }
    mProcessor->processCommand(cmd, sender, payload);
  } else {
    DEBUG_LOG(<< "Invalid cmd");
  }
}

void QtiBusSocketTransport::clientLoop() {
  char *recv_buffer = new char[MAX_BUFFER_SIZE];
  ssize_t message_size = 0;

  if (!recv_buffer) {
    DEBUG_LOG(<< "Failed to allocate recv_buffer; abort" << std::endl);
    abort();
  }

  mClientFd = connectToServerSocket();
  DEBUG_LOG(<< "mClientFd: " << mClientFd);
  if (mClientFd < 0) {
    DEBUG_LOG(<< "Failed to connect to server socket; abort" << std::endl);
    abort();
  }

  while (1) {
    DEBUG_LOG(<< "waiting for data");

    message_size = receiveMessage(recv_buffer, MAX_BUFFER_SIZE);
    DEBUG_LOG(<< "message_size: " << message_size);

    if (message_size == -1) {
      serverDied();
    } else {
      processMessage(recv_buffer, message_size);
    }
  }
}

void QtiBusSocketTransport::monitorCmd(SetStartCb setStarted) {
    setThreadName("QtiBus-MON");
    mProcessor = std::make_shared<CommandProcessor>(onClientConnected, onClientDead, onNewMessage);
    if (mProcessor == nullptr) {
      abort();
    }
    mProcessor->start();

    setStarted(true);

    clientLoop();
}

void QtiBusSocketTransport::openTransport(const char * /*sockname*/) {
  DEBUG_LOG(<< "Starting monitorCmd thread");
  mCommandReceiver = std::thread{&QtiBusSocketTransport::monitorCmd, this,
                                 [this](bool started) { setStarted(started); }};

  waitUntilStarted();
  return;
}

bool QtiBusSocketTransport::write(const std::string &buffer) {
  int rc = -1;
  bool ret = false;

  DEBUG_LOG(<< "> " << __func__);

  DEBUG_LOG(<< "mClientFd = " << mClientFd << " Send Buffer: " << stringToHex(buffer));
  if (mClientFd > 0) {
    rc = send(mClientFd, buffer.c_str(), buffer.size(), 0);
    DEBUG_LOG(<< "rc = " << rc << std::endl);
  }

  DEBUG_LOG(<< "< " << __func__);
  return ret;
}

QtiBusSocketTransport::QtiBusSocketTransport(std::string sockname)
    : mClientFd(-1), mSockname(sockname) {}

void QtiBusSocketTransport::start() {
  openTransport(mSockname.c_str());
}

void QtiBusSocketTransport::stop() {
    if (getStarted()) {
        mCommandReceiver.join();
    }
}

std::string QtiBusSocketTransport::receive() {
  std::string ret;
  return ret;
}

std::string stringToHex(const std::string& in) {
  std::string out;
  static const char* const hexRef = "0123456789ABCDEF";
  size_t len = in.length();
  out.reserve(2 * len);
  for (size_t i = 0; i < len; ++i) {
    const unsigned char c = in[i];
    out.push_back(hexRef[c >> 4]);
    out.push_back(hexRef[c & 15]);
  }
  return out;
}
