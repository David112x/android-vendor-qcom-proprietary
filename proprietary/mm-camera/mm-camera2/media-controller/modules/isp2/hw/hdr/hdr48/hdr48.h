/* hdr48.h
 *
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __hdr48_H__
#define __hdr48_H__

/* mctl headers */
#include "chromatix.h"
#include "chromatix_common.h"

/* isp headers */
#include "hdr_reg.h"
#include "isp_sub_module_common.h"


typedef boolean (*get_2d_interpolation)(void *, void *);
typedef void (*apply_2d_interpolation)(void *);

typedef struct {
  get_2d_interpolation get_2d_interpolation;
  apply_2d_interpolation apply_2d_interpolation;
} ext_override_func;

/** hdr_cfg_cmd_t:
*
*  Configure hdr config
*
 **/
typedef struct {
  ISP_HdrCfg0 cfg0;
  ISP_HdrCfg1 cfg1;
  ISP_HdrCfg2 cfg2;
  ISP_HdrCfg3 cfg3;
  ISP_HdrCfg4 cfg4;
} hdr_cfg_cmd_t;

/** hdr_cfg_2_cmd_t:
*
*  Configure hdr config
*
 **/
typedef struct {
  ISP_HdrCfg5 cfg5;
} hdr_cfg_2_cmd_t;

/** hdr_recon_cmd_t:
*
*  Configure hdr recon config
*
 **/
typedef struct {
  ISP_HdrReconCfg0 recon_cfg0;
  ISP_HdrReconCfg1 recon_cfg1;
  ISP_HdrReconCfg2 recon_cfg2;
  ISP_HdrReconCfg3 recon_cfg3;
  ISP_HdrReconCfg4 recon_cfg4;
} hdr_recon_cmd_t;

/** hdr_recon2_cmd_t:
*
*  Configure hdr recon config
*
 **/
typedef struct {
  ISP_HdrReconCfg5 recon_cfg5;
  ISP_HdrReconCfg6 recon_cfg6;
} hdr_recon2_cmd_t;

/** hdr_mac_cmd_t:
*
*  Configure hdr mac config
*
 **/
typedef struct {
  ISP_HdrMacCfg0 mac_cfg0;
  ISP_HdrMacCfg1 mac_cfg1;
  ISP_HdrMacCfg2 mac_cfg2;
  ISP_HdrMacCfg3 mac_cfg3;
  ISP_HdrMacCfg4 mac_cfg4;
  ISP_HdrMacCfg5 mac_cfg5;
  ISP_HdrMacCfg6 mac_cfg6;
  ISP_HdrMacCfg7 mac_cfg7;
} hdr_mac_cmd_t;

/** hdr_reg_cmd_t:
*
*  Configure all hdr registers
*
 **/
typedef struct {
  hdr_cfg_cmd_t cfg_cmd;
  hdr_cfg_2_cmd_t cfg_cmd2;
  hdr_recon_cmd_t recon_cmd;
  hdr_recon2_cmd_t recon_cmd2;
  hdr_mac_cmd_t mac_cmd;
} hdr_reg_cmd_t;

