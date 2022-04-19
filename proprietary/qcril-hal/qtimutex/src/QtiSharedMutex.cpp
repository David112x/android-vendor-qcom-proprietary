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

QtiSharedMutex *qtimutex_shared_mutex_create(void) {
    QtiSharedMutex *theMutex = new QtiSharedMutex();
    return theMutex;
}

void QtiSharedMutex::lock() {
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
    debug_logger("Entering QtiSharedMutex::lock()");
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
    debug_logger("Exiting QtiSharedMutex::lock()");
  }
}

void qtimutex_shared_mutex_lock(QtiSharedMutex *theMutex) {
    if (theMutex) {
        theMutex->lock();
    }
}

void QtiSharedMutex::lock_shared() {
  #ifndef QTIMUTEX_HOST_BUILD
  char prop_str[PROPERTY_VALUE_MAX];
  std::string prop_name = QtiMutexDefaults::getInstance().getPropertyName();
  if (!prop_name.empty()) {
    property_get(prop_name.c_str(), prop_str, "");
    if (strcmp(prop_str, "false") == 0) {
      mutex.lock_shared();
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
    debug_logger("Entering QtiSharedMutex::lock_shared()");
  }

  auto deadline = std::chrono::steady_clock::now() +
      QtiMutexDefaults::getInstance().getDeadlockTimeout();
  bool lock_acquired = false;

  while (!lock_acquired && std::chrono::steady_clock::now() < deadline) {
    lock_acquired = mutex.try_lock_shared_until(deadline);
  }

  if (!lock_acquired) {
    deadlock_logger("FAILURE - QtiSharedMutex timeout.");
    deadlock_action();
  }

  if (QtiMutexDefaults::getInstance().getDebugFlag()) {
    debug_logger("Exiting QtiSharedMutex::lock_shared()");
  }
}

void qtimutex_shared_mutex_lock_shared(QtiSharedMutex *theMutex) {
    if (theMutex) {
        theMutex->lock_shared();
    }
}

bool QtiSharedMutex::try_lock() {
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
    debug_logger("Entering QtiSharedMutex::try_lock()");
  }

  try_lock_success = mutex.try_lock();

  if (QtiMutexDefaults::getInstance().getDebugFlag()) {
    if (try_lock_success) {
      debug_logger("Exiting QtiSharedMutex::try_lock() with return true");
    } else {
      debug_logger("Exiting QtiSharedMutex::try_lock() with return false");
    }
  }

  return try_lock_success;
}

int qtimutex_shared_mutex_try_lock(QtiSharedMutex *theMutex) {
    int ret = 0;

    if (theMutex) {
        ret = theMutex->try_lock()? 1 : 0;
    }

    return ret;
}

bool QtiSharedMutex::try_lock_shared() {
  #ifndef QTIMUTEX_HOST_BUILD
  char prop_str[PROPERTY_VALUE_MAX];
  std::string prop_name = QtiMutexDefaults::getInstance().getPropertyName();
  if (!prop_name.empty()) {
    property_get(prop_name.c_str(), prop_str, "");
    if (strcmp(prop_str, "false") == 0) {
      return mutex.try_lock_shared();
    }
  }
  #endif

  std::function<void(std::string)> debug_logger =
      QtiMutexDefaults::getInstance().getDebugLogger();
  bool try_lock_shared_success = false;

  if (QtiMutexDefaults::getInstance().getDebugFlag()) {
    debug_logger("Entering QtiSharedMutex::try_lock_shared()");
  }

  try_lock_shared_success = mutex.try_lock_shared();

  if (QtiMutexDefaults::getInstance().getDebugFlag()) {
    if (try_lock_shared_success) {
      debug_logger("Exiting QtiSharedMutex::try_lock_shared() with return true");
    } else {
      debug_logger("Exiting QtiSharedMutex::try_lock_shared() with return false");
    }
  }

  return try_lock_shared_success;
}

int qtimutex_shared_mutex_try_lock_shared(QtiSharedMutex *theMutex) {
    int ret = 0;
    if (theMutex) {
        ret = theMutex->try_lock_shared() ? 1 : 0;
    }
    return ret;
}

void QtiSharedMutex::unlock() {
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
    debug_logger("Entering QtiSharedMutex::unlock()");
  }

  mutex.unlock();

  if (QtiMutexDefaults::getInstance().getDebugFlag()) {
    debug_logger("Exiting QtiSharedMutex::unlock()");
  }
}

void qtimutex_shared_mutex_unlock(QtiSharedMutex *theMutex) {
    if (theMutex) {
        theMutex->unlock();
    }
}

void QtiSharedMutex::unlock_shared() {
  #ifndef QTIMUTEX_HOST_BUILD
  char prop_str[PROPERTY_VALUE_MAX];
  std::string prop_name = QtiMutexDefaults::getInstance().getPropertyName();
  if (!prop_name.empty()) {
    property_get(prop_name.c_str(), prop_str, "");
    if (strcmp(prop_str, "false") == 0) {
      mutex.unlock_shared();
      return;
    }
  }
  #endif

  std::function<void(std::string)> debug_logger =
      QtiMutexDefaults::getInstance().getDebugLogger();

  if (QtiMutexDefaults::getInstance().getDebugFlag()) {
    debug_logger("Entering QtiSharedMutex::unlock_shared()");
  }

  mutex.unlock_shared();

  if (QtiMutexDefaults::getInstance().getDebugFlag()) {
    debug_logger("Exiting QtiSharedMutex::unlock_shared()");
  }
}

void qtimutex_shared_mutex_unlock_shared(QtiSharedMutex *theMutex) {
    if (theMutex) {
        theMutex->unlock_shared();
    }
}

void qtimutex_shared_mutex_destroy(QtiSharedMutex *theMutex) {
    if (theMutex) {
        delete theMutex;
    }
}
