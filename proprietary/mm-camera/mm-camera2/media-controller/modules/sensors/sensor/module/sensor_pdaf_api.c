/* sensor_pdaf_api.c
 *
 * Copyright (c) 2015-2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
#include "modules.h"
#include "sensor.h"
#include "sensor_pdaf_api.h"
#include "pdaf_lib.h"
#include "pdaf_api.h"
#include "pdaf_camif_api.h"
#include "PdafLibrary.h"

#define INTERPOLATION_RATIO(p0, p1, p_tg) \
  ((p0 == p1) ? 0.5 : ((float)abs(p_tg-p0)/abs(p1-p0)))

#define MAX_FLOATING_WIN 8
#define MAX_FIXED_WIN 192

/* Marcos for PDAF threshold line */
#define POINT_NUM 4
#define X_KNOT 8
#define Y_KNOT 6

/* For DensityOfPhasePix */
#define D_PD_LIB_DENSITY_SENS_MODE0            (2304)
/* For AdjCoeffSlope */
#define D_PD_LIB_SLOPE_ADJ_COEFF_SENS_MODE0    (2304)


/* pick up a bit field.
  When 'MSBpos' is the same as 'LSBpos', single bit is extracted. */
static inline unsigned int bitPick(unsigned int x, int MSBpos, int LSBpos)
{
  return (x >> LSBpos) & (~(~0 << (MSBpos - LSBpos + 1)));
}

/** find_reg_data:
 *    @reg_array: register array
 *    @size: size of register array
 *    @addr: addr to be searched
 *
 *  Return: register data on success
 *    0 on failure
 *
 **/
static unsigned short find_reg_data(struct camera_i2c_reg_array reg_array[],
  int size, unsigned short addr)
{
  int i;

  for(i = 0;i < size;i++) {
    if(reg_array[i].reg_addr == addr)
      return reg_array[i].reg_data;
  }

  return 0;
}

/** PdLibInterpretRegData:
 *    @p_regData: interpreted register setting array
 *    @p_setting: register setting array
 *    @reg_array: register array
 *    @size: size of register array
 *    @reg_info: register addresses
 *
 *  Return: 0 on success
 *    error code on failure
 *
 *  The function reads PDAF related settings and interprets the them.
 **/
static int PdLibInterpretRegData(PdLibSensorRegData_t *p_regData,
  PdLibSensorSetting_t *p_setting,
  struct camera_i2c_reg_array reg_array[], int size, pdaf_reg_info_t *reg_info)
{
  int ret = D_PD_LIB_REGDATA_IS_OK;
  int i = 0;

  /* Load data */
  p_regData->orientation = find_reg_data(reg_array, size,
    reg_info->orientation_addr);
  p_regData->hdr_en = find_reg_data(reg_array, size,
    reg_info->hdr_en_addr);
  p_regData->hdr_binning = find_reg_data(reg_array, size,
    reg_info->hdr_binning_addr);
  p_regData->x_add_sta_h = find_reg_data(reg_array, size,
    reg_info->x_add_sta_addr);
  p_regData->x_add_sta_l = find_reg_data(reg_array, size,
    reg_info->x_add_sta_addr + 1);
  p_regData->y_add_sta_h = find_reg_data(reg_array, size,
    reg_info->y_add_sta_addr);
  p_regData->y_add_sta_l = find_reg_data(reg_array, size,
    reg_info->y_add_sta_addr + 1);
  p_regData->x_add_end_h = find_reg_data(reg_array, size,
    reg_info->x_add_end_addr);
  p_regData->x_add_end_l = find_reg_data(reg_array, size,
    reg_info->x_add_end_addr + 1);
  p_regData->y_add_end_h = find_reg_data(reg_array, size,
    reg_info->y_add_end_addr);
  p_regData->y_add_end_l = find_reg_data(reg_array, size,
    reg_info->y_add_end_addr + 1);
  p_regData->x_out_size_h = find_reg_data(reg_array, size,
    reg_info->x_out_size_addr);
  p_regData->x_out_size_l = find_reg_data(reg_array, size,
    reg_info->x_out_size_addr + 1);
  p_regData->y_out_size_h = find_reg_data(reg_array, size,
    reg_info->y_out_size_addr);
  p_regData->y_out_size_l = find_reg_data(reg_array, size,
    reg_info->y_out_size_addr + 1);
  p_regData->x_evn_inc = find_reg_data(reg_array, size,
    reg_info->x_evn_inc_addr);
  p_regData->x_odd_inc = find_reg_data(reg_array, size,
    reg_info->x_odd_inc_addr);
  p_regData->y_evn_inc = find_reg_data(reg_array, size,
    reg_info->y_evn_inc_addr);
  p_regData->y_odd_inc = find_reg_data(reg_array, size,
    reg_info->y_odd_inc_addr);
  p_regData->scale_mode = find_reg_data(reg_array, size,
    reg_info->scale_mode_addr);
  p_regData->scale_m_h = find_reg_data(reg_array, size,
    reg_info->scale_m_addr);
  p_regData->scale_m_l = find_reg_data(reg_array, size,
    reg_info->scale_m_addr + 1);
  p_regData->dig_crop_x_offset_h = find_reg_data(reg_array, size,
    reg_info->dig_crop_x_offset_addr);
  p_regData->dig_crop_x_offset_l = find_reg_data(reg_array, size,
    reg_info->dig_crop_x_offset_addr + 1);
  p_regData->dig_crop_y_offset_h = find_reg_data(reg_array, size,
    reg_info->dig_crop_y_offset_addr);
  p_regData->dig_crop_y_offset_l = find_reg_data(reg_array, size,
    reg_info->dig_crop_y_offset_addr + 1);
  p_regData->dig_crop_image_width_h = find_reg_data(reg_array, size,
    reg_info->dig_crop_image_width_addr);
  p_regData->dig_crop_image_width_l = find_reg_data(reg_array, size,
    reg_info->dig_crop_image_width_addr + 1);
  p_regData->dig_crop_image_height_h = find_reg_data(reg_array, size,
    reg_info->dig_crop_image_height_addr);
  p_regData->dig_crop_image_height_l = find_reg_data(reg_array, size,
    reg_info->dig_crop_image_height_addr + 1);
  p_regData->binning_mode = find_reg_data(reg_array, size,
    reg_info->binning_mode_addr);
  p_regData->binning_type = find_reg_data(reg_array, size,
    reg_info->binning_type_addr);
  p_regData->pd_offset_x_h = find_reg_data(reg_array, size,
    reg_info->pd_area_x_offset_addr);
  p_regData->pd_offset_x_l = find_reg_data(reg_array, size,
    reg_info->pd_area_x_offset_addr + 1);
  p_regData->pd_offset_y_h = find_reg_data(reg_array, size,
    reg_info->pd_area_y_offset_addr);
  p_regData->pd_offset_y_l = find_reg_data(reg_array, size,
    reg_info->pd_area_y_offset_addr + 1);
  p_regData->pd_area_w_h = find_reg_data(reg_array, size,
    reg_info->pd_area_width_addr);
  p_regData->pd_area_w_l = find_reg_data(reg_array, size,
    reg_info->pd_area_width_addr + 1);
  p_regData->pd_area_h_h = find_reg_data(reg_array, size,
    reg_info->pd_area_height_addr);
  p_regData->pd_area_h_l = find_reg_data(reg_array, size,
    reg_info->pd_area_height_addr + 1);

  for(i = 0; i < MAX_FLOATING_WIN*4*2; i++) {
    p_regData->reg_addr_flex_win[i] = find_reg_data(reg_array, size,
      reg_info->flex_area_xsta0_addr + i);
  }

  /* Interpret data */
  p_setting->img_orientation_h = bitPick(p_regData->orientation, 0, 0);
  p_setting->img_orientation_v = bitPick(p_regData->orientation, 1, 1);

  p_setting->hdr_mode_en = bitPick(p_regData->hdr_en, 0, 0);
  p_setting->hdr_reso_redu_h = bitPick(p_regData->hdr_binning, 7, 4);
  p_setting->hdr_reso_redu_v = bitPick(p_regData->hdr_binning, 3, 0);

  p_setting->x_add_sta = (bitPick(p_regData->x_add_sta_h, 4, 0) << 8)
    + bitPick(p_regData->x_add_sta_l, 7, 0);
  p_setting->y_add_sta = (bitPick(p_regData->y_add_sta_h, 3, 0) << 8)
    + bitPick(p_regData->y_add_sta_l, 7, 0);
  p_setting->x_add_end = (bitPick(p_regData->x_add_end_h, 4, 0) << 8)
    + bitPick(p_regData->x_add_end_l, 7, 0);
  p_setting->y_add_end = (bitPick(p_regData->y_add_end_h, 3, 0) << 8)
    + bitPick(p_regData->y_add_end_l, 7, 0);
  p_setting->x_out_size = (bitPick(p_regData->x_out_size_h, 4, 0) << 8)
    + bitPick(p_regData->x_out_size_l, 7, 0);
  p_setting->y_out_size = (bitPick(p_regData->y_out_size_h, 4, 0) << 8)
    + bitPick(p_regData->y_out_size_l, 7, 0);

