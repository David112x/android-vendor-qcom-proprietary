/* isp_sub_module_zoom.c
 *
 * Copyright (c) 2011-2014, 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/* std headers */
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

/* kernel headers */
#include "media/msmb_isp.h"

/* isp headers */
#include "isp_sub_module_zoom.h"
#include "isp_log.h"
#include "isp_defs.h"

/* #define ENABLE_ZOOM_LOGGING */
#ifdef ENABLE_ZOOM_LOGGING
  #undef ISP_DBG
  #define ISP_DBG ISP_ERR
#endif

static uint32_t isp_zoom_table_def_hal[MAX_ZOOMS_CNT];

static const uint32_t isp_zoom_table_def[ZOOM_TABLE_MAX_DEF] = {
  4096, 4191, 4289, 4389, 4492,
  4597, 4705, 4815, 4927, 5042,
  5160, 5281, 5404, 5531, 5660,
  5792, 5928, 6066, 6208, 6353,
  6501, 6653, 6809, 6968, 7131,
  7298, 7468, 7643, 7822, 8004,
  8192, 8383, 8579, 8779, 8985,
  9195, 9410, 9630, 9855, 10085,
  10321, 10562, 10809, 11062, 11320,
  11585, 11856, 12133, 12416, 12706,
  13003, 13307, 13619, 13937, 14263,
  14596, 14937, 15286, 15644, 16009,
  16384, 16766, 17158, 17559, 17970,
  18390, 18820, 19260, 19710, 20171,
  20642, 21125, 21618, 22124, 22641,
  23170, 23712, 24266, 24833, 25413,
  26007, 26615, 27238, 27874, 28526,
  29192, 29875, 30573, 31288, 32019,
  32768, 33533, 34317, 35119, 35940,
  36780, 37640, 38520, 39420, 40342,
  41285, 42250, 43237, 44248, 45282,
  46340, 47424, 48532, 49666, 50827,
  52015, 53231, 54476, 55749, 57052,
  58385, 59750, 61147, 62576, 64039,
  65536, 67067, 68635, 70239, 71881,
  73561, 75281, 77040, 78841, 80684,
  82570, 84500, 86475, 88496, 90565,
  92681, 94848, 97065, 99334, 101655,
  104031, 106463, 108952, 111498, 114104,
  116771, 119501, 122294, 125152, 128078,
  131072, 134135, 137270, 140479, 143763,
  147123, 150562, 154081, 157682, 161368,
  165140, 169000, 172950, 176993, 181130,
  185363, 189696, 194130, 198668, 203311,
  208063, 212927, 217904, 222997, 228209,
  233543, 239002, 244589, 250305, 256156,
  262144, 999999
};

/** isp_zoom_calc_num_fovs:
 *
 *  @isp_version:
 *
 *  Return number of fov's based on ISP version
 **/
static int isp_zoom_calc_num_fovs(uint32_t isp_version)
{
  switch (isp_version) {
  case ISP_VERSION_44:
  case ISP_VERSION_40:
    return 2;

  case ISP_VERSION_32:
    return 1;

  default:
    return 1;
  }
}

/** zoom_init:
 *
 *  @zoom: handle to isp_zoom_t
 *
 *  Initialize zoom params
 *
 *  Return void
 **/
static void zoom_init(isp_zoom_t *zoom)
{
  int     i = 0;
  int32_t minimum_value = 0; /* Minimum allowed value */
  int32_t maximum_value = 0; /* Maximum allowed value */

  if (!zoom) {
    ISP_ERR("failed: zoom %p", zoom);
    return;
  }

  /* Compute min and max zoom values:
   * now that we know the resize factor, figure out which entry in our zoom
   * table equals this zoom factor. This entry actually will correspond to
   * NO zoom, since it will tell the VFE to crop the entire image. Entry
   * min_decimation (often 0) in the table, is the smallest amount we
   * can crop, which is max zoom. */
  for (i = 0; i < zoom->zoom_data.zoom_table_size; i++)
    if (zoom->zoom_data.resize_factor < zoom->zoom_data.zoom_table[i + 1])
      break;

  /* if value not found, handle gracefully */
  if (i == zoom->zoom_data.zoom_table_size)
    i = 0;

  /* define which zoom entry defines no zoom. */
  maximum_value = i;

  /* define which zoom entry defines no zoom. */
  /* setup ui values */

  maximum_value = (maximum_value - minimum_value);

  /* if we always want to have 10 zoom steps (a good feature until the UI can
   * handle press and hold zooming where say, 60 steps (4x), zooming does
   * not require 60 key presses, then lets do the following
   */
  if (maximum_value > (MAX_ZOOM_STEPS - 1)) {
    zoom->zoom_data.zoom_step_size =
      (maximum_value / (MAX_ZOOM_STEPS - 1));
    maximum_value = MAX_ZOOM_STEPS - 1;
  } else {
    zoom->zoom_data.zoom_step_size = 1;
  }

  /* If we have say computed 34 zoom steps, but we want to have
   * 10 steps in the UI, then we move 34/(10-1), or 3 steps in the zoom
   * table per click. However, that would take us to 3*9, 27, not 34.
   *
   * 0 3 6 9 12 15 18 21 24 27
   *
   * So we need to bump each value by bump number, specifically,
   *
   * 0 1 2 3  4  5  6  6  7  7
   *
   * which will give us
   *
   * 0 4 8 12 16 20 24 27 31 34
   *
   * This bit of code sets up that bump table */
  if ((maximum_value - minimum_value) > (MAX_ZOOM_STEPS - 1)) {
    int32_t num;

    num = ((maximum_value - minimum_value) % (MAX_ZOOM_STEPS - 1));
    for (i = 0; i < MAX_ZOOM_STEPS; i++) {
      zoom->zoom_data.zoom_table_bump[i] =
        ((i << 4) / (MAX_ZOOM_STEPS - 1)) * num;
      zoom->zoom_data.zoom_table_bump[i] >>= 4;
    }
  } else {
    for (i = 0; i < MAX_ZOOM_STEPS; i++)
      zoom->zoom_data.zoom_table_bump[i] = 0;
  }
}

