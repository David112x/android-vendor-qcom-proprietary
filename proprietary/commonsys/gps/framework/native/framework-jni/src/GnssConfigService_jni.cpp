/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/
#define LOG_TAG "LocSvc_HIDL_GnssConfigService_jni"
#define LOG_NDEBUG 0

#include <inttypes.h>
#include <vector>
#include <string>
#include <nativehelper/JNIHelp.h>
#include "jni.h"
#include "android_runtime/AndroidRuntime.h"
#include "android_runtime/Log.h"
#include "utils_jni.h"
#include <vendor/qti/gnss/2.1/ILocHidlGnss.h>
#include <vendor/qti/gnss/2.1/ILocHidlGnssConfigServiceCallback.h>

using namespace std;
using namespace android;

using ::vendor::qti::gnss::V2_1::ILocHidlGnss;
using ::vendor::qti::gnss::V2_1::ILocHidlGnssConfigService;
using ::vendor::qti::gnss::V2_1::ILocHidlGnssConfigServiceCallback;

using ::android::hardware::gnss::V1_0::GnssConstellationType;

using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::hidl::base::V1_0::IBase;
using ::android::sp;

static inline void CheckExceptions(JNIEnv* env, const char* methodName) {

    if(!env->ExceptionCheck()) {
        return;
    }
    ALOGE("An exception was thrown by '%s'.", methodName);
    LOGE_EX(env);
    env->ExceptionClear();
}

/* References to required java methods */
static jobject sGnssConfigServiceObj = NULL;
static jmethodID sOnGnssSvTypeConfigCb = NULL;

/* ===================================================================
 *   HIDL Interface
 * ===================================================================*/
static sp<ILocHidlGnssConfigService> gnssConfigServiceIface = nullptr;
static sp<ILocHidlGnssConfigServiceCallback> gnssConfigServiceCb = nullptr;

/* ===========================================================
 *   HIDL Callbacks : ILocHidlGnssConfigServiceCallback.hal
 * ===========================================================*/
struct GnssConfigServiceCallback : public ILocHidlGnssConfigServiceCallback {

    Return<void> getGnssSvTypeConfigCb(
            const hidl_vec<GnssConstellationType>& disabledSvTypeList) override;
};

static void GnssConfigService_init(JNIEnv* env, jobject obj);

static sp<ILocHidlGnssConfigService>& getGnssConfigIface() {
    if (gnssConfigServiceIface == nullptr) {
        ALOGD("Get GnssVendor Service.");
        auto gnssVendorHal = ILocHidlGnss::getService(GNSS_VENDOR_SERVICE_INSTANCE);
        if (gnssVendorHal == nullptr) {
            ALOGE("Unable to get GnssVendor service\n");
        } else {
            ALOGD("Get GnssConfigService Extension");
            auto gcs = gnssVendorHal->getExtensionLocHidlGnssConfigService_2_1();
            if (!gcs.isOk()) {
                ALOGE("Unable to get a handle to GnssConfigService extension !");
            } else {
                gnssConfigServiceIface = gcs;
                ALOGD("Got GnssConfigService Extension.");
            }
        }
    }

    return gnssConfigServiceIface;
}

static void GnssConfigService_class_init(JNIEnv* env, jclass clazz)
{
    ALOGD("GnssConfigService_class_init");

    // get references to Java methods
    sOnGnssSvTypeConfigCb = env->GetMethodID(clazz,
                                         "onGnssSvTypeConfigCb",
                                         "([I)V");

    /* Now fetch HIDL interface */
    getGnssConfigIface();

    registerDeathRecipient("GnssConfigService", []() {
            gnssConfigServiceIface = nullptr;
            ALOGE("ILocHidlGnssConfigService died");
            GnssConfigService_init(nullptr, nullptr);
    });
}

static void GnssConfigService_init(JNIEnv* env, jobject obj)
{
    ALOGD("GnssConfigService_init");

    auto iface = getGnssConfigIface();
    if (iface == nullptr) {
        ALOGE("GnssConfigService Iface NULL !");
        return;
    }

    if (sGnssConfigServiceObj == NULL) {
        if (nullptr != env) {
            sGnssConfigServiceObj = env->NewGlobalRef(obj);
        }
    }

    if (gnssConfigServiceCb == nullptr) {
        gnssConfigServiceCb = new GnssConfigServiceCallback();
    }

    TO_HIDL_SERVICE();
    auto r = iface->init(gnssConfigServiceCb);
    if (!r.isOk() || r == false) {
        ALOGE("Error invoking HIDL API [%s]", r.description().c_str());
    }
}

static GnssConstellationType intToGnssConstellation(int32_t svTypeInt) {

    switch(svTypeInt) {
    case 1:
        return GnssConstellationType::GPS;
    case 2:
        return GnssConstellationType::SBAS;
    case 3:
        return GnssConstellationType::GLONASS;
    case 4:
        return GnssConstellationType::QZSS;
    case 5:
        return GnssConstellationType::BEIDOU;
    case 6:
        return GnssConstellationType::GALILEO;
    default:
        return GnssConstellationType::UNKNOWN;
    }
}

