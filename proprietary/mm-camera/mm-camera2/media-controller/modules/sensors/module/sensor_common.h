/*==========================================================
 Copyright (c) 2012-2019 Qualcomm Technologies, Inc.
 All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.
 ===========================================================*/

/*
 * Sensor_common.h
 * This file will be used to place all data structures and enums using which
   sensor sub modules -  sensor actuator, eeprom, flash, csid, csiphy will
   interact among themselves */

#ifndef __SENSOR_COMMON_H__
#define __SENSOR_COMMON_H__

#include <media/msm_cam_sensor.h>
#include "mct_list.h"
#include "mct_bus.h"
#include "mct_pipeline.h"
#include "modules.h"
#include "sensor_debug.h"
#include "cam_intf.h"
#include "mtype.h"
#include "sensor_sdk_common.h"
#include "sensor_lib.h"
#include "chromatix.h"
#include "actuator_driver.h"
#include "chromatix_common.h"
#include "../ois/module/ois.h"
#include "sensor_sdk_utils.h"
#include "chromatix_cpp.h"
#include "chromatix_swpostproc.h"
#include "mct_stream.h"
#include "eeprom_lib.h"
#include "flash_lib.h"
#include "ois_driver.h"
#include "../chromatix/module/chromatix_manager.h"
#include "camera_config.h"

#define CONFIG_XML_PATH "/data/vendor/camera/"

#ifdef MSM8996_SENSORS
#undef SECURE_CAM_RST_MODULES
#undef SECURE_CAMERA
#endif

#ifndef MOD_SENSOR_LIB_PATH
#define MOD_SENSOR_LIB_PATH NULL
#endif

#define SENSOR_SUCCESS 0
#define SENSOR_FAILURE -1
#define SENSOR_ERROR_IO -2
#define SENSOR_ERROR_NOMEM -3
#define SENSOR_ERROR_INVAL -4

#define MAX_SUBDEV_SIZE 32

/* Last queue num 6 is dedicated for sensor internal ops*/
#define FRAME_CTRL_SIZE 7

#define MAX_CAMERA_ID_VAL 3
#define MAX_CSID_CORE_VAL 3

#define MAXQ(x, y) (((x) >= (y)) ? (x):(y))
#define MINQ(x, y) (((x) <= (y)) ? (x):(y))

#define MAX_ACTUATOR_PLOT_INFO 50

#define OIS_INIT_QUEUE_MASK 0x0001
#define OIS_INIT_DONE_MASK 0x0010
#define OIS_ENABLE_VALUE_MASK 0x0100
#define OIS_ENABLE_CALLED_MASK 0x1000
#define BUFF_SIZE_255          255
#define AE_BRACKET_MAX_ENTRIES 10

#define MAX_CID 16
#define MAX_CSID 4

#define IS_SENSOR_SUBMOD_VALID(s_bundle, mod) \
  ((s_bundle)->sensor_info->subdev_id[(mod)] != -1)

