/*======================================================================
Copyright (c) 2015 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
=======================================================================*/
#ifndef _PDAF_API_H
#define _PDAF_API_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "pdaf_lib.h"

/*defocus confidence*/
#define PDAF_LIB_E_OK                 (0)
#define PDAF_LIB_E_NG                 (-1)

typedef struct
{
  pdaf_vendor_t  vendor_id;
  unsigned int   major_version;
  unsigned int   minor_version;
  unsigned int   tiny_revision;
}pdaf_lib_version_t;

/* input for init */
typedef struct
{
  /*pdaf pattern, location of pdaf pixels*/
  pdaf_sensor_native_info_t  native_pattern_info;
  /*sensor black level*/
  unsigned int               black_level;
  unsigned int               pixel_depth;
  unsigned int               is_hdr;
  unsigned int               PD_pix_overflow_threshold;
  short                      macro_dac;
  short                      infinity_dac;
  float                      actuator_sensitivity;
  unsigned short             cur_logical_lens_pos;
  /*calibration type*/
  pdaf_calibration_version_t cali_version;
  pdaf_pixel_order_type_t   pixel_order_type;
  /* pointer to calibration parameters from sensor*/
  void                       *p_calibration_para;
  pdaf_data_buffer_info_t   buffer_data_info;
  PDLibTuning               PDLibTuningParam;
} pdaf_init_param_t;

/* input for defocus */
typedef struct
{
  /*pdaf left and right image, with size info*/
  pdaf_camif_output_data_t  camif_out;
  /*support fixed window configuration */
  /*and floating window configuration */
  pdaf_window_configure_t   window_configure;
  /* image sensor gain */
  unsigned int              image_analog_gain;
  /* threshold parameter */
  int                       defocus_confidence_th;
  /* real-time FPS */
  double                    fps;
  float                     integration_time;    /* in seconds */
  short                     lens_position;       /* in DAC */
  unsigned short            cur_logical_lens_pos;
  /*reserved*/
  void                      *p_tunnable;
  unsigned char             *p_pdaf_input;
} pdaf_param_t;

/* output for defocus */
typedef struct
{
  pdaf_lib_defocus_t defocus[MAX_PDAF_WINDOW];
  unsigned short is_peripheral_valid;
  pdaf_lib_defocus_t result_peripheral[MAX_PERIPHERAL_WINDOW];
  unsigned short is_single_valid;
  pdaf_lib_defocus_t result_single[MAX_SINGLE_WINDOWS];
  /* number of grids, output data arranged in grid row sequence*/
  /* for floating window, output data arranged in defining order*/
  int               window_number;
  pdaf_processing_result_t   processing_result;
} pdaf_output_data_t;

/*return lib version*/
void PDAF_get_lib_version(pdaf_lib_version_t *p_version);

/*get lib handle, call per camera session*/
PD_HANDLE PDAF_PhaseDetection_init(pdaf_init_param_t *p_init_param);

/*processing function, call per frame*/
PDAF_RET_t PDAF_PhaseDetection(PD_HANDLE handle,
    pdaf_param_t *p_param,
    unsigned short num_of_lines,
    pdaf_output_data_t *p_result);

/*deinit, call per camera session*/
PDAF_RET_t PDAF_PhaseDetection_deinit(PD_HANDLE handle);
#endif
