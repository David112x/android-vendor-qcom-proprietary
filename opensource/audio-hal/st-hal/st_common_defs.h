/* st_common_defs.h
 *
 * Copyright (c) 2016-2019, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef ST_COMMON_DEFS_H
#define ST_COMMON_DEFS_H

#include <stdbool.h>
#include <dlfcn.h>

/* #define VERY_VERBOSE_LOGGING */
#ifdef VERY_VERBOSE_LOGGING
#define ALOGVV ALOGV
#else
#define ALOGVV(a...) do { } while(0)
#endif

#define NSECS_PER_MSEC (1000000LL)
#define NSECS_PER_SEC (1000000000LL)

#define SOUND_TRIGGER_APE_BUFFER_DURATION_MS (480)
#define SOUND_TRIGGER_PCM_BUFFER_DURATION_MS (160)

#define SOUND_TRIGGER_CPE_LAB_DRV_BUF_DURATION_MS (240)
#define ST_GRAPHITE_LAB_BUF_DURATION_MS (480)

#define SOUND_TRIGGER_SAMPLING_RATE_16000 (16000)
#define SOUND_TRIGGER_SAMPLING_RATE_48000 (48000)
#define SOUND_TRIGGER_SAMPLING_RATE_384000 (384000)

#define SOUND_TRIGGER_CHANNEL_MODE_MONO (1)
#define SOUND_TRIGGER_CHANNEL_MODE_STEREO (2)
#define SOUND_TRIGGER_CHANNEL_MODE_TRI (3)
#define SOUND_TRIGGER_CHANNEL_MODE_QUAD (4)
#define SOUND_TRIGGER_CHANNEL_MODE_HEX (6)
#define SOUND_TRIGGER_CHANNEL_MODE_OCT (8)

#define SOUND_TRIGGER_BIT_WIDTH  (16)

#define ST_READ_WAIT_TIME_OUT_SEC (2)

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define SET_BIT(a, b) (a |= b)
#define CLEAR_BIT(a, b) (a &= ~(b))
#define CHECK_BIT(a, b) ((a) & (b))

#define SET_STATE(a, b) SET_BIT(a, b)
#define CLEAR_STATE(a, b) CLEAR_BIT(a, b)
#define CHECK_STATE(a, b) CHECK_BIT(a, b)

#define ALIGN(number, align) \
        ((number + align - 1) & ~(align - 1))
#define CALCULATE_PERIOD_SIZE(duration_ms, sample_rate, period_cnt, align) \
       (ALIGN(((sample_rate * duration_ms) /(period_cnt * 1000)), align))

#define GET_WAIT_TIMESPEC(timeout, wait_time) \
do {\
    clock_gettime(CLOCK_MONOTONIC, &timeout); \
    timeout.tv_sec += wait_time / NSECS_PER_SEC; \
    timeout.tv_nsec += wait_time % NSECS_PER_SEC; \
    if (timeout.tv_nsec >= NSECS_PER_SEC) { \
        timeout.tv_sec += 1; \
        timeout.tv_nsec -= NSECS_PER_SEC; \
    } \
} while(0)

/* fwk mode definitions */
#define SOUND_TRIGGER_EVENT_NON_TIME_STAMP_MODE (0)
#define SOUND_TRIGGER_EVENT_TIME_STAMP_MODE (1)

#define DEFAULT_CUSTOM_CONFIG_MINOR_VERSION (2)
#define CUSTOM_CONFIG_OPAQUE_DATA_SIZE (12)
#define MAX_LIB_HANDLE_SIZE (64)
#define KW_LEN_WARNING (500)
#define PREROLL_LEN_WARNING (150)
#define GENERIC_DET_EVENT_HEADER_SIZE (8)
#define FIRST_STAGE_KW_START_TOLERANCE_MS (300)
#define FIRST_STAGE_KW_END_TOLERANCE_MS (240)
#define CNN_DATA_AFTER_TRUE_KW_END_MS (200)
#define GENERIC_DET_EVENT_USER_LEVEL_OFFSET (17)
#define GCS_NON_GENERIC_USER_LEVEL_OFFSET (3)
#define LSM_NON_GENERIC_USER_LEVEL_OFFSET (1)
#define GENERIC_DET_EVENT_KW_START_OFFSET (3)
#define GENERIC_DET_EVENT_KW_END_OFFSET (4)
#define CONF_LEVELS_INTF_VERSION_0002 (0x02)

#define KEY_ID_CONFIDENCE_LEVELS (0x2000)
#define KEY_ID_KEYWORD_POSITION_STATS (0x2001)

#define DLSYM(handle, ptr, symbol, err) \
do {\
    err = 0;\
    ptr = dlsym(handle, #symbol);\
    if (ptr == NULL) {\
        ALOGW("%s: %s not found. %s", __func__, #symbol, dlerror());\
        err = -ENODEV;\
    }\
} while(0)

typedef enum {
    ST_DEVICE_HW_NONE,
    ST_DEVICE_HW_APE,
    ST_DEVICE_HW_CPE,
    ST_DEVICE_HW_ARM
}st_hw_type_t;

typedef enum st_exec_mode {
    ST_EXEC_MODE_NONE = -1,
    ST_EXEC_MODE_ADSP,
    ST_EXEC_MODE_CPE,
    ST_EXEC_MODE_ARM,
    ST_EXEC_MODE_MAX
} st_exec_mode_t;

/* defines possible configuration modes for
   execution mode that can be selecet through
   config file */
typedef enum st_exec_mode_config {
   EXEC_MODE_CFG_APE,
   EXEC_MODE_CFG_CPE,
   EXEC_MODE_CFG_DYNAMIC,
   EXEC_MODE_CFG_ARM
} st_exec_mode_config_t;

#endif /* ST_COMMON_DEFS_H */
