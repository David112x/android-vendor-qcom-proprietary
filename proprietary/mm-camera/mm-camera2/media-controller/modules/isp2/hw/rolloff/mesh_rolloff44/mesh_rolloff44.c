/* Copyright (c) 2012-2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

/* std headers */
#include <unistd.h>
#include <math.h>

/* mctl headers */

/* isp headers */
#include "isp_sub_module_log.h"
#include "isp_defs.h"
#include "isp_sub_module_util.h"
#include "isp_pipeline_reg.h"
#include "isp_log.h"
#include "mesh_rolloff44.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_ROLLOFF, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_ROLLOFF, fmt, ##args)

#ifdef _ANDROID_
#include <cutils/properties.h>
#endif

/* #define MESH_ROLLOFF_DEBUG */
#ifdef MESH_ROLLOFF_DEBUG
#undef ISP_DBG
#define ISP_DBG ISP_ERR
#undef ISP_HIGH
#define ISP_HIGH ISP_ERR
#undef ISP_DBG_NO_FUNC
#define ISP_DBG_NO_FUNC ALOGE
#endif

/* print hw table */
/* #define PRINT_HW_TABLE */

static boolean mesh_rolloff44_calc_grid_config(int raw_width, int raw_height,
  int *scale_cubic, int *deltah, int *deltav, int *subgridh, int *subgridv,
  int *nh, int *nv);

void mesh_rolloff44_bicubic (float fs, float* fc0, float* fc1, float* fc2,
  float* fc3)
{
  /* calculate interpolation */
  float fs2 = fs*fs;
  float fs3 = fs*fs2;
  *fc0 = 0.5f*(-fs3+2.0f*fs2-fs);
  *fc1 = 0.5f*(3.0f*fs3-5.0f*fs2+2.0f);
  *fc2 = 0.5f*(-3.0f*fs3+4.0f*fs2+fs);
  *fc3 = 0.5f*(fs3-fs2);
}

/**  mesh_extend_1block :

 * MeshIn: the input ideal Rolloff table (31x 41)at the full sensor
 * MeshOut: the output rolloff table at the current output resolution
 *
  **/
void mesh_rolloff44_extend_1block(float *MeshIn,  float *MeshOut, int Nx,
  int Ny)
{
  ISP_DBG(" Entered %s \n", __func__);

  int i, j;

  for (i=1; i<Ny+1; i++)
    for (j=1; j<Nx+1; j++)
      MeshOut[i*(Nx+2)+j] = MeshIn[(i-1)*Nx+j-1];

  MeshOut[0*(Nx+2)+0] = MeshOut[1*(Nx+2)+1]*2 - MeshOut[2*(Nx+2)+2];
  MeshOut[(Ny+1)*(Nx+2)+0] = MeshOut[(Ny)*(Nx+2)+1]*2 -
    MeshOut[(Ny-1)*(Nx+2)+2];
  MeshOut[(Ny+1)*(Nx+2)+Nx+1] = MeshOut[(Ny)*(Nx+2)+Nx]*2 -
    MeshOut[(Ny-1)*(Nx+2)+Nx-1];
  MeshOut[0*(Nx+2)+Nx+1] = MeshOut[1*(Nx+2)+Nx]*2 - MeshOut[2*(Nx+2)+Nx-1];


  for (i=1; i<Ny+1; i++)
  {
    MeshOut[i*(Nx+2)+0] = MeshOut[i*(Nx+2)+1]*2 - MeshOut[i*(Nx+2)+2];
    MeshOut[i*(Nx+2)+Nx+1] = MeshOut[i*(Nx+2)+Nx]*2 - MeshOut[i*(Nx+2)+Nx-1];
  }

  for (j=1; j<Nx+1; j++)
  {
    MeshOut[0*(Nx+2)+j] = MeshOut[1*(Nx+2)+j]*2 - MeshOut[2*(Nx+2)+j];
    MeshOut[(Ny+1)*(Nx+2)+j] = MeshOut[(Ny)*(Nx+2)+j]*2 -
      MeshOut[(Ny-1)*(Nx+2)+j];
  }
}

/** mesh_rolloff_V4_ScaleRolloffMesh_by_sensor_info:
 *    @meshOut: output new meshtbl according to camif info
 *
 *  @MeshIn: the input Rolloff table (10x13) at the full sensor
 *  @MeshOut: the output rolloff table (10x13) at the current
 *           output resolution
 *  @full_width: the full-resolution width
 *  @full_height: the full-resolution height
 *  @output_width: the output width
 *  @output_height: the output height
 *  @offset_x: x-index of the top-left corner of output image on
 *            the full-resolution sensor
 *  @offset_y: y-index of the top-left corner of output image on
 *            the full-resolution sensor
 *  @scale_factor: the sensor scaling factor
 *  (= binning_factor digal_sampling_factor)
 *
 *  This function is used to resample the Rolloff table from
 *  the full-resolution sensor to the (CAMIF) output size, which
 *  is decided by the sensor mode. This function is the same as
 *  mesh_rolloff_V4_ScaleMesh_latest, just add more programmable
 *  input.
 *
 *  Return void
 **/
static void mesh_rolloff44_scale_rolloff_table(float *MeshIn,  float *MeshOut,
  int full_width, int full_height, int output_width, int output_height,
  int offset_x, int offset_y, int scale_factor)
{
  float cxm, cx0, cx1, cx2, cym, cy0, cy1, cy2;
  float am, a0, a1, a2, bm, b0, b1, b2;
  float tx , ty;
  int ix, iy;
  int i, j;
  int MESH_H, MESH_V;
  int index1, index2;

  /* Initialize the roll-off mesh grid */
  int scale, w, h, sgh, sgv, gh, gv, dh, dv;
  int gh_up, gv_up, dh_up, dv_up;
  float gh_full, gv_full;

  int MESH_H_MAX;
  int MESH_V_MAX;
  float Extend_Mesh[(HW_MESH_ROLL_NUM_ROW + 4) * (HW_MESH_ROLL_NUM_COL + 4)];

  mesh_rolloff44_calc_grid_config(output_width, output_height, &scale, &dh, &dv,
    &sgh, &sgv, &MESH_H, &MESH_V);
  gh = sgh * scale;
  gv = sgv * scale;

  /*  Upsampling the output roll-off mesh grid by scale factor */
  gh_up = gh * scale_factor;
  gv_up = gv * scale_factor;
  dh_up = dh * scale_factor;
  dv_up = dv * scale_factor;

  ISP_DBG(" gh_up =%d, gv_up =%d, dh_up=%d, dv_up=%d", gh_up, gv_up, dh_up,
    dv_up);

  /* Calculate the rolloff grid at the full resolution */
  MESH_H_MAX = HW_MESH_ROLL_NUM_COL - 1; /* 16 */
  MESH_V_MAX = HW_MESH_ROLL_NUM_ROW - 1; /* 12 */

  w = full_width >> 1;  /* per-channel image width */
  h = full_height >> 1;  /* per-channel image height */

  gh_full = (w - 1) / (float) MESH_H_MAX;
  gv_full = (h - 1) / (float) MESH_V_MAX;


  mesh_rolloff44_extend_1block(MeshIn,  &Extend_Mesh[0], MESH_H_MAX + 1,
    MESH_V_MAX + 1);

  /*  resample Extended Mesh data onto the roll-off mesh grid */
  for (i = 0; i < (MESH_V + 1); i++) {
    for (j = 0; j < (MESH_H + 1); j++) {
      tx =  ((float)((j * gh_up) - dh_up + (offset_x / 2) + (gh_full))) /
        (gh_full);
      ix = (int)floor(tx);
      tx -= (float)ix;

      ty =  ((float)((i * gv_up) - dv_up + (offset_y / 2) + (gv_full))) /
        (gv_full);
      iy = (int)floor(ty);
      ty -= (float)iy;

      if (i == 0 || j == 0  || i == MESH_V|| j == MESH_H) {
        /* for boundary points, use bilinear interpolation */
        index1 = iy    *(MESH_H_MAX+3) + ix;
        index2 = (iy+1)*(MESH_H_MAX+3) + ix;
        b1 = (1 - tx)* Extend_Mesh[index1] + tx* Extend_Mesh[index1+1];
        b2 = (1 - tx)* Extend_Mesh[index2] + tx* Extend_Mesh[index2+1];

        MeshOut[(i * (MESH_H + 1)) + j] = (float)((1 - ty) * b1 + ty * b2);
        MeshOut[(i * (MESH_H + 1)) + j]  = MIN(MAX(MeshOut[(i * (MESH_H + 1)) + j], 1.0), 7.9);
      } else {
        /* for nonboundary points, use bicubic interpolation */
        /* get x direction coeff and y direction coeff */

        mesh_rolloff44_bicubic(tx,  &cxm, &cx0, &cx1, &cx2);
        mesh_rolloff44_bicubic(ty,  &cym, &cy0, &cy1, &cy2);

        am = Extend_Mesh[(iy - 1) * (MESH_H_MAX + 3) +  (ix - 1)];
        a0 = Extend_Mesh[(iy - 1) * (MESH_H_MAX + 3) +  (ix)];
        a1 = Extend_Mesh[(iy - 1) * (MESH_H_MAX + 3) +  (ix + 1)];
        a2 = Extend_Mesh[(iy - 1) * (MESH_H_MAX + 3) +  (ix + 2)];
        bm = ((cxm * am)  +  (cx0 * a0)  +  (cx1 * a1)  +  (cx2 * a2));

        am = Extend_Mesh[(iy) * (MESH_H_MAX + 3) +  (ix - 1)];
        a0 = Extend_Mesh[(iy) * (MESH_H_MAX + 3) +  (ix)];
        a1 = Extend_Mesh[(iy) * (MESH_H_MAX + 3) +  (ix + 1)];
        a2 = Extend_Mesh[(iy) * (MESH_H_MAX + 3) +  (ix + 2)];
        b0 = ((cxm * am)  +  (cx0 * a0)  +  (cx1 * a1)  +  (cx2 * a2));

        am = Extend_Mesh[(iy + 1) * (MESH_H_MAX + 3) +  (ix - 1)];
        a0 = Extend_Mesh[(iy + 1) * (MESH_H_MAX + 3) +  (ix)];
        a1 = Extend_Mesh[(iy + 1) * (MESH_H_MAX + 3) +  (ix + 1)];
        a2 = Extend_Mesh[(iy + 1) * (MESH_H_MAX + 3) +  (ix + 2)];
        b1 = ((cxm * am) + (cx0 * a0) + (cx1 * a1) + (cx2 * a2));

        am = Extend_Mesh[(iy + 2) * (MESH_H_MAX + 3) +  (ix - 1)];
        a0 = Extend_Mesh[(iy + 2) * (MESH_H_MAX + 3) +  (ix)];
        a1 = Extend_Mesh[(iy + 2) * (MESH_H_MAX + 3) +  (ix + 1)];
        a2 = Extend_Mesh[(iy + 2) * (MESH_H_MAX + 3) +  (ix + 2)];
        b2 = ((cxm * am) + (cx0 * a0) + (cx1 * a1) + (cx2 * a2));

        MeshOut[(i * (MESH_H + 1)) + j] = (float)((cym * bm) + (cy0 * b0) +
          (cy1 * b1) + (cy2 * b2));
        MeshOut[(i * (MESH_H + 1)) + j]  = MIN(MAX(MeshOut[(i * (MESH_H + 1)) + j], 1.0), 7.9);
      }
    }
  }
}

/** mesh_rolloff44_mesh_table_debug:
 *
 *  @meshtbl: Input 13x10 table to be printed
 *
 *  Print the 13x10 Rolloff table
 *
 *  Return void
 **/
static void mesh_rolloff44_mesh_table_debug(mesh_rolloff_array_type *meshtbl)
{
  int  i, j;
  char line_str[128], *p_str;

  if (!meshtbl) {
    ISP_ERR("failed: %p", meshtbl);
    return;
  }

  line_str[0] = '\0';
  p_str = line_str;

  ISP_DBG("Rolloff44 Chromatix Tbl R, H_grid %d, V_grid %d",
    CHROMATIX_MESH_ROLL_NUM_COL, CHROMATIX_MESH_ROLL_NUM_ROW);
  for (i = 0; i < CHROMATIX_MESH_ROLL_NUM_ROW; i++) {
    for(j = 0; j < CHROMATIX_MESH_ROLL_NUM_COL; j++) {
      snprintf(p_str, 8, "%1.2f ",
        meshtbl->r_gain[(i*CHROMATIX_MESH_ROLL_NUM_COL)+j]);
      p_str += 5;
    }
    ISP_DBG_NO_FUNC("%s\n", line_str);
    line_str[0] = '\0';
    p_str = line_str;
  }

  ISP_DBG("Rolloff44 Chromatix Tbl GR, H_grid %d, V_grid %d",
    CHROMATIX_MESH_ROLL_NUM_COL, CHROMATIX_MESH_ROLL_NUM_ROW);
  for (i = 0; i < CHROMATIX_MESH_ROLL_NUM_ROW; i++) {
    for(j = 0; j < CHROMATIX_MESH_ROLL_NUM_COL; j++) {
      snprintf(p_str, 8, "%1.2f ",
        meshtbl->gr_gain[(i*CHROMATIX_MESH_ROLL_NUM_COL)+j]);
      p_str += 5;
    }
    ISP_DBG_NO_FUNC("%s\n", line_str);
    line_str[0] = '\0';
    p_str = line_str;
  }

  ISP_DBG("Rolloff44 ChromatixTbl GB, H_grid %d, V_grid %d",
    CHROMATIX_MESH_ROLL_NUM_COL, CHROMATIX_MESH_ROLL_NUM_ROW);
  for (i = 0; i < CHROMATIX_MESH_ROLL_NUM_ROW; i++) {
    for(j = 0; j < CHROMATIX_MESH_ROLL_NUM_COL; j++) {
      snprintf(p_str, 8, "%1.2f ",
        meshtbl->gb_gain[(i*CHROMATIX_MESH_ROLL_NUM_COL)+j]);
      p_str += 5;
    }
    ISP_DBG_NO_FUNC("%s\n", line_str);
    line_str[0] = '\0';
    p_str = line_str;
  }

  ISP_DBG("Rolloff44 Chromatix Tbl B, H_grid %d, V_grid %d",
    CHROMATIX_MESH_ROLL_NUM_COL, CHROMATIX_MESH_ROLL_NUM_ROW);
  for (i = 0; i < CHROMATIX_MESH_ROLL_NUM_ROW; i++) {
    for(j = 0; j < CHROMATIX_MESH_ROLL_NUM_COL; j++) {
      snprintf(p_str, 8, "%1.2f ",
        meshtbl->b_gain[(i*CHROMATIX_MESH_ROLL_NUM_COL)+j]);
      p_str += 5;
    }
    ISP_DBG_NO_FUNC("%s\n", line_str);
    line_str[0] = '\0';
    p_str = line_str;
  }
  if (line_str[0] != '\0')
    ISP_DBG_NO_FUNC("%s\n", line_str);
}

/** mesh_rolloff44_mesh_downscaled_table_debug:
 *
 *  @meshtbl: Input 13x10 table to be printed
 *
 *  Print the 13x10 Rolloff table
 *
 *  Return void
 **/
static void mesh_rolloff44_mesh_downscaled_table_debug(float *meshtbl)
{
  int  i, j;
  char line_str[128], *p_str;

  if (!meshtbl) {
    ISP_ERR("failed: %p", meshtbl);
    return;
  }

  line_str[0] = '\0';
  p_str = line_str;

  ISP_DBG("Rolloff44 Tbl R, H_grid %d, V_grid %d",
    HW_MESH_ROLL_NUM_ROW, HW_MESH_ROLL_NUM_COL);
  for (i = 0; i < HW_MESH_ROLL_NUM_COL; i++) {
    for(j = 0; j < HW_MESH_ROLL_NUM_ROW; j++) {
      snprintf(p_str, 8, "%1.2f ", meshtbl[(i*HW_MESH_ROLL_NUM_ROW)+j]);
      p_str += 5;
    }
    ISP_DBG_NO_FUNC("%s\n", line_str);
    line_str[0] = '\0';
    p_str = line_str;
  }

  return;
}

/** mesh_rolloff44_mesh_sensor_calc_table_debug:
 *
 *  @meshtbl: Input 13x10 table to be printed
 *
 *  Print the 13x10 Rolloff table
 *
 *  Return void
 **/
static void mesh_rolloff44_mesh_sensor_calc_table_debug(float *meshtbl)
{
  int  i, j;
  char line_str[128], *p_str;

  if (!meshtbl) {
    ISP_ERR("failed: %p", meshtbl);
    return;
  }

  line_str[0] = '\0';
  p_str = line_str;

  ISP_DBG("Rolloff44 Tbl R, H_grid %d, V_grid %d",
    HW_MESH_ROLL_NUM_ROW, HW_MESH_ROLL_NUM_COL);
  for (i = 0; i < HW_MESH_ROLL_NUM_COL; i++) {
    for(j = 0; j < HW_MESH_ROLL_NUM_ROW; j++) {
      snprintf(p_str, 8, "%1.2f ", meshtbl[(i*HW_MESH_ROLL_NUM_ROW)+j]);
      p_str += 5;
    }
    ISP_DBG("");
    line_str[0] = '\0';
    p_str = line_str;
  }

  return;
} /* mesh_rolloff44_mesh_sensor_calc_table_debug */

/** mesh_rolloff44_sensor_calc_tbl:
 *
 *  @inTbl: input table
 *  @outTbl: output table
 *  @sensor_info: handle to sensor_rolloff_config_t
 *
 *  Get the tables from the chromatix pointer and normalize the values to ensure
 *  all values are >1
 *
 *  Return void
 **/
