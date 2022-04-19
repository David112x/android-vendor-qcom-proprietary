/* aec_biz.h
*
* Copyright (c) 2014-2016,2019 Qualcomm Technologies, Inc. All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*/

#ifndef __BIZ_AEC_H__
#define __BIZ_AEC_H__

#include "aec_core.h"
#include "aec.h"
#include <dlfcn.h>

/* THis skip count is used during transition of Manual mode to
   Auto Mode. It ensure that enought frames are skipped to
   get stats with auto setting reflected in it*/
#define MANUAL_TO_AUTO_SKIP_CNT     3

/**
 * Defines whether AEC should lock/unlock in longshot flash snapshot.
 */
#define LONGSHOT_FLASH_AEC_LOCK  (TRUE)

/* This is to configure the AEC behavior in AEC locked state and flash is forced On
 * Setting this flag will enable the falsh in "Flash ON and AEC is locked"
 * This may cause image overexposed since no estimation is performed
 */
#define ENABLE_FLASH_FOR_AEC_LOCKED_STATE           (0)

/* Preferred AEC exp index for running dual led awb calibration */

#define DUAL_LED_CALIB_AEC_PREFERED_EXP_INDEX       (220)

/* Max number of outputs that algo instance could process */
#define MAX_AEC_ALGO_OUTPUTS        1

/* Create function pointers matching aec_core.h function definitions */
typedef q3a_core_result_type (* aec_set_parameters_core_func)
  (
    aec_handle_type p_handle,
    aec_core_set_param_type *param
  );
typedef q3a_core_result_type (* aec_get_parameters_core_func)
  (
    aec_handle_type p_handle,
    const aec_core_get_info_type *get_param_info,
    aec_core_get_output_type* get_param_output
  );
typedef q3a_core_result_type (* aec_process_core_func)
  (
    aec_handle_type p_handle,
    const aec_core_input_type *input,
    aec_core_output_type *output
  );
typedef q3a_core_result_type (* aec_init_core_func)
  (
    aec_handle_type *p_handle
  );
typedef q3a_core_result_type (*aec_deinit_core_func)
  (
    aec_handle_type p_handle
  );
/* Function pointer to set log level */
typedef void (*aec_set_log_core_func)(void);
/* Function pointer to set log level */
typedef void (*aec_get_version_number_func)
  (
    void *version
  );
typedef void (*aec_print_change_id_func)(void);

/**
 * aec_biz_algo_ops_t:Defines a structure with all the AEC algo interface operations
 *
 * @set_parameters: Set operation
 * @get_parameters: Get operation
 * @process: Process stats operation
 * @init: Initialization
 * @deinit: De-init
 * @set_log: Set logs in algo
 * @get_version: Gets AEC version number
 **/
typedef struct {
  aec_set_parameters_core_func    set_parameters;
  aec_get_parameters_core_func    get_parameters;
  aec_process_core_func           process;
  aec_init_core_func              init;
  aec_deinit_core_func            deinit;
  aec_set_log_core_func           set_log;
  aec_get_version_number_func     get_version;
  aec_print_change_id_func        print_change_id;
} aec_biz_algo_ops_t;

/**
 * Define the data structure need to restore when switch to bestshot off mode.
 *
 * @iso_mode:       iso mode before enter bestshot mode.
 * @iso_value:      iso value before enter bestshot mode.
 * @exp_comp_value: EV value before enter bestshot mode.
 * @metering_type:  metering type before enter bestshot mode.
 **/
typedef struct {
  aec_iso_mode_type iso_mode;
  uint32 iso_value;
  int32 exp_comp_val;
  aec_auto_exposure_mode_t metering_type;
  aec_bestshot_mode_type_t curr_mode;
} aec_bestshot_data_t;

typedef struct {
  boolean enable;
  boolean save_done;
  boolean load_done;
  aec_led_cal_data_t cal_data;
} aec_biz_led_cal_data_t;

/**
 * Define the data structure for the AEC biz logic.
 *
 * @sof_id:     sof id from isp.
 * @manual_to_auto_skip_cnt:
 * @bestshot_off_data: bestshot off data
 * @aec_enable_stats_mask: Stats that AEC requires HW to enable
 **/
typedef struct
{
  aec_biz_algo_ops_t aec_algo_ops;
  uint32 sof_id;
  uint32 manual_to_auto_skip_cnt;
  aec_bestshot_data_t bestshot_off_data;
  boolean exif_dbg_enable;
  q3a_stream_crop_t stream_crop_info;
  int preview_width;
  int preview_height;
  aec_sensor_info_t sensor_info;
  uint32 aec_enable_stats_mask;
  aec_config_t stats_config;
  aec_core_input_type  core_input;
  aec_core_output_type core_output;
  aec_handle_type handle;
  uint32  stats_depth;
  boolean fast_aec_enable;
  uint8 exif_dbg_level;
  aec_video_hdr_mode_t  video_hdr;
  aec_core_hdr_type hdr_type;
  aec_biz_led_cal_data_t led_cal_data;
  aec_bestshot_mode_type_t prev_bestshot_mode;
} aec_biz_t;

void * aec_biz_load_function(aec_object_t *aec_object);
void aec_biz_unload_function(aec_object_t *aec_object, void *lib_handler);

#endif /* __BIZ_AEC_H__ */

