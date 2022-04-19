/* sensor_util.h
 *
 * Copyright (c) 2012-2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __SENSOR_UTIL_H__
#define __SENSOR_UTIL_H__

#include <linux/media.h>
#include "mct_stream.h"
#include "module_sensor.h"

#define SENSOR_SESSIONID(id) (((id) & 0xFFFF0000) >> 16)
#define SENSOR_STREAMID(id) ((id) & 0x0000FFFF)
#define SENSOR_IDENTITY(session_id, stream_id) \
  ((((session_id) & 0x0000FFFF) << 16) | ((stream_id) & 0x0000FFFF))

#define FLOAT_TO_Q(exp, f) \
    ((int32_t)((f*(1<<(exp))) + ((f<0) ? -0.5 : 0.5)))

/* Convert M by N float matrix into M by N rational matrix */
#define FLOAT_TO_RATIONAL(In,Out, M, N) ({ \
    int i, j; \
    for (i=0; i<M ; i++) \
    for (j=0; j<N; j++){ \
      Out[i][j].numerator = FLOAT_TO_Q(7, In[i][j]); \
      Out[i][j].denominator = FLOAT_TO_Q(7, 1); \
    } \
})

#define MATRIX_MULT(IN1, IN2, OUT, M, N, L) ({ \
    int i, j, k; \
    for (i=0; i<M; i++) \
    for (j=0; j<L; j++) { \
      OUT[i][j] = 0; \
      for (k=0; k<N; k++) \
      OUT[i][j] += (IN1[i][k] * IN2[k][j]); \
    } })

#define MATRIX_INVERSE_3x3(MatIn, MatOut) ({\
    typeof (MatOut[0]) __det; \
    __det = MatIn[0]*(MatIn[4]*MatIn[8]-MatIn[5]*MatIn[7]) + \
    MatIn[1]*(MatIn[5]*MatIn[6]-MatIn[3]*MatIn[8]) + \
    MatIn[2]*(MatIn[3]*MatIn[7]-MatIn[4]*MatIn[6]); \
    if (__det == 0) \
    return; \
    MatOut[0] = (MatIn[4]*MatIn[8] - MatIn[5]*MatIn[7]) / __det; \
    MatOut[1] = (MatIn[2]*MatIn[7] - MatIn[1]*MatIn[8]) / __det; \
    MatOut[2] = (MatIn[1]*MatIn[5] - MatIn[2]*MatIn[4]) / __det; \
    MatOut[3] = (MatIn[5]*MatIn[6] - MatIn[3]*MatIn[8]) / __det; \
    MatOut[4] = (MatIn[0]*MatIn[8] - MatIn[2]*MatIn[6]) / __det; \
    MatOut[5] = (MatIn[2]*MatIn[3] - MatIn[0]*MatIn[5]) / __det; \
    MatOut[6] = (MatIn[3]*MatIn[7] - MatIn[4]*MatIn[6]) / __det; \
    MatOut[7] = (MatIn[1]*MatIn[6] - MatIn[0]*MatIn[7]) / __det; \
    MatOut[8] = (MatIn[0]*MatIn[4] - MatIn[1]*MatIn[3]) / __det; \
})

typedef struct {
  cam_stream_type_t stream_type;
  uint16_t session_id;
  uint16_t stream_id;
} module_sensor_util_stream_info_t;

boolean sensor_util_pass_fast_aec_mode_event(mct_port_t *port,
  uint32_t identity, uint8_t enable, uint16_t num_frames);
boolean sensor_util_pass_stream_off_event(mct_port_t *port,
  uint32_t identity, mct_stream_info_t *stream_info);
boolean sensor_util_pass_hfr_event(mct_port_t *port,
  uint32_t identity, cam_hfr_mode_t hfr_mode);
boolean sensor_util_pass_bundle_event(mct_port_t *port,
  uint32_t identity, cam_bundle_config_t *bundle_config);
uint32_t sensor_util_get_ide_from_stream_type(
  mct_list_t *port_stream_info_list, cam_stream_type_t type);
mct_stream_info_t *sensor_util_get_stream_info(
  mct_list_t *port_stream_info_list, uint32_t stream_id);
boolean sensor_util_unpack_identity(uint32_t identity,
  uint32_t *session_id, uint32_t *stream_id);
boolean sensor_util_find_bundle(void *data1, void *data2);
boolean sensor_util_find_identity(void *data1, void *data2);
mct_port_t *sensor_util_find_src_port_with_identity(mct_module_t *module,
  uint32_t identity);
int32_t sensor_util_assign_bundle_id(mct_module_t* module,
  uint32_t identity, cam_bundle_config_t* bundle);
void sensor_util_dump_bundle_and_stream_lists(mct_port_t* port,
  const char *func, int32_t line);
boolean sensor_util_post_event_on_src_port(mct_module_t *module,
  mct_event_t *event);
int32_t sensor_util_find_bundle_id_for_stream(mct_port_t* port,
  uint32_t identity);
boolean sensor_util_find_stream_identity_by_stream_type_and_session(
  mct_module_t *module, cam_stream_type_t stream_type,
  uint16_t session_id, uint32_t *stream_identity);