typedef struct
{
    trigger_point2_type aec_sensitivity_ratio;          // For CONTROL_AEC_EXP_SENSITIVITY_RATIO
    trigger_point2_type exp_time_trigger;           // For CONTROL_EXP_TIME_RATIO
    // RECON parameters
    float      recon_min_factor;           // 5uQ4, range 0-31. Scaling factor to force vertical direction: if 16*d_min>min_factor*d_sec_min, force vertical edge direction
    float      recon_flat_region_th;       // 10u. Threshold for flat region. If d_max<th, consider as flat region and force vertical edge direction.
    float      recon_h_edge_th1;           // 10u. Th1 for horizontal edge switching function.
    float      recon_motion_th1;           // 10u. Th1 for motion switching function.
    float      recon_dark_th1;             // 10u. Th1 for dark noise switching function.

    // zzHDR parameters
    float      hdr_zrec_prefilt_tap0;      //6u. Zigzag HDR prefilter side-taps (0~64)  - default: 16
    float      hdr_zrec_g_grad_th1;        //12u. Zigzag HDR G gradient threshold_1   - default: 32
    float      hdr_zrec_rb_grad_th1;       //12u. Zigzag HDR R/B gradient threshold_1,   - default: 32
    // MAC parameters
    float      mac_motion0_th1;            // 10u. Noise floor value for motion detection. Noise model is: noise = th2*sqrt(gain*itensity)+th1.
    float      mac_motion0_th2;            // 8u. Noise-luma slope for motion detection.
    float      mac_motion_strength;        // 5uQ4, range 0-16. Scaling factor for motion adaptation strength in MAC.
    float      mac_low_light_th1;          // 14u. Th1 for low light switching.
    float      mac_low_light_strength;     // 5uQ4, range 0-16. Scaling factor for low light switching.
    float      mac_high_light_th1;         // 14u. Th1 for high light switching.
    float      mac_high_light_dth_log2;    // 4u, range 2-14. log2(th2-th1) for high light switching.
} chromatix_videoHDR_float_core;


typedef struct {
  /* Module Params*/
    float                                   hdr_th1_adj;
    float                                   hdr_th2_adj;
    float                                   hdr_strength_adj;
    float                                   hdr_recon_h_edge_th1_adj;
    float                                   hdr_recon_motion_th1_adj;
    float                                   hdr_recon_dark_th1_adj;
} hdr48_ext_t;

/** hdr48_t:
 *
 *  @lux_idx: aec lux value
 *  @real_gain: aec gain value
 *  @hdr_exp_ratio: aec exposure ratio
 *  @exp_time: aec exposure time
 *  @color_temp: awb cct type
 *  @awb_gain: awb rgb gain
 *  @rgn_idx: current region
 *  @is_3d_mode_on: 3d mode
 *  @interp_data: interpolated data to be written to registers
 *  @reg_cmd: reg cmd to be upsated to hw
 **/
typedef struct {
  float real_gain;
  float lux_gain;
  float hdr_sensitivity_ratio;
  float hdr_exp_time_ratio;
  awb_gain_t awb_gain;
  boolean is_3d_mode_on;
  uint32_t BLSS_offset;
  uint32_t BLSS_out;
  float multfactor;
  chromatix_videoHDR_float_core interp_data;
  ext_override_func        *ext_func_table;
  hdr_reg_cmd_t reg_cmd;
  uint32_t apply_hdr_effects;
  hdr48_ext_t              hdr48_ext_params;
} hdr48_t;

/* funtion declaration */
boolean hdr48_set_hdr_mode(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean hdr48_init(isp_sub_module_t *isp_sub_module);

void hdr48_destroy(isp_sub_module_t *isp_sub_module);

boolean hdr48_streamoff(isp_sub_module_t *isp_sub_module, void *data);

boolean hdr48_streamon(isp_sub_module_t *isp_sub_module, void *data);

boolean hdr48_set_chromatix_ptr(isp_sub_module_t *isp_sub_module,
  void *data);

boolean hdr48_stats_aec_update(isp_sub_module_t *isp_sub_module,
  void *data);

boolean hdr48_stats_awb_update(isp_sub_module_t *isp_sub_module,
  void *data);

boolean hdr48_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module,
  void *data,
  uint32_t identity);

#if OVERRIDE_FUNC
boolean hdr48_fill_func_table_ext(hdr48_t *hdr);
#define FILL_FUNC_TABLE(field) hdr48_fill_func_table_ext(field)
#else
boolean hdr48_fill_func_table(hdr48_t *hdr);
#define FILL_FUNC_TABLE(field) hdr48_fill_func_table(field)
#endif

#endif /* __hdr48_H__ */
