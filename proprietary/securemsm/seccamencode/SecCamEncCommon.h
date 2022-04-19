/**
 * Copyright (c) 2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef SEC_CAM_ENC_COMMON_H
#define SEC_CAM_ENC_COMMON_H

#include <utils/Log.h>

#undef LOG_TAG
#define LOG_TAG "SecCamEnc"

#define BAIL_OUT()                                        \
    {                                                     \
        LOGE("Error at %s : %d", __FUNCTION__, __LINE__); \
        goto exit;                                        \
    }

#define ENTER() LOGD("%s: enter", __FUNCTION__)

#define EXIT() LOGD("%s: exit ret %d", __FUNCTION__, ret)

#define CHECK_COND(cond) \
    if ((cond)) {        \
        BAIL_OUT();      \
    }

#define ERROR(err_code) \
    ret = err_code;     \
    BAIL_OUT();

#define CHECK_COND_ERROR(cond, err_code) \
    if ((cond)) {                        \
        ret = err_code;                  \
        BAIL_OUT();                      \
    }

#define LOG_MSG(fmt, ...) \
    LOGE("SCE_ERROR %s::%d " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define ERROR(rv, ...)    \
    LOG_MSG(__VA_ARGS__); \
    goto exit;

#define CHECK_COND_MSG(cond, error_code, ...) \
    if (cond) {                               \
        ret = error_code;                     \
        ERROR(rv, __VA_ARGS__);               \
    }

#define NUM_ELEMS(x) (sizeof(x) / sizeof((x)[0]))

#define SCE_LOCK_MUTEX(mutex)                                                 \
    {                                                                         \
        CHECK_COND_MSG(!mutex, SCE_ERROR_COPY_FAILED, "mutex is null");       \
        CHECK_COND_MSG(pthread_mutex_lock(mutex) != 0, SCE_ERROR_COPY_FAILED, \
                       "locking mutex failed");                               \
    }

#define SCE_UNLOCK_MUTEX(mutex)                                          \
    {                                                                    \
        CHECK_COND_MSG(!mutex, SCE_ERROR_COPY_FAILED, "mutex is null");  \
        CHECK_COND_MSG(pthread_mutex_unlock(mutex) != 0,                 \
                       SCE_ERROR_COPY_FAILED, "unlocking mutex failed"); \
    }

#endif /* SEC_CAM_ENC_COMMON_H */
