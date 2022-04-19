/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2015-2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/
#define LOG_TAG "LocSvc_HIDL_utils_jni"
#define LOG_NDEBUG 0

#include "utils_jni.h"
#include <android_runtime/AndroidRuntime.h>
#include <vendor/qti/gnss/1.1/ILocHidlGnss.h>
#include "utils/Log.h"

using namespace android;
using ::vendor::qti::gnss::V1_0::LocHidlLocationFlagsBits;

class VMAssociator {

private:
    JNIEnv* mJNIEnv = nullptr;

public:
    VMAssociator() {

        JavaVM* javaVm = AndroidRuntime::getJavaVM();
        JavaVMAttachArgs args = {
             JNI_VERSION_1_6,
             "IzatSvc Callback Thread",
             NULL
        };
        jint result = javaVm->AttachCurrentThread(&mJNIEnv, &args);
        if (result != 0) {
            ALOGE("Callback thread attach error %d", result);
        } else {
            ALOGV("Callback thread attached to VM, jniEnv %p", mJNIEnv);
        }
    }

    ~VMAssociator() {

        JavaVM* javaVm = AndroidRuntime::getJavaVM();
        jint result = javaVm->DetachCurrentThread();
        if (result != 0) {
             ALOGE("Callback thread detach error %d", result);
        }
    }

    JNIEnv* getJNIEnv() {
        if (AndroidRuntime::getJNIEnv() != mJNIEnv) {
            ALOGE("Environment mismatch !");
            return NULL;
        }
        return mJNIEnv;
    }
};

thread_local std::unique_ptr<VMAssociator> sVMAssociator;

JNIEnv* getCallbackEnv() {

    JNIEnv* env = AndroidRuntime::getJNIEnv();

    if (env == nullptr || sVMAssociator == nullptr) {
        sVMAssociator.reset(new VMAssociator());
        env = sVMAssociator->getJNIEnv();
    }
    return env;
}

void checkAndClearExceptionsFromCallback(JNIEnv *env, const char* methodName)
{
    if (env->ExceptionCheck()) {
        ALOGE("An exception was thrown from callback '%s'", methodName);
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}

uint16_t remapHidlFlags(uint16_t inFlags) {
    uint16_t resFlags = 0;
    resFlags |= (inFlags & LOC_GPS_LOCATION_HAS_LAT_LONG) ?
            (uint16_t) LocHidlLocationFlagsBits::LAT_LONG_BIT : 0;
    resFlags |= (inFlags & LOC_GPS_LOCATION_HAS_ALTITUDE) ?
            (uint16_t) LocHidlLocationFlagsBits::ALTITUDE_BIT : 0;
    resFlags |= (inFlags & LOC_GPS_LOCATION_HAS_SPEED) ?
            (uint16_t) LocHidlLocationFlagsBits::SPEED_BIT : 0;
    resFlags |= (inFlags & LOC_GPS_LOCATION_HAS_BEARING) ?
            (uint16_t) LocHidlLocationFlagsBits::BEARING_BIT : 0;
    resFlags |= (inFlags & LOC_GPS_LOCATION_HAS_ACCURACY) ?
            (uint16_t) LocHidlLocationFlagsBits::ACCURACY_BIT : 0;
    resFlags |= (inFlags & LOC_GPS_LOCATION_HAS_VERT_UNCERTAINITY) ?
            (uint16_t) LocHidlLocationFlagsBits::VERTICAL_ACCURACY_BIT : 0;
    resFlags |= (inFlags & LOC_GPS_LOCATION_HAS_SPEED_ACCURACY) ?
            (uint16_t) LocHidlLocationFlagsBits::SPEED_ACCURACY_BIT : 0;
    resFlags |= (inFlags & LOC_GPS_LOCATION_HAS_BEARING_ACCURACY) ?
            (uint16_t) LocHidlLocationFlagsBits::BEARING_ACCURACY_BIT : 0;

    return resFlags;
}
