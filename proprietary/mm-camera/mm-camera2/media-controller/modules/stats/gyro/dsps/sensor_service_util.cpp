/* sensor_service_util.cpp
 *
 * Copyright (c) 2015,2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#include "sensor_service_util.h"

#ifdef SENSOR_NATIVE_LAYER
#include <android/sensor.h>
#include <gui/Sensor.h>
#include <gui/SensorManager.h>
#include <gui/SensorEventQueue.h>


using namespace android;

ASensorManager* sensor_service_get_object(void)
{
#ifdef UPGRADE_SENSOR_MANAGER //M-frameworks
  return ASensorManager_getInstanceForPackage("Camera");
#else // L-frameworks
  return (ASensorManager *)(new SensorManager());
#endif
}

void sensor_service_destroy(ASensorManager* smgr)
{
  delete smgr;
}


int ASensor_getMaxRange(ASensor const* sensor)
{
  return static_cast<Sensor const*>(sensor)->getMaxValue();
}
#endif /*SENSOR_NATIVE_LAYER*/
