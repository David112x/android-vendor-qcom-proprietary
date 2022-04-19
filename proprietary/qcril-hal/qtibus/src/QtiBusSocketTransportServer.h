/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#pragma once

#include <qtibus/QtiBusTransportServer.h>
#include <functional>
#include <string>
#include <thread>
#include <vector>
#include "QtiBusSocketTransport.h"
#include "QtiBusTransport.h"

class QtiBusSocketTransportServer : public QtiBusTransportServer {
 private:
  std::vector<int> mClientFdList;
  std::string mSocketName;
  std::thread mServerThread;
  using SetStartCb = std::function<void(bool)>;
  void serverLoop(SetStartCb setStarted);

  void addClient(int fd);
  void removeClient(int fd);
  void cleanUpClientFds();
  void broadcastMessage(int sender, CommandId cmd, std::string payload, int receiver = -1);

 public:
  QtiBusSocketTransportServer(std::string sockname);
  void start();
  void stop();
};
