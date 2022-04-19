/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#define LOG_TAG "LocSvc_HIDL_WWANDBProvider_jni"
#define LOG_NDEBUG 0
#define LOCATION_CLASS_NAME "android/location/Location"

#include <inttypes.h>
#include <nativehelper/JNIHelp.h>
#include "jni.h"
#include "android_runtime/AndroidRuntime.h"
#include "android_runtime/Log.h"
#include "utils_jni.h"
#include <vendor/qti/gnss/2.1/ILocHidlGnss.h>


using ::vendor::qti::gnss::V1_0::LocHidlLocation;
using ::vendor::qti::gnss::V1_0::LocHidlLocationFlagsBits;
using ::vendor::qti::gnss::V1_0::LocHidlBatchMode;
using ::vendor::qti::gnss::V1_0::LocHidlBatchStatus;
using ::vendor::qti::gnss::V1_0::LocHidlBatchOptions;
using ::vendor::qti::gnss::V1_0::LocHidlBatchStatusInfo;

using ::vendor::qti::gnss::V2_1::ILocHidlGnss;
using ::vendor::qti::gnss::V2_1::ILocHidlWWANDBProvider;
using ::vendor::qti::gnss::V2_1::ILocHidlWWANDBProviderCallback;
using ::vendor::qti::gnss::V2_1::LocHidlUlpLocation;


using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hidl::base::V1_0::IBase;
using ::android::hardware::Void;
using ::android::sp;
using ::android::hardware::hidl_string;

using namespace android;

static void* sWWANDBProviderHandle = NULL;
static JavaVM* sWWANDBProvCbJavaVm = NULL;
static jobject sCallbacksObj = NULL;

static jmethodID method_onBsObsLocDataAvailable = NULL;
static jmethodID method_onServiceRequest = NULL;

static jclass class_BSObsLocData= NULL;
static jclass class_BSInfo= NULL;


typedef enum
{
  eQMI_LOC_FDCL_AIR_INTERFACE_TYPE_CDMA_V02 = 0, /**<  FDCL CDMA cell  */
  eQMI_LOC_FDCL_AIR_INTERFACE_TYPE_GSM_V02 = 1, /**<  FDCL GSM cell  */
  eQMI_LOC_FDCL_AIR_INTERFACE_TYPE_WCDMA_V02 = 2, /**<  FDCL WCDMA cell  */
  eQMI_LOC_FDCL_AIR_INTERFACE_TYPE_LTE_V02 = 3 /**<  FDCL LTE cell  */
} eQMI_LOC_FDCL_AIR_INTERFACE;

typedef enum
{
    GSM,     /**< GSM cell type. */
    WCDMA,   /**< WCDMA cell type. */
    CDMA,    /**< CDMA cell type. */
    LTE      /**< LTE cell type. */
} IZatCellTypes;


static void convertMac48toStr(char mac_str[], int str_size, uint64_t mac) {
    snprintf(mac_str, str_size, "%012" PRIX64, mac);
}

/* =================================================================
 *   HIDL Data Item Translation APIs
 * =================================================================*/
