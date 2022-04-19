/* ============================================================
 *
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * ============================================================
 */

#include <QtiMutex.h>

#ifndef QTIMUTEX_HOST_BUILD
#include <cutils/properties.h>
#endif

using namespace qtimutex;

QtiRecursiveMutex *qtimutex_recursive_mutex_create() {
    QtiRecursiveMutex *ret = new QtiRecursiveMutex();
    return ret;
}

void QtiRecursiveMutex::lock() {
  #ifndef QTIMUTEX_HOST_BUILD
  char prop_str[PROPERTY_VALUE_MAX];
  std::string prop_name = QtiMutexDefaults::getInstance().getPropertyName();
  if (!prop_name.empty()) {
    property_get(prop_name.c_str(), prop_str, "");
    if (strcmp(prop_str, "false") == 0) {
      mutex.lock();
      return;
    }
  }
  #endif

  std::function<void(std::string)> debug_logger =
      QtiMutexDefaults::getInstance().getDebugLogger();
  std::function<void(std::string)> deadlock_logger =
      QtiMutexDefaults::getInstance().getDeadlockLogger();
  std::function<void()> deadlock_action =
      QtiMutexDefaults::getInstance().getDeadlockAction();

  if (QtiMutexDefaults::getInstance().getDebugFlag()) {
    debug_logger("Entering QtiRecursiveMutex::lock()");
  }

  auto deadline = std::chrono::steady_clock::now() +
      QtiMutexDefaults::getInstance().getDeadlockTimeout();
  bool lock_acquired = false;

  while (!lock_acquired && std::chrono::steady_clock::now() < deadline) {
    lock_acquired = mutex.try_lock_until(deadline);
  }

  if (!lock_acquired) {
    deadlock_logger("FAILURE - QtiSharedMutex timeout.");
    deadlock_action();
  }

  if (QtiMutexDefaults::getInstance().getDebugFlag()) {
    debug_logger("Exiting QtiRecursiveMutex::lock()");
  }
}

void qtimutex_recursive_mutex_lock(QtiRecursiveMutex *theMutex) {
    if (theMutex) {
        theMutex->lock();
    }
}

bool QtiRecursiveMutex::try_lock() {
  #ifndef QTIMUTEX_HOST_BUILD
  char prop_str[PROPERTY_VALUE_MAX];
  std::string prop_name = QtiMutexDefaults::getInstance().getPropertyName();
  if (!prop_name.empty()) {
    property_get(prop_name.c_str(), prop_str, "");
    if (strcmp(prop_str, "false") == 0) {
      return mutex.try_lock();
    }
  }
  #endif

  std::function<void(std::string)> debug_logger =
      QtiMutexDefaults::getInstance().getDebugLogger();
  bool try_lock_success = false;

  if (QtiMutexDefaults::getInstance().getDebugFlag()) {
    debug_logger("Entering QtiRecursiveMutex::try_lock()");
  }

  try_lock_success = mutex.try_lock();

  if (QtiMutexDefaults::getInstance().getDebugFlag()) {
    if (try_lock_success) {
      debug_logger("Exiting QtiRecursiveMutex::try_lock() with return true");
    } else {
      debug_logger("Exiting QtiRecursiveMutex::try_lock() with return false");
    }
  }

  return try_lock_success;
}

int qtimutex_recursive_mutex_try_lock(QtiRecursiveMutex *theMutex) {
    int ret = 0;

    if (theMutex) {
        ret = theMutex->try_lock() ? 1 : 0;
    }

    return ret;
}

void QtiRecursiveMutex::unlock() {
  #ifndef QTIMUTEX_HOST_BUILD
  char prop_str[PROPERTY_VALUE_MAX];
  std::string prop_name = QtiMutexDefaults::getInstance().getPropertyName();
  if (!prop_name.empty()) {
    property_get(prop_name.c_str(), prop_str, "");
    if (strcmp(prop_str, "false") == 0) {
      mutex.unlock();
      return;
    }
  }
  #endif

  std::function<void(std::string)> debug_logger =
      QtiMutexDefaults::getInstance().getDebugLogger();

  if (QtiMutexDefaults::getInstance().getDebugFlag()) {
    debug_logger("Entering QtiRecursiveMutex::unlock()");
  }

  mutex.unlock();

  if (QtiMutexDefaults::getInstance().getDebugFlag()) {
    debug_logger("Exiting QtiRecursiveMutex::unlock()");
  }
}

void qtimutex_recursive_mutex_unlock(QtiRecursiveMutex *theMutex) {
    if (theMutex) {
        theMutex->unlock();
    }
}

void qtimutex_recursive_mutex_destroy(QtiRecursiveMutex *theMutex) {
    if (theMutex) {
        delete theMutex;
    }
}