  p_setting->x_evn_inc = bitPick(p_regData->x_evn_inc, 2, 0);
  p_setting->x_odd_inc = bitPick(p_regData->x_odd_inc, 2, 0);
  p_setting->y_evn_inc = bitPick(p_regData->y_evn_inc, 3, 0);
  p_setting->y_odd_inc = bitPick(p_regData->y_odd_inc, 3, 0);

  p_setting->scale_mode = bitPick(p_regData->scale_mode, 1, 0);
  p_setting->scale_m = (bitPick(p_regData->scale_m_h, 0, 0) << 8)
    + bitPick(p_regData->scale_m_l, 7, 0);

  p_setting->dig_crop_x_offset     = (bitPick(p_regData->dig_crop_x_offset_h, 4, 0) << 8)
    + bitPick(p_regData->dig_crop_x_offset_l, 7, 0);
  p_setting->dig_crop_y_offset     = (bitPick(p_regData->dig_crop_y_offset_h, 3, 0) << 8)
    + bitPick(p_regData->dig_crop_y_offset_l, 7, 0);
  p_setting->dig_crop_image_width  = (bitPick(p_regData->dig_crop_image_width_h, 4, 0) << 8)
    + bitPick(p_regData->dig_crop_image_width_l, 7, 0);
  p_setting->dig_crop_image_height = (bitPick(p_regData->dig_crop_image_height_h, 3, 0) << 8)
    + bitPick(p_regData->dig_crop_image_height_l, 7, 0);

  p_setting->binning_mode = bitPick(p_regData->binning_mode, 0, 0);
  p_setting->binning_type_h = bitPick(p_regData->binning_type, 7, 4);
  p_setting->binning_type_v = bitPick(p_regData->binning_type, 3, 0);

  p_setting->pd_offset_x = (bitPick(p_regData->pd_offset_x_h, 4, 0) << 8)
    + bitPick(p_regData->pd_offset_x_l, 7, 0);
  p_setting->pd_offset_y = (bitPick(p_regData->pd_offset_y_h, 4, 0) << 8)
    + bitPick(p_regData->pd_offset_y_l, 7, 0);
  p_setting->pd_area_w = (bitPick(p_regData->pd_area_w_h, 4, 0) << 8)
    + bitPick(p_regData->pd_area_w_l, 7, 0);
  p_setting->pd_area_h = (bitPick(p_regData->pd_area_h_h, 4, 0) << 8)
    + bitPick(p_regData->pd_area_h_l, 7, 0);

  for(i = 0;i < MAX_FLOATING_WIN*4;i++) {
    p_setting->flex_win_addr[i] =
     (bitPick(p_regData->reg_addr_flex_win[i * 2], 4, 0) << 8) +
     bitPick(p_regData->reg_addr_flex_win[i * 2 + 1], 7, 0);
  }

  SLOW("x_add_sta: %d, y_add_sta: %d, x_add_end: %d, y_add_end: %d",
    p_setting->x_add_sta, p_setting->y_add_sta,
    p_setting->x_add_end, p_setting->y_add_end);

  if (p_setting->hdr_reso_redu_h != p_setting->hdr_reso_redu_v) {
      ret = D_PD_LIB_REGDATA_IS_NG;
  }
  if ((p_setting->y_evn_inc != 1) || (p_setting->x_evn_inc != 1)) {
      ret = D_PD_LIB_REGDATA_IS_NG;
  }
  if (p_setting->y_odd_inc != p_setting->x_odd_inc) {
      ret = D_PD_LIB_REGDATA_IS_NG;
  }
  if (p_setting->binning_type_h != p_setting->binning_type_v) {
      ret = D_PD_LIB_REGDATA_IS_NG;
  }

  return ret;
}

/** PdLibTransOutputPointToPdafPoint:
 *    @onOutImage: point coordinates on output image
 *    @p_setting: interpreted settings
 *
 *  Return: point on full image
 *
 *  The function remaps point from out image to full image.
 **/
static PdLibPoint_t PdLibTransOutputPointToPdafPoint(PdLibPoint_t onOutImage,
  PdLibSensorSetting_t *p_setting)
{
    PdLibPoint_t ret;
    int x4, y4;
    int x3, y3;
    int x2, y2;
    int x1, y1;
    int x0, y0;
    int mag_scaleX, mag_scaleY, mag_binsub;

    x4 = onOutImage.x;
    y4 = onOutImage.y;

    /* inverse scaling */
    switch (p_setting->scale_mode) {
    case H_SCALING:
        mag_scaleX = p_setting->scale_m;
        mag_scaleY = 16;
        break;
    case H_AND_V_SCALING:
        mag_scaleX = p_setting->scale_m;
        mag_scaleY = p_setting->scale_m;
        break;
    default:
        mag_scaleX = 16;
        mag_scaleY = 16;
        break;
    }
    x3 = x4 * mag_scaleX / 16 + (mag_scaleX / 16) / 2;
    y3 = y4 * mag_scaleY / 16 + (mag_scaleY / 16) / 2;

    /* inverse digital crop */
    x2 = x3 + p_setting->dig_crop_x_offset;
    y2 = y3 + p_setting->dig_crop_y_offset;

    /* inverse binning and sub-sampling */
    if (p_setting->hdr_mode_en == 0) {
        /* Normal capture mode */
        /* check only binning_type_v,
               because binning_type_h should be the same value. */
        mag_binsub = p_setting->binning_type_v;
        /* check only y_odd_inc, because x_odd_inc should be the same value. */
        if (p_setting->y_odd_inc == 3) {
            mag_binsub *= 2;
        }
    } else {
        /* HDR capture mode */
        /* check only hdr_reso_redu_v,
               because hdr_reso_redu_h should be the same value. */
        mag_binsub = p_setting->hdr_reso_redu_v;
    }
    x1 = x2 * mag_binsub + (mag_binsub / 2);
    y1 = y2 * mag_binsub + (mag_binsub / 2);

    /* inverse analog crop */
    if (p_setting->img_orientation_h == 0) {
        x0 = x1 + p_setting->x_add_sta;
    } else {
        /* mirroring */
        x0 = p_setting->x_add_end - x1;
    }
    if (p_setting->img_orientation_v == 0) {
        y0 = y1 + p_setting->y_add_sta;
    } else {
        /* flipping */
        y0 = p_setting->y_add_end - y1;
    }

    ret.x = x0;
    ret.y = y0;

    return ret;
}

static void get_image_readout_info
  (PdLibSensorSetting_t *p_setting, float *scale_x, float *scale_y,
   int *crop_x, int *crop_y, int *crop_w, int *crop_h,
   int *is_mirror, int *is_flip)
{
    int mag_scaleX, mag_scaleY, mag_binsub;

    /* inverse scaling */
    switch (p_setting->scale_mode) {
    case H_SCALING:
        mag_scaleX = p_setting->scale_m;
        mag_scaleY = 16;
        break;
    case H_AND_V_SCALING:
        mag_scaleX = p_setting->scale_m;
        mag_scaleY = p_setting->scale_m;
        break;
    default:
        mag_scaleX = 16;
        mag_scaleY = 16;
        break;
    }

    /* IMX sensors calculates down scale factor as (16 / scale_m);
         minimum value of scale_m is 16 */
    *scale_x = 16 / mag_scaleX;
    *scale_y = 16 / mag_scaleY;

    /* inverse digital crop */
    *crop_x =  p_setting->dig_crop_x_offset;
    *crop_y =  p_setting->dig_crop_y_offset;

    /* inverse binning and sub-sampling */
    if (p_setting->hdr_mode_en == 0) {
        /* Normal capture mode */
        /* check only binning_type_v,
               because binning_type_h should be the same value. */
        mag_binsub = p_setting->binning_type_v;
        /* check only y_odd_inc, because x_odd_inc should be the same value. */
        if (p_setting->y_odd_inc == 3) {
            mag_binsub *= 2;
        }
    } else {
        /* HDR capture mode */
        /* check only hdr_reso_redu_v,
               because hdr_reso_redu_h should be the same value. */
        mag_binsub = p_setting->hdr_reso_redu_v;
    }
    *scale_x *= mag_binsub;
    *scale_y *= mag_binsub;

    *crop_x += p_setting->x_add_sta;
    *crop_y += p_setting->y_add_sta;

    *crop_w = p_setting->x_add_end - p_setting->x_add_sta + 1;
    *crop_h = p_setting->y_add_end - p_setting->y_add_sta + 1;

    /* inverse analog crop */
    if (p_setting->img_orientation_h == 0) {
        *is_mirror = 0;
    } else {
        /* mirroring */
        *is_mirror = 1;
    }
    if (p_setting->img_orientation_v == 0) {
        *is_flip = 0;
    } else {
        /* flipping */
        *is_flip = 1;
    }
}


/** convert_pd_win_addr:
 *    @p_setting: interpreted settings
 *    @p_x_sta, p_y_sta, p_x_end, p_y_end: window coordinates
 *
 *  Return: none
 *
 *  The function remaps PD window from out image to full image.
 **/