void mesh_rolloff44_sensor_calc_tbl(mesh_rolloff_array_type *inTbl,
  mesh_rolloff_array_type *outTbl, sensor_rolloff_config_t *sensor_info)
{
  if (!inTbl || !outTbl || !sensor_info) {
    ISP_ERR("failed: %p %p %p", inTbl, outTbl, sensor_info);
    return;
  }

  if (sensor_info->full_width == 0 || sensor_info->output_width == 0 ||
      sensor_info->scale_factor == 0) {
    ISP_DBG("sensor send wrong config!! not exec algo!"
      "full_w = %d, out_w = %d, scale_factor = %d",
      sensor_info->full_width, sensor_info->output_width,
      sensor_info->scale_factor);
    return;
  }

  if (sensor_info->full_width <
     (sensor_info->scale_factor * sensor_info->output_width +
     sensor_info->offset_x)) {
    ISP_DBG("sensor send wrong config!! not exec algo!"
      "full_w = %d, out_w = %d, offset_x = %d, scale_factor = %d",
      sensor_info->full_width,
      sensor_info->output_width, sensor_info->offset_x,
      sensor_info->scale_factor);
    return;
  }

  ISP_DBG("===== input R table======");
#ifdef ROLLOFF_TBL_DEBUG
  mesh_rolloff44_mesh_sensor_calc_table_debug(&inTbl->r_gain[0]);
#endif
  mesh_rolloff44_scale_rolloff_table(
    &inTbl->r_gain[0], &outTbl->r_gain[0],
    sensor_info->full_width, sensor_info->full_height,
    sensor_info->output_width, sensor_info->output_height,
    sensor_info->offset_x, sensor_info->offset_y, sensor_info->scale_factor);
  ISP_DBG("===== output R table======");
#ifdef ROLLOFF_TBL_DEBUG
  mesh_rolloff44_mesh_sensor_calc_table_debug(&outTbl->r_gain[0]);
#endif

  ISP_DBG("===== input B table======");
#ifdef ROLLOFF_TBL_DEBUG
  mesh_rolloff44_mesh_sensor_calc_table_debug(&inTbl->b_gain[0]);
#endif
  mesh_rolloff44_scale_rolloff_table(
    &inTbl->b_gain[0], &outTbl->b_gain[0],
    sensor_info->full_width, sensor_info->full_height,
    sensor_info->output_width, sensor_info->output_height,
    sensor_info->offset_x, sensor_info->offset_y, sensor_info->scale_factor);
  ISP_DBG("===== output B table======");
#ifdef ROLLOFF_TBL_DEBUG
  mesh_rolloff44_mesh_sensor_calc_table_debug(&outTbl->b_gain[0]);
#endif

  ISP_DBG("===== input GB table======");
#ifdef ROLLOFF_TBL_DEBUG
  mesh_rolloff44_mesh_sensor_calc_table_debug(&inTbl->gb_gain[0]);
#endif
  mesh_rolloff44_scale_rolloff_table(
    &inTbl->gb_gain[0], &outTbl->gb_gain[0],
    sensor_info->full_width, sensor_info->full_height,
    sensor_info->output_width, sensor_info->output_height,
    sensor_info->offset_x, sensor_info->offset_y, sensor_info->scale_factor);
 ISP_DBG("===== output GB table======");
#ifdef ROLLOFF_TBL_DEBUG
  mesh_rolloff44_mesh_sensor_calc_table_debug(&outTbl->gb_gain[0]);
#endif

  ISP_DBG("===== input GR table======");
#ifdef ROLLOFF_TBL_DEBUG
  mesh_rolloff44_mesh_sensor_calc_table_debug(&inTbl->gr_gain[0]);
#endif
  mesh_rolloff44_scale_rolloff_table(
    &inTbl->gr_gain[0], &outTbl->gr_gain[0],
    sensor_info->full_width, sensor_info->full_height,
    sensor_info->output_width, sensor_info->output_height,
    sensor_info->offset_x, sensor_info->offset_y, sensor_info->scale_factor);
  ISP_DBG("===== output GR table======");
#ifdef ROLLOFF_TBL_DEBUG
  mesh_rolloff44_mesh_sensor_calc_table_debug(&outTbl->gr_gain[0]);
#endif
}

/** mesh_rolloff44_prepare_tables
 *
 *  @isp_sub_module: handle to isp sub module
 *  @mesh_mod: Pointer to rolloff module struct
 *
 *  Allocate and initialize rolloff tables
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean mesh_rolloff44_prepare_tables(isp_sub_module_t *isp_sub_module,
  mesh_rolloff44_t *mesh_mod)
{
  int i = 0, j = 0, k = 0;

  if (!isp_sub_module || !mesh_mod) {
    ISP_ERR("failed: %p %p", isp_sub_module, mesh_mod);
    return FALSE;
  }

  if (mesh_mod->rolloff_tbls.rolloff_tableset[0] == NULL) {
    mesh_mod->rolloff_tbls.rolloff_tableset[0] =
      (isp_rolloff_info_t*)CAM_CALLOC(sizeof(isp_rolloff_info_t), 1 );
    mesh_mod->rolloff_calibration_table.rolloff_tableset[0] =
      (isp_rolloff_info_t*)CAM_CALLOC(sizeof(isp_rolloff_info_t), 1);
  }

  if (mesh_mod->af_rolloff_info_valid == TRUE &&
      mesh_mod->rolloff_calibration_table.rolloff_tableset[1] == NULL) {
    mesh_mod->rolloff_calibration_table.rolloff_tableset[1] =
      (isp_rolloff_info_t*)CAM_CALLOC(sizeof(isp_rolloff_info_t), 1);
    /* point to the table from sensor since its not changing */
    mesh_mod->rolloff_tbls.rolloff_tableset[1] =
      (isp_rolloff_info_t*)CAM_CALLOC(sizeof(isp_rolloff_info_t), 1);
  }

  /* Left frame tables */
  if (mesh_mod->ext_func_table &&
      mesh_mod->ext_func_table->normalize) {
      mesh_mod->ext_func_table->normalize(isp_sub_module, mesh_mod);
  } else {
    ISP_ERR("failed: %p", mesh_mod->ext_func_table);
    return FALSE;
  }


  return TRUE;
} /* mesh_rolloff44_prepare_tables */

/** mesh_rolloff44_table_debug:
 *
 *  @meshtbl: Table to be printed
 *
 *  Print the R, Gr, Gb, B values from input 13x10 table
 *
 *  Return void
 **/
static void mesh_rolloff44_table_debug(MESH_RollOffTable_V4 *meshtbl)
{
  int i = 0;

  if (!meshtbl) {
    ISP_ERR("failed: meshtbl %p", meshtbl);
    return;
  }

#ifdef ROLLOFF_TBL_DEBUG
  ISP_DBG("Mesh Rolloff table");
  for (i = 0; i < MESH_ROLL_OFF_V4_TABLE_SIZE; i++) {
    ISP_DBG("%u %u %u %u", meshtbl->TableR[i], meshtbl->TableGr[i],
      meshtbl->TableGb[i], meshtbl->TableB[i]);
  }
#endif

  return;
} /* mesh_rolloff44_table_debug */

/** mesh_rolloff44_table_interpolate:
 *
 *  @in1: input table 1
 *  @in2: input table 2
 *  @out: output interpolated table
 *  ratio: ratio to be used for interpolation
 *
 *  Interpolate table 1 and table 2 using input ratio
 *
 *  Return void
 **/
void mesh_rolloff44_table_interpolate(MESH_RollOffTable_V4 *in1,
  MESH_RollOffTable_V4 *in2, MESH_RollOffTable_V4 *out, float ratio)
{
  int i = 0;
  int mesh_rolloff_table_size = MESH_ROLL_OFF_V4_TABLE_SIZE;

  if (!in1 || !in2 || !out) {
    ISP_ERR("failed: %p %p %p", in1, in2, out);
    return;
  }

  TBL_INTERPOLATE_ROLL_OFF(in1->TableR, in2->TableR, out->TableR, ratio,
    mesh_rolloff_table_size, i);
  TBL_INTERPOLATE_ROLL_OFF(in1->TableGb, in2->TableGb, out->TableGb, ratio,
    mesh_rolloff_table_size, i);
  TBL_INTERPOLATE_ROLL_OFF(in1->TableGr, in2->TableGr, out->TableGr, ratio,
    mesh_rolloff_table_size, i);
  TBL_INTERPOLATE_ROLL_OFF(in1->TableB, in2->TableB, out->TableB, ratio,
    mesh_rolloff_table_size, i);
} /* mesh_rolloff44_table_interpolate */

static void mesh_rolloff44_print_mesh_table(MESH_RollOffTable_V4 *tbl)
{
  uint32_t i = 0;

  if (!tbl) {
    return;
  }
  /* Print table */
  ALOGE("%s:%d ***********Start*****************\n", __func__, __LINE__);

  ALOGE("%s:%d ==========R===========\n", __func__, __LINE__);
  for (i = 0; i < ARRAY_SIZE(tbl->TableR) / 17 ; i ++) {
    ALOGE("%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f\n",
      tbl->TableR[(i*17) + 0], tbl->TableR[(i*17) + 1],
      tbl->TableR[(i*17) + 2], tbl->TableR[(i*17) + 3],
      tbl->TableR[(i*17) + 4], tbl->TableR[(i*17) + 5],
      tbl->TableR[(i*17) + 6], tbl->TableR[(i*17) + 7],
      tbl->TableR[(i*17) + 8], tbl->TableR[(i*17) + 9],
      tbl->TableR[(i*17) + 10], tbl->TableR[(i*17) + 11],
      tbl->TableR[(i*17) + 12], tbl->TableR[(i*17) + 13],
      tbl->TableR[(i*17) + 14], tbl->TableR[(i*17) + 15],
      tbl->TableR[(i*17) + 16]);
  }
  ALOGE("%s:%d ==========GR===========\n", __func__, __LINE__);
  for (i = 0; i < ARRAY_SIZE(tbl->TableGr) / 17 ; i ++) {
    ALOGE("%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f\n",
      tbl->TableGr[(i*17) + 0], tbl->TableGr[(i*17) + 1],
      tbl->TableGr[(i*17) + 2], tbl->TableGr[(i*17) + 3],
      tbl->TableGr[(i*17) + 4], tbl->TableGr[(i*17) + 5],
      tbl->TableGr[(i*17) + 6], tbl->TableGr[(i*17) + 7],
      tbl->TableGr[(i*17) + 8], tbl->TableGr[(i*17) + 9],
      tbl->TableGr[(i*17) + 10], tbl->TableGr[(i*17) + 11],
      tbl->TableGr[(i*17) + 12], tbl->TableGr[(i*17) + 13],
      tbl->TableGr[(i*17) + 14], tbl->TableGr[(i*17) + 15],
      tbl->TableGr[(i*17) + 16]);
   }
  ALOGE("%s:%d ==========GB===========\n", __func__, __LINE__);
  for (i = 0; i < ARRAY_SIZE(tbl->TableGb) / 17 ; i ++) {
    ALOGE("%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f\n",
      tbl->TableGb[(i*17) + 0], tbl->TableGb[(i*17) + 1],
      tbl->TableGb[(i*17) + 2], tbl->TableGb[(i*17) + 3],
      tbl->TableGb[(i*17) + 4], tbl->TableGb[(i*17) + 5],
      tbl->TableGb[(i*17) + 6], tbl->TableGb[(i*17) + 7],
      tbl->TableGb[(i*17) + 8], tbl->TableGb[(i*17) + 9],
      tbl->TableGb[(i*17) + 10], tbl->TableGb[(i*17) + 11],
      tbl->TableGb[(i*17) + 12], tbl->TableGb[(i*17) + 13],
      tbl->TableGb[(i*17) + 14], tbl->TableGb[(i*17) + 15],
      tbl->TableGb[(i*17) + 16]);
   }
  ALOGE("%s:%d ==========B===========\n", __func__, __LINE__);
   for (i = 0; i < ARRAY_SIZE(tbl->TableB) / 17 ; i ++) {
    ALOGE("%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f\n",
      tbl->TableB[(i*17) + 0], tbl->TableB[(i*17) + 1],
      tbl->TableB[(i*17) + 2], tbl->TableB[(i*17) + 3],
      tbl->TableB[(i*17) + 4], tbl->TableB[(i*17) + 5],
      tbl->TableB[(i*17) + 6], tbl->TableB[(i*17) + 7],
      tbl->TableB[(i*17) + 8], tbl->TableB[(i*17) + 9],
      tbl->TableB[(i*17) + 10], tbl->TableB[(i*17) + 11],
      tbl->TableB[(i*17) + 12], tbl->TableB[(i*17) + 13],
      tbl->TableB[(i*17) + 14], tbl->TableB[(i*17) + 15],
      tbl->TableB[(i*17) + 16]);
  }
  ALOGE("%s:%d ***********End*****************\n", __func__, __LINE__);

}

/** mesh_rolloff44_print_hw_table:
 *
 *  @cmd: hw table data
 *
 *  Print hw table
 *
 *  Return void
 **/
static void mesh_rolloff44_print_hw_table(MESH_RollOff_V4_ConfigCmdType *cmd)
{
  uint32_t data1 = 0, data2 = 0, i = 0;
  MESH_RollOffTable_V4 tbl;

  if (!cmd) {
    ISP_ERR("failed: cmd %p", cmd);
    return;
  }

  memset(&tbl, 0, sizeof(MESH_RollOffTable_V4));

  for (i = 0; i < ARRAY_SIZE(cmd->Table_GR_R.Table); i++) {
    data1 = cmd->Table_GR_R.Table[i] & 0x00001FFF;
    data2 = cmd->Table_GR_R.Table[i] >> 13;
    tbl.TableR[i] = Q_TO_FLOAT(10, data1);
    tbl.TableGr[i] = Q_TO_FLOAT(10, data2);
  }

  for (i = 0; i < ARRAY_SIZE(cmd->Table_GB_B.Table); i++) {
    data1 = cmd->Table_GB_B.Table[i] & 0x00001FFF;
    data2 = cmd->Table_GB_B.Table[i] >> 13;
    tbl.TableB[i] = Q_TO_FLOAT(10, data1);
    tbl.TableGb[i] = Q_TO_FLOAT(10, data2);
  }

  mesh_rolloff44_print_mesh_table(&tbl);
}

/** mesh_rolloff44_prepare_hw_table:
 *
 *  @pIn: Input rolloff table to be written to hw
 *  @cmd: output rolloff config cmd
 *
 *  Prepare the config cmd that will be written to hw from the input table
 *
 *  Return void
 **/
static void mesh_rolloff44_prepare_hw_table(const MESH_RollOffTable_V4 *pIn,
  MESH_RollOff_V4_ConfigCmdType *cmd)
{
  uint16_t i = 0;
  uint16_t data1 = 0;
  uint16_t data2 = 0;
  uint16_t MESH_H_PT = 0;
  uint16_t MESH_V_PT = 0;

  if (!pIn || !cmd) {
    ISP_ERR("failed: %p %p", pIn, cmd);
    return;
  }

  const float *Gr = pIn->TableGr;
  const float *Gb = pIn->TableGb;
  const float *B =  pIn->TableB;
  const float *R =  pIn->TableR;

  MESH_H_PT = cmd->CfgParams.num_meshgain_h + 2 ;
  MESH_V_PT = cmd->CfgParams.num_meshgain_v + 2;

  /*Gr & R pack together*/
  for (i = 0; i < (MESH_H_PT * MESH_V_PT); i++) {
    ISP_DBG("i=%d, R=0x%f, Gr=0x%f", i, *R, *Gr);
    data1 = (uint16_t)Round((*R) * (float)(1 << 10));
    CLAMP_DATA(data1);
    data2 =  (uint16_t)Round((*Gr) *(float)(1 << 10));
    CLAMP_DATA(data2);
    cmd->Table_GR_R.Table[i] = (((uint32_t)data1) &0x00001FFF) |
      (((uint32_t)data2)<<13);
    R++;
    Gr++;
  }

  /*Gb & B pack together*/
  for (i = 0; i < (MESH_H_PT * MESH_V_PT); i++) {
    ISP_DBG("i=%d, B=0x%f, Gb=0x%f", i, *B, *Gb);
    data1 = (uint16_t)Round((*B) * (float)(1 << 10));
    CLAMP_DATA(data1);
    data2 =  (uint16_t)Round((*Gb) *(float)(1 << 10));
    CLAMP_DATA(data2);
    cmd->Table_GB_B.Table[i] = (((uint32_t)data1) &0x00001FFF) |
      (((uint32_t)data2)<<13);

    B++;
    Gb++;
  }

#ifdef PRINT_HW_TABLE
    mesh_rolloff44_print_hw_table(cmd);
#endif

} /* mesh_rolloff44_prepare_hw_table */

/** mesh_rolloff44_cmd_debug:
 *
 *  @cmd: Input cmd that is to be printed
 *
 *  Print the parameters in the rolloff config cmd
 *
 *  Return void
 **/
static void mesh_rolloff44_cmd_debug(MESH_RollOff_V4_ConfigCmdType *cmd)
{
  int i;

  if (!cmd) {
    ISP_ERR("failed: cmd %p", cmd);
    return;
  }

  ISP_DBG("blockWidth = %d, blockHeight = %d, interp_factor = %d",
    cmd->CfgParams.blockWidth, cmd->CfgParams.blockHeight,
    cmd->CfgParams.interpFactor);

  ISP_DBG("subGridWidth=%d, subGridHeight=%d",
    cmd->CfgParams.subGridWidth, cmd->CfgParams.subGridHeight);

  ISP_DBG("subGridXDelta = %d,subGridYDelta=%d",
    cmd->CfgParams.subGridXDelta, cmd->CfgParams.subGridYDelta);

  ISP_DBG("BlockXIndex=%d, BlockYIndex=%d",
    cmd->CfgParams.blockXIndex, cmd->CfgParams.blockYIndex);

  ISP_DBG("PixelXIndex=%d, PixelYIndex=%d ",
    cmd->CfgParams.PixelXIndex, cmd->CfgParams.PixelYIndex);

  ISP_DBG("subGridXIndex = %d, subGridYIndex = %d",
    cmd->CfgParams.subGridXIndex, cmd->CfgParams.subGridYIndex);

  ISP_DBG("yDeltaAccum=%d, pixelOffset = %d pcaLutBankSel =%d",
    cmd->CfgParams.yDeltaAccum, cmd->CfgParams.pixelOffset,
    cmd->CfgParams.pcaLutBankSel);

  for (i = 0; i < (MESH_ROLL_OFF_V4_TABLE_SIZE - 1); i++) {
    ISP_DBG("HW Gr Table[%d]=0x%x Gb Table[%d]=0x%x", i,
      cmd->Table_GR_R.Table[i], i, cmd->Table_GB_B.Table[i]);
  }
} /* mesh_rolloff44_cmd_debug */

/** msh_rolloff44_calc_grid_config:
 *    @tableIn: Input 17x13 table
 *    @tableOut: Output 13x10 table
 *
 *  Convert 17x13 table to 13x10 table if size is >130. Then convert 13x10 float
 *  table to Q10
 *
 *
 *  Return void
 **/
static boolean mesh_rolloff44_calc_grid_config(int raw_width, int raw_height,
  int *scale_cubic, int *deltah, int *deltav, int *subgridh, int *subgridv,
  int *nh, int *nv)
{
  int level, w, h, sgh, sgv, gh, gv, dh, dv, nx, ny;

  nx = MESH_ROLLOFF_HORIZONTAL_GRIDS_MAX;
  ny = MESH_ROLLOFF_VERTICAL_GRIDS_MAX;

  w = raw_width >> 1;  /* per-channel image width */
  h = raw_height >> 1;  /* per-channel image height */

  level = 4; /* Initial bicubic level level as 1 more than maximum 3 */
  do {
      if (level==0 && nx <= MIN_BICUBIC_H_GRID_NUM &&
        ny <= MIN_BICUBIC_V_GRID_NUM) {
        ISP_ERR("Error: small image");
        return FALSE;
      }
      if (level > 0) {
        level--;
      }
      else if ((nx > MIN_BICUBIC_H_GRID_NUM) &&
          (ny > MIN_BICUBIC_V_GRID_NUM)) {
        nx -= 4;
        ny -= 3;
        level =3;
      }

      sgh = (w + nx - 1) / nx;  /* Ceil */
      sgh = (sgh + (1 << level) - 1) >> level;  /* Ceil */
      gh = sgh << level;     /* Bayer grid width */
      dh = gh * nx - w; /* two-side overhead */
      sgv = (h + ny - 1) / ny;  /* Ceil */
      sgv = (sgv + (1 << level) - 1) >> level;   /* Ceil */
      gv = sgv << level;     /* Bayer grid height */
      dv = gv * ny - h; /* two-side overhead */
    } while ((sgh < 9) || (sgv < 9) || (dh >= gh) || (dv >= gv) ||
      (gh - (dh + 1) / 2 < 18) ||(gv - (dv + 1) / 2 < 9) ||
      (sgh - (((dh + 1) / 2) % sgh)) < 9);

  ISP_DBG(" %s level = %d, sgh = %d, sgv = %d\n", __func__, level ,sgh, sgv);
  ISP_DBG(" %s nx = %d, ny = %d,\n", __func__, nx, ny);

  *scale_cubic = 1 << level;
  *deltah = (dh + 1) >> 1;
  *deltav = (dv + 1) >> 1;
  *subgridh = sgh;
  *subgridv = sgv;
  *nh = nx;
  *nv = ny;
  return TRUE;
}


