/*===========================================================================
Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#ifndef __VFE_TEST_VECTOR_H__
#define __VFE_TEST_VECTOR_H__

#include "isp_pipeline.h"
#include "cam_types.h"
#include <stdio.h>
#include <string.h>
#include "aec/aec.h"
#include "chromatix.h"
#include "chromatix_common.h"

extern int show_mismatch;
extern int has_mismatch;

#define CDBG_TV  printf

#ifndef Q_TO_FLOAT
#define Q_TO_FLOAT(exp, q) \
  ((double)((double)(q-(q/(1<<(exp)))*(1<<(exp)))/(1<<(exp))+(q/(1<<(exp)))))
#endif

typedef struct {
  chromatix_manual_white_balance_type awb_gains;
  uint32_t color_temp;
  camera_flash_type flash_mode;
  flash_sensitivity_t sensitivity_led;
  flash_sensitivity_t sensitivity_strobe;
  float lux_idx;
  float cur_real_gain;
  float digital_gain;
  q3a_ihist_stats_t ihist_stats;
  q3a_bg_stats_t bg_stats;
  uint32_t raw_bhist_stats[4096*4];
  int32_t led_enable;
  int32_t strobe_enable;
  float led1_current;
  float led2_current;
} vfe_test_params_t;

typedef struct {
  uint32_t *table;
  uint32_t size;
  uint32_t mask;
} vfe_test_table_t;

typedef struct {
  uint64_t *table;
  uint32_t size;
  uint64_t mask;
} vfe_test_table_64_t;

typedef struct {
  vfe_test_table_t gr_r;
  vfe_test_table_t gb_b;
} vfe_test_mesh_table_t;

typedef struct {
  uint32_t *reg_dump;
  uint32_t *reg_mask;
  uint32_t reg_size;
  vfe_test_table_t gamma_r;
  vfe_test_table_t gamma_g;
  vfe_test_table_t gamma_b;
  vfe_test_table_t la;
  vfe_test_table_64_t linearization;
  vfe_test_mesh_table_t mesh_rolloff;
  vfe_test_table_t abf_sig2_l0;
  vfe_test_table_t abf_sig2_l1;
  vfe_test_table_t abf_std2_l0;
  vfe_test_table_t abf_std2_l1;
  vfe_test_table_t abf_mesh_2D;
  vfe_test_table_t gic_std2_l0;
  vfe_test_table_t gic_sig2_l0;
  vfe_test_table_t pedestal_t1_gr_r;
  vfe_test_table_t pedestal_t1_gb_b;
  vfe_test_table_t pedestal_t2_gr_r;
  vfe_test_table_t pedestal_t2_gb_b;
  vfe_test_table_64_t gtm;
  vfe_test_table_t ltm_weight;
  vfe_test_table_t ltm_mask_curve;
  vfe_test_table_t ltm_master_curve;
  vfe_test_table_t ltm_master_scale;
  vfe_test_table_t ltm_shift_curve;
  vfe_test_table_t ltm_shift_scale;
  vfe_test_table_t ltm_sat_curve;
} vfe_test_module_data_t;

typedef vfe_test_module_data_t vfe_test_module_input_t;
typedef vfe_test_module_data_t vfe_test_module_output_t;

typedef struct {
  int width;
  int height;
} camera_size_t;

typedef struct {
  boolean mismatch[ISP_MOD_MAX_NUM];
  char* module_name[ISP_MOD_MAX_NUM];
} testcase_report_t;

typedef struct {
  int enable;
  FILE *fp;
  camera_size_t camif_size;
  camera_size_t output_size;
  camera_size_t full_size;
  char *input_data;
  int input_size;
  int current;
  void* hw_prms; /* pmg This is isp_hw_update_params_t */
  int parse_mode;
  vfe_test_params_t params;
  int snapshot_mode;
  vfe_test_module_input_t mod_input;
  isp_hw_module_id_t module_type;
  int rolloff_type;
  vfe_test_module_output_t mod_output;
  uint64_t modules_to_test;
  uint32_t sensor_scaling;
  uint32_t x_offset;
  uint32_t y_offset;
  chromatix_parms_type *chromatix_data;
  int adrc_enable;
  float adrc_total_gain;
  float adrc_color_gain;
  float adrc_gamma_ratio;
  float adrc_ltm_ratio;
  float adrc_la_ratio;
  float adrc_gtm_ratio;
  uint8_t bayer_pattern;
} vfe_test_vector_t;

typedef enum {
  VFE_SUCCESS = 0,
  VFE_ERROR_GENERAL,
  VFE_ERROR_INVALID_OPERATION,
  VFE_ERROR_NO_MEMORY,
  VFE_ERROR_NOT_SUPPORTED,
}vfe_status_t;

vfe_status_t vfe_test_vector_init(vfe_test_vector_t *mod, char* datafile);
vfe_status_t vfe_test_vector_deinit(vfe_test_vector_t *mod);
vfe_status_t vfe_test_vector_fetch_params(
  vfe_test_vector_t *mod,
  q3a_ihist_stats_t *ihist_stats,
  q3a_bg_stats_t    *bg_stats);

vfe_status_t vfe_test_vector_execute(vfe_test_vector_t *mod,
  testcase_report_t* testcase_report, char *mod_name[]);

#define VALIDATE_TST_VEC(ip, op, range, str)({\
  if (abs(ip - op) > range) {\
      has_mismatch = 1; \
      if (show_mismatch) \
      CDBG_TV("\tBET: %s: Mismatch, %s, diff: %d, ip: %08x, op : %08x\n",\
      __func__, str, abs(ip - op), ip, op);\
  }\
})

#define VALIDATE_TST_VEC2(ip, op, range, str1, str2)({\
    if (abs(ip - op) > range) {\
      has_mismatch = 1; \
      if (show_mismatch) \
        CDBG_TV("\tBET: %s %s: Mismatch, %s, diff: %d, ip: %08x, op : %08x\n",\
        __func__, str1, str2, abs(ip - op), ip, op);\
    }\
  })

#define VALIDATE_TST_LUT(ip, op, range, str, index)({\
    if ((ip - op) > range) {\
      has_mismatch = 1; \
      if (show_mismatch) \
        CDBG_TV("\tBET: %s: Mismatch, %s, Index :%d, diff: %d, ip: %08x, op : %08x\n",\
         __func__, str, index, (ip - op), ip, op);\
    }\
})

#endif //__VFE_TEST_VECTOR_H__