static void convert_pd_win_addr(PdLibSensorSetting_t *p_setting,
  unsigned short *p_x_sta, unsigned short *p_y_sta,
  unsigned short *p_x_end, unsigned short *p_y_end)
{
  PdLibPoint_t p1 = {*p_x_sta, *p_y_sta};
  PdLibPoint_t p2 = {*p_x_end, *p_y_end};
  PdLibPoint_t ret_p1 = {0, 0};
  PdLibPoint_t ret_p2 = {0, 0};

  if(p_setting->binning_type_h == 0 &&
    p_setting->hdr_reso_redu_h == 0) {
    SHIGH("Sensor drive does not have proper registers specified. "
      "Conversion is not done.");
    return;
  }

  ret_p1 = PdLibTransOutputPointToPdafPoint(p1, p_setting);
  ret_p2 = PdLibTransOutputPointToPdafPoint(p2, p_setting);

  if(ret_p1.x < ret_p2.x) {
    *p_x_sta = ret_p1.x;
    *p_x_end = ret_p2.x;
  } else {
    *p_x_sta = ret_p2.x;
    *p_x_end = ret_p1.x;
  }

  if(ret_p1.y < ret_p2.y) {
    *p_y_sta = ret_p1.y;
    *p_y_end = ret_p2.y;
  } else {
    *p_y_sta = ret_p2.y;
    *p_y_end = ret_p1.y;
  }
}


/** pdaf_get_defocus_t1:
 *    @ctrl: sensor control struct
 *    @s_pdaf: pdaf struct
 *
 *  Return: 0 on success
 *             -1 on failure
 *
 *  The function gets defocus for PDAF T1.
 **/
static int32_t pdaf_get_defocus_t1 (sensor_ctrl_t *ctrl, pdaf_params_t *s_pdaf)
{
  int32_t                      rc = 0;
  uint32_t                      i = 0;
  int32_t                      cal_data_present = 0;
  DefocusOKNGThrLine_t         threshold[X_KNOT*Y_KNOT];
  unsigned long                conf[POINT_NUM];
  unsigned long                analog_gain[POINT_NUM];
  unsigned short               x_knot_addr[X_KNOT];
  unsigned short               y_knot_addr[Y_KNOT];
  pdafcalib_data_t             *dcc;
  PdLibInputData_t             input;
  PdLibOutputData_t            output;
  signed long                  OffsetData[8*6] = {0};
  signed long                  SlopeData[8*6] = {0};
  short                        cur_res = ctrl->s_data->cur_res;
  int32_t                      pd_win_x = ctrl->s_data->pd_x_win_num;
  int32_t                      pd_win_y = ctrl->s_data->pd_y_win_num;
  sensor_lib_t                 *sensor_lib = ctrl->lib_params->sensor_lib_ptr;
  pdaf_lib_t                   *params = &sensor_lib->pdaf_config;

  PdLibSensorRegData_t        *reg_data = &ctrl->s_data->reg_data;
  PdLibSensorSetting_t        *reg_setting = &ctrl->s_data->reg_setting;

  RETURN_ERROR_ON_NULL(s_pdaf);

  s_pdaf->status = FALSE;

  SLOW("PDAF calculate defocus, cur_res:%d",cur_res);

  if(pd_win_x == 0 || pd_win_y == 0) {
    SLOW("No PD window update from 3A. Use default window");
    pdaf_set_window_update(ctrl, &params->window_configure);
  }

  /* set calibration data */
  if(s_pdaf->eeprom_data != NULL) {
    dcc = &((format_data_t *)(s_pdaf->eeprom_data))->pdafc;
    input.XKnotNumSlopeOffset = dcc->XKnotNumSlopeOffset;
    input.YKnotNumSlopeOffset = dcc->YKnotNumSlopeOffset;
    for(i=0;i < dcc->XKnotNumSlopeOffset * dcc->YKnotNumSlopeOffset;i++) {
      /* restore to integer u6.10*/
      SlopeData[i] = (signed long)(dcc->SlopeData[i] * 1024.0f);
      OffsetData[i] = (signed long)dcc->OffsetData[i];
    }
    input.p_SlopeData = SlopeData;
    input.p_OffsetData= OffsetData;
    input.p_XAddressKnotSlopeOffset = dcc->XAddressKnotSlopeOffset;
    input.p_YAddressKnotSlopeOffset = dcc->YAddressKnotSlopeOffset;

    if(input.p_SlopeData[0] == 0 && input.p_SlopeData[1] == 0)
      cal_data_present = 0;
    else
      cal_data_present = 1;
  }

  if (!cal_data_present) {
    SERR("PDAF ERROR calibration data unavailable");
    return 0;
  }

  /* set sensor data */
  input.ImagerAnalogGain = s_pdaf->sensor_real_gain;
  input.DensityOfPhasePix =
    D_PD_LIB_DENSITY_SENS_MODE0; /* all pixel mode */
  input.AdjCoeffSlope =
    D_PD_LIB_SLOPE_ADJ_COEFF_SENS_MODE0; /* all pixel mode */
  /* image size always as in all-pixel mode */
  input.XSizeOfImage =
    sensor_lib->pixel_array_size_info.active_array_size.width;
  input.YSizeOfImage =
    sensor_lib->pixel_array_size_info.active_array_size.height;

  /* set fixed threshold line */
  input.XKnotNumDefocusOKNG = X_KNOT;
  input.YKnotNumDefocusOKNG = Y_KNOT;
  for(i=0;i<POINT_NUM;i++) {
    conf[i] = params->defocus_confidence_th;
    analog_gain[i] = i * 8 / (POINT_NUM-1);
  }

  for(i=0;i<X_KNOT*Y_KNOT;i++) {
    threshold[i].PointNum = POINT_NUM;
    threshold[i].p_AnalogGain = analog_gain;
    threshold[i].p_Confidence = conf;
  }

  for(i=0;i<X_KNOT;i++)
    x_knot_addr[i] = i * input.XSizeOfImage / (X_KNOT-1);

  for(i=0;i<Y_KNOT;i++)
    y_knot_addr[i] = i * input.YSizeOfImage / (Y_KNOT-1);

  input.p_DefocusOKNGThrLine = threshold;
  input.p_XAddressKnotDefocusOKNG = x_knot_addr;
  input.p_YAddressKnotDefocusOKNG = y_knot_addr;

  /* get register settings */
  if (ctrl->s_data->prev_res != cur_res) {
    PdLibInterpretRegData(reg_data, reg_setting,
    sensor_lib->res_settings_array.reg_settings[cur_res].reg_setting_a,
    sensor_lib->res_settings_array.reg_settings[cur_res].size,
    &params->pdaf_reg_info);
    ctrl->s_data->prev_res = cur_res;
  }

  /* get defocus for each focus window */
    for(i = 0;i < ctrl->s_data->pd_x_win_num * ctrl->s_data->pd_y_win_num;i++){
      /* restore to integer s27.4 from float */
      input.PhaseDifference =
        (signed long)(s_pdaf->defocus[i].phase_diff * 16.0f);
      input.ConfidenceLevel
         = s_pdaf->defocus[i].df_conf_level;

      input.XAddressOfWindowStart = ctrl->s_data->cur_pd_windows[i].x_sta;
      input.YAddressOfWindowStart = ctrl->s_data->cur_pd_windows[i].y_sta;
      input.XAddressOfWindowEnd = ctrl->s_data->cur_pd_windows[i].x_end;
      input.YAddressOfWindowEnd = ctrl->s_data->cur_pd_windows[i].y_end;


      /* map PD window addr to full size */
      convert_pd_win_addr(reg_setting, &input.XAddressOfWindowStart,
        &input.YAddressOfWindowStart, &input.XAddressOfWindowEnd,
        &input.YAddressOfWindowEnd);

      SLOW("PDAF INPUT[%d]: window: X_STA %d X_END %d, Y_STA %d Y_END %d",
        i,
        input.XAddressOfWindowStart, input.XAddressOfWindowEnd,
        input.YAddressOfWindowStart, input.YAddressOfWindowEnd);

      rc = PdLibGetDefocus(&input, &output);
      if (rc != D_PD_LIB_E_OK) {
        SERR("PDAF LIB ERROR: D_PD_LIB_E_NG");
        return 0;
      }

      /* set output */
      /* convert to 12 bit dac + sign bit: s12.0 by keeping two bits after
          decimal point; defocus in same sign of PD.
          Note that in this design, calibration is done in 10-bit DAC.
          The shifting (>>12) is for a 12-bit actuator for the given
          calibration data format. In case 10-bit actuator used with same
          calibration data format, (>>14)  should be shifted */
      s_pdaf->defocus[i].defocus =
        output.Defocus >> params->defocus_bit_shift;
      s_pdaf->defocus[i].df_confidence =
        output.DefocusConfidence;
      s_pdaf->defocus[i].df_conf_level =
        output.DefocusConfidenceLevel;

      SLOW("PDAF OUTPUT win:%d, defocus: %d, conf level: %d, "
        "phase diff: %f, DCC: %f",
        i, s_pdaf->defocus[i].defocus,
        s_pdaf->defocus[i].df_conf_level,
        s_pdaf->defocus[i].phase_diff,
        (output.Defocus >> 14)/s_pdaf->defocus[i].phase_diff);
  }

  s_pdaf->status = TRUE;

  return 0;
}


static void pdaf_rm_pd_offset(void *data){
  pdaf_sensor_native_info_t       *native_pattern
   = (pdaf_sensor_native_info_t *) data;
  unsigned int i = 0;
  for ( i = 0; i< native_pattern->block_pattern.pix_count; i++)
  {
    native_pattern->block_pattern.pix_coords[i].x -=
     native_pattern->block_pattern.pd_offset_horizontal;
    native_pattern->block_pattern.pix_coords[i].y -=
     native_pattern->block_pattern.pd_offset_vertical;
  }
}

