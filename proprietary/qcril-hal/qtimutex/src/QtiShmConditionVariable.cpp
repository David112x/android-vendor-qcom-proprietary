/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <QtiMutex.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>

namespace qtimutex {
    QtiShmConditionVariable::QtiShmConditionVariable() {
        pthread_condattr_t attrs;
        int rc = 0;
        pthread_condattr_init(&attrs);
        rc = pthread_condattr_setpshared(&attrs, PTHREAD_PROCESS_SHARED);
        if (rc) {
            abort();
        }
        rc = pthread_condattr_setclock(&attrs, CLOCK_MONOTONIC);
        pthread_cond_init(&condVar, &attrs);
    }

    QtiShmConditionVariable::~QtiShmConditionVariable() {
        pthread_cond_destroy(&condVar);
    }

    void QtiShmConditionVariable::notify_one() noexcept {
        pthread_cond_signal(&condVar);
    }

    void QtiShmConditionVariable::notify_all() noexcept {
        pthread_cond_broadcast(&condVar);
    }

};
