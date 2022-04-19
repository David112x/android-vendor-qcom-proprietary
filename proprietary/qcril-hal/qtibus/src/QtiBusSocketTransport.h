/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#pragma once

#include "QtiBusTransport.h"
#include <framework/message_id.h>
#include <functional>
#include <string>
#include <thread>

#ifdef __ANDROID__
#define IPC_SOCKET_NAME "/dev/socket/qmux_radio/ril_ipc"
#else
#define IPC_SOCKET_NAME "ril_ipc"
#endif

#define MAX_BUFFER_SIZE 1024

enum class CommandId {
  UNKNOWN,
  NEW_CLIENT,
  NEW_MESSAGE,
  CLIENT_DEAD,
};

static inline std::ostream & operator <<(std::ostream& stream, CommandId id) {
  std::string name = "<<UNKNOWN>>";
  switch (id) {
  case CommandId::NEW_CLIENT:
    name = "NEW_CLIENT";
    break;
  case CommandId::NEW_MESSAGE:
    name = "NEW_MESSAGE";
    break;
  case CommandId::CLIENT_DEAD:
    name = "CLIENT_DEAD";
    break;
  case CommandId::UNKNOWN:
  default:
    name = "UNKNOWN";
    break;
  }
  return stream << std::string("CommandId::") << name;
}

struct Command {
  pid_t sender;
  CommandId id;
  std::string buffer;
  Command() {}
  Command(pid_t sender, CommandId id, std::string buffer)
      : sender(sender), id(id), buffer(buffer) {}
};

class CommandProcessor;

class QtiBusSocketTransport : public QtiBusTransport {
 private:
  int mClientFd;
  std::vector<int> mClientFdList;
  std::string mSockname;
  std::thread mCommandReceiver;
  std::shared_ptr<CommandProcessor> mProcessor;
  qtimutex::QtiSharedMutex mutex;
  void openTransport(const char *sockname);
  int connectToServerSocket();
  void serverDied();
  void processMessage(char *message, ssize_t message_len);
  ssize_t receiveMessage(char *recv_buf, size_t recv_buf_len);
  void clientLoop();
  using SetStartCb = std::function<void(bool)>;
  void monitorCmd(SetStartCb setStarted);

 public:
  QtiBusSocketTransport(std::string sockname);
  bool write(const std::string &buffer);
  std::string receive();
  void start();
  void stop();

  template <class T>
  void debugHook(T &);
};

std::string stringToHex(const std::string& in);
