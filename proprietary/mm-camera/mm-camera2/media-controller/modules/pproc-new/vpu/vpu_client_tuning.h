/*============================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

#ifndef VPU_CLIENT_TUNING_H
#define VPU_CLIENT_TUNING_H

#include <stdint.h>
#include <stdbool.h>
#include "chromatix_cpp.h"

#define VPU_EXT_CTRL_SUB_ID             805306642
#define VPU_EXT_CMD_ID_NR_LEVEL_TABLE   68
#define VPU_EXT_CMD_ID_NR_CONFIG_TABLE  77

/* nr level table dimensions, length includes scope field */
#define NR_LEVEL_TABLE_NUM_ROWS   11
#define NR_LEVEL_TABLE_NUM_COLS   7
#define NR_LEVEL_TABLE_DATA_LENGTH \
  (NR_LEVEL_TABLE_NUM_ROWS * NR_LEVEL_TABLE_NUM_COLS)

/* col indexes in nr level table */
#define VPU_NR_LEVEL_TABLE_COL_RNR    0
#define VPU_NR_LEVEL_TABLE_COL_SRNR   3
#define VPU_NR_LEVEL_TABLE_COL_TNR    4

/* nr config table dimensions */
#define NR_CONFIG_TABLE_NUM_ROWS   48
#define NR_CONFIG_TABLE_NUM_COLS   16

#define NR_CONFIG_TABLE_DATA_LENGTH \
  (NR_LEVEL_TABLE_NUM_ROWS * NR_LEVEL_TABLE_NUM_COLS + 2)

/* row indexes in nr config table  */
#define VPU_NR_CONFIG_TABLE_ROW_KMINY         2
#define VPU_NR_CONFIG_TABLE_ROW_KMINC         3
#define VPU_NR_CONFIG_TABLE_ROW_RNRFFACT      7
#define VPU_NR_CONFIG_TABLE_ROW_TXTTHR4       11
#define VPU_NR_CONFIG_TABLE_ROW_TXTTHR3       15
#define VPU_NR_CONFIG_TABLE_ROW_RNRLEVEL      16
#define VPU_NR_CONFIG_TABLE_ROW_CALCUPLIMIT   32
#define VPU_NR_CONFIG_TABLE_ROW_RNRMINNOISE   38
#define VPU_NR_CONFIG_TABLE_ROW_RNRMAXNOISE   39
#define VPU_NR_CONFIG_TABLE_ROW_CHROMAFFACT   40
#define VPU_NR_CONFIG_TABLE_ROW_RNRSKTNFACT   43
#define VPU_NR_CONFIG_TABLE_ROW_SKTNCRTHRLO   44
#define VPU_NR_CONFIG_TABLE_ROW_SKTNCRTHRHI   45
#define VPU_NR_CONFIG_TABLE_ROW_SKTNSIGTHRLO  46
#define VPU_NR_CONFIG_TABLE_ROW_SKTNSIGTHRHI  47

#define VPU_NR_NUM_TUNING_REGIONS     10

#define VPU_TNR_CONTROL_LUX_BASED   0
#define VPU_TNR_CONTROL_GAIN_BASED  1

typedef struct _nr_config_table_t {
  /* length includes, scope and table id */
  int32_t data[NR_CONFIG_TABLE_NUM_COLS];
  int32_t table_id;
  int32_t scope;
} nr_config_table_t;

typedef struct _nr_level_table_t {
  int32_t data[NR_LEVEL_TABLE_DATA_LENGTH];
  int32_t scope;
} nr_level_table_t;

typedef struct _vpu_tuning_ctrl_t {
  nr_level_table_t                nr_level_table;
  nr_config_table_t               nr_config_table[NR_CONFIG_TABLE_NUM_ROWS];
  trigger_point_type              tnr_triggers[VPU_NR_NUM_TUNING_REGIONS];
  bool                            tuning_initialized;
  chromatix_VDP_temporal_denoise_type *chromatix_tnr;
  uint32_t                        cmd_packets_remaining;
} vpu_tuning_ctrl_t;

#endif /* VPU_CLIENT_TUNING_H */
