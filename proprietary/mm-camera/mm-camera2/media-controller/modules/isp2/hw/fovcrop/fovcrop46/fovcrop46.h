/* fovcrop46.h
 *
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __FOVCROP46_H__
#define __FOVCROP46_H__

/* isp headers */
#include "module_fovcrop46.h"

boolean fovcrop46_init(fovcrop46_t *mod);

int fovcrop46_destroy(void *mod_ctrl);

boolean fovcrop46_handle_isp_private_event(mct_module_t *module,
  fovcrop46_t *fovcrop, void *data, uint32_t identity);

boolean fovcrop46_config(fovcrop46_t *fovcrop);

void fovcrop46_reset(fovcrop46_t *mod);

boolean fovcrop46_trigger_update(fovcrop46_t *mod);

void fov46_reset(fovcrop46_t *fovcrop);

boolean fovcrop46_config_crop_window(fovcrop46_t *fovcrop);

#endif //__FOVCROP46_H__