/** mesh_rolloff_calc_config:
 *
 *  @camif_width:
 *  @camif_height:
 *  @interp_factor: output interpolation factor
 *  @block_width: output block width
 *  @block height: output block height
 *  @sub_grid_width: output sub grid width
 *  @sub_grid_height: output sub grid height
 *
 *  Calculate parameters related to sub grid
 *
 *  Return void
 **/
static boolean mesh_rolloff_calc_config(uint16_t camif_width,
  uint16_t camif_height, MESH_RollOff_v4_ConfigParams *config)
{
  int SGwidth, BlockWidth, MeshOverWidthHalf;
  int SGheight, BlockHeight, MeshOverHeightHalf;
  int subgrid_num = 0;
  int NumHorizontalGrids = MESH_ROLLOFF_HORIZONTAL_GRIDS_MAX;
  int NumVerticalGrids = MESH_ROLLOFF_VERTICAL_GRIDS_MAX;

  if (FALSE == mesh_rolloff44_calc_grid_config(camif_width, camif_height,
    &subgrid_num , &MeshOverWidthHalf, &MeshOverHeightHalf, &SGwidth,
    &SGheight, &NumHorizontalGrids, &NumVerticalGrids))
  {
    ISP_ERR("mesh_rolloff44_calc_grid_config is failed");
    return FALSE;
  }

  ISP_DBG("camif_width %d camif_height %d subgrid_num %d", camif_width,
    camif_height, subgrid_num);
  ISP_DBG("MeshOverWidthHalf %d MeshOverHeightHalf %d",
    MeshOverWidthHalf, MeshOverHeightHalf);
  ISP_DBG("SGwidth %d SGheight %d", SGwidth, SGheight);
  ISP_DBG("NumHorizontalGrids %d NumVerticalGrids %d",
    NumHorizontalGrids, NumVerticalGrids);
  config->num_meshgain_h = (uint32_t)(NumHorizontalGrids - 1);
  config->num_meshgain_v = (uint32_t)(NumVerticalGrids - 1);
  config->blockWidth = (uint32_t)(SGwidth * subgrid_num  - 1);
  config->blockHeight = (uint32_t)(SGheight * subgrid_num  - 1);

  config->subGridWidth = (uint32_t)(SGwidth - 1);
  config->subGridHeight = (uint32_t)(SGheight - 1);
  config->interpFactor = (uint32_t)((log10((float)subgrid_num ) /
    log10(2.0))+0.5);
  config->subGridXDelta = (uint32_t)((1 << X_DELTA_Q_LEN) / SGwidth);
  config->subGridYDelta = (uint32_t)((1 << Y_DELTA_Q_LEN) / SGheight);

  config->PixelXIndex = (uint32_t)(((MeshOverWidthHalf * 2) >> 1) % SGwidth);
  config->PixelYIndex = (uint32_t)(((MeshOverHeightHalf * 2) >> 1) % SGheight);
  config->blockXIndex = (uint32_t)(((MeshOverWidthHalf * 2) >> 1) /
    (SGwidth * subgrid_num));
  config->blockYIndex = (uint32_t)(((MeshOverHeightHalf * 2) >> 1) /
    (SGheight * subgrid_num));
  config->yDeltaAccum = (uint32_t)(config->subGridYDelta * config->PixelYIndex);
  config->subGridXIndex = (uint32_t)(((MeshOverWidthHalf * 2) >> 1) / SGwidth);
  config->subGridYIndex = (uint32_t)(((MeshOverHeightHalf * 2) >> 1) /
    SGheight);

  return TRUE;

} /* mesh_rolloff44_calc_sub_grid */

/** mesh_rolloff44_update_hw_table:
 *
 *  @cmd: input config cmd that is to be written to hw
 *  @tableIn: final rolloff table
 *
 *  Method invoked at SOF to write config and table to hw
 *
 *  Return void
 **/
static boolean mesh_rolloff44_update_hw_table(mesh_rolloff44_t *mesh_rolloff,
  MESH_RollOff_V4_ConfigCmdType *cmd, MESH_RollOffTable_V4 *tableIn)
{
  uint32_t camif_width = 0, camif_height = 0;

  if (!mesh_rolloff || !cmd || !tableIn) {
    ISP_ERR("failed: %p %p %p", mesh_rolloff, cmd, tableIn);
    return FALSE;
  }

  ISP_DBG("sensor_parms.lastPixel=%d sensor_parms.firstPixel=%d",
          mesh_rolloff->sensor_out_info.request_crop.last_pixel,
          mesh_rolloff->sensor_out_info.request_crop.first_pixel);
  ISP_DBG("sensor_parms.lastLine=%d sensor_parms.firstLine=%d",
          mesh_rolloff->sensor_out_info.request_crop.last_line,
          mesh_rolloff->sensor_out_info.request_crop.first_line);

  camif_width = mesh_rolloff->sensor_out_info.request_crop.last_pixel
     - mesh_rolloff->sensor_out_info.request_crop.first_pixel + 1;
  camif_height = mesh_rolloff->sensor_out_info.request_crop.last_line
     - mesh_rolloff->sensor_out_info.request_crop.first_line + 1;

  /* Update RollOffTableConfig command and send command */
  /* Note: Bank selection will be handled in the kernel */
  cmd->CfgParams.pcaLutBankSel = 0;
  cmd->CfgParams.pixelOffset = 0;

  /* ROLLOFF STRIPE CFG: in non-striping mode, these registers programmed 0 */
  cmd->CfgParams.blockXIndex = 0;
  cmd->CfgParams.blockYIndex = 0;
  cmd->CfgParams.PixelXIndex = 0;
  cmd->CfgParams.PixelYIndex = 0;
  cmd->CfgParams.subGridXIndex = 0;
  cmd->CfgParams.subGridYIndex = 0;

  /*configurable H grid number and V grid number*/
  cmd->CfgParams.num_meshgain_h = MESH_ROLLOFF_HORIZONTAL_GRIDS_MAX;
  cmd->CfgParams.num_meshgain_v = MESH_ROLLOFF_VERTICAL_GRIDS_MAX;

  /* the sub grid here is bayer channel based in stead of pixel base
    bayer_width(height) = pixel_Width(Height)/2, the calc_sub_grid system algo
    already calculate the subgrid based on bayer channel.
    the configuration are all based on Bayer width and height */
  if (FALSE == mesh_rolloff_calc_config(camif_width, camif_height,
    &cmd->CfgParams)) {
     ISP_DBG("%s: mesh_rolloff_calc_config is failed", __func__);
     return  FALSE;
  }

  /* calculate right stripe settings if needed based on the whole frame
     setting */
  if (mesh_rolloff->ispif_out_info.is_split &&
      mesh_rolloff->isp_out_info.stripe_id == ISP_STRIPE_RIGHT) {
    uint32_t block_w      = (cmd->CfgParams.blockWidth + 1) * 2;
    uint32_t subgrid_w    = (cmd->CfgParams.subGridWidth + 1) * 2;
    uint32_t pixelXIndex  = cmd->CfgParams.PixelXIndex * 2;
    uint32_t right_offset = pixelXIndex +
      block_w * cmd->CfgParams.blockXIndex +
      subgrid_w * cmd->CfgParams.subGridXIndex +
      mesh_rolloff->isp_out_info.right_stripe_offset;

    cmd->CfgParams.blockXIndex   = right_offset / block_w;
    cmd->CfgParams.subGridXIndex =
      (right_offset - cmd->CfgParams.blockXIndex * block_w) / subgrid_w;
    cmd->CfgParams.PixelXIndex   =
      (right_offset - cmd->CfgParams.blockXIndex * block_w -
      cmd->CfgParams.subGridXIndex * subgrid_w) / 2;
  }
  cmd->CfgParams.yDeltaAccum =
    cmd->CfgParams.PixelYIndex * cmd->CfgParams.subGridYDelta;

  mesh_rolloff44_prepare_hw_table(tableIn, cmd);
  mesh_rolloff44_cmd_debug(cmd);
  ISP_DBG("pixelOffset %d", cmd->CfgParams.pixelOffset);
  ISP_DBG("pcaLutBankSel %d", cmd->CfgParams.pcaLutBankSel);
  ISP_DBG("num_meshgain_h %d", cmd->CfgParams.num_meshgain_h);
  ISP_DBG("num_meshgain_v %d", cmd->CfgParams.num_meshgain_v);
  ISP_DBG("blockWidth %d", cmd->CfgParams.blockWidth);
  ISP_DBG("blockHeight %d", cmd->CfgParams.blockHeight);
  ISP_DBG("subGridXDelta %d", cmd->CfgParams.subGridXDelta);
  ISP_DBG("subGridYDelta %d", cmd->CfgParams.subGridYDelta);
  ISP_DBG("interpFactor %d", cmd->CfgParams.interpFactor);
  ISP_DBG("subGridWidth %d", cmd->CfgParams.subGridWidth);
  ISP_DBG("subGridHeight %d", cmd->CfgParams.subGridHeight);
  ISP_DBG("blockWidthRight %d", cmd->CfgParams.blockWidthRight);
  ISP_DBG("blockHeightRight %d", cmd->CfgParams.blockHeightRight);
  ISP_DBG("subGridXDeltaRight %d", cmd->CfgParams.subGridXDeltaRight);
  ISP_DBG("subGridYDeltaRight %d", cmd->CfgParams.subGridYDeltaRight);
  ISP_DBG("interpFactorRight %d", cmd->CfgParams.interpFactorRight);
  ISP_DBG("subGridWidthRight %d", cmd->CfgParams.subGridWidthRight);
  ISP_DBG("subGridHeightRight %d", cmd->CfgParams.subGridHeightRight);
  ISP_DBG("blockXIndex %d", cmd->CfgParams.blockXIndex);
  ISP_DBG("blockYIndex %d", cmd->CfgParams.blockYIndex);
  ISP_DBG("PixelXIndex %d", cmd->CfgParams.PixelXIndex);
  ISP_DBG("PixelYIndex %d", cmd->CfgParams.PixelYIndex);
  ISP_DBG("yDeltaAccum %d", cmd->CfgParams.yDeltaAccum);
  ISP_DBG("subGridXIndex %d", cmd->CfgParams.subGridXIndex);
  ISP_DBG("subGridYIndex %d", cmd->CfgParams.subGridYIndex);

  /* save the full roi cfg params to BE CFG before calculating for split stripe */
  cmd->be_cfg_params.scale_cubic = pow(2, cmd->CfgParams.interpFactor) ;
  cmd->be_cfg_params.deltah = (cmd->CfgParams.blockWidth * cmd->CfgParams.blockXIndex) +
   (cmd->CfgParams.subGridWidth * cmd->CfgParams.subGridXIndex) + cmd->CfgParams.PixelXIndex;
  cmd->be_cfg_params.deltav = (cmd->CfgParams.blockHeight * cmd->CfgParams.blockYIndex) +
   (cmd->CfgParams.subGridHeight * cmd->CfgParams.subGridYIndex) + cmd->CfgParams.PixelYIndex;
  cmd->be_cfg_params.subgridh = cmd->CfgParams.subGridWidth + 1;
  cmd->be_cfg_params.subgridv = cmd->CfgParams.subGridHeight + 1;
  ISP_DBG("scale_cubic %d delta h %d deltav %d subgridh %d subgridv %d",
    cmd->be_cfg_params.scale_cubic,
    cmd->be_cfg_params.deltah, cmd->be_cfg_params.deltav,
    cmd->be_cfg_params.subgridh, cmd->be_cfg_params.subgridv);

  return TRUE;
} /* mesh_rolloff44_update_hw_table */

/** mesh_rolloff44_update_min_stripe_overlap
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  trigger update based on 3A event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean mesh_rolloff44_update_min_stripe_overlap(
  isp_sub_module_t *isp_sub_module, void *data)
{
  boolean                        ret = TRUE;
  mesh_rolloff44_t               *mesh_rolloff = NULL;
  isp_private_event_t           *private_event = NULL;
  int                            rc = 0;
  isp_stripe_limit_info_t       *stripe_limit = NULL;
  uint32_t                       sensor_output_width = 0;
  uint32_t                       sensor_output_height = 0;
  uint32_t                       block_width = 0;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  if (!isp_sub_module->submod_enable) {
    ISP_DBG("skip, rolloff enabled %d", isp_sub_module->submod_enable);
    return TRUE;
  }

  mesh_rolloff = (mesh_rolloff44_t *)isp_sub_module->private_data;
  if (!mesh_rolloff) {
    ISP_ERR("failed: mesh_rolloff %p", mesh_rolloff);
    return FALSE;
  }

  private_event = (isp_private_event_t *)data;
  if (!private_event) {
    ISP_ERR("failed: private_event %p", private_event);
    return FALSE;
  }

  stripe_limit = (isp_stripe_limit_info_t *)private_event->data;
  if (!stripe_limit) {
    ISP_ERR("failed: stripe_limit %p", stripe_limit);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  sensor_output_width = mesh_rolloff->sensor_out_info.request_crop.last_pixel
    - mesh_rolloff->sensor_out_info.request_crop.first_pixel + 1;
  sensor_output_height = mesh_rolloff->sensor_out_info.request_crop.last_line
    - mesh_rolloff->sensor_out_info.request_crop.first_line + 1;
  mesh_rolloff_calc_config(sensor_output_width, sensor_output_height,
    &mesh_rolloff->mesh_rolloff_cmd.CfgParams);
  block_width = mesh_rolloff->mesh_rolloff_cmd.CfgParams.blockWidth + 1;

  /*rolloff limitation*/
  if ((block_width * 2) < ISP_MESH_ROLLOFF_MIN_STRIPE_OVERLAP) {
    ISP_ERR("failed: rolloff gridwidth %d < min overlap half %d, can not"\
      "support!!!", block_width, ISP_MESH_ROLLOFF_MIN_STRIPE_OVERLAP);
    ret = FALSE;
    goto ERROR;
  } else {
    ISP_DBG("<stripe_dbg> sub_grid_width %d", block_width);
    stripe_limit->rolloff_min_dist_from_grid= ISP_MESH_ROLLOFF_MIN_STRIPE_OVERLAP;
    stripe_limit->rolloff_grid_width = block_width * 2;
    stripe_limit->rolloff_subgrid_width =
      (mesh_rolloff->mesh_rolloff_cmd.CfgParams.subGridWidth + 1) * 2;
    stripe_limit->rolloff_min_dist_from_subgrid =
      ISP_MESH_ROLLOFF_MIN_DIST_TO_SUBGRID;
  }

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;
} /* mesh_rolloff44_update_min_stripe_overlap */

/** mesh_rolloff44_set_stripe_info
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  trigger update based on 3A event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean mesh_rolloff44_set_stripe_info(
  isp_sub_module_t *isp_sub_module, void *data)
{
  boolean                        ret = TRUE;
  mesh_rolloff44_t               *mesh_rolloff = NULL;
  isp_private_event_t           *private_event = NULL;
  int                            rc = 0;
  ispif_out_info_t               *ispif_out_info = NULL;
  uint32_t                       block_width = 0;
  uint32_t                       sensor_output_width = 0;
  uint32_t                       sensor_output_height = 0;
  uint32_t                       distance_from_grid = 0;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  if (!isp_sub_module->submod_enable) {
    ISP_DBG("skip, rolloff enabled %d", isp_sub_module->submod_enable);
    return TRUE;
  }

  mesh_rolloff = (mesh_rolloff44_t *)isp_sub_module->private_data;
  if (!mesh_rolloff) {
    ISP_ERR("failed: mesh_rolloff %p", mesh_rolloff);
    return FALSE;
  }

  private_event = (isp_private_event_t *)data;
  if (!private_event) {
    ISP_ERR("failed: private_event %p", private_event);
    return FALSE;
  }

  ispif_out_info = (ispif_out_info_t *)private_event->data;
  if (!ispif_out_info) {
    ISP_ERR("failed: stripe_limit %p", ispif_out_info);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  sensor_output_width = mesh_rolloff->sensor_out_info.request_crop.last_pixel
    - mesh_rolloff->sensor_out_info.request_crop.first_pixel + 1;
  sensor_output_height = mesh_rolloff->sensor_out_info.request_crop.last_line
    - mesh_rolloff->sensor_out_info.request_crop.first_line + 1;
  mesh_rolloff_calc_config(sensor_output_width, sensor_output_height,
    &mesh_rolloff->mesh_rolloff_cmd.CfgParams);
  block_width = mesh_rolloff->mesh_rolloff_cmd.CfgParams.blockWidth + 1;

  mesh_rolloff->ispif_out_info = *ispif_out_info;
  distance_from_grid = (sensor_output_width / 2) -
    (ispif_out_info->right_stripe_offset -
    mesh_rolloff->sensor_out_info.request_crop.first_pixel);

  /*rolloff limitation*/
  if (distance_from_grid % (block_width * 2) <
        ISP_MESH_ROLLOFF_MIN_STRIPE_OVERLAP) {
    ISP_ERR("failed: rolloff gridwidth %d, dist_from_grid %d, can not "\
      "support!!!", block_width, (ispif_out_info->overlap / 2));
    ret = FALSE;
    goto ERROR;
  }

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;
} /* mesh_rolloff44_update_min_stripe_overlap */

