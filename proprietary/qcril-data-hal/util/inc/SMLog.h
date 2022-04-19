/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <android/log.h>
#include <framework/Log.h>

#ifdef QCRIL_LOG_DEBUG
#define TAG "RILQ_SM"
#define SM_LOGD(str) QCRIL_LOG_DEBUG("%s", string(str).c_str())
#define SM_LOGE(str) QCRIL_LOG_DEBUG("%s", string(str).c_str())
#else
#define SM_LOGD(str) if (SMLog::DBG) __android_log_print(ANDROID_LOG_DEBUG, SMLog::LOGTAG, "%s", string(str).c_str())
#define SM_LOGE(str) __android_log_print(ANDROID_LOG_ERROR, SMLog::LOGTAG, "%s", string(str).c_str())
#endif

namespace SM {

class SMLog {
public:
    static constexpr bool DBG = true;
    static constexpr const char *LOGTAG = "SM";
};

}
