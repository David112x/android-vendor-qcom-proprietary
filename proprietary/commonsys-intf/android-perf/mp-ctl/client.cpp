/******************************************************************************
  @file    client.cpp
  @brief   Android performance PerfLock library

  DESCRIPTION

  ---------------------------------------------------------------------------

  Copyright (c) 2014,2017 Qualcomm Technologies, Inc.
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

#define LOG_TAG         "ANDR-PERF"
#include <log/log.h>
#include <cutils/properties.h>
#include "MpctlUtils.h"
#include "client.h"
#include "mp-ctl.h"
#include <vendor/qti/hardware/perf/2.0/IPerf.h>

#define FAILED                  -1
#define SUCCESS                 0

using android::hardware::Return;
using android::hardware::Void;
using android::hardware::hidl_death_recipient;
using android::hardware::hidl_vec;
using android::hardware::hidl_string;
using android::hidl::base::V1_0::IBase;
using vendor::qti::hardware::perf::V2_0::IPerf;
using ::android::sp;
using ::android::wp;
using std::unique_lock;
using std::shared_lock;
using std::shared_timed_mutex;

static sp<IPerf> gPerfHal = NULL;
static shared_timed_mutex gPerf_lock;

struct PerfDeathRecipient : virtual public hidl_death_recipient
{
    virtual void serviceDied(uint64_t /*cookie*/, const wp<IBase>& /*who*/) override {
        unique_lock<shared_timed_mutex> write_lock(gPerf_lock);
        gPerfHal = NULL;
        ALOGE("IPerf serviceDied");
    }
};
static sp<PerfDeathRecipient> perfDeathRecipient = NULL;

static void getPerfServiceAndLinkToDeath() {
    shared_lock<shared_timed_mutex> read_lock(gPerf_lock);
    char trace_prop[PROPERTY_VALUE_MAX];

    if (gPerfHal == NULL) {
        read_lock.unlock();
        {
            unique_lock<shared_timed_mutex> write_lock(gPerf_lock);
            if (gPerfHal == NULL) {
                gPerfHal = IPerf::tryGetService();
                if (gPerfHal != NULL) {
                    perfDeathRecipient = new PerfDeathRecipient();
                    android::hardware::Return<bool> linked = gPerfHal->linkToDeath(perfDeathRecipient, 0);
                    if (!linked || !linked.isOk()) {
                        gPerfHal = NULL;
                        ALOGE("Unable to link to gPerfHal death notifications!");
                    }
                } else {
                    QLOGE("IPerf:: Perf HAL Service is not available.");
                }
            }
        }
    }
}

static int processIntReturn(const Return<int32_t> &intReturn, const char* funcName) {
    int ret = -1;
    if (!intReturn.isOk()) {
        unique_lock<shared_timed_mutex> write_lock(gPerf_lock);
        gPerfHal = NULL;
        ALOGE("%s() failed: perf HAL service not available.", funcName);
    } else {
        ret = intReturn;
    }
    return ret;
}

static void processVoidReturn(const Return<void> &voidReturn, const char* funcName) {
    if (!voidReturn.isOk()) {
        unique_lock<shared_timed_mutex> write_lock(gPerf_lock);
        gPerfHal = NULL;
        ALOGE("%s() failed: perf HAL service not available.", funcName);
    }
}

int perf_lock_acq(int handle, int duration, int list[], int numArgs)
{
    int rc = -1;
    std::vector<int32_t> boostsList;
    std::vector<int32_t> paramList;

    if (duration < 0 || numArgs <= 0 || numArgs > MAX_ARGS_PER_REQUEST)
        return FAILED;

    paramList.push_back(gettid());
    paramList.push_back(getpid());
    boostsList.assign(list, (list + numArgs));
    getPerfServiceAndLinkToDeath();
    {
        shared_lock<shared_timed_mutex> read_lock(gPerf_lock);
        if (gPerfHal != NULL) {
            Return<int32_t> intReturn = gPerfHal->perfLockAcquire(handle, duration, boostsList, paramList);
            read_lock.unlock();
            rc = processIntReturn(intReturn, "perfLockAcquire");
        }
    }
    return rc;
}

int perf_lock_rel(int handle)
{
    QLOGI("inside perf_lock_rel of client");
    if (handle == 0)
        return FAILED;

    std::vector<int32_t> paramList;
    paramList.push_back(gettid());
    paramList.push_back(getpid());

    getPerfServiceAndLinkToDeath();
    {
        shared_lock<shared_timed_mutex> read_lock(gPerf_lock);
        if (gPerfHal != NULL) {
            Return<void> voidReturn = gPerfHal->perfLockRelease(handle, paramList);
            read_lock.unlock();
            processVoidReturn(voidReturn, "perfLockRelease");
            return SUCCESS;
        }
    }
    return FAILED;
}