/** mesh_rolloff44_set_split_info:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to sensor_out_info_t
 *
 *  Copy camif dimension in module private
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean mesh_rolloff44_set_split_info(isp_sub_module_t *isp_sub_module,
  void *data)
{
  mesh_rolloff44_t            *mesh_rolloff = NULL;
  isp_out_info_t              *isp_split_out_info = NULL;
  isp_private_event_t         *private_event = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  private_event = (isp_private_event_t *)data;
  isp_split_out_info = (isp_out_info_t *)private_event->data;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  mesh_rolloff = (mesh_rolloff44_t *)isp_sub_module->private_data;
  if (!mesh_rolloff) {
    ISP_ERR("failed: mesh_rolloff %p", mesh_rolloff);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }
  mesh_rolloff->isp_out_info = *isp_split_out_info;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
}

/** mesh_rolloff44_store_hw_update
 *
 *  @isp_sub_module: isp sub module handle
 *  @mesh_rolloff: mesh_rolloff handle
 *
 *  Create hw update list and store in isp sub module
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean mesh_rolloff44_store_hw_update(isp_sub_module_t *isp_sub_module,
  mesh_rolloff44_t *mesh_rolloff)
{
  boolean                        ret = TRUE;
  int                            i = 0;
  struct msm_vfe_cfg_cmd2       *cfg_cmd = NULL;
  struct msm_vfe_reg_cfg_cmd    *reg_cfg_cmd = NULL;
  struct msm_vfe_cfg_cmd_list   *hw_update = NULL;
  MESH_RollOff_V4_ConfigCmdType *mesh_rolloff_cmd = NULL;

  if (!isp_sub_module || !mesh_rolloff) {
    ISP_ERR("failed: %p %p", isp_sub_module, mesh_rolloff);
    return FALSE;
  }

  if (ROLLOFF_CGC_OVERRIDE == TRUE) {
    ret = isp_sub_module_util_update_cgc_mask(isp_sub_module,
      ROLLOFF_CGC_OVERRIDE_REGISTER, ROLLOFF_CGC_OVERRIDE_BIT, TRUE);
    if (ret == FALSE) {
      ISP_ERR("failed: enable cgc");
    }
  }

  hw_update = (struct msm_vfe_cfg_cmd_list *)CAM_MALLOC(sizeof(*hw_update));
  if (!hw_update) {
    ISP_ERR("failed: hw_update %p", hw_update);
    return FALSE;
  }
  memset(hw_update, 0, sizeof(*hw_update));

  cfg_cmd = &hw_update->cfg_cmd;

  reg_cfg_cmd = (struct msm_vfe_reg_cfg_cmd *)CAM_MALLOC(11 * sizeof(*reg_cfg_cmd));
  if (!reg_cfg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
    goto ERROR_REG_CFG_CMD;
  }
  memset(reg_cfg_cmd, 0, sizeof(*reg_cfg_cmd));

  mesh_rolloff_cmd =
    (MESH_RollOff_V4_ConfigCmdType *)CAM_MALLOC(sizeof(*mesh_rolloff_cmd));
  if (!mesh_rolloff_cmd) {
    ISP_ERR("failed: mesh_rolloff_cmd %p", mesh_rolloff_cmd);
    goto ERROR_MESH_ROLLOFF_CMD;
  }
  memset(mesh_rolloff_cmd, 0, sizeof(*mesh_rolloff_cmd));

  uint32_t rolloff_lut_channel_left_GR_R =
    (mesh_rolloff->mesh_rolloff_cmd.CfgParams.pcaLutBankSel == 0) ?
      ROLLOFF_RAM_L_GR_R_BANK0 : ROLLOFF_RAM_L_GR_R_BANK1;

  uint32_t rolloff_lut_channel_left_GB_B =
    (mesh_rolloff->mesh_rolloff_cmd.CfgParams.pcaLutBankSel == 0) ?
      ROLLOFF_RAM_L_GB_B_BANK0 : ROLLOFF_RAM_L_GB_B_BANK1;

    /* prepare dmi_set and dmi_reset fields */
    mesh_rolloff->mesh_rolloff_cmd.dmi_set_gr_r[0] =
      ISP_DMI_CFG_DEFAULT + rolloff_lut_channel_left_GR_R;
    mesh_rolloff->mesh_rolloff_cmd.dmi_set_gr_r[1] = 0;

    mesh_rolloff->mesh_rolloff_cmd.dmi_reset_gr_r[0] =
      ISP_DMI_CFG_DEFAULT + ISP_DMI_NO_MEM_SELECTED;
    mesh_rolloff->mesh_rolloff_cmd.dmi_reset_gr_r[1] = 0;

    mesh_rolloff->mesh_rolloff_cmd.dmi_set_gb_b[0] =
      ISP_DMI_CFG_DEFAULT + rolloff_lut_channel_left_GB_B;
    mesh_rolloff->mesh_rolloff_cmd.dmi_set_gb_b[1] = 0;

    mesh_rolloff->mesh_rolloff_cmd.dmi_reset_gb_b[0] =
      ISP_DMI_CFG_DEFAULT + ISP_DMI_NO_MEM_SELECTED;
    mesh_rolloff->mesh_rolloff_cmd.dmi_reset_gb_b[1] = 0;

    *mesh_rolloff_cmd = mesh_rolloff->mesh_rolloff_cmd;
    cfg_cmd->cfg_data = (void *)mesh_rolloff_cmd;
    cfg_cmd->cmd_len = sizeof(mesh_rolloff->mesh_rolloff_cmd);
    cfg_cmd->cfg_cmd = (void *)reg_cfg_cmd;
    cfg_cmd->num_cfg = 11;

    /* set dmi to proper linearization bank */
    reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
    reg_cfg_cmd[0].cmd_type = VFE_WRITE_MB;
    reg_cfg_cmd[0].u.rw_info.reg_offset = ISP_DMI_CFG_OFF;
    reg_cfg_cmd[0].u.rw_info.len = 1 * sizeof(uint32_t);

    reg_cfg_cmd[1].u.rw_info.cmd_data_offset =
      reg_cfg_cmd[0].u.rw_info.cmd_data_offset +
      reg_cfg_cmd[0].u.rw_info.len;
    reg_cfg_cmd[1].cmd_type = VFE_WRITE_MB;
    reg_cfg_cmd[1].u.rw_info.reg_offset = ISP_DMI_ADDR;
    reg_cfg_cmd[1].u.rw_info.len = 1 * sizeof(uint32_t);

    /* send dmi data for Gr-R */
    reg_cfg_cmd[2].cmd_type = VFE_WRITE_DMI_32BIT;
    reg_cfg_cmd[2].u.dmi_info.hi_tbl_offset = 0;
    reg_cfg_cmd[2].u.dmi_info.lo_tbl_offset =
      reg_cfg_cmd[1].u.rw_info.cmd_data_offset +
      reg_cfg_cmd[1].u.rw_info.len;
    reg_cfg_cmd[2].u.dmi_info.len = sizeof(uint32_t) *
        MESH_ROLL_OFF_V4_TABLE_SIZE * 1;

    /* reset dmi to no bank*/
    reg_cfg_cmd[3].u.rw_info.cmd_data_offset =
      reg_cfg_cmd[2].u.dmi_info.lo_tbl_offset +
      reg_cfg_cmd[2].u.dmi_info.len;
    reg_cfg_cmd[3].cmd_type = VFE_WRITE_MB;
    reg_cfg_cmd[3].u.rw_info.reg_offset = ISP_DMI_CFG_OFF;
    reg_cfg_cmd[3].u.rw_info.len = 1 * sizeof(uint32_t);

    reg_cfg_cmd[4].u.rw_info.cmd_data_offset =
      reg_cfg_cmd[3].u.rw_info.cmd_data_offset +
      reg_cfg_cmd[3].u.rw_info.len;
    reg_cfg_cmd[4].cmd_type = VFE_WRITE_MB;
    reg_cfg_cmd[4].u.rw_info.reg_offset = ISP_DMI_ADDR;
    reg_cfg_cmd[4].u.rw_info.len = 1 * sizeof(uint32_t);

    /* set dmi to proper linearization bank */
    reg_cfg_cmd[5].u.rw_info.cmd_data_offset =
      reg_cfg_cmd[4].u.rw_info.cmd_data_offset +
      reg_cfg_cmd[4].u.rw_info.len;
    reg_cfg_cmd[5].cmd_type = VFE_WRITE_MB;
    reg_cfg_cmd[5].u.rw_info.reg_offset = ISP_DMI_CFG_OFF;
    reg_cfg_cmd[5].u.rw_info.len = 1 * sizeof(uint32_t);

    reg_cfg_cmd[6].u.rw_info.cmd_data_offset =
      reg_cfg_cmd[5].u.rw_info.cmd_data_offset +
      reg_cfg_cmd[5].u.rw_info.len;
    reg_cfg_cmd[6].cmd_type = VFE_WRITE_MB;
    reg_cfg_cmd[6].u.rw_info.reg_offset = ISP_DMI_ADDR;
    reg_cfg_cmd[6].u.rw_info.len = 1 * sizeof(uint32_t);

    /* send dmi data for Gb-B */
    reg_cfg_cmd[7].cmd_type = VFE_WRITE_DMI_32BIT;
    reg_cfg_cmd[7].u.dmi_info.hi_tbl_offset = 0;
    reg_cfg_cmd[7].u.dmi_info.lo_tbl_offset =
      reg_cfg_cmd[6].u.rw_info.cmd_data_offset +
        reg_cfg_cmd[6].u.rw_info.len;
    reg_cfg_cmd[7].u.dmi_info.len = sizeof(uint32_t) *
        MESH_ROLL_OFF_V4_TABLE_SIZE * 1;

    /* reset dmi to no bank*/
    reg_cfg_cmd[8].u.rw_info.cmd_data_offset =
      reg_cfg_cmd[7].u.dmi_info.lo_tbl_offset +
      reg_cfg_cmd[7].u.dmi_info.len;
    reg_cfg_cmd[8].cmd_type = VFE_WRITE_MB;
    reg_cfg_cmd[8].u.rw_info.reg_offset = ISP_DMI_CFG_OFF;
    reg_cfg_cmd[8].u.rw_info.len = 1 * sizeof(uint32_t);

    reg_cfg_cmd[9].u.rw_info.cmd_data_offset =
      reg_cfg_cmd[8].u.rw_info.cmd_data_offset +
      reg_cfg_cmd[8].u.rw_info.len;
    reg_cfg_cmd[9].cmd_type = VFE_WRITE_MB;
    reg_cfg_cmd[9].u.rw_info.reg_offset = ISP_DMI_ADDR;
    reg_cfg_cmd[9].u.rw_info.len = 1 * sizeof(uint32_t);

    reg_cfg_cmd[10].u.rw_info.cmd_data_offset =
      reg_cfg_cmd[9].u.rw_info.cmd_data_offset +
      reg_cfg_cmd[9].u.rw_info.len;
    reg_cfg_cmd[10].cmd_type = VFE_WRITE;
    reg_cfg_cmd[10].u.rw_info.reg_offset = ISP_MESH_ROLLOFF_CFG_OFF;
    reg_cfg_cmd[10].u.rw_info.len = ISP_MESH_ROLLOFF_CFG_LEN *
      sizeof(uint32_t);

    mesh_rolloff44_cmd_debug(&mesh_rolloff->mesh_rolloff_cmd);

    ISP_LOG_LIST("hw_update %p cfg_cmd %p", hw_update, cfg_cmd->cfg_cmd);
    ret = isp_sub_module_util_store_hw_update(isp_sub_module, hw_update);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_sub_module_util_store_hw_update");
      goto ERROR_APPEND;
    }
    mesh_rolloff->applied_hw_table_gr_r =
      mesh_rolloff->mesh_rolloff_cmd.Table_GR_R;
    mesh_rolloff->applied_hw_table_gb_b =
      mesh_rolloff->mesh_rolloff_cmd.Table_GB_B;
    mesh_rolloff->applied_table =
      mesh_rolloff->mesh_rolloff_param.input_table;
    mesh_rolloff->mesh_rolloff_cmd.CfgParams.pcaLutBankSel ^= 1;

  if (ROLLOFF_CGC_OVERRIDE == TRUE) {
    ret = isp_sub_module_util_update_cgc_mask(isp_sub_module,
      ROLLOFF_CGC_OVERRIDE_REGISTER, ROLLOFF_CGC_OVERRIDE_BIT, FALSE);
    if (ret == FALSE) {
      ISP_ERR("failed: disable cgc");
    }
  }

  return TRUE;

ERROR_APPEND:
  free(mesh_rolloff_cmd);
ERROR_MESH_ROLLOFF_CMD:
  free(reg_cfg_cmd);
ERROR_REG_CFG_CMD:
  free(hw_update);
  return FALSE;
} /* mesh_rolloff44_store_hw_update */

/** rolloff_ez_isp_update
 *
 *  @isp_sub_module: isp sub module handle
 *  @mesh_rolloff: rolloff module handle
 *  @rolloff_Diag: roll off Diag handle
 *
 *  eztune update
 *
 *  Return NONE
 **/
static void mesh_rolloff44_ez_isp_update(
 mesh_rolloff44_t *mesh_rolloff,
 rolloff_t *rolloff_Diag)
{
  uint32_t i = 0;
  for (i = 0; i < MESH_ROLLOFF_SIZE; i++) {
    rolloff_Diag->coefftable_R[i] =
      Q_TO_FLOAT(10, mesh_rolloff->applied_table.TableR[i]);
    rolloff_Diag->coefftable_Gr[i] =
      Q_TO_FLOAT(10, mesh_rolloff->applied_table.TableGr[i]);
    rolloff_Diag->coefftable_Gb[i] =
      Q_TO_FLOAT(10, mesh_rolloff->applied_table.TableB[i]);
    rolloff_Diag->coefftable_B[i] =
      Q_TO_FLOAT(10, mesh_rolloff->applied_table.TableGb[i]);
  }

}

/** mesh_rolloff44_fill_vfe_diag_data:
 *
 *  @mod: mesh_rolloff module instance
 *  @isp_sub_module: isp sub module
 *  @sub_module_output: output to be filled
 *  This function fills vfe diagnostics information
 *
 *  Return: TRUE success
 **/
static boolean mesh_rolloff44_fill_vfe_diag_data(mesh_rolloff44_t *mod,
  isp_sub_module_t *isp_sub_module, isp_sub_module_output_t *sub_module_output)
{
  boolean                   ret = TRUE;
  rolloff_t                *rolloff_diag = NULL;
  vfe_diagnostics_t        *vfe_diag = NULL;

  if (sub_module_output->frame_meta) {
    sub_module_output->frame_meta->vfe_diag_enable =
      isp_sub_module->vfe_diag_enable;
    vfe_diag = &sub_module_output->frame_meta->vfe_diag;
    rolloff_diag = &(vfe_diag->prev_rolloff);
    mesh_rolloff44_ez_isp_update(mod, rolloff_diag);
  }
  return ret;
}

/** mesh_rolloff_tintless_lowlight_adjust:
 *
 *  @tintless_params: tintless params
 *
 *  adjust tintless output table
 *
 *  Return void
 **/
static void mesh_rolloff44_tintless_lowlight_adjust(
  mesh_rolloff44_tintless_params_t *tintless_params)
{
  int                      i = 0, j = 0;
  float                    min_gain = 1.0;
  mesh_rolloff_array_type *tintless_array = NULL;
  mesh_rolloff_array_type *adjust_tintless_array = NULL;

  if (!tintless_params) {
    ISP_ERR("failed: tintless_params %p", tintless_params);
    return;
  }

  tintless_array = &tintless_params->tintless_array;
  adjust_tintless_array = &tintless_params->adjust_tintless_array;

  /*adjust rolloff table for low light, also get normalize ratio by min gain*/
  for (i = 0; i < tintless_array->mesh_rolloff_table_size; i++) {
    adjust_tintless_array->r_gain[i] =
      tintless_array->r_gain[i] * tintless_params->current_adjust[i];
    min_gain =
      MIN(min_gain, adjust_tintless_array->r_gain[i]);

    adjust_tintless_array->gb_gain[i] =
      tintless_array->gb_gain[i] * tintless_params->current_adjust[i];
    min_gain =
      MIN(min_gain, adjust_tintless_array->gb_gain[i]);

    adjust_tintless_array->gr_gain[i] =
      tintless_array->gr_gain[i] * tintless_params->current_adjust[i];
    min_gain =
      MIN(min_gain, adjust_tintless_array->gr_gain[i]);

    adjust_tintless_array->b_gain[i] =
      tintless_array->b_gain[i] * tintless_params->current_adjust[i];
    min_gain =
      MIN(min_gain, adjust_tintless_array->b_gain[i]);
  }

  if (min_gain < 1.0 && min_gain > 0.0) {
    ISP_DBG("min_gain = %f, Normalize rolloff table!\n", min_gain);
    for (i = 0; i < MESH_ROLLOFF_SIZE; i++) {
      adjust_tintless_array->r_gain[i] /= min_gain;
      adjust_tintless_array->gb_gain[i] /= min_gain;
      adjust_tintless_array->gr_gain[i] /= min_gain;
      adjust_tintless_array->b_gain[i] /= min_gain;
    }
  }

} /* mesh_rolloff_tintless_lowlight_adjust */

/** mesh_rolloff44_tintless_set_hw_table:
 *
 *  @Q10_table: mesh rolloff44 v4 table
 *  @tintless_params: tintless params
 *
 *  overwrite tintless output to mod input table
 *
 *  Return 0 on Success, negative on ERROR
 **/
static void mesh_rolloff44_tintless_set_hw_table(
  MESH_RollOffTable_V4 *Q10_table,
  mesh_rolloff44_tintless_params_t *tintless_params)
{
  int32_t                  i = 0;
  mesh_rolloff_array_type *tintless_array = NULL;

  tintless_array = &tintless_params->adjust_tintless_array;

  /* Fill out Q10 tables*/
  for (i = 0; i < tintless_array->mesh_rolloff_table_size; i++) {
    Q10_table->TableR[i]  = tintless_array->r_gain[i];
    Q10_table->TableGr[i] = tintless_array->gr_gain[i];
    Q10_table->TableGb[i] = tintless_array->gb_gain[i];
    Q10_table->TableB[i]  = tintless_array->b_gain[i];
  }
  tintless_params->tintless_array_valid = FALSE;
} /* mesh_rolloff44_tintless_set_hw_table */

/** mesh_rolloff44_convert_table:
 *
 *  @input: input table
 *  @output: output table
 *
 *  Convert input to output
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean mesh_rolloff44_convert_table(MESH_RollOffTable_V4 *input,
  mesh_rolloff_array_type *output)
{
  boolean  ret = TRUE;
  uint32_t i = 0;

  if (!input || !output) {
    ISP_ERR("failed: %p %p", input, output);
    return FALSE;
  }

  output->mesh_rolloff_table_size = MESH_ROLLOFF_SIZE;
  for (i = 0; i < MESH_ROLLOFF_SIZE; i++) {
    output->r_gain[i] = input->TableR[i];
    output->gr_gain[i] = input->TableGr[i];
    output->gb_gain[i] = input->TableGb[i];
    output->b_gain[i] = input->TableB[i];
  }

  return TRUE;
}

/** mesh_rolloff44_update_tintless_params:
 *
 *  @tintless_params: tintless params
 *  @chromatix_rolloff: chromatix rolloff
 *  @algo_params: algo params
 *  @CfgParams: cfg params
 *  @tint_strength: tintless strength config
 *
 *  Update rolloff config to be passed to tintless algo
 *
 *  Return void
 **/
static void mesh_rolloff44_update_tintless_params(
  mesh_rolloff44_t *mesh_rolloff,
  mesh_rolloff44_tintless_params_t *tintless_params,
  isp_algo_params_t *algo_params,
  MESH_RollOff_v4_ConfigParams *CfgParams,
  MESH_RollOffTable_V4 *meshRolloffTableFinal,
  chromatix_color_tint_correction_type *tint_strength)
{
  boolean                         ret = TRUE;
  isp_tintless_mesh_config_t     *tintless_mesh_config = NULL;
  isp_tintless_strength_params_t *tintless_strength_params = NULL;
  mesh_rolloff_array_type         tintless_input_table;

  if (!mesh_rolloff || !tintless_params || !algo_params ||
    !CfgParams || !tint_strength) {
    ISP_ERR("failed: %p %p %p %p %p", mesh_rolloff, tintless_params,
      algo_params, CfgParams, tint_strength);
    return;
  }

  tintless_mesh_config = &algo_params->tintless_mesh_config;

  /* mesh rolloff config tintless */
  tintless_mesh_config->is_valid = TRUE;
  tintless_mesh_config->subgrid_height = (CfgParams->subGridHeight + 1) * 2;
  tintless_mesh_config->subgrid_width = (CfgParams->subGridWidth + 1) * 2;
  tintless_mesh_config->subgrids = (1 << CfgParams->interpFactor);
  tintless_mesh_config->num_mesh_elem_cols =  HW_MESH_ROLL_NUM_COL;
  tintless_mesh_config->num_mesh_elem_rows =  HW_MESH_ROLL_NUM_ROW;
  tintless_mesh_config->offset_horizontal =
    (CfgParams->blockXIndex   * 2 * (CfgParams->blockWidth   + 1)) +
    (CfgParams->subGridXIndex * 2 * (CfgParams->subGridWidth + 1)) +
    (CfgParams->PixelXIndex   * 2);
  tintless_mesh_config->offset_vertical =
    (CfgParams->blockYIndex   * 2 * (CfgParams->blockHeight   + 1)) +
    (CfgParams->subGridYIndex * 2 * (CfgParams->subGridHeight + 1)) +
    (CfgParams->PixelYIndex   * 2);
  ISP_DBG("tintless_mesh_config->offset_horizontal %d",
    tintless_mesh_config->offset_horizontal);
  ISP_DBG("CfgParams->blockXIndex %d", CfgParams->blockXIndex);
  ISP_DBG("CfgParams->blockWidth %d", CfgParams->blockWidth);
  ISP_DBG("CfgParams->subGridXIndex %d", CfgParams->subGridXIndex);
  ISP_DBG("CfgParams->subGridWidth %d", CfgParams->subGridWidth);
  ISP_DBG("CfgParams->PixelXIndex %d", CfgParams->PixelXIndex);
  ISP_DBG("tintless_mesh_config->offset_vertical %d",
    tintless_mesh_config->offset_vertical);
  ISP_DBG("CfgParams->blockYIndex %d", CfgParams->blockYIndex);
  ISP_DBG("CfgParams->blockHeight %d", CfgParams->blockHeight);
  ISP_DBG("CfgParams->subGridYIndex %d", CfgParams->subGridYIndex);
  ISP_DBG("CfgParams->subGridHeight %d", CfgParams->subGridHeight);
  ISP_DBG("CfgParams->PixelYIndex %d", CfgParams->PixelYIndex);

  /* Update chromatix tintless strength */
  tintless_strength_params = &algo_params->tintless_strength_params;
  tintless_strength_params->is_tintless_strength_valid = TRUE;
  memcpy(&tintless_strength_params->tintless_strength, tint_strength,
    sizeof(chromatix_color_tint_correction_type));

  if (tintless_params->update_input_table == FALSE) {
    if (!meshRolloffTableFinal)
        ret = mesh_rolloff44_convert_table(
          &mesh_rolloff->rolloff_tbls.rolloff_tableset[0]->
          left[ISP_ROLLOFF_TL84_LIGHT],
          &tintless_mesh_config->input_tintless_array);
    else
        ret = mesh_rolloff44_convert_table(
          meshRolloffTableFinal,
          &tintless_mesh_config->input_tintless_array);

    if (ret == TRUE) {
      tintless_mesh_config->is_tintless_array_valid = TRUE;
    } else {
      tintless_mesh_config->is_tintless_array_valid = FALSE;
    }
  }
}