boolean pdaf_get_native_pattern(void *sctrl, void *data){
  int32_t                         rc    = SENSOR_SUCCESS;
  sensor_ctrl_t                   *ctrl = (sensor_ctrl_t *)sctrl;
  pdaf_lib_t                      *params;
  sensor_imaging_pixel_array_size *array_size_info;
  sensor_lib_t                    *sensor_lib;
  pdaf_sensor_native_info_t       *native_pattern
   = (pdaf_sensor_native_info_t *) data;
  struct sensor_lib_out_info_t    *out_info;
  PdLibSensorSetting_t            *reg_setting;
  int                              is_flip = 0, is_mirror = 0;

  RETURN_ERROR_ON_NULL(ctrl);
  RETURN_ERROR_ON_NULL(native_pattern);

  reg_setting = &ctrl->s_data->reg_setting;
  sensor_lib = ctrl->lib_params->sensor_lib_ptr;
  params     = &sensor_lib->pdaf_config;
  array_size_info = &sensor_lib->pixel_array_size_info;
  out_info       = &sensor_lib->out_info_array.out_info[ctrl->s_data->cur_res];
  /* pack native pattern */
  memcpy( &native_pattern->block_pattern,
          &params->sensor_native_pattern_info[ctrl->s_data->cur_res].block_pattern,
          sizeof(native_pattern->block_pattern));
  native_pattern->orientation = params->orientation;

  /* MIPI readout size for pd gain calibration
     assume pd gain calibration is done on the active pixel region
  */
  native_pattern->ori_image_height = array_size_info->active_array_size.height;
  native_pattern->ori_image_width = array_size_info->active_array_size.width;
  /* no isp crop involved
     stats streams only depend on hw register setting
     crop region from effective pixels
  */
  if(reg_setting->binning_type_h == 0 &&
    reg_setting->hdr_reso_redu_h == 0) {
    SHIGH("Sensor drive does not have proper registers specified. "
      "Using static value in driver.");
    if(native_pattern->crop_region.w == 0) {
    /* use out_info when value not populated in pdaf header */
      native_pattern->crop_region.w = out_info->x_output;
      native_pattern->crop_region.h = out_info->y_output;
    }
    native_pattern->crop_region.x = out_info->offset_x;
    native_pattern->crop_region.y = out_info->offset_y;
    native_pattern->downscale_factor_h =
    native_pattern->downscale_factor_v =
      out_info->binning_factor * out_info->scale_factor;
  } else {
    /* using value read from register settings */
    get_image_readout_info(reg_setting, &native_pattern->downscale_factor_h,
      &native_pattern->downscale_factor_v,&native_pattern->crop_region.x,
      &native_pattern->crop_region.y, &native_pattern->crop_region.w,
      &native_pattern->crop_region.h,&is_mirror,
      &is_flip);

    if(is_mirror && is_flip)
      native_pattern->orientation = PDAF_ORIENTATION_MIRROR_AND_FLIP;
    else if (is_mirror)
      native_pattern->orientation = PDAF_ORIENTATION_MIRROR;
    else if (is_flip)
      native_pattern->orientation = PDAF_ORIENTATION_FLIP;
    else
      native_pattern->orientation = PDAF_ORIENTATION_DEFAULT;
  }

    SLOW("res: %d, downscale factor: %f %f, crop region: %d %d %d %d, "
      "orientation: %d\n", ctrl->s_data->cur_res,
      native_pattern->downscale_factor_h,
      native_pattern->downscale_factor_v,
      native_pattern->crop_region.x,
      native_pattern->crop_region.y,
      native_pattern->crop_region.w,
      native_pattern->crop_region.h,
      native_pattern->orientation);

  native_pattern->cur_image_width = out_info->x_output;
  native_pattern->cur_image_height = out_info->y_output;

  native_pattern->block_count_horizontal =
   params->sensor_native_pattern_info[ctrl->s_data->cur_res].block_count_horizontal;
  native_pattern->block_count_vertical =
   params->sensor_native_pattern_info[ctrl->s_data->cur_res].block_count_vertical;

  native_pattern->buffer_data_type =
   params->buffer_block_pattern_info[ctrl->s_data->cur_res].buffer_data_type;

  native_pattern->pd_offset_correction = params->pd_offset_correction;
  return rc;
}

boolean pdaf_deinit(void *sctrl){
  int32_t            rc        = SENSOR_SUCCESS;
  sensor_ctrl_t      *ctrl     = (sensor_ctrl_t *)sctrl;
  pdaf_lib_t          *params;
  sensor_custom_API_t *sensor_custom_API = NULL;
  struct sensor_lib_out_info_t  *out_info;
  sensor_lib_t                  *sensor_lib;
  RETURN_ERROR_ON_NULL(sctrl);
  sensor_lib         = ctrl->lib_params->sensor_lib_ptr;
  params             = &sensor_lib->pdaf_config;
  RETURN_ERROR_ON_NULL(params);
  sensor_custom_API = &ctrl->lib_params->sensor_custom_API;
  out_info = &sensor_lib->out_info_array.out_info[ctrl->s_data->cur_res];

  if(!out_info->is_pdaf_supported)
   return rc;

  if (!ctrl->is_pdaf_initiated)
    return rc;

  /* for thrid party OEM */
  if (sensor_custom_API->pdlib_deinit)
    rc = sensor_custom_API->pdlib_deinit();
  else if(params->vendor_id == QTI)
  {
    /* avoid multiple free */
    if (ctrl->pd_camif_handle && ctrl->pd_handle)
    {
    rc = PDAF_CAMIF_deinit(ctrl->pd_camif_handle);
    if ( rc != PDAF_LIB_OK)
    {
      SERR("PDAF CAMIF deinit failed %d", rc);
    }

    rc = PDAF_PhaseDetection_deinit(ctrl->pd_handle);
    if ( rc != PDAF_LIB_OK)
    {
      SERR("PDAF PhaseDetection deinit failed %d", rc);
    }
     }
  }
  /* reset */
  ctrl->pd_camif_handle = ctrl->pd_handle = 0;
  ctrl->is_pdaf_initiated = false;
  return rc;
}