static void TranslateHidlToObjectLocation(JNIEnv* env, const LocHidlLocation* location,
        jobject& locationObject, uint16_t position_source) {

    jclass locationClass = env->FindClass(LOCATION_CLASS_NAME);
    jmethodID locationCtor = env->GetMethodID(locationClass,
                                                       "<init>",
                                                       "(Ljava/lang/String;)V");
    if (position_source == ULP_LOCATION_IS_FROM_GNSS ) {
        jstring jStrProv = env->NewStringUTF("GPS");
        locationObject = env->NewObject(locationClass, locationCtor, jStrProv);
        env->DeleteLocalRef(jStrProv);
    } else {
        jstring jStrProv = env->NewStringUTF("Hybrid");
        locationObject = env->NewObject(locationClass, locationCtor, jStrProv);
        env->DeleteLocalRef(jStrProv);
    }
    jint flags = remapHidlFlags(location->locationFlagsMask);

    // set the valid information in the object
    if (flags & LocHidlLocationFlagsBits::LAT_LONG_BIT) {
        jmethodID setLatitude = env->GetMethodID(locationClass,
                                                          "setLatitude",
                                                          "(D)V");
        env->CallVoidMethod(locationObject, setLatitude, location->latitude);

        jmethodID setLongitude = env->GetMethodID(locationClass,
                                                           "setLongitude",
                                                           "(D)V");
        env->CallVoidMethod( locationObject, setLongitude, location->longitude);

        jmethodID setTime = env->GetMethodID(locationClass,
                                                      "setTime",
                                                      "(J)V");
        env->CallVoidMethod(locationObject, setTime, location->timestamp);
    }

    if (flags & LocHidlLocationFlagsBits::ALTITUDE_BIT) {
        jmethodID setAltitude = env->GetMethodID(locationClass,
                                                          "setAltitude",
                                                          "(D)V");
        env->CallVoidMethod(locationObject, setAltitude, location->altitude);
    }

    if (flags & LocHidlLocationFlagsBits::SPEED_BIT) {
        jmethodID setSpeed = env->GetMethodID(locationClass,
                                                       "setSpeed",
                                                       "(F)V");
        env->CallVoidMethod(locationObject, setSpeed, location->speed);
    }

    if (flags & LocHidlLocationFlagsBits::BEARING_BIT) {
        jmethodID setBearing = env->GetMethodID(locationClass,
                                                         "setBearing",
                                                         "(F)V");
        env->CallVoidMethod(locationObject, setBearing, location->bearing);
    }

    if (flags & LocHidlLocationFlagsBits::ACCURACY_BIT) {
        jmethodID setAccuracy = env->GetMethodID(locationClass,
                                                          "setAccuracy",
                                                          "(F)V");
        env->CallVoidMethod(locationObject, setAccuracy, location->accuracy);
    }

    if (flags & LocHidlLocationFlagsBits::VERTICAL_ACCURACY_BIT) {
        jmethodID setVerticalAccuracy = env->GetMethodID(locationClass,
                                                            "setVerticalAccuracyMeters",
                                                            "(F)V");
        env->CallVoidMethod(locationObject, setVerticalAccuracy, location->verticalAccuracy);
    }

    if (flags & LocHidlLocationFlagsBits::SPEED_ACCURACY_BIT) {
        jmethodID setSpeedAccuracy = env->GetMethodID(locationClass,
                                                            "setSpeedAccuracyMetersPerSecond",
                                                            "(F)V");
        env->CallVoidMethod(locationObject, setSpeedAccuracy, location->speedAccuracy);
    }

    if (flags & LocHidlLocationFlagsBits::BEARING_ACCURACY_BIT) {
        jmethodID setBearingAccuracy = env->GetMethodID(locationClass,
                                                            "setBearingAccuracyDegrees",
                                                            "(F)V");
        env->CallVoidMethod(locationObject, setBearingAccuracy, location->bearingAccuracy);
    }
    env->DeleteLocalRef(locationClass);
}

static void TranslateHidlToObjectCell(JNIEnv* env,
                                  const ILocHidlWWANDBProviderCallback::LocHidlBsCellInfo& bs,
                                  jobject& bsInfoObject) {
    if (env) {
        if (class_BSInfo) {
            jmethodID jConstructorBSInfo = env->GetMethodID(class_BSInfo,
                                                            "<init>",
                                                            "()V");
            static jfieldID jFieldCellType = env->GetFieldID(class_BSInfo,
                                                             "mCellType", "I");
            static jfieldID jFieldCell1 = env->GetFieldID(class_BSInfo,
                                                          "mCellRegionID1", "I");
            static jfieldID jFieldCell2 = env->GetFieldID(class_BSInfo,
                                                          "mCellRegionID2", "I");;
            static jfieldID jFieldCell3 = env->GetFieldID(class_BSInfo,
                                                          "mCellRegionID3", "I");
            static jfieldID jFieldCell4 = env->GetFieldID(class_BSInfo,
                                                          "mCellRegionID4", "I");
            bsInfoObject = env->NewObject(class_BSInfo, jConstructorBSInfo, NULL);
            if (bsInfoObject) {
                int16_t cell_type = -1;
                switch (bs.cell_type)
                {
                    case eQMI_LOC_FDCL_AIR_INTERFACE_TYPE_GSM_V02:
                        {
                            cell_type = GSM;
                        }
                        break;
                    case eQMI_LOC_FDCL_AIR_INTERFACE_TYPE_WCDMA_V02:
                        {
                            cell_type = WCDMA;
                        }
                        break;
                    case eQMI_LOC_FDCL_AIR_INTERFACE_TYPE_CDMA_V02:
                        {
                            cell_type = CDMA;
                        }
                        break;
                    case eQMI_LOC_FDCL_AIR_INTERFACE_TYPE_LTE_V02:
                        {
                            cell_type = LTE;
                        }
                        break;
                } //end switch

                if (-1 == cell_type) {
                    ALOGV("bs.cell_type invalid: %u cell_type: %d", bs.cell_type, cell_type);
                } else {
                    ALOGV("bs.cell_type valid: %u cell_type: %d bs.cell_id1: %u bs.cell_id2: %u " \
                          "bs.cell_id3: %u bs.cell_id4: %u", bs.cell_type, cell_type, bs.cell_id1,
                          bs.cell_id2, bs.cell_id3, bs.cell_id4);
                    env->SetIntField(bsInfoObject, jFieldCellType, cell_type);
                    env->SetIntField(bsInfoObject, jFieldCell1, bs.cell_id1);
                    env->SetIntField(bsInfoObject, jFieldCell2, bs.cell_id2);
                    env->SetIntField(bsInfoObject, jFieldCell3, bs.cell_id3);
                    env->SetIntField(bsInfoObject, jFieldCell4, bs.cell_id4);
                }
            }
        }
    }
}

