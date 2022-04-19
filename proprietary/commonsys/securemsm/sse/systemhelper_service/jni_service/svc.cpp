/*
 * Copyright (c) 2012-2013, 2018-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#define MAIN_C
#include "SystemEvent.h"
#include "SystemResource.h"
#include <android-base/logging.h>
#include <common_log.h>
#include <errno.h>
#include <hidl/HidlTransportSupport.h>
#include <iostream>
#include <jni.h>
#include <mutex>
#include <queue>
#include <semaphore.h>
#include <string>
#include <unistd.h>

using namespace android;
using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;
using android::sp;
using vendor::qti::hardware::systemhelper::V1_0::implementation::SystemEvent;
using vendor::qti::hardware::systemhelper::V1_0::implementation::SystemResource;
using vendor::qti::hardware::systemhelper::V1_0::SystemResourceType;
using namespace std;

std::mutex resourceLock;
static queue<string> msg_q;
sem_t sem;

extern "C" {

#include "com_qualcomm_qti_services_systemhelper_SysHelperService.h"

extern "C" jint
Java_com_qualcomm_qti_services_systemhelper_SysHelperService_init(
    JNIEnv *env, jclass /*cls*/) {

  jint ret = -1;
  configureRpcThreadpool(1, true /*willJoinThreadpool*/);

  SystemEvent *sysEvent = new SystemEvent();
  LOGD("SystemEvent interface registering.");
  auto status = sysEvent->registerAsService();
  if (status != android::OK) {
    LOGE("Could not register SystemEvent 1.0 interface");
  } else {
    SystemResource *sysResource = new SystemResource();
    LOGD("SystemResource interface registering.");
    auto status1 = sysResource->registerAsService();
    if (status1 != android::OK) {
      LOGE("Could not register SystemResource 1.0 interface");
    } else {
      ret = 0;
      LOGD("Successfully registered for both SystemEvent and SystemResource "
           "1.0 interfaces");
    }
  }
  sem_init(&sem, 0, 0);
  return ret;
}

extern "C" jstring
Java_com_qualcomm_qti_services_systemhelper_SysHelperService_getMessage(
    JNIEnv *env, jclass /*cls*/) {
  do {
    sem_wait(&sem);
  } while(msg_q.empty());

  string str = msg_q.front();
  msg_q.pop();
  jstring jresource = env->NewStringUTF(str.c_str());
  return jresource;
}

void processRequest(string resource) {
  std::lock_guard<std::mutex> lock(resourceLock);
  msg_q.push(resource);
  sem_post(&sem);
}

void acquireWakeLock() {
    processRequest("WL_AQUIRE");
}

void releaseWakeLock() {
    processRequest("WL_RELEASE");
}

void lockRotation(jboolean lock) {
    string str = (lock) ? "ROT_LOCK" : "ROT_UNLOCK";
    processRequest(str);
}

extern "C" jint
Java_com_qualcomm_qti_services_systemhelper_SysHelperService_terminate(
    JNIEnv * /*env*/, jclass /*cls*/) {
  return 0;
}
}