/** mesh_rolloff44_dump_tintless_array:
 *
 *  @fptr: file pointer
 *  @data: tintless array to be dumped
 *  @frame_id: frame id
 *
 *  Dump tintless array to file
 *
 *  Return void
 **/
static void mesh_rolloff44_dump_tintless_array(mesh_rolloff_array_type *data,
  uint32_t frame_id)
{
  FILE    *fptr = NULL;
  char     buf[32];
  int32_t  i = 0;

  if (!data) {
    ISP_ERR("failed: data %p", data);
    return;
  }

  snprintf(buf, sizeof(buf), QCAMERA_DUMP_FRM_LOCATION"mesh_rolloff44_%d.txt", frame_id);
  fptr = fopen(buf, "w+");
  if (!fptr) {
    ISP_ERR("failed: ptr = %p", fptr);
    return;
  }

  fprintf(fptr, "==========R===========\n");
  for (i = 0; i < 13 ; i ++) {
    fprintf(fptr, "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f\n",
      data->r_gain[(i*17) + 0], data->r_gain[(i*17) + 1],
      data->r_gain[(i*17) + 2], data->r_gain[(i*17) + 3],
      data->r_gain[(i*17) + 4], data->r_gain[(i*17) + 5],
      data->r_gain[(i*17) + 6], data->r_gain[(i*17) + 7],
      data->r_gain[(i*17) + 8], data->r_gain[(i*17) + 9],
      data->r_gain[(i*17) + 10], data->r_gain[(i*17) + 11],
      data->r_gain[(i*17) + 12], data->r_gain[(i*17) + 13],
      data->r_gain[(i*17) + 14], data->r_gain[(i*17) + 15],
      data->r_gain[(i*17) + 16]);
  }
  fprintf(fptr, "==========GR=========== \n");
  for (i = 0; i < 13 ; i ++) {
    fprintf(fptr, "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f\n",
      data->gr_gain[(i*17) + 0], data->gr_gain[(i*17) + 1],
      data->gr_gain[(i*17) + 2], data->gr_gain[(i*17) + 3],
      data->gr_gain[(i*17) + 4], data->gr_gain[(i*17) + 5],
      data->gr_gain[(i*17) + 6], data->gr_gain[(i*17) + 7],
      data->gr_gain[(i*17) + 8], data->gr_gain[(i*17) + 9],
      data->gr_gain[(i*17) + 10], data->gr_gain[(i*17) + 11],
      data->gr_gain[(i*17) + 12], data->gr_gain[(i*17) + 13],
      data->gr_gain[(i*17) + 14], data->gr_gain[(i*17) + 15],
      data->gr_gain[(i*17) + 16]);
   }
  fprintf(fptr, "==========GB=========== \n");
  for (i = 0; i < 13 ; i ++) {
    fprintf(fptr, "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f\n",
      data->gb_gain[(i*17) + 0], data->gb_gain[(i*17) + 1],
      data->gb_gain[(i*17) + 2], data->gb_gain[(i*17) + 3],
      data->gb_gain[(i*17) + 4], data->gb_gain[(i*17) + 5],
      data->gb_gain[(i*17) + 6], data->gb_gain[(i*17) + 7],
      data->gb_gain[(i*17) + 8], data->gb_gain[(i*17) + 9],
      data->gb_gain[(i*17) + 10], data->gb_gain[(i*17) + 11],
      data->gb_gain[(i*17) + 12], data->gb_gain[(i*17) + 13],
      data->gb_gain[(i*17) + 14], data->gb_gain[(i*17) + 15],
      data->gb_gain[(i*17) + 16]);
   }
  fprintf(fptr, "==========B=========== \n");
   for (i = 0; i < 13 ; i ++) {
    fprintf(fptr, "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f\n",
      data->b_gain[(i*17) + 0], data->b_gain[(i*17) + 1],
      data->b_gain[(i*17) + 2], data->b_gain[(i*17) + 3],
      data->b_gain[(i*17) + 4], data->b_gain[(i*17) + 5],
      data->b_gain[(i*17) + 6], data->b_gain[(i*17) + 7],
      data->b_gain[(i*17) + 8], data->b_gain[(i*17) + 9],
      data->b_gain[(i*17) + 10], data->b_gain[(i*17) + 11],
      data->b_gain[(i*17) + 12], data->b_gain[(i*17) + 13],
      data->b_gain[(i*17) + 14], data->b_gain[(i*17) + 15],
      data->b_gain[(i*17) + 16]);
  }

  fclose(fptr);
  return;
} /* mesh_rolloff44_dump_tintless_array */

/** mesh_rolloff44_tintless_trigger_update:
 *
 *  @isp_sub_module: isp sub module handle handle
 *  @mesh_rolloff: mesh rolloff module handle
 *
 *  Handle tintless rolloff trigger update
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean mesh_rolloff44_tintless_trigger_update(
  isp_sub_module_t *isp_sub_module, mesh_rolloff44_t *mesh_rolloff,
  MESH_RollOffTable_V4 *meshRolloffTableFinal,
  isp_sub_module_output_t *sub_module_output)
{
  int32_t                           i = 0;
  chromatix_VFE_common_type        *chrComPtr = NULL;
  MESH_RollOff_V4_ConfigCmdType    *cmd = NULL;
  MESH_RollOffTable_V4             *input_table_Q10 = NULL;
  float                             aec_ratio = 0.0;
  chromatix_rolloff_trigger_type   *chromatix_rolloff_trigger = NULL;
  chromatix_parms_type             *chromatix_ptr = NULL;
  boolean                           tintless_low_light_mode = FALSE;
  mesh_rolloff44_tintless_params_t *tintless_params = NULL;
  MESH_RollOff_v4_ConfigParams     *CfgParams = NULL;
  char                              value[PROPERTY_VALUE_MAX];
  boolean                           ret = TRUE;

  if (!isp_sub_module || !mesh_rolloff || !sub_module_output) {
    ISP_ERR("failed: %p %p %p", isp_sub_module, mesh_rolloff,
      sub_module_output);
    return FALSE;
  }

  if (!sub_module_output->stats_params || !sub_module_output->algo_params) {
    ISP_ERR("failed: %p %p", sub_module_output->stats_params,
      sub_module_output->algo_params);
    return FALSE;
  }

  input_table_Q10 = &mesh_rolloff->mesh_rolloff_param.input_table;
  cmd = &mesh_rolloff->mesh_rolloff_cmd;
  CfgParams = &cmd->CfgParams;
  tintless_params = &mesh_rolloff->tintless_params;

  chrComPtr =
    (chromatix_VFE_common_type *)isp_sub_module->chromatix_ptrs.chromatixComPtr;
  if (!chrComPtr) {
    ISP_ERR("failed: chrComPtr %p", chrComPtr);
    return FALSE;
  }

  chromatix_ptr =
    (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    return FALSE;
  }

  chromatix_rolloff_trigger =
    &chromatix_ptr->chromatix_VFE.chromatix_rolloff_trigger;

  if ((isp_sub_module->trigger_update_pending == FALSE) ||
    (isp_sub_module->submod_trigger_enable == FALSE)) {
    /* Tintless rolloff table is not received from tintless algo */
    /* Update tintless mesh config */
    ISP_DBG("Tintless rolloff table is not received from tintless algo");
    mesh_rolloff44_update_tintless_params(mesh_rolloff, tintless_params,
      sub_module_output->algo_params, CfgParams, meshRolloffTableFinal,
      &chromatix_ptr->chromatix_post_processing.
      chromatix_color_tint_correction);
    return TRUE;
  }

  aec_ratio = isp_sub_module_util_get_aec_ratio_lowlight(
    chromatix_rolloff_trigger->control_rolloff,
    &(chromatix_rolloff_trigger->rolloff_lowlight_trigger),
    &mesh_rolloff->aec_update, 1);

  /*
   * determine low light condition for low light ratio adjust
   * only pure low light will go into tintless lowlight mode
   */
  if (aec_ratio < 1.0f) {
    tintless_low_light_mode = TRUE;
  } else {
    tintless_low_light_mode = FALSE;
  }

  if (mesh_rolloff->prev_low_light_mode == TRUE &&
    tintless_low_light_mode == FALSE) {
    /* Reset current_adjust to unity */
    for (i = 0; i < MESH_ROLLOFF_SIZE; i++) {
      tintless_params->current_adjust[i] = 1.0f;
    }
  }

  mesh_rolloff->prev_low_light_mode = tintless_low_light_mode;

  /*
   * use flash mode to decide if its flash on, instead of using flash type
   *  flash type was set to be LED for early flash implementation
   */
  if ((mesh_rolloff->cur_flash_mode != CAM_FLASH_MODE_ON) &&
    (mesh_rolloff->cur_flash_mode != CAM_FLASH_MODE_SINGLE)) {
    if (!meshRolloffTableFinal) {
        /* Add temporal filter */
        if (tintless_low_light_mode == TRUE) {
          for (i = 0; i < MESH_ROLLOFF_SIZE; i++) {
            tintless_params->current_adjust[i] =
              ((((1 - aec_ratio) * tintless_params->lowlight_adjust[i]) +
               aec_ratio) * TINTLESS_TEMPORAL_RATIO) +
              (tintless_params->current_adjust[i] * (1 - TINTLESS_TEMPORAL_RATIO));
          }

          /*
           * Change tintless params based on temporal filter co-efficients
           * to avoid sharp changes in hardware configuration
           */
          mesh_rolloff44_tintless_lowlight_adjust(tintless_params);
        } else {
          tintless_params->adjust_tintless_array = tintless_params->tintless_array;
        }
    } else {
      /* Add temporal filter */
      // TINTLESS_TEMPORAL_RATIO to tunable later.
      for (i = 0; i < tintless_params->tintless_array.mesh_rolloff_table_size; i++) {
        tintless_params->tintless_array.r_gain[i] =
          (tintless_params->tintless_array.r_gain[i] * TINTLESS_TEMPORAL_RATIO) +
          (tintless_params->pre_tintless_array.r_gain[i] * (1 - TINTLESS_TEMPORAL_RATIO));
        tintless_params->pre_tintless_array.r_gain[i]=  tintless_params->tintless_array.r_gain[i];
        tintless_params->tintless_array.gr_gain[i] =
          (tintless_params->tintless_array.gr_gain[i] * TINTLESS_TEMPORAL_RATIO) +
          (tintless_params->pre_tintless_array.gr_gain[i] * (1 - TINTLESS_TEMPORAL_RATIO));
        tintless_params->pre_tintless_array.gr_gain[i]=  tintless_params->tintless_array.gr_gain[i];
        tintless_params->tintless_array.gb_gain[i] =
          (tintless_params->tintless_array.gb_gain[i] * TINTLESS_TEMPORAL_RATIO) +
          (tintless_params->pre_tintless_array.gb_gain[i] * (1 - TINTLESS_TEMPORAL_RATIO));
        tintless_params->pre_tintless_array.gb_gain[i]=  tintless_params->tintless_array.gb_gain[i];
        tintless_params->tintless_array.b_gain[i] =
          (tintless_params->tintless_array.b_gain[i] * TINTLESS_TEMPORAL_RATIO) +
          (tintless_params->pre_tintless_array.b_gain[i] * (1 - TINTLESS_TEMPORAL_RATIO));
        tintless_params->pre_tintless_array.b_gain[i]=  tintless_params->tintless_array.b_gain[i];
      }
      ISP_DBG("input_rolloff_R[0]=%f", meshRolloffTableFinal->TableR[0]);
      ISP_DBG("output_rolloff_R[0]=%f", tintless_params->tintless_array.r_gain[0]);
   }
    /* mesh table to Q10 table*/
    mesh_rolloff44_tintless_set_hw_table(input_table_Q10, tintless_params);

    /*HW packing*/
    mesh_rolloff44_prepare_hw_table(input_table_Q10, cmd);
  } else {
    /*flash implementation*/
    mesh_rolloff44_calc_flash_trigger(isp_sub_module, mesh_rolloff,
      &tintless_params->last_non_flash_tbl, input_table_Q10,
      mesh_rolloff->rolloff_tbls.rolloff_tableset[0]);
#ifdef PRINT_MESH_TABLE
    mesh_rolloff44_print_mesh_table(input_table_Q10);
#endif
    /* after interpolation, its already Q10 table*/
    mesh_rolloff44_prepare_hw_table(input_table_Q10, cmd);
  }

  /* Check whether setprop is enabled to dump configuration */
  if (mesh_rolloff->dump_enabled == FALSE) {
     property_get("persist.vendor.camera.dumpmetadata", value, "0");
     mesh_rolloff->dump_frames = atoi(value);
     mesh_rolloff->dump_enabled = TRUE;
     if(mesh_rolloff->dump_frames > 1)
       mesh_rolloff->dump_frames = 1;
  }

  if (mesh_rolloff->dump_frames) {
    mesh_rolloff44_dump_tintless_array(&tintless_params->tintless_array,
      sub_module_output->frame_id);
    mesh_rolloff->dump_frames--;
  }
  mesh_rolloff44_update_tintless_params(mesh_rolloff, tintless_params,
    sub_module_output->algo_params, CfgParams, meshRolloffTableFinal,
    &chromatix_ptr->chromatix_post_processing.
    chromatix_color_tint_correction);

  ret = mesh_rolloff44_store_hw_update(isp_sub_module, mesh_rolloff);
  if (ret == FALSE) {
    ISP_ERR("failed: mesh_rolloff44_store_hw_update");
  }

  return TRUE;
} /* mesh_rolloff44_tintless_trigger_update */

