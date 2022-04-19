/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#define LOG_TAG "LocSvc_HIDL_WiFiDBProvider_jni"
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
using ::vendor::qti::gnss::V2_1::ILocHidlWiFiDBProvider;
using ::vendor::qti::gnss::V2_1::ILocHidlWiFiDBProviderCallback;
using ::vendor::qti::gnss::V2_1::LocHidlWifiDBListStatus;
using ::vendor::qti::gnss::V2_1::LocHidlUlpLocation;

using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hidl::base::V1_0::IBase;
using ::android::hardware::Void;
using ::android::sp;
using ::android::hardware::hidl_string;

using namespace android;

static void* sWiFiDBProviderHandle = NULL;
static JavaVM* sWiFiDBProvCbJavaVm = NULL;
static jobject sCallbacksObj = NULL;

static jmethodID method_onApObsLocDataAvailable = NULL;
static jmethodID method_onServiceRequest = NULL;

static jclass class_APObsLocData= NULL;
static jclass class_BSInfo= NULL;
static jclass class_APScan= NULL;


typedef enum
{
  LOC_RIL_TECH_CDMA = 0x1,
  LOC_RIL_TECH_GSM = 0x2,
  LOC_RIL_TECH_WCDMA = 0x4,
  LOC_RIL_TECH_LTE = 0x8,
  LOC_RIL_TECH_TD_SCDMA = 0x16,
  LOC_RIL_TECH_MAX = 0x10000000
} LOC_RilTechType;

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

    env->DeleteLocalRef(locationClass);
}

static void TranslateHidlToObjectCell(JNIEnv* env,
                                  const ILocHidlWiFiDBProviderCallback::LocHidlApCellInfo& ap,
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
                switch (ap.cell_type)
                {
                    case LOC_RIL_TECH_GSM:
                        {
                            cell_type = GSM;
                        }
                        break;
                    case LOC_RIL_TECH_WCDMA:
                        {
                            cell_type = WCDMA;
                        }
                        break;
                    case LOC_RIL_TECH_CDMA:
                        {
                            cell_type = CDMA;
                        }
                        break;
                    case LOC_RIL_TECH_LTE:
                        {
                            cell_type = LTE;
                        }
                        break;
                } //end switch

                if (-1 == cell_type) {
                    ALOGV("ap.cell_type invalid: %u cell_type: %d", ap.cell_type, cell_type);
                } else {
                    env->SetIntField(bsInfoObject, jFieldCellType, cell_type);
                    env->SetIntField(bsInfoObject, jFieldCell1, ap.cell_id1);
                    env->SetIntField(bsInfoObject, jFieldCell2, ap.cell_id2);
                    env->SetIntField(bsInfoObject, jFieldCell3, ap.cell_id3);
                    env->SetIntField(bsInfoObject, jFieldCell4, ap.cell_id4);
                }
            }
        }
    }
}

static void TranslateHidlToObjectApScan(JNIEnv* env,
        const ILocHidlWiFiDBProviderCallback::LocHidlApScanData& ap_scan,
        jobject& apScanObject) {
    if (env) {
        static jfieldID jFieldAPScan_mac = env->GetFieldID(class_APScan, "mMacAddress",
                "Ljava/lang/String;");
        static jfieldID jFieldAPScan_rssi = env->GetFieldID(class_APScan, "mRssi", "F");
        static jfieldID jFieldAPScan_delta = env->GetFieldID(class_APScan, "mDeltaTime", "I");
        static jfieldID jFieldAPScan_ssid = env->GetFieldID(class_APScan, "mSSID", "[B");
        static jfieldID jFieldAPScan_channel = env->GetFieldID(class_APScan, "mChannelNumber", "I");
        char macStr[13];
        convertMac48toStr(macStr, 13, ap_scan.mac_R48b);
        jstring jfieldMac = (jstring)env->GetObjectField(apScanObject,
                                             jFieldAPScan_mac);
        jstring jStrMac = env->NewStringUTF(macStr);
        if (jStrMac) {
            env->SetObjectField(apScanObject, jFieldAPScan_mac, jStrMac);

            jbyteArray jssid = env->NewByteArray(ap_scan.ssid_valid_byte_count);
            if (jssid) {
                env->SetByteArrayRegion(jssid, 0, ap_scan.ssid_valid_byte_count,
                                        (jbyte*) ap_scan.ssid.c_str());
                env->SetObjectField(apScanObject, jFieldAPScan_ssid, jssid);
                // Managing overwhelming number of requests
                env->DeleteLocalRef(jssid);
            } else {
                ALOGW("NO jssid !!");
            }
            env->DeleteLocalRef(jStrMac);
        } else {
            ALOGW("NO jStrMac !!");
        }

        if (apScanObject) {
            env->SetFloatField(apScanObject, jFieldAPScan_rssi, ap_scan.rssi);
            env->SetIntField (apScanObject, jFieldAPScan_delta, ap_scan.age_usec);
            // channel_id should have been uint8_t in hidl
            env->SetIntField(apScanObject, jFieldAPScan_channel, (uint8_t) ap_scan.channel_id);
        } else {
            ALOGW("NO apScanObject !!");
        }
    }
}