static int32_t gnssConstellationToInt(GnssConstellationType svType) {

    switch(svType) {
    case GnssConstellationType::GPS:
        return 1;
    case GnssConstellationType::SBAS:
        return 2;
    case GnssConstellationType::GLONASS:
        return 3;
    case GnssConstellationType::QZSS:
        return 4;
    case GnssConstellationType::BEIDOU:
        return 5;
    case GnssConstellationType::GALILEO:
        return 6;
    default:
        return 0;
    }
}

static string getSvTypeStr(GnssConstellationType type) {

    switch(type) {
    case GnssConstellationType::GPS:
        return "GPS";
    case GnssConstellationType::SBAS:
        return "SBAS";
    case GnssConstellationType::GLONASS:
        return "GLO";
    case GnssConstellationType::QZSS:
        return "QZSS";
    case GnssConstellationType::BEIDOU:
        return "BDS";
    case GnssConstellationType::GALILEO:
        return "GAL";
    default:
        return "INVALID";
    }
}
static void printSvTypeVec(const char* msg,
        const hidl_vec<GnssConstellationType>& disabledSvTypeVec) {

    string printStr;
    for (GnssConstellationType type: disabledSvTypeVec) {
        switch(type) {
        case GnssConstellationType::GPS:
        case GnssConstellationType::SBAS:
        case GnssConstellationType::GLONASS:
        case GnssConstellationType::QZSS:
        case GnssConstellationType::BEIDOU:
        case GnssConstellationType::GALILEO:
            printStr.append(getSvTypeStr(type));
            printStr.append("(DISABLED) ");
            break;
        default:
            // Negated value = enabled
            GnssConstellationType negatedType = GnssConstellationType::UNKNOWN;
            if ((negatedType = intToGnssConstellation((uint8_t)(~(int32_t)type))) !=
                    GnssConstellationType::UNKNOWN) {
                printStr.append(getSvTypeStr(negatedType));
                printStr.append("(ENABLED) ");
            } else {
                printStr.append(" INVALID ");
            }
            break;
        }
    }
    ALOGV("%s [ %s]", msg, printStr.c_str());
}

static void populateDisabledSvTypeVec(
        JNIEnv* env, const jintArray& disabledSvTypeArray,
        std::vector<GnssConstellationType>& disabledSvTypeVec) {

    jint* svTypeArray = env->GetIntArrayElements(disabledSvTypeArray, 0);
    int svTypeArrayLen = env->GetArrayLength(disabledSvTypeArray);

    disabledSvTypeVec.clear();
    for (int index=0; index < svTypeArrayLen; index++) {
        GnssConstellationType type = intToGnssConstellation(svTypeArray[index]);
        if (GnssConstellationType::UNKNOWN != type) {
            disabledSvTypeVec.push_back(type);
        } else {
            type = intToGnssConstellation((uint8_t)(~svTypeArray[index]));
            if (GnssConstellationType::UNKNOWN != type) {
                // Enabled SV Types are pushed as negative numbers in the disabled vector
                disabledSvTypeVec.push_back(
                        (GnssConstellationType)svTypeArray[index]);
            } else {
                ALOGE("%s] Invalid SV Type : 0x%x", __FUNCTION__, svTypeArray[index]);
            }
        }
    }
}

static int validGnssConstellationTypeCount(
        const hidl_vec<GnssConstellationType>& disabledSvTypeVec) {

    int validSvTypes = 0;
    for (GnssConstellationType type: disabledSvTypeVec) {
        if (0 != gnssConstellationToInt(type) ||
                GnssConstellationType::UNKNOWN !=
                        intToGnssConstellation((uint8_t)(~(int32_t)type))) {
            validSvTypes++;
        }
    }
    return validSvTypes;
}

static void populateDisabledSvTypeArray(
        JNIEnv* env, const hidl_vec<GnssConstellationType>& disabledSvTypeVec,
        jintArray& disabledSvTypeArray) {

    int count = disabledSvTypeVec.size();
    int arrIdx = 0;
    for (int index=0; index < count; index++) {
        jint disabledSvType = gnssConstellationToInt(disabledSvTypeVec[index]);
        if (0 != disabledSvType) {
            ALOGV("%s] Disabled SV Type : %d", __FUNCTION__, disabledSvType);
            env->SetIntArrayRegion(disabledSvTypeArray, (int)arrIdx++, 1, &disabledSvType);
        } else {
            jint enabledSvType = (int32_t)disabledSvTypeVec[index];
            if (GnssConstellationType::UNKNOWN !=
                    intToGnssConstellation((uint8_t)~enabledSvType)) {
                ALOGV("%s] Enabled SV Type : %d", __FUNCTION__, (uint8_t)~enabledSvType);
                env->SetIntArrayRegion(disabledSvTypeArray, (int)arrIdx++, 1, &enabledSvType);
            } else {
                ALOGE("%s] Invalid SV Type: %d", __FUNCTION__, (int32_t)disabledSvTypeVec[index]);
            }
        }
    }
}

