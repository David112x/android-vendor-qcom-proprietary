/*
 * Copyright 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <condition_variable>
#include <memory>
#include <mutex>

#include <base/macros.h>

namespace bluetooth {

namespace common {

/**
 * A utility to wait for an event on another thread
 *
 * This class can be used once only. This means that after the first time
 * NotifyFinished() is called, WaitForExecution() will no longer block. User
 * needs to create a new instance if another ExecutionBarrier is needed.
 *
 * No reset mechanism is provided for this class to avoid racy scenarios and
 * unsafe API usage
 *
 * Similar to std::experimental::barrier, but this can be used only once
 */
class ExecutionBarrier final {
 public:
  explicit ExecutionBarrier() : finished_(false){};

  /**
   * Blocks until NotifyFinished() is called on this object
   *
   */
  void WaitForExecution();

  /**
   * Unblocks any caller who are blocked on WaitForExecution() method call
   */
  void NotifyFinished();

 private:
  bool finished_;
  std::mutex execution_mutex_;
  std::condition_variable execution_cv_;

  /**
   * Prevent COPY and ASSIGN since many internal states cannot be copied or
   * assigned
   */
  DISALLOW_COPY_AND_ASSIGN(ExecutionBarrier);
};

}  // namespace common

}  // namespace bluetooth