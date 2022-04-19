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

#include <base/bind.h>
#include <base/logging.h>
#include <base/run_loop.h>
#include <base/threading/thread.h>
#include <benchmark/benchmark.h>
#include <memory>
#include <thread>

#include "common/execution_barrier.h"
#include "common/message_loop_thread.h"
#include "osi/include/fixed_queue.h"
#include "osi/include/thread.h"

using ::benchmark::State;
using bluetooth::common::ExecutionBarrier;
using bluetooth::common::MessageLoopThread;

#define NUM_MESSAGES_TO_SEND 100000

volatile static int g_counter = 0;
static std::unique_ptr<ExecutionBarrier> g_counter_barrier = nullptr;

void pthread_callback_batch(void* context) {
  auto queue = static_cast<fixed_queue_t*>(context);
  CHECK_NE(queue, nullptr);
  fixed_queue_dequeue(queue);
  g_counter++;
  if (g_counter >= NUM_MESSAGES_TO_SEND) {
    g_counter_barrier->NotifyFinished();
  }
}

void callback_sequential(void* context) { g_counter_barrier->NotifyFinished(); }

void callback_sequential_queue(fixed_queue_t* queue, void* context) {
  CHECK_NE(queue, nullptr);
  fixed_queue_dequeue(queue);
  g_counter_barrier->NotifyFinished();
}

void callback_batch(fixed_queue_t* queue, void* data) {
  CHECK_NE(queue, nullptr);
  fixed_queue_dequeue(queue);
  g_counter++;
  if (g_counter >= NUM_MESSAGES_TO_SEND) {
    g_counter_barrier->NotifyFinished();
  }
}

class BM_ThreadPerformance : public ::benchmark::Fixture {
 protected:
  void SetUp(State& st) override {
    benchmark::Fixture::SetUp(st);
    set_up_barrier_ = std::make_unique<ExecutionBarrier>();
    g_counter = 0;
    bt_msg_queue_ = fixed_queue_new(SIZE_MAX);
  }
  void TearDown(State& st) override {
    fixed_queue_free(bt_msg_queue_, nullptr);
    bt_msg_queue_ = nullptr;
    set_up_barrier_.reset(nullptr);
    g_counter_barrier.reset(nullptr);
    benchmark::Fixture::TearDown(st);
  }
  fixed_queue_t* bt_msg_queue_ = nullptr;
  std::unique_ptr<ExecutionBarrier> set_up_barrier_;
};

class BM_MessageLoop : public BM_ThreadPerformance {
 public:
  static void RunThread(void* context) {
    auto test = static_cast<BM_MessageLoop*>(context);
    test->RunMessageLoop();
  }
  static void* RunPThread(void* context) {
    auto test = static_cast<BM_MessageLoop*>(context);
    test->RunMessageLoop();
    return nullptr;
  }
  void RunMessageLoop() {
    message_loop_ = new base::MessageLoop();
    run_loop_ = new base::RunLoop();
    message_loop_->task_runner()->PostTask(
        FROM_HERE, base::BindOnce(&ExecutionBarrier::NotifyFinished,
                                  base::Unretained(set_up_barrier_.get())));
    run_loop_->Run();
    delete message_loop_;
    message_loop_ = nullptr;
    delete run_loop_;
    run_loop_ = nullptr;
  }

 protected:
  base::MessageLoop* message_loop_ = nullptr;
  base::RunLoop* run_loop_ = nullptr;
};

class BM_MessageLoopOsiThread : public BM_MessageLoop {
 protected:
  void SetUp(State& st) override {
    BM_MessageLoop::SetUp(st);
    thread_ = thread_new("BM_MessageLoopOnOsiThread thread");
    thread_post(thread_, &BM_MessageLoop::RunThread, this);
    set_up_barrier_->WaitForExecution();
  }

