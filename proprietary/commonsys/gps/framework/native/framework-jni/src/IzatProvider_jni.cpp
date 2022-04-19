/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2015-2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/
#define LOG_TAG "LocSvc_HIDL_IzatProvider_jni"
#define LOG_NDEBUG 0

#include <string>
#include <jni.h>
#include <nativehelper/JNIHelp.h>
#include "utils_jni.h"
#include <android_runtime/AndroidRuntime.h>
#include <utils/Log.h>
#include <map>
#include <inttypes.h>

#include <vendor/qti/gnss/1.0/ILocHidlGnss.h>
#include <vendor/qti/gnss/1.0/ILocHidlIzatProvider.h>
#include <vendor/qti/gnss/1.0/ILocHidlIzatProviderCallback.h>

using ::vendor::qti::gnss::V1_0::ILocHidlGnss;
using ::vendor::qti::gnss::V1_0::ILocHidlIzatProvider;
using ::vendor::qti::gnss::V1_0::ILocHidlIzatProviderCallback;
using ::vendor::qti::gnss::V1_0::LocHidlIzatStreamType;
using ::vendor::qti::gnss::V1_0::LocHidlIzatLocation;
using ::vendor::qti::gnss::V1_0::LocHidlIzatProviderStatus;
using ::vendor::qti::gnss::V1_0::LocHidlIzatRequest;
using ::vendor::qti::gnss::V1_0::LocHidlNetworkPositionSourceType;
using ::vendor::qti::gnss::V1_0::LocHidlIzatHorizontalAccuracy;

using ::android::hardware::hidl_vec;
using ::android::hardware::hidl_string;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::hidl::base::V1_0::IBase;
using ::android::sp;

using namespace android;
using namespace std;

/* ===================================================================
 *   HIDL Interface
 * ===================================================================*/
class IzatProviderGlue;

static sp<ILocHidlGnss> sGnssVendorHal;
static sp<IzatProviderGlue> sNetworkProvider;
static sp<IzatProviderGlue> sFusedProvider;
static sp<IzatProviderGlue> sNullProvider;

static jmethodID method_onLocationChanged;
static jmethodID method_onStatusChanged;

static void onJavaStaticLoad(JNIEnv* /*env*/, jclass /*clazz*/);

static sp<ILocHidlGnss>& getVendorHal() {
    if (sGnssVendorHal == nullptr) {
        // HIDL Service
        sGnssVendorHal = ILocHidlGnss::getService(GNSS_VENDOR_SERVICE_INSTANCE);
        if (sGnssVendorHal == nullptr) {
            ALOGE("Unable to get GnssVendor service\n");
        } else {
            ALOGD("Got GnssVendor Service.");
        }
    }

    return sGnssVendorHal;
}

inline static int providerStatusLookUp(const LocHidlIzatProviderStatus status) {
    switch (status) {
    case LocHidlIzatProviderStatus::GNSS_STATUS_SESSION_BEGIN:
        return 4; // GNSS_SESSION_BEGIN
    case LocHidlIzatProviderStatus::GNSS_STATUS_SESSION_END:
        return 5; // GNSS_SESSION_END
    case LocHidlIzatProviderStatus::GNSS_STATUS_ENGINE_ON:
        return 6; // GNSS_ENGINE_ON
    case LocHidlIzatProviderStatus::GNSS_STATUS_ENGINE_OFF:
        return 7; // GNSS_ENGINE_OFF
    default:
        return -1;
    }
}

/* ===========================================================
 *   HIDL Callbacks : ILocHidlIzatProviderCallback.hal
 * ===========================================================*/
class IzatProviderGlue : public ILocHidlIzatProviderCallback {
    enum State {DISABLED, ENABLED, STARTED};
    const LocHidlIzatStreamType mStreamType;
    const jobject mJavaProvider;
    const string mTag;
    sp<IzatProviderGlue>& mHolder;
    State mState;
    sp<ILocHidlIzatProvider> mNativeProvider;
    Mutex mLock;
    const std::function<void ()> mDeathRecipient;
    LocHidlIzatRequest mLastRequest;

