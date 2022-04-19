/*============================================================================
* Copyright (c) 2017 Qualcomm Technologies, Inc.                             *
* All Rights Reserved.                                                       *
* Confidential and Proprietary - Qualcomm Technologies, Inc.                 *
* ===========================================================================*/
#include "mutex_cond.h"
#include <stdint.h>

#if defined(_POSIX_VERSION) || defined(__arm__)

#define NSECS_IN_SEC 1000000000LL
#define NSECS_IN_MS  1000000LL

int set_timeout(struct timespec *ts, int timeout_ms)
{
    int err = 0;
    int64_t t;
    if (ts == NULL) {
        return -1;
    }
    err = clock_gettime(CLOCK_REALTIME, ts);
    if (err != 0) {
        return err;
    }

    t = ((int64_t) ts->tv_sec * NSECS_IN_SEC) + ((int64_t)ts->tv_nsec);
    t += (int64_t)timeout_ms * NSECS_IN_MS;
    ts->tv_nsec = (long) (t % NSECS_IN_SEC);
    ts->tv_sec = (time_t) (t / NSECS_IN_SEC);

    return 0;
}

#endif /* _POSIX_VERSION */
