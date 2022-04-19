/*******************************************************************************

 Copyright (c) 2015-2017 Qualcomm Technologies, Inc.
 All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.

 ---------------------------------------------------------------------------

 @file    qti-activity_trigger.cpp
 @brief   Android activity trigger library
 DESCRIPTION

 ---------------------------------------------------------------------------
*******************************************************************************/

#define LOG_TAG "ANDR-PERF-ACT-TRIG"
#include <cutils/log.h>
#include <cutils/properties.h>
#include <string.h>

#include "mp-ctl.h"
#include "client.h"
#include "atClient.h"

///////////////////////////////////////////////////////////////////////////////////
///activity trigger apis
///////////////////////////////////////////////////////////////////////////////////
extern "C" void activity_trigger_init(void) {
    QLOGI("Activity trigger library initialized successfully");
}

extern "C" void activity_trigger_start(const char *name, int *reserved) {
    QLOGI("App trigger starting '%s'", (name) ? name : "<null>");

    if (!name)
        return;

    QLOGI("perf:calling NotifyFgAppChange");
    perf_hint(VENDOR_HINT_ACTIVITY_START, name, -1, -1);
}

extern "C" void activity_trigger_qspm_start(const char *name, int *reserved) {

    if (!name)
        return;

#ifdef QVA
    perf_hint_qspm(VENDOR_HINT_ACTIVITY_START, name, -1, -1);
#endif
}

extern "C" void activity_trigger_resume(const char *name) {

    /* Do the same as activity start */
    if (!name)
        return;

    perf_hint(VENDOR_HINT_ACTIVITY_RESUME, name, -1, -1);
}

extern "C" void activity_trigger_pause(const char *name) {
    /* Do the same as activity start */
    if (!name)
        return;

    perf_hint(VENDOR_HINT_ACTIVITY_PAUSE, name, -1, -1);
}

extern "C" void activity_trigger_stop(const char *name) {
    /* Do the same as activity start */
    if (!name)
        return;

    perf_hint(VENDOR_HINT_ACTIVITY_STOP, name, -1, -1);
}

extern "C" void activity_trigger_misc(int function, const char *name, int type, int flag, float *scale) {
    //do nothing
}

extern "C" void activity_trigger_deinit(void) {
}
