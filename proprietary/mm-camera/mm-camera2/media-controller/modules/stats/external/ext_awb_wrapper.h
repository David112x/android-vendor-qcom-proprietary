/* ext_awb_wrapper.h
 *
 * Copyright (c) 2015-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __EXT_AWB_WRAPPER_H__
#define __EXT_AWB_WRAPPER_H__

#include <cam_types.h>
#include <awb.h>
#include <mct_event_stats.h>

#define EXT_AWB_ALGO_MAX_BG_STATS_NUM (72 * 54)
#define CCM_SIZE                      (3 * 3)
#define COLOR_RATIO_SIZE              (3)

#define _CCM_ROW        3
#define _CCM_COL        3
#define _CCM_TOTAL      (_CCM_ROW * _CCM_COL)

#define ROI_CNT     256     /* (16 * 16) */
#define CENTER_IDX  1568    /* (64 * 24 + 32) */

typedef void * _handle;

/* Pre-defined structures */
typedef struct _ext_awb_algo_dual_led_info_t {
  int enable;
  int cct;
  int brightness;
  int led_cool;
  int led_warm;
} ext_awb_algo_dual_led_info_t;

typedef struct _ext_awb_algo_get_args_t {
  float wbgain_r;
  float wbgain_g;
  float wbgain_b;
  int ctemp;
  float ccm[CCM_SIZE];
  ext_awb_algo_dual_led_info_t dual_led_info;
} ext_awb_algo_get_args_t;

typedef struct _ext_awb_algo_process_inargs_t {
  unsigned long r[EXT_AWB_ALGO_MAX_BG_STATS_NUM];  /**< WD data R per block */
  unsigned long g[EXT_AWB_ALGO_MAX_BG_STATS_NUM];  /**< WD data G per block */
  unsigned long b[EXT_AWB_ALGO_MAX_BG_STATS_NUM];  /**< WD data B per block */
  unsigned long c[EXT_AWB_ALGO_MAX_BG_STATS_NUM];  /**< sum   */
  unsigned short blockH;   //Statistics block Horizontal number per frame
  unsigned short blockV;   //Statistics block Vertical number per frame
  unsigned short blockPixH;  //pixel number Horizontal per block
  unsigned short blockPixV;  //pixel number Vertical per block
} ext_awb_algo_process_inargs_t;

typedef struct _ext_awb_algo_process_outargs_t {
  ext_awb_algo_get_args_t   result;
  unsigned char*            debug_information;
  unsigned long             debug_information_size;
} ext_awb_algo_process_outargs_t;

typedef struct _ext_awb_algo_set_init_args_t {
  struct {
    float rg;
    float bg;
  } color_ratio[COLOR_RATIO_SIZE];
  int valid_num;
} ext_awb_algo_set_init_args_t;

typedef struct _ext_awb_algo_set_aec {
  float  Av_wb;
  float  Tv_wb;
  float  Sv_wb;

  float  Ev_wb;
  float  Bv_wb;
  int led_state;
  int use_led_algo;

  float flash_sensitivity_off;
  float flash_sensitivity_low;
  float flash_sensitivity_high;

  int led_mode;
} ext_awb_algo_set_aec_t;

typedef struct _ext_awb_algo_get_version_t {
  char* model;
  char* version;
  char* description;
} ext_awb_algo_get_version_t;

typedef int (* _ext_awb_algo_init_front)(void **handle);
typedef int (* _ext_awb_algo_init_rear)(void **handle);
typedef int (* _ext_awb_algo_deinit)(void *handle);
typedef int (* _ext_awb_algo_process)(void *handle,
    ext_awb_algo_process_inargs_t *,
    ext_awb_algo_process_outargs_t *);
typedef int (* _ext_awb_algo_set_parm_init_chromatix)(void *handle,
    ext_awb_algo_set_init_args_t *args);
typedef int (* _ext_awb_algo_set_parm_aec_parm)(void *handle,
    ext_awb_algo_set_aec_t *args);
typedef int (* _ext_awb_algo_set_parm_whaitbalance)(void *handle, void *args);
typedef int (* _ext_awb_algo_set_parm_bestshot)(void *handle, void *args);
typedef int (* _ext_awb_algo_set_parm_opmode)(void *handle, void *args);
typedef int (* _ext_awb_algo_set_parm_lock)(void *handle, void *args);
typedef int (* _ext_awb_algo_get_parm_awb_gains)(void *handle,
    ext_awb_algo_get_args_t *args);
typedef int (* _ext_awb_algo_get_parm_awb_params)(void *handle,
    ext_awb_algo_get_args_t *args);
typedef int (* _ext_awb_algo_get_parm_led_gains)(void *handle,
    ext_awb_algo_get_args_t *args);
