/* camera_config.h
 *
 * Copyright (c) 2015-2016 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#ifndef __CAMERA_CONFIG_H__
#define __CAMERA_CONFIG_H__

#include "sensor_sdk_common.h"
#include "cam_types.h"

#define MAX_CHROMATIX_ARRAY 30

typedef enum {
  SENSOR_SPECIAL_MODE_NONE,

  SENSOR_SPECIAL_GROUP_GLOBAL_BEGIN, // 1
  SENSOR_SPECIAL_MODE_FLASH = SENSOR_SPECIAL_GROUP_GLOBAL_BEGIN,
  SENSOR_SPECIAL_MODE_ZOOM_UPSCALE,
  SENSOR_SPECIAL_MODE_ZOOM_DOWNSCALE,
  SENSOR_SPECIAL_MODE_IR,
  SENSOR_SPECIAL_MODE_OIS_CAPTURE,
  SENSOR_SPECIAL_GROUP_GLOBAL_END = SENSOR_SPECIAL_MODE_OIS_CAPTURE,

  SENSOR_SPECIAL_GROUP_ISO_BEGIN, // 5
  SENSOR_SPECIAL_MODE_ISO_100 = SENSOR_SPECIAL_GROUP_ISO_BEGIN,
  SENSOR_SPECIAL_MODE_ISO_200,
  SENSOR_SPECIAL_MODE_ISO_400,
  SENSOR_SPECIAL_MODE_ISO_800,
  SENSOR_SPECIAL_MODE_ISO_1600,
  SENSOR_SPECIAL_GROUP_ISO_END = SENSOR_SPECIAL_MODE_ISO_1600,

  SENSOR_SPECIAL_GROUP_DIM_BEGIN, // 10
  SENSOR_SPECIAL_MODE_DIM_720P = SENSOR_SPECIAL_GROUP_DIM_BEGIN,
  SENSOR_SPECIAL_MODE_DIM_1080P,
  SENSOR_SPECIAL_MODE_DIM_UHD,
  SENSOR_SPECIAL_GROUP_DIM_END = SENSOR_SPECIAL_MODE_DIM_UHD,

  SENSOR_SPECIAL_GROUP_SCENE_BEGIN, //13
  SENSOR_SPECIAL_MODE_SCENE_LANDSCAPE = SENSOR_SPECIAL_GROUP_SCENE_BEGIN,
  SENSOR_SPECIAL_MODE_SCENE_SNOW,
  SENSOR_SPECIAL_MODE_SCENE_BEACH,
  SENSOR_SPECIAL_MODE_SCENE_SUNSET,
  SENSOR_SPECIAL_MODE_SCENE_NIGHT,
  SENSOR_SPECIAL_MODE_SCENE_PORTRAIT,
  SENSOR_SPECIAL_MODE_SCENE_BACKLIGHT,
  SENSOR_SPECIAL_MODE_SCENE_SPORTS,
  SENSOR_SPECIAL_MODE_SCENE_ANTISHAKE,
  SENSOR_SPECIAL_MODE_SCENE_FLOWERS,
  SENSOR_SPECIAL_MODE_SCENE_CANDLELIGHT,
  SENSOR_SPECIAL_MODE_SCENE_FIREWORKS,
  SENSOR_SPECIAL_MODE_SCENE_PARTY,
  SENSOR_SPECIAL_MODE_SCENE_NIGHT_PORTRAIT,
  SENSOR_SPECIAL_MODE_SCENE_THEATRE,
  SENSOR_SPECIAL_MODE_SCENE_ACTION,
  SENSOR_SPECIAL_MODE_SCENE_AR,
  SENSOR_SPECIAL_MODE_SCENE_FACE_PRIORITY,
  SENSOR_SPECIAL_MODE_SCENE_BARCODE,
  SENSOR_SPECIAL_GROUP_SCENE_END = SENSOR_SPECIAL_MODE_SCENE_BARCODE,
  SENSOR_SPECIAL_MODE_MAX
}sensor_special_mode;

typedef struct {
  float focal_length;
  float f_number;
  float total_f_dist;
  float hor_view_angle;
  float ver_view_angle;
  float min_focus_distance;
  cam_lens_type_t lens_type;
  float max_roll_degree;
  float max_pitch_degree;
  float max_yaw_degree;
} camera_lens_info_t;

typedef struct {
  uint8_t  csid_core;
  uint16_t lane_mask;
  uint16_t lane_assign;
  uint8_t  combo_mode;
} camera_csi_params_t;

typedef struct {
  unsigned char special_mode_type;
  unsigned long long int special_mode_mask;
  unsigned char sensor_resolution_index;
  char isp_common[NAME_SIZE_MAX];
  char isp_preview[NAME_SIZE_MAX];
  char isp_snapshot[NAME_SIZE_MAX];
  char isp_video[NAME_SIZE_MAX];
  char cpp_preview[NAME_SIZE_MAX];
  char cpp_snapshot[NAME_SIZE_MAX];
  char cpp_video[NAME_SIZE_MAX];
  char cpp_liveshot[NAME_SIZE_MAX];
  char postproc[NAME_SIZE_MAX];
  char a3_preview[NAME_SIZE_MAX];
  char a3_video[NAME_SIZE_MAX];
  char external[NAME_SIZE_MAX];
  char iot[NAME_SIZE_MAX];
} module_chromatix_name_t;

typedef struct {
  module_chromatix_name_t chromatix_name[MAX_CHROMATIX_ARRAY];
  uint16_t size;
} module_chromatix_info_t;

typedef struct {
  char vendor_name[NAME_SIZE_MAX];
  char sensor_name[NAME_SIZE_MAX];
  char actuator_name[NAME_SIZE_MAX];
  char eeprom_name[NAME_SIZE_MAX];
  char flash_name[NAME_SIZE_MAX];
  char ois_name[NAME_SIZE_MAX];
  char chromatix_name[NAME_SIZE_MAX];
  enum camera_i2c_freq_mode i2c_freq_mode;
  uint8_t camera_id;
  int modes_supported;
  cam_position_t position;
  unsigned int sensor_mount_angle;
  unsigned short sensor_slave_addr;

  camera_csi_params_t camera_csi_params;
  camera_lens_info_t lens_info;

  /* Sensor Chromatix Info */
  module_chromatix_info_t chromatix_info;
} camera_module_config_t;

#endif /* __CAMERA_CONFIG_H__ */

