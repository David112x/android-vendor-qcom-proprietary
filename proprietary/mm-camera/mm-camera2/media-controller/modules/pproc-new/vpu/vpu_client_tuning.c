/*============================================================================

 Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 Qualcomm Technologies Proprietary and Confidential.

 ============================================================================*/

#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <media/msm_vpu.h>
#include <string.h>
#include <stdlib.h>

#include "chromatix.h"
#include "chromatix_common.h"
#include "vpu_client_tuning.h"
#include "chromatix_cpp.h"
#include "vpu_client_private.h"
#include "vpu_log.h"
#include "vpu_client.h"

/** VPU_USE_DEFAULT_TUNING
 * @0 use chromatix based tuning and aec control
 * @1 use vpu internal default tuning and no aec control
 */
#define VPU_USE_DEFAULT_TUNING  0

static const int32_t nr_level_table_header_data[] = {
  348,  /* total content size */
  VPU_EXT_CTRL_SUB_ID, /* sub id */
  340,  /* command group size */
  1,    /* number of commands */
  332,  /* command packet size */
  0,    /* private */
  VPU_EXT_CMD_ID_NR_LEVEL_TABLE, /* Command ID*/
  0,    /* private */
  0,    /* private */
};
#define NR_LEVEL_TABLE_HEADER_DATA_LEN \
  (sizeof(nr_level_table_header_data)/sizeof(int32_t))

static const int32_t nr_config_table_header_data[] = {
  108, /* total content size */
  VPU_EXT_CTRL_SUB_ID, /* sub id */
  100, /* command group size */
  1,   /* number of commands */
  92,  /* command packet size */
  0,    /* private */
  VPU_EXT_CMD_ID_NR_CONFIG_TABLE, /* Command ID*/
  0,    /* private */
  0,    /* private */
};
#define NR_CONFIG_TABLE_HEADER_DATA_LEN \
  (sizeof(nr_config_table_header_data)/sizeof(int32_t))