    void initLowerHalf() {
        auto r = (mStreamType == LocHidlIzatStreamType::NETWORK) ?
                sGnssVendorHal->getExtensionLocHidlIzatNetworkProvider() :
                sGnssVendorHal->getExtensionLocHidlIzatFusedProvider();
        if (r.isOk()) {
            mNativeProvider = r;
            TO_HIDL_SERVICE();
            auto r3 = mNativeProvider->init(mHolder);
            if (!r3.isOk() || (r3 == false)) {
                ALOGE("%s: Error invoking HIDL API [%s]",
                      __func__, r.description().c_str());
                mNativeProvider = nullptr;
            }
        }
    }

    inline sp<ILocHidlIzatProvider>& getNativeProvider() {
        Mutex::Autolock autoLock(mLock);
        return mNativeProvider;
    }

public:
    inline IzatProviderGlue(LocHidlIzatStreamType streamType, jobject javaProvider,
                            string&& tag, sp<IzatProviderGlue>& holder) :
            mStreamType(streamType), mJavaProvider(javaProvider), mTag(tag), mHolder(holder),
            mState(DISABLED), mNativeProvider(nullptr), mLock(),
            mDeathRecipient([this]() {
                    {
                        Mutex::Autolock autoLock(mLock);
                        mNativeProvider = nullptr;
                        sGnssVendorHal = nullptr;
                        getVendorHal();
                        initLowerHalf();
                    }
                    if (mState != DISABLED) {
                        setEnabled(mState != DISABLED);
                        if (STARTED == mState) {
                            auto r = mNativeProvider->onAddRequest(mLastRequest);
                            if (!r.isOk() || (r == false)) {
                                ALOGE("%s: Error invoking HIDL API [%s]",
                                      __func__, r.description().c_str());
                            }
                        }
                    }
                }) {
        mHolder = this;
        registerDeathRecipient(mTag, mDeathRecipient);
        initLowerHalf();
    }

    inline void onAddRequest(jint numUpdates, jlong interval,
                             jfloat smallestDisplacement, jint accuracy) {
        mLastRequest.provider = mStreamType;
        mLastRequest.numUpdates = numUpdates;
        mLastRequest.timeIntervalBetweenFixes = interval;
        mLastRequest.smallestDistanceBetweenFixes = smallestDisplacement;
        mLastRequest.suggestedHorizontalAccuracy =
                static_cast<LocHidlIzatHorizontalAccuracy>(accuracy);
        ALOGD ("Provider Type: %d; Num Updates: %d; Interval: %" PRId64
               "Displacement: %f; Accuracy: %d", mStreamType, numUpdates,
               interval, smallestDisplacement, accuracy);

        auto nativeProvider = getNativeProvider();
        if (nativeProvider != nullptr) {
            TO_HIDL_SERVICE();
            auto r = nativeProvider->onAddRequest(mLastRequest);
            mState = STARTED;
            if (!r.isOk() || (r == false)) {
                ALOGE("%s: Error invoking HIDL API [%s]",
                      __func__, r.description().c_str());
            }
        } else {
            ALOGW("%s: nativeProvider is null", __func__);
        }
    }

    inline void onRemoveRequest() {
        auto nativeProvider = getNativeProvider();
        if (nativeProvider != nullptr) {
            TO_HIDL_SERVICE();
            auto r = nativeProvider->onRemoveRequest(mLastRequest);
            mState = ENABLED;
            if (!r.isOk() || (r == false)) {
                ALOGE("%s: Error invoking HIDL API [%s]",
                      __func__, r.description().c_str());
            }
        } else {
            ALOGW("%s: nativeProvider is null", __func__);
        }
    }

    inline ~IzatProviderGlue() {
        deregisterDeathRecipient(mTag);
        JNIEnv* env = android::AndroidRuntime::getJNIEnv();
        if (env != NULL && mJavaProvider != NULL) {
            env->DeleteGlobalRef(mJavaProvider);
        }
    }