boolean pdaf_set_window_update(void *sctrl, void *data){
  sensor_ctrl_t                      *ctrl = (sensor_ctrl_t *)sctrl;
  pdaf_lib_t                         *params;
  pdaf_window_configure_t            *default_window = NULL, *window = NULL;
  struct sensor_lib_out_info_t       *out_info;
  sensor_lib_t                       *sensor_lib;
  int32_t                             rc = SENSOR_SUCCESS;
  unsigned short                      x_sta[8] = {0}, y_sta[8] = {0};
  unsigned short                      x_end[8] = {0}, y_end[8] = {0};
  short                               cur_res = 0;
  pdaf_reg_info_t                    *reg_info =  NULL;
  unsigned short                      i = 0, j = 0;
  unsigned short                      win_num = 0, flex_area_addr = 0;
  short                               x_offset = 0, y_offset = 0, w = 0, h = 0;
  pdaf_float_window_configure_t      *float_win = NULL;
  pdaf_fixed_grid_window_configure_t *fixed_win = NULL;
  int32_t                             pd_win_x = 0;
  int32_t                             pd_win_y = 0;
  struct camera_i2c_reg_setting_array settings;
  sensor_data_t                      *s_data;

  RETURN_ERROR_ON_NULL(sctrl);
  RETURN_ERROR_ON_NULL(data);

  memset(&settings, 0, sizeof(settings));

  cur_res = ctrl->s_data->cur_res;
  sensor_lib = ctrl->lib_params->sensor_lib_ptr;
  s_data = ctrl->s_data;

  params = &sensor_lib->pdaf_config;
  default_window = &params->window_configure;
  window = (pdaf_window_configure_t*)data;
  reg_info = &params->pdaf_reg_info;

  out_info = &sensor_lib->out_info_array.out_info[cur_res];
  settings.addr_type =
    sensor_lib->res_settings_array.reg_settings[cur_res].addr_type;
  settings.data_type =
    sensor_lib->res_settings_array.reg_settings[cur_res].data_type;

  if(!out_info->is_pdaf_supported) {
    SERR("failed. PDAF not supported.");
    return SENSOR_FAILURE;
  }

  SLOW("PD window mode from 3A: %d",window->pdaf_sw_window_mode);
  if(window->pdaf_sw_window_mode == DEFAULT_WINDOW) {
    memcpy(window, default_window, sizeof(pdaf_window_configure_t));
  }

  fixed_win = &window->fixed_grid_window_configure;
  float_win = &window->float_window_configure;

  switch (window->pdaf_sw_window_mode) {
  case  FLOAT_WINDOW:
    win_num = float_win->pdaf_number_windows;
    if(win_num > MAX_FLOATING_WIN || win_num == 0) {
      SERR("failed, invalid win_num %d", win_num);
      return SENSOR_FAILURE;
    }
    if (params->type == 1) {
      flex_area_addr = reg_info->flex_area_xsta0_addr;
      for (i = 0; i < win_num; i++) {
        x_sta[i] = float_win->af_float_window[i].pdaf_address_start_hori
          * out_info->x_output;
        y_sta[i] = float_win->af_float_window[i].pdaf_address_start_ver
          * out_info->y_output;
        x_end[i] = float_win->af_float_window[i].pdaf_address_end_hori
          * out_info->x_output;
        y_end[i] = float_win->af_float_window[i].pdaf_address_end_ver
          * out_info->y_output;

        settings.reg_setting_a[settings.size].reg_addr = flex_area_addr;
        settings.reg_setting_a[settings.size].reg_data = x_sta[i] >> 8;
        flex_area_addr++;
        settings.size++;

        settings.reg_setting_a[settings.size].reg_addr = flex_area_addr;
        settings.reg_setting_a[settings.size].reg_data = x_sta[i] & 0xFF;
        flex_area_addr++;
        settings.size++;

        settings.reg_setting_a[settings.size].reg_addr = flex_area_addr;
        settings.reg_setting_a[settings.size].reg_data = y_sta[i] >> 8;
        flex_area_addr++;
        settings.size++;

        settings.reg_setting_a[settings.size].reg_addr = flex_area_addr;
        settings.reg_setting_a[settings.size].reg_data = y_sta[i] & 0xFF;
        flex_area_addr++;
        settings.size++;

        settings.reg_setting_a[settings.size].reg_addr = flex_area_addr;
        settings.reg_setting_a[settings.size].reg_data = x_end[i] >> 8;
        flex_area_addr++;
        settings.size++;

        settings.reg_setting_a[settings.size].reg_addr = flex_area_addr;
        settings.reg_setting_a[settings.size].reg_data = x_end[i] & 0xFF;
        flex_area_addr++;
        settings.size++;

        settings.reg_setting_a[settings.size].reg_addr = flex_area_addr;
        settings.reg_setting_a[settings.size].reg_data = y_end[i] >> 8;
        flex_area_addr++;
        settings.size++;

        settings.reg_setting_a[settings.size].reg_addr = flex_area_addr;
        settings.reg_setting_a[settings.size].reg_data = y_end[i] & 0xFF;
        flex_area_addr++;
        settings.size++;

        /* save current window in sensor ctrl */
        ctrl->s_data->cur_pd_windows[i].x_sta = x_sta[i];
        ctrl->s_data->cur_pd_windows[i].y_sta = y_sta[i];
        ctrl->s_data->cur_pd_windows[i].x_end = x_end[i];
        ctrl->s_data->cur_pd_windows[i].y_end = y_end[i];
      }

      settings.reg_setting_a[settings.size].reg_addr =
        reg_info->area_mode_addr;
      settings.reg_setting_a[settings.size].reg_data =
        reg_info->flex_mode_data;
      settings.size++;

      rc = sensor_write_i2c_setting_array(ctrl, &settings);
      RETURN_FALSE_IF(rc < 0);

      ctrl->s_data->pd_x_win_num = 1;
      ctrl->s_data->pd_y_win_num = win_num;
    } else {
      /* calling event is responsible for matching data size */
      memcpy(&s_data->new_window, window, sizeof(pdaf_window_configure_t));
      s_data->window_updated = TRUE;
    }
  break;
  case FIXED_GRID_WINDOW:
    win_num = fixed_win->window_number_hori * fixed_win->window_number_ver;
    if(win_num > MAX_FIXED_WIN || win_num == 0) {
      SERR("failed, invalid win_num %d", win_num);
      return SENSOR_FAILURE;
    }

    if (params->type == 1) {
      if (win_num == 1) {
        /* Update as 1 floating window*/
        x_sta[0] =
          fixed_win->af_fix_window.pdaf_address_start_hori * out_info->x_output;
        y_sta[0] =
          fixed_win->af_fix_window.pdaf_address_start_ver * out_info->y_output;
        x_end[0] =
          fixed_win->af_fix_window.pdaf_address_end_hori * out_info->x_output;
        y_end[0] =
          fixed_win->af_fix_window.pdaf_address_end_ver * out_info->y_output;

        SLOW("updating float win: x_sta: %d, y_sta: %d, x_end: %d, y_end: %d",
          i, x_sta[0], y_sta[0], x_end[0], y_end[0]);

        settings.size = 8;
        flex_area_addr = reg_info->flex_area_xsta0_addr;

        settings.reg_setting_a[0].reg_addr = flex_area_addr;
        settings.reg_setting_a[0].reg_data = x_sta[0] >> 8;
        flex_area_addr++;

        settings.reg_setting_a[1].reg_addr = flex_area_addr;
        settings.reg_setting_a[1].reg_data = x_sta[0] & 0xFF;
        flex_area_addr++;

        settings.reg_setting_a[2].reg_addr = flex_area_addr;
        settings.reg_setting_a[2].reg_data = y_sta[0] >> 8;
        flex_area_addr++;

        settings.reg_setting_a[3].reg_addr = flex_area_addr;
        settings.reg_setting_a[3].reg_data = y_sta[0] & 0xFF;
        flex_area_addr++;

        settings.reg_setting_a[4].reg_addr = flex_area_addr;
        settings.reg_setting_a[4].reg_data = x_end[0] >> 8;
        flex_area_addr++;

        settings.reg_setting_a[5].reg_addr = flex_area_addr;
        settings.reg_setting_a[5].reg_data = x_end[0] & 0xFF;
        flex_area_addr++;

        settings.reg_setting_a[6].reg_addr = flex_area_addr;
        settings.reg_setting_a[6].reg_data = y_end[0] >> 8;
        flex_area_addr++;

        settings.reg_setting_a[7].reg_addr = flex_area_addr;
        settings.reg_setting_a[7].reg_data = y_end[0] & 0xFF;
        flex_area_addr++;

        settings.reg_setting_a[settings.size].reg_addr =
          reg_info->area_mode_addr;
        settings.reg_setting_a[settings.size].reg_data =
          reg_info->flex_mode_data;
        settings.size++;

        ctrl->s_data->cur_pd_windows[0].x_sta = x_sta[0];
        ctrl->s_data->cur_pd_windows[0].y_sta = y_sta[0];
        ctrl->s_data->cur_pd_windows[0].x_end = x_end[0];
        ctrl->s_data->cur_pd_windows[0].y_end = y_end[0];
      } else {
      /* Fixed grid mode: 3A responsible to set size that sensor can support */
        settings.reg_setting_a[settings.size].reg_data =
          reg_info->fixed_mode_data;
        settings.reg_setting_a[settings.size].reg_addr =
          reg_info->area_mode_addr;
        settings.size++;

        x_offset = out_info->x_output *
          fixed_win->af_fix_window.pdaf_address_start_hori;
        y_offset = out_info->y_output *
          fixed_win->af_fix_window.pdaf_address_start_ver;
        w = out_info->x_output *
          (fixed_win->af_fix_window.pdaf_address_end_hori -
          fixed_win->af_fix_window.pdaf_address_start_hori) /
          fixed_win->window_number_hori;
        h = out_info->y_output *
          (fixed_win->af_fix_window.pdaf_address_end_ver -
          fixed_win->af_fix_window.pdaf_address_start_ver) /
          fixed_win->window_number_ver;

        SLOW("pdating fixed offset: %d, %d, w: %d, h: %d",
          x_offset, y_offset, w, h);

        settings.reg_setting_a[settings.size].reg_addr =
          reg_info->pd_area_x_offset_addr;
        settings.reg_setting_a[settings.size].reg_data = x_offset >> 8;
        settings.size++;

        settings.reg_setting_a[settings.size].reg_addr =
          reg_info->pd_area_x_offset_addr + 1;
        settings.reg_setting_a[settings.size].reg_data = x_offset & 0xFF;
        settings.size++;

        settings.reg_setting_a[settings.size].reg_addr =
          reg_info->pd_area_y_offset_addr;
        settings.reg_setting_a[settings.size].reg_data = y_offset >> 8;
        settings.size++;

        settings.reg_setting_a[settings.size].reg_addr =
          reg_info->pd_area_y_offset_addr + 1;
        settings.reg_setting_a[settings.size].reg_data = y_offset & 0xFF;
        settings.size++;

        settings.reg_setting_a[settings.size].reg_addr =
          reg_info->pd_area_width_addr;
        settings.reg_setting_a[settings.size].reg_data = w >> 8;
        settings.size++;

        settings.reg_setting_a[settings.size].reg_addr =
          reg_info->pd_area_width_addr + 1;
        settings.reg_setting_a[settings.size].reg_data = w & 0xFF;
        settings.size++;

        settings.reg_setting_a[settings.size].reg_addr =
          reg_info->pd_area_height_addr;
        settings.reg_setting_a[settings.size].reg_data = h >> 8;
        settings.size++;

        settings.reg_setting_a[settings.size].reg_addr =
          reg_info->pd_area_height_addr + 1;
        settings.reg_setting_a[settings.size].reg_data = h & 0xFF;
        settings.size++;

        for(j = 0; j < ctrl->s_data->pd_y_win_num; j++)
          for(i = 0; i < ctrl->s_data->pd_x_win_num; i++) {
            ctrl->s_data->cur_pd_windows[i + pd_win_x*j].x_sta = x_offset + w * i;
            ctrl->s_data->cur_pd_windows[i + pd_win_x*j].x_end = x_offset + w * i
              + w - 1;
            ctrl->s_data->cur_pd_windows[i + pd_win_x*j].y_sta = y_offset + h * j;
            ctrl->s_data->cur_pd_windows[i + pd_win_x*j].y_end = y_offset + h * j
              + h - 1;
        }
      }
      rc = sensor_write_i2c_setting_array(ctrl, &settings);
        RETURN_FALSE_IF(rc < 0);

      /* save current window in sensor ctrl */
      pd_win_x = ctrl->s_data->pd_x_win_num = fixed_win->window_number_hori;
      pd_win_y = ctrl->s_data->pd_y_win_num = fixed_win->window_number_ver;

    } else {
      memcpy(&s_data->new_window, window, sizeof(pdaf_window_configure_t));
      s_data->window_updated = TRUE;
    }
  break;
  default:
    SERR("unsupported window type");
    return SENSOR_FAILURE;
  }

  for(i = 0; i < settings.size; i++) {
    SLOW("addr 0x%x data 0x%x",
      settings.reg_setting_a[i].reg_addr, settings.reg_setting_a[i].reg_data);
  }

  for(i = 0; i < win_num; i++) {
    SLOW("current PD windows: %d %d %d %d",
      ctrl->s_data->cur_pd_windows[i].x_sta,
      ctrl->s_data->cur_pd_windows[i].y_sta,
      ctrl->s_data->cur_pd_windows[i].x_end,
      ctrl->s_data->cur_pd_windows[i].y_end);
  }

  return rc;
}

