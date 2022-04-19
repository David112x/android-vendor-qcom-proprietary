/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#ifndef QMI_RIL_UTF

#include "gtest/gtest.h"
#include <QtiMutex.h>
#include <chrono>
#include <functional>
#include <iostream>
#include <condition_variable>
#include <thread>

#define TIMEOUT 1000

using namespace qtimutex;

std::mutex mutex;
std::condition_variable cv;
QtiSharedMutex shared_mutex;
QtiRecursiveMutex rec_mutex;
bool abort_received;
bool ready;
bool lock_acquired;

void logger(std::string msg) {
  printf("%s\n", msg.c_str());
}

void abort_sim() {
  logger("abort_sim()");
  abort_received = true;
}

void set_defaults() {
  QtiMutexDefaults::getInstance().setDebugFlag(true);
  QtiMutexDefaults::getInstance().setDefaultDebugLogger(logger);
  QtiMutexDefaults::getInstance().setDefaultDeadlockLogger(logger);
  QtiMutexDefaults::getInstance().setDefaultDeadlockAction(abort_sim);
  QtiMutexDefaults::getInstance().setDefaultDeadlockTimeout(std::chrono::milliseconds(TIMEOUT));
}

void force_timeout() {
  logger("forcing timeout");
  std::this_thread::sleep_for(std::chrono::milliseconds(TIMEOUT * 2));
}

//==============================================================
// QtiSharedMutex test cases
//==============================================================
void shared_mtx_lock_with_timeout() {
  //call lock() on shared mutex
  {
    std::lock_guard<std::mutex> lock(mutex);
    shared_mutex.lock();
    ready = true;
    cv.notify_one();
  }

  //invoke timeout
  force_timeout();

  //unlock shared mutex
  shared_mutex.unlock();
}

void shared_mtx_lock_shared_with_timeout() {
  //call shared_lock shared mutex
  {
    std::lock_guard<std::mutex> lock(mutex);
    shared_mutex.lock_shared();
    ready = true;
    cv.notify_one();
  }

  //invoke timeout
  force_timeout();

  //unlock shared mutex
  shared_mutex.unlock_shared();
}

//Testing QtiMutexShared::lock -- pass case
TEST (LockTest_Shared, AquireCase) {
  //reset abort_received
  abort_received = false;

  shared_mutex.lock();
  shared_mutex.unlock();

  EXPECT_EQ (false, abort_received);
}

//Testing QtiMutexShared::lock -- fail case
TEST (LockTest_Shared, TimeoutCase) {
  //reset abort_received
  abort_received = false;

  //reset ready flag
  ready = false;

  //other thread calls lock() and invokes timeout
  std::thread t1(shared_mtx_lock_with_timeout);

  //wait for ready flag
  {
    std::unique_lock<std::mutex> lock(mutex);
    cv.wait(lock, []{return ready;});
  }

  //try to lock shared_mutex -- timeout expected
  shared_mutex.lock();
  EXPECT_EQ (true, abort_received);

  t1.join();
}

//Testing QtiMutexShared::lock_shared -- pass case
TEST (LockSharedTest_Shared, AquireCase) {
  //reset abort_received
  abort_received = false;

  //reset ready flag
  ready = false;

  //other thread calls lock_shared() and invokes timeout
  std::thread t1(shared_mtx_lock_shared_with_timeout);

  //wait for ready flag
  {
    std::unique_lock<std::mutex> lock(mutex);
    cv.wait(lock, []{return ready;});
  }

  //try to shared_mutex with lock_shared -- no timeout expected
  shared_mutex.lock_shared();
  EXPECT_EQ (false, abort_received);

  t1.join();

  shared_mutex.unlock_shared();
}

//Testing QtiMutexShared::lock_shared -- fail case
TEST (LockSharedTest_Shared, TimeoutCase) {
  //reset abort_received
  abort_received = false;

  //reset ready flag
  ready = false;

  //other thread calls lock() and invokes timeout
  std::thread t1(shared_mtx_lock_with_timeout);

  //wait for ready flag
  {
    std::unique_lock<std::mutex> lock(mutex);
    cv.wait(lock, []{return ready;});
  }

  //try to shared_mutex with lock_shared -- timeout expected
  shared_mutex.lock_shared();
  EXPECT_EQ (true, abort_received);

  t1.join();
}

//Testing QtiMutexShared::try_lock -- pass case
TEST (TryLockTest_Shared, AquireCase) {
  //reset lock_acquired
  lock_acquired = false;

  lock_acquired = shared_mutex.try_lock();
  EXPECT_EQ (true, lock_acquired);

  shared_mutex.unlock();
}