  void TearDown(State& st) override {
    message_loop_->task_runner()->PostTask(FROM_HERE,
                                           run_loop_->QuitWhenIdleClosure());
    thread_free(thread_);
    thread_ = nullptr;
    BM_MessageLoop::TearDown(st);
  }

  thread_t* thread_ = nullptr;
};

BENCHMARK_F(BM_MessageLoopOsiThread, batch_enque_dequeue)(State& state) {
  for (auto _ : state) {
    g_counter = 0;
    g_counter_barrier = std::make_unique<ExecutionBarrier>();
    for (int i = 0; i < NUM_MESSAGES_TO_SEND; i++) {
      fixed_queue_enqueue(bt_msg_queue_, (void*)&g_counter);
      message_loop_->task_runner()->PostTask(
          FROM_HERE, base::BindOnce(&callback_batch, bt_msg_queue_, nullptr));
    }
    g_counter_barrier->WaitForExecution();
  }
};

BENCHMARK_F(BM_MessageLoopOsiThread, sequential_execution)(State& state) {
  for (auto _ : state) {
    for (int i = 0; i < NUM_MESSAGES_TO_SEND; i++) {
      g_counter_barrier = std::make_unique<ExecutionBarrier>();
      message_loop_->task_runner()->PostTask(
          FROM_HERE, base::BindOnce(&callback_sequential, nullptr));
      g_counter_barrier->WaitForExecution();
    }
  }
};

class BM_MessageLoopStlThread : public BM_MessageLoop {
 protected:
  void SetUp(State& st) override {
    BM_MessageLoop::SetUp(st);
    thread_ = new std::thread(&BM_MessageLoop::RunThread, this);
    set_up_barrier_->WaitForExecution();
  }

  void TearDown(State& st) override {
    message_loop_->task_runner()->PostTask(FROM_HERE,
                                           run_loop_->QuitWhenIdleClosure());
    thread_->join();
    delete thread_;
    thread_ = nullptr;
    BM_MessageLoop::TearDown(st);
  }

  std::thread* thread_ = nullptr;
};

BENCHMARK_F(BM_MessageLoopStlThread, batch_enque_dequeue)(State& state) {
  for (auto _ : state) {
    g_counter = 0;
    g_counter_barrier = std::make_unique<ExecutionBarrier>();
    for (int i = 0; i < NUM_MESSAGES_TO_SEND; i++) {
      fixed_queue_enqueue(bt_msg_queue_, (void*)&g_counter);
      message_loop_->task_runner()->PostTask(
          FROM_HERE, base::BindOnce(&callback_batch, bt_msg_queue_, nullptr));
    }
    g_counter_barrier->WaitForExecution();
  }
};

BENCHMARK_F(BM_MessageLoopStlThread, sequential_execution)(State& state) {
  for (auto _ : state) {
    for (int i = 0; i < NUM_MESSAGES_TO_SEND; i++) {
      g_counter_barrier = std::make_unique<ExecutionBarrier>();
      message_loop_->task_runner()->PostTask(
          FROM_HERE, base::BindOnce(&callback_sequential, nullptr));
      g_counter_barrier->WaitForExecution();
    }
  }
};

class BM_MessageLoopPosixThread : public BM_MessageLoop {
 protected:
  void SetUp(State& st) override {
    BM_MessageLoop::SetUp(st);
    pthread_create(&thread_, nullptr, &BM_MessageLoop::RunPThread, (void*)this);
    set_up_barrier_->WaitForExecution();
  }

  void TearDown(State& st) override {
    message_loop_->task_runner()->PostTask(FROM_HERE,
                                           run_loop_->QuitWhenIdleClosure());
    pthread_join(thread_, nullptr);
    BM_MessageLoop::TearDown(st);
  }

  pthread_t thread_ = -1;
};