boolean pdaf_set_buf_data_type(void *sctrl, void *data){
  int32_t                         rc = SENSOR_SUCCESS;
  sensor_ctrl_t                   *ctrl;
  sensor_lib_t                    *sensor_lib;
  pdaf_lib_t                      *params;
  int16_t                         cur_res;
  RETURN_ERROR_ON_NULL(sctrl);

  ctrl       = (sensor_ctrl_t *)sctrl;
  sensor_lib = ctrl->lib_params->sensor_lib_ptr;
  params     = &sensor_lib->pdaf_config;
  cur_res    = ctrl->s_data->cur_res;

  params->buffer_block_pattern_info[cur_res].buffer_data_type =
    *(pdaf_buffer_data_type_t *)data;

  return rc;
}

boolean pdaf_init(void *sctrl, void *data){
  int32_t                         rc = SENSOR_SUCCESS;
  pdaf_camif_init_param_t         input_camif;
  pdaf_init_param_t               input;
  pdaf_init_info_t                *s_pdaf = NULL;
  sensor_ctrl_t                   *ctrl;
  pdaf_lib_t                      *params;
  sensor_lib_t                    *sensor_lib;
  unsigned int                    i;
  int16_t                         cur_res;
  struct sensor_lib_out_info_t    *out_info;
  sensor_custom_API_t             *sensor_custom_API;
  sensor_data_t                   *s_data;
  PdLibSensorRegData_t            *reg_data;
  PdLibSensorSetting_t            *reg_setting;

  RETURN_ERROR_ON_NULL(sctrl);
  RETURN_ERROR_ON_NULL(data);

  ctrl       = (sensor_ctrl_t *)sctrl;
  reg_data   = &ctrl->s_data->reg_data;
  reg_setting = &ctrl->s_data->reg_setting;
  s_pdaf     = (pdaf_init_info_t *)data;
  sensor_lib = ctrl->lib_params->sensor_lib_ptr;
  params     = &sensor_lib->pdaf_config;
  sensor_custom_API = &ctrl->lib_params->sensor_custom_API;
  cur_res           = ctrl->s_data->cur_res;
  out_info        = &sensor_lib->out_info_array.out_info[ctrl->s_data->cur_res];
  s_data     = ctrl->s_data;

  if(!out_info->is_pdaf_supported)
   return rc;

  if (ctrl->is_pdaf_initiated)
   return rc;

  /* for thrid party OEM */
  if (sensor_custom_API->pdlib_init)
    rc = sensor_custom_API->pdlib_init(params, s_pdaf);
  else if(params->vendor_id == QTI)
  /* for qti pdaf solution */
  {
    /* qti solution based on valid pd calibration data */
    RETURN_ERROR_ON_NULL(s_pdaf->pdaf_cal_data);

    memset(&input_camif, 0, sizeof(pdaf_camif_init_param_t));
    memset(&input, 0, sizeof(pdaf_init_param_t));
    memcpy(&s_data->curr_window, &params->window_configure, sizeof(pdaf_window_configure_t));

    PdLibInterpretRegData(reg_data, reg_setting,
      sensor_lib->res_settings_array.reg_settings[cur_res].reg_setting_a,
      sensor_lib->res_settings_array.reg_settings[cur_res].size,
      &params->pdaf_reg_info);

    pdaf_get_native_pattern(sctrl, &input_camif.sensor_native_info);

    pdaf_rm_pd_offset(&input_camif.sensor_native_info);

    /* pack buffer pattern */
    /* T3 buffer pattern is depend by isp configuration */
    switch(params->buffer_block_pattern_info[cur_res].buffer_type)
    {
     case PDAF_BUFFER_FLAG_SPARSE:{
      if (!s_pdaf->isp_config)
      {
       SERR("invalid T3 buffer pattern");
       return SENSOR_FAILURE;
      }
      /* T3 overwrite the buffer info for camif configuration */
      memcpy(&input_camif.buffer_data_info,
             s_pdaf->isp_config,
             sizeof(input_camif.buffer_data_info));
      input_camif.buffer_data_info.buffer_data_type =
        params->buffer_block_pattern_info[cur_res].buffer_data_type;
      input_camif.buffer_data_info.buffer_type      =
        params->buffer_block_pattern_info[cur_res].buffer_type;
     }
     break;
     case PDAF_BUFFER_FLAG_SEQUENTIAL_LINE:
      /* T2 buffer pattern is self-contained in header description */
      input_camif.buffer_data_info.buffer_block_pattern_left =
       params->buffer_block_pattern_info[cur_res].block_pattern;
      input_camif.buffer_data_info.camif_left_buffer_stride  =
       params->buffer_block_pattern_info[cur_res].stride;
      input_camif.buffer_data_info.buffer_data_type          =
        params->buffer_block_pattern_info[cur_res].buffer_data_type;
      input_camif.buffer_data_info.buffer_type               =
        params->buffer_block_pattern_info[cur_res].buffer_type;
      input_camif.buffer_data_info.camif_left_buffer_width   =
        input_camif.sensor_native_info.block_count_horizontal *
        input_camif.buffer_data_info.buffer_block_pattern_left.block_dim.width;
      input_camif.buffer_data_info.camif_buffer_height       =
        input_camif.sensor_native_info.block_count_vertical *
        input_camif.buffer_data_info.buffer_block_pattern_left.block_dim.height;
      break;
     case PDAF_BUFFER_FLAG_INTERLEAVE_CHANNEL:
      /* T2 2PD */
      input_camif.buffer_data_info.camif_left_buffer_stride  =
        params->buffer_block_pattern_info[cur_res].stride;
      input_camif.buffer_data_info.buffer_data_type          =
        params->buffer_block_pattern_info[cur_res].buffer_data_type;
      input_camif.buffer_data_info.buffer_type               =
        params->buffer_block_pattern_info[cur_res].buffer_type;
      input_camif.buffer_data_info.camif_left_buffer_width   =
        params->sensor_native_pattern_info[cur_res].block_count_horizontal;
      input_camif.buffer_data_info.camif_buffer_height       =
        params->sensor_native_pattern_info[cur_res].block_count_vertical;
      break;
     default:
      SHIGH("unsupported buffer buffer type");
    }

    ctrl->pd_camif_handle = PDAF_CAMIF_init(&input_camif);

    /* fill in QC pd input */
    pdaf_get_native_pattern(sctrl, &input.native_pattern_info);

    pdaf_rm_pd_offset(&input.native_pattern_info);
    input.p_calibration_para   = s_pdaf->pdaf_cal_data;
    input.macro_dac = s_pdaf->macro_dac;
    input.infinity_dac = s_pdaf->infinity_dac;
    input.actuator_sensitivity = s_pdaf->actuator_sensitivity;
    input.cur_logical_lens_pos = s_pdaf->cur_logical_lens_pos;
    memcpy(&input.buffer_data_info, &input_camif.buffer_data_info,
      sizeof(input.buffer_data_info));

    /* not the exactly data in chromatix
       estimation of the module black level
      */
    input.black_level          = params->black_level;
    input.cali_version         = params->cali_version;

    switch(sensor_lib->sensor_output.raw_output) {
      case SENSOR_8_BIT_DIRECT:
        input.pixel_depth = 8;
      break;
      case SENSOR_10_BIT_DIRECT:
        input.pixel_depth = 10;
      break;
      case SENSOR_12_BIT_DIRECT:
        input.pixel_depth = 12;
      break;
      case SENSOR_14_BIT_DIRECT:
        input.pixel_depth = 14;
      break;
    }

    if(sensor_lib->out_info_array.out_info[cur_res].mode == SENSOR_HDR_MODE ||
      sensor_lib->out_info_array.out_info[cur_res].mode == SENSOR_RAW_HDR_MODE)
      input.is_hdr = 1;

    input.PD_pix_overflow_threshold= params->PD_pix_overflow_threshold;
    input.pixel_order_type = params->pixel_order_type;

    ctrl->pd_handle = PDAF_PhaseDetection_init(&input);

    if (!ctrl->pd_handle ||!ctrl->pd_camif_handle)
    {
     SERR("PDAF_init failed");
     return SENSOR_FAILURE;
    }
    ctrl->is_pdaf_initiated = true;
  }
  return rc;
}

