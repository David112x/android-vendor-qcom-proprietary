/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#pragma once

#include <functional>
#include <string>
#include <thread>
#include <vector>

class QtiBusTransportServer {
 private:
  bool mStarted = false;

 public:
  QtiBusTransportServer() {}
  virtual ~QtiBusTransportServer() {}
  bool getStarted() { return mStarted; }
  void setStarted(bool started) { mStarted = started; }
  virtual void start() = 0;
  virtual void stop() = 0;
  static QtiBusTransportServer &get();
};
