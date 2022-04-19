/*======================================================================
Copyright (c) 2015 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
=======================================================================*/

#ifndef _PDAF_CAMIF_API_H
#define _PDAF_CAMIF_API_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "pdaf_lib.h"

typedef struct{
  unsigned int major_version;
  unsigned int minor_version;
  unsigned int tiny_revision;
}pdaf_camif_lib_version_t;

/* input for init */
typedef struct{
  /* sensor native info */
  pdaf_sensor_native_info_t sensor_native_info;
  /* data buffer info */
  pdaf_data_buffer_info_t   buffer_data_info;
} pdaf_camif_init_param_t;

/* input for defocus */
typedef struct{
  /* camif input data info */
  /* for type3 pdaf, pointer to downsampled camif pdaf buffer */
  /* for type2 pdaf, pointer to pdaf stats */
  unsigned char           *p_left;
  /* reserved, set to NULL */
  unsigned char           *p_right;
  /*support fixed window configuration */
  /*and floating window configuration */
  pdaf_window_configure_t  window_configure;
  /*Logical lens position to dump the raw */
  /*file according to the lens position */
  unsigned short           cur_logical_lens_pos;
} pdaf_camif_param_t;

/* return lib version info */
/* printf feature in log */
void PDAF_CAMIF_get_lib_version(pdaf_camif_lib_version_t *p_version);

/* get lib handle, call per camera session */
PD_CAMIF_HANDLE PDAF_CAMIF_init(pdaf_camif_init_param_t *p_init_param);

/* get pdaf left and right images for the whole frame */
/* call per frame*/
PDAF_RET_t PDAF_CAMIF_getPDAF(PD_CAMIF_HANDLE handle,
  pdaf_camif_param_t *p_param,
  pdaf_camif_output_data_t * p_out);

/*deinit, call per camera session*/
PDAF_RET_t PDAF_CAMIF_deinit(PD_CAMIF_HANDLE handle);

#endif
