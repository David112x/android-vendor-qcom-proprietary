/* sensor.h
 *
 * Copyright (c) 2012-2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __SENSOR_H__
#define __SENSOR_H__

#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include "sensor_common.h"
#include "sensor_thread.h"
#include "sensor_pick_res.h"
#include "sensor_pdaf_api.h"
#include "sensor_delay_api.h"
#include "sensor_gain_exposure.h"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define Q8  0x00000100
#define Q10 0x00000400

#define NANO_SEC_PER_SEC 1000000000

#define MAX_EXPOSURE_REGISTERS 64
#define MAX_FPS_REGISTERS 64


typedef enum
{
  CAMERA_MODULE_ISP,
  CAMERA_MODULE_CPP,
  CAMERA_MODULE_SW_PPROC,
  CAMERA_MODULE_3A,
  CAMERA_MODULE_EXTERNAL,
  CAMERA_MODULE_IOT,
  CAMERA_MODULE_MAX
} camera_module;

typedef enum
{
  SENSOR_CHROMATIX_TYPE_COMMON,
  SENSOR_CHROMATIX_TYPE_PREVIEW,
  SENSOR_CHROMATIX_TYPE_SNAPSHOT,
  SENSOR_CHROMATIX_TYPE_VIDEO,
  SENSOR_CHROMATIX_TYPE_LIVESHOT,
  SENSOR_CHROMATIX_TYPE_MAX,
} sensor_chroamtix_type;

typedef struct {
  uint32_t width;
  uint32_t height;
  uint32_t first_pixel;
  uint32_t last_pixel;
  uint32_t first_line;
  uint32_t last_line;
  sensor_camif_inputformat_t format;
} sensor_camif_setting_t;

typedef struct {
  uint32_t width;
  uint32_t height;
  uint32_t first_pixel;
  uint32_t last_pixel;
  uint32_t first_line;
  uint32_t last_line;
} sensor_dim_info_t;

typedef struct {
  float max_fps;
  float min_fps;
} sensor_fps_range_t;

typedef struct {
  cam_exp_bracketing_t ae_bracket_config;
  float real_gain[AE_BRACKET_MAX_ENTRIES];
  uint32_t linecount[AE_BRACKET_MAX_ENTRIES];
  int32_t valid_entries;
  int32_t apply_index;
  int32_t sof_counter;
  int32_t post_meta_bus;
  uint32_t prev_gain;
  uint32_t prev_linecount;
} sensor_ae_bracket_info_t;

typedef struct {
  float     real_gain;
  uint32_t  linecount;
  uint32_t  luma_hdr;
  uint32_t  fgain_hdr;
  float     s_real_gain;
  int32_t   s_linecount;
  float     m_real_gain;
  int32_t   m_linecount;
} sensor_exp_t;

typedef struct {
  int32_t stats_type;
  cam_format_t fmt;
  uint32_t width;
  uint32_t height;
} meta_sensor_stats;

typedef struct {
  pthread_mutex_t                mutex;
  int32_t                        fd;
  cam_stream_type_t              cur_stream_mask;
  double                         current_fps_div;
  uint32_t                       prev_gain;
  uint32_t                       prev_linecount;
  uint32_t                       prev_sensor_digital_gain;
  uint32_t                       current_gain;
  uint32_t                       current_linecount;
  uint32_t                       current_luma_hdr;
  uint32_t                       current_fgain_hdr;
  uint32_t                       sensor_digital_gain;
  float                          sensor_real_gain;
  float                          sensor_real_digital_gain;
  float                          gain_from_aec;
  float                          max_fps;
  float                          cur_fps;
  double                         realtime_fps;
  uint16_t                       last_updated_fll;
  uint16_t                       cur_frame_length_lines;
  uint16_t                       cur_line_length_pclk;
  int16_t                        prev_res;
  int16_t                        cur_res;
  float                          digital_gain;
  float                          ev_gain;
  sensor_camif_setting_t         camif_setting;
  struct msm_sensor_init_params *sensor_init_params;
  struct msm_sensor_info_t      *sensor_info;
  cam_hfr_mode_t                 hfr_mode;
  uint16_t                       wait_frame_count;
  sensor_ae_bracket_info_t       ae_bracket_info;
  uint32_t                       hdr_sof_counter;
  uint8_t                        hdr_zsl_mode;
  uint8_t                        manual_exposure_mode;
  uint8_t                        isp_frame_skip;/* Default is 0*/
  /*Sensor skip counter is delta of initial skip and sensor hardware delay
    * to determine in non zsl mode when to apply exposure values*/
  int16_t                        sensor_skip_counter;
  cam_sensor_hdr_type_t          hdr_mode;
  uint32_t                       is_quadra_mode;
  int32_t                        dis_enable;
  /* Assume a sensor whose output large enough that it requires more than
     one VFE to process. when that sensor is streaming, assume another sensor
     is opened in simultaneous camera usecase. If VFE does not have enough
     resources to process second camera open, it posts upstream event providing
     op pixel clk to be used by first sensor. Dynamic restart happens where
     first sensor will be stopped and started again where it has to choose a
     resolution whose op pixel clk fits in the value sent by VFE */
  uint32_t                       isp_pixel_clk_max;
  cam_dimension_t                max_dim;
  uint32_t                       cur_exposure_time;
  int32_t                        hal_version;
  int32_t                        capture_intent;
  uint8_t                        next_valid_frame;
  uint32_t                       lock_aec;
  sensor_pick_dev_t              sensor_pick;
  cam_test_pattern_mode_t        cur_test_pattern_mode;
  cam_test_pattern_data_t        test_pattern_data;
  float                          iso100_gain;
  float                          exp_multiplier;
  format_data_t                  *eeprom_data;
  int32_t                        delay_en;
  float                          s_real_gain;
  uint32_t                       prev_s_reg_gain;
  uint32_t                       prev_s_dig_reg_gain;
  int32_t                        prev_s_linecount;
  uint32_t                       prev_m_reg_gain;
  uint32_t                       prev_m_dig_reg_gain;
  int32_t                        prev_m_linecount;
  uint32_t                       s_reg_gain;
  uint32_t                       s_dig_reg_gain;
  int32_t                        s_linecount;
  float                          m_real_gain;
  uint32_t                       m_reg_gain;
  uint32_t                       m_dig_reg_gain;
  int32_t                        m_linecount;
  struct camera_i2c_reg_array   *exposure_regs;
  struct camera_i2c_reg_array   *fps_regs;
  sensor_submod_common_info_t   *sensor_common_info;
  uint64_t                       special_mode_mask;
  unsigned int                   pd_x_win_num;
  unsigned int                   pd_y_win_num;
  meta_sensor_stats              addl_sensor_stats;
  pdaf_win_cordinates_t          cur_pd_windows[MAX_PDAF_WIN]; /* T1 */
  PdLibSensorRegData_t           reg_data;
  PdLibSensorSetting_t           reg_setting;
  boolean                        window_updated;
  pdaf_window_configure_t        new_window;
  pdaf_window_configure_t        curr_window;  /* T2/3 */
} sensor_data_t;