static void TranslateHidlToObjectBsObs(JNIEnv* env,
                                  const ILocHidlWWANDBProviderCallback::LocHidlBSObsData& bsObsData,
                                  jobject& jBSObsObject) {

    if (class_BSObsLocData && class_BSInfo) {
        static jfieldID jFieldLocation = env->GetFieldID(class_BSObsLocData, "mLocation",
                                                         "Landroid/location/Location;");
        static jfieldID jFieldCellInfo = env->GetFieldID(class_BSObsLocData, "mCellInfo",
                                                         "Lcom/qti/wwandbreceiver/BSInfo;");
        static jfieldID jFieldScanTimestamp = env->GetFieldID(class_BSObsLocData, "mScanTimestamp",
                                                              "I");

        if (jBSObsObject) {
            if (bsObsData.bUlpLocValid) {
                ALOGD("bsObsData.bUlpLocValid adding location");
                jobject locationObject = NULL;
                TranslateHidlToObjectLocation(env, &bsObsData.gpsLoc.gpsLocation, locationObject,
                                      bsObsData.gpsLoc.position_source);

                if (locationObject) {
                    env->SetObjectField(jBSObsObject, jFieldLocation, locationObject);
                    env->DeleteLocalRef(locationObject);
                }
            } else {
                ALOGD("NOT bsObsData.bUlpLocValid");
            }

            jobject bsInfoObject = NULL;
            TranslateHidlToObjectCell(env, bsObsData.cellInfo, bsInfoObject);

            if (bsInfoObject) {
                env->SetObjectField(jBSObsObject, jFieldCellInfo, bsInfoObject);
                env->DeleteLocalRef(bsInfoObject);
            }

            ALOGD("JNI bsObsData.scanTimestamp_ms: %" PRIu64, bsObsData.scanTimestamp_ms);
            env->SetIntField(jBSObsObject, jFieldScanTimestamp, bsObsData.scanTimestamp_ms / 1000);
        }
    }
}

/* ===========================================================
 *   HIDL Callbacks : ILocHidlWWANDBProviderCallback.hal
 * ===========================================================*/

struct LocHidlWWANDBProviderCallback : public ILocHidlWWANDBProviderCallback {
    Return<void> bsObsLocDataUpdateCallback(
            const hidl_vec<LocHidlBSObsData>& bsObsLocDataList,
            int32_t bsObsLocDataListSize, int8_t bsListStatus) override;
    Return<void> serviceRequestCallback() override;
    Return<void> attachVmOnCallback() override;
};

