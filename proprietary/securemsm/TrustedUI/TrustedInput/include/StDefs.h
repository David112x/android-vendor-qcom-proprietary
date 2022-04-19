/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef STDEFS_H
#define STDEFS_H

#include <string>

using namespace std;

// TODO Replace this with some dynamic logging approach
#define LOGV(...) do { } while (0)  // Comment this line to enable LOGV
//#define LOGD(...) do { } while (0)  // Comment this line to enable LOGD

#define TUI_ERROR(error_code)                                  \
    ret = (error_code);                                        \
    LOGE("%s::%d err=%x (%s)", __func__, __LINE__, error_code, \
         strerror(error_code));                                \
    goto end;

#define TUI_CHECK_ERR(cond, error_code) \
    if (!(cond)) {                      \
        TUI_ERROR(error_code)           \
    }

#define TUI_CHECK(cond)                     \
    if (!(cond)) {                          \
        LOGE("%s::%d", __func__, __LINE__); \
        goto end;                           \
    }

static std::string locations[] = {
#if defined(ST_TARGET_MSM8953)
    "/sys/devices/soc/78b7000.i2c/i2c-3/3-004b",
#elif defined(ST_TARGET_THORIUM)
    "/sys/devices/soc/78b7000.i2c/i2c-3/3-0038",
    "/sys/devices/soc/78b7000.i2c/i2c-3/3-0020",
#elif defined(ST_TARGET_MSM8974)
    "/sys/devices/f9924000.i2c/i2c-2/2-004a",
#elif defined(ST_TARGET_APQ8084)
    "/sys/devices/f9966000.i2c/i2c-1/1-004a",
    "/sys/devices/f9966000.i2c/i2c-1/1-0020",
#elif defined(ST_TARGET_MSM8916)
    "/sys/devices/soc.0/78b9000.i2c/i2c-5/5-0020",
    "/sys/devices/soc.0/78b9000.i2c/i2c-5/5-0020/input/input2",
#elif defined(ST_TARGET_MSM8994)
    "/sys/devices/soc.0/f9924000.i2c/i2c-2/2-0020/input/input0",
    "/sys/devices/soc.0/f9924000.i2c/i2c-2/2-004a",
#elif defined(ST_TARGET_MSM8996)
    "/sys/devices/soc/75ba000.i2c/i2c-12/12-0020",
    "/sys/devices/soc/75ba000.i2c/i2c-12/12-004a",
#elif defined(ST_TARGET_MSM8952)
    "/sys/devices/soc.0/78b9000.i2c/i2c-5/5-0020",
#elif defined(ST_TARGET_MSM8998)
    "/sys/devices/soc/c179000.i2c/i2c-5/5-0020",
#elif defined(ST_TARGET_SDM660)
    "/sys/devices/soc/c178000.i2c/i2c-4/4-0020",
#elif defined(ST_TARGET_SDM845)
    "/sys/devices/platform/soc/a98000.i2c/i2c-2/2-0020",
#elif defined(ST_TARGET_SDM710)
    "/sys/devices/platform/soc/a84000.i2c/i2c-2/2-0020",
#elif defined(ST_TARGET_TS_LOOPBACK)
    "/data/vendor/ts_loopback",
#endif
};

#endif  // STDEFS_H
