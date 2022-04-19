/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/
#include <loc_pla.h>
#include <log_util.h>
#include <GardenUtil.h>

namespace garden {

uint64_t getRealTimeNsec()
{
    struct timespec ts;

    ts.tv_sec = ts.tv_nsec = 0;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

uint64_t getUpTimeSec()
{
    struct timespec ts;

    ts.tv_sec = ts.tv_nsec = 0;
    clock_gettime(CLOCK_BOOTTIME, &ts);
    return ts.tv_sec + (ts.tv_nsec / 1000000000LL);
}

int timeval_difference (struct timeval *result, struct timeval *x, struct timeval *y)
{
    if (x->tv_usec < y->tv_usec) {
        int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
        y->tv_usec -= 1000000 * nsec;
        y->tv_sec += nsec;
    }
    if (x->tv_usec - y->tv_usec > 1000000) {
        int nsec = (x->tv_usec - y->tv_usec) / 1000000;
        y->tv_usec += 1000000 * nsec;
        y->tv_sec -= nsec;
    }

    result->tv_sec = x->tv_sec - y->tv_sec;
    result->tv_usec = x->tv_usec - y->tv_usec;

    return x->tv_sec < y->tv_sec;
}

} // namespace garden
