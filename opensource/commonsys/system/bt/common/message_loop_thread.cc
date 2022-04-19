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

#include <sys/syscall.h>
#include <unistd.h>
#include <thread>

#include <base/strings/stringprintf.h>

#include "message_loop_thread.h"

namespace bluetooth {

namespace common {

static constexpr int kRealTimeFifoSchedulingPriority = 1;

MessageLoopThread::MessageLoopThread(const std::string& thread_name)
    : thread_name_(thread_name),
      message_loop_(nullptr),
      run_loop_(nullptr),
      thread_(nullptr),
      thread_id_(-1),
      linux_tid_(-1) {}

MessageLoopThread::~MessageLoopThread() {
  std::lock_guard<std::recursive_mutex> api_lock(api_mutex_);
  if (thread_ != nullptr) {
    ShutDown();
  }
}

void MessageLoopThread::StartUp() {
  std::lock_guard<std::recursive_mutex> api_lock(api_mutex_);
  if (thread_ != nullptr) {
    LOG(WARNING) << __func__ << ": thread " << *this << " is already started";
    return;
  }
  std::shared_ptr<ExecutionBarrier> start_up_barrier =
      std::make_shared<ExecutionBarrier>();
  thread_ =
      new std::thread(&MessageLoopThread::RunThread, this, start_up_barrier);
  start_up_barrier->WaitForExecution();
}

bool MessageLoopThread::DoInThread(const tracked_objects::Location& from_here,
                                   base::OnceClosure task) {
  std::lock_guard<std::recursive_mutex> api_lock(api_mutex_);
  if (message_loop_ == nullptr) {
    LOG(ERROR) << __func__ << ": message loop is null for thread " << *this
               << ", from " << from_here.ToString();
    return false;
  }
  if (!message_loop_->task_runner()->PostTask(from_here, std::move(task))) {
    LOG(ERROR) << __func__
               << ": failed to post task to message loop for thread " << *this
               << ", from " << from_here.ToString();
    return false;
  }
  return true;
}

void MessageLoopThread::ShutDown() {
  std::lock_guard<std::recursive_mutex> api_lock(api_mutex_);
  if (thread_ == nullptr) {
    LOG(WARNING) << __func__ << ": thread " << *this << " is already stopped";
    return;
  }
  CHECK_NE(thread_id_, base::PlatformThread::CurrentId())
      << __func__ << " should not be called on the thread itself. "
      << "Otherwise, deadlock may happen.";
  if (message_loop_ != nullptr) {
    if (!message_loop_->task_runner()->PostTask(
            FROM_HERE, run_loop_->QuitWhenIdleClosure())) {
      LOG(FATAL) << __func__
                 << ": failed to post task to message loop for thread "
                 << *this;
    }
  }
  thread_->join();
  delete thread_;
  thread_ = nullptr;
}

base::PlatformThreadId MessageLoopThread::GetThreadId() const {
  std::lock_guard<std::recursive_mutex> api_lock(api_mutex_);
  return thread_id_;
}

std::string MessageLoopThread::GetName() const {
  std::lock_guard<std::recursive_mutex> api_lock(api_mutex_);
  return thread_name_;
}

std::string MessageLoopThread::ToString() const {
  std::lock_guard<std::recursive_mutex> api_lock(api_mutex_);
  return base::StringPrintf("%s(%d)", thread_name_.c_str(), thread_id_);
}

bool MessageLoopThread::IsRunning() const {
  std::lock_guard<std::recursive_mutex> api_lock(api_mutex_);
  return message_loop_ != nullptr;
}

// Non API method, should not be protected by API mutex
void MessageLoopThread::RunThread(
    MessageLoopThread* thread,
    std::shared_ptr<ExecutionBarrier> start_up_barrier) {
  thread->Run(std::move(start_up_barrier));
}

base::MessageLoop* MessageLoopThread::message_loop() const {
  std::lock_guard<std::recursive_mutex> api_lock(api_mutex_);
  return message_loop_;
}

bool MessageLoopThread::EnableRealTimeScheduling() {
  std::lock_guard<std::recursive_mutex> api_lock(api_mutex_);
  if (!IsRunning()) {
    LOG(ERROR) << __func__ << ": thread " << *this << " is not running";
    return false;
  }
  struct sched_param rt_params = {.sched_priority =
                                      kRealTimeFifoSchedulingPriority};
  int rc = sched_setscheduler(linux_tid_, SCHED_FIFO, &rt_params);
  if (rc != 0) {
    LOG(ERROR) << __func__ << ": unable to set SCHED_FIFO priority "
               << kRealTimeFifoSchedulingPriority << " for linux_tid "
               << std::to_string(linux_tid_) << ", thread " << *this
               << ", error: " << strerror(errno);
    return false;
  }
  return true;
}

// Non API method, should NOT be protected by API mutex to avoid deadlock
void MessageLoopThread::Run(
    std::shared_ptr<ExecutionBarrier> start_up_barrier) {
  LOG(INFO) << __func__ << ": message loop starting for thread "
            << thread_name_;
  base::PlatformThread::SetName(thread_name_);
  message_loop_ = new base::MessageLoop();
  run_loop_ = new base::RunLoop();
  thread_id_ = base::PlatformThread::CurrentId();
  linux_tid_ = static_cast<pid_t>(syscall(SYS_gettid));
  start_up_barrier->NotifyFinished();
  // Blocking until ShutDown() is called
  run_loop_->Run();
  thread_id_ = -1;
  linux_tid_ = -1;
  delete message_loop_;
  message_loop_ = nullptr;
  delete run_loop_;
  run_loop_ = nullptr;
  LOG(INFO) << __func__ << ": message loop finished for thread "
            << thread_name_;
}

}  // namespace common

}  // namespace bluetooth
