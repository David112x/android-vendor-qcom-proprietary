/*==========================================================

  Copyright (c) 2014-2019 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

===========================================================*/
#ifndef __EEPROM_LIB_H__
#define __EEPROM_LIB_H__

#include "actuator_driver.h"
#include "chromatix_params.h"

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#define SENSOR_SUCCESS 0
#define SENSOR_FAILURE -1
#define SENSOR_ERROR_IO -2
#define SENSOR_ERROR_NOMEM -3
#define SENSOR_ERROR_INVAL -4

#define MAX_PDAF_KNOT_X      16
#define MAX_PDAF_KNOT_Y      12

#define MAX_RESOLUTION_MODES 14
#define MAX_DPC_COORD       512

#define MAX_AF_SYNC_POINTS 20

#define EEPROM_MEMORY_MAP_MAX_SIZE  80
#define EEPROM_MAX_MEM_MAP_CNT      9

/*
 * EEPROM driver version is given by:
 * <Major version>.<Minor version>.<Patch version>
 */
#define EEPROM_DRIVER_VERSION "2.3.0"
#define EEPROM_SDK_CAPABILITIES "US_DRIVER_POWERUP, custom cal data"

#define MAXWIDTH               25  /* =8K/256 */
#define MAXHEIGHT              19
#define MAXLENGTH2D            MAXWIDTH * MAXHEIGHT
#define MAX_PDAF_WINDOW        200  /*max fixed pdaf window number*/
#define MAXLENGTH1D            256

typedef struct {
  sensor_chromatix_params_t     chromatix;
  actuator_driver_params_t      *af_driver_ptr;
#if defined(CHROMATIX_VERSION) && CHROMATIX_VERSION < 0x306
  af_algo_tune_parms_t          *af_algo_ptr[2];
#endif
} eeprom_set_chroma_af_t;

typedef struct {
  unsigned char     is_supported;
  char        eeprom_name[32];
  unsigned char     *buffer;
  unsigned int    num_bytes;
} eeprom_params_t;

struct eeprom_memory_map {
  unsigned int slave_addr;
  struct camera_reg_settings_t mem_settings[EEPROM_MEMORY_MAP_MAX_SIZE];
  unsigned int memory_map_size;
};

struct eeprom_memory_map_array {
  struct eeprom_memory_map memory_map[EEPROM_MAX_MEM_MAP_CNT];
  unsigned int size_map_array;
};

struct eeprom_info_t {
  struct camera_power_setting_array power_setting_array;
  enum camera_i2c_freq_mode i2c_freq_mode;
  struct eeprom_memory_map_array mem_map_array;
};

typedef struct {
  void (*get_calibration_items)(void *);
  void (*format_calibration_data)(void *);
  int  (*do_af_calibration) (void *);
  void (*do_wbc_calibration) (void *);
  void (*do_lsc_calibration) (void *);
  int (*get_raw_data) (void *, void *);
  int (*get_ois_raw_data) (void *, void *);
  void (*do_dual_calibration) (void *, void*);
  struct eeprom_info_t eeprom_info;
} eeprom_lib_func_t;

typedef struct {
  void                *eeprom_lib_handle;
  eeprom_lib_func_t   *func_tbl;
} eeprom_lib_params_t;

typedef struct {
  int   is_insensor;
  int   is_afc;
  int   is_wbc;
  int   is_lsc;
  int   is_dpc;
  int   is_ois;
  int   is_dual;
} eeprom_calib_items_t;

typedef struct {
  float   r_over_g[AGW_AWB_MAX_LIGHT];
  float   b_over_g[AGW_AWB_MAX_LIGHT];
  float   gr_over_gb;
} wbcalib_data_t;

typedef struct {
  short    macro_dac;
  short    infinity_dac;
  short    starting_dac;
  float    infinity_margin;
  float    macro_margin;
} afcalib_data_t;

typedef struct {
  mesh_rolloff_array_type   lsc_calib[ROLLOFF_MAX_LIGHT];
} lsccalib_data_t;

typedef struct {
  int   x;
  int   y;
} pixel_t;

typedef struct {
  pixel_t   pix[MAX_DPC_COORD];
  int   count;
} pixels_array_t;

typedef struct {
  pixels_array_t   dpc_calib[MAX_RESOLUTION_MODES];
} dpccalib_data_t;

typedef struct {
  unsigned short XKnotNumSlopeOffset;
  unsigned short YKnotNumSlopeOffset;
  float  SlopeData[MAX_PDAF_KNOT_X*MAX_PDAF_KNOT_Y];
  float  OffsetData[MAX_PDAF_KNOT_X*MAX_PDAF_KNOT_Y];
  unsigned short XAddressKnotSlopeOffset[MAX_PDAF_KNOT_X];
  unsigned short YAddressKnotSlopeOffset[MAX_PDAF_KNOT_Y];
} pdafcalib_data_t;

/**
 * dc_sensor_lens_calib_data_t parameters:
 * focal_length = normalized focal length of the lens
 * native_sensor_resolution_width = Native sensor resolution that was used to capture calibration image
 * native_sensor_resolution_height = Native sensor resolution that was used to capture calibration image
 * calibration_resolution_width = Image size used internally by calibration tool
 * calibration_resolution_height = Image size used internally by calibration tool
 * focal_length_ratio = Focal length ratio @ Calibration
 */