/** isp_zoom_get_crop_factor:
 *
 *  @pzoom: handle to isp_zoom_t
 *  @zoom_val: zoom value
 *  @crop_factor: crop factor
 *
 *  Convert zoom value to crop factor
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_zoom_get_crop_factor(isp_zoom_t *pzoom, int zoom_val,
  uint32_t *crop_factor)
{
  int idx;

  if (!pzoom || !crop_factor) {
    ISP_ERR("failed: %p %p", pzoom, crop_factor);
    return FALSE;
  }

  idx = pzoom->zoom_data.zoom_step_size * zoom_val +
    pzoom->zoom_data.zoom_table_bump[zoom_val];

  /* This is the meat of the zoom algorithm. This is where we use a table
   * to determine how many times greater than the smallest crop window
   * (which provides maximum zoom) should we crop. If our crop is the
   * smallest crop permissible (low table entry), we get maximum zoom, if
   * we are high in the LUT, we are then a big multiple of min crop window,
   * i.e. big crop window, i.e. little zooming. */
  *crop_factor = pzoom->zoom_data.zoom_table[idx];

  return TRUE;
}

/** isp_zoom_query_mod_func:
 *
 *  @cap_buf: handle to mct_pipeline_isp_cap_t
 *
 *  Fill ISP zoom table into mctl structure
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_zoom_query_mod_func(mct_pipeline_cap_t *cap_buf)
{
  int                     i, rc = 0;
  int                    *pos = NULL;
  mct_pipeline_isp_cap_t *isp_cap = NULL;
  uint32_t smooth_zoom = 0;
  char value[PROPERTY_VALUE_MAX];

  if (!cap_buf) {
    ISP_ERR("failed: %p", cap_buf);
    return FALSE;
  }

  isp_cap = &cap_buf->isp_cap;
  isp_cap->zoom_ratio_tbl_cnt = MAX_ZOOMS_CNT;
  pos = isp_cap->zoom_ratio_tbl;

  property_get("persist.vendor.camera.isp.smoothzoom", value, "0");
  smooth_zoom = atoi(value);


  if (smooth_zoom) {
    for (i=0; i < isp_cap->zoom_ratio_tbl_cnt; i++) {
      *pos = (isp_zoom_table_def_hal[i] * 100) / isp_zoom_table_def_hal[0];
      pos++;
    }
  } else {
    if (isp_cap->zoom_ratio_tbl_cnt > MAX_ZOOM_STEPS) {
      isp_cap->zoom_ratio_tbl_cnt = MAX_ZOOM_STEPS;
    }

    for (i=0; i < isp_cap->zoom_ratio_tbl_cnt; i++) {
      *pos = (isp_zoom_table_def[i] * 100) / isp_zoom_table_def[0];
      pos++;
    }
  }

  return TRUE;
}

/** isp_zoom_create:
 *
 *  @pzoom: handle to isp_zoom_t
 *  @session_id: session id
 *  @isp_version: isp version
 *
 *  Init zoom
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_zoom_init(isp_zoom_t *pzoom, uint32_t session_id,
  uint32_t isp_version)
{
  float min_log_zoom = 0.0f, max_log_zoom = 0.0f;
  float step_zoom =0.0f;
  uint32_t i = 0, smooth_zoom = 0;
  char value[PROPERTY_VALUE_MAX];

  if (!pzoom) {
    ISP_ERR("failed: %p", pzoom);
    return FALSE;
  }

  property_get("persist.vendor.camera.isp.smoothzoom", value, "0");
  smooth_zoom = atoi(value);

  memset(pzoom, 0, sizeof(isp_zoom_t));

  pzoom->zoom_data.resize_factor = Q12 * 4;

  if(smooth_zoom){
    min_log_zoom = log(ZOOM_MIN);
    max_log_zoom = log(ZOOM_MAX);

    step_zoom = (max_log_zoom - min_log_zoom) / (MAX_ZOOMS_CNT - 1);

    for(i = 0; i < MAX_ZOOMS_CNT; i++) {
      isp_zoom_table_def_hal[i] = Round(exp(min_log_zoom + (step_zoom * i)));
    }

    pzoom->zoom_data.zoom_table = (uint32_t *)&isp_zoom_table_def_hal[0];
    pzoom->zoom_data.zoom_table_size = MAX_ZOOMS_CNT;
    pzoom->zoom_data.zoom_step_size = 1;
  }
  else {
    pzoom->zoom_data.zoom_table = (uint32_t *)&isp_zoom_table_def[0];
    pzoom->zoom_data.zoom_table_size = ZOOM_TABLE_MAX_DEF;
    zoom_init(pzoom);
  }

  pzoom->sessions.session_id = session_id;
  pzoom->sessions.num_fovs = isp_zoom_calc_num_fovs(isp_version);

  return TRUE;
}
