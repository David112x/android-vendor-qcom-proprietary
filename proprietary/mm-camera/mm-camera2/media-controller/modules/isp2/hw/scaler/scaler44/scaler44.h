/* scaler44.h
 *
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __SCALER44_H__
#define __SCALER44_H__

/* isp headers */
#include "module_scaler44.h"

boolean scaler44_init(scaler44_t *mod);

boolean scaler44_handle_isp_private_event(mct_module_t *module,
  scaler44_t *scaler, void *data, uint32_t identity);

void scaler44_calculate_dis(scaler44_t *scaler);
boolean scaler44_config(scaler44_t *scaler);

void scaler44_reset(scaler44_t *mod);

boolean scaler44_trigger_update(scaler44_t *mod);

boolean scaler44_handle_set_chromatix_ptr(scaler44_t *scaler,
  mct_event_t *event);

boolean scaler44_parm_zoom(mct_module_t *module, scaler44_t *scaler,
  cam_zoom_info_t *zoom_val, uint32_t identity);

boolean scaler44_parm_crop_region(mct_module_t *module, scaler44_t *scaler,
  cam_crop_region_t  *crop_window, uint32_t identity);

#endif //__SCALER44_H__