boolean pdaf_calc_defocus(void *sctrl, void *data){
  int32_t                      rc = SENSOR_SUCCESS;
  pdaf_lib_t                   *params = NULL;
  sensor_ctrl_t                *ctrl = (sensor_ctrl_t *)sctrl;
  pdaf_camif_param_t           input_camif;
  pdaf_param_t                 input;
  pdaf_camif_output_data_t     output_camif;
  pdaf_output_data_t           output;
  pdaf_params_t                *s_pdaf = NULL;
  sensor_custom_API_t          *sensor_custom_API = NULL;
  struct sensor_lib_out_info_t  *out_info;
  sensor_lib_t                  *sensor_lib;
  sensor_data_t                *s_data;

  RETURN_ERROR_ON_NULL(ctrl);
  RETURN_ERROR_ON_NULL(data);
  s_pdaf = (pdaf_params_t *)data;
  sensor_lib = ctrl->lib_params->sensor_lib_ptr;
  params = &sensor_lib->pdaf_config;
  sensor_custom_API = &ctrl->lib_params->sensor_custom_API;
  out_info = &sensor_lib->out_info_array.out_info[ctrl->s_data->cur_res];
  s_data = ctrl->s_data;
  if(!out_info->is_pdaf_supported)
   return rc;

  if(params->type == 1) {
    pdaf_get_defocus_t1(ctrl,s_pdaf);
    return rc;
  }

  output.processing_result = PDAF_RESULT_PARTIAL;
  /* for thrid party OEM */
  if (sensor_custom_API->pdlib_get_defocus)
    rc = sensor_custom_API->pdlib_get_defocus(s_pdaf, &output);
  else if (params->vendor_id == QTI)
  /* for qti pdaf solution */
  {
    /* abort if any of pd handle is null */
    if ( !ctrl->pd_camif_handle || ! ctrl->pd_handle)
      return SENSOR_FAILURE;

    if (s_data->window_updated == TRUE) {
      memcpy(&s_data->curr_window, &s_data->new_window,
        sizeof(pdaf_window_configure_t));
      s_data->window_updated = FALSE;
    }

    memset(&input, 0, sizeof(input));
    memset(&input_camif, 0, sizeof(input_camif));
    memset(&output_camif, 0, sizeof(output_camif));
    memset(&output, 0, sizeof(output));
    /* isp always just sent one buffer */
    input_camif.p_left = s_pdaf->pd_stats;
    input_camif.p_right = NULL;
    input_camif.cur_logical_lens_pos = s_pdaf->cur_logical_lens_pos;
    memcpy(&input_camif.window_configure, &s_data->curr_window,
         sizeof(pdaf_window_configure_t));
    rc = PDAF_CAMIF_getPDAF(ctrl->pd_camif_handle, &input_camif, &output_camif);
    if (rc != PDAF_LIB_OK)
    {
     SERR("PDAF_CAMIF_getPDAF failed");
     return -rc;
    }
    input.camif_out = output_camif;
    /* do not have any info for ROI */
    /* current gain is applied on next frame
       prev_gain takes effect on current frame
    */
    input.image_analog_gain = ctrl->s_data->prev_gain;
    input.defocus_confidence_th = params->defocus_confidence_th;
    input.fps = ctrl->s_data->realtime_fps;
    input.lens_position= s_pdaf->cur_lens_pos;
    input.cur_logical_lens_pos = s_pdaf->cur_logical_lens_pos;
    sensor_get_integration_time(ctrl, &input.integration_time);

    SLOW("sensor gain: %d, fps: %f, lens position: %d, integration time: %f",
      input.image_analog_gain, input.fps, input.lens_position,
      input.integration_time);
    memcpy(&input.window_configure, &s_data->curr_window,
         sizeof(pdaf_window_configure_t));
    unsigned short num_of_lines = s_pdaf->num_of_valid_data_wrt_camif;
    rc = PDAF_PhaseDetection(ctrl->pd_handle, &input, num_of_lines, &output);
    if ( rc != PDAF_LIB_OK)
    {
     SERR("PDAF_PhaseDetection failed");
     return -rc;
    }
  }
  else
  {
     SERR("Invalid Vendor_id");
     return SENSOR_FAILURE;
  }

  /*same way: populate defocus value back */
  memcpy(s_pdaf->defocus, output.defocus, sizeof(s_pdaf->defocus));
  s_pdaf->is_peripheral_valid = output.is_peripheral_valid;
  memcpy(s_pdaf->peripheral, output.result_peripheral, sizeof(s_pdaf->peripheral));

  s_pdaf->is_single_valid = output.is_single_valid;
  memcpy(s_pdaf->single, output.result_single, sizeof(s_pdaf->single));

  s_pdaf->stat_process_result = output.processing_result;
  SLOW("output.processing_result %d", output.processing_result);

  switch(params->window_configure.pdaf_sw_window_mode){
   case FLOAT_WINDOW:
    /*
      do not set feedback status to be ture
      af hal will not connect to hybrid-framework-pdaf-algo
    */
    SHIGH("unsupported window configuration");
    break;
   case FIXED_GRID_WINDOW:
     s_pdaf->x_win_num =
      s_data->curr_window.fixed_grid_window_configure.window_number_hori;
     s_pdaf->y_win_num =
      s_data->curr_window.fixed_grid_window_configure.window_number_ver;
     s_pdaf->x_offset =
      s_data->curr_window.fixed_grid_window_configure.af_fix_window.pdaf_address_start_hori;
     s_pdaf->y_offset =
      s_data->curr_window.fixed_grid_window_configure.af_fix_window.pdaf_address_start_ver;
     s_pdaf->width =
        s_data->curr_window.fixed_grid_window_configure.af_fix_window.pdaf_address_end_hori -
        s_data->curr_window.fixed_grid_window_configure.af_fix_window.pdaf_address_start_hori;
     s_pdaf->height =
        s_data->curr_window.fixed_grid_window_configure.af_fix_window.pdaf_address_end_ver -
        s_data->curr_window.fixed_grid_window_configure.af_fix_window.pdaf_address_start_ver;
     s_pdaf->status = TRUE;
     break;
   default:
    SERR("wrong such pd window configuration");
    break;
  }

  return rc;
}

boolean pdaf_get_type(void *sctrl, void *data){
  int32_t                      rc = SENSOR_SUCCESS;
  pdaf_lib_t                   *params = NULL;
  sensor_ctrl_t                *ctrl = (sensor_ctrl_t *)sctrl;
  int32_t                      *buffer_type = (int32_t *)data;
  sensor_custom_API_t          *sensor_custom_API = NULL;
  struct sensor_lib_out_info_array *out_info_array_ptr = NULL;
  RETURN_ON_NULL(sctrl);
  RETURN_ON_NULL(data);
  sensor_custom_API = &ctrl->lib_params->sensor_custom_API;
  params = &ctrl->lib_params->sensor_lib_ptr->pdaf_config;
  out_info_array_ptr = &ctrl->lib_params->sensor_lib_ptr->out_info_array;

  /* default status as invalid */
  *buffer_type = PDAF_BUFFER_FLAG_INVALID;
  if (out_info_array_ptr->out_info[ctrl->s_data->cur_res].is_pdaf_supported)
   *buffer_type =
     params->buffer_block_pattern_info[ctrl->s_data->cur_res].buffer_type;
  SHIGH("buffer_type %d", *buffer_type);
  return rc;
}

/** pdaf_parse_pd:
 *    @arg1: custom
 *    @arg2: pdaf_params_t
 *
 *  Return: 0 on success
 *    -1 on failure
 *
 *  This function parses phase difference data from sensor HW,
 *  containing flexible window flag, fixed window mode,
 *  confidence level and phase difference
 **/

