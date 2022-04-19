/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2015-2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/
#ifndef __IZAT_MANAGER_UTILS_JNI_H__
#define __IZAT_MANAGER_UTILS_JNI_H__

#include <functional>
#include <string>
#ifdef __ANDROID__
#include <jni.h>
#endif
#include "utils/Log.h"

#define GNSS_VENDOR_SERVICE_INSTANCE "gnss_vendor"

#define TO_HIDL_SERVICE()   ALOGV("[%s][%d] [HC] =>> [HS]", __func__, __LINE__)
#define FROM_HIDL_SERVICE() ALOGV("[%s][%d] [HC] <<= [HS]", __func__, __LINE__)
/** Flags to indicate which values are valid in a LocGpsLocation. */
typedef uint16_t LocGpsLocationFlags;
/* IMPORTANT: Note that the following values must match
 * constants in GpsLocationProvider.java. */
/** LocGpsLocation has valid latitude and longitude. */
#define LOC_GPS_LOCATION_HAS_LAT_LONG   0x0001
/** LocGpsLocation has valid altitude. */
#define LOC_GPS_LOCATION_HAS_ALTITUDE   0x0002
/** LocGpsLocation has valid speed. */
#define LOC_GPS_LOCATION_HAS_SPEED      0x0004
/** LocGpsLocation has valid bearing. */
#define LOC_GPS_LOCATION_HAS_BEARING    0x0008
/** LocGpsLocation has valid accuracy. */
#define LOC_GPS_LOCATION_HAS_ACCURACY   0x0010
/** LocGpsLocation has valid vertical uncertainity */
#define LOC_GPS_LOCATION_HAS_VERT_UNCERTAINITY   0x0040
/** LocGpsLocation has valid spoof mask */
#define LOC_GPS_LOCATION_HAS_SPOOF_MASK   0x0080
/** LocGpsLocation has valid speed accuracy */
#define LOC_GPS_LOCATION_HAS_SPEED_ACCURACY   0x0100
/** LocGpsLocation has valid bearing accuracy */
#define LOC_GPS_LOCATION_HAS_BEARING_ACCURACY 0x0200

/** Position source is ULP */
#define ULP_LOCATION_IS_FROM_HYBRID   0x0001
/** Position source is GNSS only */
#define ULP_LOCATION_IS_FROM_GNSS     0x0002
/** Position is from a Geofence Breach Event */
#define ULP_LOCATION_IS_FROM_GEOFENCE 0X0004
/** Position is from Hardware FLP */
#define ULP_LOCATION_IS_FROM_HW_FLP   0x0008
/** Position is from NLP */
#define ULP_LOCATION_IS_FROM_NLP      0x0010
/** Position is from external DR solution*/
#define ULP_LOCATION_IS_FROM_EXT_DR   0X0020
/** Raw GNSS position fixes */
#define ULP_LOCATION_IS_FROM_GNSS_RAW   0X0040

JNIEnv* getCallbackEnv();
void checkAndClearExceptionsFromCallback(JNIEnv *env, const char* methodName);
uint16_t remapHidlFlags(uint16_t inFlags);
void registerDeathRecipient(std::string who, const std::function<void ()>& runnable);
void registerDeathRecipient(std::string who, const std::function<void ()>&& runnable);
void deregisterDeathRecipient(const std::string who);

#endif // #ifndef __IZAT_MANAGER_UTILS_JNI_H__