/** mesh_rolloff44_trigger_update
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  trigger update based on 3A event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean mesh_rolloff44_trigger_update(isp_sub_module_t *isp_sub_module,
  void *data)
{
  boolean                        ret = TRUE;
  mesh_rolloff44_t               *mesh_rolloff = NULL;
  isp_private_event_t           *private_event = NULL;
  isp_sub_module_output_t       *sub_module_output = NULL;
  LensRolloffStruct             *rolloff_config = NULL;
  int                            i, rc = 0;
  MESH_RollOffTable_V4          *meshRolloffTableFinal = NULL;
  MESH_RollOffTable_V4           meshRolloffTableTintless;
  MESH_RollOffTable_V4           meshRolloffTableAEC;
  MESH_RollOffTable_V4           meshRolloffTableInf;
  isp_rolloff_info_t            *mesh_tbls = NULL;
  MESH_RollOff_V4_ConfigCmdType *cmd = NULL;
  isp_meta_entry_t              *mesh_rolloff_dmi_info = NULL;
  uint32_t                      *mesh_rolloff_dmi_tbl = NULL;
  int8_t                         module_enable;
  float                         *shading = NULL;
  boolean                        led_enable = FALSE;
  float                          led_sensitivity = 0.0f;
  int32_t                        led_sensitivityQ10 = 0;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }
  private_event = (isp_private_event_t *)data;
  if (!private_event) {
    ISP_ERR("failed: private_event %p", private_event);
    return FALSE;
  }

  sub_module_output = (isp_sub_module_output_t *)private_event->data;
  if (!sub_module_output) {
    ISP_ERR("failed: output %p", sub_module_output);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  mesh_rolloff = (mesh_rolloff44_t *)isp_sub_module->private_data;
  if (!mesh_rolloff) {
    ISP_ERR("failed: mesh_rolloff %p", mesh_rolloff);
    goto ERROR;
  }

  if (isp_sub_module->manual_ctrls.manual_update &&
      !mesh_rolloff->module_streamon &&
       isp_sub_module->chromatix_module_enable) {
    isp_sub_module->manual_ctrls.manual_update = FALSE;
    if (isp_sub_module->manual_ctrls.cc_mode == CAM_COLOR_CORRECTION_TRANSFORM_MATRIX &&
      ((isp_sub_module->manual_ctrls.wb_mode == CAM_WB_MODE_OFF &&
      isp_sub_module->manual_ctrls.app_ctrl_mode == CAM_CONTROL_AUTO) ||
      isp_sub_module->manual_ctrls.app_ctrl_mode == CAM_CONTROL_OFF))
        isp_sub_module->submod_trigger_enable = FALSE;
    else
      isp_sub_module->submod_trigger_enable = TRUE;

    module_enable = (isp_sub_module->manual_ctrls.lens_shading_mode ==
                     CAM_LENS_SHADING_MODE_OFF) ? FALSE : TRUE;

    if (sub_module_output->meta_dump_params) {
      mesh_rolloff->metadump_enable =
        sub_module_output->metadata_dump_enable;
    }
    if (isp_sub_module->submod_enable != module_enable) {
      isp_sub_module->submod_enable = module_enable;
      sub_module_output->stats_params->
        module_enable_info.reconfig_needed = TRUE;
      sub_module_output->stats_params->module_enable_info.
        submod_enable[isp_sub_module->hw_module_id] = module_enable;
      sub_module_output->stats_params->module_enable_info.
        submod_mask[isp_sub_module->hw_module_id] = 1;

    }
  }

  /* No need to execute tintless algo during manual update request/submod_trigger disabled */
  sub_module_output->algo_params->manual_control_enable =
    (!isp_sub_module->submod_trigger_enable);

  /* Need to send the unity metadata when the module is disabled */
  if (!isp_sub_module->submod_enable ||
      !isp_sub_module->stream_on_count) {
    ISP_DBG("skip trigger update, enabled %d, stream_on_count %d",
      isp_sub_module->submod_enable, isp_sub_module->stream_on_count);
    goto FILL_METADATA;
  }

  if (isp_sub_module->tintless_enabled == TRUE) {
    if (mesh_rolloff->module_streamon == TRUE ||
        isp_sub_module->ae_bracketing_enable == FALSE) {
         if (mesh_rolloff->ext_func_table &&
            mesh_rolloff->ext_func_table->calc_tintless_src_table) {
              sub_module_output->algo_params->tintless_mesh_config.update_algo = 1;
              mesh_rolloff->ext_func_table->calc_tintless_src_table(isp_sub_module,
                    &meshRolloffTableTintless);
              ret = mesh_rolloff44_tintless_trigger_update(isp_sub_module,
                     mesh_rolloff, &meshRolloffTableTintless, sub_module_output);
         }
         else
             ret = mesh_rolloff44_tintless_trigger_update(isp_sub_module,
                                     mesh_rolloff, NULL, sub_module_output);
      if (ret == FALSE)
        ISP_ERR("failed: mesh_rolloff44_tintless_trigger_update");
      isp_sub_module->trigger_update_pending = FALSE;
    } else {
      ret = TRUE;
      ISP_DBG("HDR ON: Skipping trigger update for tintless");
      goto FILL_METADATA;
    }

    if (mesh_rolloff->module_streamon)
      mesh_rolloff->module_streamon = FALSE;
  }
  else if (isp_sub_module->trigger_update_pending == TRUE &&
             isp_sub_module->submod_trigger_enable  == TRUE) {
    ISP_DBG("trigger_update_pending");
    if (mesh_rolloff->module_streamon)
          mesh_rolloff->module_streamon = FALSE;

    meshRolloffTableFinal = &(mesh_rolloff->mesh_rolloff_param.input_table);

    mesh_tbls = mesh_rolloff->rolloff_tbls.rolloff_tableset[0];

   if (mesh_rolloff->ext_func_table &&
    mesh_rolloff->ext_func_table->calc_interpolation) {
     mesh_rolloff->ext_func_table->calc_interpolation(isp_sub_module,
     mesh_rolloff, mesh_tbls, &meshRolloffTableAEC);
   } else {
     ISP_ERR("failed: %p", mesh_rolloff->ext_func_table);
     ret = FALSE;
     goto ERROR;
   }

    if (mesh_rolloff->cur_flash_mode != CAM_FLASH_MODE_OFF) {
      mesh_rolloff44_calc_flash_trigger(isp_sub_module, mesh_rolloff,
        &meshRolloffTableAEC, &meshRolloffTableInf, mesh_tbls);
    } else {
      meshRolloffTableInf = meshRolloffTableAEC;
    }

    /* Interpolate with AF tbls, Rolloff Macro and Infinity tbls*/
    if (mesh_rolloff->rolloff_tbls.rolloff_tableset[1] == NULL) {
      *meshRolloffTableFinal = meshRolloffTableInf;
    } else {
      MESH_RollOffTable_V4 meshRolloffTableMacro;
      float af_ratio = 0.0;
      int af_start, af_end, af_value;

      mesh_tbls = mesh_rolloff->rolloff_tbls.rolloff_tableset[1];

      if (mesh_rolloff->ext_func_table &&
       mesh_rolloff->ext_func_table->calc_interpolation) {
        mesh_rolloff->ext_func_table->calc_interpolation(isp_sub_module,
        mesh_rolloff, mesh_tbls, &meshRolloffTableAEC);
      } else {
        ISP_ERR("failed: %p", mesh_rolloff->ext_func_table);
        ret = FALSE;
        goto ERROR;
      }

      if (mesh_rolloff->cur_flash_mode != CAM_FLASH_MODE_OFF) {
         mesh_rolloff44_calc_flash_trigger(isp_sub_module, mesh_rolloff,
          &meshRolloffTableAEC, &meshRolloffTableMacro, mesh_tbls);
      } else {
        meshRolloffTableMacro = meshRolloffTableAEC;
      }

      af_start = mesh_rolloff->af_infinity;
      af_end = mesh_rolloff->af_macro;
      af_value = mesh_rolloff->lens_current_step;
      af_ratio = 1.0 - isp_sub_module_util_calc_interpolation_weight(af_value,
        af_start, af_end);

    ISP_DBG(" af_start %d af_end %d af_value %d af_ratio %f ", af_start, af_end, af_value, af_ratio);
      mesh_rolloff44_table_interpolate(&meshRolloffTableInf,
        &meshRolloffTableMacro, meshRolloffTableFinal, af_ratio);
    }

    mesh_rolloff44_prepare_hw_table(meshRolloffTableFinal,
      &(mesh_rolloff->mesh_rolloff_cmd));

    ret = mesh_rolloff44_store_hw_update(isp_sub_module, mesh_rolloff);
    if (ret == FALSE) {
      ISP_ERR("failed: mesh_rolloff44_store_hw_update");
      goto ERROR;
    }
    isp_sub_module->trigger_update_pending = FALSE;
  }

  ret = isp_sub_module_util_append_hw_update_list(isp_sub_module,
    sub_module_output);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_append_hw_update_list");
    goto ERROR;
  }

FILL_METADATA:
#ifdef POST_ROLLOFF_INFO
   /* save the full roi cfg params to BE CFG before calculating for split stripe */
   rolloff_config = &(sub_module_output->stats_params->be_info.rolloff_info);
   rolloff_config->scale_cubic = mesh_rolloff->mesh_rolloff_cmd.be_cfg_params.scale_cubic;
   rolloff_config->deltah = mesh_rolloff->mesh_rolloff_cmd.be_cfg_params.deltah;
   rolloff_config->deltav = mesh_rolloff->mesh_rolloff_cmd.be_cfg_params.deltav;
   rolloff_config->subgridh = mesh_rolloff->mesh_rolloff_cmd.be_cfg_params.subgridh;
   rolloff_config->subgridv = mesh_rolloff->mesh_rolloff_cmd.be_cfg_params.subgridv;
   rolloff_config->nh =  MESH_ROLLOFF_HORIZONTAL_GRIDS_MAX;
   rolloff_config->nv =  MESH_ROLLOFF_VERTICAL_GRIDS_MAX;

   for(i = 0; i < MESH_ROLL_OFF_V4_TABLE_SIZE; i++) {
     rolloff_config->r_gainf[i] =
       mesh_rolloff->applied_table.TableR[i];
     rolloff_config->gr_gainf[i] =
       mesh_rolloff->applied_table.TableGr[i];
     rolloff_config->b_gainf[i] =
       mesh_rolloff->applied_table.TableB[i];
     rolloff_config->gb_gainf[i] =
       mesh_rolloff->applied_table.TableGb[i];
   }
#endif
  if (sub_module_output->meta_dump_params &&
    sub_module_output->metadata_dump_enable == 1) {
    /*fill in aec parm*/
    sub_module_output->meta_dump_params->frame_meta.lux_idx =
      mesh_rolloff->aec_update.lux_idx;

      /*fill in DMI info*/
      mesh_rolloff_dmi_info = &sub_module_output->
        meta_dump_params->meta_entry[ISP_META_ROLLOFF_TBL];
      /*dmi table length*/
      mesh_rolloff_dmi_info->len =
        sizeof(uint32_t) * MESH_ROLL_OFF_V4_TABLE_SIZE * 2;
      /*dmi type*/
      mesh_rolloff_dmi_info->dump_type  = ISP_META_ROLLOFF_TBL;
      mesh_rolloff_dmi_info->start_addr = 0;
      sub_module_output->meta_dump_params->frame_meta.num_entry++;

#ifdef DYNAMIC_DMI_ALLOC
      mesh_rolloff_dmi_tbl =
        (uint32_t *)CAM_MALLOC(mesh_rolloff_dmi_info->len);
      if (!mesh_rolloff_dmi_tbl) {
        ISP_ERR("failed: %p", mesh_rolloff_dmi_tbl);
        ret = FALSE;
        goto ERROR;
      }
#endif
      memcpy(mesh_rolloff_dmi_info->isp_meta_dump,
        &mesh_rolloff->applied_hw_table_gr_r, mesh_rolloff_dmi_info->len/2);
      memcpy(&mesh_rolloff_dmi_info->isp_meta_dump[mesh_rolloff_dmi_info->len/2],
        &mesh_rolloff->applied_hw_table_gb_b, mesh_rolloff_dmi_info->len/2);
      //mesh_rolloff_dmi_info->hw_dmi_tbl = mesh_rolloff_dmi_tbl;
      if (mesh_rolloff->cur_flash_mode != CAM_FLASH_MODE_OFF) {
        led_enable = TRUE;
        if (mesh_rolloff->aec_update.flash_sensitivity.high > 0) {
          led_sensitivity = mesh_rolloff->aec_update.flash_sensitivity.off /
            mesh_rolloff->aec_update.flash_sensitivity.high;
        } else {
          ISP_ERR("Invalid high sensitivity %f",
            mesh_rolloff->aec_update.flash_sensitivity.high);
        }
      }
      sub_module_output->meta_dump_params->frame_meta.addlinfo.reserved[3] =
        led_enable;
      led_sensitivityQ10 = FLOAT_TO_Q(10, led_sensitivity);
      sub_module_output->meta_dump_params->frame_meta.addlinfo.reserved[4] =
        led_sensitivityQ10;
      sub_module_output->meta_dump_params->frame_meta.addlinfo.reserved[5] =
        (isp_sub_module->tintless_enabled << 2) |
        (mesh_rolloff->ispif_out_info.is_split << 1);
    }

  if (isp_sub_module->vfe_diag_enable) {
    ret = mesh_rolloff44_fill_vfe_diag_data(mesh_rolloff,
      isp_sub_module, sub_module_output);
    if (ret == FALSE) {
      ISP_ERR("failed: mesh_rolloff44_fill_vfe_diag_data");
    }
  }

  if (sub_module_output->frame_meta) {
    sub_module_output->frame_meta->app_ctrl_mode =
      isp_sub_module->manual_ctrls.app_ctrl_mode;
    sub_module_output->frame_meta->lens_shading_mode =
      isp_sub_module->manual_ctrls.lens_shading_mode;
    sub_module_output->frame_meta->lens_shading_map_mode =
      isp_sub_module->manual_ctrls.lens_shading_map_mode;
    sub_module_output->frame_meta->tintless_enable =
      isp_sub_module->tintless_enabled;

    shading = sub_module_output->frame_meta->rolloff_tbl.lens_shading;

    /* Save the gains for the R/Gr/B/Gb, to populate it to metadata*/
    if (!isp_sub_module->submod_enable) {
      /* Set unity values for report in metadata, as the module is disabled */
      for (i= 0; i < MESH_ROLL_OFF_V4_TABLE_SIZE * 4; i++)
      *shading++ = 1.0;
    } else {
      for (i= 0; i < MESH_ROLL_OFF_V4_TABLE_SIZE; i++){
        *shading++ = mesh_rolloff->applied_table.TableR[i];
        if (mesh_rolloff->g_even_is_Gr) {
          *shading++ = mesh_rolloff->applied_table.TableGr[i];
          *shading++ = mesh_rolloff->applied_table.TableGb[i];
        } else {
          *shading++ = mesh_rolloff->applied_table.TableGb[i];
          *shading++ = mesh_rolloff->applied_table.TableGr[i];
        }
        *shading++ = mesh_rolloff->applied_table.TableB[i];
      }
    }
  }

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;
} /* mesh_rolloff44_trigger_update */

/** mesh_rolloff_config
 *
 *  @isp_sub_module: isp sub module handle
 *  @mesh_mod: mesh_rolloff handle
 *
 *  config default params
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean mesh_rolloff44_config(isp_sub_module_t *isp_sub_module,
  mesh_rolloff44_t *mesh_mod)
{
  boolean                        ret = TRUE;
  uint32_t                       i = 0;
  chromatix_parms_type          *chrPtr = NULL;
  enum ISP_START_PIXEL_PATTERN   pix_pattern;


  if (!isp_sub_module || !mesh_mod) {
    ISP_ERR("failed: %p %p", isp_sub_module, mesh_mod);
    return FALSE;
  }

  pix_pattern = isp_sub_module_fmt_to_pix_pattern(
    mesh_mod->sensor_out_info.fmt);
  switch (pix_pattern) {
    /* bayer patterns */
  case ISP_BAYER_GRGRGR:
  case ISP_BAYER_RGRGRG:
    /* row 0(Even) Gr, row 1(odd) Gb*/
    mesh_mod->g_even_is_Gr = 1;
    break;

  case ISP_BAYER_GBGBGB:
  case ISP_BAYER_BGBGBG:
    /* row 0(Even) Gb, row 1(odd) Gr*/
    mesh_mod->g_even_is_Gr = 0;
    break;

  default:
    ISP_ERR("Error ISP input not configured!!!\n");
    ret = FALSE;
    return ret;
  }

  /* Get all rolloff tables from chromatix and normalize them */
  ret = mesh_rolloff44_prepare_tables(isp_sub_module, mesh_mod);
  if (ret == FALSE) {
    ISP_ERR("rolloff prepare initial table failed");
    return ret;
  }

  /*rollff initial table from chromatix normalized TL84 table */
  mesh_mod->mesh_rolloff_param.input_table =
    mesh_mod->rolloff_tbls.rolloff_tableset[0]->left[ISP_ROLLOFF_TL84_LIGHT];

  if (mesh_mod->af_rolloff_info_valid == TRUE) {
    mesh_mod->af_macro = mesh_mod->af_rolloff_info.af_macro;
    mesh_mod->af_infinity = mesh_mod->af_rolloff_info.af_infinity;
  } else {
    mesh_mod->af_macro = 0;
    mesh_mod->af_infinity = 0;
  }

  return ret;
} /* mesh_rolloff44_config */

/** mesh_rolloff44_tintless_config
 *
 *  @isp_sub_module: isp sub module handle
 *  @mesh_mod: mesh_rolloff handle
 *
 *  config default params for tintless
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean mesh_rolloff44_tintless_config(isp_sub_module_t *isp_sub_module,
  mesh_rolloff44_t *mesh_mod)
{
  boolean                           ret = TRUE;
  uint32_t                          i = 0;
  chromatix_parms_type             *chrPtr = NULL;
  mesh_rolloff_array_type          *normal_table = NULL;
  mesh_rolloff_array_type          *low_light_table = NULL;
  chromatix_VFE_common_type        *chrComPtr = NULL;
  chromatix_rolloff_type           *chromatix_rolloff = NULL;
  mesh_rolloff44_tintless_params_t *tintless_params = NULL;

  if (!isp_sub_module || !mesh_mod) {
    ISP_ERR("failed: %p %p", isp_sub_module, mesh_mod);
    return FALSE;
  }

  tintless_params = &mesh_mod->tintless_params;

  chrComPtr = isp_sub_module->chromatix_ptrs.chromatixComPtr;
  if (!chrComPtr) {
    ISP_ERR("failed: chrComPtr %p", chrComPtr);
    return FALSE;
  }

  chromatix_rolloff = &chrComPtr->chromatix_rolloff;

  /* fixed table */
  normal_table =
    &chromatix_rolloff->chromatix_mesh_rolloff_table[ROLLOFF_TL84_LIGHT];

  /* low light table */
  low_light_table = &chromatix_rolloff->
    chromatix_mesh_rolloff_table_lowlight[ROLLOFF_TL84_LIGHT];

  for (i = 0; i < MESH_ROLLOFF_SIZE; i++) {
     if (normal_table->gr_gain[i] != 0) {
       tintless_params->lowlight_adjust[i] =
         low_light_table->gr_gain[i] / normal_table->gr_gain[i];
     } else {
       ISP_ERR("normal light ratio = 0!");
       return FALSE;
     }
  }

  for (i = 0; i < MESH_ROLLOFF_SIZE; i++) {
    tintless_params->current_adjust[i] = 1.0;
  }

  return ret;
}

/** mesh_rolloff44_get_abf_table_from_cct:
 *
 *  @mesh_table: pointer to table array in which to copy the rolloff table
 *  @isp_sub_module: isp sub module handle
 *  @mesh_rolloff: private module data
 *
 *  Handle AEC update event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean mesh_rolloff44_get_abf_table_from_cct(float *mesh_table,
  isp_sub_module_t *isp_sub_module, mesh_rolloff44_t *mesh_rolloff)
{
  cct_trigger_info           trigger_info;
  chromatix_parms_type      *chrPtr = NULL;
  chromatix_VFE_common_type *chrComPtr = NULL;
  chromatix_rolloff_type    *chromatix_rolloff = NULL;
  float                      color_temp = 0;


  chrPtr = isp_sub_module->chromatix_ptrs.chromatixPtr;
  chrComPtr = isp_sub_module->chromatix_ptrs.chromatixComPtr;

  if (!chrPtr || !chrComPtr) {
    ISP_ERR("failed: chromatix_ptr %p chromatix_common %p", chrPtr, chrComPtr);
    return FALSE;
  }

  chromatix_rolloff = &chrComPtr->chromatix_rolloff;

  trigger_info.mired_color_temp = (float)1000000.0f /
    (float)(mesh_rolloff->cur_mired_color_temp);

  color_temp = trigger_info.mired_color_temp;
  if (color_temp < chromatix_rolloff->rolloff_A_trigger.CCT_start) {
    memcpy(mesh_table, mesh_rolloff->applied_table.TableB,
      sizeof(mesh_rolloff->applied_table.TableB));
  } else if (color_temp <= chromatix_rolloff->rolloff_D65_trigger.CCT_start) {
    memcpy(mesh_table, mesh_rolloff->applied_table.TableGr,
      sizeof(mesh_rolloff->applied_table.TableGr));
  } else {
    memcpy(mesh_table, mesh_rolloff->applied_table.TableR,
      sizeof(mesh_rolloff->applied_table.TableR));
   }

  return TRUE;
}

/** mesh_rolloff44_stats_aec_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to stats_update_t
 *
 *  Handle AEC update event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean mesh_rolloff44_stats_aec_update(isp_sub_module_t *isp_sub_module,
  void *data)
{
  stats_update_t            *stats_update = NULL;
  mesh_rolloff44_t           *mesh_rolloff = NULL;
  aec_update_t              *aec_update = NULL;
  int32_t filter_off = 0;
  char value[PROPERTY_VALUE_MAX];

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  mesh_rolloff = (mesh_rolloff44_t *)isp_sub_module->private_data;
  if (!mesh_rolloff) {
    ISP_ERR("failed: mesh_rolloff %p", mesh_rolloff);
    return FALSE;
  }

  stats_update = (stats_update_t *)data;
  if (!stats_update) {
    ISP_ERR("failed: stats_update %p", stats_update);
    return FALSE;
  }

  aec_update = &stats_update->aec_update;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (!F_EQUAL(mesh_rolloff->aec_update.lux_idx, aec_update->lux_idx) ||
    !F_EQUAL(mesh_rolloff->aec_update.sensor_gain,
     aec_update->sensor_gain)) {
    isp_sub_module->trigger_update_pending = TRUE;
    mesh_rolloff->aec_update = *aec_update;
  }

  property_get("persist.vendor.camera.rolloff.filter", value, "0");
  filter_off = atoi(value);

  /*Filtering only when non LED flash
     * and when meta dump is not enable*/
  if (mesh_rolloff->cur_flash_mode == CAM_FLASH_MODE_OFF &&
    mesh_rolloff->metadump_enable != 1 && !filter_off) {
    isp_sub_module_util_temporal_lux_calc(&(mesh_rolloff->lux_filter),
      &mesh_rolloff->aec_update.lux_idx); //LPF lux_index
  }

  if (stats_update->aec_update.dual_led_setting.is_valid) {
    mesh_rolloff->dual_led_setting.led1_low_setting  = stats_update->aec_update.dual_led_setting.led1_low_setting;
    mesh_rolloff->dual_led_setting.led2_low_setting  = stats_update->aec_update.dual_led_setting.led2_low_setting;
    mesh_rolloff->dual_led_setting.led1_high_setting = stats_update->aec_update.dual_led_setting.led1_high_setting;
    mesh_rolloff->dual_led_setting.led2_high_setting = stats_update->aec_update.dual_led_setting.led2_high_setting;
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return FALSE;
} /* mesh_rolloff44_stats_aec_update */

