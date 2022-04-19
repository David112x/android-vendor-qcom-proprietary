
/* sensor_pick_res.h
 *
 * Copyright (c) 2014-2016 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __SENSOR_PICK_RES_H__
#define __SENSOR_PICK_RES_H__

typedef enum {
  SEN_USECASE_FASTAEC,
  SEN_USECASE_QUADRA,
  SEN_USECASE_HFR,
  SEN_USECASE_IHDR,/*in-sensor HDR*/
  SEN_USECASE_RHDR,/*raw HDR*/
  SEN_USECASE_VHDR,/*in-sensor video HDR*/
  SEN_USECASE_SNAP,
  SEN_USECASE_VP,
  SEN_USECASE_MAX,
} sensor_usecase_t;

#define SEN_USECASE_DEFAULT SEN_USECASE_VP

typedef enum {
  SEN_COND_FPS,
  SEN_COND_BOUNDED_FPS,
  SEN_COND_ASPR,
  SEN_COND_W,
  SEN_COND_H,
  SEN_COND_CLK,
  SEN_COND_MODE_QUADRA,
  SEN_COND_MODE_HFR,
  SEN_COND_MODE_DEF,
  SEN_COND_MODE_IHDR,
  SEN_COND_MODE_RHDR,
  SEN_COND_MODE_MPIX,
  SEN_COND_MODE_BEST_RES,
  SEN_COND_MAX,
} sensor_pick_condition_t;

typedef struct {
  uint32_t stream_mask;
  uint32_t hfr_mode;
  uint32_t is_quadra_mode;
  int32_t in_sensor_hdr_enable;
  int32_t raw_hdr_enable;
  boolean is_fast_aec_mode_on;
} sensor_pick_usecase_t;

typedef struct {
  uint16_t ex_w;
  uint16_t ex_h;
  uint16_t in_w;
  uint16_t in_h;
  uint16_t in_mode;
  uint16_t ex_mode;
  uint32_t last_mpix;
  uint32_t tmp_mpix;
  uint16_t last_ar;
  uint16_t tmp_ar;
  float in_fps;
  float ex_fps;
  uint32_t ex_clk;
  uint32_t in_clk;
  int32_t ex_ar;
  int32_t in_ar;
} sensor_pick_data_t;

typedef boolean (*is_usecase_func_t[SEN_USECASE_MAX])(sensor_pick_usecase_t *sctrl);
typedef boolean (*check_func_t[SEN_COND_MAX])(sensor_pick_data_t *pick_data);

typedef struct {
  is_usecase_func_t      *func;
  check_func_t           *check;
  uint32_t              (*pick)[][SEN_USECASE_MAX];
  cam_hal_version_t      hal_version;
  sensor_pick_data_t     pick_data;
  sensor_pick_usecase_t  pick_usecase;
  sensor_set_res_cfg_t   res_cfg;
  sensor_usecase_t       usecase;
  uint16_t               mode_array_size;
}sensor_pick_dev_t;


boolean sensor_pick_check_mode(sensor_pick_data_t *pick_data);
boolean sensor_pick_check_pixclk(sensor_pick_data_t *pick_data);
boolean sensor_pick_check_height(sensor_pick_data_t *pick_data);
boolean sensor_pick_check_width(sensor_pick_data_t *pick_data);
boolean sensor_pick_check_mpix(sensor_pick_data_t *pick_data);
boolean sensor_pick_check_best_res(sensor_pick_data_t *pick_data);
boolean sensor_pick_check_aspect_ratio(sensor_pick_data_t *pick_data);
boolean sensor_pick_check_fps(sensor_pick_data_t *pick_data);
boolean sensor_pick_check_bounded_fps(sensor_pick_data_t *pick_data);

boolean sensor_pick_res_is_hfr(sensor_pick_usecase_t *sctrl);
boolean sensor_pick_res_is_ihdr(sensor_pick_usecase_t *sctrl);/*in-sensor HDR*/
boolean sensor_pick_res_is_vhdr(sensor_pick_usecase_t *sctrl);/*in-sensor video HDR*/
boolean sensor_pick_res_is_rhdr(sensor_pick_usecase_t *sctrl);/*raw HDR*/
boolean sensor_pick_res_is_video_preview(sensor_pick_usecase_t *sctrl);
boolean sensor_pick_res_is_snapshot(sensor_pick_usecase_t *sctrl);
boolean sensor_pick_res_is_fast_aec(sensor_pick_usecase_t *sctrl);
boolean sensor_pick_res_is_quadra(sensor_pick_usecase_t *sctrl);

int32_t sensor_pick_resolution(void *sctrl,
  sensor_set_res_cfg_t *res_cfg, int32_t *pick_res);

#endif /* __SENSOR_PICK_RES_H__ */