Return<void> LocHidlWWANDBProviderCallback::bsObsLocDataUpdateCallback(
        const hidl_vec<LocHidlBSObsData>& bsObsLocDataList, int32_t bsObsLocDataListSize,
        int8_t bsListStatus) {

    FROM_HIDL_SERVICE();
    ALOGD("bsObsLocDataUpdateCallback bsObsLocDataListSize: %d bsListStatus: %u\n",
          bsObsLocDataListSize, bsListStatus);

    JNIEnv* env = NULL;
    if (sWWANDBProvCbJavaVm) {
        sWWANDBProvCbJavaVm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);
    }

    if (env && sCallbacksObj && method_onBsObsLocDataAvailable) {
        if (class_BSObsLocData && class_BSInfo) {
            jobjectArray jBSObsLocDataList = env->NewObjectArray(bsObsLocDataListSize,
                                                                 class_BSObsLocData,
                                                                 NULL);


            if (jBSObsLocDataList) {
                ALOGD("bsObsLocDataUpdateCallback list");
                for (int32_t iBsObs = 0; iBsObs < bsObsLocDataListSize; iBsObs++) {
                    ALOGV("bsObsLocDataUpdateCallback elem %d of %d", iBsObs, bsObsLocDataListSize);
                    jmethodID jConstructorBsObsData =
                            env->GetMethodID(class_BSObsLocData, "<init>", "()V");
                    jobject jBSObsObject =
                            env->NewObject(class_BSObsLocData, jConstructorBsObsData, NULL);
                    if (jBSObsObject) {
                        auto& info = bsObsLocDataList[iBsObs];

                        TranslateHidlToObjectBsObs(env, info, jBSObsObject);
                        if (jBSObsObject) {
                            env->SetObjectArrayElement(jBSObsLocDataList, iBsObs, jBSObsObject);
                        } else {
                            ALOGW("jBSObsObject is null");
                        }
                        // Managing overwhelming number of requests
                        env->DeleteLocalRef(jBSObsObject);
                    } else {
                        ALOGW("jBSObsObject is null!");
                    }
                }
            }

            env->CallVoidMethod(sCallbacksObj,
                                method_onBsObsLocDataAvailable,
                                jBSObsLocDataList,
                                (int) bsListStatus);
            // Managing overwhelming number of requests
            env->DeleteLocalRef(jBSObsLocDataList);
            checkAndClearExceptionsFromCallback(env, __FUNCTION__);
        }
    }
    return Void();
}

Return<void> LocHidlWWANDBProviderCallback::serviceRequestCallback() {

  FROM_HIDL_SERVICE();

  JNIEnv* env = NULL;
  if (sWWANDBProvCbJavaVm) {
      sWWANDBProvCbJavaVm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);
  }
  if (env && sCallbacksObj && method_onServiceRequest) {
      env->CallVoidMethod(sCallbacksObj,
                          method_onServiceRequest);
      checkAndClearExceptionsFromCallback(env, __FUNCTION__);
  }

  return Void();
}

Return<void> LocHidlWWANDBProviderCallback::attachVmOnCallback() {

  FROM_HIDL_SERVICE();

  void* clientData = NULL;

  sWWANDBProvCbJavaVm = AndroidRuntime::getJavaVM();
  JNIEnv *callbackEnv = NULL;
  JavaVMAttachArgs args = {
              JNI_VERSION_1_6,
              "WFD Service Callback Thread",
              NULL
          };
  jint attachResult = sWWANDBProvCbJavaVm->AttachCurrentThread(&callbackEnv, &args);
  if (attachResult != 0) {
      ALOGE("Callback thread attachment error: %d", attachResult);
      callbackEnv = NULL;
  }

  return Void();
}


/* ===================================================================
 *   HIDL Interface
 * ===================================================================*/
static sp<ILocHidlWWANDBProvider> wwanDBProviderIface = nullptr;
static sp<ILocHidlWWANDBProviderCallback> wwanDBProviderCallback = nullptr;
static bool isWwanDBProviderIfaceSupported = true;

static void WWANDBProvider_init(JNIEnv *env, jobject obj);

static sp<ILocHidlWWANDBProvider>& getWWANDBProviderIface() {
    if (wwanDBProviderIface == nullptr) {
        auto gnssVendorHal = ILocHidlGnss::getService(GNSS_VENDOR_SERVICE_INSTANCE);
        if (gnssVendorHal == nullptr) {
            ALOGE("Unable to get GnssVendor service\n");
        } else if (isWwanDBProviderIfaceSupported) {
            auto wdr = gnssVendorHal->getExtensionLocHidlWWANDBProvider();

            if (!wdr.isOk()) {
                ALOGE("Unable to get a handle to WWANDBProvider extension !");
                isWwanDBProviderIfaceSupported = false;
            } else {
                wwanDBProviderIface = wdr;
            }
        }
    }
    return wwanDBProviderIface;
}

static void WWANDBProvider_class_init(JNIEnv* env, jclass clazz)
{
    // get references to the Java methods
    method_onBsObsLocDataAvailable = env->GetMethodID(clazz,
            "onBsObsLocDataAvailable",
            "([Lcom/qti/wwandbprovider/BSObsLocationData;I)V");
    method_onServiceRequest = env->GetMethodID(clazz, "onServiceRequest", "()V");

    /* Now fetch HIDL interface */
    getWWANDBProviderIface();

    registerDeathRecipient("WWANDBProvider", []() {
            wwanDBProviderIface = nullptr;
            ALOGE("ILocHidlWWANDBProvider died");
            WWANDBProvider_init(nullptr, nullptr);
    });
}

