/* gyro_port.h
 *
 * Copyright (c) 2013 - 2017,2019 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __GYRO_PORT_H__
#define __GYRO_PORT_H__
#include "mct_port.h"
#include "mct_stream.h"
#include "modules.h"
#include "stats_module.h"
#include "stats_event.h"
#include "dsps_hw_interface.h"
#include "dsps_hw.h"
#include "aec.h"
#include "camera_dbg.h"
#include "sensor_intf.h"
#include "sensor_laser.h"
#include "gyro_thread.h"



#define SENSORS_MAX_STREAMS 2
#define SENSORS_PREVIEW 0
#define SENSORS_VIDEO 1
#define BUF_SIZE        (4)


#define MAX_CALLBACK_REGISTER 10

typedef struct _callbackPtr{
  void *ptr;
  int isValid;
}callbackPtr;

typedef struct _callbackRegister{
  callbackPtr callbackPtrArr[MAX_CALLBACK_REGISTER];
  pthread_mutex_t cbMutex;
}callbackRegister;

typedef enum {
  ACCELEROMETER_TYPE        = (0x1 << 0),
  GYRO_TYPE                 = (0x1 << 1),
  MAGNETOMETER_TYPE         = (0x1 << 2),
  GRAVITY_TYPE              = (0x1 << 3),
  LINEAR_ACCELERATION_TYPE  = (0x1 << 4),
  TIME_OF_FLIGHT_TYPE       = (0x1 << 5),
  LIGHT_TYPE                = (0x1 << 6)
} gyro_port_sensor_type;

typedef enum {
  GYRO_PORT_STATE_CREATED = 1,
  GYRO_PORT_STATE_RESERVED,
  GYRO_PORT_STATE_LINKED,
  GYRO_PORT_STATE_UNLINKED,
  GYRO_PORT_STATE_UNRESERVED
} gyro_port_state_t;


/** _gyro_port_private_t:
 *    @reserved_id: session id | stream id
 *    @state: state of gyro port
 *    @sof: start of frame timestamp in microseconds.
 *    @frame_time: time when the last line of the sensor is read in
 *                 microseconds.  Does not include vertical blanking time
 *    @last_time: end time of the last gyro sample interval in microseconds
 *    @exposure_time: exposure time in seconds
 *    @gyro_stats: MCT_EVENT_MODULE_STATS_GYRO_STATS event body
 *    @dsps_handle: DSPS handle for access to gyro data
 *    @rs_interval_offset: maximum rolling shutter gyro interval offset
 *    @s3d_interval_offset: maximum shake 3D gyro interval offset
 *    @gyro_sample_rate: gyro sample rate in Hz
 *    @gyro_sample_rate_eis: gyro sample rate for EIS
 *    @is_type[]: IS type (DIS, EIS 2.0, etc.)
 *    @and_sens_handle: Android native sensor interface handle
 *    @sensor_list: List all availables sensors asigning one bit per sensor; type: gyro_port_sensor_type
 *    @dual_cam_sensor_info: To know if we are in Main Camera Mode or Auxillary
 *    @intra_peer_id: Intra peer Id
 *    @sof_ns: start of frame timestamp in nanoseconds.
 *
 *  This structure defines gyro port's private variables
 **/
typedef struct _gyro_port_private {
  unsigned int reserved_id;
  gyro_port_state_t state;
  unsigned int frame_id[BUF_SIZE];
  unsigned long long sof[BUF_SIZE];
  unsigned int sof_event_identity;
  unsigned long long frame_time;
  unsigned long long last_time;
  float exposure_time[BUF_SIZE];
  mct_event_gyro_stats_t gyro_stats;
  void *dsps_handle;
  int64_t rs_interval_offset;
  int64_t s3d_interval_offset;
  uint32_t gyro_sample_rate;
  uint32_t gyro_sample_rate_eis;
  cam_is_type_t is_type[SENSORS_MAX_STREAMS];
  void *and_sens_handle;
  unsigned int sensor_list;
  unsigned int dsps_sensor_list;
  unsigned int current_sof_id;
  boolean tof_intf_direct;
  enum camb_position_t sensor_position;
  void *laser_sns_hndl;
  callbackPtr *cbPtr;
  cam_sync_type_t      dual_cam_sensor_info;
  uint32_t             intra_peer_id;
  sensor_android_fmwk_mask sensor_intf_mask;
  int32_t enqueue_every_frame;
  boolean is_af_hj_enabled;
  // is IS enabled
  boolean is_IS_enabled;
  uint32_t af_hj_gyro_sampling_rate;
  uint32_t af_hj_gyro_report_rate;
  gyro_thread_data_t *thread_data;
  callbackRegister CBRegister;
  boolean sensors_disabled;
  uint32_t accel_sample_rate;
  unsigned long long sof_ns[BUF_SIZE];
} gyro_port_private_t;



void gyro_port_deinit(mct_port_t *port);
boolean gyro_port_find_identity(mct_port_t *port, unsigned int identity);
boolean gyro_port_init(mct_port_t *port, unsigned int session_id);
void gyro_port_callback(void *port, unsigned int frame_id);
boolean gyro_port_init_sensors(mct_port_t *port);
void dsps_port_handle_set_common_params(mct_port_t *port, boolean stream_on);
int dsps_port_handle_set_is_enable(gyro_port_private_t *private,int32_t is_enable);
void gyro_port_handle_sof_event(mct_event_t *event,
  gyro_port_private_t *private);
void gyro_port_process_callback(mct_port_t *port, dsps_cb_data_t *cb_data);
void sensor_event_cb_sensor_data(void *port,
  sensor_callback_data_type* sensor_data);

static boolean gyro_port_start_thread(mct_port_t *port);
void gyro_thread_process_sof_event(gyro_request_t *req_data, gyro_port_private_t *private);
#endif /* __GYRO_PORT_H__ */
