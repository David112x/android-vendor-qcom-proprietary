/******************************************************************************
  @file    client.cpp
  @brief   Android performance iop library

  DESCRIPTION

  ---------------------------------------------------------------------------

  Copyright (c) 2014-2015,2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <vector>
#include <shared_mutex>

#define LOG_TAG         "ANDR-IOP"

#include <log/log.h>
#include <cutils/properties.h>
#include "client.h"
#include "io-p.h"
#include <vendor/qti/hardware/iop/2.0/IIop.h>

#include "io-prefetch/list_capture.h"

#if defined(ANDROID_JELLYBEAN)
#include "common_log.h"
#endif

#define FAILED    -1
#define SUCCESS   0

using android::hardware::Return;
using android::hardware::Void;
using android::hardware::hidl_vec;
using android::hardware::hidl_string;
using vendor::qti::hardware::iop::V2_0::IIop;
using ::android::sp;
using ::android::wp;
using android::hardware::hidl_death_recipient;
using android::hidl::base::V1_0::IBase;
using std::unique_lock;
using std::shared_lock;
using std::shared_timed_mutex;

static sp<IIop> gIopHal = NULL;
static shared_timed_mutex gIop_lock;


struct IopDeathRecipient : virtual public hidl_death_recipient
{
    virtual void serviceDied(uint64_t /*cookie*/, const wp<IBase>& /*who*/) override {
        unique_lock<shared_timed_mutex> write_lock(gIop_lock);
        gIopHal = NULL;
        ALOGE("IIop serviceDied");
    }
};
static sp<IopDeathRecipient> iopDeathRecipient = NULL;

static void getIopServiceAndLinkToDeath() {
    shared_lock<shared_timed_mutex> read_lock(gIop_lock);

    if (gIopHal == NULL) {
        read_lock.unlock();
        {
            unique_lock<shared_timed_mutex> write_lock(gIop_lock);
            if (gIopHal == NULL) {
                gIopHal = IIop::tryGetService();
                if (gIopHal != NULL) {
                    iopDeathRecipient = new IopDeathRecipient();
                    android::hardware::Return<bool> linked = gIopHal->linkToDeath(iopDeathRecipient, 0);
                    if (!linked || !linked.isOk()) {
                        gIopHal = NULL;
                        ALOGE("Unable to link to gIopHal death notifications!");
                    }
                } else {
                    ALOGE("IIop:: Iop HAL Service is not available.");
                }
            }
        }
    }
}

static int processIntReturn(const Return<int32_t> &intReturn, const char* funcName) {
    int ret = -1;
    if (!intReturn.isOk()) {
        unique_lock<shared_timed_mutex> write_lock(gIop_lock);
        gIopHal = NULL;
        ALOGE("%s() failed: iop HAL service not available.", funcName);
    } else {
        ret = intReturn;
    }
    return ret;
}

static void processVoidReturn(const Return<void> &voidReturn, const char* funcName) {
    if (!voidReturn.isOk()) {
        unique_lock<shared_timed_mutex> write_lock(gIop_lock);
        gIopHal = NULL;
        ALOGE("%s() failed: iop HAL service not available.", funcName);
    }
}

int perf_io_prefetch_start(int pid, const char *pkg_name, const char *code_path)
{
    int rc = -1;
    getIopServiceAndLinkToDeath();
    {
        shared_lock<shared_timed_mutex> read_lock(gIop_lock);
        if (gIopHal != NULL) {
            Return<int32_t> intReturn = gIopHal->iopStart(pid, pkg_name, code_path);
            read_lock.unlock();
            rc = processIntReturn(intReturn, "perf_io_prefetch_start");
        }
    }
    return rc;
}


int perf_io_prefetch_stop()
{
    getIopServiceAndLinkToDeath();
    {
        shared_lock<shared_timed_mutex> read_lock(gIop_lock);
        if (gIopHal != NULL) {
            Return<void> voidReturn = gIopHal->iopStop();
            read_lock.unlock();
            processVoidReturn(voidReturn, "perf_io_prefetch_stop");
        }
        else {
            return FAILED;
        }
    }
    return SUCCESS;
}

int perf_ux_engine_events(int opcode, int pid, const char *pkg_name, int lat)
{
    int rc = -1;
    getIopServiceAndLinkToDeath();
    {
        shared_lock<shared_timed_mutex> read_lock(gIop_lock);
        if (gIopHal != NULL) {
            Return<int32_t> intReturn = gIopHal->uxEngine_events(opcode, pid, pkg_name, lat);
            read_lock.unlock();
            rc = processIntReturn(intReturn, "perf_ux_engine_events");
        }
    }
    return rc;
}

void perf_ux_engine_trigger(int opcode, char *preferred_apps)
{
    char *out = (char *)malloc(PREFERRED_OUT_LENGTH * sizeof(char));
    if (out != NULL) {
        out[0] = '\0';
        getIopServiceAndLinkToDeath();
        {
            shared_lock<shared_timed_mutex> read_lock(gIop_lock);
            if (gIopHal != NULL) {
                Return<void> voidReturn = gIopHal->uxEngine_trigger(opcode, [&out](const auto &res) { strlcpy(out,res.c_str(),PREFERRED_OUT_LENGTH);});
                read_lock.unlock();
                processVoidReturn(voidReturn, "perf_ux_engine_trigger");
            }
        }
        strlcpy(preferred_apps, out, PREFERRED_OUT_LENGTH );
        free(out);
    }
}
