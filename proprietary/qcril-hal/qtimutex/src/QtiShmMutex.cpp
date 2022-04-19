/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <QtiMutex.h>
#include <pthread.h>


namespace qtimutex {
QtiShmMutex::QtiShmMutex() {
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    // Not currently supported by Android
    // pthread_mutexattr_setrobust(&attr, PTHREAD_MUTEX_ROBUST);
    pthread_mutex_init(&mutex, &attr);
}

void QtiShmMutex::lock() {
    pthread_mutex_lock(&mutex);
}

bool QtiShmMutex::try_lock() {
    return pthread_mutex_trylock(&mutex) != 0;
}

void QtiShmMutex::unlock() {
    pthread_mutex_unlock(&mutex);
}
};
