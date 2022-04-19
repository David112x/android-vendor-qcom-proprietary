/* q3a_port.h
 *
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __Q3A_PORT_H__
#define __Q3A_PORT_H__

#include "mct_port.h"
#include "q3a_thread.h"

#define Q3A_PORT_STATE_CREATED        0x1
#define Q3A_PORT_STATE_RESERVED       0x2
#define Q3A_PORT_STATE_UNRESERVED     0x3
#define Q3A_PORT_STATE_LINKED         0x4
#define Q3A_PORT_STATE_UNLINKED       0x5

#define Q3A_FIXED_INVALID_VALUE (-99999999.9f)

typedef enum {
  AF_WITH_LED_STATE_IDLE,
  AF_WITH_LED_STATE_AEC_RUNNING,
  AF_WITH_LED_STATE_AF_RUNNING,
  AF_WITH_LED_STATE_AF_DONE,
  AF_WITH_LED_STATE_MAX
} af_with_led_state_t;

typedef enum{
  AF_WITH_LED_NONE,
  AF_WITH_LED_TOUCH,
  AF_WITH_LED_SNAPSHOT,
  AF_WITH_LED_MAX
} af_with_led_type_t;

/** q3a_port_xml_values_t
 *    @key:     XML key.
 *    @value:   Placeholder to return the value related to key.
 **/
typedef struct {
  char *key;
  float *value;
} q3a_port_xml_values_t;

/** q3a_port_af_led_t
 *    @state:           current state of the feature
 *    @state_lock:      mutex to protect the access to the state
 *    @led_status:      LED status - on or off
 *    @aec_update_data: used to store the estimated AEC values
 *    @aec_no_led_data: used to store the AEC values before the LED is turned ON
 *    @led_needed:              a flag to track the AEC update and store the
 *                              condition if the LED is needed or not
 *    @lux_idx                  Trigger for LED AF
 **/
typedef struct _q3a_port_af_led_t {
  /* LED AF parameters */
  af_with_led_state_t state;
  pthread_mutex_t     state_lock;
  int                 led_status;
  boolean             led_needed;
  aec_sensitivity_data_t led_off_params;
  boolean             send_est_skip;

  /* LED AF post time out parameters */
  pthread_mutex_t     timer_lock;
  int                 preview_fps;
  int                 led_wait_count;
  boolean             restore_led_off;

  /* AF parameters */
  int                 af_mode;
  boolean             af_focus_mode_block;
  boolean             af_scene_mode_block;

  /* Capture, presnapshot trigger */
  boolean             prepare_snapshot_trigger;
  cam_intent_t        capture_intent;
} q3a_port_af_led_t;

typedef struct _q3a_port_fast_aec_t {
  boolean enable;
  q3a_fast_aec_state_t state;
  uint16_t num_frames;
  pthread_mutex_t  state_lock;
} q3a_port_fast_aec_t;

/** q3a_port_private_t
 *    @reserved_id:       The reserved ID
 *    @stream_type:       The type of the stream
 *    @state:             The state of the port - created, reserved, linked,
 *                        etc.
 *    @preview_stream_id: The ID of the preview stream
 *    @aec_port:          The pointer to the AEC port
 *    @awb_port:          The pointer to the AWB port
 *    @af_port:           The pointer to the AF port
 *    @aecawb_data:       The thread data of the aecawb thread
 *    @af_data:           The thread data of the af thread
 *    @af_led_data:       The data regarding the LED assisted AF feature
 *    @cur_sof_id:        The ID of the current SOF
 *
 *  This is the private data of the Q3A port.
 **/
typedef struct _q3a_port_private {
  unsigned int             reserved_id;
  unsigned int             stream_type;
  unsigned int             state;
  uint32_t                 preview_stream_id;
  mct_port_t               *aec_port;
  mct_port_t               *awb_port;
  mct_port_t               *af_port;
  q3a_thread_aecawb_data_t *aecawb_data;
  q3a_thread_af_data_t     *af_data;
  q3a_port_af_led_t        af_led_data;
  unsigned int             cur_sof_id;
  boolean                  stream_on;
  boolean                  aec_roi_enable;
  boolean                  aec_settled;
  uint32_t                 aec_ocsillate_cnt;
  boolean                  af_supported;
  uint8_t                  q3a_meta_mode;       /* main 3a switch */
  boolean                  q3a_enable_aec;      /* individual AEC switch */
  q3a_port_fast_aec_t      fast_aec_data;
  boolean                  aec_estimation;
  int                      led_state;
} q3a_port_private_t;

boolean q3a_port_map_lens_type(q3a_lens_type_t *q3a_lens_type,
  cam_lens_type_t cam_lens_type);
boolean q3a_port_map_sensor_format(q3a_sensor_type_t *q3a_sensor_type,
  mct_sensor_format_t cam_sensor_format);
boolean q3a_port_map_perf_type(q3a_dual_camera_perf_mode_t *q3a_perf_type,
  cam_dual_camera_perf_mode_t cam_perf_type);

void    q3a_port_deinit(mct_port_t *port);
boolean q3a_port_find_identity(mct_port_t *port, unsigned int identity);
boolean q3a_port_init(mct_port_t *port, mct_port_t *aec_port,
  mct_port_t *awb_port, mct_port_t *af_port, unsigned int identity);
boolean q3a_port_load_values_from_xml(
  const char *nodeName, q3a_port_xml_values_t* xml_values, const uint32 max_size);

#endif /* __Q3A_PORT_H__ */
