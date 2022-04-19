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

#include <condition_variable>
#include <memory>
#include <mutex>

#include <gtest/gtest.h>

#include <base/bind.h>
#include <base/threading/platform_thread.h>
#include <base/tracked_objects.h>
#include <sys/capability.h>
#include <syscall.h>

#include "execution_barrier.h"
#include "message_loop_thread.h"

using bluetooth::common::ExecutionBarrier;
using bluetooth::common::MessageLoopThread;

/**
 * Unit tests to verify MessageLoopThread. Must have CAP_SYS_NICE capability.
 */
class MessageLoopThreadTest : public ::testing::Test {
 public:
  void ShouldNotHappen() { FAIL() << "Should not happen"; }

  void GetThreadId(base::PlatformThreadId* thread_id,
                   std::shared_ptr<ExecutionBarrier> execution_barrier) {
    *thread_id = base::PlatformThread::CurrentId();
    execution_barrier->NotifyFinished();
  }

  void GetLinuxTid(pid_t* tid,
                   std::shared_ptr<ExecutionBarrier> execution_barrier) {
    *tid = static_cast<pid_t>(syscall(SYS_gettid));
    execution_barrier->NotifyFinished();
  }

  void GetName(std::string* name,
               std::shared_ptr<ExecutionBarrier> execution_barrier) {
    char my_name[256];
    pthread_getname_np(pthread_self(), my_name, sizeof(my_name));
    name->append(my_name);
    execution_barrier->NotifyFinished();
  }

  void GetSchedulingPolicyAndPriority(
      int* scheduling_policy, int* schedule_priority,
      std::shared_ptr<ExecutionBarrier> execution_barrier) {
    *scheduling_policy = sched_getscheduler(0);
    struct sched_param param = {};
    ASSERT_EQ(sched_getparam(0, &param), 0);
    *schedule_priority = param.sched_priority;
    execution_barrier->NotifyFinished();
  }

 protected:
  static bool CanSetCurrentThreadPriority() {
    struct __user_cap_header_struct linux_user_header = {
        .version = _LINUX_CAPABILITY_VERSION_3};
    struct __user_cap_data_struct linux_user_data = {};
    if (capget(&linux_user_header, &linux_user_data) != 0) {
      LOG(ERROR) << "Failed to get capability for current thread, error: "
                 << strerror(errno);
      // Log record in XML
      RecordProperty("MessageLoopThreadTestCannotGetCapabilityReason",
                     strerror(errno));
      return false;
    }
    return ((linux_user_data.permitted >> CAP_SYS_NICE) & 0x1) != 0;
  }
};

TEST_F(MessageLoopThreadTest, test_running_thread) {
  MessageLoopThread message_loop_thread("test_thread");
  message_loop_thread.StartUp();
  ASSERT_GE(message_loop_thread.GetThreadId(), 0);
  ASSERT_TRUE(message_loop_thread.IsRunning());
  message_loop_thread.ShutDown();
  ASSERT_LT(message_loop_thread.GetThreadId(), 0);
  ASSERT_FALSE(message_loop_thread.IsRunning());
}

TEST_F(MessageLoopThreadTest, test_not_self) {
  MessageLoopThread message_loop_thread("test_thread");
  message_loop_thread.StartUp();
  ASSERT_GE(message_loop_thread.GetThreadId(), 0);
  ASSERT_NE(message_loop_thread.GetThreadId(),
            base::PlatformThread::CurrentId());
}

TEST_F(MessageLoopThreadTest, test_shutdown_without_start) {
  MessageLoopThread message_loop_thread("test_thread");
  message_loop_thread.ShutDown();
  ASSERT_LT(message_loop_thread.GetThreadId(), 0);
}

TEST_F(MessageLoopThreadTest, test_do_in_thread_before_start) {
  std::string name = "test_thread";
  MessageLoopThread message_loop_thread(name);
  ASSERT_FALSE(message_loop_thread.DoInThread(
      FROM_HERE, base::Bind(&MessageLoopThreadTest::ShouldNotHappen,
                            base::Unretained(this))));
}

TEST_F(MessageLoopThreadTest, test_do_in_thread_after_shutdown) {
  std::string name = "test_thread";
  MessageLoopThread message_loop_thread(name);
  message_loop_thread.StartUp();
  message_loop_thread.ShutDown();
  ASSERT_FALSE(message_loop_thread.DoInThread(
      FROM_HERE, base::Bind(&MessageLoopThreadTest::ShouldNotHappen,
                            base::Unretained(this))));
}

TEST_F(MessageLoopThreadTest, test_name) {
  std::string name = "test_thread";
  MessageLoopThread message_loop_thread(name);
  message_loop_thread.StartUp();
  ASSERT_GE(message_loop_thread.GetThreadId(), 0);
  std::shared_ptr<ExecutionBarrier> execution_barrier =
      std::make_shared<ExecutionBarrier>();
  std::string myName;
  message_loop_thread.DoInThread(
      FROM_HERE,
      base::Bind(&MessageLoopThreadTest::GetName, base::Unretained(this),
                 &myName, execution_barrier));
  execution_barrier->WaitForExecution();
  ASSERT_EQ(name, myName);
  ASSERT_EQ(name, message_loop_thread.GetName());
}