//Testing QtiMutexShared::try_lock -- fail case
TEST (TryLockTest_Shared, TimeoutCase) {
  //reset lock_acquired
  lock_acquired = false;

  //reset ready flag
  ready = false;

  //other thread calls lock() and invokes timeout
  std::thread t1(shared_mtx_lock_with_timeout);

  //wait for ready flag
  {
    std::unique_lock<std::mutex> lock(mutex);
    cv.wait(lock, []{return ready;});
  }

  //call try_lock -- expect failure
  lock_acquired = shared_mutex.try_lock();
  EXPECT_EQ (false, lock_acquired);

  t1.join();
}

//Testing QtiMutexShared::try_lock_shared -- pass case
TEST (TryLockSharedTest_Shared, AquireCase) {
  //reset lock_acquired
  lock_acquired = false;

  //reset ready flag
  ready = false;

  //other thread calls lock_shared() and invokes timeout
  std::thread t1(shared_mtx_lock_shared_with_timeout);

  //wait for ready flag
  {
    std::unique_lock<std::mutex> lock(mutex);
    cv.wait(lock, []{return ready;});
  }

  //call try_lock_shared -- expect success
  lock_acquired = shared_mutex.try_lock_shared();
  EXPECT_EQ (true, lock_acquired);

  //call unlock_shared()
  shared_mutex.unlock_shared();

  t1.join();
}

//Testing QtiMutexShared::try_lock_shared -- fail case
TEST (TryLockSharedTest_Shared, TimeoutCase) {
  //reset lock_acquired
  lock_acquired = false;

  //reset ready flag
  ready = false;

  //other thread calls lock() and invokes timeout
  std::thread t1(shared_mtx_lock_with_timeout);

  //wait for ready flag
  {
    std::unique_lock<std::mutex> lock(mutex);
    cv.wait(lock, []{return ready;});
  }

  //call try_lock_shared -- expect failure
  lock_acquired = shared_mutex.try_lock_shared();
  EXPECT_EQ (false, lock_acquired);

  t1.join();
}

//==============================================================
// QtiRecursiveMutex test cases
//==============================================================
void rec_mtx_lock_x2_with_timeout() {
  //call lock x2 on recursive mutex
  {
    std::lock_guard<std::mutex> lock(mutex);
    rec_mutex.lock();
    rec_mutex.lock();
    ready = true;
    cv.notify_one();
  }

  //invoke timeout
  force_timeout();

  //unlock shared mutex
  rec_mutex.unlock();
  rec_mutex.unlock();
}

TEST (LockTest_Recursive, AquireCase) {
  //reset abort_received
  abort_received = false;

  rec_mutex.lock();
  rec_mutex.lock();
  rec_mutex.unlock();
  rec_mutex.lock();
  rec_mutex.unlock();
  rec_mutex.unlock();

  EXPECT_EQ (false, abort_received);
}

TEST (LockTest_Recursive, TimeoutCase) {
  //reset abort_received
  abort_received = false;

  //reset ready flag
  ready = false;

  //other thread calls lock() twice and invokes timeout
  std::thread t1(rec_mtx_lock_x2_with_timeout);

  //wait for ready flag
  {
    std::unique_lock<std::mutex> lock(mutex);
    cv.wait(lock, []{return ready;});
  }

  //try to lock shared_mutex -- timeout expected
  rec_mutex.lock();
  EXPECT_EQ (true, abort_received);

  t1.join();
}

TEST (TryLockTest_Recursive, AquireCase) {
  //reset abort_received
  abort_received = false;

  //reset lock_acquired
  lock_acquired = false;

  //lock x1
  rec_mutex.lock();

  //try lock and expect success
  lock_acquired = rec_mutex.try_lock();
  EXPECT_EQ (lock_acquired, true);

  //unlock x2
  rec_mutex.unlock();
  rec_mutex.unlock();

  EXPECT_EQ (false, abort_received);
}

TEST (TryLockTest_Recursive, TimeoutCase) {
  //reset lock_acquired
  lock_acquired = false;

  //reset ready flag
  ready = false;

  //other thread calls lock() twice and invokes timeout
  std::thread t1(rec_mtx_lock_x2_with_timeout);

  //wait for ready flag
  {
    std::unique_lock<std::mutex> lock(mutex);
    cv.wait(lock, []{return ready;});
  }

  //call try_lock() -- failure expected
  lock_acquired = rec_mutex.try_lock();
  EXPECT_EQ (false, lock_acquired);

  t1.join();
}

int main(int argc, char **argv) {
  set_defaults();
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

#endif