static void WWANDBProvider_init(JNIEnv *env, jobject obj){

    if (nullptr != env) {
        if (sCallbacksObj == NULL) {
            sCallbacksObj = env->NewGlobalRef(obj);
        }
        if (class_BSObsLocData == NULL) {
            jclass class_BSObsLocData_temp =
                    env->FindClass("com/qti/wwandbprovider/BSObsLocationData");
            if (!class_BSObsLocData_temp) {
                ALOGE("Class not found : com/qti/wwandbprovider/BSObsLocationData");
            } else {
                class_BSObsLocData = (jclass)env->NewGlobalRef(class_BSObsLocData_temp);
            }
        }

        if (class_BSInfo == NULL) {
            jclass class_BSInfo_temp = env->FindClass("com/qti/wwandbreceiver/BSInfo");
            if (!class_BSInfo_temp) {
                ALOGE("Class not found : com/qti/wwandbreceiver/BSInfo");
            } else {
                class_BSInfo = (jclass)env->NewGlobalRef(class_BSInfo_temp);
            }
        }
    }

    auto iface = getWWANDBProviderIface();
    if (iface == nullptr) {
      ALOGE("WWANDBProvider Iface NULL !");
      return;
    }

    if (wwanDBProviderCallback == nullptr) {
        wwanDBProviderCallback = new LocHidlWWANDBProviderCallback();
    }

    TO_HIDL_SERVICE();
    auto r = iface->init(wwanDBProviderCallback);
    if (!r.isOk() || r == false) {
      ALOGE("Error invoking HIDL API [%s]", r.description().c_str());
      return;
    }
    auto rt = iface->registerWWANDBProvider(wwanDBProviderCallback);
    if (!rt.isOk()) {
      ALOGE("Error invoking HIDL API [%s]", rt.description().c_str());
      return;
    }
}

static void WWANDBProvider_deinit(JNIEnv *env, jobject /*obj*/)
{
    // clear Global References if any
    if (sCallbacksObj) env->DeleteGlobalRef(sCallbacksObj);
    sCallbacksObj = NULL;

    if (class_BSObsLocData) env->DeleteGlobalRef(class_BSObsLocData);
    class_BSObsLocData = NULL;

    if (class_BSInfo) env->DeleteGlobalRef(class_BSInfo);
    class_BSInfo = NULL;

    auto iface = getWWANDBProviderIface();
    if (iface == nullptr) {
        ALOGE("WWANDBProvider Iface NULL !");
        return;
    }

    TO_HIDL_SERVICE();
    auto r = iface->unregisterWWANDBProvider();
    if (!r.isOk()) {
        ALOGE("Error invoking HIDL API [%s]", r.description().c_str());
        return;
    }
}

static jint WWANDBProvider_requestBSObsLocData(JNIEnv* /*env*/, jobject /*obj*/) {
    jint result = 0;

    auto iface = getWWANDBProviderIface();
    if (iface == nullptr) {
      ALOGE("WWANDBProvider Iface NULL !");
      return result;
    }

    TO_HIDL_SERVICE();
    auto r = iface->sendBSObsLocDataRequest();
    if (!r.isOk()) {
      ALOGE("Error invoking HIDL API [%s]", r.description().c_str());
    }
    return result;
}

static JNINativeMethod sMethods[] = {
    /* name, signature, funcPtr */
    { "native_wwan_db_provider_class_init",
     "()V",
     reinterpret_cast<void*>(WWANDBProvider_class_init)},
    {"native_wwan_db_provider_init",
     "()V",
     reinterpret_cast<void*>(WWANDBProvider_init)},
    {"native_wwan_db_provider_deinit",
     "()V",
     reinterpret_cast<void*>(WWANDBProvider_deinit)},
    {"native_wwan_db_provider_request_bs_obs_data",
     "()I",
     reinterpret_cast<void*>(WWANDBProvider_requestBSObsLocData)},
};

int register_WWANDBProvider(JNIEnv* env)
{
    return jniRegisterNativeMethods(env,
                                    "com/qualcomm/location/izat/wwandbprovider/WWANDBProvider",
                                    sMethods,
                                    NELEM(sMethods));
}