typedef int (* _ext_awb_algo_get_parm_pre_led_gains)(void *handle,
    ext_awb_algo_get_args_t *args);
typedef int (* _ext_awb_algo_estimate_cct)(void *handle,
    float rgain, float ggain, float bgain, float *colortemp);
typedef int (* _ext_awb_algo_estimate_ccm)(void *handle,
    float colortemp, float *ccm);
typedef int (* _ext_awb_algo_get_version_information)(void *args);

typedef struct _ext_awb_algo_ops_t {
  _ext_awb_algo_init_front                 init_front;
  _ext_awb_algo_init_rear                  init_rear;
  _ext_awb_algo_deinit                     deinit;
  _ext_awb_algo_process                    process;
  _ext_awb_algo_set_parm_init_chromatix    init_chromatix;
  _ext_awb_algo_set_parm_aec_parm          set_aec;
  _ext_awb_algo_set_parm_whaitbalance      set_whitebalance;
  _ext_awb_algo_set_parm_bestshot          set_bestshot;
  _ext_awb_algo_set_parm_opmode            set_opmode;
  _ext_awb_algo_set_parm_lock              set_lock;
  _ext_awb_algo_get_parm_awb_gains         get_gains;
  _ext_awb_algo_get_parm_awb_params        get_awb_parms;
  _ext_awb_algo_get_parm_led_gains         get_led_gains;
  _ext_awb_algo_get_parm_pre_led_gains     get_pre_led_gains;
  _ext_awb_algo_estimate_cct               estimate_cct;
  _ext_awb_algo_estimate_ccm               estimate_ccm;
  _ext_awb_algo_get_version_information    get_version;
} ext_awb_algo_ops_t;

typedef struct _ext_awb_algo_bestshot_t {
  int                         stored_wb;
  awb_bestshot_mode_type_t    curr_mode;
} ext_awb_algo_bestshot_t;

typedef struct _ext_awb_algo_stats_enable_t {
  uint32  awb_enable_stats_mask;
  uint32  awb_config_stats_mask;
} ext_awb_algo_stats_enable_t;

typedef struct _result {
  chromatix_manual_white_balance_type   gains;
  uint32_t                              colortemp;
  awb_ccm_type                          ccm;
  int                                   ccm_enable;
} algo_result;

typedef struct {
  void                                  *libptr;    /* ALGO file lib pointer */
  void                                  *opsptr;    /* ALGO ops lib pointer */

  cam_position_t                        position;
  int                                   awb_update;
  uint32_t                              sof_id;
  awb_operation_mode_t                  op_mode;
  uint32_t                              frame_id;
  aec_led_est_state_t                   est_state;
  int                                   use_led_aec;
  int                                   use_led_algo;
  q3a_flash_sensitivity_type            flash_si;
  boolean                               awb_locked;
  boolean                               hal_lock;
  boolean                               is_preflash;
  boolean                               is_preflash_awb;
  boolean                               is_preflash_af;
  boolean                               is_ledgain;
  boolean                               is_ledgain_valid;
  boolean                               is_flash;
  boolean                               is_restoregain;

  algo_result                           normal;
  algo_result                           led;
  algo_result                           keepgain;
  awb_dual_led_settings_t               dual_led_info;

  stats_t                               stats;
  ext_awb_algo_stats_enable_t           stats_enable;

  ext_awb_algo_bestshot_t               bestshot;
  awb_config3a_wb_t                     wb;
  float                                 Bv;
  float                                 Tv;
  float                                 Sv;
  float                                 Av;

  unsigned char                         *exif;
  uint32_t                              exif_size;

  ext_awb_algo_set_init_args_t          chromatix;
  ext_awb_algo_ops_t                    ops;
  ext_awb_algo_get_version_t            libver;

} ext_awb_algo_control_t;

void *ext_awb_algo_front_init(void *obj);
void *ext_awb_algo_rear_init(void *obj);
void ext_awb_algo_deinit(void *obj);

boolean ext_awb_algo_get_param(awb_get_parameter_t *param, void *obj);
boolean ext_awb_algo_set_param(awb_set_parameter_t *param,
  awb_output_data_t *output, uint8_t num_of_outs, void *obj);
boolean ext_awb_algo_estimate_cct(void *obj,
  float rgain, float ggain, float bgain, float *colortemp, uint8_t camera_id);
boolean ext_awb_algo_estimate_ccm(void *obj, float cct, awb_ccm_type *ccm,
  uint8_t camera_id);
void ext_awb_algo_process(stats_t *stats, void *obj,
  awb_output_data_t *output, uint8_t num_of_outs);

#endif /* __EXT_AWB_WRAPPER_H__ */
