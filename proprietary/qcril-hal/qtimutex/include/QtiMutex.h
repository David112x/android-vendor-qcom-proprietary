/* ============================================================
 *
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * ============================================================
 */

#pragma once

#ifdef __cplusplus
#include <chrono>
#include <mutex>
#include <shared_mutex>
#include <functional>
#include <string>
#include <pthread.h>

namespace qtimutex {
  struct QtiMutexDefaults {
    public:
      static QtiMutexDefaults &getInstance();

      void setDefaultDeadlockTimeout(std::chrono::milliseconds timeout);
      void setDefaultDeadlockAction(std::function<void()> action);
      void setDefaultDeadlockLogger(std::function<void(std::string)> logger);
      void setDefaultDebugLogger(std::function<void(std::string)> logger);
      void setDebugFlag(bool);
      void setPropertyName(std::string);

      std::chrono::milliseconds getDeadlockTimeout();
      std::function<void()> getDeadlockAction();
      std::function<void(std::string)> getDeadlockLogger();
      std::function<void(std::string)> getDebugLogger();
      std::string getPropertyName();
      bool getDebugFlag();

    private:
      QtiMutexDefaults();
      std::chrono::milliseconds deadlock_timeout;
      std::function<void()> deadlock_action;
      std::function<void(std::string)> deadlock_logger;
      std::function<void(std::string)> debug_logger;
      std::string property_name;
      bool debug_flag;
  };

  struct QtiRecursiveMutex
  {
    public:
      void lock();
      bool try_lock();
      void unlock();
    private:
      std::recursive_timed_mutex mutex;
  };

  struct QtiSharedMutex
  {
    public:
      void lock();
      void lock_shared();
      bool try_lock();
      bool try_lock_shared();
      void unlock();
      void unlock_shared();
    private:
      std::shared_timed_mutex mutex;
  };

  class QtiShmMutex
  {
      public:
          using native_handle_type = pthread_mutex_t *;
          QtiShmMutex();
          void lock();
          bool try_lock();
          void unlock();
          native_handle_type native_handle() { return &mutex; }
      private:
          pthread_mutex_t mutex;
  };

  class QtiShmRobustMutex
  {
      public:
          using native_handle_type = void *;
          QtiShmRobustMutex();
          void lock();
          bool try_lock();
          void unlock();
      private:
          void *mutex;
  };

  enum class QtiShmCVStatus {
      no_timeout,
      timeout,
      owner_dead,
  };

  class QtiShmConditionVariable
  {
      private:
          pthread_cond_t condVar;
      public:
          using native_handle_type = pthread_cond_t *;
          QtiShmConditionVariable();
          QtiShmConditionVariable(const QtiShmConditionVariable&) = delete;
          ~QtiShmConditionVariable();
          void notify_one() noexcept;
          void notify_all() noexcept;
          template <class Lock>
              void wait( Lock& lock ) {
                  wait(lock, []() {return false;});
              }
          template <class Lock, class Predicate>
              void wait( Lock& lock, Predicate pred) {
                  while(!pred()) {
                      pthread_mutex_t *nativeMutex = lock.mutex()->native_handle();
                      pthread_cond_wait(&condVar, nativeMutex);
                  }
              }
          template <class Lock, class Rep, class Period>
              std::cv_status wait_for(Lock& lock, const std::chrono::duration<Rep, Period>& rel_time);
          template <class Lock, class Rep, class Period, class Predicate>
              bool wait_for(
                      Lock& lock,
                      const std::chrono::duration<Rep, Period>& rel_time,
                      Predicate pred);
          template <class Lock, class Clock, class Duration>
              QtiShmCVStatus wait_until(
                      Lock& lock,
                      const std::chrono::time_point<Clock, Duration>& timeout_time) {
                  struct timespec ts = {};
                  QtiShmCVStatus ret;
                  auto secs = std::chrono::time_point_cast<std::chrono::seconds>(timeout_time);
                  if (secs > timeout_time) {
                      secs = secs - std::chrono::seconds{1};
                  }
                  auto nsecs = std::chrono::duration_cast<std::chrono::nanoseconds>(timeout_time - secs);
                  ts.tv_sec = secs.time_since_epoch().count();
                  ts.tv_nsec = nsecs.count();

                 int  rc = pthread_cond_timedwait(&condVar,
                          lock.mutex()->native_handle(),
                          &ts);
                  switch(rc) {
                      case EOWNERDEAD:
                          // Currently Android has no support for robust mutexes.
                          // Leaving in place in case support becomes available.
                          ret = QtiShmCVStatus::owner_dead;
                          break;
                      case ETIMEDOUT:
                          ret = QtiShmCVStatus::timeout;
                          break;
                      default:
                          ret = QtiShmCVStatus::no_timeout;
                          break;
                  }
                  return ret;
              }

          template <class Lock, class Clock, class Duration, class Pred>
              QtiShmCVStatus wait_until(
                      Lock& lock,
                      const std::chrono::time_point<Clock, Duration>& timeout_time,
                      Pred pred) {

                  QtiShmCVStatus rc = QtiShmCVStatus::no_timeout;
                  while(!pred() && rc != QtiShmCVStatus::timeout) {
                      rc = wait_until(lock, timeout_time);
                      if (pred()) {
                          rc = QtiShmCVStatus::no_timeout;
                      }
                  }
                  return rc;
              }

          native_handle_type native_handle();
  };
}
typedef struct qtimutex::QtiMutexDefaults QtiMutexDefaults;
typedef struct qtimutex::QtiRecursiveMutex QtiRecursiveMutex;
typedef struct qtimutex::QtiSharedMutex QtiSharedMutex;
#else
struct QtiMutexDefaults;
struct QtiRecursiveMutex;
struct QtiSharedMutex;
#endif

extern "C" {
    QtiRecursiveMutex *qtimutex_recursive_mutex_create(void);
    void qtimutex_recursive_mutex_lock(QtiRecursiveMutex *);
    int  qtimutex_recursive_mutex_try_lock(QtiRecursiveMutex *);
    void qtimutex_recursive_mutex_unlock(QtiRecursiveMutex *);
    void qtimutex_recursive_mutex_destroy(QtiRecursiveMutex *);

    QtiSharedMutex *qtimutex_shared_mutex_create(void);
    void qtimutex_shared_mutex_lock(QtiSharedMutex *);
    void qtimutex_shared_mutex_lock_shared(QtiSharedMutex *);
    int  qtimutex_shared_mutex_try_lock(QtiSharedMutex *);
    int  qtimutex_shared_mutex_try_lock_shared(QtiSharedMutex *);
    void qtimutex_shared_mutex_unlock(QtiSharedMutex *);
    void qtimutex_shared_mutex_unlock_shared(QtiSharedMutex *);
    void qtimutex_shared_mutex_destroy(QtiSharedMutex *);
}
