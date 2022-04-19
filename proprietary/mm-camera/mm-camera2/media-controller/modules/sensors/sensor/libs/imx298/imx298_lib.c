/*
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <stdio.h>
#include <string.h>
#include "eeprom_lib.h"
#include "debug_lib.h"
#include "pdaf_lib.h"
#include "imx298_lib.h"

#define VIDEO_HDR_LUMA_MASK_RATIO_1  0x0F
#define VIDEO_HDR_EXPOSURE_RATIO_CORRECTION_1  2

/* For DensityOfPhasePix */
#define D_PD_LIB_DENSITY_SENS_MODE0            (2304)
/* For AdjCoeffSlope */
#define D_PD_LIB_SLOPE_ADJ_COEFF_SENS_MODE0    (2304)

/* Marcos for PDAF threshold line */
#define POINT_NUM 4
#define X_KNOT 8
#define Y_KNOT 6

/* pick up a bit field.
  When 'MSBpos' is the same as 'LSBpos', single bit is extracted. */
static unsigned int bitPick(unsigned int x, int MSBpos, int LSBpos)
{
  return (x >> LSBpos) & (~(~0 << (MSBpos - LSBpos + 1)));
}

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

static int PdLibInterpretRegData(PdLibSensorRegData_t *p_regData,
  PdLibSensorSetting_t *p_setting,
  struct camera_i2c_reg_array reg_array[], int size, int res)
{
  int ret = D_PD_LIB_REGDATA_IS_OK;
  int i = 0;
  static int saved_res = -1;

  SLOW("saved res: %d, cur res: %d", saved_res, res);
  if(saved_res == res)
    return ret;

  saved_res = res;

  /* Load data */
  p_regData->orientation = find_reg_data(reg_array, size, 0x0101);
  p_regData->hdr_en = find_reg_data(reg_array, size, 0x0220);
  p_regData->hdr_binning = find_reg_data(reg_array, size, 0x0221);

  p_regData->x_add_sta_h = find_reg_data(reg_array, size, 0x0344);
  p_regData->x_add_sta_l = find_reg_data(reg_array, size, 0x0345);
  p_regData->y_add_sta_h = find_reg_data(reg_array, size, 0x0346);
  p_regData->y_add_sta_l = find_reg_data(reg_array, size, 0x0347);
  p_regData->x_add_end_h = find_reg_data(reg_array, size, 0x0348);
  p_regData->x_add_end_l = find_reg_data(reg_array, size, 0x0349);
  p_regData->y_add_end_h = find_reg_data(reg_array, size, 0x034A);
  p_regData->y_add_end_l = find_reg_data(reg_array, size, 0x034B);
  p_regData->x_out_size_h = find_reg_data(reg_array, size, 0x034C);
  p_regData->x_out_size_l = find_reg_data(reg_array, size, 0x034D);
  p_regData->y_out_size_h = find_reg_data(reg_array, size, 0x034E);
  p_regData->y_out_size_l = find_reg_data(reg_array, size, 0x034F);

  p_regData->x_evn_inc = find_reg_data(reg_array, size, 0x0381);
  p_regData->x_odd_inc = find_reg_data(reg_array, size, 0x0383);
  p_regData->y_evn_inc = find_reg_data(reg_array, size, 0x0385);
  p_regData->y_odd_inc = find_reg_data(reg_array, size, 0x0387);

  p_regData->scale_mode = find_reg_data(reg_array, size, 0x0401);
  p_regData->scale_m_h = find_reg_data(reg_array, size, 0x0404);
  p_regData->scale_m_l = find_reg_data(reg_array, size, 0x0405);
  p_regData->dig_crop_x_offset_h = find_reg_data(reg_array, size, 0x0408);
  p_regData->dig_crop_x_offset_l = find_reg_data(reg_array, size, 0x0409);
  p_regData->dig_crop_y_offset_h = find_reg_data(reg_array, size, 0x040A);
  p_regData->dig_crop_y_offset_l = find_reg_data(reg_array, size, 0x040B);
  p_regData->dig_crop_image_width_h = find_reg_data(reg_array, size, 0x040C);
  p_regData->dig_crop_image_width_l = find_reg_data(reg_array, size, 0x040D);
  p_regData->dig_crop_image_height_h = find_reg_data(reg_array, size, 0x040E);
  p_regData->dig_crop_image_height_l = find_reg_data(reg_array, size, 0x040F);

