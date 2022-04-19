/******************************************************************************
  @file    Perf.cpp
  @brief   Android performance HAL module

  DESCRIPTION

  ---------------------------------------------------------------------------

  Copyright (c) 2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
******************************************************************************/

#include <dlfcn.h>
#include <pthread.h>
#include <cutils/properties.h>
#include <utils/Log.h>
#include <hidl/HidlTransportSupport.h>
#include <hwbinder/IPCThreadState.h>
#include "Perf.h"
#include "mp-ctl.h"
#include <cutils/properties.h>
#include "MpctlUtils.h"
#include <stdio.h>
#include <stdlib.h>

#define MEM_2GB 2*1024*1024
#define MEM_8GB 8*1024*1024
enum INDEX_REFERENCE {INDEX_CLIENT_TID=0, INDEX_CLIENT_PID =1};

namespace vendor {
namespace qti {
namespace hardware {
namespace perf {
namespace V2_0 {
namespace implementation {

// Methods from ::vendor::qti::hardware::perf::V2_0::IPerf follow.
Return<int32_t> Perf::perfLockAcquire(int32_t pl_handle, uint32_t duration, const hidl_vec<int32_t>& boostsList, const hidl_vec<int32_t>& reserved) {
    mpctl_msg_t pMsg;
    uint32_t retVal = -1;
    memset(&pMsg, 0, sizeof(mpctl_msg_t));

    pMsg.req_type = MPCTL_CMD_PERFLOCKACQ;
    for (uint32_t i = 0; i < reserved.size(); i++) {
        switch (i) {
            case INDEX_CLIENT_TID: pMsg.client_tid = reserved[i];
                    break;
            case INDEX_CLIENT_PID: pMsg.client_pid = reserved[i];
                    break;
            default: ALOGE("Unknown params");
                    break;
        }
    }
    uint32_t size  = boostsList.size();
    if (size > MAX_ARGS_PER_REQUEST) {
       ALOGE("Maximum number of arguments alowed exceeded");
       return retVal;
    }
    std::copy(boostsList.begin(), boostsList.end(), pMsg.pl_args);
    pMsg.data = size;
    pMsg.pl_time = duration;
    pMsg.pl_handle = pl_handle;

    retVal = mImpl.PerfGlueLayerSubmitRequest(&pMsg);
    return retVal;
}

Return<void> Perf::perfLockRelease(int32_t pl_handle, const hidl_vec<int32_t>& reserved) {
    mpctl_msg_t pMsg;
    memset(&pMsg, 0, sizeof(mpctl_msg_t));

    pMsg.req_type = MPCTL_CMD_PERFLOCKREL;
    for (uint32_t i = 0; i < reserved.size(); i++) {
        switch (i) {
            case INDEX_CLIENT_TID: pMsg.client_tid = reserved[i];
                    break;
            case INDEX_CLIENT_PID: pMsg.client_pid = reserved[i];
                    break;
            default: ALOGE("Unknown params");
                    break;
        }
    }
    pMsg.pl_handle = pl_handle;

    mImpl.PerfGlueLayerSubmitRequest(&pMsg);
    return Void();
}

Return<int32_t> Perf::perfHint(uint32_t hint, const hidl_string& userDataStr, int32_t userData1, int32_t userData2, const hidl_vec<int32_t>& reserved) {
    mpctl_msg_t pMsg;
    uint32_t retVal = -1;
    memset(&pMsg, 0, sizeof(mpctl_msg_t));

    pMsg.req_type = MPCTL_CMD_PERFLOCKHINTACQ;
    for (uint32_t i = 0; i < reserved.size(); i++) {
        switch (i) {
            case INDEX_CLIENT_TID: pMsg.client_tid = reserved[i];
                    break;
            case INDEX_CLIENT_PID: pMsg.client_pid = reserved[i];
                    break;
            default: ALOGE("Unknown params");
                    break;
        }
    }
    pMsg.hint_id = hint;
    pMsg.pl_time = userData1;
    pMsg.hint_type = userData2;
    strlcpy(pMsg.usrdata_str, userDataStr.c_str(), MAX_MSG_APP_NAME_LEN);

    retVal = mImpl.PerfGlueLayerSubmitRequest(&pMsg);
    return retVal;
}

Return<int32_t> Perf::perfProfile(int32_t pl_handle, int32_t profile, int32_t reserved) {
    mpctl_msg_t pMsg;
    uint32_t retVal = -1;
    memset(&pMsg, 0, sizeof(mpctl_msg_t));

    pMsg.req_type = MPCTL_CMD_PERFLOCK_PROFILE;
    pMsg.client_pid = android::hardware::IPCThreadState::self()->getCallingPid();
    pMsg.client_tid = reserved;
    pMsg.pl_handle = pl_handle;
    pMsg.profile = profile;

    retVal = mImpl.PerfGlueLayerSubmitRequest(&pMsg);
    return retVal;
}

Return<void> Perf::perfLockCmd(int32_t cmd, int32_t reserved) {
    mpctl_msg_t pMsg;
    memset(&pMsg, 0, sizeof(mpctl_msg_t));

    pMsg.req_type = cmd;
    pMsg.client_pid = android::hardware::IPCThreadState::self()->getCallingPid();
    pMsg.client_tid = reserved;

    mImpl.PerfGlueLayerSubmitRequest(&pMsg);
    return Void();
}

Return<void> Perf::perfGetProp(const hidl_string& propName, const hidl_string& defaultVal, perfGetProp_cb _hidl_cb) {
    char prop_val[PROP_VAL_LENGTH], prop_name[PROP_VAL_LENGTH];
    char trace_buf[TRACE_BUF_SZ];
    char *retVal;
    strlcpy(prop_name, propName.c_str(), PROP_VAL_LENGTH);
    retVal = mPerfDataStore.GetProperty(prop_name, prop_val, sizeof(prop_val));

    if (retVal != NULL) {
         if (perf_debug_output) {
             snprintf(trace_buf, TRACE_BUF_SZ, "perfGetProp: Return Val from Perf.cpp is %s",retVal);
             ALOGE("%s", trace_buf);
         }
        _hidl_cb(prop_val);
    } else {
        _hidl_cb(defaultVal);
    }
    return Void();
}

Return<int32_t> Perf::perfSetProp(const hidl_string& propName, const hidl_string& value) {
    // TODO implement
    return int32_t {};
}

Return<int32_t> Perf::perfAsyncRequest(int32_t cmd, const hidl_string& userDataStr, const hidl_vec<int32_t>& params) {
    // TODO implement
    return int32_t {};
}

Return<void> Perf::perfSyncRequest(int32_t cmd, const hidl_string& userDataStr, const hidl_vec<int32_t>& params, perfSyncRequest_cb _hidl_cb) {
    // TODO implement
    return Void();
}

Perf::Perf() {
    Init();
}

Perf::~Perf() {
    mImpl.PerfGlueLayerExit();
}

int Perf::getMemTotal(void) {
    FILE *memInfo = fopen("/proc/meminfo", "r");
    if (memInfo == NULL) {
        return -1;
    }
    char line[256];
    while (fgets(line, sizeof(line), memInfo)) {
        int memTotal = -1;
        char* end = NULL;
        char* token = strtok_r(line, ":", &end);
        if ((end != NULL) && (token != NULL) && (strncmp(token, "MemTotal", 8) == 0)) {
            token = strtok_r(NULL, " ", &end);
            if ((end != NULL) && (token != NULL)) {
                memTotal = strtol(token, &end, 10);
                fclose(memInfo);
                return memTotal;
            }
        }
    }
    fclose(memInfo);
    return -1;
}

void Perf::Init() {
    char trace_prop[PROPERTY_VALUE_MAX];

    //init PerfConfigStore
    mPerfDataStore.ConfigStoreInit();

    //register mp-ctl
    mImpl.LoadPerfLib("libqti-perfd.so");

    char property[PROP_VAL_LENGTH] = "";

    perfGetProp("vendor.debug.enable.lm", "false", [&property](const auto &res) { strlcpy(property,res.c_str(),PROP_VAL_LENGTH);});
    bool enableLM = (!strncmp(property,"false",PROP_VAL_LENGTH))? false : true;
    if(enableLM) {
        ALOGE("LM enabled : Loading liblearningmodule.so");
        mImpl.LoadPerfLib("liblearningmodule.so");
    }

    perfGetProp("vendor.debug.enable.memperfd", "false", [&property](const auto &res) { strlcpy(property,res.c_str(),PROP_VAL_LENGTH);});
    bool enableMemperfd = (!strncmp(property,"false",PROP_VAL_LENGTH))? false : true;
    if(enableLM && enableMemperfd) {
        int memTotal = getMemTotal();
        if (memTotal > MEM_2GB && memTotal <= MEM_8GB) {
            mImpl.LoadPerfLib("libmemperfd.so");
        }
    }

    /* Enable systraces by adding vendor.debug.trace.perf=1 into build.prop */
    if (property_get(PROP_NAME, trace_prop, NULL) > 0) {
        if (trace_prop[0] == '1') {
            perf_debug_output = PERF_SYSTRACE = atoi(trace_prop);
        }
    }

    //init all modules
    mImpl.PerfGlueLayerInit();
}

}  // namespace implementation
}  // namespace V2_0
}  // namespace perf
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
