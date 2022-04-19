/* sensor_intf.c
 *
 * Copyright (c) 2014-2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#include "sensor_intf.h"


/*Native frameworks interface APIs*/
sensor_return_type sensor_native_init(
  void **handle,
  void *cb_hndl,
  sensor_data_cb cb);
sensor_return_type sensor_native_deinit(void *handle);
sensor_return_type sensor_native_set_params(
  void *handle, sensor_set_data_type set_data);


/*HIDL interface APIs*/
sensor_return_type sensor_hidl_intf_init(
  void **handle,
  void *cb_hndl,
  sensor_data_cb cb);
sensor_return_type sensor_hidl_intf_deinit(
  void *handle);
sensor_return_type sensor_hidl_intf_set_params(
  void *handle, sensor_set_data_type set_data);



/*===========================================================================
 * FUNCTION      sensor_intf_init
 *    @handle: Handler
 *    @cb_handle:Callback function of client
 *    @cb: Callback data fnction
 *
 * DESCRIPTION : initializaties the sensor interface
 *=========================================================================*/
sensor_return_type sensor_intf_init(
  void **handle,
  void *cb_hndl,
  sensor_data_cb cb)
{
  sensor_return_type rc = SENSOR_RET_SUCCESS;
#ifdef _ANDROID_
  /* just call native interface */
#ifdef SENSOR_NATIVE_LAYER
  rc = sensor_native_init(handle, cb_hndl, cb);
#endif
#ifdef SENSOR_HIDL_INTF//call hidl interface
  rc = sensor_hidl_intf_init(handle,cb_hndl,cb);
#endif
#else
  /* Handle compilation warnings */
  STATS_UNUSED(handle);
  STATS_UNUSED(cb_hndl);
  STATS_UNUSED(cb);
  rc = SENSOR_RET_FAILURE;
#endif
  return rc;
}

/*===========================================================================
 * FUNCTION      sensor_intf_deinit
 *    @handle: Handler
 *
 * DESCRIPTION : Deinitializaties the sensor interface
 *=========================================================================*/
sensor_return_type sensor_intf_deinit(void *handle)
{

#ifdef _ANDROID_
#ifdef SENSOR_NATIVE_LAYER
  sensor_native_deinit(handle);
#endif
#ifdef SENSOR_HIDL_INTF
  sensor_hidl_intf_deinit(handle);
#endif
#else
  /* Handle compilation warnings */
  STATS_UNUSED(handle);
#endif
  return SENSOR_RET_SUCCESS;
}

/*===========================================================================
 * FUNCTION      sensor__set_params
 *    @handle: Handler
 *    @set_data : set/req data info
 *
 * DESCRIPTION : sets/config the sensor data.
 *=========================================================================*/

sensor_return_type sensor_set_params(
  void *handle, sensor_set_data_type set_data){
  sensor_return_type rc = SENSOR_RET_SUCCESS;
#ifdef _ANDROID_
#ifdef SENSOR_NATIVE_LAYER
  rc = sensor_native_set_params(handle, set_data);
#endif
  #ifdef SENSOR_HIDL_INTF
  rc = sensor_hidl_intf_set_params(handle, set_data);
#endif
#else
  STATS_UNUSED(handle);
  STATS_UNUSED(set_data);
  rc = SENSOR_RET_FAILURE;
#endif
  return rc;
}