static void GnssConfigService_getGnssSvTypeConfig(JNIEnv* env, jobject /*obj*/)
{
    ALOGV("%s", __FUNCTION__);

    auto gnssConfigIface = getGnssConfigIface();
    if (gnssConfigIface == nullptr) {
        ALOGE("GnssConfigService Iface NULL !");
        return;
    }

    TO_HIDL_SERVICE();
    auto r = gnssConfigIface->getGnssSvTypeConfig();
    if (!r.isOk()) {
        ALOGE("Error invoking HIDL API [%s]", r.description().c_str());
        return;
    }
}

static void GnssConfigService_setGnssSvTypeConfig(JNIEnv* env, jobject /*obj*/,
        jintArray disabledSvTypeArray)
{
    ALOGV("%s", __FUNCTION__);

    auto gnssConfigIface = getGnssConfigIface();
    if (gnssConfigIface == nullptr) {
        ALOGE("GnssConfigService Iface NULL !");
        return;
    }

    std::vector<GnssConstellationType> disabledSvTypeVec;
    populateDisabledSvTypeVec(env, disabledSvTypeArray, disabledSvTypeVec);
    printSvTypeVec("Set Enabled/Disabled SV Types: ", disabledSvTypeVec);

    TO_HIDL_SERVICE();
    auto r = gnssConfigIface->setGnssSvTypeConfig(disabledSvTypeVec);
    if (!r.isOk()) {
        ALOGE("Error invoking HIDL API [%s]", r.description().c_str());
        return;
    }
}

static void GnssConfigService_resetGnssSvTypeConfig(JNIEnv* /*env*/, jobject /*obj*/)
{
    ALOGV("%s", __FUNCTION__);

    auto gnssConfigIface = getGnssConfigIface();
    if (gnssConfigIface == nullptr) {
        ALOGE("GnssConfigService Iface NULL !");
        return;
    }

    TO_HIDL_SERVICE();
    auto r = gnssConfigIface->resetGnssSvTypeConfig();
    if (!r.isOk()) {
        ALOGE("Error invoking HIDL API [%s]", r.description().c_str());
        return;
    }
}

Return<void> GnssConfigServiceCallback::getGnssSvTypeConfigCb(
        const hidl_vec<GnssConstellationType>& disabledSvTypeVec) {

    FROM_HIDL_SERVICE();
    printSvTypeVec("Got Enabled/Disabled SV Types: ", disabledSvTypeVec);

    JNIEnv* env = getCallbackEnv();
    if (env == NULL) {
        ALOGE("JNIEnv NULL");
        return Void();
    }

    int validCount = validGnssConstellationTypeCount(disabledSvTypeVec);
    ALOGD("Disabled SV Type List size: %zu valid count %d",
            disabledSvTypeVec.size(), validCount);

    jintArray disabledSvTypeArray = (jintArray)env->NewIntArray(validCount);
    if (disabledSvTypeArray == NULL) {
        ALOGD("Failed to allocate array for disabled sv types");
        return Void();
    }

    if (validCount > 0) {
        populateDisabledSvTypeArray(env, disabledSvTypeVec, disabledSvTypeArray);
    }

    env->CallVoidMethod(sGnssConfigServiceObj,
                        sOnGnssSvTypeConfigCb,
                        disabledSvTypeArray);
    CheckExceptions(env, __FUNCTION__);

    if(disabledSvTypeArray != NULL) {
        env->DeleteLocalRef(disabledSvTypeArray);
    }

    return Void();
}

static JNINativeMethod sMethods[] = {
    /* name, signature, funcPtr */
    {"native_gnssconfig_class_init",
     "()V",
     reinterpret_cast<void*>(GnssConfigService_class_init)},
    {"native_gnssconfig_init",
     "()V",
     reinterpret_cast<void*>(GnssConfigService_init)},
    {"native_gnssconfig_getGnssSvTypeConfig",
     "()V",
     reinterpret_cast<void*>(GnssConfigService_getGnssSvTypeConfig)},
    {"native_gnssconfig_setGnssSvTypeConfig",
     "([I)V",
     reinterpret_cast<void*>(GnssConfigService_setGnssSvTypeConfig)},
    {"native_gnssconfig_resetGnssSvTypeConfig",
     "()V",
     reinterpret_cast<void*>(GnssConfigService_resetGnssSvTypeConfig)}
};

int register_GnssConfigService(JNIEnv* env)
{
    ALOGD("register_GnssConfigService");
    return jniRegisterNativeMethods(env,
                                    "com/qualcomm/location/izat/gnssconfig/GnssConfigService",
                                    sMethods,
                                    NELEM(sMethods));
}