/* This macro will return errval if evaluated expression returned NULL */
#define RETURN_ERR_ON_NULL(expr, ret, args...)                \
    if ((expr) == NULL) {                                     \
        SERR("failed NULL pointer detected: " #expr " : " args); \
        return ret;                                           \
    }

// This macro will return FALSE to ret if evaluated expression returned NULL.
#define RETURN_ON_NULL(expr)         \
    if ((expr) == NULL) {            \
        SERR("failed NULL pointer detected " #expr); \
        return FALSE;                \
     }

// This macro will return if evaluated expression returned NULL.
#define RETURN_VOID_ON_NULL(expr)         \
    if ((expr) == NULL) {            \
        SERR("failed NULL pointer detected " #expr); \
        return;                \
     }

/* This macro will return SENSOR_FAILURE if evaluated expression returned NULL */
#define RETURN_ERROR_ON_NULL(expr)                         \
    if ((expr) == NULL) {                            \
        SERR("failed evaluated expression is NULL"); \
        return SENSOR_FAILURE;                                 \
     }

/* This macro will return SENSOR_FAILURE if evaluated expression returned NULL */
#define RETURN_ERROR_ON_FALSE(expr)                         \
    if ((expr) == FALSE) {                            \
        SERR("failed evaluated expression is FALSE"); \
        return SENSOR_FAILURE;                                 \
     }

// This macro will reurn FALSE to ret if evaluated expression returned FALSE.
#define RETURN_ON_FALSE(expr)                         \
    if ((expr) == FALSE) {                            \
        SERR("failed evaluated expression is FALSE"); \
        return FALSE;                                 \
     }

// This macro will return FALSE to ret if evaluated expression returned NULL.
#define BREAK_ON_NULL(expr)         \
    if ((expr) == NULL) {            \
        SERR("failed NULL pointer detected " #expr); \
        break;                \
     }

// This macro will report FALSE to ret if evaluated expression returned FALSE.
#define REPORT_ON_FALSE(expr, ret)                    \
    if ((expr) == FALSE) {                            \
        SERR("failed evaluated expression is FALSE"); \
        ret = FALSE;                                  \
     }

// This macro will report FALSE to ret if evaluated expression returned FALSE.
#define BREAK_ON_FALSE(expr)                     \
    if ((expr) == FALSE) {                            \
        SERR("failed evaluated expression is FALSE"); \
        break;                                        \
     }

// This macro will jump to label if evaluated expression returned NULL.
#define JUMP_ON_NULL(expr, label) \
    if ((expr) == NULL) {         \
        SERR("failed");           \
        goto label;               \
        }

// This macro will return FALSE to ret if evaluated expression returned FALSE.
#define RETURN_FALSE_IF(_cond, args...)                \
    if ((_cond)) {                                     \
        SERR(" failed evaluated expression is FALSE: " #_cond " : " args); \
        return FALSE;                                           \
    }

// This macro will jump to label if evaluated expression returned NULL.
#define JUMP_ON_FALSE(expr, label) \
    if ((expr) == FALSE) {         \
        SERR("failed");           \
        goto label;               \
        }
// This macro will call the process event function of submodule with event and puts the returned value in rc
#define SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, submodule, event, data, rc)      \
    if (s_bundle->module_sensor_params[(submodule)]->func_tbl.process != NULL){    \
        rc = s_bundle->module_sensor_params[(submodule)]->func_tbl.process(        \
        s_bundle->module_sensor_params[(submodule)]->sub_module_private,           \
        (event), (data));                                                          \
    } else {                                                                       \
        SERR("func_tbl for submodule %d is NULL", submodule);                      \
        rc = SENSOR_FAILURE;                                                       \
    }

/* This macro will perform timed wait for event completion until success or timeout.
 * It ensures that the timeout value specified does not overflow the timespec */
#define PTHREAD_COND_WAIT_TIME(cond_ptr, mutex_ptr, \
    timeout_ptr, _ns_, rc) \
  do { \
    unsigned long ns = _ns_; \
    clock_gettime(CLOCK_MONOTONIC, (timeout_ptr)); \
    while (ns > 999999999) {\
      ns -= 1000000000; \
      (timeout_ptr)->tv_sec += 1; \
    } \
    (timeout_ptr)->tv_nsec += ns;\
    if ((timeout_ptr)->tv_nsec > 999999999) { \
      (timeout_ptr)->tv_nsec -= 1000000000; \
      (timeout_ptr)->tv_sec += 1; \
    } \
    rc = pthread_cond_timedwait(cond_ptr, \
      mutex_ptr, timeout_ptr);\
  } while(0)

/*This macro will perform timed wait for event completion until success or timeout
  * and return failure based if fail set to TRUE*/
#define TIMED_WAIT_ON_EVENT(mutex, cond, ts, timeout, event, fail, str) \
  do {                                                                  \
    int _ret_ = 0;                                                          \
    PTHREAD_MUTEX_LOCK(&mutex);                                         \
    if (event == 0) {                                                   \
      PTHREAD_COND_WAIT_TIME(&cond, &mutex, &ts, timeout, _ret_);       \
    }                                                                   \
    if (fail && event <= 0) {                                           \
        SERR("fatal: " str " event completion failed %d (%d)", event, _ret_); \
        PTHREAD_MUTEX_UNLOCK(&mutex);                                   \
        return FALSE;                                                   \
    }                                                                   \
    PTHREAD_MUTEX_UNLOCK(&mutex);                                       \
  } while(0)

/*This function checks if event is pending or not*/
inline static boolean _check_event_pending(pthread_mutex_t *mutex, int event, boolean *cond) {
  PTHREAD_MUTEX_LOCK(mutex);
  if (event < 0)
    *cond = FALSE;
  if (event == 0) {
    PTHREAD_MUTEX_UNLOCK(mutex);
    return TRUE;
  }
  PTHREAD_MUTEX_UNLOCK(mutex);
  return FALSE;
}

typedef enum {
  /* Sensor framework enums */
  SENSOR_ENUM_MIN,

  /* Get enums */
  SENSOR_GET_CAPABILITIES, /* struct msm_camera_csiphy_params * */
  SENSOR_GET_CUR_CSI_CFG, /* struct msm_camera_csiphy_params * */
  SENSOR_GET_CUR_CHROMATIX_NAME, /* char * */
  SENSOR_GET_INTEGRATION_TIME, /* float * */
  SENSOR_GET_CUR_FPS, /* uint32_t * */
  SENSOR_GET_RESOLUTION_INFO, /* */
  SENSOR_GET_SENSOR_PORT_INFO, /* struct sensor_port_into_array_t * */
  SENSOR_GET_DIGITAL_GAIN, /* float * */
  SENSOR_GET_AEC_DIGITAL_GAIN, /* sensor_digital_gain_t * */
  SENSOR_GET_SENSOR_FORMAT, /* sensor_output_format_t * */
  SENSOR_GET_PROPERTY, /* sensor_property_t */
  SENSOR_GET_RAW_DIMENSION, /* cam_dimension_t * */
  SENSOR_GET_BET_METADATA, /* sensor_bet_metadata_t */
  SENSOR_GET_FRAME_DURATION, /* int64_t * */
  SENSOR_GET_OUTPUT_INFO,
  SENSOR_GET_EXPOSURE_TIME,
  SENSOR_GET_SENSITIVITY,
  SENSOR_GET_EXPOSURE_START_TIME,
  SENSOR_GET_FAST_AEC_WAIT_FRAMES, /* uint16_t */
  SENSOR_GET_TEST_PATTERN_DATA,
  SENSOR_GET_CSID_TEST_MODE,
  SENSOR_GET_EXP_MULTIPLIER,
  SENSOR_GET_SENSOR_MODE,
  SENSOR_GET_META_OUT_DIM,
  SENSOR_GET_FILTER_ARRANGEMENT,
  SENSOR_GET_RAW_OUTPUT_BIT_WIDTH,

  /* Set enums */
  SENSOR_SET_LIB_PARAMS, /* sensor_lib_params_t * */
  SENSOR_INIT, /* NULL */
  SENSOR_STOP_STREAM, /* NULL */
  SENSOR_START_STREAM, /* NULL */
  SENSOR_DELAYED_START_STREAM, /* NULL */
  SENSOR_SET_RESOLUTION, /* sensor_set_res_cfg_t * */
  SENSOR_SET_AEC_UPDATE, /* sensor_set_aec_data_t * */
  SENSOR_SET_AEC_MANUAL_UPDATE,
  SENSOR_SET_AEC_UPDATE_FOR_DUAL,
  SENSOR_SET_AEC_INIT_SETTINGS, /* aec_get_t */
  SENSOR_SET_VFE_SOF, /* NULL */
  SENSOR_SET_FPS, /* uint16_t */
  SENSOR_SET_HFR_MODE, /* cam_hfr_mode_t */
  SENSOR_SET_QUADRA_MODE, /* uint32_t */
  SENSOR_SET_HDR_AE_BRACKET, /* cam_exp_bracketing_t* */
  SENSOR_SET_HDR_MODE, /* cam_sensor_hdr_type_t */
  SENSOR_SET_DIS_ENABLE, /* int32_t * */
  SENSOR_SET_OP_PIXEL_CLK_CHANGE, /* uint32_t * */
  SENSOR_SET_CALIBRATION_DATA, /* msm_camera_i2c_reg_setting * */
  SENSOR_SET_CONTRAST,
  SENSOR_SET_AUTOFOCUS,
  SENSOR_CANCEL_AUTOFOCUS,
  SENSOR_SET_SATURATION,
  SENSOR_SET_BRIGHTNESS,
  SENSOR_SET_SHARPNESS,
  SENSOR_SET_ISO,
  SENSOR_SET_ISO100,
  SENSOR_SET_EXPOSURE_COMPENSATION,
  SENSOR_SET_MANUAL_EXPOSURE_TIME,
  SENSOR_SET_ANTIBANDING,
  SENSOR_SET_EFFECT,
  SENSOR_SET_WHITE_BALANCE,
  SENSOR_SET_MAX_DIMENSION, /* cam_dimension_t * */
  SENSOR_SET_HAL_VERSION, /* int32_t */
  SENSOR_SET_CAPTURE_INTENT, /* int32_t */
  SENSOR_SET_FRAME_DURATION, /* int64_t * */
  SENSOR_SET_AWB_UPDATE, /* sensor_set_awb_data_t * */
  SENSOR_SET_MANUAL_EXPOSURE_MODE, /*sensor_set_manual_exposure_mode */
  SENSOR_SET_HDR_ZSL_MODE,/*sensor_hdr_info_t*/
  SENSOR_SET_AEC_ZSL_SETTINGS, /* aec_get_t */
  SENSOR_SET_TEST_PATTERN,
  SENSOR_SET_FORMATTED_CAL_DATA,
  SENSOR_PDAF_CAL_DEFOCUS,
  SENSOR_PDAF_PARSE_PD,
  SENSOR_PDAF_DEINIT,
  SENSOR_PDAF_GET_NATIVE_PATTERN,
  SENSOR_PDAF_GET_TYPE,
  SENSOR_PDAF_SET_AF_WINDOW,
  SENSOR_SET_DELAY_CFG,
  SENSOR_GET_DELAY_CFG,
  SENSOR_SET_EXP_MULTIPLIER,
  SENSOR_SET_CUR_FLL,
  SENSOR_PDAF_INIT,
  SENSOR_PDAF_SET_BUF_DATA_TYPE, /*Update the Buffer type */
  SENSOR_SET_DUALCAM_SETTINGS,
  SENSOR_SET_META_DIM,
  SENSOR_STANDBY_STREAM,
  SENSOR_SET_AEC_UPDATE_FOR_SLAVE,
  SENSOR_GET_EXPOSURE_INFO,
  SENSOR_SET_ALTERNATIVE_SLAVE,
  SENSOR_ADJUST_DUALC_SLAVE_LINECOUNT,
  SENSOR_DUALC_APPLY_PHASE_CORRECTION,
  SENSOR_ENUM_MAX, /* End of Sensor framework enums */
  /* End of Sensor framework enums */

  CHROMATIX_ENUM_MIN, /* start of chromatix enums */
  /* Get enums */
  CHROMATIX_GET_PTR, /* sensor_chromatix_params_t * */
  CHROMATIX_GET_CACHED_PTR,
  /* Set enums */
  CHROMATIX_SET_CM,
  CHROMATIX_RESET_NAME,
  CHROMATIX_ENUM_MAX, /* End of Chromatix enums*/
  /* End of Actuator enums*/

  ACTUATOR_ENUM_MIN, /* Actuator enums*/
  /* Get enums */
  ACTUATOR_GET_AF_DRIVER_PARAM_PTR,
  ACTUATOR_GET_DAC_VALUE,
  /* Set enums */
  ACTUATOR_INIT, /* NULL */
  ACTUATOR_MOVE_FOCUS, /* af_update_t * */
  ACTUATOR_LOAD_HEADER,
  ACTUATOR_SET_PARAMETERS,
  ACTUATOR_FOCUS_TUNING,
  ACTUATOR_FOCUS_LIVE_TUNING,
  ACTUATOR_ENUM_MAX, /* End of Actuator enums*/
  ACTUATOR_SET_EEBIN_DATA,
  ACTUATOR_GET_LENS_MOVING_RANGE,
  ACTUATOR_SET_POSITION,
  /* End of Actuator enums*/

  EEPROM_ENUM_MIN, /* EEPROM enums*/
  /* Get enums */
  EEPROM_READ_DATA, /*const char **/
  EEPROM_CALIBRATE_WB,
  EEPROM_CALIBRATE_WB_GREEN,
  EEPROM_CALIBRATE_LSC,
  EEPROM_SET_CALIBRATE_DUALCAM_PARAM,
  EEPROM_CALIBRATE_FOCUS_DATA,
  EEPROM_GET_ISINSENSOR_CALIB,
  EEPROM_GET_ISOIS_CALIB,
  EEPROM_GET_FORMATTED_DATA,
  EEPROM_GET_RAW_DATA,
  EEPROM_GET_WB_GRGB,
  EEPROM_GET_WB_CAL,
  EEPROM_GET_OIS_RAW_DATA,
  EEPROM_GET_ISDPC_CALIB,
  /* Set enums */
  EEPROM_SET_FORMAT_DATA,
  EEPROM_WRITE_DATA, /*const char **/
  EEPROM_DUMP_CALIB_DATA,
  EEPROM_INIT,
  EEPROM_ENUM_MAX, /* End of EEEPROM enums*/

  LED_FLASH_ENUM_MIN, /* LED flash enums*/
  LED_FLASH_INIT,
  /* Get enums */
  LED_FLASH_GET_MAX_CURRENT,
  LED_FLASH_GET_MAX_DURATION,
  LED_FLASH_GET_RER_PARAMS,
  LED_FLASH_QUERY_CURRENT, /* uint32_t */
  /* Set enums */
  LED_FLASH_SET_CURRENT,        /* NULL */
  LED_FLASH_SET_RER_CHROMATIX,  /* NULL */
  LED_FLASH_SET_RER_PARAMS,     /* NULL */
  LED_FLASH_SET_RER_PROCESS,    /* NULL */
  LED_FLASH_SET_OFF,                /* NULL */
  LED_FLASH_SET_TORCH,              /* NULL */
  LED_FLASH_SET_PRE_FLASH,          /* NULL */
  LED_FLASH_SET_RER_PULSE_FLASH,    /* NULL */
  LED_FLASH_SET_MAIN_FLASH,         /* NULL */
  LED_FLASH_SET_FIRING_POWER, /* cam_flash_firing_level_t */
  LED_FLASH_SET_FIRING_TIME, /* uinit64_t */
  LED_FLASH_GET_FRAME_SKIP_TIME_PARAMS,

  LED_FLASH_ENUM_MAX, /* End of LED flash enums*/

  STROBE_FLASH_ENUM_MIN, /* Strobe flash enums*/
  /* Get enums */
  /* Set enums */
  STROBE_FLASH_ENUM_MAX, /* End of Strobe flash enums*/

  CSIPHY_ENUM_MIN, /* CSIPHY enums*/
  /* Get enums */
  /* Set enums */
  CSIPHY_SET_CFG, /* struct msm_camera_csiphy_params * */
  CSIPHY_ENUM_MAX, /* End of CSIPHY enums*/

  CSID_ENUM_MIN, /* CSID enums*/
  CSID_GET_VERSION, /* uint32_t * */
  /* Get enums */
  /* Set enums */
  CSID_SET_CFG, /* struct msm_camera_csid_params * */
  CSID_SET_TEST_MODE,
  CSID_SECCAM_SEND_TOPOLOGY,
  CSID_SECCAM_RESET_PIPELINE,
  CSID_ENUM_MAX, /* End of CSID enums*/

  OIS_ENUM_MIN, /* OIS enums*/
  OIS_INIT,
  OIS_ENABLE,
  OIS_DISABLE,
  OIS_MOVIE_MODE,
  OIS_STILL_MODE,
  OIS_CENTERING_ON,
  OIS_CENTERING_OFF,
  OIS_SCENE_FILTER_OFF,
  OIS_SCENE_FILTER_ON,
  OIS_SCENE_RANGE_OFF,
  OIS_SCENE_RANGE_ON,
  OIS_SCENE_SET,
  OIS_PANTILT_ON,
  OIS_CALIBRATION,
  OIS_READ_DATA,
  OIS_ENUM_MAX, /* End of OIS enums*/

  IR_LED_ENUM_MIN,
  IR_LED_SET_OFF, /* NULL */
  IR_LED_SET_ON,  /* sensor_ir_led_cfg_t */
  IR_LED_ENUM_MAX,

  IR_CUT_ENUM_MIN,
  IR_CUT_SET_OFF, /* NULL */
  IR_CUT_SET_ON,  /* NULL */
  IR_CUT_ENUM_MAX,

  LASER_LED_ENUM_MIN,
  LASER_LED_INIT,
  LASER_LED_SET_OFF,
  LASER_LED_SET_ON,
  LASER_LED_SET_CURRENT,
  LASER_LED_ENUM_MAX,

  EXT_ENUM_MIN, /*Ext enums*/
  EXT_AEC_UPDATE,
  EXT_GET_INFO,
  EXT_ENUM_MAX,

  SENSOR_SUBMOD_EVENT_MAX, /* Max value. This should be at last */
} sensor_submodule_event_type_t;

typedef enum {
  ACTUATOR_TUNE_RELOAD_PARAMS,
  ACTUATOR_TUNE_TEST_LINEAR,
  ACTUATOR_TUNE_TEST_RING,
  ACTUATOR_TUNE_DEF_FOCUS,
  ACTUATOR_TUNE_MOVE_FOCUS,
}actuator_tuning_type_t;

typedef struct {
  /* Open func for sub module
     1st param -> Address of function table to be filled by sub module
     2nd param -> Subdev info
     return status -> success / failure */
  int32_t (*open)(void **, void *);
  /* Set param for sub module
     1st param -> module pointer
     2nd param -> event type
     3rd param -> private data
     return status -> success / failure */
  int32_t (*process)(void *, sensor_submodule_event_type_t, void *);
  /* close func for sub module
     1st param -> module pointer
     return status -> success / failure */
  int32_t (*close)(void *);
} sensor_func_tbl_t;

typedef struct {
  chromatix_parms_type *ls_chromatix_ptr;
} sensor_ls_chromatix_params_t;

typedef struct meta_info {
  unsigned char dt;
  unsigned char vc;
  cam_format_t  fmt;
} sensor_meta_info_t;

typedef struct {
  uint32_t width;
  uint32_t height;
  uint32_t stream_mask;
  boolean is_fast_aec_mode_on;
  cam_hfr_mode_t fast_aec_sensor_mode;
  uint32_t aspect_r;
} sensor_set_res_cfg_t;

typedef struct {
  sensor_set_res_cfg_t res_cfg;
  sensor_set_dim_t     output_info;
  sensor_request_crop_t request_crop;
  cam_stream_secure_t   is_secure;
} sensor_get_output_info_t;

typedef struct {
  sensor_set_res_cfg_t res_cfg;
  int32_t              *cfg_done_ptr;
} sensor_set_res_t;

typedef struct {
  struct sensor_csi_params      *csi_params;
  struct sensor_csid_lut_params *lut_params;
  camera_csi_params_t           *camera_csi_params;
  uint8_t                        phy_sel;
  boolean                        is_plain16;
  uint64_t                       data_rate;
  uint32_t                       topology;
  uint8_t                        is_stream_on;
} sensor_csi_config;

typedef union {
  sensor_csi_config                 csi_cfg;
  struct sensor_csid_testmode_parms csid_test_mode_params;
  struct sensor_csiphy_params      *csiphy_params;
  struct sensor_csid_params        *csid_params;
} sensor_get_t;

typedef struct {
  sensor_func_tbl_t   func_tbl;
  void                *sub_module_private;
} module_sensor_params_t;

typedef struct {
  unsigned long major_ver;
  unsigned long minor_ver;
} sensor_lib_version_t;

typedef struct {
  void *handle;
  void (*pdlib_get_version)(void *output);
  signed long (*pdlib_get_defocus)(void *input, void *output);
  int (*pdlib_init)(void *arg1, void *arg2);
  int (*pdlib_deinit)();
  /* special defocus warpper for T1 */
  int (*sensor_custom_calc_defocus)
  (void*, signed long (*get_defocus)(void *input, void *output));

} sensor_custom_API_t;

typedef struct {
  void                *sensor_lib_handle;
  sensor_lib_t        *sensor_lib_ptr;
  sensor_custom_API_t  sensor_custom_API;
} sensor_lib_params_t;

typedef enum {
    ACTUATOR_CAM_MODE_CAMERA,
    ACTUATOR_CAM_MODE_CAMCORDER,
    ACTUATOR_CAM_MODE_MAX,
} actuator_cam_mode_t;

/* General info */
typedef struct {
  uint32_t                  min_frame_idx;     /* cam_frame_idx_range_t */
  uint32_t                  max_frame_idx;     /* cam_frame_idx_range_t */
  uint32_t                  captured_count;    /* captured bracket count*/
  uint8_t                   is_post_msg;       /* meta msg enable flag */
  uint8_t                   enable;            /* enable*/
} sensor_frame_order_ctrl_t;

/* Sensor Bracketing common structures and macro*/
#define ACTUATOR_MAX_WAIT_FRAME  30            /* maximum frames to wait
                                                  for lens movement */
#define ACTUATOR_WAIT_FRAME      1             /* frame delay after lens move */
#define ACTUATOR_BRACKET_MIN     2             /* minimum bracket entry */
#define ACTUATOR_BRACKET_MAX     10            /* maximum bracket entry */

/* Focus bracketing parameters */
typedef struct {
  int32_t                 steps[ACTUATOR_BRACKET_MAX]; /* actuator move steps
                                              for each entry */
  int32_t                 direction[ACTUATOR_BRACKET_MAX]; /* actuator move
                                              direction for each entry */
  uint8_t                 burst_count;     /* burst count */
  uint8_t                 enable;          /* enable */
  uint32_t                lens_reset;      /* flag indicating whether
                                              lens is reset */
  int32_t                 wait_frame;      /* actuator wait frame */
  boolean                 lens_move_progress;  /* flag to indicate lens movement */
  pthread_mutex_t         lens_move_done_sig;/* mutex indicating
                                                end of lens movement */
  sensor_frame_order_ctrl_t ctrl;
} sensor_af_bracket_t;

typedef enum {
  SENSOR_AEC_EST_NONE=0,
  SENSOR_AEC_EST_START,
  SENSOR_AEC_EST_DONE,
  SENSOR_AEC_EST_NO_LED,
} sensor_aec_est_state_t;


/* Flash control trigger events*/
typedef enum {
  SENSOR_DEFAULT_EVT=0,
  SENSOR_AEC_EST_EVT,
  SENSOR_ZSL_START_CAPTURE_EVT,
  SENSOR_ZSL_STOP_CAPTURE_EVT,
  SENSOR_FLASH_EXP_BKT_EVT,
  SENSOR_FLASH_LED_CAL_EVT,
  SENSOR_EXP_BKT_EVT,
} sensor_flash_control_event_t;

/* sensor batch config parameters*/
typedef struct {
  cam_capture_type             type;              /* Type of frame batch*/
  cam_flash_mode_t             cam_flash_mode;    /* HAL defined flash mode*/
  uint8_t                      flash_mode;        /* Internal Flash mode*/
  uint8_t                      led_toggle_pending;/* pending:1, success/fail:0*/
  uint32_t                     ctrl_start_frame;  /* First Valid control frame */
  uint32_t                     ctrl_end_frame;    /* Last valid control frame */
  uint32_t                     burst_count;       /* Burst Count*/
  uint32_t                     interval;          /* Num of frame to repeat setting*/
  uint8_t                      skip_config_mode;  /* skip flash mode config
                                                     in offload thread*/
  int32_t                      low_light_mode;    /* OIS capture */
} sensor_batch_config_t;

/* Flash exposure bracket control parameters*/
typedef struct {
  uint8_t                      enable;              /* Turn on/off */
  uint8_t                      trigger;             /* Trigger new batch setting */
  int8_t                       idx;
  /* 0 - triggered by sof , 1 - stop command expected */
  uint8_t                      mode;
  uint8_t                      delay_batch;         /* Delay current batch setting */
  sensor_flash_control_event_t evt[MAX_CAPTURE_BATCH_NUM]; /* Flash evt */
  int                          delay_evt[MAX_CAPTURE_BATCH_NUM]; /* delay event to end of SOF */
  int32_t                      num_batches;         /* Number of frame batches */
  sensor_batch_config_t        bctrl[MAX_CAPTURE_BATCH_NUM]; /* Frame control for each batch */
  int8_t                       retry_frame_skip;    /* Retry frame skip to avoid partial lit frame */
  uint32_t                     max_pipeline_delay;  /* Pipeline delay */
} sensor_capture_control_t;

/* Flash bracketing parameters */
typedef struct {
  uint8_t                   burst_count;       /* cam_flash_bracketing_t*/
  uint8_t                   enable;            /* cam_flash_bracketing_t */
  uint32_t                  snapshot_identity; /* snapshot stream identity */
  uint32_t                  preview_identity;  /* preview stream identity */
} sensor_flash_bracket_t;

/* Data structure for general bracketing control */
typedef struct {
  uint8_t                   enable;            /* internal enable */
  uint32_t                  captured_count;    /* captured bracket count*/
  uint32_t                  min_frame_idx;     /* cam_frame_idx_range_t */
  uint32_t                  max_frame_idx;     /* cam_frame_idx_range_t */
  uint8_t                   is_post_msg;       /* meta msg enable flag */
} sensor_bracket_ctrl_t;

/* Data structure for general bracketing params */
typedef struct {
  sensor_flash_bracket_t    flash_bracket;     /* cam_flash_bracketing_t*/
  sensor_bracket_ctrl_t     ctrl;              /* sensor_bracket_ctrl_t */
} sensor_bracket_params_t;

typedef struct {
  float   real_gain;
  float   lux_idx;
  int32_t linecount;
} sensor_exp_pack_info_t;

typedef struct {
  uint8_t   hdr_zsl_mode;
  uint8_t   isp_frame_skip;
  stats_get_data_t stats_get;
  sensor_output_format_t output_format;
} sensor_set_hdr_ae_t;

typedef struct {
  uint32_t current_sof;
  uint8_t  post_meta_bus;
  uint8_t  isp_frame_skip;
  int16_t  aec_index;
} sensor_hdr_meta_t;

typedef enum {
  SENSOR_GET_HDR_AE_MODE=0,
  SENSOR_GET_HDR_AE_EXP,
} sensor_get_hdr_ae_type_t;

typedef struct {
  void     *raw_dim;
  uint32_t  stream_mask;
  sensor_set_res_cfg_t *res_cfg;
} sensor_get_raw_dimension_t;

typedef struct {
  uint32_t  position_far_end;
  uint32_t  position_near_end;
} sensor_get_lens_lens_range_t;

typedef struct {
  actuator_cam_mode_t cam_mode;
  af_algo_tune_parms_t *af_tune_ptr;
} sensor_get_af_algo_ptr_t;

typedef struct {
  uint32_t  width;
  uint32_t  height;
  uint32_t  bayer_pattern;
  float     real_gain;
  float     isp_digital_gain;
  uint32_t  exposure_time;
  uint32_t  dac_value;
  float     se_real_gain;
  float     se_digital_gain;
  uint32_t  se_exposure_time;
  uint32_t  reserved;
} sensor_bet_metadata_t;

typedef struct {
  uint32_t frame_id;
  uint32_t identity;
  boolean capture_intent;
  cam_flash_mode_t mode;
} sensor_led_mode_info_t;

typedef struct {
  cam_intf_parm_type_t type;
  union {
    int32_t data;
  } u;
} sensor_yuv_parm_t;

typedef enum {
  SENSOR_FRAME_CTRL_EXP = 0,
  SENSOR_FRAME_CTRL_EXP_META,
  SENSOR_FRAME_CTRL_BET_META,
  SENSOR_FRAME_CTRL_FRAME_DURATION,
  SENSOR_FRAME_CTRL_LED_FLASH,
  SENSOR_FRAME_CTRL_CHANGE_FLASH_STATE,
  SENSOR_FRAME_CTRL_DELAY_CONFIG_CHANGE,
  SENSOR_FRAME_CTRL_REMOVE_IR_CUT_FILTER,
  SENSOR_FRAME_CTRL_IR_LED_ON,
  SENSOR_FRAME_CTRL_IR_CHROMATIX_RELOAD,
  SENSOR_FRAME_CTRL_CALCULATE_EXP_START_TIMESTAMP,
  SENSOR_FRAME_CTRL_TEST_PATTERN_DATA,
  SENSOR_FRAME_CTRL_EXT_AEC_UPDATE,
  SENSOR_FRAME_CTRL_MANUAL_EXPOSURE_TIME,

  /* Post meta */
  SENSOR_FRAME_CTRL_POST_EXP_TIME,
  SENSOR_FRAME_CTRL_POST_FRAME_DURATION,
  SENSOR_FRAME_CTRL_POST_SENSITIVITY,
  SENSOR_FRAME_CTRL_POST_LED_BUS_MSG,
  SENSOR_FRAME_CTRL_POST_LED_DELAY_REQ,
  SENSRO_FRAME_CTRL_POST_EXP_START_TIMESTAMP,
  SENSOR_FRAME_CTRL_POST_ROLLINGSHUTTERSKEW,
  SENSOR_FRAME_CTRL_POST_TEST_PATTERN_DATA,

  /* Sensor internal events*/
  SENSOR_FRAME_CTRL_INT_FPS_RANGE,
  SENSOR_FRAME_CTRL_INT_SET_AFTUNING,
  SENSOR_FRAME_CTRL_INT_TEST_PATTERN,
  SENSOR_FRAME_CTRL_INT_LED_MODE,
  SENSOR_FRAME_CTRL_INT_HAL_SET_PARM,
  SENSOR_FRAME_CTRL_INT_FOCUS_BKT,
  SENSOR_FRAME_CTRL_MAX,
} sensor_frame_ctrl_type_t;

typedef struct {
  float                         aperture;
  float                         filter_density;
  float                         focal_length;
  float                         opt_wb_grgb;
  uint8_t                       ois_mode;
  cam_stream_crop_info_t        snap_crop_sensor;
  cam_stream_crop_info_t        snap_crop_camif;
//  mct_bus_msg_sensor_metadata_t chromatix_metadata;
} sensor_static_medatadata_t;

typedef struct {
  float                 real_gain;
  float                 sensor_gain;
  int32_t               linecount;
  aec_led_est_state_t   est_state;
  float                 lux_idx;
  float                 s_real_gain;
  int32_t               s_linecount;
  float                 hdr_sensitivity_ratio;
  float                 hdr_exp_time_ratio;
  float                 total_drc_gain;
} sensor_aec_update_t;

typedef struct {
  sensor_aec_update_t *aec_update;
  float                digital_gain;
} sensor_digital_gain_t;

/** sensor_flash_params_t:
 *
 *  @flash_ctrl: flash control mode
 *
 *  @flash_mode: flash mode
 *
 *  @flash state: flash state
 **/
typedef struct {
  cam_flash_mode_t    flash_mode;
  cam_flash_state_t   flash_state;
} sensor_flash_params_t;

typedef struct {
  boolean    ir_led_event;
  float      ir_led_brightness;
} sensor_ir_led_params_t;

typedef struct {
  uint32_t                  current_frame_id;
  sensor_frame_ctrl_type_t  frame_ctrl_type;
  union {
    sensor_bet_metadata_t         bet_meta;
    int64_t                       frame_duration;
    sensor_aec_update_t           aec_update;
    sensor_submodule_event_type_t flash_mode;
    int64_t                       post_frame_time;
    int64_t                       post_exp_time;
    int32_t                       post_sensitivity;
    sensor_submodule_event_type_t flash_event;
    sensor_flash_params_t         flash_params;
    cam_flash_ctrl_t              flash_ctrl;
    sensor_exp_pack_info_t        sensor_exp;
    int64_t                       exp_time;
    int64_t                       manual_exp_time;
    int64_t                       post_exp_start_timestamp;
    int64_t                       frame_readout_time;
    cam_test_pattern_data_t       test_pattern;
    cam_fps_range_t               fps_info;
    tune_actuator_t               tdata;
    cam_test_pattern_data_t       tpattern;
    sensor_led_mode_info_t        led_info;
    sensor_yuv_parm_t             yuv_parm;
    cam_af_bracketing_t           af_bkt;
    int32_t                       delay_state;
    sensor_submodule_event_type_t ir_cut_event;
    sensor_ir_led_params_t        led_params;
    boolean                       ir_mode;
  } u;
} sensor_frame_ctrl_data_t;

typedef struct {
  mct_pipeline_session_data_t    session_data;
  mct_queue_t                   *frame_ctrl_q[FRAME_CTRL_SIZE];
  pthread_mutex_t                frame_ctrl_mutex[FRAME_CTRL_SIZE];
  sensor_frame_ctrl_data_t       sensor_frame_ctrl_data;
  sensor_static_medatadata_t     static_metadata;
  cam_format_t                   fmt;
  uint32_t                       max_sensor_delay;
} sensor_frame_ctrl_params_t;

/* AE bracketing parameters */
typedef struct {
  uint8_t                 enable;          /* cam_ae_bracketing_t */
  float real_gain[AE_BRACKET_MAX_ENTRIES];
  uint32_t linecount[AE_BRACKET_MAX_ENTRIES];
  float lux_idx;
} sensor_ae_bracket_t;
/** sensor_hal_params_t:
 *
 *  @control_mode: control mode indicates whether HAL is
 *               controlling params
 *  OFF - All 3A controls from HAL is disabled
 *
 *  AUTO - Individual algorithms control 3A params
 *
 *  USE_SCENE_MODE - HAL implements one of scene mode
 *
 *  @ae_mode: AEC control
 *
 *  OFF - manual control for exposure, sensitivity and frame
 *  duration
 **/
typedef struct {
  cam_control_mode_t      control_mode;
  cam_ae_mode_type        ae_mode;
  int32_t                 hal_frame_skip_pattern;
  cam_test_pattern_mode_t test_pattern_mode;
} sensor_hal_params_t;

 typedef struct {
  char sensor_sd_name[MAX_SUBDEV_SIZE];
  int32_t subdev_id;
} sensor_submodule_intf_info_t;

typedef enum {
  SUBDEV_INTF_PRIMARY = 0,
  SUBDEV_INTF_SECONDARY,
  SUBDEV_INTF_MAX,
} sensor_subdev_intf_type_t;

typedef struct {
  sensor_submodule_intf_info_t intf_info[SUBDEV_INTF_MAX];
  uint8_t intf_mask;
  int8_t  sub_mod_open_flag;
  void * data;
} sensor_submodule_info_t;

typedef struct {
  float           exp_multiplier;
  float           adjusted_frame_duration;
  uint16_t        master_lc;
  uint16_t        master_fl_lines;
  stats_update_t *stats_update;
} sensor_dual_exp_t;

typedef enum {
  SENSOR_EXTERNAL_LIB_AEC,
  SENSOR_EXTERNAL_LIB_AWB,
  SENSOR_EXTERNAL_LIB_AF,
  SENSOR_EXTERNAL_LIB_MAX,
} sensor_external_library_t;

typedef enum {
  SENSOR_MODE_PREVIEW,
  SENSOR_MODE_SNAPSHOT,
  SENSOR_MODE_ZSL,
  SENSOR_MODE_VIDEO,
  SENSOR_MODE_RAW,
  SENSOR_MODE_DEPTH,
  SENSOR_MODE_MAX
} sensor_mode_t;

typedef enum {
  SENSOR_IR_MODE_OFF,
  SENSOR_IR_MODE_ON
} sensor_ir_mode_t;

typedef struct {
  float intensity;
} sensor_ir_led_cfg_t;

typedef struct {
  float current;
} sensor_laser_led_cfg_t;


typedef enum {
    SENSOR_PACK_MODE_NONE,
    SENSOR_PACK_MODE_PLAIN8,
    SENSOR_PACK_MODE_PLAIN16
} sensor_pack_mode_t;

typedef struct {
  camera_module_config_t camera_config;
  uint64_t               special_mode_mask;
  uint64_t               prev_special_mode_mask;
  sensor_mode_t          sensor_mode;
  sensor_lib_params_t   *sensor_lib_params;
  sensor_output_format_t output_format;
  sensor_ae_bracket_t    ae_bracket_params;
  sensor_ir_mode_t       ir_mode;
  cam_stream_secure_t    is_secure;
} sensor_submod_common_info_t;

typedef struct {
  /* HW-Sync Master(0) or Slave(1) in Dual Camera operation */
  uint32_t  dual_mode;
  /* 1 if this session is sleeping */
  boolean   sensor_sleeping;
  /* 0 if this session is preview master */
  uint32_t  streaming_master;
  /* Frame ID of this session wrt peer */
  uint32_t  frame_idx;
  /* Timestamp for phase correction calculations in ms wrt peer */
  uint32_t  sof_timestamp;
  /* Checks the number of frames to wait for for correction to be applied */
  int32_t   phase_correction_count;
  /* Indicates when sensor should resume using AEC update */
  boolean   reset_correction;
  /* Indicates when fps matching and phase correction needs to be enabled */
  boolean   fps_match_enabled;
  /* Indicates what sycn mechanism is being used in the system */
  cam_sync_mechanism_t sync_mechanism;
} sensor_dualc_info;

/* peer_info_t - returns the frame id and timestamp of the peer
 * session while calculating phase difference in dual camera.
 */
typedef struct peer_info_t {
    uint32_t  peer_frameid;
    uint32_t  peer_timestamp;
} peer_info_t;

typedef struct module_sensor_bundle_info_t {
  /* info common for module and sensor submodule */
  sensor_submod_common_info_t    sensor_common_info;
  /* for intra port communication : initial value -1*/
  uint32_t                       peer_identity;

  /* Entity to store subdev name for all imager modules */
  char sensor_sd_name[SUB_MODULE_MAX][MAX_SUBDEV_SIZE];
  /* Entity to store information of all subdev interfaces */
  sensor_submodule_info_t        subdev_info[SUB_MODULE_MAX];
  /* Entity to store sensor name and index of its sub modules */
  struct msm_sensor_info_t      *sensor_info;
  /* Sensor library params */
  sensor_lib_params_t           *sensor_lib_params;
  /* chromatix manager */
  chromatix_manager_type         chromatix_manager;
  /* eeprom data */
  format_data_t                 *formatted_data;
  /* Sensor module params, these params will to be created and destroyed
     during sensor open and release */
  module_sensor_params_t        *module_sensor_params[SUB_MODULE_MAX];
  /* stream ref count */
  uint16_t                       ref_count;
  /* max width of all streams */
  uint32_t                       max_width;
  /* max height of all streams */
  uint32_t                       max_height;
  /* stream mask of all streams */
  uint32_t                       stream_mask;
  /* refcount for non bundle stream on / off */
  int32_t                        stream_on_count;
  uint32_t                       last_idx;
  uint16_t                       num_skip;
  /* 1 started, 2, done, 3 done no led */
  sensor_aec_est_state_t         state;
  uint32_t                       regular_led_trigger;
  int32_t                        regular_led_af;
  cam_flash_mode_t               last_flash_request;
  uint32_t                       stream_thread_wait_time;
  /* store chromatix pointers to post to bus */
  mct_bus_msg_sensor_metadata_t    chromatix_metadata;
  /* store trigger update to post to bus */
  mct_bus_msg_stats_aec_metadata_t aec_metadata;
  sensor_eeprom_data_t            *eeprom_data;
  /* Store sensor_params to post to bus */
  cam_sensor_params_t            sensor_params;
  uint32_t                       torch_on;
  int32_t                        longshot;
  cam_fps_range_t                fps_info;
  sensor_frame_ctrl_params_t     frame_ctrl;
  pthread_mutex_t                mutex;
  pthread_cond_t                 cond;
  pthread_condattr_t             condattr;
  int32_t                        init_config_done;
  int32_t                        close_done;
  int32_t                        close_pending;
  int32_t                        open_done;
  int32_t                        res_cfg_done;
  int32_t                        actuator_cfg_done;
  int32_t                        parm_pending;
  /* To be set when set/get parm need to be blocked */
  int32_t                        block_parm;
  /* Store HAL version */
  int32_t                        hal_version;
  /* Store capture intent */
  int32_t                        capture_intent;
  sensor_hal_params_t            hal_params;
  sensor_flash_params_t          flash_params;
  /* Sensor Bracketing Feature Specific */
  sensor_af_bracket_t            af_bracket_params;
  sensor_bracket_params_t        flash_bracket_params;
  sensor_capture_control_t       cap_control;

  /* counter for LED off in burst mode */
  int16_t                        led_off_count;
  float                          led_off_gain;
  uint32_t                       led_off_linecount;
  uint32_t                       burst_num;
  uint32_t                       led_on_period;
  uint32_t                       flash_rer_enable;
  /**** Flash control info ****/
  uint32_t                       main_flash_on_frame_skip;
  uint32_t                       main_flash_off_frame_skip;
  uint32_t                       torch_on_frame_skip;
  uint32_t                       torch_off_frame_skip;
  int8_t                         retry_frame_skip;
  uint32_t                       capture_pipeline_delay;
  uint8_t                        partial_flash_frame_skip;
  int8_t                         batch_idx;
  uint32_t                       delay_frame_idx;
  pthread_mutex_t                capture_control_mutex;
  /**********************/
  int32_t                        pfd[2];
  uint32_t                       isp_frameskip[CAM_STREAM_TYPE_MAX];
  uint8_t                        is_bayer;
  uint32_t                       identity[CAM_STREAM_TYPE_MAX];
  /*This mask is used to execute OIS commands after OIS init is done*/
  uint16_t                       ois_cmd_queue_mask;
  /* During fast aec mode, mctl thread will block on this thread */
  pthread_cond_t                 fast_aec_cond;
  pthread_condattr_t             fast_aec_condattr;
  /* Mutex used for fast aec mode */
  pthread_mutex_t                fast_aec_mutex;
  /* number of frames to skip for fast AEC use case */
  uint16_t                       sensor_num_fast_aec_frame_skip;
  uint16_t                       fast_aec_wait_count;
  boolean                        fast_aec_is_running;
  boolean                        fast_aec_required;
  boolean                        is_plain16;
  uint32_t                       ois_enabled;
  int32_t                        ois_mode;
  uint32_t                       max_isp_frame_skip;
  void *                         external_library[SENSOR_EXTERNAL_LIB_MAX];
  float                          actuator_sensitivity;
  cam_stream_size_info_t         stream_size_info;
  cam_capture_frame_config_t     hal_frame_batch; /*from HAL*/
  int32_t                        live_connect_fd[2];
  boolean                        live_connect_thread_started;
  uint32_t                       is_valid_dualcalib;
  cam_related_system_calibration_data_t dualcam_tune_data;
  uint8_t                        subsequent_bundle_stream;
  uint32_t                       delay_frame_cnt;
  uint32_t                       ext_pipeline_delay;
  uint32_t                       cur_scene_mode;
  float                          digital_gain;
  void                           *isp_cap;
  camif_cap_t                    camif_data_fmt;
  uint32_t                       dual_led_calib_enabled;
  uint16_t                       cur_logical_lens_pos;
  sensor_ir_mode_t               previous_ir_mode;
  sensor_ir_mode_t               ir_mode;
  float                          previous_ir_led_brightness;
  float                          ir_led_brightness;
  boolean                        is_stereo_configuration;
  struct module_sensor_bundle_info_t*   stereo_peer_s_bundle;
  sensor_pack_mode_t             pack_mode;
  sensor_dualc_info              dualc_info;
  uint32_t                       topology;
  uint8_t                        is_first_stream_on;
  uint8_t                        is_camera_closing;
} module_sensor_bundle_info_t;

typedef struct {
  module_sensor_bundle_info_t *s_bundle;
  uint32_t                     session_id;
  uint32_t                     stream_id;
} sensor_bundle_info_t;

typedef struct {
  /*Sensor module information for each sensor */
  mct_list_t                 *sensor_bundle; /* module_sensor_bundle_info_t * */
  void                       *eebin_hdl;
  /*Number of sensor bundle information - one per sensor successfully probed */
  uint8_t                     size;
  mct_stream_info_t           streaminfo;
  int32_t                     session_count;

  /* Two sessions are linked for dual cam support. */
  boolean                     is_dual_cam;
  /* is_dual_stream_on:
   * PRIMARY session- sets to true to indicate a successful streamon
   * SLAVE session- sets to true to indicate it is ready to be streamed on */
  boolean                     is_dual_stream_on;
  /* actual streaming for both sessions */
  boolean                     is_dual_streaming;
  pthread_mutex_t             dual_cam_mutex;
} module_sensor_ctrl_t;

typedef enum {
  ACTUATOR_LIVE_TUNE_GET_CUR_STEP_POSITION,
  ACTUATOR_LIVE_TUNE_GET_CUR_LENS_POSITION,
  ACTUATOR_LIVE_TUNE_LINEARITY_TEST,
  ACTUATOR_LIVE_TUNE_RINGING_TEST,
  ACTUATOR_LIVE_TUNE_START_PLOT,
  ACTUATOR_LIVE_TUNE_STOP_PLOT,
  ACTUATOR_LIVE_TUNE_SET_DEFAULT_FOCUS,
}actuator_live_tune_cmd;

typedef struct {
  int32_t size;
  int32_t step_pos[MAX_ACTUATOR_PLOT_INFO];
  int32_t lens_pos[MAX_ACTUATOR_PLOT_INFO];
} actuator_plot_t;

typedef struct {
  actuator_live_tune_cmd tuning_cmd;
  union {
    int32_t lens_position;
    int32_t step_position;
    int32_t delay;
    actuator_plot_t plot_info;
  } u;
  uint8_t step_size;
}actuator_live_tune_ctrl_t;

int32_t sensor_load_library(const char *name, void *sensor_lib_params,
  char *path);
boolean sensor_load_binary(const char *name, void *data, char *path);
int32_t sensor_unload_library(sensor_lib_params_t *sensor_lib_params);
int32_t sensor_sub_module_init(sensor_func_tbl_t *func_tbl);
int32_t chromatix_sub_module_init(sensor_func_tbl_t *func_tbl);
int32_t eeprom_sub_module_init(sensor_func_tbl_t *func_tbl);
int32_t actuator_sub_module_init(sensor_func_tbl_t *func_tbl);
int32_t led_flash_sub_module_init(sensor_func_tbl_t *func_tbl);
int32_t strobe_flash_sub_module_init(sensor_func_tbl_t *func_tbl);
int32_t csiphy_sub_module_init(sensor_func_tbl_t *func_tbl);
int32_t csid_sub_module_init(sensor_func_tbl_t *func_tbl);
int32_t ois_sub_module_init(sensor_func_tbl_t *func_tbl);
int32_t external_sub_module_init(sensor_func_tbl_t *func_tbl);

int32_t eeprom_load_library(sensor_eeprom_data_t *e_ctrl, char *path);
int32_t eeprom_unload_library(sensor_eeprom_data_t *e_ctrl);
int32_t load_chromatix(const char *name, const char *path,
  void **handle, void **symbol);
int32_t unload_chromatix(void *handle, void* symbol);
int32_t ir_led_sub_module_init(sensor_func_tbl_t *func_tbl);
int32_t ir_cut_sub_module_init(sensor_func_tbl_t *func_tbl);
int32_t laser_led_sub_module_init(sensor_func_tbl_t *func_tbl);

#endif