int perf_hint(int hint, const char *pkg, int duration, int type)
{
    int rc = -1;

    QLOGI("inside perf_hint of client");

    std::vector<int32_t> paramList;
    paramList.push_back(gettid());
    paramList.push_back(getpid());

    getPerfServiceAndLinkToDeath();
    {
        shared_lock<shared_timed_mutex> read_lock(gPerf_lock);
        if (gPerfHal != NULL) {
            Return<int32_t> intReturn = gPerfHal->perfHint(hint, pkg, duration, type, paramList);
            read_lock.unlock();
            rc = processIntReturn(intReturn, "perfHint");
        }
    }
    return rc;
}

int perf_get_feedback(int req, const char *pkg) {
    int rc = -1;

    QLOGI("inside perf_hint of client");

    std::vector<int32_t> paramList;
    paramList.push_back(gettid());
    paramList.push_back(getpid());

    getPerfServiceAndLinkToDeath();
    {
        shared_lock<shared_timed_mutex> read_lock(gPerf_lock);
        if (gPerfHal != NULL) {
            Return<int32_t> intReturn = gPerfHal->perfHint(req, pkg, 0, 0, paramList);
            read_lock.unlock();
            rc = processIntReturn(intReturn, "perfHint");
        }
    }
    return rc;
}

int perf_lock_use_profile(int handle, int profile)
{
    int rc = -1;
    pid_t client_tid = 0;

    QLOGI("inside perf_lock_use_profile of client");
    client_tid = gettid();
    getPerfServiceAndLinkToDeath();
    {
        shared_lock<shared_timed_mutex> read_lock(gPerf_lock);
        if (gPerfHal != NULL) {
            Return<int32_t> intReturn = gPerfHal->perfProfile(handle, profile, client_tid);
            read_lock.unlock();
            rc = processIntReturn(intReturn, "perfProfile");
        }
    }
    return rc;

}

void perf_lock_cmd(int cmd)
{
    pid_t client_tid = 0;

    QLOGI("inside perf_lock_cmd of client");
    client_tid = gettid();
    getPerfServiceAndLinkToDeath();
    {
        shared_lock<shared_timed_mutex> read_lock(gPerf_lock);
        if (gPerfHal != NULL) {
            Return<void> voidReturn = gPerfHal->perfLockCmd(cmd, client_tid);
            read_lock.unlock();
            processVoidReturn(voidReturn, "perfLockCmd");
        }
    }
}

PropVal perf_get_prop(const char *prop , const char *def_val) {

    pid_t client_tid = 0;
    PropVal return_value;
    return_value.value[0] = '\0';
    char *out = (char *)malloc(PROP_VAL_LENGTH * sizeof(char));
    if(out != NULL) {
        out[0] = '\0';
        client_tid = gettid();

        getPerfServiceAndLinkToDeath();
        {
            shared_lock<shared_timed_mutex> read_lock(gPerf_lock);
            if (gPerfHal != NULL) {
                Return<void> voidReturn = gPerfHal->perfGetProp(prop, def_val, [&out](const auto &res) { strlcpy(out,res.c_str(),PROP_VAL_LENGTH);});
                read_lock.unlock();
                processVoidReturn(voidReturn, "perfGetProp");
            } else if (def_val != NULL) {
                strlcpy(out, def_val, PROP_VAL_LENGTH);
            }
        }

        strlcpy(return_value.value, out, PROP_VAL_LENGTH);
        QLOGI("VALUE RETURNED FOR PROPERTY %s :: %s",prop,return_value.value);
        free(out);
    } else if (def_val != NULL) {
        strlcpy(return_value.value, def_val, PROP_VAL_LENGTH);
    }
    return return_value;
}

PropVal perf_wait_get_prop(const char *prop , const char *def_val) {

    static sp<IPerf> gPerfHal_new = NULL;
    gPerfHal_new = IPerf::getService();
    pid_t client_tid = 0;
    PropVal return_value;
    return_value.value[0] = '\0';
    char *out = (char *)malloc(PROP_VAL_LENGTH * sizeof(char));

    if (out != NULL) {
        out[0] = '\0';
        client_tid = gettid();
        if (gPerfHal_new != NULL) {
           Return<void> voidReturn = gPerfHal_new->perfGetProp(prop, def_val, [&out](const auto &res) { strlcpy(out,res.c_str(),PROP_VAL_LENGTH);});
        } else if (def_val != NULL) {
           strlcpy(out, def_val, PROP_VAL_LENGTH);
        }
        strlcpy(return_value.value, out, PROP_VAL_LENGTH);
        QLOGI("VALUE RETURNED FOR PROPERTY %s :: %s ",prop,return_value.value);
        free(out);
    } else if (def_val != NULL) {
        strlcpy(return_value.value, def_val, PROP_VAL_LENGTH);
    }
    return return_value;
}