BENCHMARK_F(BM_MessageLoopPosixThread, batch_enque_dequeue)(State& state) {
  for (auto _ : state) {
    g_counter = 0;
    g_counter_barrier = std::make_unique<ExecutionBarrier>();
    for (int i = 0; i < NUM_MESSAGES_TO_SEND; i++) {
      fixed_queue_enqueue(bt_msg_queue_, (void*)&g_counter);
      message_loop_->task_runner()->PostTask(
          FROM_HERE, base::BindOnce(&callback_batch, bt_msg_queue_, nullptr));
    }
    g_counter_barrier->WaitForExecution();
  }
};

BENCHMARK_F(BM_MessageLoopPosixThread, sequential_execution)(State& state) {
  for (auto _ : state) {
    for (int i = 0; i < NUM_MESSAGES_TO_SEND; i++) {
      g_counter_barrier = std::make_unique<ExecutionBarrier>();
      message_loop_->task_runner()->PostTask(
          FROM_HERE, base::BindOnce(&callback_sequential, nullptr));
      g_counter_barrier->WaitForExecution();
    }
  }
};

class BM_OsiReactorThread : public BM_ThreadPerformance {
 protected:
  void SetUp(State& st) override {
    BM_ThreadPerformance::SetUp(st);
    thread_ = thread_new("BM_OsiReactorThread thread");
  }

  void TearDown(State& st) override {
    thread_free(thread_);
    thread_ = nullptr;
    BM_ThreadPerformance::TearDown(st);
  }

  thread_t* thread_ = nullptr;
};

BENCHMARK_F(BM_OsiReactorThread, batch_enque_dequeue_using_thread_post)
(State& state) {
  for (auto _ : state) {
    g_counter = 0;
    g_counter_barrier = std::make_unique<ExecutionBarrier>();
    for (int i = 0; i < NUM_MESSAGES_TO_SEND; i++) {
      fixed_queue_enqueue(bt_msg_queue_, (void*)&g_counter);
      thread_post(thread_, pthread_callback_batch, bt_msg_queue_);
    }
    g_counter_barrier->WaitForExecution();
  }
};

BENCHMARK_F(BM_OsiReactorThread, sequential_execution_using_thread_post)
(State& state) {
  for (auto _ : state) {
    for (int i = 0; i < NUM_MESSAGES_TO_SEND; i++) {
      g_counter_barrier = std::make_unique<ExecutionBarrier>();
      thread_post(thread_, callback_sequential, nullptr);
      g_counter_barrier->WaitForExecution();
    }
  }
};

BENCHMARK_F(BM_OsiReactorThread, batch_enque_dequeue_using_reactor)
(State& state) {
  fixed_queue_register_dequeue(bt_msg_queue_, thread_get_reactor(thread_),
                               callback_batch, nullptr);
  for (auto _ : state) {
    g_counter = 0;
    g_counter_barrier = std::make_unique<ExecutionBarrier>();
    for (int i = 0; i < NUM_MESSAGES_TO_SEND; i++) {
      fixed_queue_enqueue(bt_msg_queue_, (void*)&g_counter);
    }
    g_counter_barrier->WaitForExecution();
  }
};

BENCHMARK_F(BM_OsiReactorThread, sequential_execution_using_reactor)
(State& state) {
  fixed_queue_register_dequeue(bt_msg_queue_, thread_get_reactor(thread_),
                               callback_sequential_queue, nullptr);
  for (auto _ : state) {
    for (int i = 0; i < NUM_MESSAGES_TO_SEND; i++) {
      g_counter_barrier = std::make_unique<ExecutionBarrier>();
      fixed_queue_enqueue(bt_msg_queue_, (void*)&g_counter);
      g_counter_barrier->WaitForExecution();
    }
  }
};

class BM_MessageLooopThread : public BM_ThreadPerformance {
 protected:
  void SetUp(State& st) override {
    BM_ThreadPerformance::SetUp(st);
    message_loop_thread_ =
        new MessageLoopThread("BM_MessageLooopThread thread");
    message_loop_thread_->StartUp();
    message_loop_thread_->DoInThread(
        FROM_HERE, base::BindOnce(&ExecutionBarrier::NotifyFinished,
                                  base::Unretained(set_up_barrier_.get())));
    set_up_barrier_->WaitForExecution();
  }