static void TranslateHidlToObjectApObs(JNIEnv* env,
                                  const ILocHidlWiFiDBProviderCallback::LocHidlApObsData& apObsData,
                                  jobject& jAPObsObject) {

    if (class_APObsLocData && class_BSInfo && class_APScan) {
        static jfieldID jFieldLocation = env->GetFieldID(class_APObsLocData, "mLocation",
                                                         "Landroid/location/Location;");
        static jfieldID jFieldCellInfo = env->GetFieldID(class_APObsLocData, "mCellInfo",
                                                         "Lcom/qti/wwandbreceiver/BSInfo;");
        static jfieldID jFieldScanTimestamp = env->GetFieldID(class_APObsLocData, "mScanTimestamp",
                                                              "I");
        static jfieldID jFieldScanList = env->GetFieldID(class_APObsLocData, "mScanList",
                                                         "[Lcom/qti/wifidbprovider/APScan;");

        if (jAPObsObject) {
            if (apObsData.bUlpLocValid) {
                ALOGD("apObsData.bUlpLocValid adding location");
                jobject locationObject = NULL;
                TranslateHidlToObjectLocation(env, &apObsData.gpsLoc.gpsLocation, locationObject,
                                      apObsData.gpsLoc.position_source);

                if (locationObject) {
                    env->SetObjectField(jAPObsObject, jFieldLocation, locationObject);
                    env->DeleteLocalRef(locationObject);
                }
            } else {
                ALOGD("NOT apObsData.bUlpLocValid");
            }

            jobject bsInfoObject = NULL;
            TranslateHidlToObjectCell(env, apObsData.cellInfo, bsInfoObject);

            if (bsInfoObject) {
                env->SetObjectField(jAPObsObject, jFieldCellInfo, bsInfoObject);
                env->DeleteLocalRef(bsInfoObject);
            }

            env->SetIntField(jAPObsObject, jFieldScanTimestamp, apObsData.scanTimestamp_ms / 1000);

            jobjectArray jAPScanList = (jobjectArray)env->NewObjectArray(
                    apObsData.ap_scan_info_size, class_APScan, NULL);
            if (jAPScanList) {
                for (uint32_t ii = 0; ii < apObsData.ap_scan_info_size; ii++) {
                    auto& info = apObsData.ap_scan_info[ii];
                    jmethodID jConstructorAPScan = env->GetMethodID(class_APScan, "<init>", "()V");
                    jobject jAPScanObject = env->NewObject(class_APScan, jConstructorAPScan, NULL);
                    if (jAPScanObject) {
                        TranslateHidlToObjectApScan(env, info, jAPScanObject);
                        env->SetObjectArrayElement(jAPScanList, ii, jAPScanObject);
                        // Managing overwhelming number of requests
                        env->DeleteLocalRef(jAPScanObject);
                    } else {
                        ALOGW("Unable to create jAPScanObject !!");
                    }
                }
            }

            env->SetObjectField(jAPObsObject, jFieldScanList, jAPScanList);
        }
    }
}

/* ===========================================================
 *   HIDL Callbacks : ILocHidlWiFiDBProviderCallback.hal
 * ===========================================================*/

struct LocHidlWiFiDBProviderCallback : public ILocHidlWiFiDBProviderCallback {
    Return<void> apObsLocDataUpdateCallback(
            const hidl_vec<LocHidlApObsData>& apObsLocDataList,
            int32_t apObsLocDataListSize, LocHidlWifiDBListStatus apListStatus) override;
    Return<void> serviceRequestCallback() override;
    Return<void> attachVmOnCallback() override;
};

