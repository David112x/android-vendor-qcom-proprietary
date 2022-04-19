/* scaler40.h
 *
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __SCALER40_H__
#define __SCALER40_H__

/* isp headers */
#include "module_scaler40.h"

boolean scaler40_init(scaler40_t *mod);

int scaler40_destroy(void *mod_ctrl);

boolean scaler40_handle_isp_private_event(scaler40_t *scaler,
  void *data, uint32_t identity);

void scaler40_calculate_dis(scaler40_t *scaler);
boolean scaler40_config(scaler40_t *scaler);

void scaler40_reset(scaler40_t *mod);

boolean scaler40_trigger_update(scaler40_t *mod);

boolean scaler40_handle_set_chromatix_ptr(scaler40_t *scaler,
  mct_event_t *event);

boolean scaler40_parm_zoom(scaler40_t *scaler, cam_zoom_info_t *zoom_val);

#endif //__SCALER40_H__