typedef struct {
  float          focal_length;
  unsigned short native_sensor_resolution_width;
  unsigned short native_sensor_resolution_height;
  unsigned short calibration_resolution_width;
  unsigned short calibration_resolution_height;
  float          focal_length_ratio;
} dc_sensor_lens_calib_data_t;

/**
 * dc_sensor_system_calib_data_t parameters:
 * calibration_format_version = Version information
 * relative_rotation_matrix = Relative viewpoint matching matrix w.r.t Main
 * relative_geometric_surface_parameters = Relative geometric surface description parameters
 * relative_principle_point_x_offset = Relative offset of sensor center from optical axis along horizontal dimension
 * relative_principle_point_y_offset = Relative offset of sensor center from optical axis along vertical dimension
 * relative_position_flag = 0=Main Camera is on the left of Aux; 1=Main Camera is on the right of Aux
 * relative_baseline_distance = Camera separation in mm
 * af_sync_calib_version dual af sync calibration version
 * nums_of_af_sync_point the number of af sync calibration points
 * dist_of_af_sync_point array of the distance of the different af sync calibration position cm
 * dac_of_af_sync_point array of the dac of the different af sync calibration position
 */
typedef struct {
  unsigned int     calibration_format_version;
  float            relative_rotation_matrix[9];
  float            relative_geometric_surface_parameters[32];
  float            relative_principle_point_x_offset;
  float            relative_principle_point_y_offset;
  unsigned short   relative_position_flag;
  float            relative_baseline_distance;
  unsigned short   master_sensor_mirror_flip_setting;
  unsigned short   aux_sensor_mirror_flip_setting;
  unsigned short   module_orientation_flag;
  unsigned short   rotation_flag;
  unsigned short   brightness_ratio;
  unsigned short   ref_aux_gain;;
  unsigned short   ref_aux_linecount;
  unsigned short   ref_master_gain;
  unsigned short   ref_master_linecount;
  unsigned short   ref_master_color_temperature;
  unsigned short   af_sync_calib_version;
  unsigned short   nums_of_af_sync_point;
  unsigned short   dist_of_af_sync_point[MAX_AF_SYNC_POINTS];
  short            dac_of_af_sync_point[MAX_AF_SYNC_POINTS];
  void             *dc_otp_params;
  int              dc_otp_size; /* in bytes */
} dc_sensor_system_calib_data_t;

/**
 * dual_camera_calib_data_t:
 * This is a master structure that holds all the
 * calibration data specific to dual camera mode.
 * dc_sensor_lens_calib_data_t master_calib_params: This holds the data specific
 *           to the main sensor.
 * dc_sensor_lens_calib_data_t aux_calib_params: This holds the data specific
 *           to the auxiliary sensor.
 * dc_sensor_system_calib_data_t system_calib_params: This holds the system
 *           specific data for dual camera calibration.
 */
typedef struct{
  dc_sensor_lens_calib_data_t   master_calib_params;
  dc_sensor_lens_calib_data_t   aux_calib_params;
  dc_sensor_system_calib_data_t system_calib_params;
  int                           dc_offset;
  int                           dc_size;
} dc_calib_data_t;

typedef struct
{
  unsigned short VersionNum;
  unsigned short OffsetX;
  unsigned short OffsetY;
  unsigned short RatioX;
  unsigned short RatioY;
  unsigned short MapWidth;
  unsigned short MapHeight;
  unsigned short Left_GainMap[MAXLENGTH2D];
  unsigned short Right_GainMap[MAXLENGTH2D];
  short   PD_conversion_coeff[MAX_PDAF_WINDOW];
  unsigned short VersionNum_DCC;
  unsigned short Q_factor_DCC;
  unsigned short MapWidth_DCC;
  unsigned short MapHeight_DCC;
} pdaf_2D_cal_data_t;

typedef struct {
  eeprom_calib_items_t            items;

  /* based on property setting
   * ("persist.vendor.camera.cal.awb", "...lsc", "...dp" and "...af")
   * indicate which items should do the calibration */
  eeprom_calib_items_t    items_for_cal;

  wbcalib_data_t                  wbc;
  afcalib_data_t                  afc;
  lsccalib_data_t                 lsc;
  dpccalib_data_t                 dpc;
  dc_calib_data_t                 dualc;
  pdafcalib_data_t                pdafc;
  pdaf_2D_cal_data_t              pdafc_2d;
  void                           *custom_data;
} format_data_t;

typedef struct {
  int                       fd;
  eeprom_params_t           eeprom_params;
  eeprom_set_chroma_af_t    eeprom_afchroma;
  eeprom_lib_params_t       eeprom_lib;
  format_data_t             eeprom_data;
  void                      *eebin_hdl;
  wbcalib_data_t            eeprom_wbc_factor;
  char                      eeprom_version_info[32];
} sensor_eeprom_data_t;

#endif /* __EEPROM_LIB_H__ */