module_sensor_port_bundle_info_t* sensor_util_find_bundle_by_id(mct_port_t* port,
  int32_t bundle_id);
void sensor_util_remove_list_entries_by_identity(mct_port_t *port,
  uint32_t identity);
boolean sensor_util_get_sbundle(mct_module_t *s_module,
  uint32_t identity, sensor_bundle_info_t *bundle_info);
boolean sensor_util_set_digital_gain_to_isp(mct_module_t* module,
  module_sensor_bundle_info_t* s_bundle, uint32_t identity,
  sensor_aec_update_t *aec_update);
int32_t sensor_util_set_frame_skip_to_isp(mct_module_t* module,
  uint32_t identity, enum msm_vfe_frame_skip_pattern frame_skip_pattern);
int32_t sensor_util_is_previous_frame_sent(mct_module_t* module,
  mct_event_t *event, uint32_t identity_s, uint32_t identity_p);
boolean sensor_util_check_format(sensor_src_port_cap_t *caps,
  mct_stream_info_t *stream_info);
boolean sensor_util_post_bus_sensor_params(mct_module_t *s_module,
  module_sensor_bundle_info_t *s_bundle, uint32_t identity);
boolean sensor_util_load_liveshot_chromatix(mct_module_t *module,
  mct_port_t *port, mct_event_t *event, module_sensor_bundle_info_t *s_bundle);
boolean sensor_util_find_is_any_bundle_started(mct_port_t *port);
boolean sensor_util_post_led_state_msg(mct_module_t *s_module,
  module_sensor_bundle_info_t *s_bundle, uint32_t identity);
boolean sensor_util_post_braketing_state_event(mct_module_t *s_module,
  module_sensor_bundle_info_t *s_bundle, uint32_t identity, boolean bracketing,
  uint32_t index);
boolean sensor_util_post_to_bus(mct_module_t* module,
  module_sensor_bundle_info_t *s_bundle, mct_bus_msg_type_t type,
  uint32_t size, void *msg);
boolean sensor_util_send_led_mode_downstream(mct_module_t *s_module,
  cam_flash_mode_t *flash_mode, uint32_t identity, uint32_t frame_id);
boolean sensor_util_post_submod_event(
  module_sensor_bundle_info_t *s_bundle,  enum sensor_sub_module_t submod_type,
  sensor_submodule_event_type_t event, void *data);
boolean sensor_util_set_stream_on(mct_module_t *module,
  uint32_t identity, boolean st_on);
int32_t sensor_util_find_stream_type_for_stream(mct_port_t* port,
  uint32_t identity);
boolean sensor_util_post_downstream_event(mct_module_t *s_module,
  uint32_t identity, mct_event_module_type_t type, void *data);
boolean sensor_util_find_is_stream_on(mct_port_t* port);
int32_t sensor_util_sw_frame_skip_to_isp(mct_module_t *module,
  uint32_t identity, module_sensor_bundle_info_t *s_bundle,
  iface_skip_mode mode, boolean frame_skip, boolean stats_skip,
  uint32_t max_frame_skip);
boolean sensor_util_post_chromatix_event_downstream(
  mct_module_t *module, module_sensor_bundle_info_t *s_bundle,
  uint32_t identity, sensor_chromatix_params_t *chromatix_params,
  uint16_t modules_reloaded);
boolean sensor_util_validate_chromatix_params(
  sensor_chromatix_params_t *chromatix_params);

boolean sensor_util_post_intramode_event(mct_module_t *module,
  uint32_t identity, uint32_t peer_identity,
  mct_event_module_type_t type, void *data);
boolean sensor_util_link_intra_session(mct_module_t *module, mct_event_t *event,
  uint32_t peer_identity, mct_port_t **this_port, mct_port_t **peer_port);
module_sensor_bundle_info_t *sensor_util_get_peer_bundle(
  mct_module_t *module, module_sensor_bundle_info_t *my_bundle);

void sensor_generate_transfrom_matrix(
  cam_rational_type_t forward_mat[3][3], cam_rational_type_t color_mat[3][3],
  chromatix_color_correction_type *cc, chromatix_manual_white_balance_type *wb,
  boolean isD65);
void sensor_generate_calib_trans(cam_rational_type_t matrix[3][3],
  float r_gain, float b_gain);
void sensor_generate_unit_matrix(cam_rational_type_t matrix[3][3]);
uint32_t sensor_util_get_fourcc_format(
  sensor_output_format_t output_format,
  sensor_filter_arrangement filter_arrangement, uint8_t decode_format);
void sensor_util_set_special_mode(
  sensor_submod_common_info_t *sensor_common_info,
  sensor_special_mode special_mode, boolean enable);
int sensor_util_get_setting_size(
  struct camera_i2c_reg_array *settings, int32_t max_size);
cam_format_t sensor_util_get_hal_format(uint32_t pix_fmt_fourcc);
int8_t sensor_util_get_decode_fmt(sensor_raw_output_t raw_output);
sensor_pack_mode_t sensor_util_get_pack_mode(
  cam_format_t stream_format, cam_format_t sensor_format);
boolean sensor_util_is_plain_packing_supported(uint32_t csid_version);
#endif /* __SENSOR_UTIL_H__ */