    inline void setEnabled(bool isEnabled) {
        auto nativeProvider = getNativeProvider();
        if (nativeProvider != nullptr) {
            TO_HIDL_SERVICE();
            auto r = isEnabled ? nativeProvider->onEnable() :
                    nativeProvider->onDisable();
            if (!r.isOk() || (r == false)) {
                ALOGE("%s: Error invoking HIDL API [%s]",
                      __func__, r.description().c_str());
            } else if (DISABLED == mState) {
                mState = ENABLED;
            }
        } else {
            ALOGW("%s: nativeProvider is null", __func__);
        }
    }

    Return<void> onLocationChanged(const LocHidlIzatLocation& location) override {
        FROM_HIDL_SERVICE();
        if (location.hasLatitude && location.hasLongitude && location.hasHorizontalAccuracy &&
            STARTED == mState && mJavaProvider && method_onLocationChanged) {

            JNIEnv* env = android::AndroidRuntime::getJNIEnv();
            if (env != NULL) {
                jlong utcTime = location.utcTimestampInMsec;
                jlong elapsedRealTimeNanos = location.elapsedRealTimeInNanoSecs;
                jdouble latitude = location.latitude;
                jdouble longitude = location.longitude;
                jboolean hasAltitude = location.hasAltitudeWrtEllipsoid;
                jdouble altitude = location.altitudeWrtEllipsoid;
                jboolean hasVerticalUnc = location.hasVertUnc;
                jfloat verticalUnc = location.vertUnc;
                jboolean hasSpeed = location.hasSpeed;
                jfloat speed = location.speed;
                jboolean hasSpeedUnc = location.hasSpeedUnc;
                jfloat speedUnc = location.speedUnc;
                jboolean hasBearing = location.hasBearing;
                jfloat bearing = location.bearing;
                jboolean hasBearingUnc = location.hasBearingUnc;
                jfloat bearingUnc = location.bearingUnc;
                jboolean hasAccuracy = location.hasHorizontalAccuracy;
                jfloat accuracy = location.horizontalAccuracy;
                jshort positionSource = -1;

                if (location.hasNetworkPositionSource) {
                    switch (location.networkPositionSource) {
                        case LocHidlNetworkPositionSourceType::WIFI:
                            positionSource = 1;
                            break;
                        case LocHidlNetworkPositionSourceType::CELL:
                            positionSource = 0;
                            break;
                    }
                }

                env->CallVoidMethod(mJavaProvider, method_onLocationChanged,
                                    utcTime, elapsedRealTimeNanos, latitude,
                                    longitude, hasAltitude, altitude, hasVerticalUnc,
                                    verticalUnc, hasSpeed, speed,
                                    hasSpeedUnc, speedUnc,
                                    hasBearing, bearing,
                                    hasBearingUnc, bearingUnc,
                                    hasAccuracy, accuracy, positionSource);
            }
        }
        return Void();
    }

    Return<void> onStatusChanged(LocHidlIzatProviderStatus status) {
        FROM_HIDL_SERVICE();
        JNIEnv* env = android::AndroidRuntime::getJNIEnv();
        jint javaStatus = providerStatusLookUp(status);
        if (mStreamType == LocHidlIzatStreamType::FUSED && mJavaProvider &&
            method_onStatusChanged && env && javaStatus != -1) {
            env->CallVoidMethod(mJavaProvider, method_onStatusChanged, javaStatus);
        }
        return Void();
    }
};

inline static sp<IzatProviderGlue>&
providerTypeToNativeObj(jint providerType, JNIEnv* env, jobject obj) {
    sp<IzatProviderGlue>& provider = sNullProvider;

    auto type = static_cast<LocHidlIzatStreamType>(providerType);
    switch (type) {
    case LocHidlIzatStreamType::FUSED:
        if (sFusedProvider == nullptr) {
            new IzatProviderGlue(type, env->NewGlobalRef(obj), "FusedProvider", sFusedProvider);
        }
        provider = sFusedProvider;
        break;
    case LocHidlIzatStreamType::NETWORK:
        if (sNetworkProvider == nullptr) {
            new IzatProviderGlue(type, env->NewGlobalRef(obj), "NetworkProvider", sNetworkProvider);
        }
        provider = sNetworkProvider;
        break;
    default:
        provider = sNullProvider;
        break;
    }

    return provider;
}