Return<void> LocHidlWiFiDBProviderCallback::apObsLocDataUpdateCallback(
        const hidl_vec<LocHidlApObsData>& apObsLocDataList, int32_t apObsLocDataListSize,
        LocHidlWifiDBListStatus apListStatus) {

    FROM_HIDL_SERVICE();

    JNIEnv* env = NULL;
    if (sWiFiDBProvCbJavaVm) {
        sWiFiDBProvCbJavaVm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);
    }

    if (env && sCallbacksObj && method_onApObsLocDataAvailable) {
        if (class_APObsLocData && class_BSInfo && class_APScan) {
            jmethodID jConstructorBSInfo = env->GetMethodID(class_APObsLocData, "<init>", "()V");

            static jfieldID jFieldLocation = env->GetFieldID(class_APObsLocData, "mLocation",
                                                             "Landroid/location/Location;");
            static jfieldID jFieldCellInfo = env->GetFieldID(class_APObsLocData, "mCellInfo",
                                                             "Lcom/qti/wwandbreceiver/BSInfo;");
            static jfieldID jFieldScanTimestamp = env->GetFieldID(class_APObsLocData,
                    "mScanTimestamp", "I");
            static jfieldID jFieldScanList = env->GetFieldID(class_APObsLocData, "mScanList",
                                                             "[Lcom/qti/wifidbprovider/APScan;");

            jobjectArray jAPObsLocDataList = env->NewObjectArray(apObsLocDataListSize,
                                                                 class_APObsLocData,
                                                                 NULL);

            if (jAPObsLocDataList) {
                ALOGD("apObsLocDataUpdateCallback list");
                for (int32_t iApObs = 0; iApObs < apObsLocDataListSize; iApObs++) {
                    ALOGV("apObsLocDataUpdateCallback elem %d of %d", iApObs, apObsLocDataListSize);
                    jmethodID jConstructorApObsData = env->GetMethodID(class_APObsLocData, "<init>",
                            "()V");
                    jobject jAPObsObject = env->NewObject(class_APObsLocData, jConstructorApObsData,
                            NULL);
                    if (jAPObsObject) {
                        auto& info = apObsLocDataList[iApObs];

                        TranslateHidlToObjectApObs(env, info, jAPObsObject);
                        if (jAPObsObject) {
                            env->SetObjectArrayElement(jAPObsLocDataList, iApObs, jAPObsObject);
                        } else {
                            ALOGW("jAPObsObject is null");
                        }
                        // Managing overwhelming number of requests
                        env->DeleteLocalRef(jAPObsObject);
                    } else {
                        ALOGW("jAPObsObject is null!");
                    }
                }
            }

            env->CallVoidMethod(sCallbacksObj,
                                method_onApObsLocDataAvailable,
                                jAPObsLocDataList,
                                (int) apListStatus);
            // Managing overwhelming number of requests
            env->DeleteLocalRef(jAPObsLocDataList);
            checkAndClearExceptionsFromCallback(env, __FUNCTION__);
        }
    }
    return Void();
}

Return<void> LocHidlWiFiDBProviderCallback::serviceRequestCallback() {

  FROM_HIDL_SERVICE();

  JNIEnv* env = NULL;
  if (sWiFiDBProvCbJavaVm) {
      sWiFiDBProvCbJavaVm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);
  }
  if (env && sCallbacksObj && method_onServiceRequest) {
      env->CallVoidMethod(sCallbacksObj,
                          method_onServiceRequest);
      checkAndClearExceptionsFromCallback(env, __FUNCTION__);
  }

  return Void();
}

Return<void> LocHidlWiFiDBProviderCallback::attachVmOnCallback() {

  FROM_HIDL_SERVICE();

  void* clientData = NULL;

  sWiFiDBProvCbJavaVm = AndroidRuntime::getJavaVM();
  JNIEnv *callbackEnv = NULL;
  JavaVMAttachArgs args = {
              JNI_VERSION_1_6,
              "WFD Service Callback Thread",
              NULL
          };
  jint attachResult = sWiFiDBProvCbJavaVm->AttachCurrentThread(&callbackEnv, &args);
  if (attachResult != 0) {
      ALOGE("Callback thread attachment error: %d", attachResult);
      callbackEnv = NULL;
  }

  return Void();
}


/* ===================================================================
 *   HIDL Interface
 * ===================================================================*/
static sp<ILocHidlWiFiDBProvider> wifiDBProviderIface = nullptr;
static sp<ILocHidlWiFiDBProviderCallback> wifiDBProviderCallback = nullptr;
static bool isWiFiDBProviderIfaceSupported = true;

static void WiFiDBProvider_init(JNIEnv *env, jobject obj);

static sp<ILocHidlWiFiDBProvider>& getWifiDBProviderIface() {
    if (wifiDBProviderIface == nullptr) {
        auto gnssVendorHal = ILocHidlGnss::getService(GNSS_VENDOR_SERVICE_INSTANCE);
        if (gnssVendorHal == nullptr) {
            ALOGE("Unable to get GnssVendor service\n");
        } else if (isWiFiDBProviderIfaceSupported) {
            auto wdr = gnssVendorHal->getExtensionLocHidlWiFiDBProvider();

            if (!wdr.isOk()) {
                ALOGE("Unable to get a handle to WiFiDBProvider extension !");
                isWiFiDBProviderIfaceSupported = false;
            } else {
                wifiDBProviderIface = wdr;
            }
        }
    }
    return wifiDBProviderIface;
}

