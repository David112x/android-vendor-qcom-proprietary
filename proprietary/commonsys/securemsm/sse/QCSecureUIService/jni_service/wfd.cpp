/*
 * Copyright (c) 2012-2013, 2018-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#undef LOG_TAG

#include <jni.h>
#include <stdint.h>
#include <vendor/display/config/1.0/IDisplayConfig.h>

#include "com_qualcomm_qti_services_secureui_WfdListener.h"
#include "com_qualcomm_qti_services_secureui_SecureUIService.h"

using vendor::display::config::V1_0::IDisplayConfig;

extern "C" jint
Java_com_qualcomm_qti_services_secureui_WfdListener_pause(
    JNIEnv * env, jclass cls, jint doPause) {
    return Java_com_qualcomm_qti_services_secureui_SecureUIService_pauseExternal(
        env, cls, (jint) IDisplayConfig::DisplayType::DISPLAY_VIRTUAL, doPause);
}
