/* sensor_service_util.h
 *
 * Copyright (c) 2015,2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#ifndef _ANDROID_
#ifdef SENSOR_NATIVE_LAYER
#undef SENSOR_NATIVE_LAYER
#endif
#endif

#ifndef __SENSOR_SERVICEUTIL_H__
#define __SENSOR_SERVICEUTIL_H__

#ifdef SENSOR_NATIVE_LAYER
#include <android/sensor.h>
#ifdef __cplusplus
extern "C" {
#endif

ASensorManager *sensor_service_get_object(void);
void sensor_service_destroy(ASensorManager *p);
int ASensor_getMaxRange(ASensor const* sensor);

#ifdef __cplusplus
}
#endif
#endif /* SENSOR_NATIVE_LAYER */
#endif /* __SENSOR_SERVICEUTIL_H__ */
