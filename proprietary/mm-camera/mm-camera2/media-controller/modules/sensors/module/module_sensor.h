/* module_sensor.h
 *
 * Copyright (c) 2012-2017 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __MODULE_SENSOR_H__
#define __MODULE_SENSOR_H__

#include "mct_stream.h"
#include "sensor_common.h"
#include "sensor.h"
#include "modules.h"
//#include "sensor_thread.h"
#include "mct_list.h"

/* macros for unpacking identity */
#define GET_STREAM_ID(identity) ((identity) & 0xFFFF)
#define GET_SESSION_ID(identity) (((identity) & 0xFFFF0000) >> 16)

#define INIT_CONFIG_TIMEOUT (500000000L)
#define RES_CONFIG_TIMEOUT (500000000L)
#define OPEN_TIMEOUT (500000000L)
#define CLOSE_TIMEOUT (500000000L)

typedef struct {
  cam_stream_type_t   stream_type;
  uint32_t            identity;
  uint32_t            width;
  uint32_t            height;
  int32_t             bundle_id;
  uint32_t            is_stream_on;
  mct_stream_info_t   *stream_info;
} module_sensor_port_stream_info_t;

typedef struct {
  cam_bundle_config_t bundle_config;
  uint32_t            stream_mask;
  int32_t             stream_on_count;
  uint32_t            fast_aec_mode_count;
} module_sensor_port_bundle_info_t;

/* Sensor port private data */
typedef struct {
  mct_list_t          *stream_list; /* module_sensor_port_stream_info_t * */
  mct_list_t          *bundle_list; /* module_sensor_port_bundle_info_t * */
} module_sensor_port_data_t;

typedef struct _module_sensor_match_id_params_t {
  enum sensor_sub_module_t  sub_module;
  int32_t                   subdev_id;
  char                      *subdev_name;
} module_sensor_match_id_params_t;

typedef struct {
  enum sensor_sub_module_t s_module;
  uint32_t p_event;
} eebin_load_data_t;

boolean module_sensor_handle_pixel_clk_change(mct_module_t *module,
  uint32_t identity, void *data);
boolean module_sensor_update_af_bracket_entry(mct_module_t *module,
  void *s_bundle, mct_event_t *event);
boolean module_sensor_update_flash_bracket_entry(mct_module_t *module,
  void *s_bundle, mct_event_t *event);

boolean module_sensor_handle_fast_aec_ack(mct_module_t *module,
  uint32_t identity);

boolean module_sensor_stream_on(mct_module_t *module,
  mct_event_t *event, module_sensor_bundle_info_t *s_bundle);

boolean module_sensor_set_new_resolution_stream_on(
    mct_module_t *module,mct_event_t *event,
    module_sensor_bundle_info_t *s_bundle,
    module_sensor_params_t* module_sensor_params,
    sensor_set_res_cfg_t *stream_on_cfg,
    mct_stream_info_t* stream_info);

boolean module_sensor_set_cfg_stream_on(sensor_get_t* sensor_get,
    module_sensor_bundle_info_t *s_bundle);

boolean module_sensor_send_event_stream_on(mct_module_t *module,
    mct_event_t *event);

boolean module_sensor_stream_on_fastaec(mct_module_t *module,
  mct_event_t *event, module_sensor_bundle_info_t *s_bundle);

mct_module_t *module_sensor_init(const char *name);
void module_sensor_deinit(mct_module_t *module);


int32_t module_sensor_get_stats_data(mct_module_t *module,
  uint32_t identity, stats_get_data_t* stats_get);

int32_t module_sensor_fill_supported_camif_data(
  module_sensor_bundle_info_t *s_bundle,
  pdaf_sensor_native_info_t *pdaf_native_pattern);

boolean modules_sensor_set_new_resolution(mct_module_t *module,
  mct_event_t *event,
  module_sensor_bundle_info_t *s_bundle,
  module_sensor_params_t *module_sensor_params,
  sensor_set_res_cfg_t *stream_on_cfg,
  boolean *is_retry,
  mct_stream_info_t *stream_info);

boolean module_sensor_post_flash_mode_to_thread(mct_module_t* module,
  uint32_t *identity,  module_sensor_bundle_info_t* s_bundle, void *data);

boolean module_sensor_flash_init(
  module_sensor_bundle_info_t *s_bundle);

boolean module_sensor_actuator_init_calibrate(
  module_sensor_bundle_info_t *s_bundle);

boolean module_sensor_load_external_libs(
  module_sensor_bundle_info_t *s_bundle);

boolean module_sensor_set_parm_led_mode(mct_module_t *module,
  module_sensor_bundle_info_t *s_bundle, boolean is_capture,
  cam_flash_mode_t mode, uint32_t frame_id, uint32_t identity);

boolean module_sensor_hal_set_parm(
   module_sensor_params_t *module_sensor_params,
   cam_intf_parm_type_t type, void *parm_data);

boolean module_sensor_set_param_af_bracket(
  module_sensor_bundle_info_t * s_bundle, void *parm_data);

boolean module_sensor_reload_chromatix_for_ir_mode(
  module_sensor_bundle_info_t *s_bundle,
  mct_module_t *module, uint32_t identity, boolean ir_mode);

boolean module_sensor_fill_frame_timing(mct_module_t *module __attribute__((unused)),
      mct_event_t *event __attribute__((unused)), sensor_bundle_info_t *bundle_info);

#endif /* __MODULE_SENSOR_H__ */
