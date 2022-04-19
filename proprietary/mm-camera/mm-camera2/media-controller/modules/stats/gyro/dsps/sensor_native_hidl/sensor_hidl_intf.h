
/* sensor_hidl_intf.h
 *
 * Copyright (c) 2017, 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
#ifndef _ANDROID_
#ifdef SENSOR_HIDL_INTF
#undef SENSOR_HIDL_INTF
#endif
#endif

#ifdef SENSOR_HIDL_INTF
#include <android/frameworks/sensorservice/1.0/ISensorManager.h>
#include <android/frameworks/sensorservice/1.0/IEventQueue.h>
#include <android/frameworks/sensorservice/1.0/types.h>
#ifdef LIKELY
#undef LIKELY
#endif
#ifdef UNLIKELY
#undef UNLIKELY
#endif

#include "sensor_intf.h"


#include <pthread.h>
#include <utils/Timers.h>
#include <utils/SystemClock.h>

using ::android::frameworks::sensorservice::V1_0::ISensorManager;
using ::android::frameworks::sensorservice::V1_0::IEventQueue;
using ::android::frameworks::sensorservice::V1_0::Result;
using ::android::frameworks::sensorservice::V1_0::IEventQueueCallback;

using ::android::hardware::sensors::V1_0::SensorInfo;
using ::android::hardware::sensors::V1_0::SensorType;
using ::android::hardware::sensors::V1_0::Event;

using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

typedef struct _sensor_hidl_sensor_info
{
  boolean present;
  boolean enabled;
  int32_t min_delay;
  int32_t max_delay;
  int32_t sample_rate;
  int32_t max_range;
  int32_t sensor_handle;
} sensor_hidl_sensor_info_type;
typedef enum _sensor_hidl_state
{
  STATE_INACTIVE = 0,
  STATE_ACTIVE,
  STATE_MAX,
}sensor_hidl_state;

typedef struct _sensor_hidl_list_info
{
  sensor_hidl_sensor_info_type s_info[SENSOR_TYPE_MAX_AVAILABLE];
} sensor_hidl_list_info;

class sensor_hidl_callback: public IEventQueueCallback {
  public:
   sensor_hidl_callback();
    ~sensor_hidl_callback();
    Return<void> onEvent(const Event &e);
    void *sensor_hidl_intf_obj;
    sensor_hidl_state state;
};

class sensor_hidl_intf {
  public:
    sensor_hidl_intf();
    ~sensor_hidl_intf();
    sensor_return_type sensor_hidl_init(
      void* cb_hndl,
      sensor_data_cb data_cb);
    sensor_return_type sensor_hidl_deinit();
    sensor_return_type sensor_hidl_enable(
      sensor_type sensorType,
      int32_t sample_rateUS);
    sensor_return_type sensor_hidl_disable(
      sensor_type sensorType);
    sensor_return_type sensor_hidl_set_data(
      sensor_set_data_type *set_data);
    /*callback function to receive events from IEventqueue*/
    Return<void> processEvent(const Event &e);

    /*Utility methods to support gyro buffering requests*/
    sensor_return_type sensor_hidl_enqueue(
      sensor_data_type sensor_data);
    sensor_return_type sensor_hidl_process_request( sensor_type type );
    sensor_return_type sensor_hidl_gyro_port_notify( sensor_type type );
    sensor_return_type sensor_hidl_gyro_start_search_backward(
      int start_pos, sensor_type type);
    sensor_return_type sensor_hidl_gyro_start_search_forward(
      int start_pos, sensor_type type);

  private:
    sp<IEventQueue> event_q_;
    sensor_hidl_callback *cbr;
    sensor_hidl_list_info list_info;
    /* callback object and function to notify data arrival*/
    void* cb_hndl;
    sensor_data_cb data_cb;
    pthread_mutex_t sof_req_mutex;
    pthread_mutex_t cb_mutex;
    boolean prev_request_proc; /* If previous request is processed */
    int16_t buf_pos;
    int16_t start_idx; /* Gyro request start timestamp index */
    int16_t end_idx;  /* Gyro request end timestamp index */
    int16_t pstart_idx; /* previous frame's start index */
    int16_t pend_idx; /* previous frame's end index */
    unsigned long long pend_ts; /*Last request end timestamp */
    sensor_data_range_type sof_info;
    sensor_data_range_type sof_info_next;
    boolean sof_request;
    sensor_data_type cirq_buffer[GYRO_BUFFER_SIZE];
    int64_t time_correction;
    uint32_t tc_counter;
#ifdef CAMERA_FEATURE_THIRDPARTYEIS
    pthread_mutex_t sof_req_mutex_accel;
    boolean prev_request_proc_accel; /* If previous request is processed */
    int16_t buf_pos_accel;
    int16_t start_idx_accel; /* Gyro request start timestamp index */
    int16_t end_idx_accel;  /* Gyro request end timestamp index */
    int16_t pstart_idx_accel; /* previous frame's start index */
    int16_t pend_idx_accel; /* previous frame's end index */
    unsigned long long pend_ts_accel; /*Last request end timestamp */
    sensor_data_range_type sof_info_accel;
    sensor_data_range_type sof_info_next_accel;
    boolean sof_request_accel;
    sensor_data_type cirq_buffer_accel[GYRO_BUFFER_SIZE];
#endif
};
#endif /*SENSOR_HIDL_INTF*/