/** mesh_rolloff44_stats_awb_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to stats_update_t
 *
 *  Handle AWB update event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean mesh_rolloff44_stats_awb_update(isp_sub_module_t *isp_sub_module,
  void *data)
{
  stats_update_t          *stats_update = NULL;
  mesh_rolloff44_t         *mesh_rolloff = NULL;
  float                    new_mired_color_temp = 0;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  mesh_rolloff = (mesh_rolloff44_t *)isp_sub_module->private_data;
  if (!mesh_rolloff) {
    ISP_ERR("failed: mesh_rolloff %p", mesh_rolloff);
    return FALSE;
  }

  stats_update = (stats_update_t *)data;
  if (!stats_update) {
    ISP_ERR("failed: stats_update %p", stats_update);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  new_mired_color_temp = (float)1000000.0f /
    (stats_update->awb_update.color_temp);
  if ((isp_sub_module->trigger_update_pending == FALSE) &&
      (mesh_rolloff->cur_mired_color_temp != new_mired_color_temp)) {
    isp_sub_module->trigger_update_pending = TRUE;
  }

  mesh_rolloff->cur_mired_color_temp = new_mired_color_temp;
  if (stats_update->awb_update.dual_led_setting.is_valid) {
    mesh_rolloff->dual_led_setting.led1_low_setting  = stats_update->awb_update.dual_led_setting.led1_low_setting;
    mesh_rolloff->dual_led_setting.led2_low_setting  = stats_update->awb_update.dual_led_setting.led2_low_setting;
    mesh_rolloff->dual_led_setting.led1_high_setting = stats_update->awb_update.dual_led_setting.led1_high_setting;
    mesh_rolloff->dual_led_setting.led2_high_setting = stats_update->awb_update.dual_led_setting.led2_high_setting;
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;
} /* mesh_rolloff44_stats_awb_update */

/** mesh_rolloff44_stats_af_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to stats_update_t
 *
 *  Handle AF update event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean mesh_rolloff44_stats_af_update(isp_sub_module_t *isp_sub_module,
  void *data)
{
  stats_update_t            *stats_update = NULL;
  mesh_rolloff44_t           *mesh_rolloff = NULL;
  af_update_t              *af_update = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  mesh_rolloff = (mesh_rolloff44_t *)isp_sub_module->private_data;
  if (!mesh_rolloff) {
    ISP_ERR("failed: mesh_rolloff %p", mesh_rolloff);
    return FALSE;
  }

  stats_update = (stats_update_t *)data;
  if (!stats_update) {
    ISP_ERR("failed: stats_update %p", stats_update);
    return FALSE;
  }

  af_update = &stats_update->af_update;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  mesh_rolloff->af_update = *af_update;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;

} /* mesh_rolloff44_stats_af_update */

/** mesh_rolloff44_set_flash_mode:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to cam_flash_mode_t
 *
 *  Handle set flash mode event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean mesh_rolloff44_set_flash_mode(isp_sub_module_t *isp_sub_module,
  void *data)
{
  mesh_rolloff44_t         *mesh_rolloff = NULL;
  cam_flash_mode_t        *flash_mode = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  mesh_rolloff = (mesh_rolloff44_t *)isp_sub_module->private_data;
  if (!mesh_rolloff) {
    ISP_ERR("failed: mesh_rolloff %p", mesh_rolloff);
    return FALSE;
  }

  flash_mode = (cam_flash_mode_t *)data;
  if (!flash_mode) {
    ISP_ERR("failed: flash_mode %p", flash_mode);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if ((isp_sub_module->trigger_update_pending == FALSE) &&
      (mesh_rolloff->cur_flash_mode != *flash_mode)) {
    isp_sub_module->trigger_update_pending = TRUE;
  }

  ISP_DBG("<flash_dbg> get new flash mode = %d", *flash_mode);

  if (isp_sub_module->tintless_enabled == TRUE &&
    isp_sub_module->stream_on_count) {
    mesh_rolloff->tintless_params.tintless_array_valid = TRUE;
  }

  if ((isp_sub_module->tintless_enabled == TRUE) &&
    (mesh_rolloff->cur_flash_mode != CAM_FLASH_MODE_TORCH) &&
    (*flash_mode == CAM_FLASH_MODE_TORCH)) {
    memcpy(&mesh_rolloff->tintless_params.last_non_flash_tbl,
      &mesh_rolloff->mesh_rolloff_param.input_table,
      sizeof(MESH_RollOffTable_V4));
#ifdef PRINT_MESH_TABLE
    mesh_rolloff44_print_mesh_table(
      &mesh_rolloff->tintless_params.last_non_flash_tbl);
#endif
  }

  mesh_rolloff->cur_flash_mode = *flash_mode;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
} /* mesh_rolloff44_stats_awb_update */

/** mesh_rolloff44_set_chromatix_ptr:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  Store chromatix ptr and make initial configuration
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean mesh_rolloff44_set_chromatix_ptr(isp_sub_module_t *isp_sub_module,
  void *data)
{
  boolean                ret = TRUE;
  mesh_rolloff44_t       *mesh_rolloff = NULL;
  modulesChromatix_t    *chromatix_ptrs = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  chromatix_ptrs = (modulesChromatix_t *)data;
  if (!chromatix_ptrs) {
    ISP_ERR("failed: chromatix_ptrs %p", chromatix_ptrs);
    return FALSE;
  }

  mesh_rolloff = (mesh_rolloff44_t *)isp_sub_module->private_data;
  if (!mesh_rolloff) {
    ISP_ERR("failed: mesh_rolloff %p", mesh_rolloff);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  isp_sub_module->chromatix_ptrs = *chromatix_ptrs;

  isp_sub_module->trigger_update_pending = TRUE;

  ret = isp_sub_module_util_configure_from_chromatix_bit(isp_sub_module);
  if (ret == FALSE) {
    ISP_ERR("failed: updating module enable bit for hw %d",
      isp_sub_module->hw_module_id);
  }

  ret = mesh_rolloff44_config(isp_sub_module, mesh_rolloff);
  if (ret == FALSE) {
    ISP_ERR("failed: mesh_rolloff44_config ret %d", ret);
    goto ERROR;
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;
} /* mesh_rolloff44_set_chromatix_ptr */

/** mesh_rolloff44_set_stream_config:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  Store chromatix ptr and make initial configuration
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean mesh_rolloff44_set_stream_config(isp_sub_module_t *isp_sub_module,
  void *data)
{
  boolean                ret = TRUE;
  mesh_rolloff44_t       *mesh_rolloff = NULL;
  sensor_out_info_t     *sensor_out_info = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  sensor_out_info = (sensor_out_info_t *)data;

  mesh_rolloff = (mesh_rolloff44_t *)isp_sub_module->private_data;
  if (!mesh_rolloff) {
    ISP_ERR("failed: mesh_rolloff %p", mesh_rolloff);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  mesh_rolloff->sensor_out_info = *sensor_out_info;

  isp_sub_module->trigger_update_pending = TRUE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;
} /* mesh_rolloff44_set_stream_config */

/** mesh_rolloff44_set_stream_config_overwrite:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  Store chromatix ptr and make initial configuration
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean mesh_rolloff44_set_stream_config_overwrite(isp_sub_module_t *isp_sub_module)
{
  boolean                ret = TRUE;
  mesh_rolloff44_t       *mesh_rolloff = NULL;

  if (!isp_sub_module) {
    ISP_ERR("failed: %p", isp_sub_module);
    return FALSE;
  }

  mesh_rolloff = (mesh_rolloff44_t *)isp_sub_module->private_data;
  if (!mesh_rolloff) {
    ISP_ERR("failed: mesh_rolloff %p", mesh_rolloff);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  mesh_rolloff44_update_hw_table(mesh_rolloff, &mesh_rolloff->mesh_rolloff_cmd,
    &mesh_rolloff->mesh_rolloff_param.input_table);

  if (isp_sub_module->ae_bracketing_enable == FALSE &&
    isp_sub_module->tintless_enabled == TRUE) {
   ret = mesh_rolloff44_tintless_config(isp_sub_module, mesh_rolloff);
   mesh_rolloff->tintless_params.tintless_array_valid = TRUE;
   if (ret == FALSE) {
    ISP_ERR("failed: mesh_rolloff44_config ret %d", ret);
    goto ERROR;
   }
  }

  isp_sub_module->trigger_update_pending = TRUE;
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;
} /* mesh_rolloff40_set_chromatix_ptr */

/** mesh_rolloff44_set_af_rolloff_params:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  Handle AF rolloff params event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean mesh_rolloff44_set_af_rolloff_params(isp_sub_module_t *isp_sub_module,
  void *data)
{
  boolean            ret = TRUE;
  mesh_rolloff44_t   *mesh_rolloff = NULL;
  af_rolloff_info_t *af_rolloff_info = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  af_rolloff_info = (af_rolloff_info_t *)data;

  mesh_rolloff = (mesh_rolloff44_t *)isp_sub_module->private_data;
  if (!mesh_rolloff) {
    ISP_ERR("failed: mesh_rolloff %p", mesh_rolloff);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  mesh_rolloff->af_rolloff_info = *af_rolloff_info;
//  mesh_rolloff->af_rolloff_info.rolloff_tables_macro = NULL;
  memcpy(&mesh_rolloff->af_rolloff_chromatix,
    af_rolloff_info->rolloff_tables_macro,
    sizeof(mesh_rolloff->af_rolloff_chromatix));
  mesh_rolloff->af_rolloff_info_valid = TRUE;
  isp_sub_module->trigger_update_pending = TRUE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;
} /* mesh_rolloff44_set_af_rolloff_params */

/** mesh_rolloff44_sensor_lens_position_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  Handle sensor lens position update
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean mesh_rolloff44_sensor_lens_position_update(
  isp_sub_module_t *isp_sub_module, void *data)
{
  boolean                     ret = TRUE;
  mesh_rolloff44_t            *mesh_rolloff = NULL;
  lens_position_update_isp_t *lens_update = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  lens_update = (lens_position_update_isp_t *)data;

  mesh_rolloff = (mesh_rolloff44_t *)isp_sub_module->private_data;
  if (!mesh_rolloff) {
    ISP_ERR("failed: mesh_rolloff %p", mesh_rolloff);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if ((isp_sub_module->trigger_update_pending == FALSE) &&
    (mesh_rolloff->lens_current_step != lens_update->current_step)) {
    isp_sub_module->trigger_update_pending = TRUE;
  }

  mesh_rolloff->lens_current_step = lens_update->current_step;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;
} /* mesh_rolloff44_sensor_lens_position_update */

/** mesh_rolloff44_set_tintless_table:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  Copy tintless rolloff table from tintless algo in internal
 *  data structure
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean mesh_rolloff44_set_tintless_table(isp_sub_module_t *isp_sub_module,
  void *data)
{
  mesh_rolloff44_t            *mesh_rolloff = NULL;
  isp_saved_tintless_params_t *saved_tintless_params = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  mesh_rolloff = (mesh_rolloff44_t *)isp_sub_module->private_data;
  if (!mesh_rolloff) {
    ISP_ERR("failed: mesh_rolloff %p", mesh_rolloff);
    return FALSE;
  }

  saved_tintless_params = (isp_saved_tintless_params_t *)data;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  /* Copy rolloff table output from tintless algo to internal data structure */
  mesh_rolloff->tintless_params.tintless_array_valid = TRUE;
  mesh_rolloff->tintless_params.tintless_array =
    saved_tintless_params->tintless_array_param;

  /*
   * Set trigger update pending to TRUE so that new tintless table will be
   * applied to hardware in next trigger update
   */
  isp_sub_module->trigger_update_pending = TRUE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;
}

/** mesh_rolloff44_set_parm_tintless:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: event handle
 *
 *  Enable / Disable tintless
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean mesh_rolloff44_set_parm_tintless(isp_sub_module_t *isp_sub_module,
  void *data)
{
  boolean                        ret = TRUE;
  mct_event_control_parm_t      *set_param = NULL;
  int32_t                       *value = NULL;
  mesh_rolloff44_t              *mesh_rolloff = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: isp_sub_module %p data %p", isp_sub_module, data);
    return FALSE;
  }

  set_param = (mct_event_control_parm_t *)data;
  if (!set_param) {
    ISP_ERR("failed: event_control %p", set_param);
    return FALSE;
  }

  if (set_param->type != CAM_INTF_PARM_TINTLESS) {
    ISP_ERR("failed: invalid set parm type %d expected %d", set_param->type,
      CAM_INTF_PARM_TINTLESS);
    return FALSE;
  }

  value = (int32_t *)set_param->parm_data;
  if (!value) {
    ISP_ERR("failed: set_param->parm_data NULL");
    return FALSE;
  }

  ISP_DBG("<tintless_dbg> %d", *value);
  if (*value == 0) {
    isp_sub_module->tintless_enabled = FALSE;
  } else {
    isp_sub_module->tintless_enabled = TRUE;
  }

  mesh_rolloff = (mesh_rolloff44_t *)isp_sub_module->private_data;
  if (!mesh_rolloff) {
    ISP_ERR("failed: mesh_rolloff %p", mesh_rolloff);
    return FALSE;
  }

  if (isp_sub_module->tintless_enabled == TRUE) {
    ret = mesh_rolloff44_tintless_config(isp_sub_module, mesh_rolloff);
    if (ret == FALSE) {
      ISP_ERR("failed: mesh_rolloff44_config ret %d", ret);
    }
  }

  return ret;
} /* mesh_rolloff44_set_parm_tintless */

/** mesh_rolloff44_set_parm_hdr:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: event handle
 *
 *  AE bracketing enable/disable
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean mesh_rolloff44_set_parm_hdr(isp_sub_module_t *isp_sub_module,
  mct_event_t *event)
{
  mct_event_control_parm_t      *param = NULL;
  cam_exp_bracketing_t          *paebracketing = NULL;

  if (!isp_sub_module || !event) {
    ISP_ERR("failed: isp_sub_module %p data %p", isp_sub_module, event);
    return FALSE;
  }
  param = event->u.ctrl_event.control_event_data;

  RETURN_IF_NULL(param);
  RETURN_IF_NULL(param->parm_data);

  paebracketing = (cam_exp_bracketing_t *)(param->parm_data);

  if (paebracketing->mode == CAM_EXP_BRACKETING_ON)
    isp_sub_module->ae_bracketing_enable = TRUE;
  else
    isp_sub_module->ae_bracketing_enable = FALSE;

  ISP_DBG("SET HDR:Enable bracketing=%d",isp_sub_module->ae_bracketing_enable);

  return TRUE;
} /* mesh_rolloff44_set_parm_hdr */


/** mesh_rolloff44_streamon:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: control event data
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean mesh_rolloff44_streamon(isp_sub_module_t *isp_sub_module, void *data)
{
  boolean                    ret = TRUE;
  mesh_rolloff44_t          *mesh_rolloff = NULL;
  isp_rolloff_info_t        *mesh_tbls = NULL;
  int32_t i = 0;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  mesh_rolloff = (mesh_rolloff44_t *)isp_sub_module->private_data;
  if (!mesh_rolloff) {
    ISP_ERR("failed: mesh_rolloff %p", mesh_rolloff);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (isp_sub_module->stream_on_count++) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  ISP_DBG("alloc");
  isp_sub_module->trigger_update_pending = TRUE;

  mesh_rolloff44_table_debug(&mesh_rolloff->mesh_rolloff_param.input_table);
  mesh_rolloff44_update_hw_table(mesh_rolloff, &mesh_rolloff->mesh_rolloff_cmd,
    &mesh_rolloff->mesh_rolloff_param.input_table);

  if (isp_sub_module->ae_bracketing_enable == FALSE &&
    isp_sub_module->tintless_enabled == TRUE) {
    ret = mesh_rolloff44_tintless_config(isp_sub_module, mesh_rolloff);
    if (ret == FALSE) {
      ISP_ERR("failed: mesh_rolloff44_config ret %d", ret);
      goto ERROR;
    }

    mesh_tbls = mesh_rolloff->rolloff_tbls.rolloff_tableset[0];
    if (!mesh_tbls) {
      ISP_ERR("failed: mesh_tbls %p", mesh_tbls);
      goto ERROR;
    }

    /* Initialize the tintless table with normalized table */
    if (mesh_rolloff->tintless_params.tintless_array_valid == FALSE) {
        mesh_rolloff->tintless_params.tintless_array_valid = TRUE;

        for (i = 0; i < MESH_ROLLOFF_SIZE; i++) {
          mesh_rolloff->tintless_params.tintless_array.r_gain[i] =
          mesh_rolloff->tintless_params.pre_tintless_array.r_gain[i] =
            mesh_tbls->left[ISP_ROLLOFF_TL84_LIGHT].TableR[i];

          mesh_rolloff->tintless_params.tintless_array.gr_gain[i] =
          mesh_rolloff->tintless_params.pre_tintless_array.gr_gain[i] =
            mesh_tbls->left[ISP_ROLLOFF_TL84_LIGHT].TableGr[i];

          mesh_rolloff->tintless_params.tintless_array.gb_gain[i] =
          mesh_rolloff->tintless_params.pre_tintless_array.gb_gain[i] =
            mesh_tbls->left[ISP_ROLLOFF_TL84_LIGHT].TableGb[i];

          mesh_rolloff->tintless_params.tintless_array.b_gain[i] =
          mesh_rolloff->tintless_params.pre_tintless_array.b_gain[i] =
            mesh_tbls->left[ISP_ROLLOFF_TL84_LIGHT].TableB[i];

        }
    }
    mesh_rolloff->tintless_params.tintless_array.
          mesh_rolloff_table_size = MESH_ROLLOFF_SIZE;
    mesh_rolloff->tintless_params.adjust_tintless_array.
          mesh_rolloff_table_size = MESH_ROLLOFF_SIZE;
  }
  mesh_rolloff->module_streamon = TRUE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;
} /* mesh_rolloff44_streamon */

/** mesh_rolloff44_streamoff:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: control event data
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean mesh_rolloff44_streamoff(isp_sub_module_t *isp_sub_module, void *data)
{
  mesh_rolloff44_t      *mesh_rolloff = NULL;
  rolloff_lens_position  pos = 0;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  mesh_rolloff = (mesh_rolloff44_t *)isp_sub_module->private_data;
  if (!mesh_rolloff) {
    ISP_ERR("failed: mesh_rolloff %p", mesh_rolloff);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (--isp_sub_module->stream_on_count) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  isp_sub_module->trigger_update_pending = FALSE;

  for (pos = 0; pos < ISP_ROLLOFF_LENS_POSITION_MAX; pos++) {
    free(mesh_rolloff->rolloff_tbls.rolloff_tableset[pos]);
    free(mesh_rolloff->rolloff_calibration_table.rolloff_tableset[pos]);
  }

  memset(mesh_rolloff, 0, sizeof(*mesh_rolloff));

  isp_sub_module->manual_ctrls.cc_mode = CAM_COLOR_CORRECTION_FAST;
  isp_sub_module->manual_ctrls.app_ctrl_mode = CAM_CONTROL_USE_SCENE_MODE;
  isp_sub_module->manual_ctrls.wb_mode = CAM_WB_MODE_AUTO;
  isp_sub_module->manual_ctrls.lens_shading_mode = CAM_LENS_SHADING_MODE_FAST;
  isp_sub_module->manual_ctrls.lens_shading_map_mode = CAM_LENS_SHADING_MAP_MODE_OFF;

  FILL_FUNC_TABLE(mesh_rolloff); /* Keep func ptr table*/
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
} /* mesh_rolloff44_streamoff */