int orig_rnr_index[] = {6, 8, 10, 12, 14, 15};
int hard_rnr_index[] = {0, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

/* cehck if a row is valid for NR config table */
static inline bool is_config_table_row_valid(int row)
{
  int i;
  int valid[15] = {2, 3, 7, 11, 15, 16, 32, 38, 39, 40, 43, 44, 45, 46, 47};
  for (i=0; i<15; i++) {
    if (row == valid[i]) {
      return true;
    }
  }
  return false;
}

static inline int32_t Round(float x)
{
  return (int32_t) ((x > 0) ? (x+0.5f) : (x-0.5f));
}

static inline void nr_level_table_set(vpu_tuning_ctrl_t *t_ctrl,
 int32_t row, int32_t col, int32_t val)
{
  t_ctrl->nr_level_table.data[row*NR_LEVEL_TABLE_NUM_COLS + col] = val;
}

inline int32_t nr_level_table_get(vpu_tuning_ctrl_t *t_ctrl,
 int row, int col)
{
  return t_ctrl->nr_level_table.data[row*NR_LEVEL_TABLE_NUM_COLS + col];
}

static inline void nr_config_table_set(vpu_tuning_ctrl_t *t_ctrl,
 int32_t row, int32_t col, int32_t val)
{
  t_ctrl->nr_config_table[row].data[col] = val;
}

inline int32_t nr_config_table_get(vpu_tuning_ctrl_t *t_ctrl,
 int row, int col)
{
  return t_ctrl->nr_config_table[row].data[col];
}

/*
 * populate the tuning regions for TNR in following manner
 *
 * R1 | I12 | R2 | I23 | R3 | I34 | R4 | I45 | R5 | R6
 * Where R: Chromatix Region, I: Intermediate Region
 * Total regions after interpolation will be 10
 *
 */
int32_t vpu_tuning_gen_nr_level_table(vpu_tuning_ctrl_t *t_ctrl)
{
  int32_t row, i;
  chromatix_VDP_temporal_denoise_type *chromatix_tnr = t_ctrl->chromatix_tnr;

  memset(&t_ctrl->nr_level_table, 0x00, sizeof(nr_level_table_t));
  /* row 0 corresponds to OFF, so all values will remain 0.
   * row 1 onwards are the interpolated tuning regions */
  row = 1;
  /* populate the tuning and intermediate interpolated regions */
  for (i = 0; i < 6; i++) {
    /* provided region */
    nr_level_table_set(t_ctrl, row, VPU_NR_LEVEL_TABLE_COL_RNR,
      chromatix_tnr->temporal_denoise[i].nRNR_index);
    nr_level_table_set(t_ctrl, row, VPU_NR_LEVEL_TABLE_COL_SRNR,
      chromatix_tnr->temporal_denoise[i].nSRNR_strength);
    nr_level_table_set(t_ctrl, row, VPU_NR_LEVEL_TABLE_COL_TNR,
      chromatix_tnr->temporal_denoise[i].nTNR_strength);

    /* interpolated region (only 4 intermediate regions needed) */
    if (i < 4) {
      nr_level_table_set(t_ctrl, row + 1, VPU_NR_LEVEL_TABLE_COL_RNR,
        (chromatix_tnr->temporal_denoise[i].nRNR_index
          + chromatix_tnr->temporal_denoise[i+1].nRNR_index) / 2);
      nr_level_table_set(t_ctrl, row + 1, VPU_NR_LEVEL_TABLE_COL_SRNR,
        Round((chromatix_tnr->temporal_denoise[i+1].nSRNR_strength
              + chromatix_tnr->temporal_denoise[i].nSRNR_strength) / 2.0f));
      nr_level_table_set(t_ctrl, row + 1, VPU_NR_LEVEL_TABLE_COL_TNR,
        Round((chromatix_tnr->temporal_denoise[i].nTNR_strength
              + chromatix_tnr->temporal_denoise[i+1].nTNR_strength) / 2.0f));
      row++;
    }
    row++;
  }

  /* hardcode RNR indexes */
  for (i=0; i<11; i++) {
    nr_level_table_set(t_ctrl, i, VPU_NR_LEVEL_TABLE_COL_RNR, hard_rnr_index[i]);
  }
  t_ctrl->nr_level_table.scope = 2; /* global */
  return 0;
}

int32_t vpu_tuning_gen_nr_config_table(vpu_tuning_ctrl_t *t_ctrl)
{
  int32_t col, i;
  chromatix_VDP_temporal_denoise_type *chromatix_tnr = t_ctrl->chromatix_tnr;

  memset(t_ctrl->nr_config_table, 0x00,
          sizeof(nr_config_table_t) * NR_CONFIG_TABLE_NUM_ROWS);
  /* populate parameters for provided and intermediate interpolated regions */
  for (i = 0; i < 6; i++) {
    //col = chromatix_tnr->temporal_denoise[i].nRNR_index;
    col = orig_rnr_index[i];
    /* provided region */
    nr_config_table_set(t_ctrl, VPU_NR_CONFIG_TABLE_ROW_KMINY, col, chromatix_tnr
        ->temporal_denoise[i].kminY);
    nr_config_table_set(t_ctrl, VPU_NR_CONFIG_TABLE_ROW_KMINC, col, chromatix_tnr
        ->temporal_denoise[i].kminC);
    nr_config_table_set(t_ctrl, VPU_NR_CONFIG_TABLE_ROW_RNRFFACT, col, chromatix_tnr
        ->temporal_denoise[i].rnrFfact);
    nr_config_table_set(t_ctrl, VPU_NR_CONFIG_TABLE_ROW_TXTTHR4, col, chromatix_tnr
        ->temporal_denoise[i].txtThr4);
    nr_config_table_set(t_ctrl, VPU_NR_CONFIG_TABLE_ROW_TXTTHR3, col, chromatix_tnr
        ->temporal_denoise[i].txtThr3);
    nr_config_table_set(t_ctrl, VPU_NR_CONFIG_TABLE_ROW_RNRLEVEL, col, chromatix_tnr
        ->temporal_denoise[i].rnr_level);
    nr_config_table_set(t_ctrl, VPU_NR_CONFIG_TABLE_ROW_CALCUPLIMIT, col, chromatix_tnr
        ->temporal_denoise[i].calcUpperLimit);
    nr_config_table_set(t_ctrl, VPU_NR_CONFIG_TABLE_ROW_RNRMINNOISE, col, chromatix_tnr
        ->temporal_denoise[i].rnrMinNoise);
    nr_config_table_set(t_ctrl, VPU_NR_CONFIG_TABLE_ROW_RNRMAXNOISE, col, chromatix_tnr
        ->temporal_denoise[i].rnrMaxNoise);
    nr_config_table_set(t_ctrl, VPU_NR_CONFIG_TABLE_ROW_CHROMAFFACT, col, chromatix_tnr
        ->temporal_denoise[i].rnrChromaFfact);
    nr_config_table_set(t_ctrl, VPU_NR_CONFIG_TABLE_ROW_RNRSKTNFACT, col, chromatix_tnr
        ->temporal_denoise[i].rnrSkinFfact);
    nr_config_table_set(t_ctrl, VPU_NR_CONFIG_TABLE_ROW_SKTNCRTHRLO, col, chromatix_tnr
        ->temporal_denoise[i].skinR.sknTnCrThrLo);
    nr_config_table_set(t_ctrl, VPU_NR_CONFIG_TABLE_ROW_SKTNCRTHRHI, col, chromatix_tnr
        ->temporal_denoise[i].skinR.sknTnCrThrHi);
    nr_config_table_set(t_ctrl, VPU_NR_CONFIG_TABLE_ROW_SKTNSIGTHRLO, col, chromatix_tnr
        ->temporal_denoise[i].skinR.sknTnSigThrLo);
    nr_config_table_set(t_ctrl, VPU_NR_CONFIG_TABLE_ROW_SKTNSIGTHRHI, col, chromatix_tnr
        ->temporal_denoise[i].skinR.sknTnSigThrHi);

    /* interpolated region (only 4 intermediate regions needed) */
    if (i < 4) {
      col++;
      nr_config_table_set(t_ctrl, VPU_NR_CONFIG_TABLE_ROW_KMINY, col, Round(
          (chromatix_tnr->temporal_denoise[i].kminY
              + chromatix_tnr->temporal_denoise[i + 1].kminY) / 2.0f));
      nr_config_table_set(t_ctrl, VPU_NR_CONFIG_TABLE_ROW_KMINC, col, Round(
          (chromatix_tnr->temporal_denoise[i].kminC
              + chromatix_tnr->temporal_denoise[i + 1].kminC) / 2.0f));
      nr_config_table_set(t_ctrl, VPU_NR_CONFIG_TABLE_ROW_RNRFFACT, col, Round(
          (chromatix_tnr->temporal_denoise[i].rnrFfact
              + chromatix_tnr->temporal_denoise[i + 1].rnrFfact) / 2.0f));
      nr_config_table_set(t_ctrl, VPU_NR_CONFIG_TABLE_ROW_TXTTHR4, col, Round(
          (chromatix_tnr->temporal_denoise[i].txtThr4
              + chromatix_tnr->temporal_denoise[i + 1].txtThr4) / 2.0f));
      nr_config_table_set(t_ctrl, VPU_NR_CONFIG_TABLE_ROW_TXTTHR3, col, Round(
          (chromatix_tnr->temporal_denoise[i].txtThr3
              + chromatix_tnr->temporal_denoise[i + 1].txtThr3) / 2.0f));
      nr_config_table_set(t_ctrl, VPU_NR_CONFIG_TABLE_ROW_RNRLEVEL, col, Round(
          (chromatix_tnr->temporal_denoise[i].rnr_level
              + chromatix_tnr->temporal_denoise[i + 1].rnr_level) / 2.0f));
      nr_config_table_set(t_ctrl, VPU_NR_CONFIG_TABLE_ROW_CALCUPLIMIT, col, Round(
          (chromatix_tnr->temporal_denoise[i].calcUpperLimit
              + chromatix_tnr->temporal_denoise[i + 1].calcUpperLimit) / 2.0f));
      nr_config_table_set(t_ctrl, VPU_NR_CONFIG_TABLE_ROW_RNRMINNOISE, col, Round(
          (chromatix_tnr->temporal_denoise[i].rnrMinNoise
              + chromatix_tnr->temporal_denoise[i + 1].rnrMinNoise) / 2.0f));
      nr_config_table_set(t_ctrl, VPU_NR_CONFIG_TABLE_ROW_RNRMAXNOISE, col, Round(
          (chromatix_tnr->temporal_denoise[i].rnrMaxNoise
              + chromatix_tnr->temporal_denoise[i + 1].rnrMaxNoise) / 2.0f));
      nr_config_table_set(t_ctrl, VPU_NR_CONFIG_TABLE_ROW_CHROMAFFACT, col, Round(
          (chromatix_tnr->temporal_denoise[i].rnrChromaFfact
              + chromatix_tnr->temporal_denoise[i + 1].rnrChromaFfact) / 2.0f));
      nr_config_table_set(t_ctrl, VPU_NR_CONFIG_TABLE_ROW_RNRSKTNFACT, col, Round(
          (chromatix_tnr->temporal_denoise[i].rnrSkinFfact
              + chromatix_tnr->temporal_denoise[i + 1].rnrSkinFfact) / 2.0f));
      nr_config_table_set(t_ctrl, VPU_NR_CONFIG_TABLE_ROW_SKTNCRTHRLO, col, Round(
          (chromatix_tnr->temporal_denoise[i].skinR.sknTnCrThrLo
              + chromatix_tnr->temporal_denoise[i + 1].skinR.sknTnCrThrLo)
              / 2.0f));
      nr_config_table_set(t_ctrl, VPU_NR_CONFIG_TABLE_ROW_SKTNCRTHRHI, col, Round(
          (chromatix_tnr->temporal_denoise[i].skinR.sknTnCrThrHi
              + chromatix_tnr->temporal_denoise[i + 1].skinR.sknTnCrThrHi)
              / 2.0f));
      nr_config_table_set(t_ctrl, VPU_NR_CONFIG_TABLE_ROW_SKTNSIGTHRLO, col, Round(
          (chromatix_tnr->temporal_denoise[i].skinR.sknTnSigThrLo
              + chromatix_tnr->temporal_denoise[i + 1].skinR.sknTnSigThrLo)
              / 2.0f));
      nr_config_table_set(t_ctrl, VPU_NR_CONFIG_TABLE_ROW_SKTNSIGTHRHI, col, Round(
          (chromatix_tnr->temporal_denoise[i].skinR.sknTnSigThrHi
              + chromatix_tnr->temporal_denoise[i + 1].skinR.sknTnSigThrHi)
              / 2.0f));
    }
  }

  for (i=0; i<NR_CONFIG_TABLE_NUM_ROWS; i++) {
    t_ctrl->nr_config_table[i].table_id = i;
    t_ctrl->nr_config_table[i].scope = 2;
  }
  return 0;
}

int32_t vpu_tuning_interpolate_triggers(vpu_tuning_ctrl_t *t_ctrl)
{
  int32_t i;
  int32_t idx = 0;
  chromatix_VDP_temporal_denoise_type *chromatix_tnr = t_ctrl->chromatix_tnr;

  for (i = 0; i < 6; i++) {
    /* provided region */
    t_ctrl->tnr_triggers[idx].gain_start = chromatix_tnr->temporal_denoise[i]
       .temporal_denoise_trigger.gain_start;
    t_ctrl->tnr_triggers[idx].lux_index_start =
       chromatix_tnr->temporal_denoise[i].temporal_denoise_trigger
       .lux_index_start;
    idx++;
    /* interpolated region (only 4 intermediate regions needed) */
    if (i < 4) {
      t_ctrl->tnr_triggers[idx].gain_start =
        (chromatix_tnr->temporal_denoise[i].temporal_denoise_trigger.gain_start
          + chromatix_tnr->temporal_denoise[i + 1].temporal_denoise_trigger
              .gain_start) / 2.0f;
      t_ctrl->tnr_triggers[idx].lux_index_start = Round(
          (chromatix_tnr->temporal_denoise[i].temporal_denoise_trigger
              .lux_index_start
              + chromatix_tnr->temporal_denoise[i+1].temporal_denoise_trigger
                  .lux_index_start) / 2.0f);
      idx++;
    }

  }
  return 0;
}

static int32_t vpu_client_program_nr_level_table(vpu_client_t client)
{
  int32_t rc;
  struct vpu_control_extended ctrl_ex;
  int32_t cmd_content[VPU_MAX_EXT_DATA_SIZE/sizeof(int32_t)];
  int32_t offset=0;
  uint32_t total_content_size =
    sizeof(nr_level_table_header_data) + sizeof(int32_t) +
     sizeof(client->t_ctrl.nr_level_table.data);

  VPU_DBG("content_size=%d = %d + %d + %d", total_content_size,
          sizeof(nr_level_table_header_data), sizeof(int32_t),
          sizeof(client->t_ctrl.nr_level_table.data));

  if (total_content_size > sizeof(cmd_content)) {
    VPU_ERR("content_size=%d, max_allowed=%d", total_content_size,
            sizeof(cmd_content));
    return -1;
  }
  memset(cmd_content, 0x00, VPU_MAX_EXT_DATA_SIZE);
  memcpy(cmd_content + offset, nr_level_table_header_data,
         sizeof(nr_level_table_header_data));
  offset += NR_LEVEL_TABLE_HEADER_DATA_LEN;
  memcpy(cmd_content + offset, client->t_ctrl.nr_level_table.data,
         sizeof(client->t_ctrl.nr_level_table.data));
  offset += sizeof(client->t_ctrl.nr_level_table.data)/sizeof(int32_t);
  *(cmd_content + offset) = client->t_ctrl.nr_level_table.scope;
  ctrl_ex.type = 1;
  ctrl_ex.data_ptr = (void*) cmd_content;
  ctrl_ex.data_len = VPU_MAX_EXT_DATA_SIZE;

  rc = ioctl(client->fd, VPU_S_CONTROL_EXTENDED, &ctrl_ex);
  if (rc < 0) {
    VPU_ERR("VPU_S_CONTROL_EXTENDED ioctl() failed");
    return -1;
  }
  VPU_DBG("nr level table programmed.");
  return 0;
}

static int32_t vpu_client_program_nr_config_table(vpu_client_t client)
{
  int32_t rc;
  struct vpu_control_extended ctrl_ex;
  int32_t cmd_content[VPU_MAX_EXT_DATA_SIZE/sizeof(int32_t)];
  int32_t i;
  int32_t offset;
  uint32_t total_content_size =
    sizeof(nr_config_table_header_data) + 2*sizeof(int32_t) +
     sizeof(client->t_ctrl.nr_config_table[0].data);

  VPU_DBG("content_size=%d = %d + %d + %d", total_content_size,
          sizeof(nr_config_table_header_data), 2*sizeof(int32_t),
          sizeof(client->t_ctrl.nr_config_table[0].data));

  if (total_content_size > sizeof(cmd_content)) {
    VPU_ERR("content_size=%d, max_allowed=%d", total_content_size,
            sizeof(cmd_content));
    return -1;
  }
  for (i = 0; i < NR_CONFIG_TABLE_NUM_ROWS; i++) {
    if (is_config_table_row_valid(i) == false) {
      continue;
    }
    offset = 0;
    memset(cmd_content, 0x00, VPU_MAX_EXT_DATA_SIZE);
    memcpy(cmd_content + offset, nr_config_table_header_data,
           sizeof(nr_config_table_header_data));
    offset += NR_CONFIG_TABLE_HEADER_DATA_LEN;
    *(cmd_content + offset) = client->t_ctrl.nr_config_table[i].table_id;
    offset ++;
    memcpy(cmd_content + offset, client->t_ctrl.nr_config_table[i].data,
           NR_CONFIG_TABLE_NUM_COLS * sizeof(int32_t));
    offset += NR_CONFIG_TABLE_NUM_COLS;
    *(cmd_content + offset) = client->t_ctrl.nr_config_table[i].scope;
    ctrl_ex.type = 1;
    ctrl_ex.data_ptr = (void*) cmd_content;
    ctrl_ex.data_len = total_content_size;//VPU_MAX_EXT_DATA_SIZE;

    rc = ioctl(client->fd, VPU_S_CONTROL_EXTENDED, &ctrl_ex);
    if (rc < 0) {
      VPU_ERR("VPU_S_CONTROL_EXTENDED ioctl() failed, i=%d, rc=%d", i, rc);
      uint32_t j;
      for (j=0; j < total_content_size/sizeof(int32_t); j++) {
        VPU_DBG("cmd_content[%03d] = %d", j, cmd_content[j]);
      }
      return -1;
    }
    usleep(10000);
  }
  VPU_DBG("nr config table programmed.");
  return 0;
}

/* API functions */

int32_t vpu_client_update_chromatix(vpu_client_t client,
    void* chromatix_ptr)
{
  int32_t rc;

  if (!chromatix_ptr) {
    VPU_ERR("failed, chromatix_ptr = NULL");
    return -1;
  }
  client->chromatix_ptr = chromatix_ptr;
  chromatix_cpp_type *chromatix_cpp = (chromatix_cpp_type *) chromatix_ptr;
  chromatix_VDP_temporal_denoise_type *chromatix_tnr =
     &(chromatix_cpp->chromatix_VDP_temporal_denoise);

  client->t_ctrl.chromatix_tnr = chromatix_tnr;
  if (VPU_USE_DEFAULT_TUNING) {
    VPU_HIGH("default vpu tuning parameters are used");
    client->t_ctrl.tuning_initialized = true;
    return 0;
  }
  vpu_tuning_gen_nr_level_table(&client->t_ctrl);
  vpu_tuning_gen_nr_config_table(&client->t_ctrl);
  vpu_tuning_interpolate_triggers(&client->t_ctrl);

  vpu_client_program_nr_level_table(client);
  vpu_client_program_nr_config_table(client);
  client->t_ctrl.tuning_initialized = true;

  return 0;
}

int32_t vpu_tuning_get_index_for_trigger(vpu_tuning_ctrl_t *t_ctrl,
  float gain, float lux_idx, int32_t ctrl_type)
{
  int32_t i, idx = -1;
  float min_diff = 1E10, cur_diff=0.0f;
  for (i=9; i >= 0; i--) {
    if (ctrl_type == VPU_TNR_CONTROL_GAIN_BASED) {
      cur_diff = fabs(gain - t_ctrl->tnr_triggers[i].gain_start);
    } else {
      cur_diff = fabs(lux_idx -
         (float) t_ctrl->tnr_triggers[i].lux_index_start);
    }
    if (cur_diff < min_diff) {
      min_diff = cur_diff;
      idx = i;
    }
  }
  /* index 0 corresponds to OFF, so valid index starts from 1 */
  return (idx+1);
}

int32_t vpu_client_aec_update(vpu_client_t client,
  float gain, float lux_idx)
{
  int32_t rc;
  int32_t idx = 0, i;
  float min = 0.0f, cur_diff;
  uint32_t nr_level;
  vpu_client_ctrl_parm_t ctrl_parm;
  ctrl_parm = client->cur_ctrl_parm;

  if (ctrl_parm.tnr_enable == false || VPU_USE_DEFAULT_TUNING) {
    return 0;
  }
  if (client->t_ctrl.tuning_initialized == false) {
    VPU_DBG("tuning parameters not initialized");
    return -1;
  }

  idx = vpu_tuning_get_index_for_trigger(&client->t_ctrl, gain, lux_idx,
                                         client->t_ctrl.chromatix_tnr->
                                         control_temporal_denoise);
  nr_level = idx * 10;

  if (ctrl_parm.tnr_value == nr_level) {
    return 0;
  }
  ctrl_parm.tnr_value = nr_level;

  rc = vpu_client_set_ctrl_parm(client, ctrl_parm);
  if (rc < 0) {
    VPU_ERR("failed");
    return -1;
  }
  VPU_DBG("tnr level updated to %d", nr_level);
  return 0;
}
