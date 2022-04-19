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

#include <chrono>
#include <thread>

#include <gtest/gtest.h>

#include "execution_barrier.h"

using bluetooth::common::ExecutionBarrier;

static constexpr int kSleepTimeMs = 100;
static constexpr int kSchedulingDelayMaxMs = 5;

TEST(ExecutionBarrierTest, test_two_threads_wait_before_execution) {
  ExecutionBarrier execution_barrier;
  std::thread caller1([&]() {
    auto start = std::chrono::high_resolution_clock::now();
    execution_barrier.WaitForExecution();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed_ms = end - start;
    EXPECT_NEAR(elapsed_ms.count(), kSleepTimeMs, kSchedulingDelayMaxMs);
  });
  std::thread executor([&]() {
    // Wait for kSleepTimeMs so that caller1 starts waiting first
    std::this_thread::sleep_for(std::chrono::milliseconds(kSleepTimeMs));
    execution_barrier.NotifyFinished();
  });
  executor.join();
  caller1.join();
  // Further calls to WaitForExecution() no longer blocks
  std::thread caller2([&]() {
    auto start = std::chrono::high_resolution_clock::now();
    execution_barrier.WaitForExecution();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed_ms = end - start;
    EXPECT_LT(elapsed_ms.count(), kSchedulingDelayMaxMs);
  });
  caller2.join();
}

TEST(ExecutionBarrierTest, test_two_threads_execution_before_wait) {
  ExecutionBarrier execution_barrier;
  std::thread executor([&]() { execution_barrier.NotifyFinished(); });
  std::thread caller1([&]() {
    // Wait for kSleepTimeMs so that executor finishes running first
    std::this_thread::sleep_for(std::chrono::milliseconds(kSleepTimeMs));
    auto start = std::chrono::high_resolution_clock::now();
    execution_barrier.WaitForExecution();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed_ms = end - start;
    EXPECT_LT(elapsed_ms.count(), kSchedulingDelayMaxMs);
  });
  executor.join();
  caller1.join();
}

TEST(ExecutionBarrierTest, test_two_callers_one_executor) {
  ExecutionBarrier execution_barrier;
  std::thread caller1([&]() {
    auto start = std::chrono::high_resolution_clock::now();
    execution_barrier.WaitForExecution();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed_ms = end - start;
    EXPECT_NEAR(elapsed_ms.count(), kSleepTimeMs, 5);
  });
  std::thread caller2([&]() {
    auto start = std::chrono::high_resolution_clock::now();
    execution_barrier.WaitForExecution();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed_ms = end - start;
    EXPECT_NEAR(elapsed_ms.count(), kSleepTimeMs, 5);
  });
  std::thread executor([&]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(kSleepTimeMs));
    execution_barrier.NotifyFinished();
  });
  executor.join();
  caller1.join();
  caller2.join();
}