typedef struct {
  boolean valid;
  sensor_exp_t exposure;
}sensor_set_expo_t;

typedef struct {
  sensor_lib_params_t  *lib_params;
  sensor_data_t        *s_data;
  int32_t               readfd;
  int32_t               writefd;
  int32_t               session_count;
  uint32_t              last_frame_id;
  sen_delay_api_t       delay_api;
  sensor_set_expo_t     expo;
  PD_HANDLE             pd_handle;
  PD_CAMIF_HANDLE       pd_camif_handle;
  boolean               is_pdaf_initiated;
} sensor_ctrl_t;

typedef struct {
  mct_module_t   *module;
  int32_t         write_fd;
  uint32_t        session_id;
}sensor_info_t;

typedef struct {
  mct_module_t   *module;
  uint32_t        session_id;
  uint32_t        frame_id;
} sensor_frame_t;

typedef struct {
  int64_t sof_timestamp;
  int64_t exposure_time;
  int64_t exposure_start_timestamp;
  int64_t frame_readout_time;
} sensor_exposure_starttime_t;

int32_t sensor_get_integration_time(void *sctrl, void *data);
float sensor_get_hfr_mode_fps(cam_hfr_mode_t mode);
int LOG_IOCTL(int d, int request, void* par1, char* trace_func);
int32_t sensor_write_i2c_setting(
  sensor_ctrl_t *ctrl, struct camera_i2c_reg_setting *setting);
boolean sensor_write_aec_settings(
  sensor_ctrl_t *ctrl, uint32_t stream_mask);
int32_t sensor_write_i2c_setting_array(
  sensor_ctrl_t *ctrl,
  struct camera_i2c_reg_setting_array *settings);
int32_t sensor_write_i2c_init_res_setting_array(
  sensor_ctrl_t *ctrl,
  struct sensor_i2c_reg_setting_array *settings);
#endif /* __SENSOR_H__ */