// Java to native calls
static void onJavaClassLoad(JNIEnv* env, jclass clazz) {
    method_onLocationChanged = env->GetMethodID(clazz,
                                                "onLocationChanged", "(JJDDZDZFZFZFZFZFZFS)V");
    method_onStatusChanged = env->GetMethodID(clazz,
                                              "onStatusChanged", "(I)V");
    // Setting the values of LocHidlIzatStreamType::NETWORK / LocHidlIzatStreamType::FUSED
    env->SetStaticIntField(clazz,
                           env->GetStaticFieldID(clazz, "IZAT_STREAM_NETWORK", "I"),
                           static_cast<int>(LocHidlIzatStreamType::NETWORK));
    env->SetStaticIntField(clazz,
                           env->GetStaticFieldID(clazz, "IZAT_STREAM_FUSED", "I"),
                           static_cast<int>(LocHidlIzatStreamType::FUSED));
    env->SetStaticIntField(clazz,
                           env->GetStaticFieldID(clazz, "IZAT_HORIZONTAL_FINE", "I"),
                           static_cast<int>(LocHidlIzatHorizontalAccuracy::FINE));
    env->SetStaticIntField(clazz,
                           env->GetStaticFieldID(clazz, "IZAT_HORIZONTAL_BLOCK", "I"),
                           static_cast<int>(LocHidlIzatHorizontalAccuracy::BLOCK));
    env->SetStaticIntField(clazz,
                           env->GetStaticFieldID(clazz, "IZAT_HORIZONTAL_NONE", "I"),
                           static_cast<int>(LocHidlIzatHorizontalAccuracy::NONE));
}

static void onJavaStaticLoad(JNIEnv* /*env*/, jclass /*clazz*/) {
    sFusedProvider = nullptr;
    sNetworkProvider = nullptr;
    sGnssVendorHal = nullptr;
    getVendorHal();
}

static void onEnable(JNIEnv* env, jobject obj, jint providerType) {
    sp<IzatProviderGlue>& provider = providerTypeToNativeObj(providerType, env, obj);

    if (provider != nullptr) {
        provider->setEnabled(true);
    }
}

static void onDisable(JNIEnv* env, jobject obj, jint providerType) {
    sp<IzatProviderGlue>& provider = providerTypeToNativeObj(providerType, env, obj);

    if (provider != nullptr) {
        provider->setEnabled(false);
    }
}

static void onAddRequest(JNIEnv* env, jobject obj, jint providerType,
                         jint numUpdates, jlong interval,
                         jfloat smallestDisplacement, jint accuracy) {
    sp<IzatProviderGlue>& provider = providerTypeToNativeObj(providerType, env, obj);

    if (provider != nullptr) {
        provider->onAddRequest(numUpdates, interval, smallestDisplacement, accuracy);
    }
}

static void onRemoveRequest(JNIEnv* env, jobject obj, jint providerType,
                            jint /* numUpdates */, jlong /* interval */,
                            jfloat /* smallestDisplacement */, jint /* accuracy */) {
    sp<IzatProviderGlue>& provider = providerTypeToNativeObj(providerType, env, obj);

    if (provider != nullptr) {
        provider->onRemoveRequest();
    }
}

static JNINativeMethod sMethods[] = {
    {"nativeOnClassLoad", "()V", (void*)onJavaClassLoad},
    {"nativeOnStaticLoad", "()V", (void*)onJavaStaticLoad},
    {"nativeOnEnableProvider","(I)V", (void*)onEnable},
    {"nativeOnDisableProvider","(I)V", (void*)onDisable},
    {"nativeOnAddRequest","(IIJFI)V", (void*)onAddRequest},
    {"nativeOnRemoveRequest", "(IIJFI)V", (void*)onRemoveRequest}
};


int register_IzatProvider(JNIEnv* env) {
    return jniRegisterNativeMethods(env,
                                    "com/qualcomm/location/izatprovider/IzatProvider",
                                    sMethods, NELEM(sMethods));
}