TEST_F(MessageLoopThreadTest, test_thread_id) {
  std::string name = "test_thread";
  MessageLoopThread message_loop_thread(name);
  message_loop_thread.StartUp();
  base::PlatformThreadId thread_id = message_loop_thread.GetThreadId();
  ASSERT_GE(thread_id, 0);
  std::shared_ptr<ExecutionBarrier> execution_barrier =
      std::make_shared<ExecutionBarrier>();
  base::PlatformThreadId my_thread_id;
  message_loop_thread.DoInThread(
      FROM_HERE,
      base::Bind(&MessageLoopThreadTest::GetThreadId, base::Unretained(this),
                 &my_thread_id, execution_barrier));
  execution_barrier->WaitForExecution();
  ASSERT_EQ(thread_id, my_thread_id);
}

TEST_F(MessageLoopThreadTest, test_set_realtime_priority_fail_before_start) {
  std::string name = "test_thread";
  MessageLoopThread message_loop_thread(name);
  ASSERT_FALSE(message_loop_thread.EnableRealTimeScheduling());
}

TEST_F(MessageLoopThreadTest, test_set_realtime_priority_success) {
  std::string name = "test_thread";
  MessageLoopThread message_loop_thread(name);
  message_loop_thread.StartUp();
  bool ret = message_loop_thread.EnableRealTimeScheduling();
  if (!ret) {
    if (CanSetCurrentThreadPriority()) {
      FAIL() << "Cannot set real time priority even though we have permission";
    } else {
      LOG(WARNING) << "Allowing EnableRealTimeScheduling to fail because we"
                      " don't have CAP_SYS_NICE capability";
      // Log record in XML
      RecordProperty("MessageLoopThreadTestConditionalSuccess",
                     "Mark test as success even though EnableRealTimeScheduling"
                     " failed because we don't have CAP_SYS_NICE capability");
      // Quit early since further verification is no longer needed
      return;
    }
  }
  std::shared_ptr<ExecutionBarrier> execution_barrier =
      std::make_shared<ExecutionBarrier>();
  int scheduling_policy = -1;
  int scheduling_priority = -1;
  message_loop_thread.DoInThread(
      FROM_HERE,
      base::Bind(&MessageLoopThreadTest::GetSchedulingPolicyAndPriority,
                 base::Unretained(this), &scheduling_policy,
                 &scheduling_priority, execution_barrier));
  execution_barrier->WaitForExecution();
  ASSERT_EQ(scheduling_policy, SCHED_FIFO);
  // Internal implementation verified here
  ASSERT_EQ(scheduling_priority, 1);
  execution_barrier = std::make_shared<ExecutionBarrier>();
  pid_t linux_tid = -1;
  message_loop_thread.DoInThread(
      FROM_HERE,
      base::Bind(&MessageLoopThreadTest::GetLinuxTid, base::Unretained(this),
                 &linux_tid, execution_barrier));
  execution_barrier->WaitForExecution();
  ASSERT_GT(linux_tid, 0);
  ASSERT_EQ(sched_getscheduler(linux_tid), SCHED_FIFO);
  struct sched_param param = {};
  ASSERT_EQ(sched_getparam(linux_tid, &param), 0);
  // Internal implementation verified here
  ASSERT_EQ(param.sched_priority, 1);
}

TEST_F(MessageLoopThreadTest, test_message_loop_null_before_start) {
  std::string name = "test_thread";
  MessageLoopThread message_loop_thread(name);
  ASSERT_EQ(message_loop_thread.message_loop(), nullptr);
}

TEST_F(MessageLoopThreadTest, test_message_loop_not_null_start) {
  std::string name = "test_thread";
  MessageLoopThread message_loop_thread(name);
  message_loop_thread.StartUp();
  ASSERT_NE(message_loop_thread.message_loop(), nullptr);
}

TEST_F(MessageLoopThreadTest, test_message_loop_null_after_stop) {
  std::string name = "test_thread";
  MessageLoopThread message_loop_thread(name);
  message_loop_thread.StartUp();
  ASSERT_NE(message_loop_thread.message_loop(), nullptr);
  message_loop_thread.ShutDown();
  ASSERT_EQ(message_loop_thread.message_loop(), nullptr);
}

TEST_F(MessageLoopThreadTest, test_to_string_method) {
  std::string name = "test_thread";
  MessageLoopThread message_loop_thread(name);
  std::string thread_string_before_start = message_loop_thread.ToString();
  ASSERT_FALSE(thread_string_before_start.empty());
  LOG(INFO) << "Before start: " << message_loop_thread;
  message_loop_thread.StartUp();
  std::string thread_string_running = message_loop_thread.ToString();
  ASSERT_FALSE(thread_string_running.empty());
  LOG(INFO) << "Running: " << message_loop_thread;
  // String representation should look different when thread is not running
  ASSERT_STRNE(thread_string_running.c_str(),
               thread_string_before_start.c_str());
  message_loop_thread.ShutDown();
  std::string thread_string_after_shutdown = message_loop_thread.ToString();
  LOG(INFO) << "After shutdown: " << message_loop_thread;
  // String representation should look the same when thread is not running
  ASSERT_STREQ(thread_string_after_shutdown.c_str(),
               thread_string_before_start.c_str());
}