static void WiFiDBProvider_class_init(JNIEnv* env, jclass clazz)
{
    // get references to the Java methods
    method_onApObsLocDataAvailable = env->GetMethodID(clazz,
            "onApObsLocDataAvailable",
            "([Lcom/qti/wifidbprovider/APObsLocData;I)V");
    method_onServiceRequest = env->GetMethodID(clazz, "onServiceRequest", "()V");

    /* Now fetch HIDL interface */
    getWifiDBProviderIface();

    registerDeathRecipient("WiFiDBProvider", []() {
            wifiDBProviderIface = nullptr;
            ALOGE("ILocHidlWiFiDBProvider died");
            WiFiDBProvider_init(nullptr, nullptr);
    });
}

static void WiFiDBProvider_init(JNIEnv *env, jobject obj){

    if (nullptr != env) {
        if (sCallbacksObj == NULL) {
            sCallbacksObj = env->NewGlobalRef(obj);
        }
        if (class_APObsLocData == NULL) {
            jclass class_APObsLocData_temp = env->FindClass("com/qti/wifidbprovider/APObsLocData");
            if (!class_APObsLocData_temp) {
                ALOGE("Class not found : com/qti/wifidbprovider/APObsLocData");
            } else {
                class_APObsLocData = (jclass)env->NewGlobalRef(class_APObsLocData_temp);
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

        if (class_APScan == NULL) {
            jclass class_APScan_temp = env->FindClass("com/qti/wifidbprovider/APScan");
            if (!class_APScan_temp) {
                ALOGE("Class not found : com/qti/wifidbprovider/APScan");
            } else {
                class_APScan = (jclass)env->NewGlobalRef(class_APScan_temp);
            }
        }
    }

    auto iface = getWifiDBProviderIface();
    if (iface == nullptr) {
      ALOGE("WiFiDBProvider Iface NULL !");
      return;
    }

    if (wifiDBProviderCallback == nullptr) {
        wifiDBProviderCallback = new LocHidlWiFiDBProviderCallback();
    }

    TO_HIDL_SERVICE();
    auto r = iface->init(wifiDBProviderCallback);
    if (!r.isOk() || r == false) {
      ALOGE("Error invoking HIDL API [%s]", r.description().c_str());
      return;
    }
    auto rt = iface->registerWiFiDBProvider(wifiDBProviderCallback);
    if (!rt.isOk()) {
      ALOGE("Error invoking HIDL API [%s]", rt.description().c_str());
      return;
    }
}

static void WiFiDBProvider_deinit(JNIEnv *env, jobject /*obj*/)
{
    // clear Global References if any
    if (sCallbacksObj) env->DeleteGlobalRef(sCallbacksObj);
    sCallbacksObj = NULL;

    if (class_APObsLocData) env->DeleteGlobalRef(class_APObsLocData);
    class_APObsLocData = NULL;

    if (class_BSInfo) env->DeleteGlobalRef(class_BSInfo);
    class_BSInfo = NULL;

    if (class_APScan) env->DeleteGlobalRef(class_APScan);
    class_APScan = NULL;

    auto iface = getWifiDBProviderIface();
    if (iface == nullptr) {
        ALOGE("WiFiDBProvider Iface NULL !");
        return;
    }

    TO_HIDL_SERVICE();
    auto r = iface->unregisterWiFiDBProvider();
    if (!r.isOk()) {
        ALOGE("Error invoking HIDL API [%s]", r.description().c_str());
        return;
    }
}

static jint WiFiDBProvider_requestAPObsLocData(JNIEnv* /*env*/, jobject /*obj*/) {
    jint result = 0;

    auto iface = getWifiDBProviderIface();
    if (iface == nullptr) {
      ALOGE("WiFiDBProvider Iface NULL !");
      return result;
    }

    TO_HIDL_SERVICE();
    auto r = iface->sendAPObsLocDataRequest();
    if (!r.isOk()) {
      ALOGE("Error invoking HIDL API [%s]", r.description().c_str());
    }
    return result;
}

static JNINativeMethod sMethods[] = {
    /* name, signature, funcPtr */
    { "native_wifi_db_provider_class_init",
     "()V",
     reinterpret_cast<void*>(WiFiDBProvider_class_init)},
    {"native_wifi_db_provider_init",
     "()V",
     reinterpret_cast<void*>(WiFiDBProvider_init)},
    {"native_wifi_db_provider_deinit",
     "()V",
     reinterpret_cast<void*>(WiFiDBProvider_deinit)},
    {"native_wifi_db_provider_request_ap_obs_data",
     "()I",
     reinterpret_cast<void*>(WiFiDBProvider_requestAPObsLocData)},
};

int register_WiFiDBProvider(JNIEnv* env)
{
    return jniRegisterNativeMethods(env,
                                    "com/qualcomm/location/izat/wifidbprovider/WiFiDBProvider",
                                    sMethods,
                                    NELEM(sMethods));
}