int32_t pdaf_parse_pd(sensor_stats_format_t format,
 pdaf_params_t *s_pdaf)
{
  uint8_t    *buf;
  uint32_t     i = 0;
  uint8_t    flex_win_flag = 0;
  uint8_t    win_mode = 0;
  uint32_t     win_num;
  int32_t    sign_ext;
  int32_t    phase_diff_raw;

  RETURN_ERROR_ON_NULL(s_pdaf);
  RETURN_ERROR_ON_NULL(s_pdaf->pd_stats);

  buf = (uint8_t *)s_pdaf->pd_stats;

  if(format == SENSOR_STATS_RAW10_8B_CONF_10B_PD) {
    flex_win_flag = buf[0];
    win_mode = buf[1] >> 6; /*window mode 0: 16x12, 1: 8x6, 2: flexible */

    SLOW("PDAF parsing PD stats: win_flag: %d, area_mode: %d",
    flex_win_flag, win_mode);

    if(win_mode == 0)
      win_num = 192;
    else if(win_mode == 1)
      win_num= 48;
    else
      win_num = 8; /* max flexible window */

    buf += 5;

    for(i = 0;i < win_num;i++) {
    s_pdaf->defocus[i].df_conf_level = buf[0];
    phase_diff_raw = buf[1] << 2 | buf[2] >> 6;
    sign_ext = (buf[1] & 0x80) ? 0xFFFFFC00 : 0;/* s5.4: 1st bit is sign bit */
    phase_diff_raw |= sign_ext;
    s_pdaf->defocus[i].phase_diff = (float)phase_diff_raw / 16.0f;
    SLOW("PDAF: window %d, conf. level:%d, phase diff: %f", i,
    s_pdaf->defocus[i].df_conf_level,
    s_pdaf->defocus[i].phase_diff);

    if(win_num == 48 && (i % 8) == 7) {
    /* HW limitation: need to skip alternate rows (8 windows) */
    buf += 45;
    }
    else
    buf += 5;
    }
  } else if(format == SENSOR_STATS_RAW10_11B_CONF_11B_PD) {
    flex_win_flag = buf[0];
    win_mode = buf[1] >> 6; /*window mode 0: 16x12, 1: 8x6, 2: flexible */

    SLOW("PDAF parsing PD stats: win_flag: %d, area_mode: %d",
    flex_win_flag, win_mode);

    if(win_mode == 0)
      win_num = 192;
    else if(win_mode == 1)
      win_num= 48;
    else
      win_num = 8; /* max flexible window */

    buf += 10;

    for(i = 0;i < win_num;i++) {
      s_pdaf->defocus[i].df_conf_level = buf[0] << 3 | buf[1] >> 5;
      phase_diff_raw = (buf[1] & 0x1F) << 6 | buf[2] >> 2;
      /* s6.4 sign extension: 11 bit to 32 bit */
      phase_diff_raw = (phase_diff_raw << 21) >> 21;

      s_pdaf->defocus[i].phase_diff = (float)phase_diff_raw / 16.0f;
      SHIGH("PDAF: window %d, conf. level:%d, phase diff: %f", i,
      s_pdaf->defocus[i].df_conf_level,
      s_pdaf->defocus[i].phase_diff);

      if(win_num == 48 && (i % 8) == 7) {
        /* HW limitation: need to skip alternate rows (8 windows) */
        buf += 45;
      } else
        buf += 5;
    }
  } else {
    SHIGH("unsupported PD data type");
  }

  return 0;
}

/** pdaf_get_defocus_with_pd_data:
 *    @arg1: PdLibInputData_t
 *    @arg2: PdLibOutputData_t
 *
 *  Return: 0 on success
 *    -1 on failure
 *
 *  This function gets defocus with PD data from sensor HW
 *
 **/
signed long pdaf_get_defocus_with_pd_data(void *arg1, void *arg2)
{
  PdLibInputData_t *input = (PdLibInputData_t *)arg1;
  PdLibOutputData_t *output = (PdLibOutputData_t *)arg2;
  PdLibPoint_t p0 = {0, 0}, p1 = {0, 0}, p2 = {0, 0}, p3 = {0, 0}, p_tg = {0, 0};
  unsigned int i = 0, x_sta_idx = 0, x_end_idx = 0, y_sta_idx = 0, y_end_idx = 0;
  int d0 = 0, d1 = 0, d2 = 0, d3 = 0, d_tg = 0;
  int temp0 = 0, temp1 = 0;
  int pd_area_x = 0, pd_area_y = 0, pd_offset_x = 0, pd_offset_y = 0;
  unsigned long threshold = 0;

  RETURN_ERR_ON_NULL(input, -1);
  RETURN_ERR_ON_NULL(output, -1);
  RETURN_ERR_ON_NULL(input->p_SlopeData, -1);
  RETURN_ERR_ON_NULL(input->p_OffsetData, -1);
  RETURN_ERR_ON_NULL(input->p_XAddressKnotSlopeOffset, -1);
  RETURN_ERR_ON_NULL(input->p_YAddressKnotSlopeOffset, -1);

  /* Find 4 adjacent data points;
   * data points is at the center of each grid.
   * Define p0 as the nearest point to p_tg
   *    p0 --------- p1
   *    |             |
   *    |    p_tg     |
   *    |             |
   *    p2 --------- p3
   */
  p_tg.x = (input->XAddressOfWindowStart + input->XAddressOfWindowEnd) / 2;
  p_tg.y = (input->YAddressOfWindowStart + input->YAddressOfWindowEnd) / 2;

  pd_area_x = input->p_XAddressKnotSlopeOffset[1]
    - input->p_XAddressKnotSlopeOffset[0];
  pd_area_y = input->p_YAddressKnotSlopeOffset[1]
    - input->p_YAddressKnotSlopeOffset[0];
  pd_offset_x = input->p_XAddressKnotSlopeOffset[0];
  pd_offset_y = input->p_YAddressKnotSlopeOffset[0];


  x_sta_idx = (p_tg.x - pd_offset_x) / pd_area_x;
  y_sta_idx = (p_tg.y - pd_offset_y) / pd_area_y;

  p0.x = input->p_XAddressKnotSlopeOffset[x_sta_idx] + pd_area_x / 2;
  p0.y = input->p_YAddressKnotSlopeOffset[y_sta_idx] + pd_area_y / 2;
  d0 = input->p_SlopeData[x_sta_idx + input->XKnotNumSlopeOffset * y_sta_idx];

  if(abs(p_tg.x - p0.x) < pd_area_x*0.01 &&
    abs(p_tg.y - p0.y) < pd_area_y*0.01) {
    /* target point at avaible data point, allow 1% diff */
    d_tg = d0;
  } else {
    if(p_tg.x < p0.x && x_sta_idx > 0)
      x_end_idx = x_sta_idx - 1;
    else if(p_tg.x > p0.x &&
      x_sta_idx < (unsigned)(input->XKnotNumSlopeOffset - 1))
      x_end_idx = x_sta_idx + 1;
    else
      x_end_idx = x_sta_idx;

    if(p_tg.y < p0.y && y_sta_idx > 0)
      y_end_idx = y_sta_idx - 1;
    else if(p_tg.y > p0.y &&
      y_sta_idx < (unsigned)(input->YKnotNumSlopeOffset - 1))
      y_end_idx = y_sta_idx + 1;
    else
      y_end_idx = y_sta_idx;

    if(x_sta_idx >= input->XKnotNumSlopeOffset ||
      x_end_idx >= input->XKnotNumSlopeOffset ||
      y_sta_idx >= input->YKnotNumSlopeOffset ||
      y_end_idx >= input->YKnotNumSlopeOffset) {
      SERR("failed: invalid PD window");
      return -1;
    }

    p2.x =
      input->p_XAddressKnotSlopeOffset[x_sta_idx] + pd_area_x / 2;
    p1.x = p3.x =
      input->p_XAddressKnotSlopeOffset[x_end_idx] + pd_area_x / 2;
    p1.y =
      input->p_YAddressKnotSlopeOffset[y_sta_idx] + pd_area_y / 2;
    p2.y = p3.y =
      input->p_YAddressKnotSlopeOffset[y_end_idx] + pd_area_y / 2;

    d0 = input->p_SlopeData[x_sta_idx + input->XKnotNumSlopeOffset*y_sta_idx];
    d1 = input->p_SlopeData[x_end_idx + input->XKnotNumSlopeOffset*y_sta_idx];
    d2 = input->p_SlopeData[x_sta_idx + input->XKnotNumSlopeOffset*y_end_idx];
    d3 = input->p_SlopeData[x_end_idx + input->XKnotNumSlopeOffset*y_end_idx];

    /* Bilinear interpolation */
    temp0 = INTERPOLATION_RATIO(p0.x, p1.x, p_tg.x) * d1 +
      INTERPOLATION_RATIO(p1.x, p0.x, p_tg.x) * d0;

    temp1 = INTERPOLATION_RATIO(p0.x, p1.x, p_tg.x) * d3 +
      INTERPOLATION_RATIO(p1.x, p0.x, p_tg.x) * d2;

    d_tg = INTERPOLATION_RATIO(p0.y, p2.y, p_tg.y) * temp1 +
      INTERPOLATION_RATIO(p2.y, p0.y, p_tg.y) * temp0;
  }
  /* Give defocus and confidence level */
  if(input->p_DefocusOKNGThrLine)
    threshold = input->p_DefocusOKNGThrLine[0].p_Confidence[0];

  if(threshold == 0)
    threshold = 255;

  output->Defocus = input->PhaseDifference * d_tg;
  output->DefocusConfidenceLevel = 1024 * input->ConfidenceLevel / threshold;
  output->DefocusConfidence = (input->ConfidenceLevel < threshold) ? 0 : 1;
  output->PhaseDifference = input->PhaseDifference;

  SLOW("x idx: %d %d, y idx: %d %d",x_sta_idx, x_end_idx, y_sta_idx, y_end_idx);
  SLOW("p0: %d %d, p1: %d %d, p2: %d %d, p3: %d %d, p_tg: %d %d",
    p0.x, p0.y,p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, p_tg.x, p_tg.y);
  SLOW("4 data %f %f %f %f, DCC: %f",
    d0/1024.0, d1/1024.0,d2/1024.0,d3/1024.0, d_tg/1024.0);

  return 0;
}