  void TearDown(State& st) override {
    message_loop_thread_->ShutDown();
    delete message_loop_thread_;
    message_loop_thread_ = nullptr;
    BM_ThreadPerformance::TearDown(st);
  }

  MessageLoopThread* message_loop_thread_ = nullptr;
};

BENCHMARK_F(BM_MessageLooopThread, batch_enque_dequeue)(State& state) {
  for (auto _ : state) {
    g_counter = 0;
    g_counter_barrier = std::make_unique<ExecutionBarrier>();
    for (int i = 0; i < NUM_MESSAGES_TO_SEND; i++) {
      fixed_queue_enqueue(bt_msg_queue_, (void*)&g_counter);
      message_loop_thread_->DoInThread(
          FROM_HERE, base::BindOnce(&callback_batch, bt_msg_queue_, nullptr));
    }
    g_counter_barrier->WaitForExecution();
  }
};

BENCHMARK_F(BM_MessageLooopThread, sequential_execution)(State& state) {
  for (auto _ : state) {
    for (int i = 0; i < NUM_MESSAGES_TO_SEND; i++) {
      g_counter_barrier = std::make_unique<ExecutionBarrier>();
      message_loop_thread_->DoInThread(
          FROM_HERE, base::BindOnce(&callback_sequential, nullptr));
      g_counter_barrier->WaitForExecution();
    }
  }
};

class BM_LibChromeThread : public BM_ThreadPerformance {
 protected:
  void SetUp(State& st) override {
    BM_ThreadPerformance::SetUp(st);
    thread_ = new base::Thread("BM_LibChromeThread thread");
    thread_->Start();
    thread_->task_runner()->PostTask(
        FROM_HERE, base::BindOnce(&ExecutionBarrier::NotifyFinished,
                                  base::Unretained(set_up_barrier_.get())));
    set_up_barrier_->WaitForExecution();
  }

  void TearDown(State& st) override {
    thread_->Stop();
    delete thread_;
    thread_ = nullptr;
    BM_ThreadPerformance::TearDown(st);
  }

  base::Thread* thread_ = nullptr;
};

BENCHMARK_F(BM_LibChromeThread, batch_enque_dequeue)(State& state) {
  for (auto _ : state) {
    g_counter = 0;
    g_counter_barrier = std::make_unique<ExecutionBarrier>();
    for (int i = 0; i < NUM_MESSAGES_TO_SEND; i++) {
      fixed_queue_enqueue(bt_msg_queue_, (void*)&g_counter);
      thread_->task_runner()->PostTask(
          FROM_HERE, base::BindOnce(&callback_batch, bt_msg_queue_, nullptr));
    }
    g_counter_barrier->WaitForExecution();
  }
};

BENCHMARK_F(BM_LibChromeThread, sequential_execution)(State& state) {
  for (auto _ : state) {
    for (int i = 0; i < NUM_MESSAGES_TO_SEND; i++) {
      g_counter_barrier = std::make_unique<ExecutionBarrier>();
      thread_->task_runner()->PostTask(
          FROM_HERE, base::BindOnce(&callback_sequential, nullptr));
      g_counter_barrier->WaitForExecution();
    }
  }
};

int main(int argc, char** argv) {
  // Disable LOG() output from libchrome
  logging::LoggingSettings log_settings;
  log_settings.logging_dest = logging::LoggingDestination::LOG_NONE;
  CHECK(logging::InitLogging(log_settings)) << "Failed to set up logging";
  ::benchmark::Initialize(&argc, argv);
  if (::benchmark::ReportUnrecognizedArguments(argc, argv)) {
    return 1;
  }
  ::benchmark::RunSpecifiedBenchmarks();
}