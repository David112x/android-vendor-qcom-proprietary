/* ============================================================
 *
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * ============================================================
 */

#include <QtiMutex.h>

using namespace qtimutex;

QtiMutexDefaults::QtiMutexDefaults() {
  this->setDebugFlag(false);
  this->setDefaultDeadlockAction(abort);
  this->setDefaultDeadlockTimeout(std::chrono::minutes(5));
}

QtiMutexDefaults& QtiMutexDefaults::getInstance() {
  static QtiMutexDefaults instance;
  return instance;
}

void QtiMutexDefaults::setDefaultDeadlockTimeout(std::chrono::milliseconds timeout) {
  deadlock_timeout = timeout;
}

void QtiMutexDefaults::setDefaultDeadlockAction(std::function<void()> action) {
  deadlock_action = action;
}

void QtiMutexDefaults::setDefaultDeadlockLogger(std::function<void(std::string)> logger) {
  deadlock_logger = logger;
}

void QtiMutexDefaults::setDefaultDebugLogger(std::function<void(std::string)> logger) {
  debug_logger = logger;
}

void QtiMutexDefaults::setDebugFlag(bool debug_flag) {
  this->debug_flag = debug_flag;
}

void QtiMutexDefaults::setPropertyName(std::string property_name) {
  this->property_name = property_name;
}

std::chrono::milliseconds QtiMutexDefaults::getDeadlockTimeout() {
  return deadlock_timeout;
}

std::function<void()> QtiMutexDefaults::getDeadlockAction() {
  return deadlock_action;
}

std::function<void(std::string)> QtiMutexDefaults::getDeadlockLogger() {
  return deadlock_logger;
}

std::function<void(std::string)> QtiMutexDefaults::getDebugLogger() {
  return debug_logger;
}

bool QtiMutexDefaults::getDebugFlag() {
  return debug_flag;
}

std::string QtiMutexDefaults::getPropertyName() {
  return property_name;
}