/** mesh_rolloff44_query_cap:
 *
 *    @module: mct module instance
 *    @query_buf: query caps buffer handle
 *
 *  @sessionid: session id
 *  Returns TRUE
 *
 **/
boolean mesh_rolloff44_query_cap(mct_module_t *module,
  void *query_buf)
{
  mct_pipeline_cap_t *cap_buf;
  mct_pipeline_isp_cap_t *mod_cap = NULL;

  if (!module || !query_buf) {
    ISP_ERR("failed: %p %p", module, query_buf);
    return FALSE;
  }

  cap_buf = (mct_pipeline_cap_t *)query_buf;
  mod_cap = &cap_buf->isp_cap;
  mod_cap->lens_shading_map_size.width = HW_MESH_ROLL_NUM_COL;
  mod_cap->lens_shading_map_size.height = HW_MESH_ROLL_NUM_ROW;
  return TRUE;
}

/** mesh_rolloff44_request_stats_type:
 *
 *  @isp_sub_module: isp_sub_module handle
 *  @data: payload of get_stats_type event
 *
 *  Enable BE / HDR BE stats type
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean mesh_rolloff44_request_stats_type(isp_sub_module_t *isp_sub_module,
  void *data)
{
  mct_event_request_stats_type *stats_type = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  if ((isp_sub_module->submod_enable == FALSE) ||
    (isp_sub_module->tintless_enabled == FALSE)) {
    ISP_HIGH("%d %d", isp_sub_module->submod_enable,
      isp_sub_module->tintless_enabled);
    return TRUE;
  }

  stats_type = (mct_event_request_stats_type *)data;

  if (stats_type->supported_stats_mask & (1 << MSM_ISP_STATS_BE)) {
    stats_type->enable_stats_mask |= (1 << MSM_ISP_STATS_BE);
    /*Enable Parse Mask to use it for Tintless algo*/
    stats_type->enable_stats_parse_mask |= (1 << MSM_ISP_STATS_BE);
  } else if (stats_type->supported_stats_mask & (1 << MSM_ISP_STATS_HDR_BE)) {
    stats_type->enable_stats_mask |= (1 << MSM_ISP_STATS_HDR_BE);
    /*Enable Parse Mask to use it for Tintless algo*/
    stats_type->enable_stats_parse_mask |= (1 << MSM_ISP_STATS_HDR_BE);
  } else {
    ISP_ERR("failed: be / hdr be not supported supported mask %x",
      stats_type->supported_stats_mask);
    return FALSE;
  }
  ISP_DBG("enable_stats_mask %x", stats_type->enable_stats_mask);

  return TRUE;
}

/** mesh_rolloff_init:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Initialize the mesh_rolloff module
 *
 *  Return TRUE on Success, FALSE on failure
 **/
boolean mesh_rolloff44_init(isp_sub_module_t *isp_sub_module)
{
  mesh_rolloff44_t *mesh_rolloff = NULL;

  if (!isp_sub_module) {
    ISP_ERR("failed: isp_sub_module %p", isp_sub_module);
    return FALSE;
  }

  mesh_rolloff = (mesh_rolloff44_t *)CAM_MALLOC(sizeof(mesh_rolloff44_t));
  if (!mesh_rolloff) {
    ISP_ERR("failed: mesh_rolloff %p", mesh_rolloff);
    return FALSE;
  }

  memset(mesh_rolloff, 0, sizeof(*mesh_rolloff));
  /* Initialize default params */
  mesh_rolloff->flash_type = CAMERA_FLASH_LED;
  mesh_rolloff->aec_update.flash_sensitivity.off = 1.0f;
  mesh_rolloff->cur_mired_color_temp = DEFAULT_COLOR_TEMP;
  mesh_rolloff->lux_filter.init_flag = FALSE;

  isp_sub_module->private_data = (void *)mesh_rolloff;
  isp_sub_module->manual_ctrls.cc_mode = CAM_COLOR_CORRECTION_FAST;
  isp_sub_module->manual_ctrls.app_ctrl_mode = CAM_CONTROL_USE_SCENE_MODE;
  isp_sub_module->manual_ctrls.wb_mode = CAM_WB_MODE_AUTO;
  isp_sub_module->manual_ctrls.lens_shading_mode = CAM_LENS_SHADING_MODE_FAST;
  isp_sub_module->manual_ctrls.lens_shading_map_mode = CAM_LENS_SHADING_MAP_MODE_OFF;
  FILL_FUNC_TABLE(mesh_rolloff);
  return TRUE;
}/* mesh_rolloff44_init */

/** mesh_rolloff44_destroy:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Destroy dynamic resources
 *
 *  Return none
 **/
void mesh_rolloff44_destroy(isp_sub_module_t *isp_sub_module)
{
  mesh_rolloff44_t *mesh_rolloff = NULL;
  rolloff_lens_position pos = 0;

  if (!isp_sub_module) {
    ISP_ERR("failed: isp_sub_module %p", isp_sub_module);
    return;
  }
  mesh_rolloff = (mesh_rolloff44_t *)isp_sub_module->private_data;
  if (!mesh_rolloff) {
    ISP_ERR("failed: mesh_rolloff %p", mesh_rolloff);
    return;
  }
  /* Free any pending pointers before destroying module */
  for (pos = 0; pos < ISP_ROLLOFF_LENS_POSITION_MAX; pos++) {
     if (mesh_rolloff->rolloff_tbls.rolloff_tableset[pos]) {
        free(mesh_rolloff->rolloff_tbls.rolloff_tableset[pos]);
        mesh_rolloff->rolloff_tbls.rolloff_tableset[pos] = NULL;
     }
     if (mesh_rolloff->rolloff_calibration_table.rolloff_tableset[pos]) {
        free(mesh_rolloff->rolloff_calibration_table.rolloff_tableset[pos]);
        mesh_rolloff->rolloff_calibration_table.rolloff_tableset[pos] = NULL;
     }
  }

  free(isp_sub_module->private_data);
  return;
} /* mesh_rolloff44_destroy */


/** mesh_rolloff44_calc_awb_trigger:
 *
 *  @isp_sub_module: isp sub module handle
 *  @mod: Pointer to rolloff module struct
 *  @tblOut: output table after interpolation
 *  @mesh_tbls: Pointer to set of different light tables
 *
 *  Use color temp and cct_type from awb_update to interpolate between different
 *  lighting conditions
 *
 *
 *  Return void
 **/
void mesh_rolloff44_calc_awb_trigger(isp_sub_module_t *isp_sub_module,
  void *data1, void *data2,
  isp_rolloff_info_t *mesh_tbls)
{
  float                      ratio = 0.0;
  int i = 0;
  cct_trigger_info           trigger_info;
  awb_cct_type               cct_type = AWB_CCT_TYPE_TL84;
  chromatix_parms_type      *chrPtr = NULL;
  chromatix_VFE_common_type *chrComPtr = NULL;
  chromatix_rolloff_type    *chromatix_rolloff = NULL;
  mesh_rolloff44_t          *mod = NULL;
  MESH_RollOffTable_V4      *tblOut = NULL;

  if (!isp_sub_module || !data1 || !data2 || !mesh_tbls) {
    ISP_ERR("failed: %p %p %p %p", isp_sub_module, data1, data2, mesh_tbls);
    return;
  }
  mod = (mesh_rolloff44_t *)data1;
  tblOut = (MESH_RollOffTable_V4 *)data2;
  chrPtr = isp_sub_module->chromatix_ptrs.chromatixPtr;
  chrComPtr = isp_sub_module->chromatix_ptrs.chromatixComPtr;
  if (!chrPtr || !chrComPtr) {
    ISP_ERR("failed: chromatix_ptr %p chromatix_common %p", chrPtr, chrComPtr);
    return;
  }
  chromatix_rolloff = &chrComPtr->chromatix_rolloff;

  trigger_info.mired_color_temp = ((float)mod->cur_mired_color_temp);
  trigger_info.trigger_A.mired_start = (float)1000000.0f /
    (chromatix_rolloff->rolloff_A_trigger.CCT_start);
  trigger_info.trigger_A.mired_end = (float)1000000.0f /
    (chromatix_rolloff->rolloff_A_trigger.CCT_end);
  trigger_info.trigger_d65.mired_start = (float)1000000.0f /
    (chromatix_rolloff->rolloff_D65_trigger.CCT_start);
  trigger_info.trigger_d65.mired_end = (float)1000000.0f /
    (chromatix_rolloff->rolloff_D65_trigger.CCT_end);

  trigger_info.trigger_H.mired_start = (float)1000000.0f /
    (chromatix_rolloff->rolloff_H_trigger.CCT_start);
  trigger_info.trigger_H.mired_end = (float)1000000.0f /
    (chromatix_rolloff->rolloff_H_trigger.CCT_end);
  cct_type = isp_sub_module_util_get_awb_cct_with_H_type(&trigger_info);

  ISP_DBG("cct_type%d", cct_type);

  switch (cct_type) {
    case AWB_CCT_TYPE_A:
      *tblOut = mesh_tbls->left[ISP_ROLLOFF_A_LIGHT];
      break;
    case AWB_CCT_TYPE_TL84_A:
      ratio = GET_INTERPOLATION_RATIO_ROLLOFF(trigger_info.mired_color_temp,
        trigger_info.trigger_A.mired_start, trigger_info.trigger_A.mired_end);
       mesh_rolloff44_table_interpolate(
         &(mesh_tbls->left[ISP_ROLLOFF_TL84_LIGHT]),
         &(mesh_tbls->left[ISP_ROLLOFF_A_LIGHT]), tblOut, ratio);
       break;
     case AWB_CCT_TYPE_D65_TL84:
       ratio = GET_INTERPOLATION_RATIO_ROLLOFF(trigger_info.mired_color_temp,
         trigger_info.trigger_d65.mired_end,
         trigger_info.trigger_d65.mired_start);
       mesh_rolloff44_table_interpolate(
         &(mesh_tbls->left[ISP_ROLLOFF_D65_LIGHT]),
         &(mesh_tbls->left[ISP_ROLLOFF_TL84_LIGHT]), tblOut, ratio);
       break;
     case AWB_CCT_TYPE_D65:
       *tblOut = mesh_tbls->left[ISP_ROLLOFF_D65_LIGHT];
       break;
     case AWB_CCT_TYPE_H:
       *tblOut = mesh_tbls->left[ISP_ROLLOFF_H_LIGHT];
       break;
     case AWB_CCT_TYPE_A_H:
       ratio = GET_INTERPOLATION_RATIO_ROLLOFF(trigger_info.mired_color_temp,
         trigger_info.trigger_H.mired_start, trigger_info.trigger_H.mired_end);
        mesh_rolloff44_table_interpolate(
          &(mesh_tbls->left[ISP_ROLLOFF_A_LIGHT]),
          &(mesh_tbls->left[ISP_ROLLOFF_H_LIGHT]), tblOut, ratio);
        break;
     case AWB_CCT_TYPE_TL84:
     default:
       *tblOut = mesh_tbls->left[ISP_ROLLOFF_TL84_LIGHT];
       break;
  }
}
/** mesh_rolloff44_calc_awb_trigger_lowLight:
 *
 *  @mod: Pointer to rolloff module struct
 *  @tblOut: output table after interpolation
 *  @mesh_tbls: Pointer to set of different light tables
 *
 *  Use color temp and cct_type from awb_update to interpolate between different
 *  low light tables
 *
 *  Return void
 **/
void mesh_rolloff44_calc_awb_trigger_lowLight(
  isp_sub_module_t *isp_sub_module, void *data1,
  void *data2, isp_rolloff_info_t *mesh_tbls)
{
  float                      ratio = 0.0;
  cct_trigger_info           trigger_info;
  awb_cct_type               cct_type = 0;
  chromatix_parms_type      *chrPtr = NULL;
  chromatix_VFE_common_type *chrComPtr = NULL;
  chromatix_rolloff_type    *chromatix_rolloff = NULL;
  mesh_rolloff44_t          *mod = NULL;
  MESH_RollOffTable_V4      *tblOut = NULL;

  if (!data1 || !isp_sub_module || !data2 || !mesh_tbls) {
    ISP_ERR("failed: %p %p %p %p", data1, isp_sub_module, data2, mesh_tbls);
    return;
  }
  mod = (mesh_rolloff44_t *)data1;
  tblOut = (MESH_RollOffTable_V4 *)data2;
  chrPtr = isp_sub_module->chromatix_ptrs.chromatixPtr;
  chrComPtr = isp_sub_module->chromatix_ptrs.chromatixComPtr;
  if (!chrPtr || !chrComPtr) {
    ISP_ERR("failed: chromatix_ptr %p chromatix_common %p", chrPtr, chrComPtr);
    return ;
  }
  chromatix_rolloff = &chrComPtr->chromatix_rolloff;

  trigger_info.mired_color_temp = mod->cur_mired_color_temp;
  trigger_info.trigger_A.mired_start = (float)1000000.0f /
    (chromatix_rolloff->rolloff_A_trigger.CCT_start);
  trigger_info.trigger_A.mired_end = (float)1000000.0f /
    (chromatix_rolloff->rolloff_A_trigger.CCT_end);
  trigger_info.trigger_d65.mired_start = (float)1000000.0f /
    (chromatix_rolloff->rolloff_D65_trigger.CCT_start);
  trigger_info.trigger_d65.mired_end = (float)1000000.0f /
    (chromatix_rolloff->rolloff_D65_trigger.CCT_end);

  trigger_info.trigger_H.mired_start = (float)1000000.0f /
    (chromatix_rolloff->rolloff_H_trigger.CCT_start);
  trigger_info.trigger_H.mired_end = (float)1000000.0f /
    (chromatix_rolloff->rolloff_H_trigger.CCT_end);
  cct_type = isp_sub_module_util_get_awb_cct_with_H_type(&trigger_info);

  switch (cct_type) {
     case AWB_CCT_TYPE_A:
       *tblOut = mesh_tbls->left[ISP_ROLLOFF_A_LOW_LIGHT];
       break;
     case AWB_CCT_TYPE_TL84_A:
       ratio = GET_INTERPOLATION_RATIO_ROLLOFF(trigger_info.mired_color_temp,
         trigger_info.trigger_A.mired_start, trigger_info.trigger_A.mired_end);
       mesh_rolloff44_table_interpolate(
         &(mesh_tbls->left[ISP_ROLLOFF_TL84_LOW_LIGHT]),
         &(mesh_tbls->left[ISP_ROLLOFF_A_LOW_LIGHT]), tblOut, ratio);
       break;
     case AWB_CCT_TYPE_D65_TL84:
       ratio = GET_INTERPOLATION_RATIO_ROLLOFF(trigger_info.mired_color_temp,
         trigger_info.trigger_d65.mired_end,
         trigger_info.trigger_d65.mired_start);
       mesh_rolloff44_table_interpolate(
         &(mesh_tbls->left[ISP_ROLLOFF_D65_LOW_LIGHT]),
         &(mesh_tbls->left[ISP_ROLLOFF_TL84_LOW_LIGHT]), tblOut, ratio);
       break;
     case AWB_CCT_TYPE_D65:
       *tblOut = mesh_tbls->left[ISP_ROLLOFF_D65_LOW_LIGHT];
       break;
     case AWB_CCT_TYPE_H:
       *tblOut = mesh_tbls->left[ISP_ROLLOFF_H_LOW_LIGHT];
       break;
     case AWB_CCT_TYPE_A_H:
       ratio = GET_INTERPOLATION_RATIO_ROLLOFF(trigger_info.mired_color_temp,
         trigger_info.trigger_H.mired_start, trigger_info.trigger_H.mired_end);
       mesh_rolloff44_table_interpolate(
         &(mesh_tbls->left[ISP_ROLLOFF_A_LOW_LIGHT]),
         &(mesh_tbls->left[ISP_ROLLOFF_H_LOW_LIGHT]), tblOut, ratio);
       break;
     case AWB_CCT_TYPE_TL84:
     default:
       *tblOut = mesh_tbls->left[ISP_ROLLOFF_TL84_LOW_LIGHT];
       break;
  }
} /* mesh_rolloff44_calc_awb_trigger_lowLight */

#if !OVERRIDE_FUNC
/** mesh_rolloff44_calc_interpolation:
 *
 *  @isp_sub_module: isp sub module handle
 *  @mod: Pointer to rolloff module struct
 *  @tblNormalLight: Normal Light rolloff table
 *  @tblLowLight: Low light rolloff table
 *  @tblOut: output table after interpolation
 *
 *  Calculate ratio based on lux or gain to interpolate between normal,
 *  outdoor and low light tables
 *
 *  Return void
 **/
static void mesh_rolloff44_calc_interpolation(isp_sub_module_t *isp_sub_module,
    void *mesh_rolloff44, isp_rolloff_info_t *mesh_tbls,
    MESH_RollOffTable_V4 *tblOut)
{
  float aec_ratio = 0.0;
  chromatix_rolloff_trigger_type *chromatix_rolloff_trigger = NULL;
  chromatix_parms_type *chromatix_ptr = NULL;
  MESH_RollOffTable_V4 meshRolloffTableNormalLight;
  MESH_RollOffTable_V4 meshRolloffTableLowLight;
  mesh_rolloff44_t *mod = (mesh_rolloff44_t*)mesh_rolloff44;

  if (!isp_sub_module || !mod || !mesh_tbls) {
    ISP_ERR("failed: %p %p %p", isp_sub_module, mod, mesh_tbls);
    return;
  }

  if (!isp_sub_module->chromatix_ptrs.chromatixComPtr ||
    !isp_sub_module->chromatix_ptrs.chromatixPtr) {
    ISP_ERR("failed: chromatix %p %p",
      isp_sub_module->chromatix_ptrs.chromatixComPtr,
      isp_sub_module->chromatix_ptrs.chromatixPtr);
    return;
  }

  chromatix_ptr =
    (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;
  chromatix_rolloff_trigger =
    &chromatix_ptr->chromatix_VFE.chromatix_rolloff_trigger;

  mesh_rolloff44_calc_awb_trigger(isp_sub_module, mod,
    &meshRolloffTableNormalLight, mesh_tbls);

  mesh_rolloff44_calc_awb_trigger_lowLight(isp_sub_module,
      mod, &meshRolloffTableLowLight, mesh_tbls);

  aec_ratio = isp_sub_module_util_get_aec_ratio_lowlight(
    chromatix_rolloff_trigger->control_rolloff,
    &(chromatix_rolloff_trigger->rolloff_lowlight_trigger),
    &mod->aec_update, 1);

  ISP_DBG("aec ratio %f", aec_ratio);

  if (F_EQUAL(aec_ratio, 0.0)) {
    ISP_DBG("Low Light ");
    *tblOut = meshRolloffTableLowLight;
  } else if (F_EQUAL(aec_ratio, 1.0)) {
    ISP_DBG("Bright Light ");
    *tblOut = meshRolloffTableNormalLight;
  } else {
    ISP_DBG("Interpolate between Normal and Low Light ");
    mesh_rolloff44_table_interpolate(&meshRolloffTableNormalLight,
     &meshRolloffTableLowLight, tblOut, aec_ratio);
  }
} /* mesh_rolloff44_calc_interpolation */

static ext_override_func mesh_rolloff_override_func = {
  .normalize          = mesh_rolloff44_normalize_table,
  .calc_interpolation = mesh_rolloff44_calc_interpolation
};

boolean mesh_rolloff44_fill_func_table(mesh_rolloff44_t *mesh_rolloff)
{
  mesh_rolloff->ext_func_table = &mesh_rolloff_override_func;
  return TRUE;
} /* mesh_rolloff44_fill_func_table */
#endif