  p_regData->binning_mode = find_reg_data(reg_array, size, 0x0900);
  p_regData->binning_type = find_reg_data(reg_array, size, 0x0901);

  p_regData->pd_offset_x_h = find_reg_data(reg_array, size, 0x3108);
  p_regData->pd_offset_x_l = find_reg_data(reg_array, size, 0x3109);
  p_regData->pd_offset_y_h = find_reg_data(reg_array, size, 0x310a);
  p_regData->pd_offset_y_l = find_reg_data(reg_array, size, 0x310b);
  p_regData->pd_area_w_h = find_reg_data(reg_array, size, 0x310c);
  p_regData->pd_area_w_l = find_reg_data(reg_array, size, 0x310d);
  p_regData->pd_area_h_h = find_reg_data(reg_array, size, 0x310e);
  p_regData->pd_area_h_l = find_reg_data(reg_array, size, 0x310f);

  for(i = 0; i < PD_FLEX_WIN_NUM*4*2; i++) {
    p_regData->reg_addr_flex_win[i] = find_reg_data(reg_array, size, 0x3110 + i);
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

  for(i = 0;i < PD_FLEX_WIN_NUM*4;i++) {
    p_setting->flex_win_addr[i] =
     (bitPick(p_regData->reg_addr_flex_win[i * 2], 4, 0) << 8) +
     bitPick(p_regData->reg_addr_flex_win[i * 2 + 1], 7, 0);
  }

  SLOW("x_add_sta: %d, y_add_sta: %d, x_add_end: %d, x_add_end: %d",
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

PdLibPoint_t PdLibTransOutputPointToPdafPoint(PdLibPoint_t onOutImage,
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
    case 1:
        mag_scaleX = p_setting->scale_m;
        mag_scaleY = 16;
        break;
    case 2:
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
static void convert_pd_win_addr(PdLibSensorSetting_t *p_setting,
  unsigned short *p_x_sta, unsigned short *p_y_sta,
  unsigned short *p_x_end, unsigned short *p_y_end)
{
  PdLibPoint_t p1 = {*p_x_sta, *p_y_sta};
  PdLibPoint_t p2 = {*p_x_end, *p_y_end};
  PdLibPoint_t ret_p1 = {0, 0};
  PdLibPoint_t ret_p2 = {0, 0};

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


int32_t imx298_pdaf_calculate_defocus
  (void * arg1, signed long (*get_defocus)(void *input, void *output))
{
  pdaf_params_t               *s_pdaf = (pdaf_params_t *)arg1;
  int32_t                      rc = 0;
  int32_t                      i = 0;
  int32_t                      j = 0;
  int32_t                      cal_data_present = 0;
  struct sensor_lib_out_info_t out_info;
  DefocusOKNGThrLine_t         threshold[X_KNOT*Y_KNOT];
  unsigned long                conf[POINT_NUM];
  unsigned long                analog_gain[POINT_NUM];
  unsigned short               x_knot_addr[X_KNOT];
  unsigned short               y_knot_addr[Y_KNOT];
  pdafcalib_data_t             *dcc;
  PdLibInputData_t             input;
  PdLibOutputData_t            output;
  int32_t                      pd_area_x_offset = PD_AREA_X_OFFSET;
  int32_t                      pd_area_y_offset = PD_AREA_Y_OFFSET;
  int32_t                      pd_area_width = PD_AREA_WIDTH;
  int32_t                      pd_area_height = PD_AREA_HEIGHT;
  int32_t                      pd_win_x = PD_WIN_X;
  int32_t                      pd_win_y = PD_WIN_Y;
  signed long                  OffsetData[8*6] = {0};
  signed long                  SlopeData[8*6] = {0};

  static PdLibSensorRegData_t    regdata;
  static PdLibSensorSetting_t    settings;

  RETURN_ERROR_ON_NULL(s_pdaf);

  s_pdaf->status = FALSE;

  SLOW("PDAF calculate defocus, cur_res:%d",s_pdaf->cur_res);
  out_info = sensor_lib_ptr.out_info_array.out_info[s_pdaf->cur_res];

  if(!out_info.is_pdaf_supported) {
    SERR("PDAF not supported for this resolution");
    return 0;
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
  input.XSizeOfImage = FULL_SIZE_WIDTH;
  input.YSizeOfImage = FULL_SIZE_HEIGHT;

  /* set fixed threshold line */
  input.XKnotNumDefocusOKNG = X_KNOT;
  input.YKnotNumDefocusOKNG = Y_KNOT;
  for(i=0;i<POINT_NUM;i++) {
    conf[i] = 255;
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
  PdLibInterpretRegData(&regdata, &settings,
    sensor_lib_ptr.res_settings_array.reg_settings[s_pdaf->cur_res].reg_setting_a,
    sensor_lib_ptr.res_settings_array.reg_settings[s_pdaf->cur_res].size,
    s_pdaf->cur_res);

  /* get pd area and offset*/
  pd_area_x_offset = settings.pd_offset_x;
  pd_area_y_offset = settings.pd_offset_y;

  pd_area_width = settings.pd_area_w;
  pd_area_height = settings.pd_area_h;

  SHIGH("PDAF AREA:x_offset:%d,y_offset:%d, w:%d, h:%d",
    pd_area_x_offset,pd_area_y_offset,pd_area_width,pd_area_height);

  /* get defocus for each focus window */
  for(j = 0;j < pd_win_y;j++)
    for(i = 0;i < pd_win_x;i++) {
      /* restore to integer s27.4 from float */
      input.PhaseDifference =
        (signed long)(s_pdaf->defocus[i + pd_win_x*j].phase_diff * 16.0f);
      input.ConfidenceLevel
         = s_pdaf->defocus[i + pd_win_x*j].df_conf_level;

      if(PD_AREA_MODE == 2) {
        /* flexible window mode */
        input.XAddressOfWindowStart = settings.flex_win_addr[j * 4];
        input.YAddressOfWindowStart = settings.flex_win_addr[j * 4 + 1];
        input.XAddressOfWindowEnd = settings.flex_win_addr[j * 4 + 2];
        input.YAddressOfWindowEnd = settings.flex_win_addr[j * 4 + 3];
      } else {
        /* fixed window mode */
        input.XAddressOfWindowStart = pd_area_x_offset + pd_area_width * i;
        input.XAddressOfWindowEnd = pd_area_x_offset + pd_area_width * i
          + pd_area_width - 1;
        input.YAddressOfWindowStart = pd_area_y_offset + pd_area_height * j;
        input.YAddressOfWindowEnd = pd_area_y_offset + pd_area_height * j
          + pd_area_height - 1;
      }

     /* map PD window addr to full size */
     convert_pd_win_addr(&settings, &input.XAddressOfWindowStart,
       &input.YAddressOfWindowStart, &input.XAddressOfWindowEnd,
       &input.YAddressOfWindowEnd);

     SHIGH("PDAF INPUT[%d][%d]: window: X_STA %d X_END %d, Y_STA %d Y_END %d",
       i, j,
       input.XAddressOfWindowStart, input.XAddressOfWindowEnd,
       input.YAddressOfWindowStart, input.YAddressOfWindowEnd);

      if (get_defocus != NULL) {
        rc = get_defocus(&input, &output);
        if (rc != D_PD_LIB_E_OK) {
          SERR("PDAF LIB ERROR: D_PD_LIB_E_NG");
          return 0;
        }
      } else {
        s_pdaf->status = 2; /* 2 - error code for API unavailable */
        SERR("PDAF LIB ERROR: API unavailable");
        return 0;
      }
      /* set output */
      /* convert to 12 bit dac + sign bit: s12.0 by keeping two bits after
          decimal point; defocus in same sign of PD.
          Note that in this design, calibration is done in 10-bit DAC.
          The shifting (>>12) is for a 12-bit actuator for the given
          calibration data format. In case 10-bit actuator used with same
          calibration data format, (>>14)  should be shifted */
      s_pdaf->defocus[i + pd_win_x*j].defocus = output.Defocus >> 12;
      s_pdaf->defocus[i + pd_win_x*j].df_confidence =
        output.DefocusConfidence;
      s_pdaf->defocus[i + pd_win_x*j].df_conf_level =
        output.DefocusConfidenceLevel;

      SHIGH("PDAF OUTPUT win:%d, defocus: %d, conf level: %d, "
        "phase diff: %f, DCC: %f",
        i+pd_win_x*j,s_pdaf->defocus[i + pd_win_x*j].defocus,
        s_pdaf->defocus[i + pd_win_x*j].df_conf_level,
        s_pdaf->defocus[i + pd_win_x*j].phase_diff,
        (output.Defocus >> 14)/s_pdaf->defocus[i + pd_win_x*j].phase_diff);
  }

  s_pdaf->x_offset = pd_area_x_offset;
  s_pdaf->y_offset = pd_area_y_offset;
  s_pdaf->x_win_num = pd_win_x;
  s_pdaf->y_win_num = pd_win_y;
  s_pdaf->status = TRUE;

  return 0;
}

/**
 * FUNCTION: imx298_real_to_register_gain
 *
 * DESCRIPTION: Calcuate sensor analog gain register value
 **/
static unsigned int imx298_real_to_register_gain(float real_gain)
{
    unsigned int reg_gain = 0;

    if (real_gain < IMX298_MIN_AGAIN)
    {
        real_gain = IMX298_MIN_AGAIN;
        SHIGH("IMX298 Requested gain(%f) is lower than the sensor minimum(%d)",
            real_gain, IMX298_MIN_AGAIN);
    }
    else if (real_gain > IMX298_MAX_AGAIN)
    {
        real_gain = IMX298_MAX_AGAIN;
    }

    reg_gain = (unsigned int)(512.0 - (512.0 / real_gain));

    return reg_gain;
}

/**
 * FUNCTION: imx298_register_to_real_gain
 *
 * DESCRIPTION: Calcuate sensor real gain value
 **/
static float imx298_register_to_real_gain(unsigned int reg_gain)
{
    float gain;

    if(reg_gain > IMX298_MAX_AGAIN_REG_VAL)
        reg_gain = IMX298_MAX_AGAIN_REG_VAL;

    gain = 512.0 / (512.0 - reg_gain);

    return gain;
}

/**
 * FUNCTION: imx298_digital_gain_calc
 *
 * DESCRIPTION: Calcuate the sensor digital gain
 **/
static unsigned int imx298_digital_gain_calc(
 float real_gain, float sensor_real_gain)
{
    unsigned int reg_dig_gain = IMX298_MIN_DGAIN_REG_VAL;
    float real_dig_gain = IMX298_MIN_DGAIN;

    if(real_gain > IMX298_MAX_AGAIN)
    {
        real_dig_gain = real_gain / sensor_real_gain;
    }
    else
    {
        real_dig_gain = IMX298_MIN_DGAIN;
    }

    if(real_dig_gain > IMX298_MAX_DGAIN)
    {
        real_dig_gain = IMX298_MAX_DGAIN;
    }

    reg_dig_gain = (unsigned int)(real_dig_gain * 256);

    return reg_dig_gain;
}

/**
 * FUNCTION: sensor_calculate_exposure
 *
 * DESCRIPTION: Calcuate the sensor exposure
 **/
int sensor_calculate_exposure(float real_gain,
  unsigned int line_count, sensor_exposure_info_t *exp_info, float s_real_gain)
{
    if (!exp_info)
    {
      return -1;
    }

    exp_info->reg_gain = imx298_real_to_register_gain(real_gain);
    exp_info->sensor_real_gain =
      imx298_register_to_real_gain(exp_info->reg_gain);
    exp_info->sensor_digital_gain =
      imx298_digital_gain_calc(real_gain, exp_info->sensor_real_gain);
    exp_info->sensor_real_dig_gain =
      (float)exp_info->sensor_digital_gain / IMX298_MAX_DGAIN_DECIMATOR;
    exp_info->digital_gain =
      real_gain /(exp_info->sensor_real_gain * exp_info->sensor_real_dig_gain);
    exp_info->line_count = line_count;
    exp_info->s_reg_gain = imx298_real_to_register_gain(s_real_gain);

    return 0;
}

/**
 * FUNCTION: imx298_fill_exposure_array
 *
 * DESCRIPTION: Fill the sensor exposure array
 **/
int sensor_fill_exposure_array(unsigned int gain,
  unsigned int digital_gain, unsigned int line, unsigned int fl_lines,
  __attribute__((unused)) int luma_avg, unsigned int hdr_param,
  struct camera_i2c_reg_setting* reg_setting,
  __attribute__((unused)) unsigned int s_gain,
  __attribute__((unused)) int s_linecount,
  __attribute__((unused)) int is_hdr_enabled)
{
    unsigned short i = 0;
    unsigned short reg_count = 0;
    unsigned int luma_delta = hdr_param & 0x0000FFFF;

    if (!reg_setting)
    {
      return -1;
    }

    for (i = 0; i < sensor_lib_ptr.groupon_settings.size; i++) {
      reg_setting->reg_setting[reg_count].reg_addr =
        sensor_lib_ptr.groupon_settings.reg_setting_a[i].reg_addr;
      reg_setting->reg_setting[reg_count].reg_data =
        sensor_lib_ptr.groupon_settings.reg_setting_a[i].reg_data;
      reg_count = reg_count + 1;
    }

#ifdef ENABLE_SHORT_EXPOSURE
    /* HDR control */
    if(is_hdr_enabled)
    {
      reg_setting->reg_setting[reg_count].reg_addr =
          IMX298_SHORT_COARSE_INT_TIME_ADDR;
      reg_setting->reg_setting[reg_count].reg_data=(s_linecount & 0xFF00)>>8;
      reg_setting->reg_setting[reg_count].delay = 0;
      reg_count = reg_count + 1;

      reg_setting->reg_setting[reg_count].reg_addr =
          IMX298_SHORT_COARSE_INT_TIME_ADDR + 1;
      reg_setting->reg_setting[reg_count].reg_data = (s_linecount & 0xFF);
      reg_setting->reg_setting[reg_count].delay = 0;
      reg_count = reg_count + 1;

      reg_setting->reg_setting[reg_count].reg_addr =
          IMX298_SHORT_GAIN_ADDR;
      reg_setting->reg_setting[reg_count].reg_data = (s_gain & 0xFF00) >> 8;
      reg_setting->reg_setting[reg_count].delay = 0;
      reg_count = reg_count + 1;

      reg_setting->reg_setting[reg_count].reg_addr =
          IMX298_SHORT_GAIN_ADDR +  1;
      reg_setting->reg_setting[reg_count].reg_data = (s_gain & 0xFF);
      reg_setting->reg_setting[reg_count].delay = 0;
      reg_count = reg_count + 1;
    }
#else
    if(luma_delta != 0){
      unsigned int hdr_indoor_detected = (hdr_param >> 16) & 0x1;
      unsigned int ratio = 8;

      if(hdr_indoor_detected)
        ratio = 2;
      else
        ratio = 8;

      reg_setting->reg_setting[reg_count].reg_addr = IMX298_EXP_RATIO_ADDR;
      reg_setting->reg_setting[reg_count].reg_data = ratio;

      reg_count = reg_count + 1;
    }
#endif

    reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.output_reg_addr.frame_length_lines;
    reg_setting->reg_setting[reg_count].reg_data = (fl_lines & 0x0000FF00) >> 8;
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count = reg_count + 1;

    reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.output_reg_addr.frame_length_lines + 1;
    reg_setting->reg_setting[reg_count].reg_data = (fl_lines & 0x000000FF);
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count = reg_count + 1;

    reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info.coarse_int_time_addr;
    reg_setting->reg_setting[reg_count].reg_data = (line & 0x0000FF00) >> 8;
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count = reg_count + 1;

    reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info.coarse_int_time_addr + 1;
    reg_setting->reg_setting[reg_count].reg_data = (line & 0x000000FF);
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count = reg_count + 1;

    reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info.global_gain_addr;
    reg_setting->reg_setting[reg_count].reg_data = (gain & 0x0000FF00) >> 8;
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count = reg_count + 1;

    reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info.global_gain_addr + 1;
    reg_setting->reg_setting[reg_count].reg_data = (gain & 0x000000FF);
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count = reg_count + 1;

    reg_setting->reg_setting[reg_count].reg_addr = IMX298_DIG_GAIN_GR_ADDR;
    reg_setting->reg_setting[reg_count].reg_data = (digital_gain & 0x0000FF00) >> 8;
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count = reg_count + 1;

    reg_setting->reg_setting[reg_count].reg_addr = IMX298_DIG_GAIN_GR_ADDR + 1;
    reg_setting->reg_setting[reg_count].reg_data = (digital_gain & 0x000000FF);
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count = reg_count + 1;

    reg_setting->reg_setting[reg_count].reg_addr = IMX298_DIG_GAIN_R_ADDR;
    reg_setting->reg_setting[reg_count].reg_data = (digital_gain & 0x0000FF00) >> 8;
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count = reg_count + 1;

    reg_setting->reg_setting[reg_count].reg_addr = IMX298_DIG_GAIN_R_ADDR + 1;
    reg_setting->reg_setting[reg_count].reg_data = (digital_gain & 0x000000FF);
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count = reg_count + 1;

    reg_setting->reg_setting[reg_count].reg_addr = IMX298_DIG_GAIN_B_ADDR;
    reg_setting->reg_setting[reg_count].reg_data = (digital_gain & 0x0000FF00) >> 8;
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count = reg_count + 1;

    reg_setting->reg_setting[reg_count].reg_addr = IMX298_DIG_GAIN_B_ADDR + 1;
    reg_setting->reg_setting[reg_count].reg_data = (digital_gain & 0x000000FF);
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count = reg_count + 1;

    reg_setting->reg_setting[reg_count].reg_addr = IMX298_DIG_GAIN_GB_ADDR;
    reg_setting->reg_setting[reg_count].reg_data = (digital_gain & 0x0000FF00) >> 8;
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count = reg_count + 1;

    reg_setting->reg_setting[reg_count].reg_addr = IMX298_DIG_GAIN_GB_ADDR + 1;
    reg_setting->reg_setting[reg_count].reg_data = (digital_gain & 0x000000FF);
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count = reg_count + 1;

    for (i = 0; i < sensor_lib_ptr.groupoff_settings.size; i++) {
      reg_setting->reg_setting[reg_count].reg_addr =
        sensor_lib_ptr.groupoff_settings.reg_setting_a[i].reg_addr;
      reg_setting->reg_setting[reg_count].reg_data =
        sensor_lib_ptr.groupoff_settings.reg_setting_a[i].reg_data;
      reg_count = reg_count + 1;
    }

    reg_setting->size = reg_count;
    reg_setting->addr_type = CAMERA_I2C_WORD_ADDR;
    reg_setting->data_type = CAMERA_I2C_BYTE_DATA;
    reg_setting->delay = 0;

    return 0;
}

/**
 * FUNCTION: sensor_fill_awb_array
 *
 * DESCRIPTION: Fill the AWB HDR array
 **/
static int sensor_fill_awb_array(unsigned short awb_gain_r,
  unsigned short awb_gain_b, struct camera_i2c_seq_reg_setting* reg_setting)
{
    unsigned short reg_count = 0;

    reg_setting->reg_setting[reg_count].reg_addr = IMX298_ABS_GAIN_R_WORD_ADDR;
    reg_setting->reg_setting[reg_count].reg_data[0] = (awb_gain_r & 0xFF00) >> 8;
    reg_setting->reg_setting[reg_count].reg_data[1] = (awb_gain_r & 0xFF);
    reg_setting->reg_setting[reg_count].reg_data_size = 2;
    reg_count = reg_count + 1;

    reg_setting->reg_setting[reg_count].reg_addr = IMX298_ABS_GAIN_B_WORD_ADDR;
    reg_setting->reg_setting[reg_count].reg_data[0] = (awb_gain_b & 0xFF00) >> 8;
    reg_setting->reg_setting[reg_count].reg_data[1] = (awb_gain_b & 0xFF);
    reg_setting->reg_setting[reg_count].reg_data_size = 2;
    reg_count = reg_count + 1;

    reg_setting->size = reg_count;
    reg_setting->addr_type = CAMERA_I2C_WORD_ADDR;
    reg_setting->delay = 0;

    return 0;
}

/**
 * FUNCTION    - parse_VHDR_stats -
 *
 * DESCRIPTION: VHDR statistic parser
 **/
static int parse_VHDR_stats(unsigned int *destLumaBuff, void *rawBuff)
{
  unsigned char       *buf = NULL;
  unsigned int      temp_y = 0;
  int       i;
  unsigned int      count = 0;
  unsigned char       bit_mask = 0;
  unsigned char       bit_shift = 0;
  unsigned char       hdr_indoor = 0;

  if (destLumaBuff == NULL || rawBuff == NULL) {
    SERR("%s: Invalid HDR Stats buffers %p %p!",
      __func__, destLumaBuff, rawBuff);
    return -1;
  }

  buf = (unsigned char *)rawBuff;

  /* HDR stats is limited to 10 bits from sensor for all ratios */
  bit_mask = VIDEO_HDR_LUMA_MASK_RATIO_1;
  bit_shift = VIDEO_HDR_EXPOSURE_RATIO_CORRECTION_1;

  /* Parse the stats only if camera is started and AF is stopped */

  /* Stats data format:
   * A pair of pixel data (10 bit each) is used for transmission of
   * each sub-block 14 bit data.
   * |y(0,0) U |y(0,0) L | y(0,1)U | y(0,1)L| ... | y(15,15)U | y(15,15)L |
   * To get 14 bit y-avg we need to combine Upper and Lower pixel:
   * Upper: D13 - D12 - D11 - D10 - D09 - D08 - D07 - D06 - 0 - 1
   * Lower: D05 - D04 - D03 - D02 - D01 - D00 - 0   -   1 - 0 - 1
   * 14-bit final data:
   * D13 - D12 - D11 - D10 - D09 - D8 - D7 - D6 - D5 - D4 - D3 - D2 - D1 -D0
   **/

  /* We have 16x16 blocks
   * Each block is 20 bits, so 256 blocks * 20 bits is 5120 bits = 640 bytes.
   * We will process 5 bytes (2 luma values) on each iteration, so we need
   * 128 iterations
   **/
  for (i = 0; i < 128; i++) {
    /* Each pixel is 10 bits. And each stat data has pair of
     * pixel - 20 bits. So for each Y we have 2 and a half bytes */

    /* take only the bits specified by the mask */
    temp_y = buf[0] & bit_mask;
    temp_y = temp_y << 6;
    temp_y |= (buf[1] >> 2);
    SLOW("%s: Value obtained for block %d: %d", __func__, i*2, temp_y);
    /* Shift at least 2 bits to make 10bit data to 8bit,
     * as AEC algorithm only takes 8bit data */
    destLumaBuff[count++] = temp_y >> bit_shift;
    /* Now calculate the next pair */
    temp_y = 0;

    temp_y = buf[2] & bit_mask;
    temp_y = temp_y << 6;
    temp_y |= (buf[3] >> 2);
    SLOW("%s: Value obtained for block %d: %d", __func__, i*2+1, temp_y);
    /* Shift at least 2 bits to make 10bit data to 8bit,
     * as AEC algorithm only takes 8bit data */
    destLumaBuff[count++] = temp_y >> bit_shift;
    /* now advance the buf pointer */
    buf += 5;
  }
  return 0;
}

/**
 * FUNCTION: sensor_open_lib
 *
 * DESCRIPTION: Open sensor library and returns data pointer
 **/
void *sensor_open_lib(void)
{
  return &sensor_lib_ptr;
}
